#include "Privacy.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace quietshare
{
namespace
{

constexpr DWORD kExcludeFromCapture = 0x00000011;
constexpr int kMinimumCaptureExclusionBuild = 19041;

template <typename Function> Function Procedure(HMODULE module, const char *name)
{
    const FARPROC address = GetProcAddress(module, name);
    Function function = nullptr;
    static_assert(sizeof(function) == sizeof(address), "Windows function pointer size");
    std::memcpy(&function, &address, sizeof(function));
    return function;
}

bool SameAdapter(const LUID &left, const LUID &right)
{
    return left.LowPart == right.LowPart && left.HighPart == right.HighPart;
}

bool IsCompositionEnabled()
{
    const HMODULE module = LoadLibraryExW(L"dwmapi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module)
        return false;
    using CompositionFunction = HRESULT(WINAPI *)(BOOL *);
    const auto function = Procedure<CompositionFunction>(module, "DwmIsCompositionEnabled");
    BOOL enabled = FALSE;
    const bool success = function && SUCCEEDED(function(&enabled)) && enabled;
    FreeLibrary(module);
    return success;
}

bool WriteAll(HANDLE file, const void *data, DWORD bytes)
{
    const auto *next = static_cast<const BYTE *>(data);
    while (bytes != 0)
    {
        DWORD written = 0;
        if (!WriteFile(file, next, bytes, &written, nullptr) || written == 0)
        {
            return false;
        }
        next += written;
        bytes -= written;
    }
    return true;
}

std::wstring ReadEnvironment(const wchar_t *name)
{
    const DWORD required = GetEnvironmentVariableW(name, nullptr, 0);
    if (!required)
        return {};
    std::vector<wchar_t> buffer(required);
    const DWORD length = GetEnvironmentVariableW(name, buffer.data(), required);
    if (!length || length >= required)
        return {};
    return std::wstring(buffer.data(), length);
}

} // namespace

int WindowsBuild()
{
    // RtlGetVersion reports the real OS version without relying on the
    // executable's supportedOS manifest entries.
    const HMODULE module = GetModuleHandleW(L"ntdll.dll");
    if (!module)
        return 0;
    using VersionFunction = LONG(WINAPI *)(OSVERSIONINFOW *);
    const auto function = Procedure<VersionFunction>(module, "RtlGetVersion");
    if (!function)
        return 0;
    OSVERSIONINFOW version{};
    version.dwOSVersionInfoSize = sizeof(version);
    if (function(&version) != 0 || version.dwMajorVersion < 10 ||
        version.dwBuildNumber > static_cast<DWORD>((std::numeric_limits<int>::max)()))
    {
        return 0;
    }
    return static_cast<int>(version.dwBuildNumber);
}

PrivacyResult ApplyPrivacy(HWND window)
{
    PrivacyResult result;
    result.requestedAffinity = kExcludeFromCapture;
    result.supported = WindowsBuild() >= kMinimumCaptureExclusionBuild;
    if (!result.supported)
    {
        result.error = ERROR_OLD_WIN_VERSION;
        result.message =
            L"Cần Windows 10 phiên bản 2004 (build 19041) hoặc mới hơn. Nội dung riêng chưa được mở.";
        return result;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (!IsWindow(window) || processId != GetCurrentProcessId() || GetAncestor(window, GA_ROOT) != window ||
        (GetWindowLongPtrW(window, GWL_STYLE) & WS_CHILD) != 0)
    {
        result.error = ERROR_INVALID_WINDOW_HANDLE;
        result.message =
            L"Chỉ có thể bảo vệ cửa sổ cấp cao nhất của QuietShare. Nội dung riêng chưa được mở.";
        return result;
    }

    if (!IsCompositionEnabled())
    {
        result.error = ERROR_NOT_SUPPORTED;
        result.message = L"Windows chưa bật bộ dựng màn hình DWM. Nội dung riêng chưa được mở.";
        return result;
    }

    SetLastError(ERROR_SUCCESS);
    if (!SetWindowDisplayAffinity(window, kExcludeFromCapture))
    {
        result.error = GetLastError();
        if (result.error == ERROR_SUCCESS)
            result.error = ERROR_GEN_FAILURE;
        result.message =
            L"Windows không áp dụng được chế độ loại trừ khỏi ảnh chụp. Nội dung riêng chưa được mở.";
        return result;
    }

    SetLastError(ERROR_SUCCESS);
    if (!GetWindowDisplayAffinity(window, &result.actualAffinity))
    {
        result.error = GetLastError();
        if (result.error == ERROR_SUCCESS)
            result.error = ERROR_GEN_FAILURE;
        result.message = L"Không đọc lại được trạng thái bảo vệ của cửa sổ. Nội dung riêng chưa được mở.";
        return result;
    }
    if (result.actualAffinity != kExcludeFromCapture)
    {
        result.error = ERROR_INVALID_DATA;
        result.message = L"Windows trả về chế độ bảo vệ khác với yêu cầu. Nội dung riêng chưa được mở.";
        return result;
    }

    result.applied = true;
    // Remains false until a separate receiver-side capture test is performed.
    result.verified = false;
    result.message = L"Windows đã nhận chế độ loại trừ khỏi ảnh chụp. Chưa xác minh hình ảnh phía người xem; "
                     L"màn hình Duplicate vẫn hiển thị cùng nội dung.";
    return result;
}

bool TryGetDisplayCloned(bool &cloned)
{
    cloned = true;
    // Active clone paths share a display source: the same adapter LUID/id
    // renders to more than one physical target. Do not infer cloning merely
    // from identical monitor dimensions or positions.
    constexpr UINT32 flags = QDC_ONLY_ACTIVE_PATHS;
    for (int attempt = 0; attempt != 3; ++attempt)
    {
        UINT32 pathCount = 0;
        UINT32 modeCount = 0;
        const LONG sizeResult = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);
        if (sizeResult != ERROR_SUCCESS || pathCount == 0)
            return false;
        std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
        std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);
        const LONG queryResult = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount,
                                                    modes.empty() ? nullptr : modes.data(), nullptr);
        if (queryResult == ERROR_INSUFFICIENT_BUFFER)
            continue;
        if (queryResult != ERROR_SUCCESS)
            return false;

        for (UINT32 left = 0; left < pathCount; ++left)
        {
            if ((paths[left].flags & DISPLAYCONFIG_PATH_ACTIVE) == 0)
                continue;
            for (UINT32 right = left + 1; right < pathCount; ++right)
            {
                if ((paths[right].flags & DISPLAYCONFIG_PATH_ACTIVE) == 0)
                    continue;
                const auto &a = paths[left].sourceInfo;
                const auto &b = paths[right].sourceInfo;
                const auto &aTarget = paths[left].targetInfo;
                const auto &bTarget = paths[right].targetInfo;
                const bool differentTarget =
                    aTarget.id != bTarget.id || !SameAdapter(aTarget.adapterId, bTarget.adapterId);
                if (differentTarget && a.id == b.id && SameAdapter(a.adapterId, b.adapterId))
                {
                    return true;
                }
            }
        }
        cloned = false;
        return true;
    }
    return false;
}

