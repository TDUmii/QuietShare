#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include "Privacy.h"
#include "Content.h"
#include <algorithm>
#include <commctrl.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <richedit.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <utility>
#include <wincrypt.h>
#include <uxtheme.h>

using namespace quietshare;
namespace
{
// QuietShare design tokens. Keep these values in one place so the native shell,
// owner-drawn controls and RichEdit surface stay visually consistent.
constexpr COLORREF BG = RGB(11, 13, 15), SIDEBAR = RGB(17, 19, 21), CARD = RGB(24, 27, 30),
                   ELEVATED = RGB(34, 38, 42), HOVER = RGB(44, 49, 53), ACCENT = RGB(29, 185, 84),
                   ACCENT_DARK = RGB(22, 93, 49), TEXT = RGB(248, 250, 252), MUTED = RGB(156, 163, 175),
                   DIVIDER = RGB(48, 52, 58), GREEN = RGB(110, 231, 160), AMBER = RGB(245, 184, 82),
                   DISABLED = RGB(105, 112, 121);
constexpr int MODE = 100, PLAY = 101, SMALLER = 102, LARGER = 103, SPEED = 104, MASK = 105, TOPMOST = 106,
              LANGUAGE = 107, RESET_ZOOM = 108, EDITOR = 110, NAV_SCRIPT = 120, NAV_READ = 121,
              NAV_HOME = 122, NAV_SETTINGS = 123, OPEN_FILE = 124, FOCUS = 125, OPEN_DROP = 126,
              HOME_SCRIPT = 127, HOME_SETTINGS = 128, VIEW_FILE = 129, FONT_SLIDER = 130;
constexpr int NORMAL_SIDEBAR = 176, COMPACT_SIDEBAR = 64, CONTROL_HEIGHT = 36, CONTROL_GAP = 8;
constexpr UINT_PTR PRIVACY_TIMER = 1, SCROLL_TIMER = 2, SAVE_TIMER = 3, QA_TIMER = 4;
HINSTANCE instance;
HWND window, editor, underlay, tooltips, hoveredButton, imageViewer, fontSlider, filePicker;
HFONT uiFont, titleFont, brandFont, metaFont;
HBRUSH bgBrush, cardBrush;
UINT dpi = 96;
PrivacyResult privacy;
bool topologyKnown = false, cloned = true, masked = false, reading = false, running = false, loading = false,
     dirty = false, saveFailed = false, f8Registered = false, savedNoteRecovered = false,
     storageBlocked = false, topmost = true, english = false;
int fontPoints = 24, speed = 1;
double scrollRemainder = 0;
ULONGLONG lastScrollTick = 0;
std::wstring dataDir;
enum class Page { Home, Content, Settings };
Page page = Page::Content;
LoadedContent openedContent;
bool viewingFile = false, focusMode = false;
int codePoints = 14, imageZoom = 100, panX = 0, panY = 0;
std::wstring scriptDraft, importError;
bool fileDialogProtected = true;
RECT dropRect{}, settingsCardRect{};
RECT fontLabelRect{};
RECT contentCardRect{};
int railWidth = NORMAL_SIDEBAR, contentLeft = NORMAL_SIDEBAR + 24, contentRight = 0;
bool compactUi = false, shortUi = false;
std::vector<HWND> buttons;
WNDPROC editorProc;
bool qa = false, qaVisual = false, qaStorage = false, qaInteractive = false, qaCaptureOk = true, qaPass = true;
bool qaReading = false, qaReadOnly = false, qaFont = false, qaScrolled = false, qaPaused = false,
     qaMasked = false, qaCompact = false, qaUnchanged = false, qaEditReturned = false, qaLanguage = false;
bool qaRoundtrip = false, qaPlaintextAbsent = false, qaAutosave = false;
bool qaLayoutMatrix = true, qaHover = false, qaEmpty = false, qaNativeScrollbar = false;
bool qaFocus = false, qaHome = false, qaSettings = false, qaCodeFile = false, qaImageFile = false,
     qaPreviewKeepsScript = false, qaDropped = false, qaImageMask = false, qaImagePan = false,
     qaFocusRestored = false;
int qaStage = 0;
LONG qaStartY = 0, qaEndY = 0, qaPausedY = 0;
std::wstring qaDir, qaContent;

const wchar_t *T(const wchar_t *vietnamese, const wchar_t *englishText)
{
    return english ? englishText : vietnamese;
}

void ApplyTextFormat();
void Pause();
void Layout();
void SetReading(bool value);
void ShowScript();
bool OpenContentPath(const std::wstring &path);
bool IsImage() { return viewingFile && openedContent.kind == ContentKind::Image; }
bool IsCode() { return viewingFile && openedContent.kind == ContentKind::Code; }
int &ActiveTextSize() { return IsCode() ? codePoints : fontPoints; }

int S(int value)
{
    return MulDiv(value, dpi, 96);
}
std::wstring TextOf(HWND control)
{
    int length = GetWindowTextLengthW(control);
    std::wstring value(length + 1, L'\0');
    GetWindowTextW(control, value.data(), length + 1);
    value.resize(length);
    return value;
}
std::string Utf8(const std::wstring &value)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), (int)value.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), (int)value.size(), result.data(), size, nullptr, nullptr);
    return result;
}
std::wstring FromUtf8(const std::vector<BYTE> &bytes)
{
    int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char *)bytes.data(),
                                   (int)bytes.size(), nullptr, 0);
    std::wstring result(size, L'\0');
    if (size)
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (const char *)bytes.data(), (int)bytes.size(),
                            result.data(), size);
    return result;
}
bool WriteBytes(const std::wstring &path, const void *bytes, DWORD size)
{
    HANDLE file =
        CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    bool ok = WriteFile(file, bytes, size, &written, nullptr) && written == size;
    CloseHandle(file);
    return ok;
}
std::vector<BYTE> ReadBytes(const std::wstring &path)
{
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return {};
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 || size.QuadPart > 3 * 1024 * 1024)
    {
        CloseHandle(file);
        return {};
    }
    std::vector<BYTE> bytes((size_t)size.QuadPart);
    DWORD read = 0;
    bool ok = ReadFile(file, bytes.data(), (DWORD)bytes.size(), &read, nullptr) && read == bytes.size();
    CloseHandle(file);
    return ok ? bytes : std::vector<BYTE>{};
}
bool SaveNotes()
{
    if (storageBlocked)
        return false;
    std::string plain = Utf8(viewingFile ? scriptDraft : TextOf(editor));
    DATA_BLOB input{(DWORD)plain.size(), (BYTE *)plain.data()}, encrypted{};
    if (!CryptProtectData(&input, L"QuietShare notes", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN,
                          &encrypted))
    {
        SecureZeroMemory(plain.data(), plain.size());
        dirty = true;
        saveFailed = true;
        InvalidateRect(window, nullptr, FALSE);
        return false;
    }
    std::wstring target = dataDir + L"\\notes.dpapi", temporary = target + L".tmp";
    bool ok = WriteBytes(temporary, encrypted.pbData, encrypted.cbData);
    LocalFree(encrypted.pbData);
    if (ok)
        ok = MoveFileExW(temporary.c_str(), target.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    SecureZeroMemory(plain.data(), plain.size());
    dirty = !ok;
    saveFailed = !ok;
    InvalidateRect(window, nullptr, FALSE);
    return ok;
}
bool ReadNotes(std::wstring &text)
{
    auto bytes = ReadBytes(dataDir + L"\\notes.dpapi");
    if (bytes.empty())
        return false;
    DATA_BLOB input{(DWORD)bytes.size(), bytes.data()}, plain{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &plain))
        return false;
    std::vector<BYTE> decoded(plain.pbData, plain.pbData + plain.cbData);
    text = FromUtf8(decoded);
    SecureZeroMemory(decoded.data(), decoded.size());
    SecureZeroMemory(plain.pbData, plain.cbData);
    LocalFree(plain.pbData);
    return true;
}
std::wstring InitialNotes()
{
    std::wstring restored;
    if (ReadNotes(restored))
        return restored;
    std::wstring original = dataDir + L"\\notes.dpapi";
    DWORD attributes = GetFileAttributesW(original.c_str());
    DWORD error = GetLastError();
    if (attributes != INVALID_FILE_ATTRIBUTES ||
        (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND))
    {
        FILETIME time{};
        GetSystemTimeAsFileTime(&time);
        ULARGE_INTEGER stamp{};
        stamp.LowPart = time.dwLowDateTime;
        stamp.HighPart = time.dwHighDateTime;
        std::wstring backup = original + L".recovery-" + std::to_wstring(stamp.QuadPart);
        // Preserve an unreadable existing document before allowing a new autosave.
        savedNoteRecovered = CopyFileW(original.c_str(), backup.c_str(), TRUE) != 0;
        storageBlocked = !savedNoteRecovered;
        return L"";
    }
    return L"MỞ ĐẦU\r\nChào mọi người. Hôm nay mình sẽ chia sẻ ba ý chính.\r\n\r\n"
           L"Ý CHÍNH\r\nDán lời cần nói vào đây bằng Ctrl+V.\r\n\r\n"
           L"KẾT THÚC\r\nCảm ơn mọi người. Mình sẵn sàng nhận câu hỏi.";
}
bool Blocked()
{
    return masked || cloned || !topologyKnown || !privacy.applied || storageBlocked;
}
void SetTopmost(bool value)
{
    topmost = value;
    SetWindowPos(window, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(window, nullptr, FALSE);
}
void AdjustFont(int delta)
{
    if (Blocked())
        return;
    Pause();
    if (IsImage())
    {
        imageZoom = std::clamp(imageZoom + (delta > 0 ? 10 : -10), 25, 400);
        panX = panY = 0;
        InvalidateRect(imageViewer, nullptr, FALSE);
    }
    else
        ActiveTextSize() = std::clamp(ActiveTextSize() + delta, 12, 72);
    ApplyTextFormat();
}
void ResetFont()
{
    if (Blocked())
        return;
    Pause();
    if (IsImage())
    {
        imageZoom = 100;
        panX = panY = 0;
        InvalidateRect(imageViewer, nullptr, FALSE);
    }
    else
        ActiveTextSize() = IsCode() ? 14 : 24;
    ApplyTextFormat();
}
void DrawTextAt(HDC dc, const std::wstring &text, RECT rect, COLORREF color, HFONT selectedFont,
                UINT flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE)
{
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    HGDIOBJ old = SelectObject(dc, selectedFont);
    DrawTextW(dc, text.c_str(), -1, &rect, flags);
    SelectObject(dc, old);
}
void FillRoundRect(HDC dc, const RECT &rect, COLORREF color, int radius)
{
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(brush);
}
void DrawShareGlyph(HDC dc, int x, int y, COLORREF color)
{
    HPEN pen = CreatePen(PS_SOLID, std::max(1, S(2)), color);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    MoveToEx(dc, x + S(11), y + S(10), nullptr);
    LineTo(dc, x + S(19), y + S(6));
    MoveToEx(dc, x + S(10), y + S(13), nullptr);
    LineTo(dc, x + S(20), y + S(19));
    Ellipse(dc, x + S(3), y + S(7), x + S(11), y + S(15));
    Ellipse(dc, x + S(19), y + S(1), x + S(27), y + S(9));
    Ellipse(dc, x + S(19), y + S(17), x + S(27), y + S(25));
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}
void DrawNavIcon(HDC dc, int id, int x, int y, COLORREF color)
{
    HPEN pen = CreatePen(PS_SOLID, std::max(1, S(2)), color);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    if (id == NAV_SCRIPT)
    {
        Rectangle(dc, x + S(3), y + S(2), x + S(19), y + S(22));
        MoveToEx(dc, x + S(7), y + S(8), nullptr);
        LineTo(dc, x + S(15), y + S(8));
        MoveToEx(dc, x + S(7), y + S(12), nullptr);
        LineTo(dc, x + S(15), y + S(12));
        MoveToEx(dc, x + S(7), y + S(16), nullptr);
        LineTo(dc, x + S(13), y + S(16));
    }
    else if (id == NAV_HOME)
    {
        POINT roof[] = {{x + S(2), y + S(10)}, {x + S(12), y + S(2)}, {x + S(22), y + S(10)}};
        Polyline(dc, roof, 3);
        MoveToEx(dc, x + S(5), y + S(9), nullptr); LineTo(dc, x + S(5), y + S(22));
        LineTo(dc, x + S(19), y + S(22)); LineTo(dc, x + S(19), y + S(9));
    }
    else if (id == NAV_SETTINGS)
    {
        for (int row : {5, 12, 19})
        {
            MoveToEx(dc, x + S(2), y + S(row), nullptr); LineTo(dc, x + S(22), y + S(row));
            int knob = row == 12 ? 16 : 8;
            Rectangle(dc, x + S(knob - 2), y + S(row - 2), x + S(knob + 2), y + S(row + 2));
        }
    }
    else
    {
        POINT triangle[] = {{x + S(5), y + S(3)}, {x + S(20), y + S(12)}, {x + S(5), y + S(21)}};
        Polygon(dc, triangle, 3);
    }
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}
void DrawSidebar(HDC dc, int sidebar, int height, bool compact)
{
    RECT rail{0, 0, S(sidebar), height};
    HBRUSH brush = CreateSolidBrush(SIDEBAR);
    FillRect(dc, &rail, brush);
    DeleteObject(brush);
    RECT divider{S(sidebar - 1), 0, S(sidebar), height};
    brush = CreateSolidBrush(DIVIDER);
    FillRect(dc, &divider, brush);
    DeleteObject(brush);

    RECT logo{S(16), S(20), S(48), S(52)};
    FillRoundRect(dc, logo, ACCENT, S(8));
    DrawShareGlyph(dc, logo.left + S(2), logo.top + S(3), BG);
    if (!compact)
    {
        RECT brand{S(60), S(20), S(sidebar - 12), S(52)};
        DrawTextAt(dc, L"QuietShare", brand, TEXT, brandFont);
    }

    int logicalHeight = MulDiv(height, 96, dpi);
    if (logicalHeight < 360)
        return;
    int statusTop = logicalHeight - (compact ? 56 : 80);
    RECT status{S(12), S(statusTop), S(sidebar - 12), height - S(16)};
    if (!compact)
        FillRoundRect(dc, status, CARD, S(16));
    bool safe = !Blocked();
    COLORREF stateColor = safe ? GREEN : masked ? MUTED : AMBER;
    HBRUSH dotBrush = CreateSolidBrush(stateColor);
    HGDIOBJ oldBrush = SelectObject(dc, dotBrush);
    HGDIOBJ oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
    int dotX = compact ? 28 : 24;
    Ellipse(dc, S(dotX), status.top + S(16), S(dotX + 8), status.top + S(24));
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(dotBrush);
    if (!compact)
    {
        RECT state{status.left + S(28), status.top + S(8), status.right - S(8), status.top + S(30)};
        DrawTextAt(dc,
                   masked ? T(L"Đã che", L"Hidden")
                   : safe ? T(L"Loại trừ capture", L"Capture exclusion")
                          : T(L"Đang khóa", L"Locked"),
                   state, stateColor, metaFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT stateHint{status.left + S(28), status.top + S(29), status.right - S(8), status.bottom - S(6)};
        DrawTextAt(dc,
                   masked ? T(L"F8 để mở lại", L"F8 to reveal")
                   : safe ? T(L"Đang bật", L"Active")
                          : T(L"Cần kiểm tra", L"Needs attention"),
                   stateHint, MUTED, metaFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
}
void SetTip(int id, const wchar_t *text)
{
    if (!tooltips)
        return;
    TOOLINFOW tip{sizeof(tip)};
    tip.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    tip.hwnd = window;
    tip.uId = (UINT_PTR)GetDlgItem(window, id);
    tip.lpszText = const_cast<wchar_t *>(text);
    SendMessageW(tooltips, TTM_UPDATETIPTEXTW, 0, (LPARAM)&tip);
}
void UpdateButtons()
{
    SetWindowTextW(window, L"QuietShare");
    SetWindowTextW(GetDlgItem(window, MODE), T(reading ? L"Sửa lời" : L"Đọc", reading ? L"Edit" : L"Read"));
    SetWindowTextW(GetDlgItem(window, NAV_SCRIPT), T(L"Lời nhắc", L"Script"));
    SetWindowTextW(GetDlgItem(window, NAV_READ), T(L"Đọc", L"Read"));
    SetWindowTextW(GetDlgItem(window, NAV_HOME), T(L"Chuẩn bị", L"Home"));
    SetWindowTextW(GetDlgItem(window, NAV_SETTINGS), T(L"Tùy chỉnh", L"Settings"));
    SetWindowTextW(GetDlgItem(window, OPEN_FILE), T(L"Mở file", L"Open file"));
    SetWindowTextW(GetDlgItem(window, OPEN_DROP), T(L"Mở hoặc thả file", L"Open or drop a file"));
    SetWindowTextW(GetDlgItem(window, HOME_SCRIPT), T(L"Lời nhắc", L"Script"));
    SetWindowTextW(GetDlgItem(window, HOME_SETTINGS), T(L"Tùy chỉnh", L"Settings"));
    SetWindowTextW(GetDlgItem(window, VIEW_FILE), T(L"Xem file", L"View file"));
    SetWindowTextW(GetDlgItem(window, FOCUS), focusMode ? L"F11" : T(L"Tập trung", L"Focus"));
    SetWindowTextW(GetDlgItem(window, SMALLER), IsImage() ? L"-" : L"A-");
    SetWindowTextW(GetDlgItem(window, LARGER), IsImage() ? L"+" : L"A+");
    SetWindowTextW(GetDlgItem(window, PLAY), T(running ? L"Dừng" : L"Chạy", running ? L"Stop" : L"Start"));
    const wchar_t *speeds[] = {T(L"Chậm", L"Slow"), T(L"Vừa", L"Medium"), T(L"Nhanh", L"Fast")};
    SetWindowTextW(GetDlgItem(window, SPEED), speeds[speed]);
    SetWindowTextW(GetDlgItem(window, MASK), masked ? T(L"Mở lại", L"Show") : T(L"Che · F8", L"Hide · F8"));
    SetWindowTextW(GetDlgItem(window, TOPMOST), topmost ? T(L"Ghim", L"Pin") : T(L"Thả", L"Free"));
    SetWindowTextW(GetDlgItem(window, LANGUAGE), english ? L"VI" : L"EN");
    SetWindowTextW(GetDlgItem(window, RESET_ZOOM), L"100%");
    for (int id : {MODE, PLAY, SMALLER, LARGER, SPEED, RESET_ZOOM, NAV_SCRIPT, NAV_READ, OPEN_FILE, OPEN_DROP,
                   HOME_SCRIPT, VIEW_FILE})
        EnableWindow(GetDlgItem(window, id), !Blocked());
    EnableWindow(GetDlgItem(window, SMALLER), !Blocked() && (IsImage() ? imageZoom > 25 : ActiveTextSize() > 12));
    EnableWindow(GetDlgItem(window, LARGER), !Blocked() && (IsImage() ? imageZoom < 400 : ActiveTextSize() < 72));
    EnableWindow(GetDlgItem(window, PLAY), !Blocked() && !IsImage() && !IsCode());
    EnableWindow(GetDlgItem(window, SPEED), !Blocked() && !IsImage() && !IsCode());
    if (fontSlider)
    {
        SendMessageW(fontSlider, TBM_SETRANGE, TRUE, MAKELPARAM(IsImage() ? 25 : 12, IsImage() ? 400 : 72));
        SendMessageW(fontSlider, TBM_SETPOS, TRUE, IsImage() ? imageZoom : ActiveTextSize());
        EnableWindow(fontSlider, !Blocked());
    }
    SetTip(NAV_SCRIPT, T(L"Lời nhắc - Ctrl+Enter", L"Script - Ctrl+Enter"));
    SetTip(NAV_READ, T(L"Đọc - Ctrl+Enter", L"Read - Ctrl+Enter"));
    SetTip(PLAY, T(running ? L"Dừng cuộn - Space" : L"Cuộn tự động - Space",
                   running ? L"Stop scrolling - Space" : L"Auto-scroll - Space"));
    SetTip(SPEED, T(L"Chọn tốc độ cuộn", L"Choose scrolling speed"));
    SetTip(MASK, T(masked ? L"Mở lại nội dung - F8" : L"Che nội dung - F8",
                   masked ? L"Reveal the script - F8" : L"Hide the script - F8"));
    SetTip(SMALLER, T(L"Giảm cỡ chữ - Ctrl+-", L"Smaller text - Ctrl+-"));
    SetTip(LARGER, T(L"Tăng cỡ chữ - Ctrl++", L"Larger text - Ctrl++"));
    SetTip(RESET_ZOOM, IsImage() ? T(L"Vừa khung - Ctrl+0", L"Fit to window - Ctrl+0")
                                : IsCode() ? T(L"Cỡ chữ mặc định 14 pt - Ctrl+0", L"Default size 14 pt - Ctrl+0")
                                           : T(L"Cỡ chữ mặc định 24 pt - Ctrl+0", L"Default size 24 pt - Ctrl+0"));
    SetTip(TOPMOST, T(topmost ? L"Bỏ ghim cửa sổ" : L"Ghim cửa sổ trên cùng",
                      topmost ? L"Unpin the window" : L"Keep the window on top"));
    SetTip(LANGUAGE, english ? L"Chuyển sang tiếng Việt" : L"Switch to English");
    SetTip(NAV_HOME, T(L"Chuẩn bị nội dung", L"Prepare content"));
    SetTip(NAV_SETTINGS, T(L"Tùy chỉnh hiển thị", L"Display settings"));
    SetTip(FOCUS, T(L"Ẩn/hiện sidebar và toolbar - F11", L"Hide/show sidebar and toolbar - F11"));
    for (HWND button : buttons)
        InvalidateRect(button, nullptr, FALSE);
    InvalidateRect(window, nullptr, FALSE);
}
void Pause()
{
    running = false;
    KillTimer(window, SCROLL_TIMER);
    scrollRemainder = 0;
    UpdateButtons();
}
void Layout()
{
    if (!window || !editor) return;
    RECT client{}; GetClientRect(window, &client);
    int width = MulDiv(client.right, 96, dpi), height = MulDiv(client.bottom, 96, dpi);
    compactUi = width < 700; shortUi = height < 380;
    railWidth = focusMode ? 0 : compactUi ? COMPACT_SIDEBAR : NORMAL_SIDEBAR;
    int gutter = focusMode ? 12 : compactUi ? 16 : 24;
    contentLeft = railWidth + gutter; contentRight = width - gutter;
    fontLabelRect = {};
    for (HWND button : buttons) ShowWindow(button, SW_HIDE);
    ShowWindow(fontSlider, SW_HIDE);
    auto move = [&](int id, int left, int top, int w, int h = CONTROL_HEIGHT) {
        HWND child = GetDlgItem(window, id);
        MoveWindow(child, S(left), S(top), S(w), S(h), TRUE); ShowWindow(child, SW_SHOW);
    };
    if (!focusMode)
    {
        int y = 72;
        for (int id : {NAV_HOME, NAV_SCRIPT, NAV_READ, NAV_SETTINGS})
        { move(id, 12, y, railWidth - 24, 32); y += 40; }
        move(FOCUS, 12, height < 360 ? height - 48 : height - 128, railWidth - 24, 36);
    }
    contentCardRect = {S(contentLeft), S(56), S(contentRight), S(height - 40)};
    if (focusMode)
    {
        contentCardRect = {S(contentLeft), S(12), S(contentRight), S(height - 12)};
        move(FOCUS, contentRight - 52, 20, 44, 28);
    }
    else if (page == Page::Home && !Blocked())
    {
        dropRect = {S(contentLeft), S(56), S(contentRight), S(height - 112)};
        move(OPEN_DROP, contentLeft, 56, contentRight - contentLeft, height - 168);
        int count = openedContent.path.empty() ? 2 : 3;
        int buttonWidth = (contentRight - contentLeft - (count - 1) * 8) / count;
        int x = contentLeft;
        if (count == 3) { move(VIEW_FILE, x, height - 96, buttonWidth, 48); x += buttonWidth + 8; }
        move(HOME_SCRIPT, x, height - 96, buttonWidth, 48); x += buttonWidth + 8;
        move(HOME_SETTINGS, x, height - 96, buttonWidth, 48);
    }
    else if (page == Page::Settings && !Blocked())
    {
        settingsCardRect = contentCardRect;
        int x = contentLeft + 16, y = 104;
        move(SMALLER, x, y, 32); x += 32;
        fontLabelRect = {S(x), S(y), S(x + 40), S(y + 36)}; x += 40;
        move(LARGER, x, y, 32); x += 40; move(RESET_ZOOM, x, y, 56);
        MoveWindow(fontSlider, S(contentLeft + 16), S(148), S(contentRight - contentLeft - 32), S(24), TRUE);
        ShowWindow(fontSlider, SW_SHOW);
        move(TOPMOST, contentLeft + 16, 192, 64);
        move(LANGUAGE, contentLeft + 88, 192, 44);
        move(OPEN_FILE, contentLeft + 148, 192, 100);
    }
    else
    {
        int x = contentLeft, y = reading ? 16 : 56;
        auto nextRow = [&]() { x = contentLeft; y += CONTROL_HEIGHT + CONTROL_GAP; };
        auto group = [&](int w) {
            if (x != contentLeft && x + 16 + w > contentRight) nextRow();
            else if (x != contentLeft) x += 16;
        };
        auto place = [&](int id, int w) { move(id, x, y, w); x += w + CONTROL_GAP; };
        if (reading && !IsImage() && !IsCode()) { place(PLAY, 68); place(SPEED, 80); }
        place(MASK, 100); x -= 8; group(168);
        move(SMALLER, x, y, 32); x += 32;
        fontLabelRect = {S(x), S(y), S(x + 40), S(y + 36)}; x += 40;
        move(LARGER, x, y, 32); x += 40; place(RESET_ZOOM, 56); x -= 8;
        group(116); place(TOPMOST, 64); place(LANGUAGE, 44);
        int top = y + CONTROL_HEIGHT + (shortUi ? 8 : 16);
        contentCardRect = {S(contentLeft), S(top), S(contentRight), S(height - 40)};
    }
    int ew = std::max(1L, contentCardRect.right - contentCardRect.left - S(16));
    int eh = std::max(1L, contentCardRect.bottom - contentCardRect.top - S(16));
    MoveWindow(editor, contentCardRect.left + S(8), contentCardRect.top + S(8), ew, eh, TRUE);
    MoveWindow(imageViewer, contentCardRect.left + S(8), contentCardRect.top + S(8), ew, eh, TRUE);
    RECT textRect{}; GetClientRect(editor, &textRect);
    textRect.left += S(compactUi ? 12 : 16); textRect.right -= S(compactUi ? 8 : 12);
    textRect.top += S(8); textRect.bottom -= S(8);
    SendMessageW(editor, EM_SETRECT, 0, (LPARAM)&textRect);
    bool showContent = page == Page::Content && !Blocked();
    ShowWindow(editor, showContent && !IsImage() ? SW_SHOW : SW_HIDE);
    ShowWindow(imageViewer, showContent && IsImage() ? SW_SHOW : SW_HIDE);
    RedrawWindow(editor, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
    if (focusMode) SetWindowPos(GetDlgItem(window, FOCUS), HWND_TOP, 0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    UpdateButtons();
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void ApplyTextFormat()
{
    loading = true;
    CHARFORMAT2W cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_BOLD;
    cf.yHeight = ActiveTextSize() * 20;
    cf.crTextColor = TEXT;
    wcscpy_s(cf.szFaceName, IsCode() ? L"Consolas" : L"Segoe UI");
    SendMessageW(editor, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    CHARRANGE selection{};
    SendMessageW(editor, EM_EXGETSEL, 0, (LPARAM)&selection);
    SendMessageW(editor, EM_SETSEL, 0, -1);
    PARAFORMAT2 pf{};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_LINESPACING | PFM_SPACEAFTER;
    pf.bLineSpacingRule = 5;
    pf.dyLineSpacing = IsCode() ? 22 : 26;
    pf.dySpaceAfter = IsCode() ? 0 : 100;
    SendMessageW(editor, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
    SendMessageW(editor, EM_EXSETSEL, 0, (LPARAM)&selection);
    loading = false;
    UpdateButtons();
}
void RefreshFonts()
{
    HFONT oldUi = uiFont, oldTitle = titleFont, oldBrand = brandFont, oldMeta = metaFont;
    uiFont = CreateFontW(-S(14), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                         CLEARTYPE_QUALITY, 0, L"Segoe UI");
    titleFont = CreateFontW(-S(22), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                            CLEARTYPE_QUALITY, 0, L"Segoe UI");
    brandFont = CreateFontW(-S(18), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                            CLEARTYPE_QUALITY, 0, L"Segoe UI");
    metaFont = CreateFontW(-S(13), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                           CLEARTYPE_QUALITY, 0, L"Segoe UI");
    for (HWND button : buttons)
        SendMessageW(button, WM_SETFONT, (WPARAM)uiFont, TRUE);
    if (oldUi)
        DeleteObject(oldUi);
    if (oldTitle)
        DeleteObject(oldTitle);
    if (oldBrand)
        DeleteObject(oldBrand);
    if (oldMeta)
        DeleteObject(oldMeta);
}
void SetReading(bool value)
{
    if (Blocked())
        return;
    if (viewingFile && !value) { ShowScript(); return; }
    Pause();
    page = Page::Content;
    reading = value;
    SendMessageW(editor, EM_SETREADONLY, reading || viewingFile, 0);
    SendMessageW(editor, EM_SETSEL, 0, 0);
    POINT origin{};
    SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&origin);
    Layout();
    SetFocus(IsImage() ? imageViewer : !reading || IsCode() ? editor : GetDlgItem(window, PLAY));
}
void ToggleRun()
{
    if (!reading || Blocked() || IsImage() || IsCode())
        return;
    if (running)
        Pause();
    else
    {
        running = true;
        lastScrollTick = GetTickCount64();
        scrollRemainder = 0;
        SetTimer(window, SCROLL_TIMER, 33, nullptr);
        UpdateButtons();
    }
}
void ScrollTick()
{
    if (!running || Blocked() || IsIconic(window) || !IsWindowVisible(window))
    {
        Pause();
        return;
    }
    ULONGLONG now = GetTickCount64();
    const int speeds[] = {18, 32, 52};
    scrollRemainder += S(speeds[speed]) * std::min<ULONGLONG>(now - lastScrollTick, 100) / 1000.0;
    lastScrollTick = now;
    int step = (int)scrollRemainder;
    if (step < 1)
        return;
    scrollRemainder -= step;
    POINT before{}, target{}, after{};
    SendMessageW(editor, EM_GETSCROLLPOS, 0, (LPARAM)&before);
    target = before;
    target.y += step;
    SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&target);
    SendMessageW(editor, EM_GETSCROLLPOS, 0, (LPARAM)&after);
    if (after.y <= before.y)
        Pause();
}
void ToggleMask()
{
    Pause();
    masked = !masked;
    if (masked && filePicker)
    {
        ShowWindow(filePicker, SW_HIDE);
        PostMessageW(filePicker, WM_COMMAND, IDCANCEL, 0);
    }
    Layout();
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void RefreshPrivacy()
{
    ShowWindow(editor, SW_HIDE);
    ShowWindow(imageViewer, SW_HIDE);
    privacy = ApplyPrivacy(window);
    topologyKnown = TryGetDisplayCloned(cloned);
    if (Blocked())
    {
        Pause();
        if (filePicker)
        {
            ShowWindow(filePicker, SW_HIDE);
            PostMessageW(filePicker, WM_COMMAND, IDCANCEL, 0);
        }
    }
    Layout();
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void ShowScript()
{
    if (Blocked()) return;
    Pause();
    if (viewingFile)
    {
        viewingFile = false;
        loading = true;
        SendMessageW(editor, EM_EXLIMITTEXT, 0, 500000);
        SetWindowTextW(editor, scriptDraft.c_str());
        loading = false;
    }
    focusMode = false;
    SetReading(false);
    ApplyTextFormat();
}
void ShowOpenedFile()
{
    if (Blocked() || openedContent.path.empty()) return;
    Pause();
    if (!viewingFile) scriptDraft = TextOf(editor);
    viewingFile = true;
    SetWindowTextW(imageViewer, openedContent.displayName.c_str());
    loading = true;
    SendMessageW(editor, EM_EXLIMITTEXT, 0, 5 * 1024 * 1024);
    SetWindowTextW(editor, openedContent.text.c_str());
    loading = false;
    panX = panY = 0;
    ApplyTextFormat();
    SetReading(true);
    SetFocus(IsImage() ? imageViewer : editor);
}
bool OpenContentPath(const std::wstring &path)
{
    if (Blocked()) return false;
    LoadedContent next;
    std::wstring error;
    if (!LoadLocalContent(path, next, error))
    {
        importError = error;
        page = Page::Home;
        focusMode = false;
        Layout();
        return false;
    }
    openedContent = std::move(next);
    importError.clear();
    ShowOpenedFile();
    return true;
}
UINT_PTR CALLBACK FilePickerHook(HWND hook, UINT message, WPARAM, LPARAM)
{
    if (message == WM_INITDIALOG)
    {
        HWND dialog = GetParent(hook);
        filePicker = dialog;
        fileDialogProtected = ApplyPrivacy(dialog).applied;
        if (!fileDialogProtected || Blocked())
        {
            ShowWindow(dialog, SW_HIDE);
            PostMessageW(dialog, WM_COMMAND, IDCANCEL, 0);
        }
    }
    else if (message == WM_DESTROY) filePicker = nullptr;
    return 0;
}
void ChooseFile()
{
    if (Blocked()) return;
    Pause();
    std::vector<wchar_t> path(32768);
    OPENFILENAMEW choice{sizeof(choice)};
    choice.hwndOwner = window;
    choice.lpstrFilter = T(L"Văn bản / Code / Ảnh\0*.txt;*.md;*.cpp;*.h;*.py;*.js;*.ts;*.tsx;*.json;*.html;*.css;*.png;*.jpg;*.jpeg;*.bmp;*.gif\0Tất cả file\0*.*\0\0",
                          L"Text / Code / Images\0*.txt;*.md;*.cpp;*.h;*.py;*.js;*.ts;*.tsx;*.json;*.html;*.css;*.png;*.jpg;*.jpeg;*.bmp;*.gif\0All files\0*.*\0\0");
    choice.lpstrFile = path.data();
    choice.nMaxFile = (DWORD)path.size();
    choice.lpstrTitle = T(L"Mở nội dung", L"Open content");
    choice.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                   OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
    choice.lpfnHook = FilePickerHook;
    fileDialogProtected = true;
    BOOL selected = GetOpenFileNameW(&choice);
    filePicker = nullptr;
    if (selected && fileDialogProtected) OpenContentPath(path.data());
    else if (!fileDialogProtected)
    {
        importError = T(L"Không bảo vệ được cửa sổ chọn file. / File picker capture exclusion failed.",
                        L"Không bảo vệ được cửa sổ chọn file. / File picker capture exclusion failed.");
        page = Page::Home;
        Layout();
    }
}
void ToggleFocus()
{
    focusMode = !focusMode;
    if (focusMode && !Blocked()) SetReading(true);
    page = Page::Content;
    Layout();
    SetFocus(IsImage() ? imageViewer : editor);
}
LRESULT CALLBACK ImageProc(HWND h, UINT message, WPARAM wp, LPARAM lp)
{
    static POINT previous{};
    if (message == WM_PAINT)
    {
        PAINTSTRUCT paint{}; HDC dc = BeginPaint(h, &paint);
        RECT bounds{}; GetClientRect(h, &bounds);
        HDC buffer = CreateCompatibleDC(dc);
        HBITMAP bitmap = CreateCompatibleBitmap(dc, std::max(1L,bounds.right), std::max(1L,bounds.bottom));
        HGDIOBJ old = SelectObject(buffer, bitmap);
        FillRect(buffer, &bounds, cardBrush);
        if (!Blocked() && IsImage())
        {
            DrawImage(buffer, openedContent, bounds, imageZoom, CARD, panX, panY);
        }
        BitBlt(dc,0,0,bounds.right,bounds.bottom,buffer,0,0,SRCCOPY);
        SelectObject(buffer,old); DeleteObject(bitmap); DeleteDC(buffer);
        EndPaint(h, &paint); return 0;
    }
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_LBUTTONDOWN && !Blocked())
    {
        SetFocus(h); SetCapture(h); previous = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)}; return 0;
    }
    if (message == WM_MOUSEMOVE && GetCapture() == h)
    {
        POINT now{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        panX += now.x - previous.x; panY += now.y - previous.y; previous = now;
        InvalidateRect(h,nullptr,FALSE); return 0;
    }
    if (message == WM_LBUTTONUP) { if(GetCapture()==h) ReleaseCapture(); return 0; }
    if (message == WM_MOUSEWHEEL)
    {
        if (GetKeyState(VK_CONTROL) & 0x8000) AdjustFont(GET_WHEEL_DELTA_WPARAM(wp)>0?2:-2);
        else { panY += GET_WHEEL_DELTA_WPARAM(wp)/2; InvalidateRect(h,nullptr,FALSE); }
        return 0;
    }
    if (message == WM_KEYDOWN)
    {
        if (wp==VK_ESCAPE && focusMode) { ToggleFocus(); return 0; }
        if (wp==L'0' && (GetKeyState(VK_CONTROL) & 0x8000)) { ResetFont(); return 0; }
        if (wp==VK_LEFT) panX += S(24); else if(wp==VK_RIGHT) panX -= S(24);
        else if(wp==VK_UP) panY += S(24); else if(wp==VK_DOWN) panY -= S(24);
        else if(wp==VK_ADD || wp==VK_OEM_PLUS) AdjustFont(2);
        else if(wp==VK_SUBTRACT || wp==VK_OEM_MINUS) AdjustFont(-2);
        else return DefWindowProcW(h,message,wp,lp);
        InvalidateRect(h,nullptr,FALSE); return 0;
    }
    return DefWindowProcW(h,message,wp,lp);
}
void DrawCenteredMessage(HDC dc, RECT bounds, const wchar_t *heading, const wchar_t *body)
{
    InflateRect(&bounds, -S(16), -S(12));
    RECT headingRect{0, 0, bounds.right - bounds.left, 0};
    RECT bodyRect = headingRect;
    HGDIOBJ old = SelectObject(dc, brandFont);
    DrawTextW(dc, heading, -1, &headingRect, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);
    SelectObject(dc, metaFont);
    DrawTextW(dc, body, -1, &bodyRect, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);
    SelectObject(dc, old);
    int total = headingRect.bottom + S(8) + bodyRect.bottom;
    int top = bounds.top + std::max(0L, (bounds.bottom - bounds.top - total) / 2);
    RECT headingDraw{bounds.left, top, bounds.right, top + headingRect.bottom};
    DrawTextAt(dc, heading, headingDraw, TEXT, brandFont, DT_CENTER | DT_WORDBREAK);
    RECT bodyDraw{bounds.left, headingDraw.bottom + S(8), bounds.right,
                  std::min(bounds.bottom, headingDraw.bottom + S(8) + bodyRect.bottom)};
    DrawTextAt(dc, body, bodyDraw, MUTED, metaFont, DT_CENTER | DT_WORDBREAK);
}
void Paint(HDC dc)
{
    RECT client{};
    GetClientRect(window, &client);
    FillRect(dc, &client, bgBrush);
    if (!focusMode) DrawSidebar(dc, railWidth, client.bottom, compactUi);
    RECT title{S(contentLeft), S(12), S(contentRight), S(40)};
    if (!focusMode && (!reading || page != Page::Content))
    {
        DrawTextAt(dc, page == Page::Home ? T(L"Chuẩn bị nội dung", L"Prepare content")
                      : page == Page::Settings ? T(L"Tùy chỉnh", L"Settings")
                                               : T(L"Nhắc chữ riêng", L"Private teleprompter"),
                   title, TEXT, titleFont);
    }
    if (page != Page::Home || Blocked()) FillRoundRect(dc, contentCardRect, CARD, S(24));
    if (page == Page::Settings && !Blocked() && !focusMode)
    {
        RECT summary{S(contentLeft + 16), S(68), S(contentRight - 16), S(96)};
        std::wstring text = IsImage() ? T(L"Ảnh · Phóng/thu", L"Image · Zoom")
                            : IsCode() ? T(L"Code · Cỡ chữ", L"Code · Text size")
                                       : T(L"Lời nhắc · Cỡ chữ", L"Script · Text size");
        DrawTextAt(dc, text, summary, TEXT, brandFont);
    }
    if (!focusMode && fontLabelRect.right > fontLabelRect.left)
    {
        RECT zoomSurface = fontLabelRect; zoomSurface.left -= S(32); zoomSurface.right += S(32);
        FillRoundRect(dc, zoomSurface, ELEVATED, S(16));
        std::wstring label = IsImage() ? std::to_wstring(imageZoom) + L"%" : std::to_wstring(ActiveTextSize());
        DrawTextAt(dc, label, fontLabelRect, TEXT, uiFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    if (Blocked())
    {
        const wchar_t *heading = storageBlocked ? T(L"Lời nhắc đang khóa", L"Script locked")
                                 : masked       ? T(L"Đã che nội dung", L"Content hidden")
                                 : cloned       ? T(L"Đang dùng Duplicate", L"Duplicate display mode")
                                 : !topologyKnown
                                     ? T(L"Chưa xác định màn hình", L"Display layout unknown")
                                     : T(L"Chưa bật loại trừ capture", L"Capture exclusion unavailable");
        const wchar_t *cover =
            storageBlocked ? T(L"Bản lưu cũ được giữ nguyên. Đóng app và kiểm tra quyền đọc tệp lời nhắc.",
                               L"The old script was preserved. Close the app and check file access.")
            : masked ? T(L"Bấm F8 hoặc Mở lại để đọc tiếp.", L"Press F8 or Show to continue.")
            : cloned ? T(L"Win+P > Extend để giữ lời nhắc trên màn hình của bạn.",
                         L"Use Win+P > Extend to keep the script on your private display.")
            : !topologyKnown ? T(L"Nội dung sẽ mở khi xác định được cấu hình màn hình.",
                                 L"Content will open when the display layout is known.")
                             : T(L"Windows chưa áp dụng được cơ chế loại trừ capture.",
                                 L"Windows could not apply capture exclusion.");
        DrawCenteredMessage(dc, contentCardRect, heading, cover);
    }
    if (focusMode) return;
    RECT footer{S(contentLeft), client.bottom - S(32), S(contentRight), client.bottom - S(8)};
    std::wstring footerText =
        masked       ? T(L"Đã che nội dung · F8 để mở lại", L"Content hidden · F8 to reveal")
        : Blocked()  ? T(L"Nội dung đang khóa", L"Content is locked")
        : saveFailed ? T(L"Không lưu được lời nhắc trên máy", L"Could not save the script")
        : savedNoteRecovered && !dirty && !reading ? T(L"Không đọc được lời cũ · Đã giữ bản dự phòng",
                                                       L"Old script unreadable · recovery copy preserved")
        : dirty    ? T(L"Đang lưu...", L"Saving...")
        : !reading ? T(L"Sửa nội dung · Tự lưu trên máy", L"Edit the script · Saved locally")
        : running  ? T(L"Đang cuộn · F8 che nhanh", L"Scrolling · F8 hides quickly")
                   : T(L"Sẵn sàng đọc · F8 che nhanh", L"Ready to read · F8 hides quickly");
    bool storageStatus = saveFailed || dirty || (savedNoteRecovered && !reading);
    if (!Blocked() && page == Page::Home && !storageStatus)
    {
        footerText = importError;
        size_t split = footerText.find(L" / ");
        if (split != std::wstring::npos) footerText = english ? footerText.substr(split + 3) : footerText.substr(0,split);
    }
    else if (!Blocked() && viewingFile && !storageStatus) footerText = openedContent.displayName;
    else if (!Blocked() && page == Page::Settings && !storageStatus) footerText.clear();
    DrawTextAt(dc, footerText, footer, saveFailed || (Blocked() && !masked) || !importError.empty() ? AMBER : MUTED, metaFont,
               DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}
void DrawButton(DRAWITEMSTRUCT *item)
{
    bool disabled = (item->itemState & ODS_DISABLED) != 0;
    bool nav = item->CtlID == NAV_SCRIPT || item->CtlID == NAV_READ || item->CtlID == NAV_HOME || item->CtlID == NAV_SETTINGS;
    bool activeNav = (item->CtlID == NAV_SCRIPT && page == Page::Content && !reading) ||
                     (item->CtlID == NAV_READ && page == Page::Content && reading) ||
                     (item->CtlID == NAV_HOME && page == Page::Home) ||
                     (item->CtlID == NAV_SETTINGS && page == Page::Settings);
    bool primary = item->CtlID == PLAY || item->CtlID == OPEN_FILE;
    bool pressed = (item->itemState & ODS_SELECTED) != 0;
    bool hover = hoveredButton == item->hwndItem && !disabled;
    bool zoom = item->CtlID == SMALLER || item->CtlID == LARGER;
    COLORREF color = disabled    ? CARD
                     : pressed   ? (primary ? ACCENT_DARK : DIVIDER)
                     : hover     ? HOVER
                     : activeNav ? ACCENT_DARK
                     : primary   ? ACCENT
                     : nav       ? SIDEBAR
                                 : ELEVATED;
    if (primary && hover && !pressed)
        color = GREEN;
    HBRUSH background = CreateSolidBrush(nav ? SIDEBAR : zoom ? ELEVATED : page == Page::Settings ? CARD : BG);
    FillRect(item->hDC, &item->rcItem, background);
    DeleteObject(background);
    RECT button = item->rcItem;
    InflateRect(&button, -1, -1);
    if (!zoom || hover || pressed)
        FillRoundRect(item->hDC, button, color, S(16));
    if (nav)
    {
        COLORREF iconColor = disabled ? DISABLED : (activeNav ? GREEN : MUTED);
        int iconLeft = compactUi ? (button.right - button.left - S(24)) / 2 : S(12);
        DrawNavIcon(item->hDC, item->CtlID, button.left + iconLeft,
                    button.top + (button.bottom - button.top - S(24)) / 2, iconColor);
        if (!compactUi)
        {
            RECT label = button;
            label.left += S(44);
            DrawTextAt(item->hDC, TextOf(item->hwndItem), label, disabled ? DISABLED : TEXT, uiFont,
                       DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
    }
    else if (item->CtlID == OPEN_DROP)
    {
        RECT heading = button;
        int middle = (button.bottom + button.top) / 2;
        heading.top = middle - S(32); heading.bottom = middle;
        DrawTextAt(item->hDC, TextOf(item->hwndItem), heading, TEXT, brandFont,
                   DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        RECT detail{button.left + S(16),middle + S(8),button.right - S(16),middle + S(32)};
        DrawTextAt(item->hDC, openedContent.path.empty() ? T(L"Text · Code · Ảnh", L"Text · Code · Images")
                                                     : openedContent.displayName,
                   detail, MUTED, metaFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    else
    {
        DrawTextAt(item->hDC, item->CtlID == FOCUS && compactUi ? L"F11" : TextOf(item->hwndItem), button,
                   disabled  ? DISABLED
                   : primary ? (pressed ? TEXT : BG)
                             : TEXT,
                   uiFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    bool focusVisible =
        GetFocus() == item->hwndItem && !(SendMessageW(window, WM_QUERYUISTATE, 0, 0) & UISF_HIDEFOCUS);
    if (focusVisible || (item->CtlID == TOPMOST && topmost))
    {
        RECT focus = button;
        InflateRect(&focus, -2, -2);
        HPEN pen = CreatePen(PS_SOLID, std::max(1, S(focusVisible ? 2 : 1)),
                             primary        ? TEXT
                             : focusVisible ? GREEN
                                            : DIVIDER);
        HGDIOBJ oldPen = SelectObject(item->hDC, pen);
        HGDIOBJ oldBrush = SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
        RoundRect(item->hDC, focus.left, focus.top, focus.right, focus.bottom, S(14), S(14));
        SelectObject(item->hDC, oldBrush);
        SelectObject(item->hDC, oldPen);
        DeleteObject(pen);
    }
}
LRESULT CALLBACK ButtonProc(HWND control, UINT message, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
    if (message == WM_MOUSEMOVE && IsWindowEnabled(control) && hoveredButton != control)
    {
        HWND previous = hoveredButton;
        hoveredButton = control;
        if (previous)
            InvalidateRect(previous, nullptr, FALSE);
        TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, control, 0};
        TrackMouseEvent(&track);
        InvalidateRect(control, nullptr, FALSE);
    }
    else if (message == WM_MOUSELEAVE && hoveredButton == control)
    {
        hoveredButton = nullptr;
        InvalidateRect(control, nullptr, FALSE);
    }
    else if (message == WM_LBUTTONDOWN)
        SendMessageW(window, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
    else if (message == WM_KEYDOWN)
        SendMessageW(window, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
    LRESULT result = DefSubclassProc(control, message, wp, lp);
    if (message == WM_SETFOCUS || message == WM_KILLFOCUS || message == WM_UPDATEUISTATE)
        InvalidateRect(control, nullptr, FALSE);
    return result;
}
LRESULT CALLBACK EditorProc(HWND control, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_PASTE && (Blocked() || reading || viewingFile)) return 0;
    if (message == WM_PASTE && !reading && !viewingFile && !Blocked())
    {
        if (OpenClipboard(window))
        {
            HANDLE clip = GetClipboardData(CF_UNICODETEXT);
            const wchar_t *plain = clip ? (const wchar_t *)GlobalLock(clip) : nullptr;
            if (plain)
            {
                SendMessageW(control, EM_REPLACESEL, TRUE, (LPARAM)plain);
                GlobalUnlock(clip);
                ApplyTextFormat();
            }
            CloseClipboard();
        }
        return 0;
    }
    if (message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000) && (wp == VK_ADD || wp == VK_OEM_PLUS))
    {
        AdjustFont(2);
        return 0;
    }
    if (message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000) &&
        (wp == VK_SUBTRACT || wp == VK_OEM_MINUS))
    {
        AdjustFont(-2);
        return 0;
    }
    if (message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000) && wp == L'0')
    {
        ResetFont();
        return 0;
    }
    if (message == WM_MOUSEWHEEL && (GetKeyState(VK_CONTROL) & 0x8000))
    {
        AdjustFont(GET_WHEEL_DELTA_WPARAM(wp) > 0 ? 2 : -2);
        return 0;
    }
    if (message == WM_KEYDOWN && wp == VK_RETURN && (GetKeyState(VK_CONTROL) & 0x8000))
    {
        SetReading(!reading);
        return 0;
    }
    if (message == WM_KEYDOWN && wp == VK_ESCAPE && focusMode) { ToggleFocus(); return 0; }
    if (message == WM_KEYDOWN && reading && wp == VK_SPACE)
    {
        ToggleRun();
        return 0;
    }
    if (message == WM_KEYDOWN && wp == VK_F8 && !f8Registered)
    {
        ToggleMask();
        return 0;
    }
    if ((message == WM_MOUSEWHEEL ||
         (message == WM_KEYDOWN && (wp == VK_UP || wp == VK_DOWN || wp == VK_PRIOR || wp == VK_NEXT ||
                                    wp == VK_HOME || wp == VK_END))) &&
        running)
        Pause();
    LRESULT result = CallWindowProcW(editorProc, control, message, wp, lp);
    if (message == WM_PAINT && GetWindowTextLengthW(control) == 0)
    {
        HDC dc = GetDC(control);
        RECT bounds{};
        GetClientRect(control, &bounds);
        DrawCenteredMessage(dc, bounds,
                            reading ? T(L"Chưa có lời nhắc", L"No script yet")
                                    : T(L"Dán hoặc nhập lời nhắc", L"Paste or type your script"),
                            reading ? T(L"Về Lời nhắc để nhập nội dung · Ctrl+Enter",
                                        L"Return to Script to add content · Ctrl+Enter")
                                    : T(L"Ctrl+V để bắt đầu", L"Ctrl+V to begin"));
        ReleaseDC(control, dc);
    }
    if (message == WM_THEMECHANGED)
        RedrawWindow(control, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    return result;
}
LRESULT CALLBACK UnderlayProc(HWND h, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_PAINT)
    {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(h, &paint);
        RECT rect{};
        GetClientRect(h, &rect);
        HBRUSH purple = CreateSolidBrush(RGB(124, 58, 237));
        FillRect(dc, &rect, purple);
        DeleteObject(purple);
        EndPaint(h, &paint);
        return 0;
    }
    return DefWindowProcW(h, message, wp, lp);
}
POINT ScrollPosition()
{
    POINT value{};
    SendMessageW(editor, EM_GETSCROLLPOS, 0, (LPARAM)&value);
    return value;
}
void Capture(const wchar_t *name)
{
    RECT rect{};
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    GetWindowRect(window, &rect);
    if (qaVisual)
    {
        RECT visibleFrame{};
        if (SUCCEEDED(DwmGetWindowAttribute(window, DWMWA_EXTENDED_FRAME_BOUNDS, &visibleFrame,
                                            sizeof(visibleFrame))))
            rect = visibleFrame;
    }
    DwmFlush();
    qaCaptureOk = CaptureRegionBmp(qaDir + L"\\" + name, rect) && qaCaptureOk;
}
bool CompactFits()
{
    RECT client{};
    GetClientRect(window, &client);
    std::vector<RECT> controls;
    for (HWND child : buttons)
    {
        if (!IsWindowVisible(child))
            continue;
        RECT rect{};
        GetWindowRect(child, &rect);
        MapWindowPoints(nullptr, window, (POINT *)&rect, 2);
        if (rect.left < 0 || rect.top < 0 || rect.right > client.right || rect.bottom > client.bottom)
            return false;
        for (const RECT &previous : controls)
        {
            RECT overlap{};
            if (IntersectRect(&overlap, &rect, &previous))
                return false;
        }
        controls.push_back(rect);
    }
    if (page != Page::Content && !Blocked()) return true;
    RECT rect{};
    GetWindowRect(IsImage() ? imageViewer : editor, &rect);
    MapWindowPoints(nullptr, window, (POINT *)&rect, 2);
    for (const RECT &control : controls)
    {
        RECT overlap{};
        if (IntersectRect(&overlap, &rect, &control) && !focusMode)
            return false;
    }
    return rect.left >= 0 && rect.right <= client.right && rect.top >= 0 &&
           rect.bottom <= client.bottom - S(focusMode ? 0 : 32) && rect.bottom - rect.top >= S(80);
}
void WriteQa()
{
    DWORD actual = 0;
    GetWindowDisplayAffinity(window, &actual);
    auto boolean = [](bool value) -> std::wstring { return value ? L"true" : L"false"; };
    std::wstring report =
        L"{\n  \"applicationVersion\": \"0.3.0\",\n  \"windowsBuild\": " + std::to_wstring(WindowsBuild()) +
        L",\n  \"displayCloned\": " + boolean(cloned) + L",\n  \"topologyKnown\": " + boolean(topologyKnown) +
        L",\n  \"privacyApplied\": " + boolean(privacy.applied) +
        L",\n  \"requestedAffinity\": 17,\n  \"actualAffinity\": " + std::to_wstring(actual) +
        L",\n  \"privacyError\": " + std::to_wstring(privacy.error) +
        L",\n  \"remoteReceiverVerified\": null,\n  \"visualMode\": " + boolean(qaVisual) +
        L",\n  \"captureFilesWritten\": " + boolean(qaCaptureOk) +
        L",\n  \"notesPersistenceTouched\": false,\n  \"readingModeEntered\": " + boolean(qaReading) +
        L",\n  \"editorReadOnly\": " + boolean(qaReadOnly) + L",\n  \"fontControlsApplied\": " +
        boolean(qaFont) + L",\n  \"scrollAdvanced\": " + boolean(qaScrolled) + L",\n  \"pauseStable\": " +
        boolean(qaPaused) + L",\n  \"maskApplied\": " + boolean(qaMasked) + L",\n  \"compactLayoutFits\": " +
        boolean(qaCompact) + L",\n  \"contentUnchanged\": " + boolean(qaUnchanged) +
        L",\n  \"editModeReturned\": " + boolean(qaEditReturned) + L",\n  \"languageToggleApplied\": " +
        boolean(qaLanguage) + L",\n  \"scrollStartY\": " + std::to_wstring(qaStartY) +
        L",\n  \"scrollEndY\": " + std::to_wstring(qaEndY) + L",\n  \"polishLayoutMatrixFits\": " +
        boolean(qaLayoutMatrix) + L",\n  \"hoverTrackingApplied\": " + boolean(qaHover) +
        L",\n  \"emptyScriptStayedEmpty\": " + boolean(qaEmpty) + L",\n  \"nativeScrollbarStillScrolls\": " +
        boolean(qaNativeScrollbar) +
        L",\n  \"focusHidesChrome\": " + boolean(qaFocus) +
        L",\n  \"homeControlsWork\": " + boolean(qaHome) +
        L",\n  \"settingsControlsWork\": " + boolean(qaSettings) +
        L",\n  \"codeFilePreview\": " + boolean(qaCodeFile) +
        L",\n  \"imageFilePreview\": " + boolean(qaImageFile) +
        L",\n  \"filePreviewPreservesScript\": " + boolean(qaPreviewKeepsScript) +
        L",\n  \"dropFileHandlerWorks\": " + boolean(qaDropped) +
        L",\n  \"imageMaskHidesCanvas\": " + boolean(qaImageMask) +
        L",\n  \"imageZoomAndPanWork\": " + boolean(qaImagePan) +
        L",\n  \"focusRestoreButtonWorks\": " + boolean(qaFocusRestored) +
        L",\n  \"dpiMatrixMethod\": \"Simulated logical scaling at 96, 120 and 144 DPI; not a physical "
        L"monitor test\"\n}\n";
    std::string bytes = Utf8(report);
    qaPass = WriteBytes(qaDir + L"\\diagnostics.json", bytes.data(), (DWORD)bytes.size()) && qaPass;
    if (qaStorage)
    {
        report = L"{\n  \"dpapiRoundTripPassed\": " + boolean(qaRoundtrip) +
                 L",\n  \"plaintextMarkerAbsent\": " + boolean(qaPlaintextAbsent) +
                 L",\n  \"autosavePassed\": " + boolean(qaAutosave) +
                 L",\n  \"notesExistingUntouched\": true\n}\n";
        bytes = Utf8(report);
        qaPass = WriteBytes(qaDir + L"\\storage-check.json", bytes.data(), (DWORD)bytes.size()) && qaPass;
    }
}
void FinishQa()
{
    Pause();
    SetWindowDisplayAffinity(window, 0x11);
    qaPass = qaPass && qaCaptureOk && privacy.applied && topologyKnown && !cloned;
    if (qaVisual)
        qaPass = qaPass && qaReading && qaReadOnly && qaFont && qaScrolled && qaPaused && qaMasked &&
                 qaCompact && qaUnchanged && qaEditReturned && qaLanguage && qaLayoutMatrix && qaHover &&
                 qaEmpty && qaNativeScrollbar && qaFocus && qaHome && qaSettings && qaCodeFile &&
                 qaImageFile && qaPreviewKeepsScript && qaDropped && qaImageMask && qaImagePan &&
                 qaFocusRestored;
    if (qaStorage)
        qaPass = qaPass && qaRoundtrip && qaPlaintextAbsent && qaAutosave;
    WriteQa();
    DestroyWindow(window);
}
void QaTick()
{
    if (qaStorage)
    {
        if (qaStage++ == 0)
            return;
        std::wstring restored;
        qaRoundtrip = ReadNotes(restored) && restored == qaContent;
        auto encrypted = ReadBytes(dataDir + L"\\notes.dpapi");
        auto marker = Utf8(qaContent);
        qaPlaintextAbsent =
            !encrypted.empty() &&
            std::search(encrypted.begin(), encrypted.end(), marker.begin(), marker.end()) == encrypted.end();
        FinishQa();
        return;
    }
    if (!qaVisual)
    {
        Capture(L"excluded.bmp");
        FinishQa();
        return;
    }
    switch (qaStage++)
    {
    case 0:
        Capture(L"editing.bmp");
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(NAV_READ, BN_CLICKED), 0);
        qaReading = reading;
        qaReadOnly = (GetWindowLongPtrW(editor, GWL_STYLE) & ES_READONLY) != 0;
        break;
    case 1:
        Capture(L"visible.bmp");
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(LARGER, BN_CLICKED), 0);
        break;
    case 2: {
        CHARFORMAT2W format{};
        format.cbSize = sizeof(format);
        SendMessageW(editor, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM)&format);
        qaFont = fontPoints == 28 && format.yHeight == 560;
        Capture(L"large-text.bmp");
        qaStartY = ScrollPosition().y;
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(PLAY, BN_CLICKED), 0);
        break;
    }
    case 3:
        qaEndY = ScrollPosition().y;
        qaScrolled = running && qaEndY > qaStartY;
        Capture(L"scrolling.bmp");
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(PLAY, BN_CLICKED), 0);
        qaPausedY = ScrollPosition().y;
        break;
    case 4:
        qaPaused = !running && ScrollPosition().y == qaPausedY;
        SendMessageW(window, WM_HOTKEY, 2, 0);
        qaMasked = masked && !IsWindowVisible(editor) && !running;
        break;
    case 5:
        Capture(L"masked.bmp");
        SendMessageW(window, WM_HOTKEY, 2, 0);
        SetWindowPos(window, HWND_TOPMOST, 0, 0, S(460), S(360), SWP_NOMOVE | SWP_NOACTIVATE);
        {
            POINT origin{};
            SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&origin);
        }
        qaCompact = CompactFits();
        break;
    case 6:
        Capture(L"compact.bmp");
        qaUnchanged = TextOf(editor) == qaContent;
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(LANGUAGE, BN_CLICKED), 0);
        qaLanguage = english && GetWindowTextLengthW(GetDlgItem(window, MODE)) > 0;
        Capture(L"english.bmp");
        break;
    case 7:
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(LANGUAGE, BN_CLICKED), 0);
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(NAV_SCRIPT, BN_CLICKED), 0);
        qaEditReturned = !reading && !(GetWindowLongPtrW(editor, GWL_STYLE) & ES_READONLY);
        {
            const UINT actualDpi = dpi;
            // Check real HWND geometry across both languages, every presentation
            // state, and three logical scales. This never changes user notes.
            for (UINT scale : {96U, 120U, 144U})
            {
                dpi = scale;
                RefreshFonts();
                for (bool language : {false, true})
                {
                    english = language;
                    for (int width : {430, 460, 640, 840, 1080})
                    {
                        SetWindowPos(window, nullptr, 0, 0, S(width), S(width < 500 ? 320 : 560),
                                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                        for (int state = 0; state < 3; ++state)
                        {
                            reading = state != 0;
                            masked = state == 2;
                            SendMessageW(editor, EM_SETREADONLY, reading, 0);
                            Layout();
                            qaLayoutMatrix = CompactFits() && qaLayoutMatrix;
                        }
                        masked = false;
                        for (Page surface : {Page::Home, Page::Settings})
                        {
                            page = surface; Layout(); qaLayoutMatrix = CompactFits() && qaLayoutMatrix;
                        }
                        page = Page::Content;
                    }
                }
            }
            dpi = actualDpi;
            RefreshFonts();
            masked = false;
            english = false;
            reading = false;
            SendMessageW(editor, EM_SETREADONLY, FALSE, 0);
            SetWindowPos(window, nullptr, 0, 0, S(430), S(320), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            Layout();
            Capture(L"minimum-editing.bmp");
            reading = true;
            masked = true;
            Layout();
            Capture(L"minimum-masked.bmp");
            masked = false;
            SetWindowPos(window, nullptr, 0, 0, S(1080), S(560), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            Layout();
            POINT origin{};
            SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&origin);
            SendMessageW(editor, WM_VSCROLL, SB_LINEDOWN, 0);
            qaNativeScrollbar = ScrollPosition().y > 0;
            SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&origin);
            Capture(L"wide.bmp");
            SendMessageW(GetDlgItem(window, PLAY), WM_MOUSEMOVE, 0, MAKELPARAM(S(12), S(12)));
            qaHover = hoveredButton == GetDlgItem(window, PLAY);
            Capture(L"hover.bmp");
            SendMessageW(GetDlgItem(window, PLAY), BM_SETSTATE, TRUE, 0);
            Capture(L"pressed.bmp");
            SendMessageW(GetDlgItem(window, PLAY), BM_SETSTATE, FALSE, 0);
            SendMessageW(GetDlgItem(window, PLAY), WM_MOUSELEAVE, 0, 0);
            reading = false;
            english = true;
            SendMessageW(editor, EM_SETREADONLY, FALSE, 0);
            SetWindowPos(window, nullptr, 0, 0, S(720), S(560), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            Layout();
            Capture(L"english-editing.bmp");
            loading = true;
            SetWindowTextW(editor, L"");
            qaEmpty = GetWindowTextLengthW(editor) == 0;
            Capture(L"empty.bmp");
            SetReading(true);
            Capture(L"empty-reading.bmp");
            qaEmpty = qaEmpty && GetWindowTextLengthW(editor) == 0;
            SetReading(false);
            SetWindowTextW(editor, qaContent.c_str());
            loading = false;
            english = false;
            for (UINT scale : {120U, 144U})
            {
                dpi = scale;
                RefreshFonts();
                SetWindowPos(window, nullptr, 0, 0, S(720), S(560),
                             SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                Layout();
                Capture(scale == 120 ? L"dpi-125.bmp" : L"dpi-150.bmp");
            }
            dpi = actualDpi;
            RefreshFonts();
            Layout();
        }
        SetWindowPos(window,nullptr,0,0,S(840),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(NAV_HOME,BN_CLICKED),0);
        qaHome = page == Page::Home && IsWindowVisible(GetDlgItem(window,OPEN_DROP)) && !IsWindowVisible(editor);
        Capture(L"home.bmp");
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(HOME_SETTINGS,BN_CLICKED),0);
        qaSettings = page == Page::Settings && IsWindowVisible(fontSlider);
        int beforeSize = ActiveTextSize();
        SendMessageW(fontSlider,TBM_SETPOS,TRUE,32);
        SendMessageW(window,WM_HSCROLL,TB_THUMBPOSITION,(LPARAM)fontSlider);
        qaSettings = qaSettings && ActiveTextSize()==32;
        ActiveTextSize() = beforeSize; ApplyTextFormat();
        Capture(L"settings.bmp");
        ShowScript(); SetReading(true); ToggleFocus();
        qaFocus = focusMode && !IsWindowVisible(GetDlgItem(window,NAV_READ)) &&
                  !IsWindowVisible(GetDlgItem(window,PLAY)) && CompactFits();
        Capture(L"focus.bmp");
        SendMessageW(window,WM_COMMAND,MAKEWPARAM(FOCUS,BN_CLICKED),0);
        qaFocusRestored = !focusMode && IsWindowVisible(GetDlgItem(window,NAV_READ)) &&
                          IsWindowVisible(GetDlgItem(window,PLAY));
        {
            const std::wstring codePath = qaDir + L"\\example.cpp";
            const std::string code = "#include <iostream>\n\n// Preview only - this file is never executed.\nint main() {\n    int total = 0;\n    for (int i = 1; i <= 5; ++i) {\n        total += i;\n        std::cout << total << '\\n';\n    }\n    return 0;\n}\n";
            WriteBytes(codePath,code.data(),(DWORD)code.size());
            HGLOBAL dropped = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                          sizeof(DROPFILES)+(codePath.size()+2)*sizeof(wchar_t));
            if (dropped)
            {
                auto data = (DROPFILES *)GlobalLock(dropped);
                if (data)
                {
                    data->pFiles = sizeof(DROPFILES); data->fWide = TRUE;
                    CopyMemory((BYTE *)data+sizeof(DROPFILES),codePath.c_str(),
                               (codePath.size()+1)*sizeof(wchar_t));
                    GlobalUnlock(dropped);
                    SendMessageW(window,WM_DROPFILES,(WPARAM)dropped,0);
                    qaDropped = viewingFile && openedContent.displayName == L"example.cpp";
                }
                else GlobalFree(dropped);
            }
            qaCodeFile = qaDropped && IsCode() &&
                         (GetWindowLongPtrW(editor,GWL_STYLE)&ES_READONLY);
            CHARFORMAT2W format{}; format.cbSize=sizeof(format);
            SendMessageW(editor,EM_GETCHARFORMAT,SCF_DEFAULT,(LPARAM)&format);
            qaCodeFile = qaCodeFile && std::wstring(format.szFaceName)==L"Consolas";
            Capture(L"code-file.bmp");
            qaImageFile = OpenContentPath(qaDir + L"\\..\\qa-content\\orientation.bmp") && IsImage() &&
                          IsWindowVisible(imageViewer) && !IsWindowVisible(editor);
            AdjustFont(2);
            SendMessageW(imageViewer,WM_KEYDOWN,VK_LEFT,0);
            qaImagePan = imageZoom == 110 && panX == S(24);
            ResetFont();
            qaImagePan = qaImagePan && imageZoom == 100 && panX == 0 && panY == 0;
            ToggleMask();
            qaImageMask = Blocked() && !IsWindowVisible(imageViewer) && !IsWindowVisible(editor) &&
                          !OpenContentPath(codePath) && IsImage();
            ToggleMask();
            qaImageMask = qaImageMask && IsWindowVisible(imageViewer);
            Capture(L"image-file.bmp");
            ToggleFocus(); Capture(L"image-focus.bmp"); ToggleFocus();
            ShowScript();
            qaPreviewKeepsScript = !viewingFile && TextOf(editor)==qaContent;
            bool fileFailure = !OpenContentPath(qaDir + L"\\missing.exe");
            qaPreviewKeepsScript = qaPreviewKeepsScript && fileFailure && TextOf(editor)==qaContent;
            Capture(L"file-error.bmp");
            importError.clear(); ShowScript();
        }
        FinishQa();
        break;
    }
}
LRESULT CALLBACK Proc(HWND h, UINT message, WPARAM wp, LPARAM lp)
{
    switch (message)
    {
    case WM_CREATE: {
        window = h;
        privacy = ApplyPrivacy(h);
        topologyKnown = TryGetDisplayCloned(cloned);
        if (qaVisual)
            SetWindowDisplayAffinity(h, 0);
        const std::vector<std::pair<int, const wchar_t *>> labels = {
            {NAV_HOME, L"Chuẩn bị"}, {NAV_SCRIPT, L"Lời nhắc"}, {NAV_READ, L"Đọc"}, {NAV_SETTINGS, L"Tùy chỉnh"},
            {MODE, L"Đọc"},   {PLAY, L"Chạy"},
            {SPEED, L"Vừa"},           {MASK, L"Che · F8"}, {SMALLER, L"A-"}, {LARGER, L"A+"},
            {RESET_ZOOM, L"100%"}, {TOPMOST, L"Ghim"}, {LANGUAGE, L"EN"}, {FOCUS, L"Tập trung"},
            {OPEN_FILE, L"Mở file"}, {OPEN_DROP, L"Mở hoặc thả file"}, {HOME_SCRIPT, L"Lời nhắc"},
            {HOME_SETTINGS, L"Tùy chỉnh"}, {VIEW_FILE, L"Xem file"}};
        for (const auto &label : labels)
        {
            buttons.push_back(CreateWindowW(L"BUTTON", label.second,
                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 0, 0, h,
                                            (HMENU)(INT_PTR)label.first, instance, nullptr));
            SetWindowSubclass(buttons.back(), ButtonProc, 1, 0);
        }
        // Native tooltips preserve readable names for the automatic compact rail.
        tooltips =
            CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                            0, 0, 0, 0, h, nullptr, instance, nullptr);
        SendMessageW(tooltips, TTM_SETMAXTIPWIDTH, 0, S(300));
        for (const auto &label : labels)
        {
            TOOLINFOW tip{sizeof(tip)};
            tip.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            tip.hwnd = h;
            tip.uId = (UINT_PTR)GetDlgItem(h, label.first);
            tip.lpszText = const_cast<wchar_t *>(label.second);
            SendMessageW(tooltips, TTM_ADDTOOLW, 0, (LPARAM)&tip);
        }
        editor =
            CreateWindowExW(0, MSFTEDIT_CLASS, L"",
                            WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL,
                            0, 0, 0, 0, h, (HMENU)EDITOR, instance, nullptr);
        if (!editor)
            return -1;
        SetWindowTheme(editor, L"DarkMode_Explorer", nullptr);
        imageViewer = CreateWindowExW(0, L"QuietShareImage", L"", WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS,
                                       0, 0, 0, 0, h, nullptr, instance, nullptr);
        fontSlider = CreateWindowExW(0, TRACKBAR_CLASSW, L"Cỡ chữ / Zoom", WS_CHILD | WS_TABSTOP | TBS_NOTICKS,
                                      0, 0, 0, 0, h, (HMENU)FONT_SLIDER, instance, nullptr);
        SetWindowTheme(fontSlider, L"DarkMode_Explorer", nullptr);
        SetWindowSubclass(fontSlider, ButtonProc, 1, 0);
        DragAcceptFiles(h, TRUE);
        SendMessageW(editor, EM_SETTEXTMODE, TM_PLAINTEXT | TM_MULTICODEPAGE, 0);
        editorProc = (WNDPROC)SetWindowLongPtrW(editor, GWLP_WNDPROC, (LONG_PTR)EditorProc);
        SendMessageW(editor, EM_SETBKGNDCOLOR, 0, CARD);
        SendMessageW(editor, EM_EXLIMITTEXT, 0, 500000);
        SendMessageW(editor, EM_SETEVENTMASK, 0, ENM_CHANGE);
        loading = true;
        std::wstring content;
        if (qa)
        {
            content = L"MỞ ĐẦU\r\nChào mọi người. Hôm nay mình sẽ chia sẻ ba ý chính.\r\n\r\n";
            for (int i = 1; i <= 12; ++i)
                content += L"Ý " + std::to_wstring(i) +
                           L"\r\nNói ngắn gọn, thêm một ví dụ dễ hiểu. "
                           L"Dừng một nhịp trước khi chuyển sang phần tiếp theo.\r\n\r\n";
            if (qaStorage)
                content = L"QUIETSHARE_STORAGE_MARKER_7F31 - Nhắc chữ có dấu: tiếng Việt.";
        }
        else
            content = InitialNotes();
        SetWindowTextW(editor, content.c_str());
        loading = false;
        qaContent = TextOf(editor);
        RefreshFonts();
        ApplyTextFormat();
        page = qa ? Page::Content : Page::Home;
        SendMessageW(editor, EM_SETSEL, 0, 0);
        if (!qa)
        {
            f8Registered = RegisterHotKey(h, 2, MOD_NOREPEAT, VK_F8) != 0;
            SetTimer(h, PRIVACY_TIMER, 1200, nullptr);
        }
        else
        {
            if (!qaInteractive) SetTimer(h, QA_TIMER, 900, nullptr);
            if (qaStorage)
            {
                SetWindowTextW(editor, (content + L" Tự lưu.").c_str());
                qaContent = TextOf(editor);
            }
        }
        return 0;
    }
    case WM_SIZE:
        if (wp == SIZE_MINIMIZED)
            Pause();
        Layout();
        return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lp)->ptMinTrackSize = {S(430), S(320)};
        return 0;
    case WM_DPICHANGED: {
        dpi = HIWORD(wp);
        RefreshFonts();
        auto rect = (RECT *)lp;
        SetWindowPos(h, nullptr, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        Layout();
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(h, &paint);
        RECT client{};
        GetClientRect(h, &client);
        HDC buffer = CreateCompatibleDC(dc);
        HBITMAP bitmap = CreateCompatibleBitmap(dc, std::max(1L, client.right), std::max(1L, client.bottom));
        HGDIOBJ old = SelectObject(buffer, bitmap);
        Paint(buffer);
        BitBlt(dc, 0, 0, client.right, client.bottom, buffer, 0, 0, SRCCOPY);
        SelectObject(buffer, old);
        DeleteObject(bitmap);
        DeleteDC(buffer);
        EndPaint(h, &paint);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DRAWITEM:
        DrawButton((DRAWITEMSTRUCT *)lp);
        return TRUE;
    case WM_NOTIFY: {
        auto notification = (NMHDR *)lp;
        if (notification->hwndFrom == fontSlider && notification->code == NM_CUSTOMDRAW)
        {
            auto draw = (NMCUSTOMDRAW *)lp;
            if (draw->dwDrawStage == CDDS_PREPAINT)
            {
                RECT bounds{}; GetClientRect(fontSlider, &bounds);
                FillRect(draw->hdc, &bounds, cardBrush);
                RECT channel{}, thumb{};
                SendMessageW(fontSlider, TBM_GETCHANNELRECT, 0, (LPARAM)&channel);
                SendMessageW(fontSlider, TBM_GETTHUMBRECT, 0, (LPARAM)&thumb);
                int centerY = (bounds.top + bounds.bottom) / 2;
                int centerX = (thumb.left + thumb.right) / 2;
                channel.top = centerY - S(2); channel.bottom = centerY + S(2);
                FillRoundRect(draw->hdc, channel, DIVIDER, S(4));
                RECT filled = channel; filled.right = centerX;
                if (filled.right > filled.left)
                    FillRoundRect(draw->hdc, filled, IsWindowEnabled(fontSlider) ? ACCENT : DISABLED, S(4));
                RECT handle{centerX-S(7),centerY-S(7),centerX+S(7),centerY+S(7)};
                bool keyboardFocus = GetFocus() == fontSlider &&
                    !(SendMessageW(window,WM_QUERYUISTATE,0,0) & UISF_HIDEFOCUS);
                if (keyboardFocus)
                {
                    RECT ring = handle; InflateRect(&ring,S(3),S(3));
                    FillRoundRect(draw->hdc,ring,GREEN,S(20));
                    InflateRect(&ring,-S(2),-S(2)); FillRoundRect(draw->hdc,ring,CARD,S(16));
                }
                FillRoundRect(draw->hdc,handle,!IsWindowEnabled(fontSlider) ? DISABLED
                    : hoveredButton == fontSlider ? GREEN : TEXT,S(14));
                return CDRF_SKIPDEFAULT;
            }
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, CARD);
        SetTextColor((HDC)wp, TEXT);
        return (LRESULT)cardBrush;
    case WM_HSCROLL:
        if ((HWND)lp == fontSlider && !Blocked())
        {
            int value = (int)SendMessageW(fontSlider, TBM_GETPOS, 0, 0);
            Pause();
            if (IsImage()) { imageZoom = value; panX = panY = 0; InvalidateRect(imageViewer,nullptr,FALSE); }
            else ActiveTextSize() = value;
            ApplyTextFormat();
        }
        return 0;
    case WM_DROPFILES: {
        HDROP drop = (HDROP)wp;
        UINT count = DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
        if (count == 1)
        {
            UINT length = DragQueryFileW(drop, 0, nullptr, 0);
            std::vector<wchar_t> path(length + 1);
            DragQueryFileW(drop,0,path.data(),length+1);
            OpenContentPath(path.data());
        }
        else if (!Blocked())
        {
            importError = L"Mở một file mỗi lần. / Open one file at a time.";
            page = Page::Home; focusMode = false; Layout();
        }
        DragFinish(drop); return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == EDITOR && HIWORD(wp) == EN_CHANGE && !loading && !viewingFile)
        {
            dirty = true;
            SetTimer(h, SAVE_TIMER, 900, nullptr);
            InvalidateRect(h, nullptr, FALSE);
        }
        if (HIWORD(wp) != BN_CLICKED)
            return 0;
        switch (LOWORD(wp))
        {
        case NAV_HOME:
            Pause(); focusMode = false; page = Page::Home; Layout(); break;
        case NAV_SETTINGS:
        case HOME_SETTINGS:
            Pause(); focusMode = false; page = Page::Settings; Layout(); break;
        case NAV_SCRIPT:
        case HOME_SCRIPT:
            ShowScript();
            break;
        case NAV_READ:
            SetReading(true);
            break;
        case MODE:
            SetReading(!reading);
            break;
        case OPEN_FILE:
        case OPEN_DROP:
            ChooseFile(); break;
        case VIEW_FILE:
            ShowOpenedFile(); break;
        case FOCUS:
            ToggleFocus(); break;
        case PLAY:
            ToggleRun();
            break;
        case SMALLER:
            AdjustFont(-4);
            break;
        case LARGER:
            AdjustFont(4);
            break;
        case SPEED:
            speed = (speed + 1) % 3;
            UpdateButtons();
            break;
        case MASK:
            ToggleMask();
            break;
        case TOPMOST:
            SetTopmost(!topmost);
            break;
        case LANGUAGE:
            english = !english;
            Layout();
            break;
        case RESET_ZOOM:
            ResetFont();
            break;
        }
        return 0;
    case WM_HOTKEY:
        if (wp == 2)
            ToggleMask();
        return 0;
    case WM_DISPLAYCHANGE:
    case WM_DWMCOMPOSITIONCHANGED:
    case WM_DEVICECHANGE:
        RefreshPrivacy();
        return 0;
    case WM_TIMER:
        if (wp == SCROLL_TIMER)
            ScrollTick();
        else if (wp == SAVE_TIMER)
        {
            KillTimer(h, SAVE_TIMER);
            bool saved = SaveNotes();
            if (qaStorage)
                qaAutosave = saved;
        }
        else if (wp == PRIVACY_TIMER)
        {
            bool previousCloned = cloned, previousKnown = topologyKnown;
            DWORD actual = 0;
            bool affinityOk = GetWindowDisplayAffinity(h, &actual) && actual == 0x11;
            topologyKnown = TryGetDisplayCloned(cloned);
            if (!affinityOk || cloned != previousCloned || topologyKnown != previousKnown)
                RefreshPrivacy();
        }
        else if (wp == QA_TIMER)
            QaTick();
        return 0;
    case WM_CLOSE:
        if (!qa && dirty && !SaveNotes())
        {
            MessageBoxW(h,
                        T(L"Chưa lưu được nội dung. Hãy sao chép lời nhắc trước khi đóng cửa sổ.",
                          L"The script could not be saved. Copy it before closing."),
                        L"QuietShare", MB_OK | MB_ICONWARNING);
            return 0;
        }
        DestroyWindow(h);
        return 0;
    case WM_DESTROY:
        for (UINT_PTR timer : {PRIVACY_TIMER, SCROLL_TIMER, SAVE_TIMER, QA_TIMER})
            KillTimer(h, timer);
        if (f8Registered)
            UnregisterHotKey(h, 2);
        if (underlay)
            DestroyWindow(underlay);
        PostQuitMessage(qa && !qaPass ? 30 : 0);
        return 0;
    }
    return DefWindowProcW(h, message, wp, lp);
}
} // namespace
int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, LPWSTR, int show)
{
    instance = inst;
    int count = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &count);
    for (int i = 1; i < count; ++i)
    {
        std::wstring flag = args[i];
        if (flag == L"--qa-visible" || flag == L"--qa-excluded" || flag == L"--qa-storage" || flag == L"--qa-scrollbar")
        {
            qa = true;
            qaVisual = flag == L"--qa-visible" || flag == L"--qa-scrollbar";
            qaInteractive = flag == L"--qa-scrollbar";
            qaStorage = flag == L"--qa-storage";
            if (i + 1 < count && args[i + 1][0] != L'-')
                qaDir = args[++i];
        }
        else
        {
            LocalFree(args);
            return 2;
        }
    }
    LocalFree(args);
    if (qa)
    {
        if (qaDir.empty())
            return 2;
        CreateDirectoryW(qaDir.c_str(), nullptr);
        dataDir = qaDir + L"\\Data";
    }
    else
        dataDir = GetLocalDataDirectory();
    if (dataDir.empty())
        return 4;
    CreateDirectoryW(dataDir.c_str(), nullptr);
    HANDLE singleInstance = nullptr;
    if (!qa)
    {
        singleInstance = CreateMutexW(nullptr, FALSE, L"Local\\QuietShare.Teleprompter");
        if (!singleInstance)
            return 7;
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            HWND existing = FindWindowW(L"QuietSharePrivate", L"QuietShare - Nhắc chữ riêng");
            if (existing)
            {
                ShowWindow(existing, SW_RESTORE);
                SetForegroundWindow(existing);
            }
            CloseHandle(singleInstance);
            return 0;
        }
    }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    dpi = GetDpiForSystem();
    HMODULE richEdit = LoadLibraryExW(L"Msftedit.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!richEdit)
        return 5;
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&controls);
    bgBrush = CreateSolidBrush(BG);
    cardBrush = CreateSolidBrush(CARD);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = Proc;
    wc.hInstance = inst;
    wc.lpszClassName = L"QuietSharePrivate";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(101));
    wc.hIconSm = reinterpret_cast<HICON>(LoadImageW(instance, MAKEINTRESOURCEW(101), IMAGE_ICON,
                                                    GetSystemMetrics(SM_CXSMICON),
                                                    GetSystemMetrics(SM_CYSMICON), LR_SHARED));
    RegisterClassExW(&wc);
    WNDCLASSEXW imageClass = wc;
    imageClass.lpfnWndProc = ImageProc;
    imageClass.lpszClassName = L"QuietShareImage";
    imageClass.hCursor = LoadCursorW(nullptr, IDC_SIZEALL);
    RegisterClassExW(&imageClass);
    RECT start{S(72), S(66), S(72 + 720), S(66 + 560)};
    if (!qa)
    {
        RECT work{};
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0))
        {
            int width = std::min<int>(S(720), work.right - work.left - 24);
            int height = std::min<int>(S(560), work.bottom - work.top - 24);
            start.left = work.left + (work.right - work.left - width) / 2;
            start.top = work.top + (work.bottom - work.top - height) / 3;
            start.right = start.left + width;
            start.bottom = start.top + height;
        }
    }
    if (qa)
    {
        wc.lpfnWndProc = UnderlayProc;
        wc.lpszClassName = L"QuietShareQaUnderlay";
        RegisterClassExW(&wc);
        underlay = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName, L"", WS_POPUP,
                                   start.left, start.top, start.right - start.left, start.bottom - start.top,
                                   nullptr, nullptr, inst, nullptr);
        ShowWindow(underlay, SW_SHOWNOACTIVATE);
        UpdateWindow(underlay);
    }
    window =
        CreateWindowExW(WS_EX_TOPMOST, L"QuietSharePrivate", L"QuietShare",
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, start.left, start.top,
                        start.right - start.left, start.bottom - start.top, nullptr, nullptr, inst, nullptr);
    if (!window)
        return 3;
    dpi = GetDpiForWindow(window);
    RefreshFonts();
    BOOL dark = TRUE;
    DwmSetWindowAttribute(window, 20, &dark, sizeof(dark));
    DWORD rounded = 2;
    DwmSetWindowAttribute(window, 33, &rounded, sizeof(rounded));
    COLORREF border = DIVIDER;
    DwmSetWindowAttribute(window, 34, &border, sizeof(border));
    ShowWindow(window, qa ? SW_SHOWNOACTIVATE : show);
    if (qa)
    {
        // The test launcher starts hidden; this second call displays only our sample UI.
        ShowWindow(underlay, SW_SHOWNOACTIVATE);
        ShowWindow(window, SW_SHOWNOACTIVATE);
        SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    Layout();
    UpdateWindow(window);
    if (!qa && !Blocked())
        SetFocus(GetDlgItem(window, OPEN_DROP));
    MSG message{};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0)
    {
        if (message.message == WM_KEYDOWN && message.wParam == VK_F11)
        { ToggleFocus(); continue; }
        if (message.message == WM_KEYDOWN && message.wParam == VK_ESCAPE && focusMode)
        { ToggleFocus(); continue; }
        if (message.message == WM_KEYDOWN && message.wParam == VK_TAB)
            SendMessageW(window, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
        if (message.message == WM_KEYDOWN && message.wParam == VK_F8 && !f8Registered)
        {
            ToggleMask();
            continue;
        }
        if (message.message == WM_KEYDOWN && message.wParam == VK_RETURN &&
            (GetKeyState(VK_CONTROL) & 0x8000))
        {
            SetReading(!reading);
            continue;
        }
        if (!IsDialogMessageW(window, &message))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    DeleteObject(uiFont);
    DeleteObject(titleFont);
    DeleteObject(brandFont);
    DeleteObject(metaFont);
    DeleteObject(bgBrush);
    DeleteObject(cardBrush);
    FreeLibrary(richEdit);
    if (singleInstance)
        CloseHandle(singleInstance);
    return result < 0 ? 6 : (int)message.wParam;
}
