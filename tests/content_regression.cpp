#include "../src/Content.h"
#include <wincodec.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using quietshare::ContentKind;
using quietshare::LoadedContent;
using quietshare::LoadLocalContent;

namespace
{
std::wstring root;
std::vector<std::pair<std::string, bool>> checks;

void Check(const char *name, bool passed)
{
    checks.emplace_back(name, passed);
    if (!passed)
        std::fprintf(stderr, "FAIL: %s\n", name);
}

std::wstring Path(const wchar_t *name)
{
    return root + L"\\" + name;
}

void Write(const std::wstring &path, const std::vector<BYTE> &bytes)
{
    HANDLE file =
        CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Cannot create isolated fixture");
    DWORD written = 0;
    const bool success = bytes.empty() || (WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()),
                                                     &written, nullptr) &&
                                           written == bytes.size());
    CloseHandle(file);
    if (!success)
        throw std::runtime_error("Cannot write isolated fixture");
}

std::vector<BYTE> Utf8(const std::wstring &text, bool bom = false)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0,
                                   nullptr, nullptr);
    std::vector<BYTE> bytes(size + (bom ? 3 : 0));
    if (bom)
        std::copy_n("\xEF\xBB\xBF", 3, bytes.begin());
    if (size)
        WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                            reinterpret_cast<char *>(bytes.data() + (bom ? 3 : 0)), size, nullptr, nullptr);
    return bytes;
}

std::vector<BYTE> Utf16(const std::wstring &text, bool little)
{
    std::vector<BYTE> bytes = little ? std::vector<BYTE>{0xFF, 0xFE} : std::vector<BYTE>{0xFE, 0xFF};
    for (wchar_t character : text)
    {
        const BYTE low = character & 255, high = character >> 8;
        bytes.push_back(little ? low : high);
        bytes.push_back(little ? high : low);
    }
    return bytes;
}

template <class T> struct Com
{
    T *p = nullptr;
    ~Com()
    {
        if (p)
            p->Release();
    }
};

void WicImageFixture(IWICImagingFactory *factory, const std::wstring &path,
                     const GUID &container = GUID_ContainerFormatPng)
{
    Com<IWICStream> stream;
    Com<IWICBitmapEncoder> encoder;
    Com<IWICBitmapFrameEncode> frame;
    Com<IPropertyBag2> options;
    HRESULT status = factory->CreateStream(&stream.p);
    if (SUCCEEDED(status))
        status = stream.p->InitializeFromFilename(path.c_str(), GENERIC_WRITE);
    if (SUCCEEDED(status))
        status = factory->CreateEncoder(container, nullptr, &encoder.p);
    if (SUCCEEDED(status))
        status = encoder.p->Initialize(stream.p, WICBitmapEncoderNoCache);
    if (SUCCEEDED(status))
        status = encoder.p->CreateNewFrame(&frame.p, &options.p);
    if (SUCCEEDED(status))
        status = frame.p->Initialize(options.p);
    if (SUCCEEDED(status))
        status = frame.p->SetSize(4, 2);
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    if (SUCCEEDED(status))
        status = frame.p->SetPixelFormat(&format);
    // Top left: opaque red. Top right: half-transparent green. Bottom: opaque blue.
    BYTE pixels[] = {0,   0, 255, 255, 0,   0, 255, 255, 0,   255, 0, 128, 0,   255, 0, 128,
                     255, 0, 0,   255, 255, 0, 0,   255, 255, 0,   0, 255, 255, 0,   0, 255};
    if (SUCCEEDED(status) && IsEqualGUID(format, GUID_WICPixelFormat32bppBGRA))
        status = frame.p->WritePixels(2, 16, sizeof(pixels), pixels);
    else
        status = E_FAIL;
    if (SUCCEEDED(status))
        status = frame.p->Commit();
    if (SUCCEEDED(status))
        status = encoder.p->Commit();
    if (FAILED(status))
        throw std::runtime_error("Cannot create image fixture with Windows WIC");
}

