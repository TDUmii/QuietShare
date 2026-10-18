# Changelog

## v0.13.0

- Cho phép Ctrl+V dán ảnh raster vào ô nhập web; giữ chặn clipboard tự đọc, hộp chọn file native và file drop.
- Thêm ô cỡ chữ, độ đậm và xuống dòng trực tiếp trên thanh xem văn bản/lời nhắc; thêm nút xuống dòng cho code. Thanh công cụ tự chia hàng khi cửa sổ hẹp.
- Xuống dòng văn bản giữ riêng theo tệp trong phiên, không thay nội dung file. Bỏ nhãn tự lưu lặp ở chân trang lời nhắc.

## v0.12.2

- Khôi phục lựa chọn hai màn khác nhau sau Duplicate → Extend; kiểm tra lại danh sách màn trước WGC. Vẫn cần xác minh và bắt đầu lại chủ động.
- Chữ giao diện dùng Segoe UI với ClearType; văn bản dùng Segoe UI; code dùng DirectWrite và chất lượng chữ LCD, giữ zoom mặc định 50%.
- Home dùng tiêu đề ngắn Trang chủ; thanh bên phân nhóm và có menu chuyển tệp qua nhấp phải. Màu ghim đồng nhất xanh lá/xám.
- Menu chuyển tệp chia trang và đóng ngay khi che/khóa nội dung, không gọi cửa sổ điều khiển lên lại do lệnh đóng an toàn.

## Unreleased

## 0.12.1

- Đặt mặc định nội dung web/tab mới, code, ảnh và PDF ở 50%; văn bản và lời nhắc ở 12 pt (50% cỡ chuẩn 24 pt). Nút đặt lại và Ctrl+0 trở về cùng mức mặc định; giữ zoom tự chọn riêng theo tab và thao tác PDF vừa khung chủ động.

## 0.12.0

- Bỏ logo tệp trang trí và tab lời nhắc cố định. Nút Tệp đang mở trở lại đúng file, giữ bản sửa, vị trí và zoom khi đi qua Home, trình duyệt hay cài đặt. Soạn lời nhắc mở chủ động từ menu; giữ bản đã lưu, không tạo lời nhắc mẫu cho hồ sơ mới.
- Home dùng bố cục tối và màu xanh lá sát ảnh tham chiếu; bố trí riêng cho cửa sổ nhỏ, tránh icon và chữ chồng lên nhau, vẫn có hàng lịch sử ở kích thước tối thiểu.
- Dòng Windows nhận loại trừ capture dùng icon xác nhận xanh; trạng thái người xem chưa xác minh giữ riêng. Thay ký tự chuyển trang và nét chevron tự vẽ bằng icon Codicons.

## 0.11.1

- Bộ chọn file/dự án có bố cục quen thuộc của File Explorer: truy cập nhanh Desktop, Downloads, Documents, Pictures và ổ đĩa; thanh địa chỉ, quay lại/tiến tới/lên trên, ngày sửa đổi và kích thước file. Giữ bảo vệ capture và kiểm tra đường dẫn cục bộ hiện có.

## 0.11.0

- Home gọn với điểm nhấn xanh lá, thao tác Mở file / Mở dự án, lịch sử thật, ghim và xóa từng mục hoặc toàn bộ lịch sử. Dữ liệu lịch sử được Windows DPAPI mã hóa cục bộ.
- Thêm nhớ kích thước cửa sổ và cho phép sửa file, cùng mặc định bật. Văn bản/code lưu bằng Lưu hoặc Ctrl+S, giữ mã hóa và chặn ghi đè khi file đổi bên ngoài. Ảnh/PDF giữ chế độ xem.
- Giảm công việc kiểm tra khi chưa trình chiếu và tránh resize viewer trùng kích thước. Các điều kiện bảo vệ và nhịp trình chiếu được giữ nguyên.
- Home separates Windows capture-exclusion acceptance from the unverified physical receiver status. Recent entries stay concealed behind the privacy gate.

## 0.10.1

