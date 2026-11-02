#pragma once
#include <windows.h>
#include <array>
#include <algorithm>
#include <utility>
#include "CustomTheme.h"
namespace quietshare {
struct ThemePalette {
    const wchar_t *vi,*en; bool dark;
    COLORREF bg,sidebar,card,elevated,hover,accent,selection,text,muted,border,green,amber,disabled,pin,home,homeCard,rail;
    COLORREF keyword,string,comment,number,error;
    bool monochrome=false;
};
inline ThemePalette DarkTheme(const wchar_t* vi,const wchar_t* en,COLORREF bg,COLORREF panel,COLORREF raised) {
    return {vi,en,true,bg,raised,panel,raised,RGB(70,70,70),RGB(0,122,204),RGB(4,57,94),
        RGB(242,245,247),RGB(190,190,190),RGB(82,89,96),RGB(110,231,160),RGB(245,184,82),RGB(120,127,136),
        RGB(35,81,64),bg,panel,panel,RGB(197,153,255),RGB(224,184,122),RGB(139,166,151),RGB(116,199,236),RGB(255,145,145)};
}
inline ThemePalette LightTheme(const wchar_t* vi,const wchar_t* en,COLORREF bg,COLORREF panel,COLORREF raised) {
    return {vi,en,false,bg,raised,panel,raised,RGB(211,220,225),RGB(0,100,164),RGB(200,220,235),
        RGB(24,32,39),RGB(74,84,94),RGB(125,135,145),RGB(13,104,61),RGB(133,77,0),RGB(110,117,123),
        RGB(204,230,213),bg,panel,raised,RGB(111,40,148),RGB(132,65,12),RGB(58,103,74),RGB(0,91,125),RGB(173,25,35)};
}
inline const std::array<ThemePalette,10> Themes=[] {
    std::array<ThemePalette,10> t{{
        DarkTheme(L"Đơn sắc tối",L"Monochrome dark",RGB(24,24,24),RGB(32,32,32),RGB(36,36,36)),
        LightTheme(L"Đơn sắc sáng",L"Monochrome light",RGB(250,250,248),RGB(255,255,255),RGB(242,242,239)),
        LightTheme(L"Giấy ấm",L"Warm paper",RGB(244,236,218),RGB(255,248,231),RGB(232,221,199)),
        DarkTheme(L"Xanh đêm",L"Midnight blue",RGB(16,25,39),RGB(24,36,53),RGB(36,51,71)),
        DarkTheme(L"Rừng đêm",L"Forest dark",RGB(19,30,25),RGB(27,42,34),RGB(40,58,47)),
        DarkTheme(L"Than chì",L"Graphite",RGB(24,26,30),RGB(33,36,41),RGB(49,53,60)),
        LightTheme(L"Sương xanh",L"Soft mint",RGB(231,242,235),RGB(247,253,249),RGB(213,230,219)),
        LightTheme(L"Xanh ban ngày",L"Daylight blue",RGB(231,240,249),RGB(248,252,255),RGB(213,226,239)),
        DarkTheme(L"Đen tương phản cao",L"High contrast black",RGB(0,0,0),RGB(0,0,0),RGB(24,24,24)),
        LightTheme(L"Trắng tương phản cao",L"High contrast white",RGB(255,255,255),RGB(255,255,255),RGB(237,237,237))
    }};
    t[0].sidebar=t[0].rail=RGB(18,18,18);t[0].hover=RGB(43,43,43);
    t[0].text=t[0].accent=RGB(242,242,239);t[0].muted=RGB(171,171,170);
    t[0].border=RGB(53,53,53);t[0].selection=t[0].pin=RGB(48,48,48);
    t[0].green=t[0].text;t[0].disabled=RGB(120,120,118);
    t[0].monochrome=true;
    t[1].sidebar=t[1].rail=RGB(240,240,237);t[1].hover=RGB(232,232,227);
    t[1].text=RGB(28,28,27);t[1].accent=RGB(32,32,30);t[1].muted=RGB(99,99,95);
    t[1].border=RGB(216,216,210);t[1].selection=t[1].pin=RGB(228,228,222);
    t[1].green=t[1].text;t[1].disabled=RGB(118,118,113);
    t[1].monochrome=true;
    t[8].text=RGB(255,255,255);t[8].muted=RGB(225,225,225);t[8].border=RGB(155,155,155);
    t[9].text=RGB(0,0,0);t[9].muted=RGB(40,40,40);t[9].border=RGB(90,90,90);
    return t;
}();
inline constexpr int CustomThemeId=10;
inline CustomThemeColors customThemeColors{};
inline int ThemeLuma(COLORREF color)
{
    return (299*GetRValue(color)+587*GetGValue(color)+114*GetBValue(color))/1000;
}
inline COLORREF MixThemeColor(COLORREF first,COLORREF second,int secondWeight)
{
    secondWeight=std::clamp(secondWeight,0,100);const int firstWeight=100-secondWeight;
    return RGB((GetRValue(first)*firstWeight+GetRValue(second)*secondWeight)/100,
               (GetGValue(first)*firstWeight+GetGValue(second)*secondWeight)/100,
               (GetBValue(first)*firstWeight+GetBValue(second)*secondWeight)/100);
}
inline ThemePalette CustomThemePalette()
{
    const bool dark=ThemeLuma(customThemeColors.background)<140;
    ThemePalette result=dark?DarkTheme(L"Tùy chỉnh",L"Custom",customThemeColors.background,
                                       customThemeColors.panel,MixThemeColor(customThemeColors.panel,customThemeColors.background,dark?18:8)):
                             LightTheme(L"Tùy chỉnh",L"Custom",customThemeColors.background,
                                        customThemeColors.panel,MixThemeColor(customThemeColors.panel,customThemeColors.background,18));
    result.bg=customThemeColors.background;result.card=customThemeColors.panel;
    result.sidebar=MixThemeColor(customThemeColors.background,customThemeColors.panel,65);
    result.elevated=MixThemeColor(customThemeColors.panel,dark?RGB(255,255,255):RGB(0,0,0),dark?8:4);
    result.hover=MixThemeColor(customThemeColors.panel,customThemeColors.accent,18);
    result.accent=customThemeColors.accent;
    result.selection=MixThemeColor(customThemeColors.background,customThemeColors.accent,55);
    result.text=customThemeColors.text;
    result.muted=MixThemeColor(customThemeColors.text,customThemeColors.background,dark?45:55);
    result.border=MixThemeColor(customThemeColors.panel,customThemeColors.text,dark?28:22);
    result.green=customThemeColors.accent;
    result.home=customThemeColors.background;result.homeCard=customThemeColors.panel;
    result.rail=MixThemeColor(customThemeColors.background,customThemeColors.panel,72);
    result.keyword=dark?RGB(197,153,255):RGB(111,40,148);
    result.string=dark?RGB(224,184,122):RGB(132,65,12);
    result.comment=MixThemeColor(customThemeColors.text,customThemeColors.background,dark?48:62);
    result.number=dark?RGB(116,199,236):RGB(0,91,125);
    result.error=dark?RGB(255,145,145):RGB(173,25,35);
    result.monochrome=false;
    return result;
}
inline ThemePalette ThemePaletteFor(int id)
{
    return id==CustomThemeId?CustomThemePalette():Themes[std::clamp(id,0,static_cast<int>(Themes.size())-1)];
}
inline ThemePalette theme=Themes[0];
inline const wchar_t* ThemeName(int id,bool english)
{
    if(id==CustomThemeId)return english?L"Custom":L"Tùy chỉnh";
    return english?Themes[std::clamp(id,0,static_cast<int>(Themes.size())-1)].en:
                  Themes[std::clamp(id,0,static_cast<int>(Themes.size())-1)].vi;
}
}
