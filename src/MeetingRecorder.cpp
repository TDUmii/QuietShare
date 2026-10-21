#include "MeetingRecorder.h"

#include <audioclient.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <shlobj.h>
#include <algorithm>
#include <cstdio>
#include <vector>

namespace quietshare {
namespace {

template <class T> void ReleaseCom(T*& value)
{
    if (value) {
        value->Release();
        value = nullptr;
    }
}

bool WriteExact(HANDLE file, const void* data, DWORD size)
{
    const auto* bytes = static_cast<const BYTE*>(data);
    while (size) {
        DWORD written = 0;
        if (!WriteFile(file, bytes, size, &written, nullptr) || !written)
            return false;
        bytes += written;
        size -= written;
    }
    return true;
}

bool WriteWaveHeader(HANDLE file, const WAVEFORMATEX* format, DWORD formatBytes)
{
    const DWORD zero = 0;
    const DWORD riffSize = 0;
    const DWORD dataSize = 0;
    const DWORD fmtSize = formatBytes;
    const char riff[] = "RIFF";
    const char wave[] = "WAVE";
    const char fmt[] = "fmt ";
    const char data[] = "data";
    if (!WriteExact(file, riff, 4) || !WriteExact(file, &riffSize, sizeof(riffSize)) ||
        !WriteExact(file, wave, 4) || !WriteExact(file, fmt, 4) ||
        !WriteExact(file, &fmtSize, sizeof(fmtSize)) ||
        !WriteExact(file, format, formatBytes) || !WriteExact(file, data, 4))
        return false;
    return WriteExact(file, &dataSize, sizeof(dataSize));
}

bool PatchWaveHeader(HANDLE file, DWORD formatBytes, unsigned long long dataBytes)
{
    if (dataBytes > 0xffffffffULL - 64ULL)
        return false;
    const DWORD dataSize = static_cast<DWORD>(dataBytes);
    const DWORD riffSize = static_cast<DWORD>(20ULL + formatBytes + dataBytes);
    LARGE_INTEGER offset{};
    offset.QuadPart = 4;
    if (!SetFilePointerEx(file, offset, nullptr, FILE_BEGIN) ||
        !WriteExact(file, &riffSize, sizeof(riffSize)))
        return false;
    offset.QuadPart = static_cast<LONGLONG>(24 + formatBytes);
    return SetFilePointerEx(file, offset, nullptr, FILE_BEGIN) &&
           WriteExact(file, &dataSize, sizeof(dataSize)) && FlushFileBuffers(file);
}

bool WriteSilence(HANDLE file, unsigned long long bytes)
{
    static const BYTE silence[8192]{};
    while (bytes) {
        const DWORD chunk = static_cast<DWORD>(std::min<unsigned long long>(bytes, sizeof(silence)));
        if (!WriteExact(file, silence, chunk))
            return false;
        bytes -= chunk;
    }
    return true;
}

std::wstring MakeOutputPath(const std::wstring& dataDirectory)
{
    const std::wstring folder = dataDirectory;
    SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr);
    SYSTEMTIME now{};
    GetLocalTime(&now);
    for (unsigned index = 0; index < 100; ++index) {
        wchar_t name[96]{};
        if (index == 0)
            swprintf_s(name, L"Meeting-%04u%02u%02u-%02u%02u%02u.wav", now.wYear, now.wMonth,
                       now.wDay, now.wHour, now.wMinute, now.wSecond);
        else
            swprintf_s(name, L"Meeting-%04u%02u%02u-%02u%02u%02u-%02u.wav", now.wYear, now.wMonth,
                       now.wDay, now.wHour, now.wMinute, now.wSecond, index);
        const auto path = folder + L"\\" + name;
        if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
            return path;
    }
    return {};
}

}

MeetingRecorder::~MeetingRecorder()
{
    Stop();
}

bool MeetingRecorder::Start(const std::wstring& dataDirectory, const std::wstring& outputDirectory)
{
    Stop();
    const std::wstring folder = outputDirectory.empty() ? dataDirectory + L"\\Meetings" : outputDirectory;
    if (folder.empty()) {
        errorCode.store(1);
        return false;
    }
    const auto path = MakeOutputPath(folder);
    if (path.empty()) {
        errorCode.store(1);
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        outputPath = path;
    }
    errorCode.store(0);
    bytesWritten.store(0);
    ready.store(false);
    stopRequested.store(false);
    startedTick = GetTickCount64();
    running.store(true);
    worker = std::thread([this, path] { Capture(path); });
    return true;
}

void MeetingRecorder::Stop()
{
    stopRequested.store(true);
    if (worker.joinable())
        worker.join();
    ready.store(false);
    running.store(false);
}

void MeetingRecorder::Fail(int code)
{
    if (errorCode.load() == 0)
        errorCode.store(code);
    ready.store(false);
}

