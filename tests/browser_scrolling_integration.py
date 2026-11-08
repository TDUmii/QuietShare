"""Owned scrolling and renderer-unresponsive recovery; never personal web tabs."""
import hashlib,http.server,json,os,subprocess,threading,time,uuid
from pathlib import Path
from urllib.parse import urlsplit,parse_qs
import production_integration as api

class Fixture(http.server.BaseHTTPRequestHandler):
    frames=[];hang_requests=0
    def do_GET(self):
        url=urlsplit(self.path)
        if url.path=='/tick':
            values=parse_qs(url.query)
            try:
                value=(time.monotonic(),int(values.get('frame',['0'])[0]),int(values.get('y',['0'])[0]))
                if len(type(self).frames)<600:type(self).frames.append(value)
            except ValueError:pass
            body=b'OK'
        elif url.path=='/hang':
            type(self).hang_requests+=1
            if type(self).hang_requests==1:
                body=b'''<!doctype html><title>Bounded busy fixture</title><p>Synthetic busy script, maximum 45 seconds.</p><script>let stop=performance.now()+45000;while(performance.now()<stop){Math.sqrt(12345)}</script>'''
            else:body=b'<!doctype html><title>Recovered fixture</title><p>Reload recovered the owned renderer.</p>'
        else:
            body=b'''<!doctype html><meta charset=utf-8><title>Scrolling fixture</title><style>body{margin:0;background:#eee}header{position:sticky;top:0;background:#fff;padding:18px;z-index:1}article{margin:16px auto;padding:16px;width:640px;background:white;border-radius:12px;box-shadow:0 2px 6px #777}canvas{width:100%;height:120px}</style><header>Owned scrolling workload</header><main></main><script>
            let main=document.querySelector('main');for(let i=0;i<240;i++){let a=document.createElement('article');a.innerHTML='<h2>Post '+i+'</h2><p>Synthetic feed content for scroll testing.</p>';if(i%8===0){let c=document.createElement('canvas');c.width=640;c.height=120;let x=c.getContext('2d');let g=x.createLinearGradient(0,0,640,120);g.addColorStop(0,'#cde');g.addColorStop(1,'#597');x.fillStyle=g;x.fillRect(0,0,640,120);a.append(c)}main.append(a)}
            let frame=0,last=0;function tick(t){frame++;scrollBy(0,12);if(t-last>250){last=t;fetch('/tick?frame='+frame+'&y='+Math.round(scrollY)).catch(()=>{})}requestAnimationFrame(tick)}requestAnimationFrame(tick);
            </script>'''
        self.send_response(200);self.send_header('Content-Type','text/html');self.send_header('Content-Length',str(len(body)));self.end_headers()
        try:self.wfile.write(body)
        except (BrokenPipeError,ConnectionResetError):pass
    def log_message(self,*args):pass

