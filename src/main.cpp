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
#include "Documents.h"
#include "DocumentBar.h"
#include "Viewers.h"
#include "DocumentHost.h"
#include "Mirror.h"
#include "ToolMenu.h"
#include "LocalPicker.h"
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
constexpr COLORREF BG = RGB(30, 30, 30), SIDEBAR = RGB(51, 51, 51), CARD = RGB(37, 37, 38),
                   ELEVATED = RGB(60, 60, 60), HOVER = RGB(70, 70, 70), ACCENT = RGB(0, 122, 204),
                   ACCENT_DARK = RGB(4, 57, 94), TEXT = RGB(242, 245, 247), MUTED = RGB(170, 170, 170),
                   DIVIDER = RGB(52, 59, 66), GREEN = RGB(110, 231, 160), AMBER = RGB(245, 184, 82),
                   DISABLED = RGB(105, 112, 121);
constexpr int MODE = 100, PLAY = 101, SMALLER = 102, LARGER = 103, SPEED = 104, MASK = 105, TOPMOST = 106,
              LANGUAGE = 107, RESET_ZOOM = 108, EDITOR = 110, NAV_SCRIPT = 120, NAV_READ = 121,
              NAV_HOME = 122, NAV_SETTINGS = 123, OPEN_FILE = 124, FOCUS = 125, OPEN_DROP = 126,
              HOME_SCRIPT = 127, HOME_SETTINGS = 128, VIEW_FILE = 129, FONT_SLIDER = 130, OPEN_FOLDER = 131, OVERFLOW = 132,
              FIND = 133, FIND_NEXT = 134, FIND_PREV = 135, FIND_CLOSE = 136,
              PDF_PREV = 137, PDF_NEXT = 138, NAV_PRESENTATION = 139,
              VERIFY_PRIVACY = 140, MIRROR_START = 141, MIRROR_STOP = 142, POINTER = 143,
              RELOAD_FILE = 144, KEEP_FILE = 145, WRAP = 146, FIT_WIDTH = 147,
              CLOSE_TAB = 148, CLOSE_OTHERS = 149, CLOSE_FILES = 150, SHOW_TREE = 151,
              INCLUDE_IGNORED = 152, CLOSE_APPLICATION = 153, SOURCE_MONITOR = 160, DEST_MONITOR = 161, FIND_INPUT = 162, SIZE_INPUT = 163;
constexpr int NORMAL_SIDEBAR = 60, COMPACT_SIDEBAR = 60, CONTROL_HEIGHT = 36, CONTROL_GAP = 8;
constexpr UINT_PTR PRIVACY_TIMER = 1, SCROLL_TIMER = 2, SAVE_TIMER = 3, QA_TIMER = 4;
HINSTANCE instance;
HWND window, editor, underlay, tooltips, hoveredButton, imageViewer, fontSlider, filePicker,
     findInput, sourceMonitor, destinationMonitor, sizeInput;
bool updatingSize=false,sliderTracking=false;
DocumentId sizeInputDocument=0;
int sliderPreview=-1;
std::wstring pickerFolder;
std::wstring sizeError;
TabManager documents;
DocumentBar documentBar;
ProjectTree projectTree;
DocumentHost documentHost;
CodeViewer &codeViewer=documentHost.code;
PdfViewer &pdfViewer=documentHost.pdf;
std::unique_ptr<MirrorController> mirror;
std::vector<MonitorInfo> monitors;
bool findVisible = false, treeVisible = true, projectDrawer = false, mirrorPointer = true, verifying = false, renderedSafe = false;
DocumentId reloadPending = 0;
std::wstring viewerError, findMessage;
constexpr UINT VerifyPrivacyMessage = WM_APP + 12;
HFONT uiFont, titleFont, brandFont, metaFont;
HBRUSH bgBrush, cardBrush;
UINT dpi = 96;
PrivacyResult privacy;
bool topologyKnown = false, cloned = true, masked = false, reading = false, running = false, loading = false,
     dirty = false, saveFailed = false, f8Registered = false, savedNoteRecovered = false,
     storageBlocked = false, topmost = true, english = false, recallRegistered = false;
int fontPoints = 24, speed = 1;
double scrollRemainder = 0;
ULONGLONG lastScrollTick = 0;
std::wstring dataDir;
enum class Page { Home, Content, Settings, Presentation };
Page page = Page::Content;
LoadedContent &ActiveContent() { return documents.Active().content; }
bool viewingFile = false, focusMode = false, scriptReading = false;
int codePoints = 14, imageZoom = 100, panX = 0, panY = 0;
std::wstring scriptDraft, importError;
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
bool qaV04 = true;
int qaStage = 0;
LONG qaStartY = 0, qaEndY = 0, qaPausedY = 0;
std::wstring qaDir, qaContent;

const wchar_t *T(const wchar_t *vietnamese, const wchar_t *englishText)
{
    return english ? englishText : vietnamese;
}

