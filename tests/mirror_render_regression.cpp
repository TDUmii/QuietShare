// Include the production renderer in this focused native test translation unit
// so no test API or projection bypass is shipped in the application.
#define QUIETSHARE_MIRROR_REGRESSION
#include "../src/Mirror.cpp"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <psapi.h>

using namespace quietshare;
namespace
{
int passed=0,failed=0;
struct SoakMetrics
{
    bool actualWgcCanary=false;
    size_t monitorCount=0;
    ULONGLONG elapsedMs=0,workingSetStart=0,workingSetEnd=0,workingSetMin=0,workingSetMax=0;
    unsigned framesPresented=0,samples=0;
    DWORD gdiStart=0,gdiEnd=0;
    bool minimized=false,restored=false;
} soak;
ULONGLONG WorkingSetBytes()
{
    using Query=BOOL(WINAPI *)(HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD);
    const auto query=Procedure<Query>(GetModuleHandleW(L"kernel32.dll"),"K32GetProcessMemoryInfo");
    PROCESS_MEMORY_COUNTERS info{};info.cb=sizeof(info);
    return query&&query(GetCurrentProcess(),&info,sizeof(info))?info.WorkingSetSize:0;
}
void Check(bool result,const char *name)
{
    std::cout<<(result?"PASS ":"FAIL ")<<name<<'\n';if(result)++passed;else++failed;
}
struct TimestampFrame final : CaptureFrame
{
    INT64 timestamp=0;
    HRESULT result=S_OK;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID,void **value) override {if(value)*value=nullptr;return E_NOINTERFACE;}
    ULONG STDMETHODCALLTYPE AddRef() override {return 1;}
    ULONG STDMETHODCALLTYPE Release() override {return 1;}
    HRESULT STDMETHODCALLTYPE GetIids(ULONG *,IID **) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE GetRuntimeClassName(HSTRING *) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE GetTrustLevel(TrustLevel *) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE Surface(IInspectable **) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE SystemRelativeTime(INT64 *time) override {*time=timestamp;return result;}
    HRESULT STDMETHODCALLTYPE ContentSize(Size32 *) override {return E_NOTIMPL;}
};
struct TestSession final : CaptureSession
{
    ULONG releases=0;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID,void **value) override {if(value)*value=nullptr;return E_NOINTERFACE;}
    ULONG STDMETHODCALLTYPE AddRef() override {return 1;}
    ULONG STDMETHODCALLTYPE Release() override {return ++releases;}
    HRESULT STDMETHODCALLTYPE GetIids(ULONG *,IID **) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE GetRuntimeClassName(HSTRING *) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE GetTrustLevel(TrustLevel *) override {return E_NOTIMPL;}
    HRESULT STDMETHODCALLTYPE StartCapture() override {return E_NOTIMPL;}
};
void CheckFrameTiming()
{
    Check(CounterTime100ns(123456789,10000000)==123456789,"QPC native 10 MHz maps to capture TimeSpan ticks");
    Check(CounterTime100ns(30000000,3000000)==100000000,"QPC frequency conversion preserves seconds");
    Check(!CounterTime100ns(0,10000000)&&!CounterTime100ns(1000,0),"missing QPC time fails closed");
    for(unsigned fps:{15u,30u,60u})
    {
        INT64 next=0;unsigned frames=0;
        for(INT64 now=1;now<100000000;now+=80000)if(FrameDue(now,fps,next))++frames;
        Check(frames>=fps*10-1&&frames<=fps*10+1,"8 ms timer honors each average frame budget without integer-ms loss");
        INT64 late=0;FrameDue(1,fps,late);
        Check(FrameDue(100000000,fps,late)&&!FrameDue(100000000,fps,late),"paused UI resumes without a same-tick catch-up burst");
        Check(!FrameDue(0,fps,late),"missing frame clock cannot release a frame");
    }
    TimestampFrame frame;
    frame.timestamp=99;Check(FrameAfter(&frame,100)==S_FALSE,"queued positive canary frame before teardown never presents");
    frame.timestamp=100;Check(FrameAfter(&frame,100)==S_FALSE,"frame at teardown barrier stays excluded");
    frame.timestamp=101;Check(FrameAfter(&frame,100)==S_OK,"only compositor frames strictly after teardown can present");
    frame.result=E_FAIL;Check(FAILED(FrameAfter(&frame,100)),"unreadable compositor timestamp fails closed");
    Check(FAILED(FrameAfter(nullptr,100)),"missing capture frame fails closed");
}
EXECUTION_STATE ExecutionState()
{
    const auto previous=SetThreadExecutionState(ES_CONTINUOUS);
    if(previous)SetThreadExecutionState(previous|ES_CONTINUOUS);
    return previous;
}
void CheckExecutionRequest()
{
    const auto baseline=ExecutionState();
    Check(baseline!=0,"thread execution state can be inspected and restored");
    ProjectionExecutionRequest request;
    Check(request.Acquire(),"projection acquires its own system/display execution request");
    constexpr EXECUTION_STATE required=ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED;
    Check((ExecutionState()&required)==required,"active projection requests both system and display awake");
    Check(request.Acquire(),"repeated projection acquisition keeps the original restoration state");
    request.Release();
    Check(ExecutionState()==baseline,"projection release restores the prior UI thread execution state");
    request.Release();
    Check(ExecutionState()==baseline,"repeated release does not alter another execution request");
}
bool ReadBack(Capture &capture,Renderer &renderer,std::vector<unsigned char>&out)
{
    Com<ID3D11Texture2D> buffer,stage;
    if(FAILED(renderer.swap->GetBuffer(0,IID_ID3D11Texture2D,reinterpret_cast<void **>(buffer.put()))))return false;
    D3D11_TEXTURE2D_DESC desc{};buffer->GetDesc(&desc);desc.Usage=D3D11_USAGE_STAGING;
    desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;desc.MiscFlags=0;
    if(FAILED(capture.device->CreateTexture2D(&desc,nullptr,stage.put())))return false;
    capture.context->CopyResource(stage.get(),buffer.get());D3D11_MAPPED_SUBRESOURCE map{};
    if(FAILED(capture.context->Map(stage.get(),0,D3D11_MAP_READ,0,&map)))return false;
    out.resize(desc.Width*desc.Height*4);
    for(UINT y=0;y<desc.Height;++y)
        std::memcpy(out.data()+y*desc.Width*4,static_cast<unsigned char *>(map.pData)+y*map.RowPitch,desc.Width*4);
    capture.context->Unmap(stage.get(),0);return true;
}
bool ColorAt(const std::vector<unsigned char> &pixels,int width,int x,int y,COLORREF color)
{
    const size_t index=(static_cast<size_t>(y)*width+x)*4;
    return index+4<=pixels.size()&&Near(pixels.data()+index,color);
}
LRESULT CALLBACK TestProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_PAINT){PAINTSTRUCT ps;BeginPaint(hwnd,&ps);EndPaint(hwnd,&ps);return 0;}
    return DefWindowProcW(hwnd,msg,wp,lp);
}
void CheckPrivateWindowLocation(HWND window)
{
    MonitorInfo source{};GetWindowRect(window,&source.bounds);
    Check(WithinMonitor(window,source)&&PrivateWindowLocationSafe(window,source),
          "visible private window must remain wholly inside its source bounds");
    Check(!PrivateWindowLocationSafe(nullptr,source),"invalid private window never passes location policy");
    ShowWindow(window,SW_SHOWMINNOACTIVE);
    Check(IsIconic(window)&&!WithinMonitor(window,source),
          "native minimize parks the private HWND outside its source bounds");
    Check(PrivateWindowLocationSafe(window,source),
          "minimized private window does not revoke an existing projection location");
    ShowWindow(window,SW_SHOWNOACTIVATE);
    Check(!IsIconic(window)&&PrivateWindowLocationSafe(window,source),
          "restored private window is checked against its source bounds again");
    SetWindowPos(window,nullptr,source.bounds.left+1,source.bounds.top,0,0,
                 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    Check(!PrivateWindowLocationSafe(window,source),
          "visible private window partly beyond the source still fails closed");
    SetWindowPos(window,nullptr,source.bounds.left,source.bounds.top,0,0,
                 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}
}
namespace quietshare
{
// Compile-only friendship exposes state to this harness, never to a shipped EXE.
struct MirrorRegressionAccess
{
    static void CheckMinimizedTick(HWND window,ID3D11Device *device)
    {
        MirrorController controller;
        auto &s=*controller.impl;
        MonitorInfo source{};GetWindowRect(window,&source.bounds);
        Check(ApplyPrivacy(window).applied,"native minimize fixture keeps capture exclusion applied");
        auto prepare=[&]() {
            s.CancelStoppedNotice();s.testElapsed=0;
            s.privateWindow=window;s.source=source;s.verified=true;s.running=true;s.state=State::Running;
            s.checkTick=s.Now();s.framesTick=s.checkTick;s.topology=EnumerateMonitors();
            // Exercise the production safety branch without fabricating a WGC
            // frame or needing a second physical destination.
            s.nextFrameAt=CaptureClock()+100000000;
            device->AddRef();*s.capture.device.put()=device;
            Check(s.execution.Acquire(),"running fixture acquires a scoped execution request");
        };
        prepare();const unsigned epoch=s.epoch;
        ShowWindow(window,SW_SHOWMINNOACTIVE);controller.Tick();
        Check(IsIconic(window)&&s.running&&controller.IsVerified()&&s.epoch==epoch,
              "production Tick preserves a minimized projection after every other safety guard");
        ShowWindow(window,SW_SHOWNOACTIVATE);
        SetWindowPos(window,nullptr,source.bounds.left+1,source.bounds.top,0,0,
                     SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        Check(s.running&&!controller.IsVerified(),
              "restored wrong-source window cannot reveal private content before the next Tick");
        controller.Tick();
        Check(!s.running&&!s.verified&&s.state==State::WrongLocation,
              "production Tick revokes projection restored partly outside its source");
        Check(!s.execution.previous,"unsafe location releases the projection execution request");
        const unsigned wrongLocationSerial=s.stoppedSerial;
        Check(controller.ConsumeStoppedNotice(wrongLocationSerial)&&!controller.ConsumeStoppedNotice(wrongLocationSerial),
              "unexpected failure notice is serial guarded and consumed only once");
        SetWindowPos(window,nullptr,source.bounds.left,source.bounds.top,0,0,
                     SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        for(bool minimized:{false,true})
        {
            prepare();ShowWindow(window,minimized?SW_SHOWMINNOACTIVE:SW_SHOWNOACTIVATE);
            const unsigned stableEpoch=s.epoch;
            for(ULONGLONG elapsed:{299999ull,300000ull,600000ull,3600000ull,86400000ull})
            {
                s.testElapsed=elapsed;s.nextFrameAt=CaptureClock()+100000000;controller.Tick();
                Check(s.running&&controller.IsVerified()&&s.epoch==stableEpoch&&s.execution.previous,
                      minimized?"healthy minimized capture survives former five/ten-minute and long-session boundaries":
                                "healthy visible capture survives former five/ten-minute and long-session boundaries");
            }
            ShowWindow(window,SW_SHOWNOACTIVATE);controller.Tick();
            Check(s.running&&controller.IsVerified(),"restoring a healthy long-running session does not restart or revoke it");
        }
        prepare();ShowWindow(window,SW_SHOWMINNOACTIVE);controller.Invalidate();
        Check(!s.running&&!s.verified,"F8/topology invalidation still revokes while minimized");
        Check(!s.execution.previous,"invalidation releases the execution request immediately");
        const unsigned staleSerial=s.stoppedSerial;controller.Stop();
        Check(!controller.ConsumeStoppedNotice(staleSerial),"explicit Stop cancels an older queued failure notice");
        prepare();controller.Stop();
        Check(!s.running&&!s.verified&&!s.execution.previous,"explicit Stop still revokes while minimized and releases awake state");
        prepare();unsigned deferredSerial=0;
        {
            MirrorController::Impl::Operation operation(s);
            controller.Invalidate();deferredSerial=s.stoppedSerial;
            Check(s.clearPending&&!s.execution.previous,"deferred native teardown still releases the execution request at revocation");
        }
        Check(controller.ConsumeStoppedNotice(deferredSerial),"deferred COM teardown retains the matching unexpected-stop notice");
        prepare();controller.Invalidate();const unsigned oldSerial=s.stoppedSerial;
        s.BeginVerification(window,L"synthetic source",false);s.busy=false;
        Check(!controller.ConsumeStoppedNotice(oldSerial),"fresh verification cancels an older queued failure notice");
        ShowWindow(window,SW_SHOWNOACTIVATE);
    }
    static void RunSoak(HWND window,unsigned seconds)
    {
        const auto monitors=EnumerateMonitors();
        soak.monitorCount=monitors.size();
        Check(!monitors.empty(),"soak has a real WGC source display");
        if(monitors.empty())return;
        const auto source=monitors.front();
        SetWindowPos(window,nullptr,source.bounds.left+16,source.bounds.top+16,350,260,
                     SWP_NOZORDER|SWP_NOACTIVATE);
        MirrorController controller;
        soak.actualWgcCanary=controller.Verify(window,source.stableId);
        Check(soak.actualWgcCanary,"soak performs a fresh real WGC positive/exclusion proof");
        if(!controller.IsVerified())return;
        auto &s=*controller.impl;
        // This harness alone mounts a small protected synthetic destination on
        // the available source monitor. Production Start still requires two
        // distinct same-GPU Extend displays and exposes no bypass.
        s.destination=source;s.destinationId=source.stableId;
        s.destination.bounds={source.bounds.right-336,source.bounds.top+16,source.bounds.right-16,source.bounds.top+196};
        const bool created=s.CreateProjector();
        Check(created,"soak creates the production renderer on an explicitly synthetic destination");
        if(!created)return;
        Check(s.execution.Acquire(),"soak acquires the real scoped awake request");
        if(!s.execution.previous)return;
        s.running=true;s.state=State::Running;s.framesTick=s.Now();
        CanaryWindows scene;
        const bool sceneReady=scene.Create(source)&&ApplyPrivacy(scene.overlay).applied;
        Check(sceneReady,"synthetic public animation keeps the real source capture active");
        if(!sceneReady)return;
        const auto started=GetTickCount64();
        auto lastReport=started,lastPaint=started;
        soak.workingSetStart=WorkingSetBytes();soak.workingSetMin=soak.workingSetMax=soak.workingSetStart;
        bool minimized=false,restored=false,healthy=true;
        soak.gdiStart=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        while(GetTickCount64()-started<ULONGLONG(seconds)*1000)
        {
            const auto now=GetTickCount64(),elapsed=now-started;
            MSG message{};
            while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE))
            {
                if(message.message==WM_QUIT){controller.Stop();healthy=false;break;}
                TranslateMessage(&message);DispatchMessageW(&message);
            }
            if(!healthy)break;
            if(!minimized&&elapsed>=ULONGLONG(seconds)*250)
            {ShowWindow(window,SW_SHOWMINNOACTIVE);minimized=true;}
            if(!restored&&elapsed>=ULONGLONG(seconds)*750)
            {ShowWindow(window,SW_SHOWNOACTIVATE);restored=true;}
            if(now-lastPaint>=100)
            {
                lastPaint=now;std::swap(scene.publicPaint.a,scene.publicPaint.b);
                InvalidateRect(scene.underlay,nullptr,FALSE);UpdateWindow(scene.underlay);
            }
            controller.Tick();
            if(!controller.Running()||!controller.IsVerified()||!s.projector)
            {healthy=false;break;}
            if(now-lastReport>=30000)
            {
                lastReport=now;++soak.samples;
                const auto memory=WorkingSetBytes();
                soak.workingSetMin=std::min(soak.workingSetMin,memory);soak.workingSetMax=std::max(soak.workingSetMax,memory);
                std::cout<<"SOAK elapsedSeconds="<<elapsed/1000<<" frames="<<s.testPresentedFrames
                         <<" minimized="<<(IsIconic(window)?1:0)<<'\n';
            }
            Sleep(8);
        }
        soak.elapsedMs=GetTickCount64()-started;soak.framesPresented=s.testPresentedFrames;
        soak.minimized=minimized;soak.restored=restored;
        soak.gdiEnd=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        soak.workingSetEnd=WorkingSetBytes();
        soak.workingSetMin=std::min(soak.workingSetMin,soak.workingSetEnd);
        soak.workingSetMax=std::max(soak.workingSetMax,soak.workingSetEnd);
        Check(healthy&&soak.elapsedMs>=ULONGLONG(seconds)*1000,
              "real WGC and production Tick remain running for the complete requested soak duration");
        Check(minimized&&restored,"real soak includes minimize and restore without restarting capture");
        Check(soak.framesPresented>=seconds,"real source continues delivering rendered frames throughout the soak");
        Check(soak.gdiEnd<=soak.gdiStart+2,
              "long-running production capture does not accumulate GDI objects");
        std::cout<<"SOAK durationSeconds="<<seconds<<" frames="<<soak.framesPresented<<" samples="<<soak.samples
                 <<" syntheticDestination=1 physicalExtend=NOT_TESTED\n";
        // A failure still revokes promptly after a long healthy session. No
        // automatic retry or projection resume is involved.
        SetWindowDisplayAffinity(window,0);controller.Tick();
        Check(!controller.Running()&&!controller.IsVerified()&&!controller.Projector()&&!s.execution.previous,
              "post-soak affinity loss immediately removes output and releases the awake request");
        const unsigned serial=s.stoppedSerial;
        Check(controller.ConsumeStoppedNotice(serial),"post-soak unexpected stop emits a consumable notice");
        controller.Stop();
        Check(!controller.ConsumeStoppedNotice(serial),"intentional Stop cannot replay the soak failure notice");
    }
    static void CheckProbePreparation()
    {
        TestSession session;
        MirrorController controller;
        auto &s=*controller.impl;
        const HWND synthetic=reinterpret_cast<HWND>(static_cast<INT_PTR>(-1));
        s.privateWindow=synthetic;s.projector=synthetic;s.sourceId=L"synthetic source";
        s.destinationId=L"synthetic destination";s.verified=true;s.running=true;
        *s.capture.session.put()=&session;
        const unsigned epoch=s.epoch;
        Check(s.BeginVerification(synthetic,s.sourceId,true),"running same-source mounted probe chooses preservation");
        Check(s.running&&s.busy&&s.projector==synthetic&&s.destinationId==L"synthetic destination"&&
              s.capture.session.get()==&session&&session.releases==0&&s.epoch==epoch,
              "browser probe entry preserves projector, destination and exact capture without Clear");
        {
            MirrorController::Impl::Operation operation(s,true);
            controller.Invalidate();
            Check(!s.running&&!s.verified&&s.epoch!=epoch&&s.clearPending&&session.releases==0,
                  "F8/invalidation revokes during a probe and defers active capture release");
        }
        Check(!s.busy&&!s.projector&&!s.capture.session&&session.releases==1,
              "probe cancellation releases deferred output and never auto-resumes");
        s.privateWindow=synthetic;s.projector=synthetic;s.sourceId=L"synthetic source";
        s.destinationId=L"synthetic destination";s.verified=true;s.running=true;
        Check(!s.BeginVerification(synthetic,s.sourceId,false)&&!s.running&&!s.projector&&s.destinationId.empty(),
              "native verification and Start still clear previous projection");
        s.busy=false;
        Check(!s.BeginVerification(synthetic,L"different source",true)&&!s.running,
              "a source change never preserves or auto-resumes output");
        s.busy=false;
    }
};
}
int wmain(int argc,wchar_t**argv)
{
    std::cout.setf(std::ios::unitbuf);
    unsigned soakSeconds=0;
    const wchar_t *reportPath=argc>1?argv[1]:nullptr;
    if(argc>1&&std::wcscmp(argv[1],L"--soak-seconds")==0)
    {
        wchar_t *end=nullptr;
        const unsigned long requested=argc>2?std::wcstoul(argv[2],&end,10):0;
        Check(requested>0&&requested<=3600&&end&&!*end,"soak duration is explicitly bounded to 1-3600 seconds");
        if(failed)return 1;
        soakSeconds=static_cast<unsigned>(requested);reportPath=argc>3?argv[3]:nullptr;
    }
    CheckFrameTiming();
    CheckExecutionRequest();
    MirrorRegressionAccess::CheckProbePreparation();
    if(argc>1&&std::wcscmp(argv[1],L"--timing-only")==0)return failed?1:0;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSW wc{};wc.lpfnWndProc=TestProc;wc.hInstance=GetModuleHandleW(nullptr);
    wc.lpszClassName=L"QuietShareSyntheticRendererTest";RegisterClassW(&wc);
    HWND window=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Synthetic D3D test",WS_POPUP,
        16,16,640,480,nullptr,nullptr,wc.hInstance,nullptr);
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);
    CheckPrivateWindowLocation(window);
    Capture capture;D3D_FEATURE_LEVEL level{};
    Check(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,0,D3D11_SDK_VERSION,capture.device.put(),&level,capture.context.put())),"software D3D11 device");
    if(!capture.device){DestroyWindow(window);return 1;}
    MirrorRegressionAccess::CheckMinimizedTick(window,capture.device.get());
    capture.size={400,200};
    std::vector<unsigned char> source(400*200*4);
    for(size_t i=0;i<source.size();i+=4){source[i]=54;source[i+1]=178;source[i+2]=31;source[i+3]=255;}
    D3D11_TEXTURE2D_DESC desc{};desc.Width=400;desc.Height=200;desc.MipLevels=1;desc.ArraySize=1;
    desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_IMMUTABLE;
    desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem=source.data();data.SysMemPitch=400*4;Com<ID3D11Texture2D> texture;
    Check(SUCCEEDED(capture.device->CreateTexture2D(&desc,&data,texture.put())),"public synthetic frame texture");
    Renderer renderer;
    Check(renderer.Create(capture,window,640,480),"production native shader and swapchain creation");
    if(!renderer.swap||!texture){DestroyWindow(window);return 1;}
    Check(renderer.Present(capture,texture.get()),"production D3D11 frame present");
    std::vector<unsigned char> pixels;
    Check(ReadBack(capture,renderer,pixels),"native swapchain readback");
    const COLORREF publicColor=RGB(31,178,54);
    Check(ColorAt(pixels,640,320,240,publicColor),"public frame appears in destination center");
    Check(ColorAt(pixels,640,320,20,0),"letterbox top is black");
    Check(ColorAt(pixels,640,320,460,0),"letterbox bottom is black");
    Check(ColorAt(pixels,640,20,240,publicColor),"aspect-preserving frame fills fitted width");
    renderer.Blank(capture);Check(ReadBack(capture,renderer,pixels)&&ColorAt(pixels,640,320,240,0),
        "stop/lock blanks old public output");
    capture.size={401,200};
    Check(!renderer.Present(capture,texture.get()),"unexpected frame size cannot project");
    capture.size={400,200};capture.context->ClearState();renderer.Clear();
    SetWindowPos(window,nullptr,0,0,800,200,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    Check(renderer.Create(capture,window,800,200),"wide target shader and swapchain recreation");
    Check(renderer.Present(capture,texture.get())&&ReadBack(capture,renderer,pixels),"wide target frame rendered");
    Check(ColorAt(pixels,800,400,100,publicColor),"wide target preserves public center");
    Check(ColorAt(pixels,800,50,100,0)&&ColorAt(pixels,800,750,100,0),"pillarbox left and right are black");
    capture.context->ClearState();renderer.Clear();
    Check(!renderer.swap&&!renderer.target&&!renderer.copy&&!renderer.resource,"renderer releases all owned GPU resources");
    if(soakSeconds)MirrorRegressionAccess::RunSoak(window,soakSeconds);
    DestroyWindow(window);
    if(reportPath)
    {
        std::ofstream report(reportPath,std::ios::binary);
        report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed
              <<",\"adapter\":\"WARP software renderer checks; real source GPU for optional soak\",\"soakSeconds\":"<<soakSeconds
              <<",\"elapsedSeconds\":"<<soak.elapsedMs/1000.0<<",\"actualWgcCanary\":"<<(soak.actualWgcCanary?"true":"false")
              <<",\"framesPresented\":"<<soak.framesPresented<<",\"monitorCount\":"<<soak.monitorCount
              <<",\"workingSetStartBytes\":"<<soak.workingSetStart<<",\"workingSetEndBytes\":"<<soak.workingSetEnd
              <<",\"workingSetMinBytes\":"<<soak.workingSetMin<<",\"workingSetMaxBytes\":"<<soak.workingSetMax
              <<",\"samples\":"<<soak.samples<<",\"minimized\":"<<(soak.minimized?"true":"false")
              <<",\"restored\":"<<(soak.restored?"true":"false")<<",\"gdiStart\":"<<soak.gdiStart<<",\"gdiEnd\":"<<soak.gdiEnd
              <<",\"physicalDestination\":\"NOT TESTED\",\"fixtureSingleMonitor\":true}\n";
    }
    return failed?1:0;
}
