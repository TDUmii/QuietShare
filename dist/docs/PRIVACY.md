# Privacy and projection / Quyền riêng tư và trình chiếu

[Overview](../README.md) · [Troubleshooting](TROUBLESHOOTING.md)

[Giới thiệu](../README.md) · [Xử lý sự cố](TROUBLESHOOTING.md)

## Set up Smart Mirror / Thiết lập Smart Mirror

1. In Windows, select **Win+P > Extend**. Keep the entire QuietShare controller on the source display.

   Trong Windows, chọn **Win+P > Extend**. Giữ toàn bộ cửa sổ QuietShare trên màn hình nguồn.

2. Open **Settings > Projection**. Select distinct source/destination displays on the same GPU.

   Mở **Cài đặt > Trình chiếu**. Chọn nguồn/đích khác nhau trên cùng GPU.

   Use the role and resolution labels to identify displays. A display number alone does not identify the primary display.

   Dùng nhãn vai trò và độ phân giải để nhận diện màn hình. Số thứ tự không nhất thiết là màn hình chính.

3. Select **Verify WGC (F9)**, then **Start projection (F10)**. Confirm the audience output with harmless sample content in the exact meeting/capture setup you will use.

   Chọn **Kiểm tra WGC (F9)**, rồi **Bắt đầu trình chiếu (F10)**. Dùng nội dung mẫu không nhạy cảm để kiểm tra phía người xem trong đúng phần mềm họp/capture sắp dùng.

4. Use **Stop/F10** to end projection, or **F8** to mask private content and stop projection and scrolling. A fresh start is required to project again.

   Dùng **Dừng/F10** để kết thúc, hoặc **F8** để che nội dung riêng và dừng cả chiếu lẫn cuộn. Cần chủ động bắt đầu lại để tiếp tục chiếu.

- While projecting, the controller stays within the verified source work area.
- Minimizing it is supported.
- QuietShare does not change Windows display mode or automatically move the private window to the audience display.

- Trong khi chiếu, cửa sổ điều khiển được giữ trong vùng làm việc nguồn đã xác minh.
- Có thể thu nhỏ cửa sổ.
- QuietShare không đổi chế độ màn hình Windows hoặc tự chuyển cửa sổ riêng sang màn hình khán giả.

## Understand the status / Hiểu đúng trạng thái

- **Windows accepted capture exclusion:** Windows read back the requested exclusion for the private window.
- This does not prove what a remote audience, capture card, meeting app or physical projector receives.
- **Viewer/physical receiver unverified:** remains a separate status until you inspect that output yourself.

- **Windows đã nhận loại trừ capture:** Windows đọc lại đúng loại trừ được yêu cầu cho cửa sổ riêng.
- Điều này không chứng minh hình ảnh mà người xem từ xa, capture card, phần mềm họp hoặc máy chiếu nhận được.
- **Người xem/đầu nhận vật lý chưa được xác minh:** là trạng thái riêng và cần bạn tự quan sát đầu ra đó.

Protection relies on Windows/DWM, Windows Graphics Capture, driver behavior and the verified display topology. It is not a guarantee against every capture path, operating-system/driver failure or an external camera photographing the screen.

Bảo vệ phụ thuộc Windows/DWM, Windows Graphics Capture, trình điều khiển và cấu hình màn hình đã xác minh. Đây không phải bảo đảm cho mọi đường capture, lỗi hệ điều hành/trình điều khiển hoặc máy ảnh bên ngoài quay màn hình.

## Safety stops and recovery / Dừng bảo vệ và khôi phục

- **Duplicate locks private content.**
- Loss of exclusion, invalid topology, capture/device failure, suspend or an unsafe private surface stops projection and locks content.
- Output never resumes automatically after a safety stop.

- **Duplicate khóa nội dung riêng.**
- Mất loại trừ, cấu hình màn hình không hợp lệ, lỗi capture/thiết bị, suspend hoặc vùng riêng không còn an toàn sẽ dừng chiếu và khóa nội dung.
- Ứng dụng không tự chiếu lại sau khi dừng bảo vệ.

An unexpected stop changes the title/status and may flash the taskbar and play a Windows alert. It does not take focus or restore a minimized window.

1. Inspect **Settings > Projection**.
2. Return to a valid Extend setup. Returning from Duplicate to Extend reselects distinct source/destination displays where available.
3. Verify WGC again.
4. Explicitly start a new session.

