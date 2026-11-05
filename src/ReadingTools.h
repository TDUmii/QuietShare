#pragma once
#include "LocalProfile.h"
#include <algorithm>

namespace quietshare {
struct ScriptSection {std::wstring title;size_t begin=0,end=0;unsigned occurrence=0;};
inline std::vector<ScriptSection> OutlineScript(const std::wstring& script){
    std::vector<ScriptSection> sections;size_t start=0;
    while(start<script.size()&&sections.size()<128){size_t end=script.find(L'\n',start);if(end==std::wstring::npos)end=script.size();
        size_t at=start;while(at<end&&(script[at]==L' '||script[at]==L'\t'))++at;
        size_t marks=at;while(at<end&&script[at]==L'#')++at;
        if(at>marks&&at-marks<=6&&at<end&&(script[at]==L' '||script[at]==L'\t')){
            while(at<end&&(script[at]==L' '||script[at]==L'\t'))++at;
            auto title=script.substr(at,std::min<size_t>(200,end-at));while(!title.empty()&&(title.back()==L'\r'||title.back()==L' '))title.pop_back();
            if(!title.empty()){if(!sections.empty())sections.back().end=start;unsigned ordinal=0;for(const auto& s:sections)if(s.title==title)++ordinal;
                sections.push_back({title,end<script.size()?end+1:end,script.size(),ordinal});}}
        start=end+1;
    }
    if(sections.empty()&&!script.empty())sections.push_back({L"",0,script.size(),0});
    return sections;
}
struct PageCue {std::wstring title;unsigned occurrence=0;int page=0;};
struct ReadingFile {std::wstring path;std::vector<int> bookmarks;std::vector<PageCue> cues;};
struct ReadingTools {
    bool pagesVisible=false,cuesVisible=false,sync=false;
    int cuePoints=12;
    std::vector<ReadingFile> files;
    ReadingFile& For(const std::wstring& path){for(auto& f:files)if(_wcsicmp(f.path.c_str(),path.c_str())==0)return f;
        if(files.size()>=32)files.erase(files.begin());files.push_back({path,{},{}});return files.back();}
    const ReadingFile* Find(const std::wstring& path)const{for(const auto& f:files)if(_wcsicmp(f.path.c_str(),path.c_str())==0)return &f;return nullptr;}
    int LinkedPage(const std::wstring& path,const ScriptSection& section)const{const auto* f=Find(path);if(f)for(const auto& c:f->cues)if(c.title==section.title&&c.occurrence==section.occurrence)return c.page;return -1;}
    void Link(const std::wstring& path,const ScriptSection& section,int page){auto& f=For(path);for(auto& c:f.cues)if(c.title==section.title&&c.occurrence==section.occurrence){c.page=page;return;}
        if(f.cues.size()>=128)f.cues.erase(f.cues.begin());f.cues.push_back({section.title,section.occurrence,page});}
    bool Save(const std::wstring& directory)const{
        ProfileWords d;d.Put(0x44525351);d.Put(1);d.Put((pagesVisible?1:0)|(cuesVisible?2:0)|(sync?4:0));d.Put(cuePoints);d.Put(static_cast<DWORD>(files.size()));
        if(files.size()>32||cuePoints<8||cuePoints>200)return false;
        for(const auto& f:files){if(f.path.empty()||f.path.size()>32767||f.bookmarks.size()>64||f.cues.size()>128)return false;d.Put(f.path);d.Put(static_cast<DWORD>(f.bookmarks.size()));
            for(int page:f.bookmarks){if(page<0||page>=10000)return false;d.Put(page);}d.Put(static_cast<DWORD>(f.cues.size()));
            for(const auto& c:f.cues){if(c.title.size()>200||c.occurrence>=128||c.page<0||c.page>=10000)return false;d.Put(c.title);d.Put(c.occurrence);d.Put(c.page);}}
        return WritePrivateProfile(directory+L"\\reading.dpapi",d);
    }
    bool Load(const std::wstring& directory){ProfileWords d;if(!ReadPrivateProfile(directory+L"\\reading.dpapi",d))return false;
        if(d.Get()!=0x44525351||d.Get()!=1)return false;ReadingTools next;const DWORD flags=d.Get();next.pagesVisible=flags&1;next.cuesVisible=flags&2;next.sync=flags&4;next.cuePoints=static_cast<int>(d.Get());
        const DWORD n=d.Get();if(flags>7||n>32||next.cuePoints<8||next.cuePoints>200)return false;
        for(DWORD i=0;d.ok&&i<n;++i){ReadingFile f;f.path=d.Text(32767);const DWORD marks=d.Get();if(f.path.empty()||marks>64)return false;
            for(DWORD j=0;j<marks;++j){const DWORD page=d.Get();if(page>=10000)return false;f.bookmarks.push_back(page);}
            const DWORD cues=d.Get();if(cues>128)return false;for(DWORD j=0;j<cues;++j){PageCue c;c.title=d.Text(200);c.occurrence=d.Get();const DWORD page=d.Get();if(c.occurrence>=128||page>=10000)return false;c.page=page;f.cues.push_back(std::move(c));}
            next.files.push_back(std::move(f));}
        if(!d.Done())return false;*this=std::move(next);return true;
    }
};
}
