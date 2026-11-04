#pragma once
#include "Documents.h"
#include <wincrypt.h>

namespace quietshare {
struct PresentationFile { std::wstring path; ViewState view; };
struct PresentationSet {
    std::wstring name, folder, script;
    ViewState scriptView;
    std::vector<PresentationFile> files;
    unsigned active = 0; // 0 = script, otherwise one-based file index.
};
// Local, user-account encrypted snapshots. Documents themselves are never copied.
class PresentationSets {
    static constexpr DWORD MaxBytes=24*1024*1024;
    static bool Valid(const PresentationSet& s) {
        return !s.name.empty()&&s.name.size()<=80&&s.folder.size()<=32767&&
            s.script.size()<=500000&&s.files.size()<TabManager::MaxTabs&&s.active<=s.files.size()&&
            std::all_of(s.files.begin(),s.files.end(),[](const auto& f){return !f.path.empty()&&f.path.size()<=32767;});
    }
public:
    static constexpr size_t Limit=8;
    std::vector<PresentationSet> items;
    bool Save(const std::wstring& directory) const {
        if(items.size()>Limit)return false;
        std::vector<DWORD> words{0x53515351,1,static_cast<DWORD>(items.size())};
        auto text=[&](const std::wstring& s){words.push_back(static_cast<DWORD>(s.size()));for(wchar_t c:s)words.push_back(c);};
        auto view=[&](const ViewState& v){for(int n:{v.scrollX,v.scrollY,v.zoom,v.page,v.panX,v.panY,v.textPoints,v.wrap?1:0,v.fitWidth?1:0})words.push_back(static_cast<DWORD>(n));};
        for(const auto& s:items){if(!Valid(s))return false;text(s.name);text(s.folder);text(s.script);view(s.scriptView);
            words.push_back(s.active);words.push_back(static_cast<DWORD>(s.files.size()));for(const auto& f:s.files){text(f.path);view(f.view);}}
        if(words.size()*sizeof(DWORD)>MaxBytes)return false;
        DATA_BLOB in{static_cast<DWORD>(words.size()*sizeof(DWORD)),reinterpret_cast<BYTE*>(words.data())},out{};
        const bool encrypted=CryptProtectData(&in,L"QuietShare presentation sets",nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out)!=FALSE;
        SecureZeroMemory(words.data(),words.size()*sizeof(DWORD));if(!encrypted)return false;
        const auto target=directory+L"\\presentations.dpapi",temp=target+L".tmp";
        HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        DWORD written=0;bool ok=file!=INVALID_HANDLE_VALUE;
        if(ok){ok=WriteFile(file,out.pbData,out.cbData,&written,nullptr)&&written==out.cbData&&FlushFileBuffers(file);CloseHandle(file);}
        SecureZeroMemory(out.pbData,out.cbData);LocalFree(out.pbData);
        if(ok)ok=MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
        if(!ok)DeleteFileW(temp.c_str());
        return ok;
    }
    bool Load(const std::wstring& directory) {
        HANDLE file=CreateFileW((directory+L"\\presentations.dpapi").c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE)return false;
        LARGE_INTEGER size{};DWORD read=0;bool ok=GetFileSizeEx(file,&size)&&size.QuadPart>0&&size.QuadPart<=MaxBytes+65536;
        std::vector<BYTE> bytes(ok?static_cast<size_t>(size.QuadPart):0);
        if(ok)ok=ReadFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&read,nullptr)&&read==bytes.size();
        CloseHandle(file);if(!ok)return false;
        DATA_BLOB in{read,bytes.data()},out{};if(!CryptUnprotectData(&in,nullptr,nullptr,nullptr,nullptr,CRYPTPROTECT_UI_FORBIDDEN,&out))return false;
        const auto* w=reinterpret_cast<const DWORD*>(out.pbData);const size_t count=out.cbData/4;size_t at=3;
        ok=out.cbData<=MaxBytes&&out.cbData%4==0&&count>=3&&w[0]==0x53515351&&w[1]==1&&w[2]<=Limit;
        auto text=[&](std::wstring& s,size_t maximum){if(!ok||at>=count){ok=false;return;}const auto length=w[at++];
            if(length>maximum||length>count-at){ok=false;return;}for(DWORD j=0;j<length;++j){const auto c=w[at++];if(!c||c>65535){ok=false;return;}s+=static_cast<wchar_t>(c);}};
        auto view=[&](ViewState& v){if(!ok||count-at<9){ok=false;return;}v.scrollX=static_cast<int>(w[at++]);v.scrollY=static_cast<int>(w[at++]);
            v.zoom=static_cast<int>(w[at++]);v.page=static_cast<int>(w[at++]);v.panX=static_cast<int>(w[at++]);v.panY=static_cast<int>(w[at++]);v.textPoints=static_cast<int>(w[at++]);
            const auto wrap=w[at++],fit=w[at++];v.wrap=wrap!=0;v.fitWidth=fit!=0;
            ok=wrap<=1&&fit<=1&&v.zoom>=25&&v.zoom<=400&&v.textPoints>=6&&v.textPoints<=200&&v.page>=0&&v.page<10000&&v.scrollX>=0&&v.scrollY>=0;};
        std::vector<PresentationSet> next;
        const DWORD total=ok?w[2]:0;
        for(DWORD i=0;ok&&i<total;++i){PresentationSet s;text(s.name,80);text(s.folder,32767);text(s.script,500000);view(s.scriptView);
            if(!ok||count-at<2){ok=false;break;}s.active=w[at++];const auto n=w[at++];if(n>=TabManager::MaxTabs){ok=false;break;}
            for(DWORD j=0;ok&&j<n;++j){PresentationFile f;text(f.path,32767);view(f.view);s.files.push_back(std::move(f));}
            ok=ok&&Valid(s);if(ok)next.push_back(std::move(s));}
        ok=ok&&at==count;SecureZeroMemory(out.pbData,out.cbData);LocalFree(out.pbData);
        if(ok)items=std::move(next);
        return ok;
    }
};
}
