"""Owned shipping process, isolated profile and synthetic files only."""
from __future__ import annotations
import ctypes
from ctypes import wintypes
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time
import uuid
import production_integration as api

ROOT, USER32, send = api.ROOT, api.USER32, api.send

class MemoryCounters(ctypes.Structure):
    _fields_=[('cb',wintypes.DWORD),('faults',wintypes.DWORD)]+[(name,ctypes.c_size_t) for name in
        ('peakWorkingSet','workingSet','peakPaged','paged','peakNonPaged','nonPaged','pagefile','peakPagefile')]

def resources(process):
    counters=MemoryCounters();counters.cb=ctypes.sizeof(counters)
    psapi=ctypes.WinDLL('psapi',use_last_error=True)
    psapi.GetProcessMemoryInfo.argtypes=[wintypes.HANDLE,ctypes.POINTER(MemoryCounters),wintypes.DWORD]
    handle=wintypes.HANDLE(int(process._handle))
    if not psapi.GetProcessMemoryInfo(handle,ctypes.byref(counters),counters.cb):raise ctypes.WinError()
    times=[wintypes.FILETIME() for _ in range(4)]
    api.KERNEL32.GetProcessTimes.argtypes=[wintypes.HANDLE]+[ctypes.POINTER(wintypes.FILETIME)]*4
    if not api.KERNEL32.GetProcessTimes(handle,*[ctypes.byref(t) for t in times]):raise ctypes.WinError()
    cpu=sum((t.dwHighDateTime<<32)|t.dwLowDateTime for t in times[2:])/10_000_000
    USER32.GetGuiResources.argtypes=[wintypes.HANDLE,wintypes.DWORD]
    return {'cpu':cpu,'workingSet':counters.workingSet,'gdi':USER32.GetGuiResources(handle,0)}

