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

- **Scripts, recent-file history and saved presentation sets:** encrypted with Windows DPAPI for the local user. Sets include script snapshots, file paths and reading positions. Restoring a set keeps one encrypted copy of the previous script. Clearing recent history does not delete saved sets.
- **Preferences, layout and optional window size:** stored locally.
- **File drafts:** stay in memory until explicitly saved. Imported file content is not autosaved over the script.
- **Clear history:** removes entries and pins without deleting original files.
- **Meeting recordings and transcripts:** are written locally under the folder selected in **Settings > Meetings** only after recording is enabled. WAV + transcript keeps the capture. Transcript only removes the temporary WAV after a successful save. The recorder uses Windows playback, no microphone, and stops when QuietShare closes. The optional Whisper runtime or installed Windows Speech API processes audio locally. Audio and transcripts are not uploaded.

- **Lời nhắc, lịch sử file và bộ thuyết trình:** được Windows DPAPI mã hóa cho người dùng cục bộ. Bộ chứa bản chụp lời nhắc, đường dẫn và vị trí đọc. Mở bộ giữ một bản dự phòng mã hóa của lời nhắc trước đó. Xóa lịch sử gần đây không xóa bộ đã lưu.
- **Cấu hình, bố cục và kích thước cửa sổ tùy chọn:** được lưu trên máy.
- **Bản sửa file:** giữ trong bộ nhớ đến khi chủ động lưu. Nội dung file đã mở không tự ghi đè lời nhắc.
- **Xóa lịch sử:** xóa mục và ghim mà không xóa file gốc.
- **Bản ghi và transcript cuộc họp:** được lưu cục bộ trong thư mục tại **Cài đặt > Cuộc họp** sau khi bật nút ghi. WAV + nội dung giữ bản ghi. Chỉ nội dung xóa WAV tạm sau khi lưu thành công. Bộ ghi lấy âm thanh Windows, không mở micro và dừng khi QuietShare đóng. Whisper tùy chọn hoặc Windows Speech API đã cài xử lý cục bộ. QuietShare không tải âm thanh hoặc transcript lên mạng.

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

## Outgoing preview / Xem trước luồng đang gửi

- The private preview uses the same GPU image as a successfully presented public frame, up to 10 FPS. It creates no second capture session, CPU screenshot or image file.
- Hidden preview does no rendering work. A stop or new verification clears the old image. The preview does not certify physical receivers or meeting participants.

- Khung xem trước riêng dùng cùng ảnh GPU với khung đã được trình chiếu thành công, tối đa 10 FPS. Không tạo phiên capture thứ hai, chụp ảnh bằng CPU hay lưu file ảnh.
- Khi ẩn, khung không dựng hình. Dừng chiếu hoặc xác minh mới sẽ xóa hình cũ. Khung không xác minh đầu nhận vật lý hay máy của người dự họp.

## Reading and meeting data / Dữ liệu đọc và cuộc họp

- PDF bookmarks, script-page links and the editable notebook use Windows DPAPI for the local user. Import retains one encrypted prior notebook. Exported TXT/VTT/Markdown and WAV files are ordinary local files.
- Installing optional recognition contacts the pinned GitHub release and Hugging Face model host. This only downloads runtime/model assets and checks SHA256. It does not upload audio, notes or meeting credentials. Offline recognition needs no connection after installation.
- Dấu trang PDF, liên kết lời nhắc-trang và sổ sửa được dùng Windows DPAPI cho tài khoản cục bộ. Nhập giữ một bản sổ cũ mã hóa. TXT/VTT/Markdown xuất và WAV là file cục bộ đọc bình thường.
- Cài nhận dạng tùy chọn kết nối bản phát hành GitHub và máy chủ mô hình Hugging Face đã ghim. Chỉ tải bộ chạy/mô hình và kiểm tra SHA256. Không gửi âm thanh, ghi chép hoặc thông tin đăng nhập họp. Sau cài, nhận dạng offline không cần mạng.

Usage profiles are DPAPI-encrypted in workspace-profiles.dpapi. The fixed browser-fault.dat record stores process type, time, count and numeric reason/exit only. Diagnostics copying is explicit.

Hồ sơ sử dụng được DPAPI mã hóa trong workspace-profiles.dpapi. browser-fault.dat chỉ giữ loại tiến trình, thời gian, số sự kiện và reason/exit dạng số. Chỉ sao chép chẩn đoán khi chủ động chọn.

## Fixed-field diagnostic history / Lịch sử chẩn đoán trường cố định

The local `error-history.dat` keeps at most 100 mapped events (7,612 bytes). It contains UTC, version, fixed QS codes, phases and numeric results only. Explicit JSON export also includes Windows build, current display/protection flags, resource mode, tab counts and private-memory measurements with completeness flags. No notes, paths, URLs, tab titles, audio, screenshots or screen identifiers are included. QuietShare does not upload these files. Default exports use Downloads / QuietShare-Diagnostics. The chosen folder preference is encrypted with Windows DPAPI.

File cục bộ `error-history.dat` giữ tối đa 100 sự kiện có mã (7.612 byte). Chỉ có UTC, phiên bản, mã QS cố định, giai đoạn và kết quả dạng số. JSON do bạn chủ động xuất thêm bản Windows, cờ màn hình/bảo vệ hiện tại, chế độ tài nguyên, số tab và bộ nhớ riêng kèm cờ đo đầy đủ. Không chứa lời nhắc, đường dẫn, URL, tên tab, âm thanh, ảnh chụp hoặc định danh màn hình. QuietShare không tải các file này lên mạng. Mặc định xuất vào Downloads / QuietShare-Diagnostics. Lựa chọn thư mục được mã hóa bằng Windows DPAPI.

Manual background-tab deep sleep discards eligible running pages after confirmation. Addresses, titles and zoom remain in memory for this session. Reopening reloads and verifies the page. Unsent forms, scroll and page navigation history may be lost. Active, audible and microphone/camera-granted tabs are retained.

Nghỉ sâu tab nền bỏ trang đang chạy đủ điều kiện sau khi xác nhận. Địa chỉ, tiêu đề và zoom giữ trong RAM của phiên. Mở lại sẽ tải và xác minh trang. Ô nhập chưa gửi, vị trí cuộn và lịch sử điều hướng có thể mất. Tab đang xem, phát âm thanh hoặc được cấp micro/camera được giữ.

New error events also retain numeric committed/physical-memory totals and available amounts at failure time. Existing v0.23.0 records have unknown memory fields. No process lists or memory dumps are collected. Memory pressure uses the supported WebView2 target-level API and leaves Windows paging settings unchanged.

Sự kiện lỗi mới còn giữ tổng và phần trống bộ nhớ cam kết/RAM vật lý tại lúc lỗi, dưới dạng số. Mục v0.23.0 cũ chưa có các trường bộ nhớ. Không thu danh sách tiến trình hoặc bản dump bộ nhớ. Áp lực bộ nhớ dùng API mức bộ nhớ WebView2 có hỗ trợ, giữ nguyên thiết lập pagefile Windows.
