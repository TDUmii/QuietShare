#include "Mirror.h"
#include "Privacy.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <inspectable.h>
#include <roapi.h>
#include <winstring.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cwchar>
#include <utility>

namespace quietshare
{
// Minimal published WinRT ABI, independently declared for the MinGW build.
// GUIDs/method order: Microsoft windows-rs metadata at commit
// 373c03a820eb5a9d48c14d532f80ae49a5c90408, Windows/Graphics/Capture/mod.rs.
// Desktop/DXGI interop: Microsoft Windows SDK documented interfaces.
struct Size32 { INT32 Width, Height; };
struct CaptureItem : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE DisplayName(HSTRING *) = 0;
    virtual HRESULT STDMETHODCALLTYPE Size(Size32 *) = 0;
    virtual HRESULT STDMETHODCALLTYPE Closed(IUnknown *, INT64 *) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveClosed(INT64) = 0;
};
struct CaptureFrame : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE Surface(IInspectable **) = 0;
    virtual HRESULT STDMETHODCALLTYPE SystemRelativeTime(INT64 *) = 0;
    virtual HRESULT STDMETHODCALLTYPE ContentSize(Size32 *) = 0;
};
struct CaptureSession : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE StartCapture() = 0;
};
struct CaptureSession2 : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE CursorEnabled(boolean *) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCursorEnabled(boolean) = 0;
};
struct FramePool : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE Recreate(IInspectable *, INT32, INT32, Size32) = 0;
    virtual HRESULT STDMETHODCALLTYPE TryGetNextFrame(CaptureFrame **) = 0;
    virtual HRESULT STDMETHODCALLTYPE FrameArrived(IUnknown *, INT64 *) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveFrameArrived(INT64) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateCaptureSession(CaptureItem *, CaptureSession **) = 0;
    virtual HRESULT STDMETHODCALLTYPE DispatcherQueue(IInspectable **) = 0;
};
struct PoolFactory : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE CreateFreeThreaded(IInspectable *, INT32, INT32,
                                                        Size32, FramePool **) = 0;
};
struct CaptureSupport : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE IsSupported(boolean *) = 0;
};
struct ItemInterop : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE CreateForWindow(HWND, REFIID, void **) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateForMonitor(HMONITOR, REFIID, void **) = 0;
};
struct DxgiAccess : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetInterface(REFIID, void **) = 0;
};
struct Closable : IInspectable
{
    virtual HRESULT STDMETHODCALLTYPE Close() = 0;
};
namespace mirror_detail
{
constexpr GUID kItem = {0x79c3f95b,0x31f7,0x4ec2,{0xa4,0x64,0x63,0x2e,0xf5,0xd3,0x07,0x60}};
constexpr GUID kInterop = {0x3628e81b,0x3cac,0x4c60,{0xb7,0xf4,0x23,0xce,0x0e,0x0c,0x33,0x56}};
constexpr GUID kPoolFactory = {0x589b103f,0x6bbc,0x5df5,{0xa9,0x91,0x02,0xe2,0x8b,0x3b,0x66,0xd5}};
constexpr GUID kSupport = {0x2224a540,0x5974,0x49aa,{0xb2,0x32,0x08,0x82,0x53,0x6f,0x4c,0xb5}};
constexpr GUID kAccess = {0xa9b3d012,0x3df2,0x4ee3,{0xb8,0xd1,0x86,0x95,0xf4,0x57,0xd3,0xc1}};
constexpr GUID kClosable = {0x30d5a829,0x7fa4,0x4026,{0x83,0xbb,0xd7,0x5b,0xae,0x4e,0xa9,0x9e}};
constexpr GUID kSession2 = {0x2c39ae40,0x7d2e,0x5044,{0x80,0x4e,0x8b,0x67,0x99,0xd4,0xcf,0x9e}};
template<class T> T Procedure(HMODULE module,const char *name)
{
    const FARPROC address=module?GetProcAddress(module,name):nullptr;
    T function=nullptr;static_assert(sizeof(function)==sizeof(address),"Windows function pointer");
    std::memcpy(&function,&address,sizeof(function));return function;
}

template<class T> class Com
{
    T *p = nullptr;
public:
    Com() = default;
    ~Com() { reset(); }
    Com(const Com &) = delete;
    Com &operator=(const Com &) = delete;
    T *get() const { return p; }
    T *operator->() const { return p; }
    explicit operator bool() const { return p != nullptr; }
    T **put() { reset(); return &p; }
    void reset() { if (p) { p->Release(); p = nullptr; } }
};
void CloseObject(IUnknown *object)
{
    if (!object) return;
    Com<Closable> close;
    if (SUCCEEDED(object->QueryInterface(kClosable, reinterpret_cast<void **>(close.put()))))
        close->Close();
}
template<class T> HRESULT Factory(const wchar_t *name, REFIID iid, Com<T> &factory)
{
    HSTRING text = nullptr;
    HRESULT hr = WindowsCreateString(name, static_cast<UINT32>(std::wcslen(name)), &text);
    if (SUCCEEDED(hr)) hr = RoGetActivationFactory(text, iid, reinterpret_cast<void **>(factory.put()));
    if (text) WindowsDeleteString(text);
    return hr;
}
bool SameLuid(LUID a, LUID b) { return a.LowPart == b.LowPart && a.HighPart == b.HighPart; }
struct ProjectionExecutionRequest
{
    EXECUTION_STATE previous=0;
    ~ProjectionExecutionRequest() { Release(); }
    bool Acquire()
    {
        if(previous)return true;
        previous=SetThreadExecutionState(ES_CONTINUOUS|ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED);
        return previous!=0;
    }
    void Release()
    {
        if(!previous)return;
        const auto restore=previous;previous=0;
        SetThreadExecutionState(restore|ES_CONTINUOUS);
    }
};
INT64 CounterTime100ns(INT64 counter, INT64 frequency)
{
    if(counter<=0||frequency<=0)return 0;
    return static_cast<INT64>(static_cast<long double>(counter)*10000000.0L/frequency);
}
INT64 CaptureClock()
{
    static const INT64 frequency=[](){LARGE_INTEGER value{};return QueryPerformanceFrequency(&value)?value.QuadPart:0;}();
    LARGE_INTEGER counter{};
    return QueryPerformanceCounter(&counter)?CounterTime100ns(counter.QuadPart,frequency):0;
}
bool FrameDue(INT64 now,unsigned fps,INT64 &next)
{
    if(now<=0||!fps||now<next)return false;
    const INT64 interval=10000000/fps;
    // Keep fractional-millisecond cadence without a catch-up burst after a busy UI.
    next=(!next||now-next>interval)?now+interval:next+interval;
    return true;
}
HRESULT FrameAfter(CaptureFrame *frame,INT64 barrier)
{
    if(!frame)return E_POINTER;
    if(!barrier)return S_OK;
    INT64 timestamp=0;
    const HRESULT hr=frame->SystemRelativeTime(&timestamp);
    // SystemRelativeTime is the compositor's QPC time, expressed as a TimeSpan.
    // https://learn.microsoft.com/uwp/api/windows.graphics.capture.direct3d11captureframe.systemrelativetime
    return FAILED(hr)?hr:(timestamp>barrier?S_OK:S_FALSE);
}
bool SameRect(RECT a, RECT b)
{
    return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}
bool ConstrainRectToBounds(RECT &rect,RECT bounds,INT64 left,INT64 top,INT64 width,INT64 height)
{
    const INT64 availableWidth=static_cast<INT64>(bounds.right)-bounds.left;
    const INT64 availableHeight=static_cast<INT64>(bounds.bottom)-bounds.top;
    if(availableWidth<=0||availableHeight<=0)return false;
    width=std::clamp<INT64>(width,1,availableWidth);height=std::clamp<INT64>(height,1,availableHeight);
    left=std::clamp<INT64>(left,bounds.left,static_cast<INT64>(bounds.right)-width);
    top=std::clamp<INT64>(top,bounds.top,static_cast<INT64>(bounds.bottom)-height);
    rect={static_cast<LONG>(left),static_cast<LONG>(top),static_cast<LONG>(left+width),static_cast<LONG>(top+height)};
    return true;
}
bool ConstrainRectToBounds(RECT &rect,RECT bounds)
{
    return ConstrainRectToBounds(rect,bounds,rect.left,rect.top,
                                static_cast<INT64>(rect.right)-rect.left,static_cast<INT64>(rect.bottom)-rect.top);
}
bool WithinMonitor(HWND hwnd,const MonitorInfo &monitor)
{
    RECT r{};
    return GetWindowRect(hwnd,&r)&&r.left>=monitor.bounds.left&&r.top>=monitor.bounds.top&&
           r.right<=monitor.bounds.right&&r.bottom<=monitor.bounds.bottom;
}
bool PrivateWindowLocationSafe(HWND hwnd,const MonitorInfo &monitor)
{
    // Windows parks minimized HWNDs off-screen. They reveal no private surface;
    // require source containment again as soon as the window is restored.
    return IsWindow(hwnd)&&(IsIconic(hwnd)||WithinMonitor(hwnd,monitor));
}
const MonitorInfo *Find(const std::vector<MonitorInfo> &monitors, const std::wstring &id)
{
    for (const auto &monitor : monitors) if (monitor.stableId == id) return &monitor;
    return nullptr;
}
bool Affinity(HWND hwnd)
{
    DWORD affinity = 0;
    return IsWindow(hwnd) && GetWindowDisplayAffinity(hwnd, &affinity) && affinity == 0x11;
}
struct PublicPopupPolicy
{
    std::vector<HWND> monitorLists;
    HWND menuOwner=nullptr;
    DWORD uiThread=GetCurrentThreadId();
};
bool PrivateWindowsCovered(const PublicPopupPolicy *policy=nullptr)
{
    struct Check { bool covered=true;const PublicPopupPolicy *policy; } check{true,policy};
    const BOOL enumerated=EnumWindows([](HWND hwnd,LPARAM context)->BOOL
    {
        if(!IsWindowVisible(hwnd))return TRUE;
        DWORD process=0;GetWindowThreadProcessId(hwnd,&process);
        if(process!=GetCurrentProcessId())return TRUE;
        wchar_t className[128]{};
        if(!GetClassNameW(hwnd,className,128))
        { reinterpret_cast<Check *>(context)->covered=false;return FALSE; }
        const auto *policy=reinterpret_cast<Check *>(context)->policy;
        if(std::wcscmp(className,L"QuietShareSyntheticCanary")==0)return TRUE;
        // A native file picker also creates these classes with private names.
        // Class names alone never establish that a popup contains public labels.
        if(policy&&std::wcscmp(className,L"ComboLBox")==0&&
           std::find(policy->monitorLists.begin(),policy->monitorLists.end(),hwnd)!=policy->monitorLists.end())
            return TRUE;
        if(policy&&policy->menuOwner&&std::wcscmp(className,L"#32768")==0)
        {
            GUITHREADINFO info{};info.cbSize=sizeof(info);
            if(GetGUIThreadInfo(policy->uiThread,&info)&&info.hwndMenuOwner==policy->menuOwner)return TRUE;
        }
        if(!Affinity(hwnd))
        { reinterpret_cast<Check *>(context)->covered=false;return FALSE; }
        return TRUE;
    },reinterpret_cast<LPARAM>(&check));
    return enumerated&&check.covered;
}
bool BasicSafe(HWND hwnd,const PublicPopupPolicy *policy=nullptr)
{
    bool cloned = true;
    BOOL composition = FALSE;
    return WindowsBuild() >= 19041 && SUCCEEDED(DwmIsCompositionEnabled(&composition)) && composition &&
           Affinity(hwnd) && PrivateWindowsCovered(policy) && TryGetDisplayCloned(cloned) && !cloned;
}
bool Near(const unsigned char *pixel, COLORREF color)
{
    return std::abs(int(pixel[2]) - GetRValue(color)) <= 18 &&
           std::abs(int(pixel[1]) - GetGValue(color)) <= 18 &&
           std::abs(int(pixel[0]) - GetBValue(color)) <= 18;
}
COLORREF PublicPixel(int x, int y, COLORREF a, COLORREF b)
{
    return ((x / 16 + y / 16) & 1) ? a : b;
}
COLORREF MarkerPixel(int x, int y)
{
    return ((x / 8 + y / 8) & 1) ? RGB(250, 20, 240) : RGB(20, 250, 60);
}
struct CanaryPaint { COLORREF a, b; bool marker; };
LRESULT CALLBACK CanaryProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NCCREATE)
    {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW *>(lparam)->lpCreateParams));
        return TRUE;
    }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT)
    {
        const auto *paint = reinterpret_cast<const CanaryPaint *>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
        PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd,&ps); RECT client{}; GetClientRect(hwnd,&client);
        if (paint)
        {
            const int step = paint->marker ? 8 : 16;
            for (int y=0;y<client.bottom;y+=step) for (int x=0;x<client.right;x+=step)
            {
                const COLORREF color = paint->marker ? MarkerPixel(x,y) : PublicPixel(x,y,paint->a,paint->b);
                HBRUSH brush = CreateSolidBrush(color); RECT r{x,y,x+step,y+step};
                FillRect(dc,&r,brush); DeleteObject(brush);
            }
        }
        EndPaint(hwnd,&ps); return 0;
    }
    return DefWindowProcW(hwnd,message,wparam,lparam);
}
void RegisterCanary()
{
    WNDCLASSW wc{}; wc.hInstance=GetModuleHandleW(nullptr); wc.lpfnWndProc=CanaryProc;
    wc.lpszClassName=L"QuietShareSyntheticCanary"; RegisterClassW(&wc);
}
struct CanaryWindows
{
    CanaryPaint publicPaint{}, markerPaint{};
    HWND underlay=nullptr, overlay=nullptr;
    RECT region{};
    std::function<void()> unmount;
    void Remove()
    {
        auto release=std::move(unmount);unmount={};
        if(release)release();
        if(overlay){const HWND old=overlay;overlay=nullptr;DestroyWindow(old);}
        if(underlay){const HWND old=underlay;underlay=nullptr;DestroyWindow(old);}
    }
    ~CanaryWindows() { Remove(); }
    bool Create(const MonitorInfo &monitor)
    {
        if(monitor.bounds.right-monitor.bounds.left<300 || monitor.bounds.bottom-monitor.bounds.top<230) return false;
        const DWORD nonce=GetTickCount();
        publicPaint={RGB(28+(nonce&15),92,182),RGB(211,147,31+((nonce>>4)&15)),false};
        markerPaint={0,0,true}; RegisterCanary();
        const int x=monitor.bounds.left+(monitor.bounds.right-monitor.bounds.left-256)/2;
        const int y=monitor.bounds.top+(monitor.bounds.bottom-monitor.bounds.top-192)/2;
        region={x,y,x+256,y+192};
        const DWORD style=WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_NOACTIVATE;
        underlay=CreateWindowExW(style,L"QuietShareSyntheticCanary",L"QuietShare privacy test",
            WS_POPUP,x,y,256,192,nullptr,nullptr,GetModuleHandleW(nullptr),&publicPaint);
        overlay=CreateWindowExW(style,L"QuietShareSyntheticCanary",L"QuietShare synthetic marker",
            WS_POPUP,x+64,y+48,128,96,nullptr,nullptr,GetModuleHandleW(nullptr),&markerPaint);
        if(!underlay||!overlay) return false;
        ShowWindow(underlay,SW_SHOWNOACTIVATE); ShowWindow(overlay,SW_SHOWNOACTIVATE);
        SetWindowPos(overlay,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
        UpdateWindow(underlay); UpdateWindow(overlay); DwmFlush(); return true;
    }
};
struct Capture
{
    Com<ID3D11Device> device;
    Com<ID3D11DeviceContext> context;
    Com<IInspectable> winrtDevice;
    Com<CaptureItem> item;
    Com<FramePool> pool;
    Com<CaptureSession> session;
    Size32 size{};
    ~Capture() { Clear(); }
    void Clear()
    {
        CloseObject(session.get()); CloseObject(pool.get()); session.reset(); pool.reset(); item.reset();
        winrtDevice.reset(); context.reset(); device.reset(); size={};
    }
    bool Create(const MonitorInfo &monitor, bool pointer)
    {
        Clear(); Com<CaptureSupport> supported; boolean yes=false;
        if(FAILED(Factory(L"Windows.Graphics.Capture.GraphicsCaptureSession",kSupport,supported)) ||
           FAILED(supported->IsSupported(&yes)) || !yes || !monitor.adapterKnown) return false;
        Com<IDXGIFactory1> factory; Com<IDXGIAdapter1> adapter;
        if(FAILED(CreateDXGIFactory1(IID_IDXGIFactory1,reinterpret_cast<void **>(factory.put())))) return false;
        for(UINT i=0;;++i)
        {
            if(factory->EnumAdapters1(i,adapter.put())==DXGI_ERROR_NOT_FOUND) return false;
            if(!adapter) return false;
            DXGI_ADAPTER_DESC1 desc{}; if(FAILED(adapter->GetDesc1(&desc))) return false;
            if(SameLuid(desc.AdapterLuid,monitor.adapter)) break;
        }
        D3D_FEATURE_LEVEL level{};
        if(FAILED(D3D11CreateDevice(adapter.get(),D3D_DRIVER_TYPE_UNKNOWN,nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,nullptr,0,D3D11_SDK_VERSION,
            device.put(),&level,context.put()))) return false;
        Com<IDXGIDevice> dxgi;
        if(FAILED(device->QueryInterface(IID_IDXGIDevice,reinterpret_cast<void **>(dxgi.put())))) return false;
        using CreateWinrt=HRESULT(WINAPI *)(IDXGIDevice *,IInspectable **);
        HMODULE dll=GetModuleHandleW(L"d3d11.dll");
        auto create=Procedure<CreateWinrt>(dll,"CreateDirect3D11DeviceFromDXGIDevice");
        if(!create || FAILED(create(dxgi.get(),winrtDevice.put()))) return false;
        Com<ItemInterop> interop;
        if(FAILED(Factory(L"Windows.Graphics.Capture.GraphicsCaptureItem",kInterop,interop)) ||
           FAILED(interop->CreateForMonitor(monitor.handle,kItem,reinterpret_cast<void **>(item.put()))) ||
           FAILED(item->Size(&size)) || size.Width<=0 || size.Height<=0 ||
           size.Width>16384 || size.Height>16384 || INT64(size.Width)*size.Height>40000000) return false;
        Com<PoolFactory> poolFactory;
        if(FAILED(Factory(L"Windows.Graphics.Capture.Direct3D11CaptureFramePool",kPoolFactory,poolFactory)) ||
           FAILED(poolFactory->CreateFreeThreaded(winrtDevice.get(),87,2,size,pool.put())) ||
           FAILED(pool->CreateCaptureSession(item.get(),session.put()))) return false;
        Com<CaptureSession2> cursor;
        if(SUCCEEDED(session->QueryInterface(kSession2,reinterpret_cast<void **>(cursor.put()))))
        {
            if(FAILED(cursor->SetCursorEnabled(pointer))) return false;
        }
        else if(!pointer) return false;
        return SUCCEEDED(session->StartCapture());
    }
    HRESULT Next(Com<CaptureFrame> &frame, Com<ID3D11Texture2D> &texture,INT64 after=0)
    {
        HRESULT hr=pool->TryGetNextFrame(frame.put()); if(FAILED(hr)||!frame) return hr;
        // The pool has two buffers: discard an older queued frame before presenting.
        // Keep the newest frame to avoid accumulating latency when the UI was busy.
        Com<CaptureFrame> newer;
        hr=pool->TryGetNextFrame(newer.put());
        if(FAILED(hr))return hr;
        if(newer){CloseObject(frame.get());newer->AddRef();*frame.put()=newer.get();}
        hr=FrameAfter(frame.get(),after);
        if(hr!=S_OK)
        {
            CloseObject(frame.get());frame.reset();
            return hr; // Never present a queued frame rendered before probe teardown.
        }
        Size32 current{};
        if(FAILED(frame->ContentSize(&current))||current.Width!=size.Width||current.Height!=size.Height)
            return E_FAIL; // New size requires a fresh preflight, never stretch an unverified region.
        Com<IInspectable> surface; Com<DxgiAccess> access;
        hr=frame->Surface(surface.put());
        if(SUCCEEDED(hr))hr=surface->QueryInterface(kAccess,reinterpret_cast<void **>(access.put()));
        if(SUCCEEDED(hr))hr=access->GetInterface(IID_ID3D11Texture2D,reinterpret_cast<void **>(texture.put()));
        return hr;
    }
    CanaryResult Check(ID3D11Texture2D *texture,const MonitorInfo &monitor,const CanaryWindows &canary)
    {
        if(size.Width!=monitor.bounds.right-monitor.bounds.left ||
           size.Height!=monitor.bounds.bottom-monitor.bounds.top) return CanaryResult::Ambiguous;
        D3D11_TEXTURE2D_DESC desc{}; desc.Width=256; desc.Height=192; desc.MipLevels=1;desc.ArraySize=1;
        desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.SampleDesc.Count=1;
        desc.Usage=D3D11_USAGE_STAGING;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
        Com<ID3D11Texture2D> stage;
        if(FAILED(device->CreateTexture2D(&desc,nullptr,stage.put())))return CanaryResult::Ambiguous;
        const UINT x=canary.region.left-monitor.bounds.left,y=canary.region.top-monitor.bounds.top;
        D3D11_BOX box{x,y,0,x+256,y+192,1};
        context->CopySubresourceRegion(stage.get(),0,0,0,0,texture,0,&box);
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if(FAILED(context->Map(stage.get(),0,D3D11_MAP_READ,0,&mapped)))return CanaryResult::Ambiguous;
        const auto result=ClassifyCanary(static_cast<const unsigned char *>(mapped.pData),256,192,
                                      mapped.RowPitch,canary.publicPaint.a,canary.publicPaint.b);
        context->Unmap(stage.get(),0); return result;
    }
};
} // namespace mirror_detail
using namespace mirror_detail;