- Căn giữa chữ và con trỏ trong các ô số/URL; thêm cụm giảm, nhập số và tăng cho độ mờ, cỡ chữ/zoom và kích thước icon.
- Giữ chấm trạng thái trọn vẹn trong thanh bên; vẽ đệm các nút và gộp repaint khi chuyển trang/tab để tránh nền trắng trung gian.
- Cho chỉnh icon 12-24 px trên thanh điều khiển, tab và Explorer; giữ cây thư mục và file đang chọn khi thay đổi.
- Thêm tab Cài đặt > Trình duyệt với tắt âm thanh và quyền camera/micro mặc định tắt. Quyền chỉ được cấp cho tab HTTPS đang hiển thị, đã xác minh capture và cùng origin; không lưu quyền vào hồ sơ web. Thu hồi quyền có thể tải lại tab liên quan để dừng luồng thiết bị.
- Đưa các nút zoom web ra thanh điều hướng; giữ chọn toàn bộ URL khi bấm vào ô địa chỉ.
- Center text and carets in numeric/address inputs; add decrement, editable value and increment controls for opacity, content size/zoom and icon size.
- Keep the sidebar status dot fully visible; buffer button drawing and consolidate page/tab repainting to avoid intermediate white backgrounds.
- Make toolbar, tab and Explorer icons configurable from 12-24 px while retaining project branches and the selected file.
- Add Settings > Browser with audio mute and camera/microphone access off by default. Grants require a visible, capture-verified, same-origin HTTPS tab and are not saved to the web profile. Revoking access may reload the affected tab to stop device streams.
- Expose web zoom directly on the navigation row while preserving click-to-select-all in the address field.

## 0.10.0

- Thay Google/ChatGPT cố định bằng trang Tab mới cục bộ có ô tìm kiếm/địa chỉ và tối đa 10 lối tắt tự thêm, sửa, xóa; lưu cùng hồ sơ web trên máy.
- Căn giữa ô địa chỉ, bấm để chọn toàn bộ URL; gom thao tác trình duyệt phụ vào menu và giữ 8 tab với lịch sử/zoom riêng.
- Chia Cài đặt thành Giao diện, Nội dung, Bố cục, Trình chiếu và Phím tắt; thêm độ mờ toàn cửa sổ 40-100% cùng độ đậm chữ lời nhắc 400/600/700. Độ mờ áp dụng cả chữ; hình chiếu công khai vẫn đục.
- Cho tùy chỉnh mười phím tắt, giữ hai tác vụ che/gọi cửa sổ toàn hệ thống; từ chối tổ hợp trùng/dành riêng và giữ thiết lập cũ khi đăng ký thất bại.
- Tích hợp kiểm tra WGC và bắt đầu/dừng chiếu trên thanh bên; chuyển thiết lập/trạng thái vào Cài đặt > Trình chiếu, thay trang riêng. Dựng icon mịn theo DPI.
- Kiểm tra WGC trong phiên đang chạy và thông báo màn hình không đổi điều kiện thực tế giữ phiên hợp lệ. Lỗi affinity, topology, capture, DWM hoặc thiết bị vẫn dừng an toàn, không tự khởi động lại; mã lý do và một bản ghi cục bộ giúp nhận biết gián đoạn mà không lưu nội dung riêng.
- Replaced fixed Google/ChatGPT buttons with a local New tab page, search/address input and up to 10 editable, locally saved shortcuts.
- Centered the address field with click-to-select-all; grouped secondary browser actions in a menu while retaining 8 tabs with independent history/zoom.
- Organized Settings into Appearance, Reading, Workspace, Projection and Shortcuts; added uniform 40-100% window opacity and 400/600/700 script weight. Opacity includes text; public projection remains opaque.
- Added ten configurable shortcuts, retaining global mask/recall; reject duplicate/reserved bindings and preserve old registrations if a change fails.
- Added WGC check and start/stop to the activity bar, moving configuration/status into Settings > Projection and replacing the separate page. Smoothed icon rendering at the active DPI.
- Active-session WGC checks and display notifications without actual invariant changes retain healthy projection. Affinity, topology, capture, DWM and device failures still stop safely without automatic restart; reason codes and one local record identify interruptions without saving private content.

## 0.9.2

