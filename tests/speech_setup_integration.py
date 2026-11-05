"""Explicit post-release download QA. Never run or download models by default."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import uuid
import production_integration as api


def digest(path):
    value=hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda:stream.read(1024*1024),b''):value.update(chunk)
    return value.hexdigest()


def main():
    sys.stdout.reconfigure(encoding='utf-8')
    if '--online' not in sys.argv:
        print('Not run. Pass --online to explicitly download the optional runtime and Small model.');return 0
    checks=[];process=None;failure=None
    root=api.ROOT/'artifacts/qa-speech-setup'/uuid.uuid4().hex;profile=root/'User';profile.mkdir(parents=True)
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)});print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    try:
        check('personal application session untouched',not api.native.app_windows())
        process=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
        window=api.wait_for(lambda:next(iter(api.native.app_windows(process.pid)),None),10)
        check('production window starts',window)
        check('actual WGC gate passes',api.wait_for(lambda:api.send(window,api.GATE,timeout=6000),20))
        api.send(window,api.WM_COMMAND,313)
        check('accuracy model is selected by default','Small' in api.native.control_text(api.USER32.GetDlgItem(window,331)))
        speech=profile/'QuietShare/Speech';check('no runtime or model is downloaded at startup',not speech.exists())
        api.send(window,api.WM_COMMAND,330)
        started=time.monotonic();last=''
        while time.monotonic()-started<1900:
            status=api.native.control_text(api.USER32.GetDlgItem(window,332))
            if status!=last:print(status.replace('\r','').replace('\n',' | '),flush=True);last=status
            if api.send(window,api.QUERY,78):break
            if 'QS-R008' in status:raise AssertionError(status)
            time.sleep(.5)
        check('real in-app setup finishes with a ready recognizer',bool(api.send(window,api.QUERY,78)))
        check('downloaded helper matches reviewed SHA256',digest(speech/'whisper-cli.exe')=='96a4d1b1342e33d39ebf6cd38997330cc7ed869e81e4f1e215858f407132dd93')
        check('downloaded Small model matches official pinned SHA256',digest(speech/'ggml-small.bin')=='1be3a9b2063867b937e64e2ec7483364a79917e157fa98c5d94b5c1fffea987b')
        check('optional licenses accompany runtime',all((speech/n).is_file() for n in ['LICENSE-whisper.cpp.txt','LICENSE-ggml.txt','LICENSE-model.txt','README.md']))
        check('setup never starts meeting recording',not list((profile/'QuietShare/Meetings').glob('*.wav')))
        api.USER32.PostMessageW(window,0x10,0,0);process.wait(timeout=10);check('owned application exits cleanly',process.returncode==0)
    except Exception as error:failure=repr(error);raise
    finally:
        if process and process.poll() is None:
            for window in api.native.app_windows(process.pid):api.USER32.PostMessageW(window,0x10,0,0)
            try:process.wait(timeout=8)
            except subprocess.TimeoutExpired:process.terminate();process.wait(timeout=5)
        (api.ROOT/'artifacts/speech-setup-integration.json').write_text(json.dumps({'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'failure':failure,'exeSha256':digest(api.EXE)},indent=2),encoding='utf-8')
    return 0


if __name__=='__main__':raise SystemExit(main())
