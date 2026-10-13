#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX
#include <windows.h>
#include "Privacy.h"
#include <algorithm>
#include <commctrl.h>
#include <dwmapi.h>
#include <richedit.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <wincrypt.h>

using namespace quietshare;
namespace
{
constexpr COLORREF BG = RGB(15, 23, 42), CARD = RGB(25, 33, 52), TEXT = RGB(241, 245, 249),
                   MUTED = RGB(170, 184, 206), GREEN = RGB(74, 222, 128), AMBER = RGB(253, 203, 105);
constexpr int MODE = 100, PLAY = 101, SMALLER = 102, LARGER = 103, SPEED = 104, MASK = 105, TOPMOST = 106,
              LANGUAGE = 107, RESET_ZOOM = 108, EDITOR = 110;
constexpr UINT_PTR PRIVACY_TIMER = 1, SCROLL_TIMER = 2, SAVE_TIMER = 3, QA_TIMER = 4;
HINSTANCE instance;
HWND window, editor, underlay;
HFONT uiFont, titleFont;
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
RECT fontLabelRect{};
std::vector<HWND> buttons;
WNDPROC editorProc;
bool qa = false, qaVisual = false, qaStorage = false, qaCaptureOk = true, qaPass = true;
bool qaReading = false, qaReadOnly = false, qaFont = false, qaScrolled = false, qaPaused = false,
     qaMasked = false, qaCompact = false, qaUnchanged = false, qaEditReturned = false, qaLanguage = false;
bool qaRoundtrip = false, qaPlaintextAbsent = false, qaAutosave = false;
int qaStage = 0;
LONG qaStartY = 0, qaEndY = 0, qaPausedY = 0;
std::wstring qaDir, qaContent;

const wchar_t *T(const wchar_t *vietnamese, const wchar_t *englishText)
{
    return english ? englishText : vietnamese;
}

void ApplyTextFormat();
void Pause();

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
    std::string plain = Utf8(TextOf(editor));
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
    fontPoints = std::clamp(fontPoints + delta, 12, 72);
    ApplyTextFormat();
}
void ResetFont()
{
    if (Blocked())
        return;
    Pause();
    fontPoints = 24;
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
void UpdateButtons()
{
    SetWindowTextW(window, T(L"QuietShare - Nhắc chữ riêng", L"QuietShare - Private teleprompter"));
    SetWindowTextW(GetDlgItem(window, MODE), T(reading ? L"Sửa lời" : L"Đọc", reading ? L"Edit" : L"Read"));
    SetWindowTextW(GetDlgItem(window, PLAY), T(running ? L"Dừng" : L"Chạy", running ? L"Stop" : L"Start"));
    const wchar_t *speeds[] = {T(L"Chậm", L"Slow"), T(L"Vừa", L"Medium"), T(L"Nhanh", L"Fast")};
    SetWindowTextW(GetDlgItem(window, SPEED), speeds[speed]);
    SetWindowTextW(GetDlgItem(window, MASK), masked ? T(L"Mở lại", L"Show") : T(L"Che · F8", L"Hide · F8"));
    SetWindowTextW(GetDlgItem(window, TOPMOST), topmost ? T(L"Ghim", L"Pin") : T(L"Thả", L"Free"));
    SetWindowTextW(GetDlgItem(window, LANGUAGE), english ? L"VI" : L"EN");
    SetWindowTextW(GetDlgItem(window, RESET_ZOOM), L"100%");
    for (int id : {MODE, PLAY, SMALLER, LARGER, SPEED, RESET_ZOOM})
        EnableWindow(GetDlgItem(window, id), !Blocked());
    EnableWindow(GetDlgItem(window, SMALLER), !Blocked() && fontPoints > 12);
    EnableWindow(GetDlgItem(window, LARGER), !Blocked() && fontPoints < 72);
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
    if (!window || !editor)
        return;
    RECT client{};
    GetClientRect(window, &client);
    int width = MulDiv(client.right, 96, dpi), height = MulDiv(client.bottom, 96, dpi);
    bool focusMode = reading && !Blocked();
    bool wrapToolbar = width < 620;
    int x = 18, y = focusMode ? 14 : 80;
    auto move = [&](int id, int left, int top, int w, int h = 34) {
        MoveWindow(GetDlgItem(window, id), S(left), S(top), S(w), S(h), TRUE);
    };
    int modeWidth = focusMode ? 70 : 80, playWidth = 62, fontWidth = focusMode ? 32 : 40,
        labelWidth = focusMode ? 24 : 36, speedWidth = focusMode ? 72 : 76, maskWidth = focusMode ? 84 : 90,
        resetWidth = focusMode ? 44 : 48, pinWidth = focusMode ? 60 : 70, languageWidth = focusMode ? 36 : 44;
    move(MODE, x, y, modeWidth);
    x += modeWidth + 8;
    if (reading)
    {
        move(PLAY, x, y, playWidth);
        x += playWidth + 8;
    }
    if (reading)
    {
        move(SPEED, x, y, speedWidth);
        x += speedWidth + 8;
    }
    if (wrapToolbar && focusMode)
    {
        move(MASK, x, y, maskWidth);
        x = 18;
        y += 42;
        move(SMALLER, x, y, fontWidth);
        x += fontWidth;
        fontLabelRect = {S(x), S(y), S(x + labelWidth), S(y + 34)};
        x += labelWidth;
        move(LARGER, x, y, fontWidth);
        x += fontWidth + 8;
        move(RESET_ZOOM, x, y, resetWidth);
        x += resetWidth + 8;
        move(TOPMOST, x, y, pinWidth);
        x += pinWidth + 8;
        move(LANGUAGE, x, y, languageWidth);
    }
    else
    {
        move(MASK, x, y, maskWidth);
        x += maskWidth + 8;
        move(SMALLER, x, y, fontWidth);
        x += fontWidth;
        fontLabelRect = {S(x), S(y), S(x + labelWidth), S(y + 34)};
        x += labelWidth;
        move(LARGER, x, y, fontWidth);
        x += fontWidth + 8;
        move(RESET_ZOOM, x, y, resetWidth);
        x += resetWidth + 8;
        move(TOPMOST, x, y, pinWidth);
        x += pinWidth + 8;
        move(LANGUAGE, x, y, languageWidth);
    }
    ShowWindow(GetDlgItem(window, PLAY), reading ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(window, SPEED), reading ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(window, MASK), SW_SHOW);
    ShowWindow(GetDlgItem(window, TOPMOST), SW_SHOW);
    ShowWindow(GetDlgItem(window, LANGUAGE), SW_SHOW);
    ShowWindow(GetDlgItem(window, RESET_ZOOM), SW_SHOW);
    int top = y + 48;
    MoveWindow(editor, S(18), S(top), S(std::max(1, width - 36)), S(std::max(1, height - top - 42)), TRUE);
    RECT textRect{};
    GetClientRect(editor, &textRect);
    textRect.left += S(20);
    textRect.right -= S(20);
    textRect.top += S(12);
    textRect.bottom -= S(12);
    SendMessageW(editor, EM_SETRECT, 0, (LPARAM)&textRect);
    // RichEdit can retain pixels outside its new formatting rectangle after resize.
    RedrawWindow(editor, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
    ShowWindow(editor, Blocked() ? SW_HIDE : SW_SHOW);
    UpdateButtons();
}
void ApplyTextFormat()
{
    loading = true;
    CHARFORMAT2W cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_BOLD;
    cf.yHeight = fontPoints * 20;
    cf.crTextColor = TEXT;
    wcscpy_s(cf.szFaceName, L"Segoe UI");
    SendMessageW(editor, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    CHARRANGE selection{};
    SendMessageW(editor, EM_EXGETSEL, 0, (LPARAM)&selection);
    SendMessageW(editor, EM_SETSEL, 0, -1);
    PARAFORMAT2 pf{};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_LINESPACING | PFM_SPACEAFTER;
    pf.bLineSpacingRule = 5;
    pf.dyLineSpacing = 26;
    pf.dySpaceAfter = 100;
    SendMessageW(editor, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
    SendMessageW(editor, EM_EXSETSEL, 0, (LPARAM)&selection);
    loading = false;
    UpdateButtons();
}
void RefreshFonts()
{
    HFONT oldUi = uiFont, oldTitle = titleFont;
    uiFont = CreateFontW(-S(15), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                         CLEARTYPE_QUALITY, 0, L"Segoe UI");
    titleFont = CreateFontW(-S(22), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                            CLEARTYPE_QUALITY, 0, L"Segoe UI");
    for (HWND button : buttons)
        SendMessageW(button, WM_SETFONT, (WPARAM)uiFont, TRUE);
    if (oldUi)
        DeleteObject(oldUi);
    if (oldTitle)
        DeleteObject(oldTitle);
}
void SetReading(bool value)
{
    if (Blocked())
        return;
    Pause();
    reading = value;
    SendMessageW(editor, EM_SETREADONLY, reading, 0);
    SendMessageW(editor, EM_SETSEL, 0, 0);
    POINT origin{};
    SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&origin);
    if (!reading)
        SetFocus(editor);
    else
        SetFocus(GetDlgItem(window, PLAY));
    Layout();
}
void ToggleRun()
{
    if (!reading || Blocked())
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
    Layout();
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void RefreshPrivacy()
{
    ShowWindow(editor, SW_HIDE);
    privacy = ApplyPrivacy(window);
    topologyKnown = TryGetDisplayCloned(cloned);
    if (Blocked())
        Pause();
    Layout();
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void Paint(HDC dc)
{
    RECT client{};
    GetClientRect(window, &client);
    FillRect(dc, &client, bgBrush);
    RECT title{S(18), S(14), client.right - S(18), S(42)};
    RECT status{S(18), S(46), client.right - S(18), S(67)};
    std::wstring message =
        storageBlocked     ? T(L"Không đọc hoặc sao lưu được lời cũ - nội dung đang khóa",
                               L"The saved script could not be read - content is locked")
        : !topologyKnown   ? T(L"Chưa xác định được màn hình - nội dung đang khóa",
                               L"Display mode is unknown - content is locked")
        : cloned           ? T(L"Duplicate - chuyển Win+P sang Extend để đọc riêng",
                               L"Duplicate - switch Win+P to Extend for private reading")
        : !privacy.applied ? T(L"Không bật được loại trừ capture - nội dung đang khóa",
                               L"Capture exclusion is unavailable - content is locked")
                           : T(L"Đã bật loại trừ cửa sổ khỏi capture", L"Window capture exclusion is active");
    if (!reading || Blocked())
    {
        DrawTextAt(dc, T(L"Nhắc chữ riêng", L"Private teleprompter"), title, TEXT, titleFont);
        DrawTextAt(dc, message, status,
                   storageBlocked || !topologyKnown || cloned || !privacy.applied ? AMBER : GREEN, uiFont,
                   DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    DrawTextAt(dc, std::to_wstring(fontPoints), fontLabelRect, MUTED, uiFont,
               DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (Blocked())
    {
        RECT card{};
        GetWindowRect(editor, &card);
        MapWindowPoints(nullptr, window, (POINT *)&card, 2);
        FillRect(dc, &card, cardBrush);
        InflateRect(&card, -S(28), -S(20));
        std::wstring cover =
            storageBlocked ? T(L"Bản lưu cũ được giữ nguyên. Đóng app và kiểm tra quyền đọc tệp lời nhắc.",
                               L"The old script was preserved. Close the app and check file access.")
            : masked         ? T(L"Đã che nội dung\nBấm F8 hoặc Mở lại để đọc tiếp.",
                                 L"Content hidden\nPress F8 or Show to continue.")
            : cloned         ? T(L"Dùng Extend để giữ lời nhắc trên màn hình của bạn.",
                                 L"Use Extend to keep the script on your private display.")
            : !topologyKnown ? T(L"Nội dung sẽ mở khi xác định được cấu hình màn hình.",
                                 L"Content will open when the display layout is known.")
                             : T(L"Windows chưa áp dụng được cơ chế loại trừ capture.",
                                 L"Windows could not apply capture exclusion.");
        DrawTextAt(dc, cover, card, MUTED, uiFont, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
    }
    RECT footer{S(18), client.bottom - S(32), client.right - S(18), client.bottom - S(8)};
    std::wstring footerText =
        saveFailed ? T(L"Không lưu được lời nhắc trên máy", L"Could not save the script")
        : savedNoteRecovered && !dirty && !reading ? T(L"Không đọc được lời cũ · Đã giữ bản dự phòng",
                                                       L"Old script unreadable · recovery copy preserved")
        : dirty    ? T(L"Đang lưu...", L"Saving...")
        : !reading ? T(L"Sửa nội dung · Tự lưu trên máy", L"Edit the script · Saved locally")
        : running  ? T(L"Đang cuộn · F8 che nhanh", L"Scrolling · F8 hides quickly")
                   : T(L"Sẵn sàng đọc · F8 che nhanh", L"Ready to read · F8 hides quickly");
    DrawTextAt(dc, footerText, footer, saveFailed ? AMBER : (reading && !Blocked() ? GREEN : MUTED), uiFont);
}
void DrawButton(DRAWITEMSTRUCT *item)
{
    bool disabled = (item->itemState & ODS_DISABLED) != 0;
    bool primary = item->CtlID == MODE || (item->CtlID == PLAY && running);
    COLORREF color = (item->itemState & ODS_SELECTED) ? RGB(49, 65, 85)
                     : primary && !disabled           ? RGB(36, 78, 99)
                                                      : CARD;
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(item->hDC, &item->rcItem, brush);
    DeleteObject(brush);
    DrawTextAt(item->hDC, TextOf(item->hwndItem), item->rcItem, disabled ? RGB(114, 130, 151) : TEXT, uiFont,
               DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (item->itemState & ODS_FOCUS)
    {
        RECT focus = item->rcItem;
        InflateRect(&focus, -3, -3);
        DrawFocusRect(item->hDC, &focus);
    }
}
LRESULT CALLBACK EditorProc(HWND control, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_PASTE && !reading)
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
    return CallWindowProcW(editorProc, control, message, wp, lp);
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
    GetWindowRect(window, &rect);
    DwmFlush();
    qaCaptureOk = CaptureRegionBmp(qaDir + L"\\" + name, rect) && qaCaptureOk;
}
bool CompactFits()
{
    RECT client{};
    GetClientRect(window, &client);
    for (HWND child : buttons)
    {
        if (!IsWindowVisible(child))
            continue;
        RECT rect{};
        GetWindowRect(child, &rect);
        MapWindowPoints(nullptr, window, (POINT *)&rect, 2);
        if (rect.left < 0 || rect.top < 0 || rect.right > client.right || rect.bottom > client.bottom)
            return false;
    }
    RECT rect{};
    GetWindowRect(editor, &rect);
    MapWindowPoints(nullptr, window, (POINT *)&rect, 2);
    return rect.left >= 0 && rect.right <= client.right && rect.top >= 0 && rect.bottom <= client.bottom &&
           rect.bottom - rect.top >= S(80);
}
void WriteQa()
{
    DWORD actual = 0;
    GetWindowDisplayAffinity(window, &actual);
    auto boolean = [](bool value) -> std::wstring { return value ? L"true" : L"false"; };
    std::wstring report =
        L"{\n  \"applicationVersion\": \"0.2.0\",\n  \"windowsBuild\": " + std::to_wstring(WindowsBuild()) +
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
        L",\n  \"scrollEndY\": " + std::to_wstring(qaEndY) + L"\n}\n";
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
                 qaCompact && qaUnchanged && qaEditReturned && qaLanguage;
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
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(MODE, BN_CLICKED), 0);
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
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(MODE, BN_CLICKED), 0);
        qaEditReturned = !reading && !(GetWindowLongPtrW(editor, GWL_STYLE) & ES_READONLY);
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
            {MODE, L"Đọc"},      {PLAY, L"Chạy"},    {SMALLER, L"A-"},  {LARGER, L"A+"},      {SPEED, L"Vừa"},
            {MASK, L"Che · F8"}, {TOPMOST, L"Ghim"}, {LANGUAGE, L"EN"}, {RESET_ZOOM, L"100%"}};
        for (const auto &label : labels)
            buttons.push_back(CreateWindowW(L"BUTTON", label.second,
                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, 0, 0, 0, 0, h,
                                            (HMENU)(INT_PTR)label.first, instance, nullptr));
        editor =
            CreateWindowExW(0, MSFTEDIT_CLASS, L"",
                            WS_CHILD | WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL,
                            0, 0, 0, 0, h, (HMENU)EDITOR, instance, nullptr);
        if (!editor)
            return -1;
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
        SendMessageW(editor, EM_SETSEL, 0, 0);
        if (!qa)
        {
            f8Registered = RegisterHotKey(h, 2, MOD_NOREPEAT, VK_F8) != 0;
            SetTimer(h, PRIVACY_TIMER, 1200, nullptr);
        }
        else
        {
            SetTimer(h, QA_TIMER, 900, nullptr);
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
        Paint(dc);
        EndPaint(h, &paint);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DRAWITEM:
        DrawButton((DRAWITEMSTRUCT *)lp);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wp) == EDITOR && HIWORD(wp) == EN_CHANGE && !loading)
        {
            dirty = true;
            SetTimer(h, SAVE_TIMER, 900, nullptr);
            InvalidateRect(h, nullptr, FALSE);
        }
        if (HIWORD(wp) != BN_CLICKED)
            return 0;
        switch (LOWORD(wp))
        {
        case MODE:
            SetReading(!reading);
            break;
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
        if (flag == L"--qa-visible" || flag == L"--qa-excluded" || flag == L"--qa-storage")
        {
            qa = true;
            qaVisual = flag == L"--qa-visible";
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
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    bgBrush = CreateSolidBrush(BG);
    cardBrush = CreateSolidBrush(CARD);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = Proc;
    wc.hInstance = inst;
    wc.lpszClassName = L"QuietSharePrivate";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);
    RECT start{S(72), S(66), S(72 + 640), S(66 + 540)};
    if (!qa)
    {
        RECT work{};
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0))
        {
            int width = std::min<int>(S(640), work.right - work.left - 24);
            int height = std::min<int>(S(540), work.bottom - work.top - 24);
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
        CreateWindowExW(WS_EX_TOPMOST, L"QuietSharePrivate", L"QuietShare - Nhắc chữ riêng",
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
    COLORREF border = RGB(71, 85, 105);
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
        SetFocus(editor);
    MSG message{};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0)
    {
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
    DeleteObject(bgBrush);
    DeleteObject(cardBrush);
    FreeLibrary(richEdit);
    if (singleInstance)
        CloseHandle(singleInstance);
    return result < 0 ? 6 : (int)message.wParam;
}