def main():
    root=ROOT/'artifacts'/'qa-workspace-ui'/uuid.uuid4().hex
    root.mkdir(parents=True)
    profile=root/'User';profile.mkdir()
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    checks=[];app=None;failure=None;resource_report={}
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)})
        print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def launch():
        process=subprocess.Popen([str(api.EXE)],env=env,cwd=str(ROOT/'dist'),startupinfo=startup)
        hwnd=api.wait_for(lambda:next(iter(api.native.app_windows(process.pid)),None),8)
        if not hwnd:raise RuntimeError('Owned application window unavailable')
        api.wait_for(lambda:USER32.GetDlgItem(hwnd,400),8)
        check('actual WGC gate ready',api.wait_for(lambda:send(hwnd,api.GATE,timeout=6000),20))
        USER32.ShowWindow(hwnd,4)
        return process,hwnd
    def command(id):send(window,api.WM_COMMAND,id)
    def close():
        USER32.PostMessageW(window,0x10,0,0);app.wait(timeout=10)
    startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=0
    try:
        check('personal application windows untouched',not api.native.app_windows())
        app,window=launch()
        data=profile/'QuietShare'
        check('first launch has no sample history',not (data/'recent.dpapi').exists() and not api.visible(USER32.GetDlgItem(window,400)))
        check('fresh workspace has no fabricated script or permanent script tab',api.native.control_text(USER32.GetDlgItem(window,110))=='' and send(window,api.QUERY,64)==0)
        check('Windows capture exclusion acceptance is read back independently',send(window,api.QUERY,65)==1)
        for width,height in ((430,320),(460,360),(800,680)):
            scale=USER32.GetDpiForWindow(window)/96
            USER32.SetWindowPos(window,None,0,0,round(width*scale),round(height*scale),0x16)
            time.sleep(0.1)
            file_card=api.rect(USER32.GetDlgItem(window,126),window)
            project_card=api.rect(USER32.GetDlgItem(window,131),window)
            clear=api.rect(USER32.GetDlgItem(window,252),window)
            previous=api.rect(USER32.GetDlgItem(window,254),window)
            next_page=api.rect(USER32.GetDlgItem(window,253),window)
            check(f'Home cards and recent actions do not overlap at {width}x{height}',file_card[2]<project_card[0]
                  and file_card[3]<previous[1] and previous[2]<=next_page[0] and next_page[2]<=clear[0])
        check('recent navigation uses accessible action names instead of angle brackets',api.native.control_text(USER32.GetDlgItem(window,254)) not in ('<','>')
              and api.native.control_text(USER32.GetDlgItem(window,253)) not in ('<','>'))
        first=root/'real-opened.py';first.write_bytes(b'value = 1\n')
        second=root/'another.txt';second.write_bytes(b'notes\n')
        api.multi_drop(window,[first]);code=USER32.GetDlgItem(window,170)
        check('code editing defaults enabled',send(code,2140)==0)
        send(code,0x102,ord('x'));command(251)
        check('explicit file save writes edits',first.read_bytes()==b'xvalue = 1\n' and send(code,2159)==0)
        send(code,0x102,ord('y'))
        file_id=send(window,api.QUERY,2)
        send(code,2025,2)
        for destination,label in ((154,'browser'),(122,'Home'),(123,'Settings')):
            command(destination);command(120)
            check(f'return from {label} retains the selected file and unsaved native buffer',send(window,api.QUERY,2)==file_id
                  and send(code,2006)==len(b'xyvalue = 1\n') and send(code,2008)==2 and send(code,2159)==1 and api.visible(code))
        check('file tab strip excludes the optional script',send(window,api.QUERY,64)==1)
        command(127)
        editor=USER32.GetDlgItem(window,110)
        check('compose script is explicit and adds no permanent file tab',send(window,api.QUERY,2)==0 and send(window,api.QUERY,64)==1 and api.visible(editor))
        api.native.set_control_text(editor,'Prepared script fixture')
        send(window,api.ACTIVATE,file_id)
        # Reopening the code control resets its native save-point flag; the
        # document model still owns the dirty draft. Verify data and disk here,
        # then prove the draft is saved explicitly below.
        check('returning from explicit script preserves file edits',send(code,2006)==len(b'xyvalue = 1\n')
              and send(window,api.QUERY,2)==file_id and api.visible(code) and first.read_bytes()==b'xvalue = 1\n')
        command(127)
        check('explicit script can be reopened without losing prepared text',api.native.control_text(editor)=='Prepared script fixture')
        send(window,api.ACTIVATE,file_id)
        api.multi_drop(window,[second]);api.multi_drop(window,[first])
        check('unsaved code draft survives tab switches',send(code,2006)==len(b'xyvalue = 1\n') and first.read_bytes()==b'xvalue = 1\n')
        command(251)
        check('draft saves only on explicit action',first.read_bytes()==b'xyvalue = 1\n')
        command(123);command(201);command(250)
        check('editing switch locks native code control',send(code,2140)==1)
        command(250);command(122)
        first_row=api.native.control_text(USER32.GetDlgItem(window,400))
        check('real recent file visible after open',first_row==first.name and (data/'recent.dpapi').exists())
        command(401);api.multi_drop(window,[second]);command(122)
        check('pinned file stays first',api.native.control_text(USER32.GetDlgItem(window,400))==first.name)
        command(402)
        check('removing recent entry never deletes source',first.exists() and api.native.control_text(USER32.GetDlgItem(window,400))==second.name)
        command(400)
        check('quick reopen activates the real file',api.native.control_text(USER32.GetDlgItem(window,110)).startswith('notes'))
        editor=USER32.GetDlgItem(window,110);send(editor,0x102,ord('a'));command(251)
        check('plain text supports explicit save',second.read_bytes().startswith(b'a'))
        api.multi_drop(window,[first]);send(code,0x102,ord('z'));first.write_bytes(b'external content changed\n')
        command(251)
        check('external conflict preserves disk and draft',first.read_bytes()==b'external content changed\n' and send(code,2159)==1)
        USER32.PostMessageW(window,api.WM_COMMAND,148,0)
        popup=api.wait_for(lambda:api.owned_popup(app.pid),3)
        affinity=wintypes.DWORD()
        check('unsaved prompt is capture excluded',popup and USER32.GetWindowDisplayAffinity(popup,ctypes.byref(affinity)) and affinity.value==0x11)
        USER32.PostMessageW(popup,api.WM_KEYDOWN,0x1b,0)
        api.wait_for(lambda:not api.owned_popup(app.pid),3)
        time.sleep(0.15)
        check('cancel close retains unsaved document',send(code,2159)==1)
        USER32.SetForegroundWindow(window)
        USER32.PostMessageW(window,api.WM_COMMAND,148,0)
        popup=api.wait_for(lambda:(h if (h:=api.owned_popup(app.pid)) and USER32.IsWindowVisible(h) else None),3)
        check('second unsaved prompt is ready for selection',bool(popup))
        bounds=api.rect(popup);row=(bounds[3]-bounds[1]-10)//4
        USER32.PostMessageW(popup,0x202,0,(5+2*row+row//2)<<16|30)
        check('discard closes the edited file without a stuck popup',api.wait_for(lambda:send(window,api.QUERY,2)==0 and not api.owned_popup(app.pid),5))
        command(122);command(252)
        check('clear removes all history including pins',not api.visible(USER32.GetDlgItem(window,400)))
        baseline=resources(app);sample_start=time.perf_counter();time.sleep(2);idle=resources(app)
        resource_report['idleCpuPercentOfOneCore']=round(100*(idle['cpu']-baseline['cpu'])/(time.perf_counter()-sample_start),2)
        resource_report['idleWorkingSetMiB']=round(idle['workingSet']/1024**2,2)
        check('idle process has no busy polling loop',resource_report['idleCpuPercentOfOneCore']<50)
        for _ in range(20):command(123);command(200);command(122)
        after_navigation=resources(app)
        resource_report['navigationGdiDelta']=after_navigation['gdi']-idle['gdi']
        resource_report['navigationWorkingSetGrowthMiB']=round((after_navigation['workingSet']-idle['workingSet'])/1024**2,2)
        check('repeated navigation has bounded native resources',resource_report['navigationGdiDelta']<=16 and resource_report['navigationWorkingSetGrowthMiB']<32)
        USER32.SetWindowPos(window,None,50,50,620,540,0x14)
        before=api.rect(window);close();check('clean exit after real edits',app.returncode==0)
        stored=(data/'window-size.dat').read_bytes()
        check('window dimensions persist in bounded local record',len(stored)==12 and struct.unpack('<3I',stored)[0]==1)
        app,window=launch();after=api.rect(window)
        check('last window dimensions restored',(before[2]-before[0],before[3]-before[1])==(after[2]-after[0],after[3]-after[1]))
        check('cleared history stays empty after restart',not api.visible(USER32.GetDlgItem(window,400)))
        command(123);command(200);command(249)
        USER32.SetWindowPos(window,None,50,50,700,600,0x14);close()
        check('disabled window memory does not overwrite stored size',(data/'window-size.dat').read_bytes()==stored)
        app,window=launch();after=api.rect(window)
        check('disabled window memory uses default dimensions',after[2]-after[0]!=before[2]-before[0])
        command(123);command(201);command(250);close()
        app,window=launch();api.multi_drop(window,[first]);code=USER32.GetDlgItem(window,170)
        check('editing preference persists across restart',send(code,2140)==1)
        command(122);command(105)
        check('mask conceals recent labels and rows',not api.visible(USER32.GetDlgItem(window,400)) and api.native.control_text(USER32.GetDlgItem(window,400))=='')
        close()
    except Exception as error:
        import traceback
        failure={'type':type(error).__name__,'message':str(error),'traceback':traceback.format_exc()}
    finally:
        if app and app.poll() is None:
            app.terminate();app.wait(timeout=5)
        report={'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'failure':failure,
                'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest(),'physicalTwoDisplayTest':False,'resources':resource_report}
        (ROOT/'artifacts/workspace-integration.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
        print(json.dumps({k:report[k] for k in ('passed','failed','failure')}),flush=True)
    return int(bool(failure or report['failed']))

if __name__=='__main__':raise SystemExit(main())
