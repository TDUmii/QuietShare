#include "../src/Browser.h"
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
    quietshare::Browser browser;
    check("zoomDefaultsTo100WithoutEngine",browser.ZoomPercent()==100&&!browser.Enabled()&&!browser.Ready()&&browser.BrowserProcessId()==0);
    check("zoomOffStateStoresPreferenceOnly",browser.SetZoomPercent(137)&&browser.ZoomPercent()==137&&!browser.Enabled()&&browser.BrowserProcessId()==0);
    check("zoomClampsLowerBound",browser.SetZoomPercent(-100)&&browser.ZoomPercent()==25);
    check("zoomClampsUpperBound",browser.SetZoomPercent(1000)&&browser.ZoomPercent()==400);
    check("zoomAdjustmentAvoidsPositiveOverflow",browser.AdjustZoomPercent((std::numeric_limits<int>::max)())&&browser.ZoomPercent()==400);
    check("zoomAdjustmentAvoidsNegativeOverflow",browser.AdjustZoomPercent((std::numeric_limits<int>::min)())&&browser.ZoomPercent()==25);
    check("zoomResetRestores100",browser.ResetZoom()&&browser.ZoomPercent()==100);
    browser.SetZoomPercent(150);browser.Disable();
    check("zoomPreferenceSurvivesDisable",browser.ZoomPercent()==150&&!browser.Enabled()&&browser.BrowserProcessId()==0);
    std::printf("\n],\"passed\":%u,\"failed\":%u}\n",passed,failed);
    return failed?1:0;
}
