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

Open **Settings > Projection > Last-stop diagnostics** or **Settings > Browser > Last-stop diagnostics**. The Projection board also has **Diagnostics**. **Copy report** copies only the displayed fixed fields when you select it. This page remains available while private content is masked. It never restarts projection.

The report combines the latest projection record and the latest browser process event. Browser details include a QS-B code, UTC, fixed process type, numeric reason/exit and event count. The local file is `browser-fault.dat`. It excludes URLs, titles, private text and page contents. A recoverable browser event can appear without a projection stop.

After a failed start, read **Settings > Projection**. QuietShare opens this category when projection start fails.

Mở **Cài đặt > Trình chiếu > Chẩn đoán lần dừng** hoặc **Cài đặt > Trình duyệt > Chẩn đoán lần dừng**. Trang Trình chiếu cũng có **Chẩn đoán**. **Sao chép báo cáo** chỉ sao chép các trường cố định đang hiện khi bạn chủ động nhấn. Trang vẫn xem được lúc nội dung riêng bị che và không tự khởi động chiếu.

Báo cáo ghép lỗi chiếu gần nhất với sự kiện tiến trình web gần nhất. Phần web có mã QS-B, UTC, loại tiến trình cố định, reason/exit dạng số và số sự kiện. File cục bộ là `browser-fault.dat`. Không có URL, tiêu đề, chữ riêng hoặc nội dung trang. Một sự kiện web có thể phục hồi vẫn có bản ghi dù trình chiếu không dừng.

Sau khi bắt đầu chiếu thất bại, xem **Cài đặt > Trình chiếu**. QuietShare tự mở nhóm này khi khởi động chiếu lỗi.

[Browser recovery codes / Mã phục hồi trình duyệt](BOOK.md#browser-process-recovery--phục-hồi-tiến-trình-trình-duyệt)

If web use feels heavy, compare one tab with several tabs, then switch to the local reader or disable browsing. QuietShare groups native UI updates from busy sites. Sleep and lower-memory requests preserve page state, but cannot force every site to release its committed memory. Include the action, tab count and exact QS-B/QS-P code when reporting a problem. Authenticated Threads and physical projection still require checking on the affected setup.

Nếu duyệt web thấy nặng, so một tab với nhiều tab, sau đó chuyển sang trình đọc cục bộ hoặc tắt trình duyệt. QuietShare gom cập nhật giao diện từ trang hoạt động liên tục. Yêu cầu nghỉ/bộ nhớ thấp giữ nội dung trang nhưng không ép mọi website trả lại bộ nhớ đã cấp. Khi báo lỗi, ghi thao tác, số tab và đúng mã QS-B/QS-P. Threads đã đăng nhập và đầu chiếu thực tế vẫn cần thử trên đúng máy gặp lỗi.

## Meeting recording or transcription issues / Sự cố ghi âm hoặc chép lời cuộc họp

1. Open Meetings and check playback/recognition. Confirm the meeting is audible through the default Windows playback device.
2. Choose English/Vietnamese and Small/Base in Settings > Meetings. Install optional local Whisper if Windows has no matching recognizer (`QS-R004`).
3. Setup requires internet and sufficient disk space. Recognition itself runs offline. Use Cancel task to stop heavy processing and retain the WAV.
4. Review the status in Meetings. Use the exact `QS-R001` through `QS-R009` or `QS-N001` through `QS-N005` code in the [book](BOOK.md).
5. Check the selected output folder. VTT export requires valid times. Untimed notes can use TXT.

1. Mở Cuộc họp và kiểm tra âm thanh/nhận dạng. Xác nhận âm thanh họp phát qua thiết bị mặc định Windows.
2. Chọn Anh/Việt và Small/Base tại Cài đặt > Cuộc họp. Cài Whisper cục bộ tùy chọn nếu Windows thiếu bộ đúng ngôn ngữ (`QS-R004`).
3. Cài cần mạng và đủ dung lượng. Nhận dạng chạy offline. Dừng xử lý giúp ngừng công việc nặng và giữ WAV.
4. Đọc trạng thái trong Cuộc họp. Tra đúng mã `QS-R001` đến `QS-R009` hoặc `QS-N001` đến `QS-N005` trong [book](BOOK.md).
5. Kiểm tra thư mục xuất đã chọn. VTT cần giờ hợp lệ. Ghi chép chưa có giờ dùng TXT.

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