void MeetingRecorder::Capture(const std::wstring& path)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comReady = SUCCEEDED(hr);
    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* device = nullptr;
    IAudioClient* client = nullptr;
    IAudioCaptureClient* capture = nullptr;
    WAVEFORMATEX* format = nullptr;
    HANDLE dataEvent = nullptr;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD formatBytes = sizeof(WAVEFORMATEX);
    DWORD streamFlags = AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
    REFERENCE_TIME bufferDuration = 10000000;
    unsigned long long dataBytes = 0;
    bool started = false;
    bool captureFailed = false;
    bool headerReady = false;

    if (!comReady || FAILED(hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                                   __uuidof(IMMDeviceEnumerator),
                                                   reinterpret_cast<void**>(&enumerator))) ||
        FAILED(hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device)) ||
        FAILED(hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                     reinterpret_cast<void**>(&client))) ||
        FAILED(hr = client->GetMixFormat(&format)) || !format) {
        Fail(2);
        goto finish;
    }

    formatBytes = format->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? sizeof(WAVEFORMATEXTENSIBLE)
                                                                 : sizeof(WAVEFORMATEX);
    file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW,
                       FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE || !WriteWaveHeader(file, format, formatBytes)) {
        Fail(3);
        goto finish;
    }
    headerReady = true;

    dataEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!dataEvent) {
        Fail(4);
        goto finish;
    }
    hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, bufferDuration, 0, format, nullptr);
    if (FAILED(hr)) {
        // Some older audio drivers reject event-driven loopback. Polling the
        // same capture client remains bounded and avoids dropping the feature.
        streamFlags = AUDCLNT_STREAMFLAGS_LOOPBACK;
        hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, bufferDuration, 0, format, nullptr);
    }
    if (FAILED(hr)) {
        Fail(4);
        goto finish;
    }
    if (streamFlags & AUDCLNT_STREAMFLAGS_EVENTCALLBACK) {
        hr = client->SetEventHandle(dataEvent);
        if (FAILED(hr)) {
            Fail(4);
            goto finish;
        }
    }
    if (FAILED(hr = client->GetService(__uuidof(IAudioCaptureClient),
                                       reinterpret_cast<void**>(&capture))) ||
        !capture || FAILED(hr = client->Start())) {
        Fail(4);
        goto finish;
    }

    started = true;
    ready.store(true);
    while (!stopRequested.load()) {
        WaitForSingleObject(dataEvent, 50);
        UINT32 packetFrames = 0;
        hr = capture->GetNextPacketSize(&packetFrames);
        if (FAILED(hr)) {
            Fail(5);
            captureFailed = true;
            break;
        }
        while (packetFrames && !stopRequested.load()) {
            BYTE* buffer = nullptr;
            UINT32 frames = 0;
            DWORD flags = 0;
            if (FAILED(hr = capture->GetBuffer(&buffer, &frames, &flags, nullptr, nullptr))) {
                Fail(5);
                captureFailed = true;
                break;
            }
            const unsigned long long bytes = static_cast<unsigned long long>(frames) * format->nBlockAlign;
            const bool written = (flags & AUDCLNT_BUFFERFLAGS_SILENT) ? WriteSilence(file, bytes)
                                                                        : WriteExact(file, buffer, static_cast<DWORD>(bytes));
            capture->ReleaseBuffer(frames);
            if (!written) {
                Fail(6);
                captureFailed = true;
                break;
            }
            dataBytes += bytes;
            bytesWritten.store(dataBytes);
            if (dataBytes > 0xffffffffULL - 64ULL) {
                Fail(7);
                captureFailed = true;
                break;
            }
            if (FAILED(hr = capture->GetNextPacketSize(&packetFrames))) {
                Fail(5);
                captureFailed = true;
                break;
            }
        }
        if (captureFailed)
            break;
    }

finish:
    if (started && client)
        client->Stop();
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        if (headerReady) {
            HANDLE patchFile = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (patchFile == INVALID_HANDLE_VALUE || !PatchWaveHeader(patchFile, formatBytes, dataBytes))
                Fail(6);
            if (patchFile != INVALID_HANDLE_VALUE)
                CloseHandle(patchFile);
        } else {
            DeleteFileW(path.c_str());
        }
    }
    if (dataEvent)
        CloseHandle(dataEvent);
    if (format)
        CoTaskMemFree(format);
    ReleaseCom(capture);
    ReleaseCom(client);
    ReleaseCom(device);
    ReleaseCom(enumerator);
    if (comReady)
        CoUninitialize();
    ready.store(false);
    running.store(false);
}

std::wstring MeetingRecorder::FileName(const std::wstring& path)
{
    const auto slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? path : path.substr(slash + 1);
}

std::wstring MeetingRecorder::OutputPath() const
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return outputPath;
}

std::wstring MeetingRecorder::Status(bool english) const
{
    if (Running()) {
        if (!Ready())
            return english ? L"Preparing local system-audio recording..."
                           : L"Đang chuẩn bị ghi âm âm thanh máy...";
        const auto seconds = (GetTickCount64() - startedTick) / 1000;
        wchar_t duration[64]{};
        swprintf_s(duration, L"%02llu:%02llu:%02llu", seconds / 3600, (seconds / 60) % 60, seconds % 60);
        return (english ? L"Recording system audio" : L"Đang ghi âm âm thanh máy") +
               std::wstring(L" · ") + duration;
    }
    switch (ErrorCode()) {
    case 1:
        return english ? L"Could not create the local Meetings folder."
                       : L"Không tạo được thư mục bản ghi cục bộ.";
    case 2:
        return english ? L"No usable Windows playback device was found."
                       : L"Không tìm thấy thiết bị phát âm thanh Windows dùng được.";
    case 3:
        return english ? L"Could not create the meeting WAV file."
                       : L"Không tạo được file WAV ghi âm cuộc họp.";
    case 4:
        return english ? L"Windows could not start system-audio capture."
                       : L"Windows không khởi động được việc lấy âm thanh máy.";
    case 5:
        return english ? L"The playback device changed or stopped the recording."
                       : L"Thiết bị phát đã thay đổi hoặc dừng bản ghi.";
    case 6:
        return english ? L"The meeting recording could not be written."
                       : L"Không ghi được dữ liệu cuộc họp vào file.";
    case 7:
        return english ? L"The WAV recording reached its 4 GB limit."
                       : L"Bản ghi WAV đã chạm giới hạn 4 GB.";
    default:
        return {};
    }
}

}
