# QuietShare - duyệt web khi cần / on-demand browsing

Trạng thái: nghiên cứu thiết kế, chưa có trình duyệt trong bản 0.4.4. / Status: design research; browsing is not enabled in 0.4.4.

## Quyết định đề xuất / Proposed decision

Dùng Microsoft WebView2 Win32 nhúng vào vùng nội dung được bảo vệ hiện có. Chỉ khởi tạo khi người dùng chọn mở web; không chạy trình duyệt nền khi chỉ đọc lời nhắc, code hoặc ảnh. Một tab web với Back, Forward, Reload, URL và zoom đủ cho lần tích hợp đầu. Không tự mở Edge/Chrome ngoài app vì cửa sổ đó không thuộc phạm vi bảo vệ của QuietShare.

Use a Win32 WebView2 hosted inside the existing protected content area, initialized only on demand. Start with one web tab, Back/Forward/Reload, URL and zoom. An external Edge/Chrome window falls outside QuietShare's protection, so it is not an automatic fallback.

WebView2 hỗ trợ web hiện đại mà không cần tự đóng gói Chromium. Bản production phải dùng WebView2 Runtime; Edge Stable không thay thế runtime. Chọn Evergreen để có cập nhật bảo mật, kiểm tra runtime trước khi mở web và hỗ trợ cài theo tài khoản hiện tại khi thiếu. Phần đọc file vẫn hoạt động offline khi không có runtime. Nguồn: [Microsoft - Runtime distribution](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution).

WebView2 provides a modern web engine with an Evergreen runtime that receives security updates. Detect a missing runtime and offer per-user installation; keep existing offline readers independent of it. Production uses the WebView2 Runtime rather than Edge Stable. Source: [Runtime distribution](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution).

## Giới hạn phạm vi / Scope boundaries

- Chỉ nhận URL HTTP/HTTPS rõ ràng; không tự chạy giao thức ngoài app, file://, script hoặc lệnh hệ thống. Website vẫn có thể giao tiếp Internet theo hành vi của website.
- Hồ sơ WebView2 riêng của QuietShare trong thư mục dữ liệu của tài khoản, không dùng dữ liệu/cookie của trình duyệt cá nhân. Không ghi URL, cookie, nội dung trang hoặc thông tin đăng nhập vào Git, báo cáo kiểm thử hay log phát hành. [Microsoft - User data folders](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/user-data-folder).
- Không cho website gọi host object, đọc dự án/lời nhắc hoặc dùng cầu nối native. Kiểm tra origin và thông điệp nếu sau này thật sự cần cầu nối. [Microsoft - Develop secure WebView2 apps](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/security).
- Tắt menu mặc định, DevTools và hộp thoại script mặc định; mọi UI cần thiết phải đi qua cửa sổ được bảo vệ của app. Chặn quyền camera, microphone, location, capture màn hình, notification và tải xuống trong lần đầu. [Microsoft - WebView2 settings](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2settings).
- Xử lý NewWindowRequested trước khi điều hướng: chặn popup tự phát; link người dùng bấm mở trong vùng web có kiểm soát. Login/OAuth cần popup không được tuyên bố hỗ trợ trước khi kiểm thử riêng. Mặc định của WebView2 có thể tạo popup nếu host không xử lý. [Microsoft - NewWindowRequested](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2newwindowrequestedeventargs).

Accept explicit HTTP/HTTPS URLs, use a dedicated QuietShare profile, expose no native host objects, and control all popup/dialog/permission/download paths. Do not import the user's browser profile or publish web data. Login flows that require popups need separate compatibility and privacy validation.

## Điều kiện bắt buộc trước khi phát hành / Release gate

WDA_EXCLUDEFROMCAPTURE chỉ áp dụng cho top-level HWND thuộc process gọi; không được suy luận rằng mọi cửa sổ renderer/popup đều được bảo vệ chỉ vì HWND chính nhận API thành công. Microsoft cũng không coi API này là biện pháp bảo mật tuyệt đối. [Microsoft - SetWindowDisplayAffinity](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity).

The top-level affinity result alone does not prove exclusion for browser-rendered surfaces or newly created windows. Validate the actual rendered browser output and every UI path before exposing personal pages.

1. Trước URL người dùng, render trang fixture có canary trong WebView2 rồi chạy WGC kiểm tra chính vùng đó. Nếu thấy canary hoặc không kiểm chứng được thì giữ vùng web khóa, không điều hướng tới nội dung riêng.
2. Thử GPU composition, video, fullscreen, select dropdown, tooltip, context menu, alert/confirm, permission, download, upload, popup và crash/restart renderer. Không mở popup hay file chooser của process khác không được bảo vệ.
3. F8 và mất điều kiện bảo vệ phải ẩn controller tức thì, dừng điều hướng đang chờ và giữ khóa tới khi kiểm chứng lại. Callback bất đồng bộ phải kiểm tra generation/lifetime để không phục hồi web sau khi đã che hoặc đóng tab.
4. Thử một màn hình, Extend, Duplicate, đổi GPU/DPI, sleep/resume và người nhận thực trong Zoom/Meet/Teams/OBS. Duplicate tiếp tục khóa như hiện tại.
5. Đo khởi động chỉ đọc file (không có process web), RAM khi mở/đóng web, resize và cuộn. Kiểm thử thiếu runtime, cài mới, nâng cấp và gỡ cài đặt mà không mất lời nhắc.

These are acceptance criteria, not completed tests. No browser-protection, login-compatibility or performance claim has been verified by this study.
