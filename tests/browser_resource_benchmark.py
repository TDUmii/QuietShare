"""Owned-process resource benchmark. Synthetic local feed, never personal tabs.

CPU is percent of one logical core. Working set can double-count shared pages.
Private bytes are committed process memory, not GPU memory or physical RAM.
Run the same script and fixtures against each binary before comparing results.
"""
import argparse,ctypes,hashlib,http.server,json,os,statistics,subprocess,threading,time,uuid
from ctypes import wintypes as W
from pathlib import Path
import production_integration as api

K=ctypes.WinDLL('kernel32',use_last_error=True);P=ctypes.WinDLL('psapi',use_last_error=True)
class Entry(ctypes.Structure):
    _fields_=[('size',W.DWORD),('usage',W.DWORD),('pid',W.DWORD),('heap',ctypes.c_size_t),('module',W.DWORD),('threads',W.DWORD),('parent',W.DWORD),('priority',W.LONG),('flags',W.DWORD),('exe',W.WCHAR*260)]
class Memory(ctypes.Structure):
    _fields_=[('cb',W.DWORD),('faults',W.DWORD)]+[(n,ctypes.c_size_t) for n in ('peak','working','poolPeak','pool','nonPoolPeak','nonPool','pagefile','peakPagefile','private')]
K.CreateToolhelp32Snapshot.argtypes=[W.DWORD,W.DWORD];K.CreateToolhelp32Snapshot.restype=W.HANDLE
K.Process32FirstW.argtypes=K.Process32NextW.argtypes=[W.HANDLE,ctypes.POINTER(Entry)]
K.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];K.OpenProcess.restype=W.HANDLE
K.CloseHandle.argtypes=[W.HANDLE]
K.GetProcessTimes.argtypes=[W.HANDLE,*([ctypes.POINTER(W.FILETIME)]*4)]
P.GetProcessMemoryInfo.argtypes=[W.HANDLE,ctypes.POINTER(Memory),W.DWORD]
def process_ids(root):
    snap=K.CreateToolhelp32Snapshot(2,0);e=Entry();e.size=ctypes.sizeof(e);parents={}
    try:
        ok=K.Process32FirstW(snap,ctypes.byref(e))
        while ok:parents[e.pid]=e.parent;ok=K.Process32NextW(snap,ctypes.byref(e))
    finally:K.CloseHandle(snap)
    owned={root}
    while True:
        more={p for p,parent in parents.items() if parent in owned}-owned
        if not more:return owned
        owned|=more
def usage(pid):
    h=K.OpenProcess(0x410,False,pid)
    if not h:return None
    try:
        m=Memory();m.cb=ctypes.sizeof(m);times=[W.FILETIME() for _ in range(4)]
        if not P.GetProcessMemoryInfo(h,ctypes.byref(m),m.cb) or not K.GetProcessTimes(h,*[ctypes.byref(t) for t in times]):return None
        seconds=sum((t.dwHighDateTime<<32)|t.dwLowDateTime for t in times[2:])/1e7
        return seconds,m.private,m.working
    finally:K.CloseHandle(h)
