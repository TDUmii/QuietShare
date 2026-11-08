// Native media-policy / rendering fixture only. This simulates a successful
// canary; it is NOT evidence of capture exclusion or monitor privacy. The
// shipping integration suite performs the actual per-controller WGC proof.
// Permission settings are synthetic; camera/microphone devices are never opened.
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
    if(currentBrowser&&message==quietshare::BrowserChangedMessage&&wp==currentBrowser->Generation()){currentBrowser->AcknowledgeChanged();return 0;}
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
void CheckPastePolicy(quietshare::Browser& browser){
    const std::wstring policy=quietshare::kDomPolicy;
    const auto start=policy.find(L" for(const name of ['paste','drop'])");
    const auto end=policy.find(L" Object.defineProperty(window,'print'",start);
    Check("imagePastePolicyAvailable",start!=std::wstring::npos&&end!=std::wstring::npos);
    if(start==std::wstring::npos||end==std::wstring::npos)return;
    // Execute the exact production listener with synthetic event records.
    // No OS clipboard, physical input, user content or remote upload is used.
    const auto listener=policy.substr(start,end-start);
    auto check=[&](const char* name,const wchar_t* event,bool allowed){
        const auto script=LR"JS((()=>{let handler;const addEventListener=(name,fn)=>{if(name==='paste')handler=fn;};)JS"+
            listener+LR"JS(const target=document.createElement('div');target.contentEditable='true';document.body.append(target);
             const image=new File(['fixture'],'fixture.png',{type:'image/png'}),file=new File(['fixture'],'fixture.txt',{type:'text/plain'});
             let stopped=false;const e={type:'paste',isTrusted:true,target,clipboardData:{files:[image]},preventDefault(){stopped=true},stopImmediatePropagation(){}};
             )JS"+std::wstring(event)+L";handler(e);target.remove();return !stopped})()";
        Check(name,Script(browser,script)==(allowed?L"true":L"false"));
    };
    check("imagePastePolicyAllowsTrustedRasterImageInEditor",L"",true);
    check("imagePastePolicyKeepsTextPaste",L"e.clipboardData.files=[]",true);
    check("imagePastePolicyRejectsSyntheticFileEvents",L"e.isTrusted=false",false);
    check("imagePastePolicyRejectsNonImageAndMixedFiles",L"e.clipboardData.files.push(file)",false);
    check("imagePastePolicyRejectsDrop",L"e.type='drop'",false);
    check("imagePastePolicyRequiresAnEditableTarget",L"target.contentEditable='false'",false);
}
}

