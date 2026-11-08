"""Shipping preview discovery, direct settings navigation and protected picker UX."""
import ctypes,hashlib,json,os,subprocess,time,uuid
from pathlib import Path
import production_integration as a
U,send=a.USER32,a.send

def main():
    root=a.ROOT/'artifacts/qa-navigation-picker'/uuid.uuid4().hex;profile=root/'User';files=root/'Files';profile.mkdir(parents=True);files.mkdir()
    for name in ['folder2','folder10']:(files/name).mkdir()
    specs=[('file2.txt',3,20),('file10.txt',30,40),('alpha.py',200,10),('zeta.md',1,30)]
    for name,size,age in specs:
        p=files/name;p.write_text('x'*size,encoding='utf-8');os.utime(p,(1700000000+age,1700000000+age))
    many=root/'Many';many.mkdir()
    for i in range(240):(many/f'item{i}.txt').write_text('fixture',encoding='utf-8')
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile);app=None;h=None;checks=[];failure=None
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)});print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def cmd(n):U.PostMessageW(h,a.WM_COMMAND,n,0);time.sleep(.15)
    def ctl(n,parent=None):return U.GetDlgItem(parent or h,n)
    def q(n):return send(h,a.QUERY,n)
    def resize(w,ht):
        scale=U.GetDpiForWindow(h)/96;U.SetWindowPos(h,None,0,0,round(w*scale),round(ht*scale),0x16);time.sleep(.15)
    def names(listing):
        out=[]
        for i in range(send(listing,0x18B)):
            text=ctypes.create_unicode_buffer(send(listing,0x18A,i)+1);send(listing,0x189,i,ctypes.addressof(text));out.append(text.value)
        return out
    def selected(listing):
        count=send(listing,0x190);indices=(ctypes.c_int*max(1,count))();send(listing,0x191,count,ctypes.addressof(indices));rows=names(listing);return {rows[indices[i]] for i in range(count)}
    def menu(parent,id,index):
        U.PostMessageW(parent,a.WM_COMMAND,id,0);popup=a.wait_for(lambda:a.owned_popup(app.pid),5);check('protected picker menu opens',popup)
        affinity=a.wintypes.DWORD();check('picker menu excludes capture',U.GetWindowDisplayAffinity(popup,ctypes.byref(affinity)) and affinity.value==0x11)
        send(popup,0x200,0,(10<<16)|10)
        for _ in range(index):send(popup,a.WM_KEYDOWN,0x28)
        U.PostMessageW(popup,a.WM_KEYDOWN,0x0D,0);check('picker menu closes',a.wait_for(lambda:not U.IsWindow(popup),5));time.sleep(.1)
    try:
        check('personal application remains untouched',not a.native.app_windows());app=subprocess.Popen([str(a.EXE)],cwd=str(a.EXE.parent),env=env)
        h=a.wait_for(lambda:next(iter(a.native.app_windows(app.pid)),None),10);check('owned shipping window opens',h)
        check('real WGC protection passes',a.wait_for(lambda:send(h,a.GATE,timeout=6000),20))
        cmd(122);check('Home exposes a named preview action',a.visible(ctl(380)) and bool(a.native.control_text(ctl(380))))
        check('preview has a persistent sidebar entry',a.visible(ctl(139)))
        cmd(380);check('Home preview action opens the projection board',a.visible(ctl(295)) and a.visible(ctl(290)))
        cmd(290);check('disabled preview keeps a findable placeholder without a frame',q(70)==0 and q(71)==0 and a.visible(ctl(295)))
        cmd(123)
        categories=[200,201,203,240,345,202,204,346];indices=[0,1,3,5,6,2,4,7]
        for w,ht in [(430,320),(640,420),(840,560)]:
            resize(w,ht);check(f'all settings categories are directly visible {w}x{ht}',not a.visible(ctl(347)) and all(a.visible(ctl(n)) for n in categories))
            rects=[a.rect(ctl(n),h) for n in categories]
            check(f'category buttons do not overlap {w}x{ht}',all(min(x[2],y[2])<=max(x[0],y[0]) or min(x[3],y[3])<=max(x[1],y[1]) for i,x in enumerate(rects) for y in rects[i+1:]))
            for n,index in zip(categories,indices):cmd(n);check(f'category {index} opens directly {w}x{ht}',q(40)==index)
        cmd(124);picker=a.wait_for(lambda:a.owned_popup(app.pid,'QuietShareLocalPicker'),5);check('protected picker opens',picker)
        affinity=a.wintypes.DWORD();check('picker excludes capture',U.GetWindowDisplayAffinity(picker,ctypes.byref(affinity)) and affinity.value==0x11)
        listing=ctl(12,picker);address=ctl(13,picker)
        check('picker accepts owned folder',a.native.set_control_text(address,str(files)));send(picker,a.WM_COMMAND,17)
        check('natural name ordering and folder-first grouping',names(listing)==['folder2','folder10','alpha.py','file2.txt','file10.txt','zeta.md'])
        rows=names(listing)
        for name in ['file2.txt','zeta.md']:send(listing,0x185,1,rows.index(name))
        send(picker,a.WM_COMMAND,27)
        check('size sort defaults to largest first',names(listing)[2:]==['alpha.py','file10.txt','file2.txt','zeta.md'])
        check('sorting retains the actual selected files',selected(listing)=={'file2.txt','zeta.md'})
        send(picker,a.WM_COMMAND,27);check('repeated header click reverses size order',names(listing)[2:]==['zeta.md','file2.txt','file10.txt','alpha.py'])
        send(picker,a.WM_COMMAND,26);check('modified date defaults to newest first',names(listing)[2:]==['file10.txt','zeta.md','file2.txt','alpha.py'])
        send(picker,a.WM_COMMAND,31);check('file type sorting works',names(listing)[2:]==['zeta.md','alpha.py','file2.txt','file10.txt'])
        check('filename filter accepts mixed case',a.native.set_control_text(ctl(28,picker),'FILE2'));check('filename filter uses actual entries',names(listing)==['file2.txt'])
        a.native.set_control_text(ctl(28,picker),'no-match');check('empty search disables opening',send(listing,0x18B)==0 and not U.IsWindowEnabled(ctl(1,picker)))
        a.native.set_control_text(ctl(28,picker),'');menu(picker,30,1);check('compact list hides metadata and uses smaller rows',send(listing,0x1A1)==round(24*U.GetDpiForWindow(picker)/96) and not a.visible(ctl(26,picker)))
        menu(picker,30,0);check('detail view restores clickable columns',a.visible(ctl(26,picker)) and a.visible(ctl(31,picker)))
        scale=U.GetDpiForWindow(picker)/96;U.SetWindowPos(picker,None,0,0,round(480*scale),round(360*scale),0x16);time.sleep(.15)
        check('narrow picker adapts to a readable list',not a.visible(ctl(26,picker)) and a.rect(listing,picker)[2]>a.rect(listing,picker)[0]+200*scale)
        U.SetWindowPos(picker,None,0,0,round(900*scale),round(560*scale),0x16);time.sleep(.15)
        check('widening restores the requested detail view',a.visible(ctl(26,picker)))
        a.native.set_control_text(address,str(many));send(picker,a.WM_COMMAND,17);check('large owned folder is listed',send(listing,0x18B)==240)
        send(listing,0x197,0);send(address,0x7)
        send(picker,0x20A,(-60&0xffff)<<16);send(picker,0x20A,(-60&0xffff)<<16)
        check('partial wheel deltas scroll without list focus',send(listing,0x18E)>0)
        top=send(listing,0x18E);send(ctl(28,picker),0x20A,(-120&0xffff)<<16)
        check('wheel over an input routes to the file list',a.wait_for(lambda:send(listing,0x18E)>top,3))
        send(picker,a.WM_COMMAND,2);check('picker closes safely',a.wait_for(lambda:not U.IsWindow(picker),5))
        cmd(124);picker=a.wait_for(lambda:a.owned_popup(app.pid,'QuietShareLocalPicker'),5)
        a.native.set_control_text(ctl(13,picker),str(files));send(picker,a.WM_COMMAND,17)
        check('sort choice survives reopening in the same session',names(ctl(12,picker))[2:]==['zeta.md','alpha.py','file2.txt','file10.txt'])
        U.PostMessageW(picker,a.WM_COMMAND,29,0);sort_popup=a.wait_for(lambda:a.owned_popup(app.pid),5);check('sort menu opens before privacy interruption',sort_popup);cmd(105)
        check('privacy interruption also closes the nested sort menu',a.wait_for(lambda:not U.IsWindow(sort_popup),5))
        check('privacy mask closes the picker',a.wait_for(lambda:not U.IsWindow(picker),5))
        U.PostMessageW(h,0x10,0,0);app.wait(timeout=10);check('owned app exits cleanly',app.returncode==0);app=None
    except Exception as error:failure=repr(error);raise
    finally:
        if app and app.poll() is None:
            for w in a.native.app_windows(app.pid):U.PostMessageW(w,0x10,0,0)
            try:app.wait(timeout=10)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
        report={'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'failure':failure,'exeSha256':hashlib.sha256(a.EXE.read_bytes()).hexdigest(),'physicalReceiver':'not tested'}
        (a.ROOT/'artifacts/navigation-picker-integration.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
if __name__=='__main__':main()
