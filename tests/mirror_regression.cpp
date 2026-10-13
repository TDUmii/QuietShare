#include "../src/Mirror.h"
#include "../src/Privacy.h"
#include "../src/ToolMenu.h"
#include "../src/LocalPicker.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <vector>

using namespace quietshare;
namespace
{
int passed=0,failed=0;
MirrorController *menuController=nullptr;
bool menuStayedVerified=false,menuProtected=false;
bool menuIdleDoesNotRepaint=false;
struct SentCancel
{
    HWND window=nullptr;
    DWORD delay=0;
    LONG duringOperation=0,completed=0;
};
DWORD WINAPI CancelFromAnotherThread(void *value)
{
    auto &request=*static_cast<SentCancel *>(value);
    Sleep(request.delay);
    DWORD_PTR result=0;
    SendMessageTimeoutW(request.window,WM_APP+13,0,reinterpret_cast<LPARAM>(&request.duringOperation),
                        SMTO_ABORTIFHUNG,2000,&result);
    InterlockedExchange(&request.completed,1);
    return 0;
}
void Check(bool result,const char *name)
{
    std::cout<<(result?"PASS ":"FAIL ")<<name<<'\n';if(result)++passed;else ++failed;
}
void Put(std::vector<unsigned char> &p,int x,int y,COLORREF c,unsigned stride=1024)
{
    auto *v=p.data()+y*stride+x*4;v[0]=GetBValue(c);v[1]=GetGValue(c);v[2]=GetRValue(c);v[3]=255;
}
std::vector<unsigned char> Fixture(bool marker,unsigned stride=1024)
{
    const COLORREF a=RGB(34,92,182),b=RGB(211,147,37);
    std::vector<unsigned char> pixels(stride*192);
    for(int y=0;y<192;++y)for(int x=0;x<256;++x)
    {
        COLORREF c=((x/16+y/16)&1)?a:b;
        if(marker&&x>=64&&x<192&&y>=48&&y<144)
            c=(((x-64)/8+(y-48)/8)&1)?RGB(250,20,240):RGB(20,250,60);
        Put(pixels,x,y,c,stride);
    }
    return pixels;
}
LRESULT CALLBACK WindowProc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_TIMER && wp==91) {
        HWND popup=FindWindowW(L"QuietShareToolMenu",nullptr);
        if(popup && menuController) {
            DWORD affinity=0;menuProtected=GetWindowDisplayAffinity(popup,&affinity)&&affinity==0x11;
            menuController->Tick();menuStayedVerified=menuController->IsVerified();
            UpdateWindow(popup);
            for(int i=0;i<100;++i)SendMessageW(popup,WM_MOUSEMOVE,0,MAKELPARAM(20,20));
            menuIdleDoesNotRepaint=!GetUpdateRect(popup,nullptr,FALSE);
            PostMessageW(popup,WM_KEYDOWN,VK_ESCAPE,0);
        }
        return 0;
    }
    if(message==WM_APP+9)
    {
        auto *controller=reinterpret_cast<MirrorController *>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
        if(controller)controller->Invalidate();
        return 0;
    }
    if(message==WM_APP+10)
    {
        auto *owner=reinterpret_cast<std::unique_ptr<MirrorController> *>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
        if(owner)owner->reset();
        return 0;
    }
    if(message==WM_APP+13)
    {
        auto *controller=reinterpret_cast<MirrorController *>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
        if(controller)
        {
            *reinterpret_cast<LONG *>(lp)=controller->Busy()?1:0;
            controller->Invalidate();
        }
        return 0;
    }
    if(message==WM_PAINT)
    {
        PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);RECT r{};GetClientRect(hwnd,&r);
        FillRect(dc,&r,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));EndPaint(hwnd,&ps);return 0;
    }
    return DefWindowProcW(hwnd,message,wp,lp);
}
}
int wmain(int argc,wchar_t **argv)
{
    Check(LocalParentFolder(L"C:\\code")==L"C:\\","picker Up preserves a drive root");
    Check(LocalParentFolder(L"C:\\note.py")==L"C:\\","file at drive root reopens the correct picker folder");
    Check(LocalParentFolder(L"C:\\code\\project\\")==L"C:\\code","picker Up handles nested trailing separators");
    Check(LocalParentFolder(L"C:\\").empty(),"picker Up from root returns to drive list");
    std::cout.setf(std::ios::unitbuf);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    const COLORREF a=RGB(34,92,182),b=RGB(211,147,37);
    auto publicOnly=Fixture(false),marker=Fixture(true),black=std::vector<unsigned char>(1024*192,0);
    auto classify=[&](const std::vector<unsigned char>&p,unsigned stride=1024)
    {return ClassifyCanary(p.data(),256,192,stride,a,b);};
    Check(classify(publicOnly)==CanaryResult::MarkerExcluded,"synthetic public pixels survive exclusion");
    Check(classify(marker)==CanaryResult::MarkerPresent,"unprotected positive control detects marker");
    Check(classify(black)==CanaryResult::Ambiguous,"black frame cannot verify privacy");
    Check(ClassifyCanary(nullptr,256,192,1024,a,b)==CanaryResult::Ambiguous,"null frame rejects");
    Check(ClassifyCanary(publicOnly.data(),255,192,1024,a,b)==CanaryResult::Ambiguous,"wrong capture dimensions reject");
    Check(ClassifyCanary(publicOnly.data(),256,192,1,a,b)==CanaryResult::Ambiguous,"undersized stride rejects");
    Check(ClassifyCanary(publicOnly.data(),256,192,1024,a,a)==CanaryResult::Ambiguous,"ambiguous checker colors reject");
    auto missingOutside=publicOnly;for(int y=0;y<48;++y)for(int x=0;x<256;++x)Put(missingOutside,x,y,0);
    Check(classify(missingOutside)==CanaryResult::Ambiguous,"missing surrounding public pixels reject");
    auto blackInside=publicOnly;for(int y=48;y<144;++y)for(int x=64;x<192;++x)Put(blackInside,x,y,0);
    Check(classify(blackInside)==CanaryResult::Ambiguous,"masked black region cannot verify exclusion");
    auto partialMarker=publicOnly;for(int y=48;y<80;++y)for(int x=64;x<192;++x)
        Put(partialMarker,x,y,(((x-64)/8+(y-48)/8)&1)?RGB(250,20,240):RGB(20,250,60));
    Check(classify(partialMarker)==CanaryResult::Ambiguous,"partly visible marker rejects");
    auto padded=Fixture(false,1088);
    Check(classify(padded,1088)==CanaryResult::MarkerExcluded,"D3D row pitch padding supported");
    auto noise=publicOnly;for(auto &value:noise)if(value<240)value+=3;
    Check(classify(noise)==CanaryResult::MarkerExcluded,"small SDR rounding tolerated");
    auto random=publicOnly;for(size_t i=0;i<random.size();++i)random[i]=static_cast<unsigned char>((i*73)%256);
    Check(classify(random)==CanaryResult::Ambiguous,"unrelated content cannot pass");
    MirrorController controller;
    Check(!controller.IsVerified()&&!controller.Running()&&!controller.Busy(),"initial content gate closed");
    Check(!controller.Verify(nullptr,L"invalid")&&!controller.IsVerified(),"invalid private window locks");
    controller.Invalidate();controller.Tick();
    Check(!controller.IsVerified()&&!controller.Projector(),"invalidation leaves no output");
    const auto monitors=EnumerateMonitors();
    Check(!monitors.empty(),"active monitors enumerated");
    if(!monitors.empty())
    {
        const auto &source=monitors.front();
        Check(!source.stableId.empty()&&source.bounds.right>source.bounds.left,"monitor identity and geometry valid");
        Check(!controller.Start(source.stableId,source.stableId)&&!controller.Projector(),"same source and target never creates output");
    }
    controller.Stop();
    Check(!controller.IsVerified()&&!controller.Running()&&controller.Fps()==0&&!controller.Projector(),"stop releases output and verification");
    bool actualWgc=false;
    std::wstring actualStatus;
    ULONGLONG elapsed=0;
    if(!monitors.empty())
    {
        WNDCLASSW wc{};wc.hInstance=GetModuleHandleW(nullptr);wc.lpfnWndProc=WindowProc;
        wc.lpszClassName=L"QuietShareMirrorRegression";RegisterClassW(&wc);
        HWND privateWindow=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Synthetic privacy regression",
            WS_OVERLAPPEDWINDOW,monitors.front().bounds.left+16,monitors.front().bounds.top+16,
            350,260,nullptr,nullptr,wc.hInstance,nullptr);
        ShowWindow(privateWindow,SW_SHOWNOACTIVATE);UpdateWindow(privateWindow);
        const ULONGLONG started=GetTickCount64();
        actualWgc=controller.Verify(privateWindow,monitors.front().stableId);
        elapsed=GetTickCount64()-started;actualStatus=controller.Status(true);
        Check(actualWgc==controller.IsVerified(),"actual canary result matches content gate");
        Check(actualWgc,"real WGC positive control then protected exclusion");
        if(actualWgc)
        {
            menuController=&controller;
            SetTimer(privateWindow,91,80,nullptr);
            ToolMenu toolMenu;RECT anchor{24,24,380,64};
            const int menuResult=toolMenu.Show(privateWindow,anchor,96,
                static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)),{{1,L"Synthetic command   Ctrl+O",true,0xea7f},{2,L"Disabled",false,0xea76}});
            KillTimer(privateWindow,91);menuController=nullptr;
            Check(menuProtected,"tools popup owns verified capture exclusion before visibility");
            Check(menuStayedVerified,"real WGC gate stays verified while tools popup is open");
            Check(menuIdleDoesNotRepaint,"100 mouse moves within one menu row schedule no repaints");
            controller.Tick();
            Check(menuResult==0&&controller.IsVerified()&&!FindWindowW(L"QuietShareToolMenu",nullptr),
                  "escape closes tools popup without invalidating capture");
            HWND combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,
                8,8,180,120,privateWindow,nullptr,wc.hInstance,nullptr);
            SendMessageW(combo,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(L"Public synthetic monitor"));
            SendMessageW(combo,CB_SETCURSEL,0,0);
            COMBOBOXINFO comboInfo{};comboInfo.cbSize=sizeof(comboInfo);
            const bool comboReady=GetComboBoxInfo(combo,&comboInfo)&&comboInfo.hwndList;
            Check(comboReady,"native public/private popup fixture created");
            if(comboReady)
            {
                SendMessageW(combo,CB_SHOWDROPDOWN,TRUE,0);
                controller.Tick();
                Check(!controller.IsVerified(),"unregistered native combo popup cannot bypass privacy");
                controller.SetPublicPopups({comboInfo.hwndList});
                Check(controller.Verify(privateWindow,monitors.front().stableId),
                      "explicit public monitor list permits a fresh real canary");
                SendMessageW(combo,CB_SHOWDROPDOWN,TRUE,0);
                controller.Tick();
                Check(controller.IsVerified(),"exact registered public list remains allowed while visible");
                controller.SetPublicPopups({});
                controller.Tick();
                Check(!controller.IsVerified(),"removing popup registration revokes a visible uncovered list");
                SendMessageW(combo,CB_SHOWDROPDOWN,FALSE,0);
                DestroyWindow(combo);
                Check(controller.Verify(privateWindow,monitors.front().stableId),
                      "private popup closure allows fresh real verification");
            }
            else if(combo)DestroyWindow(combo);
            HWND unprotected=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Synthetic unprotected dialog",
                WS_POPUP,24,24,64,64,privateWindow,nullptr,wc.hInstance,nullptr);
            ShowWindow(unprotected,SW_SHOWNOACTIVATE);controller.Tick();
            Check(!controller.IsVerified()&&!controller.Running(),
                  "unprotected extra top-level surface immediately locks all viewers");
            DestroyWindow(unprotected);
            Check(controller.Verify(privateWindow,monitors.front().stableId),
                  "fresh real canary restores confidence after unsafe surface removed");
            SetWindowDisplayAffinity(privateWindow,0);controller.Tick();
            Check(!controller.IsVerified()&&!controller.Running(),"affinity loss immediately revokes confidence");
        }
        const DWORD gdiBefore=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        SetWindowLongPtrW(privateWindow,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(&controller));
        PostMessageW(privateWindow,WM_APP+9,0,0);
        const ULONGLONG cancellationStarted=GetTickCount64();
        const bool cancelled=controller.Verify(privateWindow,monitors.front().stableId);
        Check(!cancelled&&!controller.IsVerified()&&!controller.Busy()&&!controller.Projector(),
              "UI cancellation during WGC preflight leaves content locked");
        Check(GetTickCount64()-cancellationStarted<6000,"preflight cancellation is bounded and responsive");
        Check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdiBefore,
              "cancelled canary releases native drawing resources");
        int interruptedOperations=0;
        for(DWORD delay:{0u,5u,10u,20u,40u,60u,100u,200u})
        {
            SentCancel request{privateWindow,delay};
            HANDLE sender=CreateThread(nullptr,0,CancelFromAnotherThread,&request,0,nullptr);
            const ULONGLONG sentStarted=GetTickCount64();
            const bool outcome=controller.Verify(privateWindow,monitors.front().stableId);
            while(!InterlockedCompareExchange(&request.completed,0,0))
            {
                MSG message{};
                while(PeekMessageW(&message,privateWindow,0,0,PM_REMOVE))
                {TranslateMessage(&message);DispatchMessageW(&message);}
                Sleep(1);
            }
            WaitForSingleObject(sender,1000);CloseHandle(sender);
            interruptedOperations+=request.duringOperation;
            Check(!controller.IsVerified()&&!controller.Busy()&&
                  (!request.duringOperation||!outcome)&&GetTickCount64()-sentStarted<6000,
                  "cross-thread synchronous stop cannot free an active WinRT call");
        }
        Check(interruptedOperations>0,"sent cancellation exercised live capture initialization/preflight");
        auto closingController=std::make_unique<MirrorController>();
        SetWindowLongPtrW(privateWindow,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(&closingController));
        PostMessageW(privateWindow,WM_APP+10,0,0);
        const bool closed=closingController->Verify(privateWindow,monitors.front().stableId);
        Check(!closed&&!closingController,"closing controller during pumped preflight safely cancels");
        SetWindowLongPtrW(privateWindow,GWLP_USERDATA,0);
        controller.Stop();DestroyWindow(privateWindow);
    }
    std::cout<<"Actual WGC: "<<(actualWgc?"PASS":"FAIL")<<"; elapsedMs="<<elapsed<<"; monitors="<<monitors.size()<<'\n';
    std::wcout<<actualStatus<<L'\n';
    if(argc>1)
    {
        std::ofstream report(argv[1],std::ios::binary);
        report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed<<",\"actualWgcCanary\":"
              <<(actualWgc?"true":"false")<<",\"canaryElapsedMs\":"<<elapsed<<",\"monitorCount\":"
              <<monitors.size()<<",\"physicalMirror\":\"NOT TESTED\",\"remoteReceiver\":\"NOT TESTED\"}\n";
    }
    return failed?1:0;
}