std::vector<BYTE> BmpFixture(LONG width, LONG height, bool withPixels)
{
    BITMAPFILEHEADER file{};
    BITMAPINFOHEADER info{};
    info.biSize = sizeof(info);
    info.biWidth = width;
    info.biHeight = height;
    info.biPlanes = 1;
    info.biBitCount = 24;
    const DWORD stride = ((width * 3 + 3) / 4) * 4;
    info.biSizeImage = stride * height;
    file.bfType = 0x4D42;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + info.biSizeImage;
    std::vector<BYTE> bytes(file.bfOffBits + (withPixels ? info.biSizeImage : 0));
    std::memcpy(bytes.data(), &file, sizeof(file));
    std::memcpy(bytes.data() + sizeof(file), &info, sizeof(info));
    if (withPixels)
        for (LONG y = 0; y < height; ++y)
            for (LONG x = 0; x < width; ++x)
            {
                BYTE *pixel = bytes.data() + file.bfOffBits + stride * y + x * 3;
                pixel[0] = y == 0 ? 255 : 0;
                pixel[2] = y == height - 1 ? 255 : 0;
            }
    return bytes;
}

std::vector<BYTE> OversizedPngHeader()
{
    // Start with our valid WIC PNG and change only IHDR dimensions + its CRC.
    // WIC reads size from this header; the loader must reject before decoding pixels.
    HANDLE file = CreateFileW(Path(L"transparent.png").c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Cannot read generated PNG fixture");
    std::vector<BYTE> bytes(GetFileSize(file, nullptr));
    DWORD read = 0;
    const bool success = ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr) &&
                         read == bytes.size() && bytes.size() > 33;
    CloseHandle(file);
    if (!success)
        throw std::runtime_error("Cannot read generated PNG header");
    const auto bigEndian = [&](size_t offset, DWORD value) {
        for (int index = 0; index < 4; ++index)
            bytes[offset + index] = static_cast<BYTE>(value >> (24 - 8 * index));
    };
    bigEndian(16, 8000);
    bigEndian(20, 6000);
    DWORD crc = 0xFFFFFFFF;
    for (size_t index = 12; index < 29; ++index)
    {
        crc ^= bytes[index];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
    }
    bigEndian(29, crc ^ 0xFFFFFFFF);
    return bytes;
}

DWORD Pixel(const LoadedContent &content, int x, int y)
{
    DIBSECTION bitmap{};
    if (!content.image || GetObjectW(content.image, sizeof(bitmap), &bitmap) != sizeof(bitmap) ||
        !bitmap.dsBm.bmBits)
        return 0;
    const BYTE *row = static_cast<const BYTE *>(bitmap.dsBm.bmBits) + y * bitmap.dsBm.bmWidthBytes;
    DWORD pixel = 0;
    std::memcpy(&pixel, row + x * 4, sizeof(pixel));
    return pixel;
}

bool SameRect(const RECT &a, const RECT &b)
{
    return EqualRect(&a, &b) != FALSE;
}

