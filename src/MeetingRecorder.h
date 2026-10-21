#pragma once

#include <windows.h>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace quietshare {

// Captures the Windows default render mix through WASAPI loopback. The
// recorder is deliberately explicit: it starts only after the user presses
// the meeting control and it never opens a microphone endpoint.
class MeetingRecorder {
public:
    MeetingRecorder() = default;
    ~MeetingRecorder();
    MeetingRecorder(const MeetingRecorder&) = delete;
    MeetingRecorder& operator=(const MeetingRecorder&) = delete;

    bool Start(const std::wstring& dataDirectory);
    void Stop();
    bool Running() const { return running.load(); }
    bool Ready() const { return ready.load(); }
    std::wstring OutputPath() const;
    std::wstring Status(bool english) const;
    int ErrorCode() const { return errorCode.load(); }
    unsigned long long BytesWritten() const { return bytesWritten.load(); }

private:
    void Capture(const std::wstring& outputPath);
    void Fail(int code);
    static std::wstring FileName(const std::wstring& path);

    std::atomic<bool> stopRequested{false};
    std::atomic<bool> running{false};
    std::atomic<bool> ready{false};
    std::atomic<int> errorCode{0};
    std::atomic<unsigned long long> bytesWritten{0};
    ULONGLONG startedTick = 0;
    mutable std::mutex stateMutex;
    std::wstring outputPath;
    std::thread worker;
};

}