CanaryResult ClassifyCanary(const unsigned char *pixels,int width,int height,unsigned stride,
                           COLORREF a,COLORREF b)
{
    if(!pixels||width!=256||height!=192||stride<1024||a==b) return CanaryResult::Ambiguous;
    int publicOutside=0,publicInside=0,markerInside=0,totalOutside=0,totalInside=0;
    // Sample away from square boundaries: tolerates small color conversion, not missing/black frames.
    for(int y=4;y<192;y+=8)for(int x=4;x<256;x+=8)
    {
        const auto *pixel=pixels+static_cast<size_t>(y)*stride+x*4;
        const bool inside=x>=64&&x<192&&y>=48&&y<144;
        const bool publicMatch=Near(pixel,PublicPixel(x,y,a,b));
        if(inside)
        {
            ++totalInside;publicInside+=publicMatch;
            markerInside+=Near(pixel,MarkerPixel(x-64,y-48));
        }
        else {++totalOutside;publicOutside+=publicMatch;}
    }
    if(publicOutside*100<totalOutside*98) return CanaryResult::Ambiguous;
    if(markerInside*100>=totalInside*98) return CanaryResult::MarkerPresent;
    if(markerInside==0&&publicInside*100>=totalInside*98) return CanaryResult::MarkerExcluded;
    return CanaryResult::Ambiguous;
}

