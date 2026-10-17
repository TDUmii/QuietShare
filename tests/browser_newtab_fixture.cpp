// Rendering / DOM fixture only. This deliberately simulates a successful
// canary; it is NOT evidence of capture exclusion or monitor privacy. The
// shipping integration suite performs the actual per-controller WGC proof.
#include <windows.h>
#include <memory>
#include <string>
#include <vector>
#define private public
#include "../src/Browser.h"
#undef private
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"
#include "../src/Browser.cpp"
#pragma GCC diagnostic pop
#include <cstdio>

namespace {
quietshare::Browser* currentBrowser=nullptr;
unsigned passed=0,failed=0;
bool first=true;
void Check(const char* name,bool result){
    std::printf("%s{\"name\":\"%s\",\"passed\":%s}",first?"":",\n",name,result?"true":"false");
    first=false;result?++passed:++failed;std::fflush(stdout);
}
bool Pump(const std::function<bool()>& ready,DWORD timeout=12000){
    const ULONGLONG end=GetTickCount64()+timeout;
    do{
        MSG message{};while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
        if(ready())return true;
        MsgWaitForMultipleObjects(0,nullptr,FALSE,15,QS_ALLINPUT);
    }while(GetTickCount64()<end);
    return ready();
}
LRESULT CALLBACK WindowProc(HWND window,UINT message,WPARAM wp,LPARAM lp){
    if(currentBrowser&&message==quietshare::BrowserProbeMessage&&wp==currentBrowser->Generation()){
        const unsigned token=static_cast<unsigned>(lp);
        // Only host-generated New Tab HTML and synthetic saved shortcut labels
        // are used. This simulates the gate solely to inspect rendering / DOM.
        if(token==currentBrowser->ProbeToken()){
            const bool mounted=currentBrowser->MountProbe(nullptr,token);
            currentBrowser->FinishProbe(mounted,token);
        }return 0;
    }
    if(currentBrowser&&message==quietshare::BrowserTabActionMessage&&wp==currentBrowser->Generation()){
        currentBrowser->ProcessTabAction(static_cast<int>(lp));return 0;
    }
    if(message==WM_ERASEBKGND)return 1;
    if(message==WM_PAINT){PAINTSTRUCT paint{};BeginPaint(window,&paint);EndPaint(window,&paint);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
std::wstring Script(quietshare::Browser& browser,const std::wstring& script){
    auto tab=browser.impl->Active();if(!tab||!tab->web)return L"";
    struct Result{bool complete=false;HRESULT status=E_PENDING;std::wstring value;};auto result=std::make_shared<Result>();
    auto callback=quietshare::Callback<ICoreWebView2ExecuteScriptCompletedHandler,HRESULT,LPCWSTR>([result](HRESULT hr,LPCWSTR json){
        result->status=hr;result->value=json?json:L"";result->complete=true;return S_OK;
    });
    const HRESULT started=tab->web->ExecuteScript(script.c_str(),callback);callback->Release();
    if(FAILED(started)||!Pump([&](){return result->complete;},5000)||FAILED(result->status))return L"";
    return result->value;
}
bool Preview(quietshare::Browser& browser,const std::wstring& path){
    auto tab=browser.impl->Active();if(!tab||!tab->web)return false;
    const auto paintDeadline=GetTickCount64()+150;Pump([&](){return GetTickCount64()>=paintDeadline;},400);
    quietshare::Com<IStream> stream;if(FAILED(CreateStreamOnHGlobal(nullptr,TRUE,stream.put())))return false;
    struct Result{bool complete=false;HRESULT status=E_PENDING;};auto result=std::make_shared<Result>();
    // CapturePreview has a one-argument completion signature.
    class Done final:public ICoreWebView2CapturePreviewCompletedHandler {
        LONG references=1;std::function<void(HRESULT)> callback;
    public:
        explicit Done(std::function<void(HRESULT)> fn):callback(std::move(fn)){}
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** out)override{if(!out)return E_POINTER;*out=nullptr;
            if(iid==IID_IUnknown||iid==WebGuid<ICoreWebView2CapturePreviewCompletedHandler>()){*out=this;AddRef();return S_OK;}return E_NOINTERFACE;}
        ULONG STDMETHODCALLTYPE AddRef()override{return InterlockedIncrement(&references);}
        ULONG STDMETHODCALLTYPE Release()override{auto count=InterlockedDecrement(&references);if(!count)delete this;return count;}
        HRESULT STDMETHODCALLTYPE Invoke(HRESULT hr)override{callback(hr);return S_OK;}
    };
    auto done=new Done([result](HRESULT hr){result->status=hr;result->complete=true;});
    const auto started=tab->web->CapturePreview(COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_PNG,stream.p,done);done->Release();
    if(FAILED(started)||!Pump([&](){return result->complete;},5000)||FAILED(result->status))return false;
    STATSTG stat{};if(FAILED(stream->Stat(&stat,STATFLAG_NONAME))||stat.cbSize.QuadPart>10000000)return false;
    std::vector<unsigned char> bytes(static_cast<size_t>(stat.cbSize.QuadPart));LARGE_INTEGER begin{};ULONG read=0;
    if(FAILED(stream->Seek(begin,STREAM_SEEK_SET,nullptr))||FAILED(stream->Read(bytes.data(),static_cast<ULONG>(bytes.size()),&read))||read!=bytes.size())return false;
    HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;bool saved=WriteFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr)&&written==bytes.size();CloseHandle(file);return saved;
}
}

