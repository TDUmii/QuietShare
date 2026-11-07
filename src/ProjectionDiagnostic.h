#pragma once
#include <windows.h>
#include <cstdio>
#include <cstring>
#include "Mirror.h"
#include <string>
#include <algorithm>
#include <cwctype>

namespace quietshare {
// Parse only the fixed, bounded record written by Mirror. Never display or
// copy arbitrary text from a user-writable diagnostic file.
inline std::wstring ReadProjectionDiagnostic(const std::wstring& path){
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return L"";
    LARGE_INTEGER size{};char data[1025]{};DWORD read=0;
    const bool ok=GetFileSizeEx(file,&size)&&size.QuadPart>0&&size.QuadPart<=1024&&ReadFile(file,data,static_cast<DWORD>(size.QuadPart),&read,nullptr)&&read==size.QuadPart;
    CloseHandle(file);if(!ok)return L"";
    char version[32]{},utc[32]{},code[16]{},phase[16]{},reason[64]{};unsigned long hr=0;unsigned long long elapsed=0;int mode=0,consumed=-1;
    const int fields=std::sscanf(data,"{\"schema\":1,\"version\":\"%31[0-9.]\",\"utc\":\"%31[0-9T:Z-]\",\"code\":\"%15[QSP0-9-]\",\"phase\":\"%15[a-z]\",\"reason\":\"%63[a-z-]\",\"hresult\":\"0x%8lx\",\"sessionMs\":%llu,\"resourceMode\":%d}%n",version,utc,code,phase,reason,&hr,&elapsed,&mode,&consumed);
    if(fields!=8||consumed<0||static_cast<unsigned>(consumed)>read||mode<0||mode>2)return L"";
    for(DWORD i=consumed;i<read;++i)if(data[i]!='\r'&&data[i]!='\n'&&data[i]!=' '&&data[i]!='\t')return L"";
    if(strcmp(phase,"start")&&strcmp(phase,"verification")&&strcmp(phase,"running"))return L"";
    if(strcmp(code,ProjectionErrorCode(reason))!=0)return L"";
    const std::string timestamp=utc;if(timestamp.size()!=20||timestamp[4]!='-'||timestamp[7]!='-'||timestamp[10]!='T'||timestamp[13]!=':'||timestamp[16]!=':'||timestamp[19]!='Z')return L"";
    auto wide=[](const char* value){std::wstring out;for(;*value;++value)out+=static_cast<unsigned char>(*value);return out;};
    wchar_t hex[16]{};swprintf_s(hex,L"0x%08lx",hr);
    return wide(code)+L"\r\nVersion: "+wide(version)+L"\r\nUTC: "+wide(utc)+L"\r\nPhase: "+wide(phase)+L"\r\nReason: "+wide(reason)+L"\r\nHRESULT: "+hex+
        L"\r\nSession ms: "+std::to_wstring(elapsed)+L"\r\nResource mode: "+std::to_wstring(mode);
}
}