class Feed(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        # Busy title changes exercise native event handling alongside a retained
        # DOM, a modest JS heap and animated canvas. All data is synthetic.
        html=b'''<!doctype html><meta charset=utf-8><title>Feed</title><textarea id=draft>UNSENT FIXTURE</textarea><canvas width=800 height=450></canvas><main></main><script>
        window.keep=new Uint8Array(16*1024*1024);for(let i=0;i<keep.length;i+=4096)keep[i]=i%255;
        let main=document.querySelector('main');for(let i=0;i<3000;i++){let a=document.createElement('article');a.style='padding:12px;border-bottom:1px solid';a.innerHTML='<h3>Local feed '+i+'</h3><p>Synthetic content for a bounded browser benchmark.</p>';main.append(a)}
        let n=0,c=document.querySelector('canvas').getContext('2d');setInterval(()=>{document.title='Feed '+(++n);c.fillStyle=n%2?'#234':'#456';c.fillRect(0,0,800,450)},20);
        </script>'''
        self.send_response(200);self.send_header('Content-Type','text/html');self.send_header('Content-Length',str(len(html)));self.end_headers();self.wfile.write(html)
    def log_message(self,*args):pass
def run(exe,seconds,verify=False):
    root=api.ROOT/'artifacts/qa-browser-resources'/uuid.uuid4().hex;root.mkdir(parents=True)
    profile=root/'User';profile.mkdir();env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Feed);threading.Thread(target=server.serve_forever,daemon=True).start()
    app=None;stages={};quick_selection_kept=None
    try:
        assert not api.native.app_windows(),'A personal QuietShare session is open'
        app=subprocess.Popen([str(exe)],cwd=str(exe.parent),env=env)
        h=api.wait_for(lambda:next(iter(api.native.app_windows(app.pid)),None),10);assert h
        assert api.wait_for(lambda:api.send(h,api.GATE,timeout=6000),20),'Actual WGC verification failed'
        def q(n):return api.send(h,api.QUERY,n)
        def cmd(n):api.USER32.PostMessageW(h,api.WM_COMMAND,n,0);time.sleep(.15)
        def measure(name):
            time.sleep(1);layouts=q(85);before={p:usage(p) for p in process_ids(app.pid)};start=time.monotonic();samples=[]
            while time.monotonic()-start<seconds:
                time.sleep(.25);values={p:usage(p) for p in process_ids(app.pid)}
                samples.append({p:v for p,v in values.items() if v})
            elapsed=time.monotonic()-start;last=samples[-1]
            cpu={p:max(0,v[0]-(before[p][0] if before.get(p) else 0))/elapsed*100 for p,v in last.items()}
            stages[name]={'hostCpuOneCorePercent':cpu.get(app.pid,0),'webCpuOneCorePercent':sum(v for p,v in cpu.items() if p!=app.pid),
                'privateMiB':statistics.median(sum(v[1] for v in s.values())/1048576 for s in samples),
                'workingSetMiB':statistics.median(sum(v[2] for v in s.values())/1048576 for s in samples),
                'hostPrivateMiB':statistics.median(s.get(app.pid,(0,0,0))[1]/1048576 for s in samples),
                'processCount':len(last),'tabs':q(34),'backgroundSuspended':q(36),'activeSuspended':q(15),
                'nativeLayoutsPerSecond':(q(85)-layouts)/elapsed,
                'readerVisible':api.visible(api.USER32.GetDlgItem(h,110)),
                'wgcVerified':bool(api.send(h,api.GATE,timeout=6000))}
            print(name,json.dumps(stages[name]),flush=True)
        measure('home')
        cmd(154);assert api.native.set_control_text(api.USER32.GetDlgItem(h,164),f'http://127.0.0.1:{server.server_port}/1');cmd(155)
        assert api.wait_for(lambda:q(10)==2 and q(13)==1,25)
        measure('oneFeed')
        for n in range(2,5):
            cmd(190);assert api.wait_for(lambda:q(10)==2 and q(37)==0,25)
            assert api.wait_for(lambda:q(13)==1 and api.native.control_text(api.USER32.GetDlgItem(h,164))=='',15)
            target=f'http://127.0.0.1:{server.server_port}/{n}'
            assert api.native.set_control_text(api.USER32.GetDlgItem(h,164),target);cmd(165)
            assert api.wait_for(lambda:q(13)==1 and api.native.control_text(api.USER32.GetDlgItem(h,164))==target,20)
        measure('fourFeeds')
        cmd(127);measure('fourFeedsHiddenByReader')
        cmd(154);assert api.wait_for(lambda:q(10)==2,10);measure('fourFeedsResumed')
        if verify:
            cmd(280);lst=api.USER32.GetDlgItem(h,282)
            assert api.wait_for(lambda:api.visible(lst),5)
            api.send(lst,0x186,1);time.sleep(.6)
            quick_selection_kept=api.send(lst,0x188)==1
            cmd(284)
        cmd(155);assert api.wait_for(lambda:q(10)==0,10);measure('browserDisabled')
    finally:
        server.shutdown();server.server_close()
        if app and app.poll() is None:
            for h in api.native.app_windows(app.pid):api.USER32.PostMessageW(h,0x10,0,0)
            try:app.wait(timeout=10)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
    return {'exeSha256':hashlib.sha256(exe.read_bytes()).hexdigest(),'secondsPerStage':seconds,'stages':stages,'quickSelectionKept':quick_selection_kept,'syntheticLocalFeed':True,'physicalProjection':False}
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--exe',type=Path,default=api.EXE);ap.add_argument('--seconds',type=float,default=6);ap.add_argument('--output',type=Path,required=True);ap.add_argument('--check',action='store_true');args=ap.parse_args()
    assert 3<=args.seconds<=30
    result=run(args.exe.resolve(),args.seconds,args.check)
    if args.check:
        s=result['stages'];checks={
            'actual WGC remains verified in every workload':all(v['wgcVerified'] for v in s.values()),
            'one busy page has bounded nonstarving native updates':0<s['oneFeed']['nativeLayoutsPerSecond']<=12,
            'four busy pages have bounded nonstarving native updates':0<s['fourFeeds']['nativeLayoutsPerSecond']<=12,
            'hidden web metadata does not relayout the active reader':s['fourFeedsHiddenByReader']['nativeLayoutsPerSecond']==0 and s['fourFeedsHiddenByReader']['readerVisible'],
            'resuming web restores bounded native updates':0<s['fourFeedsResumed']['nativeLayoutsPerSecond']<=12,
            'all four controllers remain represented as tabs':all(s[n]['tabs']==4 for n in ('fourFeeds','fourFeedsHiddenByReader','fourFeedsResumed')),
            'busy web updates preserve quick switch selection':result['quickSelectionKept'] is True,
            'disabling browsing releases every tab and owned child process':s['browserDisabled']['tabs']==0 and s['browserDisabled']['processCount']==1,
            'offline Home starts with no web processes':s['home']['tabs']==0 and s['home']['processCount']==1}
        result.update(checks=checks,passed=sum(checks.values()),failed=sum(not v for v in checks.values()))
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(result,indent=2),encoding='utf-8')
    if args.check:
        print(json.dumps(result['checks']),flush=True)
        raise SystemExit(bool(result['failed']))
