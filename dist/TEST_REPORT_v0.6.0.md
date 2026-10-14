# QuietShare v0.6.0 - Test report

Build and automated validation completed on Windows build 26200 x64. **999 checks PASS, 0 FAIL.** Physical receiver sign-off remains **NOT TESTED**. The original v0.3 baseline passed before changes; its results are retained in the sanitized evidence.

Host video controllers: Parsec Virtual Display Adapter (driver 0.45.0.0); NVIDIA GeForce GTX 1660 Ti (driver 32.0.15.9597). WGC captured the enumerated desktop display; D3D renderer readback used WARP software. A physical destination and its controller mapping were not observed.

## Reproduce

Requires Windows, Python 3, MinGW-w64 GCC 13.1.0 (or compatible x64 toolchain) and internet for the pinned dependency bootstrap. The installed application itself runs offline.

```powershell
.\build.ps1 -CompilerRoot "C:\path\to\mingw64\bin"
.\tests\run-all.ps1 -CompilerRoot "C:\path\to\mingw64\bin"
.\package.ps1
```

`run-all.ps1` serializes GUI tests because the product is single-instance. Pixel checks measure edit/read changes in the native tab/toolbar rows below the caption; read-only state and control behavior are checked independently. White PDF paper is preserved and checked separately from the dark shell.

## Automated results

| Suite | PASS | FAIL | Result |
|---|---:|---:|---|
| Content / file policy | 110 | 0 | PASS |
| Tabs / lazy project tree | 53 | 0 | PASS |
| Scintilla / Lexilla / PDFium | 151 | 0 | PASS |
| WGC / canary / cancellation | 52 | 0 | PASS |
| D3D11 production renderer / WARP | 16 | 0 | PASS |
| Native UI / fixtures / DPAPI / resources | 236 | 0 | PASS |
| Release PE / imports / embedded payloads | 30 | 0 | PASS |
| Shipping EXE / script lifecycle | 42 | 0 | PASS |
| Native scrollbar / separate QA EXE | 17 | 0 | PASS |
| Shipping EXE / multi-view integration | 179 | 0 | PASS |
| Optional browser / rendered WGC surface / lifecycle | 78 | 0 | PASS |
| Settings / slider live paint | 16 | 0 | PASS |
| Installer / upgrade / uninstall / rollback | 19 | 0 | PASS |
| **Total** | **999** | **0** | **PASS** |

Detailed sanitized check names and payload metadata: [test-evidence-v0.6.0.json](docs/test-evidence-v0.6.0.json). Raw fixtures, pixels, temporary profiles and diagnostics are local-only and excluded from Git and the portable package; temporary payloads can be cleaned after the final evidence is exported.

## What the checks establish

