#include "SpeechRuntime.h"
#include "SpeechAssets.h"
#include "MeetingTranscriptImport.h"
#include <wincrypt.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>

namespace quietshare {
namespace {
std::wstring Quote(const std::wstring& value){std::wstring out=L"\"";size_t slashes=0;for(wchar_t c:value){if(c==L'\\'){++slashes;continue;}if(c==L'\"')out.append(slashes*2+1,L'\\');else out.append(slashes,L'\\');slashes=0;out+=c;}out.append(slashes*2,L'\\');return out+L"\"";}
std::wstring SystemTool(const wchar_t* name){wchar_t path[32768]{};const auto n=GetSystemDirectoryW(path,32768);return n&&n<32768?std::wstring(path)+L"\\"+name:L"";}
unsigned long long FileBytes(const std::wstring& path){WIN32_FILE_ATTRIBUTE_DATA a{};return GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&a)?(static_cast<unsigned long long>(a.nFileSizeHigh)<<32)|a.nFileSizeLow:0;}
std::wstring SpeechRoot(const std::wstring& directory){return directory+L"\\Speech";}
bool ExtractRuntime(const std::wstring& archive,const std::wstring& folder,SpeechProcess& process,const std::atomic<bool>& cancel){
    if(FileBytes(archive)!=SpeechArchiveBytes||SpeechFileHash(archive,&cancel)!=SpeechArchiveHash)return false;
    auto literal=[](std::wstring s){size_t at=0;while((at=s.find(L'\'',at))!=std::wstring::npos){s.insert(at,L"'");at+=2;}return L"'"+s+L"'";};
    const std::wstring script=L"$ErrorActionPreference='Stop'; Add-Type -AssemblyName System.IO.Compression.FileSystem; [IO.Compression.ZipFile]::ExtractToDirectory("+literal(archive)+L","+literal(folder)+L")";
    DWORD n=0;if(!CryptBinaryToStringW(reinterpret_cast<const BYTE*>(script.data()),static_cast<DWORD>(script.size()*2),CRYPT_STRING_BASE64|CRYPT_STRING_NOCRLF,nullptr,&n))return false;
    std::wstring encoded(n,L'\0');if(!CryptBinaryToStringW(reinterpret_cast<const BYTE*>(script.data()),static_cast<DWORD>(script.size()*2),CRYPT_STRING_BASE64|CRYPT_STRING_NOCRLF,encoded.data(),&n))return false;
    encoded.resize(wcslen(encoded.c_str()));
    return process.Run(SystemTool(L"WindowsPowerShell\\v1.0\\powershell.exe"),L"-NoProfile -NonInteractive -EncodedCommand "+encoded,cancel)&&SpeechFileHash(folder+L"\\whisper-cli.exe",&cancel)==SpeechExecutableHash;
}
bool ReadAudio(const std::wstring& path,std::vector<BYTE>& bytes){HANDLE f=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);if(f==INVALID_HANDLE_VALUE)return false;
    LARGE_INTEGER size{};DWORD read=0;bool ok=GetFileSizeEx(f,&size)&&size.QuadPart>=0&&size.QuadPart<=16*1024*1024;if(ok){bytes.resize(static_cast<size_t>(size.QuadPart));ok=ReadFile(f,bytes.data(),static_cast<DWORD>(bytes.size()),&read,nullptr)&&read==bytes.size();}CloseHandle(f);return ok;}
