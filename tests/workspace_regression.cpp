#include "../src/Workspace.h"
#include "../src/Documents.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace quietshare;
int wmain(){
    int passed=0,failed=0;
    auto check=[&](bool ok,const char* name){std::cout<<(ok?"PASS ":"FAIL ")<<name<<'\n';ok?++passed:++failed;};
    wchar_t temp[MAX_PATH]{};GetTempPathW(MAX_PATH,temp);
    const auto root=std::wstring(temp)+L"QuietShareWorkspace-"+std::to_wstring(GetCurrentProcessId());CreateDirectoryW(root.c_str(),nullptr);
    RecentWorkspace recent;
    check(recent.entries.empty()&&!recent.Load(root),"first launch has no fabricated history");
    recent.Touch(L"C:\\Synthetic\\alpha.py",false);recent.Touch(L"C:\\Synthetic\\project",true);
    recent.entries[1].pinned=true;recent.Sort();recent.Touch(L"c:\\synthetic\\ALPHA.py",false);
    check(recent.entries.size()==2&&recent.entries[0].pinned,"case insensitive deduplication preserves pinned entries");
    check(recent.Save(root),"history saved with Windows DPAPI");
    RecentWorkspace loaded;check(loaded.Load(root)&&loaded.entries.size()==2&&loaded.entries[0].pinned&&loaded.entries[1].folder,"encrypted history round trip retains pins and projects");
    auto read=[](const std::wstring& path){std::ifstream f(path.c_str(),std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    auto write=[](const std::wstring& path,const std::string& text){std::ofstream f(path.c_str(),std::ios::binary|std::ios::trunc);f.write(text.data(),text.size());};
    const auto profile=root+L"\\recent.dpapi";
    check(read(profile).find("Synthetic")==std::string::npos,"history does not persist plaintext filenames");
    auto corrupted=read(profile);corrupted.resize(corrupted.size()/2);write(profile,corrupted);
    check(!loaded.Load(root)&&loaded.entries.size()==2,"corrupted history does not mutate valid in memory entries");
    for(int i=0;i<100;++i)recent.Touch(L"C:\\Synthetic\\"+std::to_wstring(i)+L".txt",false);
    check(recent.entries.size()==RecentWorkspace::Limit&&recent.entries[0].pinned,"history is bounded while pins remain first");
    recent.entries.clear();check(recent.Save(root)&&loaded.Load(root)&&loaded.entries.empty(),"clearing history removes entries and pins across restart");
    const auto size=FitRememberedSize({2000,1600},144,{-1920,0,0,1040});
    check(size.cx==1920&&size.cy==1040,"remembered dimensions fit the current signed display work area");
    const auto small=FitRememberedSize({1,1},96,{0,0,300,200});
    check(small.cx==300&&small.cy==200,"small work areas do not create inverted clamp bounds");
    std::wstring error;
    check(SameDocumentText(L"a\rb\r",L"a\r\nb\n")&&!SameDocumentText(L"a",L"ab"),"RichEdit newline normalization is not treated as an unsaved edit");
    const auto file=root+L"\\sample.txt";
    for(const auto& fixture:{std::string(),std::string("first\nsecond\n"),std::string("\xef\xbb\xbf" "first\r\nsecond\r\n"),std::string("\xff\xfe",2)+std::string("f\0i\0r\0s\0t\0\n\0",12),std::string("\xfe\xff",2)+std::string("\0f\0i\0r\0s\0t\0\n",12)}){
        write(file,fixture);TabManager tabs;auto opened=tabs.Open(file,error);
        check(static_cast<bool>(opened),"editable encoding fixture opens through local file policy");
        if(!opened)continue;
        auto& doc=tabs.Active();
        check(tabs.Save(doc.id,error)&&read(file)==fixture,"saving a clean document cannot clear its original bytes");
        doc.draft=L"edited\nline\n";doc.dirty=true;
        check(tabs.Save(doc.id,error)&&!doc.dirty,"explicit save updates the file and clears dirty state");
        const auto saved=read(file);
        const bool utf16=fixture.size()>1&&((unsigned char)fixture[0]==255||(unsigned char)fixture[0]==254);
        check(utf16?saved.substr(0,2)==fixture.substr(0,2):(fixture.substr(0,3)=="\xef\xbb\xbf"?saved.substr(0,3)==fixture.substr(0,3):(fixture.empty()?saved.rfind("edited\r\n",0)==0:saved.rfind("edited\n",0)==0)),"save retains original encoding and BOM policy");
        LoadedContent reopened;check(LoadLocalContent(file,reopened,error)&&reopened.text.find(L"edited")!=std::wstring::npos,"saved bytes reopen as valid text");
        doc.draft=L"\xd800";doc.dirty=true;
        check(!tabs.Save(doc.id,error)&&read(file)==saved,"invalid Unicode cannot corrupt an existing file");
    }
    write(file,"original");TabManager tabs;tabs.Open(file,error);auto& doc=tabs.Active();doc.draft=L"my changes";doc.dirty=true;
    write(file,"external update with different size");
    check(!tabs.Save(doc.id,error)&&doc.dirty&&doc.draft==L"my changes"&&read(file)=="external update with different size","external modification is never overwritten and the draft survives");
    tabs.Reload(doc.id,error);doc.draftUnavailable=true;doc.dirty=true;doc.draft.clear();
    check(!tabs.Save(doc.id,error)&&read(file)=="external update with different size","an unavailable oversized draft cannot be saved as an empty file");
    check(!tabs.Save(0,error),"script autosave is separate from imported file save");
    DeleteFileW(file.c_str());DeleteFileW(profile.c_str());RemoveDirectoryW(root.c_str());
    std::ofstream report("artifacts/qa-workspace/workspace-regression.json");report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed<<"}";
    return failed?1:0;
}
