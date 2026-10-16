// Include the production renderer in this focused native test translation unit
// so no test API or projection bypass is shipped in the application.
#define QUIETSHARE_MIRROR_REGRESSION
#include "../src/Mirror.cpp"
#include <fstream>
#include <iostream>

using namespace quietshare;
namespace
{
int passed=0,failed=0;
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
}
namespace quietshare
{
// Compile-only friendship exposes state to this harness, never to a shipped EXE.
struct MirrorRegressionAccess
{
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
    CheckFrameTiming();
    MirrorRegressionAccess::CheckProbePreparation();
    if(argc>1&&std::wcscmp(argv[1],L"--timing-only")==0)return failed?1:0;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSW wc{};wc.lpfnWndProc=TestProc;wc.hInstance=GetModuleHandleW(nullptr);
    wc.lpszClassName=L"QuietShareSyntheticRendererTest";RegisterClassW(&wc);
    HWND window=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Synthetic D3D test",WS_POPUP,
        16,16,640,480,nullptr,nullptr,wc.hInstance,nullptr);
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);
    Capture capture;D3D_FEATURE_LEVEL level{};
    Check(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,0,D3D11_SDK_VERSION,capture.device.put(),&level,capture.context.put())),"software D3D11 device");
    if(!capture.device){DestroyWindow(window);return 1;}
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
    DestroyWindow(window);
    if(argc>1)
    {
        std::ofstream report(argv[1],std::ios::binary);
        report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed
              <<",\"adapter\":\"WARP software\",\"physicalDestination\":\"NOT TESTED\"}\n";
    }
    return failed?1:0;
}
