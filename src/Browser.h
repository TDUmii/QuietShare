#pragma once
#include <windows.h>
#include <memory>
#include <string>
namespace quietshare {
constexpr UINT BrowserChangedMessage=WM_APP+20, BrowserProbeMessage=WM_APP+21;
// Starts only on explicit enable. Callbacks hold weak state, never the UI owner.
class Browser {
public:
    Browser(); ~Browser();
    Browser(const Browser&)=delete;Browser& operator=(const Browser&)=delete;
    bool Enable(HWND parent,const std::wstring& profile);
    void Disable();
    void SetBounds(RECT bounds);
    void ParentMoved();
    void Show(bool visible);
    void Lock();
    bool MountProbe(HWND probe);
    void FinishProbe(bool passed);
    void Navigate(const std::wstring& address);
    void Back();void Forward();void Reload();void Focus();
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
}
