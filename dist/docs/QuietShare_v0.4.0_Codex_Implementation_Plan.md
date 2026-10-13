# QUIETSHARE v0.4.0 — IMPLEMENTATION SPEC (COMPACT)

Repo: `https://github.com/TDUmii/QuietShare.git` (`main`, private). Scope: **ship actual Windows x64 application** with refined UI, code syntax highlighting, multi-file tabs/project browsing, PDF reader, compact focus toolbar, and privacy-gated Smart Mirror. This document replaces the longer previous plan. No need to restate the plan; **implement, build, test, package, report**. Never claim successful hardware/capture testing without evidence. Do not push/publish/install without explicit request.

## 1. Baseline & non-negotiables

Read `README.md`, `TEST_REPORT.md`, `build.ps1`, `src/main.cpp`, `src/Privacy.*`, `src/Content.*`, `src/Installer.cpp`, and `tests/` before editing. Create a working branch; run existing tests and preserve their baseline. Prefer incremental refactor over a full rewrite.

Preserve: Win10/11 x64 native offline app; per-user/portable no-admin install; Windows DPAPI `notes.dpapi` compatibility; script autosave and restoration; editing vs reading; image preview/zoom/pan; F8 instant hide; F11 focus; Ctrl+Enter edit/read; pin/always-on-top; VI/EN; HiDPI; single instance; read-only file previews. **Previewed file content must never replace/autosave over script.** Maintain `WDA_EXCLUDEFROMCAPTURE=0x11` and the fail-closed policy for Windows physical Duplicate. Preserve existing file/security limits or introduce stricter justified limits.

Baseline source locations: `main.cpp` (UI, event loop, autosave), `Privacy.cpp` (`ApplyPrivacy`, display topology), `Content.cpp` (file validation/WIC), `Installer.cpp` (per-user installer). Split UI, viewer, tabs, PDF, editor, mirror, tests into modest focused modules; keep changes reviewable.

## 2. Definition of finished user experience

- Open `QuietShare.exe` to a polished dark desktop app with pages **Tổng quan / Lời nhắc / Trình chiếu / Cài đặt**. No oversized decorative cards inside content readers.
- Open **multiple files at once** (multi-select / drag-drop), or **open a local project folder** and browse its file tree lazily. Each file opens in a **compact tab**. Script has its own persistent tab; switch freely between script, code, images, PDFs without losing position/zoom.
- Source code shows **real token-based syntax colors for its language**, line numbers, proper monospace indentation, selection/find, UTF-8 and UTF-16 BOM as already supported. Code is **never executed**.
- PDFs render inside the app with page navigation, scroll and zoom; they are not just opened in external Edge/Acrobat or converted to screenshots in a browser. PNG/JPG/BMP/GIF first frame and plain text continue to work.
- In **F11 focus mode**, **remove the lone large `F11` button** shown in the screenshot. Replace it with **one slim compact top command/tab strip**, icon-only actions, tooltips and a horizontally scrollable overflow. Content remains visually dominant.
- With 2 monitors in **Extend**, Smart Mirror reproduces the public source monitor on a full-screen destination monitor, while private QuietShare remains local **only if actual capture-exclusion preflight passes**. In **physical Duplicate**, keep private content locked and explain Extend/Share Window options.
- Deliver updated EXE + Setup + portable ZIP, README, test report, any third-party notices, and truthful test evidence. If blocked by hardware, finish build + automated work and label manual tests pending rather than pretending they passed.

## 3. UI/UX specification (compact premium dark, not a brand clone)

**Look:** modern streaming-player inspiration, but QuietShare identity; restrained emerald accent, dark neutral surfaces, clear hierarchy, polished hover/pressed/disabled states. Suggested design tokens:
`bg #101215`, `panel #191D21`, `raised #252B30`, `border #343B42`, `text #F2F5F7`, `muted #9AA6AF`, `accent #23C876`, `danger #EB6868`; 8px spacing rhythm; radius 8–12px; Segoe UI Variable (fallback Segoe UI), Consolas/Cascadia Mono for code. Adequate contrast, 125/150/200% DPI, no clipped Vietnamese diacritics. No unnecessary shadows/gradients or large icons.

**Normal window:** slim title/status bar; narrow collapsible left navigation (icons+labels where space allows); main document area, compact tabs; settings/presentation in dedicated pages. Don't waste 20–30% of editor height on headings/toolbars. Reduce density gracefully in 430x320 minimum window; no controls over text. Show clear `Capture exclusion: Windows accepted / Viewer unverified / Locked`, never misleading green “100% hidden”.

**Focus mode is the main requirement (see screenshot):**

