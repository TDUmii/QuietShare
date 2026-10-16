# Changelog

## 0.8.1

- Thu nhỏ bằng nút **-** giữ phiên Smart Mirror đang chiếu. F8, Dừng, đóng ứng dụng và các điều kiện mất an toàn tiếp tục kết thúc chiếu theo chức năng tương ứng.
- Đặt nút **Ghim** tại góc trên để bật/tắt nhanh; đồng bộ với thiết lập ghim trong Cài đặt.
- Làm mịn đường bo và nút tròn trong Cài đặt theo DPI để giảm răng cưa.
- Viết lại hướng dẫn VI/EN theo cấu trúc tài liệu kỹ thuật, bổ sung bảng phím tắt với phạm vi sử dụng. Đặc tả phiên bản và ghi chú phát hành chỉ mô tả chức năng, yêu cầu và giới hạn vận hành.
- Minimizing with **-** keeps an active Smart Mirror projection running. F8, Stop, application closure and unsafe conditions retain their respective projection-stop behavior.
- Moved **Pin** to the upper corner for quick access, synchronized with Settings.
- Smoothed Settings rounding and circular controls with DPI-aware rendering.
- Reorganized VI/EN guidance as technical user documentation with scoped shortcut tables. Version specifications and release notes describe functionality, requirements and operating limits.

## 0.8.0

- Thêm zoom web 25-400% bằng nút thu/phóng, nút phần trăm hiện tại kiêm đặt lại 100%, Ctrl+−/+/0 và Ctrl rồi lăn chuột; tách biệt với zoom của file.
- Bật web trong lúc Smart Mirror đang chiếu giữ khung hình công khai gần nhất trong kiểm tra canary. Đạt thì tiếp tục phiên chiếu; thất bại/hủy vẫn dừng và khóa. Rào thời gian sau khi gỡ canary loại khung hình cũ trước khi chiếu tiếp.
- Thanh cuộn Cài đặt giữ cùng hình học hiển thị/vùng bắt chuột, theo con trỏ khi kéo và có hover nhất quán. Giữ xử lý touchpad mượt và focus tự cuộn.
- Dùng chung icon loại file cho Explorer và tab, gồm các ngôn ngữ code, JSON/XML/Markdown/config, ảnh/PDF và thư mục mở/đóng; image list native được cache theo DPI.
- Thêm **Hiệu suất > Tài nguyên**: Tiết kiệm 15 FPS, Cân bằng 30 FPS, Hiệu năng 60 FPS mặc định. Lưu lựa chọn trên máy và đọc được bố cục cũ; web ẩn vẫn yêu cầu tạm ngưng, không cam kết mức giảm RAM.
- Nhãn màn hình gồm số nhận diện Windows, Chính, Cửa sổ này và độ phân giải; nguồn mặc định là màn chính, đích là màn khác. Nhãn cập nhật theo vị trí cửa sổ/ngôn ngữ, không tự đổi lựa chọn màn hình.
- Added independent 25-400% web zoom, current-percentage/reset control, Ctrl+−/+/0 and Ctrl plus wheel.
- Enabling web during Smart Mirror holds the last public frame through the canary check and resumes the same projection only after success. Failure/cancellation remains fail-closed; a post-teardown timestamp barrier rejects stale frames.
- Unified Settings scrollbar geometry, pointer tracking and hover while retaining proportional touchpad scrolling and focus visibility.
- Shared DPI-cached file-type icons between Explorer and tabs, including language badges, documents/media and open/closed folders.
- Added locally persisted Saver/Balanced/Performance projection caps of 15/30/60 FPS, with Performance as default and backward-compatible layout loading. Hidden-browser suspension remains automatic; no fixed RAM saving is promised.
- Display labels include Windows number, Main, This window and resolution, updating with window location/language while preserving selections; source defaults to primary and destination to another display.

## 0.7.1

- Ô cỡ chữ/zoom đặt dấu nháy ở cuối khi nhận focus hoặc được bấm lần đầu; các lần bấm tiếp theo vẫn cho đặt vị trí/chọn chữ để sửa. Áp dụng giá trị không đưa dấu nháy về đầu.
- Cài đặt cuộn theo độ lớn của từng tín hiệu touchpad, giữ phần lẻ và đổi chiều ngay; cập nhật vị trí các hàng theo lô thay vì dựng lại toàn bộ workspace trong mỗi bước.
- Thanh tab xử lý cuộn dọc/ngang theo delta, giữ phần lẻ riêng và không đo lại độ rộng tên tab mỗi lần cuộn.
- Text size/zoom places the caret at the end on focus or the first click; subsequent clicks retain native editing/selection. Applying a value preserves the end position.
- Settings consumes high-resolution wheel deltas proportionally, preserves fractional motion and batches row movement without rebuilding the workspace on each packet.
- The tab strip scales vertical/horizontal wheel movement by delta, with separate fractional remainders and no repeated tab text measurement while scrolling.

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
- Actual browser-rendered positive/excluded WGC canary before navigating to user pages; native black backing cannot satisfy browser capture verification.
- F8 masks the browser and address; topology changes close browsing and require a fresh check. Pending initialization callbacks cannot revive a disabled browser.
- Controlled navigation/popups and disabled uploads/downloads, device/screen permissions, native script dialogs/context menus, autofill, password-save UI and DevTools.
- Bundled only the pinned 195 KB WebView2 loader; Evergreen Runtime remains a separate shared prerequisite. Existing offline readers do not load WebView2 when browsing is off.

## 0.4.4

- Narrower 44 px navigation rail, 18 px navigation icons and a centered 24 px logo.
- Project Explorer remains beside content from 740 px client width; opening or switching files no longer hides it at medium window sizes.
- Project Explorer adapts between 176 and 240 px and includes native hierarchy guides. Narrow windows use a full-width tree until a file is opened.
- Ctrl+B toggles the project tree.
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

## 0.4.0

- Added read-only file tabs, multi-select/multi-drop opening, canonical-path deduplication, explicit reload for changed files, tab shortcuts and per-tab view state. Script and DPAPI autosave remain separate from previews.
- Added a lazy local project tree with configurable hidden/heavy-folder filtering, bounded enumeration and junction/symlink avoidance.
- Added native Scintilla/Lexilla code viewing with token styles, line numbers, selection/copy, find and wrapping.
- Added native PDFium viewing with on-demand pages, zoom, fit-page/fit-width, navigation and text selection/copy/find when available. Password-protected PDFs return a clear unsupported error.
- Replaced the focus corner F11 button with a slim common tab/command strip, overflow actions and tab scrolling; refined dark tokens and VI/EN copy.
- Added Smart Mirror with Windows Graphics Capture, D3D11, synthetic canary preflight, explicit Start/Stop and fail-closed transitions. Duplicate stays locked; projection requires two Extend displays on the same GPU.
- Bundled native viewer libraries and redistribution notices in the per-user installer; added replacement/rollback, exact-file uninstall, VI/EN UI.

## 0.3.0

- Added content preparation/settings pages, local text/code/image previews and F11 focus.
- Preserved scripts during previews; added image zoom/pan and a persistent dark scrollbar.
- Updated bilingual installation/use guidance and native UI examples.

## 0.2.0

- Released the native x64 bilingual teleprompter with DPAPI autosave, reading/auto-scroll, F8 hide, pinning and per-user installation.
- Refined typography, spacing, dark controls, empty/masked states and resize layouts.
