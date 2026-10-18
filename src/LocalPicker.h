#pragma once
#include "Theme.h"
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
#include <shlobj.h>
namespace quietshare {
inline std::wstring LocalParentFolder(const std::wstring &path) {
    size_t end=path.size();while(end>3&&(path[end-1]==L'\\'||path[end-1]==L'/'))--end;
    if(end<=3)return {};
    auto split=path.find_last_of(L"\\/",end-1);
    return split==std::wstring::npos?L"":path.substr(0,split==2?3:split);
}
// One protected native HWND. No Explorer broker, history dropdowns or shell menus.
class LocalPicker {
    struct Entry { std::wstring name,path;bool directory; ULONGLONG bytes=0; FILETIME modified{}; };
    HWND popup=nullptr,owner=nullptr,list=nullptr,pathLabel=nullptr,status=nullptr,accept=nullptr;
    std::vector<Entry> entries;
    std::vector<std::wstring> chosen;
    std::wstring folder,error;
    std::vector<std::wstring> history;
    size_t historyIndex=0;
    std::wstring shortcuts[5];
    HFONT font=nullptr;
    UINT dpi=96;
    bool english=false,foldersOnly=false,loading=false,restoreOwnerFocus=true;
    HBRUSH background=nullptr;
    int S(int n)const{return MulDiv(n,dpi,96);}
    const wchar_t *T(const wchar_t *vi,const wchar_t *en)const{return english?en:vi;}
    static LRESULT CALLBACK AddressProc(HWND h,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR) {
        // The stock edit context menu creates a separate unprotected popup.
        if(msg==WM_CONTEXTMENU)return 0;
        if(msg==WM_KEYDOWN&&wp=='A'&&(GetKeyState(VK_CONTROL)&0x8000)) {
            SendMessageW(h,EM_SETSEL,0,-1);return 0;
        }
        return DefSubclassProc(h,msg,wp,lp);
    }
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
    void Navigate(const std::wstring &path,bool record=true) {
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
                    next.push_back({data.cFileName,canonical+L"\\"+data.cFileName,directory,
                        (ULONGLONG(data.nFileSizeHigh)<<32)|data.nFileSizeLow,data.ftLastWriteTime});
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
        if(record&&(history.empty()||history[historyIndex]!=folder)) {
            if(!history.empty())history.resize(historyIndex+1);
            history.push_back(folder);
            if(history.size()>64)history.erase(history.begin());
            historyIndex=history.size()-1;
        }
        EnableWindow(GetDlgItem(popup,15),historyIndex>0);
        EnableWindow(GetDlgItem(popup,16),!history.empty()&&historyIndex+1<history.size());
        EnableWindow(GetDlgItem(popup,10),!folder.empty());
        SendMessageW(list,WM_SETREDRAW,FALSE,0);SendMessageW(list,LB_RESETCONTENT,0,0);
        for(const auto &entry:entries)SendMessageW(list,LB_ADDSTRING,0,(LPARAM)entry.name.c_str());
        SendMessageW(list,WM_SETREDRAW,TRUE,0);InvalidateRect(list,nullptr,FALSE);
        SetWindowTextW(pathLabel,folder.empty()?L"":folder.c_str());
        for(int i=0;i<5;++i)InvalidateRect(GetDlgItem(popup,20+i),nullptr,FALSE);
        SetWindowTextW(status,capped?T(L"Hiện 1.000 mục đầu. Mở thư mục con để tìm tiếp.",L"Showing the first 1,000 entries. Open a subfolder to continue."):
            foldersOnly?T(L"Nhấp đúp để vào thư mục.",L"Double-click to enter a folder."):
                T(L"Ctrl / Shift để chọn nhiều file.",L"Ctrl / Shift to select multiple files."));
        EnableWindow(accept,foldersOnly?!folder.empty():FALSE);Layout();SetFocus(list);loading=false;
    }
    void Up() {
        if(folder.empty())return;
        Navigate(LocalParentFolder(folder));
    }
    void Travel(bool forward) {
        if(history.empty()||(forward?historyIndex+1>=history.size():historyIndex==0))return;
        size_t previous=historyIndex;
        historyIndex=forward?historyIndex+1:historyIndex-1;
        Navigate(history[historyIndex],false);
        if(folder!=history[historyIndex])historyIndex=previous;
        EnableWindow(GetDlgItem(popup,15),historyIndex>0);
        EnableWindow(GetDlgItem(popup,16),historyIndex+1<history.size());
    }
    void Layout() {
        RECT r{};GetClientRect(popup,&r);
        MoveWindow(GetDlgItem(popup,15),S(12),S(14),S(34),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,16),S(50),S(14),S(34),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,10),S(88),S(14),S(34),S(30),TRUE);
        MoveWindow(pathLabel,S(136),S(14),std::max(S(32),int(r.right)-S(196)),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,17),r.right-S(52),S(14),S(40),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,18),S(20),S(68),S(136),S(24),TRUE);
        for(int i=0;i<5;++i)MoveWindow(GetDlgItem(popup,20+i),S(12),S(100+i*42),S(148),S(36),TRUE);
        MoveWindow(list,S(174),S(96),r.right-S(186),std::max(S(80),int(r.bottom)-S(174)),TRUE);
        RECT contents{};GetClientRect(list,&contents);int right=S(174)+contents.right;
        MoveWindow(GetDlgItem(popup,19),S(210),S(64),std::max(S(32),right-S(466)),S(28),TRUE);
        MoveWindow(GetDlgItem(popup,26),right-S(248),S(64),S(154),S(28),TRUE);
        MoveWindow(GetDlgItem(popup,27),right-S(88),S(64),S(80),S(28),TRUE);
        MoveWindow(status,S(174),r.bottom-S(68),r.right-S(190),S(24),TRUE);
        MoveWindow(GetDlgItem(popup,IDCANCEL),r.right-S(228),r.bottom-S(38),S(92),S(30),TRUE);
        MoveWindow(accept,r.right-S(128),r.bottom-S(38),S(112),S(30),TRUE);
    }
    static LRESULT CALLBACK Proc(HWND h,UINT msg,WPARAM wp,LPARAM lp) {
        auto self=(LocalPicker*)GetWindowLongPtrW(h,GWLP_USERDATA);
        if(msg==WM_NCCREATE){self=(LocalPicker*)((CREATESTRUCTW*)lp)->lpCreateParams;self->popup=h;SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)self);}
        if(!self)return DefWindowProcW(h,msg,wp,lp);
        if(msg==WM_CLOSE){self->Finish(false);return 0;}
        if(msg==WM_SIZE){if(self->list)self->Layout();return 0;}
        if(msg==WM_GETMINMAXINFO){auto p=(MINMAXINFO*)lp;p->ptMinTrackSize={self->S(640),self->S(420)};return 0;}
        if(msg==WM_COMMAND) {
            int id=LOWORD(wp);
            if(id==IDCANCEL)self->Finish(false);
            else if(id==IDOK)self->Finish(true);
            else if(id==10)self->Up();
            else if(id==11)self->Navigate(L"");
            else if(id==15||id==16)self->Travel(id==16);
            else if(id==17){wchar_t path[32768]{};GetWindowTextW(self->pathLabel,path,32768);self->Navigate(path);}
            else if(id>=20&&id<25)self->Navigate(self->shortcuts[id-20]);
            else if(id==12&&HIWORD(wp)==LBN_SELCHANGE)EnableWindow(self->accept,self->foldersOnly?!self->folder.empty():SendMessageW(self->list,LB_GETSELCOUNT,0,0)>0);
            else if(id==12&&HIWORD(wp)==LBN_DBLCLK){int index=(int)SendMessageW(self->list,LB_GETCARETINDEX,0,0);
                if(index>=0&&index<(int)self->entries.size()){if(self->entries[index].directory)self->Navigate(self->entries[index].path);else self->Finish(true);}}
            return 0;
        }
        if(msg==WM_CTLCOLORSTATIC||msg==WM_CTLCOLORLISTBOX||msg==WM_CTLCOLOREDIT){auto dc=(HDC)wp;SetBkColor(dc,theme.bg);SetTextColor(dc,theme.text);return (LRESULT)self->background;}
        if(msg==WM_ERASEBKGND){RECT r{};GetClientRect(h,&r);FillRect((HDC)wp,&r,self->background);return 1;}
        if(msg==WM_DRAWITEM) {
            auto item=(DRAWITEMSTRUCT*)lp;auto dc=item->hDC;RECT r=item->rcItem;
            auto brush=CreateSolidBrush((item->itemState&ODS_SELECTED)?theme.selection:theme.bg);FillRect(dc,&r,brush);DeleteObject(brush);
            if(item->CtlType==ODT_LISTBOX&&item->itemID<self->entries.size()) {
                auto &entry=self->entries[item->itemID];RECT icon=r;icon.right=icon.left+self->S(32);
                DrawCodicon(dc,entry.directory?0xeaf7:0xea7b,icon,theme.muted,self->S(16));r.left+=self->S(36);
                auto old=SelectObject(dc,self->font);SetTextColor(dc,theme.text);SetBkMode(dc,TRANSPARENT);
                RECT name=r;name.right-=self->S(256);
                DrawTextW(dc,entry.name.c_str(),-1,&name,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
                RECT detail=r;detail.left=r.right-self->S(248);detail.right=r.right-self->S(94);
                FILETIME local{};SYSTEMTIME time{};wchar_t date[80]{};
                if(FileTimeToLocalFileTime(&entry.modified,&local)&&FileTimeToSystemTime(&local,&time)&&entry.modified.dwHighDateTime)
                    GetDateFormatEx(LOCALE_NAME_USER_DEFAULT,DATE_SHORTDATE,&time,nullptr,date,80,nullptr);
                SetTextColor(dc,theme.muted);DrawTextW(dc,date,-1,&detail,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
                if(!entry.directory){wchar_t size[40]{};swprintf_s(size,L"%llu KB",entry.bytes/1024+(entry.bytes%1024!=0));
                    detail.left=r.right-self->S(88);detail.right=r.right-self->S(8);
                    DrawTextW(dc,size,-1,&detail,DT_RIGHT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);}
                SelectObject(dc,old);
            } else if(item->CtlType==ODT_BUTTON) {
                bool shortcut=item->CtlID>=20&&item->CtlID<25;
                bool current=shortcut&&IsWindowEnabled(item->hwndItem)&&self->folder==self->shortcuts[item->CtlID-20];
                brush=CreateSolidBrush(item->CtlID==IDOK&&IsWindowEnabled(item->hwndItem)?theme.pin:
                    current?theme.pin:shortcut?theme.bg:theme.elevated);FillRect(dc,&r,brush);DeleteObject(brush);
                wchar_t label[80]{};GetWindowTextW(item->hwndItem,label,80);auto old=SelectObject(dc,self->font);
                SetTextColor(dc,IsWindowEnabled(item->hwndItem)?theme.text:theme.disabled);SetBkMode(dc,TRANSPARENT);
                if(shortcut){RECT icon=r;icon.left+=self->S(8);icon.right=icon.left+self->S(24);
                    const unsigned codes[]={0xeb06,0xea9a,0xea7b,0xeaea,0xea7a};
                    DrawCodicon(dc,codes[item->CtlID-20],icon,theme.green,self->S(16));r.left+=self->S(40);}
                if(item->CtlID==15||item->CtlID==16||item->CtlID==10||item->CtlID==17) {
                    unsigned code=item->CtlID==15?0xea9b:item->CtlID==10?0xeaa1:0xea9c;
                    DrawCodicon(dc,code,r,IsWindowEnabled(item->hwndItem)?theme.text:theme.disabled,self->S(16));
                } else DrawTextW(dc,label,-1,&r,(shortcut?DT_LEFT:DT_CENTER)|DT_VCENTER|DT_SINGLELINE);
                SelectObject(dc,old);
            }
            if(item->itemState&ODS_FOCUS)DrawFocusRect(dc,&item->rcItem);
            return TRUE;
        }
        return DefWindowProcW(h,msg,wp,lp);
    }
public:
    std::vector<std::wstring> Show(HWND parent,bool directory,const std::wstring &initial,bool en,HFONT uiFont,HWND &active,std::wstring &failure) {
        owner=parent;english=en;foldersOnly=directory;font=uiFont;dpi=GetDpiForWindow(parent);
        background=CreateSolidBrush(theme.bg);WNDCLASSW type{};type.lpfnWndProc=Proc;type.hInstance=GetModuleHandleW(nullptr);type.hCursor=LoadCursorW(nullptr,IDC_ARROW);type.lpszClassName=L"QuietShareLocalPicker";RegisterClassW(&type);
        MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);GetMonitorInfoW(MonitorFromWindow(parent,MONITOR_DEFAULTTONEAREST),&monitor);
        int w=std::min(S(900),int(monitor.rcWork.right-monitor.rcWork.left)),ht=std::min(S(560),int(monitor.rcWork.bottom-monitor.rcWork.top));
        popup=CreateWindowExW(WS_EX_TOOLWINDOW,type.lpszClassName,directory?T(L"Mở dự án",L"Open project"):T(L"Mở file",L"Open files"),WS_OVERLAPPEDWINDOW,
            monitor.rcWork.left+(monitor.rcWork.right-monitor.rcWork.left-w)/2,monitor.rcWork.top+(monitor.rcWork.bottom-monitor.rcWork.top-ht)/2,w,ht,parent,nullptr,type.hInstance,this);
        if(!popup||!ApplyPrivacy(popup).applied){if(popup)Finish(false);DeleteObject(background);failure=T(L"Không bảo vệ được hộp chọn.",L"The picker could not be protected.");return {};}
        active=popup;
        const BOOL dark=theme.dark;DwmSetWindowAttribute(popup,20,&dark,sizeof(dark));
        auto control=[&](const wchar_t *kind,const wchar_t *name,int id,DWORD style){
            HWND child=CreateWindowExW(0,kind,name,WS_CHILD|WS_VISIBLE|style,0,0,0,0,popup,(HMENU)(INT_PTR)id,type.hInstance,nullptr);SendMessageW(child,WM_SETFONT,(WPARAM)font,FALSE);return child;};
        control(L"BUTTON",T(L"Quay lại",L"Back"),15,BS_OWNERDRAW|WS_TABSTOP);control(L"BUTTON",T(L"Tiến tới",L"Forward"),16,BS_OWNERDRAW|WS_TABSTOP);
        control(L"BUTTON",T(L"Thư mục cha",L"Up"),10,BS_OWNERDRAW|WS_TABSTOP);
        pathLabel=control(L"EDIT",L"",13,WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP);
        SetWindowSubclass(pathLabel,AddressProc,1,0);
        SendMessageW(pathLabel,EM_SETCUEBANNER,TRUE,(LPARAM)T(L"Máy tính này / Nhập đường dẫn thư mục",L"This PC / Enter folder path"));
        SendMessageW(pathLabel,EM_SETLIMITTEXT,32767,0);
        control(L"BUTTON",T(L"Đi tới",L"Go"),17,BS_OWNERDRAW|WS_TABSTOP);
        control(L"STATIC",T(L"Truy cập nhanh",L"Quick access"),18,SS_LEFT);
        control(L"STATIC",T(L"Tên",L"Name"),19,SS_LEFT);
        control(L"STATIC",T(L"Ngày sửa đổi",L"Date modified"),26,SS_LEFT);
        control(L"STATIC",T(L"Kích thước",L"Size"),27,SS_RIGHT);
        const KNOWNFOLDERID *known[]={&FOLDERID_Desktop,&FOLDERID_Downloads,&FOLDERID_Documents,&FOLDERID_Pictures};
        const wchar_t *labels[]={L"Desktop",L"Downloads",T(L"Tài liệu",L"Documents"),T(L"Hình ảnh",L"Pictures"),T(L"Máy tính này",L"This PC")};
        for(int i=0;i<5;++i){HWND button=control(L"BUTTON",labels[i],20+i,BS_OWNERDRAW|WS_TABSTOP);
            if(i<4){PWSTR path=nullptr;HRESULT result=SHGetKnownFolderPath(*known[i],KF_FLAG_DONT_VERIFY,nullptr,&path);
                if(SUCCEEDED(result)&&path)shortcuts[i]=path;
                CoTaskMemFree(path);EnableWindow(button,!shortcuts[i].empty());}}
        status=control(L"STATIC",L"",14,SS_LEFT);
        list=control(L"LISTBOX",L"",12,LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_EXTENDEDSEL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP);
        SetWindowTheme(list,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
        SendMessageW(list,LB_SETITEMHEIGHT,0,S(30));
        control(L"BUTTON",T(L"Hủy",L"Cancel"),IDCANCEL,BS_OWNERDRAW|WS_TABSTOP);
        accept=control(L"BUTTON",directory?T(L"Chọn thư mục",L"Select folder"):T(L"Mở",L"Open"),IDOK,BS_OWNERDRAW|WS_TABSTOP);
        Layout();Navigate(initial);if(history.empty())Navigate(L"");EnableWindow(parent,FALSE);ShowWindow(popup,SW_SHOW);SetForegroundWindow(popup);SetFocus(list);
        MSG msg{};while(popup&&IsWindow(popup)) {
            int state=GetMessageW(&msg,nullptr,0,0);if(state<=0){Finish(false);if(state==0)PostQuitMessage((int)msg.wParam);break;}
            if(!IsWindow(parent)){Finish(false);break;}
            if(msg.message==WM_KEYDOWN&&(msg.hwnd==popup||IsChild(popup,msg.hwnd))) {
                if(msg.wParam==VK_ESCAPE){Finish(false);continue;}
                if(msg.wParam==VK_RETURN){
                    int id=GetDlgCtrlID(msg.hwnd);
                    if(id==10)Up();else if(id==15||id==16)Travel(id==16);
                    else if(id==13||id==17)SendMessageW(popup,WM_COMMAND,17,0);
                    else if(id>=20&&id<25)Navigate(shortcuts[id-20]);else Finish(id!=IDCANCEL);
                    continue;
                }
                if(msg.wParam==VK_BACK&&msg.hwnd==list){Up();continue;}
                if(msg.wParam=='L'&&(GetKeyState(VK_CONTROL)&0x8000)){SetFocus(pathLabel);SendMessageW(pathLabel,EM_SETSEL,0,-1);continue;}
            }
            if(msg.message==WM_SYSKEYDOWN&&(msg.hwnd==popup||IsChild(popup,msg.hwnd))) {
                if(msg.wParam==VK_LEFT||msg.wParam==VK_RIGHT){Travel(msg.wParam==VK_RIGHT);continue;}
                if(msg.wParam==VK_UP){Up();continue;}
            }
            if(!popup||!IsDialogMessageW(popup,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        }
        active=nullptr;if(IsWindow(parent)){EnableWindow(parent,TRUE);if(restoreOwnerFocus)SetForegroundWindow(parent);}DeleteObject(background);
        failure.clear();return chosen;
    }
};
}