```text
┌───────────────────────────────────────────────────────────────────────────┐
│ [◀] [note ×] [main.py ×] [util.cpp ×] [guide.pdf ×] ‹ › │ + │ ⊖ 100% ⊕ │
│                                               ⛶  📌  👁/F8  ⋯         │
├───────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│   CONTENT: colored code / readable script / image / PDF, scrollable      │
│                                                                           │
└───────────────────────────────────────────────────────────────────────────┘
```

Mockup conceptual only: **actual implementation must remain native and use consistent vector/native-drawn icons**, not emoji. One narrow **30–34px** bar (32px target) along the top. Tabs ~24–28px high with ellipsized filename, close `×`, active underline; filename tooltip reveals full path. Keep toolbar buttons **26–30px** hit targets; pin, Focus exit/enter, zoom−/+, reset, Hide(F8), new/open file, overflow. Tool order: tab strip takes leftover width, action group compact on right. Overflow tabs horizontally via wheel/Shift+wheel/touchpad or small ‹ › arrows; overflow actions reachable via `⋯` menu, **not off-screen/unreachable**. Compact top bar never overlays text; focus on content. Optional auto-hide toolbar only with persistent discoverable top-edge hotspot + keyboard/menu fallback; default **bar always visible**. Preserve `F11` as keyboard shortcut and use a small **exit-focus icon**, NOT a standalone large F11 pill. F8 Hide must immediately conceal **all** tabs/viewers/project tree/dialog previews; don't show secret tab titles while locked.

Layout invariants: scrollbars dark/consistent; no nested awkward scrolling; unobtrusive 12–16px content padding; selectable/copyable text, no emoji icons in shipping UI; focus traversal & tooltips in both VI/EN. Dark code token palette distinct from normal text. On resize, keep active tab and scroll/zoom; avoid flicker with buffered painting.

## 4. Multi-tab and project architecture

Introduce `Document { id, type(script|text|code|image|pdf), canonicalPath, displayName, language, loadingState, scrollState, zoomState, dirty, ... }`, `TabManager`, `DocumentHost` and per-view controllers. One script document independent of file documents. **External file previews read-only** (do not add editing unless explicitly planned). De-duplicate tabs by normalized canonical local path (case-insensitive on Windows), activate existing tab instead of duplicating. Close active tab, Ctrl+W; Ctrl+Tab/Ctrl+Shift+Tab; middle-click close; Ctrl+O multi-select; Ctrl+Shift+O open folder; Ctrl+F find for text/code and searchable PDFs when text is available. Closing a tab releases per-tab resources, not script autosave. Recent tab position/zoom in memory per session. Do not persist sensitive file paths by default.

Project folder tree: opt-in, local drives only; lazy enumerate folders; ignore hidden/build `.git`, `node_modules`, `.venv`, `dist`, `build` by default (configurable), avoid reparse/junction recursion, cap expansion and tab count with graceful UX. Drag-dropping N valid files creates N tabs; failure of one file must not clear others. Reopening a modified file offers reload, not silent clobber. Show tab basename, type icon and tooltip full path. Allow quickly closing others/all except script. Support keyboard navigation without requiring mouse.

## 5. Syntax highlighting (must be real, not colored whole lines)

Recommended for Win32 native: **Scintilla + Lexilla** (`Scintilla` read-only code widget, `Lexilla` lexers), bundled x64 with licenses. Verify compatibility, version and supply-chain provenance. Keep RichEdit for editable script/plain text where it already works. Apply native dark token styles: keywords, strings, comments, functions/types/numbers/operators; detect extension and special basenames. Required coverage: Python, C/C++, C#, Java, JavaScript/TypeScript, HTML, CSS, JSON, XML, YAML, Markdown, SQL, Bash/PowerShell, Rust/Go (reasonable fallback for missing lexer). Unknown file -> monochrome read-only. Must support long lines, tabs/spaces, UTF-8 Unicode/Vietnamese, horizontal/vertical scroll, line numbers, line wrapping toggle, Ctrl+F find, text selection/copy, and 5 MiB file cap. Files never execute nor initiate arbitrary processes.

Do not use regex-only syntax highlighting for all languages or embed full VS Code/browser just to color code. Tests need fixtures that assert **different lexer/token style IDs** for comment, string, keyword (at least Python/C++/JS/JSON/HTML/CSS) and verify theme readability. If Scintilla/Lexilla choice is technically blocked, document reason and substitute equally native embeddable lexer with demonstrable token styling.

## 6. Native PDF viewer

