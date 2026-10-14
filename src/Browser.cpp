#include "Browser.h"
#include "Privacy.h"
#include <WebView2.h>
#include "../vendor/webview2/WebGuids.h"
#include <algorithm>
#include <functional>
#include <cwctype>
#include <sstream>
namespace quietshare {
namespace {
template<class T> struct Com {
    T* p=nullptr;~Com(){reset();}Com()=default;Com(const Com&)=delete;
    void reset(T* value=nullptr){if(p)p->Release();p=value;}
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
struct Browser::Impl:std::enable_shared_from_this<Browser::Impl> {
    HWND parent=nullptr;RECT bounds{};bool enabled=false,ready=false,verified=false,visible=false,probing=false,locked=false,suspendPending=false;
    unsigned generation=0,pid=0;UINT64 navigationId=0;int error=0;bool loaded=false;std::wstring address,pending,notice;
    Com<ICoreWebView2Environment> environment;Com<ICoreWebView2Controller> controller;Com<ICoreWebView2> web;
    void Notify(){if(IsWindow(parent))PostMessageW(parent,BrowserChangedMessage,generation,0);}
    void Close(){++generation;enabled=ready=verified=visible=probing=locked=suspendPending=false;pid=0;error=0;loaded=false;address.clear();pending.clear();notice.clear();
        if(controller){controller->put_IsVisible(FALSE);controller->Close();}web.reset();controller.reset();environment.reset();Notify();}
    void Fail(int code=1){++generation;ready=verified=visible=probing=false;if(!error)error=code;if(controller){controller->put_IsVisible(FALSE);controller->Close();}web.reset();controller.reset();environment.reset();Notify();}
    template<class I> bool Query(Com<I>& out){return web&&SUCCEEDED(web->QueryInterface(WebGuid<I>(),reinterpret_cast<void**>(out.put())));}
    void Suspend(){
        if(!ready||probing||visible||suspendPending)return;
        Com<ICoreWebView2_3> control;if(!Query(control))return;suspendPending=true;
        const unsigned epoch=generation;std::weak_ptr<Impl> weak=shared_from_this();
        auto done=Callback<ICoreWebView2TrySuspendCompletedHandler,HRESULT,BOOL>([weak,epoch](auto,auto){
            if(auto s=weak.lock();s&&s->generation==epoch){s->suspendPending=false;if(s->visible){Com<ICoreWebView2_3> c;if(s->Query(c))c->Resume();}}return S_OK;
        });
        if(FAILED(control->TrySuspend(done)))suspendPending=false;done->Release();
    }
    template<class I,class A,class B,class F,class Register> bool Event(F&& fn,Register reg){
        auto handler=Callback<I,A,B>(std::forward<F>(fn));EventRegistrationToken token{};auto hr=reg(handler,&token);handler->Release();return SUCCEEDED(hr);
    }
    bool Configure(){
        error=10;Com<ICoreWebView2Settings> settings;if(FAILED(web->get_Settings(settings.put())))return false;
        if(FAILED(settings->put_AreDefaultContextMenusEnabled(FALSE))||FAILED(settings->put_AreDefaultScriptDialogsEnabled(FALSE))||
           FAILED(settings->put_AreDevToolsEnabled(FALSE))||FAILED(settings->put_AreHostObjectsAllowed(FALSE))||
           FAILED(settings->put_IsWebMessageEnabled(FALSE))||FAILED(settings->put_IsStatusBarEnabled(FALSE)))return false;
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
        auto valid=[weak,epoch](){auto s=weak.lock();return s&&s->enabled&&s->generation==epoch?s:nullptr;};
        bool ok=true;
        ok=Event<ICoreWebView2NavigationStartingEventHandler,ICoreWebView2*,ICoreWebView2NavigationStartingEventArgs*>([valid](auto,auto args){
            auto s=valid();if(!s||s->locked){args->put_Cancel(TRUE);return S_OK;}
            // During initialization only the host's static NavigateToString
            // fixture exists; public Navigate() is disabled until proof passes.
            if(s->probing){args->get_NavigationId(&s->navigationId);return S_OK;}
            LPWSTR raw=nullptr;args->get_Uri(&raw);auto uri=TakeString(raw);std::wstring normalized;
            if(!s->verified||!NormalizeWebAddress(uri,normalized))args->put_Cancel(TRUE);
            else args->get_NavigationId(&s->navigationId);
            return S_OK;
        },[&](auto h,auto t){return web->add_NavigationStarting(h,t);})&&ok;
        ok=Event<ICoreWebView2NavigationCompletedEventHandler,ICoreWebView2*,ICoreWebView2NavigationCompletedEventArgs*>([valid](auto,auto args){
            auto s=valid();if(!s)return S_OK;UINT64 id=0;args->get_NavigationId(&id);if(id!=s->navigationId)return S_OK;
            BOOL success=FALSE;args->get_IsSuccess(&success);
            if(s->probing){if(!success){COREWEBVIEW2_WEB_ERROR_STATUS status{};args->get_WebErrorStatus(&status);s->Fail(100+int(status));}
                else {
                    auto policy=Callback<ICoreWebView2ExecuteScriptCompletedHandler,HRESULT,LPCWSTR>([valid](HRESULT hr,LPCWSTR value){
                        if(auto current=valid()){if(FAILED(hr)||!value||wcscmp(value,L"true")!=0)current->Fail(310);
                            else PostMessageW(current->parent,BrowserProbeMessage,current->generation,0);}return S_OK;
                    });
                    auto hr=s->web->ExecuteScript(L"window.__quietShareNativeUiPolicy===true",policy);policy->Release();if(FAILED(hr))s->Fail(311);
                }
            }
            else{s->loaded=success;s->notice=success?L"":L"navigation";s->Notify();}return S_OK;
        },[&](auto h,auto t){return web->add_NavigationCompleted(h,t);})&&ok;
        ok=Event<ICoreWebView2SourceChangedEventHandler,ICoreWebView2*,ICoreWebView2SourceChangedEventArgs*>([valid](auto,auto){auto s=valid();if(s&&!s->probing){LPWSTR p=nullptr;s->web->get_Source(&p);s->address=TakeString(p);s->Notify();}return S_OK;},[&](auto h,auto t){return web->add_SourceChanged(h,t);})&&ok;
        ok=Event<ICoreWebView2HistoryChangedEventHandler,ICoreWebView2*,IUnknown*>([valid](auto,auto){if(auto s=valid())s->Notify();return S_OK;},[&](auto h,auto t){return web->add_HistoryChanged(h,t);})&&ok;
        ok=Event<ICoreWebView2NewWindowRequestedEventHandler,ICoreWebView2*,ICoreWebView2NewWindowRequestedEventArgs*>([valid](auto,auto args){
            args->put_Handled(TRUE);auto s=valid();BOOL initiated=FALSE;args->get_IsUserInitiated(&initiated);
            if(s&&s->verified&&!s->locked&&initiated){LPWSTR p=nullptr;args->get_Uri(&p);auto uri=TakeString(p);std::wstring normalized;if(NormalizeWebAddress(uri,normalized))s->web->Navigate(normalized.c_str());}return S_OK;
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
bool Browser::Enable(HWND parent,const std::wstring& profile){
    auto s=impl;if(s->enabled)return true;s->Close();s->parent=parent;s->enabled=true;s->error=0;s->locked=false;
    // Load only our pinned loader from beside the EXE, never a working-directory DLL.
    wchar_t executable[32768]{};GetModuleFileNameW(nullptr,executable,32768);std::wstring path=executable;path=path.substr(0,path.find_last_of(L"\\"))+L"\\WebView2Loader.dll";
    static HMODULE loader=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    using Create=HRESULT(STDAPICALLTYPE*)(PCWSTR,PCWSTR,ICoreWebView2EnvironmentOptions*,ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
    auto create=loader?reinterpret_cast<Create>(GetProcAddress(loader,"CreateCoreWebView2EnvironmentWithOptions")):nullptr;
    if(!create){s->Fail();return false;}
    SetEnvironmentVariableW(L"COREWEBVIEW2_FORCED_HOSTING_MODE",L"COREWEBVIEW2_HOSTING_MODE_WINDOW_TO_VISUAL");
    for(const auto* key:{L"WEBVIEW2_USER_DATA_FOLDER",L"WEBVIEW2_BROWSER_EXECUTABLE_FOLDER",L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS",L"WEBVIEW2_PIPE_FOR_SCRIPT_DEBUGGER",L"WEBVIEW2_WAIT_FOR_SCRIPT_DEBUGGER"})SetEnvironmentVariableW(key,nullptr);
    const unsigned epoch=s->generation;std::weak_ptr<Impl> weak=s;
    auto handler=Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,HRESULT,ICoreWebView2Environment*>([weak,epoch](HRESULT result,auto environment){
        auto s=weak.lock();if(!s||!s->enabled||s->generation!=epoch)return S_OK;
        if(FAILED(result)||!environment){s->Fail();return S_OK;}environment->AddRef();s->environment.reset(environment);
        auto created=Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,HRESULT,ICoreWebView2Controller*>([weak,epoch](HRESULT hr,auto controller){
            auto s=weak.lock();if(!s||!s->enabled||s->generation!=epoch){if(controller)controller->Close();return S_OK;}
            if(FAILED(hr)||!controller){s->Fail();return S_OK;}controller->AddRef();s->controller.reset(controller);controller->put_IsVisible(FALSE);
            if(FAILED(controller->get_CoreWebView2(s->web.put()))){s->Fail();return S_OK;}s->web->get_BrowserProcessId(&s->pid);
            s->probing=true;if(!s->Configure())s->Fail();s->Notify();return S_OK;
        });
        auto hr=environment->CreateCoreWebView2Controller(s->parent,created);created->Release();if(FAILED(hr))s->Fail();return S_OK;
    });
    auto hr=create(nullptr,profile.c_str(),nullptr,handler);handler->Release();if(FAILED(hr)){s->Fail();return false;}s->Notify();return true;
}
void Browser::Disable(){impl->Close();}
void Browser::SetBounds(RECT r){if(EqualRect(&impl->bounds,&r))return;impl->bounds=r;if(impl->controller&&!impl->probing){impl->controller->put_Bounds(r);impl->controller->NotifyParentWindowPositionChanged();}}
void Browser::ParentMoved(){if(impl->controller&&!impl->probing)impl->controller->NotifyParentWindowPositionChanged();}
void Browser::Show(bool show){auto s=impl;if(show&&s->verified)s->locked=false;bool visible=show&&s->verified&&!s->locked;if(visible==s->visible)return;s->visible=visible;
    if(s->controller&&!s->probing){if(visible){Com<ICoreWebView2_3> control;if(s->Query(control))control->Resume();}s->controller->put_IsVisible(visible);if(!visible)s->Suspend();}}
void Browser::Lock(){auto s=impl;s->locked=true;s->visible=false;if(s->controller)s->controller->put_IsVisible(FALSE);if(s->web)s->web->Stop();s->Suspend();s->Notify();}
bool Browser::MountProbe(HWND probe){auto s=impl;if(!s->controller||!s->enabled)return false;
    if(!probe){s->controller->put_IsVisible(FALSE);s->controller->put_ParentWindow(s->parent);s->controller->put_Bounds(s->bounds);return true;}
    RECT r{0,0,128,96};return SUCCEEDED(s->controller->put_ParentWindow(probe))&&SUCCEEDED(s->controller->put_Bounds(r))&&SUCCEEDED(s->controller->put_IsVisible(TRUE));
}
void Browser::FinishProbe(bool passed){auto s=impl;if(!s->enabled)return;s->probing=false;s->verified=passed&&!s->locked;s->ready=s->verified;if(!passed)s->Fail();s->Notify();}
void Browser::Navigate(const std::wstring& value){auto s=impl;std::wstring address;if(!s->ready||s->locked||!s->web)return;
    if(!NormalizeWebAddress(value,address)){s->notice=L"address";s->Notify();return;}s->loaded=false;s->notice.clear();s->web->Navigate(address.c_str());}
void Browser::Back(){if(Ready()&&!impl->locked)impl->web->GoBack();}void Browser::Forward(){if(Ready()&&!impl->locked)impl->web->GoForward();}void Browser::Reload(){if(Ready()&&!impl->locked)impl->web->Reload();}
void Browser::Focus(){if(Ready()&&impl->visible)impl->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);}
bool Browser::Enabled()const{return impl->enabled;}bool Browser::Ready()const{return impl->ready;}bool Browser::Verified()const{return impl->verified;}
bool Browser::CanBack()const{BOOL value=FALSE;if(impl->web)impl->web->get_CanGoBack(&value);return value;}bool Browser::CanForward()const{BOOL value=FALSE;if(impl->web)impl->web->get_CanGoForward(&value);return value;}
bool Browser::Loaded()const{return impl->loaded&&Ready();}
bool Browser::Suspended()const{Com<ICoreWebView2_3> control;BOOL value=FALSE;if(impl->Query(control))control->get_IsSuspended(&value);return value;}
int Browser::ErrorCode()const{return impl->error;}
int Browser::State()const{return !impl->enabled?0:impl->error?3:impl->locked?4:impl->ready?2:1;}unsigned Browser::BrowserProcessId()const{return impl->pid;}unsigned Browser::Generation()const{return impl->generation;}
std::wstring Browser::Address()const{return impl->address;}
std::wstring Browser::Status(bool en)const{
    if(State()==0)return en?L"Browser is off. Enable only when needed.":L"Trình duyệt đang tắt. Bật khi cần dùng web.";
    if(State()==3)return en?L"Browser unavailable. Check the WebView2 Runtime or retry enabling.":L"Không mở được trình duyệt. Kiểm tra WebView2 Runtime hoặc bật lại.";
    if(State()==1)return en?L"Starting browser and verifying its rendered surface...":L"Đang mở trình duyệt và kiểm tra vùng render...";
    if(State()==4)return en?L"Browser hidden. Disable and enable to verify again.":L"Trình duyệt đã che. Tắt và bật lại để kiểm tra lại.";
    if(impl->notice==L"address")return en?L"Enter an HTTP or HTTPS address.":L"Nhập địa chỉ HTTP hoặc HTTPS.";
    if(impl->notice==L"navigation")return en?L"Page could not load. Check the connection or reload.":L"Không tải được trang. Kiểm tra kết nối hoặc tải lại.";
    return en?L"Web · Separate profile · Text chat":L"Web · Hồ sơ riêng · Chat bằng chữ";
}
}
