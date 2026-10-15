# QuietShare - duyệt web tùy chọn / optional browsing

Trạng thái: triển khai từ 0.5.0, cập nhật tìm kiếm trong 0.6.0. / Status: introduced in 0.5.0, search updated in 0.6.0.

## Thiết kế / Design

Một WebView2 Win32 trong vùng nội dung của QuietShare, dùng chế độ Window to Visual. Mặc định tắt ở mỗi lần khởi động; chỉ tạo environment/controller khi người dùng bật. Có ô địa chỉ/tìm kiếm, Back, Forward, Reload và nút Google/ChatGPT. Địa chỉ HTTP/HTTPS mở trực tiếp; từ khóa được mã hóa UTF-8 để tìm Google. Bộ xử lý từ khóa chỉ áp dụng cho nội dung người dùng nhập; navigation và popup vẫn kiểm tra URL HTTP/HTTPS chặt chẽ. Không tự mở trình duyệt ngoài app cho URL người dùng nhập.

One on-demand Win32 WebView2 in QuietShare's content area, using Window to Visual hosting. It starts disabled, with URL/search entry, Back/Forward/Reload and Google/ChatGPT shortcuts. User queries become UTF-8 encoded Google searches; navigation and popup handlers retain strict HTTP/HTTPS validation. [Microsoft - Hosting modes](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/windowed-vs-visual-hosting).

Chỉ đóng gói loader x64 194.912 byte, pin SHA-256 và SDK 1.0.4258.31. Runtime Evergreen dùng bản cài riêng trên Windows. Nếu thiếu hoặc quá cũ, vùng web báo không khả dụng; phần đọc file vẫn hoạt động. README có đường dẫn cài/cập nhật runtime chính thức.

The package includes only the 194,912-byte pinned loader. Browsing requires a separate current Evergreen Runtime; offline readers remain independent. [Microsoft - Runtime distribution](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution).

## Tài nguyên và dữ liệu / Resources and data

- Tắt browser đóng controller, thả environment và các COM object. Kiểm thử theo dõi process web thực và xác nhận process kết thúc.
- Đọc file hoặc thu nhỏ sẽ ẩn browser và yêu cầu TrySuspend; trở lại browser sẽ Resume. Đã kiểm tra IsSuspended thực trên runtime của máy kiểm thử. Không vẽ lại bounds không đổi.
- Hồ sơ riêng trong thư mục dữ liệu QuietShare của tài khoản. Cookie được giữ khi tắt/bật để đăng nhập thuận tiện; không dùng hồ sơ Edge cá nhân. Khởi động app vẫn tắt browser dù đã có hồ sơ web.
- URL, nội dung trang, cookie, thông tin đăng nhập và dữ liệu profile không được xuất vào Git hay bằng chứng phát hành. Chỉ số thời gian và tên kiểm tra được xuất; fixture/profile thô chỉ ở artifacts bị ignore.

Disabling closes web objects and tests confirm browser-process exit. Hidden browsing requests suspension and resumes on return. A separate profile preserves cookies while every app startup remains opt-in. [Microsoft - TrySuspend](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2_3), [User data folders](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/user-data-folder).

## Bảo vệ và giới hạn / Protection and boundaries

1. Trước URL thật, chỉ nạp canary tĩnh vào WebView2. Kiểm tra chính sách DOM đã chạy, rồi gắn controller vào cửa sổ thử tổng hợp. Nền native đen không thể tạo marker: WGC phải thấy SVG thực từ browser ở pha không bảo vệ, rồi thấy nền công khai sau khi áp dụng WDA 0x11 ở pha bảo vệ. Sau khi đạt mới đưa controller về cửa sổ chính và cho điều hướng.
2. F8 ẩn controller và địa chỉ, dừng điều hướng, yêu cầu suspend. Thay đổi màn hình đóng browser để lần bật tiếp theo kiểm tra lại. Generation và weak-state callbacks ngăn khởi tạo muộn làm sống lại browser đã tắt.
3. Chỉ chấp nhận HTTP/HTTPS, không mở file:// hoặc giao thức ngoài. Không expose host objects/web-message bridge. Camera, microphone, location, notification, screen capture và native download đều bị từ chối.
4. Tắt DevTools, menu native, autofill/password-save UI và browser accelerators có thể mở UI ngoài. Chặn file picker, drop/paste file, native select/date/color picker và datalist/tooltip bằng chính sách DOM cài trước document; vẫn cho phép Tab và chỉnh select/date bằng phím khi an toàn. Tắt in web và nút lưu/in của PDF web. Client-certificate và HTTP Basic credential dialogs bị hủy; form đăng nhập HTML thông thường vẫn dùng được.
5. Popup tự phát bị chặn. Link mở cửa sổ mới do người dùng bấm điều hướng trong cùng vùng web. Alert được xử lý không tạo dialog; confirm/prompt không được tự chấp nhận. OAuth cần popup riêng có thể không tương thích.

The browser must pass an actual positive/excluded WGC render test before user navigation. F8, lifecycle generations and topology invalidation prevent unintended revival. Native UI/permission paths are restricted. This is a focused browser for reading and text chat, with one web view. [Microsoft - Security](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/security), [NewWindowRequested](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2newwindowrequestedeventargs).

## Bằng chứng và giới hạn kiểm thử / Evidence and limits

Xem [báo cáo 0.7.1](../TEST_REPORT_v0.7.1.md) và [bằng chứng đã lọc](test-evidence-v0.7.1.json). Kiểm thử shipping EXE dùng profile riêng, HTTP fixture cục bộ và WGC thực, kiểm tra chuyển đến tìm kiếm Google bằng tiếng Việt cùng các shortcut. Bằng chứng ghi riêng trạng thái tải HTTPS công khai; không đăng nhập tài khoản cá nhân hoặc gửi tin nhắn khi kiểm thử. Bật web từ Cài đặt giữ nguyên trang cấu hình; người dùng chọn biểu tượng trình duyệt để mở vùng web.

Authenticated ChatGPT chat, third-party OAuth, client-certificate sites, a clean machine without Evergreen Runtime, physical Extend/Duplicate and meeting receivers remain unverified. Capture exclusion is not DRM or protection against external cameras. [Microsoft - SetWindowDisplayAffinity](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity).
