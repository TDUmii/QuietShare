#include "Browser.h"
#include "Privacy.h"
#include <WebView2.h>
#include "../vendor/webview2/WebGuids.h"
#include <algorithm>
#include <functional>
#include <cwctype>
#include <cmath>
#include <sstream>
#include <deque>
namespace quietshare {
namespace {
template<class T> struct Com {
    T* p=nullptr;~Com(){reset();}Com()=default;Com(const Com&)=delete;
    void reset(T* value=nullptr){T* previous=p;p=value;if(previous)previous->Release();}
    T* detach(){T* value=p;p=nullptr;return value;}
    void hold(T* value){if(value)value->AddRef();reset(value);}
    T** put(){reset();return &p;}T* operator->()const{return p;}explicit operator bool()const{return p!=nullptr;}
};
template<class I,class A,class B> class Handler final:public I {
    LONG refs=1;std::function<HRESULT(A,B)> fn;
public:
    explicit Handler(std::function<HRESULT(A,B)> f):fn(std::move(f)){}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id,void** result)override {
        if(!result)return E_POINTER;*result=nullptr;
        if(id==IID_IUnknown||id==WebGuid<I>()){*result=static_cast<I*>(this);AddRef();return S_OK;}return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&refs);}
    ULONG STDMETHODCALLTYPE Release()override{auto n=InterlockedDecrement(&refs);if(!n)delete this;return n;}
    HRESULT STDMETHODCALLTYPE Invoke(A a,B b)override{return fn(a,b);}
};
template<class I,class A,class B,class F> I* Callback(F&& fn){return new Handler<I,A,B>(std::forward<F>(fn));}
class PermissionCompletion final:public ICoreWebView2SetPermissionStateCompletedHandler {
    LONG refs=1;std::function<void(HRESULT)> fn;
public:
    explicit PermissionCompletion(std::function<void(HRESULT)> f):fn(std::move(f)){}
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID id,void** out)override{if(!out)return E_POINTER;*out=nullptr;
        if(id==IID_IUnknown||id==WebGuid<ICoreWebView2SetPermissionStateCompletedHandler>()){*out=this;AddRef();return S_OK;}return E_NOINTERFACE;}
    ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&refs);}
    ULONG STDMETHODCALLTYPE Release()override{auto count=InterlockedDecrement(&refs);if(!count)delete this;return count;}
    HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr)override{fn(hr);return S_OK;}
};
std::wstring TakeString(LPWSTR p){std::wstring result=p?p:L"";CoTaskMemFree(p);return result;}
// WebView2 supplies canonical URLs. Accept a deliberately narrow HTTPS origin
// here; malformed authorities, credentials and cross-origin frames fail closed.
std::wstring SecureMediaOrigin(const std::wstring& uri){
    std::wstring value=uri;std::transform(value.begin(),value.end(),value.begin(),::towlower);
    if(value.rfind(L"https://",0)!=0)return L"";
    auto authority=value.substr(8,value.find_first_of(L"/?#",8)-8);if(authority.empty())return L"";
    std::wstring host,port;size_t separator=std::wstring::npos;
    if(authority[0]==L'['){const auto end=authority.find(L']');if(end==std::wstring::npos||end<3)return L"";
        host=authority.substr(0,end+1);for(size_t i=1;i<end;++i)if(!((host[i]>=L'0'&&host[i]<=L'9')||(host[i]>=L'a'&&host[i]<=L'f')||host[i]==L':'||host[i]==L'.'))return L"";
        if(end+1<authority.size()){if(authority[end+1]!=L':')return L"";separator=end+1;}}
    else{separator=authority.find(L':');host=authority.substr(0,separator);for(wchar_t c:host)
        if(!((c>=L'a'&&c<=L'z')||(c>=L'0'&&c<=L'9')||c==L'-'||c==L'.'))return L"";}
    if(host.empty())return L"";
    if(separator!=std::wstring::npos){const auto digits=authority.substr(separator+1);unsigned number=0;if(digits.empty()||digits.size()>5)return L"";
        for(wchar_t c:digits){if(c<L'0'||c>L'9')return L"";number=number*10+c-L'0';}if(!number||number>65535)return L"";
        if(number!=443)port=L":"+std::to_wstring(number);}
    return L"https://"+host+port;
}
std::wstring CanaryHtml(){
    std::wstring html=L"<!doctype html><html><body style='margin:0;overflow:hidden'><svg xmlns='http://www.w3.org/2000/svg' width='100%' height='100%' viewBox='0 0 128 96' preserveAspectRatio='none' style='position:fixed;inset:0'>";
    for(int y=0;y<96;y+=8)for(int x=0;x<128;x+=8)
        html+=L"<rect x='"+std::to_wstring(x)+L"' y='"+std::to_wstring(y)+L"' width='8' height='8' fill='"+(((x/8+y/8)&1)?std::wstring(L"#fa14f0"):std::wstring(L"#14fa3c"))+L"'/>";
    return html+L"</svg></body></html>";
}
// Native browser dialogs/popups are outside the app-owned protection boundary.
// Suppress file uploads and native select popups; DOM dropdowns remain usable.
constexpr wchar_t kDomPolicy[]=LR"JS((()=>{
 const blocked=e=>e.target.closest&&e.target.closest('input[type=file],input[type=color],input[type=date],input[type=datetime-local],input[type=time],input[type=month],input[type=week],select');
 for(const name of ['click','mousedown','keydown'])addEventListener(name,e=>{
  const element=blocked(e);if(!element)return;
  if(name==='keydown'&&e.key==='Tab'&&!e.altKey&&!e.ctrlKey&&!e.metaKey)return;
  if(name==='keydown'&&element.tagName==='SELECT'&&!e.altKey&&!e.ctrlKey&&!e.metaKey&&['ArrowUp','ArrowDown','Tab'].includes(e.key))return;
  if(name==='keydown'&&element.tagName==='INPUT'&&!['file','color'].includes(element.type)&&!e.altKey&&!['F4','Enter',' '].includes(e.key))return;
  e.preventDefault();e.stopImmediatePropagation();
 },true);
 if(HTMLInputElement.prototype.showPicker)HTMLInputElement.prototype.showPicker=function(){throw new DOMException('File picker unavailable','NotAllowedError')};
 if(HTMLSelectElement.prototype.showPicker)HTMLSelectElement.prototype.showPicker=function(){throw new DOMException('Native popup unavailable','NotAllowedError')};
 const stripTitle=node=>{if(node.nodeType!==1)return;for(const key of ['title','list']){node.removeAttribute(key);for(const el of node.querySelectorAll('['+key+']'))el.removeAttribute(key)}};
 addEventListener('mouseover',e=>{if(e.target.closest){const el=e.target.closest('[title]');if(el)el.removeAttribute('title')}},true);
 const observer=new MutationObserver(records=>{for(const r of records){if(r.type==='attributes'){r.target.removeAttribute(r.attributeName)}else for(const n of r.addedNodes)stripTitle(n)}});
 observer.observe(document,{subtree:true,childList:true,attributes:true,attributeFilter:['title','list']});
 for(const name of ['paste','drop'])addEventListener(name,e=>{const data=e.clipboardData||e.dataTransfer;if(data&&data.files&&data.files.length){e.preventDefault();e.stopImmediatePropagation()}},true);
 Object.defineProperty(window,'print',{value:()=>{},writable:false,configurable:false});
 Object.defineProperty(window,'__quietShareNativeUiPolicy',{value:true});
})())JS";
constexpr wchar_t kShortcutCommand[]=L"https://quietshare.invalid/shortcut/";
constexpr size_t kShortcutLimit=10;
std::wstring Html(const std::wstring& value){
    std::wstring result;for(wchar_t c:value){switch(c){case L'&':result+=L"&amp;";break;case L'<':result+=L"&lt;";break;
        case L'>':result+=L"&gt;";break;case L'\"':result+=L"&quot;";break;case L'\'':result+=L"&#39;";break;default:result+=c;}}
    return result;
}
bool DecodeComponent(const std::wstring& value,std::wstring& output){
    output.clear();if(value.size()>24576)return false;std::string bytes;
    auto hex=[](wchar_t c){return c>=L'0'&&c<=L'9'?c-L'0':c>=L'A'&&c<=L'F'?c-L'A'+10:c>=L'a'&&c<=L'f'?c-L'a'+10:-1;};
    for(size_t i=0;i<value.size();++i){const wchar_t c=value[i];if(c==L'%'){if(i+2>=value.size())return false;
        int a=hex(value[++i]),b=hex(value[++i]);if(a<0||b<0)return false;bytes+=static_cast<char>(a*16+b);}
        else if(c>127)return false;else bytes+=static_cast<char>(c);}
    if(bytes.empty())return true;
    int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),nullptr,0);if(!count)return false;
    output.resize(count);return MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,bytes.data(),static_cast<int>(bytes.size()),output.data(),count)==count;
}
std::wstring HostHtmlUri(const std::wstring& html){
    const int count=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,html.data(),static_cast<int>(html.size()),nullptr,0,nullptr,nullptr);
    if(!count)return L"";std::string bytes(count,'\0');
    if(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,html.data(),static_cast<int>(html.size()),bytes.data(),count,nullptr,nullptr)!=count)return L"";
    constexpr wchar_t base64[]=L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::wstring uri=L"data:text/html;charset=utf-8;base64,";
    for(size_t i=0;i<bytes.size();i+=3){const unsigned a=static_cast<unsigned char>(bytes[i]);
        const unsigned b=i+1<bytes.size()?static_cast<unsigned char>(bytes[i+1]):0,c=i+2<bytes.size()?static_cast<unsigned char>(bytes[i+2]):0;
        uri+=base64[a>>2];uri+=base64[((a&3)<<4)|(b>>4)];uri+=i+1<bytes.size()?base64[((b&15)<<2)|(c>>6)]:L'=';uri+=i+2<bytes.size()?base64[c&63]:L'=';
    }return uri;
}
bool ReadShortcuts(const std::wstring& path,std::vector<BrowserShortcut>& result){
    result.clear();HANDLE file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return GetLastError()==ERROR_FILE_NOT_FOUND||GetLastError()==ERROR_PATH_NOT_FOUND;
    LARGE_INTEGER size{};const bool bounded=GetFileSizeEx(file,&size)&&size.QuadPart>=2&&size.QuadPart<=131072&&size.QuadPart%2==0;
    std::wstring text(bounded?static_cast<size_t>(size.QuadPart/2):0,L'\0');DWORD read=0;
    const bool readOk=bounded&&ReadFile(file,text.data(),static_cast<DWORD>(size.QuadPart),&read,nullptr)&&read==size.QuadPart;CloseHandle(file);
    if(!readOk||text[0]!=0xfeff)return false;
    size_t start=1;while(start<text.size()){
        auto end=text.find(L'\n',start);if(end==std::wstring::npos){result.clear();return false;}auto line=text.substr(start,end-start);auto split=line.find(L'\t');BrowserShortcut item;
        if(split==std::wstring::npos||result.size()>=kShortcutLimit||!NormalizeBrowserShortcut(line.substr(0,split),line.substr(split+1),item)){result.clear();return false;}
        result.push_back(std::move(item));start=end+1;
    }return true;
}
bool WriteShortcuts(const std::wstring& profile,const std::vector<BrowserShortcut>& values){
    if(profile.empty()||values.size()>kShortcutLimit)return false;
    std::wstring text(1,0xfeff);for(const auto& item:values)text+=item.name+L"\t"+item.address+L"\n";
    if(!CreateDirectoryW(profile.c_str(),nullptr)&&GetLastError()!=ERROR_ALREADY_EXISTS)return false;
    const auto path=profile+L"\\quietshare-shortcuts.txt",temp=path+L".tmp";
    HANDLE file=CreateFileW(temp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0,bytes=static_cast<DWORD>(text.size()*sizeof(wchar_t));bool ok=WriteFile(file,text.data(),bytes,&written,nullptr)&&written==bytes&&FlushFileBuffers(file);CloseHandle(file);
    if(ok)ok=MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=FALSE;
    if(!ok)DeleteFileW(temp.c_str());return ok;
}
std::wstring NewTabHtml(bool en,const std::vector<BrowserShortcut>& shortcuts,const std::wstring& nonce,const std::wstring& error){
    auto t=[&](const wchar_t* vi,const wchar_t* english){return en?english:vi;};
    const std::wstring globe=L"<svg viewBox='0 0 24 24' aria-hidden='true'><circle cx='12' cy='12' r='9'/><path d='M3 12h18M12 3a17 17 0 0 1 0 18 17 17 0 0 1 0-18'/></svg>";
    const std::wstring plus=L"<svg viewBox='0 0 24 24' aria-hidden='true'><path d='M12 5v14M5 12h14'/></svg>";
    std::wstring html=LR"HTML(<!doctype html><html><head><meta charset='utf-8'><meta name='referrer' content='no-referrer'><meta http-equiv='Content-Security-Policy' content="default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; form-action 'none'; base-uri 'none'"><title>)HTML";
    html+=t(L"Tab mới",L"New tab");
    html+=LR"HTML(</title><style>
*{box-sizing:border-box}html{color-scheme:dark;scrollbar-color:#484e50 #1e1e1e}body{margin:0;background:#1e1e1e;color:#f2f5f7;font:13px 'Segoe UI',sans-serif;min-height:100vh;display:grid;place-items:center}::selection{background:#094771;color:#fff}main{width:100%;max-width:500px;padding:24px 18px}h1{font-size:20px;line-height:1.25;font-weight:600;text-align:center;margin:0 0 20px}button,input{font:inherit;color:inherit}button,a{touch-action:manipulation}button{cursor:pointer}button:disabled{opacity:.45;cursor:default}a,button,input{outline:none}a:focus-visible,button:focus-visible,input:focus-visible{outline:2px solid #007acc;outline-offset:3px}input{width:100%;background:#252526;border:1px solid #414749;border-radius:7px;padding:9px 11px;caret-color:#007acc}input::placeholder{color:#aaaaaa}.search{margin-bottom:22px}.search input{border-radius:18px;padding:10px 16px;background:#252526}.grid{display:flex;justify-content:center;flex-wrap:wrap;gap:10px 6px}.tile{position:relative;flex:0 0 82px;width:82px;border-radius:9px}.tile>a,.add{display:flex;align-items:center;flex-direction:column;gap:8px;width:100%;min-height:88px;padding:10px 3px 7px;color:inherit;text-decoration:none;border:0;background:transparent;border-radius:9px}.tile:hover,.add:hover{background:#37373d}.symbol{display:grid;place-items:center;width:36px;height:36px;border-radius:50%;background:#333337;color:#9cdcfe}svg{width:18px;height:18px;fill:none;stroke:currentColor;stroke-width:1.7;stroke-linecap:round;stroke-linejoin:round}.caption{display:block;max-width:76px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font-size:12px}.edit{position:absolute;right:0;top:0;display:grid;place-items:center;width:23px;height:23px;background:#343a3b;border:0;border-radius:50%;opacity:0}.edit svg{width:14px;height:14px}.tile:hover .edit,.edit:focus-visible{opacity:1}.edit:hover{background:#485052}form[hidden],.grid[hidden]{display:none}.editor{margin:0 auto;max-width:360px}.editor label{display:block;color:#bbc5c0;font-size:12px;margin:12px 0 6px}.actions{display:flex;justify-content:flex-end;align-items:center;gap:7px;margin-top:15px}.actions button{background:#333337;border:0;border-radius:6px;min-height:31px;padding:6px 12px}.actions button:hover{background:#41494b}.actions .save{background:#007acc;color:#ffffff;font-weight:600}.actions .save:hover{background:#168bd2}.actions .remove{margin-right:auto;color:#ffa9a2;background:transparent}.notice{margin:14px 0 0;color:#ffbbb4;font-size:12px;text-align:center;min-height:16px}.empty{color:#aaaaaa;text-align:center;font-size:12px;line-height:1.5;margin:8px 0 14px}@media(max-width:340px),(max-height:260px){main{padding:14px 12px}h1{font-size:18px;margin-bottom:14px}.search{margin-bottom:12px}.tile>a,.add{min-height:76px;padding-top:6px;gap:6px}.symbol{width:30px;height:30px}.grid{gap:4px}}
</style><style>.notice:empty{display:none}@media(max-height:260px){h1,.empty{display:none}main{padding:12px}.search{margin-bottom:10px}}</style></head><body><main><h1>QuietShare</h1><form class='search' id='search'><input id='query' autocomplete='off' aria-label=')HTML";
    const std::wstring search=t(L"Tìm kiếm hoặc nhập địa chỉ web",L"Search or enter a web address");html+=Html(search)+L"' placeholder='"+Html(search)+L"'></form><div class='grid' id='grid'>";
    for(size_t i=0;i<shortcuts.size();++i){const auto& item=shortcuts[i];html+=L"<div class='tile'><a href='"+Html(item.address)+L"'><span class='symbol'>"+globe+L"</span><span class='caption'>"+Html(item.name)+L"</span></a><button class='edit' data-index='"+std::to_wstring(i)+L"' data-name='"+Html(item.name)+L"' data-url='"+Html(item.address)+L"' aria-label='"+Html(std::wstring(t(L"Sửa ",L"Edit "))+item.name)+L"'><svg viewBox='0 0 24 24'><circle cx='5' cy='12' r='1'/><circle cx='12' cy='12' r='1'/><circle cx='19' cy='12' r='1'/></svg></button></div>";}
    if(shortcuts.size()<kShortcutLimit)html+=L"<div class='tile'><button class='add' id='add'><span class='symbol'>"+plus+L"</span><span class='caption'>"+t(L"Thêm lối tắt",L"Add shortcut")+L"</span></button></div>";
    html+=L"</div>";if(shortcuts.empty())html+=L"<p class='empty' id='empty'>"+std::wstring(t(L"Lưu trang bạn dùng để mở nhanh tại đây.",L"Save the pages you use for quick access here."))+L"</p>";
    html+=L"<form class='editor' id='editor' hidden><label for='name'>"+std::wstring(t(L"Tên",L"Name"))+L"</label><input id='name' maxlength='80' autocomplete='off'><label for='url'>"+t(L"Địa chỉ web",L"Web address")+L"</label><input id='url' maxlength='4096' autocomplete='off' placeholder='https://example.com'><div class='actions'><button type='button' class='remove' id='remove'>"+t(L"Xóa",L"Remove")+L"</button><button type='button' id='cancel'>"+t(L"Hủy",L"Cancel")+L"</button><button class='save' id='save'>"+t(L"Lưu",L"Save")+L"</button></div></form><p class='notice' id='notice' role='status'>"+Html(error)+L"</p>";
    html+=LR"JS(<script>(()=>{'use strict';const $=id=>document.getElementById(id);let index=-1;const prefix=')JS"+std::wstring(kShortcutCommand)+nonce+LR"JS(/';
const send=(action,value='')=>{location.href=prefix+action+'/'+index+'?'+value};
const edit=(n,name='',url='')=>{index=n;$('grid').hidden=true;$('search').hidden=true;if($('empty'))$('empty').hidden=true;$('editor').hidden=false;$('name').value=name;$('url').value=url;$('remove').hidden=n<0;$('name').focus()};
const cancel=()=>{$('editor').hidden=true;$('grid').hidden=false;$('search').hidden=false;if($('empty'))$('empty').hidden=false};
if($('add'))$('add').onclick=()=>edit(-1);for(const button of document.querySelectorAll('.edit'))button.onclick=()=>edit(Number(button.dataset.index),button.dataset.name,button.dataset.url);
$('cancel').onclick=cancel;$('remove').onclick=()=>send('remove');$('editor').onsubmit=e=>{e.preventDefault();send('save','name='+encodeURIComponent($('name').value)+'&url='+encodeURIComponent($('url').value))};
$('search').onsubmit=e=>{e.preventDefault();const value=$('query').value.trim();if(value)send('search','value='+encodeURIComponent(value))};
addEventListener('keydown',e=>{if(e.key==='Escape'&&!$('editor').hidden){e.preventDefault();cancel()}});
})()</script></main></body></html>)JS";
    return html;
}
}
bool NormalizeWebAddress(const std::wstring& input,std::wstring& output){
    output.clear();if(input.empty()||input.size()>4096)return false;
    for(wchar_t c:input)if(c<=32||c==L'\\'||c==L'"'||c==L'<'||c==L'>')return false;
    std::wstring address=input,lower=input;std::transform(lower.begin(),lower.end(),lower.begin(),::towlower);
    if(lower.rfind(L"https://",0)!=0&&lower.rfind(L"http://",0)!=0){if(input.find(L':')!=std::wstring::npos)return false;address=L"https://"+input;}
    auto start=address.find(L"://")+3,end=address.find_first_of(L"/?#",start);
    auto host=address.substr(start,end-start);if(host.empty()||host.find(L'@')!=std::wstring::npos)return false;
    output=address;return true;
}
bool ResolveWebInput(const std::wstring& input,std::wstring& output){
    output.clear();if(input.size()>4096)return false;
    auto begin=input.find_first_not_of(L" \t\r\n"),end=input.find_last_not_of(L" \t\r\n");
    if(begin==std::wstring::npos)return false;
    std::wstring value=input.substr(begin,end-begin+1),lower=value;
    std::transform(lower.begin(),lower.end(),lower.begin(),::towlower);
    for(wchar_t c:value)if(c<32||c==127)return false;
    if(lower.rfind(L"https://",0)==0||lower.rfind(L"http://",0)==0)return NormalizeWebAddress(value,output);
    auto authority=value.substr(0,value.find_first_of(L"/?#")),host=authority;
    const auto colon=authority.find(L':');
    bool port=false;
    if(colon!=std::wstring::npos){
        const auto digits=authority.substr(colon+1);
        port=!digits.empty()&&digits.size()<=5&&std::all_of(digits.begin(),digits.end(),[](wchar_t c){return c>=L'0'&&c<=L'9';});
        host=authority.substr(0,colon);
    }
    std::wstring hostLower=host;std::transform(hostLower.begin(),hostLower.end(),hostLower.begin(),::towlower);
    const bool bareHost=host.find(L'.')!=std::wstring::npos||hostLower==L"localhost";
    // A scheme must never turn into a request to an external/native application.
    if(colon!=std::wstring::npos&&!(bareHost&&port)){
        const bool scheme=colon>0&&std::iswalpha(value[0])&&std::all_of(value.begin(),value.begin()+colon,
            [](wchar_t c){return std::iswalnum(c)||c==L'+'||c==L'-'||c==L'.';});
        if(scheme)return false;
    }
    if(bareHost&&value.find_first_of(L" \t\r\n")==std::wstring::npos&&(colon==std::wstring::npos||port))
        return NormalizeWebAddress(L"https://"+value,output);
    const int bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),static_cast<int>(value.size()),nullptr,0,nullptr,nullptr);
    if(!bytes)return false;
    std::string utf8(bytes,'\0');
    if(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),static_cast<int>(value.size()),utf8.data(),bytes,nullptr,nullptr)!=bytes)return false;
    std::wstring address=L"https://www.google.com/search?q=";constexpr wchar_t hex[]=L"0123456789ABCDEF";
    for(unsigned char c:utf8){
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'||c=='~')address+=wchar_t(c);
        else{address+=L'%';address+=hex[c>>4];address+=hex[c&15];}
    }
    return NormalizeWebAddress(address,output);
}
bool NormalizeBrowserShortcut(const std::wstring& name,const std::wstring& address,BrowserShortcut& output){
    output={};if(name.size()>80||address.size()>4096)return false;
    const auto start=name.find_first_not_of(L' '),end=name.find_last_not_of(L' ');
    if(start==std::wstring::npos)return false;
    std::wstring clean=name.substr(start,end-start+1);for(wchar_t c:clean)if(c<32||c==127)return false;
    if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,clean.data(),static_cast<int>(clean.size()),nullptr,0,nullptr,nullptr))return false;
    auto beginUrl=address.find_first_not_of(L' '),endUrl=address.find_last_not_of(L' ');std::wstring normalized;
    if(beginUrl==std::wstring::npos||!NormalizeWebAddress(address.substr(beginUrl,endUrl-beginUrl+1),normalized)||normalized.size()>4096||
       !WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,normalized.data(),static_cast<int>(normalized.size()),nullptr,0,nullptr,nullptr))return false;
    output={std::move(clean),std::move(normalized)};return true;
}
struct Browser::Impl:std::enable_shared_from_this<Browser::Impl> {
    HWND parent=nullptr;RECT bounds{};bool enabled=false,ready=false,verified=false,visible=false,probing=false,locked=false,suspendPending=false;
    unsigned generation=0,pid=0;UINT64 navigationId=0;int error=0;bool loaded=false,navigating=false;std::wstring address,pending,notice,title;
    int zoomPercent=DefaultZoomPercent;
    bool english=false,localPage=false,localNavigation=false,hasNewTab=false;
    bool muted=false,allowCamera=false,allowMicrophone=false,mediaReady=false;
    std::wstring cameraOrigin,microphoneOrigin;
    std::wstring profile,localNonce,localDocumentUri,newTabError;
    std::vector<BrowserShortcut> shortcuts;
    std::vector<BrowserCommandShortcut> commandShortcuts;
    // The manager owns one environment; tab states own only their controller.
    std::weak_ptr<Impl> owner;
    std::vector<std::shared_ptr<Impl>> tabs;
    unsigned id=0,nextId=0,activeId=0,pendingId=0;
    struct Action {unsigned generation,id;int kind,value;std::wstring address;};
    std::deque<Action> actions;
    Com<ICoreWebView2Environment> environment;Com<ICoreWebView2Controller> controller;Com<ICoreWebView2> web;
    std::shared_ptr<Impl> Find(unsigned token)const {for(const auto& tab:tabs)if(tab->id==token)return tab;return {};}
    std::shared_ptr<Impl> Active()const{return Find(activeId);}
    bool Current(const std::shared_ptr<Impl>& tab,unsigned epoch)const{return enabled&&generation==epoch&&tab&&tab->enabled&&Find(tab->id)==tab;}
    void Notify(){auto manager=owner.lock();if(IsWindow(parent))PostMessageW(parent,BrowserChangedMessage,manager?manager->generation:generation,id);}
    void NotifyZoom(){auto manager=owner.lock();if(IsWindow(parent)&&(!manager||manager->activeId==id))PostMessageW(parent,BrowserZoomChangedMessage,manager?manager->generation:generation,0);}
    void Queue(int kind,int value=0,std::wstring address={}){
        auto manager=owner.lock();if(!manager||manager->actions.size()>=16)return;
        manager->actions.push_back({manager->generation,id,kind,value,std::move(address)});
        PostMessageW(parent,BrowserTabActionMessage,manager->generation,BrowserTabQueued);
    }
    void LoadNewTab(){
        auto manager=owner.lock();if(!manager||!enabled||!ready||!verified||locked||!web)return;
        GUID guid{};if(FAILED(CoCreateGuid(&guid))){Fail(331);return;}
        localNonce.clear();constexpr wchar_t hex[]=L"0123456789abcdef";for(unsigned char c:std::string(reinterpret_cast<const char*>(&guid),sizeof(guid))){localNonce+=hex[c>>4];localNonce+=hex[c&15];}
        hasNewTab=localPage=localNavigation=true;address.clear();title=manager->english?L"New tab":L"Tab mới";loaded=false;navigating=true;notice.clear();
        auto html=NewTabHtml(manager->english,manager->shortcuts,localNonce,newTabError);
        localDocumentUri=HostHtmlUri(html);if(localDocumentUri.empty()){Fail(332);return;}
        if(FAILED(web->NavigateToString(html.c_str()))){localPage=localNavigation=false;navigating=false;notice=L"navigation";}Notify();
    }
    void Close(){++generation;enabled=ready=verified=visible=probing=locked=suspendPending=mediaReady=false;pid=0;error=0;loaded=navigating=localPage=localNavigation=false;cameraOrigin.clear();microphoneOrigin.clear();localNonce.clear();localDocumentUri.clear();newTabError.clear();address.clear();pending.clear();notice.clear();title.clear();
        auto closingTabs=std::move(tabs);tabs.clear();actions.clear();activeId=pendingId=0;
        Com<ICoreWebView2Controller> closingController;closingController.reset(controller.detach());
        Com<ICoreWebView2> closingWeb;closingWeb.reset(web.detach());
        Com<ICoreWebView2Environment> closingEnvironment;closingEnvironment.reset(environment.detach());
        // Revoke state before COM can pump nested host messages or callbacks.
        for(auto& tab:closingTabs)tab->Close();
        if(closingController){closingController->put_IsVisible(FALSE);closingController->Close();}Notify();}
    void Fail(int code=1){++generation;ready=verified=visible=probing=navigating=false;if(!error)error=code;
        if(auto manager=owner.lock();manager&&manager->pendingId==id)manager->pendingId=0;
        Com<ICoreWebView2Controller> closingController;closingController.reset(controller.detach());
        Com<ICoreWebView2> closingWeb;closingWeb.reset(web.detach());
        if(closingController){closingController->put_IsVisible(FALSE);closingController->Close();}Notify();}
    template<class I> bool Query(Com<I>& out){Com<ICoreWebView2> source;source.hold(web.p);return source&&SUCCEEDED(source->QueryInterface(WebGuid<I>(),reinterpret_cast<void**>(out.put())));}
    bool ApplyMute(){
        auto manager=owner.lock();if(!manager||!web)return false;
        Com<ICoreWebView2_8> audio;
        // An unavailable mute API must never leave requested audio suppression ineffective.
        return Query(audio)?SUCCEEDED(audio->put_IsMuted(manager->muted)):!manager->muted;
    }
    std::shared_ptr<Com<ICoreWebView2Profile4>> MediaProfile(){
        Com<ICoreWebView2_13> view;Com<ICoreWebView2Profile> profile;
        auto result=std::make_shared<Com<ICoreWebView2Profile4>>();
        if(!Query(view)||FAILED(view->get_Profile(profile.put()))||!profile||
           FAILED(profile->QueryInterface(WebGuid<ICoreWebView2Profile4>(),reinterpret_cast<void**>(result->put()))))return {};
        return result;
    }
    void ResetMediaPermissions(std::function<void(bool)> done){
        auto profile=MediaProfile();if(!profile){done(false);return;}
        auto listed=Callback<ICoreWebView2GetNonDefaultPermissionSettingsCompletedHandler,HRESULT,ICoreWebView2PermissionSettingCollectionView*>(
            [profile,done](HRESULT hr,auto settings){
                UINT count=0;if(FAILED(hr)||!settings||FAILED(settings->get_Count(&count))){done(false);return S_OK;}
                std::vector<std::pair<COREWEBVIEW2_PERMISSION_KIND,std::wstring>> media;
                for(UINT i=0;i<count;++i){Com<ICoreWebView2PermissionSetting> setting;COREWEBVIEW2_PERMISSION_KIND kind{};
                    if(FAILED(settings->GetValueAtIndex(i,setting.put()))||!setting||FAILED(setting->get_PermissionKind(&kind))){done(false);return S_OK;}
                    if(kind!=COREWEBVIEW2_PERMISSION_KIND_CAMERA&&kind!=COREWEBVIEW2_PERMISSION_KIND_MICROPHONE)continue;
                    LPWSTR raw=nullptr;if(FAILED(setting->get_PermissionOrigin(&raw))){done(false);return S_OK;}
                    media.push_back({kind,TakeString(raw)});
                }
                if(media.empty()){done(true);return S_OK;}
                auto pending=std::make_shared<size_t>(media.size());auto success=std::make_shared<bool>(true);
                for(const auto& item:media){auto complete=new PermissionCompletion([profile,pending,success,done](HRESULT result){
                    if(FAILED(result))*success=false;if(!--*pending)done(*success);});
                    const HRESULT result=profile->p->SetPermissionState(item.first,item.second.c_str(),COREWEBVIEW2_PERMISSION_STATE_DEFAULT,complete);
                    if(FAILED(result))complete->Invoke(result);complete->Release();}
                return S_OK;
            });
        const HRESULT result=profile->p->GetNonDefaultPermissionSettings(listed);if(FAILED(result))done(false);listed->Release();
    }
    void RevokeMedia(bool camera,bool microphone){
        std::vector<std::pair<COREWEBVIEW2_PERMISSION_KIND,std::wstring>> revoked;
        if(camera&&!cameraOrigin.empty())revoked.push_back({COREWEBVIEW2_PERMISSION_KIND_CAMERA,cameraOrigin});
        if(microphone&&!microphoneOrigin.empty())revoked.push_back({COREWEBVIEW2_PERMISSION_KIND_MICROPHONE,microphoneOrigin});
        if(revoked.empty())return;
        if(camera)cameraOrigin.clear();if(microphone)microphoneOrigin.clear();
        auto profile=MediaProfile();if(!profile){Fail(341);return;}
        std::weak_ptr<Impl> weak=shared_from_this();const unsigned epoch=generation;
        auto pending=std::make_shared<size_t>(revoked.size());
        for(const auto& item:revoked){auto denied=new PermissionCompletion([profile,weak,epoch,pending,item](HRESULT hr){
            auto s=weak.lock();if(!s||!s->enabled||s->generation!=epoch)return;
            if(FAILED(hr)){s->Fail(342);return;}
            // DENY revokes the native permission. Restore DEFAULT afterwards:
            // only the host's nonpersistent permission handler may grant access.
            auto reset=new PermissionCompletion([profile,weak,epoch,pending](HRESULT result){
                auto current=weak.lock();if(!current||!current->enabled||current->generation!=epoch)return;
                if(FAILED(result)){current->Fail(343);return;}
                if(!--*pending&&current->web){Com<ICoreWebView2> web;web.hold(current->web.p);
                    Com<ICoreWebView2_3> control;if(current->Query(control))control->Resume();
                    if(!current->enabled||current->generation!=epoch||current->web.p!=web.p)return;
                    const HRESULT reloaded=web->Reload();if(current->enabled&&current->generation==epoch&&FAILED(reloaded))current->Fail(344);}
            });
            const HRESULT result=profile->p->SetPermissionState(item.first,item.second.c_str(),COREWEBVIEW2_PERMISSION_STATE_DEFAULT,reset);
            if(FAILED(result))reset->Invoke(result);reset->Release();
        });
            const HRESULT result=profile->p->SetPermissionState(item.first,item.second.c_str(),COREWEBVIEW2_PERMISSION_STATE_DENY,denied);
            if(FAILED(result))denied->Invoke(result);denied->Release();
        }
    }
    bool SetZoom(int percent){
        const int next=std::clamp(percent,25,400);
        if(enabled&&controller&&!probing){
            const unsigned epoch=generation;Com<ICoreWebView2Controller> current;current.hold(controller.p);
            if(!ready||locked||FAILED(current->put_ZoomFactor(next/100.0))||generation!=epoch||!enabled||locked)return false;
        }
        const bool changed=zoomPercent!=next;zoomPercent=next;if(changed)NotifyZoom();return true;
    }
    void Suspend(){
        if(!ready||probing||visible||suspendPending)return;
        Com<ICoreWebView2_3> control;if(!Query(control))return;suspendPending=true;
        const unsigned epoch=generation;std::weak_ptr<Impl> weak=shared_from_this();
        auto done=Callback<ICoreWebView2TrySuspendCompletedHandler,HRESULT,BOOL>([weak,epoch](auto,auto){
            if(auto s=weak.lock();s&&s->generation==epoch){s->suspendPending=false;if(s->visible){Com<ICoreWebView2_3> c;if(s->Query(c))c->Resume();}}return S_OK;
        });
        if(FAILED(control->TrySuspend(done)))suspendPending=false;done->Release();
    }
    void SetVisible(bool show){
        const bool next=show&&verified&&!locked;
        if(next==visible)return;
        visible=next;
        if(controller&&!probing){const unsigned epoch=generation;Com<ICoreWebView2Controller> current;current.hold(controller.p);
            if(visible){Com<ICoreWebView2_3> control;if(Query(control))control->Resume();}
            if(!enabled||generation!=epoch||current.p!=controller.p)return;
            current->put_IsVisible(visible);if(enabled&&generation==epoch&&!visible)Suspend();}
    }
    void UpdateVisible(){const auto snapshot=tabs;const unsigned epoch=generation;for(const auto& tab:snapshot){
        if(!enabled||generation!=epoch)return;if(!Current(tab,epoch))continue;
        if(visible&&!locked&&tab->id==activeId&&tab->verified)tab->locked=false;tab->SetVisible(visible&&!locked&&tab->id==activeId);}}
    void CreateController(const std::shared_ptr<Impl>& tab){
        if(!Current(tab,generation)||locked)return;
        const unsigned epoch=generation,token=tab->id;std::weak_ptr<Impl> weak=shared_from_this();
        auto created=Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,HRESULT,ICoreWebView2Controller*>([weak,epoch,token](HRESULT hr,auto controller){
            auto manager=weak.lock();auto current=manager?manager->Find(token):nullptr;
            if(!manager||!manager->enabled||manager->locked||manager->generation!=epoch||!current||!current->enabled){if(controller)controller->Close();return S_OK;}
            if(FAILED(hr)||!controller){current->Fail();return S_OK;}
            const unsigned tabEpoch=current->generation;Com<ICoreWebView2Controller> incoming;incoming.hold(controller);current->controller.hold(controller);
            auto valid=[&](){return manager->Current(current,epoch)&&!manager->locked&&current->generation==tabEpoch&&current->controller.p==incoming.p;};
            hr=incoming->put_IsVisible(FALSE);if(!valid())return S_OK;if(FAILED(hr)){current->Fail();return S_OK;}
            Com<ICoreWebView2> web;hr=incoming->get_CoreWebView2(web.put());if(!valid())return S_OK;if(FAILED(hr)||!web){current->Fail();return S_OK;}
            current->web.reset(web.detach());UINT pid=0;hr=current->web->get_BrowserProcessId(&pid);if(!valid())return S_OK;
            if(FAILED(hr)){current->Fail();return S_OK;}current->pid=pid;
            current->probing=true;const bool configured=current->Configure();if(!valid())return S_OK;if(!configured)current->Fail();current->Notify();return S_OK;
        });
        Com<ICoreWebView2Environment> currentEnvironment;currentEnvironment.hold(environment.p);
        const auto hr=currentEnvironment?currentEnvironment->CreateCoreWebView2Controller(parent,created):E_ABORT;created->Release();if(FAILED(hr)&&Current(tab,epoch))tab->Fail();
    }
    std::shared_ptr<Impl> AddTab(const std::wstring& initial){
        auto tab=std::make_shared<Impl>();tab->owner=shared_from_this();tab->parent=parent;tab->bounds=bounds;tab->id=++nextId;tab->enabled=true;tab->address=tab->pending=initial;
        if(tabs.empty())tab->zoomPercent=zoomPercent;
        actions.clear();tabs.push_back(tab);activeId=pendingId=tab->id;UpdateVisible();Notify();return tab;
    }
    template<class I,class A,class B,class F,class Register> bool Event(F&& fn,Register reg){
        auto handler=Callback<I,A,B>(std::forward<F>(fn));EventRegistrationToken token{};auto hr=reg(handler,&token);handler->Release();return SUCCEEDED(hr);
    }
    bool Configure(){
        Com<ICoreWebView2> web;web.hold(this->web.p);Com<ICoreWebView2Controller> controller;controller.hold(this->controller.p);if(!web||!controller)return false;
        error=340;if(!ApplyMute())return false;
        error=10;Com<ICoreWebView2Settings> settings;if(FAILED(web->get_Settings(settings.put())))return false;
        if(FAILED(settings->put_AreDefaultContextMenusEnabled(FALSE))||FAILED(settings->put_AreDefaultScriptDialogsEnabled(FALSE))||
           FAILED(settings->put_AreDevToolsEnabled(FALSE))||FAILED(settings->put_AreHostObjectsAllowed(FALSE))||
           FAILED(settings->put_IsWebMessageEnabled(FALSE))||FAILED(settings->put_IsStatusBarEnabled(FALSE))||
           FAILED(settings->put_IsZoomControlEnabled(FALSE))||FAILED(controller->put_ZoomFactor(1.0)))return false;
        Com<ICoreWebView2Settings3> settings3;Com<ICoreWebView2Settings4> settings4;Com<ICoreWebView2Settings5> settings5;
        error=11;if(FAILED(settings->QueryInterface(WebGuid<ICoreWebView2Settings4>(),reinterpret_cast<void**>(settings4.put())))||
           FAILED(settings->QueryInterface(WebGuid<ICoreWebView2Settings5>(),reinterpret_cast<void**>(settings5.put()))))return false;
        if(FAILED(settings4->put_IsGeneralAutofillEnabled(FALSE))||FAILED(settings4->put_IsPasswordAutosaveEnabled(FALSE))||FAILED(settings5->put_IsPinchZoomEnabled(TRUE)))return false;
        error=12;if(FAILED(settings->QueryInterface(WebGuid<ICoreWebView2Settings3>(),reinterpret_cast<void**>(settings3.put()))))return false;
        if(FAILED(settings3->put_AreBrowserAcceleratorKeysEnabled(FALSE)))return false;
        Com<ICoreWebView2Controller4> input;Com<ICoreWebView2Settings7> pdf;
        if(FAILED(controller->QueryInterface(WebGuid<ICoreWebView2Controller4>(),reinterpret_cast<void**>(input.put())))||FAILED(input->put_AllowExternalDrop(FALSE)))return false;
        if(FAILED(settings->QueryInterface(WebGuid<ICoreWebView2Settings7>(),reinterpret_cast<void**>(pdf.put())))||
           FAILED(pdf->put_HiddenPdfToolbarItems(static_cast<COREWEBVIEW2_PDF_TOOLBAR_ITEMS>(COREWEBVIEW2_PDF_TOOLBAR_ITEMS_SAVE|COREWEBVIEW2_PDF_TOOLBAR_ITEMS_SAVE_AS|COREWEBVIEW2_PDF_TOOLBAR_ITEMS_PRINT))))return false;
        Com<ICoreWebView2_4> download;Com<ICoreWebView2_5> certificates;Com<ICoreWebView2_10> authentication;
        Com<ICoreWebView2_18> external;Com<ICoreWebView2_27> capture;
        error=13;if(!Query(download))return false;error=14;if(!Query(external))return false;error=15;if(!Query(capture))return false;error=16;
        if(!Query(certificates)||!Query(authentication))return false;
        std::weak_ptr<Impl> weak=shared_from_this();const unsigned epoch=generation;
        auto valid=[weak,epoch](){auto s=weak.lock();auto manager=s?s->owner.lock():nullptr;return s&&s->enabled&&s->generation==epoch&&manager&&manager->enabled?s:nullptr;};
        bool ok=true;
        ok=Event<ICoreWebView2AcceleratorKeyPressedEventHandler,ICoreWebView2Controller*,ICoreWebView2AcceleratorKeyPressedEventArgs*>([valid](auto,auto args){
            auto s=valid();if(!s||!s->ready||!s->verified||s->locked||!s->visible)return S_OK;
            COREWEBVIEW2_KEY_EVENT_KIND kind{};UINT key=0;
            if(FAILED(args->get_KeyEventKind(&kind))||FAILED(args->get_VirtualKey(&key)))return S_OK;
            const UINT modifiers=((GetKeyState(VK_CONTROL)&0x8000)?MOD_CONTROL:0)|((GetKeyState(VK_MENU)&0x8000)?MOD_ALT:0)|
                ((GetKeyState(VK_SHIFT)&0x8000)?MOD_SHIFT:0)|(((GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN))&0x8000)?MOD_WIN:0);
            if(auto manager=s->owner.lock())for(const auto& shortcut:manager->commandShortcuts)if(shortcut.key==key&&shortcut.modifiers==modifiers){
                if(FAILED(args->put_Handled(TRUE)))return S_OK;
                COREWEBVIEW2_PHYSICAL_KEY_STATUS status{};args->get_PhysicalKeyStatus(&status);
                if(!status.WasKeyDown&&(kind==COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN||kind==COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN))s->Queue(9,shortcut.command);
                return S_OK;
            }
            if(!(GetKeyState(VK_CONTROL)&0x8000)||(GetKeyState(VK_MENU)&0x8000)||
               (GetKeyState(VK_LWIN)&0x8000)||(GetKeyState(VK_RWIN)&0x8000))return S_OK;
            int action=0,delta=0;
            if(key==L'T')action=BrowserTabNew;
            else if(key==L'W')action=BrowserTabClose;
            else if(key==VK_TAB)action=(GetKeyState(VK_SHIFT)&0x8000)?BrowserTabPrevious:BrowserTabNext;
            else if(key==VK_ADD||key==VK_OEM_PLUS)delta=10;
            else if(key==VK_SUBTRACT||key==VK_OEM_MINUS)delta=-10;
            else if(key!=L'0'&&key!=VK_NUMPAD0)return S_OK;
            // Windowed accelerator callbacks block browser input. Mark handled
            // immediately and let the host apply zoom after this stack unwinds.
            if(FAILED(args->put_Handled(TRUE)))return S_OK;
            if(kind==COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN||kind==COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN)
                s->Queue(action?action:7,delta);
            return S_OK;
        },[&](auto h,auto t){return controller->add_AcceleratorKeyPressed(h,t);})&&ok;
        ok=Event<ICoreWebView2ZoomFactorChangedEventHandler,ICoreWebView2Controller*,IUnknown*>([valid](auto sender,auto){
            auto s=valid();if(!s||!s->ready||!s->verified||s->locked||s->probing)return S_OK;
            double factor=1.0;if(FAILED(sender->get_ZoomFactor(&factor))||!std::isfinite(factor))return S_OK;
            const int percent=static_cast<int>(std::lround(std::clamp(factor,0.25,4.0)*100.0));
            // Promote native Ctrl+wheel to the same session zoom as our buttons.
            // Host-applied factors update immediately without another event.
            if(FAILED(sender->put_ZoomFactor(percent/100.0)))return S_OK;
            if(s->zoomPercent!=percent){s->zoomPercent=percent;s->NotifyZoom();}return S_OK;
        },[&](auto h,auto t){return controller->add_ZoomFactorChanged(h,t);})&&ok;
        ok=Event<ICoreWebView2NavigationStartingEventHandler,ICoreWebView2*,ICoreWebView2NavigationStartingEventArgs*>([valid](auto,auto args){
            auto s=valid();if(!s||s->locked){args->put_Cancel(TRUE);return S_OK;}
            // During initialization only the host's static NavigateToString
            // fixture exists; public Navigate() is disabled until proof passes.
            if(s->probing){args->get_NavigationId(&s->navigationId);return S_OK;}
            LPWSTR raw=nullptr;args->get_Uri(&raw);auto uri=TakeString(raw);std::wstring normalized;
            // NavigateToString emits about:blank on some runtimes, and the
            // exact UTF-8 data URI on others. This single pending host HTML
            // navigation is the only data document ever admitted.
            if(s->verified&&s->localPage&&s->localNavigation&&(uri==L"about:blank"||uri==s->localDocumentUri)){
                s->localNavigation=false;args->get_NavigationId(&s->navigationId);return S_OK;
            }
            // A history step to our former blank entry is rebuilt from native
            // saved data. Never trust HTML restored by the browser's cache.
            if(s->verified&&s->hasNewTab&&(uri==L"about:blank"||uri==s->localDocumentUri)){args->put_Cancel(TRUE);if(s->visible)s->Queue(10);return S_OK;}
            const auto commandPrefix=std::wstring(kShortcutCommand)+s->localNonce+L"/";
            if(uri.rfind(kShortcutCommand,0)==0){
                args->put_Cancel(TRUE);
                if(s->verified&&s->visible&&s->localPage&&!s->localNavigation&&!s->localNonce.empty()&&uri.rfind(commandPrefix,0)==0)s->Queue(8,0,std::move(uri));
                return S_OK;
            }
            if(!s->verified||!NormalizeWebAddress(uri,normalized))args->put_Cancel(TRUE);
            else {s->localPage=s->localNavigation=false;s->localNonce.clear();args->get_NavigationId(&s->navigationId);s->navigating=true;s->loaded=false;s->Notify();}
            return S_OK;
        },[&](auto h,auto t){return web->add_NavigationStarting(h,t);})&&ok;
        ok=Event<ICoreWebView2NavigationCompletedEventHandler,ICoreWebView2*,ICoreWebView2NavigationCompletedEventArgs*>([valid](auto,auto args){
            auto s=valid();if(!s)return S_OK;UINT64 id=0;args->get_NavigationId(&id);if(id!=s->navigationId)return S_OK;
            BOOL success=FALSE;args->get_IsSuccess(&success);
            if(s->probing){if(!success){COREWEBVIEW2_WEB_ERROR_STATUS status{};args->get_WebErrorStatus(&status);s->Fail(100+int(status));}
                else {
                    auto policy=Callback<ICoreWebView2ExecuteScriptCompletedHandler,HRESULT,LPCWSTR>([valid](HRESULT hr,LPCWSTR value){
                        if(auto current=valid()){if(FAILED(hr)||!value||wcscmp(value,L"true")!=0)current->Fail(310);
                            else if(auto manager=current->owner.lock())PostMessageW(current->parent,BrowserProbeMessage,manager->generation,current->id);}return S_OK;
                    });
                    auto hr=s->web->ExecuteScript(L"window.__quietShareNativeUiPolicy===true",policy);policy->Release();if(FAILED(hr))s->Fail(311);
                }
            }
            else{s->navigating=false;s->loaded=success;s->notice=success?L"":L"navigation";s->Notify();if(!s->visible)s->Suspend();}return S_OK;
        },[&](auto h,auto t){return web->add_NavigationCompleted(h,t);})&&ok;
        ok=Event<ICoreWebView2SourceChangedEventHandler,ICoreWebView2*,ICoreWebView2SourceChangedEventArgs*>([valid](auto,auto){auto s=valid();if(s&&!s->probing){LPWSTR p=nullptr;s->web->get_Source(&p);auto source=TakeString(p);s->address=s->localPage&&(source==L"about:blank"||source==s->localDocumentUri)?L"":source;s->Notify();}return S_OK;},[&](auto h,auto t){return web->add_SourceChanged(h,t);})&&ok;
        ok=Event<ICoreWebView2HistoryChangedEventHandler,ICoreWebView2*,IUnknown*>([valid](auto,auto){if(auto s=valid())s->Notify();return S_OK;},[&](auto h,auto t){return web->add_HistoryChanged(h,t);})&&ok;
        ok=Event<ICoreWebView2DocumentTitleChangedEventHandler,ICoreWebView2*,IUnknown*>([valid](auto,auto){
            if(auto s=valid();s&&!s->probing){LPWSTR raw=nullptr;s->web->get_DocumentTitle(&raw);s->title=TakeString(raw);if(s->title.size()>160)s->title.resize(160);
                for(auto& c:s->title)if(c<32||c==127)c=L' ';
                s->Notify();}return S_OK;
        },[&](auto h,auto t){return web->add_DocumentTitleChanged(h,t);})&&ok;
        ok=Event<ICoreWebView2NewWindowRequestedEventHandler,ICoreWebView2*,ICoreWebView2NewWindowRequestedEventArgs*>([valid](auto,auto args){
            args->put_Handled(TRUE);auto s=valid();BOOL initiated=FALSE;args->get_IsUserInitiated(&initiated);
            if(s&&s->verified&&!s->locked&&s->visible&&initiated){LPWSTR p=nullptr;args->get_Uri(&p);auto uri=TakeString(p);std::wstring normalized;if(NormalizeWebAddress(uri,normalized))s->Queue(6,0,std::move(normalized));}return S_OK;
        },[&](auto h,auto t){return web->add_NewWindowRequested(h,t);})&&ok;
        ok=Event<ICoreWebView2PermissionRequestedEventHandler,ICoreWebView2*,ICoreWebView2PermissionRequestedEventArgs*>([valid](auto sender,auto args){
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
            Com<ICoreWebView2PermissionRequestedEventArgs3> policy;
            if(FAILED(args->QueryInterface(WebGuid<ICoreWebView2PermissionRequestedEventArgs3>(),reinterpret_cast<void**>(policy.put())))||
               FAILED(policy->put_SavesInProfile(FALSE)))return S_OK;
            // Handle all permissions here; no native permission prompts may
            // escape the app's protected surface. Unsupported kinds stay denied.
            if(FAILED(policy->put_Handled(TRUE)))return S_OK;
            auto s=valid();auto manager=s?s->owner.lock():nullptr;
            if(!s||!manager||!s->mediaReady||!s->ready||!s->verified||s->probing||s->locked||!s->visible||s->localPage)return S_OK;
            COREWEBVIEW2_PERMISSION_KIND kind{};if(FAILED(args->get_PermissionKind(&kind)))return S_OK;
            if(!((kind==COREWEBVIEW2_PERMISSION_KIND_CAMERA&&manager->allowCamera)||
                 (kind==COREWEBVIEW2_PERMISSION_KIND_MICROPHONE&&manager->allowMicrophone)))return S_OK;
            LPWSTR raw=nullptr;if(FAILED(args->get_Uri(&raw)))return S_OK;const auto origin=SecureMediaOrigin(TakeString(raw));
            raw=nullptr;if(FAILED(sender->get_Source(&raw)))return S_OK;const auto top=SecureMediaOrigin(TakeString(raw));
            if(origin.empty()||origin!=top)return S_OK;
            if(SUCCEEDED(args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW))){
                if(kind==COREWEBVIEW2_PERMISSION_KIND_CAMERA)s->cameraOrigin=origin;else s->microphoneOrigin=origin;
            }return S_OK;
        },[&](auto h,auto t){return web->add_PermissionRequested(h,t);})&&ok;
        ok=Event<ICoreWebView2ScriptDialogOpeningEventHandler,ICoreWebView2*,ICoreWebView2ScriptDialogOpeningEventArgs*>([](auto,auto args){
            COREWEBVIEW2_SCRIPT_DIALOG_KIND kind{};args->get_Kind(&kind);if(kind==COREWEBVIEW2_SCRIPT_DIALOG_KIND_ALERT)args->Accept();return S_OK;
        },[&](auto h,auto t){return web->add_ScriptDialogOpening(h,t);})&&ok;
        ok=Event<ICoreWebView2DownloadStartingEventHandler,ICoreWebView2*,ICoreWebView2DownloadStartingEventArgs*>([](auto,auto args){args->put_Cancel(TRUE);args->put_Handled(TRUE);return S_OK;},[&](auto h,auto t){return download->add_DownloadStarting(h,t);})&&ok;
        ok=Event<ICoreWebView2ClientCertificateRequestedEventHandler,ICoreWebView2*,ICoreWebView2ClientCertificateRequestedEventArgs*>([](auto,auto args){args->put_Cancel(TRUE);args->put_Handled(TRUE);return S_OK;},[&](auto h,auto t){return certificates->add_ClientCertificateRequested(h,t);})&&ok;
        ok=Event<ICoreWebView2BasicAuthenticationRequestedEventHandler,ICoreWebView2*,ICoreWebView2BasicAuthenticationRequestedEventArgs*>([](auto,auto args){args->put_Cancel(TRUE);return S_OK;},[&](auto h,auto t){return authentication->add_BasicAuthenticationRequested(h,t);})&&ok;
        ok=Event<ICoreWebView2LaunchingExternalUriSchemeEventHandler,ICoreWebView2*,ICoreWebView2LaunchingExternalUriSchemeEventArgs*>([](auto,auto args){args->put_Cancel(TRUE);return S_OK;},[&](auto h,auto t){return external->add_LaunchingExternalUriScheme(h,t);})&&ok;
        ok=Event<ICoreWebView2ScreenCaptureStartingEventHandler,ICoreWebView2*,ICoreWebView2ScreenCaptureStartingEventArgs*>([](auto,auto args){args->put_Cancel(TRUE);args->put_Handled(TRUE);return S_OK;},[&](auto h,auto t){return capture->add_ScreenCaptureStarting(h,t);})&&ok;
        ok=Event<ICoreWebView2ProcessFailedEventHandler,ICoreWebView2*,ICoreWebView2ProcessFailedEventArgs*>([valid](auto,auto args){if(auto s=valid()){COREWEBVIEW2_PROCESS_FAILED_KIND kind{};args->get_ProcessFailedKind(&kind);s->Fail(200+int(kind));}return S_OK;},[&](auto h,auto t){return web->add_ProcessFailed(h,t);})&&ok;
        auto installed=Callback<ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler,HRESULT,LPCWSTR>([valid](HRESULT result,LPCWSTR){
            if(auto s=valid()){if(FAILED(result))s->Fail(300);else s->ResetMediaPermissions([valid](bool cleared){
                if(auto current=valid()){if(!cleared)current->Fail(345);else{current->mediaReady=true;
                    if(FAILED(current->web->NavigateToString(CanaryHtml().c_str())))current->Fail(301);}}
            });}return S_OK;});
        auto hr=web->AddScriptToExecuteOnDocumentCreated(kDomPolicy,installed);installed->Release();if(ok&&SUCCEEDED(hr)){error=0;return true;}return false;
    }
};
Browser::Browser():impl(std::make_shared<Impl>()){}Browser::~Browser(){Disable();}
void Browser::SetEnglish(bool english){auto s=impl;if(s->english==english)return;s->english=english;auto tab=s->Active();if(tab&&tab->localPage)tab->LoadNewTab();}
void Browser::SetCommandShortcuts(const std::vector<BrowserCommandShortcut>& shortcuts){impl->commandShortcuts=shortcuts;}
void Browser::SetMediaPreferences(bool muted,bool allowCamera,bool allowMicrophone){
    if(impl->muted==muted&&impl->allowCamera==allowCamera&&impl->allowMicrophone==allowMicrophone)return;
    auto s=impl;const bool revokeCamera=s->allowCamera&&!allowCamera,revokeMicrophone=s->allowMicrophone&&!allowMicrophone;
    s->muted=muted;s->allowCamera=allowCamera;s->allowMicrophone=allowMicrophone;
    const auto snapshot=s->tabs;const unsigned epoch=s->generation;
    for(const auto& tab:snapshot){if(!s->enabled||s->generation!=epoch)return;if(!s->Current(tab,epoch)||!tab->web)continue;
        if(!tab->ApplyMute()){tab->Fail(340);continue;}
        if(s->Current(tab,epoch))tab->RevokeMedia(revokeCamera,revokeMicrophone);
    }
}
bool Browser::Enable(HWND parent,const std::wstring& profile,const std::wstring& initial){
    auto s=impl;if(s->enabled)return true;
    std::wstring address;if(!initial.empty())ResolveWebInput(initial,address);
    s->Close();s->parent=parent;s->enabled=true;s->error=0;s->locked=false;
    s->profile=profile;ReadShortcuts(profile+L"\\quietshare-shortcuts.txt",s->shortcuts);
    auto first=s->AddTab(address);
    // Load only our pinned loader from beside the EXE, never a working-directory DLL.
    wchar_t executable[32768]{};GetModuleFileNameW(nullptr,executable,32768);std::wstring path=executable;path=path.substr(0,path.find_last_of(L"\\"))+L"\\WebView2Loader.dll";
    static HMODULE loader=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    using Create=HRESULT(STDAPICALLTYPE*)(PCWSTR,PCWSTR,ICoreWebView2EnvironmentOptions*,ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
    auto create=loader?reinterpret_cast<Create>(GetProcAddress(loader,"CreateCoreWebView2EnvironmentWithOptions")):nullptr;
    if(!create){first->Fail();return false;}
    SetEnvironmentVariableW(L"COREWEBVIEW2_FORCED_HOSTING_MODE",L"COREWEBVIEW2_HOSTING_MODE_WINDOW_TO_VISUAL");
    for(const auto* key:{L"WEBVIEW2_USER_DATA_FOLDER",L"WEBVIEW2_BROWSER_EXECUTABLE_FOLDER",L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS",L"WEBVIEW2_PIPE_FOR_SCRIPT_DEBUGGER",L"WEBVIEW2_WAIT_FOR_SCRIPT_DEBUGGER"})SetEnvironmentVariableW(key,nullptr);
    const unsigned epoch=s->generation;std::weak_ptr<Impl> weak=s;
    auto handler=Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,HRESULT,ICoreWebView2Environment*>([weak,epoch](HRESULT result,auto environment){
        auto s=weak.lock();if(!s||!s->enabled||s->generation!=epoch)return S_OK;
        auto first=s->Find(s->pendingId);if(!first)return S_OK;
        if(FAILED(result)||!environment){first->Fail();return S_OK;}environment->AddRef();s->environment.reset(environment);
        s->CreateController(first);return S_OK;
    });
    auto hr=create(nullptr,profile.c_str(),nullptr,handler);handler->Release();if(FAILED(hr)){first->Fail();return false;}s->Notify();return true;
}
void Browser::Disable(){auto current=impl->Active();if(current)impl->zoomPercent=current->zoomPercent;impl->Close();}
void Browser::SetBounds(RECT r){auto s=impl;if(EqualRect(&s->bounds,&r))return;s->bounds=r;const auto snapshot=s->tabs;const unsigned epoch=s->generation;
    for(const auto& tab:snapshot){if(!s->enabled||s->generation!=epoch||!EqualRect(&s->bounds,&r))return;if(!s->Current(tab,epoch))continue;
        tab->bounds=r;if(tab->controller&&!tab->probing){Com<ICoreWebView2Controller> current;current.hold(tab->controller.p);current->put_Bounds(r);
            if(s->Current(tab,epoch)&&!tab->probing&&tab->controller.p==current.p&&EqualRect(&s->bounds,&r))current->NotifyParentWindowPositionChanged();}}}
void Browser::ParentMoved(){auto s=impl;const auto snapshot=s->tabs;const unsigned epoch=s->generation;for(const auto& tab:snapshot){if(!s->enabled||s->generation!=epoch)return;
    if(s->Current(tab,epoch)&&tab->controller&&!tab->probing){Com<ICoreWebView2Controller> current;current.hold(tab->controller.p);current->NotifyParentWindowPositionChanged();}}}
void Browser::Show(bool show){auto s=impl;if(show){auto tab=s->Active();if(tab&&tab->verified)s->locked=false;}else s->actions.clear();s->visible=show;s->UpdateVisible();}
void Browser::Lock(){auto s=impl;s->locked=true;s->visible=false;s->actions.clear();s->pendingId=0;
    const auto snapshot=s->tabs;const unsigned epoch=s->generation;for(const auto& tab:snapshot){tab->locked=true;tab->visible=false;tab->navigating=false;}
    for(const auto& tab:snapshot){if(!s->enabled||s->generation!=epoch||!s->locked)return;if(!s->Current(tab,epoch))continue;
        if(!tab->verified){tab->Close();tab->locked=true;tab->error=323;continue;}
        Com<ICoreWebView2Controller> current;current.hold(tab->controller.p);Com<ICoreWebView2> web;web.hold(tab->web.p);
        if(current)current->put_IsVisible(FALSE);if(!s->Current(tab,epoch)||!s->locked)return;
        if(web)web->Stop();if(s->Current(tab,epoch)&&s->locked)tab->Suspend();}s->Notify();}
unsigned Browser::ProbeToken()const{return impl->pendingId;}
bool Browser::MountProbe(HWND probe,unsigned token){auto s=impl;auto tab=s->Find(token?token:s->pendingId);
    if(!tab||!tab->controller||!tab->enabled||!s->enabled||s->locked||!tab->probing||s->pendingId!=tab->id)return false;
    const unsigned epoch=s->generation;Com<ICoreWebView2Controller> current;current.hold(tab->controller.p);
    auto valid=[&](){return s->Current(tab,epoch)&&!s->locked&&tab->probing&&s->pendingId==tab->id&&tab->controller.p==current.p;};
    if(!probe){if(FAILED(current->put_IsVisible(FALSE))||!valid())return false;
        return SUCCEEDED(current->put_ParentWindow(s->parent))&&valid()&&SUCCEEDED(current->put_Bounds(s->bounds))&&valid();}
    RECT r{0,0,128,96};return SUCCEEDED(current->put_ParentWindow(probe))&&valid()&&SUCCEEDED(current->put_Bounds(r))&&valid()&&SUCCEEDED(current->put_IsVisible(TRUE))&&valid();
}
void Browser::FinishProbe(bool passed,unsigned token){auto s=impl;auto tab=s->Find(token?token:s->pendingId);
    if(!s->enabled||s->locked||!tab||!tab->enabled||!tab->probing||s->pendingId!=tab->id)return;
    const unsigned epoch=s->generation,tabEpoch=tab->generation;Com<ICoreWebView2> web;web.hold(tab->web.p);
    auto valid=[&](){return s->Current(tab,epoch)&&!s->locked&&!tab->locked&&tab->generation==tabEpoch&&tab->web.p==web.p;};
    tab->probing=false;tab->verified=passed&&!tab->locked;tab->ready=tab->verified;
    if(!passed){tab->Fail();return;}
    Com<ICoreWebView2Settings> settings;
    bool configured=web&&SUCCEEDED(web->get_Settings(settings.put()));if(!valid())return;
    if(configured)configured=SUCCEEDED(settings->put_IsZoomControlEnabled(TRUE));if(!valid())return;
    if(configured)configured=tab->SetZoom(tab->zoomPercent);if(!valid())return;
    if(!configured){tab->Fail(320);return;}
    // The canary has passed before any untrusted address reaches this controller.
    s->pendingId=0;
    auto initial=std::move(tab->pending);tab->pending.clear();tab->loaded=false;tab->navigating=true;
    if(initial.empty())tab->LoadNewTab();
    else{const auto hr=web->Navigate(initial.c_str());if(!valid())return;if(FAILED(hr)){tab->navigating=false;tab->notice=L"navigation";}}
    s->UpdateVisible();tab->Notify();
}
std::vector<BrowserTab> Browser::Tabs()const{std::vector<BrowserTab> result;
    for(const auto& tab:impl->tabs)result.push_back({tab->id,tab->title,tab->address,(!tab->error&&!tab->ready)||tab->navigating,tab->ready,impl->activeId==tab->id,tab->zoomPercent});
    return result;}
int Browser::ActiveTab()const{for(size_t i=0;i<impl->tabs.size();++i)if(impl->tabs[i]->id==impl->activeId)return static_cast<int>(i);return -1;}
unsigned Browser::ActiveTabId()const{return impl->activeId;}
bool Browser::CanNewTab()const{return impl->enabled&&!impl->locked&&impl->environment&&!impl->pendingId&&impl->tabs.size()<8;}
bool Browser::NewTab(const std::wstring& value){auto s=impl;std::wstring address;if(!CanNewTab()||(!value.empty()&&!ResolveWebInput(value,address)))return false;auto tab=s->AddTab(address);s->CreateController(tab);return true;}
bool Browser::CloseTab(unsigned id){auto s=impl;auto found=std::find_if(s->tabs.begin(),s->tabs.end(),[&](const auto& tab){return tab->id==id;});if(found==s->tabs.end())return false;
    auto closing=*found;const size_t index=static_cast<size_t>(found-s->tabs.begin());const bool active=s->activeId==id;const unsigned epoch=s->generation;
    if(s->tabs.size()==1)s->zoomPercent=closing->zoomPercent;if(s->pendingId==id)s->pendingId=0;
    s->tabs.erase(found);
    s->actions.erase(std::remove_if(s->actions.begin(),s->actions.end(),[&](const auto& item){return item.id==id;}),s->actions.end());
    if(s->tabs.empty()){s->Close();closing->Close();return true;}
    if(active)s->activeId=s->tabs[std::min(index,s->tabs.size()-1)]->id;
    closing->Close();if(!s->enabled||s->generation!=epoch)return true;s->UpdateVisible();s->Notify();return true;}
bool Browser::ActivateTab(unsigned id){auto s=impl;auto tab=s->Find(id);if(!s->enabled||s->locked||!tab)return false;if(s->activeId==id)return true;
    s->actions.clear();s->activeId=id;s->UpdateVisible();if(tab->localPage)tab->LoadNewTab();s->Notify();s->NotifyZoom();return true;}
bool Browser::CycleTab(bool previous){const int active=ActiveTab(),count=static_cast<int>(impl->tabs.size());if(active<0||!count)return false;return ActivateTab(impl->tabs[(active+count+(previous?-1:1))%count]->id);}
bool Browser::ProcessTabAction(int action){auto s=impl;int value=0;std::wstring address;
    if(action==BrowserTabQueued){if(s->actions.empty())return false;auto queued=std::move(s->actions.front());s->actions.pop_front();auto source=s->Find(queued.id);
        if(queued.generation!=s->generation||!source||source->id!=s->activeId||!source->visible||!source->ready||source->locked||s->locked)return false;
        action=queued.kind;value=queued.value;address=std::move(queued.address);}
    if(!s->enabled||s->locked)return false;
    if(action==BrowserTabNew)return NewTab();
    if(action==BrowserTabClose)return CloseTab(s->activeId);
    if(action==BrowserTabNext)return CycleTab();
    if(action==BrowserTabPrevious)return CycleTab(true);
    if(action==6)return NewTab(address);
    if(action==7)return value?AdjustZoomPercent(value):ResetZoom();
    if(action==9){PostMessageW(s->parent,BrowserCommandMessage,s->generation,value);return true;}
    if(action==10){auto tab=s->Active();if(!tab||!tab->hasNewTab)return false;tab->LoadNewTab();return true;}
    if(action==8){
        auto tab=s->Active();if(!tab||!tab->localPage||tab->localNonce.empty())return false;
        auto prefix=std::wstring(kShortcutCommand)+tab->localNonce+L"/";if(address.rfind(prefix,0)!=0)return false;
        auto command=address.substr(prefix.size());auto slash=command.find(L'/'),query=command.find(L'?');
        if(slash==std::wstring::npos||query==std::wstring::npos||query<=slash)return false;
        auto kind=command.substr(0,slash),indexText=command.substr(slash+1,query-slash-1),fields=command.substr(query+1);
        int index=-1;if(indexText!=L"-1"){
            if(indexText.empty()||indexText.size()>2||!std::all_of(indexText.begin(),indexText.end(),[](wchar_t c){return c>=L'0'&&c<=L'9';}))return false;
            index=std::stoi(indexText);if(index>=static_cast<int>(s->shortcuts.size()))return false;
        }
        if(kind==L"search"){
            std::wstring input;if(fields.rfind(L"value=",0)!=0||!DecodeComponent(fields.substr(6),input))return false;
            Navigate(input);return true;
        }
        auto updated=s->shortcuts;bool valid=false;
        if(kind==L"remove"&&index>=0&&fields.empty()){updated.erase(updated.begin()+index);valid=true;}
        if(kind==L"save"&&(index>=0||updated.size()<kShortcutLimit)&&fields.rfind(L"name=",0)==0){
            auto separator=fields.find(L"&url=",5);std::wstring name,url;BrowserShortcut item;
            if(separator!=std::wstring::npos&&DecodeComponent(fields.substr(5,separator-5),name)&&DecodeComponent(fields.substr(separator+5),url)&&NormalizeBrowserShortcut(name,url,item)){
                if(index<0)updated.push_back(std::move(item));else updated[index]=std::move(item);valid=true;
            }
        }
        if(!valid)tab->newTabError=s->english?L"Enter a name and an HTTP / HTTPS web address.":L"Nhập tên và địa chỉ web HTTP / HTTPS.";
        else if(!WriteShortcuts(s->profile,updated))tab->newTabError=s->english?L"Could not save the shortcut. Check access to the browser profile.":L"Không lưu được lối tắt. Kiểm tra quyền ghi hồ sơ trình duyệt.";
        else{s->shortcuts=std::move(updated);tab->newTabError.clear();}
        tab->LoadNewTab();return true;
    }
    return false;
}
void Browser::ClearQueuedActions(){impl->actions.clear();}
unsigned Browser::BackgroundSuspendedCount()const{auto s=impl;unsigned count=0;const auto snapshot=s->tabs;const unsigned epoch=s->generation;
    for(const auto& tab:snapshot){if(!s->enabled||s->generation!=epoch)return 0;if(s->Current(tab,epoch)&&tab->id!=s->activeId&&tab->web){Com<ICoreWebView2_3> control;BOOL suspended=FALSE;
        if(tab->Query(control)&&SUCCEEDED(control->get_IsSuspended(&suspended))&&suspended&&s->Current(tab,epoch)&&tab->id!=s->activeId)++count;}}return count;}
unsigned Browser::PendingTabCount()const{return impl->pendingId?1:0;}
unsigned Browser::ControllerCount()const{unsigned count=0;for(const auto& tab:impl->tabs)if(tab->controller)++count;return count;}
void Browser::Navigate(const std::wstring& value){auto s=impl;auto tab=s->Active();std::wstring address;if(!tab||!tab->ready||tab->locked||s->locked||!tab->web)return;
    if(value.empty()){tab->newTabError.clear();tab->LoadNewTab();return;}
    if(!ResolveWebInput(value,address)){tab->notice=L"address";tab->Notify();return;}tab->loaded=false;tab->navigating=true;tab->notice.clear();if(FAILED(tab->web->Navigate(address.c_str()))){tab->navigating=false;tab->notice=L"navigation";tab->Notify();}}
void Browser::Back(){auto tab=impl->Active();if(Ready()&&!impl->locked&&!tab->locked)tab->web->GoBack();}
void Browser::Forward(){auto tab=impl->Active();if(Ready()&&!impl->locked&&!tab->locked)tab->web->GoForward();}
void Browser::Reload(){auto tab=impl->Active();if(Ready()&&!impl->locked&&!tab->locked){if(tab->localPage)tab->LoadNewTab();else tab->web->Reload();}}
void Browser::Focus(){auto tab=impl->Active();if(Ready()&&tab->visible&&!impl->locked)tab->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);}
int Browser::ZoomPercent()const{auto tab=impl->Active();return tab?tab->zoomPercent:impl->zoomPercent;}
bool Browser::SetZoomPercent(int percent){auto tab=impl->Active();if(tab)return !impl->locked&&tab->SetZoom(percent);return impl->SetZoom(percent);}
bool Browser::AdjustZoomPercent(int delta){return SetZoomPercent(static_cast<int>(std::clamp(static_cast<long long>(ZoomPercent())+delta,25LL,400LL)));}
bool Browser::ResetZoom(){return SetZoomPercent(DefaultZoomPercent);}
bool Browser::Enabled()const{return impl->enabled;}
bool Browser::Ready()const{auto tab=impl->Active();return tab&&tab->ready;}
bool Browser::Verified()const{auto tab=impl->Active();return tab&&tab->verified;}
bool Browser::CanBack()const{auto tab=impl->Active();BOOL value=FALSE;if(tab&&tab->web)tab->web->get_CanGoBack(&value);return value;}
bool Browser::CanForward()const{auto tab=impl->Active();BOOL value=FALSE;if(tab&&tab->web)tab->web->get_CanGoForward(&value);return value;}
bool Browser::Loaded()const{auto tab=impl->Active();return tab&&tab->loaded&&tab->ready;}
bool Browser::Suspended()const{auto tab=impl->Active();Com<ICoreWebView2_3> control;BOOL value=FALSE;if(tab&&tab->Query(control))control->get_IsSuspended(&value);return value;}
int Browser::ErrorCode()const{auto tab=impl->Active();return tab?tab->error:impl->error;}
int Browser::State()const{return !impl->enabled?0:impl->locked?4:ErrorCode()?3:Ready()?2:1;}
unsigned Browser::BrowserProcessId()const{auto tab=impl->Active();return tab?tab->pid:0;}
unsigned Browser::Generation()const{return impl->generation;}
std::wstring Browser::Address()const{auto tab=impl->Active();return tab?tab->address:L"";}
std::wstring Browser::Status(bool en)const{
    if(State()==0)return en?L"Browser is off. Enable only when needed.":L"Trình duyệt đang tắt. Bật khi cần dùng web.";
    if(State()==3)return en?L"Browser unavailable. Check the WebView2 Runtime or retry enabling.":L"Không mở được trình duyệt. Kiểm tra WebView2 Runtime hoặc bật lại.";
    if(State()==1)return en?L"Starting browser and verifying its rendered surface...":L"Đang mở trình duyệt và kiểm tra vùng render...";
    if(State()==4)return en?L"Browser hidden. Disable and enable to verify again.":L"Trình duyệt đã che. Tắt và bật lại để kiểm tra lại.";
    auto tab=impl->Active();
    if(tab&&tab->notice==L"address")return en?L"Enter search terms or an HTTP / HTTPS web address.":L"Nhập từ khóa tìm kiếm hoặc địa chỉ web HTTP / HTTPS.";
    if(tab&&tab->notice==L"navigation")return en?L"Page could not load. Check the connection or reload.":L"Không tải được trang. Kiểm tra kết nối hoặc tải lại.";
    return en?L"Web · Separate profile":L"Web · Hồ sơ riêng";
}
}
