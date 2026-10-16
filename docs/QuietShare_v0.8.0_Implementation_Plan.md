# QuietShare 0.8.0 - Functional specification

## Tiếng Việt

### Phạm vi

Hoàn thiện bản native Windows x64 trên giao diện hiện có. Giữ nguyên lời nhắc tự lưu bằng DPAPI, file preview chỉ đọc, tab/project, PDF/ảnh/code, VI/EN, F8, F11, Ctrl+Alt+Q, bố cục đã lưu và giới hạn quyền của trình duyệt. Chỉ thay các phần dưới đây; không thêm runtime hoặc dependency icon mới.

### 1. Zoom trình duyệt

- Thêm ba điều khiển trên thanh web: thu nhỏ (`167`), phần trăm hiện tại kiêm đặt lại (`168`), phóng to (`169`). Nút phần trăm phải phản ánh giá trị thực của controller, không luôn hiện 100%.
- Phạm vi 25-400%, bước nút 10 điểm phần trăm; đặt lại 100%. Ctrl+−, Ctrl++, Ctrl+0 và Ctrl rồi lăn chuột điều khiển web khi ngữ cảnh browser đang hoạt động, không làm thay đổi cỡ chữ/zoom của file.
- Đồng bộ thay đổi zoom từ WebView2 với nhãn UI. Giữ preference trong phiên browser; không dùng profile hoặc lịch sử để lưu đường dẫn/nội dung riêng.
- Canary web luôn chạy ở 100%, rồi khôi phục zoom người dùng sau khi đạt. Khi đang probe, chưa ready hoặc bị khóa, các lệnh zoom không được mở nội dung hay bỏ qua privacy gate.

### 2. Bật web trong phiên Smart Mirror đang chạy

- Nếu đang chạy trên cùng nguồn hợp lệ, mounted browser probe được dùng lại phiên capture/projector đã có. Giữ khung hình công khai an toàn gần nhất trong lúc probe, không tự kết thúc phiên chiếu thành công và không yêu cầu người dùng bấm Bắt đầu lại.
- Giữ positive control chứng minh marker hiện trong capture trước khi loại trừ; sau đó yêu cầu **hai khung excluded liên tiếp** đạt bộ phân loại hiện có, đồng thời xác nhận vùng công khai xung quanh. Thời gian probe tối đa **5 giây**.
- Trong probe, không đưa marker thử hoặc khung capture chưa được chứng minh an toàn lên projector. Chỉ giữ frame công khai cuối cùng đã trình chiếu.
- Sau khi tháo probe, dựng rào thời gian QPC và loại toàn bộ frame cũ hoặc frame chưa chứng minh có timestamp mới hơn rào. Chỉ frame mới sau teardown được phép thay khung đang giữ. Không suy ra độ mới từ việc frame vừa được lấy khỏi queue.
- Timeout, hủy, F8, đóng app, thay topology/nguồn, affinity sai, dữ liệu mơ hồ hoặc lỗi capture/GPU vẫn dừng, làm trống/đóng đầu chiếu và thu hồi trạng thái verified. Quy tắc giữ frame chỉ áp dụng khi toàn bộ điều kiện an toàn còn hợp lệ.

### 3. Thanh cuộn Cài đặt

- Dùng cùng hình học track/thumb cho vẽ, hit-test và pointer tracking. Chiều rộng/vùng bắt chuột không thay đổi khi hover; chiều cao/travel phản ánh đúng viewport và tổng nội dung.
- Kéo thumb bám theo con trỏ với offset ban đầu, giữ capture khi chuột ra khỏi thanh; thả chuột hoặc mất capture phải kết thúc kéo đúng cách. Không nhảy vị trí do đổi hover hoặc redraw.
- Giữ màu tối khi idle/hover/pressed, cuộn touchpad theo delta nhỏ, chặn hai đầu và tự cuộn theo focus. Không thay cỡ chữ hoặc scroll của file ở phía sau.

### 4. Icon loại file

- Explorer và tab dùng cùng `FileIconForName`/`DrawFileTypeIcon`, dựa vào basename/extension trong bộ nhớ. Không gọi shell association hoặc trích icon từ từng file.
- Dùng Codicons đã bundle, kết hợp màu/ký hiệu ngắn cho Python, JavaScript, TypeScript, C/C++, C#, Java, Rust và Go; có nhóm JSON/XML/Markdown/text/config, HTML/CSS/shell, ảnh/PDF, generic fallback và thư mục mở/đóng.
- Native tree dùng image list cache theo DPI: 16 logical px, chỉ tạo lại khi DPI thay đổi; giải phóng list cũ khi thay và list sở hữu khi đóng. Màu/icon không được tạo thêm metadata surface ngoài cây/tab vốn đã bảo vệ.
- F8/khóa vẫn ẩn cả cây, tên tab, tooltip và viewer; không thêm ngoại lệ privacy vì icon.

### 5. Tài nguyên

Thêm nhóm **Hiệu suất > Tài nguyên** trong Cài đặt, control `189`, bấm để chuyển chế độ. Mặc định là mã `2`.

| Mã | Tiếng Việt / English | Giới hạn chiếu | Timer poll khi chạy |
|---|---|---:|---:|
| 0 | Tiết kiệm / Saver | 15 FPS | 33 ms |
| 1 | Cân bằng / Balanced | 30 FPS | 16 ms |
| 2 | Hiệu năng / Performance | 60 FPS | 8 ms |

