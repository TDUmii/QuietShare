#pragma once
#include <windows.h>
#include <algorithm>
#include <vector>
// Microsoft Codicons 0.0.39, CC BY 4.0; embedded for portable use.
namespace quietshare {
inline HANDLE codiconResource=nullptr;
inline int interfaceIconSize=16;
inline void SetInterfaceIconSize(int size){interfaceIconSize=std::clamp(size,12,24);}
inline bool LoadCodicons(HINSTANCE module) {
    HRSRC resource=FindResourceW(module,MAKEINTRESOURCEW(401),RT_RCDATA);
    if(!resource)return false;
    DWORD count=0;
    codiconResource=AddFontMemResourceEx(LockResource(LoadResource(module,resource)),SizeofResource(module,resource),nullptr,&count);
    return codiconResource&&count;
}
inline void DrawCodicon(HDC dc,wchar_t glyph,RECT bounds,COLORREF color,int size,bool scale=true) {
    size=std::min(scale?MulDiv(size,interfaceIconSize,16):size,int(std::min(bounds.right-bounds.left,bounds.bottom-bounds.top)));
    if(size<=0||bounds.right<=bounds.left||bounds.bottom<=bounds.top)return;
    // Cache coverage, not coloured pixels: the same smooth glyph works on every
    // surface and hover state. Three-times sampling preserves small curves at
    // fractional display scaling without ClearType colour fringes.
    struct Coverage {wchar_t glyph;int size;std::vector<BYTE> alpha;};
    static std::vector<Coverage> cache;
    auto found=std::find_if(cache.begin(),cache.end(),[&](const auto& c){return c.glyph==glyph&&c.size==size;});
    const int edge=size+4;
    HDC memory=CreateCompatibleDC(dc);if(!memory)return;
    BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    if(found==cache.end()) {
        const int high=edge*3;info.bmiHeader.biWidth=high;info.bmiHeader.biHeight=-high;
        void* pixels=nullptr;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
        if(!bitmap){DeleteDC(memory);return;}
        auto oldBitmap=SelectObject(memory,bitmap);ZeroMemory(pixels,high*high*4);
        HFONT font=CreateFontW(-size*3,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"codicon");
        auto oldFont=SelectObject(memory,font);SetBkMode(memory,TRANSPARENT);SetTextColor(memory,RGB(255,255,255));
        RECT glyphRect{0,0,high,high};DrawTextW(memory,&glyph,1,&glyphRect,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
        GdiFlush();Coverage entry{glyph,size,std::vector<BYTE>(edge*edge)};
        const auto source=static_cast<const DWORD*>(pixels);
        for(int y=0;y<edge;++y)for(int x=0;x<edge;++x) {
            unsigned coverage=0;for(int j=0;j<3;++j)for(int i=0;i<3;++i)coverage+=source[(y*3+j)*high+x*3+i]&255;
            entry.alpha[y*edge+x]=(BYTE)((coverage+4)/9);
        }
        SelectObject(memory,oldFont);DeleteObject(font);SelectObject(memory,oldBitmap);DeleteObject(bitmap);
        if(cache.size()>=256)cache.clear();
        cache.push_back(std::move(entry));found=cache.end()-1;
    }
    info.bmiHeader.biWidth=edge;info.bmiHeader.biHeight=-edge;
    void* pixels=nullptr;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
    if(bitmap) {
        auto target=static_cast<DWORD*>(pixels);
        for(int i=0;i<edge*edge;++i) {
            const unsigned a=found->alpha[i];target[i]=(a<<24)|((GetRValue(color)*a/255)<<16)|((GetGValue(color)*a/255)<<8)|(GetBValue(color)*a/255);
        }
        auto old=SelectObject(memory,bitmap);BLENDFUNCTION blend{AC_SRC_OVER,0,255,AC_SRC_ALPHA};
        // The extra coverage pixels are a raster halo, not part of the glyph's
        // requested size. Crop that halo to the caller's slot without shrinking.
        const int x=(bounds.left+bounds.right-edge)/2,y=(bounds.top+bounds.bottom-edge)/2;
        const int left=std::max(int(bounds.left),x),top=std::max(int(bounds.top),y);
        const int width=std::min(int(bounds.right),x+edge)-left,height=std::min(int(bounds.bottom),y+edge)-top;
        AlphaBlend(dc,left,top,width,height,memory,left-x,top-y,width,height,blend);
        SelectObject(memory,old);DeleteObject(bitmap);
    }
    DeleteDC(memory);
}
}
