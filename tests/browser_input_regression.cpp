// Include the implementation once so the isolated fixture can exercise the
// private on-disk format and HTML escaping without adding shipping test hooks.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsubobject-linkage"
#include "../src/Browser.cpp"
#pragma GCC diagnostic pop
#include <cstdio>
#include <string>
#include <utility>
#include <limits>

int main(){
    unsigned passed=0,failed=0;bool first=true;
    std::puts("{\"checks\":[");
    auto check=[&](const char* name,bool ok){
        std::printf("%s{\"name\":\"%s\",\"passed\":%s}",first?"":",\n",name,ok?"true":"false");
        first=false;ok?++passed:++failed;
    };
    std::wstring result;
    check("mediaOriginNormalizesHttpsCaseAndDefaultPort",quietshare::SecureMediaOrigin(L"HTTPS://Example.com:0443/path?q=1#part")==L"https://example.com");
    check("mediaOriginRetainsNondefaultPort",quietshare::SecureMediaOrigin(L"https://example.com:8443/path")==L"https://example.com:8443");
    check("mediaOriginAcceptsCanonicalIpv6",quietshare::SecureMediaOrigin(L"https://[2001:db8::1]:443/path")==L"https://[2001:db8::1]");
    check("mediaOriginRejectsHttpAndLocalhost",quietshare::SecureMediaOrigin(L"http://localhost/path").empty()&&quietshare::SecureMediaOrigin(L"http://127.0.0.1/").empty());
    check("mediaOriginRejectsHostlessAndLocalDocuments",quietshare::SecureMediaOrigin(L"https:///path").empty()&&quietshare::SecureMediaOrigin(L"about:blank").empty()&&quietshare::SecureMediaOrigin(L"data:text/html,fixture").empty());
    check("mediaOriginRejectsCredentials",quietshare::SecureMediaOrigin(L"https://trusted.example@other.example/").empty());
    check("mediaOriginRejectsMalformedPorts",quietshare::SecureMediaOrigin(L"https://example.com:").empty()&&quietshare::SecureMediaOrigin(L"https://example.com:0").empty()&&quietshare::SecureMediaOrigin(L"https://example.com:65536").empty());
    check("mediaOriginRejectsAuthorityEscapes",quietshare::SecureMediaOrigin(L"https://example.com%2f.other.example").empty()&&quietshare::SecureMediaOrigin(L"https://example.com\\other.example").empty()&&quietshare::SecureMediaOrigin(L"https://example.com other.example").empty());
    auto resolve=[&](const wchar_t* value,const wchar_t* expected){return quietshare::ResolveWebInput(value,result)&&result==expected;};
    auto reject=[&](const std::wstring& value){result=L"stale";return !quietshare::ResolveWebInput(value,result)&&result.empty();};
    check("inputExplicitHttps",resolve(L"https://example.com/page?q=1#part",L"https://example.com/page?q=1#part"));
    check("inputExplicitHttpLocalPort",resolve(L"http://127.0.0.1:1234/fixture",L"http://127.0.0.1:1234/fixture"));
    check("inputUppercaseHttps",resolve(L"HTTPS://example.com",L"HTTPS://example.com"));
    check("inputTrimOuterWhitespace",resolve(L" \thttps://example.com/page\r\n",L"https://example.com/page"));
    check("inputBareDomain",resolve(L"example.com",L"https://example.com"));
    check("inputBareDomainPath",resolve(L"example.com/path?q=a+b",L"https://example.com/path?q=a+b"));
    check("inputBareLocalhostPort",resolve(L"localhost:8080/page",L"https://localhost:8080/page"));
    check("inputPlainWordSearch",resolve(L"QuietShare",L"https://www.google.com/search?q=QuietShare"));
    check("inputSpaceSearch",resolve(L"native desktop browser",L"https://www.google.com/search?q=native%20desktop%20browser"));
    check("inputVietnameseUtf8",resolve(L"tìm kiếm tiếng Việt",L"https://www.google.com/search?q=t%C3%ACm%20ki%E1%BA%BFm%20ti%E1%BA%BFng%20Vi%E1%BB%87t"));
    check("inputReservedQueryCharacters",resolve(L"C++ & UI #1",L"https://www.google.com/search?q=C%2B%2B%20%26%20UI%20%231"));
    check("inputDotInPhraseStaysSearch",resolve(L"learn node.js",L"https://www.google.com/search?q=learn%20node.js"));
    check("inputEmojiUtf8",resolve(L"\U0001F680",L"https://www.google.com/search?q=%F0%9F%9A%80"));
    for(const auto& item:{std::pair<const char*,const wchar_t*>{"inputRejectScript",L"javascript:alert(1)"},
            {"inputRejectNumericUnsafeScheme",L"javascript:1234"},
            {"inputRejectData",L"data:text/html,<p>test</p>"},{"inputRejectFile",L"file:///C:/secret.txt"},
            {"inputRejectExternalProtocol",L"ms-excel:ofe|u|https://example.com"},
            {"inputRejectCredentials",L"https://name:password@example.com"},{"inputRejectEmptyHost",L"https://"},
            {"inputRejectMalformedHttps",L"https:example.com"},{"inputRejectBlank",L" \t\r\n"},
            {"inputRejectInteriorControl",L"two\nwords"}})check(item.first,reject(item.second));
    check("inputRejectOversize",reject(std::wstring(4097,L'a')));
    check("inputRejectOversizeEncodedQuery",reject(std::wstring(1500,L'ệ')));
    check("inputRejectInvalidUtf16",reject(std::wstring(1,wchar_t(0xd800))));
    check("strictNavigationDoesNotAcceptSearch",!quietshare::NormalizeWebAddress(L"tìm kiếm tiếng Việt",result));
    check("strictNavigationDoesNotAcceptUnsafeScheme",!quietshare::NormalizeWebAddress(L"javascript:alert(1)",result));
    std::wstring normalized;
    check("strictNavigationAcceptsResolvedSearch",resolve(L"test query",L"https://www.google.com/search?q=test%20query")&&quietshare::NormalizeWebAddress(result,normalized)&&normalized==result);
    quietshare::BrowserShortcut shortcut;
    auto saved=[&](const wchar_t* name,const wchar_t* url,const wchar_t* cleanName,const wchar_t* cleanUrl){
        return quietshare::NormalizeBrowserShortcut(name,url,shortcut)&&shortcut.name==cleanName&&shortcut.address==cleanUrl;
    };
    auto rejectShortcut=[&](const std::wstring& name,const std::wstring& url){
        shortcut={L"stale",L"stale"};return !quietshare::NormalizeBrowserShortcut(name,url,shortcut)&&shortcut.name.empty()&&shortcut.address.empty();
    };
    check("shortcutAcceptsExplicitHttps",saved(L"Documentation",L"https://example.com/docs",L"Documentation",L"https://example.com/docs"));
    check("shortcutTrimsOuterSpaces",saved(L"  Tài liệu  ",L" example.com/docs ",L"Tài liệu",L"https://example.com/docs"));
    check("shortcutAllowsLocalHttpWithoutExternalScheme",saved(L"Local",L"http://localhost:8080",L"Local",L"http://localhost:8080"));
    check("shortcutPreservesTextWithoutInterpretingMarkup",saved(L"<Example> & 'Docs'",L"https://example.com/?a=1&b=2",L"<Example> & 'Docs'",L"https://example.com/?a=1&b=2"));
    check("shortcutRejectsBlankName",rejectShortcut(L"   ",L"https://example.com"));
    check("shortcutRejectsNameRecordSeparator",rejectShortcut(L"A\tB",L"https://example.com"));
    check("shortcutRejectsNameLineBreak",rejectShortcut(L"A\nB",L"https://example.com"));
    check("shortcutRejectsNameControl",rejectShortcut(std::wstring(1,wchar_t(127)),L"https://example.com"));
    check("shortcutRejectsOversizeName",rejectShortcut(std::wstring(81,L'a'),L"https://example.com"));
    check("shortcutRejectsInvalidNameUtf16",rejectShortcut(std::wstring(1,wchar_t(0xd800)),L"https://example.com"));
    check("shortcutRejectsScriptUrl",rejectShortcut(L"Page",L"javascript:alert(1)"));
    check("shortcutRejectsDataUrl",rejectShortcut(L"Page",L"data:text/html,test"));
    check("shortcutRejectsFileUrl",rejectShortcut(L"Page",L"file:///C:/private.txt"));
    check("shortcutRejectsExternalProtocol",rejectShortcut(L"Page",L"ms-excel:test"));
    check("shortcutRejectsSearchQuery",rejectShortcut(L"Page",L"search words"));
    check("shortcutRejectsCredentialUrl",rejectShortcut(L"Page",L"https://user:password@example.com"));
    check("shortcutRejectsUrlRecordSeparator",rejectShortcut(L"Page",L"https://example.com\tvalue"));
    check("shortcutRejectsInvalidUrlUtf16",rejectShortcut(L"Page",L"https://example.com/"+std::wstring(1,wchar_t(0xd800))));
    check("shortcutRejectsOversizeUrl",rejectShortcut(L"Page",L"https://example.com/"+std::wstring(4096,L'a')));
    check("shortcutEnforcesLimitAfterAddingHttps",rejectShortcut(L"Page",L"example.com/"+std::wstring(4084,L'a')));
    check("shortcutDecodesVietnameseCommand",quietshare::DecodeComponent(L"T%C3%A0i%20li%E1%BB%87u",result)&&result==L"Tài liệu");
    check("shortcutRejectsTruncatedPercentEncoding",!quietshare::DecodeComponent(L"test%2",result));
    check("shortcutRejectsInvalidPercentEncoding",!quietshare::DecodeComponent(L"test%GG",result));
    check("shortcutRejectsMalformedUtf8Command",!quietshare::DecodeComponent(L"%ED%A0%80",result));
    const std::wstring hostile=L"</script><script>alert('fixture')</script>";
    auto html=quietshare::NewTabHtml(true,{{hostile,L"https://example.com/?a='value'&b=2"}},L"fixture-nonce",L"");
    check("shortcutHtmlEscapesLabelMarkup",html.find(hostile)==std::wstring::npos&&html.find(quietshare::Html(hostile))!=std::wstring::npos);
    check("shortcutHtmlEscapesUrlAttribute",html.find(L"https://example.com/?a=&#39;value&#39;&amp;b=2")!=std::wstring::npos);
    check("newTabCannotLoadSubresources",html.find(L"default-src 'none'")!=std::wstring::npos&&html.find(L"<img")==std::wstring::npos&&html.find(L"<iframe")==std::wstring::npos);
    check("newTabDoesNotContainFixedGoogleOrChatGptLinks",html.find(L"google.com")==std::wstring::npos&&html.find(L"chatgpt.com")==std::wstring::npos);
    check("newTabLocalCommandsCarryRenderNonce",html.find(L"https://quietshare.invalid/shortcut/fixture-nonce/")!=std::wstring::npos);
    check("newTabValidationStaysInsideProtectedPage",html.find(L"reportValidity")==std::wstring::npos&&html.find(L" required")==std::wstring::npos);
    check("hostHtmlUriEncodesExactAsciiDocument",quietshare::HostHtmlUri(L"<p>OK</p>")==L"data:text/html;charset=utf-8;base64,PHA+T0s8L3A+");
    check("hostHtmlUriIncludesUtf8Padding",quietshare::HostHtmlUri(L"Aệ")==L"data:text/html;charset=utf-8;base64,QeG7hw==");
    check("hostHtmlUriRejectsInvalidUtf16",quietshare::HostHtmlUri(std::wstring(1,wchar_t(0xd800))).empty());
    check("hostHtmlUriDoesNotWeakenPublicDataUrlValidation",!quietshare::NormalizeWebAddress(quietshare::HostHtmlUri(L"<p>OK</p>"),result)&&!quietshare::ResolveWebInput(quietshare::HostHtmlUri(L"<p>OK</p>"),result));
    wchar_t module[MAX_PATH]{};GetModuleFileNameW(nullptr,module,MAX_PATH);std::wstring folder=module;folder=folder.substr(0,folder.find_last_of(L"\\"));
    folder+=L"\\shortcut-store-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
    const auto store=folder+L"\\quietshare-shortcuts.txt",temporary=store+L".tmp";
    const bool folderCreated=CreateDirectoryW(folder.c_str(),nullptr)!=FALSE;
    std::vector<quietshare::BrowserShortcut> restored,values={{L"Tài liệu",L"https://example.com/docs"},{L"Local",L"http://localhost:8080"}};
    check("shortcutFixtureDirectoryIsIsolated",folderCreated);
    if(folderCreated){
        check("shortcutMissingStoreStartsEmpty",quietshare::ReadShortcuts(store,restored)&&restored.empty());
        check("shortcutExplicitSaveWritesStore",quietshare::WriteShortcuts(folder,values));
        check("shortcutRoundtripRetainsNamesAndUrls",quietshare::ReadShortcuts(store,restored)&&restored.size()==2&&restored[0].name==values[0].name&&restored[1].address==values[1].address);
        values[0]={L"Renamed",L"https://example.com/new"};
        check("shortcutEditReplacesExistingStore",quietshare::WriteShortcuts(folder,values)&&quietshare::ReadShortcuts(store,restored)&&restored.size()==2&&restored[0].name==L"Renamed"&&restored[0].address==L"https://example.com/new");
        const bool blocked=CreateDirectoryW(temporary.c_str(),nullptr)!=FALSE;
        check("shortcutFailedAtomicSavePreservesOriginal",blocked&&!quietshare::WriteShortcuts(folder,{})&&quietshare::ReadShortcuts(store,restored)&&restored.size()==2&&restored[0].name==L"Renamed");
        if(blocked)RemoveDirectoryW(temporary.c_str());
        values.erase(values.begin());
        check("shortcutRemovePersistsRemainingEntries",quietshare::WriteShortcuts(folder,values)&&quietshare::ReadShortcuts(store,restored)&&restored.size()==1&&restored[0].name==L"Local");
        check("shortcutRemoveLastPersistsEmptyStore",quietshare::WriteShortcuts(folder,{})&&quietshare::ReadShortcuts(store,restored)&&restored.empty());
        std::vector<quietshare::BrowserShortcut> overLimit(11,{L"Page",L"https://example.com"});
        check("shortcutStoreEnforcesTenEntryLimit",!quietshare::WriteShortcuts(folder,overLimit));
        HANDLE file=CreateFileW(store.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        std::wstring corrupt=L"\xfeffValid\thttps://example.com\nUnsafe\tjavascript:alert(1)\n";DWORD written=0;
        bool corruptWritten=file!=INVALID_HANDLE_VALUE&&WriteFile(file,corrupt.data(),static_cast<DWORD>(corrupt.size()*2),&written,nullptr);
        if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
        check("shortcutCorruptStoreDoesNotExposePartialData",corruptWritten&&!quietshare::ReadShortcuts(store,restored)&&restored.empty());
        DeleteFileW(store.c_str());DeleteFileW(temporary.c_str());RemoveDirectoryW(folder.c_str());
    }
    quietshare::Browser browser;
    browser.SetEnglish(true);browser.SetCommandShortcuts({{VK_F9,0,101},{VK_F10,MOD_CONTROL,102}});
    browser.SetMediaPreferences(true,true,true);browser.SetMediaPreferences(false,false,false);
    check("browserPreferencesDoNotLaunchEngine",!browser.Enabled()&&browser.ControllerCount()==0&&browser.BrowserProcessId()==0);
    check("tabsOffStateHasNoSession",browser.Tabs().empty()&&browser.ActiveTabId()==0&&browser.ActiveTab()<0&&
          browser.PendingTabCount()==0&&browser.ControllerCount()==0&&browser.BackgroundSuspendedCount()==0);
    check("tabsOffStateDoesNotStartEngine",!browser.CanNewTab()&&!browser.NewTab()&&!browser.Enabled()&&browser.BrowserProcessId()==0);
    check("tabsOffStateRejectsStaleActions",!browser.ActivateTab(1)&&!browser.CloseTab(1)&&!browser.CycleTab()&&!browser.CycleTab(true)&&
          !browser.ProcessTabAction(quietshare::BrowserTabNew)&&!browser.ProcessTabAction(quietshare::BrowserTabClose)&&
          !browser.ProcessTabAction(quietshare::BrowserTabNext)&&!browser.ProcessTabAction(quietshare::BrowserTabPrevious)&&
          !browser.ProcessTabAction(quietshare::BrowserTabQueued)&&!browser.ProcessTabAction(999));
    check("tabsOffStateRejectsProbeWithoutController",browser.ProbeToken()==0&&!browser.MountProbe(nullptr,1));
    check("zoomDefaultsTo100WithoutEngine",browser.ZoomPercent()==100&&!browser.Enabled()&&!browser.Ready()&&browser.BrowserProcessId()==0);
    check("zoomOffStateStoresPreferenceOnly",browser.SetZoomPercent(137)&&browser.ZoomPercent()==137&&!browser.Enabled()&&browser.BrowserProcessId()==0);
    check("zoomClampsLowerBound",browser.SetZoomPercent(-100)&&browser.ZoomPercent()==25);
    check("zoomClampsUpperBound",browser.SetZoomPercent(1000)&&browser.ZoomPercent()==400);
    check("zoomAdjustmentAvoidsPositiveOverflow",browser.AdjustZoomPercent((std::numeric_limits<int>::max)())&&browser.ZoomPercent()==400);
    check("zoomAdjustmentAvoidsNegativeOverflow",browser.AdjustZoomPercent((std::numeric_limits<int>::min)())&&browser.ZoomPercent()==25);
    check("zoomResetRestores100",browser.ResetZoom()&&browser.ZoomPercent()==100);
    browser.SetZoomPercent(150);browser.Disable();
    check("zoomPreferenceSurvivesDisable",browser.ZoomPercent()==150&&!browser.Enabled()&&browser.BrowserProcessId()==0);
    browser.FinishProbe(true,1);browser.ProcessTabAction(quietshare::BrowserTabQueued);
    check("tabsDisabledCallbacksCannotReviveSession",browser.Tabs().empty()&&browser.State()==0&&browser.ProbeToken()==0&&browser.ControllerCount()==0);
    std::printf("\n],\"passed\":%u,\"failed\":%u}\n",passed,failed);
    return failed?1:0;
}
