# QuietShare - duyệt web tùy chọn / optional browsing

Trạng thái: triển khai từ 0.5.0; tìm kiếm từ 0.6.0, zoom và tiếp tục chiếu từ 0.8.0, đa tab từ 0.9.0. / Status: introduced in 0.5.0; search since 0.6.0, zoom and projection continuity since 0.8.0, multiple tabs since 0.9.0.

## Thiết kế / Design

Tối đa tám tab WebView2 Win32 trong vùng nội dung của QuietShare, dùng chế độ Window to Visual. Mặc định tắt ở mỗi lần khởi động; chỉ tạo environment/controller khi người dùng bật. Mỗi tab giữ trang, lịch sử và zoom riêng, dùng chung environment và hồ sơ đăng nhập. Có ô địa chỉ/tìm kiếm, Back, Forward, Reload và nút Google/ChatGPT khi đủ chiều rộng. Địa chỉ HTTP/HTTPS mở trực tiếp; từ khóa được mã hóa UTF-8 để tìm Google. Bộ xử lý từ khóa chỉ áp dụng cho nội dung người dùng nhập; navigation và popup vẫn kiểm tra URL HTTP/HTTPS chặt chẽ. Không tự mở trình duyệt ngoài app cho URL người dùng nhập.

Up to eight on-demand Win32 WebView2 tabs in QuietShare's content area, using Window to Visual hosting. They share one environment/profile while retaining separate pages, history and zoom. Browsing starts disabled, with URL/search entry, Back/Forward/Reload and Google/ChatGPT shortcuts at wider layouts. User queries become UTF-8 encoded Google searches; navigation and popup handlers retain strict HTTP/HTTPS validation. [Microsoft - Hosting modes](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/windowed-vs-visual-hosting).

Chỉ đóng gói loader x64 194.912 byte, pin SHA-256 và SDK 1.0.4258.31. Runtime Evergreen dùng bản cài riêng trên Windows. Nếu thiếu hoặc quá cũ, vùng web báo không khả dụng; phần đọc file vẫn hoạt động. README có đường dẫn cài/cập nhật runtime chính thức.

The package includes only the 194,912-byte pinned loader. Browsing requires a separate current Evergreen Runtime; offline readers remain independent. [Microsoft - Runtime distribution](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution).

## Tài nguyên và dữ liệu / Resources and data

- Tắt browser đóng controller, thả environment và các COM object.
- Đọc file hoặc thu nhỏ sẽ ẩn browser và yêu cầu TrySuspend; trở lại browser sẽ Resume. Không vẽ lại bounds không đổi.
- Hồ sơ riêng trong thư mục dữ liệu QuietShare của tài khoản. Cookie được giữ khi tắt/bật để đăng nhập thuận tiện; không dùng hồ sơ Edge cá nhân. Khởi động app vẫn tắt browser dù đã có hồ sơ web.
- URL, nội dung trang, cookie, thông tin đăng nhập và dữ liệu profile được giữ ngoài Git, tài liệu và gói phát hành.

Disabling closes the controller, environment and web objects. Hidden browsing requests suspension and resumes on return. A separate profile preserves cookies while every app startup remains opt-in. [Microsoft - TrySuspend](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2_3), [User data folders](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/user-data-folder).

## Bảo vệ và giới hạn / Protection and boundaries