- Giới hạn FPS dùng frame pacing riêng; khoảng poll không phải tuyên bố FPS trình chiếu thực tế. Khi không chạy mirror, timer dùng 33 ms ở cả ba chế độ.
- Lưu lựa chọn cùng preference bố cục phiên bản V2; vẫn đọc được V1. V1 không chứa chế độ tài nguyên nên giữ mặc định Performance. Dữ liệu hỏng/ngoài phạm vi không được ghi đè state bằng giá trị không hợp lệ.
- Đổi chế độ chỉ thay cadence/giới hạn đã nêu, không hạ tiêu chuẩn canary, affinity hoặc kiểm tra topology. Không làm mất nguồn/đích, browser zoom hay vị trí tài liệu.
- Browser ẩn vẫn yêu cầu `TrySuspend` và được resume khi hiện lại như hành vi hiện có; tắt browser vẫn đóng engine. Không cam kết giảm một số MB hay phần trăm RAM, vì runtime/website quyết định bộ nhớ được giữ.

### 6. Nhãn và mặc định màn hình

- Danh sách nguồn/đích dùng số nhận diện Windows khi có, kèm **Chính / Main**, **Cửa sổ này / This window** và độ phân giải. Chỉ hiện tag đúng với màn đó; không suy ra màn chính từ thứ tự enumeration hoặc số 1.
- Mặc định nguồn là monitor có cờ primary, đích là monitor khác nếu có. Lựa chọn đã có được giữ theo stable ID khi còn hợp lệ.
- Cập nhật tag Cửa sổ này khi QuietShare đi sang màn khác và dịch lại nhãn khi đổi VI/EN. Cập nhật nhãn không tự đổi nguồn/đích hay topology của Windows.
- Chỉ một màn hình vẫn xác minh cửa sổ riêng được; Smart Mirror yêu cầu hai màn khác nhau, Extend và cùng GPU theo giới hạn hiện tại.

### Bảo vệ nội dung và giới hạn vận hành

Giữ cơ chế xác minh capture thực tế và khóa khi Duplicate hoặc topology không xác định. Không mở trang web của người dùng trước khi xác minh vùng web đạt. Nội dung, URL, tên/đường dẫn riêng, cookie và hồ sơ trình duyệt không thuộc tài liệu hoặc gói phát hành.

Giới hạn FPS là mức xử lý mục tiêu của ứng dụng, không bảo đảm 60 FPS thực tế hoặc mức giảm RAM cố định. Smart Mirror yêu cầu hai màn hình Extend khác nhau trên cùng GPU. Các hạn chế quyền trình duyệt và PDF có mật khẩu giữ nguyên.

## English

### Scope

Refine the existing native Windows x64 product while preserving DPAPI scripts, read-only previews, tabs/project browsing, PDF/image/code readers, VI/EN, F8/F11/Ctrl+Alt+Q, saved layout and browser permission restrictions. Add no icon dependency or runtime.

### Behavior

1. **Web zoom:** controls `167/168/169` provide zoom out, actual current percentage/reset, and zoom in. Clamp to 25-400%, step buttons by 10 percentage points and reset to 100%. Ctrl±/0 and Ctrl+wheel act on browsing without changing file zoom. Synchronize controller-originated zoom changes. Keep the canary at 100%, restoring the user's session zoom only after success; probing/locked states remain gated.
2. **Browser verification during projection:** reuse a valid running same-source capture/projector while holding its last safe public frame. Require the positive marker control followed by **two consecutive excluded frames** with public surroundings, within a **five-second timeout**. Do not project the test marker. After removing the probe, use a QPC timestamp barrier to reject every stale/not-provably-new frame before resuming presentation. Failure, cancellation, F8, shutdown, topology/affinity/capture/GPU faults still blank/stop projection and revoke verification.
3. **Settings scrollbar:** share track/thumb geometry between drawing and hit-testing. Hover must not change width/hit area. Dragging retains the initial pointer offset and capture outside the track, and ends correctly on release/capture loss. Preserve the dark idle/hover/pressed theme, proportional touchpad input, edge clamping and focus scrolling, without moving the background document.
4. **File-type icons:** share the in-memory basename/extension classifier and Codicons-based renderer across Explorer and tabs. Distinguish common languages, structured/text/config files, images/PDFs and open/closed folders, with a generic fallback. Use one native image list per DPI at 16 logical px, replace/free it correctly and avoid per-file shell association/extraction. Existing metadata privacy gates remain unchanged.
5. **Resource modes:** **Settings > Performance > Resources**, control `189`, cycles Saver (0, 15 FPS, 33 ms running poll), Balanced (1, 30 FPS, 16 ms) and default Performance (2, 60 FPS, 8 ms). Independent frame pacing enforces the cap; achieved FPS is not inferred from timer frequency. Idle polling is 33 ms in every mode. Persist layout/preferences V2 while accepting V1 and validating corrupt input; V1 retains default Performance. Preserve hidden-browser suspension requests and resume, without claiming a fixed RAM saving or relaxing privacy checks.
6. **Display labels:** show the Windows display number when available, Main, This window and resolution. Source defaults to primary and destination to another display; preserve valid existing selections by stable ID. Update location tags/language without changing selected displays or Windows topology. One display supports private-window verification; Smart Mirror still needs different Extend displays on one GPU.

### Content protection and operating limits

Retain actual capture verification and lock content in Duplicate or unknown topology. User web pages remain unavailable until browser verification succeeds. Private content, URLs/paths, cookies and browser profiles do not belong in documentation or shipping archives.

FPS limits describe application processing targets rather than guaranteed 60 FPS output or a fixed RAM reduction. Smart Mirror requires two different Extend displays on the same GPU. Existing browser permission restrictions and the unsupported password-protected PDF limit remain in force.