Use **PDFium native C API** (bundle redistributable x64 binary or reproducible build with license/third-party notice; validate provenance, architecture, versions). Do not require Acrobat, Internet, administrator rights, WebView2, Chromium browser, or network. New PDF tab renders pages on demand to memory bitmap/cache with capped memory + lazy prefetch; scroll continuous preferred, single-page fallback if justified; page `n/N`, previous/next, fit-width/fit-page, Ctrl+wheel zoom (25–400%), Ctrl+0 reset, wheel/pan, mouse selection/copy/find when PDF text layer exists. For image-only scanned PDFs, render faithfully and disable unavailable text search rather than fake it. Support rotation if inexpensive; otherwise defer explicitly. Password-protected documents: secure password prompt and PDFium open; if not implemented, display a clear unsupported error without leaking file path/content. Bound memory/file size/page pixel dimensions; reject corrupt/enormous docs gracefully, free resources on tab close, never write thumbnails/pages to temp/log by default. Apply capture exclusion to **any extra top-level PDF/print/password dialogs**; safer to use protected owner/inline UI. PDF content is only visible after the common privacy gate.

## 7. Smart Mirror: Windows Extend only

Keep `Privacy.cpp` `WDA_EXCLUDEFROMCAPTURE (0x11)`, `TryGetDisplayCloned()` and `Blocked()` semantics. Introduce `MonitorService` (enumerate source/destination stable IDs, DPI/resolution/topology), `CaptureSession` (`Windows.Graphics.Capture` monitor capture), `D3DRenderer` (D3D11, aspect-preserving letterbox), `ProjectorWindow` (borderless, monitor destination, non-capturable/self-protected where appropriate), `MirrorController` (state/errors/retry). Ensure separate adapter handling or clear unsupported status, minimize GPU copies, handle device lost, suspend, disconnect, resize, mode switch, multi-monitor negative coordinates, content-size change and cancellation. Do not change Windows topology automatically. If MinGW toolchain cannot use WinRT WGC cleanly, add documented MSVC + Windows SDK build for the expanded product; retain legacy build option only where genuinely supported. Bundle all runtime DLLs including PDFium, Scintilla/Lexilla and installer payload; preserve x64/per-user behavior.

**Mandatory privacy gate:** before revealing private notes or starting projection, require supported Win10 2004+; DWM enabled; `SetWindowDisplayAffinity` succeeded + readback `0x11`; display topology known, not cloned; private windows covered; actual **WGC synthetic canary** test for the same capture method. Render an obvious synthetic secret marker in a private protected overlay on source, acquire real WGC frames of source and programmatically confirm marker is absent and surrounding public pixels are present. Fail, timeout, ambiguous, black frame or untested capture path -> **LOCK/NO START**, never “probably safe”. Test without actual secrets. Repeat safety validation after topology/device/capture session changes; suspend/blank projector immediately on lost confidence. Explicitly label remote Zoom/Meet/OBS and physical-projector receiver unverified until human observation test. Never imply software prevents external camera or OS/driver captures not honoring exclusion. Windows true Duplicate **always lock**, show guidance `Win+P -> Extend` or share individual window. No override toggle.

Smart Mirror controls: Source dropdown, Destination dropdown, Verify privacy, Start, Stop, pointer option, FPS if measured, status/error and clear Extend/physical Duplicate explanation. On target show only public frame, with no private UI, to avoid feedback loops; target must differ from source and be in Extend. Require explicit Start. On close, stop and release capture/GPU/window. Optionally add `Share Window` instructions, not a fake API toggle for meeting software.

## 8. Security and performance edge cases

- One gate (`IsPrivateContentSafe`) consulted by script, code, PDF, image, dialogs, opened project tree and tooltip/path surfaces; mask/duplicate/unknown => suppress *content and sensitive metadata*. F8 hide must also stop auto-scroll and mask native sub-windows.
- UI must never expose private data through capture canary logs, diagnostics, thumbnails, clipboard writes, crash logs, title bar, taskbar previews or history. No analytics/cloud/network. Sanitized errors only; never silently emit PDF/code content to files.
- Keep notes DPAPI storage backward compatible and atomic; unreadable old note backed up rather than overwritten; test corruption/recovery and second instance.
- Caps: keep files/code ≤5 MiB, images ≤25 MiB/40 MP as baseline; define conservative PDF cap and rendering pixel/memory budgets with actionable error messages. Tabs load lazily, free inactive heavyweight views. Performance target: prompt tab switching for small files, responsive UI under 430×320, mirror aim ≥30fps at 1080p where supported; report measured values not invented claims.
- Third-party libraries: record pinned versions, licenses and SHA-256 checksums; package DLLs in install + portable; installer uninstall must leave user notes intact.

## 9. Implementation order (execute without stopping at proposal)

