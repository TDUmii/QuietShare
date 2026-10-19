# Changelog / Lịch sử cập nhật

English summaries appear first, followed by Vietnamese. Entries describe each release at the time it shipped; see the current user guide for current behavior.

Tóm tắt tiếng Anh đứng trước, tiếng Việt ngay bên dưới. Mỗi mục mô tả phiên bản tại thời điểm phát hành; xem hướng dẫn sử dụng để tra hành vi hiện tại.

## v0.14.1 - 2026-10-19

Shorter English-first README with Vietnamese below each section. Added focused user, privacy, troubleshooting, build and dependency guides; portable documentation now excludes historical implementation plans.

Rút gọn README với tiếng Anh trước, tiếng Việt dưới mỗi phần. Thêm hướng dẫn sử dụng, quyền riêng tư, sự cố, biên dịch và thư viện; tài liệu portable bỏ các kế hoạch hiện thực cũ.

## v0.14.0 - 2026-10-18

Added 10 persistent light, dark and high-contrast themes, selectable in Settings and the viewer toolbar. Theme changes preserve drafts and reading positions; image, PDF and website colors stay unchanged.

Thêm 10 theme sáng, tối và tương phản cao, lưu lựa chọn và đổi tại Cài đặt hoặc thanh xem. Giữ bản sửa, vị trí đọc và màu gốc ảnh, PDF, website.

## v0.13.1

Fixed reading-position jumps in long scripts/text when changing size, weight, wrapping or resetting formatting. Selection and content are preserved.

Sửa nhảy vị trí ở lời nhắc/văn bản dài khi đổi cỡ, độ đậm, xuống dòng hoặc đặt lại. Giữ vùng chọn và nội dung.

## v0.13.0

Added explicit raster-image paste into compatible web editors and in-viewer text size, weight and wrap controls. Narrow toolbars reflow; automatic clipboard reads and native web file pickers remain blocked.

Thêm dán ảnh raster chủ động vào ô nhập web tương thích và chỉnh cỡ, độ đậm, xuống dòng ngay trên thanh xem. Thanh hẹp tự chia hàng; vẫn chặn tự đọc clipboard và hộp chọn file web native.

## v0.12.2

Repaired source/destination selection after Duplicate returns to Extend. Improved font rendering, compact Home/sidebar layout and consistent pin colors; protected file switching is paginated and closes on locking.

Sửa chọn nguồn/đích khi Duplicate trở về Extend. Cải thiện chữ, Home/thanh bên nhỏ và màu ghim; menu chuyển file chia trang, đóng khi khóa.

## v0.12.1

Changed default content zoom to 50% and text/script size to 12 pt. Reset and Ctrl+0 restore these defaults; per-tab zoom and explicit PDF fitting remain available.

Đổi zoom nội dung mặc định thành 50%, văn bản/lời nhắc 12 pt. Đặt lại và Ctrl+0 dùng cùng mặc định; giữ zoom riêng từng tab và vừa khung PDF chủ động.

## v0.12.0

Removed decorative file branding and the permanent script tab. Returning from Home, browser or Settings preserves the active file, draft and viewport. Improved compact Home and capture-status icons.

Bỏ logo file trang trí và tab lời nhắc cố định. Trở lại từ Home, trình duyệt hoặc Cài đặt giữ file, bản sửa và vị trí. Cải thiện Home nhỏ và icon trạng thái capture.

## v0.11.1

Added an Explorer-style protected local picker with common folders, drives, address navigation, modification dates and file sizes.

Thêm hộp chọn cục bộ được bảo vệ theo bố cục Explorer, có thư mục thường dùng, ổ đĩa, địa chỉ, ngày sửa và dung lượng.

## v0.11.0

Added real recent-file history with pin/remove/clear, optional window-size memory and explicit text/code saving with encoding and external-change protection. Separated Windows exclusion acceptance from receiver verification.

Thêm lịch sử file thật với ghim/xóa, tùy chọn nhớ kích thước và lưu văn bản/code, giữ mã hóa và chống ghi đè thay đổi bên ngoài. Tách Windows nhận loại trừ khỏi xác minh đầu nhận.

## v0.10.1

Refined numeric/address inputs, icon sizing and repainting. Added browser mute and opt-in camera/microphone settings with visible, verified, same-origin HTTPS restrictions.

Chỉnh ô số/địa chỉ, kích thước icon và repaint. Thêm tắt âm thanh và tùy chọn camera/micro, giới hạn ở tab HTTPS đang hiện, đã xác minh và cùng origin.

## v0.10.0

Added a local New tab page with saved shortcuts, categorized Settings, window opacity, script weight and customizable shortcuts. Moved projection controls/status into the sidebar and Settings with explicit stop diagnostics.

Thêm Tab mới cục bộ với lối tắt, Cài đặt theo nhóm, độ mờ, độ đậm lời nhắc và phím tắt tùy chỉnh. Đưa điều khiển/trạng thái chiếu vào thanh bên và Cài đặt, có chẩn đoán dừng.

## v0.9.2

Kept the controller inside the verified source work area during moves, resizing and restore. Source-containment failures retain the existing safe-stop behavior.

Giữ cửa sổ điều khiển trong vùng nguồn đã xác minh khi kéo, đổi kích thước hoặc khôi phục. Lỗi vùng nguồn vẫn dừng theo cơ chế bảo vệ.

