"""Shipping UI checks with synthetic PDF/transcript files and an isolated profile."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time
import uuid
import production_integration as api
from workspace_integration import resources

U,send=api.USER32,api.send


def main():
    checks=[];process=None;window=None;failure=None;metrics={}
    root=api.ROOT/'artifacts/qa-reading-ui'/uuid.uuid4().hex
    profile=root/'User';profile.mkdir(parents=True)
    fixtures=root/'import';fixtures.mkdir()
    vtt=fixtures/'meeting.vtt'
    vtt.write_text('WEBVTT\n\n00:00:01.000 --> 00:00:03.000\n<v Minh>Confirm the delivery date.</v>\n\n00:00:04.000 --> 00:00:06.000\n<v An>Send the notes tomorrow.</v>\n',encoding='utf-8')
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile)
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)});print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def cmd(n):send(window,api.WM_COMMAND,n,timeout=15000)
    def ctl(n):return U.GetDlgItem(window,n)
    def text(n,value):check('owned edit accepts synthetic text',api.native.set_control_text(ctl(n),value))
    def query(n):return send(window,api.QUERY,n)
    def shortcut(key,receiver,posted=False):
        own=api.KERNEL32.GetCurrentThreadId();target=U.GetWindowThreadProcessId(window,None)
        message=api.wintypes.MSG();U.PeekMessageW(api.ctypes.byref(message),None,0,0,0)
        check('keyboard state is shared only with the owned test window',U.AttachThreadInput(own,target,True))
        previous=(api.ctypes.c_ubyte*256)();U.GetKeyboardState(previous);state=(api.ctypes.c_ubyte*256).from_buffer_copy(previous);state[0x11]=0x80
        try:
            U.SetKeyboardState(state)
            if posted:U.PostMessageW(receiver,api.WM_KEYDOWN,key,0);time.sleep(.08)
            else:send(receiver,api.WM_KEYDOWN,key)
        finally:U.SetKeyboardState(previous);U.AttachThreadInput(own,target,False)
    def launch():
        nonlocal process,window
        process=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
        window=api.wait_for(lambda:next(iter(api.native.app_windows(process.pid)),None),10)
        check('production window starts',window)
        check('real WGC gate passes',api.wait_for(lambda:send(window,api.GATE,timeout=6000),20))
    def close():
        nonlocal process
        U.PostMessageW(window,0x10,0,0);process.wait(timeout=10)
        check('owned app exits cleanly',process.returncode==0);process=None
    def menu(command,index):
        U.PostMessageW(window,api.WM_COMMAND,command,0)
        popup=api.wait_for(lambda:api.owned_popup(process.pid),5);check('protected menu opens',popup)
        send(popup,0x200,0,(10<<16)|10)
        for _ in range(index):send(popup,api.WM_KEYDOWN,0x28)
        U.PostMessageW(popup,api.WM_KEYDOWN,0x0D,0)
        check('menu action completes',api.wait_for(lambda:not api.owned_popup(process.pid),5));time.sleep(.15)
    def import_provider():
        U.PostMessageW(window,api.WM_COMMAND,263,0)
        picker=api.wait_for(lambda:api.owned_popup(process.pid,'QuietShareLocalPicker'),5);check('protected provider picker opens',picker)
        api.native.set_control_text(U.GetDlgItem(picker,13),str(fixtures));send(picker,api.WM_COMMAND,17)
        listing=U.GetDlgItem(picker,12);check('provider fixture is the only candidate',send(listing,0x18B)==1)
        send(listing,0x185,1,0);U.PostMessageW(picker,api.WM_COMMAND,1,0)
        check('provider import completes',api.wait_for(lambda:not api.owned_popup(process.pid,'QuietShareLocalPicker'),5));time.sleep(.2)
    def geometry(ids):
        visible=[n for n in ids if api.visible(ctl(n))]
        bounds=api.wintypes.RECT();U.GetClientRect(window,api.ctypes.byref(bounds));rects=[api.rect(ctl(n),window) for n in visible]
        return all(0<=a<c<=bounds.right and 0<=b<d<=bounds.bottom for a,b,c,d in rects) and all(
            min(a[2],b[2])<=max(a[0],b[0]) or min(a[3],b[3])<=max(a[1],b[1]) for i,a in enumerate(rects) for b in rects[i+1:])
    try:
        check('personal app session untouched',not api.native.app_windows());launch()
        cmd(139);check('preview starts enabled',query(70)==1);cmd(290)
        check('preview off hides canvas and releases surface',query(70)==0 and query(71)==0 and not api.visible(ctl(295)))
        close();launch();cmd(139);check('preview off persists across restart',query(70)==0 and not api.visible(ctl(295)))
        cmd(127);text(110,'# Opening\n'+'first section line\n'*150+'# Details\nSecond section notes')
        api.multi_drop(window,[api.ROOT/'artifacts/qa-viewers/many.pdf'])
        check('long PDF opens all pages',query(6)==120)
        cmd(301);check('thumbnail rail appears',query(73)==1 and api.visible(ctl(301)))
        text(302,'37');cmd(303);check('go to page uses one based number',query(5)==37)
        cmd(304);cmd(305);check('bookmark filter contains saved page',send(ctl(301),0x18B)==1)
        cmd(301);check('all pages return from bookmarks',send(ctl(301),0x18B)==120)
        cmd(301);check('hiding thumbnails frees all thumbnail memory',query(73)==0 and query(72)==0)
        cmd(306);check('page-linked reader appears',query(74)==1 and api.visible(ctl(311)) and send(ctl(312),0x18B)==2)
        cmd(310);text(302,'72');cmd(303)
        send(ctl(312),0x186,1);send(window,api.WM_COMMAND,(1<<16)|312,ctl(312));cmd(310);cmd(307)
        cmd(308);check('previous linked section changes PDF page',query(5)==37 and query(75)==0)
        cmd(309);check('next linked section changes PDF page',query(5)==72 and query(75)==1)
        for _ in range(3):cmd(315)
        check('cue size changes at top do not jump to bottom',send(ctl(311),0xCE)==0)
        # Import through the real protected native picker rather than seeding notebook data.
        cmd(103);pdf_zoom=query(4);cmd(313);check('Meetings is a dedicated workspace',query(79)==1 and api.visible(ctl(328)))
        cmd(330);check('setup exposes an immediate cancel control',api.visible(ctl(337)));cmd(337);time.sleep(.7)
        check('setup cancellation remains cancellation after timer update','QS-R006' in api.native.control_text(ctl(332)) and not api.visible(ctl(337)))
        import_provider();check('provider transcript is retained as separate segments',query(76)==2)
        check('provider speaker and timestamp preserved',api.native.control_text(ctl(322))=='Minh' and api.native.control_text(ctl(323))=='00:00:01.000')
        shortcut(ord('0'),ctl(321),True);check('meeting keyboard zoom does not change the background PDF',query(4)==pdf_zoom)
        cmd(127);check('provider import preserves the existing script',api.native.control_text(ctl(110)).startswith('# Opening'));cmd(313)
        for n in (321,322,323,324,338):send(ctl(n),0x7B,ctl(n),-1)
        check('private meeting inputs do not open unprotected stock context menus',not api.owned_popup(process.pid,'#32768'))
        cache=profile/'QuietShare/meeting-notes.dpapi';before=cache.read_bytes()
        text(321,'Confirm the reviewed delivery date.');shortcut(ord('S'),ctl(321))
        check('Ctrl S immediately persists the selected meeting segment',cache.read_bytes()!=before)
        cmd(326);cmd(325)
        first=resources(process);time.sleep(2);last=resources(process)
        metrics={'idleSeconds':2,'idleCpuSeconds':last['cpu']-first['cpu'],'idleGdiGrowth':last['gdi']-first['gdi'],'idleWorkingSetGrowth':last['workingSet']-first['workingSet']}
        check('idle notebook has no busy loop or GDI resource growth',metrics['idleCpuSeconds']<.4 and metrics['idleGdiGrowth']<=2 and metrics['idleWorkingSetGrowth']<=8*1024*1024)
        check('mark decision is visible on the native control','có' in api.native.control_text(ctl(326)) or 'on' in api.native.control_text(ctl(326)))
        send(ctl(320),0x186,1);send(window,api.WM_COMMAND,(1<<16)|320,ctl(320))
        text(322,'An reviewed');cmd(327);cmd(325)
        text(338,'An reviewed');check('notebook search filters segments',send(ctl(320),0x18B)==1);text(338,'')
        cmd(339);check('decisions filter returns marked outcome',send(ctl(320),0x18B)==1);cmd(339);check('actions filter returns marked task',send(ctl(320),0x18B)==1);cmd(339)
        text(324,'00:00:01.000');cmd(325);check('bad timing has a specific code','QS-N002' in api.native.control_text(ctl(332)));text(324,'00:00:06.000');cmd(325)
        menu(328,0);folder=profile/'QuietShare/Meetings'
        check('TXT export is visible on disk with reviewed content',any('An reviewed' in p.read_text(encoding='utf-8') for p in folder.glob('Meeting-notes-*.txt')))
        menu(328,1);check('VTT export preserves timing and speakers',any('<v An reviewed>' in p.read_text(encoding='utf-8') for p in folder.glob('*.vtt')))
        menu(328,4);check('last VTT export opens directly in the real viewer',api.native.control_text(ctl(110)).startswith('WEBVTT'));cmd(313)
        menu(328,2);check('minutes export contains reviewed actions and decisions',any('An reviewed' in p.read_text(encoding='utf-8') and '- [ ]' in p.read_text(encoding='utf-8') for p in folder.glob('*.md')))
        all_meeting=[214,328,332,320,321,322,323,324,325,326,327,333,334,338,339,340,341,342]
        for width,height in ((430,320),(840,560)):
            scale=U.GetDpiForWindow(window)/96;U.SetWindowPos(window,None,0,0,round(width*scale),round(height*scale),0x16)
            for view in (340,341,342):cmd(view);check(f'notebook controls fit without overlap {width} view {view}',geometry(all_meeting))
        cmd(335);check('manual segment can be added',query(76)==3);text(321,'Untimed manual note');cmd(325)
        menu(328,1);check('untimed VTT export reports a recovery code','QS-N003' in api.native.control_text(ctl(332)))
        cmd(336);check('manual segment can be removed',query(76)==2)
        cmd(105);check('F8 masks every private notebook surface',all(not api.visible(ctl(n)) for n in (320,321,322,323,324,338)) and all(query(n)==0 for n in range(70,80)))
        cmd(105);close();launch();cmd(313)
        check('reviewed notebook survives restart',query(76)==2 and 'reviewed delivery' in api.native.control_text(ctl(321)))
        raw=(profile/'QuietShare/meeting-notes.dpapi').read_bytes();check('notebook cache has no plaintext transcript',b'Confirm' not in raw and 'Confirm'.encode('utf-16-le') not in raw)
        close();bad=raw[:len(raw)//2];(profile/'QuietShare/meeting-notes.dpapi').write_bytes(bad)
        launch();cmd(313);check('unreadable notebook reports code and is preserved','QS-N001' in api.native.control_text(ctl(332)) and (profile/'QuietShare/meeting-notes.dpapi').read_bytes()==bad);close()
    except Exception as error:
        failure=repr(error);raise
    finally:
        if process and process.poll() is None:
            for h in api.native.app_windows(process.pid):U.PostMessageW(h,0x10,0,0)
            try:process.wait(timeout=8)
            except subprocess.TimeoutExpired:process.terminate();process.wait(timeout=5)
        report={'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'checks':checks,'failure':failure,'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest(),'metrics':metrics,'physicalReceiver':'not tested'}
        (api.ROOT/'artifacts/reading-meeting-integration.json').write_text(json.dumps(report,indent=2),encoding='utf-8')


if __name__=='__main__':main()
