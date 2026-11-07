#include "../src/WorkspaceProfiles.h"
#include <fstream>
#include <iostream>

int wmain(){using namespace quietshare;int passed=0,failed=0;std::string entries;
    auto check=[&](const char* name,bool ok){(ok?++passed:++failed);std::cout<<(ok?"PASS ":"FAIL ")<<name<<"\n";if(!entries.empty())entries+=",";entries+="{\"name\":\""+std::string(name)+"\",\"passed\":"+(ok?"true":"false")+"}";};
    wchar_t tmp[32768]{};GetTempPathW(32768,tmp);const auto dir=std::wstring(tmp)+L"QuietShare-profile-"+std::to_wstring(GetCurrentProcessId());CreateDirectoryW(dir.c_str(),nullptr);
    WorkspaceProfiles profiles;check("three slots start empty without fabricated saved choices",std::none_of(profiles.slots.begin(),profiles.slots.end(),[](const auto& p){return p.saved;}));
    auto& p=profiles.slots[1];p.saved=true;p.theme=CustomThemeId;p.points=37;p.weight=700;p.opacity=83;p.icons=20;p.zoom=125;p.speed=2;p.resource=0;p.explorer=218;p.flags=173;p.colors={RGB(10,20,30),RGB(20,30,40),RGB(240,245,250),RGB(70,180,100)};
    check("profile metadata saves with Windows user encryption",profiles.Save(dir));WorkspaceProfiles restored;
    check("profile roundtrip preserves all workspace and theme fields",restored.Load(dir)&&restored.slots[1].saved&&!restored.slots[0].saved&&restored.slots[1].theme==CustomThemeId&&restored.slots[1].points==37&&restored.slots[1].weight==700&&restored.slots[1].opacity==83&&restored.slots[1].icons==20&&restored.slots[1].zoom==125&&restored.slots[1].speed==2&&restored.slots[1].resource==0&&restored.slots[1].explorer==218&&restored.slots[1].flags==173&&restored.slots[1].colors.background==RGB(10,20,30));
    p.points=201;check("invalid text size cannot replace good profile",!profiles.Save(dir));p.points=37;
    p.flags=256;check("unknown profile flags are rejected",!profiles.Save(dir));p.flags=173;
    p.colors.accent=0xffffffff;check("non RGB theme colors are rejected",!profiles.Save(dir));p.colors.accent=RGB(70,180,100);
    p.explorer=421;check("layout geometry is bounded",!profiles.Save(dir));p.explorer=218;
    WorkspaceProfiles intact;check("failed saves preserve previous profile",intact.Load(dir)&&intact.slots[1].points==37&&intact.slots[1].flags==173);
    ProfileWords malformed;malformed.Put(0x46505351);malformed.Put(1);malformed.Put(999);check("malformed fixture written",WritePrivateProfile(dir+L"\\workspace-profiles.dpapi",malformed,4096));
    check("corrupt profile load is atomic",!restored.Load(dir)&&restored.slots[1].points==37);
    DeleteFileW((dir+L"\\workspace-profiles.dpapi").c_str());RemoveDirectoryW(dir.c_str());
    std::ofstream report("artifacts/qa-workspace-profiles/workspace-profiles-regression.json");report<<"{\"passed\":"<<passed<<",\"failed\":"<<failed<<",\"checks\":["<<entries<<"]}\n";return failed?1:0;
}
