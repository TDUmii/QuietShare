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
std::wstring TakeString(LPWSTR p){std::wstring result=p?p:L"";CoTaskMemFree(p);return result;}
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
struct Browser::Impl:std::enable_shared_from_this<Browser::Impl> {
    HWND parent=nullptr;RECT bounds{};bool enabled=false,ready=false,verified=false,visible=false,probing=false,locked=false,suspendPending=false;
    unsigned generation=0,pid=0;UINT64 navigationId=0;int error=0;bool loaded=false,navigating=false;std::wstring address,pending,notice,title;
    int zoomPercent=100;
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
    void Close(){++generation;enabled=ready=verified=visible=probing=locked=suspendPending=false;pid=0;error=0;loaded=navigating=false;address.clear();pending.clear();notice.clear();title.clear();
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
            if(!(GetKeyState(VK_CONTROL)&0x8000)||(GetKeyState(VK_MENU)&0x8000)||
               (GetKeyState(VK_LWIN)&0x8000)||(GetKeyState(VK_RWIN)&0x8000))return S_OK;
            COREWEBVIEW2_KEY_EVENT_KIND kind{};UINT key=0;
            if(FAILED(args->get_KeyEventKind(&kind))||FAILED(args->get_VirtualKey(&key)))return S_OK;
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
            if(!s->verified||!NormalizeWebAddress(uri,normalized))args->put_Cancel(TRUE);
            else {args->get_NavigationId(&s->navigationId);s->navigating=true;s->loaded=false;s->Notify();}
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
        ok=Event<ICoreWebView2SourceChangedEventHandler,ICoreWebView2*,ICoreWebView2SourceChangedEventArgs*>([valid](auto,auto){auto s=valid();if(s&&!s->probing){LPWSTR p=nullptr;s->web->get_Source(&p);s->address=TakeString(p);s->Notify();}return S_OK;},[&](auto h,auto t){return web->add_SourceChanged(h,t);})&&ok;
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
        ok=Event<ICoreWebView2PermissionRequestedEventHandler,ICoreWebView2*,ICoreWebView2PermissionRequestedEventArgs*>([](auto,auto args){args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);return S_OK;},[&](auto h,auto t){return web->add_PermissionRequested(h,t);})&&ok;
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
            if(auto s=valid()){if(FAILED(result))s->Fail(300);else if(FAILED(s->web->NavigateToString(CanaryHtml().c_str())))s->Fail(301);}return S_OK;});
        auto hr=web->AddScriptToExecuteOnDocumentCreated(kDomPolicy,installed);installed->Release();if(ok&&SUCCEEDED(hr)){error=0;return true;}return false;
    }
};
Browser::Browser():impl(std::make_shared<Impl>()){}Browser::~Browser(){Disable();}
bool Browser::Enable(HWND parent,const std::wstring& profile,const std::wstring& initial){
    auto s=impl;if(s->enabled)return true;
    std::wstring address;if(!ResolveWebInput(initial,address))address=L"https://www.google.com";
    s->Close();s->parent=parent;s->enabled=true;s->error=0;s->locked=false;
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
    const auto hr=web->Navigate(initial.c_str());if(!valid())return;if(FAILED(hr)){tab->navigating=false;tab->notice=L"navigation";}
    s->UpdateVisible();tab->Notify();
}
std::vector<BrowserTab> Browser::Tabs()const{std::vector<BrowserTab> result;
    for(const auto& tab:impl->tabs)result.push_back({tab->id,tab->title,tab->address,(!tab->error&&!tab->ready)||tab->navigating,tab->ready,impl->activeId==tab->id,tab->zoomPercent});
    return result;}
