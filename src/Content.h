#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <vector>

namespace quietshare
{

enum class ContentKind
{
    Text,
    Code,
    Image,
    Pdf
};

// Local previews own their bitmap. They do not read, write or replace saved notes.
struct LoadedContent
{
    ContentKind kind = ContentKind::Text;
    std::wstring path;
    std::wstring displayName;
    std::wstring text;
    // Retained PDF bytes keep PDFium's memory document alive. Other types leave this empty.
    std::vector<unsigned char> bytes;
    HBITMAP image = nullptr;
    int width = 0;
    int height = 0;

    LoadedContent() = default;
    ~LoadedContent();
    LoadedContent(const LoadedContent &) = delete;
    LoadedContent &operator=(const LoadedContent &) = delete;
    LoadedContent(LoadedContent &&other) noexcept;
    LoadedContent &operator=(LoadedContent &&other) noexcept;
    void Clear() noexcept;
};

// UTF-8 and BOM-marked UTF-16 only. Source code is plain text and never executed.
// Text: 5 MiB. Encoded images: 25 MiB. Decoded images: 40 megapixels.
// PDFs: 64 MiB. Header checked here; structural validation belongs to PDFium.
// Only disk files on local drives are accepted; URLs, UNC, devices and ADS are rejected.
// Failure leaves result unchanged and returns a readable Vietnamese/English error.
bool LoadLocalContent(const std::wstring &path, LoadedContent &result, std::wstring &error);

// Resolves an existing local file/folder through its actual handle for deduplication.
// Directory roots reject reparse points. Does not persist or log the result.
bool CanonicalLocalPath(const std::wstring &path, bool directory, std::wstring &result,
                        std::wstring &error);

// 100% fits the entire image to the viewport; zoom is clamped to 25..400%.
RECT FitImageRect(const RECT &viewport, int imageWidth, int imageHeight, int zoomPercent = 100);
// Paints the background, then the first image frame, clipped to the viewport.
// The bitmap uses WIC premultiplied BGRA, so transparent images blend correctly.
bool DrawImage(HDC dc, const LoadedContent &content, const RECT &viewport, int zoomPercent = 100,
               COLORREF background = RGB(24, 27, 30), int panX = 0, int panY = 0);

} // namespace quietshare
