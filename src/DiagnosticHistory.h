#pragma once
#include <windows.h>
#include "Version.h"
#include <array>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <psapi.h>
#include <algorithm>
#include <cstddef>

namespace quietshare {
inline bool DiagnosticPrivateMemory(HANDLE process,unsigned long long& bytes){
    bytes=0;using Fn=BOOL(WINAPI*)(HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD);
    static const Fn memory=[](){Fn function=nullptr;const FARPROC address=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetProcessMemoryInfo");static_assert(sizeof(function)==sizeof(address));std::memcpy(&function,&address,sizeof(function));return function;}();
    PROCESS_MEMORY_COUNTERS_EX info{};info.cb=sizeof(info);if(!memory||!memory(process,reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&info),sizeof(info)))return false;bytes=info.PrivateUsage;return true;
}
// A small fixed-field journal. Never accept free-form messages, paths or URLs.
struct DiagnosticMemory {
    DWORD commitLimitMiB=0,availableCommitMiB=0,physicalMiB=0,availablePhysicalMiB=0;
    bool Known()const{return commitLimitMiB&&physicalMiB;}
    bool Valid()const{return Known()?(availableCommitMiB<=commitLimitMiB&&availablePhysicalMiB<=physicalMiB):(!commitLimitMiB&&!availableCommitMiB&&!physicalMiB&&!availablePhysicalMiB);}
    bool Low()const{return Known()&&availableCommitMiB<std::max<DWORD>(512,commitLimitMiB/50);}
    bool Recovered()const{return Known()&&availableCommitMiB>=std::max<DWORD>(1024,commitLimitMiB/25);}
};
inline DiagnosticMemory ReadDiagnosticMemory(){MEMORYSTATUSEX state{};state.dwLength=sizeof(state);if(!GlobalMemoryStatusEx(&state))return {};
    auto mib=[](ULONGLONG value){return static_cast<DWORD>(std::min<ULONGLONG>(value>>20,0xffffffffULL));};
    return {mib(state.ullTotalPageFile),mib(state.ullAvailPageFile),mib(state.ullTotalPhys),mib(state.ullAvailPhys)};
}
struct DiagnosticEvent {
    DWORD sequence=0,group=0,number=0,phase=0,detail=0,hresult=0;
    DWORD year=0,month=0,day=0,hour=0,minute=0,second=0;
    DWORD major=0,minor=0,patch=0;
    DiagnosticMemory memory{};
};
static_assert(offsetof(DiagnosticEvent,memory)==60&&sizeof(DiagnosticEvent)==76,"Versioned fixed-field history layout");
inline bool ValidDiagnosticCode(DWORD group,DWORD number){
    switch(group){case 'P':return number<=24||number==99;case 'B':return (number>=1&&number<=11)||number==99;
    case 'W':return number>=1&&number<=10;case 'D':return number>=1&&number<=5;case 'N':return number>=1&&number<=5;
    case 'R':return number>=1&&number<=9;case 'F':return number>=1&&number<=3;case 'J':return number>=1&&number<=3;default:return false;}
}
inline bool ValidDiagnosticEvent(const DiagnosticEvent& e){return e.sequence&&ValidDiagnosticCode(e.group,e.number)&&e.phase<=4&&e.year>=2020&&e.year<=9999&&e.month>=1&&e.month<=12&&e.day>=1&&e.day<=31&&e.hour<=23&&e.minute<=59&&e.second<=59&&e.major<=999&&e.minor<=999&&e.patch<=999&&e.memory.Valid();}
class DiagnosticHistory {
public:
    static constexpr size_t Limit=100;
    std::vector<DiagnosticEvent> events;
    bool Load(const std::wstring& path){
        HANDLE f=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(f==INVALID_HANDLE_VALUE){const auto error=GetLastError();if(error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND){events.clear();return true;}return false;}
        DWORD header[3]{},read=0;LARGE_INTEGER size{};
        bool ok=GetFileSizeEx(f,&size)&&size.QuadPart>=static_cast<LONGLONG>(sizeof(header))&&size.QuadPart<=static_cast<LONGLONG>(sizeof(header)+Limit*sizeof(DiagnosticEvent))&&ReadFile(f,header,sizeof(header),&read,nullptr)&&read==sizeof(header)&&header[0]==0x48515351&&(header[1]==1||header[1]==2)&&header[2]<=Limit;
        const DWORD eventBytes=header[1]==1?60:sizeof(DiagnosticEvent);
        ok=ok&&size.QuadPart==static_cast<LONGLONG>(sizeof(header)+header[2]*eventBytes);
        std::vector<DiagnosticEvent> next;
        if(ok){next.resize(header[2]);if(header[1]==2){const DWORD bytes=static_cast<DWORD>(next.size()*sizeof(DiagnosticEvent));ok=!bytes||(ReadFile(f,next.data(),bytes,&read,nullptr)&&read==bytes);}
            else for(auto& e:next){if(!ReadFile(f,&e,eventBytes,&read,nullptr)||read!=eventBytes){ok=false;break;}}}
        CloseHandle(f);if(!ok)return false;
        DWORD previous=0;for(const auto& e:next){if(!ValidDiagnosticEvent(e)||e.sequence<=previous)return false;previous=e.sequence;}
        events=std::move(next);return true;
    }
    bool Save(const std::wstring& path)const{
        if(events.size()>Limit)return false;
        DWORD previous=0;for(const auto& e:events){if(!ValidDiagnosticEvent(e)||e.sequence<=previous)return false;previous=e.sequence;}
        const auto temp=path+L".tmp";HANDLE f=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(f==INVALID_HANDLE_VALUE)return false;
        const DWORD header[]={0x48515351,2,static_cast<DWORD>(events.size())};DWORD written=0,bytes=static_cast<DWORD>(events.size()*sizeof(DiagnosticEvent));
        const bool ok=WriteFile(f,header,sizeof(header),&written,nullptr)&&written==sizeof(header)&&(!bytes||(WriteFile(f,events.data(),bytes,&written,nullptr)&&written==bytes))&&FlushFileBuffers(f);CloseHandle(f);
        if(!ok){DeleteFileW(temp.c_str());return false;}return MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
    }
    bool Add(const std::wstring& path,DWORD group,DWORD number,DWORD phase=0,DWORD detail=0,DWORD hr=0){
        if(!ValidDiagnosticCode(group,number)||phase>4||!Load(path))return false;
        SYSTEMTIME t{};GetSystemTime(&t);DiagnosticEvent e{events.empty()?1u:events.back().sequence+1,group,number,phase,detail,hr,t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,QUIETSHARE_VERSION_MAJOR,QUIETSHARE_VERSION_MINOR,QUIETSHARE_VERSION_PATCH};
        e.memory=ReadDiagnosticMemory();
        if(!e.sequence){events.clear();e.sequence=1;}
        if(events.size()>=Limit)events.erase(events.begin());
        events.push_back(e);return Save(path);
    }
    static const char* Phase(DWORD phase){const char* names[]={"event","start","verification","running","ui"};return phase<=4?names[phase]:"event";}
    std::string Json()const{
        for(const auto& e:events)if(!ValidDiagnosticEvent(e))return "[]";
        std::string out="[";for(size_t i=0;i<events.size();++i){const auto& e=events[i];char row[640]{};
            std::snprintf(row,sizeof(row),"%s{\"sequence\":%lu,\"utc\":\"%04lu-%02lu-%02luT%02lu:%02lu:%02luZ\",\"code\":\"QS-%c%03lu\",\"phase\":\"%s\",\"detail\":%lu,\"%s\":\"0x%08lx\",\"version\":\"%lu.%lu.%lu\"",i?",\n":"\n",static_cast<unsigned long>(e.sequence),static_cast<unsigned long>(e.year),static_cast<unsigned long>(e.month),static_cast<unsigned long>(e.day),static_cast<unsigned long>(e.hour),static_cast<unsigned long>(e.minute),static_cast<unsigned long>(e.second),static_cast<char>(e.group),static_cast<unsigned long>(e.number),Phase(e.phase),static_cast<unsigned long>(e.detail),e.group=='B'?"exitCode":"hresult",static_cast<unsigned long>(e.hresult),static_cast<unsigned long>(e.major),static_cast<unsigned long>(e.minor),static_cast<unsigned long>(e.patch));out+=row;
            std::snprintf(row,sizeof(row),",\"memoryKnown\":%s,\"commitLimitMiB\":%lu,\"availableCommitMiB\":%lu,\"physicalMiB\":%lu,\"availablePhysicalMiB\":%lu}",e.memory.Known()?"true":"false",static_cast<unsigned long>(e.memory.commitLimitMiB),static_cast<unsigned long>(e.memory.availableCommitMiB),static_cast<unsigned long>(e.memory.physicalMiB),static_cast<unsigned long>(e.memory.availablePhysicalMiB));out+=row;}
        return out+"\n]";
    }
    std::wstring Text()const{for(const auto& e:events)if(!ValidDiagnosticEvent(e))return L"";std::wstring out;for(auto it=events.rbegin();it!=events.rend();++it){const auto& e=*it;wchar_t row[384]{};
        swprintf_s(row,L"#%lu  %04lu-%02lu-%02lu %02lu:%02lu:%02lu UTC  QS-%c%03lu  %hs  detail=%lu  %hs=0x%08lx  v%lu.%lu.%lu\r\n",static_cast<unsigned long>(e.sequence),static_cast<unsigned long>(e.year),static_cast<unsigned long>(e.month),static_cast<unsigned long>(e.day),static_cast<unsigned long>(e.hour),static_cast<unsigned long>(e.minute),static_cast<unsigned long>(e.second),static_cast<wchar_t>(e.group),static_cast<unsigned long>(e.number),Phase(e.phase),static_cast<unsigned long>(e.detail),e.group=='B'?"Exit":"HRESULT",static_cast<unsigned long>(e.hresult),static_cast<unsigned long>(e.major),static_cast<unsigned long>(e.minor),static_cast<unsigned long>(e.patch));out+=row;
        if(e.memory.Known()){swprintf_s(row,L"  Available commit: %lu / %lu MiB. Available physical RAM: %lu / %lu MiB.\r\n",static_cast<unsigned long>(e.memory.availableCommitMiB),static_cast<unsigned long>(e.memory.commitLimitMiB),static_cast<unsigned long>(e.memory.availablePhysicalMiB),static_cast<unsigned long>(e.memory.physicalMiB));out+=row;}
        else out+=L"  Memory snapshot unavailable for this event.\r\n";}return out;}
};
inline std::wstring DiagnosticHistoryPath(const std::wstring& record){const auto at=record.find_last_of(L"\\/");return at==std::wstring::npos?L"":record.substr(0,at+1)+L"error-history.dat";}
inline bool& DiagnosticWriteFailed(){static bool value=false;return value;}
inline void RecordDiagnostic(const std::wstring& record,DWORD group,DWORD number,DWORD phase=0,DWORD detail=0,DWORD hr=0){const auto path=DiagnosticHistoryPath(record);if(!path.empty()){DiagnosticHistory history;if(!history.Add(path,group,number,phase,detail,hr))DiagnosticWriteFailed()=true;}}
}
