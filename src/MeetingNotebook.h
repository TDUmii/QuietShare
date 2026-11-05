#pragma once
#include "LocalProfile.h"
#include <algorithm>
#include <sstream>

namespace quietshare {
inline int MeetingTime(const std::wstring& value){
    if(value.empty())return -1;
    std::wstring s=value;std::replace(s.begin(),s.end(),L',',L'.');
    const auto dot=s.find(L'.');if(dot==std::wstring::npos||s.size()-dot<2||s.size()-dot>4)return -2;
    unsigned part=0,ms=0;bool digits=false;std::vector<unsigned> units;
    for(size_t i=0;i<dot;++i){const auto c=s[i];if(c==L':'){if(!digits)return -2;units.push_back(part);part=0;digits=false;}
        else if(c>=L'0'&&c<=L'9'){digits=true;part=part*10+c-L'0';if(part>167)return -2;}else return -2;}
    units.push_back(part);if(!digits||(units.size()!=2&&units.size()!=3))return -2;
    for(size_t i=dot+1;i<s.size();++i){if(s[i]<L'0'||s[i]>L'9')return -2;ms=ms*10+s[i]-L'0';}
    for(size_t i=s.size()-dot-1;i<3;++i)ms*=10;
    const unsigned h=units.size()==3?units[0]:0,m=units[units.size()-2],sec=units.back();
    if(h>167||m>59||sec>59)return -2;
    return static_cast<int>(((h*60+m)*60+sec)*1000+ms);
}
inline std::wstring MeetingClock(int ms){if(ms<0)return L"";wchar_t s[32]{};swprintf_s(s,L"%02d:%02d:%02d.%03d",ms/3600000,(ms/60000)%60,(ms/1000)%60,ms%1000);return s;}
struct MeetingSegment {int start=-1,end=-1;std::wstring speaker,text;bool decision=false,action=false;};
struct MeetingNotebook {
    static constexpr size_t Limit=10000,TextLimit=2*1024*1024,SegmentLimit=32000;
    std::vector<MeetingSegment> segments;
    std::wstring source;
    bool Valid()const {size_t total=0;if(segments.size()>Limit||source.size()>32767)return false;
        for(const auto& s:segments){total+=s.text.size()+s.speaker.size();if(s.text.size()>SegmentLimit||s.speaker.size()>100||s.start< -1||s.end< -1||s.start>=604800000||s.end>=604800000||
            ((s.start<0)!=(s.end<0))||(s.start>=0&&s.end<=s.start))return false;
            for(wchar_t c:s.speaker)if(c<32)return false;}return total<=TextLimit;}
    bool Parse(const std::wstring& raw,const std::wstring& origin,bool speakerPrefixes=false){
        if(raw.size()>TextLimit)return false;
        MeetingNotebook next;next.source=origin;MeetingSegment current;bool vtt=false,skip=false,timed=false;
        auto trim=[](std::wstring s){while(!s.empty()&&(s.back()==L'\r'||s.back()==L' '||s.back()==L'\t'))s.pop_back();size_t i=0;while(i<s.size()&&(s[i]==L' '||s[i]==L'\t'||s[i]==0xFEFF))++i;return s.substr(i);};
        auto flush=[&]{if(!current.text.empty())next.segments.push_back(std::move(current));current={};};
        std::wistringstream input(raw);std::wstring line;
        while(std::getline(input,line)){
            line=trim(line);if(line.empty()){flush();skip=false;continue;}
            if(line==L"WEBVTT"||line.rfind(L"WEBVTT ",0)==0){vtt=true;continue;}
            if(vtt&&(line==L"NOTE"||line.rfind(L"NOTE ",0)==0||line.rfind(L"NOTE\t",0)==0||line==L"STYLE"||line==L"REGION")){skip=true;continue;}if(skip)continue;
            const auto arrow=line.find(L"-->");
            if(arrow!=std::wstring::npos){flush();std::wstring left=line.substr(0,arrow),right=trim(line.substr(arrow+3));
                if(!left.empty()&&left.front()==L'[')left.erase(0,1);
                const auto end=right.find_first_of(L" ]\t");const auto after=right.find(L']');
                current.start=MeetingTime(trim(left));current.end=MeetingTime(right.substr(0,end));
                if(current.start<0||current.end<=current.start)return false;timed=true;
                if(after!=std::wstring::npos&&after+1<right.size())line=trim(right.substr(after+1));else continue;
            }else if(vtt&&current.start<0)continue; // WebVTT cue identifiers and header metadata.
            if(line.rfind(L"<v ",0)==0){const auto at=line.find(L'>');if(at!=std::wstring::npos){current.speaker=line.substr(3,at-3);line=line.substr(at+1);}}
            else if(speakerPrefixes&&current.text.empty()){
                const auto colon=line.find(L": ");if(colon>0&&colon<=100&&colon!=std::wstring::npos){current.speaker=line.substr(0,colon);line=line.substr(colon+2);}}
            if(vtt){std::wstring plain;bool tag=false;for(wchar_t c:line){if(c==L'<')tag=true;else if(c==L'>')tag=false;else if(!tag)plain+=c;}line=std::move(plain);}
            for(const auto& entity:std::vector<std::pair<std::wstring,std::wstring>>{{L"&lt;",L"<"},{L"&gt;",L">"},{L"&amp;",L"&"}}){size_t at=0;while((at=line.find(entity.first,at))!=std::wstring::npos){line.replace(at,entity.first.size(),entity.second);at+=entity.second.size();}}
            if(!current.text.empty())current.text+=L"\r\n";current.text+=line;
            if(current.text.size()>SegmentLimit||next.segments.size()>=Limit)return false;
            if(!vtt&&!timed)flush();timed=false;
        }
        flush();if(next.segments.empty()||!next.Valid())return false;*this=std::move(next);return true;
    }
    bool Save(const std::wstring& directory)const{if(!Valid())return false;ProfileWords d;d.Put(0x4e4d5351);d.Put(1);d.Put(source);d.Put(static_cast<DWORD>(segments.size()));
        for(const auto& s:segments){d.Put(static_cast<DWORD>(s.start));d.Put(static_cast<DWORD>(s.end));d.Put(s.speaker);d.Put(s.text);d.Put((s.decision?1u:0u)|(s.action?2u:0u));}
        return WritePrivateProfile(directory+L"\\meeting-notes.dpapi",d);}
    bool Load(const std::wstring& directory){ProfileWords d;if(!ReadPrivateProfile(directory+L"\\meeting-notes.dpapi",d))return false;
        if(d.Get()!=0x4e4d5351||d.Get()!=1)return false;MeetingNotebook next;next.source=d.Text(32767);const DWORD n=d.Get();if(n>Limit)return false;
        for(DWORD i=0;d.ok&&i<n;++i){MeetingSegment s;s.start=static_cast<int>(d.Get());s.end=static_cast<int>(d.Get());s.speaker=d.Text(100);s.text=d.Text(SegmentLimit);const DWORD flags=d.Get();if(flags>3)return false;s.decision=flags&1;s.action=flags&2;next.segments.push_back(std::move(s));}
        if(!d.Done()||!next.Valid())return false;*this=std::move(next);return true;}
    std::wstring Text()const{std::wstring text;for(const auto& s:segments){if(s.start>=0)text+=L"["+MeetingClock(s.start)+L" --> "+MeetingClock(s.end)+L"] ";if(!s.speaker.empty())text+=s.speaker+L": ";text+=s.text+L"\r\n\r\n";}return text;}
    std::wstring Vtt()const{if(!Valid()||segments.empty())return L"";std::wstring text=L"WEBVTT\r\n\r\n";unsigned n=0;
        auto escape=[](const std::wstring& s){std::wstring out;for(wchar_t c:s){if(c==L'&')out+=L"&amp;";else if(c==L'<')out+=L"&lt;";else if(c==L'>')out+=L"&gt;";else if(c!=L'\r')out+=c;}return out;};
        for(const auto& s:segments){if(s.text.empty())continue;if(s.start<0)return L"";text+=std::to_wstring(++n)+L"\r\n"+MeetingClock(s.start)+L" --> "+MeetingClock(s.end)+L"\r\n";
            auto body=escape(s.text);size_t at=0;while((at=body.find(L"\n\n"))!=std::wstring::npos)body.replace(at,2,L"\n");
            text+=(s.speaker.empty()?body:L"<v "+escape(s.speaker)+L">"+body+L"</v>")+L"\r\n\r\n";}return n?text:L"";}
    std::wstring Minutes(bool english)const{std::wstring text=english?L"# Meeting notes\r\n\r\n":L"# Biên bản cuộc họp\r\n\r\n";
        for(bool actions:{false,true}){text+=actions?(english?L"## Action items\r\n":L"## Việc cần làm\r\n"):(english?L"## Decisions\r\n":L"## Quyết định\r\n");
            for(const auto& s:segments)if(actions?s.action:s.decision){auto body=s.text;std::replace(body.begin(),body.end(),L'\n',L' ');std::replace(body.begin(),body.end(),L'\r',L' ');
                text+=actions?L"- [ ] ":L"- ";if(!s.speaker.empty())text+=s.speaker+L": ";text+=body;if(s.start>=0)text+=L" ("+MeetingClock(s.start)+L")";text+=L"\r\n";}text+=L"\r\n";}return text;}
};
}
