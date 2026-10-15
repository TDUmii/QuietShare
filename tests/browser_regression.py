"""Shipping browser integration with an isolated profile and local synthetic web.
Never reads personal browser data or sends messages to ChatGPT.
"""
import ctypes
from ctypes import wintypes
import hashlib,json,os,subprocess,sys,time,threading,shutil
from pathlib import Path
from http.server import BaseHTTPRequestHandler,ThreadingHTTPServer
from urllib.parse import urlsplit,parse_qs
sys.dont_write_bytecode=True
import lifecycle_regression as native
ROOT,EXE,USER32=native.ROOT,native.EXE,native.USER32
USER32.IsWindowVisible.argtypes=[wintypes.HWND]
USER32.IsWindowVisible.restype=wintypes.BOOL
USER32.PostMessageW.argtypes=[wintypes.HWND,wintypes.UINT,wintypes.WPARAM,wintypes.LPARAM]
USER32.PostMessageW.restype=wintypes.BOOL
USER32.SetWindowPos.argtypes=[wintypes.HWND,wintypes.HWND,ctypes.c_int,ctypes.c_int,ctypes.c_int,ctypes.c_int,wintypes.UINT]
native.KERNEL32.OpenProcess.argtypes=[wintypes.DWORD,wintypes.BOOL,wintypes.DWORD]
native.KERNEL32.OpenProcess.restype=wintypes.HANDLE
native.KERNEL32.WaitForSingleObject.argtypes=[wintypes.HANDLE,wintypes.DWORD]
native.KERNEL32.CloseHandle.argtypes=[wintypes.HANDLE]
hits=[]
cookies=[]
class Site(BaseHTTPRequestHandler):
    def log_message(self,*args):pass
    def do_GET(self):
        hits.append(self.path)
        cookies.append(bool(self.headers.get('Cookie')))
        if self.path=='/credential':
            self.send_response(401);self.send_header('WWW-Authenticate','Basic realm="QuietShare synthetic fixture"');self.send_header('Content-Length','0');self.end_headers();return
        body=("<!doctype html><meta charset=utf-8><style>body{background:#1e1e1e;color:#ddd;font:18px sans-serif;padding:24px}</style>"
              "<h1>QuietShare synthetic browser fixture</h1><p>Trang web mẫu - local only.</p>"
              "<a href='/second'>Second page</a><input type=file id=f><select id=s><option>A</option><option>B</option></select>"
              "<script>if(location.pathname==='/policy'){window.open('/blocked-popup');alert('synthetic');"
              "window.print();document.querySelector('#f').click();try{document.querySelector('#f').showPicker()}catch(e){}"
              "try{document.querySelector('#s').showPicker()}catch(e){}"
              "navigator.geolocation.getCurrentPosition(()=>fetch('/unexpected-location'),()=>{});fetch('/policy-finished');}</script>").encode()
        self.send_response(200);self.send_header('Set-Cookie','quietshare_fixture=1; Path=/; SameSite=Lax');self.send_header('Content-Type','text/html; charset=utf-8');self.send_header('Content-Length',str(len(body)));self.end_headers();self.wfile.write(body)
def wait(predicate,seconds=8):
    end=time.monotonic()+seconds
    while time.monotonic()<end:
        if predicate():return True
        time.sleep(.05)
    return False
