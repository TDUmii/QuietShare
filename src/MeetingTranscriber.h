#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace quietshare {

enum class TranscriptionLanguage {
    Vietnamese = 0,
    English = 1
};

// Runs Windows SAPI against a finalized local WAV. The recognizer receives
// only the recorded playback mix; it never opens a microphone endpoint.
class MeetingTranscriber {
public:
    MeetingTranscriber() = default;
    ~MeetingTranscriber();
    MeetingTranscriber(const MeetingTranscriber&) = delete;
    MeetingTranscriber& operator=(const MeetingTranscriber&) = delete;

    bool Start(const std::wstring& wavPath, TranscriptionLanguage language);
    void Stop();
    bool Running() const { return running.load(); }
    bool Completed() const { return completed.load(); }
    int ErrorCode() const { return errorCode.load(); }
    // Consume the completed result once. outputPath is the UTF-8 transcript
    // file next to the WAV; text is also suitable for inserting into notes.
    bool TakeResult(std::wstring& text, std::wstring& outputPath);
    std::wstring Status(bool english) const;
    std::wstring OutputPath() const;

private:
    void Run(const std::wstring& wavPath, TranscriptionLanguage language);
    bool Recognize(const std::wstring& wavPath, TranscriptionLanguage language,
                   std::wstring& transcript, int& errorCode);
    bool WriteTranscript(const std::wstring& wavPath, const std::wstring& text,
                         std::wstring& outputPath);

    std::atomic<bool> stopRequested{false};
    std::atomic<bool> running{false};
    std::atomic<bool> completed{false};
    std::atomic<bool> consumed{false};
    std::atomic<int> errorCode{0};
    mutable std::mutex stateMutex;
    std::wstring transcript;
    std::wstring outputPath;
    TranscriptionLanguage language = TranscriptionLanguage::Vietnamese;
    std::thread worker;
};

}
