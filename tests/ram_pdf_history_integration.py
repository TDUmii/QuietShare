"""Shipping continuous PDF, explicit tab discard and private support export."""
import ctypes,hashlib,json,os,subprocess,time,uuid
from pathlib import Path
import production_integration as api
U,send=api.USER32,api.send
def main():
    root=api.ROOT/'artifacts/qa-ram-pdf-history'/uuid.uuid4().hex;profile=root/'User';profile.mkdir(parents=True);out=root/'Reports';out.mkdir()
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile);app=None;window=None;checks=[];failure=None
    report_cache={}
    def reports():
        # Export flushes and closes before success. A directory entry can be
        # visible earlier, so wait for complete readable JSON rather than its name.
        for path in out.glob('*.json'):
            if path in report_cache:continue
            try:
                text=path.read_text(encoding='utf-8');json.loads(text);report_cache[path]=text
            except (OSError,ValueError):pass
        return report_cache
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)});print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def q(n):return send(window,api.QUERY,n)
    def cmd(n):U.PostMessageW(window,api.WM_COMMAND,n,0);time.sleep(.15)
    def ctl(n):return U.GetDlgItem(window,n)
    def launch():
        nonlocal app,window
        app=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
        window=api.wait_for(lambda:next(iter(api.native.app_windows(app.pid)),None),10)
        check('owned shipping window opens',window);check('actual WGC gate passes',api.wait_for(lambda:send(window,api.GATE,timeout=6000),20))
    def close():
        nonlocal app
        U.PostMessageW(window,0x10,0,0);app.wait(timeout=10);check('owned app exits cleanly',app.returncode==0);app=None
    try:
        check('personal application remains untouched',not api.native.app_windows());launch()
        cmd(123);cmd(346);cmd(379)
        popup=api.wait_for(lambda:api.owned_popup(app.pid,'QuietShareLocalPicker'),5);check('protected report folder picker opens',popup)
        affinity=api.wintypes.DWORD();U.GetWindowDisplayAffinity(popup,ctypes.byref(affinity));check('folder picker has capture exclusion',affinity.value==0x11)
        check('report folder accepts owned path',api.native.set_control_text(U.GetDlgItem(popup,13),str(out)))
        send(popup,api.WM_COMMAND,17);U.PostMessageW(popup,api.WM_COMMAND,1,0)
        check('folder picker closes after explicit choice',api.wait_for(lambda:not U.IsWindow(popup),5))
        cmd(127);check('synthetic private script is accepted',api.native.set_control_text(ctl(110),'PRIVATE_NOTES_NEVER_EXPORT'))
        api.multi_drop(window,[api.ROOT/'artifacts/qa-viewers/many.pdf'])
        check('PDF opens',api.wait_for(lambda:q(6)==120,10))
        check('PDF paints several adjacent pages',api.wait_for(lambda:q(87)>=2,5))
        check('PDF image cache is bounded',0<q(89)<=50*1024*1024)
        pdf=next(h for h in api.children(window,app.pid) if api.class_name(h)=='QuietSharePdfCanvas')
        before=(q(5),ctypes.c_ssize_t(q(88)).value)
        send(pdf,0x20A,(ctypes.c_ushort(-60).value<<16))
        check('partial wheel input advances a continuous viewport',api.wait_for(lambda:(q(5),ctypes.c_ssize_t(q(88)).value)!=before,5))
        for _ in range(6):send(pdf,0x20A,(ctypes.c_ushort(-120).value<<16))
        check('wheel reaches following pages without resetting to top',q(5)>1 and ctypes.c_ssize_t(q(88)).value!=0)
        saved=(q(5),ctypes.c_ssize_t(q(88)).value);cmd(154);cmd(120)
        check('return from browser preserves continuous PDF position',(q(5),ctypes.c_ssize_t(q(88)).value)==saved)
        bad=root/'broken.pdf';bad.write_bytes(b'%PDF-1.7\ninvalid PRIVATE_PATH_MARKER\n%%EOF')
        api.multi_drop(window,[bad]);cmd(370)
        text=api.native.control_text(ctl(373));check('file viewer errors appear in history','QS-F003' in text)
        cmd(378);check('explicit export writes one JSON report',api.wait_for(lambda:len(reports())==1,5))
        payload=next(iter(report_cache.values()));report=json.loads(payload)
        check('export has fixed version environment and error fields',report['version']=='0.23.1' and report['windowsBuild']>0 and any(e['code']=='QS-F003' for e in report['events']))
        check('export includes current and per-error memory snapshots',report['schema']==2 and report['memoryKnown'] and report['commitLimitMiB']>=report['availableCommitMiB'] and all(e['memoryKnown'] and e['commitLimitMiB']>=e['availableCommitMiB'] for e in report['events']))
        check('export excludes private text file paths and URLs',all(s not in payload for s in ['PRIVATE_NOTES_NEVER_EXPORT','PRIVATE_PATH_MARKER',str(root),'http','broken.pdf']))
        cmd(105);cmd(378);check('fixed-field report can export while masked',api.wait_for(lambda:len(reports())==2,5))
        cmd(105);cmd(154);cmd(155)
        check('owned browser starts with real capture verification',api.wait_for(lambda:q(10)==2 and q(13)==1,25))
        cmd(370);cmd(378)
        check('browser environment report is written',api.wait_for(lambda:len(reports())==3,5))
        web_report=json.loads(report_cache[max(report_cache,key=lambda p:p.stat().st_mtime_ns)])
        check('report measures dedicated browser and host private bytes',web_report['browserTabs']==1 and web_report['browserPrivateBytes']>0 and web_report['browserMemoryComplete'] and web_report['hostPrivateBytes']>0 and web_report['hostMemoryComplete'])
        close();launch();cmd(370);check('history survives a process restart','QS-F003' in api.native.control_text(ctl(373)))
        close();journal=profile/'QuietShare/error-history.dat';raw=journal.read_bytes();journal.write_bytes(raw+b'INJECTED_PRIVATE_URL https://secret.invalid')
        launch();cmd(370);text=api.native.control_text(ctl(373));check('malformed history is reported without disclosing its contents','QS-J001' in text and 'INJECTED_PRIVATE_URL' not in text)
        count=len(list(out.glob('*.json')));cmd(378);check('malformed history is preserved and never exported',journal.read_bytes()==raw+b'INJECTED_PRIVATE_URL https://secret.invalid' and len(list(out.glob('*.json')))==count);close()
    except Exception as e:failure=repr(e);raise
    finally:
        if app and app.poll() is None:
            for h in api.native.app_windows(app.pid):U.PostMessageW(h,0x10,0,0)
            try:app.wait(timeout=10)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
        result={'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'failure':failure,'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest()}
        (api.ROOT/'artifacts/ram-pdf-history-integration.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
if __name__=='__main__':main()
