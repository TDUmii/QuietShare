#include <windows.h>
#include <cstdio>
#include <string>

namespace
{
unsigned noticeBeepCalls=0,noticeFlashCalls=0;
UINT noticeBeepType=0;
FLASHWINFO noticeLastFlash{};
BOOL WINAPI NoticeTestBeep(UINT type)
{
    ++noticeBeepCalls;noticeBeepType=type;return TRUE;
}
BOOL WINAPI NoticeTestFlash(PFLASHWINFO info)
{
    ++noticeFlashCalls;if(info)noticeLastFlash=*info;return TRUE;
}
}

// Exercise the actual shell functions and message cases. These spies exist only
// in this test translation unit, never in the shipped application.
#define MessageBeep NoticeTestBeep
#define FlashWindowEx NoticeTestFlash
#undef NOMINMAX
#include "../src/main.cpp"
#undef MessageBeep
#undef FlashWindowEx

int wmain()
{
    unsigned passed=0,failed=0;bool first=true;
    std::puts("{\"checks\":[");
    auto check=[&](const char* name,bool ok) {
        std::printf("%s{\"name\":\"%s\",\"passed\":%s}",first?"":",\n",name,ok?"true":"false");
        first=false;ok?++passed:++failed;
    };
    // This fixture stays hidden throughout; it cannot capture or show content.
    window=CreateWindowExW(0,L"STATIC",L"QuietShare",WS_OVERLAPPEDWINDOW,
                           12,24,430,320,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    check("hiddenNoticeFixtureCreated",window&&IsWindow(window)&&!IsWindowVisible(window));
    if(!window)return 1;
    const HWND stopButton=CreateWindowExW(0,L"BUTTON",L"Stop",WS_CHILD|WS_DISABLED,
                                         0,0,80,24,window,(HMENU)MIRROR_STOP,GetModuleHandleW(nullptr),nullptr);
    check("hiddenStopButtonStartsDisabled",stopButton&&!IsWindowVisible(stopButton)&&!IsWindowEnabled(stopButton));
    mirror=std::make_unique<MirrorController>();
    auto caption=[](){wchar_t value[256]{};GetWindowTextW(window,value,256);return std::wstring(value);};
    const HWND foreground=GetForegroundWindow();RECT before{};GetWindowRect(window,&before);
    english=false;qa=false;masked=false;shuttingDown=false;
    scriptDraft=L"Synthetic private text that must never enter the public caption";

    Proc(window,MirrorStoppedMessage,0,0);
    Proc(window,MirrorStoppedMessage,999,0);
    check("unsolicitedAndStaleStopMessagesRejected",!projectionStoppedNotice&&caption()==L"QuietShare"&&noticeBeepCalls==0&&noticeFlashCalls==0);

    ShowProjectionStoppedNotice();
    check("unexpectedStopHasFixedVietnameseCaption",projectionStoppedNotice&&caption()==L"QuietShare - Trình chiếu đã dừng");
    check("unexpectedStopEnablesAcknowledgmentButton",IsWindowEnabled(stopButton));
    UpdateButtons();
    check("buttonRefreshPreservesStopAcknowledgmentAndCaption",IsWindowEnabled(stopButton)&&caption()==L"QuietShare - Trình chiếu đã dừng");
    check("unexpectedStopSignalsOnceWithoutPopup",noticeBeepCalls==1&&noticeBeepType==MB_ICONEXCLAMATION&&noticeFlashCalls==1&&
          noticeLastFlash.hwnd==window&&noticeLastFlash.dwFlags==FLASHW_TRAY&&noticeLastFlash.uCount==3);
    RECT after{};GetWindowRect(window,&after);
    check("noticeDoesNotRestoreMoveOrFocus",!IsWindowVisible(window)&&EqualRect(&before,&after)&&GetForegroundWindow()==foreground);
    ShowProjectionStoppedNotice();
    check("duplicateVisualNoticeDoesNotRepeatSignals",noticeBeepCalls==1&&noticeFlashCalls==1);

    Proc(window,WM_COMMAND,MAKEWPARAM(LANGUAGE,BN_CLICKED),0);
    check("noticeLanguageUpdatesWithoutRepeatingSignals",english&&caption()==L"QuietShare - Projection stopped"&&noticeBeepCalls==1&&noticeFlashCalls==1);
    Proc(window,WM_COMMAND,MAKEWPARAM(MIRROR_START,BN_CLICKED),0);
    check("failedRestartPreservesNotice",projectionStoppedNotice&&caption()==L"QuietShare - Projection stopped");
    qa=true;VerifyPrivacy();
    check("verificationDoesNotDismissStoppedProjection",projectionStoppedNotice&&caption()==L"QuietShare - Projection stopped");
    qa=false;
    Proc(window,WM_COMMAND,MAKEWPARAM(MIRROR_STOP,BN_CLICKED),0);
    check("explicitStopDismissesAndStopsTaskbarAttention",!projectionStoppedNotice&&caption()==L"QuietShare"&&noticeBeepCalls==1&&
          noticeFlashCalls==2&&noticeLastFlash.dwFlags==FLASHW_STOP);
    check("acknowledgmentDisablesStoppedButton",!IsWindowEnabled(stopButton));

    qa=true;ShowProjectionStoppedNotice();
    check("qaKeepsVisualNoticeWithoutSoundOrFlash",projectionStoppedNotice&&caption()==L"QuietShare - Projection stopped"&&noticeBeepCalls==1&&noticeFlashCalls==2);
    ClearProjectionStoppedNotice();
    check("qaDismissalDoesNotFlash",!projectionStoppedNotice&&caption()==L"QuietShare"&&noticeBeepCalls==1&&noticeFlashCalls==2);

    qa=false;ShowProjectionStoppedNotice();
    ToggleMask();
    check("f8DismissesOldAlarm",masked&&!projectionStoppedNotice&&caption()==L"QuietShare"&&noticeBeepCalls==2&&noticeFlashCalls==4&&noticeLastFlash.dwFlags==FLASHW_STOP);
    ShowProjectionStoppedNotice();
    check("maskedContentDoesNotRaiseAlarm",!projectionStoppedNotice&&noticeBeepCalls==2&&noticeFlashCalls==4);
    masked=false;shuttingDown=true;ShowProjectionStoppedNotice();
    check("shutdownRejectsLateAlarm",!projectionStoppedNotice&&noticeBeepCalls==2&&noticeFlashCalls==4);
    shuttingDown=false;
    const HWND old=window;DestroyWindow(old);ShowProjectionStoppedNotice();
    check("destroyedWindowRejectsLateAlarm",!projectionStoppedNotice&&noticeBeepCalls==2&&noticeFlashCalls==4);
    window=nullptr;mirror.reset();
    std::printf("\n],\"passed\":%u,\"failed\":%u}\n",passed,failed);
    return failed?1:0;
}
