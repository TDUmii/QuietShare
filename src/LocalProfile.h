#pragma once
#include <windows.h>
#include <wincrypt.h>
#include <string>
#include <vector>

namespace quietshare {
// Small local metadata, encrypted for the Windows user and replaced atomically.
struct ProfileWords {
    std::vector<DWORD> words;
    size_t at=0;
    bool ok=true;
    ~ProfileWords(){if(!words.empty())SecureZeroMemory(words.data(),words.size()*sizeof(DWORD));}
    void Put(DWORD n){words.push_back(n);}
    void Put(const std::wstring& s){Put(static_cast<DWORD>(s.size()));for(wchar_t c:s)Put(c);}
    DWORD Get(){if(at>=words.size()){ok=false;return 0;}return words[at++];}
    std::wstring Text(size_t limit){const DWORD n=Get();std::wstring s;
        if(!ok||n>limit||n>words.size()-at){ok=false;return s;}
        s.reserve(n);for(DWORD i=0;i<n;++i){const DWORD c=Get();if(!c||c>65535){ok=false;return {}; }s+=static_cast<wchar_t>(c);}return s;}
    bool Done()const{return ok&&at==words.size();}
};
inline bool WritePrivateProfile(const std::wstring& path,ProfileWords& data,DWORD maximum=16*1024*1024){
    if(!data.ok||data.words.size()*sizeof(DWORD)>maximum)return false;
    DATA_BLOB in{static_cast<DWORD>(data.words.size()*sizeof(DWORD)),reinterpret_cast<BYTE*>(data.words.data())},out{};
    if(!CryptProtectData(&in,L"QuietShare local workspace",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out))return false;
    const auto temporary=path+L".tmp";DWORD written=0;
    HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    bool ok=file!=INVALID_HANDLE_VALUE;
    if(ok){ok=WriteFile(file,out.pbData,out.cbData,&written,nullptr)&&written==out.cbData&&FlushFileBuffers(file);CloseHandle(file);}
    SecureZeroMemory(out.pbData,out.cbData);LocalFree(out.pbData);
    if(ok)ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
    if(!ok)DeleteFileW(temporary.c_str());
    return ok;
}
inline bool ReadPrivateProfile(const std::wstring& path,ProfileWords& data,DWORD maximum=16*1024*1024){
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return false;
    LARGE_INTEGER size{};DWORD read=0;bool ok=GetFileSizeEx(file,&size)&&size.QuadPart>0&&size.QuadPart<=maximum+65536;
    std::vector<BYTE> bytes(ok?static_cast<size_t>(size.QuadPart):0);
    if(ok)ok=ReadFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&read,nullptr)&&read==bytes.size();
    CloseHandle(file);if(!ok)return false;
    DATA_BLOB in{read,bytes.data()},out{};
    if(!CryptUnprotectData(&in,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out))return false;
    ok=out.cbData<=maximum&&out.cbData%sizeof(DWORD)==0;
    if(ok){const auto* first=reinterpret_cast<DWORD*>(out.pbData);data.words.assign(first,first+out.cbData/sizeof(DWORD));data.at=0;data.ok=true;}
    SecureZeroMemory(out.pbData,out.cbData);LocalFree(out.pbData);return ok;
}
}
