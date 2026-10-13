# Changelog

## 0.4.0

- Added read-only file tabs, multi-select/multi-drop opening, canonical-path deduplication, explicit reload for changed files, tab shortcuts and per-tab view state. Script and DPAPI autosave remain separate from previews.
- Added a lazy local project tree with configurable hidden/heavy-folder filtering, bounded enumeration and junction/symlink avoidance.
- Added native Scintilla/Lexilla code viewing with token styles, line numbers, selection/copy, find and wrapping.
- Added native PDFium viewing with on-demand pages, zoom, fit-page/fit-width, navigation and text selection/copy/find when available. Password-protected PDFs return a clear unsupported error.
- Replaced the focus corner F11 button with a slim common tab/command strip, overflow actions and tab scrolling; refined dark tokens and VI/EN copy.
- Added Smart Mirror with Windows Graphics Capture, D3D11, synthetic canary preflight, explicit Start/Stop and fail-closed transitions. Duplicate stays locked; projection requires two Extend displays on the same GPU.
- Bundled native viewer libraries and redistribution notices in the per-user installer; added replacement/rollback, exact-file uninstall, VI/EN UI and isolated install/upgrade/uninstall regression checks.

## 0.3.0

- Added content preparation/settings pages, local text/code/image previews and F11 focus.
- Preserved scripts during previews; added image zoom/pan and a persistent dark scrollbar.
- Updated bilingual installation/use guidance and native UI examples.

## 0.2.0

- Released the native x64 bilingual teleprompter with DPAPI autosave, reading/auto-scroll, F8 hide, pinning and per-user installation.
- Refined typography, spacing, dark controls, empty/masked states and resize layouts.
