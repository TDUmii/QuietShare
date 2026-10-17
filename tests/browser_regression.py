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
USER32.GetClientRect.argtypes=[wintypes.HWND,ctypes.POINTER(wintypes.RECT)]
USER32.GetClientRect.restype=wintypes.BOOL
USER32.GetWindowRect.argtypes=[wintypes.HWND,ctypes.POINTER(wintypes.RECT)]
USER32.GetWindowRect.restype=wintypes.BOOL
USER32.MapWindowPoints.argtypes=[wintypes.HWND,wintypes.HWND,ctypes.POINTER(wintypes.POINT),wintypes.UINT]
USER32.MapWindowPoints.restype=ctypes.c_int
USER32.GetDpiForWindow.argtypes=[wintypes.HWND]
USER32.GetDpiForWindow.restype=wintypes.UINT
USER32.AttachThreadInput.argtypes=[wintypes.DWORD,wintypes.DWORD,wintypes.BOOL]
USER32.AttachThreadInput.restype=wintypes.BOOL
USER32.GetKeyboardState.argtypes=[ctypes.POINTER(ctypes.c_ubyte)]
USER32.GetKeyboardState.restype=wintypes.BOOL
USER32.SetKeyboardState.argtypes=[ctypes.POINTER(ctypes.c_ubyte)]
USER32.SetKeyboardState.restype=wintypes.BOOL
USER32.PeekMessageW.argtypes=[ctypes.POINTER(wintypes.MSG),wintypes.HWND,wintypes.UINT,wintypes.UINT,wintypes.UINT]
USER32.PeekMessageW.restype=wintypes.BOOL
native.KERNEL32.GetCurrentThreadId.restype=wintypes.DWORD
native.KERNEL32.OpenProcess.argtypes=[wintypes.DWORD,wintypes.BOOL,wintypes.DWORD]
native.KERNEL32.OpenProcess.restype=wintypes.HANDLE
native.KERNEL32.WaitForSingleObject.argtypes=[wintypes.HANDLE,wintypes.DWORD]
native.KERNEL32.CloseHandle.argtypes=[wintypes.HANDLE]
hits=[]
cookies=[]
viewports=[]
form_states=[]
class Site(BaseHTTPRequestHandler):
    def log_message(self,*args):pass
    def do_GET(self):
        hits.append(self.path)
        cookies.append(bool(self.headers.get('Cookie')))
        if self.path.startswith('/viewport?'):
            values=parse_qs(urlsplit(self.path).query)
            viewports.append({'width':int(values['w'][0]),'height':int(values['h'][0]),'dpr':float(values['dpr'][0]),'page':values['page'][0],'tick':float(values['tick'][0])})
            self.send_response(204);self.end_headers();return
        if self.path.startswith('/form-state?'):
            values=parse_qs(urlsplit(self.path).query)
            form_states.append({**{key:values.get(key,[''])[0] for key in ('page','instance','draft','visibility')},'tick':float(values['tick'][0])})
            self.send_response(204);self.end_headers();return
        if self.path=='/credential':
            self.send_response(401);self.send_header('WWW-Authenticate','Basic realm="QuietShare synthetic fixture"');self.send_header('Content-Length','0');self.end_headers();return
        body=("<!doctype html><meta charset=utf-8><title>QuietShare local fixture</title><style>body{background:#1e1e1e;color:#ddd;font:18px sans-serif;padding:24px}</style>"
              "<h1>QuietShare synthetic browser fixture</h1><p>Trang web mẫu - local only.</p>"
              "<a href='/second'>Second page</a><input type=file id=f><select id=s><option>A</option><option>B</option></select>"
              "<textarea id=draft aria-label='Unsent synthetic draft'></textarea>"
              "<script>if(location.pathname==='/policy'){window.open('/blocked-popup');alert('synthetic');"
              "window.print();document.querySelector('#f').click();try{document.querySelector('#f').showPicker()}catch(e){}"
              "try{document.querySelector('#s').showPicker()}catch(e){}"
              "navigator.geolocation.getCurrentPosition(()=>fetch('/unexpected-location'),()=>{});fetch('/policy-finished');}"
              "const tick=()=>performance.timeOrigin+performance.now();"
              "const viewport=()=>fetch('/viewport?w='+innerWidth+'&h='+innerHeight+'&dpr='+devicePixelRatio+'&page='+encodeURIComponent(location.pathname)+'&tick='+tick());"
              "addEventListener('resize',viewport);viewport();for(const ms of [250,750,1500])setTimeout(viewport,ms);"
              # A per-document unsent form value proves that switching tabs
              # keeps the existing DOM; this does not emulate physical typing.
              "const instance=String(performance.timeOrigin),draft=document.querySelector('#draft');draft.value='unsent-'+instance;"
              "document.title='QuietShare '+location.pathname;"
              "const state=()=>fetch('/form-state?'+new URLSearchParams({page:location.pathname,instance,draft:draft.value,visibility:document.visibilityState,tick:tick()}));"
              "for(const name of ['visibilitychange','focus','pageshow'])addEventListener(name,()=>{viewport();state()});state();</script>").encode()
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
    checks=[];app=None;failure=None;metrics={};not_tested=[]
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
        # HTTP worker completion can reorder beacons; use the in-page capture
        # clock, not server receipt order, to identify the latest rendered state.
        def viewport(page):return max((entry for entry in viewports if entry['page']==page),key=lambda entry:entry['tick'],default=None)
        def form_state(page):return max((entry for entry in form_states if entry['page']==page),key=lambda entry:entry['tick'],default=None)
        def modifier_key(key,condition,shift=False):
            # Only the owned app's input queue is attached. No SendInput,
            # clipboard, physical keystrokes or foreign UI is touched.
            pending=wintypes.MSG();USER32.PeekMessageW(ctypes.byref(pending),None,0,0,0)
            owner=USER32.GetWindowThreadProcessId(window,None);current=native.KERNEL32.GetCurrentThreadId()
            if not USER32.AttachThreadInput(current,owner,True):return None
            original=(ctypes.c_ubyte*256)()
            try:
                if not USER32.GetKeyboardState(original):return None
                state=(ctypes.c_ubyte*256).from_buffer_copy(original)
                state[0x11]=state[0xA2]=0x80
                state[0x10]=state[0xA0]=0x80 if shift else 0
                if not USER32.SetKeyboardState(state):return None
                USER32.PostMessageW(address,0x100,key,1)
                return wait(condition,6)
            finally:
                USER32.SetKeyboardState(original);USER32.AttachThreadInput(current,owner,False)
        check('browserOffByDefault',q(10)==0 and q(11)==0)
        check('noBrowserProfileBeforeEnable',not (profile/'QuietShare'/'WebProfile').exists())
        cmd(154);address=USER32.GetDlgItem(window,164)
        check('browserPageDoesNotStartRuntime',q(10)==0 and q(11)==0 and not hits)
        check('browserZoomDefaults100AndControlsWaitForEnable',q(24)==100 and not USER32.IsWindowEnabled(USER32.GetDlgItem(window,167)))
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
        check('zoomFixtureReportsActualViewport',wait(lambda:viewport('/first')))
        initial_viewport=viewport('/first').copy()
        # Verify compact browser chrome against the live WebView viewport,
        # including the optional toolbar, without changing Windows DPI.
        original_bounds=wintypes.RECT();USER32.GetWindowRect(window,ctypes.byref(original_bounds))
        scale=USER32.GetDpiForWindow(window)/96
        compact_metrics=[]
        controls=(122,120,139,154,123,105,188,180,156,157,158,164,165,166,159,167,168,169,155,190,191)
        essential=(156,157,158,164,165,155,190,191,105,188,180)
        def chrome_fits():
            client=wintypes.RECT();USER32.GetClientRect(window,ctypes.byref(client));bounds=[]
            for control_id in controls:
                control=USER32.GetDlgItem(window,control_id)
                if not control or not USER32.IsWindowVisible(control):continue
                box=wintypes.RECT();USER32.GetWindowRect(control,ctypes.byref(box))
                points=(wintypes.POINT*2)(wintypes.POINT(box.left,box.top),wintypes.POINT(box.right,box.bottom))
                USER32.MapWindowPoints(None,window,points,2)
                box=(points[0].x,points[0].y,points[1].x,points[1].y)
                if not (0<=box[0]<box[2]<=client.right and 0<=box[1]<box[3]<=client.bottom):return False
                if any(min(box[2],previous[2])>max(box[0],previous[0]) and
                       min(box[3],previous[3])>max(box[1],previous[1]) for previous in bounds):return False
                bounds.append(box)
            return True
        for language in ('vi','en'):
            if language=='en':cmd(107)
            for width,height in ((430,320),(460,360)):
                label='compactBrowser:'+language+':'+str(width)
                before=viewport('/first').copy()
                USER32.SetWindowPos(window,None,0,0,round(width*scale),round(height*scale),0x16)
                check(label+':actualViewportResizes',wait(lambda:viewport('/first')['tick']>before['tick'] and
                      (viewport('/first')['width'],viewport('/first')['height'])!=(before['width'],before['height'])))
                shown=viewport('/first').copy();client=wintypes.RECT();USER32.GetClientRect(window,ctypes.byref(client))
                check(label+':allControlsFitWithoutOverlap',chrome_fits())
                check(label+':essentialControlsVisible',all(USER32.IsWindowVisible(USER32.GetDlgItem(window,control)) for control in essential))
                check(label+':webDominatesSmallWindow',shown['width']*shown['dpr']>=client.right*.84-2 and
                      shown['height']*shown['dpr']>=client.bottom*.58-2)
                cmd(183)
                check(label+':hiddenToolbarGivesWebMoreHeight',wait(lambda:viewport('/first')['tick']>shown['tick'] and
                      viewport('/first')['height']>shown['height']))
                hidden=viewport('/first').copy()
                check(label+':hiddenToolbarKeepsEssentialControls',not q(16)&4 and chrome_fits() and
                      all(USER32.IsWindowVisible(USER32.GetDlgItem(window,control)) for control in essential))
                check(label+':hiddenToolbarWebDominatesHeight',hidden['height']*hidden['dpr']>=client.bottom*.70-2)
                check(label+':layoutKeepsControllerAndPrivacy',q(10)==2 and q(12)==1 and q(11)!=0 and q(34)==1 and
                      native.control_text(address)==base+'/first')
                compact_metrics.append({'language':language,'logicalSize':[width,height],'dpi':round(scale*96),
                    'clientPixels':[client.right,client.bottom],'withToolbar':shown,'withoutToolbar':hidden})
                cmd(183)
                check(label+':toolbarRestoresViewport',wait(lambda:q(16)&4 and viewport('/first')['tick']>hidden['tick'] and
                      abs(viewport('/first')['height']-shown['height'])<=1))
        cmd(107)
        USER32.SetWindowPos(window,None,0,0,original_bounds.right-original_bounds.left,original_bounds.bottom-original_bounds.top,0x16)
        check('compactBrowserRestoresOriginalViewport',wait(lambda:
              abs(viewport('/first')['width']-initial_viewport['width'])<=1 and abs(viewport('/first')['height']-initial_viewport['height'])<=1))
        metrics['compactBrowser']=compact_metrics
        check('browserZoomStarts100',q(24)==100 and native.control_text(USER32.GetDlgItem(window,168))=='100%')
        cmd(169)
        check('browserZoomInUpdatesActualViewport',wait(lambda:q(24)==110 and viewport('/first')['width']<initial_viewport['width']
              and abs(viewport('/first')['width']-initial_viewport['width']/1.1)<=3))
        check('browserZoomLabelTracksActualValue',native.control_text(USER32.GetDlgItem(window,168))=='110%')
        cmd(167)
        check('browserZoomOutRestoresActualViewport',wait(lambda:q(24)==100 and abs(viewport('/first')['width']-initial_viewport['width'])<=1))
        for _ in range(40):native.send(window,0x111,169)
        check('browserZoomClamps400',q(24)==400)
        check('maximumZoomIsRendered',wait(lambda:abs(viewport('/first')['dpr']-initial_viewport['dpr']*4)<.03))
        for _ in range(50):native.send(window,0x111,167)
        check('browserZoomClamps25',q(24)==25)
        check('minimumZoomIsRendered',wait(lambda:abs(viewport('/first')['dpr']-initial_viewport['dpr']*.25)<.03))
        cmd(168)
        check('browserZoomResetRestores100',wait(lambda:q(24)==100 and abs(viewport('/first')['width']-initial_viewport['width'])<=1))
        cmd(169)
        native.set_control_text(address,base+'/second');cmd(165)
        check('navigateSecondPage',wait(lambda:q(13)==1 and '/second' in hits))
        check('browserZoomPersistsAcrossNavigation',wait(lambda:q(24)==110 and viewport('/second') and
              abs(viewport('/second')['dpr']-initial_viewport['dpr']*1.1)<.03))
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
        check('F8MasksBrowserAndAddress',q(10)==q(11)==q(12)==q(24)==0 and not USER32.IsWindowVisible(address))
        native.send(window,0x312,2)
        check('F8RestoresVerifiedBrowser',wait(lambda:q(10)==2 and q(12)==1))
        check('F8PreservesBrowserZoomPreference',q(24)==110)
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
        check('disableKeepsBrowserZoomSessionPreference',q(24)==110)
        try:check('disabledBrowserProcessExits',native.KERNEL32.WaitForSingleObject(process_handle,8000)==0)
        finally:native.KERNEL32.CloseHandle(process_handle)
        for cycle in range(3):
            # Cancel the pending environment before starting a mounted canary.
            # cmd()'s 150 ms pause can enter that proof; cancelling it correctly
            # revokes the common gate and is not a harmless pending-enable case.
            native.send(window,0x111,155);native.send(window,0x111,155);time.sleep(.15)
            check('cancelPendingEnable:'+str(cycle),wait(lambda:native.send(window,0x800E,timeout=6000)==1 and
                  q(10)==q(11)==q(34)==q(38)==0))
        viewport_start=len(viewports);form_start=len(form_states)
        native.set_control_text(address,base+'/first');cmd(155)
        check('reEnableRunsFreshCanary',wait(lambda:q(10)==2 and q(12)==1,25))
        check('reEnableRestoresZoomAfterFreshCanary',wait(lambda:q(24)==110 and q(13)==1 and
              any(entry['page']=='/first' and abs(entry['dpr']-initial_viewport['dpr']*1.1)<.03 for entry in viewports[viewport_start:])))
        check('separateProfilePreservesSyntheticCookie',wait(lambda:any(cookies)))
        # These controls use the same activation/close messages as native tab
        # mouse handlers. Every new controller still passes the real WGC gate.
        USER32.SetWindowPos(window,None,0,0,1080,760,0x16);time.sleep(.15)
        tab_a=q(35);session_pid=q(11);bar=USER32.GetDlgItem(window,191)
        check('oneVerifiedTabAfterReEnable',q(34)==1 and tab_a!=0 and q(38)==1 and q(37)==0)
        check('nativeWebTabControlsVisible',bool(bar) and USER32.IsWindowVisible(bar) and USER32.IsWindowVisible(USER32.GetDlgItem(window,190)))
        check('firstTabHasActualUnsentForm',wait(lambda:any(entry['page']=='/first' and entry['draft'].startswith('unsent-') for entry in form_states[form_start:])))
        form_a=form_state('/first').copy()
        def new_tab(label,shortcut=False):
            before=q(34);previous=q(35)
            if shortcut:
                routed=modifier_key(ord('T'),lambda:q(34)==before+1)
                if routed is None:
                    not_tested.append('Owned Ctrl+T input attachment unavailable');cmd(190)
                else:check(label+'CtrlT',routed)
            else:cmd(190)
            check(label+'GetsStableNewId',wait(lambda:q(34)==before+1 and q(35) not in (0,previous)))
            selected=q(35)
            check(label+'PassesOwnRenderedCanary',wait(lambda:q(10)==2 and q(12)==1 and q(37)==0,25))
            return selected
        def navigate(path):
            native.set_control_text(address,base+path);cmd(165)
            return wait(lambda:q(13)==1 and native.control_text(address)==base+path and form_state(path),12)
        def activate_tab(tab):
            native.send(window,0x8000+25,tab)
            return wait(lambda:q(35)==tab and q(10)==2)
        tab_b=new_tab('secondTab')
        check('newTabZoomDefaults100',q(24)==100)
        check('secondTabSharesBrowserProcess',q(11)==session_pid)
        check('secondTabLoadsOwnPage',navigate('/tab-b-one'))
        cmd(169);cmd(169)
        check('secondTabOwnZoomIsRendered',wait(lambda:q(24)==120 and viewport('/tab-b-one') and
              abs(viewport('/tab-b-one')['dpr']-initial_viewport['dpr']*1.2)<.03))
        check('secondTabCreatesOwnHistory',navigate('/tab-b-two'))
        form_b=form_state('/tab-b-two').copy()
        tab_c=new_tab('thirdTab',shortcut=True)
        check('thirdTabDoesNotInheritOtherTabZoom',q(24)==100)
        check('thirdTabLoadsOwnPage',navigate('/tab-c'))
        check('tabsShareOneBrowserEnvironment',q(38)==3 and q(11)==session_pid)
        check('backgroundTabsActuallySuspend',wait(lambda:q(36)==2,12))
        # Do not wait for readiness: close the asynchronously created tab as
        # soon as the synchronous native New command returns.
        native.send(window,0x111,190);pending_id=q(35)
        metrics['canceledNewTabBeforeProofCompleted']=q(37)==1
        check('pendingTabHasIndependentStableId',q(34)==4 and pending_id not in (0,tab_a,tab_b,tab_c))
        native.send(window,0x8000+26,pending_id)
        check('closingPendingTabRestoresPreviousTab',wait(lambda:q(34)==3 and q(35)==tab_c and q(37)==0))
        time.sleep(.7)
        check('closedPendingCallbacksDoNotReviveController',q(34)==q(38)==3 and q(35)==tab_c and q(37)==0)
        native.send(window,0x312,2)
        check('F8GatesAllTabDiagnostics',all(q(key)==0 for key in range(34,39)))
        check('F8HidesWebTabCaptions',not USER32.IsWindowVisible(bar) and not USER32.IsWindowVisible(address))
        native.send(window,0x312,2)
        check('F8RestoresVerifiedTabCollection',wait(lambda:q(34)==3 and q(38)==3 and q(35)==tab_c and q(12)==1))
        bounds=wintypes.RECT();USER32.GetClientRect(bar,ctypes.byref(bounds));scale=USER32.GetDpiForWindow(window)/96
        native.send(bar,0x202,0,((bounds.bottom//2)<<16)|round(16*scale))
        check('nativeTabClickActivatesFirstTab',wait(lambda:q(35)==tab_a))
        check('firstTabKeepsAddressAndZoom',wait(lambda:native.control_text(address)==base+'/first' and q(24)==110))
        check('firstTabKeepsActualDomAndUnsentDraft',wait(lambda:form_state('/first')['visibility']=='visible' and
              form_state('/first')['instance']==form_a['instance'] and form_state('/first')['draft']==form_a['draft']))
        check('activateSecondStableTabId',activate_tab(tab_b))
        check('secondTabKeepsAddressAndZoom',wait(lambda:native.control_text(address)==base+'/tab-b-two' and q(24)==120))
        check('secondTabKeepsActualDomAndUnsentDraft',wait(lambda:form_state('/tab-b-two')['visibility']=='visible' and
              form_state('/tab-b-two')['instance']==form_b['instance'] and form_state('/tab-b-two')['draft']==form_b['draft']))
        cmd(156);check('tabBackUsesItsOwnHistory',wait(lambda:native.control_text(address)==base+'/tab-b-one'))
        cmd(157);check('tabForwardUsesItsOwnHistory',wait(lambda:native.control_text(address)==base+'/tab-b-two'))
        routed=modifier_key(9,lambda:q(35)==tab_c)
        if routed is None:
            not_tested.append('Owned Ctrl+Tab input attachment unavailable');activate_tab(tab_c)
        else:check('CtrlTabCyclesBrowserTabs',routed)
        routed=modifier_key(9,lambda:q(35)==tab_b,shift=True)
        if routed is None:
            not_tested.append('Owned Ctrl+Shift+Tab input attachment unavailable');activate_tab(tab_b)
        else:check('CtrlShiftTabCyclesBrowserTabsBackwards',routed)
        # Reach the actual controller cap, then exercise both the disabled
        # native button's command route and its keyboard sibling at the limit.
        # Readiness is sufficient; public Google navigation is not awaited.
        extra_tabs=[new_tab('capacityTab'+str(index)) for index in range(4,9)]
        check('eightVerifiedTabsReachControllerLimit',q(34)==q(38)==8 and q(37)==0 and q(11)==session_pid)
        check('newTabButtonDisabledAtEight',not USER32.IsWindowEnabled(USER32.GetDlgItem(window,190)))
        limited_id=q(35);native.send(window,0x111,190);time.sleep(.15)
        check('directNewCommandCannotExceedEight',q(34)==q(38)==8 and q(35)==limited_id and q(37)==0)
        shortcut_settle=time.monotonic()+.3
        routed=modifier_key(ord('T'),lambda:time.monotonic()>=shortcut_settle and q(34)==q(38)==8 and q(35)==limited_id and q(37)==0)
        if routed is None:not_tested.append('Owned Ctrl+T at tab limit input attachment unavailable')
        else:check('CtrlTCannotExceedEightTabs',routed)
        for index,tab in enumerate(reversed(extra_tabs)):
            native.send(window,0x8000+26,tab)
            check('capacityTabCloseReleasesController:'+str(index),wait(lambda:q(34)==q(38)==7-index and q(37)==0))
        check('capacityCloseRestoresOriginalThreeTabs',q(34)==q(38)==3 and q(35)==tab_c and USER32.IsWindowEnabled(USER32.GetDlgItem(window,190)))
        check('originalFirstTabSurvivesCapacityCycle',activate_tab(tab_a) and native.control_text(address)==base+'/first' and q(24)==110)
        check('originalFormSurvivesCapacityCycle',wait(lambda:form_state('/first')['visibility']=='visible' and
              form_state('/first')['instance']==form_a['instance'] and form_state('/first')['draft']==form_a['draft']))
        check('originalSecondTabSurvivesCapacityCycle',activate_tab(tab_b) and native.control_text(address)==base+'/tab-b-two' and q(24)==120)
        routed=modifier_key(ord('W'),lambda:q(34)==2 and q(35)!=tab_b)
        if routed is None:
            not_tested.append('Owned Ctrl+W input attachment unavailable');native.send(window,0x8000+26,tab_b)
        else:check('CtrlWClosesOnlyActiveBrowserTab',routed)
        check('closeActiveTabReleasesOneController',wait(lambda:q(34)==q(38)==2))
        check('activateRemainingThirdTab',activate_tab(tab_c))
        native.send(bar,0x208,0,((bounds.bottom//2)<<16)|round(16*scale))
        check('middleClickClosesBackgroundFirstTab',wait(lambda:q(34)==q(38)==1 and q(35)==tab_c))
        check('backgroundCloseKeepsActivePage',native.control_text(address)==base+'/tab-c' and q(24)==100)
        native.send(window,0x8000+26,tab_c)
        check('closingLastTabDisablesEngine',wait(lambda:q(10)==q(11)==q(34)==q(35)==q(37)==q(38)==0))
        native.set_control_text(address,base+'/first');cmd(155)
        check('enableAfterLastCloseCreatesOneFreshVerifiedTab',wait(lambda:q(10)==2 and q(12)==1 and q(34)==q(38)==1 and q(37)==0,25))
        check('reEnableAfterTabCloseLoadsConfiguredAddress',wait(lambda:q(13)==1 and native.control_text(address)==base+'/first',12))
        new_tab('disableAllSecondTab')
        owned_browser=native.KERNEL32.OpenProcess(0x00100000,False,q(11))
        check('multiTabBrowserProcessCanBeObserved',bool(owned_browser))
        cmd(155)
        check('disableClosesEveryTabAndController',q(10)==q(11)==q(12)==q(34)==q(35)==q(37)==q(38)==0)
        try:check('disableAllTabsBrowserProcessExits',native.KERNEL32.WaitForSingleObject(owned_browser,8000)==0)
        finally:native.KERNEL32.CloseHandle(owned_browser)
        native.set_control_text(address,base+'/first');cmd(155)
        check('enableAfterMultiTabDisableStartsOneVerifiedTab',wait(lambda:q(10)==2 and q(12)==1 and q(34)==q(38)==1 and q(37)==0,25))
        check('multiTabDisableDoesNotRestoreOldTabUrls',wait(lambda:q(13)==1 and native.control_text(address)==base+'/first',12))
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
        check('displayChangeClosesBrowser',q(10)==0 and q(11)==0 and q(34)==q(38)==0)
        native.send(window,0x10,timeout=6000);app.wait(timeout=8)
        app=subprocess.Popen([str(EXE)],cwd=EXE.parent,env=env,startupinfo=startup)
        check('restartWithExistingWebProfile',wait(get_window))
        check('restartStillDefaultsBrowserOff',wait(lambda:native.send(window,0x800E,timeout=6000),20) and q(10)==q(11)==q(34)==q(38)==0)
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
    except Exception as e:
        failure={'type':type(e).__name__,'message':str(e)}
        if app and app.poll() is None and window:
            try:metrics['failureState']={'baseGate':native.send(window,0x800E,timeout=6000),'caption':native.control_text(window),
                'zoom':q(24),'tab':q(35),'state':q(10),'address':native.control_text(address),'recentViewport':viewports[-10:]}
            except Exception:pass
    finally:
        if app and app.poll() is None:
            for w in native.app_windows(app.pid):native.send(w,0x10,timeout=6000)
            try:app.wait(timeout=8)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
        server.shutdown();server.server_close()
        report={'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'checks':checks,'failure':failure,
                'exeSha256':hashlib.sha256(EXE.read_bytes()).hexdigest(),'metrics':metrics,
                'method':'Shipping EXE; actual per-controller rendered positive and excluded WGC canary; isolated profile; native tab/shortcut messages; local HTTP fixtures report viewport DPR and per-document unsent form state',
                'notTested':['ChatGPT authenticated session and third-party OAuth','Physical meeting receivers and two physical monitors','Missing Evergreen Runtime on a clean machine','Physical keyboard input, Ctrl+wheel and WebView-focused shortcut delivery','Physical typing: synthetic in-page unsent drafts validate DOM retention only',*not_tested]}
        (folder/'browser-regression.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps({k:report[k] for k in ('passed','failed','failure','metrics')}),flush=True)
    return 1 if failure or report['failed'] else 0
if __name__=='__main__':raise SystemExit(main())
