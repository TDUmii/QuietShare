#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace quietshare {

enum class MeetingProvider : DWORD {
    Generic = 0,
    Teams = 1,
    Zoom = 2,
    GoogleMeet = 3
};

enum class MeetingExportMode : DWORD {
    WavAndTranscript = 0,
    TranscriptOnly = 1
};

struct MeetingSettings {
    bool transcriptionEnabled = true;
    MeetingProvider provider = MeetingProvider::Generic;
    MeetingExportMode exportMode = MeetingExportMode::WavAndTranscript;
    std::wstring outputDirectory;
};

namespace meeting_settings_detail {
constexpr DWORD Magic = 0x514D5351; // QSMQ
constexpr DWORD Version = 1;
constexpr size_t MaxPathChars = 32768;

inline bool WriteAll(HANDLE file, const void *data, DWORD bytes)
{
    const auto *cursor = static_cast<const BYTE *>(data);
    while (bytes) {
        DWORD written = 0;
        if (!WriteFile(file, cursor, bytes, &written, nullptr) || !written)
            return false;
        cursor += written;
        bytes -= written;
    }
    return true;
}

inline bool ReadAll(HANDLE file, void *data, DWORD bytes)
{
    auto *cursor = static_cast<BYTE *>(data);
    while (bytes) {
        DWORD read = 0;
        if (!ReadFile(file, cursor, bytes, &read, nullptr) || !read)
            return false;
        cursor += read;
        bytes -= read;
    }
    return true;
}

inline bool IsValid(const MeetingSettings &settings)
{
    if (settings.outputDirectory.empty() || settings.outputDirectory.size() > MaxPathChars)
        return false;
    if (static_cast<DWORD>(settings.provider) > static_cast<DWORD>(MeetingProvider::GoogleMeet))
        return false;
    if (static_cast<DWORD>(settings.exportMode) > static_cast<DWORD>(MeetingExportMode::TranscriptOnly))
        return false;
    for (const wchar_t value : settings.outputDirectory)
        if (value < 0x20)
            return false;
    return true;
}
}

inline std::wstring DefaultMeetingDirectory(const std::wstring &dataDirectory)
{
    return dataDirectory.empty() ? std::wstring{} : dataDirectory + L"\\Meetings";
}

inline bool SaveMeetingSettings(const std::wstring &dataDirectory, const MeetingSettings &settings)
{
    if (dataDirectory.empty() || !meeting_settings_detail::IsValid(settings))
        return false;
    const DWORD flags = (settings.transcriptionEnabled ? 1U : 0U) |
                        (settings.exportMode == MeetingExportMode::TranscriptOnly ? 2U : 0U);
    const DWORD provider = static_cast<DWORD>(settings.provider);
    const DWORD pathChars = static_cast<DWORD>(settings.outputDirectory.size());
    const DWORD header[] = {meeting_settings_detail::Magic, meeting_settings_detail::Version,
                            flags, provider, pathChars};
    const std::wstring target = dataDirectory + L"\\meeting-settings.dat";
    const std::wstring temporary = target + L".tmp";
    HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    const bool ok = meeting_settings_detail::WriteAll(file, header, sizeof(header)) &&
                    (!pathChars || meeting_settings_detail::WriteAll(
                        file, settings.outputDirectory.data(), pathChars * sizeof(wchar_t))) &&
                    FlushFileBuffers(file);
    CloseHandle(file);
    if (!ok || !MoveFileExW(temporary.c_str(), target.c_str(),
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temporary.c_str());
        return false;
    }
    return true;
}

inline bool LoadMeetingSettings(const std::wstring &dataDirectory, MeetingSettings &settings)
{
    if (dataDirectory.empty())
        return false;
    const std::wstring path = dataDirectory + L"\\meeting-settings.dat";
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    LARGE_INTEGER size{};
    DWORD header[5]{};
    bool ok = GetFileSizeEx(file, &size) && size.QuadPart >= sizeof(header) &&
              size.QuadPart <= static_cast<LONGLONG>(sizeof(header) + meeting_settings_detail::MaxPathChars * sizeof(wchar_t)) &&
              meeting_settings_detail::ReadAll(file, header, sizeof(header));
    if (!ok || header[0] != meeting_settings_detail::Magic || header[1] != meeting_settings_detail::Version ||
        header[2] > 3 || header[3] > static_cast<DWORD>(MeetingProvider::GoogleMeet) ||
        header[4] > meeting_settings_detail::MaxPathChars ||
        size.QuadPart != static_cast<LONGLONG>(sizeof(header) + header[4] * sizeof(wchar_t))) {
        CloseHandle(file);
        return false;
    }
    std::wstring directory(header[4], L'\0');
    if (header[4] && !meeting_settings_detail::ReadAll(file, directory.data(), header[4] * sizeof(wchar_t)))
        ok = false;
    CloseHandle(file);
    if (!ok)
        return false;
    MeetingSettings loaded;
    loaded.transcriptionEnabled = (header[2] & 1U) != 0;
    loaded.exportMode = (header[2] & 2U) ? MeetingExportMode::TranscriptOnly : MeetingExportMode::WavAndTranscript;
    loaded.provider = static_cast<MeetingProvider>(header[3]);
    loaded.outputDirectory = std::move(directory);
    if (!meeting_settings_detail::IsValid(loaded))
        return false;
    settings = std::move(loaded);
    return true;
}

}