def main():
    root=api.ROOT/'artifacts/qa-browser-scrolling'/uuid.uuid4().hex;profile=root/'User';profile.mkdir(parents=True)
    env=os.environ.copy();env['LOCALAPPDATA']=str(profile);server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Fixture);threading.Thread(target=server.serve_forever,daemon=True).start()
    app=None;window=None;checks=[];metrics={};failure=None
    def check(name,ok):
        checks.append({'name':name,'passed':bool(ok)});print(('PASS ' if ok else 'FAIL ')+name,flush=True)
        if not ok:raise AssertionError(name)
    def q(n):return api.send(window,api.QUERY,n)
    def cmd(n):api.USER32.PostMessageW(window,api.WM_COMMAND,n,0);time.sleep(.15)
    try:
        check('personal session remains untouched',not api.native.app_windows())
        app=subprocess.Popen([str(api.EXE)],cwd=str(api.EXE.parent),env=env)
        window=api.wait_for(lambda:next(iter(api.native.app_windows(app.pid)),None),10)
        check('owned shipping window starts',window)
        check('real WGC protection passes',api.wait_for(lambda:api.send(window,api.GATE,timeout=6000),20))
        cmd(154);api.native.set_control_text(api.USER32.GetDlgItem(window,164),f'http://127.0.0.1:{server.server_port}/feed');cmd(155)
        check('owned scrolling page is verified and loaded',api.wait_for(lambda:q(10)==2 and q(12)==1 and q(13)==1 and len(Fixture.frames)>2,25))
        initial=len(Fixture.frames);layouts=q(85);latencies=[];start=time.monotonic()
        while time.monotonic()-start<10:
            before=time.monotonic();q(10);latencies.append((time.monotonic()-before)*1000);time.sleep(.25)
        samples=Fixture.frames[initial:];metrics={'scrollSamples':len(samples),'maximumHostQueryMs':max(latencies),'nativeLayoutsDuringScroll':q(85)-layouts,'observedFrames':samples[-1][1]-samples[0][1] if len(samples)>1 else 0,'scrollDistance':samples[-1][2]-samples[0][2] if len(samples)>1 else 0}
        check('feed continues producing scroll frames',len(samples)>=10 and metrics['observedFrames']>=30 and metrics['scrollDistance']>300)
        check('scrolling does not relayout the native shell every frame',metrics['nativeLayoutsDuringScroll']<=2)
        check('native controls remain responsive while scrolling',metrics['maximumHostQueryMs']<1500)
        check('scrolling preserves browser and WGC verification',q(10)==2 and q(12)==1 and api.send(window,api.GATE,timeout=6000)==1)
        api.native.set_control_text(api.USER32.GetDlgItem(window,164),f'http://127.0.0.1:{server.server_port}/hang');cmd(165)
        check('bounded busy renderer fixture starts',api.wait_for(lambda:Fixture.hang_requests==1,10))
        check('native controls and capture remain available during a busy renderer',q(10)==2 and q(12)==1 and api.send(window,api.GATE,timeout=6000)==1)
        # The runtime need not emit ProcessFailed without genuine platform
        # input. Exercise the always-available native recovery menu instead.
        cmd(212);popup=api.wait_for(lambda:api.owned_popup(app.pid),5)
        check('native recovery menu remains usable while web script is busy',popup)
        api.send(popup,0x200,0,(10<<16)|10);api.send(popup,api.WM_KEYDOWN,0x28)
        api.USER32.PostMessageW(popup,api.WM_KEYDOWN,0x0D,0)
        check('explicit tab recreation recovers a stalled renderer',api.wait_for(lambda:Fixture.hang_requests>=2 and q(10)==2 and q(12)==1 and q(13)==1 and not api.visible(api.USER32.GetDlgItem(window,375)),25))
        check('recovery preserves capture verification',q(12)==1 and q(13)==1 and api.send(window,api.GATE,timeout=6000)==1)
        cmd(374);text=api.native.control_text(api.USER32.GetDlgItem(window,373))
        check('support report includes current committed and physical memory',('Available committed memory:' in text or 'Bộ nhớ cam kết còn:' in text) and ('Available physical RAM:' in text or 'RAM vật lý còn:' in text))
        api.USER32.PostMessageW(window,0x10,0,0);app.wait(timeout=10);check('owned app exits cleanly',app.returncode==0);app=None
    except Exception as error:failure=repr(error);raise
    finally:
        server.shutdown();server.server_close()
        if app and app.poll() is None:
            for h in api.native.app_windows(app.pid):api.USER32.PostMessageW(h,0x10,0,0)
            try:app.wait(timeout=10)
            except subprocess.TimeoutExpired:app.terminate();app.wait(timeout=5)
        report={'checks':checks,'passed':sum(c['passed'] for c in checks),'failed':sum(not c['passed'] for c in checks),'failure':failure,'metrics':metrics,'exeSha256':hashlib.sha256(api.EXE.read_bytes()).hexdigest(),'authenticatedFacebook':'not tested','physicalReceiver':'not tested'}
        (api.ROOT/'artifacts/browser-scrolling-integration.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
if __name__=='__main__':main()
