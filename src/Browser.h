#pragma once
#include <windows.h>
#include <memory>
#include <string>
#include <vector>
namespace quietshare {
constexpr UINT BrowserChangedMessage=WM_APP+20, BrowserProbeMessage=WM_APP+21,
               BrowserZoomMessage=WM_APP+22, // WPARAM generation, LPARAM delta; zero resets.
               BrowserZoomChangedMessage=WM_APP+23,
               BrowserTabActionMessage=WM_APP+24;
enum BrowserTabAction { BrowserTabNew=1, BrowserTabClose, BrowserTabNext, BrowserTabPrevious, BrowserTabQueued };
struct BrowserTab {
    unsigned id=0;
    std::wstring title,address;
    bool loading=false,ready=false,active=false;
    int zoomPercent=100;
};
// Starts only on explicit enable. Callbacks hold weak state, never the UI owner.
class Browser {
public:
    Browser(); ~Browser();
    Browser(const Browser&)=delete;Browser& operator=(const Browser&)=delete;
    bool Enable(HWND parent,const std::wstring& profile,const std::wstring& address=L"https://www.google.com");
    void Disable();
    void SetBounds(RECT bounds);
    void ParentMoved();
    void Show(bool visible);
    void Lock();
    // The token identifies one pending controller, including across tab changes.
    unsigned ProbeToken()const;
    bool MountProbe(HWND probe,unsigned token=0);
    void FinishProbe(bool passed,unsigned token=0);
    std::vector<BrowserTab> Tabs()const;
    int ActiveTab()const;
    unsigned ActiveTabId()const;
    bool CanNewTab()const;
    bool NewTab(const std::wstring& address=L"https://www.google.com");
    bool CloseTab(unsigned id);
    bool ActivateTab(unsigned id);
    bool CycleTab(bool previous=false);
    bool ProcessTabAction(int action);
    void ClearQueuedActions();
    unsigned BackgroundSuspendedCount()const;
    unsigned PendingTabCount()const;
    unsigned ControllerCount()const;
    void Navigate(const std::wstring& address);
    void Back();void Forward();void Reload();void Focus();
    // Session preference, 25-400%. The canary always renders at 100%.
    int ZoomPercent()const;
    bool SetZoomPercent(int percent);
    bool AdjustZoomPercent(int delta);
    bool ResetZoom();
    bool Enabled()const;bool Ready()const;bool Verified()const;
    bool CanBack()const;bool CanForward()const;
    bool Loaded()const;
    bool Suspended()const;
    int ErrorCode()const;
    int State()const; // 0 off, 1 loading/probe, 2 ready, 3 error, 4 locked
    unsigned BrowserProcessId()const;
    unsigned Generation()const;
    std::wstring Address()const;
    std::wstring Status(bool english)const;
private:
    struct Impl;std::shared_ptr<Impl> impl;
};
bool NormalizeWebAddress(const std::wstring& input,std::wstring& output);
// Address-bar input only. Navigation events still use the strict URL validator.
bool ResolveWebInput(const std::wstring& input,std::wstring& output);
}