std::vector<MonitorInfo> EnumerateMonitors()
{
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(nullptr,nullptr,[](HMONITOR handle,HDC,LPRECT,LPARAM param)->BOOL
    {
        auto &list=*reinterpret_cast<std::vector<MonitorInfo> *>(param);
        MONITORINFOEXW info{};info.cbSize=sizeof(info);
        if(!GetMonitorInfoW(handle,&info))return TRUE;
        MonitorInfo monitor;monitor.handle=handle;monitor.bounds=info.rcMonitor;
        monitor.primary=(info.dwFlags&MONITORINFOF_PRIMARY)!=0;
        monitor.stableId=info.szDevice;monitor.name=info.szDevice;
        const auto numberAt=monitor.name.find_last_not_of(L"0123456789");
        if(numberAt!=std::wstring::npos&&numberAt+1<monitor.name.size())
            monitor.displayNumber=static_cast<int>(std::wcstol(monitor.name.c_str()+numberAt+1,nullptr,10));
        DISPLAY_DEVICEW device{};device.cb=sizeof(device);
        if(EnumDisplayDevicesW(info.szDevice,0,&device,EDD_GET_DEVICE_INTERFACE_NAME))
        {
            if(device.DeviceID[0])monitor.stableId=device.DeviceID;
            if(device.DeviceString[0])monitor.name=device.DeviceString;
        }
        list.push_back(std::move(monitor));return TRUE;
    },reinterpret_cast<LPARAM>(&monitors));
    Com<IDXGIFactory1> factory;
    if(FAILED(CreateDXGIFactory1(IID_IDXGIFactory1,reinterpret_cast<void **>(factory.put()))))return monitors;
    for(UINT a=0;;++a)
    {
        Com<IDXGIAdapter1> adapter;
        if(factory->EnumAdapters1(a,adapter.put())==DXGI_ERROR_NOT_FOUND)break;
        if(!adapter)break;
        DXGI_ADAPTER_DESC1 ad{};if(FAILED(adapter->GetDesc1(&ad)))continue;
        for(UINT o=0;;++o)
        {
            Com<IDXGIOutput> output;if(adapter->EnumOutputs(o,output.put())==DXGI_ERROR_NOT_FOUND)break;
            if(!output)break;
            DXGI_OUTPUT_DESC desc{};if(FAILED(output->GetDesc(&desc)))continue;
            for(auto &m:monitors)if(m.handle==desc.Monitor){m.adapter=ad.AdapterLuid;m.adapterKnown=true;}
        }
    }
    return monitors;
}

