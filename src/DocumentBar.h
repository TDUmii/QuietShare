#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace quietshare {
constexpr UINT DocumentActivateMessage = WM_APP + 10;
constexpr UINT DocumentCloseMessage = WM_APP + 11;
struct TabVisual { std::uint64_t id; std::wstring name, path; int kind; bool script; };
class DocumentBar {
public:
    bool Create(HWND parent, HINSTANCE instance);
    HWND Window() const { return window_; }
    void Update(std::vector<TabVisual> tabs, std::uint64_t active, bool safe,
                bool english, UINT dpi, HFONT font);
    bool Overflowing() const;
    int Offset() const { return offset_; }
    void Scroll(int amount);
private:
    static LRESULT CALLBACK Proc(HWND,UINT,WPARAM,LPARAM);
    void Paint(HDC);
    void Reflow(bool ensureActive);
    void UpdateTooltipRects(bool addTools);
    void ScrollPixels(int amount);
    void ScrollWheel(int delta, int &remainder);
    void Hit(POINT, bool middle);
    HWND window_ = nullptr, tooltip_ = nullptr;
    HFONT font_ = nullptr;
    UINT dpi_ = 96;
    std::vector<TabVisual> tabs_;
    std::vector<RECT> rects_;
    std::uint64_t active_ = 0;
    int offset_ = 0, total_ = 0, hovered_ = -1;
    int verticalWheelRemainder_ = 0, horizontalWheelRemainder_ = 0;
    bool safe_ = false, english_ = false, tooltipSafe_ = false;
};
}
