// Include the production renderer in this focused native test translation unit
// so no test API or projection bypass is shipped in the application.
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
int wmain(int argc,wchar_t**argv)
{
    std::cout.setf(std::ios::unitbuf);
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
