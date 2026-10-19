# Troubleshooting / Xử lý sự cố

[Overview](../README.md) · [Projection setup](PRIVACY.md)

[Giới thiệu](../README.md) · [Thiết lập trình chiếu](PRIVACY.md)

## Content is locked or Start is unavailable / Nội dung bị khóa hoặc không bấm được Bắt đầu

Check **Win+P > Extend**, two distinct displays on the same GPU and the full controller window inside the source work area. In **Settings > Projection**, confirm different source/destination selections. Select **Verify WGC**, wait for the actual result, then **Start projection**. Failed checks cannot be bypassed.

Kiểm tra **Win+P > Extend**, hai màn hình khác nhau trên cùng GPU và toàn bộ cửa sổ điều khiển nằm trong vùng làm việc nguồn. Tại **Cài đặt > Trình chiếu**, xác nhận nguồn/đích khác nhau. Chọn **Kiểm tra WGC**, đợi kết quả thật, rồi **Bắt đầu trình chiếu**. Không thể bỏ qua kiểm tra thất bại.

After Duplicate, reconnect or resume, recheck the monitor pair and verify again. Projection does not restart itself. If it still fails, note the version shown on Home and the stop reason in Projection settings. Windows accepting exclusion alone does not validate audience output.

Sau Duplicate, rút/cắm màn hình hoặc thức dậy, kiểm tra cặp màn hình và xác minh lại. Ứng dụng không tự chiếu lại. Nếu vẫn lỗi, ghi phiên bản ở Home và lý do dừng trong Cài đặt Trình chiếu. Windows nhận loại trừ không tự xác nhận hình ảnh phía người xem.

## Text is hard to read / Chữ khó đọc

Choose a light or high-contrast theme on the viewer toolbar, increase text size/weight and enable word wrap for long lines. Defaults are 50% zoom and 12 pt text/scripts. If transparency makes text faint, raise window opacity toward 100%. Themes do not recolor websites, images or PDF pages; use their zoom controls instead.

Chọn theme sáng hoặc tương phản cao trên thanh xem, tăng cỡ/độ đậm và bật xuống dòng nếu dòng dài. Mặc định là zoom 50% và văn bản/lời nhắc 12 pt. Nếu chữ nhạt do cửa sổ trong suốt, tăng độ mờ về gần 100%. Theme không đổi màu website, ảnh hoặc trang PDF; dùng zoom tương ứng.

## Browser or website issues / Sự cố trình duyệt hoặc website

Install/update the official [WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/), then disable and enable browsing again. Offline readers remain available without it. Close a tab if the 8-tab limit is reached. New tabs must finish verification before loading user pages.

Cài/cập nhật [WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) chính thức, rồi tắt và bật lại trình duyệt. Trình đọc offline vẫn hoạt động khi chưa có runtime. Đóng bớt nếu đã đủ 8 tab. Tab mới phải xác minh xong trước khi tải trang người dùng.

Native web file pickers, file drops and downloads are blocked; separate OAuth popups may be unsupported. Explicit Ctrl+V image paste depends on the site's editor. Camera/microphone require the enabled setting, a visible verified HTTPS tab, matching origin and Windows permission.

Hộp chọn file web native, thả file và tải xuống bị chặn; popup OAuth riêng có thể chưa được hỗ trợ. Ctrl+V ảnh chủ động còn tùy ô nhập của website. Camera/micro cần bật cài đặt, tab HTTPS đang hiện đã xác minh, cùng origin và quyền Windows.

## A file will not open or save / Không mở hoặc lưu được file

Use a supported local file within the size/format limits in the [user guide](USER_GUIDE.md). Password PDFs, incompatible encodings, binaries and network paths are unsupported. If a file changed in another app, saving is blocked to prevent overwriting it. Keep your draft, review the external change and reload only when you can discard the current draft.

Dùng file cục bộ đúng giới hạn dung lượng/định dạng trong [hướng dẫn sử dụng](USER_GUIDE.md). Chưa hỗ trợ PDF mật khẩu, mã hóa không tương thích, file nhị phân hoặc đường dẫn mạng. Nếu file đã đổi trong ứng dụng khác, lưu bị chặn để tránh ghi đè. Giữ bản sửa, xem thay đổi bên ngoài và chỉ tải lại khi có thể bỏ bản đang sửa.

## Shortcuts and window recall / Phím tắt và gọi lại cửa sổ

Another app may already own a global shortcut. Review **Settings > Shortcuts**; local fallback and on-screen controls remain available. **Ctrl+Alt+Q** recalls QuietShare when registered, preserving masking and foreground focus. It never overrides a failed protection check.

Ứng dụng khác có thể đã giữ phím tắt toàn hệ thống. Xem **Cài đặt > Phím tắt**; vẫn có phím dự phòng cục bộ và nút giao diện. **Ctrl+Alt+Q** gọi lại QuietShare khi đăng ký được, giữ trạng thái che và focus hiện tại. Phím này không bỏ qua kiểm tra bảo vệ thất bại.

For repeatable bugs, include the app/Windows versions, exact steps and expected/observed behavior. Remove private file content, URLs, sign-in data and personal paths from shared information.

Khi báo lỗi lặp lại, cung cấp phiên bản ứng dụng/Windows, các bước và kết quả mong đợi/thực tế. Loại nội dung file riêng, URL, dữ liệu đăng nhập và đường dẫn cá nhân khỏi thông tin chia sẻ.
