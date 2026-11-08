#include "../src/DiagnosticHistory.h"
#include <cstdio>
#include <fstream>
using namespace quietshare;
int wmain(int argc,wchar_t** argv){if(argc!=2)return 2;const std::wstring path=std::wstring(argv[1])+L"\\history-test.dat";DeleteFileW(path.c_str());
    unsigned pass=0,fail=0;auto check=[&](const char* name,bool ok){std::printf("%s %s\n",ok?"PASS":"FAIL",name);ok?++pass:++fail;};
    static_assert(sizeof(DiagnosticEvent)==60,"Journal format must remain fixed DWORD fields");
    DiagnosticHistory h;check("missing history is a valid empty journal",h.Load(path)&&h.events.empty());
    bool added=true;for(int i=0;i<110;++i)added=h.Add(path,'P',15,3,0,0x80004005)&&added;
    check("journal has a fixed 100 event cap",added&&h.events.size()==100&&h.events.front().sequence==11&&h.events.back().sequence==110);
    DiagnosticHistory restored;check("history survives restart with ordered fixed fields",restored.Load(path)&&restored.events.size()==100&&restored.events.back().hresult==0x80004005);
    const auto json=restored.Json();check("JSON contains codes time phase and versions",json.find("QS-P015")!=std::string::npos&&json.find("\"phase\":\"running\"")!=std::string::npos&&json.find("\"utc\"")!=std::string::npos);
    check("report has no paths URLs or free form text",json.find("http")==std::string::npos&&json.find("Users")==std::string::npos&&json.find("private")==std::string::npos);
    check("unknown event codes are rejected",!h.Add(path,'Q',1)&&!h.Add(path,'B',99,9)&&!h.Add(path,'F',999));
    {std::ofstream append(path.c_str(),std::ios::binary|std::ios::app);append<<"private-injected-content";}
    const auto count=restored.events.size();check("malformed journal preserves loaded state",!restored.Load(path)&&restored.events.size()==count);
    std::ifstream in(path.c_str(),std::ios::binary);const std::string bad((std::istreambuf_iterator<char>(in)),{});in.close();
    check("malformed existing journal is not overwritten",!h.Add(path,'B',1));
    std::ifstream same(path.c_str(),std::ios::binary);check("malformed bytes are preserved",std::string((std::istreambuf_iterator<char>(same)),{})==bad);same.close();
    DeleteFileW(path.c_str());check("recovery can explicitly start a new journal",h.Add(path,'B',1)&&h.events.size()==1);
    h.events[0].group='\n';check("export rejects invalid in memory records",h.Json()=="[]"&&h.Text().empty()&&!h.Save(path));
    check("only the record parent can select history location",DiagnosticHistoryPath(L"x\\projection-stop.json")==L"x\\error-history.dat"&&DiagnosticHistoryPath(L"no-folder").empty());
    std::printf("{\"passed\":%u,\"failed\":%u}\n",pass,fail);return fail?1:0;
}
