# Changelog

## 0.7.0

- Gom cấu hình vào Cài đặt với các nhóm Hiển thị, Đọc nội dung, Bố cục, Tiện ích và Trình chiếu; form phẳng căn giữa có chiều rộng tối đa 760 px, thanh cuộn tối và tự cuộn theo focus.
- Bỏ hoàn toàn slider cỡ chữ; giữ ô nhập, đơn vị pt/% và nút Đặt lại, với phạm vi/lỗi ngay trong nhóm Đọc nội dung. Các nút zoom nhanh vẫn có trong toolbar.
- Chuyển chọn màn hình nguồn/đích và con trỏ về Cài đặt. Trang Trình chiếu hiển thị tóm tắt màn hình, lối tắt Thiết lập màn hình và các lệnh Kiểm tra WGC/Bắt đầu/Dừng.
- Bật trình duyệt từ Cài đặt không tự chuyển trang; kiểm tra vùng web vẫn bắt buộc. Giữ menu Bố cục và Ghim để thao tác nhanh, đưa các cấu hình khác khỏi menu công cụ và thêm lối tắt Cài đặt.
- Centralized configuration in Appearance, Reading, Workspace, Tools and Presentation groups; a flat centered form up to 760 px wide includes a dark scrollbar and automatic scrolling to focused controls.
- Removed the text-size slider completely. Retained manual input, pt/% units, Reset and inline range/errors; quick zoom controls remain in the toolbar.
- Moved source/destination display selection and pointer configuration to Settings. Presentation shows the display summary, a Display settings shortcut and Verify WGC/Start/Stop actions.
- Enabling browsing from Settings preserves the current page and the mandatory browser capture check. Layout and Pin remain quick controls; other configuration entries leave the tools menu, which gains a Settings shortcut.

## 0.6.0

- Explorer giữ nguyên thư mục, các nhánh mở và lựa chọn khi mở file, đổi tab hoặc resize; cây và nội dung vẫn hiện cạnh nhau trong cửa sổ hẹp. Kéo đường phân cách để chỉnh độ rộng, dùng Ctrl+B hoặc nút đóng trên tiêu đề để ẩn/hiện.
- Nút Bố cục bật/tắt riêng thanh icon, Explorer, tab, công cụ và trạng thái. Các lựa chọn và độ rộng được lưu cục bộ; có lệnh khôi phục mặc định. F11 giữ các nút cần thiết, ẩn tab và khôi phục bố cục khi thoát.
- Ô địa chỉ nhận URL HTTP/HTTPS, tên miền, localhost có cổng và từ khóa tìm Google, bao gồm tiếng Việt. Thêm nút Google, giữ nút ChatGPT và mở Google mặc định khi bật trình duyệt.
- Bo góc nhẹ các vùng nội dung, căn chỉnh các thanh nhất quán và giảm repaint khi hover/resize để giao diện ổn định hơn.
- Explorer preserves the folder, expanded branches and selection when opening files, switching tabs or resizing; the tree and content remain side by side in narrow windows. Drag the divider to resize it; use Ctrl+B or the header close button to hide/show it.
- Layout independently toggles the activity bar, Explorer, tabs, toolbar and status bar. Choices and width are saved locally, with a reset command. F11 keeps essential actions, hides tabs and restores the chosen layout on exit.
- Address entry accepts HTTP/HTTPS URLs, domains, localhost with a port, and Google search terms including Vietnamese. Added a Google shortcut alongside ChatGPT and Google as the default browser destination.
- Softly rounded content surfaces, consistent bar alignment and fewer unnecessary hover/resize repaints improve the desktop interface.

## 0.5.0

- Optional on-demand WebView2 browser with URL entry, Back/Forward/Reload and ChatGPT shortcut; disabled at every app startup.
- Separate local web profile, explicit runtime teardown on disable, and background suspension requests while reading files.
- Actual browser-rendered positive/excluded WGC canary before navigating to user pages; native black backing cannot satisfy the browser marker test.
- F8 masks the browser and address; topology changes close browsing and require a fresh check. Pending initialization callbacks cannot revive a disabled browser.
- Controlled navigation/popups and disabled uploads/downloads, device/screen permissions, native script dialogs/context menus, autofill, password-save UI and DevTools.
- Bundled only the pinned 195 KB WebView2 loader; Evergreen Runtime remains a separate shared prerequisite. Existing offline readers do not load WebView2 when browsing is off.

## 0.4.4

- Narrower 44 px navigation rail, 18 px navigation icons and a centered 24 px logo.
- Project Explorer remains beside content from 740 px client width; opening or switching files no longer hides it at medium window sizes.
- Project Explorer adapts between 176 and 240 px and includes native hierarchy guides. Narrow windows use a full-width tree until a file is opened.
- Ctrl+B toggles the project tree. Folder expansion, tab switching, resizing and protected folder selection are covered by native regression checks.
- Fixed Enter being intercepted by dialog navigation before the project tree could open the selected file; Enter also toggles the selected directory.
- Added a sourced WebView2 integration study; browser support is a proposal and is not enabled in this release.

## 0.4.3

- Replaced shell file/project dialogs with a capture-protected native local picker, bounded folder navigation, multi-file selection and no Explorer auxiliary surfaces.
- Unrelated device and battery notifications no longer close a picker or invalidate capture; real display, suspend and privacy failures retain fail-closed behavior.
- Buffered tool menu painting and stopped repainting when mouse movement stays within the same row.
- Added exact numeric text-size/zoom input in Settings. Script/text size supports 8-200 pt; code/images/PDF retain 25-400% zoom.
- Slider fill and numeric preview update during dragging without resetting its range/position. Expensive PDF rendering occurs on release.
- Installer payloads are embedded with Windows UpdateResource after linking to avoid large-payload windres heap failures.

## 0.4.2

- Added global Ctrl+Alt+Q to recall a covered, hidden or minimized window without opening Alt+Tab, stealing foreground focus or changing the pin setting.
- Recall preserves F8 masking and checks the existing capture/privacy gate before displaying private readers or metadata. It never overrides Duplicate restrictions.
- The pin tooltip describes the shortcut and reports an unavailable registration if another application occupies it.

## 0.4.1

- Replaced navigation and tool action sketches with bundled Microsoft Codicons; icon-only navigation now reveals bilingual names after an 850 ms hover.
- Refined the native shell and per-user installer with VS Code-inspired neutral surfaces, blue actions and shorter installer copy.
- Replaced the system tools menu with a capture-protected native popup, keyboard navigation, aligned shortcut labels and contextual PDF/scroll commands. Opening or dismissing it no longer relies on native-menu exceptions.
- Raised capture polling from 33 ms to 8 ms, presents the newest queued WGC frame and limits presentation status repainting to four times per second. Actual FPS depends on display refresh, GPU and source activity.
- Added real WGC regressions for tools popup protection and verification continuity on open/close. Physical two-monitor throughput and receiver-side checks remain untested on this one-monitor host.

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