void ApplyTextFormat();
void SyncSizeInput(bool force=false);
void CommitSizeInput();
void Pause();
void Layout();
void SetReading(bool value);
void ShowScript();
void SaveActiveState();
void ActivateDocument(DocumentId id);
void CloseDocument(DocumentId id);
void ChooseFolder();
void OverflowMenu();
void VerifyPrivacy();
void UpdateDocumentBar();
void RunFind(bool forward);
std::wstring SelectedMonitor(HWND combo);
bool OpenContentPath(const std::wstring &path);
bool IsImage() { return viewingFile && ActiveContent().kind == ContentKind::Image; }
bool IsCode() { return viewingFile && (ActiveContent().kind == ContentKind::Code || documents.Active().language==L"md" || documents.Active().language==L"markdown"); }
bool IsPdf() { return viewingFile && ActiveContent().kind == ContentKind::Pdf; }
HWND ContentWindow() { return projectDrawer?projectTree.Handle():IsImage()?imageViewer:IsCode()?codeViewer.Window():IsPdf()?pdfViewer.Window():editor; }
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
bool IsPrivateContentSafe()
{
    // QA uses only synthetic fixtures; production has no privacy override.
    return !masked && !cloned && topologyKnown && privacy.applied && !storageBlocked &&
           !verifying && (qa || (mirror && mirror->IsVerified() && !mirror->Busy()));
}
bool Blocked() { return !IsPrivateContentSafe(); }
void SetTopmost(bool value)
{
    topmost = value;
    SetWindowPos(window, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(window, nullptr, FALSE);
}
void RecallWindow()
{
    // Conceal all readers and metadata before restoring a minimized/covered HWND.
    // A recall never changes pinning, foreground focus or the explicit F8 mask.
    topologyKnown=false;
    documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);
    Layout();
    DWORD affinity=0;
    if(!GetWindowDisplayAffinity(window,&affinity)||affinity!=0x11) {
        privacy=ApplyPrivacy(window);
        if(mirror)mirror->Invalidate();
    }
    if(IsIconic(window)||!IsWindowVisible(window))ShowWindow(window,SW_SHOWNOACTIVATE);
    topologyKnown=TryGetDisplayCloned(cloned);
    if(mirror)mirror->Tick();
    if(mirror&&!mirror->IsVerified()&&!masked)PostMessageW(window,VerifyPrivacyMessage,0,0);
    Layout();
    SetWindowPos(window,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
}
void AdjustFont(int delta)
{
    if (Blocked())
        return;
    Pause();
    if(IsCode()){codeViewer.SetZoom(std::clamp(codeViewer.GetState().zoom+(delta>0?10:-10),25,400));}
    else if (IsPdf()) { pdfViewer.SetZoom(std::clamp(pdfViewer.GetState().zoom+(delta>0?10:-10),25,400)); }
    else if (IsImage())
    {
        imageZoom = std::clamp(imageZoom + (delta > 0 ? 10 : -10), 25, 400);
        panX = panY = 0;
        InvalidateRect(imageViewer, nullptr, FALSE);
    }
    else { ActiveTextSize() = std::clamp(ActiveTextSize() + delta, 8, 200);
        if(IsCode())codeViewer.SetZoom(ActiveTextSize()*100/14); }
    ApplyTextFormat();
}
void ResetFont()
{
    if (Blocked())
        return;
    Pause();
    if (IsPdf()) {pdfViewer.FitPage();}
    else if (IsImage())
    {
        imageZoom = 100;
        panX = panY = 0;
        InvalidateRect(imageViewer, nullptr, FALSE);
    }
    else
        ActiveTextSize() = IsCode() ? 14 : 24;
    if(IsCode())codeViewer.SetZoom(100);
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
    DrawCodicon(dc,0xeaf0,logo,TEXT,S(24));
    if (!compact)
    {
        RECT brand{S(56), S(20), S(sidebar - 8), S(52)};
        DrawTextAt(dc, L"QuietShare", brand, TEXT, uiFont);
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
                   : safe ? T(L"Đã loại trừ", L"Excluded")
                          : T(L"Đang khóa", L"Locked"),
                   state, stateColor, metaFont, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT stateHint{status.left + S(28), status.top + S(29), status.right - S(8), status.bottom - S(6)};
        DrawTextAt(dc,
                   masked ? T(L"F8 để mở lại", L"F8 to reveal")
                   : safe ? T(L"WGC đạt", L"WGC passed")
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
    SetWindowTextW(GetDlgItem(window, NAV_HOME), T(L"Tổng quan", L"Overview"));
    SetWindowTextW(GetDlgItem(window, NAV_SETTINGS), T(L"Cài đặt", L"Settings"));
    SetWindowTextW(GetDlgItem(window, NAV_PRESENTATION),T(L"Trình chiếu",L"Presentation"));
    SetWindowTextW(GetDlgItem(window, OPEN_FILE), T(L"Mở file", L"Open file"));
    SetWindowTextW(GetDlgItem(window, OPEN_DROP), T(L"Mở hoặc thả file", L"Open or drop a file"));
    SetWindowTextW(GetDlgItem(window, HOME_SCRIPT), T(L"Lời nhắc", L"Script"));
    SetWindowTextW(GetDlgItem(window, HOME_SETTINGS), T(L"Cài đặt", L"Settings"));
    SetWindowTextW(GetDlgItem(window, VIEW_FILE), T(L"Xem file", L"View file"));
    SetWindowTextW(GetDlgItem(window, FOCUS), T(focusMode?L"Thoát tập trung":L"Tập trung",focusMode?L"Exit focus":L"Focus"));
    SetWindowTextW(GetDlgItem(window, SMALLER), IsImage() ? L"-" : L"A-");
    SetWindowTextW(GetDlgItem(window, LARGER), IsImage() ? L"+" : L"A+");
    SetWindowTextW(GetDlgItem(window, PLAY), T(running ? L"Dừng" : L"Chạy", running ? L"Stop" : L"Start"));
    const wchar_t *speeds[] = {T(L"Chậm", L"Slow"), T(L"Vừa", L"Medium"), T(L"Nhanh", L"Fast")};
    SetWindowTextW(GetDlgItem(window, SPEED), speeds[speed]);
    SetWindowTextW(GetDlgItem(window, MASK), masked ? T(L"Mở lại", L"Show") : T(L"Che · F8", L"Hide · F8"));
    SetWindowTextW(GetDlgItem(window, TOPMOST), topmost ? T(L"Ghim", L"Pin") : T(L"Thả", L"Free"));
    SetWindowTextW(GetDlgItem(window, LANGUAGE), english ? L"VI" : L"EN");
    int zoom=IsPdf()?pdfViewer.GetState().zoom:IsCode()?codeViewer.GetState().zoom:IsImage()?imageZoom:fontPoints*100/24;
    std::wstring zoomLabel=std::to_wstring(zoom)+L"%";
    SetWindowTextW(GetDlgItem(window, RESET_ZOOM),page==Page::Settings&&!IsCode()&&!IsPdf()&&!IsImage()?L"24":zoomLabel.c_str());
    SetWindowTextW(GetDlgItem(window, OPEN_FOLDER),T(L"Mở dự án",L"Open project"));
    SetWindowTextW(GetDlgItem(window, VERIFY_PRIVACY),T(L"Kiểm tra WGC",L"Verify WGC privacy"));
    SetWindowTextW(GetDlgItem(window, MIRROR_START),T(L"Bắt đầu",L"Start"));
    SetWindowTextW(GetDlgItem(window, MIRROR_STOP),T(L"Dừng",L"Stop"));
    SetWindowTextW(GetDlgItem(window, POINTER),T(mirrorPointer?L"Con trỏ: bật":L"Con trỏ: tắt",mirrorPointer?L"Pointer: on":L"Pointer: off"));
    SetWindowTextW(GetDlgItem(window, RELOAD_FILE),T(L"Tải lại",L"Reload"));
    SetWindowTextW(GetDlgItem(window, KEEP_FILE),T(L"Giữ bản đang xem",L"Keep current preview"));
    SetWindowTextW(GetDlgItem(window, INCLUDE_IGNORED),T(projectTree.IncludeIgnored()?L"Ẩn thư mục nặng / ẩn":L"Hiện thư mục nặng / ẩn",projectTree.IncludeIgnored()?L"Ignore heavy / hidden folders":L"Include heavy / hidden folders"));
    for(int id:{OPEN_FOLDER,RELOAD_FILE,KEEP_FILE,INCLUDE_IGNORED,FIND_NEXT,FIND_PREV,FIND_INPUT})
        EnableWindow(GetDlgItem(window,id),!Blocked());
    EnableWindow(GetDlgItem(window,VERIFY_PRIVACY),!masked&&!verifying&&!storageBlocked);
    EnableWindow(GetDlgItem(window,MIRROR_START),!Blocked()&&monitors.size()>1&&SelectedMonitor(sourceMonitor)!=SelectedMonitor(destinationMonitor));
    EnableWindow(GetDlgItem(window,MIRROR_STOP),mirror&&mirror->Running());
    for (int id : {MODE, PLAY, SMALLER, LARGER, SPEED, RESET_ZOOM, NAV_SCRIPT, NAV_READ, OPEN_FILE, OPEN_DROP,
                   HOME_SCRIPT, VIEW_FILE})
        EnableWindow(GetDlgItem(window, id), !Blocked());
    EnableWindow(GetDlgItem(window, SMALLER), !Blocked() && (IsPdf()?pdfViewer.GetState().zoom>25:IsCode()?codeViewer.GetState().zoom>25:IsImage() ? imageZoom > 25 : ActiveTextSize() > 8));
    EnableWindow(GetDlgItem(window, LARGER), !Blocked() && (IsPdf()?pdfViewer.GetState().zoom<400:IsCode()?codeViewer.GetState().zoom<400:IsImage() ? imageZoom < 400 : ActiveTextSize() < 200));
    EnableWindow(GetDlgItem(window, PLAY), !Blocked() && !IsImage() && !IsCode()&&!IsPdf());
    EnableWindow(GetDlgItem(window, SPEED), !Blocked() && !IsImage() && !IsCode()&&!IsPdf());
    if (fontSlider)
    {
        const int low=IsImage()||IsPdf()||IsCode()?25:8,high=IsImage()||IsPdf()||IsCode()?400:200;
        if(!sliderTracking) {
            if(SendMessageW(fontSlider,TBM_GETRANGEMIN,0,0)!=low||SendMessageW(fontSlider,TBM_GETRANGEMAX,0,0)!=high)
                SendMessageW(fontSlider,TBM_SETRANGE,TRUE,MAKELPARAM(low,high));
            int value=IsPdf()?pdfViewer.GetState().zoom:IsCode()?codeViewer.GetState().zoom:IsImage()?imageZoom:ActiveTextSize();
            if(SendMessageW(fontSlider,TBM_GETPOS,0,0)!=value)SendMessageW(fontSlider,TBM_SETPOS,TRUE,value);
        }
        EnableWindow(fontSlider, !Blocked());
    }
    if(sizeInput){EnableWindow(sizeInput,!Blocked());SyncSizeInput();}
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
    SetTip(TOPMOST,recallRegistered
        ? T(L"Ghim / Bỏ ghim. Gọi cửa sổ: Ctrl+Alt+Q (không đổi focus)",L"Pin / unpin. Recall: Ctrl+Alt+Q (keeps focus)")
        : T(L"Ghim / Bỏ ghim. Ctrl+Alt+Q không khả dụng: có thể đang được app khác dùng.",L"Pin / unpin. Ctrl+Alt+Q unavailable: another app may use it."));
    SetTip(LANGUAGE, english ? L"Chuyển sang tiếng Việt" : L"Switch to English");
    SetTip(NAV_HOME, T(L"Chuẩn bị nội dung", L"Prepare content"));
    SetTip(NAV_SETTINGS, T(L"Tùy chỉnh hiển thị", L"Display settings"));
    SetTip(NAV_PRESENTATION,T(L"Trình chiếu Smart Mirror",L"Smart Mirror presentation"));
    SetTip(OPEN_FILE,T(L"Mở nhiều file - Ctrl+O",L"Open multiple files - Ctrl+O"));
    SetTip(OPEN_FOLDER,T(L"Mở dự án - Ctrl+Shift+O",L"Open project - Ctrl+Shift+O"));
    SetTip(OVERFLOW,T(L"Các lệnh và tab khác",L"More commands and tab actions"));
    SetTip(MODE,T(L"Soạn / đọc lời nhắc - Ctrl+Enter",L"Edit / read script - Ctrl+Enter"));
    SetTip(FIND_PREV,T(L"Kết quả trước",L"Previous match"));
    SetTip(FIND_NEXT,T(L"Kết quả sau - Enter",L"Next match - Enter"));
    SetTip(FIND_CLOSE,T(L"Đóng tìm kiếm - Esc",L"Close find - Esc"));
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
void UpdateDocumentBar()
{
    std::vector<TabVisual> tabs;
    if(!Blocked()) for(const auto &doc:documents.Items())
        tabs.push_back({doc->id,doc->id?doc->displayName:T(L"Lời nhắc",L"Script"),doc->canonicalPath,(int)doc->type,doc->id==0});
    documentBar.Update(std::move(tabs),documents.Active().id,!Blocked(),english,dpi,uiFont);
}
void Layout()
{
    if(!window || !editor) return;
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    bool caption=(style&WS_CAPTION)==WS_CAPTION;
    if(caption==focusMode) {
        SetWindowLongPtrW(window,GWL_STYLE,focusMode?style&~WS_CAPTION:style|WS_CAPTION);
        SetWindowPos(window,nullptr,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    }
    RECT client{};GetClientRect(window,&client);
    int width=MulDiv(client.right,96,dpi),height=MulDiv(client.bottom,96,dpi);
    compactUi=width<750;shortUi=height<380;
    railWidth=focusMode?0:compactUi?COMPACT_SIDEBAR:NORMAL_SIDEBAR;
    int gutter=focusMode?8:12;
    contentLeft=railWidth+gutter;contentRight=width-gutter;
    for(HWND button:buttons)ShowWindow(button,SW_HIDE);
    for(HWND control:{fontSlider,sizeInput,findInput,sourceMonitor,destinationMonitor,documentBar.Window(),projectTree.Handle()})
        if(control)ShowWindow(control,SW_HIDE);
    fontLabelRect={};
    auto move=[&](int id,int left,int top,int w,int h=32){
        HWND child=GetDlgItem(window,id);MoveWindow(child,S(left),S(top),S(w),S(h),TRUE);ShowWindow(child,SW_SHOW);
    };
    if(!focusMode) {
        int y=64;for(int id:{NAV_HOME,NAV_SCRIPT,NAV_PRESENTATION,NAV_SETTINGS}){
            move(id,8,y,railWidth-16,32);y+=40;
        }
    }
    contentCardRect={S(contentLeft),S(48),S(contentRight),S(height-(focusMode?8:24))};
    if(page==Page::Content || focusMode) {
        int left=contentLeft;
        if(!focusMode && treeVisible && !projectTree.Root().empty() && !Blocked() && width>=900) {
            int treeWidth=176;MoveWindow(projectTree.Handle(),S(left),S(40),S(treeWidth),S(height-64),TRUE);
            ShowWindow(projectTree.Handle(),SW_SHOW);left+=treeWidth+8;
        }
        contentLeft=left;
        int x=contentRight;
        auto action=[&](int id,int w=28){x-=w;move(id,x,2,w,28);};
        action(OVERFLOW);action(MASK);action(FOCUS);
        if(contentRight-contentLeft>=440)action(TOPMOST);
        action(LARGER);action(RESET_ZOOM,48);action(SMALLER);
        if(contentRight-contentLeft>=520)action(OPEN_FILE);
        if(!viewingFile && contentRight-contentLeft>=430) {
            action(MODE);if(reading)action(PLAY);
        }
        int tabWidth=std::max(64,x-contentLeft-4);
        MoveWindow(documentBar.Window(),S(contentLeft),0,S(tabWidth),S(32),TRUE);
        ShowWindow(documentBar.Window(),SW_SHOW);
        contentCardRect={S(contentLeft),S(40),S(contentRight),S(height-(focusMode?8:24))};
        if(findVisible && !reloadPending && !Blocked()) {
            int y=40, inputWidth=std::max(80,contentRight-contentLeft-104);
            MoveWindow(findInput,S(contentLeft),S(y),S(inputWidth),S(28),TRUE);ShowWindow(findInput,SW_SHOW);
            move(FIND_PREV,contentRight-96,y,28,28);move(FIND_NEXT,contentRight-64,y,28,28);move(FIND_CLOSE,contentRight-32,y,28,28);
            contentCardRect.top=S(76);
        }
        if(reloadPending && !Blocked()) {
            move(RELOAD_FILE,contentLeft,40,112,28);move(KEEP_FILE,contentLeft+120,40,144,28);
            contentCardRect.top=S(76);
        }
    }
    else if(page==Page::Home) {
        int w=(contentRight-contentLeft-8)/2;
        move(OPEN_DROP,contentLeft,64,w);move(OPEN_FOLDER,contentLeft+w+8,64,w);
        move(HOME_SCRIPT,contentLeft,108,w);move(HOME_SETTINGS,contentLeft+w+8,108,w);
        move(VERIFY_PRIVACY,contentLeft,160,std::min(220,contentRight-contentLeft));
    }
    else if(page==Page::Settings) {
        int x=contentLeft+12,y=64;move(SMALLER,x,y,32);x+=32;
        x+=4;move(SIZE_INPUT,x,y,64);x+=64;fontLabelRect={S(x),S(y),S(x+28),S(y+32)};x+=32;move(LARGER,x,y,32);x+=40;move(RESET_ZOOM,x,y,56);
        MoveWindow(fontSlider,S(contentLeft+12),S(112),S(contentRight-contentLeft-24),S(24),TRUE);ShowWindow(fontSlider,SW_SHOW);
        move(TOPMOST,contentLeft+12,160,64);move(LANGUAGE,contentLeft+84,160,44);
        move(INCLUDE_IGNORED,contentLeft+12,208,std::min(260,contentRight-contentLeft-24),28);
    }
    else if(page==Page::Presentation) {
        int available=contentRight-contentLeft-88;
        MoveWindow(sourceMonitor,S(contentLeft+88),S(48),S(available),S(160),TRUE);ShowWindow(sourceMonitor,SW_SHOW);
        MoveWindow(destinationMonitor,S(contentLeft+88),S(88),S(available),S(160),TRUE);ShowWindow(destinationMonitor,SW_SHOW);
        move(VERIFY_PRIVACY,contentLeft,132,std::min(220,contentRight-contentLeft));
        move(MIRROR_START,contentLeft,176,84);move(MIRROR_STOP,contentLeft+92,176,84);
        if(contentRight-contentLeft>=280)move(POINTER,contentLeft+184,176,96);
    }
    RECT body=contentCardRect;InflateRect(&body,-S(4),-S(4));
    projectDrawer=!focusMode && page==Page::Content && treeVisible && !projectTree.Root().empty() && !Blocked() && width<900;
    if(projectDrawer) {
        MoveWindow(projectTree.Handle(),body.left,body.top,body.right-body.left,body.bottom-body.top,TRUE);
        ShowWindow(projectTree.Handle(),SW_SHOW);
    }
    documentHost.SetBounds(body);
    RECT textRect{};GetClientRect(editor,&textRect);
    InflateRect(&textRect,-S(12),-S(8));SendMessageW(editor,EM_SETRECT,0,(LPARAM)&textRect);
    DocumentType displayType=IsCode()?DocumentType::Code:documents.Active().type;
    renderedSafe=!Blocked();
    documentHost.Show(displayType,renderedSafe,page==Page::Content&&!projectDrawer);
    UpdateButtons();UpdateDocumentBar();
    RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);
}
void ApplyTextFormat()
{
    loading = true;
    CHARFORMAT2W cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_BOLD;
    cf.yHeight = ActiveTextSize() * 20;
    cf.crTextColor = TEXT;
    wcscpy_s(cf.szFaceName, IsCode() ? L"Consolas" : L"Segoe UI Variable");
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
                         CLEARTYPE_QUALITY, 0, L"Segoe UI Variable");
    titleFont = CreateFontW(-S(22), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                            CLEARTYPE_QUALITY, 0, L"Segoe UI Variable");
    brandFont = CreateFontW(-S(16), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                            CLEARTYPE_QUALITY, 0, L"Segoe UI Variable");
    metaFont = CreateFontW(-S(13), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                           CLEARTYPE_QUALITY, 0, L"Segoe UI Variable");
    for (HWND button : buttons)
        SendMessageW(button, WM_SETFONT, (WPARAM)uiFont, TRUE);
    for(HWND control:{findInput,sizeInput,sourceMonitor,destinationMonitor,projectTree.Handle()})if(control)SendMessageW(control,WM_SETFONT,(WPARAM)uiFont,TRUE);
    if (oldUi)
        DeleteObject(oldUi);
    if (oldTitle)
        DeleteObject(oldTitle);
    if (oldBrand)
        DeleteObject(oldBrand);
    if (oldMeta)
        DeleteObject(oldMeta);
}
int CurrentSize()
{
    return IsPdf()?pdfViewer.GetState().zoom:IsCode()?codeViewer.GetState().zoom:IsImage()?imageZoom:ActiveTextSize();
}
void SyncSizeInput(bool force)
{
    if(!sizeInput||(!force&&GetFocus()==sizeInput))return;
    updatingSize=true;SetWindowTextW(sizeInput,std::to_wstring(sliderTracking&&sliderPreview>=0?sliderPreview:CurrentSize()).c_str());updatingSize=false;
}
void SetContentSize(int value,bool refresh=true)
{
    Pause();sizeError.clear();
    if(IsCode())codeViewer.SetZoom(value);
    else if(IsPdf())pdfViewer.SetZoom(value);
    else if(IsImage()){imageZoom=value;panX=panY=0;InvalidateRect(imageViewer,nullptr,FALSE);}
    else {
        ActiveTextSize()=value;
        CHARFORMAT2W format{};format.cbSize=sizeof(format);format.dwMask=CFM_SIZE;format.yHeight=value*20;
        loading=true;SendMessageW(editor,EM_SETCHARFORMAT,SCF_ALL,(LPARAM)&format);loading=false;
    }
    if(refresh)UpdateButtons();
    SyncSizeInput(true);InvalidateRect(window,&fontLabelRect,FALSE);
}
void CommitSizeInput()
{
    if(updatingSize||!sizeInput||Blocked()||page!=Page::Settings||sizeInputDocument!=documents.Active().id)return;
    const auto text=TextOf(sizeInput);int value=0;bool valid=!text.empty()&&text.size()<=4;
    for(wchar_t character:text){if(character<L'0'||character>L'9'){valid=false;break;}value=value*10+character-L'0';}
    int low=IsCode()||IsImage()||IsPdf()?25:8,high=IsCode()||IsImage()||IsPdf()?400:200;
    if(!valid||value<low||value>high) {
        sizeError=IsCode()||IsImage()||IsPdf()?T(L"Zoom: 25-400%.",L"Zoom: 25-400%."):T(L"Cỡ chữ: 8-200 pt.",L"Text size: 8-200 pt.");
        SyncSizeInput(true);InvalidateRect(window,nullptr,FALSE);return;
    }
    if(value!=CurrentSize())SetContentSize(value);
}
LRESULT CALLBACK SizeInputProc(HWND control,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR)
{
    if(message==WM_SETFOCUS){sizeInputDocument=documents.Active().id;SendMessageW(control,EM_SETSEL,0,-1);}
    if(message==WM_SETFOCUS||message==WM_KILLFOCUS)InvalidateRect(window,nullptr,FALSE);
    if(message==WM_KEYDOWN&&wp==VK_RETURN){CommitSizeInput();return 0;}
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE){sizeError.clear();SyncSizeInput(true);InvalidateRect(window,nullptr,FALSE);return 0;}
    return DefSubclassProc(control,message,wp,lp);
}
void SetReading(bool value)
{
    if (Blocked())
        return;
    if (viewingFile && !value) { ShowScript(); return; }
    Pause();
    page = Page::Content;
    reading = value;
    if(!viewingFile)scriptReading=value;
    SendMessageW(editor, EM_SETREADONLY, reading || viewingFile, 0);
    SendMessageW(editor, EM_SETSEL, 0, 0);
    POINT origin{};
    SendMessageW(editor, EM_SETSCROLLPOS, 0, (LPARAM)&origin);
    Layout();
    SetFocus(ContentWindow());
}
void ToggleRun()
{
    if (!reading || Blocked() || IsImage() || IsCode() || IsPdf())
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
    masked = !masked;
    Pause();
    if(masked){documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);}
    if (masked && filePicker)
    {
        ShowWindow(filePicker, SW_HIDE);
        PostMessageW(filePicker, WM_COMMAND, IDCANCEL, 0);
    }
    // Conceal labels, paths, queries and every viewer before native teardown.
    Layout();
    if(masked && mirror && (mirror->Running()||mirror->Busy()))mirror->Stop();
    RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
void RefreshPrivacy()
{
    topologyKnown=false;
    Pause();
    ShowWindow(editor, SW_HIDE);
    ShowWindow(imageViewer, SW_HIDE);
    documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);
    if(filePicker){ShowWindow(filePicker,SW_HIDE);PostMessageW(filePicker,WM_COMMAND,IDCANCEL,0);}
    Layout();
    if(mirror)mirror->Invalidate();
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
void SaveActiveState()
{
    auto &doc=documents.Active();
    auto &state=doc.state;
    if(doc.id==0) {scriptDraft=TextOf(editor);doc.content.text=scriptDraft;scriptReading=reading;}
    if(IsCode() || IsPdf()) {
        auto view=IsCode()?codeViewer.GetState():pdfViewer.GetState();
        state.scrollX=view.scrollX;state.scrollY=view.scrollY;state.zoom=view.zoom;
        state.page=view.page;state.panX=view.panX;state.panY=view.panY;state.wrap=view.wrap;state.fitWidth=view.fitWidth;
    } else {
        POINT scroll{};SendMessageW(editor,EM_GETSCROLLPOS,0,(LPARAM)&scroll);
        state.scrollX=scroll.x;state.scrollY=scroll.y;
        state.textPoints=ActiveTextSize();state.zoom=imageZoom;state.panX=panX;state.panY=panY;
    }
}
void RenderDocument()
{
    if(Blocked()) {Layout();return;}
    auto &doc=documents.Active();viewingFile=doc.id!=0;
    loading=true;SendMessageW(editor,EM_EXLIMITTEXT,0,viewingFile?5*1024*1024:500000);
    SetWindowTextW(editor,viewingFile?doc.content.text.c_str():scriptDraft.c_str());loading=false;
    reading=viewingFile?true:scriptReading;
    SendMessageW(editor,EM_SETREADONLY,reading || viewingFile,0);
    fontPoints=doc.state.textPoints;codePoints=doc.state.textPoints;
    imageZoom=doc.state.zoom;panX=doc.state.panX;panY=doc.state.panY;
    if(IsCode()) {
        if(!codeViewer.Open(doc.content,viewerError)) {Layout();return;}
        codeViewer.SetState({doc.state.scrollX,doc.state.scrollY,doc.state.zoom,doc.state.page,doc.state.panX,doc.state.panY,doc.state.wrap});
    } else if(IsPdf()) {
        if(!pdfViewer.Open(doc.content,viewerError)) {Layout();return;}
        pdfViewer.SetState({doc.state.scrollX,doc.state.scrollY,doc.state.zoom,doc.state.page,doc.state.panX,doc.state.panY,doc.state.wrap,doc.state.fitWidth});
    }
    ApplyTextFormat();POINT scroll{doc.state.scrollX,doc.state.scrollY};SendMessageW(editor,EM_SETSCROLLPOS,0,(LPARAM)&scroll);
    page=Page::Content;Layout();SetFocus(ContentWindow());
}
void ActivateDocument(DocumentId id)
{
    if(Blocked())return;Pause();SaveActiveState();documentHost.CloseExternal();
    if(compactUi || contentRight-contentLeft<740)treeVisible=false;
    if(!documents.Activate(id,importError)){RenderDocument();return;}
    viewingFile=id!=0;RenderDocument();documents.EvictInactiveHeavy();
}
void CloseDocument(DocumentId id)
{
    if(Blocked() || id==0)return;if(reloadPending==id)reloadPending=0;Pause();SaveActiveState();documentHost.CloseExternal();
    documents.Close(id);viewingFile=documents.Active().id!=0;RenderDocument();documents.EvictInactiveHeavy();
}
void ShowScript()
{
    if(Blocked())return;SaveActiveState();documentHost.CloseExternal();documents.Activate(0);
    if(compactUi || contentRight-contentLeft<740)treeVisible=false;
    viewingFile=false;focusMode=false;reading=false;scriptReading=false;RenderDocument();
}
void ShowOpenedFile()
{
    if(Blocked() || documents.Active().id==0)return;RenderDocument();
}
bool OpenContentPath(const std::wstring &path)
{
    if(Blocked())return false;SaveActiveState();DocumentId previous=documents.Active().id;
    documentHost.CloseExternal();auto result=documents.Open(path,importError);
    if(result.status==OpenStatus::ReloadRequired) {
        reloadPending=result.id;findVisible=false;RenderDocument();return false;
    }
    if(!result) {RenderDocument();return false;}
    if(compactUi || contentRight-contentLeft<740)treeVisible=false;
    viewingFile=true;viewerError.clear();RenderDocument();
    if(!viewerError.empty()) {
        importError=viewerError;documentHost.CloseExternal();
        if(result.status==OpenStatus::Opened)documents.Close(result.id);
        documents.Activate(previous);viewingFile=previous!=0;viewerError.clear();RenderDocument();return false;
    }
    pickerFolder=LocalParentFolder(ActiveContent().path);
    importError.clear();documents.EvictInactiveHeavy();return true;
}
void ChooseFile()
{
    if(Blocked())return;
    Pause();LocalPicker picker;
    auto paths=picker.Show(window,false,pickerFolder,english,uiFont,filePicker,importError);
    if(!paths.empty()) {
        pickerFolder=LocalParentFolder(paths.front());
        if(mirror&&!mirror->IsVerified())VerifyPrivacy();
        for(const auto &path:paths)OpenContentPath(path);
    }
    Layout();
}
void ChooseFolder()
{
    if(Blocked())return;
    Pause();LocalPicker picker;
    auto paths=picker.Show(window,true,projectTree.Root().empty()?pickerFolder:projectTree.Root(),english,uiFont,filePicker,importError);
    if(!paths.empty()) {
        pickerFolder=paths.front();
        if(mirror&&!mirror->IsVerified())VerifyPrivacy();
        if(!Blocked()&&projectTree.OpenFolder(paths.front(),importError)) {
            treeVisible=true;page=Page::Content;focusMode=false;
            TreeView_Expand(projectTree.Handle(),TreeView_GetRoot(projectTree.Handle()),TVE_EXPAND);
        }
    }
    Layout();
    if(!Blocked()&&IsWindowVisible(projectTree.Handle()))SetFocus(projectTree.Handle());
}
std::wstring SelectedMonitor(HWND combo)
{
    int index=(int)SendMessageW(combo,CB_GETCURSEL,0,0);
    return index>=0 && index<(int)monitors.size()?monitors[index].stableId:L"";
}
void UpdateMonitors()
{
    auto source=SelectedMonitor(sourceMonitor),destination=SelectedMonitor(destinationMonitor);
    monitors=EnumerateMonitors();
    for(HWND combo:{sourceMonitor,destinationMonitor})SendMessageW(combo,CB_RESETCONTENT,0,0);
    int src=0,dst=monitors.size()>1?1:0;
    for(size_t i=0;i<monitors.size();++i) {
        const auto &monitor=monitors[i];std::wstring label=monitor.name+L"  "+std::to_wstring(monitor.bounds.right-monitor.bounds.left)+L"x"+std::to_wstring(monitor.bounds.bottom-monitor.bounds.top);
        SendMessageW(sourceMonitor,CB_ADDSTRING,0,(LPARAM)label.c_str());SendMessageW(destinationMonitor,CB_ADDSTRING,0,(LPARAM)label.c_str());
        if(monitor.stableId==source)src=(int)i;if(monitor.stableId==destination)dst=(int)i;
    }
    SendMessageW(sourceMonitor,CB_SETCURSEL,src,0);SendMessageW(destinationMonitor,CB_SETCURSEL,dst,0);
}
void VerifyPrivacy()
{
    if(qa || verifying || !mirror || masked || storageBlocked)return;
    verifying=true;documentHost.Hide();Layout();privacy=ApplyPrivacy(window);topologyKnown=TryGetDisplayCloned(cloned);
    bool basic=privacy.applied && topologyKnown && !cloned;
    if(basic)mirror->Verify(window,SelectedMonitor(sourceMonitor));
    else mirror->Invalidate();
    verifying=false;Layout();
}
void RunFind(bool forward)
{
    if(Blocked())return;std::wstring query=TextOf(findInput);if(query.empty())return;
    bool found=false;
    if(IsCode())found=codeViewer.Find(query,forward);
    else if(IsPdf())found=pdfViewer.Find(query,forward);
    else {
        CHARRANGE selection{};SendMessageW(editor,EM_EXGETSEL,0,(LPARAM)&selection);
        FINDTEXTEXW search{};search.chrg={forward?selection.cpMax:selection.cpMin,forward?-1:0};search.lpstrText=query.c_str();
        auto result=SendMessageW(editor,EM_FINDTEXTEXW,forward?FR_DOWN:0,(LPARAM)&search);
        if(result<0){search.chrg={forward?0:GetWindowTextLengthW(editor),forward?-1:0};result=SendMessageW(editor,EM_FINDTEXTEXW,forward?FR_DOWN:0,(LPARAM)&search);}
        if(result>=0){SendMessageW(editor,EM_EXSETSEL,0,(LPARAM)&search.chrgText);SendMessageW(editor,EM_SCROLLCARET,0,0);found=true;}
    }
    findMessage=found?L"":IsPdf()&&!pdfViewer.HasText()?T(L"PDF ảnh không có lớp chữ để tìm.",L"This scanned PDF has no searchable text."):
                                      T(L"Không tìm thấy. Thử từ khác.",L"No match. Try another term.");
    InvalidateRect(window,nullptr,FALSE);
}
void OverflowMenu()
{
    std::vector<ToolMenuItem> entries;bool safe=!Blocked();
    auto item=[&](int id,const wchar_t*vn,const wchar_t*en,bool enabled=true){
        wchar_t icon=id==OPEN_FILE?0xea7f:id==OPEN_FOLDER?0xeaf7:id==FIND?0xea6d:
            id==PLAY?0xeb2c:id==TOPMOST?0xeb2b:id==FOCUS?0xeb4c:
            id==CLOSE_APPLICATION||id==CLOSE_TAB||id==CLOSE_OTHERS||id==CLOSE_FILES?0xea76:0xeb51;
        entries.push_back({id,T(vn,en),enabled,icon});};
    item(OPEN_FILE,L"Mở file...   Ctrl+O",L"Open files...   Ctrl+O",safe);
    item(OPEN_FOLDER,L"Mở dự án...   Ctrl+Shift+O",L"Open project...   Ctrl+Shift+O",safe);
    item(FIND,L"Tìm...   Ctrl+F",L"Find...   Ctrl+F",safe&&!IsImage());
    item(WRAP,L"Bật/tắt xuống dòng",L"Toggle line wrap",safe&&IsCode());
    item(MODE,L"Soạn / Đọc   Ctrl+Enter",L"Edit / Read   Ctrl+Enter",safe);
    if(!IsImage()&&!IsCode()&&!IsPdf()) {
        item(PLAY,L"Chạy / Dừng cuộn   Space",L"Start / stop scrolling   Space",safe);
        item(SPEED,L"Đổi tốc độ cuộn",L"Change scroll speed",safe);
    }
    if(IsPdf()) {
        item(PDF_PREV,L"Trang PDF trước",L"Previous PDF page",safe);item(PDF_NEXT,L"Trang PDF sau",L"Next PDF page",safe);
        item(FIT_WIDTH,L"PDF vừa chiều ngang",L"Fit PDF width",safe);
    }
    item(TOPMOST,L"Ghim / Bỏ ghim",L"Pin / unpin");item(FOCUS,L"Tập trung   F11",L"Focus   F11");
    item(SHOW_TREE,L"Ẩn / Hiện cây thư mục",L"Hide / show project tree",safe);
    item(LANGUAGE,L"English / Tiếng Việt",L"Tiếng Việt / English");
    item(CLOSE_TAB,L"Đóng tab   Ctrl+W",L"Close tab   Ctrl+W",safe&&viewingFile);
    item(CLOSE_OTHERS,L"Đóng các tab khác",L"Close other tabs",safe);
    item(CLOSE_FILES,L"Đóng mọi file",L"Close all file tabs",safe);
    item(VERIFY_PRIVACY,L"Kiểm tra WGC",L"Verify WGC privacy",!masked);
    item(CLOSE_APPLICATION,L"Đóng ứng dụng   Alt+F4",L"Close application   Alt+F4");
    RECT r{};GetWindowRect(GetDlgItem(window,OVERFLOW),&r);
    ToolMenu menu;
    int selected=menu.Show(window,r,dpi,uiFont,std::move(entries));
    if(selected)SendMessageW(window,WM_COMMAND,selected,0);
}
void ToggleFocus()
{
    focusMode = !focusMode;
    if (focusMode && !Blocked()) { if(!viewingFile)SetReading(true); }
    page = Page::Content;
    Layout();
    SetFocus(ContentWindow());
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
            DrawImage(buffer, ActiveContent(), bounds, imageZoom, CARD, panX, panY);
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
    RECT client{};GetClientRect(window,&client);FillRect(dc,&client,bgBrush);
    if(!focusMode)DrawSidebar(dc,railWidth,client.bottom,true);
    RECT title{S(contentLeft),S(12),S(contentRight),S(40)};
    if(!focusMode && page!=Page::Content)DrawTextAt(dc,page==Page::Home?T(L"Tổng quan",L"Overview"):
        page==Page::Settings?T(L"Cài đặt",L"Settings"):T(L"Trình chiếu",L"Presentation"),title,TEXT,titleFont);
    if(page==Page::Content || focusMode) {
        FillRect(dc,&contentCardRect,cardBrush);
        if(Blocked()) {
            const wchar_t *heading=storageBlocked?T(L"Lời nhắc đang khóa",L"Script locked"):
                masked?T(L"Đã che nội dung",L"Content hidden"):
                cloned?T(L"Đang dùng Duplicate",L"Duplicate display mode"):
                verifying?T(L"Đang kiểm tra WGC",L"Verifying WGC privacy"):
                !topologyKnown?T(L"Chưa xác định màn hình",L"Display layout unknown"):T(L"Chưa xác minh capture",L"Capture not verified");
            const wchar_t *body=storageBlocked?T(L"Bản lưu cũ được giữ nguyên. Kiểm tra quyền đọc tệp lời nhắc.",L"Old script preserved. Check access to the saved script."):
                masked?T(L"F8 để mở lại.",L"F8 to reveal."):
                cloned?T(L"Win+P > Extend, hoặc chia sẻ riêng một cửa sổ.",L"Use Win+P > Extend, or share an individual window."):
                T(L"Về Tổng quan hoặc Trình chiếu và bấm Kiểm tra WGC. Nội dung chỉ mở sau khi canary đạt.",L"Open Overview or Presentation and verify WGC. Content opens only after the canary passes.");
            DrawCenteredMessage(dc,contentCardRect,heading,body);
        }
    }
    else if(page==Page::Home) {
        RECT formats{S(contentLeft),S(208),S(contentRight),S(244)};
        DrawTextAt(dc,T(L"Văn bản · Code · Ảnh · PDF",L"Text · Code · Images · PDF"),formats,MUTED,metaFont,DT_LEFT|DT_WORDBREAK);
        RECT status{S(contentLeft),S(248),S(contentRight),client.bottom-S(36)};
        std::wstring text=verifying?T(L"Đang kiểm tra canary bằng WGC...",L"Checking the WGC canary..."):
            privacy.applied?T(L"Windows đã nhận loại trừ capture. Người xem chưa được xác minh.",L"Windows accepted capture exclusion. Viewer unverified."):
                             T(L"Windows chưa nhận loại trừ capture. Nội dung đang khóa.",L"Capture exclusion unavailable. Content locked.");
        if(!qa && mirror)text+=L"\n"+mirror->Status(english);
        DrawTextAt(dc,text,status,MUTED,metaFont,DT_LEFT|DT_WORDBREAK);
    }
    else if(page==Page::Settings) {
        RECT summary{S(contentLeft+12),S(44),S(contentRight-12),S(64)};
        DrawTextAt(dc,IsImage()||IsPdf()?T(L"Phóng / thu",L"Zoom"):T(L"Cỡ chữ",L"Text size"),summary,MUTED,metaFont);
    }
    else if(page==Page::Presentation) {
        DrawTextAt(dc,T(L"Nguồn",L"Source"),{S(contentLeft),S(48),S(contentLeft+80),S(76)},MUTED,metaFont);
        DrawTextAt(dc,T(L"Đích",L"Destination"),{S(contentLeft),S(88),S(contentLeft+80),S(116)},MUTED,metaFont);
        RECT status{S(contentLeft),S(220),S(contentRight),client.bottom-S(24)};
        std::wstring text=mirror?mirror->Status(english):L"";
        if(monitors.size()<2)text+=T(L"\nCần hai màn hình ở chế độ Extend để chiếu.",L"\nTwo displays in Extend are required for projection.");
        else text+=T(L"\nĐầu nhận vật lý chưa được xác minh.",L"\nPhysical receiver unverified.");
        if(mirror && mirror->Running())text+=L"\nFPS: "+std::to_wstring((int)mirror->Fps());
        DrawTextAt(dc,text,status,MUTED,metaFont,DT_LEFT|DT_WORDBREAK);
    }
    if(fontLabelRect.right>fontLabelRect.left) {
        if(page==Page::Settings&&sizeInput) {
            RECT input{};GetWindowRect(sizeInput,&input);MapWindowPoints(nullptr,window,(POINT*)&input,2);InflateRect(&input,1,1);
            HPEN pen=CreatePen(PS_SOLID,1,GetFocus()==sizeInput?ACCENT:DIVIDER);
            auto oldPen=SelectObject(dc,pen),oldBrush=SelectObject(dc,GetStockObject(NULL_BRUSH));
            Rectangle(dc,input.left,input.top,input.right,input.bottom);
            SelectObject(dc,oldBrush);SelectObject(dc,oldPen);DeleteObject(pen);
        }
        std::wstring label=IsPdf()||IsCode()||IsImage()?L"%":L"pt";
        DrawTextAt(dc,label,fontLabelRect,TEXT,uiFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
    if(focusMode)return;
    RECT footer{S(contentLeft),client.bottom-S(22),S(contentRight),client.bottom-S(2)};
    std::wstring label=Blocked()?T(L"Nội dung đang khóa",L"Content locked"):
        saveFailed?T(L"Không lưu được lời nhắc",L"Script save failed"):
        savedNoteRecovered&&!dirty&&!reading?T(L"Đã giữ bản dự phòng lời nhắc cũ",L"Old script recovery copy preserved"):
        dirty?T(L"Đang lưu...",L"Saving..."):!sizeError.empty()?sizeError:!findMessage.empty()?findMessage:
        !importError.empty()?importError:IsPdf()?std::to_wstring(pdfViewer.GetState().page+1)+L" / "+std::to_wstring(pdfViewer.PageCount()):
        viewingFile?ActiveContent().displayName:page==Page::Content?T(L"Lời nhắc · Tự lưu trên máy",L"Script · Saved locally"):L"";
    size_t split=label.find(L" / ");if(split!=std::wstring::npos && !IsPdf())label=english?label.substr(split+3):label.substr(0,split);
    DrawTextAt(dc,label,footer,saveFailed||!importError.empty()?AMBER:MUTED,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
}
void DrawActionIcon(HDC dc,int id,RECT r,COLORREF color)
{
    wchar_t glyph=id==SMALLER?0xeb3b:id==LARGER?0xea60:id==FOCUS?(focusMode?0xeb4d:0xeb4c):
        id==MASK?(masked?0xeae7:0xea70):id==TOPMOST?0xeb2b:id==OPEN_FILE?0xea7f:
        id==PLAY?(running?0xead1:0xeb2c):id==MODE?0xea73:id==OVERFLOW?0xea7c:
        id==FIND_CLOSE?0xea76:id==FIND_PREV?0xeab5:0xeab6;
    DrawCodicon(dc,glyph,r,color,S(16));
}
void DrawButton(DRAWITEMSTRUCT *item)
{
    bool disabled = (item->itemState & ODS_DISABLED) != 0;
    bool nav = item->CtlID == NAV_SCRIPT || item->CtlID == NAV_READ || item->CtlID == NAV_HOME || item->CtlID == NAV_SETTINGS || item->CtlID==NAV_PRESENTATION;
    bool activeNav = (item->CtlID == NAV_SCRIPT && page == Page::Content) ||
                     (item->CtlID == NAV_READ && page == Page::Content && reading) ||
                     (item->CtlID == NAV_HOME && page == Page::Home) ||
                     (item->CtlID == NAV_SETTINGS && page == Page::Settings) || (item->CtlID==NAV_PRESENTATION && page==Page::Presentation);
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
        color = RGB(17,139,220);
    HBRUSH background = CreateSolidBrush(nav ? SIDEBAR : zoom ? ELEVATED : page == Page::Settings ? CARD : BG);
    FillRect(item->hDC, &item->rcItem, background);
    DeleteObject(background);
    RECT button = item->rcItem;
    InflateRect(&button, -1, -1);
    if (!zoom || hover || pressed)
        FillRoundRect(item->hDC, button, color, S(8));
    bool strip=page==Page::Content || focusMode;
    if (nav)
    {
        COLORREF iconColor = disabled ? DISABLED : (activeNav ? TEXT : MUTED);
        wchar_t glyph=item->CtlID==NAV_HOME?0xeb06:item->CtlID==NAV_SCRIPT?0xeaf0:
            item->CtlID==NAV_SETTINGS?0xeb51:0xea7a;
        DrawCodicon(item->hDC,glyph,button,iconColor,S(24));
    }
    else if(strip && (item->CtlID==FOCUS || item->CtlID==MASK || item->CtlID==TOPMOST || item->CtlID==SMALLER || item->CtlID==LARGER || item->CtlID==MODE || item->CtlID==PLAY || item->CtlID==OPEN_FILE || item->CtlID==OVERFLOW || item->CtlID==FIND_CLOSE || item->CtlID==FIND_PREV || item->CtlID==FIND_NEXT))
        DrawActionIcon(item->hDC,item->CtlID,button,disabled?DISABLED:TEXT);
    else
    {
        DrawTextAt(item->hDC, TextOf(item->hwndItem), button,
                   disabled  ? DISABLED
                   : primary ? TEXT
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
    if(control==fontSlider&&message==WM_ERASEBKGND)return 1;
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
LRESULT CALLBACK MonitorComboProc(HWND control,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR)
{
    if(message==WM_PAINT) {
        PAINTSTRUCT paint{};HDC dc=BeginPaint(control,&paint);RECT bounds{};GetClientRect(control,&bounds);
        FillRoundRect(dc,bounds,ELEVATED,S(8));
        int index=(int)SendMessageW(control,CB_GETCURSEL,0,0);std::wstring text;
        if(index>=0) {
            int length=(int)SendMessageW(control,CB_GETLBTEXTLEN,index,0);
            if(length>=0 && length<512){text.resize(length+1);SendMessageW(control,CB_GETLBTEXT,index,(LPARAM)text.data());text.resize(length);}
        }
        RECT label=bounds;label.left+=S(10);label.right-=S(28);
        DrawTextAt(dc,text,label,TEXT,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        int x=bounds.right-S(14),y=(bounds.top+bounds.bottom)/2;
        HPEN pen=CreatePen(PS_SOLID,std::max(1,S(1)),GetFocus()==control?ACCENT:DIVIDER);
        auto old=SelectObject(dc,pen),brush=SelectObject(dc,GetStockObject(NULL_BRUSH));
        RoundRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom,S(8),S(8));
        SelectObject(dc,brush);SelectObject(dc,old);DeleteObject(pen);
        pen=CreatePen(PS_SOLID,std::max(1,S(2)),MUTED);old=SelectObject(dc,pen);
        MoveToEx(dc,x-S(3),y-S(2),nullptr);LineTo(dc,x,y+S(2));LineTo(dc,x+S(3),y-S(2));
        SelectObject(dc,old);DeleteObject(pen);EndPaint(control,&paint);return 0;
    }
    if(message==WM_ERASEBKGND)return 1;
    LRESULT result=DefSubclassProc(control,message,wp,lp);
    if(message==WM_SETFOCUS || message==WM_KILLFOCUS || message==CB_SETCURSEL)InvalidateRect(control,nullptr,FALSE);
    return result;
}
LRESULT CALLBACK EditorProc(HWND control, UINT message, WPARAM wp, LPARAM lp)
{
    if(Blocked() && (message==WM_COPY || message==WM_CUT || message==WM_PASTE))return 0;
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
    if (message == WM_KEYDOWN && wp == VK_F8 && !f8Registered && !(lp & (1L<<30)))
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
           rect.bottom <= client.bottom - S(focusMode ? 4 : 24) && rect.bottom - rect.top >= S(80);
}
void WriteQa()
{
    DWORD actual = 0;
    GetWindowDisplayAffinity(window, &actual);
    auto boolean = [](bool value) -> std::wstring { return value ? L"true" : L"false"; };
    std::wstring report =
        L"{\n  \"applicationVersion\": \"0.4.3\",\n  \"windowsBuild\": " + std::to_wstring(WindowsBuild()) +
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
        L",\n  \"v04TabsProjectPdfFocusFit\": " + boolean(qaV04) +
        L",\n  \"dpiMatrixMethod\": \"Simulated logical scaling at 96, 120, 144 and 192 DPI; not a physical "
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
                 qaFocusRestored && qaV04;
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
            for (UINT scale : {96U, 120U, 144U, 192U})
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
                        for (Page surface : {Page::Home, Page::Settings, Page::Presentation})
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
        RECT strip{};GetWindowRect(documentBar.Window(),&strip);
        RECT focusButton{};GetWindowRect(GetDlgItem(window,FOCUS),&focusButton);
        qaFocus = focusMode && !(GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION) && !IsWindowVisible(GetDlgItem(window,NAV_SCRIPT)) &&
                  IsWindowVisible(documentBar.Window()) && strip.bottom-strip.top==S(32) &&
                  focusButton.right-focusButton.left==S(28) && CompactFits();
        Capture(L"focus.bmp");
        SendMessageW(window,WM_COMMAND,MAKEWPARAM(FOCUS,BN_CLICKED),0);
        qaFocusRestored = !focusMode && (GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION)==WS_CAPTION && IsWindowVisible(GetDlgItem(window,NAV_SCRIPT)) &&
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
                    qaDropped = viewingFile && ActiveContent().displayName == L"example.cpp";
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
        {
            // Synthetic native renderer fixtures; the shipping EXE rejects QA switches.
            for(int n=0;n<8;n++) {
                const auto file=qaDir+L"\\sample-"+std::to_wstring(n)+L".py";
                const std::string text="# QuietShare sample\ndef explain(value):\n    return value + 1\n";
                WriteBytes(file,text.data(),(DWORD)text.size());
                qaV04=OpenContentPath(file)&&qaV04;
            }
            qaV04=documents.Count()>=9&&documentBar.Overflowing()&&qaV04;
            Capture(L"tabs-overflow.bmp");ToggleFocus();Capture(L"code-focus.bmp");
            const UINT actualDpi=dpi;
            for(UINT scale:{96U,120U,144U,192U})for(bool language:{false,true}) {
                dpi=scale;english=language;RefreshFonts();
                for(int width:{430,460,640,840,1080}) {
                    SetWindowPos(window,nullptr,0,0,S(width),S(width<500?320:560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
                    Layout();RECT strip{};GetClientRect(documentBar.Window(),&strip);
                    qaV04=strip.bottom==S(32)&&CompactFits()&&qaV04;
                }
            }
            dpi=actualDpi;english=false;RefreshFonts();ToggleFocus();
            SetWindowPos(window,nullptr,0,0,S(1080),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            qaV04=projectTree.OpenFolder(qaDir,importError)&&qaV04;
            treeVisible=true;TreeView_Expand(projectTree.Handle(),TreeView_GetRoot(projectTree.Handle()),TVE_EXPAND);Layout();
            Capture(L"project-wide.bmp");
            SetWindowPos(window,nullptr,0,0,S(430),S(320),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();
            qaV04=projectDrawer&&!IsWindowVisible(codeViewer.Window())&&qaV04;Capture(L"project-minimum.bmp");
            treeVisible=false;SetWindowPos(window,nullptr,0,0,S(840),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            qaV04=OpenContentPath(qaDir+L"\\..\\qa-viewers\\unicode.pdf")&&IsPdf()&&qaV04;
            Capture(L"pdf-file.bmp");ToggleFocus();Capture(L"pdf-focus.bmp");ToggleFocus();
            page=Page::Presentation;Layout();Capture(L"presentation.bmp");
            english=true;Layout();Capture(L"presentation-en.bmp");
            english=false;page=Page::Content;Layout();ToggleMask();Capture(L"tabs-masked.bmp");ToggleMask();
            ShowScript();
        }
        FinishQa();
        break;
    }
}
void PublicThumbnail(HWND owner,int width,int height,bool live)
{
    width=std::clamp(width,64,512);height=std::clamp(height,48,320);
    HDC screen=GetDC(owner),dc=CreateCompatibleDC(screen);
    HBITMAP bitmap=CreateCompatibleBitmap(screen,width,height);auto old=SelectObject(dc,bitmap);
    RECT bounds{0,0,width,height};FillRect(dc,&bounds,bgBrush);
    DrawCodicon(dc,0xeaf0,bounds,ACCENT,S(28));
    if(live){POINT origin{};DwmSetIconicLivePreviewBitmap(owner,bitmap,&origin,DWM_SIT_DISPLAYFRAME);}
    else DwmSetIconicThumbnail(owner,bitmap,DWM_SIT_DISPLAYFRAME);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(owner,screen);
}
LRESULT CALLBACK Proc(HWND h, UINT message, WPARAM wp, LPARAM lp)
{
    switch (message)
    {
    case WM_DWMSENDICONICTHUMBNAIL:PublicThumbnail(h,HIWORD(lp),LOWORD(lp),false);return 0;
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:PublicThumbnail(h,320,180,true);return 0;
    case WM_CREATE: {
        if(!LoadCodicons(instance))return -1;
        window = h;
        mirror=std::make_unique<MirrorController>();
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
            {HOME_SETTINGS, L"Cài đặt"}, {VIEW_FILE, L"Xem file"},
            {OPEN_FOLDER,L"Mở dự án"},{OVERFLOW,L"Thêm"},{FIND,L"Tìm"},{FIND_PREV,L"Trước"},
            {FIND_NEXT,L"Sau"},{FIND_CLOSE,L"Đóng"},{PDF_PREV,L"Trang trước"},{PDF_NEXT,L"Trang sau"},
            {NAV_PRESENTATION,L"Trình chiếu"},{VERIFY_PRIVACY,L"Kiểm tra WGC"},{MIRROR_START,L"Bắt đầu"},
            {MIRROR_STOP,L"Dừng"},{POINTER,L"Con trỏ"},{RELOAD_FILE,L"Tải lại"},{KEEP_FILE,L"Giữ bản đang xem"},
            {INCLUDE_IGNORED,L"Hiện thư mục đang bỏ qua"}};
        for (const auto &label : labels)
        {
            buttons.push_back(CreateWindowW(L"BUTTON", label.second,
                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 0, 0, h,
                                            (HMENU)(INT_PTR)label.first, instance, nullptr));
            SetWindowSubclass(buttons.back(), ButtonProc, 1, 0);
            if(label.first==OPEN_FILE || label.first==OPEN_DROP || label.first==OPEN_FOLDER ||
               label.first==NAV_SCRIPT || label.first==HOME_SCRIPT || label.first==MODE ||
               label.first==PLAY || label.first==MIRROR_START)
                EnableWindow(buttons.back(),FALSE);
        }
        // Native tooltips preserve readable names for the automatic compact rail.
        tooltips =
            CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                            0, 0, 0, 0, h, nullptr, instance, nullptr);
        SendMessageW(tooltips, TTM_SETMAXTIPWIDTH, 0, S(300));
        SendMessageW(tooltips, TTM_SETDELAYTIME, TTDT_INITIAL, 850);
        SendMessageW(tooltips, TTM_SETDELAYTIME, TTDT_RESHOW, 850);
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
        sizeInput=CreateWindowExW(0,L"EDIT",L"24",WS_CHILD|WS_TABSTOP|ES_CENTER|ES_NUMBER|ES_AUTOHSCROLL,
            0,0,0,0,h,(HMENU)SIZE_INPUT,instance,nullptr);
        SendMessageW(sizeInput,EM_SETLIMITTEXT,4,0);SetWindowTheme(sizeInput,L"DarkMode_Explorer",nullptr);
        SetWindowSubclass(sizeInput,SizeInputProc,1,0);
        SetWindowTheme(fontSlider, L"DarkMode_Explorer", nullptr);
        SetWindowSubclass(fontSlider, ButtonProc, 1, 0);
        if(!documentHost.Create(h,editor,imageViewer,viewerError))return -1;
        if(!documentBar.Create(h,instance) || !projectTree.Create(h,172))return -1;
        HWND treeTip=TreeView_GetToolTips(projectTree.Handle());if(treeTip && !ApplyPrivacy(treeTip).applied)
            SetWindowLongPtrW(projectTree.Handle(),GWL_STYLE,GetWindowLongPtrW(projectTree.Handle(),GWL_STYLE)|TVS_NOTOOLTIPS);
        ApplyPrivacy(tooltips);
        findInput=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL,0,0,0,0,h,(HMENU)FIND_INPUT,instance,nullptr);
        sourceMonitor=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,0,0,0,0,h,(HMENU)SOURCE_MONITOR,instance,nullptr);
        destinationMonitor=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,0,0,0,0,h,(HMENU)DEST_MONITOR,instance,nullptr);
        for(HWND combo:{sourceMonitor,destinationMonitor})SetWindowSubclass(combo,MonitorComboProc,1,0);
        std::vector<HWND> publicLists;
        for(HWND combo:{sourceMonitor,destinationMonitor}) {
            COMBOBOXINFO info{};info.cbSize=sizeof(info);
            if(GetComboBoxInfo(combo,&info))publicLists.push_back(info.hwndList);
        }
        mirror->SetPublicPopups(publicLists);
        for(HWND control:{findInput,sourceMonitor,destinationMonitor})SetWindowTheme(control,L"DarkMode_Explorer",nullptr);
        UpdateMonitors();
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
        scriptDraft=content;documents.Get(0)->content.text=content;
        loading = false;
        qaContent = TextOf(editor);
        RefreshFonts();
        ApplyTextFormat();
        page = qa ? Page::Content : Page::Home;
        SendMessageW(editor, EM_SETSEL, 0, 0);
        if (!qa)
        {
            f8Registered = RegisterHotKey(h, 2, MOD_NOREPEAT, VK_F8) != 0;
            recallRegistered=RegisterHotKey(h,3,MOD_CONTROL|MOD_ALT|MOD_NOREPEAT,L'Q')!=0;
            UpdateButtons();
            SetTimer(h, PRIVACY_TIMER, 250, nullptr);
            SetTimer(h, 5, 8, nullptr);
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
    case WM_MEASUREITEM: {
        auto measure=(MEASUREITEMSTRUCT*)lp;
        if(measure->CtlType==ODT_MENU){measure->itemWidth=S(280);measure->itemHeight=S(28);return TRUE;}
        if(measure->CtlType==ODT_COMBOBOX){measure->itemHeight=S(28);return TRUE;}
        break;
    }
    case WM_DRAWITEM:
        if(((DRAWITEMSTRUCT*)lp)->CtlType==ODT_COMBOBOX) {
            auto draw=(DRAWITEMSTRUCT*)lp;bool selected=draw->itemState&ODS_SELECTED;
            HBRUSH brush=CreateSolidBrush(selected?ACCENT_DARK:ELEVATED);FillRect(draw->hDC,&draw->rcItem,brush);DeleteObject(brush);
            if(draw->itemID!=(UINT)-1) {
                wchar_t text[512]{};int length=(int)SendMessageW(draw->hwndItem,CB_GETLBTEXTLEN,draw->itemID,0);
                if(length>=0 && length<512)SendMessageW(draw->hwndItem,CB_GETLBTEXT,draw->itemID,(LPARAM)text);
                RECT label=draw->rcItem;label.left+=S(10);label.right-=S(10);DrawTextAt(draw->hDC,text,label,TEXT,uiFont);
            }
            return TRUE;
        }
        if(((DRAWITEMSTRUCT*)lp)->CtlType==ODT_MENU) {
            auto draw=(DRAWITEMSTRUCT*)lp;bool selected=draw->itemState&ODS_SELECTED;
            HBRUSH brush=CreateSolidBrush(selected?HOVER:CARD);FillRect(draw->hDC,&draw->rcItem,brush);DeleteObject(brush);
            RECT text=draw->rcItem;text.left+=S(12);text.right-=S(12);
            DrawTextAt(draw->hDC,(LPCWSTR)draw->itemData,text,draw->itemState&ODS_DISABLED?DISABLED:TEXT,uiFont);
            return TRUE;
        }
        DrawButton((DRAWITEMSTRUCT *)lp);
        return TRUE;
    case VerifyPrivacyMessage:VerifyPrivacy();return 0;
    case WM_APP+14:
        if(mirror && !qa)mirror->Tick();
        if(Blocked()) {
            documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);
            if(filePicker){ShowWindow(filePicker,SW_HIDE);PostMessageW(filePicker,WM_COMMAND,IDCANCEL,0);}
            if(renderedSafe)Layout();
            return 0;
        }
        return 1;
    case WM_APP+15:
        // Read-only diagnostics expose numeric state only after the common gate.
        if(Blocked())return 0;
        switch(wp) {
        case 1:return (LRESULT)documents.Count();
        case 2:return (LRESULT)documents.Active().id;
        case 3:return (LRESULT)documents.Active().type;
        case 4:return IsCode()?codeViewer.GetState().zoom:
            documents.Active().type==DocumentType::Pdf?pdfViewer.GetState().zoom:
            documents.Active().type==DocumentType::Image?imageZoom:fontPoints*100/24;
        case 5:return pdfViewer.GetState().page+1;
        case 6:return pdfViewer.PageCount();
        case 7:return documentBar.Offset();
        case 8:return 32;
        case 9:return (LRESULT)projectTree.NodeCount();
        }
        return 0;
    case WM_APP+16:
        if(!Blocked())UpdateButtons();
        return 0;
    case DocumentActivateMessage:ActivateDocument((DocumentId)wp);return 0;
    case DocumentCloseMessage:CloseDocument((DocumentId)wp);return 0;
    case WM_NOTIFY: {
        auto notification = (NMHDR *)lp;
        if(notification->hwndFrom==codeViewer.Window() && notification->code==2018 && !Blocked())UpdateButtons();
        std::wstring projectFile;
        if(!Blocked() && projectTree.HandleNotify(notification,projectFile,importError)) {
            if(!projectFile.empty())OpenContentPath(projectFile);return 0;
        }
        if (notification->hwndFrom == fontSlider && notification->code == NM_CUSTOMDRAW)
        {
            auto draw = (NMCUSTOMDRAW *)lp;
            if (draw->dwDrawStage == CDDS_PREPAINT)
            {
                RECT bounds{}; GetClientRect(fontSlider, &bounds);
                FillRect(draw->hdc, &bounds, cardBrush);
                RECT channel{};
                SendMessageW(fontSlider, TBM_GETCHANNELRECT, 0, (LPARAM)&channel);
                int centerY = (bounds.top + bounds.bottom) / 2;
                int low=(int)SendMessageW(fontSlider,TBM_GETRANGEMIN,0,0),high=(int)SendMessageW(fontSlider,TBM_GETRANGEMAX,0,0);
                int value=sliderPreview>=0?sliderPreview:(int)SendMessageW(fontSlider,TBM_GETPOS,0,0);
                int centerX=channel.left+MulDiv(std::clamp(value,low,high)-low,channel.right-channel.left,std::max(1,high-low));
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
                FillRoundRect(draw->hdc,handle,!IsWindowEnabled(fontSlider) ? DIVIDER : ACCENT,S(14));
                InflateRect(&handle,-S(2),-S(2));
                FillRoundRect(draw->hdc,handle,hoveredButton == fontSlider ? BG : ELEVATED,S(10));
                return CDRF_SKIPDEFAULT;
            }
        }
        break;
    }
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, CARD);
        SetTextColor((HDC)wp, TEXT);
        return (LRESULT)cardBrush;
    case WM_HSCROLL:
        if ((HWND)lp == fontSlider && !Blocked())
        {
            int action=LOWORD(wp);
            int value=action==TB_THUMBTRACK?HIWORD(wp):(int)SendMessageW(fontSlider,TBM_GETPOS,0,0);
            sliderTracking=action==TB_THUMBTRACK;sliderPreview=value;
            InvalidateRect(fontSlider,nullptr,FALSE);UpdateWindow(fontSlider);SyncSizeInput(true);
            // PDF rendering is intentionally deferred until release; the slider
            // and numeric preview remain immediate even for expensive pages.
            if(!IsPdf()||!sliderTracking)if(value!=CurrentSize())SetContentSize(value,false);
            if(!sliderTracking){sliderPreview=-1;UpdateButtons();}
        }
        return 0;
    case WM_DROPFILES: {
        HDROP drop=(HDROP)wp;UINT count=DragQueryFileW(drop,0xFFFFFFFF,nullptr,0);
        if(!Blocked())for(UINT i=0;i<std::min<UINT>(count,31);i++) {
            UINT length=DragQueryFileW(drop,i,nullptr,0);std::vector<wchar_t> path(length+1);
            DragQueryFileW(drop,i,path.data(),length+1);OpenContentPath(path.data());
        }
        if(count>31)importError=T(L"Tối đa 31 file mỗi lần.",L"Up to 31 files per drop.");
        DragFinish(drop);Layout();return 0;
    }
    case WM_COMMAND:
        if(LOWORD(wp)==SIZE_INPUT&&HIWORD(wp)==EN_KILLFOCUS){CommitSizeInput();return 0;}
        if((LOWORD(wp)==SOURCE_MONITOR || LOWORD(wp)==DEST_MONITOR) && HIWORD(wp)==CBN_SELCHANGE) {
            documentHost.Hide();if(mirror)mirror->Invalidate();Layout();return 0;
        }
        if(LOWORD(wp)==FIND_INPUT && HIWORD(wp)==EN_CHANGE){findMessage.clear();return 0;}
        if (LOWORD(wp) == EDITOR && HIWORD(wp) == EN_CHANGE && !loading && !viewingFile)
        {
            dirty = true;scriptDraft=TextOf(editor);documents.Get(0)->content.text=scriptDraft;
            SetTimer(h, SAVE_TIMER, 900, nullptr);
            InvalidateRect(h, nullptr, FALSE);
        }
        if (HIWORD(wp) != BN_CLICKED)
            return 0;
        switch (LOWORD(wp))
        {
        case CLOSE_APPLICATION:PostMessageW(h,WM_CLOSE,0,0);break;
        case NAV_HOME:
            Pause(); focusMode = false; page = Page::Home; Layout(); break;
        case NAV_PRESENTATION:
            Pause();focusMode=false;page=Page::Presentation;Layout();break;
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
        case VERIFY_PRIVACY:VerifyPrivacy();break;
        case MIRROR_START:
            if(!mirror || masked)break;
            documentHost.Hide();verifying=true;Layout();
            mirror->Start(SelectedMonitor(sourceMonitor),SelectedMonitor(destinationMonitor),mirrorPointer);
            verifying=false;Layout();break;
        case MIRROR_STOP:documentHost.Hide();if(mirror)mirror->Stop();Layout();break;
        case POINTER:mirrorPointer=!mirrorPointer;UpdateButtons();break;
        case OPEN_FOLDER:ChooseFolder();break;
        case OVERFLOW:OverflowMenu();break;
        case FIND:if(!Blocked()&&!IsImage()&&!reloadPending){findVisible=true;Layout();SetFocus(findInput);}break;
        case FIND_NEXT:RunFind(true);break;
        case FIND_PREV:RunFind(false);break;
        case FIND_CLOSE:findVisible=false;Layout();SetFocus(ContentWindow());break;
        case PDF_PREV:if(!Blocked()&&IsPdf())pdfViewer.GoToPage(pdfViewer.GetState().page-1);UpdateButtons();break;
        case PDF_NEXT:if(!Blocked()&&IsPdf())pdfViewer.GoToPage(pdfViewer.GetState().page+1);UpdateButtons();break;
        case FIT_WIDTH:if(!Blocked()&&IsPdf())pdfViewer.FitWidth();UpdateButtons();break;
        case WRAP:if(!Blocked()&&IsCode()){codeViewer.SetWrap(!codeViewer.GetState().wrap);UpdateButtons();}break;
        case CLOSE_TAB:CloseDocument(documents.Active().id);break;
        case CLOSE_OTHERS:if(!Blocked()){SaveActiveState();documentHost.CloseExternal();documents.CloseOthers(documents.Active().id);RenderDocument();}break;
        case CLOSE_FILES:if(!Blocked()){SaveActiveState();documentHost.CloseExternal();documents.CloseAllFiles();viewingFile=false;reading=false;RenderDocument();}break;
        case SHOW_TREE:treeVisible=!treeVisible;Layout();break;
        case INCLUDE_IGNORED:if(!Blocked())projectTree.SetIncludeIgnored(!projectTree.IncludeIgnored(),importError);UpdateButtons();break;
        case RELOAD_FILE:
            if(!Blocked()&&reloadPending){SaveActiveState();documentHost.CloseExternal();documents.Reload(reloadPending,importError);reloadPending=0;RenderDocument();}break;
        case KEEP_FILE:reloadPending=0;Layout();break;
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
        else if(wp==3)RecallWindow();
        return 0;
    case WM_POWERBROADCAST:
        if(wp==PBT_APMSUSPEND||wp==PBT_APMRESUMEAUTOMATIC||wp==PBT_APMRESUMESUSPEND)RefreshPrivacy();
        else if(wp==PBT_POWERSETTINGCHANGE&&lp) {
            const auto *setting=(const POWERBROADCAST_SETTING*)lp;
            constexpr GUID consoleState{0x6fe69556,0x704a,0x47a0,{0x8f,0x24,0xc2,0x8d,0x93,0x6f,0xda,0x47}};
            constexpr GUID sessionState{0x2b84c20e,0xad23,0x4ddf,{0x93,0xdb,0x05,0xff,0xbd,0x7e,0xfc,0xa5}};
            constexpr GUID monitorState{0x02731015,0x4510,0x4526,{0x99,0xe6,0xe5,0xa1,0x7e,0xbd,0x1a,0xea}};
            DWORD state=1;
            if(setting->DataLength==sizeof(state))memcpy(&state,setting->Data,sizeof(state));
            if(state==0&&(IsEqualGUID(setting->PowerSetting,consoleState)||IsEqualGUID(setting->PowerSetting,sessionState)||IsEqualGUID(setting->PowerSetting,monitorState)))RefreshPrivacy();
        }
        return TRUE;
    case WM_DEVICECHANGE: {
        const auto current=EnumerateMonitors();bool changed=current.size()!=monitors.size();
        for(const auto &a:current) {
            const auto found=std::find_if(monitors.begin(),monitors.end(),[&](const MonitorInfo &b){return a.stableId==b.stableId;});
            if(found==monitors.end()||!EqualRect(&a.bounds,&found->bounds)||a.adapter.LowPart!=found->adapter.LowPart||a.adapter.HighPart!=found->adapter.HighPart)changed=true;
        }
        if(changed){RefreshPrivacy();UpdateMonitors();}
        else if(mirror)mirror->Tick();
        return TRUE;
    }
    case WM_DISPLAYCHANGE:
    case WM_DWMCOMPOSITIONCHANGED:
        RefreshPrivacy();UpdateMonitors();
        return 0;
    case WM_TIMER:
        if(wp==5 && mirror) {
            mirror->Tick();
            if(renderedSafe!=IsPrivateContentSafe()) {
                if(Blocked()) {
                    documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);
                    if(filePicker){ShowWindow(filePicker,SW_HIDE);PostMessageW(filePicker,WM_COMMAND,IDCANCEL,0);}
                }
                Layout();
            }
            static ULONGLONG lastStatusPaint=0;
            if(page==Page::Presentation && GetTickCount64()-lastStatusPaint>=250) {
                lastStatusPaint=GetTickCount64();InvalidateRect(h,nullptr,FALSE);
            }
        }
        else if (wp == SCROLL_TIMER)
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
        else if (wp == QA_TIMER && qa)
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
        for (UINT_PTR timer : {PRIVACY_TIMER, SCROLL_TIMER, SAVE_TIMER, QA_TIMER,(UINT_PTR)5})
            KillTimer(h, timer);
        if(mirror)mirror->Stop();
        documentHost.CloseExternal();
        if (f8Registered)
            UnregisterHotKey(h, 2);
        if(recallRegistered)UnregisterHotKey(h,3);
        if (underlay)
            DestroyWindow(underlay);
        window=nullptr;editor=nullptr;
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
#ifndef QUIETSHARE_QA
    if(count>1){LocalFree(args);return 2;}
#endif
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
    {
        dataDir = GetLocalDataDirectory();
        page = Page::Home;
    }
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
            HWND existing = FindWindowW(L"QuietSharePrivate", nullptr);
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
    BOOL privatePreview=TRUE;
    DwmSetWindowAttribute(window,DWMWA_FORCE_ICONIC_REPRESENTATION,&privatePreview,sizeof(privatePreview));
    DwmSetWindowAttribute(window,DWMWA_HAS_ICONIC_BITMAP,&privatePreview,sizeof(privatePreview));
    DwmSetWindowAttribute(window,DWMWA_DISALLOW_PEEK,&privatePreview,sizeof(privatePreview));
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
    if(!qa)PostMessageW(window,VerifyPrivacyMessage,0,0);
    if (!qa && !Blocked())
        SetFocus(GetDlgItem(window, OPEN_DROP));
    MSG message{};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0)
    {
        if(message.message==WM_KEYDOWN) {
            bool control=(GetKeyState(VK_CONTROL)&0x8000)!=0,shift=(GetKeyState(VK_SHIFT)&0x8000)!=0;
            if(control && message.wParam==L'O'){shift?ChooseFolder():ChooseFile();continue;}
            if(control && message.wParam==L'F'){SendMessageW(window,WM_COMMAND,FIND,0);continue;}
            if(control && message.wParam==L'W'){CloseDocument(documents.Active().id);continue;}
            if(control && message.wParam==VK_TAB && !Blocked()) {
                SaveActiveState();documentHost.CloseExternal();documents.Cycle(shift?-1:1,importError);
                viewingFile=documents.Active().id!=0;RenderDocument();continue;
            }
            if(control && message.wParam==L'C' && IsPdf()) {if(!Blocked())pdfViewer.CopySelection();continue;}
            if(control && message.wParam==L'0'){ResetFont();continue;}
            if(control && (message.wParam==VK_ADD || message.wParam==VK_OEM_PLUS)){AdjustFont(2);continue;}
            if(control && (message.wParam==VK_SUBTRACT || message.wParam==VK_OEM_MINUS)){AdjustFont(-2);continue;}
            if(message.hwnd==findInput && message.wParam==VK_RETURN){RunFind(!shift);continue;}
            if(message.wParam==VK_ESCAPE && findVisible){findVisible=false;Layout();SetFocus(ContentWindow());continue;}
            if(message.wParam==VK_SPACE && reading && !viewingFile && message.hwnd!=findInput && !Blocked()){ToggleRun();continue;}
        }
        if (message.message == WM_KEYDOWN && message.wParam == VK_F11)
        { ToggleFocus(); continue; }
        if (message.message == WM_KEYDOWN && message.wParam == VK_ESCAPE && focusMode)
        { ToggleFocus(); continue; }
        if (message.message == WM_KEYDOWN && message.wParam == VK_TAB)
            SendMessageW(window, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
        if (message.message == WM_KEYDOWN && message.wParam == VK_F8 && !f8Registered && !(message.lParam & (1L<<30)))
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