- **Core and persistence:** native plaintext script, Unicode DPAPI autosave/restart, corrupt-note recovery, single instance, read-only file previews, image orientation/transparency/zoom/pan and script preservation. Installer smoke installs, upgrades, rolls back and uninstalls in a fixed fixture, checks exact payload hashes, and verifies that the actual account's files, notes, shortcuts and registry are unchanged.
- **Tabs and project:** eight files plus script in the shipping EXE, invalid-file isolation, canonical case-insensitive deduplication, explicit reload/keep, close/middle-click/close-all, per-tab scroll/zoom/page, lazy folder expansion and hidden/heavy-directory filtering. Shipping checks cover two-level expansion, Enter file/folder actions, Ctrl+B, persistent side-by-side tree/content at narrow and medium widths, preserved branches on file/tab changes, splitter dragging and preferred-width recovery after resizing. Reparse/junction recursion and enumeration/tab/memory limits are checked in native module fixtures.
- **Code:** actual Scintilla class and Lexilla token-style IDs for Python, C++, JS, JSON, HTML and CSS; distinct comments/keywords/strings, contrast at least 4.5:1, Unicode/UTF-16 BOM input, line numbers, wrapping/find, Ctrl+wheel/reset and unknown-text null-lexer fallback. No code is executed.
- **PDF:** PDFium rendering inside the native canvas, 1-page and 120-page documents, image-only and mixed text/image PDFs, Vietnamese text-layer search/selection, invalid/password documents, zoom/fit/navigation/pan, 40 open/render/close cycles, bitmap/GDI release and recovery after size/time limits. Cross-thread sent close during render/find safely cancels; nested replacement/page/state changes are rejected until the active PDF call unwinds.
- **Focus and UI:** a minimal command strip with tabs and Windows caption hidden in F11; restored caption and user preferences on exit, compact exit action, overflowing tabs and content below controls at 826x553 and 430x320. Independent activity/tab/toolbar/status toggles, a protected Layout menu, local preference persistence and reset are exercised in the shipping EXE. Shell layouts cover VI/EN at simulated 96/120/144/192 DPI (280 normal-page/state combinations and 40 focus combinations), including the optional browser's off state. These are logical-scale tests, not physical multi-DPI monitor observations. Native captures of all pages, code, image, PDF, empty/error/masked states and focus were visually inspected.
- **Production privacy:** all four QA command-line switches are rejected by the shipping EXE. The separately compiled QA EXE uses only synthetic data. Production integration requires the actual WGC gate; F8 hides every host, tab/path/project/find surface and returns zero for gated numeric diagnostics. Ctrl+Tab/Shift+Tab/W travel through the real message loop using keyboard state shared only with the test-owned app; no global physical keystrokes are injected.
- **Recall:** the shipping Ctrl+Alt+Q handler is exercised through PID-bound WM_HOTKEY messages. Hidden/minimized restoration preserves foreground focus, keeps the unpinned style, retains capture exclusion, restores the real WGC gate and never clears F8 masking. Any required preflight is queued after restoration. Physical shortcut delivery while using another application is not part of these automated message tests.
- **Native pickers and size controls:** the shipping EXE opens capture-protected file/project pickers, multi-opens actual fixture files and loads a project tree on this host's single display. Battery and unrelated device notifications preserve the dialog; F8 closes it and locks content. Exact 37 pt input, a live 91 pt drag and invalid-size rejection are tested. A separate synthetic QA slider test reads only its owned client pixels at five drag values, confirming monotonically increasing blue fill before the native thumb position catches up.
- **WGC:** real positive-control capture sees an unprotected marker, then protected capture excludes it while public checkerboard pixels remain visible. Canary PASS on 1 enumerated display, observed 328 ms. Black/ambiguous/partial/invalid frames fail. Affinity loss, uncovered private popups, state invalidation, UI cancellation and synchronous close during WinRT initialization revoke confidence safely. The tools popup owns WDA 0x11 before it appears; real WGC tests confirm verification stays valid while open and after Esc. Only exact registered public monitor lists use the public-popup exception.
- **Capture pacing:** polling changed from 33 ms to 8 ms, the newest queued frame is presented, and status repainting is limited to 250 ms. This removes the former nominal 30 Hz polling ceiling; physical two-monitor FPS and latency were not measured on this one-monitor host.
- **Optional browser:** the shipping EXE starts with browsing off and creates no web profile until explicit enable. The actual WebView2 SVG marker passes positive and excluded WGC capture; native black backing cannot satisfy its positive test. Local HTTP fixtures exercise navigation, Back/Forward/Reload, popup/script/credential-dialog suppression, geolocation denial, F8, actual background suspension/resume, process exit on disable, pending-enable cancellation, separate-profile cookies, restart-default-off and offline readers without the loader. Input checks cover domains, URLs, localhost ports, Vietnamese/emoji UTF-8 queries and unsafe-scheme rejection. Shipping checks observe Vietnamese Google search navigation and Google/ChatGPT shortcuts. Cold start plus browser canary on this host was 3047 ms. Public ChatGPT HTTPS load observed: True; public Google search HTTPS load observed: True. Authenticated ChatGPT messages, third-party OAuth, client certificates and clean-machine missing-runtime behavior are not verified. User web data and raw profiles are excluded from exported evidence and Git.
- **D3D11:** the production shader/swapchain renderer presents a synthetic public texture using WARP, with framebuffer readback for aspect-preserving letterbox/pillarbox, blanking and owned GPU resource release. This is real software D3D rendering, not evidence of a physical destination screen.

## PASS / FAIL / NOT TESTED matrix

| Area | Status | Evidence / boundary |
|---|---|---|
| Current Windows x64 build, all automated suites | PASS | 999 checks, final binaries and embedded installer payloads match |
| Native UI screenshots and compact layouts | PASS | Actual native renderer; harmless synthetic data; no simulated HTML screens |
| WDA apply/readback and actual local WGC canary | PASS | Protected/unprotected controls; same WGC capture method |
| Synthetic failure classification and cancellation | PASS | No manual physical-Duplicate claim |
| Two real monitors in Extend with an observed receiver | NOT TESTED | Only 1 display enumerated on the test host |
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
- The 750 ms PDF budget is **cooperative**, not a hard real-time deadline. The stress fixture took **1359 ms** before rejecting unfinished output and recovering. A PDFium call may overrun a checkpoint.
- The EXE and Setup are **NotSigned** according to Windows Authenticode inspection. No signing certificate is bundled. Bundled DLLs are pinned AMD64 files and direct PE imports are Windows system libraries; clean-machine compatibility remains unverified.

## Final payload hashes

| File | Bytes | SHA-256 |
|---|---:|---|
| `QuietShare.exe` | 792,576 | `630998d957d1f5c44d4aebe3215db65042e25dc0213aedd61d228ed024aacf90` |
| `QuietShare-Setup.exe` | 12,418,560 | `da08214275ec32ceb87f5b4bcedffe64228745bd34cf874c86763a41741761c3` |
| `Scintilla.dll` | 1,131,880 | `a6957ae53a5bc92b3bdfe24c938c80df248a0a1f7a4c937ec5222646a52402fa` |
| `Lexilla.dll` | 1,400,168 | `3487bc191eb25b8475f55975341190817c9d37296e9ffa2e0179804eb7a2f087` |
| `pdfium.dll` | 7,512,064 | `ae913f407e8d3f5cdbf08a9869564d647b13e132e01eb376c0a5a33b7a227a45` |
| `WebView2Loader.dll` | 194,912 | `3426dcc55fdfb8b5e7ac623cf33b4ccf283fbf68a0c5a65bddfdb4d749a8abee` |

Installer RCDATA 301-306 matches the portable EXE, four DLLs and full redistribution notices byte for byte. Only the 195 KB WebView2 loader is bundled; the shared Evergreen Runtime is separate and optional for offline readers. The portable ZIP is checked against every staged release file before publication. The implementation specification is copied unchanged; old Git releases/history are retained.