1. Trước URL thật, chỉ nạp canary tĩnh vào WebView2. Kiểm tra chính sách DOM đã chạy, rồi gắn controller vào cửa sổ thử tổng hợp. Nền native đen không thể tạo marker: WGC phải thấy SVG thực từ browser ở pha không bảo vệ, rồi thấy nền công khai sau khi áp dụng WDA 0x11 ở pha bảo vệ. Sau khi đạt mới đưa controller về cửa sổ chính và cho điều hướng.
2. F8 ẩn controller và địa chỉ, dừng điều hướng, yêu cầu suspend. Thay đổi màn hình đóng browser để lần bật tiếp theo kiểm tra lại. Generation và weak-state callbacks ngăn khởi tạo muộn làm sống lại browser đã tắt.
3. Chỉ chấp nhận HTTP/HTTPS, không mở file:// hoặc giao thức ngoài. Không expose host objects/web-message bridge. Camera, microphone, location, notification, screen capture và native download đều bị từ chối.
4. Tắt DevTools, menu native, autofill/password-save UI và browser accelerators có thể mở UI ngoài. Chặn file picker, drop/paste file, native select/date/color picker và datalist/tooltip bằng chính sách DOM cài trước document; vẫn cho phép Tab và chỉnh select/date bằng phím khi an toàn. Tắt in web và nút lưu/in của PDF web. Client-certificate và HTTP Basic credential dialogs bị hủy; form đăng nhập HTML thông thường vẫn dùng được.
5. Popup tự phát bị chặn. Link mở cửa sổ mới do người dùng bấm được xếp hàng để mở thành tab nội bộ, trong giới hạn tám tab và một tab đang khởi tạo. Tab mới cần hoàn tất xác minh vùng render riêng. Alert được xử lý không tạo dialog; confirm/prompt không được tự chấp nhận. OAuth cần popup riêng có thể không tương thích.

Each new browser tab must pass an actual positive/excluded WGC render verification before user navigation. Up to eight controllers share one environment and local profile while retaining independent pages, history and zoom. Inactive tabs are hidden and request suspension. F8, lifecycle generations, stable tab tokens and topology invalidation prevent unintended revival. Native UI/permission paths remain restricted. User-initiated new-window links open an internal tab after its own privacy check; unsolicited popups are blocked. [Microsoft - Security](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/security), [NewWindowRequested](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2newwindowrequestedeventargs).

## Zoom và phiên chiếu / Zoom and projection sessions

Từ 0.8.0, web có zoom 25-400% với nút − / phần trăm đặt lại / + và Ctrl+- / Ctrl++ / Ctrl+0. Zoom dùng controller WebView2; canary luôn ở 100%, rồi mới áp dụng mức người dùng sau khi đạt. Bật web trong phiên Smart Mirror giữ khung hình công khai an toàn gần nhất trong lúc kiểm tra (tối đa 5 giây), giữ nguyên capture và projector khi kiểm tra đạt; timestamp QPC sau tháo canary loại các khung thử còn đệm. Lỗi hoặc thay đổi trạng thái an toàn vẫn khóa/dừng output. [WebView2 controller](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller), [Capture frame timestamp](https://learn.microsoft.com/en-us/uwp/api/windows.graphics.capture.direct3d11captureframe.systemrelativetime).

Enabling browsing from Settings keeps that page open; the user selects the globe icon to display web content. Web zoom uses the controller, independent of file zoom. A running same-source Smart Mirror session holds the last safe public frame through browser verification and resumes only after post-probe frames are eligible.

## Giới hạn vận hành / Operating limits

Trình duyệt tập trung vào đọc và chat bằng chữ. Không hỗ trợ quyền thiết bị, upload/download, đăng nhập qua chứng chỉ client hoặc HTTP Basic và popup đăng nhập riêng. Bảo vệ capture phụ thuộc đường capture và hệ thống hiển thị; trạng thái phía người nhận vẫn cần xác nhận riêng trong phần mềm họp hoặc máy chiếu sử dụng. Loại trừ capture không ngăn máy ảnh bên ngoài quay màn hình.

This browser is scoped to reading and text chat. Device permissions, uploads/downloads, client-certificate/HTTP Basic authentication and separate login popups are unsupported. Capture exclusion depends on the capture path and display system, and receiver status requires separate confirmation in the actual meeting or projector setup. It cannot prevent an external camera from filming a display. [Microsoft - SetWindowDisplayAffinity](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity).
