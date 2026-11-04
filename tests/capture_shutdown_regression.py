"""Bounded real-WGC startup/verification/shutdown cycles in isolated profiles."""
import hashlib
import json
import os
import subprocess
import uuid
import production_integration as api

def main():
    root=api.ROOT/'artifacts'/'qa-capture-shutdown'/uuid.uuid4().hex
    root.mkdir(parents=True);checks=[];process=None
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)})
        print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    try:
        check('personal application session remains untouched',not api.native.app_windows())
        for cycle in range(16):
            profile=root/str(cycle);profile.mkdir()
            env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
            process=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
            window=api.wait_for(lambda:next(iter(api.native.app_windows(process.pid)),None),8)
            check(f'{cycle}: owned window starts',window)
            check(f'{cycle}: real WGC verifies',api.wait_for(lambda:api.send(window,api.GATE,timeout=6000),20))
            api.send(window,api.WM_COMMAND,139)
            api.send(window,api.GATE,timeout=6000)
            api.USER32.PostMessageW(window,0x10,0,0)
            process.wait(timeout=10)
            check(f'{cycle}: WGC process shuts down cleanly',process.returncode==0)
            process=None
    finally:
        if process and process.poll() is None:
            for h in api.native.app_windows(process.pid):api.USER32.PostMessageW(h,0x10,0,0)
            try:process.wait(timeout=8)
            except subprocess.TimeoutExpired:process.terminate();process.wait(timeout=5)
        report={'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),
                'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest(),'cycles':16,'physicalReceiver':'not tested'}
        (api.ROOT/'artifacts'/'capture-shutdown.json').write_text(json.dumps(report,indent=2),encoding='utf-8')

if __name__=='__main__':main()