def main():
    folder=ROOT/'artifacts'/'qa-browser';folder.mkdir(parents=True,exist_ok=True)
    profile=folder/('User-'+str(time.time_ns()));profile.mkdir()
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    checks=[];app=None;failure=None;metrics={}
    def check(name,value):
        checks.append({'name':name,'passed':bool(value)});print(('PASS ' if value else 'FAIL ')+name,flush=True)
        if not value:raise AssertionError(name)
    server=ThreadingHTTPServer(('127.0.0.1',0),Site);thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
    base='http://127.0.0.1:'+str(server.server_port)
    try:
        compiler=shutil.which('g++.exe') or shutil.which('g++')
        if not compiler:raise RuntimeError('MinGW-w64 is required for the browser input regression.')
        input_exe=folder/'browser-input-regression.exe'
        subprocess.run([compiler,'-std=c++17','-O2','-static','-static-libgcc','-static-libstdc++','-DUNICODE','-D_UNICODE',
            '-D_WIN32_WINNT=0x0A00',str(ROOT/'tests/browser_input_regression.cpp'),str(ROOT/'src/Browser.cpp'),
            '-I'+str(ROOT/'vendor/webview2/build/native/include'),'-o',str(input_exe),'-lole32','-luuid','-luser32'],check=True)
        input_result=subprocess.run([str(input_exe)],capture_output=True,text=True)
        for item in json.loads(input_result.stdout)['checks']:check(item['name'],item['passed'])
        check('browserInputRegressionExit',input_result.returncode==0)
        check('noExistingUserApp',not native.app_windows())
        startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=0
        app=subprocess.Popen([str(EXE)],cwd=EXE.parent,env=env,startupinfo=startup)
        window=None
        def get_window():
            nonlocal window
            window=next(iter(native.app_windows(app.pid)),None);return window
        check('shippingWindowStarts',wait(get_window))
        q=lambda n:native.send(window,0x800F,n,timeout=6000)
        check('realWgcBaseGate',wait(lambda:native.send(window,0x800E,timeout=6000),20))
        USER32.ShowWindow(window,4)
        def cmd(n):USER32.PostMessageW(window,0x111,n,0);time.sleep(.15)
        check('browserOffByDefault',q(10)==0 and q(11)==0)
        check('noBrowserProfileBeforeEnable',not (profile/'QuietShare'/'WebProfile').exists())
        cmd(154);address=USER32.GetDlgItem(window,164)
        check('browserPageDoesNotStartRuntime',q(10)==0 and q(11)==0 and not hits)
        native.set_control_text(address,base+'/first')
        cmd(123)
        check('browserCanBeConfiguredFromSettings',q(21)==1 and q(10)==0)
        begin=time.monotonic();cmd(155)
        opened=wait(lambda:q(10)==2 and q(12)==1,25)
        print('Browser state:',q(10),'PID:',q(11),'error:',q(14),'base gate:',native.send(window,0x800E,timeout=6000),flush=True)
        check('browserStartsAndPassesRenderedCanary',opened)
        check('enablingBrowserKeepsSettingsOpen',q(21)==1)
        metrics['coldStartAndCanaryMs']=round((time.monotonic()-begin)*1000)
        cmd(154)
        check('localWebActuallyLoads',wait(lambda:q(13)==1 and '/first' in hits,12))
        check('dedicatedBrowserProcess',q(11)!=0 and q(11)!=app.pid)
        check('dedicatedProfileCreatedOnlyOnEnable',(profile/'QuietShare'/'WebProfile').exists())
        check('addressReflectsPage',native.control_text(address)==base+'/first')
        native.set_control_text(address,base+'/second');cmd(165)
        check('navigateSecondPage',wait(lambda:q(13)==1 and '/second' in hits))
        cmd(156);check('backRestoresFirstPage',wait(lambda:native.control_text(address)==base+'/first'))
        cmd(157);check('forwardRestoresSecondPage',wait(lambda:native.control_text(address)==base+'/second'))
        count=hits.count('/second');cmd(158);check('reloadHitsLocalPageAgain',wait(lambda:hits.count('/second')>count))
        for label,value in [('script','javascript:alert(1)'),('file','file:///C:/QuietShare-synthetic.txt'),('external','ms-excel:ofe|u|https://example.com')]:
            native.set_control_text(address,value);cmd(165)
            check(label+'SchemeRejected',q(10)==2 and q(12)==1 and native.control_text(address)==base+'/second')
        original_windows=set()
        @native.ENUM_PROC
        def baseline(hwnd,_):
            if native.window_pid(hwnd) in (app.pid,q(11)) and USER32.IsWindowVisible(hwnd):original_windows.add(hwnd)
            return True
        USER32.EnumWindows(baseline,0)
        native.set_control_text(address,base+'/policy');cmd(165)
        check('policyFixtureLoads',wait(lambda:q(13)==1 and '/policy' in hits))
        time.sleep(.8)
        check('policyScriptCompletesWithoutModalInterruption',wait(lambda:'/policy-finished' in hits))
        check('unsolicitedPopupDoesNotNavigate','/blocked-popup' not in hits)
        check('geolocationPermissionDenied','/unexpected-location' not in hits)
        dialog_windows=[];browser_pid=q(11)
        @native.ENUM_PROC
        def inspect(hwnd,_):
            if native.window_pid(hwnd) in (app.pid,browser_pid) and USER32.IsWindowVisible(hwnd):
                name=ctypes.create_unicode_buffer(128);USER32.GetClassNameW(hwnd,name,128)
                if name.value=='#32770' or (name.value.startswith('Chrome_WidgetWin') and hwnd not in original_windows):
                    title=native.control_text(hwnd)
                    print('Owned fixture window:',name.value,'caption:',title[:120],'style:',hex(USER32.GetWindowLongW(hwnd,-16)&0xffffffff),flush=True)
                    dialog_windows.append(hwnd)
            return True
        USER32.EnumWindows(inspect,0)
        check('noNewNativeDialogFromScriptOrUpload',not dialog_windows)
        native.set_control_text(address,base+'/credential');cmd(165)
        check('basicAuthenticationFixtureReached',wait(lambda:'/credential' in hits))
        time.sleep(.6);dialog_windows.clear();USER32.EnumWindows(inspect,0)
        check('basicAuthenticationDoesNotOpenCredentialDialog',not dialog_windows and q(10)==2)
        native.send(window,0x312,2)
        check('F8MasksBrowserAndAddress',q(10)==q(11)==q(12)==0 and not USER32.IsWindowVisible(address))
        native.send(window,0x312,2)
        check('F8RestoresVerifiedBrowser',wait(lambda:q(10)==2 and q(12)==1))
        pid=q(11);cmd(120)
        check('scriptReaderHidesWebButKeepsSession',not USER32.IsWindowVisible(address) and q(11)==pid)
        check('hiddenBrowserActuallySuspends',wait(lambda:q(15)==1))
        cmd(154);check('browserSessionReturns',q(10)==2 and q(11)==pid)
        check('visibleBrowserResumes',wait(lambda:q(15)==0))
        USER32.SetWindowPos(window,None,0,0,460,360,0x16);time.sleep(.2)
        check('narrowBrowserKeepsControls',USER32.IsWindowVisible(address) and USER32.IsWindowVisible(USER32.GetDlgItem(window,155)))
        process_handle=native.KERNEL32.OpenProcess(0x00100000,False,pid)
        check('ownedBrowserProcessCanBeObserved',bool(process_handle))
        cmd(155);check('disableReleasesBrowserObjects',q(10)==0 and q(11)==0 and q(12)==0)
        try:check('disabledBrowserProcessExits',native.KERNEL32.WaitForSingleObject(process_handle,8000)==0)
        finally:native.KERNEL32.CloseHandle(process_handle)
        for cycle in range(3):
            cmd(155);cmd(155);check('cancelPendingEnable:'+str(cycle),wait(lambda:q(10)==0 and q(11)==0))
        native.set_control_text(address,base+'/first');cmd(155)
        check('reEnableRunsFreshCanary',wait(lambda:q(10)==2 and q(12)==1,25))
        check('separateProfilePreservesSyntheticCookie',wait(lambda:any(cookies)))
        cmd(159)
        metrics['chatgptPublicHttpsLoaded']=wait(lambda:native.control_text(address).startswith('https://chatgpt.com') and q(13)==1,20)
        query='QuietShare nội dung mẫu'
        native.set_control_text(address,query);cmd(165)
        def query_address():
            current=urlsplit(native.control_text(address))
            return current.scheme=='https' and current.hostname in ('www.google.com','google.com') and current.path=='/search' and parse_qs(current.query).get('q')==[query]
        check('addressBarVietnameseSearchNavigatesGoogle',wait(query_address,12))
        metrics['googleSearchPublicHttpsLoaded']=wait(lambda:query_address() and q(13)==1,12)
        cmd(166)
        def google_home():
            current=urlsplit(native.control_text(address))
            return current.scheme=='https' and current.hostname in ('www.google.com','google.com') and current.path in ('','/')
        check('googleShortcutNavigatesHome',wait(google_home,12))
        native.send(window,0x7E,0,0)
        check('displayChangeClosesBrowser',q(10)==0 and q(11)==0)
        native.send(window,0x10,timeout=6000);app.wait(timeout=8)
        app=subprocess.Popen([str(EXE)],cwd=EXE.parent,env=env,startupinfo=startup)
        check('restartWithExistingWebProfile',wait(get_window))
        check('restartStillDefaultsBrowserOff',wait(lambda:native.send(window,0x800E,timeout=6000),20) and q(10)==0 and q(11)==0)
        native.send(window,0x10,timeout=6000);app.wait(timeout=8)
        missing=folder/'missing-loader';missing.mkdir(exist_ok=True)
        assert missing.resolve().is_relative_to(ROOT.resolve())
        for name in ('QuietShare.exe','Scintilla.dll','Lexilla.dll','pdfium.dll'):shutil.copy2(EXE.parent/name,missing/name)
        app=subprocess.Popen([str(missing/'QuietShare.exe')],cwd=missing,env=env,startupinfo=startup)
        check('offlineReadersStartWithoutBrowserLoader',wait(get_window) and wait(lambda:native.send(window,0x800E,timeout=6000),20))
        cmd(154);cmd(155);check('missingLoaderFailsOnlyOptionalBrowser',wait(lambda:q(10)==3) and native.send(window,0x800E,timeout=6000)==1)
        reader=folder/'reader.txt';reader.write_text('QuietShare offline reader fixture',encoding='utf-8')
        native.drop_file(window,reader)
        check('fileReaderWorksAfterBrowserUnavailable',wait(lambda:q(1)==2) and native.control_text(USER32.GetDlgItem(window,110))=='QuietShare offline reader fixture')
    except Exception as e:failure={'type':type(e).__name__,'message':str(e)}
    finally:
        if app and app.poll() is None:
            for w in native.app_windows(app.pid):native.send(w,0x10,timeout=6000)
            try:app.wait(timeout=8)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
        server.shutdown();server.server_close()
        report={'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'checks':checks,'failure':failure,
                'exeSha256':hashlib.sha256(EXE.read_bytes()).hexdigest(),'metrics':metrics,
                'method':'Shipping EXE; actual browser-rendered positive and excluded WGC canary; isolated user profile; local HTTP fixtures only',
                'notTested':['ChatGPT authenticated session and third-party OAuth','Physical meeting receivers and two physical monitors','Missing Evergreen Runtime on a clean machine']}
        (folder/'browser-regression.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps({k:report[k] for k in ('passed','failed','failure','metrics')}),flush=True)
    return 1 if failure or report['failed'] else 0
if __name__=='__main__':raise SystemExit(main())