- Giữ toàn bộ cửa sổ điều khiển trong vùng làm việc của màn hình nguồn đã xác minh khi đang chiếu, kể cả khi kéo sát mép, đổi kích thước, phóng to hoặc khôi phục cửa sổ.
- Giữ các kiểm tra quyền riêng tư, capture, cấu hình màn hình và thiết bị. Lỗi an toàn vẫn dừng chiếu và yêu cầu người dùng chủ động bắt đầu lại.
- Thu nhỏ thanh bên, icon và nút điều khiển để dành thêm diện tích cho nội dung trong cửa sổ nhỏ.
- Thu gọn trang trình duyệt: tab và nút cửa sổ dùng chung hàng trên, điều hướng và địa chỉ dùng chung một hàng; thanh Google/ChatGPT/zoom có thể ẩn tại **Bố cục > Thanh công cụ**.
- Đồng bộ phiên bản trên Tổng quan, ứng dụng, bộ cài và hướng dẫn VI/EN thành **v0.9.2**.
- Keep the entire control window inside the verified source display's work area during projection, including edge dragging, resizing, maximizing and restoring.
- Retain privacy, capture, display-topology and device checks. Safety failures still stop projection and require an explicit restart by the user.
- Reduce sidebar, icon and control sizes to give content more room in small windows.
- Compact the browser page: tabs share the top row with window controls, navigation shares a row with the address field, and **Layout > Toolbar** can hide the Google/ChatGPT/zoom row.
- Align Overview, application, installer and VI/EN guidance with **v0.9.2**.

## 0.9.1

- Sửa Smart Mirror tự dừng sau khoảng 5 phút do giới hạn thời gian xác minh cố định. Phiên Extend tiếp tục khi capture, cấu hình màn hình và điều kiện bảo vệ còn hợp lệ.
- Giữ kiểm tra bảo vệ trong lúc chạy; thay đổi màn hình, suspend và lỗi capture vẫn dừng chiếu, không tự khởi động lại đầu chiếu.
- Yêu cầu Windows giữ máy/màn hình hoạt động khi đang chiếu để tránh ngủ do không có thao tác; dừng chiếu sẽ khôi phục yêu cầu trước đó. Lệnh ngủ chủ động và đóng nắp máy vẫn theo Windows.
- Báo gián đoạn qua tiêu đề cửa sổ, thanh trạng thái màu hổ phách, nháy taskbar và một yêu cầu âm báo Windows; không giành focus hoặc tự khôi phục cửa sổ. Thao tác Dừng hoặc F8 chủ động không tạo cảnh báo.
- Cập nhật phiên bản trên Tổng quan, bộ cài và hướng dẫn VI/EN lên **v0.9.1**.
- Fixed Smart Mirror stopping after about five minutes because of a fixed verification lifetime. Extend sessions continue while capture, display configuration and protection conditions remain valid.
- Retained runtime protection checks. Display changes, suspend and capture failures still stop projection without automatically restarting it.
- Request Windows system/display wakefulness during projection to prevent idle sleep; stopping restores the previous thread requirements. Intentional sleep and lid closure retain Windows behavior.
- Indicate interruptions through the window title, amber status bar, taskbar flash and one requested Windows warning sound without taking focus or restoring the window. Intentional Stop and F8 actions do not trigger the warning.
- Updated the Overview label, installer and VI/EN guidance to **v0.9.1**.

## 0.9.0

- Trình duyệt hỗ trợ tối đa 8 tab với trang, lịch sử điều hướng và mức zoom riêng; các tab dùng chung hồ sơ web cục bộ. Thanh địa chỉ và các nút điều hướng áp dụng cho tab đang chọn.
- Thêm điều khiển mở, đóng và chuyển tab web cùng Ctrl+T, Ctrl+W, Ctrl+Tab và Ctrl+Shift+Tab trong trang trình duyệt. Tab mới mở Google sau khi hoàn tất xác minh vùng web; tab nền nhận yêu cầu tạm ngưng để giảm công việc xử lý khi không sử dụng. Đóng tab cuối cùng sẽ tắt trình duyệt và giữ QuietShare đang mở.
- Link mở cửa sổ mới do người dùng bấm được chuyển vào tab QuietShare trong giới hạn 8 tab; popup tự phát vẫn bị chặn. Trình duyệt vẫn tắt sau mỗi lần khởi động ứng dụng và không khôi phục danh sách tab của phiên trước.
- Hiển thị phiên bản **v0.9.0** ở góc trên trang Tổng quan.
- The browser supports up to 8 tabs with independent pages, navigation histories and zoom levels, sharing the same local web profile. The address field and navigation controls apply to the selected tab.
- Added controls to create, close and switch web tabs, with Ctrl+T, Ctrl+W, Ctrl+Tab and Ctrl+Shift+Tab on the browser page. New tabs open Google after web capture verification; background tabs request suspension to reduce inactive processing. Closing the last tab disables browsing while keeping QuietShare open.
- User-initiated new-window links open in QuietShare tabs within the 8-tab limit; unsolicited popups remain blocked. Browsing starts disabled on every application launch, and previous session tab lists are not restored.
- Displayed **v0.9.0** in the upper corner of Overview.

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