namespace mirror_detail
{
struct Renderer
{
    Com<IDXGISwapChain> swap;
    Com<ID3D11RenderTargetView> target;
    Com<ID3D11VertexShader> vertex;
    Com<ID3D11PixelShader> pixel;
    Com<ID3D11SamplerState> sampler;
    Com<ID3D11Texture2D> copy;
    Com<ID3D11ShaderResourceView> resource;
    int width=0,height=0,sourceWidth=0,sourceHeight=0;
    void Clear()
    {
        resource.reset();copy.reset();sampler.reset();pixel.reset();vertex.reset();target.reset();swap.reset();
        width=height=sourceWidth=sourceHeight=0;
    }
    bool Create(Capture &capture,HWND hwnd,int w,int h)
    {
        Clear();width=w;height=h;
        Com<IDXGIDevice> dxgi;Com<IDXGIAdapter> adapter;Com<IDXGIFactory> factory;
        if(FAILED(capture.device->QueryInterface(IID_IDXGIDevice,reinterpret_cast<void **>(dxgi.put()))) ||
           FAILED(dxgi->GetAdapter(adapter.put())) ||
           FAILED(adapter->GetParent(IID_IDXGIFactory,reinterpret_cast<void **>(factory.put()))))return false;
        DXGI_SWAP_CHAIN_DESC desc{};desc.BufferDesc.Width=w;desc.BufferDesc.Height=h;
        desc.BufferDesc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;desc.SampleDesc.Count=1;
        desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;desc.BufferCount=2;desc.OutputWindow=hwnd;
        desc.Windowed=TRUE;desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
        if(FAILED(factory->CreateSwapChain(capture.device.get(),&desc,swap.put())))return false;
        factory->MakeWindowAssociation(hwnd,DXGI_MWA_NO_ALT_ENTER);
        Com<ID3D11Texture2D> back;
        if(FAILED(swap->GetBuffer(0,IID_ID3D11Texture2D,reinterpret_cast<void **>(back.put()))) ||
           FAILED(capture.device->CreateRenderTargetView(back.get(),nullptr,target.put())))return false;
        // Compile two small fixed shaders from our own strings. No file, include,
        // user document or arbitrary shader source is read by D3DCompile.
        HMODULE compiler=LoadLibraryExW(L"d3dcompiler_47.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!compiler)return false;
        using Compile=HRESULT(WINAPI *)(LPCVOID,SIZE_T,LPCSTR,const D3D_SHADER_MACRO *,ID3DInclude *,
            LPCSTR,LPCSTR,UINT,UINT,ID3DBlob **,ID3DBlob **);
        auto compile=Procedure<Compile>(compiler,"D3DCompile");
        constexpr char shader[]=
            "struct V{float4 p:SV_POSITION;float2 u:TEXCOORD0;};"
            "V VS(uint i:SV_VertexID){V o;float2 p=float2((i<<1)&2,i&2);"
            "o.u=p;o.p=float4(p.x*2-1,1-p.y*2,0,1);return o;}"
            "Texture2D t:register(t0);SamplerState s:register(s0);"
            "float4 PS(V i):SV_TARGET{return t.Sample(s,i.u);}";
        Com<ID3DBlob> vs,ps,errors;
        bool ok=compile&&SUCCEEDED(compile(shader,sizeof(shader)-1,nullptr,nullptr,nullptr,"VS","vs_4_0",
            D3DCOMPILE_OPTIMIZATION_LEVEL3,0,vs.put(),errors.put()))&&
            SUCCEEDED(compile(shader,sizeof(shader)-1,nullptr,nullptr,nullptr,"PS","ps_4_0",
            D3DCOMPILE_OPTIMIZATION_LEVEL3,0,ps.put(),errors.put()));
        if(ok)ok=SUCCEEDED(capture.device->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),
            nullptr,vertex.put()))&&SUCCEEDED(capture.device->CreatePixelShader(ps->GetBufferPointer(),
            ps->GetBufferSize(),nullptr,pixel.put()));
        // Shader blobs are COM objects implemented by the compiler DLL.
        errors.reset();ps.reset();vs.reset();FreeLibrary(compiler);
        if(!ok)return false;
        D3D11_SAMPLER_DESC sample{};sample.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sample.AddressU=sample.AddressV=sample.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
        sample.MaxLOD=D3D11_FLOAT32_MAX;
        return SUCCEEDED(capture.device->CreateSamplerState(&sample,sampler.put()));
    }
    void Blank(Capture &capture)
    {
        if(target&&capture.context)
        {
            const float black[4]={0,0,0,1};capture.context->ClearRenderTargetView(target.get(),black);
            if(swap)swap->Present(0,0);
        }
    }
    bool Present(Capture &capture,ID3D11Texture2D *texture)
    {
        D3D11_TEXTURE2D_DESC desc{};texture->GetDesc(&desc);
        if(desc.Format!=DXGI_FORMAT_B8G8R8A8_UNORM || desc.Width!=UINT(capture.size.Width) ||
           desc.Height!=UINT(capture.size.Height))return false;
        if(!copy||sourceWidth!=int(desc.Width)||sourceHeight!=int(desc.Height))
        {
            resource.reset();copy.reset();desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags=0;desc.MiscFlags=0;desc.MipLevels=1;desc.ArraySize=1;desc.SampleDesc.Count=1;
            if(FAILED(capture.device->CreateTexture2D(&desc,nullptr,copy.put()))||
               FAILED(capture.device->CreateShaderResourceView(copy.get(),nullptr,resource.put())))return false;
            sourceWidth=desc.Width;sourceHeight=desc.Height;
        }
        ID3D11ShaderResourceView *none=nullptr;
        capture.context->PSSetShaderResources(0,1,&none);
        capture.context->CopyResource(copy.get(),texture);
        const float black[4]={0,0,0,1};capture.context->ClearRenderTargetView(target.get(),black);
        const float scale=std::min(float(width)/sourceWidth,float(height)/sourceHeight);
        D3D11_VIEWPORT viewport{};viewport.Width=sourceWidth*scale;viewport.Height=sourceHeight*scale;
        viewport.TopLeftX=(width-viewport.Width)/2;viewport.TopLeftY=(height-viewport.Height)/2;
        viewport.MinDepth=0;viewport.MaxDepth=1;
        auto *rt=target.get();auto *srv=resource.get();auto *sample=sampler.get();
        capture.context->OMSetRenderTargets(1,&rt,nullptr);capture.context->RSSetViewports(1,&viewport);
        capture.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        capture.context->VSSetShader(vertex.get(),nullptr,0);capture.context->PSSetShader(pixel.get(),nullptr,0);
        capture.context->PSSetShaderResources(0,1,&srv);capture.context->PSSetSamplers(0,1,&sample);
        capture.context->Draw(3,0);capture.context->PSSetShaderResources(0,1,&none);
        const HRESULT hr=swap->Present(0,0);
        return SUCCEEDED(hr)&&SUCCEEDED(capture.device->GetDeviceRemovedReason());
    }
};
enum class State
{
    Unverified,Verifying,Verified,Running,Unsafe,Unsupported,CaptureFailed,
    CanaryFailed,NeedTwoMonitors,SameMonitor,DifferentAdapters,RenderFailed,Changed,WrongLocation
};
} // namespace

