#pragma once
#include <windows.h>
#include <wincrypt.h>
#include <algorithm>
#include <string>
#include <vector>

namespace quietshare {
struct RecentEntry { std::wstring path; bool folder=false,pinned=false; };
// Local profile only. DPAPI prevents filenames from being stored as plaintext.
class RecentWorkspace {
public:
    static constexpr size_t Limit=32;
    std::vector<RecentEntry> entries;
    void Touch(const std::wstring& path,bool folder) {
        auto it=std::find_if(entries.begin(),entries.end(),[&](const auto& e){return _wcsicmp(e.path.c_str(),path.c_str())==0;});
        const bool pinned=it!=entries.end()&&it->pinned;
        if(it!=entries.end())entries.erase(it);
        entries.insert(entries.begin(),{path,folder,pinned});Sort();
        if(entries.size()>Limit)entries.resize(Limit);
    }
    void Sort(){std::stable_partition(entries.begin(),entries.end(),[](const auto& e){return e.pinned;});}
    bool Save(const std::wstring& directory)const {
        std::vector<DWORD> words{0x52515351,1,static_cast<DWORD>(entries.size())};
        for(const auto& e:entries){
            if(e.path.empty()||e.path.size()>32767)return false;
            words.push_back((e.folder?1u:0u)|(e.pinned?2u:0u));words.push_back(static_cast<DWORD>(e.path.size()));
            for(wchar_t c:e.path)words.push_back(c);
        }
        DATA_BLOB input{static_cast<DWORD>(words.size()*sizeof(DWORD)),reinterpret_cast<BYTE*>(words.data())},output{};
        if(!CryptProtectData(&input,L"QuietShare workspace",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&output))return false;
        const auto target=directory+L"\\recent.dpapi",temporary=target+L".tmp";
        HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        DWORD written=0;bool ok=file!=INVALID_HANDLE_VALUE;
        if(ok){ok=WriteFile(file,output.pbData,output.cbData,&written,nullptr)&&written==output.cbData&&FlushFileBuffers(file);CloseHandle(file);}
        SecureZeroMemory(output.pbData,output.cbData);LocalFree(output.pbData);
        SecureZeroMemory(words.data(),words.size()*sizeof(DWORD));
        if(ok)ok=MoveFileExW(temporary.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
        if(!ok)DeleteFileW(temporary.c_str());
        return ok;
    }
    bool Load(const std::wstring& directory) {
        HANDLE file=CreateFileW((directory+L"\\recent.dpapi").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE)return false;
        LARGE_INTEGER size{};DWORD read=0;
        bool ok=GetFileSizeEx(file,&size)&&size.QuadPart>0&&size.QuadPart<=5*1024*1024;
        std::vector<BYTE> bytes(ok?static_cast<size_t>(size.QuadPart):0);
        if(ok)ok=ReadFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&read,nullptr)&&read==bytes.size();
        CloseHandle(file);
        if(!ok)return false;
        DATA_BLOB input{read,bytes.data()},output{};
        if(!CryptUnprotectData(&input,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&output))return false;
        std::vector<RecentEntry> next;
        const DWORD* words=reinterpret_cast<const DWORD*>(output.pbData);const size_t count=output.cbData/sizeof(DWORD);
        ok=output.cbData%sizeof(DWORD)==0&&count>=3&&words[0]==0x52515351&&words[1]==1&&words[2]<=Limit;
        size_t at=3;
        for(DWORD i=0;ok&&i<words[2];++i){
            if(at+2>count){ok=false;break;}
            const DWORD flags=words[at++],length=words[at++];
            if(flags>3||!length||length>32767||length>count-at){ok=false;break;}
            RecentEntry e;e.folder=flags&1;e.pinned=flags&2;
            for(DWORD j=0;j<length;++j){const DWORD c=words[at++];if(!c||c>65535){ok=false;break;}e.path+=static_cast<wchar_t>(c);}
            if(ok)next.push_back(std::move(e));
        }
        ok=ok&&at==count;SecureZeroMemory(output.pbData,output.cbData);LocalFree(output.pbData);
        if(ok){entries=std::move(next);Sort();}return ok;
    }
};
inline SIZE FitRememberedSize(SIZE logical,UINT dpi,RECT work) {
    return {std::clamp<LONG>(MulDiv(logical.cx,static_cast<int>(dpi),96),std::min(430L,work.right-work.left),work.right-work.left),
            std::clamp<LONG>(MulDiv(logical.cy,static_cast<int>(dpi),96),std::min(320L,work.bottom-work.top),work.bottom-work.top)};
}
}
