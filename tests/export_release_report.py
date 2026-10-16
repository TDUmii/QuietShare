"""Export only sanitized test names/results and release metadata, never fixture data."""
from pathlib import Path
import hashlib
import json
import sys
import os
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def read(name):
    return json.loads((ROOT / 'artifacts' / name).read_text(encoding='utf-8-sig'))

def checks(value):
    entries = value.get('checks', {})
    if isinstance(entries, dict):
        return [{'name': name, 'status': 'PASS' if result else 'FAIL'} for name, result in entries.items()]
    return [{'name': entry.get('name', entry.get('check')), 'status': entry.get('status',
             'PASS' if entry.get('passed') is True else 'NOT TESTED' if entry.get('passed') is None else 'FAIL')}
            for entry in entries]

def main():
    inventory = read('release-inventory.json')
    production = read('production-integration.json')
    life = read('lifecycle-check.json')
    browser = read('qa-browser/browser-regression.json')
    settings = read('qa-settings/settings-regression.json')
    actual_hash = hashlib.sha256((ROOT / 'dist/QuietShare.exe').read_bytes()).hexdigest()
    assert inventory['passed'] and production['exeSha256'] == life['exeSha256'] == actual_hash
    assert browser['exeSha256']==actual_hash and not browser['failure']
    assert settings['testedBinarySha256']==hashlib.sha256((ROOT/'build/QuietShare-QA.exe').read_bytes()).hexdigest() and not settings['failure']
    reports = [
        ('Content / file policy','qa-content/content-regression.json'),
        ('Tabs / lazy project tree','qa-documents/documents-regression.json'),
        ('Scintilla / Lexilla / PDFium','qa-viewers/viewer-regression.json'),
        ('WGC / canary / cancellation','qa-mirror/mirror-regression.json'),
        ('D3D11 production renderer / WARP','qa-mirror/mirror-render-regression.json'),
        ('Native UI / fixtures / DPAPI / resources','local-qa-report.json'),
        ('Release PE / imports / embedded payloads','release-inventory.json'),
        ('Shipping EXE / script lifecycle','lifecycle-check.json'),
        ('Native scrollbar / separate QA EXE','scrollbar-check.json'),
        ('Shipping EXE / multi-view integration','production-integration.json'),
        ('Optional browser / rendered WGC surface / lifecycle','qa-browser/browser-regression.json'),
        ('Consolidated Settings / validation / scrolling / controls','qa-settings/settings-regression.json'),
        ('Installer / upgrade / uninstall / rollback','qa-installer-smoke/installer-smoke.json')]
    suites=[]
    for name,path in reports:
        data=read(path)
        items=checks(data)
        if not items: # Mirror reports retain aggregate counts, with no private pixels/content.
            passed,failed=data['passed'],data['failed']
        else:
            passed=sum(item['status']=='PASS' for item in items)
            failed=sum(item['status']=='FAIL' for item in items)
        assert passed and not failed, (name,failed)
        suites.append({'name':name,'passed':passed,'failed':failed,
                       'notTested':sum(item['status']=='NOT TESTED' for item in items),'checks':items})
    total=sum(suite['passed'] for suite in suites)
    viewer=read('qa-viewers/viewer-regression.json')
    mirror=read('qa-mirror/mirror-regression.json')
    probe_env=os.environ.copy();probe_env.pop('PSModulePath',None)
    controllers=json.loads(subprocess.check_output([shutil.which('pwsh.exe') or 'powershell.exe','-NoProfile','-Command',
        'Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion | ConvertTo-Json -Compress'],
        env=probe_env,text=True))
    if isinstance(controllers,dict):controllers=[controllers]
    gpu_text='; '.join(f"{item['Name']} (driver {item['DriverVersion']})" for item in controllers)
    evidence={'version':'0.8.0','platform':'win-x64','windowsBuild':sys.getwindowsversion().build,
              'baselineV03':{'nativeUI':179,'content':60,'lifecycle':29,'scrollbar':17,'releasePE':7,'status':'PASS before changes'},
              'passed':total,'failed':0,'suites':suites,'files':inventory['files'],
              'authenticode':inventory['authenticode'],'monitorCount':mirror['monitorCount'],
              'actualWgcCanary':mirror['actualWgcCanary'],'canaryElapsedMs':mirror['canaryElapsedMs'],
              'listedVideoControllers':controllers,
              'pdfCooperativeBudgetMs':750,'pdfStressObservedMs':viewer['renderBudgetTestMilliseconds'],
              'browser':{'metrics':browser['metrics'],'notTested':browser['notTested'],'profile':'Separate local profile; never exported'},
              'settingsInteraction':{'metrics':settings.get('metrics',{}),'method':'Owned native-window wheel/focus/click messages; no physical touchpad observation'},
              'resourceProfiles':{'targetFps':[15,30,60],'pollIntervalMs':[33,16,8],'idleWatchdogMs':33,
                  'default':'Performance 60','wgcProducerRate':'Not forced; profiles cap app polling/copy/render work',
                  'userReportedExtendFpsApprox':60,'physicalExtendIndependentlyVerified':False},
              'screenshots':'Native renderer with synthetic data in a separate QA build; not HTML mockups or receiver evidence'}
    (ROOT / 'docs/test-evidence-v0.8.0.json').write_text(json.dumps(evidence,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    rows='\n'.join(f"| {s['name']} | {s['passed']} | 0 | PASS |" for s in suites)
    hashes='\n'.join(f"| `{name}` | {file['bytes']:,} | `{file['sha256']}` |" for name,file in inventory['files'].items())
    report=f'''# QuietShare v0.8.0 - Test report

Build and automated validation completed on Windows build {evidence['windowsBuild']} x64. **{total} checks PASS, 0 FAIL.** Physical receiver sign-off remains **NOT TESTED**. The original v0.3 baseline passed before changes; its results are retained in the sanitized evidence.

Host video controllers: {gpu_text}. WGC captured the enumerated desktop display; D3D renderer readback used WARP software. A physical destination and its controller mapping were not observed.

## Reproduce

Requires Windows, Python 3, MinGW-w64 GCC 13.1.0 (or compatible x64 toolchain) and internet for the pinned dependency bootstrap. The installed application itself runs offline.

```powershell
.\\build.ps1 -CompilerRoot "C:\\path\\to\\mingw64\\bin"
.\\tests\\run-all.ps1 -CompilerRoot "C:\\path\\to\\mingw64\\bin"
.\\package.ps1
```

`run-all.ps1` serializes GUI tests because the product is single-instance. Pixel checks measure edit/read changes in the native tab/toolbar rows below the caption; read-only state and control behavior are checked independently. White PDF paper is preserved and checked separately from the dark shell.

## Automated results

| Suite | PASS | FAIL | Result |
|---|---:|---:|---|
{rows}
| **Total** | **{total}** | **0** | **PASS** |

Detailed sanitized check names and payload metadata: [test-evidence-v0.8.0.json](docs/test-evidence-v0.8.0.json). Raw fixtures, pixels, temporary profiles and diagnostics are local-only and excluded from Git and the portable package; temporary payloads can be cleaned after the final evidence is exported.

## What the checks establish

- **Core and persistence:** native plaintext script, Unicode DPAPI autosave/restart, corrupt-note recovery, single instance, read-only file previews, image orientation/transparency/zoom/pan and script preservation. Installer smoke installs, upgrades, rolls back and uninstalls in a fixed fixture, checks exact payload hashes, and verifies that the actual account's files, notes, shortcuts and registry are unchanged.
- **Tabs and project:** eight files plus script in the shipping EXE, invalid-file isolation, canonical case-insensitive deduplication, explicit reload/keep, close/middle-click/close-all, per-tab scroll/zoom/page, lazy folder expansion and hidden/heavy-directory filtering. Shipping checks cover two-level expansion, Enter file/folder actions, Ctrl+B, persistent side-by-side tree/content at narrow and medium widths, preserved branches on file/tab changes, splitter dragging and preferred-width recovery after resizing. Reparse/junction recursion and enumeration/tab/memory limits are checked in native module fixtures.
- **Code:** actual Scintilla class and Lexilla token-style IDs for Python, C++, JS, JSON, HTML and CSS; distinct comments/keywords/strings, contrast at least 4.5:1, Unicode/UTF-16 BOM input, line numbers, wrapping/find, Ctrl+wheel/reset and unknown-text null-lexer fallback. No code is executed.
- **PDF:** PDFium rendering inside the native canvas, 1-page and 120-page documents, image-only and mixed text/image PDFs, Vietnamese text-layer search/selection, invalid/password documents, zoom/fit/navigation/pan, 40 open/render/close cycles, bitmap/GDI release and recovery after size/time limits. Cross-thread sent close during render/find safely cancels; nested replacement/page/state changes are rejected until the active PDF call unwinds.
- **Focus and UI:** a minimal command strip with tabs and Windows caption hidden in F11; restored caption and user preferences on exit, compact exit action, overflowing tabs and content below controls at 826x553 and 430x320. Independent activity/tab/toolbar/status toggles, a protected Layout menu, local preference persistence and reset are exercised in the shipping EXE. Shell layouts cover VI/EN at simulated 96/120/144/192 DPI (320 normal-page/state combinations and 40 focus combinations), including the optional browser's off state. These are logical-scale tests, not physical multi-DPI monitor observations. Native captures of all pages, code, image, PDF, empty/error/masked states and focus were visually inspected.
- **Production privacy:** all four QA command-line switches are rejected by the shipping EXE. The separately compiled QA EXE uses only synthetic data. Production integration requires the actual WGC gate; F8 hides every host, tab/path/project/find surface and returns zero for gated numeric diagnostics. Ctrl+Tab/Shift+Tab/W travel through the real message loop using keyboard state shared only with the test-owned app; no global physical keystrokes are injected.
- **Recall:** the shipping Ctrl+Alt+Q handler is exercised through PID-bound WM_HOTKEY messages. Hidden/minimized restoration preserves foreground focus, keeps the unpinned style, retains capture exclusion, restores the real WGC gate and never clears F8 masking. Any required preflight is queued after restoration. Physical shortcut delivery while using another application is not part of these automated message tests.
- **Native pickers and Settings:** the shipping EXE opens capture-protected file/project pickers, multi-opens actual fixture files and loads a project tree on this host's single display. Battery and unrelated device notifications preserve the dialog; F8 closes it and locks content. Exact manual 37 / 91 pt input and invalid-size rejection with a hidden status bar are tested. The separate synthetic Settings suite covers native scrolling/clipping, focus-into-view handlers, 8-200 pt and 25-400% manual input, inline validation, consolidated toggles, layout reset and the Display shortcut. Caret-at-end on focus/first click, native subsequent editing, small wheel packets, fractional motion, reversal and edge clamping are covered. Shipping tab-strip checks compare a whole notch with ten and 120 smaller packets on each axis and preserve active-tab visibility. Settings moves existing rows in a deferred native batch instead of refreshing the document/browser workspace for each wheel packet. The removed font slider is absent. These HWND tests do not claim physical touchpad, Tab or pointer delivery.
- **WGC:** real positive-control capture sees an unprotected marker, then protected capture excludes it while public checkerboard pixels remain visible. Canary PASS on {mirror['monitorCount']} enumerated display, observed {mirror['canaryElapsedMs']} ms. Black/ambiguous/partial/invalid frames fail. Affinity loss, uncovered private popups, state invalidation, UI cancellation and synchronous close during WinRT initialization revoke confidence safely. The tools popup owns WDA 0x11 before it appears; real WGC tests confirm verification stays valid while open and after Esc. Only exact registered public monitor lists use the public-popup exception.
- **Capture pacing and resources:** Saver/Balanced/Performance target 15/30/60 FPS, with 33/16/8 ms polling and a QPC frame deadline that retains fractional cadence without catch-up bursts. Performance 60 is the default. Changing profiles does not stop projection; the idle safety watchdog uses 33 ms rather than the former 8 ms, while the independent 250 ms display-affinity/topology guard remains. Profiles cap application polling/copy/render work, not the Windows WGC producer or website memory. Native timing checks cover the targets, and shipping checks cover local preference V2 persistence with V1 migration. The user reported approximately 60 FPS in Extend before this release; physical two-monitor FPS and latency were not independently measured on this one-monitor host.
- **Browser verification during projection:** a same-owner/source mounted browser proof preserves the existing capture, destination and projector while presentation stays frozen on its last safe public frame. The positive control and two consecutive excluded frames still run. Canary unmount/destruction and DWM flush precede a compositor QPC timestamp barrier; queued positive/checker frames at or before that barrier cannot reach output. Focused native-state and frame-timestamp regressions cover preservation, cancellation, no automatic resurrection and stale-frame rejection. Failed/unsafe proofs still stop and lock output. The actual browser WGC proof is exercised on this host; the complete simultaneous browser-plus-physical-projector sequence remains unverified without a second display.
- **File icons and screen identity:** Explorer caches its type-icon image list per DPI, with distinct code/language/config/image/PDF/folder icons and matching document-tab icons. Native tests cover image-list reuse/replacement and folder expansion state. Display labels use the Windows display number, primary role and the current window's monitor, retaining distinct source/destination defaults even at equal resolutions. Single-monitor live identity is tested; the physical dual-monitor label mapping still needs receiver-side observation.
- **Optional browser:** the shipping EXE starts with browsing off and creates no web profile until explicit enable. The actual WebView2 SVG marker passes positive and excluded WGC capture; native black backing cannot satisfy its positive test. Local HTTP fixtures exercise navigation, Back/Forward/Reload, popup/script/credential-dialog suppression, geolocation denial, F8, actual background suspension/resume, process exit on disable, pending-enable cancellation, separate-profile cookies, restart-default-off and offline readers without the loader. Input checks cover domains, URLs, localhost ports, Vietnamese/emoji UTF-8 queries and unsafe-scheme rejection. Shipping checks observe Vietnamese Google search navigation and Google/ChatGPT shortcuts. Cold start plus browser canary on this host was {browser['metrics']['coldStartAndCanaryMs']} ms. Public ChatGPT HTTPS load observed: {browser['metrics'].get('chatgptPublicHttpsLoaded',False)}; public Google search HTTPS load observed: {browser['metrics'].get('googleSearchPublicHttpsLoaded',False)}. Authenticated ChatGPT messages, third-party OAuth, client certificates and clean-machine missing-runtime behavior are not verified. User web data and raw profiles are excluded from exported evidence and Git.
- **D3D11:** the production shader/swapchain renderer presents a synthetic public texture using WARP, with framebuffer readback for aspect-preserving letterbox/pillarbox, blanking and owned GPU resource release. This is real software D3D rendering, not evidence of a physical destination screen.

## PASS / FAIL / NOT TESTED matrix

| Area | Status | Evidence / boundary |
|---|---|---|
| Current Windows x64 build, all automated suites | PASS | {total} checks, final binaries and embedded installer payloads match |
| Native UI screenshots and compact layouts | PASS | Actual native renderer; harmless synthetic data; no simulated HTML screens |
| WDA apply/readback and actual local WGC canary | PASS | Protected/unprotected controls; same WGC capture method |
| Synthetic failure classification and cancellation | PASS | No manual physical-Duplicate claim |
| Two real monitors in Extend with an observed receiver | NOT TESTED | Only {mirror['monitorCount']} display enumerated on the test host |
| Physical Duplicate, disconnect/reconnect, source/destination switch | NOT TESTED | Fail-closed code and synthetic checks exist; no hardware observation |
| Zoom / Meet / Teams / OBS receiving side | NOT TESTED | API acceptance/canary does not certify another capture path |
| Physical keyboard, Explorer drag/drop, modal folder-picker keyboard | NOT TESTED | Message/HDROP/controller paths tested, no human device interaction |
| Real 125/150/200% monitors and cross-monitor DPI changes | NOT TESTED | Simulated logical shell geometry only |
| Clean machine, standard non-admin account, Windows 10, ARM64 emulation | NOT TESTED | Per-user/asInvoker implementation and isolated current-host smoke only |
| Actual account installation | NOT TESTED | User account payload/notes/registry preserved; no production install performed |
| Resistance to reverse engineering | NOT TESTED | Native optimized/stripped release; no unbreakable protection claim |

## Limits

- Projection requires two Extend displays on the same GPU, different source/destination and the whole private window on the verified source. Duplicate, unknown topology, failed/expired verification, uncovered private surfaces and capture/device changes lock content and stop output. No override toggle is provided. External cameras and other OS/driver capture paths are outside the verified scope.
- PDF is deliberately one active page and one in-memory bitmap, capped at 64 MiB source, 10,000 pages and 16 million rendered pixels / 64 MiB bitmap. Password PDFs and rotation are unsupported. Its PDFium viewer uses no browser, OCR, disk thumbnails or persistent file-path history; optional web browsing is a separate on-demand surface.
- The 750 ms PDF budget is **cooperative**, not a hard real-time deadline. The stress fixture took **{viewer['renderBudgetTestMilliseconds']} ms** before rejecting unfinished output and recovering. A PDFium call may overrun a checkpoint.
- The EXE and Setup are **NotSigned** according to Windows Authenticode inspection. No signing certificate is bundled. Bundled DLLs are pinned AMD64 files and direct PE imports are Windows system libraries; clean-machine compatibility remains unverified.

## Final payload hashes

| File | Bytes | SHA-256 |
|---|---:|---|
{hashes}

Installer RCDATA 301-306 matches the portable EXE, four DLLs and full redistribution notices byte for byte. Only the 195 KB WebView2 loader is bundled; the shared Evergreen Runtime is separate and optional for offline readers. The portable ZIP is checked against every staged release file before publication. The implementation specification is copied unchanged; old Git releases/history are retained.
'''
    for name in ('TEST_REPORT_v0.8.0.md','TEST_REPORT.md'):
        (ROOT / name).write_text(report,encoding='utf-8')
    print(f'Sanitized report exported: {total} PASS, 0 FAIL; shipping EXE {actual_hash}')

if __name__=='__main__':
    main()