bool IsDisplayCloned()
{
    bool cloned = true;
    TryGetDisplayCloned(cloned);
    return cloned;
}

std::wstring JsonEscape(const std::wstring &text)
{
    static constexpr wchar_t hex[] = L"0123456789abcdef";
    std::wstring escaped;
    escaped.reserve(text.size());
    for (const wchar_t character : text)
    {
        switch (character)
        {
        case L'"':
            escaped += L"\\\"";
            break;
        case L'\\':
            escaped += L"\\\\";
            break;
        case L'\b':
            escaped += L"\\b";
            break;
        case L'\f':
            escaped += L"\\f";
            break;
        case L'\n':
            escaped += L"\\n";
            break;
        case L'\r':
            escaped += L"\\r";
            break;
        case L'\t':
            escaped += L"\\t";
            break;
        default:
            if (static_cast<unsigned int>(character) < 0x20)
            {
                escaped += L"\\u00";
                escaped += hex[(character >> 4) & 0xf];
                escaped += hex[character & 0xf];
            }
            else
            {
                escaped += character;
            }
            break;
        }
    }
    return escaped;
}

std::wstring GetLocalDataDirectory()
{
    std::wstring directory = ReadEnvironment(L"LOCALAPPDATA");
    if (directory.empty())
    {
        std::vector<wchar_t> buffer(MAX_PATH + 1);
        const DWORD length = GetTempPathW(static_cast<DWORD>(buffer.size()), buffer.data());
        if (!length || length >= buffer.size())
            return {};
        directory.assign(buffer.data(), length);
    }
    if (directory.back() != L'\\' && directory.back() != L'/')
        directory += L'\\';
    directory += L"QuietShare";
    if (!CreateDirectoryW(directory.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return {};
    }
    const DWORD attributes = GetFileAttributesW(directory.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return {};
    }
    return directory;
}

bool CaptureDesktopBmp(const std::wstring &path)
{
    const LONG left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const LONG top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const RECT region{left, top, left + GetSystemMetrics(SM_CXVIRTUALSCREEN),
                      top + GetSystemMetrics(SM_CYVIRTUALSCREEN)};
    return CaptureRegionBmp(path, region);
}

bool CaptureRegionBmp(const std::wstring &path, const RECT &region, bool includeLayered)
{
    const auto width64 = static_cast<std::int64_t>(region.right) - region.left;
    const auto height64 = static_cast<std::int64_t>(region.bottom) - region.top;
    if (path.empty() || width64 <= 0 || height64 <= 0 || width64 > (std::numeric_limits<int>::max)() ||
        height64 > (std::numeric_limits<int>::max)())
        return false;
    const int width = static_cast<int>(width64);
    const int height = static_cast<int>(height64);
    const std::uint64_t imageBytes = static_cast<std::uint64_t>(width) * height * 4;
    const std::uint64_t totalBytes = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imageBytes;
    if (totalBytes > (std::numeric_limits<DWORD>::max)())
        return false;

    const HDC screen = GetDC(nullptr);
    if (!screen)
        return false;
    const HDC memory = CreateCompatibleDC(screen);
    if (!memory)
    {
        ReleaseDC(nullptr, screen);
        return false;
    }
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biSizeImage = static_cast<DWORD>(imageBytes);
    void *pixels = nullptr;
    const HBITMAP bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!bitmap || !pixels)
    {
        if (bitmap)
            DeleteObject(bitmap);
        DeleteDC(memory);
        ReleaseDC(nullptr, screen);
        return false;
    }
    const HGDIOBJ previous = SelectObject(memory, bitmap);
    bool success = previous && previous != HGDI_ERROR &&
                   BitBlt(memory, 0, 0, width, height, screen, region.left, region.top,
                          SRCCOPY | (includeLayered ? CAPTUREBLT : 0));
    // GDI batches drawing operations; flush before reading DIB pixels.
    if (success)
        success = GdiFlush() != FALSE;
    if (success)
    {
        BITMAPFILEHEADER header{};
        header.bfType = 0x4d42;
        header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        header.bfSize = static_cast<DWORD>(totalBytes);
        const HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL, nullptr);
        success = file != INVALID_HANDLE_VALUE;
        if (success)
        {
            success = WriteAll(file, &header, sizeof(header)) &&
                      WriteAll(file, &info.bmiHeader, sizeof(info.bmiHeader)) &&
                      WriteAll(file, pixels, static_cast<DWORD>(imageBytes)) && FlushFileBuffers(file);
            CloseHandle(file);
            if (!success)
                DeleteFileW(path.c_str());
        }
    }
    if (previous && previous != HGDI_ERROR)
        SelectObject(memory, previous);
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(nullptr, screen);
    return success;
}

void SetClipboardText(HWND owner, const std::wstring &text)
{
    if (text.size() >= (std::numeric_limits<SIZE_T>::max)() / sizeof(wchar_t))
        return;
    const SIZE_T bytes = (text.size() + 1) * sizeof(wchar_t);
    const HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!memory)
        return;
    void *data = GlobalLock(memory);
    if (!data)
    {
        GlobalFree(memory);
        return;
    }
    std::memcpy(data, text.c_str(), bytes);
    GlobalUnlock(memory);
    if (!OpenClipboard(owner))
    {
        GlobalFree(memory);
        return;
    }
    if (!EmptyClipboard() || !SetClipboardData(CF_UNICODETEXT, memory))
        GlobalFree(memory);
    CloseClipboard();
}

} // namespace quietshare
