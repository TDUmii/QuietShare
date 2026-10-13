#pragma once
#include <windows.h>
// Microsoft Codicons 0.0.39, CC BY 4.0; embedded for portable use.
namespace quietshare {
inline HANDLE codiconResource=nullptr;
inline bool LoadCodicons(HINSTANCE module) {
    HRSRC resource=FindResourceW(module,MAKEINTRESOURCEW(401),RT_RCDATA);
    if(!resource)return false;
    DWORD count=0;
    codiconResource=AddFontMemResourceEx(LockResource(LoadResource(module,resource)),SizeofResource(module,resource),nullptr,&count);
    return codiconResource&&count;
}
inline void DrawCodicon(HDC dc,wchar_t glyph,RECT bounds,COLORREF color,int size) {
    HFONT font=CreateFontW(-size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"codicon");
    auto old=SelectObject(dc,font);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,color);
    DrawTextW(dc,&glyph,1,&bounds,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    SelectObject(dc,old);DeleteObject(font);
}
}
