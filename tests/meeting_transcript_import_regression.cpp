#include "../src/MeetingTranscriptImport.h"

#include <windows.h>

#include <iostream>
#include <string>

namespace {
bool WriteUtf8(const std::wstring &path, const std::string &text)
{
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    const bool ok = WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) && written == text.size();
    CloseHandle(file);
    return ok;
}
bool Check(bool value, const char *name, int &passed, int &failed)
{
    std::cout << (value ? "PASS " : "FAIL ") << name << "\n";
    value ? ++passed : ++failed;
    return value;
}
}

int wmain()
{
    wchar_t temp[MAX_PATH]{};
    GetTempPathW(MAX_PATH, temp);
    const std::wstring root = std::wstring(temp) + L"QuietShare-meeting-import-qa";
    CreateDirectoryW(root.c_str(), nullptr);
    const std::wstring vtt = root + L"\\teams.vtt";
    const std::wstring txt = root + L"\\zoom.txt";
    WriteUtf8(vtt, "WEBVTT\n\n00:00:01.000 --> 00:00:03.000\n<v Alice>Xin chao\n\n00:00:04.000 --> 00:00:05.000\n<v Bob>Welcome\n");
    WriteUtf8(txt, "Alice: First point\nBob: Second point\n");
    int passed = 0, failed = 0;
    std::wstring transcript, error;
    const bool teamsOk=quietshare::ImportMeetingTranscript(vtt, quietshare::MeetingProvider::Teams, transcript, error);
    Check(teamsOk &&
              transcript.find(L"[Microsoft Teams]") != std::wstring::npos &&
              transcript.find(L"Alice: Xin chao") != std::wstring::npos &&
              transcript.find(L"Bob: Welcome") != std::wstring::npos,
          "teamsVttKeepsSpeakerAttribution", passed, failed);
    transcript.clear(); error.clear();
    const bool zoomOk=quietshare::ImportMeetingTranscript(txt, quietshare::MeetingProvider::Zoom, transcript, error);
    Check(zoomOk &&
              transcript.find(L"[Zoom]") != std::wstring::npos && transcript.find(L"Bob: Second point") != std::wstring::npos,
          "zoomTextKeepsSpeakerAttribution", passed, failed);
    DeleteFileW(vtt.c_str()); DeleteFileW(txt.c_str()); RemoveDirectoryW(root.c_str());
    std::cout << "{\"passed\":" << passed << ",\"failed\":" << failed << "}\n";
    return failed ? 1 : 0;
}
