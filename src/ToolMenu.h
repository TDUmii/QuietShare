#pragma once
#include "Icons.h"
#include <windowsx.h>
#include <algorithm>
#include <string>
#include <vector>
namespace quietshare {
struct ToolMenuItem { int id;std::wstring label;bool enabled;wchar_t icon;bool checked=false; };
class ToolMenu {
    std::vector<ToolMenuItem> items;
    HWND popup=nullptr;
    int current=-1,result=0,row=30,scale=96;
    HFONT font=nullptr;
    int S(int n)const{return MulDiv(n,scale,96);}
    void Move(int direction) {
        for(size_t n=0;n<items.size();++n){current=(current+direction+int(items.size()))%int(items.size());if(items[current].enabled)break;}
        InvalidateRect(popup,nullptr,FALSE);
    }
    void Finish(bool choose) {
        if(choose&&current>=0&&items[current].enabled)result=items[current].id;
        HWND closing=popup;popup=nullptr;if(closing)DestroyWindow(closing);
    }
    static LRESULT CALLBACK Proc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
        auto self=reinterpret_cast<ToolMenu*>(GetWindowLongPtrW(hwnd,GWLP_USERDATA));
        if(msg==WM_NCCREATE){self=static_cast<ToolMenu*>(reinterpret_cast<CREATESTRUCTW*>(lp)->lpCreateParams);SetWindowLongPtrW(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));}
        if(!self)return DefWindowProcW(hwnd,msg,wp,lp);
        if(msg==WM_ACTIVATE&&LOWORD(wp)==WA_INACTIVE){self->Finish(false);return 0;}
        if(msg==WM_KEYDOWN){if(wp==VK_ESCAPE)self->Finish(false);else if(wp==VK_RETURN||wp==VK_SPACE)self->Finish(true);else if(wp==VK_DOWN)self->Move(1);else if(wp==VK_UP)self->Move(-1);return 0;}
        if(msg==WM_MOUSEMOVE||msg==WM_LBUTTONUP){int index=(GET_Y_LPARAM(lp)-self->S(5))/self->row;
            int next=index>=0&&index<int(self->items.size())?index:-1;
            if(msg==WM_MOUSEMOVE&&next==self->current)return 0;
            self->current=next;
            if(msg==WM_LBUTTONUP)self->Finish(true);else InvalidateRect(hwnd,nullptr,FALSE);return 0;}
        if(msg==WM_ERASEBKGND)return 1;
        if(msg==WM_PAINT){PAINTSTRUCT ps{};auto target=BeginPaint(hwnd,&ps);RECT bounds{};GetClientRect(hwnd,&bounds);
            HDC dc=CreateCompatibleDC(target);HBITMAP bitmap=CreateCompatibleBitmap(target,bounds.right,bounds.bottom);auto previous=SelectObject(dc,bitmap);
            auto brush=CreateSolidBrush(RGB(37,37,38));FillRect(dc,&bounds,brush);DeleteObject(brush);
            for(size_t i=0;i<self->items.size();++i){const auto &item=self->items[i];RECT r{self->S(5),self->S(5)+int(i)*self->row,bounds.right-self->S(5),self->S(5)+int(i+1)*self->row};
                if(int(i)==self->current&&item.enabled){brush=CreateSolidBrush(RGB(4,57,94));FillRect(dc,&r,brush);DeleteObject(brush);}
                auto color=item.enabled?RGB(204,204,204):RGB(115,115,115);
                RECT icon=r;icon.right=icon.left+self->S(32);DrawCodicon(dc,item.checked?0xeab2:item.icon,icon,color,self->S(16));
                RECT text=r;text.left+=self->S(36);text.right-=self->S(12);auto old=SelectObject(dc,self->font);SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);
                size_t split=item.label.find(L"   ");std::wstring label=item.label.substr(0,split);DrawTextW(dc,label.c_str(),-1,&text,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
                if(split!=std::wstring::npos){SetTextColor(dc,item.enabled?RGB(160,160,160):RGB(115,115,115));DrawTextW(dc,item.label.c_str()+split+3,-1,&text,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);}
                SelectObject(dc,old);}
            BitBlt(target,0,0,bounds.right,bounds.bottom,dc,0,0,SRCCOPY);SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
            EndPaint(hwnd,&ps);return 0;}
        return DefWindowProcW(hwnd,msg,wp,lp);
    }
public:
    int Show(HWND parent,RECT anchor,UINT dpi,HFONT uiFont,std::vector<ToolMenuItem> entries) {
        scale=dpi;font=uiFont;items=std::move(entries);
        WNDCLASSW type{};type.lpfnWndProc=Proc;type.hInstance=GetModuleHandleW(nullptr);type.hCursor=LoadCursorW(nullptr,IDC_ARROW);type.lpszClassName=L"QuietShareToolMenu";RegisterClassW(&type);
        MONITORINFO info{};info.cbSize=sizeof(info);GetMonitorInfoW(MonitorFromWindow(parent,MONITOR_DEFAULTTONEAREST),&info);
        row=std::min(S(30),std::max(S(20),int(info.rcWork.bottom-info.rcWork.top-S(20))/int(items.size())));
        int width=std::min(S(420),int(info.rcWork.right-info.rcWork.left)),height=row*int(items.size())+S(10);
        int x=std::clamp(anchor.right-width,info.rcWork.left,std::max(info.rcWork.left,info.rcWork.right-width));
        int y=std::clamp(anchor.bottom,info.rcWork.top,std::max(info.rcWork.top,info.rcWork.bottom-height));
        popup=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,type.lpszClassName,L"",WS_POPUP|WS_BORDER,x,y,width,height,parent,nullptr,type.hInstance,this);
        if(!popup)return 0;
        HRGN region=CreateRoundRectRgn(0,0,width+1,height+1,S(12),S(12));
        if(region&&!SetWindowRgn(popup,region,TRUE))DeleteObject(region);
        DWORD affinity=0;
        if(!SetWindowDisplayAffinity(popup,0x11)||!GetWindowDisplayAffinity(popup,&affinity)||affinity!=0x11){Finish(false);return 0;}
        ShowWindow(popup,SW_SHOW);SetFocus(popup);Move(1);
        MSG message{};
        while(popup&&IsWindow(popup)){int status=GetMessageW(&message,nullptr,0,0);
            if(status<=0){Finish(false);if(status==0)PostQuitMessage(int(message.wParam));break;}
            if(!IsWindow(parent)){Finish(false);break;}
            TranslateMessage(&message);DispatchMessageW(&message);}
        if(IsWindow(parent))SetForegroundWindow(parent);
        return result;
    }
};
}
