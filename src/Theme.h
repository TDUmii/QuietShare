#pragma once
#include <windows.h>
#include <array>
namespace quietshare {
struct ThemePalette {
    const wchar_t *vi,*en; bool dark;
    COLORREF bg,sidebar,card,elevated,hover,accent,selection,text,muted,border,green,amber,disabled,pin,home,homeCard,rail;
    COLORREF keyword,string,comment,number,error;
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
        DarkTheme(L"Tối cổ điển",L"Classic dark",RGB(30,30,30),RGB(37,37,38),RGB(60,60,60)),
        LightTheme(L"Sáng",L"Light",RGB(243,245,247),RGB(255,255,255),RGB(229,234,238)),
        LightTheme(L"Giấy ấm",L"Warm paper",RGB(244,236,218),RGB(255,248,231),RGB(232,221,199)),
        DarkTheme(L"Xanh đêm",L"Midnight blue",RGB(16,25,39),RGB(24,36,53),RGB(36,51,71)),
        DarkTheme(L"Rừng đêm",L"Forest dark",RGB(19,30,25),RGB(27,42,34),RGB(40,58,47)),
        DarkTheme(L"Than chì",L"Graphite",RGB(24,26,30),RGB(33,36,41),RGB(49,53,60)),
        LightTheme(L"Sương xanh",L"Soft mint",RGB(231,242,235),RGB(247,253,249),RGB(213,230,219)),
        LightTheme(L"Xanh ban ngày",L"Daylight blue",RGB(231,240,249),RGB(248,252,255),RGB(213,226,239)),
        DarkTheme(L"Đen tương phản cao",L"High contrast black",RGB(0,0,0),RGB(0,0,0),RGB(24,24,24)),
        LightTheme(L"Trắng tương phản cao",L"High contrast white",RGB(255,255,255),RGB(255,255,255),RGB(237,237,237))
    }};
    // Retain the established default Home surfaces.
    t[0].home=RGB(20,22,23);t[0].homeCard=RGB(34,39,40);t[0].rail=RGB(35,40,40);
    t[8].text=RGB(255,255,255);t[8].muted=RGB(225,225,225);t[8].border=RGB(155,155,155);
    t[9].text=RGB(0,0,0);t[9].muted=RGB(40,40,40);t[9].border=RGB(90,90,90);
    return t;
}();
inline ThemePalette theme=Themes[0];
inline const wchar_t* ThemeName(int id,bool english) {return english?Themes[id].en:Themes[id].vi;}
}
