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

namespace quietshare {
inline bool DiagnosticPrivateMemory(HANDLE process,unsigned long long& bytes){
    bytes=0;using Fn=BOOL(WINAPI*)(HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD);
    static const Fn memory=[](){Fn function=nullptr;const FARPROC address=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"K32GetProcessMemoryInfo");static_assert(sizeof(function)==sizeof(address));std::memcpy(&function,&address,sizeof(function));return function;}();
    PROCESS_MEMORY_COUNTERS_EX info{};info.cb=sizeof(info);if(!memory||!memory(process,reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&info),sizeof(info)))return false;bytes=info.PrivateUsage;return true;
}
// A small fixed-field journal. Never accept free-form messages, paths or URLs.
struct DiagnosticEvent {
    DWORD sequence=0,group=0,number=0,phase=0,detail=0,hresult=0;
    DWORD year=0,month=0,day=0,hour=0,minute=0,second=0;
    DWORD major=0,minor=0,patch=0;
};
inline bool ValidDiagnosticCode(DWORD group,DWORD number){
    switch(group){case 'P':return number<=24||number==99;case 'B':return (number>=1&&number<=10)||number==99;
    case 'W':return number>=1&&number<=10;case 'D':return number>=1&&number<=5;case 'N':return number>=1&&number<=5;
    case 'R':return number>=1&&number<=9;case 'F':return number>=1&&number<=3;case 'J':return number>=1&&number<=3;default:return false;}
}
inline bool ValidDiagnosticEvent(const DiagnosticEvent& e){return e.sequence&&ValidDiagnosticCode(e.group,e.number)&&e.phase<=4&&e.year>=2020&&e.year<=9999&&e.month>=1&&e.month<=12&&e.day>=1&&e.day<=31&&e.hour<=23&&e.minute<=59&&e.second<=59&&e.major<=999&&e.minor<=999&&e.patch<=999;}
class DiagnosticHistory {
public:
    static constexpr size_t Limit=100;
    std::vector<DiagnosticEvent> events;
    bool Load(const std::wstring& path){
        HANDLE f=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(f==INVALID_HANDLE_VALUE){const auto error=GetLastError();if(error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND){events.clear();return true;}return false;}
        DWORD header[3]{},read=0;LARGE_INTEGER size{};
        bool ok=GetFileSizeEx(f,&size)&&size.QuadPart>=static_cast<LONGLONG>(sizeof(header))&&size.QuadPart<=static_cast<LONGLONG>(sizeof(header)+Limit*sizeof(DiagnosticEvent))&&ReadFile(f,header,sizeof(header),&read,nullptr)&&read==sizeof(header)&&header[0]==0x48515351&&header[1]==1&&header[2]<=Limit&&size.QuadPart==static_cast<LONGLONG>(sizeof(header)+header[2]*sizeof(DiagnosticEvent));
        std::vector<DiagnosticEvent> next;
        if(ok){next.resize(header[2]);const DWORD bytes=static_cast<DWORD>(next.size()*sizeof(DiagnosticEvent));ok=!bytes||(ReadFile(f,next.data(),bytes,&read,nullptr)&&read==bytes);}
        CloseHandle(f);if(!ok)return false;
        DWORD previous=0;for(const auto& e:next){if(!ValidDiagnosticEvent(e)||e.sequence<=previous)return false;previous=e.sequence;}
        events=std::move(next);return true;
    }
    bool Save(const std::wstring& path)const{
        if(events.size()>Limit)return false;
        DWORD previous=0;for(const auto& e:events){if(!ValidDiagnosticEvent(e)||e.sequence<=previous)return false;previous=e.sequence;}
        const auto temp=path+L".tmp";HANDLE f=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(f==INVALID_HANDLE_VALUE)return false;
        const DWORD header[]={0x48515351,1,static_cast<DWORD>(events.size())};DWORD written=0,bytes=static_cast<DWORD>(events.size()*sizeof(DiagnosticEvent));
        const bool ok=WriteFile(f,header,sizeof(header),&written,nullptr)&&written==sizeof(header)&&(!bytes||(WriteFile(f,events.data(),bytes,&written,nullptr)&&written==bytes))&&FlushFileBuffers(f);CloseHandle(f);
        if(!ok){DeleteFileW(temp.c_str());return false;}return MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
    }
    bool Add(const std::wstring& path,DWORD group,DWORD number,DWORD phase=0,DWORD detail=0,DWORD hr=0){
        if(!ValidDiagnosticCode(group,number)||phase>4||!Load(path))return false;
        SYSTEMTIME t{};GetSystemTime(&t);DiagnosticEvent e{events.empty()?1u:events.back().sequence+1,group,number,phase,detail,hr,t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,QUIETSHARE_VERSION_MAJOR,QUIETSHARE_VERSION_MINOR,QUIETSHARE_VERSION_PATCH};
        if(!e.sequence){events.clear();e.sequence=1;}
        if(events.size()>=Limit)events.erase(events.begin());
        events.push_back(e);return Save(path);
    }
    static const char* Phase(DWORD phase){const char* names[]={"event","start","verification","running","ui"};return phase<=4?names[phase]:"event";}
    std::string Json()const{
        for(const auto& e:events)if(!ValidDiagnosticEvent(e))return "[]";
        std::string out="[";for(size_t i=0;i<events.size();++i){const auto& e=events[i];char row[384]{};
            std::snprintf(row,sizeof(row),"%s{\"sequence\":%lu,\"utc\":\"%04lu-%02lu-%02luT%02lu:%02lu:%02luZ\",\"code\":\"QS-%c%03lu\",\"phase\":\"%s\",\"detail\":%lu,\"%s\":\"0x%08lx\",\"version\":\"%lu.%lu.%lu\"}",i?",\n":"\n",static_cast<unsigned long>(e.sequence),static_cast<unsigned long>(e.year),static_cast<unsigned long>(e.month),static_cast<unsigned long>(e.day),static_cast<unsigned long>(e.hour),static_cast<unsigned long>(e.minute),static_cast<unsigned long>(e.second),static_cast<char>(e.group),static_cast<unsigned long>(e.number),Phase(e.phase),static_cast<unsigned long>(e.detail),e.group=='B'?"exitCode":"hresult",static_cast<unsigned long>(e.hresult),static_cast<unsigned long>(e.major),static_cast<unsigned long>(e.minor),static_cast<unsigned long>(e.patch));out+=row;}
        return out+"\n]";
    }
    std::wstring Text()const{for(const auto& e:events)if(!ValidDiagnosticEvent(e))return L"";std::wstring out;for(auto it=events.rbegin();it!=events.rend();++it){const auto& e=*it;wchar_t row[256]{};
        swprintf_s(row,L"#%lu  %04lu-%02lu-%02lu %02lu:%02lu:%02lu UTC  QS-%c%03lu  %hs  detail=%lu  %hs=0x%08lx  v%lu.%lu.%lu\r\n",static_cast<unsigned long>(e.sequence),static_cast<unsigned long>(e.year),static_cast<unsigned long>(e.month),static_cast<unsigned long>(e.day),static_cast<unsigned long>(e.hour),static_cast<unsigned long>(e.minute),static_cast<unsigned long>(e.second),static_cast<wchar_t>(e.group),static_cast<unsigned long>(e.number),Phase(e.phase),static_cast<unsigned long>(e.detail),e.group=='B'?"Exit":"HRESULT",static_cast<unsigned long>(e.hresult),static_cast<unsigned long>(e.major),static_cast<unsigned long>(e.minor),static_cast<unsigned long>(e.patch));out+=row;}return out;}
};
inline std::wstring DiagnosticHistoryPath(const std::wstring& record){const auto at=record.find_last_of(L"\\/");return at==std::wstring::npos?L"":record.substr(0,at+1)+L"error-history.dat";}
inline bool& DiagnosticWriteFailed(){static bool value=false;return value;}
inline void RecordDiagnostic(const std::wstring& record,DWORD group,DWORD number,DWORD phase=0,DWORD detail=0,DWORD hr=0){const auto path=DiagnosticHistoryPath(record);if(!path.empty()){DiagnosticHistory history;if(!history.Add(path,group,number,phase,detail,hr))DiagnosticWriteFailed()=true;}}
}
