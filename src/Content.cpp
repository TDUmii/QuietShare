#include "Content.h"

#include <wincodec.h>

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <limits>
#include <utility>
#include <vector>

namespace quietshare
{
namespace
{
constexpr LONGLONG TextLimit = 5LL * 1024 * 1024;
constexpr LONGLONG ImageLimit = 25LL * 1024 * 1024;
constexpr ULONGLONG PixelLimit = 40ULL * 1000 * 1000;

template <class T> struct ComObject
{
    T *value = nullptr;
    ~ComObject()
    {
        if (value)
            value->Release();
    }
    T *operator->() const
    {
        return value;
    }
};

struct FileHandle
{
    HANDLE value = INVALID_HANDLE_VALUE;
    ~FileHandle()
    {
        if (value != INVALID_HANDLE_VALUE)
            CloseHandle(value);
    }
};

struct ComApartment
{
    HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    ~ComApartment()
    {
        if (SUCCEEDED(initialized))
            CoUninitialize();
    }
};

bool Fail(std::wstring &error, const wchar_t *message)
{
    error = message;
    return false;
}

bool LocalDrivePath(const std::wstring &path)
{
    if (path.size() < 3 || !((path[0] >= L'A' && path[0] <= L'Z') || (path[0] >= L'a' && path[0] <= L'z')) ||
        path[1] != L':' || (path[2] != L'\\' && path[2] != L'/') || path.find(L':', 2) != std::wstring::npos)
        return false;
    const wchar_t root[] = {path[0], L':', L'\\', 0};
    const UINT drive = GetDriveTypeW(root);
    return drive == DRIVE_FIXED || drive == DRIVE_REMOVABLE || drive == DRIVE_RAMDISK;
}

bool ResolveLocalPath(const std::wstring &input, std::wstring &resolved, std::wstring &error)
{
    if (input.empty() || input.size() > 32760 || input.find(L'\0') != std::wstring::npos ||
        input.rfind(L"\\\\", 0) == 0 || input.rfind(L"//", 0) == 0)
        return Fail(error, L"Chỉ mở tệp trên ổ đĩa cục bộ. / Open a file on a local drive.");
    DWORD length = GetFullPathNameW(input.c_str(), 0, nullptr, nullptr);
    if (!length || length > 32767)
        return Fail(error, L"Đường dẫn tệp không hợp lệ. / The file path is invalid.");
    std::vector<wchar_t> buffer(length);
    DWORD written = GetFullPathNameW(input.c_str(), length, buffer.data(), nullptr);
    if (!written || written >= length)
        return Fail(error, L"Đường dẫn tệp không hợp lệ. / The file path is invalid.");
    resolved.assign(buffer.data(), written);
    if (!LocalDrivePath(resolved))
        return Fail(error, L"Chỉ mở tệp trên ổ đĩa cục bộ. / Open a file on a local drive.");
    return true;
}

bool KindForName(const std::wstring &name, ContentKind &kind)
{
    std::wstring lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](wchar_t character) { return static_cast<wchar_t>(std::towlower(character)); });
    const size_t dot = lower.find_last_of(L'.');
    const std::wstring extension = dot == std::wstring::npos ? L"" : lower.substr(dot);
    static const wchar_t *images[] = {L".png", L".jpg", L".jpeg", L".bmp", L".gif"};
    for (const wchar_t *candidate : images)
        if (extension == candidate)
        {
            kind = ContentKind::Image;
            return true;
        }
    if (lower == L"dockerfile" || lower == L"makefile" || lower == L"cmakelists.txt")
    {
        kind = ContentKind::Code;
        return true;
    }
    static const wchar_t *documents[] = {L".txt", L".md", L".markdown", L".log"};
    for (const wchar_t *candidate : documents)
        if (extension == candidate)
        {
            kind = ContentKind::Text;
            return true;
        }
    static const wchar_t *sources[] = {
        L".c",    L".cc",   L".cpp",  L".cxx",    L".h",      L".hh",    L".hpp", L".hxx",
        L".cs",   L".java", L".kt",   L".kts",    L".py",     L".pyw",   L".js",  L".jsx",
        L".mjs",  L".cjs",  L".ts",   L".tsx",    L".html",   L".htm",   L".css", L".scss",
        L".sass", L".less", L".json", L".jsonc",  L".xml",    L".svg",   L".yml", L".yaml",
        L".toml", L".ini",  L".cfg",  L".conf",   L".sh",     L".bash",  L".ps1", L".psm1",
        L".bat",  L".cmd",  L".sql",  L".rs",     L".go",     L".rb",    L".php", L".swift",
        L".lua",  L".r",    L".vue",  L".svelte", L".gradle", L".cmake", L".tex"};
    for (const wchar_t *candidate : sources)
        if (extension == candidate)
        {
            kind = ContentKind::Code;
            return true;
        }
    return false;
}

