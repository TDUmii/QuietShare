#include "../src/MeetingRecorder.h"

#include <windows.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

namespace {
bool Check(bool value, const char* name, int& passed, int& failed)
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
    const std::wstring root = std::wstring(temp) + L"QuietShare-meeting-recorder-qa";
    CreateDirectoryW(root.c_str(), nullptr);

    quietshare::MeetingRecorder recorder;
    int passed = 0, failed = 0;
    Check(recorder.Start(root), "explicitStartAccepted", passed, failed);
    Sleep(1500);
    const bool usable = recorder.Ready();
    recorder.Stop();

    const auto path = recorder.OutputPath();
    if (!usable) {
        // Headless build hosts can have no default render endpoint. The
        // recorder must report that condition and leave the app recoverable.
        const bool unavailable = recorder.ErrorCode() == 2 || recorder.ErrorCode() == 4;
        Check(unavailable, "missingPlaybackDeviceIsReported", passed, failed);
    } else {
        std::ifstream input(path.c_str(), std::ios::binary);
        char header[12]{};
        input.read(header, sizeof(header));
        const bool riff = input.gcount() == sizeof(header) &&
                          std::string(header, 4) == "RIFF" && std::string(header + 8, 4) == "WAVE";
        Check(riff, "wavHeaderIsValid", passed, failed);
        input.seekg(0, std::ios::end);
        const auto fileBytes = static_cast<unsigned long long>(input.tellg());
        DWORD riffBytes = 0;
        std::memcpy(&riffBytes, header + 4, sizeof(riffBytes));
        Check(fileBytes > 44 && riffBytes + 8ULL == fileBytes, "wavSizeMatchesPayload", passed, failed);
    }
    if (!path.empty())
        DeleteFileW(path.c_str());
    RemoveDirectoryW((root + L"\\Meetings").c_str());
    RemoveDirectoryW(root.c_str());
    std::cout << "{\"passed\":" << passed << ",\"failed\":" << failed << "}\n";
    return failed ? 1 : 0;
}
