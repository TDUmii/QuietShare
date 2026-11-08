"""Owned production process, local dynamic page and isolated workspace profile."""
import ctypes
import hashlib
import http.server
import json
import os
import subprocess
import threading
import time
import uuid
import production_integration as api

U,send=api.USER32,api.send
class Feed(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        html=b"<!doctype html><meta charset=utf-8><title>Dynamic feed fixture</title><main></main><script>let m=document.querySelector('main');for(let i=0;i<100;i++){let a=document.createElement('article');a.style='padding:24px;border-bottom:1px solid';a.textContent='Synthetic feed item '+i;m.append(a)}setTimeout(()=>{document.title='Dynamic feed updated';for(let i=0;i<100;i++)m.children[i].style.background=i%2?'#eef':'#fff'},300)</script>"
        self.send_response(200);self.send_header('Content-Type','text/html; charset=utf-8');self.send_header('Content-Length',str(len(html)));self.end_headers();self.wfile.write(html)
    def log_message(self,*args):pass

def main():
    checks=[];app=None;window=None;failure=None
    root=api.ROOT/'artifacts/qa-settings-browser-upgrade'/uuid.uuid4().hex;profile=root/'User';profile.mkdir(parents=True)
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Feed);threading.Thread(target=server.serve_forever,daemon=True).start()
    url=f'http://127.0.0.1:{server.server_port}/feed'
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)});print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def cmd(n):send(window,api.WM_COMMAND,n,timeout=15000)
    def ctl(n):return U.GetDlgItem(window,n)
    def q(n):return send(window,api.QUERY,n)
    def text(n,value):check('owned input accepts synthetic text',api.native.set_control_text(ctl(n),value))
    def launch():
        nonlocal app,window
        app=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
        window=api.wait_for(lambda:next(iter(api.native.app_windows(app.pid)),None),10)
        check('owned production window starts',window)
        check('real WGC gate passes before private actions',api.wait_for(lambda:send(window,api.GATE,timeout=6000),20))
    def close():
        nonlocal app
        U.PostMessageW(window,0x10,0,0);app.wait(timeout=10);check('owned process exits cleanly',app.returncode==0);app=None
    def resize(w,h):
        scale=U.GetDpiForWindow(window)/96;U.SetWindowPos(window,None,0,0,round(w*scale),round(h*scale),0x16);time.sleep(.12)
    def menu(n,index):
        U.PostMessageW(window,api.WM_COMMAND,n,0);popup=api.wait_for(lambda:api.owned_popup(app.pid),5);check('protected settings picker opens',popup)
        affinity=api.wintypes.DWORD();U.GetWindowDisplayAffinity(popup,ctypes.byref(affinity));check('settings picker has capture exclusion',affinity.value==0x11)
        check('opening settings picker preserves actual WGC gate',send(window,api.GATE,timeout=6000)==1)
        send(popup,0x200,0,(10<<16)|10)
        for _ in range(index):send(popup,api.WM_KEYDOWN,0x28)
        U.PostMessageW(popup,api.WM_KEYDOWN,0x0D,0);check('settings selection closes picker',api.wait_for(lambda:not api.owned_popup(app.pid),5));time.sleep(.1)
    try:
        check('personal application session untouched',not api.native.app_windows());launch();cmd(123);resize(840,560)
        check('wide Settings has left category navigation',q(80)==1 and all(api.visible(ctl(n)) for n in (200,201,202,203,204,240,345,346)) and not api.visible(ctl(347)))
        check('category column and form do not overlap',api.rect(ctl(200),window)[2]<api.rect(ctl(255),window)[0])
        check('advanced controls start collapsed',q(84)==0 and not api.visible(ctl(205)))
        cmd(348);check('advanced controls expand explicitly',q(84)==1 and api.visible(ctl(205)));cmd(348)
        cmd(345);check('Meetings has a separate category',q(40)==6 and api.visible(ctl(259)) and not api.visible(ctl(163)))
        cmd(201);check('reading category has no meeting model controls',not api.visible(ctl(331)) and api.visible(ctl(163)))
        for w,h in ((430,320),(840,320)):
            resize(w,h);check(f'short Settings exposes all categories without a dropdown {w}x{h}',q(80)==0 and not api.visible(ctl(347)) and all(api.visible(ctl(n)) for n in (200,201,202,203,204,240,345,346)))
            cmd(345);check('direct category button selects Meetings',q(40)==6)
        resize(840,560);cmd(127);text(110,'PRIVATE_PROFILE_SCRIPT');cmd(123);cmd(201);text(163,'37');send(ctl(163),api.WM_KEYDOWN,0x0D)
        cmd(349);cmd(203);cmd(290);check('preview preference can be disabled from Settings',q(70)==0)
        cmd(202);check('profiles start empty and cannot apply fabricated defaults',q(81)==0 and not U.IsWindowEnabled(ctl(362)))
        cmd(361);check('current workspace saves as a usable profile',q(81)==1 and U.IsWindowEnabled(ctl(362)))
        cached=(profile/'QuietShare/workspace-profiles.dpapi').read_bytes()
        check('profile contains no plaintext script or paths',b'PRIVATE_PROFILE_SCRIPT' not in cached and 'PRIVATE_PROFILE_SCRIPT'.encode('utf-16-le') not in cached)
        cmd(350);cmd(201);text(163,'18');send(ctl(163),api.WM_KEYDOWN,0x0D);cmd(203);cmd(290);cmd(202);cmd(362)
        check('profile restores reading size and preview choice',q(4)==37*100//24 and q(70)==0)
        check('applying profile starts no browser capture or recording',q(10)==0 and q(25)==0 and not list((profile/'QuietShare/Meetings').glob('*.wav')))
        cmd(127);check('profile preserves original script content',api.native.control_text(ctl(110))=='PRIVATE_PROFILE_SCRIPT')
        close();launch();cmd(123);cmd(202);check('saved profile persists across restart',q(81)==1);cmd(362);check('saved profile applies after restart',q(4)==37*100//24 and q(70)==0)
        cmd(139);cmd(141);cmd(370);report=api.native.control_text(ctl(373))
        check('last-stop diagnostics are visible and identify error',api.visible(ctl(373)) and 'QS-P001' in report and 'UTC:' in report and 'need-two-monitors' in report)
        check('diagnostics exclude script URLs and local paths','PRIVATE_PROFILE_SCRIPT' not in report and str(root) not in report and url not in report)
        cmd(105);check('sanitized diagnostics remain available while private content is masked',api.visible(ctl(373)) and q(83)==0);cmd(105);cmd(372)
        # Queue verification like real input. Cross-process SendMessage can
        # prohibit the synchronous WinRT activation required by WGC.
        U.PostMessageW(window,api.WM_COMMAND,140,0)
        check('queued user verification recovers after masking diagnostics without restart',api.wait_for(lambda:send(window,api.GATE,timeout=6000),20))
        close();launch();cmd(370);check('last-stop report remains available after restart','QS-P001' in api.native.control_text(ctl(373)));cmd(372)
        cmd(154);text(164,url);cmd(155);check('browser passes actual rendered WGC proof',api.wait_for(lambda:q(10)==2 and q(12)==1,25));cmd(154);cmd(165)
        check('dynamic local feed loads',api.wait_for(lambda:q(13)==1,15));time.sleep(.6)
        check('dynamic DOM and title changes keep WGC confidence',q(10)==2 and q(12)==1 and send(window,api.GATE,timeout=6000)==1)
        own_pid=q(11);check('only the test-owned browser process is selected',own_pid not in (0,app.pid))
        api.KERNEL32.OpenProcess.argtypes=[api.wintypes.DWORD,api.wintypes.BOOL,api.wintypes.DWORD];api.KERNEL32.OpenProcess.restype=api.wintypes.HANDLE
        handle=api.KERNEL32.OpenProcess(0x00100001,False,own_pid);check('owned browser process handle is available',bool(handle))
        api.KERNEL32.TerminateProcess.argtypes=[api.wintypes.HANDLE,api.wintypes.UINT]
        try:check('owned browser failure is triggered',api.KERNEL32.TerminateProcess(handle,42))
        finally:api.KERNEL32.CloseHandle(handle)
        check('fatal browser error becomes an actionable retained tab',api.wait_for(lambda:q(10)==3 and api.visible(ctl(375)),15))
        check('web failure alone does not revoke real WGC confidence',send(window,api.GATE,timeout=6000)==1)
        check('browser retry action stays enabled',U.IsWindowEnabled(ctl(375)))
        U.PostMessageW(window,api.WM_COMMAND,375,0)
        check('retry recreates browser and verifies it before revealing content',api.wait_for(lambda:q(10)==2 and q(12)==1 and q(13)==1,25))
        check('retry keeps the original local page address',api.native.control_text(ctl(164))==url)
        cmd(374);report=api.native.control_text(ctl(373));check('browser and projection failures have separate codes in one report','QS-B001' in report and 'QS-P001' in report and url not in report)
        close();bad=cached[:len(cached)//2];(profile/'QuietShare/workspace-profiles.dpapi').write_bytes(bad)
        launch();cmd(123);cmd(202);check('unreadable profiles are preserved and cannot be overwritten',not U.IsWindowEnabled(ctl(361)) and not U.IsWindowEnabled(ctl(362)))
        cmd(361);check('failed profile read preserves exact old bytes',(profile/'QuietShare/workspace-profiles.dpapi').read_bytes()==bad);close()
    except Exception as error:failure=repr(error);raise
    finally:
        server.shutdown();server.server_close()
        if app and app.poll() is None:
            for h in api.native.app_windows(app.pid):U.PostMessageW(h,0x10,0,0)
            try:app.wait(timeout=8)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
        report={'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'failure':failure,'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest(),'physicalThreadsSession':'not tested','physicalReceiver':'not tested'}
        (api.ROOT/'artifacts/settings-browser-upgrade.json').write_text(json.dumps(report,indent=2),encoding='utf-8')

if __name__=='__main__':main()