int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 2;
    std::puts("{\"method\":\"Native media-policy / rendering; simulated canary gate; isolated synthetic profile; WebView CapturePreview; synthetic paste event records; no OS clipboard access, device access, desktop capture or network navigation\",\"checks\":[");
    const std::wstring output=argv[1];CreateDirectoryW(output.c_str(),nullptr);
    const std::wstring profile=output+L"\\Profile-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
    if(FAILED(CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED)))return 2;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    WNDCLASSW cls{};cls.lpfnWndProc=WindowProc;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"QuietShareSyntheticNewTabFixture";RegisterClassW(&cls);
    HWND window=CreateWindowExW(WS_EX_TOOLWINDOW,cls.lpszClassName,L"QuietShare synthetic New Tab rendering fixture",WS_POPUP,30,30,358,197,nullptr,nullptr,cls.hInstance,nullptr);
    Check("renderFixtureWindowCreated",window!=nullptr);
    quietshare::Browser browser;currentBrowser=&browser;
    Check("browserMediaDefaultsToSoundOnAndDevicesOff",!browser.impl->muted&&!browser.impl->allowCamera&&!browser.impl->allowMicrophone);
    browser.SetMediaPreferences(true,true,true);
    Check("browserMediaPreferencesDoNotStartEngine",browser.impl->muted&&browser.impl->allowCamera&&browser.impl->allowMicrophone&&!browser.Enabled()&&browser.ControllerCount()==0);
    if(window){
        browser.SetBounds({0,0,358,197});browser.Enable(window,profile);browser.Show(true);
        // The launcher hides the console with STARTUPINFO; Windows applies
        // that flag to the first ShowWindow call. Show only this owned sample.
        ShowWindow(window,SW_SHOWNOACTIVATE);ShowWindow(window,SW_SHOWNOACTIVATE);
        const bool loaded=Pump([&](){return browser.Ready()&&browser.Loaded();},25000);
        Check("localNewTabRendersWithSyntheticGate",loaded&&browser.Address().empty());
        if(!loaded)std::fprintf(stderr,"Browser fixture state=%d error=%d ready=%d loaded=%d\n",browser.State(),browser.ErrorCode(),browser.Ready(),browser.Loaded());
        if(loaded){
            auto allMuted=[&](bool expected){for(const auto& tab:browser.impl->tabs){quietshare::Com<ICoreWebView2_8> audio;BOOL muted=FALSE;
                if(!tab->Query(audio)||FAILED(audio->get_IsMuted(&muted))||bool(muted)!=expected)return false;}return !browser.impl->tabs.empty();};
            Check("nativeFirstTabInheritsMutedBeforeEnable",allMuted(true)&&browser.impl->Active()->mediaReady);
            const auto originalTab=browser.ActiveTabId();const bool added=browser.NewTab();
            Check("nativeNewTabInheritsMuteAndPrivacyGate",added&&Pump([&](){return browser.Ready()&&browser.Loaded();},25000)&&allMuted(true)&&browser.Verified());
            browser.SetMediaPreferences(false,true,true);
            Check("nativeUnmuteUpdatesEveryExistingTab",allMuted(false));
            browser.SetMediaPreferences(true,true,true);
            Check("nativeMuteUpdatesEveryExistingTab",allMuted(true));
            const auto secondTab=browser.ActiveTabId();auto firstState=browser.impl->Find(originalTab);firstState->cameraOrigin=L"https://example.com";
            Check("deepSleepKeepsDeviceGrantedTabs",browser.SleepBackgroundTabs()==0&&browser.ControllerCount()==2);
            firstState->cameraOrigin.clear();
            Check("explicitDeepSleepReleasesOnlyBackgroundController",browser.SleepBackgroundTabs()==1&&browser.ControllerCount()==1&&browser.DormantCount()==1&&browser.Ready()&&browser.Verified()&&browser.ActiveTabId()==secondTab);
            Check("deepSleepPreservesTabMetadataWithoutLoadingSpinner",browser.Tabs().size()==2&&!browser.Tabs().front().loading&&browser.Tabs().front().id==originalTab);
            Check("openingDormantTabRequiresANewProof",browser.ActivateTab(originalTab)&&!browser.Ready()&&!browser.Verified()&&browser.PendingTabCount()==1);
            Check("dormantTabRestoresAfterFixtureProof",Pump([&](){return browser.Ready()&&browser.Loaded();},25000)&&browser.DormantCount()==0&&browser.ControllerCount()==2&&browser.ActiveTabId()==originalTab);
            browser.CloseTab(secondTab);
            if(browser.ActiveTabId()!=originalTab){browser.CloseTab(browser.ActiveTabId());Pump([&](){return browser.ActiveTabId()==originalTab&&browser.Loaded();});}
            auto tab=browser.impl->Active();auto profile4=tab->MediaProfile();auto permissionComplete=std::make_shared<bool>(false);
            auto remembered=new quietshare::PermissionCompletion([permissionComplete](HRESULT hr){*permissionComplete=SUCCEEDED(hr);});
            const HRESULT rememberedHr=profile4?profile4->p->SetPermissionState(COREWEBVIEW2_PERMISSION_KIND_CAMERA,L"https://example.com",COREWEBVIEW2_PERMISSION_STATE_ALLOW,remembered):E_NOINTERFACE;
            remembered->Release();
            Check("nativeSyntheticRememberedGrantCreatedWithoutDeviceAccess",SUCCEEDED(rememberedHr)&&Pump([&](){return *permissionComplete;}));
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
            Check("compactVietnameseNewTabHasNoHorizontalOverflow",Script(browser,L"innerWidth>250&&innerHeight>130&&document.documentElement.scrollWidth<=innerWidth&&(getComputedStyle(document.querySelector('h1')).display==='none')===matchMedia('(max-width:340px),(max-height:260px)').matches")==L"true");
            Check("compactVietnameseNewTabPreviewSaved",Preview(browser,output+L"\\newtab-vi-358x197.png"));
            const auto beforeLanguage=nonce();browser.SetEnglish(true);
            Check("newTabLanguageChangeUpdatesActualDom",Pump([&](){return browser.Loaded()&&nonce()!=beforeLanguage;})&&Script(browser,L"document.title==='New tab'&&document.querySelector('.add .caption').textContent==='Add shortcut'")==L"true");
            Check("compactEnglishNewTabHasNoHorizontalOverflow",Script(browser,L"innerWidth>250&&innerHeight>130&&document.documentElement.scrollWidth<=innerWidth")==L"true");
            Check("compactEnglishNewTabPreviewSaved",Preview(browser,output+L"\\newtab-en-358x197.png"));
            CheckPastePolicy(browser);
            auto memoryTab=browser.impl->Active();auto keptController=memoryTab->controller.p;
            Check("resourcePolicyHasSyntheticUnsentState",Script(browser,L"window.resourceDraft='UNSENT FIXTURE';true")==L"true");
            browser.Show(false);Check("hiddenTabCompletesItsSleepRequest",Pump([&](){return !memoryTab->suspendPending;},10000));
            quietshare::Com<ICoreWebView2_3> sleeping;quietshare::Com<ICoreWebView2_19> memory;
            Check("memoryTargetApiAvailableOnTestRuntime",memoryTab->Query(sleeping)&&memoryTab->Query(memory));
            if(sleeping&&memory){
                sleeping->Resume();memoryTab->SuspendComplete(E_FAIL,FALSE);
                COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL target{};
                Check("refusedSleepRequestsLowMemoryWithoutDiscardingController",SUCCEEDED(memory->get_MemoryUsageTargetLevel(&target))&&target==COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL_LOW&&memoryTab->controller.p==keptController&&browser.Verified());
                browser.Show(true);
                Check("foregroundRestoresNormalMemoryTarget",SUCCEEDED(memory->get_MemoryUsageTargetLevel(&target))&&target==COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL_NORMAL&&!memoryTab->reducedMemory);
                Check("resourcePolicyPreservesUnsentPageState",Script(browser,L"window.resourceDraft==='UNSENT FIXTURE'")==L"true");
            }else browser.Show(true);
            const auto stableTab=browser.ActiveTabId();auto stable=browser.impl->Active();const auto stableController=stable->controller.p;const auto stableGeneration=stable->generation;
            const auto countBefore=browser.Tabs().size();
            browser.SetDiagnosticPath(output+L"\\browser-fault.dat");
            for(const auto kind:{COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_UNRESPONSIVE,COREWEBVIEW2_PROCESS_FAILED_KIND_FRAME_RENDER_PROCESS_EXITED,
                COREWEBVIEW2_PROCESS_FAILED_KIND_UTILITY_PROCESS_EXITED,COREWEBVIEW2_PROCESS_FAILED_KIND_SANDBOX_HELPER_PROCESS_EXITED,COREWEBVIEW2_PROCESS_FAILED_KIND_GPU_PROCESS_EXITED,
                COREWEBVIEW2_PROCESS_FAILED_KIND_PPAPI_PLUGIN_PROCESS_EXITED,COREWEBVIEW2_PROCESS_FAILED_KIND_PPAPI_BROKER_PROCESS_EXITED,COREWEBVIEW2_PROCESS_FAILED_KIND_UNKNOWN_PROCESS_EXITED}){
                stable->ProcessFailure(kind,0,0);
                Check(("nonfatalProcessKeepsVerifiedController:"+std::to_string(kind)).c_str(),browser.Ready()&&browser.Verified()&&stable->controller.p==stableController&&stable->generation==stableGeneration&&browser.ActiveTabId()==stableTab&&browser.Tabs().size()==countBefore);
            }
            const auto diagnostic=browser.LastFailureDiagnostic();quietshare::Browser afterRestart;afterRestart.SetDiagnosticPath(output+L"\\browser-fault.dat");
            Check("browserDiagnosticSurvivesRestartWithoutStartingEngine",afterRestart.LastFailureDiagnostic()==diagnostic&&!afterRestart.Enabled());
            Check("browserDiagnosticContainsNoTitleAddressOrContent",diagnostic.find(L"QS-B010")!=std::wstring::npos&&diagnostic.find(L"https")==std::wstring::npos&&diagnostic.find(L"Documentation")==std::wstring::npos);
            browser.AcknowledgeChanged();MSG queued{};while(PeekMessageW(&queued,window,quietshare::BrowserChangedMessage,quietshare::BrowserChangedMessage,PM_REMOVE)){}
            for(int i=0;i<100;++i)stable->Notify();int notifications=0;
            while(PeekMessageW(&queued,window,quietshare::BrowserChangedMessage,quietshare::BrowserChangedMessage,PM_REMOVE))++notifications;
            Check("oneHundredBrowserUpdatesCoalesceIntoOneLayoutMessage",notifications==1);
            browser.AcknowledgeChanged();stable->Notify();Check("newBrowserUpdateCanNotifyAfterAcknowledgment",PeekMessageW(&queued,window,quietshare::BrowserChangedMessage,quietshare::BrowserChangedMessage,PM_REMOVE));browser.AcknowledgeChanged();
            browser.impl->memoryPressure=true;browser.impl->memory={40000,100,32000,12000};
            Check("criticalCommitPressureBlocksAdditionalControllersWithoutDiscardingTabs",!browser.CanNewTab()&&!browser.NewTab()&&browser.Tabs().size()==countBefore&&browser.Ready()&&browser.Verified()&&browser.Status(true).find(L"QS-B011")!=std::wstring::npos);
            stable->ResumeActivity();quietshare::Com<ICoreWebView2_19> memoryTarget;COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL targetLevel=COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL_NORMAL;
            Check("criticalCommitPressureRequestsLowMemoryWithoutSuspendingActivePage",stable->Query(memoryTarget)&&SUCCEEDED(memoryTarget->get_MemoryUsageTargetLevel(&targetLevel))&&targetLevel==COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL_LOW&&!browser.Suspended()&&browser.Ready());
            browser.impl->memoryPressure=false;browser.impl->memory={};
            stable->ResumeActivity();Check("recoveredCommitPressureRestoresNormalMemoryTarget",SUCCEEDED(memoryTarget->get_MemoryUsageTargetLevel(&targetLevel))&&targetLevel==COREWEBVIEW2_MEMORY_USAGE_TARGET_LEVEL_NORMAL&&!stable->reducedMemory);
            memoryTarget.reset();
            stable->ProcessFailure(COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_UNRESPONSIVE,1,259);
            Check("unresponsiveRendererOffersExplicitRetryWithoutRevokingVerifiedController",browser.CanRetry()&&browser.Ready()&&browser.Verified()&&stable->controller&&browser.Status(true).find(L"QS-B003")!=std::wstring::npos);
            browser.Reload();Check("explicitReloadRecreatesAndReverifiesUnresponsivePage",Pump([&](){return browser.Ready()&&browser.Loaded();},20000)&&!browser.CanRetry());
            stable->ProcessFailure(COREWEBVIEW2_PROCESS_FAILED_KIND_FRAME_RENDER_PROCESS_EXITED,COREWEBVIEW2_PROCESS_FAILED_REASON_OUT_OF_MEMORY,39);
            Check("subframeOutOfMemoryIsActionableRatherThanReportedRecovered",browser.Ready()&&browser.CanRetry()&&browser.Status(true).find(L"QS-B004")!=std::wstring::npos&&browser.Status(true).find(L"out of memory")!=std::wstring::npos&&browser.LastFailureDiagnostic().find(L"out-of-memory")!=std::wstring::npos);
            browser.Reload();Check("subframeMemoryRecoveryRestoresVerifiedTab",Pump([&](){return browser.Ready()&&browser.Loaded();},20000)&&browser.Verified());
            stable->suspendPending=true;stable->ProcessFailure(COREWEBVIEW2_PROCESS_FAILED_KIND_RENDER_PROCESS_EXITED,COREWEBVIEW2_PROCESS_FAILED_REASON_OUT_OF_MEMORY,39);
            Check("fatalRendererOutOfMemoryExplainsCommittedMemoryRecovery",browser.Status(true).find(L"out of memory")!=std::wstring::npos&&browser.CanRetry());
            Check("rendererFailureClearsAnOldPendingSleepRequest",!stable->suspendPending&&!stable->reducedMemory);
            Check("rendererFailureRetainsTabAndOffersExplicitRetry",!browser.Ready()&&!browser.Verified()&&browser.CanRetry()&&browser.ActiveTabId()==stableTab&&browser.Tabs().size()==countBefore&&!stable->controller);
            browser.Reload();Check("retryCreatesANewControllerAndPassesFixtureProof",Pump([&](){return browser.Ready()&&browser.Loaded();},20000)&&browser.ActiveTabId()==stableTab&&browser.Tabs().size()==countBefore);
            // An actual isolated renderer crash drives the production event
            // handler. This bypass is fixture-only, never a shipping URL.
            stable=browser.impl->Active();stable->web->Navigate(L"edge://crash");
            Check("actualRendererCrashIsHandledWithoutLosingTab",Pump([&](){return browser.CanRetry();},20000)&&browser.ActiveTabId()==stableTab&&browser.Tabs().size()==countBefore);
            browser.Reload();Check("actualRendererCrashCanBeRecreatedAndReverified",Pump([&](){return browser.Ready()&&browser.Loaded();},20000)&&browser.ActiveTabId()==stableTab);
            const auto ownBrowserPid=browser.BrowserProcessId();HANDLE ownBrowser=ownBrowserPid&&ownBrowserPid!=GetCurrentProcessId()?OpenProcess(PROCESS_TERMINATE|SYNCHRONIZE,FALSE,ownBrowserPid):nullptr;
            Check("isolatedBrowserProcessIsIdentifiedByItsOwnController",ownBrowser!=nullptr);
            if(ownBrowser){Check("isolatedBrowserCrashIsTriggered",TerminateProcess(ownBrowser,42));CloseHandle(ownBrowser);
                Check("actualBrowserExitRetainsAllTabsAndWaitsForExitedEvent",Pump([&](){return browser.CanRetry()&&browser.impl->environmentExited;},15000)&&!browser.Ready()&&browser.Tabs().size()==countBefore);
                browser.Reload();Check("browserExitRecreatesEnvironmentAndVerifiesActiveTab",Pump([&](){return browser.Ready()&&browser.Loaded();},20000)&&browser.ActiveTabId()==stableTab&&browser.Tabs().size()==countBefore);}
            browser.Disable();
            Check("browserMediaPreferencesSurviveDisable",browser.impl->muted&&browser.impl->allowCamera&&browser.impl->allowMicrophone&&!browser.Enabled());
            browser.Enable(window,profile);browser.Show(true);
            Check("newTabShortcutsSurviveEnvironmentRestart",Pump([&](){return browser.Ready()&&browser.Loaded();},25000)&&count(2)&&Script(browser,L"document.querySelectorAll('.tile>a').length===2&&document.querySelector('.caption').textContent==='Documentation'")==L"true");
            Check("nativeMutePreferenceSurvivesEnvironmentRestart",allMuted(true));
            auto restarted=browser.impl->Active();auto resetProfile=restarted->MediaProfile();auto permissionCount=std::make_shared<int>(-1);
            auto listed=quietshare::Callback<ICoreWebView2GetNonDefaultPermissionSettingsCompletedHandler,HRESULT,ICoreWebView2PermissionSettingCollectionView*>([permissionCount](HRESULT hr,auto settings){
                UINT count=0;if(FAILED(hr)||!settings||FAILED(settings->get_Count(&count)))return S_OK;*permissionCount=0;
                for(UINT i=0;i<count;++i){quietshare::Com<ICoreWebView2PermissionSetting> item;COREWEBVIEW2_PERMISSION_KIND kind{};
                    if(SUCCEEDED(settings->GetValueAtIndex(i,item.put()))&&SUCCEEDED(item->get_PermissionKind(&kind))&&
                       (kind==COREWEBVIEW2_PERMISSION_KIND_CAMERA||kind==COREWEBVIEW2_PERMISSION_KIND_MICROPHONE))++*permissionCount;}return S_OK;
            });
            const HRESULT listedHr=resetProfile?resetProfile->p->GetNonDefaultPermissionSettings(listed):E_NOINTERFACE;listed->Release();
            Check("nativeStartupClearsRememberedMediaGrants",SUCCEEDED(listedHr)&&Pump([&](){return *permissionCount>=0;})&&*permissionCount==0);
            const auto beforeRevoke=nonce();restarted->cameraOrigin=L"https://example.com";
            browser.SetMediaPreferences(true,false,true);
            Check("nativeRevocationClearsGrantAndReloadsDocument",restarted->cameraOrigin.empty()&&Pump([&](){return browser.Loaded()&&nonce()!=beforeRevoke;})&&browser.Verified()&&!browser.impl->allowCamera&&browser.impl->allowMicrophone);
            Check("actualDomRemoveAfterRestartPersists",mutate(L"document.querySelectorAll('.edit')[1].click();document.getElementById('remove').click()")&&count(1));
            Check("actualDomRemoveLastReturnsToEmptyState",mutate(L"document.querySelector('.edit').click();document.getElementById('remove').click()")&&count(0)&&Script(browser,L"document.querySelectorAll('.tile>a').length===0&&!!document.getElementById('add')")==L"true");
        }
        browser.Disable();currentBrowser=nullptr;DestroyWindow(window);
    }
    CoUninitialize();std::printf("\n],\"passed\":%u,\"failed\":%u}\n",passed,failed);return failed?1:0;
}