int Browser::ActiveTab()const{for(size_t i=0;i<impl->tabs.size();++i)if(impl->tabs[i]->id==impl->activeId)return static_cast<int>(i);return -1;}
unsigned Browser::ActiveTabId()const{return impl->activeId;}
bool Browser::CanNewTab()const{return impl->enabled&&!impl->locked&&impl->environment&&!impl->pendingId&&impl->tabs.size()<8;}
bool Browser::NewTab(const std::wstring& value){auto s=impl;std::wstring address;if(!CanNewTab()||!ResolveWebInput(value,address))return false;auto tab=s->AddTab(address);s->CreateController(tab);return true;}
bool Browser::CloseTab(unsigned id){auto s=impl;auto found=std::find_if(s->tabs.begin(),s->tabs.end(),[&](const auto& tab){return tab->id==id;});if(found==s->tabs.end())return false;
    auto closing=*found;const size_t index=static_cast<size_t>(found-s->tabs.begin());const bool active=s->activeId==id;const unsigned epoch=s->generation;
    if(s->tabs.size()==1)s->zoomPercent=closing->zoomPercent;if(s->pendingId==id)s->pendingId=0;
    s->tabs.erase(found);
    s->actions.erase(std::remove_if(s->actions.begin(),s->actions.end(),[&](const auto& item){return item.id==id;}),s->actions.end());
    if(s->tabs.empty()){s->Close();closing->Close();return true;}
    if(active)s->activeId=s->tabs[std::min(index,s->tabs.size()-1)]->id;
    closing->Close();if(!s->enabled||s->generation!=epoch)return true;s->UpdateVisible();s->Notify();return true;}
bool Browser::ActivateTab(unsigned id){auto s=impl;auto tab=s->Find(id);if(!s->enabled||s->locked||!tab)return false;if(s->activeId==id)return true;
    s->actions.clear();s->activeId=id;s->UpdateVisible();s->Notify();s->NotifyZoom();return true;}
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
    return false;
}
void Browser::ClearQueuedActions(){impl->actions.clear();}
unsigned Browser::BackgroundSuspendedCount()const{auto s=impl;unsigned count=0;const auto snapshot=s->tabs;const unsigned epoch=s->generation;
    for(const auto& tab:snapshot){if(!s->enabled||s->generation!=epoch)return 0;if(s->Current(tab,epoch)&&tab->id!=s->activeId&&tab->web){Com<ICoreWebView2_3> control;BOOL suspended=FALSE;
        if(tab->Query(control)&&SUCCEEDED(control->get_IsSuspended(&suspended))&&suspended&&s->Current(tab,epoch)&&tab->id!=s->activeId)++count;}}return count;}
unsigned Browser::PendingTabCount()const{return impl->pendingId?1:0;}
unsigned Browser::ControllerCount()const{unsigned count=0;for(const auto& tab:impl->tabs)if(tab->controller)++count;return count;}
void Browser::Navigate(const std::wstring& value){auto s=impl;auto tab=s->Active();std::wstring address;if(!tab||!tab->ready||tab->locked||s->locked||!tab->web)return;
    if(!ResolveWebInput(value,address)){tab->notice=L"address";tab->Notify();return;}tab->loaded=false;tab->navigating=true;tab->notice.clear();if(FAILED(tab->web->Navigate(address.c_str()))){tab->navigating=false;tab->notice=L"navigation";tab->Notify();}}
void Browser::Back(){auto tab=impl->Active();if(Ready()&&!impl->locked&&!tab->locked)tab->web->GoBack();}
void Browser::Forward(){auto tab=impl->Active();if(Ready()&&!impl->locked&&!tab->locked)tab->web->GoForward();}
void Browser::Reload(){auto tab=impl->Active();if(Ready()&&!impl->locked&&!tab->locked)tab->web->Reload();}
void Browser::Focus(){auto tab=impl->Active();if(Ready()&&tab->visible&&!impl->locked)tab->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);}
int Browser::ZoomPercent()const{auto tab=impl->Active();return tab?tab->zoomPercent:impl->zoomPercent;}
bool Browser::SetZoomPercent(int percent){auto tab=impl->Active();if(tab)return !impl->locked&&tab->SetZoom(percent);return impl->SetZoom(percent);}
bool Browser::AdjustZoomPercent(int delta){return SetZoomPercent(static_cast<int>(std::clamp(static_cast<long long>(ZoomPercent())+delta,25LL,400LL)));}
bool Browser::ResetZoom(){return SetZoomPercent(100);}
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
