#include "../src/ReadingTools.h"
#include "../src/MeetingNotebook.h"
#include "../src/MeetingSettings.h"
#include <filesystem>
#include <fstream>
#include <iostream>

int wmain(){
    using namespace quietshare;
    int passed=0,failed=0;std::string checks;
    auto check=[&](const char* name,bool ok){(ok?++passed:++failed);std::cout<<(ok?"PASS ":"FAIL ")<<name<<"\n";
        if(!checks.empty())checks+=",";checks+="{\"name\":\""+std::string(name)+"\",\"passed\":"+(ok?"true":"false")+"}";};
    wchar_t temp[32768]{};GetTempPathW(32768,temp);const std::wstring root=std::wstring(temp)+L"QuietShare-reading-notebook-"+std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(root.c_str(),nullptr);
    const auto sections=OutlineScript(L"# Opening\r\nHello\r\n## Details\nFacts\n# Opening\nFinish");
    check("outline keeps duplicate heading ordinals",sections.size()==3&&sections[2].occurrence==1&&sections[0].end<sections[1].end);
    check("plain script remains a readable section",OutlineScript(L"plain notes").size()==1&&OutlineScript(L"").empty());
    std::wstring huge;for(int i=0;i<500;++i)huge+=L"# Topic\nbody\n";
    check("outline work is capped at 128 sections",OutlineScript(huge).size()==128);
    ReadingTools reading;reading.pagesVisible=true;reading.cuesVisible=true;reading.sync=true;reading.cuePoints=38;
    reading.For(L"C:\\Talk.pdf").bookmarks={0,12,119};reading.Link(L"c:\\talk.PDF",sections[2],47);
    check("case insensitive PDF cue identity",reading.files.size()==1&&reading.LinkedPage(L"C:\\TALK.pdf",sections[2])==47&&reading.LinkedPage(L"C:\\Talk.pdf",sections[0])==-1);
    check("reading metadata saves encrypted",reading.Save(root));ReadingTools restored;
    check("reading metadata roundtrip preserves flags size bookmarks cues",restored.Load(root)&&restored.sync&&restored.cuePoints==38&&restored.files[0].bookmarks.size()==3&&restored.LinkedPage(L"c:\\talk.pdf",sections[2])==47);
    reading.files[0].bookmarks.push_back(10000);check("invalid bookmark cannot overwrite profile",!reading.Save(root));
    ReadingTools intact;check("rejected write preserves last good reading profile",intact.Load(root)&&intact.files[0].bookmarks.size()==3);
    for(int i=0;i<80;++i)intact.For(L"C:\\"+std::to_wstring(i)+L".pdf");check("PDF history has a fixed 32 file cap",intact.files.size()==32);
    ProfileWords bad;bad.Put(0x44525351);bad.Put(1);bad.Put(0);bad.Put(12);bad.Put(33);WritePrivateProfile(root+L"\\reading.dpapi",bad);
    check("oversized private metadata is rejected atomically",!restored.Load(root)&&restored.files.size()==1&&restored.cuePoints==38);
    check("timestamp handles subsecond fractions and comma",MeetingTime(L"01:02.1")==62100&&MeetingTime(L"00:01:02,012")==62012);
    check("invalid times never become fabricated timestamps",MeetingTime(L"")==-1&&MeetingTime(L"00:60:00.000")==-2&&MeetingTime(L"168:00:00.000")==-2&&MeetingTime(L"00:01.000junk")==-2&&MeetingTime(L"-1:00.000")==-2);
    check("empty timestamp fields are invalid",MeetingTime(L"00::01.000")==-2&&MeetingTime(L":01.000")==-2);
    MeetingNotebook notes;
    const std::wstring vtt=L"WEBVTT\n\nNOTE provider note\nignore me\n\n1\n00:00:01.000 --> 00:00:03.200 align:start\n<v An>Xin chào &amp; welcome</v>\nline two\n\n2\n00:00:04.000 --> 00:00:06.000\n<v Bình>Chốt lịch họp.</v>\n";
    check("provider VTT retains exact speakers timing and Unicode",notes.Parse(vtt,L"Teams.vtt")&&notes.segments.size()==2&&notes.segments[0].speaker==L"An"&&notes.segments[1].speaker==L"Bình"&&notes.segments[0].end==3200&&notes.segments[0].text==L"Xin chào & welcome\r\nline two");
    notes.segments[0].decision=true;notes.segments[1].action=true;
    check("notebook persists encrypted with review flags",notes.Save(root));MeetingNotebook loaded;
    check("notebook DPAPI roundtrip",loaded.Load(root)&&loaded.segments.size()==2&&loaded.segments[0].decision&&loaded.segments[1].action);
    MeetingNotebook roundtrip;check("VTT export can be reimported without losing speakers",roundtrip.Parse(loaded.Vtt(),L"export.vtt")&&roundtrip.segments[0].speaker==L"An"&&roundtrip.segments[1].start==4000);
    check("minutes contain only user marked outcomes",loaded.Minutes(true).find(L"## Decisions")!=std::wstring::npos&&loaded.Minutes(true).find(L"- [ ] Bình")!=std::wstring::npos&&loaded.Minutes(false).find(L"## Quyết định")!=std::wstring::npos);
    MeetingNotebook plain;check("mixed audio text does not infer a speaker",plain.Parse(L"Budget: approved\nAnother sentence",L"speech.txt")&&plain.segments[0].speaker.empty()&&plain.Vtt().empty());
    check("provider name prefixes require explicit provider import",plain.Parse(L"[00:00:01.000 --> 00:00:02.000] Minh: Hello",L"Zoom.txt",true)&&plain.segments[0].speaker==L"Minh");
    MeetingNotebook native;check("native recognition timestamp format imports",native.Parse(L"[00:00:01.000 --> 00:00:02.500] hello\r\n[00:00:03.000 --> 00:00:04.000] world",L"native.txt")&&native.segments.size()==2&&native.segments[0].end==2500);
    check("WebVTT string cue identifiers do not become speech",native.Parse(L"WEBVTT\n\nprovider-cue-id\n00:00:01.000 --> 00:00:02.000\nhello",L"id.vtt")&&native.segments.size()==1&&native.segments[0].text==L"hello");
    check("plain NOTE text is retained",native.Parse(L"NOTE keep this thought",L"plain.txt")&&native.segments[0].text==L"NOTE keep this thought");
    check("malformed VTT import preserves previous notes",!loaded.Parse(L"WEBVTT\n\n00:00:09.000 --> 00:00:01.000\nwrong",L"bad.vtt")&&loaded.segments.size()==2);
    check("oversized segment is rejected",!loaded.Parse(std::wstring(MeetingNotebook::SegmentLimit+1,L'x'),L"huge.txt"));
    loaded.segments[0].speaker=L"bad\nname";check("speaker metadata rejects multiline names",!loaded.Valid());
    MeetingSettings settings;settings.outputDirectory=root;check("accuracy model is the default",settings.speechModel==1);
    check("settings v2 roundtrip retains smaller optional model",(settings.speechModel=0,SaveMeetingSettings(root,settings))&&LoadMeetingSettings(root,settings)&&settings.speechModel==0);
    const DWORD header[]={meeting_settings_detail::Magic,1,3,1,static_cast<DWORD>(root.size())};
    HANDLE f=CreateFileW((root+L"\\meeting-settings.dat").c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,0,nullptr);DWORD n=0;WriteFile(f,header,sizeof(header),&n,nullptr);WriteFile(f,root.data(),static_cast<DWORD>(root.size()*2),&n,nullptr);CloseHandle(f);
    check("v1 settings migrate to accuracy model without losing choices",LoadMeetingSettings(root,settings)&&settings.speechModel==1&&settings.transcriptionEnabled&&settings.exportMode==MeetingExportMode::TranscriptOnly&&settings.provider==MeetingProvider::Teams);
    for(const wchar_t* name:{L"reading.dpapi",L"meeting-notes.dpapi",L"meeting-settings.dat"})DeleteFileW((root+L"\\"+name).c_str());RemoveDirectoryW(root.c_str());
    std::ofstream report("artifacts/qa-reading-meeting/reading-meeting-regression.json");report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed<<",\"checks\":["<<checks<<"]}\n";
    return failed?1:0;
}