bool WriteExact(HANDLE file,const void* data,DWORD n){DWORD done=0;return WriteFile(file,data,n,&done,nullptr)&&done==n;}
// Stream the recorded WASAPI mix to the 16 kHz mono PCM16 required by the CLI.
// Memory stays bounded regardless of meeting length.
bool ConvertWave(const std::wstring& input,const std::wstring& output,const std::atomic<bool>& stop,bool* signal=nullptr){
    if(signal)*signal=false;
    HANDLE in=CreateFileW(input.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);if(in==INVALID_HANDLE_VALUE)return false;
    HANDLE out=INVALID_HANDLE_VALUE;bool ok=false;LARGE_INTEGER size{};DWORD read=0;BYTE header[12]{};std::vector<BYTE> format;
    unsigned long long dataOffset=0,dataBytes=0;
    if(!GetFileSizeEx(in,&size)||size.QuadPart<44||!ReadFile(in,header,12,&read,nullptr)||read!=12||memcmp(header,"RIFF",4)||memcmp(header+8,"WAVE",4))goto finish;
    for(unsigned long long pos=12;pos+8<=static_cast<unsigned long long>(size.QuadPart);){
        struct Chunk {char id[4];DWORD bytes;} c{};LARGE_INTEGER offset{};offset.QuadPart=pos;
        if(!SetFilePointerEx(in,offset,nullptr,FILE_BEGIN)||!ReadFile(in,&c,8,&read,nullptr)||read!=8||c.bytes>static_cast<unsigned long long>(size.QuadPart)-pos-8)goto finish;
        if(!memcmp(c.id,"fmt ",4)){if(c.bytes<16||c.bytes>65536)goto finish;format.resize(c.bytes);if(!ReadFile(in,format.data(),c.bytes,&read,nullptr)||read!=c.bytes)goto finish;}
        if(!memcmp(c.id,"data",4)&&!dataOffset){dataOffset=pos+8;dataBytes=c.bytes;}
        pos+=8ULL+c.bytes+(c.bytes&1);
    }
    if(format.size()<16||!dataOffset||!dataBytes)goto finish;
    {
        auto u16=[&](size_t at){WORD n;memcpy(&n,format.data()+at,2);return n;};auto u32=[&](size_t at){DWORD n;memcpy(&n,format.data()+at,4);return n;};
        WORD kind=u16(0),channels=u16(2),align=u16(12),bits=u16(14);const DWORD rate=u32(4);
        if(kind==0xfffe){if(format.size()<40)goto finish;kind=static_cast<WORD>(u32(24));}
        if(channels<1||channels>8||rate<8000||rate>192000||align!=channels*(bits/8)||(bits!=16&&bits!=24&&bits!=32)||(kind!=1&&kind!=3)||(kind==3&&bits!=32)||dataBytes%align)goto finish;
        const auto frames=dataBytes/align;const auto samples=frames*16000/rate;if(!samples||samples>0x7fffffff/2)goto finish;
        out=CreateFileW(output.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(out==INVALID_HANDLE_VALUE)goto finish;
        BYTE wav[44]{};memcpy(wav,"RIFF",4);DWORD n=static_cast<DWORD>(36+samples*2);memcpy(wav+4,&n,4);memcpy(wav+8,"WAVEfmt ",8);n=16;memcpy(wav+16,&n,4);
        WORD w=1;memcpy(wav+20,&w,2);memcpy(wav+22,&w,2);n=16000;memcpy(wav+24,&n,4);n=32000;memcpy(wav+28,&n,4);w=2;memcpy(wav+32,&w,2);w=16;memcpy(wav+34,&w,2);memcpy(wav+36,"data",4);n=static_cast<DWORD>(samples*2);memcpy(wav+40,&n,4);
        if(!WriteExact(out,wav,44))goto finish;
        LARGE_INTEGER offset{};offset.QuadPart=dataOffset;if(!SetFilePointerEx(in,offset,nullptr,FILE_BEGIN))goto finish;
        std::vector<BYTE> buffer(4096*align);std::vector<short> pcm;pcm.reserve(8192);unsigned long long index=0,written=0;double next=0;const double step=double(rate)/16000;float previous=0;
        while(index<frames&&!stop.load()){
            const DWORD batch=static_cast<DWORD>(std::min<unsigned long long>(4096,frames-index));if(!ReadFile(in,buffer.data(),batch*align,&read,nullptr)||read!=batch*align)goto finish;
            pcm.clear();for(DWORD i=0;i<batch;++i,++index){double mono=0;
                for(WORD c=0;c<channels;++c){const auto* p=buffer.data()+i*align+c*(bits/8);double v=0;
                    if(kind==3){float f;memcpy(&f,p,4);v=std::isfinite(f)?f:0;}else if(bits==16){short x;memcpy(&x,p,2);v=x/32768.0;}
                    else if(bits==24){int x=p[0]|(p[1]<<8)|(p[2]<<16);if(x&0x800000)x|=~0xffffff;v=x/8388608.0;}else{int32_t x;memcpy(&x,p,4);v=x/2147483648.0;}mono+=v;}
                mono/=channels;if(signal&&std::abs(mono)>0.00003)*signal=true;
                while(written<samples&&next<=index){const double v=index?previous+(mono-previous)*(next-(index-1)):mono;pcm.push_back(static_cast<short>(std::clamp(v,-1.0,1.0)*32767));++written;next+=step;}previous=static_cast<float>(mono);}
            if(!pcm.empty()&&!WriteExact(out,pcm.data(),static_cast<DWORD>(pcm.size()*2)))goto finish;
        }
        while(written<samples&&!stop.load()){short sample=static_cast<short>(std::clamp(previous,-1.0f,1.0f)*32767);if(!WriteExact(out,&sample,2))goto finish;++written;}
        ok=!stop.load()&&written==samples&&FlushFileBuffers(out);
    }
finish:
    CloseHandle(in);if(out!=INVALID_HANDLE_VALUE)CloseHandle(out);if(!ok)DeleteFileW(output.c_str());return ok;
}
}
std::wstring SpeechFileHash(const std::wstring& path,const std::atomic<bool>* cancel){
    HCRYPTPROV provider=0;HCRYPTHASH hash=0;HANDLE file=INVALID_HANDLE_VALUE;std::wstring result;
    if(!CryptAcquireContextW(&provider,nullptr,nullptr,PROV_RSA_AES,CRYPT_VERIFYCONTEXT)||!CryptCreateHash(provider,CALG_SHA_256,0,0,&hash))goto finish;
    file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)goto finish;
    {BYTE buffer[65536];DWORD n=0;for(;;){if(cancel&&cancel->load())goto finish;if(!ReadFile(file,buffer,sizeof(buffer),&n,nullptr))goto finish;if(!n)break;if(!CryptHashData(hash,buffer,n,0))goto finish;}
        BYTE digest[32];n=sizeof(digest);if(!CryptGetHashParam(hash,HP_HASHVAL,digest,&n,0))goto finish;const wchar_t* hex=L"0123456789abcdef";for(BYTE c:digest){result+=hex[c>>4];result+=hex[c&15];}}
