#pragma once
#include "Content.h"
#include "Privacy.h"
#include "Icons.h"
#include <algorithm>
#include <commctrl.h>
#include <string>
#include <vector>
#include <cwctype>
#include <windowsx.h>
#include <dwmapi.h>
#include <uxtheme.h>
namespace quietshare {
inline std::wstring LocalParentFolder(const std::wstring &path) {
    size_t end=path.size();while(end>3&&(path[end-1]==L'\\'||path[end-1]==L'/'))--end;
    if(end<=3)return {};
    auto split=path.find_last_of(L"\\/",end-1);
    return split==std::wstring::npos?L"":path.substr(0,split==2?3:split);
}
// One protected native HWND. No Explorer broker, history dropdowns or shell menus.
class LocalPicker {
    struct Entry { std::wstring name,path;bool directory; };
    HWND popup=nullptr,owner=nullptr,list=nullptr,pathLabel=nullptr,status=nullptr,accept=nullptr;
    std::vector<Entry> entries;
    std::vector<std::wstring> chosen;
    std::wstring folder,error;
    HFONT font=nullptr;
    UINT dpi=96;
    bool english=false,foldersOnly=false,loading=false,restoreOwnerFocus=true;
    HBRUSH background=nullptr;
    int S(int n)const{return MulDiv(n,dpi,96);}
    const wchar_t *T(const wchar_t *vi,const wchar_t *en)const{return english?en:vi;}
    void Finish(bool take) {
        if(take) {
            int count=(int)SendMessageW(list,LB_GETSELCOUNT,0,0);
            std::vector<int> selected(std::max(0,count));
            if(count>0)SendMessageW(list,LB_GETSELITEMS,count,(LPARAM)selected.data());
            if(foldersOnly) {
                if(count>1){SetWindowTextW(status,T(L"Chọn một thư mục.",L"Select one folder."));return;}
                if(count==1 && entries[selected.front()].directory)chosen.push_back(entries[selected.front()].path);
                else if(!folder.empty())chosen.push_back(folder);
            } else {
                if(count==1&&entries[selected.front()].directory){Navigate(entries[selected.front()].path);return;}
                for(int index:selected)if(!entries[index].directory && chosen.size()<31)chosen.push_back(entries[index].path);
            }
            if(chosen.empty())return;
        }
        HWND closing=popup;restoreOwnerFocus=GetForegroundWindow()==closing;
        popup=nullptr;if(closing)DestroyWindow(closing);
    }
    void Navigate(const std::wstring &path) {
        if(loading)return;
        loading=true;std::vector<Entry> next;
        std::wstring canonical;
        bool capped=false;
        if(path.empty()) {
            wchar_t drives[256]{};DWORD length=GetLogicalDriveStringsW(256,drives);
            if(length && length<256)for(const wchar_t *drive=drives;*drive;drive+=wcslen(drive)+1) {
                UINT type=GetDriveTypeW(drive);
                if(type==DRIVE_FIXED||type==DRIVE_REMOVABLE||type==DRIVE_RAMDISK)next.push_back({drive,drive,true});
            }
        } else if(CanonicalLocalPath(path,true,canonical,error)) {
            WIN32_FIND_DATAW data{};HANDLE search=FindFirstFileExW((canonical+L"\\*").c_str(),FindExInfoBasic,&data,FindExSearchNameMatch,nullptr,0);
            if(search!=INVALID_HANDLE_VALUE) {
                do {
                    if(wcscmp(data.cFileName,L".")==0||wcscmp(data.cFileName,L"..")==0)continue;
                    const bool directory=(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
                    if(data.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)continue;
                    if(foldersOnly&&!directory)continue;
                    if(next.size()>=1000){capped=true;break;}
                    next.push_back({data.cFileName,canonical+L"\\"+data.cFileName,directory});
                }while(FindNextFileW(search,&data));
                FindClose(search);
            } else if(GetLastError()!=ERROR_FILE_NOT_FOUND) {
                error=T(L"Không đọc được thư mục này.",L"This folder could not be read.");
                SetWindowTextW(status,error.c_str());loading=false;return;
            }
        } else {SetWindowTextW(status,error.c_str());loading=false;return;}
        std::stable_sort(next.begin(),next.end(),[](const Entry&a,const Entry&b){
            if(a.directory!=b.directory)return a.directory;
            return CompareStringOrdinal(a.name.c_str(),-1,b.name.c_str(),-1,TRUE)==CSTR_LESS_THAN;});
        entries=std::move(next);folder=canonical;error.clear();
        SendMessageW(list,WM_SETREDRAW,FALSE,0);SendMessageW(list,LB_RESETCONTENT,0,0);
        for(const auto &entry:entries)SendMessageW(list,LB_ADDSTRING,0,(LPARAM)entry.name.c_str());
        SendMessageW(list,WM_SETREDRAW,TRUE,0);InvalidateRect(list,nullptr,FALSE);
        SetWindowTextW(pathLabel,folder.empty()?T(L"Ổ đĩa cục bộ",L"Local drives"):folder.c_str());
        SetWindowTextW(status,capped?T(L"Hiện 1.000 mục đầu. Mở thư mục con để tìm tiếp.",L"Showing the first 1,000 entries. Open a subfolder to continue."):
            foldersOnly?T(L"Nhấp đúp để vào thư mục.",L"Double-click to enter a folder."):
                T(L"Ctrl / Shift để chọn nhiều file.",L"Ctrl / Shift to select multiple files."));
        EnableWindow(accept,foldersOnly?!folder.empty():FALSE);SetFocus(list);loading=false;
    }
    void Up() {
        if(folder.empty())return;
        Navigate(LocalParentFolder(folder));
    }
    void Layout() {
        RECT r{};GetClientRect(popup,&r);
        MoveWindow(GetDlgItem(popup,10),S(16),S(16),S(52),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,11),S(76),S(16),S(84),S(30),TRUE);
        MoveWindow(pathLabel,S(172),S(16),std::max(S(32),int(r.right)-S(188)),S(30),TRUE);
        MoveWindow(list,S(16),S(58),r.right-S(32),std::max(S(80),int(r.bottom)-S(134)),TRUE);
        MoveWindow(status,S(16),r.bottom-S(68),r.right-S(32),S(24),TRUE);
        MoveWindow(GetDlgItem(popup,IDCANCEL),r.right-S(228),r.bottom-S(38),S(92),S(30),TRUE);
        MoveWindow(accept,r.right-S(128),r.bottom-S(38),S(112),S(30),TRUE);
    }
    static LRESULT CALLBACK Proc(HWND h,UINT msg,WPARAM wp,LPARAM lp) {
        auto self=(LocalPicker*)GetWindowLongPtrW(h,GWLP_USERDATA);
        if(msg==WM_NCCREATE){self=(LocalPicker*)((CREATESTRUCTW*)lp)->lpCreateParams;self->popup=h;SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)self);}
        if(!self)return DefWindowProcW(h,msg,wp,lp);
        if(msg==WM_CLOSE){self->Finish(false);return 0;}
        if(msg==WM_SIZE){if(self->list)self->Layout();return 0;}
        if(msg==WM_GETMINMAXINFO){auto p=(MINMAXINFO*)lp;p->ptMinTrackSize={self->S(440),self->S(340)};return 0;}
        if(msg==WM_COMMAND) {
            int id=LOWORD(wp);
            if(id==IDCANCEL)self->Finish(false);
            else if(id==IDOK)self->Finish(true);
            else if(id==10)self->Up();
            else if(id==11)self->Navigate(L"");
            else if(id==12&&HIWORD(wp)==LBN_SELCHANGE)EnableWindow(self->accept,self->foldersOnly?!self->folder.empty():SendMessageW(self->list,LB_GETSELCOUNT,0,0)>0);
            else if(id==12&&HIWORD(wp)==LBN_DBLCLK){int index=(int)SendMessageW(self->list,LB_GETCARETINDEX,0,0);
                if(index>=0&&index<(int)self->entries.size()){if(self->entries[index].directory)self->Navigate(self->entries[index].path);else self->Finish(true);}}
            return 0;
        }
        if(msg==WM_CTLCOLORSTATIC||msg==WM_CTLCOLORLISTBOX){auto dc=(HDC)wp;SetBkColor(dc,RGB(30,30,30));SetTextColor(dc,RGB(204,204,204));return (LRESULT)self->background;}
        if(msg==WM_ERASEBKGND){RECT r{};GetClientRect(h,&r);FillRect((HDC)wp,&r,self->background);return 1;}
        if(msg==WM_DRAWITEM) {
            auto item=(DRAWITEMSTRUCT*)lp;auto dc=item->hDC;RECT r=item->rcItem;
            auto brush=CreateSolidBrush((item->itemState&ODS_SELECTED)?RGB(4,57,94):RGB(30,30,30));FillRect(dc,&r,brush);DeleteObject(brush);
            if(item->CtlType==ODT_LISTBOX&&item->itemID<self->entries.size()) {
                auto &entry=self->entries[item->itemID];RECT icon=r;icon.right=icon.left+self->S(32);
                DrawCodicon(dc,entry.directory?0xeaf7:0xea7b,icon,RGB(170,170,170),self->S(16));r.left+=self->S(36);
                auto old=SelectObject(dc,self->font);SetTextColor(dc,RGB(204,204,204));SetBkMode(dc,TRANSPARENT);
                DrawTextW(dc,entry.name.c_str(),-1,&r,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);SelectObject(dc,old);
            } else if(item->CtlType==ODT_BUTTON) {
                brush=CreateSolidBrush(item->CtlID==IDOK&&IsWindowEnabled(item->hwndItem)?RGB(0,122,204):RGB(60,60,60));FillRect(dc,&r,brush);DeleteObject(brush);
                wchar_t label[80]{};GetWindowTextW(item->hwndItem,label,80);auto old=SelectObject(dc,self->font);
                SetTextColor(dc,IsWindowEnabled(item->hwndItem)?RGB(242,245,247):RGB(140,140,140));SetBkMode(dc,TRANSPARENT);
                DrawTextW(dc,label,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SelectObject(dc,old);
            }
            if(item->itemState&ODS_FOCUS)DrawFocusRect(dc,&item->rcItem);
            return TRUE;
        }
        return DefWindowProcW(h,msg,wp,lp);
    }
public:
    std::vector<std::wstring> Show(HWND parent,bool directory,const std::wstring &initial,bool en,HFONT uiFont,HWND &active,std::wstring &failure) {
        owner=parent;english=en;foldersOnly=directory;font=uiFont;dpi=GetDpiForWindow(parent);
        background=CreateSolidBrush(RGB(30,30,30));WNDCLASSW type{};type.lpfnWndProc=Proc;type.hInstance=GetModuleHandleW(nullptr);type.hCursor=LoadCursorW(nullptr,IDC_ARROW);type.lpszClassName=L"QuietShareLocalPicker";RegisterClassW(&type);
        MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);GetMonitorInfoW(MonitorFromWindow(parent,MONITOR_DEFAULTTONEAREST),&monitor);
        int w=std::min(S(660),int(monitor.rcWork.right-monitor.rcWork.left)),ht=std::min(S(460),int(monitor.rcWork.bottom-monitor.rcWork.top));
        popup=CreateWindowExW(WS_EX_TOOLWINDOW,type.lpszClassName,directory?T(L"Mở dự án",L"Open project"):T(L"Mở file",L"Open files"),WS_OVERLAPPEDWINDOW,
            monitor.rcWork.left+(monitor.rcWork.right-monitor.rcWork.left-w)/2,monitor.rcWork.top+(monitor.rcWork.bottom-monitor.rcWork.top-ht)/2,w,ht,parent,nullptr,type.hInstance,this);
        if(!popup||!ApplyPrivacy(popup).applied){if(popup)Finish(false);DeleteObject(background);failure=T(L"Không bảo vệ được hộp chọn.",L"The picker could not be protected.");return {};}
        active=popup;
        const BOOL dark=TRUE;DwmSetWindowAttribute(popup,20,&dark,sizeof(dark));
        auto control=[&](const wchar_t *kind,const wchar_t *name,int id,DWORD style){
            HWND child=CreateWindowExW(0,kind,name,WS_CHILD|WS_VISIBLE|style,0,0,0,0,popup,(HMENU)(INT_PTR)id,type.hInstance,nullptr);SendMessageW(child,WM_SETFONT,(WPARAM)font,FALSE);return child;};
        control(L"BUTTON",T(L"Lên",L"Up"),10,BS_OWNERDRAW|WS_TABSTOP);control(L"BUTTON",T(L"Ổ đĩa",L"Drives"),11,BS_OWNERDRAW|WS_TABSTOP);
        pathLabel=control(L"STATIC",L"",13,SS_PATHELLIPSIS);status=control(L"STATIC",L"",14,SS_LEFT);
        list=control(L"LISTBOX",L"",12,LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_EXTENDEDSEL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP);
        SetWindowTheme(list,L"DarkMode_Explorer",nullptr);
        SendMessageW(list,LB_SETITEMHEIGHT,0,S(30));
        control(L"BUTTON",T(L"Hủy",L"Cancel"),IDCANCEL,BS_OWNERDRAW|WS_TABSTOP);
        accept=control(L"BUTTON",directory?T(L"Chọn thư mục",L"Select folder"):T(L"Mở",L"Open"),IDOK,BS_OWNERDRAW|WS_TABSTOP);
        Layout();Navigate(initial);EnableWindow(parent,FALSE);ShowWindow(popup,SW_SHOW);SetForegroundWindow(popup);SetFocus(list);
        MSG msg{};while(popup&&IsWindow(popup)) {
            int state=GetMessageW(&msg,nullptr,0,0);if(state<=0){Finish(false);if(state==0)PostQuitMessage((int)msg.wParam);break;}
            if(!IsWindow(parent)){Finish(false);break;}
            if(msg.message==WM_KEYDOWN&&(msg.hwnd==popup||IsChild(popup,msg.hwnd))) {
                if(msg.wParam==VK_ESCAPE){Finish(false);continue;}
                if(msg.wParam==VK_RETURN){
                    int id=GetDlgCtrlID(msg.hwnd);
                    if(id==10)Up();else if(id==11)Navigate(L"");else Finish(id!=IDCANCEL);
                    continue;
                }
                if(msg.wParam==VK_BACK&&msg.hwnd==list){Up();continue;}
            }
            if(!popup||!IsDialogMessageW(popup,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        }
        active=nullptr;if(IsWindow(parent)){EnableWindow(parent,TRUE);if(restoreOwnerFocus)SetForegroundWindow(parent);}DeleteObject(background);
        failure.clear();return chosen;
    }
};
}
