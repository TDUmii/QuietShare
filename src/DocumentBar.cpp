#include "DocumentBar.h"
#include "Icons.h"
#include <commctrl.h>
#include <windowsx.h>
#include <algorithm>

namespace quietshare {
namespace {
constexpr COLORREF Panel=RGB(37,37,38), Raised=RGB(60,60,60), Border=RGB(52,59,66),
                   Text=RGB(242,245,247), Muted=RGB(170,170,170), Accent=RGB(0,122,204);
void Fill(HDC dc, RECT rect, COLORREF color) {
    HBRUSH brush=CreateSolidBrush(color); FillRect(dc,&rect,brush); DeleteObject(brush);
}
void Line(HDC dc,int x,int y,int endX,int endY,COLORREF color,int width=1) {
    HPEN pen=CreatePen(PS_SOLID,width,color); auto old=SelectObject(dc,pen);
    MoveToEx(dc,x,y,nullptr); LineTo(dc,endX,endY); SelectObject(dc,old); DeleteObject(pen);
}
}
bool DocumentBar::Create(HWND parent,HINSTANCE instance) {
    WNDCLASSEXW cls{}; cls.cbSize=sizeof(cls); cls.hInstance=instance;
    cls.lpszClassName=L"QuietShareDocumentBar"; cls.lpfnWndProc=Proc;
    cls.hCursor=LoadCursorW(nullptr,IDC_ARROW); RegisterClassExW(&cls);
    window_=CreateWindowExW(0,cls.lpszClassName,L"",WS_CHILD|WS_TABSTOP|WS_CLIPSIBLINGS,
                           0,0,0,0,parent,nullptr,instance,this);
    tooltip_=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,nullptr,
        WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,0,0,0,0,parent,nullptr,instance,nullptr);
    SendMessageW(tooltip_,TTM_SETMAXTIPWIDTH,0,700);
    DWORD affinity=0;
    tooltipSafe_=SetWindowDisplayAffinity(tooltip_,0x11) &&
        GetWindowDisplayAffinity(tooltip_,&affinity) && affinity==0x11;
    return window_!=nullptr;
}
bool DocumentBar::Overflowing() const { RECT r{}; GetClientRect(window_,&r); return total_>r.right; }
void DocumentBar::Update(std::vector<TabVisual> tabs,std::uint64_t active,bool safe,
                         bool english,UINT dpi,HFONT font) {
    bool changed=active_!=active || tabs_.size()!=tabs.size();
    for(size_t i=0;i<tabs_.size();++i) {
        TOOLINFOW tip{}; tip.cbSize=sizeof(tip); tip.hwnd=window_; tip.uId=i+1;
        SendMessageW(tooltip_,TTM_DELTOOLW,0,(LPARAM)&tip);
    }
    tabs_=safe?std::move(tabs):std::vector<TabVisual>{}; active_=active;
    safe_=safe; english_=english; dpi_=dpi; font_=font;
    if(!safe) { SendMessageW(tooltip_,TTM_POP,0,0); hovered_=-1; }
    SendMessageW(tooltip_,TTM_ACTIVATE,safe && tooltipSafe_,0);
    Reflow(changed); InvalidateRect(window_,nullptr,FALSE);
}
void DocumentBar::Reflow(bool ensureActive) {
    RECT client{}; GetClientRect(window_,&client); rects_.clear(); total_=0;
    HDC dc=GetDC(window_); auto old=SelectObject(dc,font_);
    for(const auto &tab:tabs_) {
        SIZE size{}; GetTextExtentPoint32W(dc,tab.name.c_str(),(int)tab.name.size(),&size);
        int width=std::clamp((int)size.cx+MulDiv(tab.script?40:60,dpi_,96),MulDiv(88,dpi_,96),MulDiv(168,dpi_,96));
        rects_.push_back({total_,MulDiv(2,dpi_,96),total_+width,client.bottom-MulDiv(2,dpi_,96)});
        total_+=width+MulDiv(2,dpi_,96);
    }
    SelectObject(dc,old); ReleaseDC(window_,dc);
    int arrows=total_>client.right?MulDiv(22,dpi_,96):0;
    int available=std::max(1,(int)client.right-2*arrows);
    offset_=std::clamp(offset_,0,std::max(0,total_-available));
    if(ensureActive) for(size_t i=0;i<tabs_.size();++i) if(tabs_[i].id==active_) {
        if(rects_[i].left<offset_) offset_=rects_[i].left;
        else if(rects_[i].right>offset_+available) offset_=std::max(0,(int)rects_[i].right-available);
    }
    for(size_t i=0;i<tabs_.size();++i) {
        RECT rect=rects_[i]; OffsetRect(&rect,arrows-offset_,0);
        rect.left=std::max<LONG>(rect.left,arrows); rect.right=std::min<LONG>(rect.right,client.right-arrows);
        TOOLINFOW tip{}; tip.cbSize=sizeof(tip); tip.hwnd=window_; tip.uId=i+1;
        tip.rect=rect; tip.lpszText=const_cast<wchar_t*>(tabs_[i].path.empty()?tabs_[i].name.c_str():tabs_[i].path.c_str());
        SendMessageW(tooltip_,TTM_ADDTOOLW,0,(LPARAM)&tip);
        SendMessageW(tooltip_,TTM_NEWTOOLRECTW,0,(LPARAM)&tip);
    }
}
void DocumentBar::Scroll(int amount) { offset_+=MulDiv(amount,dpi_,96); Reflow(false); InvalidateRect(window_,nullptr,FALSE); }
void DocumentBar::Paint(HDC dc) {
    RECT client{}; GetClientRect(window_,&client); Fill(dc,client,Panel);
    SetBkMode(dc,TRANSPARENT); auto old=SelectObject(dc,font_);
    int s=MulDiv(1,dpi_,96), arrows=Overflowing()?MulDiv(22,dpi_,96):0;
    if(!safe_) {
        SetTextColor(dc,Muted); RECT label=client; label.left+=MulDiv(12,dpi_,96);
        DrawTextW(dc,english_?L"Content locked":L"Nội dung đang khóa",-1,&label,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    }
    int saved=SaveDC(dc); IntersectClipRect(dc,arrows,0,client.right-arrows,client.bottom);
    for(size_t i=0;i<tabs_.size();++i) {
        RECT r=rects_[i]; OffsetRect(&r,arrows-offset_,0);
        if(r.right<=arrows || r.left>=client.right-arrows) continue;
        bool active=tabs_[i].id==active_;
        Fill(dc,r,active || (int)i==hovered_?Raised:Panel);
        int iconX=r.left+MulDiv(10,dpi_,96), center=(r.top+r.bottom)/2;
        RECT icon{iconX,center-MulDiv(8,dpi_,96),iconX+MulDiv(16,dpi_,96),center+MulDiv(8,dpi_,96)};
        DrawCodicon(dc,tabs_[i].kind==2?0xeae9:tabs_[i].kind==3?0xeb0e:0xea7b,icon,Muted,MulDiv(16,dpi_,96));
        RECT label=r; label.left+=MulDiv(26,dpi_,96); label.right-=MulDiv(tabs_[i].script?8:24,dpi_,96);
        SetTextColor(dc,active?Text:Muted); DrawTextW(dc,tabs_[i].name.c_str(),-1,&label,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
        if(!tabs_[i].script) {
            int x=r.right-MulDiv(13,dpi_,96), k=MulDiv(3,dpi_,96);
            Line(dc,x-k,center-k,x+k,center+k,Muted,s); Line(dc,x-k,center+k,x+k,center-k,Muted,s);
        }
        if(active) { RECT underline{r.left,r.bottom-MulDiv(2,dpi_,96),r.right,r.bottom}; Fill(dc,underline,Accent); }
    }
    RestoreDC(dc,saved);
    if(arrows) for(int right=0;right<2;right++) {
        int x=right?client.right-arrows/2:arrows/2,y=client.bottom/2,k=MulDiv(3,dpi_,96);
        Line(dc,x+(right?-k:k),y-k,x,y,Muted,s); Line(dc,x,y,x+(right?-k:k),y+k,Muted,s);
    }
    SelectObject(dc,old);
}
void DocumentBar::Hit(POINT point,bool middle) {
    if(!safe_) return; RECT c{}; GetClientRect(window_,&c);
    int arrows=Overflowing()?MulDiv(22,dpi_,96):0;
    if(arrows && point.x<arrows) { Scroll(-120); return; }
    if(arrows && point.x>=c.right-arrows) { Scroll(120); return; }
    for(size_t i=0;i<tabs_.size();++i) {
        RECT r=rects_[i]; OffsetRect(&r,arrows-offset_,0);
        if(PtInRect(&r,point)) {
            bool close=!tabs_[i].script && (middle || point.x>r.right-MulDiv(24,dpi_,96));
            PostMessageW(GetParent(window_),close?DocumentCloseMessage:DocumentActivateMessage,(WPARAM)tabs_[i].id,0);
            return;
        }
    }
}
LRESULT CALLBACK DocumentBar::Proc(HWND h,UINT message,WPARAM wp,LPARAM lp) {
    auto self=(DocumentBar*)GetWindowLongPtrW(h,GWLP_USERDATA);
    if(message==WM_NCCREATE) { self=(DocumentBar*)((CREATESTRUCTW*)lp)->lpCreateParams; SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)self); }
    if(!self) return DefWindowProcW(h,message,wp,lp);
    switch(message) {
    case WM_PAINT: {
        PAINTSTRUCT p{}; HDC dc=BeginPaint(h,&p); RECT c{}; GetClientRect(h,&c);
        HDC buffer=CreateCompatibleDC(dc); HBITMAP bitmap=CreateCompatibleBitmap(dc,std::max(1L,c.right),std::max(1L,c.bottom));
        auto old=SelectObject(buffer,bitmap); self->Paint(buffer); BitBlt(dc,0,0,c.right,c.bottom,buffer,0,0,SRCCOPY);
        SelectObject(buffer,old); DeleteObject(bitmap); DeleteDC(buffer); EndPaint(h,&p); return 0;
    }
    case WM_ERASEBKGND:return 1;
    case WM_SIZE:self->Reflow(true);return 0;
    case WM_MOUSEWHEEL:self->Scroll(GET_WHEEL_DELTA_WPARAM(wp)>0?-80:80);return 0;
    case WM_MOUSEHWHEEL:self->Scroll(GET_WHEEL_DELTA_WPARAM(wp)>0?80:-80);return 0;
    case WM_LBUTTONDOWN: {
        POINT point{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};RECT bounds{};GetClientRect(h,&bounds);
        int arrows=self->Overflowing()?MulDiv(22,self->dpi_,96):0;
        if(point.x<arrows || point.x>=bounds.right-arrows)return 0;
        for(RECT rect:self->rects_) {
            OffsetRect(&rect,arrows-self->offset_,0);if(PtInRect(&rect,point))return 0;
        }
        ReleaseCapture();SendMessageW(GetParent(h),WM_NCLBUTTONDOWN,HTCAPTION,0);return 0;
    }
    case WM_LBUTTONUP:case WM_MBUTTONUP:self->Hit({GET_X_LPARAM(lp),GET_Y_LPARAM(lp)},message==WM_MBUTTONUP);return 0;
    case WM_MOUSEMOVE: {
        MSG msg{h,message,wp,lp,0,{}}; SendMessageW(self->tooltip_,TTM_RELAYEVENT,0,(LPARAM)&msg);
        TRACKMOUSEEVENT track{sizeof(track),TME_LEAVE,h,0}; TrackMouseEvent(&track);
        int previousHover=self->hovered_;self->hovered_=-1; RECT c{}; GetClientRect(h,&c);int arrows=self->Overflowing()?MulDiv(22,self->dpi_,96):0;
        POINT point{GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        for(size_t i=0;i<self->rects_.size();i++) { RECT r=self->rects_[i]; OffsetRect(&r,arrows-self->offset_,0);if(PtInRect(&r,point))self->hovered_=(int)i; }
        if(previousHover!=self->hovered_)InvalidateRect(h,nullptr,FALSE);return 0;
    }
    case WM_MOUSELEAVE:self->hovered_=-1;SendMessageW(self->tooltip_,TTM_POP,0,0);InvalidateRect(h,nullptr,FALSE);return 0;
    case WM_KEYDOWN:
        if(wp==VK_LEFT || wp==VK_RIGHT) {
            for(size_t i=0;i<self->tabs_.size();++i) if(self->tabs_[i].id==self->active_) {
                size_t next=(i+self->tabs_.size()+(wp==VK_LEFT?-1:1))%self->tabs_.size();
                PostMessageW(GetParent(h),DocumentActivateMessage,(WPARAM)self->tabs_[next].id,0);break;
            } return 0;
        } break;
    }
    return DefWindowProcW(h,message,wp,lp);
}
}