1. **Baseline:** checkout branch, compile/run existing app/tests, record failing baseline. Audit current controls/keyboard/privacy logic. Implement shared document model and safety guard before showing new viewers.
2. **UI shell:** theme tokens; navigation; compact common document host; high-DPI controls; F11 focus bar with tab scroll/overflow/pin/zoom/hide, VI/EN tooltips; retain F8/F11 shortcuts. Use screenshot in task as UX evidence for the oversized lone F11 control that must disappear.
3. **Tabs/project:** multi-select, multi-drop, lazy folder tree, identity/de-dupe, resource lifetime, per-tab view state. Script tab must be isolated from file previews.
4. **Code viewer:** integrate Scintilla/Lexilla, lexer mapping, token styles, line numbers, find, wrapping; test fixture code paths.
5. **PDF:** integrate PDFium, page render/zoom/navigation/search where text is available, memory caps/error handling; other viewers unchanged.
6. **Mirror POC → production:** create real WGC/D3D11 capture on Extend, destination presenter window; privacy canary preflight on exact capture path, state transitions/error handling; no true Duplicate bypass.
7. **QA:** add regression/integration tests below, run all old tests, manually review focus window at screenshot dimensions, keyboard and DPI/VI/EN. Never convert old test report into v0.4 results without rerunning.
8. **Release:** update README/CHANGELOG/THIRD-PARTY, package portable EXE + per-user setup + ZIP including DLLs/resources, install/uninstall smoke test where permissions allow; provide evidence, limitations and final paths.

## 10. Acceptance matrix — must report actual PASS / FAIL / NOT TESTED

| Area | Required verification |
|---|---|
| Core regressions | F8/F11/Ctrl+Enter, DPAPI restore/backup, 1 instance, drag drop, text/image + original tests |
| Focus UI | At ~826×553 from screenshot and min 430×320: one slim strip; no big `F11` pill; icon actions reachable, no text clipping; tabs scroll, tooltip, focus keyboard |
| Tabs | ≥8 files open, switch/close/reorder if implemented, duplicate tab prevention, Ctrl+Tab/Ctrl+W, reopen script intact, failed file does not discard other tabs |
| Project | Open folder, lazy browse, ignore heavy dirs, handle symlinks/junctions, no UI freeze |
| Code | Token-style assertions for Python/C++/JS/JSON/HTML/CSS; line numbers, UTF-8 VN, wrap/find/copy; never execute |
| PDF | 1 page, 100+ pages lazy load, scanned, UTF-8 text layer, invalid/password-protected, zoom 25–400, fit, search when supported, no resource leak |
| Privacy | WDA apply/readback; canary positive/negative tests; F8 masks every view and tab/title; fail closed on clone, unknown topology, permission/API failure and state changes |
| Mirror | Real Extend 2 monitors, correct src/dst, fullscreen public frames, private canary absent, no mirror loop, disconnect/reconnect, Stop releases resources |
| Packaging | portable + per-user installer smoke; required DLLs/resources included; clean machine/deps no admin when verified; old DPAPI file survives upgrade/uninstall |
| UI/accessibility | VI/EN full strings, 96/120/144/192 DPI, contrast, keyboard-only navigation, no flicker/overlap, error text actionable |

**Manual hardware QA that cannot be faked:** two real displays in Extend and Duplicate, actual person/viewer on destination, Google Meet/Zoom/Teams capture if claiming compatibility; physically inspect output during F8/lock/source change. If environment lacks two monitors, report NOT TESTED for physical-output claims. Record OS build, GPU, API path, reproduction steps and observed outcome. Security promises are limited to tested capture paths.

## 11. Expected deliverables & closing report

- Native working source + focused modules/tests; updated `build.ps1` or compatible MSVC build instructions; pinned dependencies with licenses; `README.md`, `CHANGELOG.md`, `TEST_REPORT_v0.4.0.md`, `THIRD-PARTY.md`.
- Release artifacts: `dist/QuietShare.exe`, `dist/QuietShare-Setup.exe`, `QuietShare-v0.4.0-win-x64.zip` (actual final paths may differ; print accurate paths). No automatic push, release, install or machine-wide changes.
- Final report **short**: files changed; features working; build commands + actual results; automated tests numbers; manual tests PASS/NOT TESTED; known limitations; exact artifact paths. No simulated screenshots, no fabricated FPS/receiver checks, no deferred “implement later” for core requirements.

**Priority if tradeoffs:** privacy fail-closed > preservation of existing features/data > tabs/code/PDF functionality > focus toolbar polish > Smart Mirror throughput > decorative UI. If a dependent library or platform API is genuinely unavailable, keep buildable components, document blocker and specific reproducible steps instead of falsely claiming release-ready.
