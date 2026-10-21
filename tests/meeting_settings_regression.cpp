#include "../src/MeetingSettings.h"

#include <windows.h>

#include <iostream>
#include <string>

namespace {
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
    const std::wstring root = std::wstring(temp) + L"QuietShare-meeting-settings-qa";
    CreateDirectoryW(root.c_str(), nullptr);
    int passed = 0, failed = 0;
    quietshare::MeetingSettings original;
    original.transcriptionEnabled = false;
    original.provider = quietshare::MeetingProvider::Teams;
    original.exportMode = quietshare::MeetingExportMode::TranscriptOnly;
    original.outputDirectory = root + L"\\Exports";
    CreateDirectoryW(original.outputDirectory.c_str(), nullptr);
    quietshare::MeetingSettings loaded;
    Check(quietshare::SaveMeetingSettings(root, original), "meetingSettingsSave", passed, failed);
    Check(quietshare::LoadMeetingSettings(root, loaded) &&
              loaded.transcriptionEnabled == original.transcriptionEnabled &&
              loaded.provider == original.provider && loaded.exportMode == original.exportMode &&
              loaded.outputDirectory == original.outputDirectory,
          "meetingSettingsRoundTrip", passed, failed);
    quietshare::MeetingSettings invalid = original;
    invalid.outputDirectory.assign(quietshare::meeting_settings_detail::MaxPathChars + 1, L'x');
    Check(!quietshare::SaveMeetingSettings(root, invalid), "oversizedMeetingPathRejected", passed, failed);
    DeleteFileW((root + L"\\meeting-settings.dat").c_str());
    RemoveDirectoryW(original.outputDirectory.c_str());
    RemoveDirectoryW(root.c_str());
    std::cout << "{\"passed\":" << passed << ",\"failed\":" << failed << "}\n";
    return failed ? 1 : 0;
}