int Run()
{
    HRESULT apartment = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(apartment))
        throw std::runtime_error("Cannot initialize Windows COM for fixtures");
    Com<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_IWICImagingFactory, reinterpret_cast<void **>(&factory.p))))
        throw std::runtime_error("Cannot initialize Windows WIC for fixtures");

    const std::wstring sample = L"Xin ch\u00e0o\r\n\t  int main() {\n    return 0;\r}\n\xD83D\xDE42";
    LoadedContent content;
    std::wstring error;
    const auto textCase = [&](const wchar_t *name, std::vector<BYTE> bytes, const char *check) {
        Write(Path(name), bytes);
        bool loaded = LoadLocalContent(Path(name), content, error);
        Check(check, loaded && content.kind == ContentKind::Text && content.text == sample &&
                         content.displayName == name && content.image == nullptr && error.empty());
    };
    textCase(L"utf8.txt", Utf8(sample), "utf8_preserves_vietnamese_emoji_newlines_tabs");
    textCase(L"utf8-bom.txt", Utf8(sample, true), "utf8_bom_removed_only_from_start");
    textCase(L"utf16-le.txt", Utf16(sample, true), "utf16_le_exact_text");
    textCase(L"utf16-be.txt", Utf16(sample, false), "utf16_be_exact_text");
    Write(Path(L"empty.txt"), {});
    Check("empty_text_valid", LoadLocalContent(Path(L"empty.txt"), content, error) && content.text.empty());
    Write(Path(L"bom-only.txt"), {0xEF, 0xBB, 0xBF});
    Check("bom_only_text_valid",
          LoadLocalContent(Path(L"bom-only.txt"), content, error) && content.text.empty());
    Write(Path(L"exact.CPP"), Utf8(sample));
    Check("code_kind_case_insensitive_and_exact_indentation",
          LoadLocalContent(Path(L"exact.CPP"), content, error) && content.kind == ContentKind::Code &&
              content.text == sample);
    Write(Path(L"Dockerfile"), Utf8(sample));
    Check("extensionless_known_code", LoadLocalContent(Path(L"Dockerfile"), content, error) &&
                                          content.kind == ContentKind::Code && content.text == sample);
    Write(Path(L"lines.md"), Utf8(sample));
    Check("markdown_is_text",
          LoadLocalContent(Path(L"lines.md"), content, error) && content.kind == ContentKind::Text);

    const auto reject = [&](const wchar_t *name, std::vector<BYTE> bytes, const char *check) {
        Write(Path(name), bytes);
        const auto text = content.text, path = content.path, displayName = content.displayName;
        const auto kind = content.kind;
        const auto image = content.image;
        const int width = content.width, height = content.height;
        bool loaded = LoadLocalContent(Path(name), content, error);
        Check(check, !loaded && !error.empty() && content.text == text && content.path == path &&
                         content.displayName == displayName && content.kind == kind &&
                         content.image == image && content.width == width && content.height == height);
    };
    reject(L"invalid-utf8.txt", {0xC0, 0xAF}, "reject_overlong_utf8_preserves_document");
    reject(L"truncated-utf8.txt", {0xE2, 0x82}, "reject_truncated_utf8_preserves_document");
    reject(L"utf8-surrogate.txt", {0xED, 0xA0, 0x80}, "reject_utf8_surrogate");
    reject(L"odd-utf16.txt", {0xFF, 0xFE, 0x41}, "reject_odd_utf16");
    reject(L"lone-high.txt", {0xFF, 0xFE, 0x00, 0xD8}, "reject_lone_high_surrogate");
    reject(L"lone-low.txt", {0xFE, 0xFF, 0xDC, 0x00}, "reject_lone_low_surrogate");
    reject(L"wrong-pair.txt", {0xFF, 0xFE, 0x00, 0xD8, 0x41, 0x00}, "reject_invalid_surrogate_pair");
    reject(L"binary.txt", {0x41, 0, 0x42}, "reject_nul_binary_text");
    reject(L"unknown.bin", {'h', 'i'}, "reject_unknown_extension");
    Write(Path(L"maximum.txt"), std::vector<BYTE>(5 * 1024 * 1024, 'a'));
    Check("accept_5mib_text_boundary",
          LoadLocalContent(Path(L"maximum.txt"), content, error) && content.text.size() == 5 * 1024 * 1024);
    reject(L"too-large.txt", std::vector<BYTE>(5 * 1024 * 1024 + 1, 'a'), "reject_text_over_5mib");
    // Replace the large baseline to keep later failure reports and comparisons inexpensive.
    LoadLocalContent(Path(L"lines.md"), content, error);
    Check("missing_file_preserves_document", !LoadLocalContent(Path(L"missing.txt"), content, error) &&
                                                 content.text == sample && !error.empty());
    Check("reject_url", !LoadLocalContent(L"https://example.invalid/sample.txt", content, error));
    Check("reject_unc", !LoadLocalContent(L"\\\\localhost\\share\\sample.txt", content, error));
    Check("reject_device_path", !LoadLocalContent(L"\\\\.\\NUL", content, error));
    Check("reject_alternate_stream", !LoadLocalContent(Path(L"lines.md:stream"), content, error));
    std::wstring nulPath = Path(L"lines.md");
    nulPath.push_back(0);
    nulPath += L"ignored.txt";
    Check("reject_embedded_nul_path", !LoadLocalContent(nulPath, content, error));

    wchar_t executable[32768]{};
    GetModuleFileNameW(nullptr, executable, 32768);
    const auto marker = Path(L"execution-marker.txt");
    // This exact test-generated file only. Remove a stale marker from a previous failed run.
    DeleteFileW(marker.c_str());
    Check("create_benign_executable_fixture",
          CopyFileW(executable, Path(L"must-never-run.exe").c_str(), FALSE) != FALSE);
    Check("reject_executable_file",
          !LoadLocalContent(Path(L"must-never-run.exe"), content, error) && content.text == sample);
    Sleep(200);
    Check("executable_not_launched", GetFileAttributesW(marker.c_str()) == INVALID_FILE_ATTRIBUTES);

    WicImageFixture(factory.p, Path(L"transparent.png"));
    Check("png_dimensions_and_kind", LoadLocalContent(Path(L"transparent.png"), content, error) &&
                                         content.kind == ContentKind::Image && content.image &&
                                         content.width == 4 && content.height == 2 && content.text.empty());
    Check("png_pixels_preserve_orientation",
          Pixel(content, 0, 0) == 0xFFFF0000 && Pixel(content, 0, 1) == 0xFF0000FF);
    Check("png_alpha_premultiplied", Pixel(content, 3, 0) == 0x80008000);
    reject(L"invalid.png", {1, 2, 3, 4}, "invalid_image_preserves_previous_bitmap");
    WicImageFixture(factory.p, Path(L"unsupported-container.png"), GUID_ContainerFormatTiff);
    const auto previousBitmap = content.image;
    Check("reject_tiff_renamed_as_png",
          !LoadLocalContent(Path(L"unsupported-container.png"), content, error) &&
              content.image == previousBitmap && !error.empty());
    Write(Path(L"orientation.bmp"), BmpFixture(4, 2, true));
    Check("bmp_dimensions_and_kind", LoadLocalContent(Path(L"orientation.bmp"), content, error) &&
                                         content.kind == ContentKind::Image && content.width == 4 &&
                                         content.height == 2);
    Check("bmp_bottom_up_converted_correctly",
          Pixel(content, 0, 0) == 0xFFFF0000 && Pixel(content, 0, 1) == 0xFF0000FF);
    // Two different frames (red, green) in one valid 1x1 GIF; preview must use the first.
    Write(Path(L"two-frames.gif"),
          {'G',  'I', 'F',  '8', '9', 'a', 1, 0, 1, 0, 0x80, 0, 0,    255, 0,    0,   0,
           255,  0,   0x2C, 0,   0,   0,   0, 1, 0, 1, 0,    0, 2,    2,   0x44, 1,   0,
           0x2C, 0,   0,    0,   0,   1,   0, 1, 0, 0, 2,    2, 0x4C, 1,   0,    0x3B});
    Check("first_image_frame_only", LoadLocalContent(Path(L"two-frames.gif"), content, error) &&
                                        content.width == 1 && content.height == 1 &&
                                        Pixel(content, 0, 0) == 0xFFFF0000);
    reject(L"oversized.png", std::vector<BYTE>(25 * 1024 * 1024 + 1, 0), "reject_encoded_image_over_25mib");
    reject(L"too-many-pixels.png", OversizedPngHeader(), "reject_decoded_image_over_40mp");
    Check("decoded_pixel_limit_identified_before_pixel_allocation", error.find(L"40") != std::wstring::npos);

    using quietshare::FitImageRect;
    const RECT viewport{10, 20, 410, 320};
    Check("fit_landscape_aspect_and_center",
          SameRect(FitImageRect(viewport, 800, 400), RECT{10, 70, 410, 270}));
    Check("fit_portrait_aspect_and_center",
          SameRect(FitImageRect(viewport, 200, 400), RECT{135, 20, 285, 320}));
    Check("zoom_200_centered", SameRect(FitImageRect(viewport, 800, 400, 200), RECT{-190, -30, 610, 370}));
    Check("zoom_clamp_minimum",
          SameRect(FitImageRect(viewport, 800, 400, 1), FitImageRect(viewport, 800, 400, 25)));
    Check("zoom_clamp_maximum",
          SameRect(FitImageRect(viewport, 800, 400, 500), FitImageRect(viewport, 800, 400, 400)));
    Check("invalid_image_size_returns_empty", SameRect(FitImageRect(viewport, 0, 400), RECT{10, 20, 10, 20}));
    Check("empty_viewport_returns_empty",
          SameRect(FitImageRect(RECT{10, 20, 10, 320}, 4, 2), RECT{10, 20, 10, 20}));
    const RECT rounded = FitImageRect(RECT{0, 0, 399, 300}, 7, 3);
    Check("fit_aspect_rounding_within_one_pixel",
          std::abs((rounded.right - rounded.left) * 3 - (rounded.bottom - rounded.top) * 7) <= 7);

    LoadLocalContent(Path(L"transparent.png"), content, error);
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = 12;
    info.bmiHeader.biHeight = -8;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *pixels = nullptr;
    HBITMAP target = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    HGDIOBJ old = SelectObject(dc, target);
    std::fill_n(static_cast<DWORD *>(pixels), 96, 0x00112233);
    const RECT drawViewport{2, 2, 10, 6};
    RECT beforeClip{}, afterClip{};
    const int beforeClipType = GetClipBox(dc, &beforeClip);
    Check("draw_image_success", quietshare::DrawImage(dc, content, drawViewport, 100, RGB(0, 0, 255)));
    const int afterClipType = GetClipBox(dc, &afterClip);
    Check("draw_preserves_dc_clip_state", beforeClipType == afterClipType && SameRect(beforeClip, afterClip));
    GdiFlush();
    const DWORD *drawn = static_cast<const DWORD *>(pixels);
    Check("draw_preserves_pixels_outside_viewport", drawn[0] == 0x00112233 && drawn[95] == 0x00112233);
    Check("draw_transparent_pixels_blend_on_background", (drawn[2 * 12 + 8] & 0x00FFFFFF) == 0x0000807F);
    Check("draw_zoom_clipped_to_viewport", quietshare::DrawImage(dc, content, drawViewport, 400) &&
                                               drawn[0] == 0x00112233 && drawn[95] == 0x00112233);

    // A separate opaque bitmap makes pan directions observable without changing orientation.bmp.
    LoadedContent panContent;
    panContent.kind = ContentKind::Image;
    panContent.width = 4;
    panContent.height = 2;
    BITMAPINFO panInfo = info;
    panInfo.bmiHeader.biWidth = 4;
    panInfo.bmiHeader.biHeight = -2;
    void *panPixels = nullptr;
    panContent.image = CreateDIBSection(nullptr, &panInfo, DIB_RGB_COLORS, &panPixels, nullptr, 0);
    const DWORD quadrants[] = {0xFFFF0000, 0xFFFF0000, 0xFF00FF00, 0xFF00FF00,
                               0xFF0000FF, 0xFF0000FF, 0xFFFFFF00, 0xFFFFFF00};
    std::copy_n(quadrants, 8, static_cast<DWORD *>(panPixels));
    const auto outsideUnchanged = [&]() {
        for (int y = 0; y < 8; ++y)
            for (int x = 0; x < 12; ++x)
                if ((x < drawViewport.left || x >= drawViewport.right || y < drawViewport.top ||
                     y >= drawViewport.bottom) &&
                    drawn[y * 12 + x] != 0x00112233)
                    return false;
        return true;
    };
    const auto panCase = [&](const char *name, int panX, int panY, DWORD upperLeft, DWORD lowerRight) {
        std::fill_n(static_cast<DWORD *>(pixels), 96, 0x00112233);
        const bool rendered =
            quietshare::DrawImage(dc, panContent, drawViewport, 200, RGB(16, 32, 48), panX, panY);
        GdiFlush();
        GetClipBox(dc, &afterClip);
        Check(name, rendered && (drawn[2 * 12 + 3] & 0x00FFFFFF) == upperLeft &&
                        (drawn[5 * 12 + 8] & 0x00FFFFFF) == lowerRight && outsideUnchanged() &&
                        SameRect(beforeClip, afterClip));
    };
    panCase("pan_right_reveals_left_edge_with_fixed_clip", 4, 0, 0x00FF0000, 0x000000FF);
    panCase("pan_left_reveals_right_edge_with_fixed_clip", -4, 0, 0x0000FF00, 0x00FFFF00);
    panCase("pan_down_reveals_top_edge_with_fixed_clip", 0, 2, 0x00FF0000, 0x0000FF00);
    panCase("pan_up_reveals_bottom_edge_with_fixed_clip", 0, -2, 0x000000FF, 0x00FFFF00);
    panCase("pan_outside_still_fills_original_viewport", 12, 0, 0x00102030, 0x00102030);
    SelectObject(dc, old);
    DeleteObject(target);
    DeleteDC(dc);
    LoadedContent moved(std::move(content));
    Check("move_owns_bitmap_once",
          moved.image && !content.image && content.width == 0 && content.height == 0);
    moved.Clear();
    Check("clear_releases_bitmap_and_metadata", !moved.image && moved.width == 0 && moved.height == 0 &&
                                                    moved.text.empty() && moved.path.empty() &&
                                                    moved.displayName.empty());
    factory.p->Release();
    factory.p = nullptr;
    CoUninitialize();
    return 0;
}
} // namespace