A healthy active-session WGC check does not by itself end the session.

Khi dừng ngoài dự kiến, tiêu đề/trạng thái đổi và có thể nháy taskbar, phát âm báo Windows. Ứng dụng không giành focus hoặc mở lại cửa sổ đang thu nhỏ.

1. Xem **Cài đặt > Trình chiếu**.
2. Trở về Extend hợp lệ. Khi đổi Duplicate về Extend, ứng dụng chọn lại nguồn/đích khác nhau nếu có.
3. Kiểm tra WGC lại.
4. Chủ động bắt đầu phiên mới.

Kiểm tra WGC trong phiên khỏe mạnh không tự kết thúc phiên.

## Local storage / Dữ liệu cục bộ

- **Scripts and recent-file history:** encrypted with Windows DPAPI for the local user.
- **Preferences, layout and optional window size:** stored locally.
- **File drafts:** stay in memory until explicitly saved. Imported file content is not autosaved over the script.
- **Clear history:** removes entries and pins without deleting original files.

- **Lời nhắc và lịch sử file:** được Windows DPAPI mã hóa cho người dùng cục bộ.
- **Cấu hình, bố cục và kích thước cửa sổ tùy chọn:** được lưu trên máy.
- **Bản sửa file:** giữ trong bộ nhớ đến khi chủ động lưu. Nội dung file đã mở không tự ghi đè lời nhắc.
- **Xóa lịch sử:** xóa mục và ghim mà không xóa file gốc.

- The optional browser uses a separate local WebView2 profile for site data, including cookies/sign-ins and saved shortcuts.
- These records do not imply the script's DPAPI storage format.
- Browsing contacts websites you open and Google when you submit search terms.
- File readers need neither the browser engine nor a network connection.

- Trình duyệt tùy chọn dùng hồ sơ WebView2 riêng trên máy cho dữ liệu website, gồm cookie/đăng nhập và lối tắt.
- Những dữ liệu này không mặc nhiên dùng định dạng DPAPI của lời nhắc.
- Trình duyệt kết nối website bạn mở và Google khi gửi từ khóa tìm kiếm.
- Trình đọc file không cần engine trình duyệt hoặc kết nối mạng.

- One local diagnostic record retains the latest failed projection start, verification failure or unexpected stop.
- It includes the handbook code, phase, time and technical state. See the [sharing handbook](BOOK.md).
- It excludes screenshots, file contents, URLs and display names.
- Review diagnostic files before sharing.

- Một bản ghi chẩn đoán cục bộ giữ lần khởi động chiếu thất bại, xác minh thất bại hoặc dừng ngoài dự kiến gần nhất.
- Bản ghi có mã trong book, giai đoạn, thời gian và trạng thái kỹ thuật. Xem [cuốn book](BOOK.md).
- Bản ghi không chứa ảnh màn hình, nội dung file, URL hoặc tên màn hình.
- Xem lại tệp chẩn đoán trước khi chia sẻ.

## Browser permissions / Quyền trình duyệt

- Camera and microphone are off by default.
- Enabling one permits a native grant only for the visible, capture-verified HTTPS tab and requests matching its top-level origin.
- Windows device permissions still apply.
- Website grants are not persisted. Revoking a grant may reload the tab to stop device streams.

- Camera và micro mặc định tắt.
- Khi bật, quyền native chỉ được cấp cho tab HTTPS đang hiện, đã xác minh capture và yêu cầu cùng origin với trang chính.
- Quyền thiết bị Windows vẫn áp dụng.
- Quyền website không được lưu. Thu hồi có thể tải lại tab để dừng luồng thiết bị.

- Automatic clipboard reads are denied.
- Explicit raster-image paste into compatible web editors is allowed.
- Downloads, web file pickers, file drops, screen sharing and other restricted native surfaces remain blocked. See the [user guide](USER_GUIDE.md).

- Đọc clipboard tự động bị từ chối.
- Cho phép người dùng chủ động dán ảnh raster vào ô nhập web tương thích.
- Tải xuống, hộp chọn file web, thả file, chia sẻ màn hình và các vùng native bị hạn chế vẫn bị chặn. Xem [hướng dẫn sử dụng](USER_GUIDE.md).
