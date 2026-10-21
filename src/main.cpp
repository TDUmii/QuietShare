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
#include "Workspace.h"
#include "FileIcons.h"
#include "LocalPicker.h"
#include "Browser.h"
#include "MeetingRecorder.h"
#include "MeetingTranscriber.h"
#include "MeetingSettings.h"
#include "MeetingTranscriptImport.h"
#include "CustomTheme.h"
#include "Version.h"
#include "Preferences.h"
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
#include <gdiplus.h>

using namespace quietshare;
namespace
{
// QuietShare design tokens. Keep these values in one place so the native shell,
// owner-drawn controls and RichEdit surface stay visually consistent.
const COLORREF& BG=theme.bg;
const COLORREF& SIDEBAR=theme.sidebar;
const COLORREF& CARD=theme.card;
const COLORREF& ELEVATED=theme.elevated;
const COLORREF& HOVER=theme.hover;
const COLORREF& ACCENT=theme.accent;
const COLORREF& ACCENT_DARK=theme.selection;
const COLORREF& TEXT=theme.text;
const COLORREF& MUTED=theme.muted;
const COLORREF& DIVIDER=theme.border;
const COLORREF& GREEN=theme.green;
const COLORREF& AMBER=theme.amber;
const COLORREF& DISABLED=theme.disabled;
const COLORREF& PIN_BG=theme.pin;
const COLORREF& HOME_BG=theme.home;
const COLORREF& HOME_CARD=theme.homeCard;
const COLORREF& HOME_RAIL=theme.rail;
constexpr int MODE = 100, PLAY = 101, SMALLER = 102, LARGER = 103, SPEED = 104, MASK = 105, TOPMOST = 106,
              LANGUAGE = 107, RESET_ZOOM = 108, EDITOR = 110, NAV_SCRIPT = 120, NAV_READ = 121,
              NAV_HOME = 122, NAV_SETTINGS = 123, OPEN_FILE = 124, FOCUS = 125, OPEN_DROP = 126,
              HOME_SCRIPT = 127, HOME_SETTINGS = 128, VIEW_FILE = 129, OPEN_FOLDER = 131, OVERFLOW = 132,
              FIND = 133, FIND_NEXT = 134, FIND_PREV = 135, FIND_CLOSE = 136,
              PDF_PREV = 137, PDF_NEXT = 138, NAV_PRESENTATION = 139,
              VERIFY_PRIVACY = 140, MIRROR_START = 141, MIRROR_STOP = 142, POINTER = 143,
              RELOAD_FILE = 144, KEEP_FILE = 145, WRAP = 146, FIT_WIDTH = 147,
              CLOSE_TAB = 148, CLOSE_OTHERS = 149, CLOSE_FILES = 150, SHOW_TREE = 151,
              INCLUDE_IGNORED = 152, CLOSE_APPLICATION = 153, NAV_BROWSER=154, BROWSER_TOGGLE=155,
              WEB_BACK=156,WEB_FORWARD=157,WEB_RELOAD=158,WEB_CHATGPT=159,
              SOURCE_MONITOR = 160, DEST_MONITOR = 161, FIND_INPUT = 162, SIZE_INPUT = 163, WEB_ADDRESS=164,WEB_GO=165,WEB_GOOGLE=166,
              WEB_ZOOM_OUT=167,WEB_ZOOM_RESET=168,WEB_ZOOM_IN=169,
              LAYOUT_MENU=180,SHOW_ACTIVITY=181,SHOW_TABS=182,SHOW_TOOLBAR=183,SHOW_STATUS=184,RESET_LAYOUT=185,
              OPEN_DISPLAY_SETTINGS=186,SETTINGS_SCROLL=187,PIN_WINDOW=188,RESOURCE_MODE=189,
              WEB_NEW_TAB=190,HOME_VERSION=192,SETTINGS_APPEARANCE=200,SETTINGS_READING=201,
              SETTINGS_WORKSPACE=202,SETTINGS_PROJECTION=203,SETTINGS_KEYS=204,OPACITY_INPUT=205,
              TEXT_WEIGHT=206,RESET_PREFERENCES=207,MIRROR_ACTION=211,WEB_MENU=212,WEB_AUDIO=213,MEETING_RECORD=214,SHORTCUT_FIRST=220,
              SETTINGS_BROWSER=240,ICON_SIZE_INPUT=241,OPACITY_LESS=242,OPACITY_MORE=243,
              ICON_SMALLER=244,ICON_LARGER=245,BROWSER_MUTED=246,BROWSER_CAMERA=247,BROWSER_MICROPHONE=248,REMEMBER_SIZE=249,ALLOW_EDIT=250,SAVE_FILE=251,RECENT_CLEAR=252,RECENT_NEXT=253,RECENT_PREV=254,THEME_SELECT=255,RECENT_FIRST=400;
constexpr int TRANSCRIPTION_LANGUAGE=256,SCRIPT_TAB_LABELS=257,BROWSER_TAB_LABELS=258,
              MEETING_TRANSCRIBE=259,MEETING_PROVIDER=260,MEETING_OUTPUT=261,MEETING_DIRECTORY=262,
              MEETING_IMPORT=263,CUSTOM_BACKGROUND=264,CUSTOM_PANEL=265,CUSTOM_TEXT=266,CUSTOM_ACCENT=267,
              CUSTOM_RESET=268,TRANSCRIPTION_VI=270,TRANSCRIPTION_EN=271,
              PROVIDER_LOCAL=272,PROVIDER_TEAMS=273,PROVIDER_ZOOM=274,PROVIDER_MEET=275,
              OUTPUT_WAV=276,OUTPUT_TRANSCRIPT=277;
constexpr int NORMAL_SIDEBAR = 36, COMPACT_SIDEBAR = 36;
constexpr UINT_PTR PRIVACY_TIMER = 1, SCROLL_TIMER = 2, SAVE_TIMER = 3, QA_TIMER = 4, MEETING_TIMER = 6;
HINSTANCE instance;
HWND window, editor, underlay, tooltips, hoveredButton, imageViewer, settingsScroll, filePicker,
     findInput, sourceMonitor, destinationMonitor, sizeInput, homeVersion;
bool updatingSize=false;
DocumentId sizeInputDocument=0;
std::wstring pickerFolder;
RecentWorkspace recentWorkspace;
size_t recentOffset=0;
int recentRows=3;
RECT homeRecentRect{},homeProtectionRect{};
int homeRowHeight=56;bool homeCompact=false;
bool closeConfirmation=false;

std::wstring sizeError;
TabManager documents;
DocumentBar documentBar;
DocumentBar browserTabBar;
ProjectTree projectTree;
Browser browser;
MeetingRecorder meetingRecorder;
MeetingTranscriber meetingTranscriber;
MeetingSettings meetingSettings;
std::wstring meetingRecordingPath;
unsigned displayedBrowserTab=0;
bool updatingWebAddress=false,webAddressEdited=false;
HWND webAddress=nullptr;
HWND opacityInput=nullptr,iconSizeInput=nullptr;
Preferences preferences=DefaultPreferences();
PreferenceGlobalHotkeys globalHotkeys;
int settingsCategory=0,capturingShortcut=-1;
bool updatingOpacity=false,updatingIconSize=false;
std::wstring preferenceError;
std::wstring transcriptionNotice;
bool transcriptionHandled=false;
DocumentHost documentHost;
CodeViewer &codeViewer=documentHost.code;
PdfViewer &pdfViewer=documentHost.pdf;
std::unique_ptr<MirrorController> mirror;
std::vector<MonitorInfo> monitors;
bool findVisible = false, treeVisible = true, mirrorPointer = true, verifying = false, renderedSafe = false;
bool activityVisible=true,tabsVisible=true,toolbarVisible=true,statusVisible=true,splitterDragging=false,splitterHovered=false;
int explorerWidth=208,resourceMode=2,mirrorTimerInterval=0;
HMONITOR labeledWindowMonitor=nullptr;
RECT explorerRect{},splitterRect{};
DocumentId reloadPending = 0;
std::wstring viewerError, findMessage;
constexpr UINT VerifyPrivacyMessage = WM_APP + 12;
HFONT uiFont, titleFont, brandFont, metaFont;
HBRUSH bgBrush, cardBrush,numberBrush;
UINT dpi = 96;
PrivacyResult privacy;
bool topologyKnown = false, cloned = true, masked = false, reading = false, running = false, loading = false,
     dirty = false, saveFailed = false, f8Registered = false, savedNoteRecovered = false,
     storageBlocked = false, topmost = true, english = false, recallRegistered = false;
int fontPoints = DefaultTextPoints, speed = 1;
double scrollRemainder = 0;
ULONGLONG lastScrollTick = 0;
std::wstring dataDir;
enum class Page { Home, Content, Settings, Presentation, Browser };
Page page = Page::Content;
LoadedContent &ActiveContent() { return documents.Active().content; }
bool viewingFile = false, focusMode = false, scriptReading = false;
int codePoints = DefaultCodePoints, imageZoom = DefaultZoomPercent, panX = 0, panY = 0;
std::wstring scriptDraft, importError;
RECT dropRect{}, settingsCardRect{};
RECT fontLabelRect{};
RECT contentCardRect{};
RECT settingsViewport{},settingsSizeHint{};
struct SettingsLine {std::wstring text;RECT bounds;bool heading;int control=0;};
std::vector<SettingsLine> settingsLines;
struct NumericSetting {HWND input;int decrement,increment;RECT bounds;};
std::vector<NumericSetting> numericSettings;
std::vector<HWND> clippedSettingsControls;
int settingsOffset=0,settingsMaximum=0,settingsDisplayAnchor=0,settingsWheelRemainder=0;
bool settingsThumbDragging=false;
int settingsThumbGrab=0,settingsPointerY=0,settingsRepeatCode=-1;
int railWidth = NORMAL_SIDEBAR, contentLeft = NORMAL_SIDEBAR + 24, contentRight = 0;
bool compactUi = false;
std::vector<HWND> buttons;
WNDPROC editorProc;
bool qa = false, qaVisual = false, qaStorage = false, qaInteractive = false, qaCaptureOk = true, qaPass = true;
bool projectionStoppedNotice=false,shuttingDown=false;
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

void UpdateProjectionNoticeCaption()
{
    if(window&&IsWindow(window))SetWindowTextW(window,projectionStoppedNotice?
        T(L"QuietShare - Trình chiếu đã dừng",L"QuietShare - Projection stopped"):L"QuietShare");
}
void UpdateProjectionStopButton()
{
    if(window&&IsWindow(window))EnableWindow(GetDlgItem(window,MIRROR_STOP),projectionStoppedNotice||(mirror&&mirror->Running()));
}
void ClearProjectionStoppedNotice()
{
    if(!projectionStoppedNotice)return;
    projectionStoppedNotice=false;UpdateProjectionNoticeCaption();UpdateProjectionStopButton();
    if(window&&IsWindow(window)&&!qa) {
        FLASHWINFO flash{sizeof(flash),window,FLASHW_STOP,0,0};FlashWindowEx(&flash);
    }
}
void ShowProjectionStoppedNotice()
{
    if(shuttingDown||masked||projectionStoppedNotice||!window||!IsWindow(window))return;
    projectionStoppedNotice=true;UpdateProjectionNoticeCaption();UpdateProjectionStopButton();InvalidateRect(window,nullptr,FALSE);
    // Fixed public text, sound and taskbar attention never restore or focus the
    // private window. The reason remains available on Overview/Presentation.
    if(!qa) {
        MessageBeep(MB_ICONEXCLAMATION);
        FLASHWINFO flash{sizeof(flash),window,FLASHW_TRAY,3,0};FlashWindowEx(&flash);
    }
}

void ApplyTextFormat();
void ApplyTextWrap();
void SyncSizeInput(bool force=false);
void CommitSizeInput();
void Pause();
void Layout();
void ScrollSettings(int position);
void EnsureSettingsVisible(HWND control);
void SetReading(bool value);
void ShowScript();
bool SaveActiveState();
bool SaveFile();
bool ConfirmFile(DocumentId id);
bool ConfirmFiles(DocumentId keep=static_cast<DocumentId>(-1));
void ActivateDocument(DocumentId id);
void CloseDocument(DocumentId id);
void ChooseFolder();
void OverflowMenu();
void VerifyPrivacy();
void UpdateDocumentBar();
void UpdateMonitors();
void UpdateMonitorLabels();
void UpdateMirrorTimer();
void HandleTranscriptionResult();
void ApplyWindowAppearance();
void PersistRecent();
bool StorePreferences(const Preferences &candidate);
void CommitOpacity();
void CommitIconSize();
void CaptureShortcut(UINT key,UINT modifiers);
bool DispatchPreferenceShortcut(UINT key,UINT modifiers);
void RunPreferenceCommand(PreferenceCommand command);
UINT ShortcutModifiers();
void RunFind(bool forward);
std::wstring SelectedMonitor(HWND combo);
bool OpenContentPath(const std::wstring &path);
void ChooseMeetingDirectory();
void ImportMeetingFile();
void ChooseCustomColor(int controlId);
bool IsImage() { return viewingFile && ActiveContent().kind == ContentKind::Image; }
bool IsCode() { return viewingFile && (ActiveContent().kind == ContentKind::Code || documents.Active().language==L"md" || documents.Active().language==L"markdown"); }
bool IsPdf() { return viewingFile && ActiveContent().kind == ContentKind::Pdf; }
HWND ContentWindow() { return IsImage()?imageViewer:IsCode()?codeViewer.Window():IsPdf()?pdfViewer.Window():editor; }
int &ActiveTextSize() { return IsCode() ? codePoints : fontPoints; }

std::wstring ColorHex(COLORREF color)
{
    wchar_t value[16]{};
    swprintf_s(value,L"#%02X%02X%02X",GetRValue(color),GetGValue(color),GetBValue(color));
    return value;
}

const wchar_t *MeetingExportLabel(MeetingExportMode mode,bool english)
{
    return mode==MeetingExportMode::TranscriptOnly?
        (english?L"Transcript only (remove WAV after success)":L"Chỉ nội dung (xóa WAV sau khi thành công)"):
        (english?L"WAV + transcript":L"WAV + nội dung");
}

int S(int value)
{
    return MulDiv(value, dpi, 96);
}
void CenterInputText(HWND control)
{
    RECT bounds{};GetClientRect(control,&bounds);
    HDC dc=GetDC(control);if(!dc)return;
    HFONT font=(HFONT)SendMessageW(control,WM_GETFONT,0,0);
    auto old=font?SelectObject(dc,font):nullptr;TEXTMETRICW metrics{};GetTextMetricsW(dc,&metrics);
    if(old)SelectObject(dc,old);ReleaseDC(control,dc);
    RECT format{S(6),std::max(0L,(bounds.bottom-metrics.tmHeight)/2),
        std::max(S(7),int(bounds.right)-S(6)),0};
    format.bottom=format.top+metrics.tmHeight;
    RECT previous{};SendMessageW(control,EM_GETRECT,0,(LPARAM)&previous);
    if(!EqualRect(&format,&previous)){SendMessageW(control,EM_SETRECTNP,0,(LPARAM)&format);InvalidateRect(control,nullptr,FALSE);}
}
bool StepperButton(int id)
{
    return id==OPACITY_LESS||id==OPACITY_MORE||id==ICON_SMALLER||id==ICON_LARGER||
           (page==Page::Settings&&(id==SMALLER||id==LARGER));
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
bool AppendMeetingTranscript(const std::wstring &text,const std::wstring &sourcePath)
{
    if(text.empty()) {
        transcriptionNotice=T(L"Không nhận ra lời nói trong bản ghi.",L"No speech was recognized in the recording.");
        return false;
    }
    if(!viewingFile)SaveActiveState();
    if(!scriptDraft.empty())scriptDraft+=L"\r\n\r\n";
    scriptDraft+=text;
    if(auto* script=documents.Get(0))script->content.text=scriptDraft;
    if(!viewingFile){
        POINT scroll{};SendMessageW(editor,EM_GETSCROLLPOS,0,(LPARAM)&scroll);
        loading=true;SetWindowTextW(editor,scriptDraft.c_str());loading=false;
        SendMessageW(editor,EM_SETSCROLLPOS,0,(LPARAM)&scroll);ApplyTextFormat();ApplyTextWrap();
    }
    if(!SaveNotes()){
        transcriptionNotice=T(L"Đã chép lời nhưng không lưu được Lời nhắc.",L"Transcript created, but the notes could not be saved.");
        return false;
    }
    if(!sourcePath.empty()) {
        recentWorkspace.Touch(sourcePath,false);PersistRecent();
    }
    const auto slash=sourcePath.find_last_of(L"\\/");
    const auto name=slash==std::wstring::npos?sourcePath:sourcePath.substr(slash+1);
    transcriptionNotice=T(L"Đã chép lời vào Lời nhắc",L"Transcript added to notes");
    if(!name.empty())transcriptionNotice+=L" · "+name;
    return true;
}
void HandleTranscriptionResult()
{
    std::wstring text,path;
    if(!meetingTranscriber.TakeResult(text,path))return;
    if(meetingTranscriber.ErrorCode()!=0){transcriptionNotice=meetingTranscriber.Status(english);return;}
    if(AppendMeetingTranscript(text,path) && meetingSettings.exportMode==MeetingExportMode::TranscriptOnly && !path.empty()) {
        // Recognition currently consumes a finalized WAV. Transcript-only mode
        // removes that temporary capture only after the text has been saved.
        if(!DeleteFileW(path.c_str()))
            transcriptionNotice+=T(L" · Không xóa được WAV tạm",L" · Temporary WAV could not be deleted");
        else
            meetingRecordingPath.clear();
    }
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
    return L"";
}
bool IsPrivateContentSafe()
{
    // QA uses only synthetic fixtures; production has no privacy override.
    return !masked && !cloned && topologyKnown && privacy.applied && !storageBlocked &&
           !verifying && (qa || (mirror && mirror->IsVerified() && !mirror->Busy()));
}
bool Blocked() { return !IsPrivateContentSafe(); }
void SaveLayoutPreferences()
{
    if(qa)return;
    const DWORD values[]={2,activityVisible,tabsVisible,toolbarVisible,statusVisible,treeVisible,(DWORD)explorerWidth,(DWORD)resourceMode};
    const auto temporary=dataDir+L"\\layout.tmp";
    if(WriteBytes(temporary,values,sizeof(values)))
        MoveFileExW(temporary.c_str(),(dataDir+L"\\layout.dat").c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
}
void LoadLayoutPreferences()
{
    if(qa)return;
    const auto bytes=ReadBytes(dataDir+L"\\layout.dat");DWORD values[8]{};
    if(bytes.size()!=7*sizeof(DWORD)&&bytes.size()!=sizeof(values))return;
    CopyMemory(values,bytes.data(),bytes.size());
    if((values[0]!=1&&values[0]!=2)||values[6]<112||values[6]>420)return;
    if(values[0]==1&&bytes.size()!=7*sizeof(DWORD))return;
    if(values[0]==2&&(bytes.size()!=sizeof(values)||values[7]>2))return;
    for(int i=1;i<=5;i++)if(values[i]>1)return;
    activityVisible=values[1];tabsVisible=values[2];toolbarVisible=values[3];statusVisible=values[4];treeVisible=values[5];explorerWidth=values[6];
    if(values[0]==2)resourceMode=values[7];
}
void UpdateMirrorTimer()
{
    if(!window||qa||!mirror)return;
    const int interval=mirror->Running()?mirror->TickIntervalMs():250;
    if(interval!=mirrorTimerInterval){mirrorTimerInterval=interval;SetTimer(window,5,interval,nullptr);}
}
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
    sizeError.clear();Pause();
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
    SyncSizeInput(true);
}
void ResetFont()
{
    if (Blocked())
        return;
    sizeError.clear();Pause();
    if (IsPdf()) {pdfViewer.FitPage();pdfViewer.SetZoom(DefaultZoomPercent);}
    else if (IsImage())
    {
        imageZoom = DefaultZoomPercent;
        panX = panY = 0;
        InvalidateRect(imageViewer, nullptr, FALSE);
    }
    else
        ActiveTextSize() = IsCode() ? DefaultCodePoints : DefaultTextPoints;
    if(IsCode())codeViewer.SetZoom(DefaultZoomPercent);
    ApplyTextFormat();
    SyncSizeInput(true);
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
    // GDI RoundRect has binary coverage; GDI+ keeps small switch curves smooth
    // at native resolution without allocating a supersampled bitmap per paint.
    static struct Smoothing {
        ULONG_PTR token=0;
        Smoothing(){Gdiplus::GdiplusStartupInput input;input.SuppressExternalCodecs=TRUE;
            if(Gdiplus::GdiplusStartup(&token,&input,nullptr)!=Gdiplus::Ok)token=0;}
        ~Smoothing(){if(token)Gdiplus::GdiplusShutdown(token);}
    } smoothing;
    const int width=rect.right-rect.left,height=rect.bottom-rect.top;
    if(width<=0||height<=0)return;
    if(smoothing.token) {
        const Gdiplus::REAL x=(Gdiplus::REAL)rect.left,y=(Gdiplus::REAL)rect.top;
        const Gdiplus::REAL w=(Gdiplus::REAL)width,h=(Gdiplus::REAL)height;
        const Gdiplus::REAL d=(Gdiplus::REAL)std::clamp(radius,1,std::min(width,height));
        Gdiplus::Graphics graphics(dc);graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        Gdiplus::GraphicsPath path;
        path.AddArc(x,y,d,d,180,90);path.AddArc(x+w-d,y,d,d,270,90);
        path.AddArc(x+w-d,y+h-d,d,d,0,90);path.AddArc(x,y+h-d,d,d,90,90);path.CloseFigure();
        Gdiplus::SolidBrush fill(Gdiplus::Color(255,GetRValue(color),GetGValue(color),GetBValue(color)));
        if(graphics.FillPath(&fill,&path)==Gdiplus::Ok)return;
    }
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
    HBRUSH brush = CreateSolidBrush(page==Page::Home?HOME_RAIL:SIDEBAR);
    FillRect(dc, &rail, brush);
    DeleteObject(brush);
    RECT divider{S(sidebar - 1), 0, S(sidebar), height};
    brush = CreateSolidBrush(DIVIDER);
    FillRect(dc, &divider, brush);
    DeleteObject(brush);

    if(compact) {
        RECT separator{S(8),S(124),S(sidebar-8),S(125)};
        HBRUSH line=CreateSolidBrush(DIVIDER);FillRect(dc,&separator,line);DeleteObject(line);
    }
    if (!compact)
    {
        RECT brand{S(56), S(20), S(sidebar - 8), S(52)};
        DrawTextAt(dc, L"QuietShare", brand, TEXT, uiFont);
    }

    int logicalHeight = MulDiv(height, 96, dpi);
    if (!compact && logicalHeight < 360)
        return;
    int statusTop = logicalHeight - (compact ? 22 : 80);
    RECT status{S(12), S(statusTop), S(sidebar - 12), height - S(16)};
    if (!compact)
        FillRoundRect(dc, status, CARD, S(16));
    bool safe = !Blocked();
    COLORREF stateColor = safe ? GREEN : masked ? MUTED : AMBER;
    int dotX = compact ? (sidebar-8)/2 : 24;
    const int dotTop=compact?height-S(14):status.top+S(16);
    RECT dot{S(dotX),dotTop,S(dotX+8),dotTop+S(8)};
    FillRoundRect(dc,dot,stateColor,S(8));
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
                   masked ? ShortcutLabel(preferences.Key(PreferenceCommand::Mask))+T(L" để mở lại",L" to reveal")
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
void UpdateBrowserZoomControls()
{
    const auto value=std::to_wstring(browser.ZoomPercent())+L"%";
    if(TextOf(GetDlgItem(window,WEB_ZOOM_RESET))!=value)SetWindowTextW(GetDlgItem(window,WEB_ZOOM_RESET),value.c_str());
    EnableWindow(GetDlgItem(window,WEB_ZOOM_RESET),browser.Ready()&&!Blocked());
    EnableWindow(GetDlgItem(window,WEB_ZOOM_OUT),browser.Ready()&&!Blocked()&&browser.ZoomPercent()>25);
    EnableWindow(GetDlgItem(window,WEB_ZOOM_IN),browser.Ready()&&!Blocked()&&browser.ZoomPercent()<400);
}
void UpdateButtons()
{
    browser.SetEnglish(english);
    std::vector<BrowserCommandShortcut> webShortcuts;
    for(unsigned i=0;i<PreferenceCommandCount;++i) {
        if(i<2&&globalHotkeys.Registered(static_cast<PreferenceCommand>(i)))continue;
        if(static_cast<PreferenceCommand>(i)==PreferenceCommand::AutoScroll)continue;
        webShortcuts.push_back({preferences.shortcuts[i].key,preferences.shortcuts[i].modifiers,i+1});
    }
    webShortcuts.push_back({L'L',MOD_CONTROL,PreferenceCommandCount+1});
    browser.SetCommandShortcuts(webShortcuts);
    if(!Blocked()&&browser.Enabled()) {
        const bool changed=displayedBrowserTab!=browser.ActiveTabId();
        if(changed||(!webAddressEdited&&GetFocus()!=webAddress)) {
            updatingWebAddress=true;SetWindowTextW(webAddress,browser.Address().c_str());updatingWebAddress=false;
        }
        if(changed)webAddressEdited=false;
        displayedBrowserTab=browser.ActiveTabId();
    }
    SetWindowTextW(GetDlgItem(window,BROWSER_TOGGLE),T(browser.Enabled()?L"Tắt trình duyệt":L"Bật trình duyệt",
        browser.Enabled()?L"Disable browser":L"Enable browser"));
    SetTip(BROWSER_TOGGLE,T(browser.Enabled()?L"Tắt trình duyệt":L"Bật trình duyệt",
        browser.Enabled()?L"Disable browser":L"Enable browser"));
    SetWindowTextW(GetDlgItem(window,WEB_AUDIO),T(preferences.browserMuted?L"Bật âm thanh":L"Tắt âm thanh",
        preferences.browserMuted?L"Unmute audio":L"Mute audio"));
    SetTip(WEB_AUDIO,T(preferences.browserMuted?L"Bật âm thanh web":L"Tắt âm thanh web",
        preferences.browserMuted?L"Unmute web audio":L"Mute web audio"));
    const bool transcribing=meetingTranscriber.Running();
    const bool willTranscribe=meetingSettings.transcriptionEnabled;
    SetWindowTextW(GetDlgItem(window,MEETING_RECORD),T(transcribing?L"Đang chép lời":meetingRecorder.Running()?L"Dừng ghi âm":willTranscribe?L"Ghi âm & chép lời":L"Ghi âm cuộc họp",
        transcribing?L"Transcribing":meetingRecorder.Running()?L"Stop meeting recording":willTranscribe?L"Record and transcribe meeting":L"Record meeting"));
    SetTip(MEETING_RECORD,T(transcribing?L"Đang chuyển âm thanh thành văn bản":meetingRecorder.Running()?L"Dừng ghi âm âm thanh máy":willTranscribe?L"Ghi âm âm thanh máy rồi chép lời":L"Ghi âm âm thanh máy, không chép lời",
        transcribing?L"Converting audio to text":meetingRecorder.Running()?L"Stop recording system audio":willTranscribe?L"Record system audio, then transcribe it":L"Record system audio without transcription"));
    SetWindowTextW(GetDlgItem(window,WEB_NEW_TAB),T(L"Tab web mới",L"New web tab"));
    EnableWindow(GetDlgItem(window,WEB_NEW_TAB),!Blocked()&&browser.CanNewTab());
    SetTip(WEB_NEW_TAB,T(L"Tab web mới · Ctrl+T",L"New web tab · Ctrl+T"));
    SetWindowTextW(GetDlgItem(window,WEB_GO),T(L"Mở",L"Go"));
    SetWindowTextW(GetDlgItem(window,WEB_ZOOM_OUT),L"−");SetWindowTextW(GetDlgItem(window,WEB_ZOOM_IN),L"+");
    UpdateBrowserZoomControls();
    const wchar_t* resourceNamesVi[]={L"Tiết kiệm · 15 FPS",L"Cân bằng · 30 FPS",L"Hiệu năng · 60 FPS"};
    const wchar_t* resourceNamesEn[]={L"Saver · 15 FPS",L"Balanced · 30 FPS",L"Performance · 60 FPS"};
    SetWindowTextW(GetDlgItem(window,RESOURCE_MODE),english?resourceNamesEn[resourceMode]:resourceNamesVi[resourceMode]);
    SetTip(RESOURCE_MODE,T(L"Bấm để đổi giới hạn FPS trình chiếu: 15 / 30 / 60. Trình duyệt tự nghỉ khi không xem.",
        L"Click to cycle presentation FPS: 15 / 30 / 60. Hidden browsing suspends automatically."));
    SetTip(WEB_ZOOM_OUT,T(L"Thu nhỏ web · Ctrl+-",L"Zoom out · Ctrl+-"));
    SetTip(WEB_ZOOM_IN,T(L"Phóng to web · Ctrl++",L"Zoom in · Ctrl++"));
    SetTip(WEB_ZOOM_RESET,T(L"Đặt lại zoom web · Ctrl+0",L"Reset web zoom · Ctrl+0"));
    SetTip(OPACITY_LESS,T(L"Giảm độ mờ 1%",L"Decrease opacity by 1%"));SetTip(OPACITY_MORE,T(L"Tăng độ mờ 1%",L"Increase opacity by 1%"));
    SetTip(ICON_SMALLER,T(L"Giảm kích thước icon 1 px",L"Decrease icon size by 1 px"));SetTip(ICON_LARGER,T(L"Tăng kích thước icon 1 px",L"Increase icon size by 1 px"));
    SetWindowTextW(GetDlgItem(window,NAV_BROWSER),T(L"Trình duyệt",L"Browser"));
    for(int id:{WEB_ADDRESS,WEB_GO,WEB_RELOAD,WEB_AUDIO,WEB_MENU})EnableWindow(GetDlgItem(window,id),browser.Ready()&&!Blocked());
    EnableWindow(GetDlgItem(window,WEB_BACK),browser.Ready()&&!Blocked()&&browser.CanBack());
    EnableWindow(GetDlgItem(window,WEB_FORWARD),browser.Ready()&&!Blocked()&&browser.CanForward());
    EnableWindow(GetDlgItem(window,BROWSER_TOGGLE),browser.Enabled()||!Blocked());
    SetTip(NAV_BROWSER,T(L"Trình duyệt tùy chọn",L"Optional browser"));
    SetTip(WEB_BACK,T(L"Quay lại",L"Back"));SetTip(WEB_FORWARD,T(L"Tiến tới",L"Forward"));SetTip(WEB_RELOAD,T(L"Tải lại trang",L"Reload page"));
    SendMessageW(webAddress,EM_SETCUEBANNER,TRUE,(LPARAM)T(L"Tìm trên Google hoặc nhập địa chỉ web",L"Search Google or enter a web address"));
    SetWindowTextW(GetDlgItem(window,LAYOUT_MENU),T(L"Bố cục",L"Layout"));
    SetTip(LAYOUT_MENU,T(L"Bố cục: thanh bên, tab, công cụ, trạng thái",L"Layout: activity bar, tabs, toolbar and status bar"));
    SetTip(SHOW_TREE,T(L"Ẩn Explorer - Ctrl+B",L"Hide Explorer - Ctrl+B"));
    UpdateProjectionNoticeCaption();
    SetWindowTextW(GetDlgItem(window, MODE), T(reading ? L"Sửa lời" : L"Đọc", reading ? L"Edit" : L"Read"));
    SetWindowTextW(GetDlgItem(window, NAV_SCRIPT), T(L"Tệp đang mở", L"Open files"));
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
    const auto maskLabel=std::wstring(masked?T(L"Mở lại",L"Show"):T(L"Che",L"Hide"))+L" · "+ShortcutLabel(preferences.Key(PreferenceCommand::Mask));
    SetWindowTextW(GetDlgItem(window,MASK),maskLabel.c_str());
    SetWindowTextW(GetDlgItem(window, TOPMOST), topmost ? T(L"Ghim", L"Pin") : T(L"Thả", L"Free"));
    SetWindowTextW(GetDlgItem(window,PIN_WINDOW),T(topmost?L"Bỏ ghim cửa sổ":L"Ghim cửa sổ trên cùng",topmost?L"Unpin window":L"Pin window on top"));
    SetWindowTextW(GetDlgItem(window, LANGUAGE), english ? L"VI" : L"EN");
    int zoom=IsPdf()?pdfViewer.GetState().zoom:IsCode()?codeViewer.GetState().zoom:IsImage()?imageZoom:fontPoints*100/24;
    std::wstring zoomLabel=std::to_wstring(zoom)+L"%";
    SetWindowTextW(GetDlgItem(window, RESET_ZOOM),page==Page::Settings?T(L"Đặt lại",L"Reset"):zoomLabel.c_str());
    SetWindowTextW(GetDlgItem(window, OPEN_FOLDER),T(L"Mở dự án",L"Open project"));
    SetWindowTextW(GetDlgItem(window,SAVE_FILE),T(L"Lưu",L"Save"));
    SetWindowTextW(GetDlgItem(window,RECENT_CLEAR),T(L"Xóa lịch sử",L"Clear history"));
    SetWindowTextW(GetDlgItem(window,RECENT_PREV),T(L"Trang trước",L"Previous page"));SetWindowTextW(GetDlgItem(window,RECENT_NEXT),T(L"Trang sau",L"Next page"));
    SetWindowTextW(GetDlgItem(window,HOME_SETTINGS),T(L"Chi tiết",L"Details"));
    SetWindowTextW(GetDlgItem(window, VERIFY_PRIVACY),T(L"Kiểm tra WGC",L"Verify WGC privacy"));
    SetWindowTextW(GetDlgItem(window, MIRROR_START),T(L"Bắt đầu",L"Start"));
    SetWindowTextW(GetDlgItem(window, MIRROR_STOP),T(L"Dừng",L"Stop"));
    SetWindowTextW(GetDlgItem(window,MIRROR_ACTION),T(mirror&&mirror->Running()?L"Dừng trình chiếu":L"Bắt đầu trình chiếu",mirror&&mirror->Running()?L"Stop projection":L"Start projection"));
    SetTip(MIRROR_ACTION,T(mirror&&mirror->Running()?L"Dừng trình chiếu":L"Bắt đầu trình chiếu",mirror&&mirror->Running()?L"Stop projection":L"Start projection"));
    SetTip(VERIFY_PRIVACY,T(mirror&&mirror->Running()?L"Kiểm tra phiên WGC đang chiếu":L"Kiểm tra WGC",mirror&&mirror->Running()?L"Check the active WGC session":L"Verify WGC privacy"));
    SetWindowTextW(GetDlgItem(window, POINTER),T(mirrorPointer?L"Con trỏ: bật":L"Con trỏ: tắt",mirrorPointer?L"Pointer: on":L"Pointer: off"));
    SetWindowTextW(GetDlgItem(window, RELOAD_FILE),T(L"Tải lại",L"Reload"));
    SetWindowTextW(GetDlgItem(window, KEEP_FILE),T(L"Giữ bản đang xem",L"Keep current preview"));
    SetWindowTextW(GetDlgItem(window, INCLUDE_IGNORED),T(projectTree.IncludeIgnored()?L"Ẩn thư mục nặng / ẩn":L"Hiện thư mục nặng / ẩn",projectTree.IncludeIgnored()?L"Ignore heavy / hidden folders":L"Include heavy / hidden folders"));
    for(int id:{OPEN_FOLDER,RELOAD_FILE,KEEP_FILE,INCLUDE_IGNORED,FIND_NEXT,FIND_PREV,FIND_INPUT})
        EnableWindow(GetDlgItem(window,id),!Blocked());
    EnableWindow(GetDlgItem(window,VERIFY_PRIVACY),!masked&&!verifying&&!storageBlocked);
    EnableWindow(GetDlgItem(window,MIRROR_START),!Blocked()&&monitors.size()>1&&SelectedMonitor(sourceMonitor)!=SelectedMonitor(destinationMonitor));
    EnableWindow(GetDlgItem(window,MIRROR_ACTION),(mirror&&mirror->Running())||(!Blocked()&&monitors.size()>1&&SelectedMonitor(sourceMonitor)!=SelectedMonitor(destinationMonitor)));
    EnableWindow(GetDlgItem(window,MEETING_RECORD),!shuttingDown&&!transcribing);
    UpdateProjectionStopButton();
    for (int id : {MODE, PLAY, SMALLER, LARGER, SPEED, RESET_ZOOM, NAV_SCRIPT, NAV_READ, OPEN_FILE, OPEN_DROP,
                   HOME_SCRIPT, VIEW_FILE})
        EnableWindow(GetDlgItem(window, id), !Blocked());
    EnableWindow(GetDlgItem(window, SMALLER), !Blocked() && (IsPdf()?pdfViewer.GetState().zoom>25:IsCode()?codeViewer.GetState().zoom>25:IsImage() ? imageZoom > 25 : ActiveTextSize() > 8));
    EnableWindow(GetDlgItem(window, LARGER), !Blocked() && (IsPdf()?pdfViewer.GetState().zoom<400:IsCode()?codeViewer.GetState().zoom<400:IsImage() ? imageZoom < 400 : ActiveTextSize() < 200));
    EnableWindow(GetDlgItem(window, PLAY), !Blocked() && !IsImage() && !IsCode()&&!IsPdf());
    EnableWindow(GetDlgItem(window, SPEED), !Blocked() && !IsImage() && !IsCode()&&!IsPdf());
    SetWindowTextW(GetDlgItem(window,THEME_SELECT),ThemeName(preferences.themeId,english));
    SetWindowTextW(GetDlgItem(window,TEXT_WEIGHT),preferences.textWeight==700?T(L"Đậm",L"Bold"):preferences.textWeight==600?T(L"Vừa",L"Semibold"):T(L"Thường",L"Regular"));
    if(page==Page::Content) {
        SetWindowTextW(GetDlgItem(window,WRAP),T(L"Xuống dòng",L"Word wrap"));
        EnableWindow(GetDlgItem(window,WRAP),!Blocked()&&!IsImage()&&!IsPdf());
        EnableWindow(GetDlgItem(window,TEXT_WEIGHT),!Blocked()&&!IsCode()&&!IsImage()&&!IsPdf());
    }
    SetTip(SIZE_INPUT,T(L"Cỡ chữ (pt). Enter để áp dụng",L"Text size (pt). Enter to apply"));
    SetTip(TEXT_WEIGHT,T(L"Độ đậm chữ: thường / vừa / đậm",L"Text weight: regular / semibold / bold"));
    SetTip(WRAP,T(L"Bật / tắt tự xuống dòng",L"Toggle word wrap"));
    if(page==Page::Settings) {
        const wchar_t* vn[]={L"Giao diện",L"Nội dung",L"Bố cục",L"Trình chiếu",L"Phím tắt",L"Trình duyệt"};
        const wchar_t* en[]={L"Appearance",L"Reading",L"Workspace",L"Projection",L"Shortcuts",L"Browser"};
        for(int i=0;i<6;++i)SetWindowTextW(GetDlgItem(window,i==5?SETTINGS_BROWSER:SETTINGS_APPEARANCE+i),T(vn[i],en[i]));
        if(opacityInput&&GetFocus()!=opacityInput){updatingOpacity=true;SetWindowTextW(opacityInput,std::to_wstring(preferences.opacityPercent).c_str());updatingOpacity=false;}
        if(iconSizeInput&&GetFocus()!=iconSizeInput){updatingIconSize=true;SetWindowTextW(iconSizeInput,std::to_wstring(preferences.iconSize).c_str());updatingIconSize=false;}
        EnableWindow(GetDlgItem(window,OPACITY_LESS),preferences.opacityPercent>40);
        EnableWindow(GetDlgItem(window,OPACITY_MORE),preferences.opacityPercent<100);
        EnableWindow(GetDlgItem(window,ICON_SMALLER),preferences.iconSize>12);
        EnableWindow(GetDlgItem(window,ICON_LARGER),preferences.iconSize<24);
        SetWindowTextW(GetDlgItem(window,RESET_PREFERENCES),T(L"Khôi phục mặc định",L"Restore defaults"));
        const wchar_t* keyNamesVi[]={L"Che / mở nội dung",L"Gọi cửa sổ",L"Kiểm tra WGC",L"Bắt đầu / dừng chiếu",L"Tập trung",L"Mở file",L"Mở thư mục",L"Ẩn / hiện Explorer",L"Soạn / đọc",L"Cuộn tự động"};
        const wchar_t* keyNamesEn[]={L"Hide / reveal content",L"Recall window",L"Verify WGC",L"Start / stop projection",L"Focus",L"Open files",L"Open folder",L"Toggle Explorer",L"Edit / read",L"Auto-scroll"};
        for(size_t i=0;i<preferences.shortcuts.size();++i){
            const std::wstring label=capturingShortcut==(int)i?T(L"Nhấn phím...",L"Press shortcut..."):ShortcutLabel(preferences.shortcuts[i]);
            SetWindowTextW(GetDlgItem(window,SHORTCUT_FIRST+(int)i),label.c_str());
            SetTip(SHORTCUT_FIRST+(int)i,T(keyNamesVi[i],keyNamesEn[i]));
        }
        SetWindowTextW(GetDlgItem(window,LANGUAGE),english?L"English":L"Tiếng Việt");
        SetWindowTextW(GetDlgItem(window,TRANSCRIPTION_LANGUAGE),preferences.transcriptionLanguage==1?T(L"Tiếng Anh",L"English"):T(L"Tiếng Việt",L"Vietnamese"));
        for(const auto setting:std::vector<std::pair<int,bool>>{{TOPMOST,topmost},{SHOW_ACTIVITY,activityVisible},{SHOW_TREE,treeVisible},
            {SHOW_TABS,tabsVisible},{SHOW_TOOLBAR,toolbarVisible},{SHOW_STATUS,statusVisible},{INCLUDE_IGNORED,projectTree.IncludeIgnored()},
            {BROWSER_TOGGLE,browser.Enabled()},{POINTER,mirrorPointer},{WRAP,IsCode()?codeViewer.GetState().wrap:documents.Active().state.wrap},
            {BROWSER_MUTED,preferences.browserMuted},{BROWSER_CAMERA,preferences.browserAllowCamera},
            {BROWSER_MICROPHONE,preferences.browserAllowMicrophone},{SCRIPT_TAB_LABELS,preferences.numberedDocumentTabs},
            {BROWSER_TAB_LABELS,preferences.numberedBrowserTabs},{MEETING_TRANSCRIBE,meetingSettings.transcriptionEnabled}}) {
            std::wstring text=T(setting.second?L"Bật":L"Tắt",setting.second?L"On":L"Off");
            for(const auto& line:settingsLines)if(line.control==setting.first){text=line.text+L": "+text;break;}
            SetWindowTextW(GetDlgItem(window,setting.first),text.c_str());
        }
        SetWindowTextW(GetDlgItem(window,MEETING_PROVIDER),MeetingProviderLabel(meetingSettings.provider,english).c_str());
        SetWindowTextW(GetDlgItem(window,MEETING_OUTPUT),MeetingExportLabel(meetingSettings.exportMode,english));
        SetWindowTextW(GetDlgItem(window,MEETING_DIRECTORY),meetingSettings.outputDirectory.c_str());
        SetTip(MEETING_DIRECTORY,meetingSettings.outputDirectory.c_str());
        SetWindowTextW(GetDlgItem(window,MEETING_IMPORT),T(L"Nhập transcript Teams / Zoom / Meet",L"Import Teams / Zoom / Meet transcript"));
        const std::pair<int,COLORREF> customColors[]={{CUSTOM_BACKGROUND,customThemeColors.background},{CUSTOM_PANEL,customThemeColors.panel},
            {CUSTOM_TEXT,customThemeColors.text},{CUSTOM_ACCENT,customThemeColors.accent}};
        for(const auto [id,color]:customColors)SetWindowTextW(GetDlgItem(window,id),ColorHex(color).c_str());
        SetWindowTextW(GetDlgItem(window,RESET_LAYOUT),T(L"Đặt lại",L"Reset"));
        EnableWindow(GetDlgItem(window,WRAP),!Blocked()&&!IsImage()&&!IsPdf());
        EnableWindow(GetDlgItem(window,SPEED),!Blocked());
    }
    SetWindowTextW(GetDlgItem(window,OPEN_DISPLAY_SETTINGS),T(L"Thiết lập màn hình",L"Display settings"));
    if(sizeInput){EnableWindow(sizeInput,!Blocked());SyncSizeInput();}
    SetTip(SPEED, T(L"Chọn tốc độ cuộn", L"Choose scrolling speed"));
    SetTip(SMALLER, T(L"Giảm cỡ chữ - Ctrl+-", L"Smaller text - Ctrl+-"));
    SetTip(LARGER, T(L"Tăng cỡ chữ - Ctrl++", L"Larger text - Ctrl++"));
    SetTip(RESET_ZOOM, IsImage() ? T(L"Vừa khung - Ctrl+0", L"Fit to window - Ctrl+0")
                                : IsCode() ? T(L"Cỡ chữ mặc định 14 pt - Ctrl+0", L"Default size 14 pt - Ctrl+0")
                                           : T(L"Cỡ chữ mặc định 24 pt - Ctrl+0", L"Default size 24 pt - Ctrl+0"));
    SetTip(LANGUAGE, english ? L"Chuyển sang tiếng Việt" : L"Switch to English");
    SetTip(NAV_HOME, T(L"Chuẩn bị nội dung", L"Prepare content"));
    SetTip(NAV_SETTINGS, T(L"Tùy chỉnh hiển thị", L"Display settings"));
    SetTip(NAV_PRESENTATION,T(L"Trình chiếu Smart Mirror",L"Smart Mirror presentation"));
    SetTip(NAV_SCRIPT,T(L"Tệp đang mở. Nhấp phải để chọn tệp hoặc soạn lời nhắc",L"Open files. Right-click to switch files or compose a script"));
    SetTip(OVERFLOW,T(L"Các lệnh và tab khác",L"More commands and tab actions"));
    SetTip(FIND_PREV,T(L"Kết quả trước",L"Previous match"));
    SetTip(FIND_NEXT,T(L"Kết quả sau - Enter",L"Next match - Enter"));
    SetTip(FIND_CLOSE,T(L"Đóng tìm kiếm - Esc",L"Close find - Esc"));
    static std::array<std::wstring,PreferenceCommandCount> currentTips;
    const int tipIds[]={MASK,PIN_WINDOW,VERIFY_PRIVACY,MIRROR_ACTION,FOCUS,OPEN_FILE,OPEN_FOLDER,SHOW_TREE,MODE,PLAY};
    const wchar_t* vi[]={L"Che / mở nội dung",L"Ghim cửa sổ. Gọi lại",L"Kiểm tra WGC",L"Bắt đầu / dừng chiếu",L"Tập trung",L"Mở file",L"Mở thư mục",L"Ẩn / hiện Explorer",L"Soạn / đọc",L"Cuộn tự động"};
    const wchar_t* en[]={L"Hide / reveal content",L"Pin window. Recall",L"Verify WGC",L"Start / stop projection",L"Focus",L"Open files",L"Open folder",L"Toggle Explorer",L"Edit / read",L"Auto-scroll"};
    for(unsigned i=0;i<PreferenceCommandCount;++i){
        currentTips[i]=std::wstring(T(vi[i],en[i]))+L" - "+ShortcutLabel(preferences.shortcuts[i]);
        if(i<2&&!qa&&!globalHotkeys.Registered(static_cast<PreferenceCommand>(i)))currentTips[i]+=T(L" (chỉ trong ứng dụng; phím toàn hệ thống chưa khả dụng)",L" (app only; global binding unavailable)");
        SetTip(tipIds[i],currentTips[i].c_str());
    }
    SetTip(TOPMOST,currentTips[1].c_str());
    SetTip(NAV_SCRIPT,T(L"Trở lại tệp đang xem",L"Return to current file"));SetTip(NAV_READ,currentTips[8].c_str());
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
    if(!Blocked()) {
        size_t number=1;
        for(const auto &doc:documents.Items()) if(doc->id!=0) {
            const std::wstring name=preferences.numberedDocumentTabs?
                std::wstring(L"Tab ")+std::to_wstring(number):
                doc->displayName+(doc->dirty?L" *":L"");
            tabs.push_back({doc->id,name,doc->canonicalPath,(int)doc->type,false});
            ++number;
        }
    }
    documentBar.Update(std::move(tabs),documents.Active().id,!Blocked(),english,dpi,uiFont);
    std::vector<TabVisual> webTabs;
    if(!Blocked()) {
        size_t number=1;
        for(const auto& tab:browser.Tabs()) {
            const std::wstring name=preferences.numberedBrowserTabs?
                std::wstring(L"Tab ")+std::to_wstring(number):
                tab.title.empty()?T(tab.loading?L"Đang mở…":L"Tab mới",tab.loading?L"Opening…":L"New tab"):tab.title;
            webTabs.push_back({tab.id,name,tab.address,0,false});
            ++number;
        }
    }
    browserTabBar.Update(std::move(webTabs),browser.ActiveTabId(),!Blocked(),english,dpi,uiFont);
}
void ClipSettingsControls(bool redraw=true)
{
    for(HWND control:clippedSettingsControls) {
        RECT bounds{};GetWindowRect(control,&bounds);MapWindowPoints(nullptr,window,(POINT*)&bounds,2);
        RECT clip{};IntersectRect(&clip,&bounds,&settingsViewport);
        HRGN region=CreateRectRgn(std::max(0L,clip.left-bounds.left),std::max(0L,clip.top-bounds.top),
            std::max(0L,clip.right-bounds.left),std::max(0L,clip.bottom-bounds.top));
        if(region&&!SetWindowRgn(control,region,redraw))DeleteObject(region);
    }
}
void Layout()
{
    if(!window || !editor) return;
    if(Blocked())EnumThreadWindows(GetCurrentThreadId(),[](HWND popup,LPARAM owner)->BOOL {
        wchar_t name[64]{};GetClassNameW(popup,name,64);
        if(GetWindow(popup,GW_OWNER)==(HWND)owner&&wcscmp(name,L"QuietShareToolMenu")==0) {
            SendMessageW(popup,WM_CLOSE,0,0);
        }
        return TRUE;
    },(LPARAM)window);
    if(IsIconic(window)) {
        // Preserve viewer bounds and scroll state while Windows gives the
        // minimized owner an off-screen rectangle. Projection has its own HWND.
        if(GetCapture()==settingsScroll)SendMessageW(settingsScroll,WM_CANCELMODE,0,0);
        if(GetCapture()==window){ReleaseCapture();splitterDragging=splitterHovered=false;SaveLayoutPreferences();}
        browser.Show(false);documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);
        renderedSafe=!Blocked();UpdateButtons();UpdateMirrorTimer();return;
    }
    if(page!=Page::Settings&&GetCapture()==settingsScroll)SendMessageW(settingsScroll,WM_CANCELMODE,0,0);
    if(page!=Page::Settings)settingsWheelRemainder=0;
    for(HWND control:clippedSettingsControls)SetWindowRgn(control,nullptr,FALSE);
    clippedSettingsControls.clear();settingsLines.clear();numericSettings.clear();settingsSizeHint={};settingsViewport={};
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    bool caption=(style&WS_CAPTION)==WS_CAPTION;
    if(caption==focusMode) {
        SetWindowLongPtrW(window,GWL_STYLE,focusMode?style&~WS_CAPTION:style|WS_CAPTION);
        SetWindowPos(window,nullptr,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    }
    RECT client{};GetClientRect(window,&client);
    int width=MulDiv(client.right,96,dpi),height=MulDiv(client.bottom,96,dpi);
    compactUi=width<750;
    railWidth=focusMode||!activityVisible?0:compactUi?COMPACT_SIDEBAR:NORMAL_SIDEBAR;
    int gutter=8;
    contentLeft=railWidth+gutter;contentRight=width-gutter;
    std::vector<HWND> visibleControls;
    auto reveal=[&](HWND control){visibleControls.push_back(control);};
    fontLabelRect={};explorerRect={};splitterRect={};
    const int bottom=height-(!focusMode&&statusVisible?20:4);
    auto move=[&](int id,int left,int top,int w,int h=32){
        HWND child=GetDlgItem(window,id);MoveWindow(child,S(left),S(top),S(w),S(h),FALSE);reveal(child);
    };
    if(!focusMode&&activityVisible) {
        int y=12;for(int id:{NAV_HOME,NAV_SCRIPT,NAV_BROWSER}){
            move(id,4,y,railWidth-8,32);y+=36;
        }
        move(VERIFY_PRIVACY,4,y+12,railWidth-8,32);move(MIRROR_ACTION,4,y+48,railWidth-8,32);
        move(MEETING_RECORD,4,y+84,railWidth-8,32);
        move(NAV_SETTINGS,4,std::max(y+124,height-56),railWidth-8,28);
    }
    contentCardRect={S(contentLeft),S(48),S(contentRight),S(bottom)};
    if(page==Page::Content || focusMode) {
        int left=contentLeft;
        if(!focusMode && treeVisible && !projectTree.Root().empty() && !Blocked()) {
            const int treeWidth=std::clamp(explorerWidth,112,std::max(112,contentRight-left-192));
            explorerRect={S(left),S(4),S(left+treeWidth),S(bottom)};
            MoveWindow(projectTree.Handle(),S(left+4),S(40),S(treeWidth-8),S(std::max(80,bottom-44)),FALSE);
            reveal(projectTree.Handle());move(SHOW_TREE,left+treeWidth-28,6,24,24);
            splitterRect={S(left+treeWidth),S(4),S(left+treeWidth+8),S(bottom)};left+=treeWidth+8;
        }
        contentLeft=left;
        int x=contentRight;
        auto action=[&](int id,int w=24){x-=w;move(id,x,2,w,24);};
        action(OVERFLOW);action(MASK);action(FOCUS);action(PIN_WINDOW);
        if(viewingFile&&preferences.allowFileEditing&&!IsImage()&&!IsPdf())action(SAVE_FILE,52);
        if(!focusMode)action(LAYOUT_MENU);
        if(!focusMode&&tabsVisible) {
            MoveWindow(documentBar.Window(),S(contentLeft),0,S(std::max(64,x-contentLeft-4)),S(28),FALSE);
            reveal(documentBar.Window());
        }
        int bodyTop=32;
        if(!focusMode&&toolbarVisible) {
            int toolX=contentLeft,toolY=30;
            auto tool=[&](int id,int w){
                if(toolX+w>contentRight){toolX=contentLeft;toolY+=28;}
                move(id,toolX,toolY,w,24);toolX+=w+4;
            };
            tool(SMALLER,24);
            if(!IsCode()&&!IsImage()&&!IsPdf())tool(SIZE_INPUT,44);
            else tool(RESET_ZOOM,44);
            tool(LARGER,24);
            if(!IsCode()&&!IsImage()&&!IsPdf())tool(TEXT_WEIGHT,60);
            tool(THEME_SELECT,160);
            if(!IsImage()&&!IsPdf())tool(WRAP,84);
            if(!IsCode()&&!IsImage()&&!IsPdf())tool(RESET_ZOOM,44);
            tool(OPEN_FILE,24);
            if(!viewingFile)tool(MODE,24);
            if(reading&&!viewingFile)tool(PLAY,24);
            bodyTop=toolY+30;
        }
        contentCardRect={S(contentLeft),S(bodyTop),S(contentRight),S(bottom)};
        if(findVisible && !reloadPending && !Blocked()) {
            int y=bodyTop, inputWidth=std::max(80,contentRight-contentLeft-104);
            MoveWindow(findInput,S(contentLeft),S(y),S(inputWidth),S(28),TRUE);reveal(findInput);
            move(FIND_PREV,contentRight-96,y,28,28);move(FIND_NEXT,contentRight-64,y,28,28);move(FIND_CLOSE,contentRight-32,y,28,28);
            contentCardRect.top=S(bodyTop+36);
        }
        if(reloadPending && !Blocked()) {
            const int w=std::min(112,(contentRight-contentLeft-8)/2);
            move(RELOAD_FILE,contentLeft,bodyTop,w,28);move(KEEP_FILE,contentLeft+w+8,bodyTop,contentRight-contentLeft-w-8,28);
            contentCardRect.top=S(bodyTop+36);
        }
    }
    else if(page==Page::Home) {
        const int padding=width<620?12:20;contentLeft+=padding;contentRight-=padding;
        homeCompact=height<480;homeRowHeight=homeCompact?40:height<620?56:72;
        const int w=(contentRight-contentLeft-12)/2;
        const int actionTop=homeCompact?44:88,actionHeight=homeCompact?60:110;
        move(OPEN_DROP,contentLeft,actionTop,w,actionHeight);move(OPEN_FOLDER,contentLeft+w+12,actionTop,w,actionHeight);
        const int recentTop=homeCompact?140:height<620?238:248;
        const int protectionTop=std::max(recentTop+(homeCompact?44:12),bottom-(homeCompact?76:108));
        homeProtectionRect={S(contentLeft),S(protectionTop+24),S(contentRight),S(bottom-4)};
        homeRecentRect={S(contentLeft),S(recentTop),S(contentRight),S(std::max(recentTop,protectionTop-(homeCompact?4:12)))};
        recentRows=std::clamp(MulDiv(homeRecentRect.bottom-homeRecentRect.top,96,dpi)/homeRowHeight,0,6);
        for(int i=0;i<6;++i)SetWindowTextW(GetDlgItem(window,RECENT_FIRST+i*3),L"");
        if(recentOffset>=recentWorkspace.entries.size())recentOffset=0;
        move(RECENT_CLEAR,contentRight-88,recentTop-30,88,24);
        move(RECENT_PREV,contentRight-156,recentTop-30,28,24);move(RECENT_NEXT,contentRight-124,recentTop-30,28,24);
        EnableWindow(GetDlgItem(window,RECENT_PREV),!Blocked()&&recentOffset>0);
        EnableWindow(GetDlgItem(window,RECENT_NEXT),!Blocked()&&recentRows>0&&recentOffset+recentRows<recentWorkspace.entries.size());
        EnableWindow(GetDlgItem(window,RECENT_CLEAR),!Blocked()&&!recentWorkspace.entries.empty());
        if(!Blocked())for(int i=0;i<recentRows&&recentOffset+i<recentWorkspace.entries.size();++i){
            const auto& entry=recentWorkspace.entries[recentOffset+i];
            SetWindowTextW(GetDlgItem(window,RECENT_FIRST+i*3),entry.path.substr(entry.path.find_last_of(L"\\/")+1).c_str());
            SetWindowTextW(GetDlgItem(window,RECENT_FIRST+i*3+1),T(entry.pinned?L"Bỏ ghim":L"Ghim",entry.pinned?L"Unpin":L"Pin"));
            const int y=recentTop+i*homeRowHeight;
            move(RECENT_FIRST+i*3,contentLeft+4,y+2,contentRight-contentLeft-82,homeRowHeight-4);
            move(RECENT_FIRST+i*3+1,contentRight-74,y+(homeRowHeight-28)/2,32,28);
            move(RECENT_FIRST+i*3+2,contentRight-38,y+(homeRowHeight-28)/2,32,28);
        }
        move(HOME_SETTINGS,contentRight-68,protectionTop,68,22);
        if(!activityVisible&&!homeCompact)move(VERIFY_PRIVACY,contentRight-150,54,150,24);
    }
    else if(page==Page::Settings) {
        const int formWidth=std::min(760,contentRight-contentLeft),formLeft=contentLeft+(contentRight-contentLeft-formWidth)/2;
        const int columns=formWidth<680?3:6,tabWidth=(formWidth-8*(columns-1))/columns;
        for(int i=0;i<6;++i)move(i==5?SETTINGS_BROWSER:SETTINGS_APPEARANCE+i,formLeft+(i%columns)*(tabWidth+8),48+(i/columns)*32,tabWidth,28);
        const int formTop=48+((6+columns-1)/columns)*32+8;
        settingsViewport={S(formLeft),S(formTop),S(formLeft+formWidth-20),S(bottom)};
        settingsCardRect=settingsViewport;
        const int left=formLeft+16,right=formLeft+formWidth-36;
        int y=formTop+16-settingsOffset;
        auto heading=[&](const wchar_t* vn,const wchar_t* en){
            settingsLines.push_back({T(vn,en),{S(left),S(y),S(right),S(y+24)},true});y+=36;
        };
        auto row=[&](const wchar_t* vn,const wchar_t* en,int id,int valueWidth=112,int height=48){
            settingsLines.push_back({T(vn,en),{S(left),S(y),S(right-valueWidth-16),S(y+32)},false,id});
            move(id,right-valueWidth,y,valueWidth,id==SOURCE_MONITOR||id==DEST_MONITOR?160:32);
            clippedSettingsControls.push_back(GetDlgItem(window,id));y+=height;
        };
        auto number=[&](int id,int minus,int plus,int x){
            numericSettings.push_back({GetDlgItem(window,id),minus,plus,{S(x),S(y),S(x+128),S(y+32)}});
            move(minus,x+2,y+2,28,28);move(id,x+34,y+2,60,28);move(plus,x+98,y+2,28,28);
            for(int child:{minus,id,plus})clippedSettingsControls.push_back(GetDlgItem(window,child));
        };
        auto numberRow=[&](const wchar_t* vn,const wchar_t* en,int id,int minus,int plus){
            settingsLines.push_back({T(vn,en),{S(left),S(y),S(right-144),S(y+32)},false,id});
            number(id,minus,plus,right-128);y+=48;
        };
        if(settingsCategory==0) {
        heading(L"Giao diện",L"Appearance");
        row(L"Ngôn ngữ",L"Language",LANGUAGE,128);
        row(L"Chủ đề màu",L"Color theme",THEME_SELECT,180);
        row(L"Ghim cửa sổ",L"Always on top",TOPMOST);
        row(L"Nhớ kích thước cửa sổ",L"Remember window size",REMEMBER_SIZE);
        numberRow(L"Độ mờ cửa sổ (%)",L"Window opacity (%)",OPACITY_INPUT,OPACITY_LESS,OPACITY_MORE);
        numberRow(L"Kích thước icon (px)",L"Icon size (px)",ICON_SIZE_INPUT,ICON_SMALLER,ICON_LARGER);
        row(L"Độ đậm chữ",L"Text weight",TEXT_WEIGHT);
        settingsLines.push_back({T(L"40-100%. Áp dụng cho cửa sổ điều khiển, kể cả chữ. Màn hình chiếu không đổi.",L"40-100%. Applies to the control window, including text. Projection stays opaque."),{S(left),S(y),S(right),S(y+56)},false});y+=64;
        heading(L"Theme tự thiết kế",L"Custom theme");
        row(L"Màu nền",L"Background color",CUSTOM_BACKGROUND,128);
        row(L"Màu bảng",L"Panel color",CUSTOM_PANEL,128);
        row(L"Màu chữ",L"Text color",CUSTOM_TEXT,128);
        row(L"Màu nhấn",L"Accent color",CUSTOM_ACCENT,128);
        row(L"Khôi phục màu theme mặc định",L"Reset custom colors",CUSTOM_RESET,180);
        }
        if(settingsCategory==1) {
        heading(L"Đọc nội dung",L"Reading");
        row(L"Cho phép sửa file văn bản / code",L"Allow text / code editing",ALLOW_EDIT);
        const int valueLeft=right-204;
        settingsLines.push_back({IsCode()||IsImage()||IsPdf()?T(L"Thu/phóng (%)",L"Zoom (%)"):T(L"Cỡ chữ (pt)",L"Text size (pt)"),
            {S(left),S(y),S(valueLeft-12),S(y+32)},false,SIZE_INPUT});
        number(SIZE_INPUT,SMALLER,LARGER,valueLeft);move(RESET_ZOOM,right-68,y,68,32);
        clippedSettingsControls.push_back(GetDlgItem(window,RESET_ZOOM));
        settingsSizeHint={S(left),S(y+36),S(right),S(y+60)};y+=72;
        row(L"Độ đậm chữ",L"Text weight",TEXT_WEIGHT);
        row(L"Tốc độ lời nhắc",L"Script scrolling",SPEED);
        row(L"Tự chép lời sau khi ghi âm",L"Transcribe after recording",MEETING_TRANSCRIBE);
        row(L"Ngôn ngữ chép lời",L"Transcription language",TRANSCRIPTION_LANGUAGE,180);
        row(L"Nền tảng cuộc họp",L"Meeting platform",MEETING_PROVIDER,200);
        row(L"Kiểu xuất cuộc họp",L"Meeting export",MEETING_OUTPUT,250);
        row(L"Thư mục lưu WAV / transcript",L"WAV / transcript folder",MEETING_DIRECTORY,260);
        row(L"Transcript có sẵn từ Teams / Zoom / Meet",L"Existing Teams / Zoom / Meet transcript",MEETING_IMPORT,260);
        row(L"Tự xuống dòng",L"Word wrap",WRAP);
        }
        if(settingsCategory==2) {
        heading(L"Bố cục",L"Workspace");
        row(L"Thanh điều hướng",L"Activity bar",SHOW_ACTIVITY);
        row(L"Explorer",L"Explorer",SHOW_TREE);
        row(L"Thanh tab",L"Tab bar",SHOW_TABS);
        heading(L"Hiển thị tab dạng Tab 1, Tab 2, ...",L"Show tabs as Tab 1, Tab 2, ...");
        row(L"Lời nhắc",L"Script",SCRIPT_TAB_LABELS);
        row(L"Trình duyệt",L"Browser",BROWSER_TAB_LABELS);
        row(L"Thanh công cụ",L"Toolbar",SHOW_TOOLBAR);
        row(L"Thanh trạng thái",L"Status bar",SHOW_STATUS);
        row(L"Khôi phục bố cục",L"Reset layout",RESET_LAYOUT);
        y+=12;heading(L"Tiện ích",L"Tools");
        row(L"Thư mục nặng / ẩn",L"Heavy / hidden folders",INCLUDE_IGNORED);
        row(L"Xóa lịch sử (kể cả ghim)",L"Clear history (including pins)",RECENT_CLEAR);
        }
        if(settingsCategory==3) {
        settingsDisplayAnchor=0;heading(L"Màn hình trình chiếu",L"Projection displays");
        const int monitorWidth=std::clamp((right-left)/2,160,280);
        row(L"Màn hình nguồn",L"Source display",SOURCE_MONITOR,monitorWidth);
        row(L"Màn hình đích",L"Destination",DEST_MONITOR,monitorWidth);
        row(L"Con trỏ khi chiếu",L"Presentation pointer",POINTER);
        row(L"Tài nguyên",L"Resources",RESOURCE_MODE,std::min(188,right-left-100));
        y+=8;settingsLines.push_back({L"",{S(left),S(y),S(right),S(y+104)},false,-1});y+=112;
        }
        if(settingsCategory==4) {
        heading(L"Phím tắt",L"Keyboard shortcuts");
        const wchar_t* vn[]={L"Che / mở nội dung",L"Gọi cửa sổ",L"Kiểm tra WGC",L"Bắt đầu / dừng chiếu",L"Tập trung",L"Mở file",L"Mở thư mục",L"Ẩn / hiện Explorer",L"Soạn / đọc",L"Cuộn tự động"};
        const wchar_t* en[]={L"Hide / reveal content",L"Recall window",L"Verify WGC",L"Start / stop projection",L"Focus",L"Open files",L"Open folder",L"Toggle Explorer",L"Edit / read",L"Auto-scroll"};
        for(int i=0;i<(int)preferences.shortcuts.size();++i)row(vn[i],en[i],SHORTCUT_FIRST+i,std::min(152,(right-left)/2));
        settingsLines.push_back({T(L"Bấm một tổ hợp để đổi, Esc để hủy. Hai phím đầu hoạt động toàn hệ thống.",L"Click a binding to change it; Esc cancels. The first two work system-wide."),{S(left),S(y),S(right),S(y+56)},false});y+=64;
        }
        if(settingsCategory==5) {
        heading(L"Trình duyệt",L"Browser");
        row(L"Bật trình duyệt",L"Enable browser",BROWSER_TOGGLE);
        row(L"Tắt âm thanh",L"Mute audio",BROWSER_MUTED);
        y+=12;heading(L"Quyền truy cập",L"Site permissions");
        row(L"Cho phép camera",L"Allow camera",BROWSER_CAMERA);
        row(L"Cho phép micro",L"Allow microphone",BROWSER_MICROPHONE);
        settingsLines.push_back({T(L"Camera và micro mặc định tắt. Khi bật, chỉ trang HTTPS đủ điều kiện mới được dùng, theo quyền Windows. Tắt quyền sẽ dừng luồng đang dùng và có thể tải lại tab.",
            L"Camera and microphone are off by default. When enabled, eligible HTTPS pages may use them, subject to Windows permissions. Turning access off stops active streams and may reload a tab."),
            {S(left),S(y),S(right),S(y+104)},false});y+=112;
        }
        if(settingsCategory==0||settingsCategory==4)row(L"",L"",RESET_PREFERENCES,std::min(188,right-left));
        if(!preferenceError.empty()){settingsLines.push_back({preferenceError,{S(left),S(y),S(right),S(y+64)},false});y+=72;}
        settingsMaximum=std::max(0,y+settingsOffset+8-bottom);
        if(settingsOffset>settingsMaximum){settingsOffset=settingsMaximum;Layout();return;}
        if(settingsMaximum) {
            move(SETTINGS_SCROLL,formLeft+formWidth-14,formTop,14,bottom-formTop);
            SCROLLINFO info{sizeof(info),SIF_RANGE|SIF_PAGE|SIF_POS};info.nMax=settingsMaximum+bottom-formTop-1;info.nPage=bottom-formTop;info.nPos=settingsOffset;
            SetScrollInfo(settingsScroll,SB_CTL,&info,FALSE);
        }
    }
    else if(page==Page::Browser) {
        // Keep the address centred between navigation and the browser menu.
        if(browser.Enabled()) {
            const int actionsLeft=contentRight-78;
            if(tabsVisible) {
                MoveWindow(browserTabBar.Window(),S(contentLeft),0,S(std::max(64,actionsLeft-contentLeft-30)),S(28),FALSE);
                if(!Blocked())reveal(browserTabBar.Window());
            }
            if(!Blocked())move(WEB_NEW_TAB,actionsLeft-26,2,24,24);
            int x=contentLeft;
            for(int id:{WEB_BACK,WEB_FORWARD,WEB_RELOAD,WEB_AUDIO}){move(id,x,30,24,26);x+=26;}
            const int addressRight=contentRight-130,addressWidth=std::min(680,std::max(48,addressRight-x-4));
            MoveWindow(webAddress,S(x+4+(addressRight-x-4-addressWidth)/2),S(30),S(addressWidth),S(26),FALSE);
            if(!Blocked()){
                reveal(webAddress);move(WEB_ZOOM_OUT,contentRight-126,30,24,26);
                move(WEB_ZOOM_RESET,contentRight-100,30,44,26);move(WEB_ZOOM_IN,contentRight-54,30,24,26);
                move(WEB_MENU,contentRight-24,30,24,26);
            }
            contentCardRect={S(contentLeft),S(60),S(contentRight),S(bottom)};
        } else {
            move(BROWSER_TOGGLE,contentLeft,48,std::min(164,contentRight-contentLeft),28);
            contentCardRect={S(contentLeft),S(84),S(contentRight),S(bottom)};
        }
    }
    else if(page==Page::Presentation) {
        move(OPEN_DISPLAY_SETTINGS,contentLeft,116,std::min(220,contentRight-contentLeft),28);
        move(VERIFY_PRIVACY,contentLeft,152,std::min(220,contentRight-contentLeft),28);
        move(MIRROR_START,contentLeft,188,84);move(MIRROR_STOP,contentLeft+92,188,84);
    }
    if(!focusMode&&page==Page::Browser) {
        move(LAYOUT_MENU,contentRight-24,2,24,24);
        move(PIN_WINDOW,contentRight-50,2,24,24);move(MASK,contentRight-76,2,24,24);
    } else if(!focusMode&&page!=Page::Content) {
        move(LAYOUT_MENU,contentRight-28,8,28,28);
        move(PIN_WINDOW,contentRight-60,8,28,28);
    }
    if(splitterRect.right<=splitterRect.left&&(splitterDragging||splitterHovered)) {
        splitterDragging=splitterHovered=false;
        if(GetCapture()==window)ReleaseCapture();
    }
    RECT body=contentCardRect;InflateRect(&body,-S(page==Page::Browser?2:4),-S(page==Page::Browser?2:4));
    std::vector<HWND> shellControls=buttons;
    for(HWND control:{settingsScroll,sizeInput,opacityInput,iconSizeInput,findInput,webAddress,sourceMonitor,destinationMonitor,documentBar.Window(),browserTabBar.Window(),homeVersion,projectTree.Handle()})shellControls.push_back(control);
    for(HWND control:shellControls)if(control) {
        const bool visible=std::find(visibleControls.begin(),visibleControls.end(),control)!=visibleControls.end();
        const bool wasVisible=(GetWindowLongPtrW(control,GWL_STYLE)&WS_VISIBLE)!=0;
        if(visible!=wasVisible)ShowWindow(control,visible?SW_SHOW:SW_HIDE);
    }
    if(page==Page::Settings) {
        HWND previous=HWND_TOP;
        for(HWND control:clippedSettingsControls) {
            SetWindowPos(control,previous,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOREDRAW);previous=control;
        }
    }
    documentHost.SetBounds(body);
    for(HWND control:{sizeInput,opacityInput,iconSizeInput,webAddress})CenterInputText(control);
    RECT textRect{};GetClientRect(editor,&textRect);
    InflateRect(&textRect,-S(12),-S(8));SendMessageW(editor,EM_SETRECT,0,(LPARAM)&textRect);
    DocumentType displayType=IsCode()?DocumentType::Code:documents.Active().type;
    renderedSafe=!Blocked();
    documentHost.Show(displayType,renderedSafe,page==Page::Content);
    browser.SetBounds(body);browser.Show(page==Page::Browser&&renderedSafe&&!IsIconic(window));
    UpdateButtons();UpdateDocumentBar();
    UpdateMirrorTimer();
    // Native EDIT formatting/text updates can repaint immediately. Apply the
    // final viewport after those updates, repainting the area released by it.
    ClipSettingsControls();
    RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);
}
void ScrollSettings(int position)
{
    const int next=std::clamp(position,0,settingsMaximum);
    if(next==settingsOffset)return;
    const int dy=S(settingsOffset)-S(next);settingsOffset=next;
    SendMessageW(tooltips,TTM_POP,0,0);
    if(page!=Page::Settings||clippedSettingsControls.empty()){Layout();return;}
    // Scrolling changes only row positions, not document/browser state or text.
    HDWP batch=BeginDeferWindowPos((int)clippedSettingsControls.size());
    for(HWND control:clippedSettingsControls) {
        if(!batch)break;
        RECT bounds{};GetWindowRect(control,&bounds);MapWindowPoints(nullptr,window,(POINT*)&bounds,2);
        batch=DeferWindowPos(batch,control,nullptr,bounds.left,bounds.top+dy,0,0,
            SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
    }
    if(!batch||!EndDeferWindowPos(batch)){Layout();return;}
    for(auto& line:settingsLines)OffsetRect(&line.bounds,0,dy);
    for(auto& setting:numericSettings)OffsetRect(&setting.bounds,0,dy);
    OffsetRect(&settingsSizeHint,0,dy);OffsetRect(&fontLabelRect,0,dy);
    SCROLLINFO info{sizeof(info),SIF_POS};info.nPos=settingsOffset;SetScrollInfo(settingsScroll,SB_CTL,&info,FALSE);
    ClipSettingsControls(false);
    RECT redraw=settingsViewport;redraw.right+=S(20);
    RedrawWindow(window,&redraw,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);
}
void EnsureSettingsVisible(HWND control)
{
    if(page!=Page::Settings||std::find(clippedSettingsControls.begin(),clippedSettingsControls.end(),control)==clippedSettingsControls.end())return;
    RECT bounds{};GetWindowRect(control,&bounds);MapWindowPoints(nullptr,window,(POINT*)&bounds,2);
    if(control==sizeInput)bounds.bottom+=S(28);
    if(bounds.top<settingsViewport.top+S(8))ScrollSettings(settingsOffset+MulDiv(bounds.top-settingsViewport.top,96,dpi)-8);
    else if(bounds.bottom>settingsViewport.bottom-S(8))ScrollSettings(settingsOffset+MulDiv(bounds.bottom-settingsViewport.bottom,96,dpi)+8);
}
struct TextViewport
{
    CHARRANGE selection{};
    LONG firstCharacter=0;
    TextViewport() {
        SendMessageW(editor,EM_EXGETSEL,0,(LPARAM)&selection);
        const auto firstLine=SendMessageW(editor,EM_GETFIRSTVISIBLELINE,0,0);
        firstCharacter=(LONG)SendMessageW(editor,EM_LINEINDEX,firstLine,0);
    }
    void Restore() const {
        SendMessageW(editor,EM_EXSETSEL,0,(LPARAM)&selection);
        // Reflow changes pixel offsets and line counts. Restore the line that
        // contains the original visible character, independently of the caret.
        // EM_LINESCROLL also avoids the 16-bit EM_GETSCROLLPOS coordinate limit.
        const auto target=SendMessageW(editor,EM_EXLINEFROMCHAR,0,std::max(0L,firstCharacter));
        const auto current=SendMessageW(editor,EM_GETFIRSTVISIBLELINE,0,0);
        SendMessageW(editor,EM_LINESCROLL,0,target-current);
    }
};
void ApplyTextWrap()
{
    if(IsCode()||IsImage()||IsPdf())return;
    const TextViewport viewport;
    const bool wrap=documents.Active().state.wrap;
    SendMessageW(editor,EM_SETTARGETDEVICE,0,wrap?0:1);
    SendMessageW(editor,EM_SHOWSCROLLBAR,SB_HORZ,!wrap);
    viewport.Restore();
}
void ApplyTextFormat()
{
    const TextViewport viewport;
    loading = true;
    CHARFORMAT2W cf{};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_BOLD | CFM_WEIGHT;
    cf.wWeight=(WORD)preferences.textWeight;
    if(preferences.textWeight>=600)cf.dwEffects|=CFE_BOLD;
    cf.yHeight = ActiveTextSize() * 20;
    cf.crTextColor = TEXT;
    wcscpy_s(cf.szFaceName, IsCode() ? L"Consolas" : L"Segoe UI");
    SendMessageW(editor, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    SendMessageW(editor, EM_SETSEL, 0, -1);
    PARAFORMAT2 pf{};
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_LINESPACING | PFM_SPACEAFTER;
    pf.bLineSpacingRule = 5;
    pf.dyLineSpacing = IsCode() ? 22 : 26;
    pf.dySpaceAfter = IsCode() ? 0 : 100;
    SendMessageW(editor, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
    viewport.Restore();
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
    brandFont = CreateFontW(-S(16), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                            CLEARTYPE_QUALITY, 0, L"Segoe UI");
    metaFont = CreateFontW(-S(13), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                           CLEARTYPE_QUALITY, 0, L"Segoe UI");
    for (HWND button : buttons)
        SendMessageW(button, WM_SETFONT, (WPARAM)uiFont, TRUE);
    for(HWND control:{findInput,sizeInput,opacityInput,iconSizeInput,webAddress,sourceMonitor,destinationMonitor,projectTree.Handle()})if(control)SendMessageW(control,WM_SETFONT,(WPARAM)uiFont,TRUE);
    if(homeVersion)SendMessageW(homeVersion,WM_SETFONT,(WPARAM)metaFont,TRUE);
    projectTree.SetDpi(dpi);
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
    updatingSize=true;SetWindowTextW(sizeInput,std::to_wstring(CurrentSize()).c_str());updatingSize=false;
    if(GetFocus()==sizeInput) {
        const int end=GetWindowTextLengthW(sizeInput);SendMessageW(sizeInput,EM_SETSEL,end,end);
    }
}
void SetContentSize(int value,bool refresh=true)
{
    Pause();sizeError.clear();
    if(IsCode())codeViewer.SetZoom(value);
    else if(IsPdf())pdfViewer.SetZoom(value);
    else if(IsImage()){imageZoom=value;panX=panY=0;InvalidateRect(imageViewer,nullptr,FALSE);}
    else {
        const TextViewport viewport;
        ActiveTextSize()=value;
        CHARFORMAT2W format{};format.cbSize=sizeof(format);format.dwMask=CFM_SIZE;format.yHeight=value*20;
        loading=true;SendMessageW(editor,EM_SETCHARFORMAT,SCF_ALL,(LPARAM)&format);viewport.Restore();loading=false;
    }
    if(refresh)UpdateButtons();
    SyncSizeInput(true);InvalidateRect(window,&fontLabelRect,FALSE);
}
void CommitSizeInput()
{
    if(updatingSize||!sizeInput||Blocked()||(page!=Page::Settings&&page!=Page::Content)||sizeInputDocument!=documents.Active().id)return;
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
    if(message==WM_SIZE||message==WM_SETFONT) {
        auto result=DefSubclassProc(control,message,wp,lp);CenterInputText(control);return result;
    }
    if(message==WM_CHAR&&(wp==L'\r'||wp==L'\n'))return 0;
    if(message==WM_GETDLGCODE) {
        LRESULT code=DefSubclassProc(control,message,wp,lp);
        return wp==VK_RETURN||wp==VK_ESCAPE?code|DLGC_WANTMESSAGE:code;
    }
    if(message==WM_SETFOCUS) {
        LRESULT result=DefSubclassProc(control,message,wp,lp);
        EnsureSettingsVisible(control);sizeInputDocument=documents.Active().id;
        const int end=GetWindowTextLengthW(control);SendMessageW(control,EM_SETSEL,end,end);SendMessageW(control,EM_SCROLLCARET,0,0);
        InvalidateRect(window,nullptr,FALSE);return result;
    }
    if(message==WM_LBUTTONDOWN&&GetFocus()!=control) {
        LRESULT result=DefSubclassProc(control,message,wp,lp);
        const int end=GetWindowTextLengthW(control);SendMessageW(control,EM_SETSEL,end,end);SendMessageW(control,EM_SCROLLCARET,0,0);return result;
    }
    if(message==WM_MOUSEWHEEL&&page==Page::Settings)return SendMessageW(window,message,wp,lp);
    if(message==WM_SETFOCUS||message==WM_KILLFOCUS)InvalidateRect(window,nullptr,FALSE);
    if(message==WM_KEYDOWN&&wp==VK_RETURN){CommitSizeInput();return 0;}
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE){sizeError.clear();SyncSizeInput(true);InvalidateRect(window,nullptr,FALSE);return 0;}
    return DefSubclassProc(control,message,wp,lp);
}
void SetReading(bool value)
{
    if (Blocked())
        return;
    if(viewingFile){
        if(IsImage()||IsPdf())return;
        reading=value||!preferences.allowFileEditing;
        SendMessageW(editor,EM_SETREADONLY,reading,0);codeViewer.SetEditable(!reading);Layout();return;
    }
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
    ClearProjectionStoppedNotice();
    masked = !masked;
    Pause();
    if(masked){browser.Lock();documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);}
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
    browser.Disable();
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
void PersistRecent(){if(!qa&&!recentWorkspace.Save(dataDir))importError=T(L"Không lưu được lịch sử cục bộ.",L"Could not save local history.");}
bool SaveActiveState()
{
    auto &doc=documents.Active();
    auto &state=doc.state;
    if(doc.id==0) {scriptDraft=TextOf(editor);doc.content.text=scriptDraft;scriptReading=reading;}
    if(doc.id&&(doc.type==DocumentType::Text||doc.type==DocumentType::Code)) {
        if((doc.type==DocumentType::Text&&doc.dirty)||(doc.type==DocumentType::Code&&(codeViewer.Modified()||doc.dirty))){
            if(doc.type==DocumentType::Code){
                if(!codeViewer.ReadText(doc.draft)){
                    doc.draftUnavailable=true;doc.dirty=true;
                    findMessage.clear();
                    importError=T(L"Code vượt giới hạn 5 MiB hoặc không hợp lệ. Giảm nội dung để lưu; file gốc được giữ nguyên.",L"Code exceeds 5 MiB or is invalid. Reduce it to save; the original file is preserved.");
                    InvalidateRect(window,nullptr,FALSE);return false;
                }
            }else doc.draft=TextOf(editor);
            doc.draftUnavailable=false;
            doc.dirty=!SameDocumentText(doc.draft,doc.content.text);
            if(!doc.dirty)doc.draft.clear();
        }
    }
    if(IsCode() || IsPdf()) {
        auto view=IsCode()?codeViewer.GetState():pdfViewer.GetState();
        state.scrollX=view.scrollX;state.scrollY=view.scrollY;state.zoom=view.zoom;
        state.page=view.page;state.panX=view.panX;state.panY=view.panY;state.wrap=view.wrap;state.fitWidth=view.fitWidth;
    } else {
        POINT scroll{};SendMessageW(editor,EM_GETSCROLLPOS,0,(LPARAM)&scroll);
        state.scrollX=scroll.x;state.scrollY=scroll.y;
        state.textPoints=ActiveTextSize();state.zoom=imageZoom;state.panX=panX;state.panY=panY;
    }
    return true;
}
bool SaveFile(){
    if(Blocked()||!preferences.allowFileEditing||!viewingFile||IsImage()||IsPdf())return false;
    if(!SaveActiveState())return false;if(!documents.Active().dirty)return true;
    const bool saved=documents.Save(documents.Active().id,importError);
    if(saved&&IsCode())codeViewer.MarkSaved();
    UpdateDocumentBar();InvalidateRect(window,nullptr,FALSE);return saved;
}
bool ConfirmFile(DocumentId id){
    auto* doc=documents.Get(id);if(!doc||!doc->dirty)return true;
    if(closeConfirmation)return false;
    closeConfirmation=true;
    RECT anchor{};GetWindowRect(window,&anchor);anchor.bottom=anchor.top+S(100);
    ToolMenu menu;const int action=menu.Show(window,anchor,dpi,uiFont,{
        {0,T(L"File có thay đổi chưa lưu",L"File has unsaved changes"),false,0xea7b},
        {1,T(L"Lưu thay đổi",L"Save changes"),!doc->draftUnavailable,0xeb4b},
        {2,T(L"Bỏ thay đổi",L"Discard changes"),true,0xea76},
        {0,T(L"Hủy thao tác",L"Cancel"),true,0xea76}});
    closeConfirmation=false;
    if(!window||!documents.Get(id))return false;
    if(action==1){if(!documents.Save(id,importError)){InvalidateRect(window,nullptr,FALSE);return false;}
        if(id==documents.Active().id&&IsCode())codeViewer.MarkSaved();return true;}
    return action==2;
}
bool ConfirmFiles(DocumentId keep){
    if(!SaveActiveState()&&keep==documents.Active().id)return false;
    std::vector<DocumentId> ids;
    for(const auto& doc:documents.Items())if(doc->id&&doc->id!=keep&&doc->dirty)ids.push_back(doc->id);
    for(auto id:ids)if(!ConfirmFile(id))return false;
    return true;
}
void RenderDocument()
{
    if(Blocked()) {Layout();return;}
    auto &doc=documents.Active();if(doc.draftUnavailable){Layout();return;}viewingFile=doc.id!=0;
    loading=true;SendMessageW(editor,EM_EXLIMITTEXT,0,viewingFile?5*1024*1024:500000);
    SetWindowTextW(editor,viewingFile?(doc.dirty?doc.draft.c_str():doc.content.text.c_str()):scriptDraft.c_str());loading=false;
    reading=viewingFile?!preferences.allowFileEditing:scriptReading;
    SendMessageW(editor,EM_SETREADONLY,viewingFile?!preferences.allowFileEditing:reading,0);
    fontPoints=doc.state.textPoints;codePoints=doc.state.textPoints;
    imageZoom=doc.state.zoom;panX=doc.state.panX;panY=doc.state.panY;
    if(IsCode()) {
        LoadedContent draft;draft.displayName=doc.content.displayName;draft.text=doc.dirty?doc.draft:doc.content.text;
        if(!codeViewer.Open(draft,viewerError)) {Layout();return;}
        codeViewer.SetEditable(preferences.allowFileEditing);
        codeViewer.SetState({doc.state.scrollX,doc.state.scrollY,doc.state.zoom,doc.state.page,doc.state.panX,doc.state.panY,doc.state.wrap});
    } else if(IsPdf()) {
        if(!pdfViewer.Open(doc.content,viewerError)) {Layout();return;}
        pdfViewer.SetState({doc.state.scrollX,doc.state.scrollY,doc.state.zoom,doc.state.page,doc.state.panX,doc.state.panY,doc.state.wrap,doc.state.fitWidth});
    }
    ApplyTextWrap();
    ApplyTextFormat();POINT scroll{doc.state.scrollX,doc.state.scrollY};SendMessageW(editor,EM_SETSCROLLPOS,0,(LPARAM)&scroll);
    page=Page::Content;Layout();SetFocus(ContentWindow());
}
void ActivateDocument(DocumentId id)
{
    if(Blocked())return;Pause();if(!SaveActiveState())return;documentHost.CloseExternal();
    if(!documents.Activate(id,importError)){RenderDocument();return;}
    viewingFile=id!=0;RenderDocument();documents.EvictInactiveHeavy();
}
void CloseDocument(DocumentId id)
{
    if(Blocked() || id==0)return;if(!SaveActiveState()&&id!=documents.Active().id)return;
    if(!ConfirmFile(id))return;if(reloadPending==id)reloadPending=0;Pause();documentHost.CloseExternal();
    documents.Close(id);viewingFile=documents.Active().id!=0;RenderDocument();documents.EvictInactiveHeavy();
}
void ShowScript()
{
    if(Blocked())return;if(!SaveActiveState())return;documentHost.CloseExternal();documents.Activate(0);
    viewingFile=false;focusMode=false;reading=false;scriptReading=false;RenderDocument();
}
void ShowOpenedFile()
{
    if(Blocked() || documents.Active().id==0)return;if(!SaveActiveState())return;RenderDocument();
}
bool OpenContentPath(const std::wstring &path)
{
    if(Blocked())return false;if(!SaveActiveState())return false;DocumentId previous=documents.Active().id;
    documentHost.CloseExternal();auto result=documents.Open(path,importError);
    if(result.status==OpenStatus::ReloadRequired) {
        reloadPending=result.id;findVisible=false;RenderDocument();return false;
    }
    if(!result) {RenderDocument();return false;}
    viewingFile=true;viewerError.clear();RenderDocument();
    if(!viewerError.empty()) {
        importError=viewerError;documentHost.CloseExternal();
        if(result.status==OpenStatus::Opened)documents.Close(result.id);
        documents.Activate(previous);viewingFile=previous!=0;viewerError.clear();RenderDocument();return false;
    }
    pickerFolder=LocalParentFolder(ActiveContent().path);
    importError.clear();recentWorkspace.Touch(documents.Active().canonicalPath,false);PersistRecent();documents.EvictInactiveHeavy();return true;
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
            recentWorkspace.Touch(projectTree.Root(),true);PersistRecent();treeVisible=true;page=Page::Content;focusMode=false;
            TreeView_Expand(projectTree.Handle(),TreeView_GetRoot(projectTree.Handle()),TVE_EXPAND);
        }
    }
    Layout();
    if(!Blocked()&&IsWindowVisible(projectTree.Handle()))SetFocus(projectTree.Handle());
}
void ChooseMeetingDirectory()
{
    if(Blocked())return;
    Pause();LocalPicker picker;
    auto paths=picker.Show(window,true,meetingSettings.outputDirectory,english,uiFont,filePicker,importError);
    if(!paths.empty()) {
        meetingSettings.outputDirectory=paths.front();
        if(!SaveMeetingSettings(dataDir,meetingSettings))
            importError=T(L"Không lưu được thư mục cuộc họp.",L"The meeting folder could not be saved.");
        else {
            importError.clear();
            transcriptionNotice=T(L"Đã đổi thư mục lưu cuộc họp",L"Meeting output folder changed");
        }
    }
    Layout();
}
void ImportMeetingFile()
{
    if(Blocked())return;
    Pause();LocalPicker picker;
    auto paths=picker.Show(window,false,meetingSettings.outputDirectory,english,uiFont,filePicker,importError);
    if(paths.empty()){Layout();return;}
    std::wstring text,error;
    if(!ImportMeetingTranscript(paths.front(),meetingSettings.provider,text,error)) {
        importError=error;
        Layout();
        return;
    }
    importError.clear();
    if(viewingFile)ShowScript();
    else if(page!=Page::Content){page=Page::Content;focusMode=false;Layout();}
    AppendMeetingTranscript(text,paths.front());
    Layout();
}
void ChooseCustomColor(int controlId)
{
    if(Blocked())return;
    COLORREF *target=nullptr;
    if(controlId==CUSTOM_BACKGROUND)target=&customThemeColors.background;
    else if(controlId==CUSTOM_PANEL)target=&customThemeColors.panel;
    else if(controlId==CUSTOM_TEXT)target=&customThemeColors.text;
    else if(controlId==CUSTOM_ACCENT)target=&customThemeColors.accent;
    if(!target)return;
    // Keep private content out of the native color dialog and redraw it only
    // after the protected owner window is visible again.
    documentHost.Hide();ShowWindow(editor,SW_HIDE);ShowWindow(imageViewer,SW_HIDE);
    if(mirror)mirror->Invalidate();
    static COLORREF customColors[16]{};
    CHOOSECOLORW chooser{sizeof(chooser)};
    chooser.hwndOwner=window;chooser.rgbResult=*target;chooser.lpCustColors=customColors;
    chooser.Flags=CC_FULLOPEN|CC_RGBINIT;
    if(ChooseColorW(&chooser)) {
        *target=chooser.rgbResult;
        SaveCustomTheme(dataDir,customThemeColors);
        auto candidate=preferences;candidate.themeId=CustomThemeId;StorePreferences(candidate);
    }
    Layout();
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
    const auto [src,dst]=SelectMonitorPair(monitors,source,destination);
    labeledWindowMonitor=nullptr;
    if(monitors.empty())for(HWND combo:{sourceMonitor,destinationMonitor})SendMessageW(combo,CB_RESETCONTENT,0,0);
    UpdateMonitorLabels();
    SendMessageW(sourceMonitor,CB_SETCURSEL,src,0);SendMessageW(destinationMonitor,CB_SETCURSEL,dst,0);
}
void UpdateMonitorLabels()
{
    if(!window||!sourceMonitor||!destinationMonitor||monitors.empty())return;
    const auto current=MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST);
    if(labeledWindowMonitor==current)return;
    labeledWindowMonitor=current;
    for(HWND combo:{sourceMonitor,destinationMonitor}) {
        const int selected=(int)SendMessageW(combo,CB_GETCURSEL,0,0);
        SendMessageW(combo,CB_RESETCONTENT,0,0);
        for(size_t i=0;i<monitors.size();++i) {
            const auto& monitor=monitors[i];
            std::wstring label=T(L"Màn ",L"Display ")+std::to_wstring(monitor.displayNumber?monitor.displayNumber:(int)i+1);
            if(monitor.primary)label+=T(L" · Chính",L" · Main");
            if(monitor.handle==current)label+=T(L" · Cửa sổ này",L" · This window");
            label+=L" · "+std::to_wstring(monitor.bounds.right-monitor.bounds.left)+L"x"+std::to_wstring(monitor.bounds.bottom-monitor.bounds.top);
            SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)label.c_str());
        }
        SendMessageW(combo,CB_SETCURSEL,std::clamp(selected,0,(int)monitors.size()-1),0);
        SendMessageW(combo,CB_SETDROPPEDWIDTH,S(480),0);
    }
}
void VerifyPrivacy()
{
    if(qa || verifying || !mirror || masked || storageBlocked)return;
    if(mirror->Running()) {
        // A quick check validates the live session; a new canary would replace
        // its capture and deliberately stop an otherwise healthy projection.
        if(!mirror->HandleDisplayChange()){browser.Lock();documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);}
        Layout();return;
    }
    UpdateMonitors();
    verifying=true;documentHost.Hide();Layout();privacy=ApplyPrivacy(window);topologyKnown=TryGetDisplayCloned(cloned);
    mirror->Verify(window,SelectedMonitor(sourceMonitor));
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
        std::wstring label=T(vn,en);
        const std::pair<int,PreferenceCommand> bindings[]={{OPEN_FILE,PreferenceCommand::OpenFile},{OPEN_FOLDER,PreferenceCommand::OpenFolder},
            {MODE,PreferenceCommand::Reading},{FOCUS,PreferenceCommand::Focus},{PLAY,PreferenceCommand::AutoScroll}};
        for(const auto& binding:bindings)if(id==binding.first)label=label.substr(0,label.find(L"   "))+L"   "+ShortcutLabel(preferences.Key(binding.second));
        entries.push_back({id,label,enabled,icon});};
    item(OPEN_FILE,L"Mở file...   Ctrl+O",L"Open files...   Ctrl+O",safe);
    item(OPEN_FOLDER,L"Mở dự án...   Ctrl+Shift+O",L"Open project...   Ctrl+Shift+O",safe);
    item(SAVE_FILE,L"Lưu file   Ctrl+S",L"Save file   Ctrl+S",safe&&viewingFile&&preferences.allowFileEditing&&!IsImage()&&!IsPdf());
    item(HOME_SCRIPT,L"Soạn lời nhắc",L"Compose script",safe);
    item(WRAP,L"Tự xuống dòng",L"Word wrap",safe&&!IsImage()&&!IsPdf());
    if(!IsCode()&&!IsImage()&&!IsPdf())item(TEXT_WEIGHT,L"Độ đậm chữ",L"Text weight",safe);
    item(FIND,L"Tìm...   Ctrl+F",L"Find...   Ctrl+F",safe&&!IsImage());
    item(MODE,L"Soạn / Đọc   Ctrl+Enter",L"Edit / Read   Ctrl+Enter",safe);
    if(!IsImage()&&!IsCode()&&!IsPdf()) {
        item(PLAY,L"Chạy / Dừng cuộn   Space",L"Start / stop scrolling   Space",safe);
    }
    if(IsPdf()) {
        item(PDF_PREV,L"Trang PDF trước",L"Previous PDF page",safe);item(PDF_NEXT,L"Trang PDF sau",L"Next PDF page",safe);
        item(FIT_WIDTH,L"PDF vừa chiều ngang",L"Fit PDF width",safe);
    }
    item(FOCUS,L"Tập trung   F11",L"Focus   F11");
    item(VERIFY_PRIVACY,L"Kiểm tra WGC",L"Verify WGC",!masked&&!verifying);
    item(MIRROR_ACTION,mirror&&mirror->Running()?L"Dừng trình chiếu":L"Bắt đầu trình chiếu",mirror&&mirror->Running()?L"Stop projection":L"Start projection",IsWindowEnabled(GetDlgItem(window,MIRROR_ACTION)));
    item(NAV_SETTINGS,L"Cài đặt...",L"Settings...");
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
void OpenContentMenu()
{
    if(Blocked())return;
    std::vector<DocumentId> ids;
    for(const auto& doc:documents.Items())if(doc->id)ids.push_back(doc->id);
    size_t offset=0;
    while(!Blocked()) {
        std::vector<ToolMenuItem> entries;
        const size_t end=std::min(ids.size(),offset+8);
        for(size_t i=offset;i<end;++i)if(const auto* doc=documents.Get(ids[i]))
            entries.push_back({7000+int(i),doc->displayName+(doc->dirty?L" *":L""),true,
                               0xea7f,doc->id==documents.Active().id});
        if(offset)entries.push_back({6998,T(L"Tệp trước",L"Previous files"),true,0xea9b});
        if(end<ids.size())entries.push_back({6999,T(L"Tệp tiếp",L"More files"),true,0xea9c});
        entries.push_back({OPEN_FILE,T(L"Mở file…",L"Open files…"),true,0xea7f});
        entries.push_back({HOME_SCRIPT,T(L"Soạn lời nhắc",L"Compose script"),true,0xea73,documents.Active().id==0});
        RECT anchor{};GetWindowRect(GetDlgItem(window,NAV_SCRIPT),&anchor);
        ToolMenu menu;const int action=menu.Show(window,anchor,dpi,uiFont,std::move(entries));
        if(Blocked())return;
        if(action==6998){offset-=8;continue;}
        if(action==6999){offset+=8;continue;}
        if(action>=7000&&size_t(action-7000)<ids.size()) {
            focusMode=false;
            if(ids[action-7000]==documents.Active().id){page=Page::Content;Layout();SetFocus(ContentWindow());}
            else ActivateDocument(ids[action-7000]);
        } else if(action)SendMessageW(window,WM_COMMAND,action,0);
        return;
    }
}
void LayoutMenu()
{
    std::vector<ToolMenuItem> entries={
        {SHOW_ACTIVITY,T(L"Thanh điều hướng",L"Activity bar"),true,0xeb51,activityVisible},
        {SHOW_TREE,T(L"Explorer   Ctrl+B",L"Explorer   Ctrl+B"),!Blocked(),0xeaf7,treeVisible},
        {SHOW_TABS,T(L"Thanh tab",L"Tab bar"),true,0xea7f,tabsVisible},
        {SHOW_TOOLBAR,T(L"Thanh công cụ",L"Toolbar"),true,0xeb51,toolbarVisible},
        {SHOW_STATUS,T(L"Thanh trạng thái",L"Status bar"),true,0xeb01,statusVisible},
        {RESET_LAYOUT,T(L"Khôi phục bố cục",L"Reset layout"),true,0xeb37},
        {NAV_HOME,T(L"Trang chủ",L"Home"),true,0xeb06},
        {NAV_SCRIPT,T(L"Tệp đang mở",L"Open files"),!Blocked(),0xeaf0},
        {HOME_SCRIPT,T(L"Soạn lời nhắc",L"Compose script"),!Blocked(),0xea73},
        {NAV_BROWSER,T(L"Trình duyệt",L"Browser"),true,0xeb01},
        {NAV_PRESENTATION,T(L"Trình chiếu",L"Presentation"),true,0xea7a},
        {NAV_SETTINGS,T(L"Cài đặt",L"Settings"),true,0xeb51}};
    RECT anchor{};GetWindowRect(GetDlgItem(window,LAYOUT_MENU),&anchor);
    ToolMenu menu;const int selected=menu.Show(window,anchor,dpi,uiFont,std::move(entries));
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
    RECT client{};GetClientRect(window,&client);
    if(page==Page::Home){HBRUSH home=CreateSolidBrush(HOME_BG);FillRect(dc,&client,home);DeleteObject(home);}else FillRect(dc,&client,bgBrush);
    if(!focusMode&&activityVisible)DrawSidebar(dc,railWidth,client.bottom,true);
    RECT title{S(contentLeft),S(12),S(contentRight),S(40)};
    if(page==Page::Settings){title.left=settingsViewport.left+S(16);title.right=settingsViewport.right-S(16);}
    if(page==Page::Home){title.right=S(contentRight-(homeCompact?64:0));title.top=S(homeCompact?8:34);title.bottom=S(homeCompact?36:66);
        if(!homeCompact)DrawTextAt(dc,L"WORKSPACE / HOME",{S(contentLeft),S(10),S(contentRight-72),S(28)},MUTED,metaFont);}
    if(!focusMode && page!=Page::Content && !(page==Page::Browser&&browser.Enabled()))DrawTextAt(dc,page==Page::Home?T(L"Trang chủ",L"Home"):
        page==Page::Settings?T(L"Cài đặt",L"Settings"):page==Page::Browser?T(L"Trình duyệt",L"Browser"):T(L"Trình chiếu",L"Presentation"),title,TEXT,page==Page::Home&&homeCompact?brandFont:titleFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    if(page==Page::Content || focusMode) {
        FillRoundRect(dc,contentCardRect,CARD,S(12));
        if(explorerRect.right>explorerRect.left) {
            FillRoundRect(dc,explorerRect,CARD,S(12));
            RECT header=explorerRect;header.left+=S(12);header.right-=S(32);header.bottom=header.top+S(32);
            DrawTextAt(dc,L"Explorer",header,MUTED,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            if(splitterHovered||splitterDragging) {
                RECT line=splitterRect;line.left+=S(3);line.right-=S(3);
                FillRoundRect(dc,line,ACCENT,S(2));
            }
        }
        if(Blocked()) {
            const auto revealHint=ShortcutLabel(preferences.Key(PreferenceCommand::Mask))+T(L" để mở lại.",L" to reveal.");
            const wchar_t *heading=storageBlocked?T(L"Lời nhắc đang khóa",L"Script locked"):
                masked?T(L"Đã che nội dung",L"Content hidden"):
                cloned?T(L"Đang dùng Duplicate",L"Duplicate display mode"):
                verifying?T(L"Đang kiểm tra WGC",L"Verifying WGC privacy"):
                !topologyKnown?T(L"Chưa xác định màn hình",L"Display layout unknown"):T(L"Chưa xác minh capture",L"Capture not verified");
            const wchar_t *body=storageBlocked?T(L"Bản lưu cũ được giữ nguyên. Kiểm tra quyền đọc tệp lời nhắc.",L"Old script preserved. Check access to the saved script."):
                masked?revealHint.c_str():
                cloned?T(L"Win+P > Extend, hoặc chia sẻ riêng một cửa sổ.",L"Use Win+P > Extend, or share an individual window."):
                T(L"Bấm Kiểm tra WGC ở thanh bên hoặc Tổng quan. Nội dung chỉ mở sau khi canary đạt.",L"Verify WGC from the activity bar or Overview. Content opens only after the canary passes.");
            DrawCenteredMessage(dc,contentCardRect,heading,body);
        }
    }
    else if(page==Page::Browser) {
        FillRoundRect(dc,contentCardRect,CARD,S(12));
        if(!Blocked()&&browser.Enabled()&&IsWindowVisible(webAddress)) {
            RECT input{};GetWindowRect(webAddress,&input);MapWindowPoints(nullptr,window,(POINT*)&input,2);InflateRect(&input,1,1);
            HPEN border=CreatePen(PS_SOLID,1,DIVIDER);auto old=SelectObject(dc,border),brush=SelectObject(dc,GetStockObject(NULL_BRUSH));
            RoundRect(dc,input.left,input.top,input.right,input.bottom,S(6),S(6));SelectObject(dc,brush);SelectObject(dc,old);DeleteObject(border);
        }
        const auto browserPrivacyHint=ShortcutLabel(preferences.Key(PreferenceCommand::Mask))+T(L" để mở lại hoặc kiểm tra WGC.",L" to reveal, or verify WGC.");
        if(Blocked()||!browser.Ready())DrawCenteredMessage(dc,contentCardRect,
            Blocked()?T(L"Nội dung đang che",L"Content hidden"):T(L"Web",L"Web"),
            Blocked()?browserPrivacyHint.c_str():browser.Status(english).c_str());
    }
    else if(page==Page::Home) {
        DrawTextAt(dc,T(L"Gần đây",L"Recent"),{S(contentLeft),homeRecentRect.top-S(32),S(contentRight-164),homeRecentRect.top-S(8)},TEXT,brandFont);
        FillRoundRect(dc,homeRecentRect,HOME_CARD,S(10));
        if(Blocked()||recentWorkspace.entries.empty()||!recentRows){
            auto hint=Blocked()?T(L"Xác minh WGC để xem lịch sử.",L"Verify WGC to view your history."):
                !recentRows?T(L"Tăng chiều cao cửa sổ để xem lịch sử.",L"Increase window height to view history."):
                T(L"Chưa có file hay dự án gần đây.",L"No recent files or projects yet.");
            RECT empty=homeRecentRect;InflateRect(&empty,-S(16),-S(8));DrawTextAt(dc,hint,empty,MUTED,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        }
        const int protectionTop=MulDiv(homeProtectionRect.top,96,dpi)-24;
        DrawTextAt(dc,T(L"Trạng thái bảo vệ",L"Protection status"),{S(contentLeft),S(protectionTop),S(contentRight-76),S(protectionTop+22)},TEXT,brandFont);
        FillRoundRect(dc,homeProtectionRect,HOME_CARD,S(10));
        DWORD affinity=0;const bool excluded=GetWindowDisplayAffinity(window,&affinity)&&affinity==0x11;
        RECT line=homeProtectionRect;line.left+=S(34);line.right-=S(12);line.top+=S(homeCompact?3:7);line.bottom=line.top+S(homeCompact?18:24);
        DrawCodicon(dc,excluded?0xeb77:0xeb53,{homeProtectionRect.left+S(10),line.top,homeProtectionRect.left+S(28),line.bottom},excluded?GREEN:AMBER,S(16));
        DrawTextAt(dc,excluded?T(L"Windows đã nhận loại trừ capture",L"Windows accepted capture exclusion"):
            T(L"Windows chưa nhận loại trừ capture",L"Windows has not accepted capture exclusion"),line,excluded?TEXT:AMBER,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        line.top+=S(homeCompact?20:26);line.bottom+=S(homeCompact?20:26);
        DrawCodicon(dc,0xeb53,{homeProtectionRect.left+S(10),line.top,homeProtectionRect.left+S(28),line.bottom},AMBER,S(16));
        DrawTextAt(dc,T(L"Người xem / đầu nhận vật lý chưa được xác minh",L"Viewer / physical receiver unverified"),line,AMBER,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        if(projectionStoppedNotice||!importError.empty())DrawTextAt(dc,projectionStoppedNotice?T(L"Trình chiếu đã dừng - xem Chi tiết.",L"Projection stopped - see Details."):importError,
            {S(contentLeft),S(42),S(contentRight),S(66)},AMBER,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

    }
    else if(page==Page::Settings) {
        FillRoundRect(dc,settingsCardRect,CARD,S(12));
        const int saved=SaveDC(dc);IntersectClipRect(dc,settingsViewport.left,settingsViewport.top,settingsViewport.right,settingsViewport.bottom);
        for(const auto& line:settingsLines) {
            if(line.control==-1) {
                std::wstring status=mirror?mirror->Status(english):L"";
                if(mirror&&mirror->Running())status+=L"\nFPS: "+std::to_wstring((int)mirror->Fps());
                DrawTextAt(dc,status,line.bounds,MUTED,uiFont,DT_LEFT|DT_WORDBREAK);
                continue;
            }
            if(line.heading)DrawTextAt(dc,line.text,line.bounds,TEXT,brandFont);
            else {
                RECT measure{0,0,line.bounds.right-line.bounds.left,0};auto old=SelectObject(dc,uiFont);
                DrawTextW(dc,line.text.c_str(),-1,&measure,DT_LEFT|DT_WORDBREAK|DT_CALCRECT);SelectObject(dc,old);
                RECT label=line.bounds;label.top+=(label.bottom-label.top-measure.bottom)/2;label.bottom=label.top+measure.bottom;
                DrawTextAt(dc,line.text,label,TEXT,uiFont,DT_LEFT|DT_WORDBREAK);
            }
        }
        for(const auto& setting:numericSettings) {
            FillRoundRect(dc,setting.bounds,ELEVATED,S(8));
            const HWND focus=GetFocus();const bool focused=focus==setting.input||focus==GetDlgItem(window,setting.decrement)||focus==GetDlgItem(window,setting.increment);
            HPEN pen=CreatePen(PS_SOLID,1,setting.input==sizeInput&&!sizeError.empty()?AMBER:focused?ACCENT:DIVIDER);
            auto old=SelectObject(dc,pen),brush=SelectObject(dc,GetStockObject(NULL_BRUSH));
            RoundRect(dc,setting.bounds.left,setting.bounds.top,setting.bounds.right,setting.bounds.bottom,S(8),S(8));
            SelectObject(dc,brush);SelectObject(dc,old);DeleteObject(pen);
        }
        if(IsWindowVisible(sizeInput)) {
        DrawTextAt(dc,sizeError.empty()?(IsCode()||IsPdf()||IsImage()?L"25-400%":L"8-200 pt"):sizeError,
            settingsSizeHint,sizeError.empty()?MUTED:AMBER,metaFont,DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        }
        RestoreDC(dc,saved);
    }
    else if(page==Page::Presentation) {
        DrawTextAt(dc,T(L"Nguồn",L"Source"),{S(contentLeft),S(48),S(contentLeft+80),S(76)},MUTED,metaFont);
        DrawTextAt(dc,TextOf(sourceMonitor),{S(contentLeft+88),S(48),S(contentRight),S(76)},TEXT,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        DrawTextAt(dc,T(L"Đích",L"Destination"),{S(contentLeft),S(80),S(contentLeft+80),S(108)},MUTED,metaFont);
        DrawTextAt(dc,TextOf(destinationMonitor),{S(contentLeft+88),S(80),S(contentRight),S(108)},TEXT,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        RECT status{S(contentLeft),S(232),S(contentRight),client.bottom-S(24)};
        std::wstring text=mirror?mirror->Status(english):L"";
        if(monitors.size()<2)text+=T(L"\nCần hai màn hình ở chế độ Extend để chiếu.",L"\nTwo displays in Extend are required for projection.");
        else text+=T(L"\nĐầu nhận vật lý chưa được xác minh.",L"\nPhysical receiver unverified.");
        if(mirror && mirror->Running())text+=L"\nFPS: "+std::to_wstring((int)mirror->Fps());
        DrawTextAt(dc,text,status,MUTED,metaFont,DT_LEFT|DT_WORDBREAK);
    }
    if(focusMode||!statusVisible)return;
    RECT footer{S(contentLeft),client.bottom-S(20),S(contentRight),client.bottom-S(2)};
    std::wstring label=projectionStoppedNotice?T(L"Trình chiếu đã dừng - xem Cài đặt > Trình chiếu.",L"Projection stopped - see Settings > Projection."):
        Blocked()?T(L"Nội dung đang khóa",L"Content locked"):
        meetingTranscriber.Running()?meetingTranscriber.Status(english):!transcriptionNotice.empty()?transcriptionNotice:
        (meetingRecorder.Running()||meetingRecorder.ErrorCode()!=0)?meetingRecorder.Status(english):page==Page::Browser?(browser.Ready()?browser.Status(english):L""):
        saveFailed?T(L"Không lưu được lời nhắc",L"Script save failed"):
        savedNoteRecovered&&!dirty&&!reading?T(L"Đã giữ bản dự phòng lời nhắc cũ",L"Old script recovery copy preserved"):
        dirty?T(L"Đang lưu...",L"Saving..."):!sizeError.empty()?sizeError:!findMessage.empty()?findMessage:
        !importError.empty()?importError:IsPdf()?std::to_wstring(pdfViewer.GetState().page+1)+L" / "+std::to_wstring(pdfViewer.PageCount()):
        viewingFile?ActiveContent().displayName:L"";
    if(page==Page::Home&&!projectionStoppedNotice&&!saveFailed&&importError.empty())label=L"QuietShare " QUIETSHARE_VERSION_WIDE;
    size_t split=label.find(L" / ");if(split!=std::wstring::npos && !IsPdf())label=english?label.substr(split+3):label.substr(0,split);
    DrawTextAt(dc,label,footer,projectionStoppedNotice||saveFailed||!importError.empty()?AMBER:MUTED,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
}
void DrawActionIcon(HDC dc,int id,RECT r,COLORREF color)
{
    wchar_t glyph=id==SMALLER?0xeb3b:id==LARGER?0xea60:id==FOCUS?(focusMode?0xeb4d:0xeb4c):
        id==MASK?(masked?0xeae7:0xea70):id==WEB_NEW_TAB?0xea60:id==TOPMOST||id==PIN_WINDOW?0xeb2b:id==OPEN_FILE?0xea7f:
        id==PLAY?(running?0xead1:0xeb2c):id==MODE?0xea73:id==OVERFLOW||id==WEB_MENU?0xea7c:id==LAYOUT_MENU?0xebeb:id==SHOW_TREE?0xea76:
        id==BROWSER_TOGGLE||id==FIND_CLOSE?0xea76:id==WEB_GO?0xea9c:
        id==WEB_ZOOM_OUT||id==OPACITY_LESS||id==ICON_SMALLER?0xeb3b:
        id==WEB_ZOOM_IN||id==OPACITY_MORE||id==ICON_LARGER?0xea60:
        id==WEB_AUDIO?(preferences.browserMuted?0xeb24:0xeb75):
        id==MEETING_RECORD?(meetingRecorder.Running()?0xeba5:0xeba7):
        id==FIND_PREV?0xeab5:0xeab6;
    DrawCodicon(dc,glyph,r,color,S(14));
}
void DrawButtonContents(DRAWITEMSTRUCT *item)
{
    if(page==Page::Home&&(item->CtlID==OPEN_DROP||item->CtlID==OPEN_FOLDER||(item->CtlID>=RECENT_FIRST&&item->CtlID<RECENT_FIRST+18))){
        const bool action=item->CtlID==OPEN_DROP||item->CtlID==OPEN_FOLDER;
        const bool disabled=(item->itemState&ODS_DISABLED)!=0,hover=hoveredButton==item->hwndItem;
        HBRUSH homeSurface=CreateSolidBrush(action?HOME_BG:HOME_CARD);FillRect(item->hDC,&item->rcItem,homeSurface);DeleteObject(homeSurface);
        RECT r=item->rcItem;InflateRect(&r,-1,-1);
        FillRoundRect(item->hDC,r,hover?HOVER:action?(item->CtlID==OPEN_DROP?PIN_BG:HOME_CARD):HOME_CARD,S(12));
        const COLORREF color=disabled?DISABLED:TEXT;
        if(action){
            const bool shortCard=r.bottom<S(80);
            if(shortCard){
                const int mid=(r.top+r.bottom)/2;
                DrawCodicon(item->hDC,item->CtlID==OPEN_DROP?0xea7f:0xea83,{S(10),mid-S(10),S(32),mid+S(10)},disabled?DISABLED:item->CtlID==OPEN_DROP?GREEN:MUTED,S(18));
                DrawTextAt(item->hDC,T(item->CtlID==OPEN_DROP?L"Mở file":L"Mở dự án",item->CtlID==OPEN_DROP?L"Open file":L"Open project"),{S(42),S(7),r.right-S(8),S(30)},color,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
                DrawTextAt(item->hDC,T(item->CtlID==OPEN_DROP?L"File trên máy":L"Thư mục code",item->CtlID==OPEN_DROP?L"Local files":L"Code folder"),{S(42),S(31),r.right-S(8),S(51)},disabled?DISABLED:item->CtlID==OPEN_DROP?TEXT:MUTED,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            }else{
            DrawCodicon(item->hDC,item->CtlID==OPEN_DROP?0xea7f:0xea83,{S(12),S(8),S(34),S(30)},disabled?DISABLED:item->CtlID==OPEN_DROP?GREEN:MUTED,S(18));
            DrawTextAt(item->hDC,T(item->CtlID==OPEN_DROP?L"Mở file":L"Mở dự án",item->CtlID==OPEN_DROP?L"Open file":L"Open project"),{S(12),S(36),r.right-S(10),S(62)},color,brandFont);
            DrawTextAt(item->hDC,item->CtlID==OPEN_DROP?T(L"Code, PDF, ảnh, văn bản",L"Code, PDF, images, text"):T(L"Mở thư mục code",L"Open a code folder"),{S(12),S(66),r.right-S(10),S(90)},disabled?DISABLED:item->CtlID==OPEN_DROP?TEXT:MUTED,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
            }
        }else if(!Blocked()){
            const size_t index=recentOffset+(item->CtlID-RECENT_FIRST)/3;const int part=(item->CtlID-RECENT_FIRST)%3;
            if(index<recentWorkspace.entries.size()){
                const auto& entry=recentWorkspace.entries[index];
                if(part) {
                    if(part==1&&entry.pinned)FillRoundRect(item->hDC,r,hover?HOVER:PIN_BG,S(8));
                    DrawCodicon(item->hDC,part==1?0xeb2b:0xea76,r,part==1&&entry.pinned?GREEN:MUTED,S(16));
                }
                else{
                    DrawFileTypeIcon(item->hDC,FileIconForName(entry.path,entry.folder),{S(10),S(homeCompact?10:16),S(28),S(homeCompact?28:34)});
                    const auto name=entry.path.substr(entry.path.find_last_of(L"\\/")+1);
                    DrawTextAt(item->hDC,name,{S(38),S(2),r.right-S(6),S(homeCompact?20:28)},TEXT,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
                    DrawTextAt(item->hDC,entry.folder?T(L"Dự án",L"Project"):T(L"File gần đây",L"Recent file"),{S(38),S(homeCompact?20:28),r.right-S(6),S(homeCompact?36:48)},MUTED,metaFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
                }
            }
        }
        if(GetFocus()==item->hwndItem){RECT focus=r;InflateRect(&focus,-3,-3);DrawFocusRect(item->hDC,&focus);}return;
    }
    if(page==Page::Settings && (item->CtlID==REMEMBER_SIZE||item->CtlID==ALLOW_EDIT||item->CtlID==TOPMOST||item->CtlID==SHOW_ACTIVITY||item->CtlID==SHOW_TREE||item->CtlID==SHOW_TABS||
        item->CtlID==SHOW_TOOLBAR||item->CtlID==SHOW_STATUS||item->CtlID==INCLUDE_IGNORED||item->CtlID==BROWSER_TOGGLE||item->CtlID==POINTER||item->CtlID==WRAP||
        item->CtlID==BROWSER_MUTED||item->CtlID==BROWSER_CAMERA||item->CtlID==BROWSER_MICROPHONE||item->CtlID==MEETING_TRANSCRIBE)) {
        const int id=item->CtlID;const bool value=id==REMEMBER_SIZE?preferences.rememberWindowSize:id==ALLOW_EDIT?preferences.allowFileEditing:id==TOPMOST?topmost:id==SHOW_ACTIVITY?activityVisible:id==SHOW_TREE?treeVisible:
            id==SHOW_TABS?tabsVisible:id==SHOW_TOOLBAR?toolbarVisible:id==SHOW_STATUS?statusVisible:id==INCLUDE_IGNORED?projectTree.IncludeIgnored():
            id==BROWSER_TOGGLE?browser.Enabled():id==POINTER?mirrorPointer:id==BROWSER_MUTED?preferences.browserMuted:
            id==BROWSER_CAMERA?preferences.browserAllowCamera:id==BROWSER_MICROPHONE?preferences.browserAllowMicrophone:id==MEETING_TRANSCRIBE?meetingSettings.transcriptionEnabled:IsCode()?codeViewer.GetState().wrap:documents.Active().state.wrap;
        const bool disabled=item->itemState&ODS_DISABLED,hover=hoveredButton==item->hwndItem;
        FillRect(item->hDC,&item->rcItem,cardBrush);
        RECT button=item->rcItem;InflateRect(&button,-1,-1);
        if(hover&&!disabled)FillRoundRect(item->hDC,button,HOVER,S(8));
        RECT track{button.right-S(44),button.top+S(6),button.right-S(8),button.top+S(24)};
        FillRoundRect(item->hDC,track,disabled?DIVIDER:value?(id==TOPMOST?PIN_BG:ACCENT):ELEVATED,S(18));
        const int knobLeft=track.left+S(value?20:2);
        RECT knob{knobLeft,track.top+S(2),knobLeft+S(14),track.bottom-S(2)};
        FillRoundRect(item->hDC,knob,disabled?DISABLED:TEXT,S(14));
        RECT label=button;label.right=track.left-S(8);
        DrawTextAt(item->hDC,T(value?L"Bật":L"Tắt",value?L"On":L"Off"),label,disabled?DISABLED:MUTED,uiFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        if(GetFocus()==item->hwndItem&&!(SendMessageW(window,WM_QUERYUISTATE,0,0)&UISF_HIDEFOCUS)) {
            HPEN pen=CreatePen(PS_SOLID,std::max(1,S(1)),ACCENT);auto old=SelectObject(item->hDC,pen),brush=SelectObject(item->hDC,GetStockObject(NULL_BRUSH));
            RoundRect(item->hDC,button.left,button.top,button.right,button.bottom,S(8),S(8));SelectObject(item->hDC,brush);SelectObject(item->hDC,old);DeleteObject(pen);
        }
        return;
    }
    if(page==Page::Settings && (item->CtlID==CUSTOM_BACKGROUND||item->CtlID==CUSTOM_PANEL||item->CtlID==CUSTOM_TEXT||item->CtlID==CUSTOM_ACCENT)) {
        const bool disabled=(item->itemState&ODS_DISABLED)!=0,hover=hoveredButton==item->hwndItem&&!disabled;
        FillRect(item->hDC,&item->rcItem,cardBrush);
        RECT button=item->rcItem;InflateRect(&button,-1,-1);if(hover)FillRoundRect(item->hDC,button,HOVER,S(8));
        COLORREF color=item->CtlID==CUSTOM_BACKGROUND?customThemeColors.background:item->CtlID==CUSTOM_PANEL?customThemeColors.panel:item->CtlID==CUSTOM_TEXT?customThemeColors.text:customThemeColors.accent;
        RECT swatch{button.left+S(8),button.top+S(7),button.left+S(38),button.bottom-S(7)};
        FillRoundRect(item->hDC,swatch,disabled?DISABLED:color,S(6));
        HPEN border=CreatePen(PS_SOLID,std::max(1,S(1)),disabled?DIVIDER:theme.border);auto oldPen=SelectObject(item->hDC,border),oldBrush=SelectObject(item->hDC,GetStockObject(NULL_BRUSH));
        RoundRect(item->hDC,swatch.left,swatch.top,swatch.right,swatch.bottom,S(6),S(6));SelectObject(item->hDC,oldBrush);SelectObject(item->hDC,oldPen);DeleteObject(border);
        DrawTextAt(item->hDC,ColorHex(color),{button.left+S(48),button.top,button.right-S(8),button.bottom},disabled?DISABLED:TEXT,uiFont,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        if(GetFocus()==item->hwndItem&&!(SendMessageW(window,WM_QUERYUISTATE,0,0)&UISF_HIDEFOCUS)){RECT focus=button;InflateRect(&focus,-2,-2);DrawFocusRect(item->hDC,&focus);}
        return;
    }
    bool disabled = (item->itemState & ODS_DISABLED) != 0;
    const bool railAction=(item->CtlID==VERIFY_PRIVACY||item->CtlID==MIRROR_ACTION||item->CtlID==MEETING_RECORD)&&item->rcItem.right<=S(36);
    const bool category=(item->CtlID>=SETTINGS_APPEARANCE&&item->CtlID<=SETTINGS_KEYS)||item->CtlID==SETTINGS_BROWSER;
    bool nav = railAction||item->CtlID == NAV_SCRIPT || item->CtlID == NAV_READ || item->CtlID == NAV_HOME || item->CtlID == NAV_SETTINGS || item->CtlID==NAV_PRESENTATION || item->CtlID==NAV_BROWSER;
    bool activeNav = (item->CtlID == NAV_SCRIPT && page == Page::Content) ||
                     (item->CtlID == NAV_READ && page == Page::Content && reading) ||
                     (item->CtlID == NAV_HOME && page == Page::Home) ||
                     (item->CtlID == NAV_SETTINGS && page == Page::Settings) || (item->CtlID==NAV_PRESENTATION && page==Page::Presentation) || (item->CtlID==NAV_BROWSER&&page==Page::Browser) ||
                     (category&&settingsCategory==(item->CtlID==SETTINGS_BROWSER?5:item->CtlID-SETTINGS_APPEARANCE))||(item->CtlID==MIRROR_ACTION&&mirror&&mirror->Running())||
                     (item->CtlID==MEETING_RECORD&&meetingRecorder.Running());
    bool primary = item->CtlID == PLAY || item->CtlID == OPEN_FILE || (item->CtlID==BROWSER_TOGGLE&&!browser.Enabled());
    bool pressed = (item->itemState & ODS_SELECTED) != 0;
    bool hover = hoveredButton == item->hwndItem && !disabled;
    bool zoom = item->CtlID == SMALLER || item->CtlID == LARGER;
    const bool stepper=StepperButton(item->CtlID);
    const bool readingToggle=(item->CtlID==WRAP&&(IsCode()?codeViewer.GetState().wrap:documents.Active().state.wrap))||(item->CtlID==TEXT_WEIGHT&&preferences.textWeight>=600);
    const bool pinnedWindow=(item->CtlID==PIN_WINDOW||item->CtlID==TOPMOST)&&topmost;
    COLORREF color = disabled    ? CARD
                     : pressed   ? (primary ? ACCENT_DARK : DIVIDER)
                     : hover     ? HOVER
                     : activeNav ? (page==Page::Home?PIN_BG:ACCENT_DARK)
                     : primary   ? ACCENT
                     : nav       ? SIDEBAR
                                 : ELEVATED;
    if (primary && hover && !pressed)
        color = RGB(0,96,160);
    if(page==Page::Browser&&!nav&&!(item->CtlID==BROWSER_TOGGLE&&!browser.Enabled())&&!hover&&!pressed)
        color=BG;
    const bool settingsRow=page==Page::Settings&&std::find(clippedSettingsControls.begin(),clippedSettingsControls.end(),item->hwndItem)!=clippedSettingsControls.end();
    if(page==Page::Home&&!nav&&!primary)color=hover?HOVER:HOME_BG;
    if(readingToggle&&!disabled)color=hover?HOVER:ACCENT_DARK;
    if(pinnedWindow&&!disabled)color=hover?HOVER:PIN_BG;
    HBRUSH background = CreateSolidBrush(page==Page::Home?(nav?HOME_RAIL:HOME_BG):nav ? SIDEBAR : zoom||stepper ? ELEVATED : settingsRow ? CARD : BG);
    FillRect(item->hDC, &item->rcItem, background);
    DeleteObject(background);
    RECT button = item->rcItem;
    InflateRect(&button, -1, -1);
    if ((!zoom&&!stepper) || hover || pressed)
        FillRoundRect(item->hDC, button, color, S(8));
    bool strip=page==Page::Content || page==Page::Browser || focusMode;
    if (nav)
    {
        COLORREF iconColor = disabled ? DISABLED : (activeNav ? TEXT : MUTED);
        wchar_t glyph=item->CtlID==NAV_HOME?0xeb06:item->CtlID==NAV_SCRIPT?0xeaf0:
            item->CtlID==NAV_SETTINGS?0xeb51:item->CtlID==NAV_BROWSER?0xeb01:item->CtlID==VERIFY_PRIVACY?0xeab2:
            item->CtlID==MIRROR_ACTION?(mirror&&mirror->Running()?0xead1:0xeb2c):
            item->CtlID==MEETING_RECORD?(meetingRecorder.Running()?0xeba5:0xeba7):0xea7a;
        if(railAction&&!disabled&&((mirror&&mirror->IsVerified())||item->CtlID==MEETING_RECORD))
            iconColor=item->CtlID==MIRROR_ACTION&&mirror->Running()?GREEN:
                item->CtlID==VERIFY_PRIVACY?GREEN:meetingRecorder.Running()?RGB(220,80,72):TEXT;
        DrawCodicon(item->hDC,glyph,button,iconColor,S(16));
    }
    else if(item->CtlID==WEB_BACK||item->CtlID==WEB_FORWARD||item->CtlID==WEB_RELOAD) {
        DrawCodicon(item->hDC,item->CtlID==WEB_BACK?0xea9b:item->CtlID==WEB_FORWARD?0xea9c:0xeb37,button,disabled?DISABLED:TEXT,S(14));
    }
    else if(item->CtlID==RECENT_PREV||item->CtlID==RECENT_NEXT)
        DrawCodicon(item->hDC,item->CtlID==RECENT_PREV?0xea9b:0xea9c,button,disabled?DISABLED:TEXT,S(16));
    else if(stepper||item->CtlID==WEB_MENU||item->CtlID==WEB_NEW_TAB||item->CtlID==WEB_GO||item->CtlID==WEB_ZOOM_OUT||item->CtlID==WEB_ZOOM_IN||item->CtlID==WEB_AUDIO||item->CtlID==MEETING_RECORD||
        (item->CtlID==BROWSER_TOGGLE&&page==Page::Browser&&browser.Enabled())||
        item->CtlID==PIN_WINDOW||item->CtlID==LAYOUT_MENU||item->CtlID==SHOW_TREE || (strip && (item->CtlID==FOCUS || item->CtlID==MASK || item->CtlID==TOPMOST || item->CtlID==SMALLER || item->CtlID==LARGER || item->CtlID==MODE || item->CtlID==PLAY || item->CtlID==OPEN_FILE || item->CtlID==OVERFLOW || item->CtlID==FIND_CLOSE || item->CtlID==FIND_PREV || item->CtlID==FIND_NEXT)))
        DrawActionIcon(item->hDC,item->CtlID,button,disabled?DISABLED:
            (item->CtlID==PIN_WINDOW||item->CtlID==TOPMOST)?(topmost?GREEN:MUTED):
            item->CtlID==WEB_AUDIO?(preferences.browserMuted?AMBER:TEXT):
            item->CtlID==MEETING_RECORD?(meetingRecorder.Running()?RGB(220,80,72):TEXT):
            primary&&!pressed?RGB(255,255,255):TEXT);
    else
    {
        DrawTextAt(item->hDC, TextOf(item->hwndItem), button,
                   disabled  ? DISABLED
                   : primary && !pressed ? RGB(255,255,255)
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
void DrawButton(DRAWITEMSTRUCT *item)
{
    const int width=item->rcItem.right-item->rcItem.left,height=item->rcItem.bottom-item->rcItem.top;
    HDC buffer=CreateCompatibleDC(item->hDC);
    HBITMAP bitmap=width>0&&height>0?CreateCompatibleBitmap(item->hDC,width,height):nullptr;
    if(!buffer||!bitmap){if(bitmap)DeleteObject(bitmap);if(buffer)DeleteDC(buffer);DrawButtonContents(item);return;}
    auto previous=SelectObject(buffer,bitmap);SetViewportOrgEx(buffer,-item->rcItem.left,-item->rcItem.top,nullptr);
    auto copy=*item;copy.hDC=buffer;DrawButtonContents(&copy);
    BitBlt(item->hDC,item->rcItem.left,item->rcItem.top,width,height,buffer,item->rcItem.left,item->rcItem.top,SRCCOPY);
    SelectObject(buffer,previous);DeleteObject(bitmap);DeleteDC(buffer);
}
LRESULT CALLBACK ButtonProc(HWND control, UINT message, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
    if(message==WM_ERASEBKGND)return 1;
    if(message==WM_SETTEXT&&lp&&TextOf(control)==reinterpret_cast<const wchar_t*>(lp))return TRUE;
    if(capturingShortcut>=0&&GetDlgCtrlID(control)==SHORTCUT_FIRST+capturingShortcut) {
        if(message==WM_GETDLGCODE)return DLGC_WANTALLKEYS;
        if(message==WM_KEYDOWN||message==WM_SYSKEYDOWN){if(!(lp&(1L<<30)))CaptureShortcut((UINT)wp,ShortcutModifiers());return 0;}
        if(message==WM_KILLFOCUS){capturingShortcut=-1;UpdateButtons();}
    }
    if(message==WM_SETFOCUS)EnsureSettingsVisible(control);
    if(message==WM_MOUSEWHEEL&&page==Page::Settings)return SendMessageW(window,message,wp,lp);
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
    if(message==WM_SETFOCUS||message==WM_KILLFOCUS)for(const auto& setting:numericSettings)
        if(GetDlgCtrlID(control)==setting.decrement||GetDlgCtrlID(control)==setting.increment)
            InvalidateRect(window,&setting.bounds,FALSE);
    return result;
}
LRESULT CALLBACK SettingsScrollProc(HWND control,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR)
{
    if(message==WM_ERASEBKGND)return 1;
    if(message==WM_MOUSEWHEEL)return SendMessageW(window,message,wp,lp);
    auto parts=[&](){SCROLLBARINFO info{};info.cbSize=sizeof(info);GetScrollBarInfo(control,OBJID_CLIENT,&info);return info;};
    auto hit=[&](int y){
        const auto info=parts();RECT bounds{};GetClientRect(control,&bounds);
        if(y<info.dxyLineButton)return SB_LINEUP;
        if(y>=bounds.bottom-info.dxyLineButton)return SB_LINEDOWN;
        if(y<info.xyThumbTop)return SB_PAGEUP;
        if(y>=info.xyThumbBottom)return SB_PAGEDOWN;
        return SB_THUMBTRACK;
    };
    auto cancel=[&](){settingsThumbDragging=false;settingsRepeatCode=-1;KillTimer(control,1);};
    // The native mouse tracker draws its own themed thumb directly. Own pointer
    // tracking so hover/drag use the same geometry and paint as the idle state.
    if(message==WM_LBUTTONDOWN||message==WM_LBUTTONDBLCLK) {
        SetFocus(control);SetCapture(control);settingsPointerY=GET_Y_LPARAM(lp);
        const auto info=parts();const int action=hit(settingsPointerY);
        settingsThumbDragging=action==SB_THUMBTRACK;
        if(settingsThumbDragging)settingsThumbGrab=settingsPointerY-info.xyThumbTop;
        else {settingsRepeatCode=action;SendMessageW(window,WM_VSCROLL,action,(LPARAM)control);SetTimer(control,1,400,nullptr);}
        InvalidateRect(control,nullptr,FALSE);return 0;
    }
    if(message==WM_MOUSEMOVE&&GetCapture()==control) {
        settingsPointerY=GET_Y_LPARAM(lp);
        if(settingsThumbDragging) {
            const auto info=parts();RECT bounds{};GetClientRect(control,&bounds);
            const int travel=bounds.bottom-2*info.dxyLineButton-(info.xyThumbBottom-info.xyThumbTop);
            if(travel>0)ScrollSettings(MulDiv(std::clamp(settingsPointerY-settingsThumbGrab-info.dxyLineButton,0,travel),settingsMaximum,travel));
        }
        InvalidateRect(control,nullptr,FALSE);return 0;
    }
    if(message==WM_TIMER&&wp==1) {
        if(GetCapture()!=control||settingsRepeatCode<0){KillTimer(control,1);return 0;}
        if(hit(settingsPointerY)==settingsRepeatCode)
            SendMessageW(window,WM_VSCROLL,settingsRepeatCode,(LPARAM)control);
        SetTimer(control,1,60,nullptr);return 0;
    }
    if(message==WM_LBUTTONUP||message==WM_CANCELMODE||message==WM_CAPTURECHANGED||message==WM_SIZE) {
        cancel();if(message!=WM_CAPTURECHANGED&&GetCapture()==control)ReleaseCapture();
        InvalidateRect(control,nullptr,FALSE);
        if(message!=WM_SIZE)return 0;
    }
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE){cancel();if(GetCapture()==control)ReleaseCapture();InvalidateRect(control,nullptr,FALSE);return 0;}
    if(message==WM_PAINT) {
        PAINTSTRUCT paint{};HDC dc=BeginPaint(control,&paint);RECT bounds{};GetClientRect(control,&bounds);FillRect(dc,&bounds,bgBrush);
        SCROLLBARINFO info{};info.cbSize=sizeof(info);
        if(GetScrollBarInfo(control,OBJID_CLIENT,&info)) {
            RECT thumb{S(4),info.xyThumbTop,bounds.right-S(4),info.xyThumbBottom};
            if(thumb.bottom>thumb.top)FillRoundRect(dc,thumb,GetCapture()==control?MUTED:HOVER,S(6));
            RECT up{0,0,bounds.right,info.dxyLineButton},down{0,bounds.bottom-info.dxyLineButton,bounds.right,bounds.bottom};
            DrawCodicon(dc,0xeab7,up,MUTED,S(10));DrawCodicon(dc,0xeab4,down,MUTED,S(10));
        }
        EndPaint(control,&paint);return 0;
    }
    LRESULT result=DefSubclassProc(control,message,wp,lp);
    if(message==WM_SETFOCUS||message==WM_KILLFOCUS||message==WM_KEYDOWN)
        RedrawWindow(control,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
    return result;
}
LRESULT CALLBACK MonitorComboProc(HWND control,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR)
{
    if(message==WM_SETFOCUS)EnsureSettingsVisible(control);
    if(message==WM_MOUSEWHEEL&&page==Page::Settings&&!SendMessageW(control,CB_GETDROPPEDSTATE,0,0))return SendMessageW(window,message,wp,lp);
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
    if((message==WM_KEYDOWN||message==WM_SYSKEYDOWN)&&!(lp&(1L<<30))&&DispatchPreferenceShortcut((UINT)wp,ShortcutModifiers()))return 0;
    if(message==WM_KEYDOWN&&wp==L'S'&&(GetKeyState(VK_CONTROL)&0x8000)){SaveFile();return 0;}
    if(Blocked() && (message==WM_COPY || message==WM_CUT || message==WM_PASTE))return 0;
    if (message == WM_PASTE && (Blocked() || reading || (viewingFile&&!preferences.allowFileEditing))) return 0;
    if (message == WM_PASTE && !reading && (!viewingFile||preferences.allowFileEditing) && !Blocked())
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
    if (message == WM_KEYDOWN && wp == VK_ESCAPE && focusMode) { ToggleFocus(); return 0; }
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
        const auto emptyReadingHint=std::wstring(T(L"Về Lời nhắc để nhập nội dung",L"Return to Script to add content"))+L" · "+ShortcutLabel(preferences.Key(PreferenceCommand::Reading));
        DrawCenteredMessage(dc, bounds,
                            reading ? T(L"Chưa có lời nhắc", L"No script yet")
                                    : T(L"Dán hoặc nhập lời nhắc", L"Paste or type your script"),
                            reading ? emptyReadingHint.c_str()
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
    // Documentation omits layered overlays; exclusion checks retain CAPTUREBLT.
    qaCaptureOk = CaptureRegionBmp(qaDir + L"\\" + name, rect, !qaVisual) && qaCaptureOk;
}
bool CompactFits()
{
    RECT client{};
    GetClientRect(window, &client);
    if(page==Page::Home) {
        HDC dc=GetDC(window);auto old=SelectObject(dc,homeCompact?brandFont:titleFont);
        SIZE label{};const auto text=T(L"Trang chủ",L"Home");
        GetTextExtentPoint32W(dc,text,lstrlenW(text),&label);SelectObject(dc,old);ReleaseDC(window,dc);
        if(label.cx>S(contentRight-contentLeft-(homeCompact?64:0)))return false;
    }
    std::vector<RECT> controls;
    std::vector<HWND> children=buttons;
    if(page==Page::Home)children.push_back(homeVersion);
    if(page==Page::Browser){children.push_back(webAddress);if(browser.Enabled()&&tabsVisible)children.push_back(browserTabBar.Window());}
    if(page==Page::Settings)for(HWND control:{sizeInput,opacityInput,iconSizeInput,sourceMonitor,destinationMonitor,settingsScroll})children.push_back(control);
    for (HWND child : children)
    {
        if (!IsWindowVisible(child))
            continue;
        RECT rect{};
        GetWindowRect(child, &rect);
        MapWindowPoints(nullptr, window, (POINT *)&rect, 2);
        if(page==Page::Settings&&std::find(clippedSettingsControls.begin(),clippedSettingsControls.end(),child)!=clippedSettingsControls.end()) {
            RECT clipped{};if(!IntersectRect(&clipped,&rect,&settingsViewport))continue;rect=clipped;
        }
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
    if (page != Page::Content && page!=Page::Browser) return true;
    RECT rect{};
    if(page==Page::Browser){rect=contentCardRect;InflateRect(&rect,-S(2),-S(2));}
    else {GetWindowRect(IsImage() ? imageViewer : editor, &rect);MapWindowPoints(nullptr, window, (POINT *)&rect, 2);}
    for (const RECT &control : controls)
    {
        RECT overlap{};
        if (IntersectRect(&overlap, &rect, &control) && !focusMode)
            return false;
    }
    return rect.left >= 0 && rect.right <= client.right && rect.top >= 0 &&
           rect.bottom <= client.bottom - S(focusMode||!statusVisible ? 4 : 20) && rect.bottom - rect.top >= S(80);
}
void WriteQa()
{
    DWORD actual = 0;
    GetWindowDisplayAffinity(window, &actual);
    auto boolean = [](bool value) -> std::wstring { return value ? L"true" : L"false"; };
    std::wstring report =
        L"{\n  \"applicationVersion\": \"" QUIETSHARE_VERSION_WIDE L"\",\n  \"windowsBuild\": " + std::to_wstring(WindowsBuild()) +
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
bool StorePreferences(const Preferences& candidate);
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
        LOGFONTW nativeFont{};GetObjectW(uiFont,sizeof(nativeFont),&nativeFont);
        qaFont = fontPoints == DefaultTextPoints+4 && format.yHeight == (DefaultTextPoints+4)*20 &&
                 nativeFont.lfQuality==CLEARTYPE_QUALITY && wcscmp(format.szFaceName,L"Segoe UI")==0;
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
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(HOME_SCRIPT, BN_CLICKED), 0);
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
                        // This synthetic matrix runs inside one QA timer tick.
                        // Drain paint/sent health checks between sizes so DWM
                        // never substitutes a stale ghost-window screenshot.
                        MSG paintMessage{};
                        while(PeekMessageW(&paintMessage,nullptr,WM_PAINT,WM_PAINT,PM_REMOVE))DispatchMessageW(&paintMessage);
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
                        for (Page surface : {Page::Home, Page::Settings, Page::Presentation,Page::Browser})
                        {
                            page = surface; Layout(); qaLayoutMatrix = CompactFits() && qaLayoutMatrix;
                            if(surface==Page::Settings) {
                                for(int category=0;category<6;++category){settingsCategory=category;settingsOffset=0;Layout();
                                    qaLayoutMatrix=CompactFits()&&qaLayoutMatrix;ScrollSettings(settingsMaximum);qaLayoutMatrix=CompactFits()&&qaLayoutMatrix;ScrollSettings(0);}
                                settingsCategory=0;
                            }
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
        SetWindowPos(window,nullptr,0,0,S(430),S(320),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();Capture(L"home-minimum.bmp");
        const int savedIcon=preferences.iconSize;preferences.iconSize=24;SetInterfaceIconSize(24);Layout();Capture(L"home-minimum-icons.bmp");
        preferences.iconSize=savedIcon;SetInterfaceIconSize(savedIcon);
        SetWindowPos(window,nullptr,0,0,S(800),S(680),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();Capture(L"home-reference.bmp");
        SetWindowPos(window,nullptr,0,0,S(840),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();
        SendMessageW(window, WM_COMMAND, MAKEWPARAM(HOME_SETTINGS,BN_CLICKED),0);
        settingsCategory=0;settingsOffset=0;Layout();Capture(L"settings-appearance.bmp");
        SendMessageW(window,WM_COMMAND,SETTINGS_BROWSER,0);Capture(L"settings-browser.bmp");
        SendMessageW(window,WM_COMMAND,SETTINGS_READING,0);
        qaSettings = page == Page::Settings && IsWindowVisible(sizeInput) && !GetDlgItem(window,130);
        int beforeSize = ActiveTextSize();
        SetFocus(sizeInput);SetWindowTextW(sizeInput,L"32");CommitSizeInput();
        qaSettings = qaSettings && ActiveTextSize()==32;
        ActiveTextSize() = beforeSize; ApplyTextFormat();SyncSizeInput(true);
        Capture(L"settings.bmp");
        english=true;Layout();Capture(L"settings-en.bmp");english=false;Layout();
        SendMessageW(window,WM_COMMAND,SETTINGS_WORKSPACE,0);Capture(L"settings-workspace.bmp");
        SendMessageW(window,WM_COMMAND,SETTINGS_PROJECTION,0);Capture(L"settings-display.bmp");
        SendMessageW(window,WM_COMMAND,SETTINGS_KEYS,0);Capture(L"settings-shortcuts.bmp");
        SendMessageW(window,WM_COMMAND,SETTINGS_APPEARANCE,0);
        RECT settingsSize{};GetWindowRect(window,&settingsSize);
        SetWindowPos(window,nullptr,0,0,S(430),S(320),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();Capture(L"settings-minimum.bmp");
        SetWindowPos(window,nullptr,0,0,settingsSize.right-settingsSize.left,settingsSize.bottom-settingsSize.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        page=Page::Browser;Layout();Capture(L"browser-off.bmp");
        english=true;Layout();Capture(L"browser-off-en.bmp");english=false;
        RECT browserSize{};GetWindowRect(window,&browserSize);
        SetWindowPos(window,nullptr,0,0,S(430),S(320),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();Capture(L"browser-minimum.bmp");
        qaV04=CompactFits()&&!browser.Enabled()&&qaV04;
        SetWindowPos(window,nullptr,0,0,browserSize.right-browserSize.left,browserSize.bottom-browserSize.top,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        ShowScript(); SetReading(true); ToggleFocus();
        RECT focusButton{};GetWindowRect(GetDlgItem(window,FOCUS),&focusButton);
        qaFocus = focusMode && !(GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION) && !IsWindowVisible(GetDlgItem(window,NAV_SCRIPT)) &&
                  !IsWindowVisible(documentBar.Window()) && !IsWindowVisible(GetDlgItem(window,SMALLER)) &&
                  focusButton.right-focusButton.left==S(24) && CompactFits();
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
                         !(GetWindowLongPtrW(editor,GWL_STYLE)&ES_READONLY);
            CHARFORMAT2W format{}; format.cbSize=sizeof(format);
            SendMessageW(editor,EM_GETCHARFORMAT,SCF_DEFAULT,(LPARAM)&format);
            qaCodeFile = qaCodeFile && std::wstring(format.szFaceName)==L"Consolas";
            Capture(L"code-file.bmp");
            qaImageFile = OpenContentPath(qaDir + L"\\..\\qa-content\\orientation.bmp") && IsImage() &&
                          IsWindowVisible(imageViewer) && !IsWindowVisible(editor);
            AdjustFont(2);
            SendMessageW(imageViewer,WM_KEYDOWN,VK_LEFT,0);
            qaImagePan = imageZoom == DefaultZoomPercent+10 && panX == S(24);
            ResetFont();
            qaImagePan = qaImagePan && imageZoom == DefaultZoomPercent && panX == 0 && panY == 0;
            ToggleMask();
            qaImageMask = Blocked() && !IsWindowVisible(imageViewer) && !IsWindowVisible(editor) &&
                          !OpenContentPath(codePath) && IsImage();
            ToggleMask();
            qaImageMask = qaImageMask && IsWindowVisible(imageViewer);
            Capture(L"image-file.bmp");
            SaveActiveState();page=Page::Home;Layout();Capture(L"home-recent.bmp");
            page=Page::Content;Layout();
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
                    Layout();RECT exit{};GetClientRect(GetDlgItem(window,FOCUS),&exit);
                    qaV04=!IsWindowVisible(documentBar.Window())&&exit.bottom==S(24)&&CompactFits()&&qaV04;
                }
            }
            dpi=actualDpi;english=false;RefreshFonts();ToggleFocus();
            SetWindowPos(window,nullptr,0,0,S(1080),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            const auto project=qaDir+L"\\demo-project";
            CreateDirectoryW(project.c_str(),nullptr);CreateDirectoryW((project+L"\\src").c_str(),nullptr);
            CreateDirectoryW((project+L"\\src\\utils").c_str(),nullptr);
            const std::string source="# QuietShare sample\ndef explain(value):\n    return value + 1\n";
            WriteBytes(project+L"\\src\\main.py",source.data(),(DWORD)source.size());
            WriteBytes(project+L"\\src\\utils\\helpers.py",source.data(),(DWORD)source.size());
            const std::string readme="# Demo project\nLocal read-only preview.\n";
            WriteBytes(project+L"\\README.md",readme.data(),(DWORD)readme.size());
            qaV04=projectTree.OpenFolder(project,importError)&&qaV04;
            treeVisible=true;TreeView_Expand(projectTree.Handle(),TreeView_GetRoot(projectTree.Handle()),TVE_EXPAND);Layout();
            auto sourceItem=TreeView_GetChild(projectTree.Handle(),TreeView_GetRoot(projectTree.Handle()));
            TreeView_Expand(projectTree.Handle(),sourceItem,TVE_EXPAND);
            TreeView_Expand(projectTree.Handle(),TreeView_GetChild(projectTree.Handle(),sourceItem),TVE_EXPAND);
            qaV04=OpenContentPath(project+L"\\src\\main.py")&&qaV04;
            Capture(L"project-wide.bmp");
            SetWindowPos(window,nullptr,0,0,S(840),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();
            qaV04=OpenContentPath(project+L"\\src\\main.py")&&treeVisible&&IsWindowVisible(projectTree.Handle())&&IsWindowVisible(codeViewer.Window())&&qaV04;
            Capture(L"project-medium.bmp");
            activityVisible=tabsVisible=toolbarVisible=statusVisible=false;Layout();
            qaV04=IsWindowVisible(codeViewer.Window())&&IsWindowVisible(projectTree.Handle())&&CompactFits()&&qaV04;
            Capture(L"layout-minimal.bmp");
            activityVisible=tabsVisible=toolbarVisible=statusVisible=true;Layout();
            SetWindowPos(window,nullptr,0,0,S(430),S(320),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();
            qaV04=treeVisible&&IsWindowVisible(projectTree.Handle())&&IsWindowVisible(codeViewer.Window())&&qaV04;Capture(L"project-minimum.bmp");
            for(UINT scale:{96U,120U,144U,192U})for(bool language:{false,true}) {
                dpi=scale;english=language;RefreshFonts();
                for(int width:{430,640,1080}) {
                    SetWindowPos(window,nullptr,0,0,S(width),S(width<500?320:560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);Layout();
                    qaV04=IsWindowVisible(projectTree.Handle())&&IsWindowVisible(codeViewer.Window())&&CompactFits()&&qaV04;
                }
            }
            dpi=actualDpi;english=false;RefreshFonts();
            treeVisible=false;SetWindowPos(window,nullptr,0,0,S(840),S(560),SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
            qaV04=OpenContentPath(qaDir+L"\\..\\qa-viewers\\unicode.pdf")&&IsPdf()&&qaV04;
            Capture(L"pdf-file.bmp");ToggleFocus();Capture(L"pdf-focus.bmp");ToggleFocus();
            page=Page::Presentation;Layout();Capture(L"presentation.bmp");
            english=true;Layout();Capture(L"presentation-en.bmp");
            english=false;page=Page::Content;Layout();ToggleMask();Capture(L"tabs-masked.bmp");ToggleMask();
            ShowScript();
        }
        if(qaVisual) {
            for(int id=0;id<static_cast<int>(Themes.size());++id) {
                auto candidate=preferences;candidate.themeId=id;StorePreferences(candidate);
                page=Page::Home;Layout();Capture((L"theme-"+std::to_wstring(id)+L"-home.bmp").c_str());
                page=Page::Content;Layout();Capture((L"theme-"+std::to_wstring(id)+L"-reader.bmp").c_str());
                page=Page::Settings;settingsCategory=0;settingsOffset=0;Layout();
                Capture((L"theme-"+std::to_wstring(id)+L"-settings.bmp").c_str());
            }
            auto candidate=preferences;candidate.themeId=0;StorePreferences(candidate);page=Page::Content;Layout();
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
UINT ShortcutModifiers()
{
    return ((GetKeyState(VK_CONTROL)&0x8000)?MOD_CONTROL:0)|((GetKeyState(VK_SHIFT)&0x8000)?MOD_SHIFT:0)|
           ((GetKeyState(VK_MENU)&0x8000)?MOD_ALT:0)|(((GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN))&0x8000)?MOD_WIN:0);
}
std::wstring PreferenceErrorText(const std::wstring& error)
{
    if(error==L"duplicate")return T(L"Tổ hợp này đã được gán cho tác vụ khác.",L"This shortcut is already assigned to another action.");
    if(error==L"mask-conflict"||error==L"recall-conflict")return T(L"Phím đang được ứng dụng khác sử dụng. Đã giữ phím cũ.",L"Another app uses this shortcut. The previous binding is retained.");
    if(error==L"reserved")return T(L"Tổ hợp dành cho Windows, trình duyệt hoặc chỉnh sửa văn bản.",L"This combination is reserved for Windows, browsing or text editing.");
    return T(L"Dùng phím F1-F24 hoặc tổ hợp Ctrl / Alt / Shift hợp lệ.",L"Use F1-F24 or a valid Ctrl / Alt / Shift combination.");
}
void ApplyWindowAppearance()
{
    SetInterfaceIconSize(preferences.iconSize);
    projectTree.SetDpi(dpi);
    const auto style=GetWindowLongPtrW(window,GWL_EXSTYLE);
    BYTE previousAlpha=255;DWORD alphaFlags=0;
    if(!GetLayeredWindowAttributes(window,nullptr,&previousAlpha,&alphaFlags)||!(alphaFlags&LWA_ALPHA))previousAlpha=255;
    const BYTE requestedAlpha=(BYTE)MulDiv(255,preferences.opacityPercent,100);
    const bool opacityChanged=previousAlpha!=requestedAlpha||(!(style&WS_EX_LAYERED)&&requestedAlpha<255);
    auto lockAppearance=[&] {
        browser.Lock();documentHost.Hide();ShowWindow(editor,SW_HIDE);ShowWindow(imageViewer,SW_HIDE);SendMessageW(tooltips,TTM_POP,0,0);
        if(mirror)mirror->Invalidate();
    };
    if(requestedAlpha<255||(style&WS_EX_LAYERED)) {
        if(!(style&WS_EX_LAYERED))SetWindowLongPtrW(window,GWL_EXSTYLE,style|WS_EX_LAYERED);
        // Removing WS_EX_LAYERED can transiently revoke capture exclusion and
        // invalidate a healthy session. Alpha 255 restores opacity without
        // replacing the protected compositing surface.
        if(!SetLayeredWindowAttributes(window,0,requestedAlpha,LWA_ALPHA)) {
            if(SetLayeredWindowAttributes(window,0,255,LWA_ALPHA)) {
                preferences.opacityPercent=100;
                preferenceError=T(L"Windows chưa áp dụng được độ mờ. Đã giữ cửa sổ rõ hoàn toàn.",L"Windows could not apply opacity. The window remains fully opaque.");
            } else {
                preferences.opacityPercent=std::clamp(MulDiv(previousAlpha,100,255),40,100);
                preferenceError=T(L"Không áp dụng được độ mờ. Nội dung riêng đã khóa; hãy kiểm tra lại WGC.",L"Opacity could not be applied. Private content is locked; verify WGC again.");
                lockAppearance();
            }
        }
    }
    DWORD actualAffinity=0;
    if(!qa&&opacityChanged&&(!GetWindowDisplayAffinity(window,&actualAffinity)||actualAffinity!=0x11)) {
        privacy.applied=false;lockAppearance();
    } else if(opacityChanged&&mirror&&mirror->Running()&&!mirror->HandleDisplayChange()) {
        browser.Lock();documentHost.Hide();ShowWindow(editor,SW_HIDE);ShowWindow(imageViewer,SW_HIDE);SendMessageW(tooltips,TTM_POP,0,0);
    }
    ApplyTextFormat();RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_ALLCHILDREN);
}
bool StorePreferences(const Preferences& candidate)
{
    std::wstring error;
    if(!ValidatePreferences(candidate,error)||(!qa&&!globalHotkeys.Apply(candidate,error,[&]{return SavePreferences(dataDir,candidate);}))) {
        preferenceError=error==L"storage"?T(L"Không lưu được thiết lập. Cấu hình trước được giữ lại.",L"Could not save preferences. The previous settings are retained."):PreferenceErrorText(error);return false;
    }
    preferences=candidate;f8Registered=globalHotkeys.Registered(PreferenceCommand::Mask);recallRegistered=globalHotkeys.Registered(PreferenceCommand::Recall);
    preferenceError.clear();browser.SetMediaPreferences(preferences.browserMuted,preferences.browserAllowCamera,preferences.browserAllowMicrophone);
    const ThemePalette nextTheme=ThemePaletteFor(preferences.themeId);
    if(theme.vi!=nextTheme.vi || theme.bg!=nextTheme.bg || theme.card!=nextTheme.card || theme.accent!=nextTheme.accent || theme.text!=nextTheme.text) {
        theme=nextTheme;
        DeleteObject(bgBrush);DeleteObject(cardBrush);DeleteObject(numberBrush);
        bgBrush=CreateSolidBrush(BG);cardBrush=CreateSolidBrush(CARD);numberBrush=CreateSolidBrush(ELEVATED);
        SendMessageW(editor,EM_SETBKGNDCOLOR,0,BG);
        BOOL dark=theme.dark;DwmSetWindowAttribute(window,20,&dark,sizeof(dark));
        COLORREF border=DIVIDER;DwmSetWindowAttribute(window,34,&border,sizeof(border));
        EnumChildWindows(window,[](HWND child,LPARAM)->BOOL {
            SetWindowTheme(child,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);return TRUE;
        },0);
        codeViewer.RefreshTheme();projectTree.RefreshTheme();
    }
    ApplyWindowAppearance();return true;
}
void CommitOpacity()
{
    if(updatingOpacity||!opacityInput)return;
    const auto text=TextOf(opacityInput);wchar_t* end=nullptr;long value=wcstol(text.c_str(),&end,10);
    if(text.empty()||!end||*end||value<40||value>100)preferenceError=T(L"Độ mờ cần nằm trong khoảng 40-100%.",L"Opacity must be between 40 and 100%.");
    else {auto candidate=preferences;candidate.opacityPercent=(int)value;StorePreferences(candidate);}
    updatingOpacity=true;SetWindowTextW(opacityInput,std::to_wstring(preferences.opacityPercent).c_str());updatingOpacity=false;
    PostMessageW(window,WM_APP+31,0,0);
}
void CommitIconSize()
{
    if(updatingIconSize||!iconSizeInput)return;
    const auto text=TextOf(iconSizeInput);wchar_t* end=nullptr;long value=wcstol(text.c_str(),&end,10);
    if(text.empty()||!end||*end||value<12||value>24)preferenceError=T(L"Kích thước icon cần nằm trong khoảng 12-24 px.",L"Icon size must be between 12 and 24 px.");
    else {auto candidate=preferences;candidate.iconSize=(int)value;StorePreferences(candidate);}
    updatingIconSize=true;SetWindowTextW(iconSizeInput,std::to_wstring(preferences.iconSize).c_str());updatingIconSize=false;
    PostMessageW(window,WM_APP+31,0,0);
}
void CaptureShortcut(UINT key,UINT modifiers)
{
    if(capturingShortcut<0||key==VK_CONTROL||key==VK_SHIFT||key==VK_MENU||key==VK_LWIN||key==VK_RWIN)return;
    if(key==VK_ESCAPE){capturingShortcut=-1;preferenceError.clear();Layout();return;}
    auto candidate=preferences;candidate.shortcuts[(size_t)capturingShortcut]={(WORD)key,(WORD)modifiers};
    capturingShortcut=-1;StorePreferences(candidate);Layout();
}
void RunPreferenceCommand(PreferenceCommand command)
{
    if(command==PreferenceCommand::Mask){ToggleMask();return;}
    if(command==PreferenceCommand::Recall){RecallWindow();return;}
    const int commands[]={0,0,VERIFY_PRIVACY,MIRROR_ACTION,FOCUS,OPEN_FILE,OPEN_FOLDER,SHOW_TREE,MODE,PLAY};
    const auto index=static_cast<unsigned>(command);
    if(index<PreferenceCommandCount)SendMessageW(window,WM_COMMAND,commands[index],0);
}
bool DispatchPreferenceShortcut(UINT key,UINT modifiers)
{
    if(capturingShortcut>=0){CaptureShortcut(key,modifiers);return true;}
    for(unsigned i=0;i<PreferenceCommandCount;++i)if(MatchShortcut(preferences.shortcuts[i],key,modifiers)) {
        const auto command=static_cast<PreferenceCommand>(i);
        if(i<2&&globalHotkeys.Registered(command))return true;
        if(command==PreferenceCommand::AutoScroll&&(page!=Page::Content||!reading||viewingFile||GetFocus()!=editor))return false;
        RunPreferenceCommand(command);return true;
    }
    return false;
}
LRESULT CALLBACK AddressInputProc(HWND control,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR)
{
    if(message==WM_CHAR&&(wp==L'\r'||wp==L'\n'||wp==L'\t'))return 0;
    if(message==WM_KEYDOWN&&wp==VK_RETURN){PostMessageW(window,WM_COMMAND,WEB_GO,0);return 0;}
    if(message==WM_PASTE&&OpenClipboard(control)) {
        HANDLE clip=GetClipboardData(CF_UNICODETEXT);const auto text=clip?static_cast<const wchar_t*>(GlobalLock(clip)):nullptr;
        std::wstring value=text?text:L"";if(text)GlobalUnlock(clip);CloseClipboard();
        if(!value.empty()){
            for(auto& c:value)if(c==L'\r'||c==L'\n'||c==L'\t')c=L' ';
            SendMessageW(control,EM_REPLACESEL,TRUE,(LPARAM)value.c_str());
        }
        return 0;
    }
    if(message==WM_LBUTTONDOWN&&GetFocus()!=control) {
        SetFocus(control);SendMessageW(control,EM_SETSEL,0,-1);SetPropW(control,L"QuietShareSelectAll",(HANDLE)1);return 0;
    }
    if(message==WM_LBUTTONUP&&RemovePropW(control,L"QuietShareSelectAll"))return 0;
    const auto result=DefSubclassProc(control,message,wp,lp);
    if(message==WM_SIZE||message==WM_SETFONT)CenterInputText(control);
    if(message==WM_SETFOCUS)SendMessageW(control,EM_SETSEL,0,-1);
    if(message==WM_SETFOCUS||message==WM_KILLFOCUS)InvalidateRect(window,nullptr,FALSE);
    if(message==WM_PAINT&&GetWindowTextLengthW(control)==0&&GetFocus()!=control) {
        HDC dc=GetDC(control);RECT bounds{};GetClientRect(control,&bounds);bounds.left+=S(6);bounds.right-=S(6);
        DrawTextAt(dc,T(L"Tìm kiếm hoặc nhập URL",L"Search or enter URL"),bounds,MUTED,uiFont,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        ReleaseDC(control,dc);
    }
    if(message==WM_NCDESTROY)RemovePropW(control,L"QuietShareSelectAll");
    return result;
}
LRESULT CALLBACK OpacityInputProc(HWND control,UINT message,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR)
{
    const bool icon=control==iconSizeInput;
    if(message==WM_LBUTTONDOWN&&GetFocus()!=control) {
        auto result=DefSubclassProc(control,message,wp,lp);const int end=GetWindowTextLengthW(control);SendMessageW(control,EM_SETSEL,end,end);return result;
    }
    if(message==WM_CHAR&&(wp==L'\r'||wp==L'\n'))return 0;
    if(message==WM_GETDLGCODE&&lp) {
        const auto key=reinterpret_cast<MSG*>(lp)->wParam;
        if(key==VK_RETURN||key==VK_ESCAPE)return DefSubclassProc(control,message,wp,lp)|DLGC_WANTMESSAGE;
    }
    if(message==WM_KEYDOWN&&wp==VK_RETURN){if(icon)CommitIconSize();else CommitOpacity();return 0;}
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE){preferenceError.clear();auto& updating=icon?updatingIconSize:updatingOpacity;
        updating=true;SetWindowTextW(control,std::to_wstring(icon?preferences.iconSize:preferences.opacityPercent).c_str());updating=false;SendMessageW(control,EM_SETSEL,-1,-1);return 0;}
    if(message==WM_MOUSEWHEEL)return SendMessageW(window,message,wp,lp);
    const auto result=DefSubclassProc(control,message,wp,lp);
    if(message==WM_SETFOCUS){EnsureSettingsVisible(control);const int end=GetWindowTextLengthW(control);SendMessageW(control,EM_SETSEL,end,end);}
    if(message==WM_SIZE||message==WM_SETFONT)CenterInputText(control);
    if(message==WM_SETFOCUS||message==WM_KILLFOCUS)InvalidateRect(window,nullptr,FALSE);
    return result;
}
LRESULT CALLBACK Proc(HWND h, UINT message, WPARAM wp, LPARAM lp)
{
    switch (message)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if(!(lp&(1L<<30))&&DispatchPreferenceShortcut((UINT)wp,ShortcutModifiers()))return 1;
        if(wp==L'S'&&(GetKeyState(VK_CONTROL)&0x8000)){SaveFile();return 0;}
        break;
    case WM_DWMSENDICONICTHUMBNAIL:PublicThumbnail(h,HIWORD(lp),LOWORD(lp),false);return 0;
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:PublicThumbnail(h,320,180,true);return 0;
    case WM_CREATE: {
        if(!LoadCodicons(instance))return -1;
        window = h;
        mirror=std::make_unique<MirrorController>();mirror->SetResourceMode(resourceMode);
        if(!qa)mirror->SetDiagnosticPath(dataDir+L"\\projection-stop.json");
        privacy = ApplyPrivacy(h);
        topologyKnown = TryGetDisplayCloned(cloned);
        if (qaVisual)
            SetWindowDisplayAffinity(h, 0);
        std::vector<std::pair<int, const wchar_t *>> labels = {
            {NAV_HOME, L"Chuẩn bị"}, {NAV_SCRIPT, L"Lời nhắc"}, {NAV_READ, L"Đọc"}, {NAV_SETTINGS, L"Tùy chỉnh"},
            {MODE, L"Đọc"},   {PLAY, L"Chạy"},
            {SPEED, L"Vừa"},           {MASK, L"Che · F8"}, {SMALLER, L"A-"}, {LARGER, L"A+"},
            {RESET_ZOOM, L"100%"}, {TOPMOST, L"Ghim"}, {PIN_WINDOW,L"Ghim cửa sổ trên cùng"}, {LANGUAGE, L"EN"}, {FOCUS, L"Tập trung"},
            {OPEN_FILE, L"Mở file"}, {OPEN_DROP, L"Mở hoặc thả file"}, {HOME_SCRIPT, L"Lời nhắc"},
            {HOME_SETTINGS, L"Cài đặt"}, {VIEW_FILE, L"Xem file"},
            {OPEN_FOLDER,L"Mở dự án"},{OVERFLOW,L"Thêm"},{FIND,L"Tìm"},{FIND_PREV,L"Trước"},
            {FIND_NEXT,L"Sau"},{FIND_CLOSE,L"Đóng"},{PDF_PREV,L"Trang trước"},{PDF_NEXT,L"Trang sau"},
            {NAV_PRESENTATION,L"Trình chiếu"},{VERIFY_PRIVACY,L"Kiểm tra WGC"},{MIRROR_START,L"Bắt đầu"},
            {MIRROR_STOP,L"Dừng"},{POINTER,L"Con trỏ"},{RELOAD_FILE,L"Tải lại"},{KEEP_FILE,L"Giữ bản đang xem"},
            {INCLUDE_IGNORED,L"Hiện thư mục đang bỏ qua"},{NAV_BROWSER,L"Trình duyệt"},{BROWSER_TOGGLE,L"Bật trình duyệt"},
            {WEB_BACK,L"Quay lại"},{WEB_FORWARD,L"Tiến tới"},{WEB_RELOAD,L"Tải lại"},{WEB_AUDIO,L"Âm thanh web"},{WEB_GO,L"Mở"},{WEB_MENU,L"Menu trình duyệt"},
            {WEB_ZOOM_OUT,L"−"},{WEB_ZOOM_RESET,L"100%"},{WEB_ZOOM_IN,L"+"},{WEB_NEW_TAB,L"Tab web mới"},{RESOURCE_MODE,L"Hiệu năng · 60 FPS"},
            {LAYOUT_MENU,L"Bố cục"},{SHOW_TREE,L"Ẩn Explorer"},{SHOW_ACTIVITY,L"Thanh điều hướng"},{SHOW_TABS,L"Thanh tab"},
            {SHOW_TOOLBAR,L"Thanh công cụ"},{SHOW_STATUS,L"Thanh trạng thái"},{RESET_LAYOUT,L"Khôi phục bố cục"},
            {WRAP,L"Xuống dòng code"},{OPEN_DISPLAY_SETTINGS,L"Thiết lập màn hình"},
            {SETTINGS_APPEARANCE,L"Giao diện"},{SETTINGS_READING,L"Nội dung"},{SETTINGS_WORKSPACE,L"Bố cục"},
            {SETTINGS_PROJECTION,L"Trình chiếu"},{SETTINGS_KEYS,L"Phím tắt"},{TEXT_WEIGHT,L"Thường"},
            {SETTINGS_BROWSER,L"Trình duyệt"},{OPACITY_LESS,L"Giảm độ mờ"},{OPACITY_MORE,L"Tăng độ mờ"},{MEETING_RECORD,L"Ghi âm cuộc họp"},
            {ICON_SMALLER,L"Giảm kích thước icon"},{ICON_LARGER,L"Tăng kích thước icon"},
            {BROWSER_MUTED,L"Tắt âm thanh"},{BROWSER_CAMERA,L"Cho phép camera"},{BROWSER_MICROPHONE,L"Cho phép micro"},
            {RESET_PREFERENCES,L"Khôi phục mặc định"},{MIRROR_ACTION,L"Bắt đầu trình chiếu"},
            {220,L"F8"},{221,L"Ctrl+Alt+Q"},{222,L"F9"},{223,L"F10"},{224,L"F11"},
            {225,L"Ctrl+O"},{226,L"Ctrl+Shift+O"},{227,L"Ctrl+B"},{228,L"Ctrl+Enter"},{229,L"Space"}};
        labels.insert(labels.end(),{{THEME_SELECT,L"Chủ đề màu"},{REMEMBER_SIZE,L"Nhớ kích thước cửa sổ"},{ALLOW_EDIT,L"Cho phép sửa file"},{SAVE_FILE,L"Lưu file"},
            {RECENT_CLEAR,L"Xóa lịch sử"},{RECENT_NEXT,L"Sau"},{RECENT_PREV,L"Trước"},{TRANSCRIPTION_LANGUAGE,L"Ngôn ngữ chép lời"},
            {SCRIPT_TAB_LABELS,L"Nhãn tab Lời nhắc"},{BROWSER_TAB_LABELS,L"Nhãn tab browser"},
            {MEETING_TRANSCRIBE,L"Tự chép lời sau khi ghi âm"},{MEETING_PROVIDER,L"Nền tảng cuộc họp"},
            {MEETING_OUTPUT,L"Kiểu xuất cuộc họp"},{MEETING_DIRECTORY,L"Thư mục lưu cuộc họp"},
            {MEETING_IMPORT,L"Nhập transcript nền tảng"},{CUSTOM_BACKGROUND,L"Màu nền theme"},
            {CUSTOM_PANEL,L"Màu bảng theme"},{CUSTOM_TEXT,L"Màu chữ theme"},{CUSTOM_ACCENT,L"Màu nhấn theme"},
            {CUSTOM_RESET,L"Khôi phục màu theme"}});
        for(int i=0;i<6;++i){labels.push_back({RECENT_FIRST+i*3,L"Mở lại"});labels.push_back({RECENT_FIRST+i*3+1,L"Ghim / bỏ ghim"});labels.push_back({RECENT_FIRST+i*3+2,L"Xóa khỏi lịch sử"});}
        for (const auto &label : labels)
        {
            buttons.push_back(CreateWindowW(L"BUTTON", label.second,
                                            WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS | BS_OWNERDRAW, 0, 0, 0, 0, h,
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
        SetWindowTheme(editor, theme.dark?L"DarkMode_Explorer":L"Explorer", nullptr);
        imageViewer = CreateWindowExW(0, L"QuietShareImage", L"", WS_CHILD | WS_TABSTOP | WS_CLIPSIBLINGS,
                                       0, 0, 0, 0, h, nullptr, instance, nullptr);
        sizeInput=CreateWindowExW(0,L"EDIT",L"24",WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS|ES_CENTER|ES_NUMBER|ES_AUTOHSCROLL|ES_MULTILINE,
            0,0,0,0,h,(HMENU)SIZE_INPUT,instance,nullptr);
        SendMessageW(sizeInput,EM_SETLIMITTEXT,4,0);SetWindowTheme(sizeInput,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
        SetWindowSubclass(sizeInput,SizeInputProc,1,0);
        opacityInput=CreateWindowExW(0,L"EDIT",L"100",WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS|ES_CENTER|ES_NUMBER|ES_AUTOHSCROLL|ES_MULTILINE,
            0,0,0,0,h,(HMENU)OPACITY_INPUT,instance,nullptr);
        SendMessageW(opacityInput,EM_SETLIMITTEXT,3,0);SetWindowTheme(opacityInput,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
        SetWindowSubclass(opacityInput,OpacityInputProc,1,0);
        iconSizeInput=CreateWindowExW(0,L"EDIT",L"16",WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS|ES_CENTER|ES_NUMBER|ES_AUTOHSCROLL|ES_MULTILINE,
            0,0,0,0,h,(HMENU)ICON_SIZE_INPUT,instance,nullptr);
        SendMessageW(iconSizeInput,EM_SETLIMITTEXT,2,0);SetWindowTheme(iconSizeInput,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
        SetWindowSubclass(iconSizeInput,OpacityInputProc,1,0);
        settingsScroll=CreateWindowExW(0,L"SCROLLBAR",L"",WS_CHILD|WS_TABSTOP|SBS_VERT,0,0,0,0,h,(HMENU)SETTINGS_SCROLL,instance,nullptr);
        SetWindowTheme(settingsScroll,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);SetWindowSubclass(settingsScroll,SettingsScrollProc,1,0);
        if(!documentHost.Create(h,editor,imageViewer,viewerError))return -1;
        if(!documentBar.Create(h,instance) || !browserTabBar.Create(h,instance,true) || !projectTree.Create(h,172))return -1;
        homeVersion=CreateWindowExW(0,L"STATIC",L"v" QUIETSHARE_VERSION_WIDE,WS_CHILD|WS_CLIPSIBLINGS|SS_RIGHT|SS_CENTERIMAGE,
                                   0,0,0,0,h,(HMENU)HOME_VERSION,instance,nullptr);
        HWND treeTip=TreeView_GetToolTips(projectTree.Handle());if(treeTip && !ApplyPrivacy(treeTip).applied)
            SetWindowLongPtrW(projectTree.Handle(),GWL_STYLE,GetWindowLongPtrW(projectTree.Handle(),GWL_STYLE)|TVS_NOTOOLTIPS);
        ApplyPrivacy(tooltips);
        findInput=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL,0,0,0,0,h,(HMENU)FIND_INPUT,instance,nullptr);
        webAddress=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS|ES_AUTOHSCROLL|ES_MULTILINE,0,0,0,0,h,(HMENU)WEB_ADDRESS,instance,nullptr);
        SendMessageW(webAddress,EM_SETLIMITTEXT,4096,0);SetWindowTheme(webAddress,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
        SetWindowSubclass(webAddress,AddressInputProc,1,0);
        sourceMonitor=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,0,0,0,0,h,(HMENU)SOURCE_MONITOR,instance,nullptr);
        destinationMonitor=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL,0,0,0,0,h,(HMENU)DEST_MONITOR,instance,nullptr);
        for(HWND combo:{sourceMonitor,destinationMonitor})SetWindowSubclass(combo,MonitorComboProc,1,0);
        std::vector<HWND> publicLists;
        for(HWND combo:{sourceMonitor,destinationMonitor}) {
            COMBOBOXINFO info{};info.cbSize=sizeof(info);
            if(GetComboBoxInfo(combo,&info))publicLists.push_back(info.hwndList);
        }
        mirror->SetPublicPopups(publicLists);
        for(HWND control:{findInput,sourceMonitor,destinationMonitor})SetWindowTheme(control,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
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
        browser.SetMediaPreferences(preferences.browserMuted,preferences.browserAllowCamera,preferences.browserAllowMicrophone);
        RefreshFonts();
        ApplyTextFormat();
        page = qa ? Page::Content : Page::Home;
        SendMessageW(editor, EM_SETSEL, 0, 0);
        if (!qa)
        {
            std::wstring hotkeyError;
            if(!globalHotkeys.Initialize(h,preferences,hotkeyError))preferenceError=PreferenceErrorText(hotkeyError);
            f8Registered=globalHotkeys.Registered(PreferenceCommand::Mask);recallRegistered=globalHotkeys.Registered(PreferenceCommand::Recall);
            ApplyWindowAppearance();
            UpdateButtons();
            SetTimer(h, PRIVACY_TIMER, 250, nullptr);
            UpdateMirrorTimer();
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
    case BrowserProbeMessage: {
        if(wp!=browser.Generation()||!browser.Enabled()||!mirror)return 0;
        const unsigned token=(unsigned)lp;
        if(!token||token!=browser.ProbeToken())return 0;
        if(Blocked()){browser.Lock();return 0;}
        verifying=true;documentHost.Hide();Layout();
        const unsigned generation=browser.Generation();
        const bool passed=mirror->Verify(h,SelectedMonitor(sourceMonitor),[token](HWND probe){return browser.MountProbe(probe,token);});
        verifying=false;
        if(window&&browser.Enabled()&&generation==browser.Generation())browser.FinishProbe(passed&&!masked&&topologyKnown&&!cloned,token);
        Layout();return 0;
    }
    case BrowserChangedMessage:
        if(wp!=browser.Generation())return 0;
        Layout();return 0;
    case BrowserCommandMessage:
        if(wp==browser.Generation()&&page==Page::Browser&&!Blocked()) {
            if(lp>0&&lp<=PreferenceCommandCount)RunPreferenceCommand(static_cast<PreferenceCommand>(lp-1));
            else if(lp==PreferenceCommandCount+1){SetFocus(webAddress);SendMessageW(webAddress,EM_SETSEL,0,-1);}
        }
        return 0;
    case WM_APP+33:if(!DispatchPreferenceShortcut(L'S',ShortcutModifiers()))SaveFile();return 0;
    case MirrorStoppedMessage:
        if(!shuttingDown&&mirror&&mirror->ConsumeStoppedNotice((unsigned)wp))ShowProjectionStoppedNotice();
        return 0;
    case BrowserTabActionMessage:
        if(wp==browser.Generation()&&page==Page::Browser&&!Blocked()) {
            browser.ProcessTabAction((int)lp);Layout();
        }
        else if(wp==browser.Generation())browser.ClearQueuedActions();
        return 0;
    case BrowserZoomMessage:
        if(wp==browser.Generation()&&page==Page::Browser&&!Blocked()&&browser.Ready()) {
            if(lp)browser.AdjustZoomPercent((int)lp);else browser.ResetZoom();UpdateBrowserZoomControls();
        }
        return 0;
    case BrowserZoomChangedMessage:
        if(wp==browser.Generation()&&!Blocked())UpdateBrowserZoomControls();
        return 0;
    case WM_MOVE:browser.ParentMoved();UpdateMonitorLabels();return 0;
    case WM_MOUSEWHEEL:
        if(page==Page::Settings) {
            const int distance=GET_WHEEL_DELTA_WPARAM(wp)*48+settingsWheelRemainder;
            const int movement=distance/WHEEL_DELTA;settingsWheelRemainder=distance%WHEEL_DELTA;
            ScrollSettings(settingsOffset-movement);
            if((settingsOffset==0&&distance>0)||(settingsOffset==settingsMaximum&&distance<0))settingsWheelRemainder=0;
            return 0;
        }
        break;
    case WM_VSCROLL:
        if(page==Page::Settings&&(HWND)lp==settingsScroll) {
            int next=settingsOffset;const int action=LOWORD(wp),visible=MulDiv(settingsViewport.bottom-settingsViewport.top,96,dpi);
            if(action==SB_LINEUP)next-=48;else if(action==SB_LINEDOWN)next+=48;
            else if(action==SB_PAGEUP)next-=std::max(48,visible-32);else if(action==SB_PAGEDOWN)next+=std::max(48,visible-32);
            else if(action==SB_TOP)next=0;else if(action==SB_BOTTOM)next=settingsMaximum;
            else if(action==SB_THUMBTRACK||action==SB_THUMBPOSITION)next=HIWORD(wp);
            ScrollSettings(next);return 0;
        }
        break;
    case WM_SETCURSOR:
        if(LOWORD(lp)==HTCLIENT) {
            POINT point{};GetCursorPos(&point);ScreenToClient(h,&point);
            if(splitterDragging||PtInRect(&splitterRect,point)){SetCursor(LoadCursorW(nullptr,IDC_SIZEWE));return TRUE;}
        }
        break;
    case WM_CONTEXTMENU:
        if((HWND)wp==GetDlgItem(h,NAV_SCRIPT)){OpenContentMenu();return 0;}
        break;
    case WM_LBUTTONDOWN: {
        POINT point{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        if(!Blocked()&&PtInRect(&splitterRect,point)){splitterDragging=true;SetCapture(h);return 0;}
        if(focusMode&&point.y<S(32)){SendMessageW(h,WM_NCLBUTTONDOWN,HTCAPTION,0);return 0;}
        break;
    }
    case WM_MOUSEMOVE: {
        POINT point{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        if(splitterDragging&&GetCapture()==h) {
            if(Blocked()||splitterRect.right<=splitterRect.left){ReleaseCapture();return 0;}
            const int left=MulDiv(explorerRect.left,96,dpi);
            const int next=std::clamp(MulDiv(point.x,96,dpi)-left,112,std::min(420,std::max(112,contentRight-left-192)));
            if(next!=explorerWidth){explorerWidth=next;Layout();}
            return 0;
        }
        const bool hover=PtInRect(&splitterRect,point);
        if(hover!=splitterHovered){splitterHovered=hover;InvalidateRect(h,&splitterRect,FALSE);}
        if(hover){TRACKMOUSEEVENT track{sizeof(track),TME_LEAVE,h,0};TrackMouseEvent(&track);}
        break;
    }
    case WM_MOUSELEAVE:
        if(splitterHovered){splitterHovered=false;InvalidateRect(h,&splitterRect,FALSE);}return 0;
    case WM_LBUTTONUP:
        if(splitterDragging){ReleaseCapture();SaveLayoutPreferences();return 0;}break;
    case WM_CAPTURECHANGED:
        if(splitterDragging){splitterDragging=false;InvalidateRect(h,&splitterRect,FALSE);}return 0;
    case WM_WINDOWPOSCHANGING: {
        // Windows applies its size constraints first; then keep the complete
        // private frame on this capture session's source before it can move.
        const auto result=DefWindowProcW(h,message,wp,lp);
        if(mirror)mirror->ConstrainPrivateWindowPosition(*reinterpret_cast<WINDOWPOS *>(lp));
        return result;
    }
    case WM_MOVING:
    case WM_SIZING:
        if(mirror&&mirror->ConstrainPrivateWindowRect(*reinterpret_cast<RECT *>(lp)))return TRUE;
        break;
    case WM_SIZE:
        if (wp == SIZE_MINIMIZED)
            Pause();
        // Revalidate restored bounds before a private viewer becomes visible.
        if(!qa&&mirror)mirror->Tick();
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
        case 10:return browser.State();
        case 11:return browser.BrowserProcessId();
        case 12:return browser.Verified()?1:0;
        case 13:return browser.Loaded()?1:0;
        case 14:return browser.ErrorCode();
        case 15:return browser.Suspended()?1:0;
        case 16:return (activityVisible?1:0)|(tabsVisible?2:0)|(toolbarVisible?4:0)|(statusVisible?8:0)|(treeVisible?16:0);
        case 17:return explorerWidth;
        case 18:return splitterDragging&&GetCapture()==h;
        case 19:return settingsOffset;
        case 20:return settingsMaximum;
        case 21:return page==Page::Settings;
        case 22:return !sizeError.empty();
        case 23:return running;
        case 24:return browser.ZoomPercent();
        case 25:return mirror&&mirror->Running();
        case 26:return resourceMode;
        case 27:return mirror?mirror->TargetFps():60;
        case 28:return mirrorTimerInterval;
        case 29:for(const auto& monitor:monitors)if(monitor.primary)return monitor.displayNumber;return 0;
        case 30:return monitors.size();
        case 31:case 32:case 33: {
            SCROLLBARINFO info{};info.cbSize=sizeof(info);
            if(!GetScrollBarInfo(settingsScroll,OBJID_CLIENT,&info))return 0;
            return wp==31?info.xyThumbBottom-info.xyThumbTop:wp==32?info.xyThumbTop:info.dxyLineButton;
        }
        case 34:return browser.Tabs().size();
        case 35:return browser.ActiveTabId();
        case 36:return browser.BackgroundSuspendedCount();
        case 37:return browser.PendingTabCount();
        case 38:return browser.ControllerCount();
        case 39:return QUIETSHARE_VERSION_NUMBER;
            case 40:return settingsCategory;
        case 41:return preferences.opacityPercent;
        case 42:return preferences.textWeight;
        case 43:return capturingShortcut+1;
        case 44:return !preferenceError.empty();
            case 64:return documentBar.Count();
        case 65:{DWORD affinity=0;return GetWindowDisplayAffinity(window,&affinity)&&affinity==0x11;}
            case 60:return preferences.iconSize;
            case 61:return preferences.browserMuted;
            case 62:return preferences.browserAllowCamera;
            case 63:return preferences.browserAllowMicrophone;
        default:if(wp>=45&&wp<45+PreferenceCommandCount){const auto key=preferences.shortcuts[wp-45];return key.key|(key.modifiers<<8);}
        }
        return 0;
    case WM_APP+16:
        if(!Blocked())UpdateButtons();
        return 0;
    case WM_APP+30:
        return MatchShortcut(preferences.Key(PreferenceCommand::Mask),(UINT)wp,ShortcutModifiers())&&!globalHotkeys.Registered(PreferenceCommand::Mask);
    case WM_APP+31:Layout();return 0;
    case DocumentActivateMessage:ActivateDocument((DocumentId)wp);return 0;
    case DocumentCloseMessage:CloseDocument((DocumentId)wp);return 0;
    case BrowserTabActivateMessage:
        if(page==Page::Browser&&!Blocked()){browser.ActivateTab((unsigned)wp);Layout();}
        return 0;
    case BrowserTabCloseMessage:
        if(page==Page::Browser&&!Blocked()){browser.CloseTab((unsigned)wp);Layout();}
        return 0;
    case WM_NOTIFY: {
        auto notification = (NMHDR *)lp;
        if(notification->hwndFrom==projectTree.Handle() && notification->code==NM_CUSTOMDRAW) {
            auto draw=reinterpret_cast<NMTVCUSTOMDRAW*>(lp);
            if(draw->nmcd.dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYITEMDRAW;
            if(draw->nmcd.dwDrawStage==CDDS_ITEMPREPAINT) {
                draw->clrText=TEXT;draw->clrTextBk=(draw->nmcd.uItemState&CDIS_SELECTED)?ACCENT_DARK:CARD;
                return CDRF_NEWFONT;
            }
        }
        if(notification->hwndFrom==codeViewer.Window() && notification->code==2018 && !Blocked())UpdateButtons();
        std::wstring projectFile;
        if(!Blocked() && projectTree.HandleNotify(notification,projectFile,importError)) {
            if(!projectFile.empty())OpenContentPath(projectFile);return 0;
        }
        break;
    }
    case WM_CTLCOLORBTN:
        SetBkColor((HDC)wp,BG);SetTextColor((HDC)wp,TEXT);return (LRESULT)bgBrush;
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORSTATIC:
        if((HWND)lp==homeVersion){SetBkColor((HDC)wp,page==Page::Home?HOME_BG:BG);SetTextColor((HDC)wp,MUTED);return (LRESULT)bgBrush;}
        if((HWND)lp==sizeInput||(HWND)lp==opacityInput||(HWND)lp==iconSizeInput){
            SetBkColor((HDC)wp,ELEVATED);SetTextColor((HDC)wp,TEXT);return (LRESULT)numberBrush;
        }
        SetBkColor((HDC)wp, CARD);
        SetTextColor((HDC)wp, TEXT);
        return (LRESULT)cardBrush;
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
        if(LOWORD(wp)==WEB_ADDRESS&&HIWORD(wp)==EN_CHANGE){if(!updatingWebAddress)webAddressEdited=true;return 0;}
        if(LOWORD(wp)==SIZE_INPUT&&HIWORD(wp)==EN_CHANGE&&!updatingSize){sizeError.clear();InvalidateRect(h,nullptr,FALSE);return 0;}
        if(LOWORD(wp)==SIZE_INPUT&&HIWORD(wp)==EN_KILLFOCUS){CommitSizeInput();return 0;}
        if(LOWORD(wp)==OPACITY_INPUT&&HIWORD(wp)==EN_KILLFOCUS){CommitOpacity();return 0;}
        if(LOWORD(wp)==OPACITY_INPUT&&HIWORD(wp)==EN_CHANGE){if(!updatingOpacity){preferenceError.clear();InvalidateRect(h,nullptr,FALSE);}return 0;}
        if(LOWORD(wp)==ICON_SIZE_INPUT&&HIWORD(wp)==EN_KILLFOCUS){CommitIconSize();return 0;}
        if(LOWORD(wp)==ICON_SIZE_INPUT&&HIWORD(wp)==EN_CHANGE){if(!updatingIconSize){preferenceError.clear();InvalidateRect(h,nullptr,FALSE);}return 0;}
        if((LOWORD(wp)==SOURCE_MONITOR || LOWORD(wp)==DEST_MONITOR) && HIWORD(wp)==CBN_SELCHANGE) {
            documentHost.Hide();if(mirror)mirror->Invalidate();Layout();return 0;
        }
        if(LOWORD(wp)==FIND_INPUT && HIWORD(wp)==EN_CHANGE){findMessage.clear();return 0;}
        if(LOWORD(wp)==EDITOR&&HIWORD(wp)==EN_CHANGE&&!loading&&viewingFile&&!Blocked()&&preferences.allowFileEditing){
            documents.Active().dirty=true;return 0;
        }
        if (LOWORD(wp) == EDITOR && HIWORD(wp) == EN_CHANGE && !loading && !viewingFile)
        {
            dirty = true;scriptDraft=TextOf(editor);documents.Get(0)->content.text=scriptDraft;
            SetTimer(h, SAVE_TIMER, 900, nullptr);
            InvalidateRect(h, nullptr, FALSE);
        }
        if (HIWORD(wp) != BN_CLICKED)
            return 0;
        if(LOWORD(wp)>=1000&&LOWORD(wp)<=1000+CustomThemeId) {
            auto candidate=preferences;candidate.themeId=LOWORD(wp)-1000;StorePreferences(candidate);Layout();return 0;
        }
        if((LOWORD(wp)>=SETTINGS_APPEARANCE&&LOWORD(wp)<=SETTINGS_KEYS)||LOWORD(wp)==SETTINGS_BROWSER) {
            capturingShortcut=-1;preferenceError.clear();settingsCategory=LOWORD(wp)==SETTINGS_BROWSER?5:LOWORD(wp)-SETTINGS_APPEARANCE;
            if(LOWORD(wp)==SETTINGS_BROWSER){Pause();focusMode=false;page=Page::Settings;}
            settingsOffset=0;settingsWheelRemainder=0;Layout();return 0;
        }
        if(LOWORD(wp)>=SHORTCUT_FIRST&&LOWORD(wp)<SHORTCUT_FIRST+PreferenceCommandCount) {
            capturingShortcut=LOWORD(wp)-SHORTCUT_FIRST;preferenceError.clear();SetFocus(GetDlgItem(h,LOWORD(wp)));UpdateButtons();return 0;
        }
        if(LOWORD(wp)>=RECENT_FIRST&&LOWORD(wp)<RECENT_FIRST+18){
            if(Blocked())return 0;
            const size_t index=recentOffset+(LOWORD(wp)-RECENT_FIRST)/3;const int part=(LOWORD(wp)-RECENT_FIRST)%3;
            if(index>=recentWorkspace.entries.size())return 0;
            const auto entry=recentWorkspace.entries[index];
            if(part==1){recentWorkspace.entries[index].pinned=!entry.pinned;recentWorkspace.Sort();PersistRecent();}
            else if(part==2){recentWorkspace.entries.erase(recentWorkspace.entries.begin()+index);PersistRecent();}
            else if(entry.folder){std::wstring canonical;if(CanonicalLocalPath(entry.path,true,canonical,importError)&&projectTree.OpenFolder(canonical,importError)){
                recentWorkspace.Touch(canonical,true);PersistRecent();treeVisible=true;page=Page::Content;focusMode=false;
            }}else OpenContentPath(entry.path);
            Layout();return 0;
        }
        switch (LOWORD(wp))
        {
        case SAVE_FILE:SaveFile();break;
        case RECENT_CLEAR:if(!Blocked()){recentWorkspace.entries.clear();recentOffset=0;PersistRecent();Layout();}break;
        case RECENT_NEXT:if(!Blocked()){recentOffset+=recentRows;Layout();}break;
        case RECENT_PREV:if(!Blocked()){recentOffset=recentOffset>=static_cast<size_t>(recentRows)?recentOffset-recentRows:0;Layout();}break;
        case REMEMBER_SIZE:{auto candidate=preferences;candidate.rememberWindowSize=!candidate.rememberWindowSize;StorePreferences(candidate);Layout();break;}
        case ALLOW_EDIT:{if(!SaveActiveState())break;auto candidate=preferences;candidate.allowFileEditing=!candidate.allowFileEditing;
            StorePreferences(candidate);if(viewingFile)reading=!preferences.allowFileEditing;
            SendMessageW(editor,EM_SETREADONLY,viewingFile?!preferences.allowFileEditing:reading,0);
            codeViewer.SetEditable(preferences.allowFileEditing);Layout();break;}

        case TRANSCRIPTION_LANGUAGE:{
            std::vector<ToolMenuItem> entries{{TRANSCRIPTION_VI,T(L"Tiếng Việt",L"Vietnamese"),true,0xeb51,preferences.transcriptionLanguage==0},
                {TRANSCRIPTION_EN,T(L"Tiếng Anh",L"English"),true,0xeb51,preferences.transcriptionLanguage==1}};
            RECT anchor{};GetWindowRect(GetDlgItem(window,TRANSCRIPTION_LANGUAGE),&anchor);ToolMenu menu;
            const int chosen=menu.Show(window,anchor,dpi,uiFont,std::move(entries));
            if(chosen==TRANSCRIPTION_VI||chosen==TRANSCRIPTION_EN){auto candidate=preferences;candidate.transcriptionLanguage=chosen==TRANSCRIPTION_EN?1:0;StorePreferences(candidate);Layout();}
            break;
        }
        case SCRIPT_TAB_LABELS:case BROWSER_TAB_LABELS:{auto candidate=preferences;
            if(LOWORD(wp)==SCRIPT_TAB_LABELS)candidate.numberedDocumentTabs=!candidate.numberedDocumentTabs;
            else candidate.numberedBrowserTabs=!candidate.numberedBrowserTabs;
            StorePreferences(candidate);Layout();break;}

        case MEETING_TRANSCRIBE:
            meetingSettings.transcriptionEnabled=!meetingSettings.transcriptionEnabled;
            if(!SaveMeetingSettings(dataDir,meetingSettings))
                importError=T(L"Không lưu được thiết lập chép lời.",L"The transcription setting could not be saved.");
            else importError.clear();
            Layout();break;
        case MEETING_PROVIDER:{
            std::vector<ToolMenuItem> entries{{PROVIDER_LOCAL,T(L"Âm thanh máy / Windows SAPI",L"System audio / Windows SAPI"),true,0xeba7,meetingSettings.provider==MeetingProvider::Generic},
                {PROVIDER_TEAMS,L"Microsoft Teams",true,0xeb51,meetingSettings.provider==MeetingProvider::Teams},
                {PROVIDER_ZOOM,L"Zoom",true,0xeb51,meetingSettings.provider==MeetingProvider::Zoom},
                {PROVIDER_MEET,L"Google Meet",true,0xeb51,meetingSettings.provider==MeetingProvider::GoogleMeet}};
            RECT anchor{};GetWindowRect(GetDlgItem(window,MEETING_PROVIDER),&anchor);ToolMenu menu;
            const int chosen=menu.Show(window,anchor,dpi,uiFont,std::move(entries));
            MeetingProvider provider=meetingSettings.provider;
            if(chosen==PROVIDER_LOCAL)provider=MeetingProvider::Generic;
            else if(chosen==PROVIDER_TEAMS)provider=MeetingProvider::Teams;
            else if(chosen==PROVIDER_ZOOM)provider=MeetingProvider::Zoom;
            else if(chosen==PROVIDER_MEET)provider=MeetingProvider::GoogleMeet;
            if(chosen>=PROVIDER_LOCAL&&chosen<=PROVIDER_MEET){meetingSettings.provider=provider;SaveMeetingSettings(dataDir,meetingSettings);Layout();}
            break;
        }
        case MEETING_OUTPUT:{
            std::vector<ToolMenuItem> entries{{OUTPUT_WAV,T(L"Giữ WAV và transcript",L"Keep WAV and transcript"),true,0xeba7,meetingSettings.exportMode==MeetingExportMode::WavAndTranscript},
                {OUTPUT_TRANSCRIPT,T(L"Chỉ giữ nội dung cuộc họp",L"Keep transcript only"),true,0xeb51,meetingSettings.exportMode==MeetingExportMode::TranscriptOnly}};
            RECT anchor{};GetWindowRect(GetDlgItem(window,MEETING_OUTPUT),&anchor);ToolMenu menu;
            const int chosen=menu.Show(window,anchor,dpi,uiFont,std::move(entries));
            if(chosen==OUTPUT_WAV||chosen==OUTPUT_TRANSCRIPT){meetingSettings.exportMode=chosen==OUTPUT_TRANSCRIPT?MeetingExportMode::TranscriptOnly:MeetingExportMode::WavAndTranscript;SaveMeetingSettings(dataDir,meetingSettings);Layout();}
            break;
        }
        case MEETING_DIRECTORY:ChooseMeetingDirectory();break;
        case MEETING_IMPORT:ImportMeetingFile();break;
        case CUSTOM_BACKGROUND:case CUSTOM_PANEL:case CUSTOM_TEXT:case CUSTOM_ACCENT:ChooseCustomColor(LOWORD(wp));break;
        case CUSTOM_RESET:
            customThemeColors=CustomThemeColors{};SaveCustomTheme(dataDir,customThemeColors);
            {auto candidate=preferences;candidate.themeId=CustomThemeId;StorePreferences(candidate);}Layout();break;

        case THEME_SELECT:{
            std::vector<ToolMenuItem> entries;
            for(int i=0;i<static_cast<int>(Themes.size());++i)
                entries.push_back({1000+i,ThemeName(i,english),true,0,preferences.themeId==i});
            entries.push_back({1000+CustomThemeId,ThemeName(CustomThemeId,english),true,0,preferences.themeId==CustomThemeId});
            RECT anchor{};GetWindowRect(GetDlgItem(window,THEME_SELECT),&anchor);
            ToolMenu menu;int chosen=menu.Show(window,anchor,dpi,uiFont,std::move(entries));
            if(chosen>=1000&&chosen<=1000+CustomThemeId) {
                SendMessageW(window,WM_COMMAND,chosen,0);
            }
            break;
        }
        case TEXT_WEIGHT:{auto candidate=preferences;candidate.textWeight=candidate.textWeight==400?600:candidate.textWeight==600?700:400;StorePreferences(candidate);Layout();break;}
        case RESET_PREFERENCES:{auto candidate=preferences;if(settingsCategory==4)candidate.shortcuts=DefaultPreferences().shortcuts;else{candidate.opacityPercent=100;candidate.textWeight=400;candidate.iconSize=16;candidate.themeId=0;}StorePreferences(candidate);Layout();break;}
        case OPACITY_LESS:case OPACITY_MORE:{auto candidate=preferences;candidate.opacityPercent=std::clamp(candidate.opacityPercent+(LOWORD(wp)==OPACITY_MORE?1:-1),40,100);StorePreferences(candidate);Layout();break;}
        case ICON_SMALLER:case ICON_LARGER:{auto candidate=preferences;candidate.iconSize=std::clamp(candidate.iconSize+(LOWORD(wp)==ICON_LARGER?1:-1),12,24);StorePreferences(candidate);Layout();break;}
        case WEB_AUDIO:case BROWSER_MUTED:case BROWSER_CAMERA:case BROWSER_MICROPHONE:{auto candidate=preferences;
            if(LOWORD(wp)==WEB_AUDIO||LOWORD(wp)==BROWSER_MUTED)candidate.browserMuted=!candidate.browserMuted;
            else if(LOWORD(wp)==BROWSER_CAMERA)candidate.browserAllowCamera=!candidate.browserAllowCamera;
            else candidate.browserAllowMicrophone=!candidate.browserAllowMicrophone;
            StorePreferences(candidate);Layout();break;}
        case MEETING_RECORD:
            if(meetingTranscriber.Running())break;
            if(meetingRecorder.Running()) {
                meetingRecorder.Stop();
                KillTimer(h,MEETING_TIMER);
                transcriptionHandled=false;transcriptionNotice.clear();
                meetingRecordingPath=meetingRecorder.OutputPath();
                if(meetingRecorder.BytesWritten()>0&&meetingSettings.transcriptionEnabled&&meetingTranscriber.Start(meetingRecordingPath,
                        preferences.transcriptionLanguage==1?TranscriptionLanguage::English:TranscriptionLanguage::Vietnamese))
                    SetTimer(h,MEETING_TIMER,500,nullptr);
                else if(meetingRecorder.BytesWritten()>0&&meetingSettings.transcriptionEnabled)
                    transcriptionNotice=meetingTranscriber.Status(english);
                else if(meetingRecorder.BytesWritten()>0)
                    transcriptionNotice=T(L"Đã lưu WAV cuộc họp · chép lời đang tắt",L"Meeting WAV saved · transcription is off");
                else if(meetingRecorder.ErrorCode()!=0)
                    transcriptionNotice=meetingRecorder.Status(english);
            } else if(meetingRecorder.Start(dataDir,meetingSettings.outputDirectory)) {
                transcriptionHandled=false;transcriptionNotice.clear();
                SetTimer(h,MEETING_TIMER,500,nullptr);
            }
            Layout();break;
        case NAV_BROWSER:Pause();focusMode=false;page=Page::Browser;Layout();break;
        case BROWSER_TOGGLE:
            if(browser.Enabled())browser.Disable();
            else if(!Blocked()&&!qa){focusMode=false;browser.Enable(h,dataDir+L"\\WebProfile",TextOf(webAddress));}
            Layout();break;
        case WEB_GO:if(!Blocked()){const auto target=TextOf(webAddress);webAddressEdited=false;browser.Navigate(target);browser.Focus();}break;
        case WEB_MENU: {
            std::vector<ToolMenuItem> entries{{WEB_NEW_TAB,T(L"Tab mới   Ctrl+T",L"New tab   Ctrl+T"),browser.CanNewTab(),0xea60},
                {BROWSER_MUTED,T(preferences.browserMuted?L"Bật âm thanh":L"Tắt âm thanh",
                    preferences.browserMuted?L"Unmute audio":L"Mute audio"),true,
                    static_cast<wchar_t>(preferences.browserMuted?0xeb75:0xeb24),preferences.browserMuted},
                {SETTINGS_BROWSER,T(L"Thiết lập trình duyệt",L"Browser settings"),true,0xeb51},
                {BROWSER_TOGGLE,T(L"Tắt trình duyệt",L"Disable browser"),true,0xea76}};
            RECT anchor{};GetWindowRect(GetDlgItem(h,WEB_MENU),&anchor);ToolMenu menu;
            const int action=menu.Show(h,anchor,dpi,uiFont,std::move(entries));if(action)PostMessageW(h,WM_COMMAND,action,0);break;
        }
        case WEB_BACK:if(!Blocked()){webAddressEdited=false;browser.Back();}break;
        case WEB_FORWARD:if(!Blocked()){webAddressEdited=false;browser.Forward();}break;
        case WEB_RELOAD:if(!Blocked()){webAddressEdited=false;browser.Reload();}break;
        case WEB_ZOOM_OUT:if(!Blocked()&&browser.Ready()){browser.AdjustZoomPercent(-10);UpdateBrowserZoomControls();}break;
        case WEB_ZOOM_IN:if(!Blocked()&&browser.Ready()){browser.AdjustZoomPercent(10);UpdateBrowserZoomControls();}break;
        case WEB_ZOOM_RESET:if(!Blocked()&&browser.Ready()){browser.ResetZoom();UpdateBrowserZoomControls();}break;
        case WEB_NEW_TAB:if(page==Page::Browser&&!Blocked()){browser.NewTab();Layout();}break;
        case RESOURCE_MODE:
            resourceMode=(resourceMode+1)%3;if(mirror)mirror->SetResourceMode(resourceMode);
            SaveLayoutPreferences();UpdateMirrorTimer();UpdateButtons();break;
        case CLOSE_APPLICATION:PostMessageW(h,WM_CLOSE,0,0);break;
        case NAV_HOME:
            Pause(); focusMode = false; page = Page::Home; Layout(); break;
        case NAV_PRESENTATION:
            Pause();focusMode=false;page=Page::Settings;settingsCategory=3;settingsOffset=0;Layout();break;
        case HOME_SETTINGS:
            settingsCategory=3;
            [[fallthrough]];
        case NAV_SETTINGS:
            Pause(); focusMode = false; page = Page::Settings; Layout(); break;
        case OPEN_DISPLAY_SETTINGS:
            Pause();focusMode=false;page=Page::Settings;settingsCategory=3;settingsOffset=0;Layout();break;
        case NAV_SCRIPT:
            if(!Blocked()){Pause();focusMode=false;page=Page::Content;Layout();SetFocus(ContentWindow());}
            break;
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
        case MIRROR_ACTION:
            SendMessageW(h,WM_COMMAND,mirror&&mirror->Running()?MIRROR_STOP:MIRROR_START,0);break;
        case MIRROR_START:
            if(!mirror || masked)break;
            documentHost.Hide();verifying=true;Layout();
            if(mirror->Start(SelectedMonitor(sourceMonitor),SelectedMonitor(destinationMonitor),mirrorPointer))ClearProjectionStoppedNotice();
            else {page=Page::Settings;settingsCategory=3;settingsOffset=0;}
            verifying=false;Layout();break;
        case MIRROR_STOP:ClearProjectionStoppedNotice();documentHost.Hide();if(mirror)mirror->Stop();Layout();break;
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
        case WRAP:if(!Blocked()&&!IsImage()&&!IsPdf()){Pause();if(IsCode())codeViewer.SetWrap(!codeViewer.GetState().wrap);else{documents.Active().state.wrap=!documents.Active().state.wrap;ApplyTextWrap();}UpdateButtons();}break;
        case CLOSE_TAB:CloseDocument(documents.Active().id);break;
        case CLOSE_OTHERS:if(!Blocked()&&ConfirmFiles(documents.Active().id)){SaveActiveState();documentHost.CloseExternal();documents.CloseOthers(documents.Active().id);RenderDocument();}break;
        case CLOSE_FILES:if(!Blocked()&&ConfirmFiles()){SaveActiveState();documentHost.CloseExternal();documents.CloseAllFiles();viewingFile=false;reading=false;RenderDocument();}break;
        case LAYOUT_MENU:LayoutMenu();break;
        case SHOW_TREE:treeVisible=!treeVisible;SaveLayoutPreferences();Layout();break;
        case SHOW_ACTIVITY:activityVisible=!activityVisible;SaveLayoutPreferences();Layout();break;
        case SHOW_TABS:tabsVisible=!tabsVisible;SaveLayoutPreferences();Layout();break;
        case SHOW_TOOLBAR:toolbarVisible=!toolbarVisible;SaveLayoutPreferences();Layout();break;
        case SHOW_STATUS:statusVisible=!statusVisible;SaveLayoutPreferences();Layout();break;
        case RESET_LAYOUT:
            activityVisible=tabsVisible=toolbarVisible=statusVisible=treeVisible=true;explorerWidth=208;
            SaveLayoutPreferences();Layout();break;
        case INCLUDE_IGNORED:if(!Blocked())projectTree.SetIncludeIgnored(!projectTree.IncludeIgnored(),importError);UpdateButtons();break;
        case RELOAD_FILE:
            if(!Blocked()&&reloadPending){SaveActiveState();if(!ConfirmFile(reloadPending))break;documentHost.CloseExternal();documents.Reload(reloadPending,importError);reloadPending=0;RenderDocument();}break;
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
            AdjustFont(page==Page::Settings?-2:-4);
            break;
        case LARGER:
            AdjustFont(page==Page::Settings?2:4);
            break;
        case SPEED:
            speed = (speed + 1) % 3;
            UpdateButtons();
            break;
        case MASK:
            ToggleMask();
            break;
        case TOPMOST:
        case PIN_WINDOW:
            SetTopmost(!topmost);
            UpdateButtons();
            break;
        case LANGUAGE:
            english = !english;
            UpdateProjectionNoticeCaption();
            labeledWindowMonitor=nullptr;UpdateMonitorLabels();
            Layout();
            break;
        case RESET_ZOOM:
            ResetFont();
            break;
        }
        return 0;
    case WM_HOTKEY:
        if(capturingShortcut>=0){CaptureShortcut(HIWORD(lp),LOWORD(lp));return 0;}
        {PreferenceCommand command;
        if(globalHotkeys.CommandForId((int)wp,command,lp))RunPreferenceCommand(command);
        else if(wp==2&&!lp)ToggleMask();else if(wp==3&&!lp)RecallWindow();}
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
        if(mirror)mirror->HandleDisplayChange();
        topologyKnown=TryGetDisplayCloned(cloned);UpdateMonitors();
        if(Blocked()){browser.Lock();documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);}
        Layout();
        return 0;
    case WM_TIMER:
        if(wp==5 && mirror) {
            mirror->Tick();
            UpdateMirrorTimer();
            if(renderedSafe!=IsPrivateContentSafe()) {
                if(Blocked()) {
                    browser.Lock();documentHost.Hide();SendMessageW(tooltips,TTM_POP,0,0);
                    if(filePicker){ShowWindow(filePicker,SW_HIDE);PostMessageW(filePicker,WM_COMMAND,IDCANCEL,0);}
                }
                Layout();
            }
            static ULONGLONG lastStatusPaint=0;
            if((page==Page::Presentation||(page==Page::Settings&&settingsCategory==3)) && GetTickCount64()-lastStatusPaint>=250) {
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
        else if (wp == MEETING_TIMER)
        {
            if (!meetingRecorder.Running()&&!meetingTranscriber.Running()&&!transcriptionHandled) {
                HandleTranscriptionResult();
                transcriptionHandled=true;
            }
            if (!meetingRecorder.Running()&&!meetingTranscriber.Running())
                KillTimer(h, MEETING_TIMER);
            UpdateButtons();
            InvalidateRect(h, nullptr, FALSE);
        }
        else if (wp == QA_TIMER && qa)
            QaTick();
        return 0;
    case WM_CLOSE:
        if(!qa&&!ConfirmFiles())return 0;
        if(!qa&&preferences.rememberWindowSize){
            WINDOWPLACEMENT placement{};placement.length=sizeof(placement);
            if(GetWindowPlacement(h,&placement)){
                RECT r=placement.rcNormalPosition;DWORD values[]={1,static_cast<DWORD>(MulDiv(r.right-r.left,96,dpi)),static_cast<DWORD>(MulDiv(r.bottom-r.top,96,dpi))};
                WriteBytes(dataDir+L"\\window-size.dat",values,sizeof(values));
            }
        }
        if (!qa && dirty && !SaveNotes())
        {
            MessageBoxW(h,
                        T(L"Chưa lưu được nội dung. Hãy sao chép lời nhắc trước khi đóng cửa sổ.",
                          L"The script could not be saved. Copy it before closing."),
                        L"QuietShare", MB_OK | MB_ICONWARNING);
            return 0;
        }
        shuttingDown=true;DestroyWindow(h);
        return 0;
    case WM_DESTROY:
        shuttingDown=true;
        browser.Disable();
        KillTimer(h, MEETING_TIMER);
        meetingRecorder.Stop();
        meetingTranscriber.Stop();
        for (UINT_PTR timer : {PRIVACY_TIMER, SCROLL_TIMER, SAVE_TIMER, QA_TIMER,(UINT_PTR)5})
            KillTimer(h, timer);
        if(mirror)mirror->Stop();
        documentHost.CloseExternal();
        globalHotkeys.Release();
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
    // The QA matrix deliberately resizes hundreds of synthetic layouts inside
    // one timer callback. Keep Windows from capturing a stale ghost surrogate
    // instead of the tested HWND. Normal launches never change this OS policy.
    if(qa)DisableProcessWindowsGhosting();
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
    LoadLayoutPreferences();
    if(!qa){
        LoadPreferences(dataDir,preferences);
        meetingSettings.outputDirectory=DefaultMeetingDirectory(dataDir);
        LoadMeetingSettings(dataDir,meetingSettings);
        if(meetingSettings.outputDirectory.empty())meetingSettings.outputDirectory=DefaultMeetingDirectory(dataDir);
        LoadCustomTheme(dataDir,customThemeColors);
        recentWorkspace.Load(dataDir);
    } else {
        meetingSettings.outputDirectory=DefaultMeetingDirectory(dataDir);
    }
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
    theme=ThemePaletteFor(preferences.themeId);
    bgBrush = CreateSolidBrush(BG);
    cardBrush = CreateSolidBrush(CARD);
    numberBrush = CreateSolidBrush(ELEVATED);
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
            SIZE logical{800,680};
            if(preferences.rememberWindowSize){const auto bytes=ReadBytes(dataDir+L"\\window-size.dat");DWORD values[3]{};
                if(bytes.size()==sizeof(values)){CopyMemory(values,bytes.data(),sizeof(values));if(values[0]==1&&values[1]>=430&&values[1]<=16384&&values[2]>=320&&values[2]<=16384)logical={static_cast<LONG>(values[1]),static_cast<LONG>(values[2])};}}
            const auto fitted=FitRememberedSize(logical,dpi,work);
            int width=fitted.cx,height=fitted.cy;
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
    BOOL dark = theme.dark;
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
        if((message.message==WM_KEYDOWN||message.message==WM_SYSKEYDOWN)&&!(message.lParam&(1L<<30))) {
            if(DispatchPreferenceShortcut((UINT)message.wParam,ShortcutModifiers()))continue;
        }
        if(message.message==WM_KEYDOWN) {
            bool control=(GetKeyState(VK_CONTROL)&0x8000)!=0,shift=(GetKeyState(VK_SHIFT)&0x8000)!=0;
            if(!control && message.hwnd==projectTree.Handle() && message.wParam==VK_RETURN && !Blocked()) {
                const auto file=projectTree.SelectedFile();
                if(!file.empty())OpenContentPath(file);
                else TreeView_Expand(projectTree.Handle(),TreeView_GetSelection(projectTree.Handle()),TVE_TOGGLE);
                continue;
            }
            if(control && message.wParam==L'F'){SendMessageW(window,WM_COMMAND,FIND,0);continue;}
            if(control&&page==Page::Browser&&(message.wParam==L'T'||message.wParam==L'W'||message.wParam==VK_TAB)) {
                if(!Blocked()){browser.ProcessTabAction(message.wParam==L'T'?1:message.wParam==L'W'?2:shift?4:3);Layout();}
                continue;
            }
            if(control && page==Page::Content && message.wParam==L'W'){CloseDocument(documents.Active().id);continue;}
            if(control && page==Page::Content && message.wParam==VK_TAB && !Blocked()) {
                if(!SaveActiveState())continue;documentHost.CloseExternal();documents.Cycle(shift?-1:1,importError);
                if(documents.Active().id==0&&documents.Count()>1)documents.Cycle(shift?-1:1,importError);
                viewingFile=documents.Active().id!=0;RenderDocument();continue;
            }
            if(control && message.wParam==L'C' && IsPdf()) {if(!Blocked())pdfViewer.CopySelection();continue;}
            if(control&&page==Page::Browser&&(message.wParam==L'0'||message.wParam==VK_ADD||message.wParam==VK_OEM_PLUS||message.wParam==VK_SUBTRACT||message.wParam==VK_OEM_MINUS)) {
                const int delta=message.wParam==L'0'?0:message.wParam==VK_SUBTRACT||message.wParam==VK_OEM_MINUS?-10:10;
                if(!Blocked()&&browser.Ready()) {
                    if(delta)browser.AdjustZoomPercent(delta);else browser.ResetZoom();UpdateBrowserZoomControls();
                }
                continue;
            }
            if(control && message.wParam==L'0'){ResetFont();continue;}
            if(control && (message.wParam==VK_ADD || message.wParam==VK_OEM_PLUS)){AdjustFont(2);continue;}
            if(control && (message.wParam==VK_SUBTRACT || message.wParam==VK_OEM_MINUS)){AdjustFont(-2);continue;}
            if(message.hwnd==findInput && message.wParam==VK_RETURN){RunFind(!shift);continue;}
            if(message.hwnd==webAddress && message.wParam==VK_RETURN){SendMessageW(window,WM_COMMAND,WEB_GO,0);continue;}
            if(control&&message.wParam==L'L'&&page==Page::Browser&&IsWindowVisible(webAddress)){SetFocus(webAddress);SendMessageW(webAddress,EM_SETSEL,0,-1);continue;}
            if(message.wParam==VK_ESCAPE && findVisible){findVisible=false;Layout();SetFocus(ContentWindow());continue;}
        }
        if (message.message == WM_KEYDOWN && message.wParam == VK_ESCAPE && focusMode)
        { ToggleFocus(); continue; }
        if (message.message == WM_KEYDOWN && message.wParam == VK_TAB)
            SendMessageW(window, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
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
    DeleteObject(numberBrush);
    FreeLibrary(richEdit);
    if (singleInstance)
        CloseHandle(singleInstance);
    return result < 0 ? 6 : (int)message.wParam;
}
