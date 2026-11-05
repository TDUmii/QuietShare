#pragma once
#include <windows.h>
#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace quietshare {
std::wstring SpeechFileHash(const std::wstring& path,const std::atomic<bool>* cancel=nullptr);
std::wstring SpeechModelPath(const std::wstring& directory,int model);
bool SpeechRuntimeReady(const std::wstring& directory,int model);
class SpeechProcess {
    std::mutex mutex;
    HANDLE active=nullptr;
public:
    void Cancel();
    bool Run(const std::wstring& executable,const std::wstring& arguments,const std::atomic<bool>& cancel,
             const std::wstring& watchedFile=L"",unsigned long long maximum=0,std::atomic<unsigned>* progress=nullptr);
};
class SpeechInstaller {
    std::thread worker;
    std::atomic<bool> cancel{false},running{false};
    std::atomic<unsigned> progress{0};
    std::atomic<int> error{0};
    SpeechProcess process;
public:
    ~SpeechInstaller(){Stop();}
    bool Start(const std::wstring& directory,int model);
    void Stop();
    bool Running()const{return running.load();}
    unsigned Progress()const{return progress.load();}
    int Error()const{return error.load();}
private:
    void Install(const std::wstring& directory,int model);
};
bool RunWhisper(const std::wstring& directory,int model,const std::wstring& wav,bool english,
                std::atomic<bool>& stop,SpeechProcess& process,std::wstring& text,std::wstring& output,int& error);
}
