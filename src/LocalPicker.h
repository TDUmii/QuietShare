#pragma once
#include "Theme.h"
#include "Content.h"
#include "Privacy.h"
#include "Icons.h"
#include "ToolMenu.h"
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
    std::vector<size_t> order,visible;
    HWND searchBox=nullptr;
    int wheelRemainder=0;
    bool capped=false,showDetails=true;
    inline static int sortKey=0;
    inline static bool ascending=true,details=true;
    const Entry* At(int row)const{return row>=0&&static_cast<size_t>(row)<visible.size()?&entries[visible[row]]:nullptr;}
    static int Names(const std::wstring& a,const std::wstring& b){
        int value=CompareStringEx(LOCALE_NAME_USER_DEFAULT,NORM_IGNORECASE|SORT_DIGITSASNUMBERS,a.c_str(),-1,b.c_str(),-1,nullptr,nullptr,0);
        if(!value)value=CompareStringOrdinal(a.c_str(),-1,b.c_str(),-1,TRUE);
        return value-CSTR_EQUAL;
    }
    static std::wstring Extension(const Entry& entry){auto dot=entry.name.find_last_of(L'.');return entry.directory||dot==0||dot==std::wstring::npos?L"":entry.name.substr(dot+1);}
    void Sort(){
        std::stable_sort(order.begin(),order.end(),[&](size_t left,size_t right){const auto& a=entries[left];const auto& b=entries[right];if(a.directory!=b.directory)return a.directory;
            int value=sortKey==1?Names(Extension(a),Extension(b)):sortKey==2?CompareFileTime(&a.modified,&b.modified):sortKey==3?(a.bytes<b.bytes?-1:a.bytes>b.bytes?1:0):Names(a.name,b.name);
            if(!value)value=Names(a.name,b.name);
            return ascending?value<0:value>0;});
    }
    void Rows(bool preserve=true){
        std::vector<bool> selected(entries.size());int count=(int)SendMessageW(list,LB_GETSELCOUNT,0,0);std::vector<int> rows(std::max(0,count));
        if(preserve&&count>0){SendMessageW(list,LB_GETSELITEMS,count,(LPARAM)rows.data());for(int row:rows)if(row>=0&&static_cast<size_t>(row)<visible.size())selected[visible[row]]=true;}
        wchar_t filter[256]{};GetWindowTextW(searchBox,filter,256);visible.clear();
        for(auto index:order)if(!filter[0]||FindStringOrdinal(FIND_FROMSTART,entries[index].name.c_str(),-1,filter,-1,TRUE)>=0)visible.push_back(index);
        SendMessageW(list,WM_SETREDRAW,FALSE,0);SendMessageW(list,LB_RESETCONTENT,0,0);
        for(size_t row=0;row<visible.size();++row){const auto index=visible[row];SendMessageW(list,LB_ADDSTRING,0,(LPARAM)entries[index].name.c_str());if(selected[index])SendMessageW(list,LB_SETSEL,TRUE,row);}
        if(filter[0]&&!foldersOnly&&visible.size()==1)SendMessageW(list,LB_SETSEL,TRUE,0);
        SendMessageW(list,LB_SETTOPINDEX,0,0);SendMessageW(list,WM_SETREDRAW,TRUE,0);InvalidateRect(list,nullptr,FALSE);wheelRemainder=0;
        EnableWindow(accept,foldersOnly?!folder.empty():SendMessageW(list,LB_GETSELCOUNT,0,0)>0);
        std::wstring message=std::to_wstring(visible.size())+L" / "+std::to_wstring(entries.size())+T(L" mục",L" entries");
        if(capped)message+=T(L". Giới hạn 1.000 mục. Mở thư mục con để tìm tiếp.",L". Limited to 1,000 entries. Open a subfolder to continue.");
        SetWindowTextW(status,message.c_str());
        Layout();for(int id:{19,26,27,31})InvalidateRect(GetDlgItem(popup,id),nullptr,FALSE);
    }
    void ChangeSort(int key){ascending=sortKey==key?!ascending:(key!=2&&key!=3);sortKey=key;Sort();Rows();}
    void SortMenu(){RECT anchor{};GetWindowRect(GetDlgItem(popup,29),&anchor);ToolMenu menu;
        const int chosen=menu.Show(popup,anchor,dpi,font,{{1,T(L"Tên",L"Name"),true,0xea7b,sortKey==0},{2,T(L"Loại tệp",L"File type"),true,0xea7b,sortKey==1},{3,T(L"Ngày sửa đổi",L"Date modified"),true,0xea7b,sortKey==2},{4,T(L"Kích thước",L"Size"),true,0xea7b,sortKey==3},{10,T(L"Tăng dần",L"Ascending"),true,0xeaa1,ascending},{11,T(L"Giảm dần",L"Descending"),true,0xea9a,!ascending}});
        if(chosen>=1&&chosen<=4){sortKey=chosen-1;ascending=sortKey!=2&&sortKey!=3;}else if(chosen==10||chosen==11)ascending=chosen==10;else return;Sort();Rows();
    }
    void ViewMenu(){RECT anchor{};GetWindowRect(GetDlgItem(popup,30),&anchor);ToolMenu menu;RECT client{};GetClientRect(popup,&client);const int chosen=menu.Show(popup,anchor,dpi,font,{{1,T(L"Chi tiết",L"Details"),client.right>=S(640),0xea7b,details},{2,T(L"Danh sách gọn",L"Compact list"),true,0xea7b,!details}});
        if(!chosen)return;
        details=chosen==1;SendMessageW(list,LB_SETITEMHEIGHT,0,S(details?30:24));SetWindowTextW(GetDlgItem(popup,30),T(details?L"Chi tiết":L"Danh sách",details?L"Details":L"List"));Layout();InvalidateRect(list,nullptr,FALSE);
    }
    void Wheel(WPARAM wp){UINT lines=3;SystemParametersInfoW(SPI_GETWHEELSCROLLLINES,0,&lines,0);if(!lines)return;
        RECT r{};GetClientRect(list,&r);const int height=std::max(1,(int)SendMessageW(list,LB_GETITEMHEIGHT,0,0)),page=std::max(1,(int)r.bottom/height);
        if(lines==WHEEL_PAGESCROLL)lines=page;
        lines=std::min<UINT>(lines,1000);wheelRemainder+=GET_WHEEL_DELTA_WPARAM(wp)*(int)lines;
        const int movement=wheelRemainder/WHEEL_DELTA;wheelRemainder%=WHEEL_DELTA;if(!movement)return;
        const int top=(int)SendMessageW(list,LB_GETTOPINDEX,0,0),maximum=std::max(0,(int)visible.size()-page),next=std::clamp(top-movement,0,maximum);
        if(next!=top)SendMessageW(list,LB_SETTOPINDEX,next,0);
    }
    static LRESULT CALLBACK ListProc(HWND h,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR data){auto self=reinterpret_cast<LocalPicker*>(data);if(msg==WM_MOUSEWHEEL){self->Wheel(wp);return 0;}return DefSubclassProc(h,msg,wp,lp);}
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
    static LRESULT CALLBACK AddressProc(HWND h,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR data) {
        if(msg==WM_MOUSEWHEEL){if(auto self=reinterpret_cast<LocalPicker*>(data))self->Wheel(wp);return 0;}
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
                if(count==1 && At(selected.front())&&At(selected.front())->directory)chosen.push_back(At(selected.front())->path);
                else if(!folder.empty())chosen.push_back(folder);
            } else {
                if(count==1&&At(selected.front())&&At(selected.front())->directory){Navigate(At(selected.front())->path);return;}
                for(int index:selected)if(auto entry=At(index);entry&&!entry->directory&&chosen.size()<31)chosen.push_back(entry->path);
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
        capped=false;
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
        entries=std::move(next);folder=canonical;error.clear();order.clear();visible.clear();
        for(size_t i=0;i<entries.size();++i)order.push_back(i);
        Sort();SetWindowTextW(searchBox,L"");
        if(record&&(history.empty()||history[historyIndex]!=folder)) {
            if(!history.empty())history.resize(historyIndex+1);
            history.push_back(folder);
            if(history.size()>64)history.erase(history.begin());
            historyIndex=history.size()-1;
        }
        EnableWindow(GetDlgItem(popup,15),historyIndex>0);
        EnableWindow(GetDlgItem(popup,16),!history.empty()&&historyIndex+1<history.size());
        EnableWindow(GetDlgItem(popup,10),!folder.empty());
        SetWindowTextW(pathLabel,folder.empty()?L"":folder.c_str());
        for(int i=0;i<5;++i)InvalidateRect(GetDlgItem(popup,20+i),nullptr,FALSE);
        Rows(false);Layout();SetFocus(list);loading=false;
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
        RECT r{};GetClientRect(popup,&r);const int listLeft=r.right<S(640)?142:174;
        showDetails=details&&r.right>=S(640);const int rowHeight=S(showDetails?30:24);
        if(SendMessageW(list,LB_GETITEMHEIGHT,0,0)!=rowHeight)SendMessageW(list,LB_SETITEMHEIGHT,0,rowHeight);
        SetWindowTextW(GetDlgItem(popup,30),T(showDetails?L"Chi tiết":L"Danh sách",showDetails?L"Details":L"List"));
        MoveWindow(GetDlgItem(popup,15),S(12),S(14),S(34),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,16),S(50),S(14),S(34),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,10),S(88),S(14),S(34),S(30),TRUE);
        MoveWindow(pathLabel,S(136),S(14),std::max(S(32),int(r.right)-S(376)),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,17),r.right-S(232),S(14),S(32),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,18),S(20),S(68),S(listLeft-38),S(24),TRUE);
        for(int i=0;i<5;++i)MoveWindow(GetDlgItem(popup,20+i),S(12),S(100+i*42),S(listLeft-24),S(36),TRUE);
        MoveWindow(GetDlgItem(popup,29),r.right-S(192),S(14),S(88),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,30),r.right-S(96),S(14),S(84),S(30),TRUE);
        MoveWindow(GetDlgItem(popup,32),S(listLeft),S(56),S(56),S(28),TRUE);
        MoveWindow(searchBox,S(listLeft+58),S(56),r.right-S(listLeft+70),S(28),TRUE);
        MoveWindow(list,S(listLeft),S(126),r.right-S(listLeft+12),std::max(S(80),int(r.bottom)-S(204)),TRUE);
        RECT contents{};GetClientRect(list,&contents);int right=S(listLeft)+contents.right;
        MoveWindow(GetDlgItem(popup,19),S(listLeft+36),S(94),std::max(S(32),right-S(listLeft+36+(showDetails?266:8))),S(28),TRUE);
        MoveWindow(GetDlgItem(popup,31),right-S(260),S(94),S(74),S(28),TRUE);
        MoveWindow(GetDlgItem(popup,26),right-S(186),S(94),S(104),S(28),TRUE);
        MoveWindow(GetDlgItem(popup,27),right-S(82),S(94),S(80),S(28),TRUE);
        for(int id:{26,27,31})ShowWindow(GetDlgItem(popup,id),showDetails?SW_SHOW:SW_HIDE);
        MoveWindow(status,S(listLeft),r.bottom-S(68),r.right-S(listLeft+16),S(24),TRUE);
        MoveWindow(GetDlgItem(popup,IDCANCEL),r.right-S(228),r.bottom-S(38),S(92),S(30),TRUE);
        MoveWindow(accept,r.right-S(128),r.bottom-S(38),S(112),S(30),TRUE);
    }
    static LRESULT CALLBACK Proc(HWND h,UINT msg,WPARAM wp,LPARAM lp) {
        auto self=(LocalPicker*)GetWindowLongPtrW(h,GWLP_USERDATA);
        if(msg==WM_NCCREATE){self=(LocalPicker*)((CREATESTRUCTW*)lp)->lpCreateParams;self->popup=h;SetWindowLongPtrW(h,GWLP_USERDATA,(LONG_PTR)self);}
        if(!self)return DefWindowProcW(h,msg,wp,lp);
        if(msg==WM_CLOSE){self->Finish(false);return 0;}
        if(msg==WM_MOUSEWHEEL){self->Wheel(wp);return 0;}
        if(msg==WM_SIZE){if(self->list)self->Layout();return 0;}
        if(msg==WM_GETMINMAXINFO){auto p=(MINMAXINFO*)lp;p->ptMinTrackSize={self->S(480),self->S(360)};return 0;}
        if(msg==WM_COMMAND) {
            int id=LOWORD(wp);
            if(id==IDCANCEL)self->Finish(false);
            else if(id==IDOK)self->Finish(true);
            else if(id==10)self->Up();
            else if(id==11)self->Navigate(L"");
            else if(id==15||id==16)self->Travel(id==16);
            else if(id==17){wchar_t path[32768]{};GetWindowTextW(self->pathLabel,path,32768);self->Navigate(path);}
            else if(id>=20&&id<25)self->Navigate(self->shortcuts[id-20]);
            else if(id==19||id==26||id==27||id==31)self->ChangeSort(id==19?0:id==31?1:id==26?2:3);
            else if(id==29)self->SortMenu();else if(id==30)self->ViewMenu();
            else if(id==28&&HIWORD(wp)==EN_CHANGE&&!self->loading)self->Rows();
            else if(id==12&&HIWORD(wp)==LBN_SELCHANGE)EnableWindow(self->accept,self->foldersOnly?!self->folder.empty():SendMessageW(self->list,LB_GETSELCOUNT,0,0)>0);
            else if(id==12&&HIWORD(wp)==LBN_DBLCLK){int index=(int)SendMessageW(self->list,LB_GETCARETINDEX,0,0);
                if(auto entry=self->At(index)){if(entry->directory)self->Navigate(entry->path);else self->Finish(true);}}
            return 0;
        }
        if(msg==WM_CTLCOLORSTATIC||msg==WM_CTLCOLORLISTBOX||msg==WM_CTLCOLOREDIT){auto dc=(HDC)wp;SetBkColor(dc,theme.bg);SetTextColor(dc,theme.text);return (LRESULT)self->background;}
        if(msg==WM_ERASEBKGND){RECT r{};GetClientRect(h,&r);FillRect((HDC)wp,&r,self->background);return 1;}
        if(msg==WM_DRAWITEM) {
            auto item=(DRAWITEMSTRUCT*)lp;auto dc=item->hDC;RECT r=item->rcItem;
            auto brush=CreateSolidBrush((item->itemState&ODS_SELECTED)?theme.selection:theme.bg);FillRect(dc,&r,brush);DeleteObject(brush);
            if(item->CtlType==ODT_LISTBOX&&item->itemID<self->visible.size()) {
                auto &entry=self->entries[self->visible[item->itemID]];RECT icon=r;icon.right=icon.left+self->S(32);
                DrawCodicon(dc,entry.directory?0xeaf7:0xea7b,icon,theme.muted,self->S(16));r.left+=self->S(36);
                auto old=SelectObject(dc,self->font);SetTextColor(dc,theme.text);SetBkMode(dc,TRANSPARENT);
                RECT name=r;if(self->showDetails)name.right-=self->S(266);
                DrawTextW(dc,entry.name.c_str(),-1,&name,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
                if(self->showDetails){RECT detail=r;detail.left=r.right-self->S(260);detail.right=r.right-self->S(186);
                auto extension=Extension(entry);for(auto& c:extension)c=(wchar_t)towupper(c);
                DrawTextW(dc,(entry.directory?self->T(L"Thư mục",L"Folder"):extension.empty()?self->T(L"Tệp",L"File"):extension.c_str()),-1,&detail,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
                detail.left=r.right-self->S(182);detail.right=r.right-self->S(82);
                FILETIME local{};SYSTEMTIME time{};wchar_t date[80]{},clock[32]{};
                if(FileTimeToLocalFileTime(&entry.modified,&local)&&FileTimeToSystemTime(&local,&time)&&entry.modified.dwHighDateTime)
                    {GetDateFormatEx(LOCALE_NAME_USER_DEFAULT,DATE_SHORTDATE,&time,nullptr,date,80,nullptr);GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT,TIME_NOSECONDS|TIME_FORCE24HOURFORMAT,&time,nullptr,clock,32);}
                const auto stamp=std::wstring(date)+(date[0]&&clock[0]?L" ":L"")+clock;
                SetTextColor(dc,theme.muted);DrawTextW(dc,stamp.c_str(),-1,&detail,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
                if(!entry.directory){wchar_t size[40]{};swprintf_s(size,L"%llu KB",entry.bytes/1024+(entry.bytes%1024!=0));
                    detail.left=r.right-self->S(78);detail.right=r.right-self->S(8);
                    DrawTextW(dc,size,-1,&detail,DT_RIGHT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);}}
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
                } else if(item->CtlID==19||item->CtlID==26||item->CtlID==27||item->CtlID==31){const int key=item->CtlID==19?0:item->CtlID==31?1:item->CtlID==26?2:3;
                    if(sortKey==key){RECT arrow=r;arrow.left=arrow.right-self->S(20);DrawCodicon(dc,ascending?0xeaa0:0xea9d,arrow,theme.green,self->S(14));r.right-=self->S(22);}
                    r.left+=self->S(4);DrawTextW(dc,label,-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
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
        SetWindowSubclass(pathLabel,AddressProc,1,(DWORD_PTR)this);
        SendMessageW(pathLabel,EM_SETCUEBANNER,TRUE,(LPARAM)T(L"Máy tính này / Nhập đường dẫn thư mục",L"This PC / Enter folder path"));
        SendMessageW(pathLabel,EM_SETLIMITTEXT,32767,0);
        control(L"BUTTON",T(L"Đi tới",L"Go"),17,BS_OWNERDRAW|WS_TABSTOP);
        control(L"STATIC",T(L"Truy cập nhanh",L"Quick access"),18,SS_LEFT);
        control(L"BUTTON",T(L"Tên",L"Name"),19,BS_OWNERDRAW|WS_TABSTOP);
        control(L"BUTTON",T(L"Ngày sửa",L"Modified"),26,BS_OWNERDRAW|WS_TABSTOP);
        control(L"BUTTON",T(L"Kích thước",L"Size"),27,BS_OWNERDRAW|WS_TABSTOP);
        control(L"BUTTON",T(L"Loại",L"Type"),31,BS_OWNERDRAW|WS_TABSTOP);
        control(L"BUTTON",T(L"Sắp xếp",L"Sort"),29,BS_OWNERDRAW|WS_TABSTOP);
        control(L"BUTTON",T(details?L"Chi tiết":L"Danh sách",details?L"Details":L"List"),30,BS_OWNERDRAW|WS_TABSTOP);
        control(L"STATIC",T(L"Tìm tên",L"Find name"),32,SS_LEFT|SS_CENTERIMAGE);
        searchBox=control(L"EDIT",L"",28,WS_BORDER|ES_AUTOHSCROLL|WS_TABSTOP);SetWindowSubclass(searchBox,AddressProc,1,(DWORD_PTR)this);
        SendMessageW(searchBox,EM_SETLIMITTEXT,255,0);SendMessageW(searchBox,EM_SETCUEBANNER,TRUE,(LPARAM)T(L"Tìm tên trong thư mục này",L"Find a name in this folder"));
        const KNOWNFOLDERID *known[]={&FOLDERID_Desktop,&FOLDERID_Downloads,&FOLDERID_Documents,&FOLDERID_Pictures};
        const wchar_t *labels[]={L"Desktop",L"Downloads",T(L"Tài liệu",L"Documents"),T(L"Hình ảnh",L"Pictures"),T(L"Máy tính này",L"This PC")};
        for(int i=0;i<5;++i){HWND button=control(L"BUTTON",labels[i],20+i,BS_OWNERDRAW|WS_TABSTOP);
            if(i<4){PWSTR path=nullptr;HRESULT result=SHGetKnownFolderPath(*known[i],KF_FLAG_DONT_VERIFY,nullptr,&path);
                if(SUCCEEDED(result)&&path)shortcuts[i]=path;
                CoTaskMemFree(path);EnableWindow(button,!shortcuts[i].empty());}}
        status=control(L"STATIC",L"",14,SS_LEFT);
        list=control(L"LISTBOX",L"",12,LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_EXTENDEDSEL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|WS_VSCROLL|WS_TABSTOP);
        SetWindowTheme(list,theme.dark?L"DarkMode_Explorer":L"Explorer",nullptr);
        SendMessageW(list,LB_SETITEMHEIGHT,0,S(details?30:24));SetWindowSubclass(list,ListProc,1,(DWORD_PTR)this);
        control(L"BUTTON",T(L"Hủy",L"Cancel"),IDCANCEL,BS_OWNERDRAW|WS_TABSTOP);
        accept=control(L"BUTTON",directory?T(L"Chọn thư mục",L"Select folder"):T(L"Mở",L"Open"),IDOK,BS_OWNERDRAW|WS_TABSTOP);
        Layout();Navigate(initial);if(history.empty())Navigate(L"");EnableWindow(parent,FALSE);ShowWindow(popup,SW_SHOW);SetForegroundWindow(popup);SetFocus(list);
        MSG msg{};while(popup&&IsWindow(popup)) {
            int state=GetMessageW(&msg,nullptr,0,0);if(state<=0){Finish(false);if(state==0)PostQuitMessage((int)msg.wParam);break;}
            if(!IsWindow(parent)){Finish(false);break;}
            if(msg.message==WM_MOUSEWHEEL&&(msg.hwnd==popup||IsChild(popup,msg.hwnd))){Wheel(msg.wParam);continue;}
            if(msg.message==WM_KEYDOWN&&(msg.hwnd==popup||IsChild(popup,msg.hwnd))) {
                if(msg.wParam==VK_ESCAPE){Finish(false);continue;}
                if(msg.wParam==VK_RETURN){
                    int id=GetDlgCtrlID(msg.hwnd);
                    if(id==19||id==26||id==27||id==31||id==29||id==30)SendMessageW(popup,WM_COMMAND,id,0);
                    else if(id==28){if(!foldersOnly&&visible.size()==1)Finish(true);else SetFocus(list);}
                    else if(id==10)Up();else if(id==15||id==16)Travel(id==16);
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
