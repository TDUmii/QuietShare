#include "../src/SpeechRuntime.h"
#include "../src/SpeechAssets.h"
#include "../src/MeetingNotebook.h"
#include "../src/SpeechRuntime.cpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

double WordErrorRate(std::wstring actual,std::wstring reference){
    auto words=[](std::wstring value){CharLowerBuffW(value.data(),static_cast<DWORD>(value.size()));for(auto& c:value)if(wcschr(L".,!?;:\"()[]",c))c=L' ';
        std::wistringstream input(value);std::vector<std::wstring> result;std::wstring word;while(input>>word)result.push_back(word);return result;};
    const auto a=words(actual),r=words(reference);std::vector<size_t> prev(r.size()+1),next(prev.size());for(size_t j=0;j<prev.size();++j)prev[j]=j;
    for(size_t i=0;i<a.size();++i){next[0]=i+1;for(size_t j=0;j<r.size();++j)next[j+1]=std::min({prev[j+1]+1,next[j]+1,prev[j]+(a[i]!=r[j])});prev.swap(next);}
    return r.empty()?1.0:double(prev.back())/r.size();
}

int wmain(int argc,wchar_t** argv){
    using namespace quietshare;
    int passed=0,failed=0,notTested=0;std::string checks;double vietnameseWer=-1;ULONGLONG inferenceMs[2]{};
    auto check=[&](const char* name,bool ok){(ok?++passed:++failed);std::cout<<(ok?"PASS ":"FAIL ")<<name<<std::endl;
        if(!checks.empty())checks+=",";checks+="{\"name\":\""+std::string(name)+"\",\"passed\":"+(ok?"true":"false")+"}";};
    SpeechProcess process;std::atomic<bool> stop{false};
    check("missing runtime is explicit",!SpeechRuntimeReady(L"not-present",1));
    check("unknown model cannot be selected",!SpeechRuntimeReady(L"not-present",9));
    stop=true;check("cancelled process never starts",!process.Run(L"missing.exe",L"",stop));stop=false;
    wchar_t system[32768]{};GetSystemDirectoryW(system,32768);
    bool result=true;std::atomic<bool> complete{false};std::thread child([&]{result=process.Run(std::wstring(system)+L"\\ping.exe",L"127.0.0.1 -n 30",stop);complete=true;});
    Sleep(200);check("cancellation fixture is actually running",!complete.load());const auto begin=GetTickCount64();stop=true;process.Cancel();child.join();
    check("owned background child cancels promptly",!result&&GetTickCount64()-begin<2000);stop=false;
    if(argc==4){
        const std::wstring directory=argv[1];
        const auto extracted=directory+L"\\extract-"+std::to_wstring(GetTickCount64());
        check("verified add-on extracts through the production installer path",ExtractRuntime(L"artifacts\\QuietShare-Speech-CPU-x64.zip",extracted,process,stop)&&SpeechFileHash(extracted+L"\\whisper-cli.exe")==SpeechExecutableHash);
        check("untrusted archive is rejected before extraction",!ExtractRuntime(L"README.md",extracted+L"-bad",process,stop));
        for(const wchar_t* name:{L"whisper-cli.exe",L"LICENSE-whisper.cpp.txt",L"LICENSE-ggml.txt",L"LICENSE-model.txt",L"README.md"})DeleteFileW((extracted+L"\\"+name).c_str());RemoveDirectoryW(extracted.c_str());
        check("optional CPU executable is pinned",SpeechFileHash(directory+L"\\Speech\\whisper-cli.exe")==SpeechExecutableHash);
        check("accuracy model size and runtime are ready",SpeechRuntimeReady(directory,1));
        check("accuracy model SHA256 matches official pin",SpeechFileHash(SpeechModelPath(directory,1))==SpeechModels[1].hash);
        for(int i=0;i<2;++i){std::wstring text,path;int code=0;const auto started=GetTickCount64();
            const bool ok=RunWhisper(directory,1,argv[i+2],i==0,stop,process,text,path,code);
            check(i==0?"English speech yields timed local transcript":"Vietnamese speech yields timed local transcript",ok&&code==0&&text.find(L"-->")!=std::wstring::npos);
            MeetingNotebook notes;const bool parsed=notes.Parse(text,path);std::wstring plain;for(const auto& segment:notes.segments)plain+=segment.text+L" ";
            const double wer=WordErrorRate(plain,L"một số loài động vật như voi và hươu cao cổ thường hay lại gần xe và chỉ với các dụng cụ bình thường chúng ta cũng sẽ quan sát một cách dễ dàng");
            check(i==0?"English model recognition contains expected speech":"Vietnamese fixture word error rate is below 40 percent",parsed&&
                (i==0?(text.find(L"country")!=std::wstring::npos&&text.find(L"you")!=std::wstring::npos):wer<=0.4));
            if(i==1){vietnameseWer=wer;std::cout<<"Vietnamese single fixture WER "<<wer<<std::endl;}
            check(i==0?"English recording retained after inference":"Vietnamese recording retained after inference",GetFileAttributesW(argv[i+2])!=INVALID_FILE_ATTRIBUTES);
            inferenceMs[i]=GetTickCount64()-started;std::cout<<"Inference milliseconds "<<inferenceMs[i]<<std::endl;
        }
        std::wstring text,path;int code=0;
        check("invalid WAV cannot start heavy inference",!RunWhisper(directory,1,L"missing.wav",true,stop,process,text,path,code)&&code==3);
    }else {notTested=1;std::cout<<"NOT TESTED optional model inference (local fixtures not installed)\n";}
    std::ofstream report("artifacts/qa-speech/speech-runtime-regression.json");report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed<<",\"notTested\":"<<notTested<<",\"singleVietnameseFixtureWer\":"<<vietnameseWer<<",\"inferenceMilliseconds\":["<<inferenceMs[0]<<","<<inferenceMs[1]<<"],\"checks\":["<<checks<<"]}\n";
    return failed?1:0;
}
