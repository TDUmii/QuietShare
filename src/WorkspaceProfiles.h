#pragma once
#include "LocalProfile.h"
#include "Theme.h"
#include <array>

namespace quietshare {
struct WorkspaceProfile {
    bool saved=false;
    int theme=0,points=12,weight=400,opacity=100,icons=16,zoom=50,speed=1,resource=2,explorer=180;
    DWORD flags=0; // activity, tabs, toolbar, status, tree, preview, pin, wrap
    CustomThemeColors colors;
    bool Valid()const{return theme>=0&&theme<=CustomThemeId&&points>=8&&points<=200&&(weight==400||weight==600||weight==700)&&opacity>=40&&opacity<=100&&icons>=12&&icons<=24&&zoom>=25&&zoom<=400&&speed>=0&&speed<=2&&resource>=0&&resource<=2&&explorer>=112&&explorer<=420&&flags<=255&&
        colors.background<=0xffffff&&colors.panel<=0xffffff&&colors.text<=0xffffff&&colors.accent<=0xffffff;}
};
struct WorkspaceProfiles {
    std::array<WorkspaceProfile,3> slots{};
    bool Save(const std::wstring& directory)const{ProfileWords d;d.Put(0x46505351);d.Put(1);
        for(const auto& p:slots){if(!p.Valid())return false;d.Put(p.saved?1u:0u);for(int n:{p.theme,p.points,p.weight,p.opacity,p.icons,p.zoom,p.speed,p.resource,p.explorer})d.Put(static_cast<DWORD>(n));
            d.Put(p.flags);for(DWORD c:{p.colors.background,p.colors.panel,p.colors.text,p.colors.accent})d.Put(c);}
        return WritePrivateProfile(directory+L"\\workspace-profiles.dpapi",d,4096);}
    bool Load(const std::wstring& directory){ProfileWords d;if(!ReadPrivateProfile(directory+L"\\workspace-profiles.dpapi",d,4096)||d.Get()!=0x46505351||d.Get()!=1)return false;
        WorkspaceProfiles next;for(auto& p:next.slots){const auto saved=d.Get();if(saved>1)return false;p.saved=saved;
            p.theme=static_cast<int>(d.Get());p.points=static_cast<int>(d.Get());p.weight=static_cast<int>(d.Get());p.opacity=static_cast<int>(d.Get());p.icons=static_cast<int>(d.Get());p.zoom=static_cast<int>(d.Get());p.speed=static_cast<int>(d.Get());p.resource=static_cast<int>(d.Get());p.explorer=static_cast<int>(d.Get());p.flags=d.Get();
            p.colors={d.Get(),d.Get(),d.Get(),d.Get()};if(!p.Valid())return false;}
        if(!d.Done())return false;slots=next.slots;return true;}
};
}