bool ReadLocalFile(const std::wstring &path, LONGLONG limit, std::vector<BYTE> &bytes, std::wstring &error)
{
    FileHandle file;
    file.value =
        CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file.value == INVALID_HANDLE_VALUE)
        return Fail(
            error,
            L"Không mở được tệp. Kiểm tra quyền truy cập. / Cannot open the file. Check access permissions.");
    BY_HANDLE_FILE_INFORMATION info{};
    if (GetFileType(file.value) != FILE_TYPE_DISK || !GetFileInformationByHandle(file.value, &info) ||
        (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        return Fail(error, L"Hãy chọn một tệp cục bộ. / Choose a local file.");
    DWORD length = GetFinalPathNameByHandleW(file.value, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (!length || length > 32767)
        return Fail(error, L"Không xác nhận được đường dẫn cục bộ. / Cannot verify the local file path.");
    std::vector<wchar_t> finalBuffer(static_cast<size_t>(length) + 1);
    DWORD written =
        GetFinalPathNameByHandleW(file.value, finalBuffer.data(), static_cast<DWORD>(finalBuffer.size()),
                                  FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (!written || written >= finalBuffer.size())
        return Fail(error, L"Không xác nhận được đường dẫn cục bộ. / Cannot verify the local file path.");
    std::wstring finalPath(finalBuffer.data(), written);
    if (finalPath.rfind(L"\\\\?\\", 0) == 0)
        finalPath.erase(0, 4);
    if (!LocalDrivePath(finalPath))
        return Fail(error, L"Tệp phải nằm trên ổ đĩa cục bộ. / The file must be on a local drive.");
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file.value, &size) || size.QuadPart < 0)
        return Fail(error, L"Không đọc được kích thước tệp. / Cannot read the file size.");
    if (size.QuadPart > limit)
        return Fail(error,
                    limit == TextLimit
                        ? L"Tệp chữ hoặc code phải nhỏ hơn hoặc bằng 5 MiB. / Text or code files must be at "
                          L"most 5 MiB."
                        : L"Tệp ảnh phải nhỏ hơn hoặc bằng 25 MiB. / Image files must be at most 25 MiB.");
    bytes.resize(static_cast<size_t>(size.QuadPart));
    DWORD read = 0;
    if (!bytes.empty() &&
        (!ReadFile(file.value, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr) ||
         read != bytes.size()))
        return Fail(error, L"Không đọc hết được tệp. Hãy thử lại. / Cannot read the whole file. Try again.");
    return true;
}

bool DecodeText(const std::vector<BYTE> &bytes, std::wstring &text, std::wstring &error)
{
    const wchar_t *encodingError =
        L"Tệp cần dùng UTF-8 hoặc UTF-16 có BOM. / The file must use UTF-8 or BOM-marked UTF-16.";
    text.clear();
    if (bytes.size() >= 2 &&
        ((bytes[0] == 0xFF && bytes[1] == 0xFE) || (bytes[0] == 0xFE && bytes[1] == 0xFF)))
    {
        if ((bytes.size() - 2) % 2 != 0)
            return Fail(error, encodingError);
        const bool littleEndian = bytes[0] == 0xFF;
        text.reserve((bytes.size() - 2) / 2);
        for (size_t offset = 2; offset < bytes.size(); offset += 2)
        {
            const unsigned first = bytes[offset];
            const unsigned second = bytes[offset + 1];
            text.push_back(
                static_cast<wchar_t>(littleEndian ? first | (second << 8) : (first << 8) | second));
        }
        for (size_t index = 0; index < text.size(); ++index)
        {
            const unsigned unit = static_cast<unsigned>(text[index]);
            if (unit >= 0xD800 && unit <= 0xDBFF)
            {
                if (++index >= text.size() || text[index] < 0xDC00 || text[index] > 0xDFFF)
                    return Fail(error, encodingError);
            }
            else if (unit >= 0xDC00 && unit <= 0xDFFF)
                return Fail(error, encodingError);
        }
    }
    else
    {
        const size_t start =
            bytes.size() >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF ? 3 : 0;
        const int byteCount = static_cast<int>(bytes.size() - start);
        if (byteCount)
        {
            const char *source = reinterpret_cast<const char *>(bytes.data() + start);
            const int units =
                MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, source, byteCount, nullptr, 0);
            if (!units)
                return Fail(error, encodingError);
            text.resize(units);
            if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, source, byteCount, text.data(), units) !=
                units)
                return Fail(error, encodingError);
        }
    }
    if (text.find(L'\0') != std::wstring::npos)
        return Fail(
            error,
            L"Tệp có dữ liệu nhị phân, không phải nội dung chữ. / The file contains binary data, not text.");
    return true;
}

