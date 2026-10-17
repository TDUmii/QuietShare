#include "../src/Viewers.h"
#include <SciLexer.h>
#include <Scintilla.h>
#include <atomic>
#include <cmath>
#include <fstream>
#include <iostream>
#include <objbase.h>
#include <string>
#include <thread>
#include <vector>

using namespace quietshare;
struct Check
{
    std::string name;
    bool pass;
};
std::vector<Check> checks;
PdfViewer *testPdf = nullptr;
int safetyCalls = 0, ordinaryCommands = 0;
bool confidenceLost = false;
UINT testMaskKey=VK_F8;
const LoadedContent *reopenFixture = nullptr;
bool busyOpenRejected = false, busyPageRejected = false, busyStateRejected = false;
LRESULT CALLBACK QaOwnerProc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    if ((message == WM_HOTKEY || message == WM_POWERBROADCAST || (message==WM_KEYDOWN&&wp==testMaskKey)) && testPdf)
    {
        ++safetyCalls;
        testPdf->SetPrivateVisible(false);
        return 0;
    }
    if (message == WM_COMMAND)
        ++ordinaryCommands;
    if (message == WM_APP + 14)
        return confidenceLost ? 0 : 1;
    if(message==WM_APP+30)return wp==testMaskKey;
    if (message == WM_CLOSE && testPdf)
    {
        if (reopenFixture)
        {
            std::wstring error;
            busyOpenRejected = !testPdf->Open(*reopenFixture, error);
            busyPageRejected = !testPdf->GoToPage(0);
            const auto before = testPdf->GetState();
            auto changed = before;
            changed.zoom = 123;
            testPdf->SetState(changed);
            busyStateRejected = testPdf->GetState().zoom == before.zoom;
        }
        testPdf->Close();
        DestroyWindow(window);
        return 0;
    }
    return DefWindowProcW(window, message, wp, lp);
}
void check(const std::string &name, bool pass)
{
    checks.push_back({name, pass});
    std::cout << (pass ? "PASS " : "FAIL ") << name << "\n";
}
double luminance(COLORREF color)
{
    double value = 0;
    const double weights[] = {0.2126, 0.7152, 0.0722};
    const int bytes[] = {GetRValue(color), GetGValue(color), GetBValue(color)};
    for (int i = 0; i < 3; ++i)
    {
        const double channel = bytes[i] / 255.0;
        value += weights[i] * (channel <= 0.04045 ? channel / 12.92 : pow((channel + 0.055) / 1.055, 2.4));
    }
    return value;
}
LoadedContent fixture(const wchar_t *name, const wchar_t *text)
{
    LoadedContent c;
    c.kind = ContentKind::Code;
    c.displayName = name;
    c.text = text;
    return c;
}
int at(const std::wstring &text, const wchar_t *value)
{
    const auto n = text.find(value);
    const auto prefix = text.substr(0, n);
    return WideCharToMultiByte(CP_UTF8, 0, prefix.data(), static_cast<int>(prefix.size()), nullptr, 0, nullptr,
                               nullptr);
}
int wmain(int argc, wchar_t **argv)
{
    if (argc != 2)
        return 2;
    const std::wstring dir = argv[1];
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    WNDCLASSW qaClass{};
    qaClass.lpfnWndProc = QaOwnerProc;
    qaClass.lpszClassName = L"QuietShareViewerQaOwner";
    qaClass.hInstance = GetModuleHandleW(nullptr);
    RegisterClassW(&qaClass);
    HWND owner = CreateWindowExW(0, qaClass.lpszClassName, L"QuietShare synthetic viewer QA", WS_OVERLAPPEDWINDOW, 0, 0,
                                 826, 553, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    check("ownerCreated", owner != nullptr);
    SetWindowDisplayAffinity(owner, 0x11);
    std::wstring error;
    CodeViewer code;
    PdfViewer pdf;
    testPdf = &pdf;
    check("ScintillaLexillaCreate", code.Create(owner, 201, error));
    check("PDFiumCreate", pdf.Create(owner, 202, error));
    MoveWindow(code.Window(), 0, 0, 780, 480, FALSE);
    MoveWindow(pdf.Window(), 0, 0, 780, 480, FALSE);
    const struct Sample
    {
        const wchar_t *name, *text, *comment, *keyword, *string;
        int commentId, keywordId, stringId;
    } samples[] = {{L"sample.py", L"# comment\ndef hello():\n    return \"tiếng Việt\"\n", L"#", L"def", L"tiếng",
                    SCE_P_COMMENTLINE, SCE_P_WORD, SCE_P_STRING},
                   {L"sample.cpp", L"// comment\nconst char* hello = \"tiếng Việt\";\n", L"//", L"const", L"tiếng",
                    SCE_C_COMMENTLINE, SCE_C_WORD, SCE_C_STRING},
                   {L"sample.js", L"// comment\nconst hello = \"tiếng Việt\";\n", L"//", L"const", L"tiếng",
                    SCE_C_COMMENTLINE, SCE_C_WORD, SCE_C_STRING},
                   {L"sample.json", L"// comment\n{\"hello\": true, \"text\": \"tiếng Việt\"}\n", L"//", L"true",
                    L"tiếng", SCE_JSON_LINECOMMENT, SCE_JSON_KEYWORD, SCE_JSON_STRING},
                   {L"sample.html", L"<!-- comment -->\n<div class=\"hello\">tiếng Việt</div>\n", L"comment", L"div",
                    L"hello", SCE_H_COMMENT, SCE_H_TAG, SCE_H_DOUBLESTRING},
                   {L"sample.css", L"/* comment */\ndiv { color: red; font-family: \"hello\"; }\n", L"comment",
                    L"color", L"hello", SCE_CSS_COMMENT, SCE_CSS_IDENTIFIER, SCE_CSS_DOUBLESTRING}};
    for (const auto &s : samples)
    {
        auto c = fixture(s.name, s.text);
        check("codeOpen:" + std::string(s.name, s.name + wcslen(s.name)), code.Open(c, error));
        code.SetPrivateVisible(true);
        const int a = code.TokenStyleAt(at(c.text, s.comment)), b = code.TokenStyleAt(at(c.text, s.keyword)),
                  d = code.TokenStyleAt(at(c.text, s.string));
        const auto name = std::string(s.name, s.name + wcslen(s.name));
        check(name + ":commentToken", a == s.commentId);
        check(name + ":keywordToken", b == s.keywordId);
        check(name + ":stringToken", d == s.stringId);
        check(name + ":differentStyles", a != b && b != d && a != d);
        check(name + ":readOnly", SendMessageW(code.Window(), SCI_GETREADONLY, 0, 0) == 1);
        check(name + ":lineNumbers", SendMessageW(code.Window(), SCI_GETMARGINTYPEN, 0, 0) == SC_MARGIN_NUMBER &&
                                         SendMessageW(code.Window(), SCI_GETMARGINWIDTHN, 0, 0) > 0);
        bool readable = true;
        for (const int style : {a, b, d})
        {
            const auto foreground = static_cast<COLORREF>(SendMessageW(code.Window(), SCI_STYLEGETFORE, style, 0));
            const auto background = static_cast<COLORREF>(SendMessageW(code.Window(), SCI_STYLEGETBACK, style, 0));
            readable &= (luminance(foreground) + 0.05) / (luminance(background) + 0.05) >= 4.5;
        }
        check(name + ":themeContrast", readable);
        check(name + ":unicodeFind",
              code.Find(L"tiếng") || (s.name == std::wstring(L"sample.css") && code.Find(L"hello")));
    }
    const struct Coverage
    {
        const wchar_t *name, *text;
        int lexer;
    } coverage[] = {{L"app.cs", L"class Test { string value = \"text\"; }", SCLEX_CPP},
                    {L"Main.java", L"class Main { public static void main() {} }", SCLEX_CPP},
                    {L"main.ts", L"interface Item { name: string; }", SCLEX_CPP},
                    {L"data.xml", L"<!--comment--><root key=\"text\"/>", SCLEX_XML},
                    {L"settings.yaml", L"# comment\nvalue: true\n", SCLEX_YAML},
                    {L"README.md", L"# Heading\n**text**\n", SCLEX_MARKDOWN},
                    {L"query.sql", L"-- comment\nSELECT name FROM items;\n", SCLEX_SQL},
                    {L"main.sh", L"# comment\nif true; then echo \"hello\"; fi\n", SCLEX_BASH},
                    {L"main.ps1", L"# comment\nfunction Test { Write-Output \"hello\" }\n", SCLEX_POWERSHELL},
                    {L"main.rs", L"// comment\nfn main() { let value = \"text\"; }\n", SCLEX_RUST},
                    {L"main.go", L"// comment\npackage main\nfunc main() {}\n", SCLEX_CPP},
                    {L"unknown.fixture", L"Plain Unicode tiếng Việt\n", SCLEX_NULL}};
    for (const auto &sample : coverage)
    {
        auto content = fixture(sample.name, sample.text);
        const std::string name(sample.name, sample.name + wcslen(sample.name));
        check("language:" + name,
              code.Open(content, error) && SendMessageW(code.Window(), SCI_GETLEXER, 0, 0) == sample.lexer);
    }
    auto large = fixture(L"long.py", L"# start\n");
    for (int n = 0; n < 200; ++n)
        large.text += L"print(\"Unicode tiếng Việt\") # long long long long long long long long long long long long "
                      L"long long long long long long long long long long\n";
    check("longCode", code.Open(large, error));
    code.SetPrivateVisible(true);
    SendMessageW(code.Window(), WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, 120), 0);
    check("codeCtrlWheelUsesPercentModel",
          code.GetState().zoom == 110 && SendMessageW(code.Window(), SCI_GETZOOM, 0, 0) == 0);
    const auto wheelState = code.GetState();
    SendMessageW(code.Window(), SCI_SETZOOM, 9, 0);
    code.SetZoom(100);
    check("codeResetClearsNativeZoomOffset",
          code.GetState().zoom == 100 && SendMessageW(code.Window(), SCI_GETZOOM, 0, 0) == 0);
    code.SetState(wheelState);
    check("codeWheelStateRestores", code.GetState().zoom == 110 && SendMessageW(code.Window(), SCI_GETZOOM, 0, 0) == 0);
    ViewerState cv;
    cv.scrollY = 80;
    cv.scrollX = 120;
    cv.zoom = 150;
    code.SetState(cv);
    check("codeState", code.GetState().scrollY == 80 && code.GetState().scrollX == 120 && code.GetState().zoom == 150);
    code.SetWrap(true);
    check("codeWrap", SendMessageW(code.Window(), SCI_GETWRAPMODE, 0, 0) == SC_WRAP_WORD);
    code.SetPrivateVisible(false);
    check("codeGateHides", !(GetWindowLongPtrW(code.Window(), GWL_STYLE) & WS_VISIBLE));
    check("codeGateFindDenied", !code.Find(L"Unicode"));
    code.Close();
    check("codeCloseClears", SendMessageW(code.Window(), SCI_GETLENGTH, 0, 0) == 0);
    LoadedContent one;
    check("validatedPdfLoad", LoadLocalContent(dir + L"\\one.pdf", one, error) && one.kind == ContentKind::Pdf);
    check("onePdfOpen", pdf.Open(one, error) && pdf.PageCount() == 1);
    pdf.SetPrivateVisible(false);
    check("pdfGateRenderDenied", !pdf.RenderCurrentPage());
    check("pdfGateFindDenied", !pdf.Find(L"QuietShare"));
    check("pdfGateCopyDenied", !pdf.CopySelection());
    pdf.SetPrivateVisible(true);
    check("onePdfRender", pdf.RenderCurrentPage() && pdf.CachedBytes() > 0);
    check("onePdfTextLayer", pdf.HasText());
    check("onePdfFind", pdf.Find(L"QuietShare"));
    check("onePdfFindWrap", pdf.Find(L"QuietShare"));
    check("onePdfSelectedText", pdf.SelectedText() == L"QuietShare");
    pdf.SetZoom(25);
    check("pdfZoom25", pdf.GetState().zoom == 25 && pdf.RenderCurrentPage());
    pdf.SetZoom(400);
    check("pdfZoom400", pdf.GetState().zoom == 400 && pdf.RenderCurrentPage());
    check("pdfCacheCap", pdf.CachedBytes() <= 64u * 1024 * 1024);
    pdf.FitPage();
    check("pdfFitPage", pdf.GetState().zoom == 100 && pdf.RenderCurrentPage());
    pdf.FitWidth();
    check("pdfFitWidth", pdf.GetState().zoom == 100 && pdf.RenderCurrentPage());
    const auto widthState = pdf.GetState();
    pdf.FitPage();
    pdf.SetState(widthState);
    check("pdfFitWidthState", pdf.GetState().fitWidth && pdf.RenderCurrentPage());
    pdf.FitPage();
    SendMessageW(pdf.Window(), WM_MOUSEWHEEL, MAKEWPARAM(0, 120), 0);
    check("pdfWheelPan", pdf.GetState().panY == 60);
    SendMessageW(pdf.Window(), WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, 120), 0);
    check("pdfCtrlWheelZoom", pdf.GetState().zoom == 110 && pdf.GetState().panY == 0);
    pdf.FitPage();
    pdf.RenderCurrentPage();
    SendMessageW(pdf.Window(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(231, 101));
    SendMessageW(pdf.Window(), WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(298, 101));
    SendMessageW(pdf.Window(), WM_LBUTTONUP, 0, MAKELPARAM(298, 101));
    check("pdfMouseTextSelection", !pdf.SelectedText().empty());
    LoadedContent many;
    check("manyPdfLoad", LoadLocalContent(dir + L"\\many.pdf", many, error));
    check("manyPdfOpen", pdf.Open(many, error) && pdf.PageCount() == 120);
    check("manyPdfLazy", pdf.CachedBytes() == 0);
    check("manyPdfNavigate", pdf.GoToPage(119) && pdf.GetState().page == 119 && pdf.RenderCurrentPage());
    check("manyPdfFindOtherPage", pdf.Find(L"page 3 test") && pdf.GetState().page == 2);
    check("pdfOutOfRange", !pdf.GoToPage(120) && pdf.GetState().page == 2);
    LoadedContent noText;
    check("textlessLoad", LoadLocalContent(dir + L"\\textless.pdf", noText, error));
    check("textlessRender", pdf.Open(noText, error) && pdf.RenderCurrentPage());
    check("textlessFindDisabled", !pdf.HasText() && !pdf.Find(L"text"));
    LoadedContent scanned;
    check("scannedLoad", LoadLocalContent(dir + L"\\scanned.pdf", scanned, error));
    check("scannedImageRender", pdf.Open(scanned, error) && !pdf.HasText() && pdf.RenderCurrentPage());
    LoadedContent mixed;
    check("mixedPdfLoad", LoadLocalContent(dir + L"\\mixed.pdf", mixed, error));
    check("mixedImageFirstPage", pdf.Open(mixed, error) && !pdf.HasText());
    check("mixedLaterPageFind", pdf.Find(L"Searchable") && pdf.GetState().page == 1 && pdf.HasText());
    check("mixedTextLayerAvailabilityKept", pdf.GoToPage(0) && pdf.HasText());
    LoadedContent uni;
    check("unicodePdfLoad", LoadLocalContent(dir + L"\\unicode.pdf", uni, error));
    check("unicodePdfRender", pdf.Open(uni, error) && pdf.RenderCurrentPage());
    check("unicodePdfTextFind", pdf.Find(L"tiếng Việt"));
    check("unicodePdfSelectedText", pdf.SelectedText() == L"tiếng Việt");
    LoadedContent password;
    check("passwordPdfLoad", LoadLocalContent(dir + L"\\password.pdf", password, error));
    check("passwordPdfUnsupported",
          !pdf.Open(password, error) && error.find(L"Password-protected") != std::wstring::npos);
    LoadedContent invalid;
    check("invalidPdfLoadValidation", LoadLocalContent(dir + L"\\invalid.pdf", invalid, error));
    const int old = pdf.PageCount();
    check("invalidPdfRejected", !pdf.Open(invalid, error) && pdf.PageCount() == old && !error.empty());
    const DWORD resources = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    bool cycles = true;
    auto openVisible = [&](const LoadedContent &content) {
        if (!pdf.Open(content, error))
            return false;
        pdf.SetPrivateVisible(true);
        return true;
    };
    for (int n = 0; n < 40; ++n)
    {
        cycles &= openVisible(one) && pdf.RenderCurrentPage();
        pdf.Close();
        cycles &= pdf.CachedBytes() == 0;
    }
    check("pdf40OpenRenderCloseCycles", cycles);
    check("pdfNoGdiResourceGrowth", GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS) <= resources + 2);
    check("pdfReopenAfterRelease", openVisible(one) && pdf.RenderCurrentPage());
    MoveWindow(pdf.Window(), 0, 0, 10000, 10000, FALSE);
    pdf.SetZoom(400);
    check("pdfEnormousRenderRejected", !pdf.RenderCurrentPage() && pdf.CachedBytes() <= 64u * 1024 * 1024);
    MoveWindow(pdf.Window(), 0, 0, 780, 480, FALSE);
    pdf.FitPage();
    check("pdfRecoveryAfterRenderCap", pdf.RenderCurrentPage());
    LoadedContent complex;
    check("complexPdfLoad", LoadLocalContent(dir + L"\\complex.pdf", complex, error));
    check("complexPdfOpen", pdf.Open(complex, error));
    pdf.SetPrivateVisible(true);
    std::thread safety([owner] { PostMessageW(owner, WM_HOTKEY, 2, 0); });
    safety.join();
    const bool rendered = pdf.RenderCurrentPage();
    check("pdfQueuedMaskPreemptsRender",
          !rendered && !(GetWindowLongPtrW(pdf.Window(), GWL_STYLE) & WS_VISIBLE) && !pdf.RenderCurrentPage());
    pdf.SetPrivateVisible(true);
    PostMessageW(owner, WM_POWERBROADCAST, PBT_APMSUSPEND, 0);
    check("pdfQueuedSuspendPreemptsRender",
          !pdf.RenderCurrentPage() && !(GetWindowLongPtrW(pdf.Window(), GWL_STYLE) & WS_VISIBLE));
    pdf.SetPrivateVisible(true);
    PostMessageW(pdf.Window(), WM_KEYDOWN, VK_F8, 1);
    check("pdfQueuedLocalF8PreemptsRender",
          !pdf.RenderCurrentPage() && !(GetWindowLongPtrW(pdf.Window(), GWL_STYLE) & WS_VISIBLE));
    pdf.SetPrivateVisible(true);
    const int callsBeforeRepeat = safetyCalls;
    PostMessageW(pdf.Window(), WM_KEYDOWN, VK_F8, static_cast<LPARAM>(1u << 30));
    check("pdfLocalF8RepeatIgnored", pdf.RenderCurrentPage() && safetyCalls == callsBeforeRepeat);
    testMaskKey=VK_F7;PostMessageW(pdf.Window(),WM_KEYDOWN,VK_F7,1);
    check("pdfConfiguredEmergencyKeyPreemptsRender",!pdf.RenderCurrentPage()&&!(GetWindowLongPtrW(pdf.Window(),GWL_STYLE)&WS_VISIBLE));
    testMaskKey=VK_F8;pdf.SetPrivateVisible(true);
    const int ordinaryBefore = ordinaryCommands;
    PostMessageW(owner, WM_COMMAND, 777, 0);
    check("pdfDoesNotPumpOrdinaryCommands", pdf.RenderCurrentPage() && ordinaryCommands == ordinaryBefore);
    MSG pending{};
    check("pdfOrdinaryCommandPreserved",
          PeekMessageW(&pending, owner, WM_COMMAND, WM_COMMAND, PM_REMOVE) && pending.wParam == 777);
    confidenceLost = true;
    // Cross two coarse Windows tick intervals before asserting the 16 ms gate poll.
    Sleep(40);
    check("pdfFreshOwnerConfidenceMasks", !pdf.RenderCurrentPage() &&
                                              !(GetWindowLongPtrW(pdf.Window(), GWL_STYLE) & WS_VISIBLE) &&
                                              pdf.CachedBytes() == 0);
    confidenceLost = false;
    pdf.SetPrivateVisible(true);
    LoadedContent expensive;
    check("expensivePdfLoad", LoadLocalContent(dir + L"\\expensive.pdf", expensive, error));
    check("expensivePdfOpen", pdf.Open(expensive, error));
    pdf.SetZoom(400);
    const ULONGLONG start = GetTickCount64();
    const bool expensiveRendered = pdf.RenderCurrentPage();
    const ULONGLONG elapsed = GetTickCount64() - start;
    std::cout << "Synthetic complex PDF render milliseconds: " << elapsed << "\n";
    check("pdfAggregateRenderBudget", !expensiveRendered && elapsed >= 750 && elapsed < 5000 && pdf.CachedBytes() == 0);
    check("pdfRecoveryAfterTimeBudget", pdf.Open(one, error) && pdf.RenderCurrentPage());
    pdf.SetPrivateVisible(false);
    check("pdfGateHides", !(GetWindowLongPtrW(pdf.Window(), GWL_STYLE) & WS_VISIBLE));
    check("pdfGateSelectionDenied", pdf.SelectedText().empty());
    pdf.Close();
    check("pdfCloseReleases", pdf.CachedBytes() == 0 && pdf.PageCount() == 0);
    for (bool finding : {false, true})
    {
        HWND closingOwner =
            CreateWindowExW(0, qaClass.lpszClassName, L"Synthetic sent-close regression", WS_OVERLAPPEDWINDOW, 0, 0,
                            826, 553, nullptr, nullptr, qaClass.hInstance, nullptr);
        PdfViewer closing;
        testPdf = &closing;
        check(finding ? "findCloseOwnerCreated" : "renderCloseOwnerCreated",
              closingOwner && closing.Create(closingOwner, 202, error));
        MoveWindow(closing.Window(), 0, 0, 780, 480, FALSE);
        check(finding ? "findCloseDocumentOpen" : "renderCloseDocumentOpen",
              closing.Open(finding ? many : expensive, error));
        closing.SetPrivateVisible(true);
        closing.SetZoom(400);
        reopenFixture = &one;
        busyOpenRejected = busyPageRejected = busyStateRejected = false;
        std::thread sentClose([closingOwner] { SendMessageW(closingOwner, WM_CLOSE, 0, 0); });
        bool queued = false;
        for (int attempt = 0; attempt < 200; ++attempt)
        {
            if ((GetQueueStatus(QS_SENDMESSAGE) >> 16) & QS_SENDMESSAGE)
            {
                queued = true;
                break;
            }
            Sleep(1);
        }
        const bool completed = finding ? closing.Find(L"missing synthetic query") : closing.RenderCurrentPage();
        sentClose.join();
        check(finding ? "findSentCloseQueued" : "renderSentCloseQueued", queued);
        check(finding ? "findSentCloseCancelsSafely" : "renderSentCloseCancelsSafely",
              !completed && !IsWindow(closingOwner) && closing.PageCount() == 0 && closing.CachedBytes() == 0);
        check(finding ? "findRejectsNestedOpenPageState" : "renderRejectsNestedOpenPageState",
              busyOpenRejected && busyPageRejected && busyStateRejected);
        reopenFixture = nullptr;
    }
    testPdf = &pdf;
    DestroyWindow(owner);
    testPdf = nullptr;
    CoUninitialize();
    const auto output = dir + L"\\viewer-regression.json";
    HANDLE file = CreateFileW(output.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    std::string json = "{\"checks\":[";
    int passed = 0;
    for (size_t i = 0; i < checks.size(); ++i)
    {
        if (i)
            json += ',';
        json += "{\"name\":\"" + checks[i].name + "\",\"passed\":" + (checks[i].pass ? "true" : "false") + "}";
        passed += checks[i].pass;
    }
    json += "],\"passed\":" + std::to_string(passed) + ",\"failed\":" + std::to_string(checks.size() - passed) +
            ",\"renderBudgetTestMilliseconds\":" + std::to_string(elapsed) + "}";
    DWORD written = 0;
    WriteFile(file, json.data(), json.size(), &written, nullptr);
    CloseHandle(file);
    return passed == static_cast<int>(checks.size()) ? 0 : 1;
}
