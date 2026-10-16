#pragma once

#include "Icons.h"

#include <algorithm>
#include <cwctype>
#include <string>

namespace quietshare
{
// Shared by the native project tree and document tabs. Classification uses only
// the supplied basename; it never opens files or queries shell associations.
enum class FileIconKind
{
    Generic, Folder, FolderOpen, Code, Python, JavaScript, TypeScript, C, Cpp,
    CSharp, Java, Rust, Go, Json, Xml, Markdown, Text, Config, Image, Pdf, Html,
    Stylesheet, Shell, Count
};

inline FileIconKind FileIconForName(const std::wstring &path, bool directory = false, bool expanded = false)
{
    if (directory) return expanded ? FileIconKind::FolderOpen : FileIconKind::Folder;
    const auto separator = path.find_last_of(L"\\/");
    std::wstring name = path.substr(separator == std::wstring::npos ? 0 : separator + 1);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    if (name == L"dockerfile" || name == L"makefile" || name == L"cmakelists.txt" ||
        name == L".gitignore" || name == L".gitattributes" || name == L".editorconfig" ||
        name == L".env" || name.rfind(L".env.", 0) == 0) return FileIconKind::Config;
    if (name == L"readme" || name == L"license" || name == L"licence") return FileIconKind::Text;
    const auto dot = name.find_last_of(L'.');
    const std::wstring extension = dot == std::wstring::npos ? L"" : name.substr(dot);
    if (extension == L".py" || extension == L".pyw") return FileIconKind::Python;
    if (extension == L".js" || extension == L".jsx" || extension == L".mjs" || extension == L".cjs") return FileIconKind::JavaScript;
    if (extension == L".ts" || extension == L".tsx") return FileIconKind::TypeScript;
    if (extension == L".c") return FileIconKind::C;
    if (extension == L".cc" || extension == L".cpp" || extension == L".cxx" || extension == L".h" ||
        extension == L".hh" || extension == L".hpp" || extension == L".hxx") return FileIconKind::Cpp;
    if (extension == L".cs") return FileIconKind::CSharp;
    if (extension == L".java" || extension == L".kt" || extension == L".kts") return FileIconKind::Java;
    if (extension == L".rs") return FileIconKind::Rust;
    if (extension == L".go") return FileIconKind::Go;
    if (extension == L".json" || extension == L".jsonc") return FileIconKind::Json;
    if (extension == L".xml" || extension == L".xaml" || extension == L".csproj" || extension == L".svg") return FileIconKind::Xml;
    if (extension == L".md" || extension == L".markdown") return FileIconKind::Markdown;
    if (extension == L".txt" || extension == L".log") return FileIconKind::Text;
    if (extension == L".ini" || extension == L".cfg" || extension == L".conf" || extension == L".toml" ||
        extension == L".yaml" || extension == L".yml" || extension == L".cmake") return FileIconKind::Config;
    if (extension == L".png" || extension == L".jpg" || extension == L".jpeg" || extension == L".bmp" ||
        extension == L".gif" || extension == L".webp" || extension == L".ico") return FileIconKind::Image;
    if (extension == L".pdf") return FileIconKind::Pdf;
    if (extension == L".html" || extension == L".htm" || extension == L".vue" || extension == L".svelte") return FileIconKind::Html;
    if (extension == L".css" || extension == L".scss" || extension == L".sass" || extension == L".less") return FileIconKind::Stylesheet;
    if (extension == L".sh" || extension == L".bash" || extension == L".ps1" || extension == L".psm1" ||
        extension == L".psd1" || extension == L".bat" || extension == L".cmd") return FileIconKind::Shell;
    if (extension == L".rb" || extension == L".php" || extension == L".swift" || extension == L".lua" ||
        extension == L".r" || extension == L".sql" || extension == L".gradle") return FileIconKind::Code;
    return FileIconKind::Generic;
}

inline void DrawFileTypeIcon(HDC dc, FileIconKind kind, RECT bounds, bool disabled = false)
{
    const int width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
    if (!dc || width <= 0 || height <= 0) return;
    wchar_t glyph = 0xea7b; // Pinned Codicons 0.0.39 file-text.
    COLORREF color = RGB(170, 170, 170);
    const wchar_t *badge = nullptr;
    switch (kind)
    {
    case FileIconKind::Folder: glyph = 0xea83; color = RGB(214, 177, 104); break;
    case FileIconKind::FolderOpen: glyph = 0xeaf7; color = RGB(230, 193, 120); break;
    case FileIconKind::Code: glyph = 0xeae9; color = RGB(144, 190, 225); break;
    case FileIconKind::Python: glyph = 0xeae9; color = RGB(121, 186, 220); badge = L"PY"; break;
    case FileIconKind::JavaScript: glyph = 0xeae9; color = RGB(221, 202, 91); badge = L"JS"; break;
    case FileIconKind::TypeScript: glyph = 0xeae9; color = RGB(113, 177, 225); badge = L"TS"; break;
    case FileIconKind::C: glyph = 0xeae9; color = RGB(129, 183, 222); badge = L"C"; break;
    case FileIconKind::Cpp: glyph = 0xeae9; color = RGB(129, 183, 222); badge = L"C+"; break;
    case FileIconKind::CSharp: glyph = 0xeae9; color = RGB(155, 205, 138); badge = L"C#"; break;
    case FileIconKind::Java: glyph = 0xeae9; color = RGB(222, 160, 111); badge = L"JV"; break;
    case FileIconKind::Rust: glyph = 0xeae9; color = RGB(211, 160, 139); badge = L"RS"; break;
    case FileIconKind::Go: glyph = 0xeae9; color = RGB(111, 202, 214); badge = L"GO"; break;
    case FileIconKind::Json: glyph = 0xeb0f; color = RGB(222, 193, 123); break;
    case FileIconKind::Xml: glyph = 0xeac4; color = RGB(220, 162, 118); break;
    case FileIconKind::Markdown: glyph = 0xeb1d; color = RGB(164, 189, 229); break;
    case FileIconKind::Config: glyph = 0xeb51; color = RGB(177, 183, 191); break;
    case FileIconKind::Image: glyph = 0xeaea; color = RGB(184, 157, 220); break;
    case FileIconKind::Pdf: glyph = 0xeaeb; color = RGB(225, 145, 142); break;
    case FileIconKind::Html: glyph = 0xeac4; color = RGB(223, 153, 116); break;
    case FileIconKind::Stylesheet: glyph = 0xeb5c; color = RGB(180, 154, 224); break;
    case FileIconKind::Shell: glyph = 0xea85; color = RGB(155, 198, 148); break;
    default: break;
    }
    if (disabled) color = RGB(105, 112, 121);
    const int saved = SaveDC(dc);
    IntersectClipRect(dc, bounds.left, bounds.top, bounds.right, bounds.bottom);
    DrawCodicon(dc, glyph, bounds, color, (std::min)(width, height));
    if (badge)
    {
        // Tiny language badges reuse the same Codicon file silhouette. They are
        // native lettering, never emoji or per-file shell/registry extraction.
        RECT label{bounds.left + MulDiv(3, width, 16), bounds.top + MulDiv(8, height, 16),
                   bounds.right, bounds.bottom};
        HBRUSH backing = CreateSolidBrush(RGB(37, 37, 38));
        FillRect(dc, &label, backing);
        DeleteObject(backing);
        HFONT font = CreateFontW(-MulDiv(8, height, 16), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Consolas");
        const auto old = SelectObject(dc, font);
        SetTextColor(dc, color);
        SetBkMode(dc, TRANSPARENT);
        DrawTextW(dc, badge, -1, &label, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(dc, old);
        DeleteObject(font);
    }
    if (saved) RestoreDC(dc, saved);
}
} // namespace quietshare