bool DecodeImage(std::vector<BYTE> &bytes, LoadedContent &content, std::wstring &error)
{
    const wchar_t *decodeError = L"Không đọc được ảnh. Hãy chọn PNG, JPEG, BMP hoặc GIF hợp lệ. / Cannot "
                                 L"decode this image. Choose a valid PNG, JPEG, BMP or GIF.";
    if (bytes.empty())
        return Fail(error, decodeError);
    ComApartment apartment;
    if (FAILED(apartment.initialized) && apartment.initialized != RPC_E_CHANGED_MODE)
        return Fail(
            error, L"Không khởi tạo được bộ đọc ảnh Windows. / Cannot initialize the Windows image decoder.");
    ComObject<IWICImagingFactory> factory;
    ComObject<IWICStream> stream;
    ComObject<IWICBitmapDecoder> decoder;
    ComObject<IWICBitmapFrameDecode> frame;
    ComObject<IWICFormatConverter> converter;
    HRESULT status = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_IWICImagingFactory, reinterpret_cast<void **>(&factory.value));
    if (FAILED(status) || FAILED(factory->CreateStream(&stream.value)) ||
        FAILED(stream->InitializeFromMemory(bytes.data(), static_cast<DWORD>(bytes.size()))) ||
        FAILED(factory->CreateDecoderFromStream(stream.value, nullptr, WICDecodeMetadataCacheOnDemand,
                                                &decoder.value)) ||
        FAILED(decoder->GetFrame(0, &frame.value)))
        return Fail(error, decodeError);
    GUID container{};
    if (FAILED(decoder->GetContainerFormat(&container)) ||
        (!IsEqualGUID(container, GUID_ContainerFormatPng) &&
         !IsEqualGUID(container, GUID_ContainerFormatJpeg) &&
         !IsEqualGUID(container, GUID_ContainerFormatBmp) &&
         !IsEqualGUID(container, GUID_ContainerFormatGif)))
        return Fail(error, decodeError);
    UINT width = 0, height = 0;
    if (FAILED(frame->GetSize(&width, &height)) || !width || !height)
        return Fail(error, decodeError);
    if (static_cast<ULONGLONG>(width) * height > PixelLimit || width > std::numeric_limits<int>::max() ||
        height > std::numeric_limits<int>::max())
        return Fail(error, L"Ảnh vượt quá 40 megapixel. Hãy dùng ảnh nhỏ hơn. / This image exceeds 40 "
                           L"megapixels. Use a smaller image.");
    if (FAILED(factory->CreateFormatConverter(&converter.value)) ||
        FAILED(converter->Initialize(frame.value, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
                                     nullptr, 0.0, WICBitmapPaletteTypeCustom)))
        return Fail(error, decodeError);
    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = static_cast<LONG>(width);
    bitmapInfo.bmiHeader.biHeight = -static_cast<LONG>(height);
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    void *pixels = nullptr;
    content.image = CreateDIBSection(nullptr, &bitmapInfo, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!content.image || !pixels)
        return Fail(error, L"Không đủ bộ nhớ để mở ảnh. / Not enough memory to open this image.");
    const UINT stride = width * 4;
    const UINT bufferSize = stride * height;
    if (FAILED(converter->CopyPixels(nullptr, stride, bufferSize, static_cast<BYTE *>(pixels))))
        return Fail(error, decodeError);
    content.width = static_cast<int>(width);
    content.height = static_cast<int>(height);
    return true;
}
} // namespace

LoadedContent::~LoadedContent()
{
    Clear();
}

LoadedContent::LoadedContent(LoadedContent &&other) noexcept
{
    *this = std::move(other);
}