finish:if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);if(hash)CryptDestroyHash(hash);if(provider)CryptReleaseContext(provider,0);return result;
}
std::wstring SpeechModelPath(const std::wstring& directory,int model){return SpeechRoot(directory)+L"\\"+SpeechModels[std::clamp(model,0,1)].name;}
bool SpeechRuntimeReady(const std::wstring& directory,int model){if(directory.empty()||model<0||model>1)return false;
    return FileBytes(SpeechModelPath(directory,model))==SpeechModels[model].bytes&&SpeechFileHash(SpeechRoot(directory)+L"\\whisper-cli.exe")==SpeechExecutableHash;}
void SpeechProcess::Cancel(){std::lock_guard<std::mutex> guard(mutex);if(active)TerminateProcess(active,ERROR_CANCELLED);}
bool SpeechProcess::Run(const std::wstring& exe,const std::wstring& args,const std::atomic<bool>& cancel,const std::wstring& watched,unsigned long long maximum,std::atomic<unsigned>* progress){
    if(exe.empty()||cancel.load())return false;SECURITY_ATTRIBUTES sa{sizeof(sa),nullptr,TRUE};HANDLE null=CreateFileW(L"NUL",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,0,nullptr);
    HANDLE job=CreateJobObjectW(nullptr,nullptr);if(null==INVALID_HANDLE_VALUE||!job){if(null!=INVALID_HANDLE_VALUE)CloseHandle(null);if(job)CloseHandle(job);return false;}
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE|JOB_OBJECT_LIMIT_PROCESS_MEMORY;limits.ProcessMemoryLimit=2ULL*1024*1024*1024;
    if(!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){CloseHandle(null);CloseHandle(job);return false;}
    STARTUPINFOW start{};start.cb=sizeof(start);start.dwFlags=STARTF_USESTDHANDLES;start.hStdInput=start.hStdOutput=start.hStdError=null;PROCESS_INFORMATION p{};
    std::wstring command=Quote(exe)+L" "+args;
    bool ok=CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED|BELOW_NORMAL_PRIORITY_CLASS,nullptr,nullptr,&start,&p)!=FALSE;
    if(ok){ok=AssignProcessToJobObject(job,p.hProcess)!=FALSE;if(!ok)TerminateProcess(p.hProcess,ERROR_ACCESS_DENIED);
        {std::lock_guard<std::mutex> guard(mutex);active=p.hProcess;}if(ok&&ResumeThread(p.hThread)==static_cast<DWORD>(-1)){TerminateProcess(p.hProcess,ERROR_ACCESS_DENIED);ok=false;}
        while(WaitForSingleObject(p.hProcess,100)==WAIT_TIMEOUT){if(cancel.load()){Cancel();ok=false;}if(maximum&&!watched.empty()){
                const auto bytes=FileBytes(watched);if(progress)progress->store(static_cast<unsigned>(std::min<unsigned long long>(99,bytes*100/maximum)));if(bytes>maximum){Cancel();ok=false;}}}
        DWORD exit=1;GetExitCodeProcess(p.hProcess,&exit);ok=ok&&exit==0&&!cancel.load();{std::lock_guard<std::mutex> guard(mutex);active=nullptr;}CloseHandle(p.hThread);CloseHandle(p.hProcess);}
    CloseHandle(null);CloseHandle(job);return ok;
}
bool SpeechInstaller::Start(const std::wstring& directory,int model){if(running.load()||directory.empty()||model<0||model>1)return false;Stop();cancel=false;error=0;progress=0;running=true;worker=std::thread([this,directory,model]{Install(directory,model);running=false;});return true;}
void SpeechInstaller::Stop(){cancel=true;process.Cancel();if(worker.joinable())worker.join();running=false;}
void SpeechInstaller::Install(const std::wstring& directory,int model){
    const auto root=SpeechRoot(directory),stage=root+L"\\setup-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());std::error_code ec;
    std::filesystem::create_directories(stage,ec);if(ec){error=8;return;}
    auto download=[&](const std::wstring& url,const std::wstring& file,unsigned long long bytes,const wchar_t* hash){
        if(!process.Run(SystemTool(L"curl.exe"),L"--location --fail --silent --show-error --connect-timeout 15 --max-time 1800 --output "+Quote(file)+L" "+Quote(url),cancel,file,bytes,&progress))return false;
        return FileBytes(file)==bytes&&SpeechFileHash(file,&cancel)==hash;};
    const auto archive=stage+L"\\runtime.zip";bool ok=download(SpeechArchiveUrl,archive,SpeechArchiveBytes,SpeechArchiveHash);
    if(ok){const auto b=stage+L"\\files";ok=ExtractRuntime(archive,b,process,cancel);
        if(ok){for(const wchar_t* name:{L"whisper-cli.exe",L"LICENSE-whisper.cpp.txt",L"LICENSE-ggml.txt",L"LICENSE-model.txt",L"README.md"}){
                if(!MoveFileExW((b+L"\\"+name).c_str(),(root+L"\\"+name).c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){ok=false;break;}}}}
    if(ok&&!cancel.load()){
        const auto modelTemp=stage+L"\\model.part";const auto& asset=SpeechModels[model];const std::wstring url=L"https://huggingface.co/ggerganov/whisper.cpp/resolve/"+std::wstring(SpeechModelRevision)+L"/"+asset.name;
        if(FileBytes(SpeechModelPath(directory,model))!=asset.bytes||SpeechFileHash(SpeechModelPath(directory,model),&cancel)!=asset.hash)
            ok=download(url,modelTemp,asset.bytes,asset.hash)&&MoveFileExW(modelTemp.c_str(),SpeechModelPath(directory,model).c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
    }
    error=ok&&!cancel.load()?0:cancel.load()?6:8;progress=ok?100:progress.load();
    // Delete only the fixed files created here. Never recursively traverse a
    // user-writable directory or a junction during cancellation or repair.
    for(const wchar_t* name:{L"runtime.zip",L"model.part"})DeleteFileW((stage+L"\\"+name).c_str());
    for(const wchar_t* name:{L"whisper-cli.exe",L"LICENSE-whisper.cpp.txt",L"LICENSE-ggml.txt",L"LICENSE-model.txt",L"README.md"})DeleteFileW((stage+L"\\files\\"+name).c_str());
    RemoveDirectoryW((stage+L"\\files").c_str());RemoveDirectoryW(stage.c_str());
}
bool RunWhisper(const std::wstring& directory,int model,const std::wstring& wav,bool english,std::atomic<bool>& stop,SpeechProcess& process,std::wstring& text,std::wstring& output,int& code){
    if(!SpeechRuntimeReady(directory,model)||SpeechFileHash(SpeechModelPath(directory,model),&stop)!=SpeechModels[model].hash){code=stop?6:8;return false;}
    const auto temporary=wav+L".speech-"+std::to_wstring(GetTickCount64())+L".wav";const auto stem=wav+L".transcript-"+std::to_wstring(GetTickCount64());
    bool signal=false;if(!ConvertWave(wav,temporary,stop,&signal)){code=stop?6:3;return false;}
    if(!signal){DeleteFileW(temporary.c_str());code=5;return false;}
    const bool ok=process.Run(SpeechRoot(directory)+L"\\whisper-cli.exe",L"-m "+Quote(SpeechModelPath(directory,model))+L" -f "+Quote(temporary)+L" -l "+(english?L"en":L"vi")+L" -t 2 -p 1 -ng -sns -otxt -ovtt -of "+Quote(stem),stop);
    DeleteFileW(temporary.c_str());if(!ok){code=stop?6:9;return false;}
    std::vector<BYTE> bytes;output=stem+L".vtt";if(!ReadAudio(output,bytes)){code=7;return false;}text=DecodeMeetingTranscript(bytes);code=text.empty()?5:0;return code==0;
}
}