int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 2;
    std::puts("{\"method\":\"Rendering / DOM only; simulated canary gate; isolated synthetic profile; WebView CapturePreview; no desktop capture or network navigation\",\"checks\":[");
    const std::wstring output=argv[1];CreateDirectoryW(output.c_str(),nullptr);
    const std::wstring profile=output+L"\\Profile-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
    if(FAILED(CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED)))return 2;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSW cls{};cls.lpfnWndProc=WindowProc;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"QuietShareSyntheticNewTabFixture";RegisterClassW(&cls);
    HWND window=CreateWindowExW(WS_EX_TOOLWINDOW,cls.lpszClassName,L"QuietShare synthetic New Tab rendering fixture",WS_POPUP,30,30,358,197,nullptr,nullptr,cls.hInstance,nullptr);
    Check("renderFixtureWindowCreated",window!=nullptr);
    quietshare::Browser browser;currentBrowser=&browser;
    if(window){
        browser.SetBounds({0,0,358,197});browser.Enable(window,profile);browser.Show(true);
        // The launcher hides the console with STARTUPINFO; Windows applies
        // that flag to the first ShowWindow call. Show only this owned sample.
        ShowWindow(window,SW_SHOWNOACTIVATE);ShowWindow(window,SW_SHOWNOACTIVATE);
        const bool loaded=Pump([&](){return browser.Ready()&&browser.Loaded();},25000);
        Check("localNewTabRendersWithSyntheticGate",loaded&&browser.Address().empty());
        if(!loaded)std::fprintf(stderr,"Browser fixture state=%d error=%d ready=%d loaded=%d\n",browser.State(),browser.ErrorCode(),browser.Ready(),browser.Loaded());
        if(loaded){
            auto nonce=[&](){auto tab=browser.impl->Active();return tab?tab->localNonce:L"";};
            auto mutate=[&](const std::wstring& script){const auto old=nonce();Script(browser,script);return Pump([&](){return browser.Loaded()&&nonce()!=old;});};
            auto count=[&](size_t expected){std::vector<quietshare::BrowserShortcut> values;return quietshare::ReadShortcuts(profile+L"\\quietshare-shortcuts.txt",values)&&values.size()==expected;};
            Check("newTabContainsSearchAndAddShortcut",Script(browser,L"!!document.getElementById('query')&&!!document.getElementById('add')")==L"true");
            Check("newTabHasNoDefaultWebsiteTiles",Script(browser,L"document.querySelectorAll('.tile>a').length")==L"0");
            Check("actualDomAddShortcutPersists",mutate(L"document.getElementById('add').click();document.getElementById('name').value='Docs';document.getElementById('url').value='https://example.com/docs';document.getElementById('editor').requestSubmit()")&&count(1));
            Check("savedShortcutAppearsInActualDom",Script(browser,L"document.querySelector('.caption').textContent==='Docs'&&document.querySelector('.tile>a').getAttribute('href')==='https://example.com/docs'")==L"true");
            Check("actualDomEditShortcutPersists",mutate(L"document.querySelector('.edit').click();document.getElementById('name').value='Documentation';document.getElementById('url').value='https://example.com/updated';document.getElementById('editor').requestSubmit()")&&count(1)&&Script(browser,L"document.querySelector('.caption').textContent==='Documentation'&&document.querySelector('.tile>a').getAttribute('href')==='https://example.com/updated'")==L"true");
            const auto stale=nonce();
            Check("actualDomRejectsUnsafeShortcutUrl",mutate(L"document.getElementById('add').click();document.getElementById('name').value='Unsafe';document.getElementById('url').value='javascript:window.fixtureLeak=1';document.getElementById('editor').requestSubmit()")&&count(1)&&Script(browser,L"document.getElementById('notice').textContent.length>0&&window.fixtureLeak===undefined")==L"true");
            const auto unchanged=nonce();
            Script(browser,L"location.href='https://quietshare.invalid/shortcut/"+stale+L"/remove/0?'");
            const auto until=GetTickCount64()+250;Pump([&](){return GetTickCount64()>=until;},500);
            Check("staleRenderNonceCannotRemoveSavedShortcut",nonce()==unchanged&&count(1)&&browser.Address().empty());
            Check("actualDomEscapesSavedLabelMarkup",mutate(L"document.getElementById('add').click();document.getElementById('name').value='<img src=x onerror=window.fixtureLeak=1>';document.getElementById('url').value='https://example.com/label';document.getElementById('editor').requestSubmit()")&&count(2)&&Script(browser,L"document.querySelectorAll('img').length===0&&window.fixtureLeak===undefined&&document.querySelectorAll('.caption')[1].textContent==='<img src=x onerror=window.fixtureLeak=1>'")==L"true");
            Check("actualDomRemoveShortcutPersists",mutate(L"document.querySelectorAll('.edit')[1].click();document.getElementById('remove').click()")&&count(1));
            Check("actualDomAddsSecondIndependentShortcut",mutate(L"document.getElementById('add').click();document.getElementById('name').value='Project';document.getElementById('url').value='https://example.com/project';document.getElementById('editor').requestSubmit()")&&count(2));
            Check("compactVietnameseNewTabHasNoHorizontalOverflow",Script(browser,L"innerWidth>250&&innerHeight>130&&document.documentElement.scrollWidth<=innerWidth&&getComputedStyle(document.querySelector('h1')).display==='none'")==L"true");
            Check("compactVietnameseNewTabPreviewSaved",Preview(browser,output+L"\\newtab-vi-358x197.png"));
            const auto beforeLanguage=nonce();browser.SetEnglish(true);
            Check("newTabLanguageChangeUpdatesActualDom",Pump([&](){return browser.Loaded()&&nonce()!=beforeLanguage;})&&Script(browser,L"document.title==='New tab'&&document.querySelector('.add .caption').textContent==='Add shortcut'")==L"true");
            Check("compactEnglishNewTabHasNoHorizontalOverflow",Script(browser,L"innerWidth>250&&innerHeight>130&&document.documentElement.scrollWidth<=innerWidth")==L"true");
            Check("compactEnglishNewTabPreviewSaved",Preview(browser,output+L"\\newtab-en-358x197.png"));
            browser.Disable();browser.Enable(window,profile);browser.Show(true);
            Check("newTabShortcutsSurviveEnvironmentRestart",Pump([&](){return browser.Ready()&&browser.Loaded();},25000)&&count(2)&&Script(browser,L"document.querySelectorAll('.tile>a').length===2&&document.querySelector('.caption').textContent==='Documentation'")==L"true");
            Check("actualDomRemoveAfterRestartPersists",mutate(L"document.querySelectorAll('.edit')[1].click();document.getElementById('remove').click()")&&count(1));
            Check("actualDomRemoveLastReturnsToEmptyState",mutate(L"document.querySelector('.edit').click();document.getElementById('remove').click()")&&count(0)&&Script(browser,L"document.querySelectorAll('.tile>a').length===0&&!!document.getElementById('add')")==L"true");
        }
        browser.Disable();currentBrowser=nullptr;DestroyWindow(window);
    }
    CoUninitialize();std::printf("\n],\"passed\":%u,\"failed\":%u}\n",passed,failed);return failed?1:0;
}
