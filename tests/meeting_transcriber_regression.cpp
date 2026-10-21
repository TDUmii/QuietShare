#include "../src/MeetingTranscriber.h"

#include <windows.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
bool Check(bool value,const char* name,int& passed,int& failed)
{
    std::cout<<(value?"PASS ":"FAIL ")<<name<<"\n";
    value?++passed:++failed;
    return value;
}

bool WriteSilenceWave(const std::wstring& path)
{
    constexpr DWORD samples=16000;
    constexpr DWORD dataBytes=samples*2;
    std::ofstream output(path.c_str(),std::ios::binary|std::ios::trunc);
    if(!output)return false;
    const char riff[]="RIFF",wave[]="WAVE",fmt[]="fmt ",data[]="data";
    const DWORD riffBytes=36+dataBytes,fmtBytes=16;
    const std::uint16_t format=1,channels=1,bits=16;
    const std::uint32_t sampleRate=16000,byteRate=32000;
    const std::uint16_t blockAlign=2;
    output.write(riff,4);output.write(reinterpret_cast<const char*>(&riffBytes),4);output.write(wave,4);
    output.write(fmt,4);output.write(reinterpret_cast<const char*>(&fmtBytes),4);
    output.write(reinterpret_cast<const char*>(&format),2);output.write(reinterpret_cast<const char*>(&channels),2);
    output.write(reinterpret_cast<const char*>(&sampleRate),4);output.write(reinterpret_cast<const char*>(&byteRate),4);
    output.write(reinterpret_cast<const char*>(&blockAlign),2);output.write(reinterpret_cast<const char*>(&bits),2);
    output.write(data,4);output.write(reinterpret_cast<const char*>(&dataBytes),4);
    std::vector<char> silence(dataBytes,0);output.write(silence.data(),static_cast<std::streamsize>(silence.size()));
    return output.good();
}
}

int wmain()
{
    wchar_t temp[MAX_PATH]{};GetTempPathW(MAX_PATH,temp);
    const std::wstring root=std::wstring(temp)+L"QuietShare-meeting-transcriber-qa";
    CreateDirectoryW(root.c_str(),nullptr);
    const std::wstring wave=root+L"\\silence.wav";
    int passed=0,failed=0;
    quietshare::MeetingTranscriber transcriber;
    Check(!transcriber.Start(root+L"\\missing.wav",quietshare::TranscriptionLanguage::Vietnamese)&&
          transcriber.ErrorCode()==1,"missingRecordingIsRejected",passed,failed);
    Check(transcriber.Status(false).find(L"Không tìm thấy")!=std::wstring::npos,
          "missingRecordingHasActionableVietnameseStatus",passed,failed);
    Check(WriteSilenceWave(wave),"syntheticWaveFixtureCreated",passed,failed);
    const bool accepted=transcriber.Start(wave,quietshare::TranscriptionLanguage::English);
    Check(accepted,"localTranscriptionStartAccepted",passed,failed);
    bool completed=false;
    for(int tick=0;accepted&&tick<120&&!completed;++tick){Sleep(100);completed=transcriber.Completed();}
    Check(!accepted||completed,"localTranscriptionCompletesWithoutHanging",passed,failed);
    if(accepted&&completed){
        std::wstring text,path;
        Check(transcriber.TakeResult(text,path),"completedTranscriptionResultIsConsumable",passed,failed);
        const int code=transcriber.ErrorCode();
        Check(code==0||code==3||code==4||code==5||code==7,"recognizerReportsBoundedOutcome",passed,failed);
        if(!path.empty())DeleteFileW(path.c_str());
    }
    transcriber.Stop();
    DeleteFileW(wave.c_str());RemoveDirectoryW(root.c_str());
    std::cout<<"{\"passed\":"<<passed<<",\"failed\":"<<failed<<"}\n";
    return failed?1:0;
}