## v0.9.1

Fixed the five-minute projection stop, retained runtime protection checks and requested Windows wakefulness while projecting. Added non-focus-stealing interruption feedback; intentional Stop/F8 does not raise an alarm.

Sửa tự dừng chiếu sau năm phút, giữ kiểm tra trong phiên và yêu cầu Windows duy trì hoạt động khi chiếu. Thêm cảnh báo gián đoạn không giành focus; Dừng/F8 chủ động không báo lỗi.

## v0.9.0

Added up to 8 verified browser tabs with independent pages, history and zoom, shared sign-in state and keyboard tab controls. User-opened new-window links become tabs; unsolicited popups remain blocked.

Thêm tối đa 8 tab web được xác minh, trang/lịch sử/zoom riêng, dùng chung đăng nhập và điều khiển tab bằng phím. Link mở cửa sổ do người dùng bấm thành tab; popup tự phát vẫn bị chặn.

## v0.8.1

Kept active projection running when the controller is minimized. Added quick pin access, synchronized with Settings, and refined DPI-aware controls and bilingual guidance.

Giữ phiên chiếu khi thu nhỏ cửa sổ điều khiển. Thêm ghim nhanh đồng bộ Cài đặt, chỉnh control theo DPI và hướng dẫn song ngữ.

## v0.8.0

Added independent 25-400% web zoom, safe browser enablement during projection, shared file-type icons and saved 15/30/60 fps resource presets. Refined Settings scrolling and display-role labels.

Thêm zoom web riêng 25-400%, bật web có bảo vệ trong lúc chiếu, icon loại file dùng chung và lưu chế độ tài nguyên 15/30/60 fps. Chỉnh cuộn Cài đặt và nhãn vai trò màn hình.

## v0.7.1

Improved numeric-input caret behavior and proportional high-resolution scrolling in Settings and tab strips.

Cải thiện con trỏ ô số và cuộn theo độ lớn tín hiệu độ phân giải cao trong Cài đặt và thanh tab.

## v0.7.0

Centralized configuration in scrollable Settings groups and replaced the text-size slider with precise numeric input. Kept capture verification mandatory when enabling browsing.

Gom cấu hình vào các nhóm Cài đặt có cuộn, thay slider cỡ chữ bằng ô số chính xác. Vẫn bắt buộc xác minh capture khi bật web.

## v0.6.0

Preserved Explorer branches/selection across file changes, added saved layout controls and flexible address/search input, and refined native surface rendering.

Giữ nhánh/lựa chọn Explorer khi đổi file, thêm lưu bố cục và nhập địa chỉ/tìm kiếm linh hoạt, cải thiện cách vẽ giao diện native.

## v0.5.0

Introduced the optional on-demand WebView2 browser with a separate local profile, real browser capture preflight and restrictions on unsafe native surfaces.

Ra mắt trình duyệt WebView2 tùy chọn, hồ sơ cục bộ riêng, kiểm tra capture vùng web thật và hạn chế vùng native không an toàn.

## v0.4.4

Refined the navigation rail and responsive Explorer, including Ctrl+B and keyboard folder/file navigation. Browser integration remained a proposal at this version.

Chỉnh thanh điều hướng và Explorer thích ứng, gồm Ctrl+B và điều hướng thư mục/file bằng phím. Ở bản này, tích hợp trình duyệt vẫn là đề xuất.

## v0.4.3

Replaced shell dialogs with a capture-protected local picker and improved tool menus, numeric size/zoom controls and installer payload handling.

Thay hộp thoại shell bằng hộp chọn cục bộ có bảo vệ capture; cải thiện menu, ô cỡ chữ/zoom và xử lý payload bộ cài.

## v0.4.2

Added global Ctrl+Alt+Q window recall while retaining focus, masking, pin state and the privacy gate.

Thêm Ctrl+Alt+Q gọi lại cửa sổ toàn hệ thống, giữ focus, trạng thái che, ghim và điều kiện bảo vệ.

## v0.4.1

Introduced Codicons, protected native tool menus, bilingual tooltips and faster capture/presentation scheduling.

Thêm Codicons, menu native được bảo vệ, tooltip song ngữ và cải thiện lịch capture/trình chiếu.

## v0.4.0

Added file tabs, local project browsing, Scintilla/Lexilla code and PDFium viewers, and Smart Mirror with capture preflight and explicit start/stop. Bundled viewer libraries in the per-user installer.

Thêm tab file, duyệt dự án cục bộ, xem code Scintilla/Lexilla và PDFium, cùng Smart Mirror có kiểm tra trước và bắt đầu/dừng chủ động. Bộ cài kèm thư viện xem nội dung.

## v0.3.0

Added preparation/settings pages, local file previews and focus mode while preserving scripts during previews.

Thêm trang chuẩn bị/cài đặt, xem file cục bộ và chế độ tập trung, giữ lời nhắc khi xem file.

## v0.2.0

Released the native x64 bilingual teleprompter with DPAPI autosave, auto-scroll, F8 masking, pinning and per-user installation.

Phát hành trình đọc lời nhắc native x64 song ngữ, tự lưu DPAPI, cuộn tự động, F8 che nội dung, ghim và cài theo tài khoản.
