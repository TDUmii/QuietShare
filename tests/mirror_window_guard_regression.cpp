// Native pre-move containment checks. No fake capture/session is presented;
// synthetic geometry is separate from real HWND move/resize lifecycle checks.
#define QUIETSHARE_MIRROR_REGRESSION
#include "../src/Mirror.cpp"
#include <fstream>
#include <iostream>
#include <climits>

using namespace quietshare;
namespace
{
int passed=0,failed=0,committedOutside=0;
MirrorController *active=nullptr;
RECT permitted{};
void Check(bool result,const char *name)
{
    std::cout<<(result?"PASS ":"FAIL ")<<name<<'\n';if(result)++passed;else ++failed;
}
bool Within(RECT rect,RECT bounds)
{
    return rect.left>=bounds.left&&rect.top>=bounds.top&&rect.right<=bounds.right&&rect.bottom<=bounds.bottom;
}
void CheckGeometry()
{
    const RECT rightSource{0,0,1920,1040},leftSource{-1920,-120,0,960};
    for(const RECT bounds:{rightSource,leftSource})
    {
        for(const POINT direction:{POINT{-5000,-5000},POINT{5000,-5000},POINT{-5000,5000},POINT{5000,5000}})
        {
            RECT proposed{direction.x,direction.y,direction.x+430,direction.y+320};
            Check(ConstrainRectToBounds(proposed,bounds)&&Within(proposed,bounds)&&
                  proposed.right-proposed.left==430&&proposed.bottom-proposed.top==320,
                  "edge/corner drag is clamped wholly inside the same signed source bounds");
        }
        RECT oversized{bounds.left-1000,bounds.top-1000,bounds.right+2000,bounds.bottom+2000};
        Check(ConstrainRectToBounds(oversized,bounds)&&SameRect(oversized,bounds),
              "oversized resize is fitted wholly within the source work area");
        RECT extreme{LONG_MIN,LONG_MIN,LONG_MAX,LONG_MAX};
        Check(ConstrainRectToBounds(extreme,bounds)&&SameRect(extreme,bounds),
              "extreme coordinates do not overflow signed width or height");
        RECT invalid{500,500,0,0};
        Check(ConstrainRectToBounds(invalid,bounds)&&Within(invalid,bounds)&&
              invalid.right-invalid.left==1&&invalid.bottom-invalid.top==1,
              "nonpositive resize cannot escape source containment");
    }
    RECT invalidBounds{0,0,0,1080},proposed{0,0,430,320};
    Check(!ConstrainRectToBounds(proposed,invalidBounds),"unusable source bounds cannot yield a permitted rectangle");
}
LRESULT CALLBACK TestProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    if(msg==WM_WINDOWPOSCHANGING)
    {
        const auto result=DefWindowProcW(hwnd,msg,wp,lp);
        if(active)active->ConstrainPrivateWindowPosition(*reinterpret_cast<WINDOWPOS *>(lp));
        return result;
    }
    if((msg==WM_MOVING||msg==WM_SIZING)&&active&&active->ConstrainPrivateWindowRect(*reinterpret_cast<RECT *>(lp)))return TRUE;
    if((msg==WM_MOVE||msg==WM_SIZE)&&active&&active->Running()&&!IsIconic(hwnd))
    {
        RECT rect{};if(GetWindowRect(hwnd,&rect)&&!Within(rect,permitted))++committedOutside;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}
}
namespace quietshare
{
struct MirrorRegressionAccess
{
    static bool Prepare(MirrorController &controller,HWND window,const MonitorInfo &source)
    {
        auto &s=*controller.impl;
        s.privateWindow=window;s.source=source;s.sourceId=source.stableId;
        s.topology=EnumerateMonitors();
        if(!s.PinSourceMovement())return false;
        permitted=s.sourceMovementBounds;
        s.verified=true;s.running=true;s.state=State::Running;return true;
    }
    static void InvalidateCachedSource(MirrorController &controller)
    {
        controller.impl->source.bounds.right+=1;
    }
    static void InvalidateCachedDevice(MirrorController &controller)
    {
        controller.impl->sourceDeviceName=L"synthetic stale source device";
    }
    static unsigned NoticeSerial(MirrorController &controller) {return controller.impl->stoppedSerial;}
    static bool Changed(MirrorController &controller) {return controller.impl->state==State::Changed;}
};
}
int wmain(int argc,wchar_t **argv)
{
    std::cout.setf(std::ios::unitbuf);
    CheckGeometry();
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    const auto monitors=EnumerateMonitors();
    Check(!monitors.empty(),"native source monitor exists for HWND lifecycle checks");
    if(monitors.empty())return 1;
    const auto source=monitors.front();
    WNDCLASSW wc{};wc.lpfnWndProc=TestProc;wc.hInstance=GetModuleHandleW(nullptr);
    wc.lpszClassName=L"QuietShareSyntheticWindowGuard";RegisterClassW(&wc);
    HWND window=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Synthetic containment fixture",WS_OVERLAPPEDWINDOW,
        source.bounds.left+16,source.bounds.top+16,430,320,nullptr,nullptr,wc.hInstance,nullptr);
    Check(window!=nullptr,"native control fixture created without elevation");
    if(!window)return 1;
    ShowWindow(window,SW_SHOWNOACTIVATE);
    MirrorController controller;
    Check(MirrorRegressionAccess::Prepare(controller,window,source),"movement is pinned to the selected native source work area");
    active=&controller;
    for(const POINT move:{POINT{source.bounds.left-5000,source.bounds.top-5000},
                          POINT{source.bounds.right+5000,source.bounds.bottom+5000}})
    {
        SetWindowPos(window,nullptr,move.x,move.y,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        RECT rect{};GetWindowRect(window,&rect);
        Check(Within(rect,permitted)&&controller.Running(),"real SetWindowPos cannot commit a cross-display move or stop the running fixture");
    }
    RECT rect{source.bounds.right,source.bounds.top,source.bounds.right+430,source.bounds.top+320};
    Check(SendMessageW(window,WM_MOVING,0,reinterpret_cast<LPARAM>(&rect))&&Within(rect,permitted),
          "native interactive drag proposal is constrained before placement");
    rect={source.bounds.left-200,source.bounds.top-200,source.bounds.right+200,source.bounds.bottom+200};
    Check(SendMessageW(window,WM_SIZING,WMSZ_BOTTOMRIGHT,reinterpret_cast<LPARAM>(&rect))&&SameRect(rect,permitted),
          "native interactive resize proposal is fitted before placement");
    SetWindowPos(window,nullptr,0,0,10000,10000,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    GetWindowRect(window,&rect);
    Check(SameRect(rect,permitted)&&controller.Running(),"size-only oversized programmatic operation may adjust both size and origin safely");
    SetWindowPos(window,nullptr,permitted.left+16,permitted.top+16,430,320,SWP_NOZORDER|SWP_NOACTIVATE);
    WINDOWPOS extreme{window,nullptr,INT_MAX,INT_MIN,INT_MAX,INT_MAX,SWP_NOZORDER|SWP_NOACTIVATE};
    controller.ConstrainPrivateWindowPosition(extreme);
    Check(extreme.x==permitted.left&&extreme.y==permitted.top&&extreme.cx==permitted.right-permitted.left&&
          extreme.cy==permitted.bottom-permitted.top,"extreme WINDOWPOS coordinates fit without addition overflow");
    ShowWindow(window,SW_MAXIMIZE);GetWindowRect(window,&rect);
    Check(IsZoomed(window)&&Within(rect,permitted)&&controller.Running(),"maximize remains contained after default min/max handling");
    ShowWindow(window,SW_RESTORE);GetWindowRect(window,&rect);
    Check(Within(rect,permitted)&&controller.Running(),"restore from maximize stays on the pinned source");
    ShowWindow(window,SW_SHOWMINNOACTIVE);
    Check(IsIconic(window)&&controller.Running(),"native minimized parking remains allowed without stopping projection");
    WINDOWPLACEMENT placement{};placement.length=sizeof(placement);GetWindowPlacement(window,&placement);
    const LONG width=placement.rcNormalPosition.right-placement.rcNormalPosition.left;
    const LONG height=placement.rcNormalPosition.bottom-placement.rcNormalPosition.top;
    placement.rcNormalPosition={source.bounds.right+1000,source.bounds.bottom+1000,
                                source.bounds.right+1000+width,source.bounds.bottom+1000+height};
    SetWindowPlacement(window,&placement);
    ShowWindow(window,SW_SHOWNOACTIVATE);GetWindowRect(window,&rect);
    Check(!IsIconic(window)&&Within(rect,permitted)&&controller.Running(),"restore from off-source saved placement is clamped before revealing the private HWND");
    Check(committedOutside==0,"no committed native move/resize exposes an unminimized fixture outside its source work area");
    MirrorRegressionAccess::InvalidateCachedSource(controller);
    RECT beforeFailure{};GetWindowRect(window,&beforeFailure);
    SetWindowPos(window,nullptr,source.bounds.right+1000,source.bounds.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    Check(!controller.Running()&&MirrorRegressionAccess::Changed(controller),
          "changed pinned source still fails closed and never follows the moved HWND");
    GetWindowRect(window,&rect);
    Check(SameRect(rect,beforeFailure),"source safety failure cancels the pending cross-display placement");
    const unsigned notices=0;Check(!controller.ConsumeStoppedNotice(notices),"unrelated notification serial cannot acknowledge a source safety failure");
    Check(controller.ConsumeStoppedNotice(MirrorRegressionAccess::NoticeSerial(controller)),
          "source safety failure retains the existing unexpected-stop notification path");
    controller.Stop();
    Check(MirrorRegressionAccess::Prepare(controller,window,source),"second fixture pins its own exact source metadata");
    MirrorRegressionAccess::InvalidateCachedDevice(controller);
    SetWindowPos(window,nullptr,source.bounds.right+1000,source.bounds.top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    Check(!controller.Running()&&MirrorRegressionAccess::Changed(controller),
          "reassigned source monitor name fails closed without retargeting the presentation");
    controller.Stop();
    SetWindowPos(window,nullptr,source.bounds.right+1000,source.bounds.top,430,320,SWP_NOZORDER|SWP_NOACTIVATE);
    GetWindowRect(window,&rect);
    Check(rect.left==source.bounds.right+1000&&!controller.Running(),"explicitly stopped session releases movement restriction without restarting capture");
    active=nullptr;DestroyWindow(window);
    if(argc>1){std::ofstream report(argv[1],std::ios::binary);report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed
        <<",\"physicalTwoDisplayTest\":false,\"testKind\":\"synthetic geometry and real HWND move/resize lifecycle\"}";}
    return failed?1:0;
}