struct MirrorController::Impl
{
    Capture capture;
    Renderer renderer;
    HWND privateWindow=nullptr,projector=nullptr;
    std::wstring sourceId,destinationId;
    MonitorInfo source{},destination{};
    RECT sourceMovementBounds{};
    std::wstring sourceDeviceName;
    PublicPopupPolicy publicPopups;
    std::vector<MonitorInfo> topology;
    State state=State::Unverified;
    bool verified=false,busy=false,running=false,roInitialized=false,clearing=false,clearPending=false;
    unsigned operations=0;
    unsigned epoch=0;
    unsigned stoppedSerial=0;
    bool stoppedNoticePending=false;
    ProjectionExecutionRequest execution;
    ULONGLONG checkTick=0,framesTick=0;
#if defined(QUIETSHARE_MIRROR_REGRESSION)
    ULONGLONG testElapsed=0;
    unsigned testPresentedFrames=0;
#endif
    INT64 presentAfter=0,nextFrameAt=0;
    int resourceMode=2;
    unsigned frameCount=0;
    double fps=0;
    ULONGLONG Now() const
    {
#if defined(QUIETSHARE_MIRROR_REGRESSION)
        return GetTickCount64()+testElapsed;
#else
        return GetTickCount64();
#endif
    }
    void CancelStoppedNotice() { ++stoppedSerial;stoppedNoticePending=false; }
    Impl()
    {
        const HRESULT hr=RoInitialize(RO_INIT_SINGLETHREADED);
        roInitialized=SUCCEEDED(hr);
    }
    ~Impl() { Clear();if(roInitialized)RoUninitialize(); }
    struct Operation
    {
        Impl &owner;
        bool verification;
        Operation(Impl &value,bool verify=false):owner(value),verification(verify) {++owner.operations;}
        ~Operation()
        {
            if(verification)owner.busy=false;
            --owner.operations;
            if(!owner.operations&&owner.clearPending)owner.Clear();
        }
    };
    bool BeginVerification(HWND owner,const std::wstring &id,bool mounted)
    {
        CancelStoppedNotice();
        const bool preserve=mounted&&running&&verified&&projector&&owner==privateWindow&&id==sourceId;
        // Keep every verified capture/output object while a same-source surface
        // is probed. Native verification and explicit Start still start fresh.
        if(!preserve){Clear();destinationId.clear();}
        busy=true;state=State::Verifying;privateWindow=owner;
        return preserve;
    }
    void Clear()
    {
        ++epoch;verified=false;running=false;fps=0;frameCount=0;presentAfter=0;nextFrameAt=0;
        // Clear revokes output immediately, even when COM teardown is deferred.
        // Release only our own UI thread's request; retain its earlier state.
        execution.Release();
        const bool defer=operations||clearing;
        if(!defer)clearing=true;
        if(projector)ShowWindow(projector,SW_HIDE);
        // WinRT and D3D calls can dispatch synchronous sent window messages.
        // Stop must revoke/hide immediately, but cannot free COM members while
        // an interrupted Create/Next/Present call still uses them on this stack.
        if(defer){clearPending=true;return;}
        clearPending=false;
        renderer.Blank(capture);
        if(projector)
        {
            const HWND old=projector;projector=nullptr;
            SetWindowLongPtrW(old,GWLP_USERDATA,0);ShowWindow(old,SW_HIDE);DestroyWindow(old);
        }
        if(capture.context)capture.context->ClearState();
        renderer.Clear();capture.Clear();
        clearing=false;clearPending=false;
    }
    bool SameTopology()
    {
        const auto current=EnumerateMonitors();
        if(current.size()!=topology.size())return false;
        for(const auto &before:topology)
        {
            const auto *now=Find(current,before.stableId);
            if(!now||!SameRect(now->bounds,before.bounds)||now->adapterKnown!=before.adapterKnown||
               !SameLuid(now->adapter,before.adapter))return false;
        }
        return true;
    }
    bool SourceMovementInfo(MONITORINFOEXW &info)
    {
        info={};info.cbSize=sizeof(info);
        return source.handle&&GetMonitorInfoW(source.handle,&info)&&SameRect(info.rcMonitor,source.bounds);
    }
    bool PinSourceMovement()
    {
        MONITORINFOEXW info{};
        if(!SourceMovementInfo(info)||!IntersectRect(&sourceMovementBounds,&source.bounds,&info.rcWork))return false;
        sourceDeviceName=info.szDevice;return true;
    }
    void Fail(State reason)
    {
        const bool wasRunning=running;
        const unsigned expectedEpoch=epoch+1,noticeAttempt=stoppedSerial;
        Clear();
        // Native teardown can dispatch an explicit Stop/close synchronously.
        // That later user action owns the outcome and invalidates this notice.
        if(epoch!=expectedEpoch||stoppedSerial!=noticeAttempt)return;
        state=reason;
        if(wasRunning&&IsWindow(privateWindow))
        {
            ++stoppedSerial;stoppedNoticePending=true;
            PostMessageW(privateWindow,MirrorStoppedMessage,stoppedSerial,0);
        }
    }
    static LRESULT CALLBACK ProjectorProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
    {
        if(message==WM_NCCREATE)
        {
            SetWindowLongPtrW(hwnd,GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW *>(lparam)->lpCreateParams));
            return TRUE;
        }
        auto *self=reinterpret_cast<Impl *>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
        if(message==WM_ERASEBKGND)return 1;
        if(message==WM_PAINT)
        {
            PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);RECT r{};GetClientRect(hwnd,&r);
            FillRect(dc,&r,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));EndPaint(hwnd,&ps);return 0;
        }
        if(self&&(message==WM_CLOSE || message==WM_DISPLAYCHANGE ||
            (message==WM_KEYDOWN&&wparam==VK_ESCAPE) ||
            (message==WM_POWERBROADCAST&&wparam==PBT_APMSUSPEND)))
        {self->Fail(State::Changed);return 0;}
        return DefWindowProcW(hwnd,message,wparam,lparam);
    }
    bool CreateProjector()
    {
        WNDCLASSW wc{};wc.hInstance=GetModuleHandleW(nullptr);wc.lpfnWndProc=ProjectorProc;
        wc.lpszClassName=L"QuietSharePublicProjector";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
        wc.hbrBackground=static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));RegisterClassW(&wc);
        const RECT r=destination.bounds;
        projector=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_NOACTIVATE,
            wc.lpszClassName,L"QuietShare public output",WS_POPUP,r.left,r.top,r.right-r.left,r.bottom-r.top,
            nullptr,nullptr,wc.hInstance,this);
        if(!projector || !ApplyPrivacy(projector).applied)return false;
        if(!renderer.Create(capture,projector,r.right-r.left,r.bottom-r.top))return false;
        renderer.Blank(capture);ShowWindow(projector,SW_SHOWNOACTIVATE);UpdateWindow(projector);return true;
    }
};