LoadedContent &LoadedContent::operator=(LoadedContent &&other) noexcept
{
    if (this != &other)
    {
        Clear();
        kind = other.kind;
        path = std::move(other.path);
        displayName = std::move(other.displayName);
        text = std::move(other.text);
        image = other.image;
        width = other.width;
        height = other.height;
        other.image = nullptr;
        other.width = other.height = 0;
    }
    return *this;
}

void LoadedContent::Clear() noexcept
{
    if (image)
        DeleteObject(image);
    image = nullptr;
    width = height = 0;
    kind = ContentKind::Text;
    path.clear();
    displayName.clear();
    text.clear();
}

bool LoadLocalContent(const std::wstring &path, LoadedContent &result, std::wstring &error)
{
    LoadedContent candidate;
    if (!ResolveLocalPath(path, candidate.path, error))
        return false;
    const size_t separator = candidate.path.find_last_of(L"\\/");
    candidate.displayName = candidate.path.substr(separator == std::wstring::npos ? 0 : separator + 1);
    if (!KindForName(candidate.displayName, candidate.kind))
        return Fail(error, L"Chọn tệp chữ, code hoặc ảnh PNG/JPEG/BMP/GIF. / Choose a text, source code or "
                           L"PNG/JPEG/BMP/GIF file.");
    std::vector<BYTE> bytes;
    if (!ReadLocalFile(candidate.path, candidate.kind == ContentKind::Image ? ImageLimit : TextLimit, bytes,
                       error))
        return false;
    if (candidate.kind == ContentKind::Image ? !DecodeImage(bytes, candidate, error)
                                             : !DecodeText(bytes, candidate.text, error))
        return false;
    result = std::move(candidate);
    error.clear();
    return true;
}

RECT FitImageRect(const RECT &viewport, int imageWidth, int imageHeight, int zoomPercent)
{
    RECT result{viewport.left, viewport.top, viewport.left, viewport.top};
    const LONGLONG viewportWidth = static_cast<LONGLONG>(viewport.right) - viewport.left;
    const LONGLONG viewportHeight = static_cast<LONGLONG>(viewport.bottom) - viewport.top;
    if (imageWidth <= 0 || imageHeight <= 0 || viewportWidth <= 0 || viewportHeight <= 0)
        return result;
    const double fit = std::min(static_cast<double>(viewportWidth) / imageWidth,
                                static_cast<double>(viewportHeight) / imageHeight);
    const double scale = fit * std::clamp(zoomPercent, 25, 400) / 100.0;
    const LONGLONG width = std::max<LONGLONG>(1, std::llround(imageWidth * scale));
    const LONGLONG height = std::max<LONGLONG>(1, std::llround(imageHeight * scale));
    const auto coordinate = [](LONGLONG value) {
        return static_cast<LONG>(
            std::clamp<LONGLONG>(value, std::numeric_limits<LONG>::min(), std::numeric_limits<LONG>::max()));
    };
    const LONGLONG left = static_cast<LONGLONG>(viewport.left) + (viewportWidth - width) / 2;
    const LONGLONG top = static_cast<LONGLONG>(viewport.top) + (viewportHeight - height) / 2;
    result = {coordinate(left), coordinate(top), coordinate(left + width), coordinate(top + height)};
    return result;
}

bool DrawImage(HDC dc, const LoadedContent &content, const RECT &viewport, int zoomPercent,
               COLORREF background, int panX, int panY)
{
    if (!dc || viewport.right <= viewport.left || viewport.bottom <= viewport.top)
        return false;
    HBRUSH brush = CreateSolidBrush(background);
    FillRect(dc, &viewport, brush);
    DeleteObject(brush);
    if (!content.image || content.width <= 0 || content.height <= 0)
        return false;
    RECT target = FitImageRect(viewport, content.width, content.height, zoomPercent);
    OffsetRect(&target, panX, panY);
    if (target.right <= target.left || target.bottom <= target.top)
        return false;
    HDC source = CreateCompatibleDC(dc);
    if (!source)
        return false;
    HGDIOBJ old = SelectObject(source, content.image);
    const int saved = SaveDC(dc);
    if (!saved)
    {
        SelectObject(source, old);
        DeleteDC(source);
        return false;
    }
    IntersectClipRect(dc, viewport.left, viewport.top, viewport.right, viewport.bottom);
    const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    const BOOL drawn =
        AlphaBlend(dc, target.left, target.top, target.right - target.left, target.bottom - target.top,
                   source, 0, 0, content.width, content.height, blend);
    RestoreDC(dc, saved);
    SelectObject(source, old);
    DeleteDC(source);
    return drawn != FALSE;
}

} // namespace quietshare
