"""Exercise shipping presentation sets and switching with an isolated local profile."""
import ctypes
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time
import uuid
import production_integration as api

U,send=api.USER32,api.send
U.SetWindowTextW.argtypes=[ctypes.c_void_p,ctypes.c_wchar_p]
U.SetWindowTextW.restype=ctypes.c_int

def main():
    checks=[];process=None;window=None
    root=api.ROOT/'artifacts'/'qa-presentation'/uuid.uuid4().hex
    root.mkdir(parents=True);profile=root/'User';profile.mkdir()
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)})
        print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def launch():
        nonlocal process,window
        process=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
        window=api.wait_for(lambda:next(iter(api.native.app_windows(process.pid)),None),10)
        check('owned production window starts',window)
        check('real WGC gate passes before private actions',api.wait_for(lambda:send(window,api.GATE,timeout=6000),20))
    def cmd(n):send(window,api.WM_COMMAND,n,timeout=10000)
    def ctl(n):return U.GetDlgItem(window,n)
    def text(n,value):check('native text input succeeds',api.native.set_control_text(ctl(n),value))
    def approved(n):
        U.PostMessageW(window,api.WM_COMMAND,n,0)
        popup=api.wait_for(lambda:api.owned_popup(process.pid),5)
        check('replacement requires an owned confirmation',popup)
        U.PostMessageW(popup,0x200,0,(10<<16)|10)
        U.PostMessageW(popup,0x100,0x0D,0)
        check('confirmation closes',api.wait_for(lambda:not api.owned_popup(process.pid),5))
        time.sleep(.15)
    try:
        check('personal application session is untouched',not api.native.app_windows())
        launch();data=profile/'QuietShare'
        first=root/'chapter-one.txt';first.write_text('First document\n'+'line\n'*120,encoding='utf-8')
        second=root/'chapter-two.py';second.write_text('value = 2\n',encoding='utf-8')
        cmd(127);text(110,'PRIVATE_SYNTHETIC_SCRIPT Vietnamese notes')
        api.multi_drop(window,[first]);send(ctl(163),0x7);text(163,'37');send(ctl(163),api.WM_KEYDOWN,0x0D)
        check('manual document size is committed',send(window,api.QUERY,4)==37*100//24)
        api.multi_drop(window,[second]);active=send(window,api.QUERY,2)
        send(ctl(170),0x102,ord('x'))
        cmd(285);cmd(287)
        check('empty name reports a handbook code','QS-W002' in api.native.control_text(ctl(294)))
        text(286,'Weekly presentation');cmd(287)
        check('set save refuses unsaved file edits',send(window,api.QUERY,66)==0 and 'value = 2' in second.read_text())
        check('save refusal has a handbook code without relying on footer',api.visible(ctl(294)) and 'QS-W003' in api.native.control_text(ctl(294)))
        cmd(251);cmd(287)
        check('set is saved through the real interface',send(window,api.QUERY,66)==1)
        encrypted=(data/'presentations.dpapi').read_bytes()
        check('saved library contains no plaintext paths or script',all(x not in encrypted for x in (b'chapter-one',b'PRIVATE_SYNTHETIC_SCRIPT','Weekly presentation'.encode('utf-16-le'))))
        cmd(280);text(281,'chapter-one')
        check('quick switch filters by filename',send(ctl(282),0x18B)==1)
        cmd(283);check('quick switch activates a different real document',send(window,api.QUERY,2)!=active and send(window,api.QUERY,1)==3)
        cmd(280);text(281,str(second));check('quick switch searches full paths',send(ctl(282),0x18B)==1)
        cmd(283);check('quick switch preserves other document tabs',send(window,api.QUERY,2)==active and send(window,api.QUERY,1)==3)
        cmd(285);text(281,'no matching collection')
        check('empty filtered results remain explanatory',send(ctl(282),0x18B)==1)
        check('empty result disables Open instead of accepting a dead click',not U.IsWindowEnabled(ctl(283)))
        cmd(283);check('empty result does not open a command',api.visible(ctl(281)))
        cmd(284);cmd(122)
        U.PostMessageW(window,0x10,0,0);process.wait(timeout=10);check('owned app exits cleanly',process.returncode==0);process=None
        second.rename(root/'moved-chapter-two.py')
        launch();check('saved sets survive application restart',send(window,api.QUERY,66)==1)
        cmd(127);text(110,'CURRENT_SCRIPT_BEFORE_RESTORE')
        backup=data/'notes-before-set.dpapi';backup.mkdir()
        cmd(285);approved(283)
        check('backup failure reports a handbook code','QS-W006' in api.native.control_text(ctl(294)))
        check('backup failure preserves the outgoing session',send(window,api.QUERY,1)==1 and api.native.control_text(ctl(110))=='CURRENT_SCRIPT_BEFORE_RESTORE')
        backup.rmdir();approved(283)
        check('missing file is skipped while available file opens',send(window,api.QUERY,1)==2)
        cmd(127);check('saved script is restored exactly',api.native.control_text(ctl(110))=='PRIVATE_SYNTHETIC_SCRIPT Vietnamese notes')
        cmd(280);text(281,'chapter-one');cmd(283)
        check('saved reading size survives actual application restart',send(window,api.QUERY,4)==37*100//24)
        check('previous script receives an exact encrypted recovery copy',backup.is_file() and api.native.decrypt(backup)=='CURRENT_SCRIPT_BEFORE_RESTORE')
        cmd(285);approved(288)
        check('deleting set keeps original files',send(window,api.QUERY,66)==0 and first.is_file() and (root/'moved-chapter-two.py').is_file())
        cmd(139)
        check('projection board exposes both monitor selectors',api.visible(ctl(160)) and api.visible(ctl(161)))
        check('projection board exposes recovery and preview controls',api.visible(ctl(289)) and api.visible(ctl(290)) and api.visible(ctl(291)))
        for width,height in ((430,320),(800,680)):
            scale=U.GetDpiForWindow(window)/96
            U.SetWindowPos(window,None,0,0,round(width*scale),round(height*scale),0x16)
            time.sleep(.1)
            a,b,c,d=[api.rect(ctl(n),window) for n in (289,140,141,142)]
            check(f'projection actions do not overlap at {width}x{height}',a[2]<=b[0] and b[2]<=c[0] and c[2]<=d[0])
            check(f'preview remains available at {width}x{height}',api.visible(ctl(295)))
            cmd(291);check(f'recovery is accessible at {width}x{height}',api.visible(ctl(293)) and bool(api.native.control_text(ctl(293))))
            cmd(291)
        if(send(window,api.QUERY,30)==1):
            cmd(141)
            check('failed Play opens actionable diagnostic board','QS-P001' in api.native.control_text(ctl(293)) and api.visible(ctl(291)))
            cmd(291);check('error recovery names Extend','Extend' in api.native.control_text(ctl(293)))
        U.PostMessageW(window,0x10,0,0);process.wait(timeout=10);check('owned app exits cleanly',process.returncode==0);process=None
        corrupt=encrypted[:len(encrypted)//2];(data/'presentations.dpapi').write_bytes(corrupt)
        launch();cmd(285)
        check('unreadable library reports a handbook code','QS-W001' in api.native.control_text(ctl(294)))
        check('unreadable library disables replacement',not U.IsWindowEnabled(ctl(287)))
        cmd(287);check('unreadable library is preserved byte for byte',(data/'presentations.dpapi').read_bytes()==corrupt)
        U.PostMessageW(window,0x10,0,0);process.wait(timeout=10);check('owned app exits cleanly',process.returncode==0);process=None
    finally:
        if process and process.poll() is None:
            for h in api.native.app_windows(process.pid):U.PostMessageW(h,0x10,0,0)
            try:process.wait(timeout=8)
            except subprocess.TimeoutExpired:process.terminate();process.wait(timeout=5)
        report={'passed':sum(x['passed'] for x in checks),'failed':sum(not x['passed'] for x in checks),'checks':checks,'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest(),'profile':'isolated synthetic profile','physicalReceiver':'not tested'}
        (api.ROOT/'artifacts'/'presentation-integration.json').write_text(json.dumps(report,indent=2),encoding='utf-8')

if __name__=='__main__':main()
