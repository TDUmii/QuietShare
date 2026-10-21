#pragma once

#include "MeetingSettings.h"

#include <windows.h>

#include <algorithm>
#include <string>
#include <vector>

namespace quietshare {

inline std::wstring MeetingProviderLabel(MeetingProvider provider, bool english)
{
    switch (provider) {
    case MeetingProvider::Teams: return L"Microsoft Teams";
    case MeetingProvider::Zoom: return L"Zoom";
    case MeetingProvider::GoogleMeet: return L"Google Meet";
    default: return english ? L"Other / local audio" : L"Khác / âm thanh máy";
    }
}

inline std::wstring TrimMeetingLine(std::wstring line)
{
    if (line.size() >= 3 && line[0] == 0xFEFF)
        line.erase(0, 1);
    while (!line.empty() && (line.back() == L'\r' || line.back() == L'\n' || line.back() == L' ' || line.back() == L'\t'))
        line.pop_back();
    size_t start = 0;
    while (start < line.size() && (line[start] == L' ' || line[start] == L'\t'))
        ++start;
    return line.substr(start);
}

inline std::wstring DecodeMeetingTranscript(const std::vector<BYTE> &bytes)
{
    if (bytes.size() >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE) {
        const auto *data = reinterpret_cast<const wchar_t *>(bytes.data() + 2);
        const size_t chars = (bytes.size() - 2) / sizeof(wchar_t);
        return std::wstring(data, data + chars);
    }
    if (bytes.size() >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF) {
        std::wstring result;
        result.reserve((bytes.size() - 2) / 2);
        for (size_t i = 2; i + 1 < bytes.size(); i += 2)
            result.push_back(static_cast<wchar_t>((bytes[i] << 8) | bytes[i + 1]));
        return result;
    }
    size_t offset = bytes.size() >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF ? 3 : 0;
    int chars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                    reinterpret_cast<const char *>(bytes.data() + offset),
                                    static_cast<int>(bytes.size() - offset), nullptr, 0);
    if (chars > 0) {
        std::wstring result(chars, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                            reinterpret_cast<const char *>(bytes.data() + offset),
                            static_cast<int>(bytes.size() - offset), result.data(), chars);
        return result;
    }
    chars = MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<const char *>(bytes.data()),
                                static_cast<int>(bytes.size()), nullptr, 0);
    std::wstring result(std::max(0, chars), L'\0');
    if (chars)
        MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<const char *>(bytes.data()),
                            static_cast<int>(bytes.size()), result.data(), chars);
    return result;
}

inline bool ImportMeetingTranscript(const std::wstring &path, MeetingProvider provider,
                                    std::wstring &transcript, std::wstring &error)
{
    transcript.clear();
    error.clear();
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        error = L"Không mở được file transcript.";
        return false;
    }
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) || size.QuadPart <= 0 || size.QuadPart > 16 * 1024 * 1024) {
        CloseHandle(file);
        error = L"File transcript rỗng hoặc quá lớn.";
        return false;
    }
    std::vector<BYTE> bytes(static_cast<size_t>(size.QuadPart));
    DWORD read = 0;
    const bool readOk = ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr) &&
                        read == bytes.size();
    CloseHandle(file);
    if (!readOk) {
        error = L"Không đọc được file transcript.";
        return false;
    }
    const auto source = DecodeMeetingTranscript(bytes);
    std::wstring line;
    size_t start = 0;
    bool vtt = false;
    while (start <= source.size()) {
        const size_t end = source.find_first_of(L"\r\n", start);
        line = TrimMeetingLine(source.substr(start, end == std::wstring::npos ? end : end - start));
        if (line == L"WEBVTT") {
            vtt = true;
        } else if (!line.empty() && line.find(L"-->") == std::wstring::npos &&
                   line.find(L"NOTE") != 0 && line.find(L"STYLE") != 0 && line.find(L"REGION") != 0) {
            // Teams VTT commonly emits <v Speaker Name>text. Keep the
            // attribution in the exported text instead of throwing it away.
            if (line.rfind(L"<v ", 0) == 0) {
                const size_t close = line.find(L'>', 3);
                if (close != std::wstring::npos) {
                    const auto speaker = line.substr(3, close - 3);
                    line = speaker + L": " + line.substr(close + 1);
                }
            }
            if (!line.empty())
                transcript += line + L"\r\n";
        }
        if (end == std::wstring::npos)
            break;
        start = end + 1;
        if (start < source.size() && source[end] == L'\r' && source[start] == L'\n')
            ++start;
    }
    while (!transcript.empty() && (transcript.back() == L'\r' || transcript.back() == L'\n'))
        transcript.pop_back();
    if (transcript.empty()) {
        error = vtt ? L"VTT không có nội dung lời nói." : L"Không tìm thấy nội dung transcript.";
        return false;
    }
    const std::wstring header = L"[" + MeetingProviderLabel(provider, false) + L"]\r\n";
    transcript = header + transcript;
    return true;
}

}