int wmain()
{
    wchar_t module[32768]{};
    GetModuleFileNameW(nullptr, module, 32768);
    const std::wstring executable(module);
    root = executable.substr(0, executable.find_last_of(L"\\/"));
    if (executable.substr(executable.find_last_of(L"\\/") + 1) == L"must-never-run.exe")
    {
        Write(Path(L"execution-marker.txt"), {'E', 'X', 'E', 'C', 'U', 'T', 'E', 'D'});
        return 99;
    }
    try
    {
        Run();
    }
    catch (const std::exception &error)
    {
        std::fprintf(stderr, "%s\n", error.what());
        Check("fixture_or_runtime_setup", false);
    }
    const size_t passed =
        std::count_if(checks.begin(), checks.end(), [](const auto &item) { return item.second; });
    std::ostringstream report;
    report << "{\n  \"passedChecks\": " << passed << ",\n  \"failedChecks\": " << checks.size() - passed
           << ",\n  \"limits\": {\"textBytes\": 5242880, \"encodedImageBytes\": 26214400, \"decodedPixels\": "
              "40000000, \"zoomPercent\": [25, 400]},\n"
           << "  \"scope\": \"Native Content module on local Windows; generated fixtures only; no file "
              "dialog, physical display or remote receiver checks\",\n  \"checks\": [\n";
    for (size_t index = 0; index < checks.size(); ++index)
        report << "    {\"name\": \"" << checks[index].first
               << "\", \"passed\": " << (checks[index].second ? "true" : "false") << "}"
               << (index + 1 == checks.size() ? "\n" : ",\n");
    report << "  ]\n}\n";
    const std::string output = report.str();
    Write(Path(L"content-regression.json"), std::vector<BYTE>(output.begin(), output.end()));
    std::printf("Content regression: %zu/%zu passed, %zu failed\n", passed, checks.size(),
                checks.size() - passed);
    return passed == checks.size() ? 0 : 1;
}
