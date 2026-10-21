# Troubleshooting / Xử lý sự cố

For projection codes and separate HDMI, wireless TV, meeting and OBS workflows, use the [sharing handbook](BOOK.md).

Tra mã lỗi trình chiếu và quy trình riêng cho HDMI, TV không dây, cuộc họp và OBS trong [cuốn book](BOOK.md).

[Overview](../README.md) · [Projection setup](PRIVACY.md)

[Giới thiệu](../README.md) · [Thiết lập trình chiếu](PRIVACY.md)

## Content is locked or Start is unavailable / Nội dung bị khóa hoặc không bấm được Bắt đầu

**English**

1. Select **Win+P > Extend**.
2. Confirm two distinct displays on the same GPU.
3. Keep the full controller window inside the source work area.
4. In **Settings > Projection**, confirm different source/destination selections.
5. Select **Verify WGC** and wait for the actual result.
6. Select **Start projection**. Failed checks cannot be bypassed.

**Tiếng Việt**

1. Chọn **Win+P > Extend**.
2. Xác nhận có hai màn hình khác nhau trên cùng GPU.
3. Giữ toàn bộ cửa sổ điều khiển trong vùng làm việc nguồn.
4. Tại **Cài đặt > Trình chiếu**, xác nhận nguồn/đích khác nhau.
5. Chọn **Kiểm tra WGC** và đợi kết quả thật.
6. Chọn **Bắt đầu trình chiếu**. Không thể bỏ qua kiểm tra thất bại.

- After Duplicate, reconnect or resume, recheck the monitor pair and verify again. Projection does not restart itself.
- If it still fails, note the version shown on Home and the stop reason in Projection settings.
- Windows accepting exclusion alone does not validate audience output.

- Sau Duplicate, rút/cắm màn hình hoặc thức dậy, kiểm tra cặp màn hình và xác minh lại. Ứng dụng không tự chiếu lại.
- Nếu vẫn lỗi, ghi phiên bản ở Home và lý do dừng trong Cài đặt Trình chiếu.
- Windows nhận loại trừ không tự xác nhận hình ảnh phía người xem.

## Text is hard to read / Chữ khó đọc

- Choose a light or high-contrast theme on the viewer toolbar.
- Increase text size/weight and enable word wrap for long lines.
- Defaults are 50% zoom and 12 pt text/scripts.
- If transparency makes text faint, raise window opacity toward 100%.
- Themes do not recolor websites, images or PDF pages. Adjust their zoom to make content larger.

- Chọn theme sáng hoặc tương phản cao trên thanh xem.
- Tăng cỡ/độ đậm và bật xuống dòng nếu dòng dài.
- Mặc định là zoom 50% và văn bản/lời nhắc 12 pt.
- Nếu chữ nhạt do cửa sổ trong suốt, tăng độ mờ về gần 100%.
- Theme không đổi màu website, ảnh hoặc trang PDF. Chỉnh zoom tương ứng để phóng lớn nội dung.

## Browser or website issues / Sự cố trình duyệt hoặc website

- Install/update the official [WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/).
- Disable and enable browsing again. Offline readers remain available without the runtime.
- Close a tab if the 8-tab limit is reached.
- Wait for new tabs to finish verification before loading user pages.

- Cài/cập nhật [WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) chính thức.
- Tắt và bật lại trình duyệt. Trình đọc offline vẫn hoạt động khi chưa có runtime.
- Đóng bớt nếu đã đủ 8 tab.
- Đợi tab mới xác minh xong trước khi tải trang người dùng.

- Native web file pickers, file drops and downloads are blocked.
- Separate OAuth popups may be unsupported.
- Explicit Ctrl+V image paste depends on the site's editor.
- Camera/microphone require the enabled setting, a visible verified HTTPS tab, matching origin and Windows permission.

- Hộp chọn file web native, thả file và tải xuống bị chặn.
- Popup OAuth riêng có thể chưa được hỗ trợ.
- Ctrl+V ảnh chủ động còn tùy ô nhập của website.
- Camera/micro cần bật cài đặt, tab HTTPS đang hiện đã xác minh, cùng origin và quyền Windows.

## Meeting recording or transcription issues / Sự cố ghi âm hoặc chép lời cuộc họp

- Select the language before recording in **Settings > Reading > Transcription language**.
- The recording control captures Windows playback only. Confirm the meeting audio is audible on the intended playback device.
- After stopping, wait for the footer to finish transcription. The WAV and UTF-8 TXT stay under `%LOCALAPPDATA%\QuietShare\Meetings`.
- If the footer shows `QS-R004`, install the matching English or Vietnamese Windows speech recognition component.
- Use the exact `QS-R001` through `QS-R007` code in the [sharing handbook](BOOK.md) when reporting a failure.

- Chọn ngôn ngữ trước khi ghi tại **Cài đặt > Nội dung > Ngôn ngữ chép lời**.
- Nút ghi chỉ lấy âm thanh phát ra từ Windows. Xác nhận âm thanh cuộc họp đang phát đúng thiết bị cần dùng.
- Sau khi dừng, đợi thanh trạng thái chép lời xong. WAV và TXT UTF-8 nằm tại `%LOCALAPPDATA%\QuietShare\Meetings`.
- Nếu thanh trạng thái hiện `QS-R004`, cài thành phần nhận dạng giọng nói Windows tương ứng với tiếng Anh hoặc tiếng Việt.
- Khi báo lỗi, ghi đúng mã `QS-R001` đến `QS-R007` trong [cuốn book](BOOK.md).

## A file will not open or save / Không mở hoặc lưu được file

- Check the local file's size/format against the [user guide](USER_GUIDE.md).
- Password PDFs, incompatible encodings, binaries and network paths are unsupported.
- If a file changed in another app, saving is blocked to prevent overwriting it.
- Keep your draft and review the external change.
- Reload only when you can discard the current draft.

- Đối chiếu dung lượng/định dạng của file cục bộ với [hướng dẫn sử dụng](USER_GUIDE.md).
- Chưa hỗ trợ PDF mật khẩu, mã hóa không tương thích, file nhị phân hoặc đường dẫn mạng.
- Nếu file đã đổi trong ứng dụng khác, lưu bị chặn để tránh ghi đè.
- Giữ bản sửa và xem thay đổi bên ngoài.
- Chỉ tải lại khi có thể bỏ bản đang sửa.

## Shortcuts and window recall / Phím tắt và gọi lại cửa sổ

- Another app may already own a global shortcut. Review **Settings > Shortcuts**.
- Local fallback and on-screen controls remain available.
- **Ctrl+Alt+Q** recalls QuietShare when registered, preserving masking and foreground focus.
- It never overrides a failed protection check.

- Ứng dụng khác có thể đã giữ phím tắt toàn hệ thống. Xem **Cài đặt > Phím tắt**.
- Vẫn có phím dự phòng cục bộ và nút giao diện.
- **Ctrl+Alt+Q** gọi lại QuietShare khi đăng ký được, giữ trạng thái che và focus hiện tại.
- Phím này không bỏ qua kiểm tra bảo vệ thất bại.

For repeatable bugs, include:

- App and Windows versions.
- Exact steps.
- Expected and observed behavior.

Remove private file content, URLs, sign-in data and personal paths from shared information.

Khi báo lỗi lặp lại, cung cấp:

- Phiên bản ứng dụng và Windows.
- Các bước cụ thể.
- Kết quả mong đợi và thực tế.

Loại nội dung file riêng, URL, dữ liệu đăng nhập và đường dẫn cá nhân khỏi thông tin chia sẻ.
