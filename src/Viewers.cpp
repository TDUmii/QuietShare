#include "Viewers.h"
#include "Theme.h"
#include <ILexer.h>
#include <Lexilla.h>
#include <SciLexer.h>
#include <Scintilla.h>
#include <algorithm>
#include <cmath>
#include <commctrl.h>
#include <cstring>
#include <cwctype>
#include <fpdf_progressive.h>
#include <fpdf_text.h>
#include <fpdfview.h>
#include <uxtheme.h>
#include <vector>
#include <windowsx.h>

namespace quietshare
{
namespace
{
const COLORREF& Background=theme.bg;
const COLORREF& Foreground=theme.text;
const COLORREF& Muted=theme.muted;
const COLORREF& Accent=theme.number;
const COLORREF& Keyword=theme.keyword;
const COLORREF& String=theme.string;
const COLORREF& Comment=theme.comment;
const COLORREF& Number=theme.number;
std::wstring Error(const wchar_t *vi, const wchar_t *en)
{
    return std::wstring(vi) + L" / " + en;
}
HMODULE SafeLoad(const wchar_t *name)
{
    wchar_t path[32768]{};
    const DWORD n = GetModuleFileNameW(nullptr, path, 32768);
    if (!n || n >= 32768)
        return nullptr;
    std::wstring full(path, n);
    const auto slash = full.find_last_of(L"\\/");
    if (slash == std::wstring::npos)
        return nullptr;
    full.resize(slash + 1);
    full += name;
    // Never search CWD, PATH, network locations, or arbitrary project directories.
    return LoadLibraryExW(full.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
}
template <class T> T Symbol(HMODULE module, const char *name)
{
    const FARPROC raw = GetProcAddress(module, name);
    T value = nullptr;
    static_assert(sizeof(value) == sizeof(raw));
    std::memcpy(&value, &raw, sizeof(value));
    return value;
}
std::string Utf8(const std::wstring &text)
{
    if (text.empty())
        return {};
    const int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()),
                                          nullptr, 0, nullptr, nullptr);
    if (!count)
        return {};
    std::string out(count, 0);
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), out.data(), count,
                        nullptr, nullptr);
    return out;
}
std::wstring Lower(std::wstring name)
{
    std::transform(name.begin(), name.end(), name.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return name;
}
struct Lexer
{
    const char *name;
    const wchar_t *label;
    const char *keywords;
};
Lexer LanguageFor(const std::wstring &filename)
{
    const auto name = Lower(filename);
    const auto dot = name.find_last_of(L'.');
    const auto ext = dot == std::wstring::npos ? name : name.substr(dot);
    if (ext == L".py" || ext == L".pyw")
        return {"python", L"Python",
                "False None True and as assert async await break class continue def del elif else except finally for "
                "from global if import in is lambda nonlocal not or pass raise return try while with yield"};
    if (ext == L".js" || ext == L".jsx" || ext == L".ts" || ext == L".tsx" || ext == L".mjs" || ext == L".cjs")
        return {"cpp", L"JavaScript / TypeScript",
                "async await break case catch class const continue debugger default delete do else enum export extends "
                "false finally for from function if implements import in instanceof interface let new null of private "
                "protected public return static super switch this throw true try type typeof undefined var void while "
                "with yield"};
    if (ext == L".cs")
        return {
            "cpp", L"C#",
            "abstract as async await base bool break byte case catch char checked class const continue decimal default "
            "delegate do double else enum event explicit extern false finally fixed float for foreach goto if implicit "
            "in int interface internal is lock long namespace new null object operator out override params private "
            "protected public readonly ref return sbyte sealed short sizeof stackalloc static string struct switch "
            "this throw true try typeof uint ulong unchecked unsafe ushort using var virtual void volatile while"};
    if (ext == L".java")
        return {"cpp", L"Java",
                "abstract assert boolean break byte case catch char class const continue default do double else enum "
                "extends final finally float for if implements import instanceof int interface long native new null "
                "package private protected public return short static strictfp super switch synchronized this throw "
                "throws transient true false try void volatile while"};
    if (ext == L".json" || ext == L".jsonc")
        return {"json", L"JSON", "false true null"};
    if (ext == L".html" || ext == L".htm" || ext == L".vue" || ext == L".svelte")
        return {"hypertext", L"HTML",
                "html head body div span script style p a title meta link input button section nav main img class id "
                "href src type lang charset"};
    if (ext == L".xml" || ext == L".svg" || ext == L".xaml" || ext == L".csproj")
        return {"xml", L"XML", ""};
    if (ext == L".css" || ext == L".scss" || ext == L".less")
        return {"css", L"CSS",
                "color background display width height margin padding border font font-size line-height position top "
                "left right bottom flex grid align-items justify-content"};
    if (ext == L".yaml" || ext == L".yml")
        return {"yaml", L"YAML", "true false null yes no"};
    if (ext == L".md" || ext == L".markdown")
        return {"markdown", L"Markdown", ""};
    if (ext == L".sql")
        return {"sql", L"SQL",
                "select from where insert into values update set delete create alter drop table index join inner outer "
                "left right group by order asc desc having limit as and or not null true false distinct union all case "
                "when then else end count sum avg min max primary key references"};
    if (ext == L".sh" || ext == L".bash" || name == L".bashrc")
        return {"bash", L"Bash", "if then else elif fi for while in do done case esac function select time until"};
    if (ext == L".ps1" || ext == L".psm1" || ext == L".psd1")
        return {
            "powershell", L"PowerShell",
            "begin break catch class continue data do dynamicparam else elseif end enum exit filter finally for "
            "foreach from function if in param process return switch throw trap try until using var while workflow"};
    if (ext == L".rs")
        return {"rust", L"Rust",
                "as async await break const continue crate dyn else enum extern false fn for if impl in let loop match "
                "mod move mut pub ref return self Self static struct super trait true type unsafe use where while"};
    if (ext == L".go")
        return {"cpp", L"Go",
                "break case chan const continue default defer else fallthrough for func go goto if import interface "
                "map package range return select struct switch type var true false nil"};
    if (ext == L".c" || ext == L".cc" || ext == L".cpp" || ext == L".cxx" || ext == L".h" || ext == L".hh" ||
        ext == L".hpp" || ext == L".hxx")
        return {"cpp", L"C / C++",
                "alignas alignof asm auto bool break case catch char class const constexpr continue decltype default "
                "delete do double else enum explicit export extern false float for friend if inline int long mutable "
                "namespace new noexcept nullptr operator private protected public register reinterpret_cast return "
                "short signed sizeof static static_assert struct switch template this thread_local throw true try "
                "typedef typename union unsigned using virtual void volatile wchar_t while"};
    return {"null", L"Plain text", ""};
}
struct CodeRuntime
{
    HMODULE scintilla = nullptr, lexilla = nullptr;
    Lexilla::CreateLexerFn create = nullptr;
    bool Load()
    {
        if (create)
            return true;
        if (!scintilla)
            scintilla = SafeLoad(L"Scintilla.dll");
        if (!lexilla)
            lexilla = SafeLoad(L"Lexilla.dll");
        if (!scintilla || !lexilla)
            return false;
        create = Symbol<Lexilla::CreateLexerFn>(lexilla, "CreateLexer");
        return create != nullptr;
    }
    // Libraries remain loaded until process teardown; native children own lexer objects.
};
CodeRuntime &CodeEngine()
{
    static CodeRuntime engine;
    return engine;
}

struct PdfRuntime
{
    HMODULE module = nullptr;
    bool ready = false;
#define PDF_FN(name) decltype(&name) name = nullptr
    PDF_FN(FPDF_InitLibrary);
    PDF_FN(FPDF_LoadMemDocument64);
    PDF_FN(FPDF_CloseDocument);
    PDF_FN(FPDF_GetLastError);
    PDF_FN(FPDF_GetPageCount);
    PDF_FN(FPDF_LoadPage);
    PDF_FN(FPDF_ClosePage);
    PDF_FN(FPDF_GetPageWidth);
    PDF_FN(FPDF_GetPageHeight);
    PDF_FN(FPDFBitmap_Create);
    PDF_FN(FPDFBitmap_Destroy);
    PDF_FN(FPDFBitmap_FillRect);
    PDF_FN(FPDFBitmap_GetBuffer);
    PDF_FN(FPDFBitmap_GetStride);
    PDF_FN(FPDF_RenderPageBitmap);
    PDF_FN(FPDF_RenderPageBitmap_Start);
    PDF_FN(FPDF_RenderPage_Continue);
    PDF_FN(FPDF_RenderPage_Close);
    PDF_FN(FPDFText_LoadPage);
    PDF_FN(FPDFText_ClosePage);
    PDF_FN(FPDFText_CountChars);
    PDF_FN(FPDFText_GetText);
    PDF_FN(FPDFText_GetCharIndexAtPos);
    PDF_FN(FPDFText_GetCharBox);
    PDF_FN(FPDFText_FindStart);
    PDF_FN(FPDFText_FindNext);
    PDF_FN(FPDFText_FindPrev);
    PDF_FN(FPDFText_GetSchResultIndex);
    PDF_FN(FPDFText_GetSchCount);
    PDF_FN(FPDFText_FindClose);
#undef PDF_FN
    bool Load()
    {
        if (ready)
            return true;
        if (!module)
            module = SafeLoad(L"pdfium.dll");
        if (!module)
            return false;
#define LOAD_PDF(name)                                                                                                 \
    name = Symbol<decltype(name)>(module, #name);                                                                      \
    if (!name)                                                                                                         \
    return false
        LOAD_PDF(FPDF_InitLibrary);
        LOAD_PDF(FPDF_LoadMemDocument64);
        LOAD_PDF(FPDF_CloseDocument);
        LOAD_PDF(FPDF_GetLastError);
        LOAD_PDF(FPDF_GetPageCount);
        LOAD_PDF(FPDF_LoadPage);
        LOAD_PDF(FPDF_ClosePage);
        LOAD_PDF(FPDF_GetPageWidth);
        LOAD_PDF(FPDF_GetPageHeight);
        LOAD_PDF(FPDFBitmap_Create);
        LOAD_PDF(FPDFBitmap_Destroy);
        LOAD_PDF(FPDFBitmap_FillRect);
        LOAD_PDF(FPDFBitmap_GetBuffer);
        LOAD_PDF(FPDFBitmap_GetStride);
        LOAD_PDF(FPDF_RenderPageBitmap);
        LOAD_PDF(FPDF_RenderPageBitmap_Start);
        LOAD_PDF(FPDF_RenderPage_Continue);
        LOAD_PDF(FPDF_RenderPage_Close);
        LOAD_PDF(FPDFText_LoadPage);
        LOAD_PDF(FPDFText_ClosePage);
        LOAD_PDF(FPDFText_CountChars);
        LOAD_PDF(FPDFText_GetText);
        LOAD_PDF(FPDFText_GetCharIndexAtPos);
        LOAD_PDF(FPDFText_GetCharBox);
        LOAD_PDF(FPDFText_FindStart);
        LOAD_PDF(FPDFText_FindNext);
        LOAD_PDF(FPDFText_FindPrev);
        LOAD_PDF(FPDFText_GetSchResultIndex);
        LOAD_PDF(FPDFText_GetSchCount);
        LOAD_PDF(FPDFText_FindClose);
#undef LOAD_PDF
        FPDF_InitLibrary();
        ready = true;
        return true;
    }
};
PdfRuntime &PdfEngine()
{
    static PdfRuntime engine;
    return engine;
}
} // namespace

struct CodeViewer::Impl
{
    HWND window = nullptr;
    bool safe = false;
    ViewerState state;
    std::wstring language;
    LRESULT Send(UINT message, WPARAM w = 0, LPARAM l = 0) const
    {
        return window ? SendMessageW(window, message, w, l) : 0;
    }
    void Zoom(int percent)
    {
        state.zoom = std::clamp(percent, 25, 400);
        Send(SCI_SETZOOM, 0);
        const int size = 1400 * state.zoom / 100;
        for (int style = 0; style <= STYLE_MAX; ++style)
            Send(SCI_STYLESETSIZEFRACTIONAL, style, size);
        const int width =
            static_cast<int>(Send(SCI_TEXTWIDTH, STYLE_LINENUMBER, reinterpret_cast<LPARAM>("99999"))) + 14;
        Send(SCI_SETMARGINWIDTHN, 0, width);
    }
    void Style(int id, COLORREF color, bool bold = false)
    {
        Send(SCI_STYLESETFORE, id, color);
        Send(SCI_STYLESETBOLD, id, bold);
    }
    std::string activeLexer="null";
    void Theme(const char *lexer)
    {
        Send(SCI_STYLESETFORE, STYLE_DEFAULT, Foreground);
        Send(SCI_STYLESETBACK, STYLE_DEFAULT, Background);
        Send(SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>("Consolas"));
        Send(SCI_STYLECLEARALL);
        Send(SCI_SETCARETFORE, Foreground);
        Send(SCI_SETSELBACK, TRUE, theme.selection);
        Send(SCI_SETSELFORE, TRUE, Foreground);
        Send(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
        Send(SCI_SETMARGINWIDTHN, 0, 52);
        Style(STYLE_LINENUMBER, Muted);
        Send(SCI_STYLESETBACK, STYLE_LINENUMBER, Background);
        Send(SCI_SETMARGINWIDTHN, 1, 0);
        Send(SCI_SETMARGINWIDTHN, 2, 0);
        Send(SCI_SETMARGINLEFT, 0, 10);
        Send(SCI_SETMARGINRIGHT, 0, 10);
        Send(SCI_SETEXTRAASCENT, 2);
        Send(SCI_SETEXTRADESCENT, 2);
        Send(SCI_SETWHITESPACESIZE, 1);
        Send(SCI_SETWHITESPACEFORE, TRUE, theme.border);
        if (!strcmp(lexer, "python"))
        {
            Style(SCE_P_COMMENTLINE, Comment);
            Style(SCE_P_COMMENTBLOCK, Comment);
            Style(SCE_P_WORD, Keyword, true);
            Style(SCE_P_WORD2, Keyword);
            Style(SCE_P_STRING, String);
            Style(SCE_P_CHARACTER, String);
            Style(SCE_P_TRIPLE, String);
            Style(SCE_P_TRIPLEDOUBLE, String);
            Style(SCE_P_NUMBER, Number);
            Style(SCE_P_DEFNAME, Accent);
            Style(SCE_P_CLASSNAME, Number);
            Style(SCE_P_OPERATOR, Foreground);
        }
        else if (!strcmp(lexer, "cpp"))
        {
            for (int s : {SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC})
                Style(s, Comment);
            Style(SCE_C_WORD, Keyword, true);
            Style(SCE_C_WORD2, Number);
            Style(SCE_C_STRING, String);
            Style(SCE_C_CHARACTER, String);
            Style(SCE_C_STRINGRAW, String);
            Style(SCE_C_NUMBER, Number);
            Style(SCE_C_PREPROCESSOR, Accent);
            Style(SCE_C_OPERATOR, Foreground);
            Style(SCE_C_REGEX, String);
        }
        else if (!strcmp(lexer, "json"))
        {
            Style(SCE_JSON_PROPERTYNAME, Accent);
            Style(SCE_JSON_STRING, String);
            Style(SCE_JSON_NUMBER, Number);
            Style(SCE_JSON_KEYWORD, Keyword, true);
            Style(SCE_JSON_LINECOMMENT, Comment);
            Style(SCE_JSON_BLOCKCOMMENT, Comment);
            Style(SCE_JSON_ERROR, theme.error);
        }
        else if (!strcmp(lexer, "hypertext") || !strcmp(lexer, "xml"))
        {
            Style(SCE_H_TAG, Keyword, true);
            Style(SCE_H_TAGUNKNOWN, Keyword);
            Style(SCE_H_ATTRIBUTE, Accent);
            Style(SCE_H_DOUBLESTRING, String);
            Style(SCE_H_SINGLESTRING, String);
            Style(SCE_H_COMMENT, Comment);
            Style(SCE_H_NUMBER, Number);
            Style(SCE_HJ_KEYWORD, Keyword);
            Style(SCE_HJ_WORD, Keyword);
            Style(SCE_HJ_DOUBLESTRING, String);
            Style(SCE_HJ_SINGLESTRING, String);
            Style(SCE_HJ_COMMENT, Comment);
            Style(SCE_HJ_COMMENTLINE, Comment);
            Style(SCE_HJ_NUMBER, Number);
        }
        else if (!strcmp(lexer, "css"))
        {
            Style(SCE_CSS_COMMENT, Comment);
            Style(SCE_CSS_DOUBLESTRING, String);
            Style(SCE_CSS_SINGLESTRING, String);
            Style(SCE_CSS_IDENTIFIER, Accent);
            Style(SCE_CSS_IDENTIFIER2, Accent);
            Style(SCE_CSS_TAG, Keyword);
            Style(SCE_CSS_CLASS, Number);
            Style(SCE_CSS_ID, Number);
            Style(SCE_CSS_VALUE, String);
            Style(SCE_CSS_DIRECTIVE, Keyword);
        }
        else if (!strcmp(lexer, "yaml"))
        {
            Style(SCE_YAML_COMMENT, Comment);
            Style(SCE_YAML_IDENTIFIER, Accent);
            Style(SCE_YAML_KEYWORD, Keyword);
            Style(SCE_YAML_NUMBER, Number);
            Style(SCE_YAML_TEXT, String);
        }
        else if (!strcmp(lexer, "markdown"))
        {
            Style(SCE_MARKDOWN_STRONG1, Foreground, true);
            Style(SCE_MARKDOWN_STRONG2, Foreground, true);
            Style(SCE_MARKDOWN_CODE, String);
            Style(SCE_MARKDOWN_CODE2, String);
            Style(SCE_MARKDOWN_CODEBK, String);
            Style(SCE_MARKDOWN_LINK, Accent);
            for (int s = SCE_MARKDOWN_HEADER1; s <= SCE_MARKDOWN_HEADER6; ++s)
                Style(s, Keyword, true);
        }
        else if (!strcmp(lexer, "sql"))
        {
            Style(SCE_SQL_WORD, Keyword, true);
            Style(SCE_SQL_WORD2, Accent);
            Style(SCE_SQL_STRING, String);
            Style(SCE_SQL_CHARACTER, String);
            Style(SCE_SQL_COMMENT, Comment);
            Style(SCE_SQL_COMMENTLINE, Comment);
            Style(SCE_SQL_NUMBER, Number);
        }
        else if (!strcmp(lexer, "bash"))
        {
            Style(SCE_SH_COMMENTLINE, Comment);
            Style(SCE_SH_WORD, Keyword, true);
            Style(SCE_SH_STRING, String);
            Style(SCE_SH_CHARACTER, String);
            Style(SCE_SH_SCALAR, Accent);
            Style(SCE_SH_NUMBER, Number);
        }
        else if (!strcmp(lexer, "powershell"))
        {
            Style(SCE_POWERSHELL_COMMENT, Comment);
            Style(SCE_POWERSHELL_COMMENTSTREAM, Comment);
            Style(SCE_POWERSHELL_KEYWORD, Keyword, true);
            Style(SCE_POWERSHELL_STRING, String);
            Style(SCE_POWERSHELL_CHARACTER, String);
            Style(SCE_POWERSHELL_VARIABLE, Accent);
            Style(SCE_POWERSHELL_NUMBER, Number);
            Style(SCE_POWERSHELL_CMDLET, Number);
        }
        else if (!strcmp(lexer, "rust"))
        {
            Style(SCE_RUST_COMMENTBLOCK, Comment);
            Style(SCE_RUST_COMMENTLINE, Comment);
            Style(SCE_RUST_WORD, Keyword, true);
            Style(SCE_RUST_STRING, String);
            Style(SCE_RUST_STRINGR, String);
            Style(SCE_RUST_NUMBER, Number);
            Style(SCE_RUST_LIFETIME, Accent);
        }
    }
    static LRESULT CALLBACK Subclass(HWND w, UINT m, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR data)
    {
        auto *p = reinterpret_cast<Impl *>(data);
        if(m==WM_KEYDOWN&&wp==L'S'&&(GetKeyState(VK_CONTROL)&0x8000)){
            SendMessageW(GetParent(w),WM_APP+33,0,0);return 0;
        }
        if (m == WM_MOUSEWHEEL && (GET_KEYSTATE_WPARAM(wp) & MK_CONTROL))
        {
            if (p->safe)
            {
                p->Zoom(p->state.zoom + (GET_WHEEL_DELTA_WPARAM(wp) > 0 ? 10 : -10));
                PostMessageW(GetParent(w), WM_APP + 16, 0, 0);
            }
            return 0;
        }
        if (!p->safe &&
            (m == WM_COPY || m == WM_PASTE || m == WM_CUT || m == SCI_COPY || m == SCI_CUT || m == SCI_PASTE ||
             m == WM_KEYDOWN || m == WM_CHAR || m == WM_LBUTTONDOWN || m == WM_RBUTTONDOWN))
            return 0;
        return DefSubclassProc(w, m, wp, lp);
    }
};
CodeViewer::CodeViewer() : impl(std::make_unique<Impl>())
{
}
CodeViewer::~CodeViewer()
{
    if (impl->window)
        DestroyWindow(impl->window);
}
bool CodeViewer::Create(HWND parent, int id, std::wstring &error)
{
    if (impl->window)
        return true;
    if (!CodeEngine().Load())
    {
        error = Error(L"Thiếu thư viện xem code. Giải nén lại gói đầy đủ.",
                      L"Code libraries are missing. Extract the complete package again.");
        return false;
    }
    impl->window =
        CreateWindowExW(0, L"Scintilla", L"", WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL, 0, 0, 100, 100, parent,
                        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    if (!impl->window)
        return false;
    SetWindowTheme(impl->window, theme.dark?L"DarkMode_Explorer":L"Explorer", nullptr);
    SetWindowSubclass(impl->window, Impl::Subclass, 1, reinterpret_cast<DWORD_PTR>(impl.get()));
    impl->Send(SCI_SETTECHNOLOGY, SC_TECHNOLOGY_DIRECTWRITE);
    impl->Send(SCI_SETFONTQUALITY, SC_EFF_QUALITY_LCD_OPTIMIZED);
    impl->Send(SCI_SETCODEPAGE, SC_CP_UTF8);
    impl->Send(SCI_SETREADONLY, TRUE);
    impl->Send(SCI_SETTABWIDTH, 4);
    impl->Send(SCI_SETINDENT, 4);
    impl->Send(SCI_SETHSCROLLBAR, TRUE);
    impl->Send(SCI_SETSCROLLWIDTHTRACKING, TRUE);
    impl->Send(SCI_USEPOPUP, SC_POPUP_NEVER);
    return true;
}
HWND CodeViewer::Window() const
{
    return impl->window;
}
bool CodeViewer::Open(const LoadedContent &content, std::wstring &error)
{
    if (!impl->window || content.text.size() > 5u * 1024 * 1024)
    {
        error = Error(L"Không thể mở code.", L"Unable to open code.");
        return false;
    }
    const auto text = Utf8(content.text);
    if (text.size() > 5u * 1024 * 1024)
    {
        error = Error(L"Code vượt giới hạn 5 MiB.", L"Code exceeds the 5 MiB limit.");
        return false;
    }
    const auto lexer = LanguageFor(content.displayName);
    auto *object = CodeEngine().create(lexer.name);
    if (!object)
    {
        error = Error(L"Không tạo được lexer.", L"Unable to create lexer.");
        return false;
    }
    impl->Send(SCI_SETILEXER, 0, reinterpret_cast<LPARAM>(object));
    impl->Send(SCI_SETKEYWORDS, 0, reinterpret_cast<LPARAM>(lexer.keywords));
    if (!strcmp(lexer.name, "cpp"))
        impl->Send(
            SCI_SETKEYWORDS, 1,
            reinterpret_cast<LPARAM>(
                "string string_view vector array map set unordered_map unordered_set optional unique_ptr shared_ptr "
                "size_t uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t String System Console "
                "Object Array Promise Map Set Date RegExp Error Number Boolean JSON Math"));
    impl->Send(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.cpp.track.preprocessor"),
               reinterpret_cast<LPARAM>("0"));
    impl->Send(SCI_SETPROPERTY, reinterpret_cast<WPARAM>("lexer.json.allow.comments"), reinterpret_cast<LPARAM>("1"));
    impl->activeLexer=lexer.name;
    impl->Theme(lexer.name);
    impl->Send(SCI_SETREADONLY, FALSE);
    impl->Send(SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
    impl->Send(SCI_EMPTYUNDOBUFFER);
    impl->Send(SCI_SETSAVEPOINT);
    impl->Send(SCI_SETREADONLY, TRUE);
    impl->Send(SCI_COLOURISE, 0, -1);
    impl->language = lexer.label;
    impl->state = {};
    SetZoom(DefaultZoomPercent);
    SetWrap(false);
    error.clear();
    return true;
}
void CodeViewer::RefreshTheme()
{
    if(!impl->window)return;
    impl->Theme(impl->activeLexer.c_str());
    impl->Zoom(impl->state.zoom);
    SetWindowTheme(impl->window,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
    impl->Send(SCI_COLOURISE,0,-1);
    InvalidateRect(impl->window,nullptr,FALSE);
}
void CodeViewer::Close()
{
    impl->Send(SCI_SETREADONLY, FALSE);
    impl->Send(SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(""));
    impl->Send(SCI_EMPTYUNDOBUFFER);
    impl->Send(SCI_SETREADONLY, TRUE);
    impl->language.clear();
}
void CodeViewer::SetPrivateVisible(bool safe)
{
    impl->safe = safe;
    if (impl->window)
        ShowWindow(impl->window, safe ? SW_SHOW : SW_HIDE);
}
ViewerState CodeViewer::GetState() const
{
    auto state = impl->state;
    state.scrollY = static_cast<int>(impl->Send(SCI_GETFIRSTVISIBLELINE));
    state.scrollX = static_cast<int>(impl->Send(SCI_GETXOFFSET));
    return state;
}
void CodeViewer::SetState(const ViewerState &state)
{
    SetZoom(state.zoom);
    SetWrap(state.wrap);
    impl->Send(SCI_SETFIRSTVISIBLELINE, std::max(0, state.scrollY));
    impl->Send(SCI_SETXOFFSET, std::max(0, state.scrollX));
}
void CodeViewer::SetZoom(int percent)
{
    impl->Zoom(percent);
}
void CodeViewer::SetWrap(bool enabled)
{
    impl->state.wrap = enabled;
    impl->Send(SCI_SETWRAPMODE, enabled ? SC_WRAP_WORD : SC_WRAP_NONE);
}
void CodeViewer::SetEditable(bool enabled){impl->Send(SCI_SETREADONLY,!enabled);}
bool CodeViewer::Modified()const{return impl->Send(SCI_GETMODIFY)!=0;}
void CodeViewer::MarkSaved(){impl->Send(SCI_SETSAVEPOINT);}
bool CodeViewer::ReadText(std::wstring& result)const {
    const auto length=impl->Send(SCI_GETLENGTH);
    if(length<0||length>5*1024*1024)return false;
    std::string bytes(static_cast<size_t>(length)+1,'\0');impl->Send(SCI_GETTEXT,bytes.size(),reinterpret_cast<LPARAM>(bytes.data()));
    const int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(length),nullptr,0);
    if(length&&!count)return false;
    std::wstring text(count,L'\0');if(count)MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(length),text.data(),count);
    result=std::move(text);return true;
}
bool CodeViewer::Find(const std::wstring &query, bool forward)
{
    if (!impl->safe || query.empty())
        return false;
    const auto text = Utf8(query);
    const auto length = impl->Send(SCI_GETLENGTH);
    const auto start = impl->Send(forward ? SCI_GETSELECTIONEND : SCI_GETSELECTIONSTART);
    impl->Send(SCI_SETSEARCHFLAGS, 0);
    impl->Send(SCI_SETTARGETSTART, start);
    impl->Send(SCI_SETTARGETEND, forward ? length : 0);
    auto pos = impl->Send(SCI_SEARCHINTARGET, text.size(), reinterpret_cast<LPARAM>(text.data()));
    if (pos < 0)
    {
        impl->Send(SCI_SETTARGETSTART, forward ? 0 : length);
        impl->Send(SCI_SETTARGETEND, start);
        pos = impl->Send(SCI_SEARCHINTARGET, text.size(), reinterpret_cast<LPARAM>(text.data()));
    }
    if (pos < 0)
        return false;
    impl->Send(SCI_SETSEL, pos, pos + text.size());
    impl->Send(SCI_SCROLLCARET);
    return true;
}
int CodeViewer::TokenStyleAt(int byteOffset) const
{
    return static_cast<int>(impl->Send(SCI_GETSTYLEAT, byteOffset));
}
std::wstring CodeViewer::Language() const
{
    return impl->language;
}

struct PdfViewer::Impl
{
    HWND window = nullptr;
    bool safe = false, dragging = false;
    FPDF_DOCUMENT document = nullptr;
    FPDF_PAGE page = nullptr;
    FPDF_TEXTPAGE text = nullptr;
    FPDF_BITMAP bitmap = nullptr;
    std::vector<unsigned char> bytes;
    ViewerState state;
    int pages = 0, chars = 0, renderWidth = 0, renderHeight = 0;
    double pageWidth = 0, pageHeight = 0;
    int renderedZoom = -1, selectStart = -1, selectEnd = -1;
    POINT dragStart{};
    int dragPanX = 0, dragPanY = 0;
    bool selecting = false, fitWidth = false, hasText = false;
    bool rendering = false;
    unsigned operationDepth = 0;
    bool deferredClose = false;
    ULONGLONG renderDeadline = 0;
    ULONGLONG lastOwnerCheck = 0;
    std::wstring renderError;
    ~Impl()
    {
        Close();
    }
    struct Operation
    {
        Impl &owner;
        explicit Operation(Impl &value) : owner(value)
        {
            ++owner.operationDepth;
        }
        ~Operation()
        {
            if (--owner.operationDepth == 0 && owner.deferredClose)
                owner.CloseNow();
        }
    };
    PdfRuntime &API()
    {
        return PdfEngine();
    }
    void SafetyMessages()
    {
        if (!window)
            return;
        const HWND owner = GetParent(window);
        if (!owner)
            return;
        MSG message{};
        // Pull only posted safety messages. PeekMessage may also dispatch sent
        // messages of any type; Operation defers close and rejects replacement.
        for (UINT type : {WM_HOTKEY, WM_DISPLAYCHANGE, WM_DWMCOMPOSITIONCHANGED, WM_DEVICECHANGE, WM_POWERBROADCAST})
            while (PeekMessageW(&message, owner, type, type, PM_REMOVE))
                DispatchMessageW(&message);
        // The app owns the common privacy gate. Its private protocol performs a
        // fresh check without dispatching autosave/scroll/QA timers or documents.
        const ULONGLONG now = GetTickCount64();
        if (!lastOwnerCheck || now - lastOwnerCheck >= 16)
        {
            lastOwnerCheck = now;
            if (SendMessageW(owner, WM_APP + 14, 0, 0) != 1)
                Mask();
        }
        // The owner identifies its current emergency binding. Only that key is
        // pulled during a render; unrelated input stays in the normal queue.
        while (PeekMessageW(&message, owner, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE) &&
               SendMessageW(owner,WM_APP+30,message.wParam,0)==1)
        {
            PeekMessageW(&message, owner, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE);
            if (!(message.lParam & (1u << 30)))
                SendMessageW(owner, WM_KEYDOWN, message.wParam, message.lParam);
        }
    }
    static FPDF_BOOL NeedPause(IFSDK_PAUSE *pause)
    {
        const auto self = static_cast<Impl *>(pause->user);
        return !self->safe || GetTickCount64() >= self->renderDeadline;
    }
    void DiscardBitmap()
    {
        if (bitmap)
            API().FPDFBitmap_Destroy(bitmap);
        bitmap = nullptr;
        renderWidth = renderHeight = 0;
        renderedZoom = -1;
    }
    void Mask()
    {
        safe = false;
        if (GetCapture() == window)
            ReleaseCapture();
        dragging = selecting = false;
        if (window)
            ShowWindow(window, SW_HIDE);
        // An active progressive context must be closed before its bitmap is freed.
        if (!rendering)
            DiscardBitmap();
    }
    void ReleasePage()
    {
        if (bitmap)
            API().FPDFBitmap_Destroy(bitmap);
        bitmap = nullptr;
        renderWidth = renderHeight = 0;
        renderedZoom = -1;
        if (text)
            API().FPDFText_ClosePage(text);
        text = nullptr;
        if (page)
            API().FPDF_ClosePage(page);
        page = nullptr;
        chars = 0;
        selectStart = selectEnd = -1;
    }
    void Close()
    {
        Mask();
        if (operationDepth)
        {
            deferredClose = true;
            return;
        }
        CloseNow();
    }
    void CloseNow()
    {
        ReleasePage();
        if (document)
            API().FPDF_CloseDocument(document);
        document = nullptr;
        bytes.clear();
        bytes.shrink_to_fit();
        pages = 0;
        state = {};
        fitWidth = false;
        hasText = false;
        lastOwnerCheck = 0;
        deferredClose = false;
    }
    bool LoadPage(int n)
    {
        if (!document || n < 0 || n >= pages)
            return false;
        ReleasePage();
        page = API().FPDF_LoadPage(document, n);
        if (!page)
            return false;
        pageWidth = API().FPDF_GetPageWidth(page);
        pageHeight = API().FPDF_GetPageHeight(page);
        if (!std::isfinite(pageWidth) || !std::isfinite(pageHeight) || pageWidth <= 0 || pageHeight <= 0 ||
            pageWidth > 144000 || pageHeight > 144000)
        {
            ReleasePage();
            return false;
        }
        text = API().FPDFText_LoadPage(page);
        chars = text ? API().FPDFText_CountChars(text) : 0;
        hasText = hasText || chars > 0;
        state.page = n;
        return true;
    }
    double Scale() const
    {
        RECT rc{};
        if (window)
            GetClientRect(window, &rc);
        const double width = std::max(1L, rc.right - 24L), height = std::max(1L, rc.bottom - 40L);
        const double fit = fitWidth ? width / pageWidth : std::min(width / pageWidth, height / pageHeight);
        return fit * state.zoom / 100.0;
    }
    bool Render()
    {
        if (operationDepth)
            return false;
        Operation operation(*this);
        SafetyMessages();
        if (!safe || !page)
            return false;
        const double scale = Scale();
        const double proposedW = std::max(1.0, std::round(pageWidth * scale)),
                     proposedH = std::max(1.0, std::round(pageHeight * scale));
        if (proposedW > 16384 || proposedH > 16384 || proposedW * proposedH > 16.0 * 1000 * 1000)
        {
            renderError =
                Error(L"Trang vượt giới hạn render. Giảm zoom.", L"Page exceeds rendering limits. Reduce zoom.");
            return false;
        }
        const int width = static_cast<int>(proposedW), height = static_cast<int>(proposedH);
        if (bitmap && renderWidth == width && renderHeight == height && renderedZoom == state.zoom)
            return true;
        if (bitmap)
            API().FPDFBitmap_Destroy(bitmap);
        bitmap = nullptr;
        renderWidth = renderHeight = 0;
        bitmap = API().FPDFBitmap_Create(width, height, 0);
        if (!bitmap)
            return false;
        renderWidth = width;
        renderHeight = height;
        renderedZoom = state.zoom;
        API().FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xffffffff);
        IFSDK_PAUSE pause{1, NeedPause, this};
        rendering = true;
        renderError.clear();
        // PDFium pauses cooperatively at its own rendering checkpoints.
        const ULONGLONG aggregateDeadline = GetTickCount64() + 750;
        renderDeadline = std::min(aggregateDeadline, GetTickCount64() + 16);
        int status = API().FPDF_RenderPageBitmap_Start(bitmap, page, 0, 0, width, height, 0, FPDF_ANNOT, &pause);
        while (status == FPDF_RENDER_TOBECONTINUED && safe && GetTickCount64() < aggregateDeadline)
        {
            SafetyMessages();
            if (!safe)
                break;
            renderDeadline = std::min(aggregateDeadline, GetTickCount64() + 16);
            status = API().FPDF_RenderPage_Continue(page, &pause);
        }
        API().FPDF_RenderPage_Close(page);
        rendering = false;
        SafetyMessages();
        const bool expired = GetTickCount64() >= aggregateDeadline;
        if (expired)
            renderError = Error(L"Trang quá phức tạp để render nhanh. Giảm zoom hoặc chuyển trang.",
                                L"Page rendering exceeded the time budget. Reduce zoom or change page.");
        if (!safe || status != FPDF_RENDER_DONE || expired)
        {
            DiscardBitmap();
            return false;
        }
        return true;
    }
    RECT PageRect() const
    {
        RECT rc{};
        GetClientRect(window, &rc);
        const int x = (rc.right - renderWidth) / 2 + state.panX;
        const int y = std::max(12L, (rc.bottom - 20 - renderHeight) / 2) + state.panY;
        return {x, y, x + renderWidth, y + renderHeight};
    }
    int CharAt(int x, int y)
    {
        if (!safe || !text || !bitmap)
            return -1;
        const auto r = PageRect();
        const double px = (x - r.left) * pageWidth / renderWidth,
                     py = pageHeight - (y - r.top) * pageHeight / renderHeight;
        return API().FPDFText_GetCharIndexAtPos(text, px, py, 12 / Scale(), 12 / Scale());
    }
    void Paint(HDC dc)
    {
        RECT rc{};
        GetClientRect(window, &rc);
        HBRUSH brush = CreateSolidBrush(Background);
        FillRect(dc, &rc, brush);
        DeleteObject(brush);
        if (!safe || !document)
            return;
        if (!Render())
        {
            if (!safe)
                return;
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, Muted);
            const wchar_t *msg =
                renderError.empty() ? L"Không thể render trang. / Unable to render page." : renderError.c_str();
            DrawTextW(dc, msg, -1, &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
            return;
        }
        const auto r = PageRect();
        BITMAPINFO info{};
        info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = renderWidth;
        info.bmiHeader.biHeight = -renderHeight;
        info.bmiHeader.biPlanes = 1;
        info.bmiHeader.biBitCount = 32;
        info.bmiHeader.biCompression = BI_RGB;
        StretchDIBits(dc, r.left, r.top, renderWidth, renderHeight, 0, 0, renderWidth, renderHeight,
                      API().FPDFBitmap_GetBuffer(bitmap), &info, DIB_RGB_COLORS, SRCCOPY);
        if (text && selectStart >= 0 && selectEnd >= 0)
        {
            const int start = std::min(selectStart, selectEnd),
                      end = std::min(chars - 1, std::max(selectStart, selectEnd));
            HBRUSH highlight = CreateSolidBrush(Accent);
            for (int i = start; i <= end && i < start + 100000; ++i)
            {
                double l = 0, rt = 0, b = 0, t = 0;
                if (API().FPDFText_GetCharBox(text, i, &l, &rt, &b, &t))
                {
                    RECT box{r.left + static_cast<LONG>(l * renderWidth / pageWidth),
                             r.top + static_cast<LONG>((pageHeight - t) * renderHeight / pageHeight),
                             r.left + static_cast<LONG>(rt * renderWidth / pageWidth) + 1,
                             r.top + static_cast<LONG>((pageHeight - b) * renderHeight / pageHeight) + 1};
                    FrameRect(dc, &box, highlight);
                }
            }
            DeleteObject(highlight);
        }
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, Muted);
        auto label = std::to_wstring(state.page + 1) + L" / " + std::to_wstring(pages) + L"  ·  " +
                     std::to_wstring(state.zoom) + L"%";
        RECT footer{0, rc.bottom - 20, rc.right, rc.bottom};
        DrawTextW(dc, label.c_str(), -1, &footer, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    }
    static LRESULT CALLBACK Proc(HWND w, UINT m, WPARAM wp, LPARAM lp)
    {
        Impl *p = reinterpret_cast<Impl *>(GetWindowLongPtrW(w, GWLP_USERDATA));
        if (m == WM_NCCREATE)
        {
            p = static_cast<Impl *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
            p->window = w;
            SetWindowLongPtrW(w, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p));
        }
        if (!p)
            return DefWindowProcW(w, m, wp, lp);
        switch (m)
        {
        case WM_ERASEBKGND:
            return 1;
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT)
            {
                POINT point{};
                GetCursorPos(&point);
                ScreenToClient(w, &point);
                SetCursor(LoadCursorW(nullptr, p->CharAt(point.x, point.y) >= 0 ? IDC_IBEAM : IDC_ARROW));
                return TRUE;
            }
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC dc = BeginPaint(w, &ps);
            RECT rc{};
            GetClientRect(w, &rc);
            HDC buffer = CreateCompatibleDC(dc);
            HBITMAP bm = CreateCompatibleBitmap(dc, std::max(1L, rc.right), std::max(1L, rc.bottom));
            HGDIOBJ old = SelectObject(buffer, bm);
            p->Paint(buffer);
            BitBlt(dc, 0, 0, rc.right, rc.bottom, buffer, 0, 0, SRCCOPY);
            SelectObject(buffer, old);
            DeleteObject(bm);
            DeleteDC(buffer);
            EndPaint(w, &ps);
            return 0;
        }
        case WM_SIZE:
            p->renderedZoom = -1;
            InvalidateRect(w, nullptr, FALSE);
            return 0;
        case WM_LBUTTONDOWN:
            if (!p->safe)
                return 0;
            SetFocus(w);
            p->selectStart = p->CharAt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            p->selectEnd = p->selectStart;
            p->selecting = p->selectStart >= 0;
            p->dragging = !p->selecting;
            p->dragStart = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            p->dragPanX = p->state.panX;
            p->dragPanY = p->state.panY;
            SetCapture(w);
            InvalidateRect(w, nullptr, FALSE);
            return 0;
        case WM_MOUSEMOVE:
            if (!p->safe)
                return 0;
            if (p->selecting)
            {
                const int n = p->CharAt(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
                if (n >= 0)
                    p->selectEnd = n;
                InvalidateRect(w, nullptr, FALSE);
            }
            else if (p->dragging)
            {
                p->state.panX = p->dragPanX + GET_X_LPARAM(lp) - p->dragStart.x;
                p->state.panY = p->dragPanY + GET_Y_LPARAM(lp) - p->dragStart.y;
                InvalidateRect(w, nullptr, FALSE);
            }
            return 0;
        case WM_LBUTTONUP:
            if (p->dragging || p->selecting)
                ReleaseCapture();
            p->dragging = p->selecting = false;
            return 0;
        case WM_CAPTURECHANGED:
            p->dragging = p->selecting = false;
            return 0;
        case WM_MOUSEWHEEL:
            if (!p->safe)
                return 0;
            if (GET_KEYSTATE_WPARAM(wp) & MK_CONTROL)
            {
                p->state.zoom = std::clamp(p->state.zoom + (GET_WHEEL_DELTA_WPARAM(wp) > 0 ? 10 : -10), 25, 400);
                p->state.panX = p->state.panY = 0;
                PostMessageW(GetParent(w), WM_APP + 16, 0, 0);
            }
            else
                p->state.panY += GET_WHEEL_DELTA_WPARAM(wp) / 2;
            InvalidateRect(w, nullptr, FALSE);
            return 0;
        case WM_KEYDOWN:
            if (!p->safe)
                return 0;
            if (wp == VK_NEXT || wp == VK_RIGHT)
            {
                if (p->LoadPage(std::min(p->pages - 1, p->state.page + 1)))
                    p->state.panX = p->state.panY = 0;
            }
            else if (wp == VK_PRIOR || wp == VK_LEFT)
            {
                if (p->LoadPage(std::max(0, p->state.page - 1)))
                    p->state.panX = p->state.panY = 0;
            }
            else if (wp == VK_UP)
                p->state.panY += 40;
            else if (wp == VK_DOWN)
                p->state.panY -= 40;
            else if (wp == VK_HOME)
            {
                p->state.zoom = DefaultZoomPercent;
                p->state.panX = p->state.panY = 0;
            }
            else
                return DefWindowProcW(w, m, wp, lp);
            InvalidateRect(w, nullptr, FALSE);
            return 0;
        case WM_NCDESTROY:
            SetWindowLongPtrW(w, GWLP_USERDATA, 0);
            p->window = nullptr;
            break;
        }
        return DefWindowProcW(w, m, wp, lp);
    }
};
PdfViewer::PdfViewer() : impl(std::make_unique<Impl>())
{
}
PdfViewer::~PdfViewer()
{
    if (impl->window)
        DestroyWindow(impl->window);
}
bool PdfViewer::Create(HWND parent, int id, std::wstring &error)
{
    if (impl->window)
        return true;
    if (!PdfEngine().Load())
    {
        error = Error(L"Thiếu thư viện PDF. Giải nén lại gói đầy đủ.",
                      L"PDF library is missing. Extract the complete package again.");
        return false;
    }
    WNDCLASSW wc{};
    wc.lpfnWndProc = Impl::Proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"QuietSharePdfCanvas";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    impl->window = CreateWindowExW(0, wc.lpszClassName, L"", WS_CHILD | WS_TABSTOP, 0, 0, 100, 100, parent,
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), wc.hInstance, impl.get());
    return impl->window != nullptr;
}
HWND PdfViewer::Window() const
{
    return impl->window;
}
bool PdfViewer::Open(const LoadedContent &content, std::wstring &error)
{
    if (impl->operationDepth)
    {
        error = Error(L"Trình xem PDF đang xử lý. Thử lại sau.", L"PDF viewer is busy. Try again shortly.");
        return false;
    }
    if (!impl->window || !PdfEngine().ready)
    {
        error = Error(L"Chưa khởi tạo trình xem PDF.", L"PDF viewer is not initialized.");
        return false;
    }
    if (content.bytes.empty() || content.bytes.size() > 64u * 1024 * 1024)
    {
        error = Error(L"PDF rỗng hoặc vượt 64 MiB.", L"PDF is empty or exceeds 64 MiB.");
        return false;
    }
    auto bytes = content.bytes;
    auto doc = PdfEngine().FPDF_LoadMemDocument64(bytes.data(), bytes.size(), nullptr);
    if (!doc)
    {
        const auto code = PdfEngine().FPDF_GetLastError();
        error = code == FPDF_ERR_PASSWORD
                    ? Error(L"PDF có mật khẩu chưa được hỗ trợ.", L"Password-protected PDF is not supported.")
                    : Error(L"PDF bị hỏng hoặc không được hỗ trợ.", L"PDF is corrupt or unsupported.");
        return false;
    }
    const int pages = PdfEngine().FPDF_GetPageCount(doc);
    if (pages < 1 || pages > 10000)
    {
        PdfEngine().FPDF_CloseDocument(doc);
        error = Error(L"PDF phải có 1–10000 trang.", L"PDF must contain 1–10000 pages.");
        return false;
    }
    auto page = PdfEngine().FPDF_LoadPage(doc, 0);
    const double width = page ? PdfEngine().FPDF_GetPageWidth(page) : 0;
    const double height = page ? PdfEngine().FPDF_GetPageHeight(page) : 0;
    if (!page || !std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0 || width > 144000 ||
        height > 144000)
    {
        if (page)
            PdfEngine().FPDF_ClosePage(page);
        PdfEngine().FPDF_CloseDocument(doc);
        error = Error(L"Không thể đọc trang PDF.", L"Unable to read PDF page.");
        return false;
    }
    const auto text = PdfEngine().FPDFText_LoadPage(page);
    impl->CloseNow();
    impl->bytes = std::move(bytes);
    impl->document = doc;
    impl->pages = pages;
    impl->state = {};
    impl->fitWidth = false;
    impl->page = page;
    impl->pageWidth = width;
    impl->pageHeight = height;
    impl->text = text;
    impl->chars = text ? PdfEngine().FPDFText_CountChars(text) : 0;
    impl->hasText = impl->chars > 0;
    error.clear();
    if (impl->window)
        InvalidateRect(impl->window, nullptr, FALSE);
    return true;
}
void PdfViewer::Close()
{
    impl->Close();
    if (impl->window)
        InvalidateRect(impl->window, nullptr, FALSE);
}
void PdfViewer::SetPrivateVisible(bool safe)
{
    if (!safe)
        impl->Mask();
    else
    {
        if (impl->deferredClose)
            return;
        impl->safe = true;
        if (impl->window)
            ShowWindow(impl->window, SW_SHOW);
    }
}
ViewerState PdfViewer::GetState() const
{
    auto state = impl->state;
    state.fitWidth = impl->fitWidth;
    return state;
}
void PdfViewer::SetState(const ViewerState &state)
{
    if (impl->operationDepth)
        return;
    if (state.page != impl->state.page)
        impl->LoadPage(std::clamp(state.page, 0, std::max(0, impl->pages - 1)));
    impl->state.zoom = std::clamp(state.zoom, 25, 400);
    impl->state.panX = state.panX;
    impl->state.panY = state.panY;
    impl->fitWidth = state.fitWidth;
    if (impl->window)
        InvalidateRect(impl->window, nullptr, FALSE);
}
void PdfViewer::SetZoom(int percent)
{
    if (impl->operationDepth)
        return;
    impl->state.zoom = std::clamp(percent, 25, 400);
    impl->state.panX = impl->state.panY = 0;
    if (impl->window)
        InvalidateRect(impl->window, nullptr, FALSE);
}
void PdfViewer::FitPage()
{
    if (impl->operationDepth)
        return;
    impl->fitWidth = false;
    SetZoom(100);
}
void PdfViewer::FitWidth()
{
    if (impl->operationDepth)
        return;
    impl->fitWidth = true;
    SetZoom(100);
}
bool PdfViewer::GoToPage(int page)
{
    if (impl->operationDepth)
        return false;
    if (!impl->safe || !impl->LoadPage(page))
        return false;
    impl->state.panX = impl->state.panY = 0;
    if (impl->window)
        InvalidateRect(impl->window, nullptr, FALSE);
    return true;
}
int PdfViewer::PageCount() const
{
    return impl->pages;
}
bool PdfViewer::HasText() const
{
    return impl->hasText;
}
bool PdfViewer::Find(const std::wstring &query, bool forward)
{
    if (impl->operationDepth)
        return false;
    Impl::Operation operation(*impl);
    if (!impl->safe || query.empty() || query.size() > 1024 || !impl->document)
        return false;
    const int starting = impl->state.page;
    const auto previous = impl->state;
    const int previousStart = impl->selectStart, previousEnd = impl->selectEnd;
    std::vector<unsigned short> word(query.begin(), query.end());
    word.push_back(0);
    for (int step = 0; step <= impl->pages; ++step)
    {
        impl->SafetyMessages();
        if (!impl->safe)
            return false;
        const int n = (starting + (forward ? step : -step) + impl->pages) % impl->pages;
        if (n != impl->state.page && !impl->LoadPage(n))
            continue;
        if (!impl->text || impl->chars < 1)
            continue;
        const int start = step == 0 ? (forward ? std::max(0, impl->selectEnd + 1)
                                               : (impl->selectStart > 0 ? impl->selectStart - 1 : -1))
                                    : (forward ? 0 : -1);
        auto search = impl->API().FPDFText_FindStart(impl->text, word.data(), 0, start);
        if (!search)
            continue;
        const bool found =
            (forward ? impl->API().FPDFText_FindNext(search) : impl->API().FPDFText_FindPrev(search)) != 0;
        if (found)
        {
            impl->selectStart = impl->API().FPDFText_GetSchResultIndex(search);
            impl->selectEnd = impl->selectStart + impl->API().FPDFText_GetSchCount(search) - 1;
            impl->API().FPDFText_FindClose(search);
            impl->state.panX = impl->state.panY = 0;
            if (impl->window)
                InvalidateRect(impl->window, nullptr, FALSE);
            return true;
        }
        impl->API().FPDFText_FindClose(search);
    }
    if (impl->state.page != starting)
        impl->LoadPage(starting);
    impl->state = previous;
    impl->selectStart = previousStart;
    impl->selectEnd = previousEnd;
    if (impl->window)
        InvalidateRect(impl->window, nullptr, FALSE);
    return false;
}
bool PdfViewer::CopySelection()
{
    const auto selected = SelectedText();
    if (selected.empty())
        return false;
    const size_t size = (selected.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!memory)
        return false;
    void *out = GlobalLock(memory);
    if (!out)
    {
        GlobalFree(memory);
        return false;
    }
    std::memcpy(out, selected.c_str(), size);
    GlobalUnlock(memory);
    if (!OpenClipboard(impl->window))
    {
        GlobalFree(memory);
        return false;
    }
    EmptyClipboard();
    if (!SetClipboardData(CF_UNICODETEXT, memory))
    {
        CloseClipboard();
        GlobalFree(memory);
        return false;
    }
    CloseClipboard();
    return true;
}
std::wstring PdfViewer::SelectedText() const
{
    if (!impl->safe || !impl->text || impl->selectStart < 0 || impl->selectEnd < 0)
        return {};
    const int start = std::min(impl->selectStart, impl->selectEnd);
    const int count = std::min(100000, std::abs(impl->selectEnd - impl->selectStart) + 1);
    std::vector<unsigned short> text(count + 1, 0);
    const int n = PdfEngine().FPDFText_GetText(impl->text, start, count, text.data());
    if (n <= 1 || n > count + 1)
        return {};
    return std::wstring(reinterpret_cast<const wchar_t *>(text.data()), n - 1);
}
size_t PdfViewer::CachedBytes() const
{
    return static_cast<size_t>(impl->renderWidth) * impl->renderHeight * 4;
}
bool PdfViewer::RenderCurrentPage()
{
    return impl->Render();
}
} // namespace quietshare