MirrorController::MirrorController():impl(new Impl) {}
MirrorController::~MirrorController() {impl->Clear();}
bool MirrorController::Verify(HWND privateWindow,const std::wstring &sourceId,const std::function<bool(HWND)>& mountSurface)
{
    // Verify pumps close/F8 messages. Keep the native resources alive until this
    // stack unwinds even if WM_DESTROY deletes the controller during that pump.
    const auto lifetime=impl;
    auto &s=*lifetime;
    if(s.busy||s.clearing||s.operations)return false;
    // A browser proof uses the exact already-verified capture path. Busy blocks
    // Tick/Present while its unprotected positive control is on the source.
    const bool preserveProjection=s.BeginVerification(privateWindow,sourceId,static_cast<bool>(mountSurface));
    Impl::Operation operation(s,true);
    const unsigned attempt=s.epoch;
    auto finish=[&](State reason)->bool {if(reason!=State::Verified)s.Fail(reason);return reason==State::Verified;};
    if(!s.roInitialized)return finish(State::Unsupported);
    if(!ApplyPrivacy(privateWindow).applied||!BasicSafe(privateWindow,&s.publicPopups))return finish(State::Unsafe);
    if(s.epoch!=attempt)return false;
    if(preserveProjection&&!s.SameTopology())return finish(State::Changed);
    if(!preserveProjection)s.topology=EnumerateMonitors();
    const auto *source=Find(s.topology,sourceId);
    if(!source)return finish(State::Changed);
    s.source=*source;s.sourceId=sourceId;
    if(s.epoch!=attempt)return false;
    if(!preserveProjection&&!s.capture.Create(*source,false))return finish(State::CaptureFailed);
    if(s.epoch!=attempt)return false;
    const INT64 probeAfter=CaptureClock();
    if(!probeAfter)return finish(State::CaptureFailed);
    CanaryWindows canary;
    if(!canary.Create(*source))return finish(State::CanaryFailed);
    if(mountSurface) {
        // Black native backing cannot satisfy the positive marker test. Only
        // the mounted browser's SVG canary can produce the expected pixels.
        canary.markerPaint={RGB(0,0,0),RGB(0,0,0),false};
        InvalidateRect(canary.overlay,nullptr,FALSE);UpdateWindow(canary.overlay);
        canary.unmount=[mountSurface](){mountSurface(nullptr);};
        if(!mountSurface(canary.overlay))return finish(State::CanaryFailed);
    }
    bool markerSeen=false,protectedOverlay=false;
    int exclusionFrames=0;
    const ULONGLONG started=GetTickCount64();
    while(GetTickCount64()-started<5000)
    {
        if(s.epoch!=attempt)return false;
        if(!BasicSafe(privateWindow,&s.publicPopups))return finish(State::Unsafe);
        if(preserveProjection&&(!PrivateWindowLocationSafe(privateWindow,*source)||!Affinity(s.projector)))
            return finish(State::Unsafe);
        if(s.epoch!=attempt)return false;
        Com<CaptureFrame> frame;Com<ID3D11Texture2D> texture;
        const HRESULT hr=s.capture.Next(frame,texture,probeAfter);
        if(s.epoch!=attempt){CloseObject(frame.get());return false;}
        if(FAILED(hr)){CloseObject(frame.get());return finish(State::CaptureFailed);}
        if(frame&&texture)
        {
            const auto evidence=s.capture.Check(texture.get(),*source,canary);
            CloseObject(frame.get());
            if(s.epoch!=attempt)return false;
            if(!markerSeen && evidence==CanaryResult::MarkerPresent)
            {
                markerSeen=true;
                if(!ApplyPrivacy(canary.overlay).applied)return finish(State::CanaryFailed);
                protectedOverlay=true;DwmFlush();
            }
            else if(markerSeen&&protectedOverlay)
            {
                if(evidence==CanaryResult::MarkerExcluded)++exclusionFrames;
                else exclusionFrames=0;
                if(exclusionFrames>=2)
                {
                    if(!Affinity(canary.overlay)||!s.SameTopology())return finish(State::Changed);
                    // Remove both public checker backing and private browser
                    // probe before allowing any public frame to render again.
                    canary.Remove();
                    if(s.epoch!=attempt)return false;
                    if(FAILED(DwmFlush()))return finish(State::CanaryFailed);
                    if(s.epoch!=attempt)return false;
                    s.presentAfter=CaptureClock();
                    if(!s.presentAfter)return finish(State::CaptureFailed);
                    if(!BasicSafe(privateWindow,&s.publicPopups)||!s.SameTopology())return finish(State::Changed);
                    if(s.epoch!=attempt)return false;
                    if(preserveProjection&&(!PrivateWindowLocationSafe(privateWindow,*source)||!Affinity(s.projector)))
                        return finish(State::Unsafe);
                    s.verified=true;s.checkTick=s.Now();
                    s.framesTick=s.checkTick;s.frameCount=0;s.fps=0;s.nextFrameAt=0;
                    s.state=preserveProjection?State::Running:State::Verified;
                    return finish(State::Verified);
                }
            }
        }
        // Keep F8/close/suspend responsive. An Invalidate during this pump changes
        // epoch and ends the test before another capture object can be touched.
        MSG message{};
        while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE))
        {
            if(message.message==WM_QUIT)
            {
                PostQuitMessage(static_cast<int>(message.wParam));s.Fail(State::Changed);
            }
            else {TranslateMessage(&message);DispatchMessageW(&message);}
            if(s.epoch!=attempt)return false;
        }
        Sleep(12);
    }
    return finish(State::CanaryFailed);
}
void MirrorController::SetResourceMode(int mode)
{
    auto &s=*impl;
    const int selected=std::clamp(mode,0,2);
    if(s.resourceMode==selected)return;
    s.resourceMode=selected;s.nextFrameAt=0;s.framesTick=s.Now();s.frameCount=0;s.fps=0;
}
int MirrorController::ResourceMode() const {return impl->resourceMode;}
unsigned MirrorController::TargetFps() const
{
    constexpr unsigned limits[]={15,30,60};
    return limits[impl->resourceMode];
}
unsigned MirrorController::TickIntervalMs() const {return 500/TargetFps();}
bool MirrorController::IsVerified() const
{
    const auto &s=*impl;
    return s.verified&&!s.busy&&(!s.running||PrivateWindowLocationSafe(s.privateWindow,s.source))&&
           BasicSafe(s.privateWindow,&s.publicPopups);
}
void MirrorController::SetPublicPopups(const std::vector<HWND> &lists)
{
    auto &policy=impl->publicPopups;
    policy.monitorLists.clear();
    for(HWND hwnd:lists)
    {
        DWORD process=0;GetWindowThreadProcessId(hwnd,&process);
        wchar_t className[128]{};GetClassNameW(hwnd,className,128);
        if(process==GetCurrentProcessId()&&std::wcscmp(className,L"ComboLBox")==0)
            policy.monitorLists.push_back(hwnd);
    }
}
void MirrorController::SetPublicMenuOwner(HWND owner)
{
    DWORD process=0;GetWindowThreadProcessId(owner,&process);
    impl->publicPopups.menuOwner=process==GetCurrentProcessId()?owner:nullptr;
}
void MirrorController::Invalidate() {impl->Fail(State::Changed);}
bool MirrorController::Start(const std::wstring &sourceId,const std::wstring &destinationId,bool pointer)
{
    const auto lifetime=impl;
    auto &s=*lifetime;
    if(s.busy||s.clearing||s.operations)return false;
    s.CancelStoppedNotice();
    const auto monitors=EnumerateMonitors();
    if(monitors.size()<2){s.Fail(State::NeedTwoMonitors);return false;}
    if(sourceId==destinationId){s.Fail(State::SameMonitor);return false;}
    const auto *source=Find(monitors,sourceId),*destination=Find(monitors,destinationId);
    if(!source||!destination){s.Fail(State::Changed);return false;}
    if(!source->adapterKnown||!destination->adapterKnown||!SameLuid(source->adapter,destination->adapter))
    {s.Fail(State::DifferentAdapters);return false;}
    const HWND owner=s.privateWindow;
    if(!owner||!WithinMonitor(owner,*source)){s.Fail(State::WrongLocation);return false;}
    if(!Verify(owner,sourceId))return false;
    Impl::Operation operation(s);
    const unsigned attempt=s.epoch;
    // Keep the exact verified capture session rather than replacing its path.
    Com<CaptureSession2> cursor;
    if(FAILED(s.capture.session->QueryInterface(kSession2,reinterpret_cast<void **>(cursor.put())))||
       FAILED(cursor->SetCursorEnabled(pointer))) {s.Fail(State::CaptureFailed);return false;}
    if(s.epoch!=attempt)return false;
    s.destination=*destination;s.destinationId=destinationId;
    if(!s.SameTopology()||!s.PinSourceMovement()){s.Fail(State::Changed);return false;}
    if(!s.CreateProjector()){s.Fail(State::RenderFailed);return false;}
    if(s.epoch!=attempt)return false;
    if(!s.execution.Acquire()){s.Fail(State::CaptureFailed);return false;}
    if(s.epoch!=attempt){s.execution.Release();return false;}
    s.running=true;s.state=State::Running;s.framesTick=s.Now();s.frameCount=0;s.fps=0;return true;
}
void MirrorController::Stop() {impl->CancelStoppedNotice();impl->Clear();impl->state=State::Unverified;}
bool MirrorController::ConsumeStoppedNotice(unsigned serial)
{
    auto &s=*impl;
    if(!s.stoppedNoticePending||serial!=s.stoppedSerial||s.running)return false;
    s.stoppedNoticePending=false;return true;
}
void MirrorController::Tick()
{
    const auto lifetime=impl;
    auto &s=*lifetime;
    if(s.busy||s.clearing||s.operations||!s.verified)return;
    Impl::Operation operation(s);
    const unsigned attempt=s.epoch;
    const ULONGLONG now=s.Now();
    // Confidence belongs to this exact capture session. Continuously check the
    // actual safety invariants rather than expiring a healthy lesson by age.
    if(!BasicSafe(s.privateWindow,&s.publicPopups)||
       FAILED(s.capture.device->GetDeviceRemovedReason())||
       (s.projector&&!Affinity(s.projector))) {s.Fail(State::Unsafe);return;}
    if(s.epoch!=attempt)return;
    if(s.running&&!PrivateWindowLocationSafe(s.privateWindow,s.source)){s.Fail(State::WrongLocation);return;}
    if(now-s.checkTick>=500)
    {
        s.checkTick=now;
        if(!s.SameTopology()){s.Fail(State::Changed);return;}
        if(s.epoch!=attempt)return;
    }
    const INT64 frameTime=CaptureClock();
    if(!frameTime){s.Fail(State::CaptureFailed);return;}
    if(!FrameDue(frameTime,TargetFps(),s.nextFrameAt))return;
    Com<CaptureFrame> frame;Com<ID3D11Texture2D> texture;
    const HRESULT hr=s.capture.Next(frame,texture,s.presentAfter);
    if(s.epoch!=attempt){CloseObject(frame.get());return;}
    if(FAILED(hr)){CloseObject(frame.get());s.Fail(State::CaptureFailed);return;}
    if(frame&&texture)
    {
        const bool presented=!s.running||s.renderer.Present(s.capture,texture.get());
        CloseObject(frame.get());
        if(!presented){s.Fail(State::RenderFailed);return;}
        if(s.running)
        {
            ++s.frameCount;
#if defined(QUIETSHARE_MIRROR_REGRESSION)
            ++s.testPresentedFrames;
#endif
        }
    }
    if(s.running&&now-s.framesTick>=1000)
    {
        s.fps=1000.0*s.frameCount/(now-s.framesTick);s.frameCount=0;s.framesTick=now;
    }
}
bool MirrorController::ConstrainPrivateWindowRect(RECT &proposed)
{
    const auto lifetime=impl;
    auto &s=*lifetime;
    if(!s.running)return false;
    MONITORINFOEXW info{};
    // The cached source is owned by this capture session. A removed/reassigned
    // monitor is a safety failure, never a reason to follow the moving HWND.
    if(!s.SourceMovementInfo(info)||s.sourceDeviceName!=info.szDevice||
       !ConstrainRectToBounds(proposed,s.sourceMovementBounds))
    {
        GetWindowRect(s.privateWindow,&proposed);
        s.Fail(State::Changed);
    }
    return true;
}
void MirrorController::ConstrainPrivateWindowPosition(WINDOWPOS &proposed)
{
    const auto lifetime=impl;
    const auto &s=*lifetime;
    if(!s.running||proposed.hwnd!=s.privateWindow||IsIconic(s.privateWindow))return;
    RECT current{};if(!GetWindowRect(s.privateWindow,&current))return;
    RECT next=current;
    if(!ConstrainPrivateWindowRect(next))return;
    if(s.running)ConstrainRectToBounds(next,s.sourceMovementBounds,
        proposed.flags&SWP_NOMOVE?current.left:proposed.x,
        proposed.flags&SWP_NOMOVE?current.top:proposed.y,
        proposed.flags&SWP_NOSIZE?static_cast<INT64>(current.right)-current.left:proposed.cx,
        proposed.flags&SWP_NOSIZE?static_cast<INT64>(current.bottom)-current.top:proposed.cy);
    // Honor moves and resizes whose original flags omit the other operation:
    // an oversized resize or a restore can need both size and position fixed.
    if(next.left!=current.left||next.top!=current.top){
        proposed.x=next.left;proposed.y=next.top;proposed.flags&=~SWP_NOMOVE;
    }else if(!(proposed.flags&SWP_NOMOVE)){
        proposed.x=next.left;proposed.y=next.top;
    }
    const int width=next.right-next.left,height=next.bottom-next.top;
    if(width!=current.right-current.left||height!=current.bottom-current.top){
        proposed.cx=width;proposed.cy=height;proposed.flags&=~SWP_NOSIZE;
    }else if(!(proposed.flags&SWP_NOSIZE)){
        proposed.cx=width;proposed.cy=height;
    }
}
bool MirrorController::Running() const {return impl->running;}
bool MirrorController::Busy() const {return impl->busy;}
double MirrorController::Fps() const {return impl->fps;}
HWND MirrorController::Projector() const {return impl->projector;}
std::wstring MirrorController::Status(bool english) const
{
    const wchar_t *vi=L"Chưa xác minh WGC. Nội dung riêng đang khóa.",*en=L"WGC is unverified. Private content is locked.";
    switch(impl->state)
    {
    case State::Verifying:vi=L"Đang kiểm tra marker giả bằng WGC…";en=L"Checking a synthetic marker through WGC…";break;
    case State::Verified:vi=L"WGC đã loại marker thử; đầu nhận bên ngoài chưa xác minh.";en=L"WGC excluded the test marker; external receivers remain unverified.";break;
    case State::Running:vi=L"Đang trình chiếu công khai. Đầu nhận thực tế chưa xác minh.";en=L"Public mirror is running. Physical receivers remain unverified.";break;
    case State::Unsafe:vi=L"Đã khóa: cần Windows 2004+, DWM, affinity 0x11 và Extend. Win+P → Extend.";en=L"Locked: requires Windows 2004+, DWM, affinity 0x11 and Extend. Win+P → Extend.";break;
    case State::Unsupported:vi=L"WinRT không khả dụng. Nội dung riêng đang khóa.";en=L"WinRT is unavailable. Private content is locked.";break;
    case State::CaptureFailed:vi=L"WGC/GPU không cấp frame hợp lệ. Đã khóa; thử xác minh lại.";en=L"WGC/GPU did not provide a valid frame. Locked; verify again.";break;
    case State::CanaryFailed:vi=L"Marker thử không chứng minh được loại trừ capture. Đã khóa.";en=L"The test marker did not prove capture exclusion. Locked.";break;
    case State::NeedTwoMonitors:vi=L"Trình chiếu cần hai màn hình ở Extend; xác minh vẫn dùng được với một màn hình.";en=L"Projection requires two displays in Extend; verification works with one display.";break;
    case State::SameMonitor:vi=L"Nguồn và đích phải là hai màn hình khác nhau.";en=L"Source and destination must be different displays.";break;
    case State::DifferentAdapters:vi=L"Bản này cần hai màn hình cùng GPU; chưa hỗ trợ sao chép giữa hai adapter.";en=L"This version requires both displays on one GPU; cross-adapter copy is unsupported.";break;
    case State::RenderFailed:vi=L"Không dựng được đầu ra D3D11. Đã dừng và khóa.";en=L"D3D11 output could not be rendered. Stopped and locked.";break;
    case State::Changed:vi=L"Capture hoặc cấu hình màn hình đã thay đổi. Đã dừng; xác minh lại.";en=L"Capture or display configuration changed. Stopped; verify again.";break;
    case State::WrongLocation:vi=L"Đã khóa: đặt toàn bộ cửa sổ riêng trên màn hình nguồn rồi xác minh lại.";en=L"Locked: place the entire private window on the source display, then verify again.";break;
    default:break;
    }
    return english?en:vi;
}
} // namespace quietshare
