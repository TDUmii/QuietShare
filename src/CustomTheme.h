#pragma once

#include <windows.h>

#include <string>

namespace quietshare {

struct CustomThemeColors {
    COLORREF background = RGB(20, 22, 23);
    COLORREF panel = RGB(34, 39, 40);
    COLORREF text = RGB(242, 245, 247);
    COLORREF accent = RGB(79, 190, 137);
};

namespace custom_theme_detail {
constexpr DWORD Magic = 0x54435351; // QSCT
constexpr DWORD Version = 1;
}

inline bool SaveCustomTheme(const std::wstring &dataDirectory, const CustomThemeColors &colors)
{
    if (dataDirectory.empty())
        return false;
    const DWORD values[] = {custom_theme_detail::Magic, custom_theme_detail::Version,
                            colors.background, colors.panel, colors.text, colors.accent};
    const std::wstring target = dataDirectory + L"\\custom-theme.dat";
    const std::wstring temporary = target + L".tmp";
    HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    const bool ok = WriteFile(file, values, sizeof(values), &written, nullptr) &&
                    written == sizeof(values) && FlushFileBuffers(file);
    CloseHandle(file);
    if (!ok || !MoveFileExW(temporary.c_str(), target.c_str(),
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temporary.c_str());
        return false;
    }
    return true;
}

inline bool LoadCustomTheme(const std::wstring &dataDirectory, CustomThemeColors &colors)
{
    if (dataDirectory.empty())
        return false;
    HANDLE file = CreateFileW((dataDirectory + L"\\custom-theme.dat").c_str(), GENERIC_READ,
                              FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    DWORD values[6]{};
    DWORD read = 0;
    const bool ok = ReadFile(file, values, sizeof(values), &read, nullptr) && read == sizeof(values);
    CloseHandle(file);
    if (!ok || values[0] != custom_theme_detail::Magic || values[1] != custom_theme_detail::Version)
        return false;
    colors = {static_cast<COLORREF>(values[2]), static_cast<COLORREF>(values[3]),
              static_cast<COLORREF>(values[4]), static_cast<COLORREF>(values[5])};
    return true;
}

}
