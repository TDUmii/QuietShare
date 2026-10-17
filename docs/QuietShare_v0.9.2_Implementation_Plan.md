# QuietShare 0.9.2 - Functional specification

## Tiếng Việt

### Phạm vi

Giữ cửa sổ điều khiển trong màn hình nguồn khi Smart Mirror đang chiếu và dành thêm diện tích cho nội dung trong cửa sổ nhỏ. Tiếp tục hỗ trợ tài liệu, Explorer, trình duyệt nhiều tab, VI/EN và các lựa chọn bố cục hiện có.

### Giới hạn cửa sổ trong màn hình nguồn

- Khi Smart Mirror đang chạy, giới hạn toàn bộ khung cửa sổ QuietShare trong vùng làm việc của màn hình nguồn thuộc phiên capture đã xác minh.
- Điều chỉnh vị trí và kích thước trước khi Windows áp dụng thao tác di chuyển hoặc đổi kích thước, bao gồm kéo sát mép, phóng to và khôi phục. Không đổi nguồn theo màn hình mà người dùng đang kéo tới.
- Cho phép thu nhỏ cửa sổ trong khi phiên hợp lệ tiếp tục chạy; khi khôi phục, áp dụng lại giới hạn trước khi hiển thị nội dung riêng.
- Khi không chiếu, giữ thao tác di chuyển cửa sổ thông thường.
- Giữ các kiểm tra quyền riêng tư, loại trừ capture, topology và thiết bị trong lúc chạy. Nếu màn hình nguồn hoặc điều kiện bảo vệ không còn hợp lệ, dừng chiếu an toàn; không tự khởi động lại phiên.
- Giữ hành vi Dừng, F8, đóng ứng dụng, suspend và cảnh báo gián đoạn hiện có.

### Bố cục gọn

- Thu nhỏ thanh điều hướng, icon và nút điều khiển; giữ tooltip VI/EN và vùng thao tác nhất quán theo DPI.
- Giữ khả năng ẩn/hiện các thanh, Explorer, trạng thái và chế độ tập trung để người dùng điều chỉnh diện tích nội dung.
- Trên trang trình duyệt đang bật, dùng chung hàng trên cho tab web và các nút điều khiển cửa sổ; không dành một hàng tiêu đề riêng.
- Đặt quay lại, tiến tới, tải lại và ô địa chỉ trên cùng một hàng. Giữ thao tác mở, đóng và chuyển tab cùng các phím tắt hiện có.
- Đặt Google, ChatGPT và zoom trên thanh phụ. **Bố cục > Thanh công cụ** ẩn thanh phụ để mở rộng trang web; điều hướng và các phím tắt zoom vẫn hoạt động.
- Giữ xác minh capture của từng tab, giới hạn tab, quyền web và cơ chế bảo vệ hiện có.

### Phiên bản và phân phối

- Tổng quan, EXE, bộ cài và tài liệu sử dụng cùng phiên bản **v0.9.2**.
- Gói portable phải giữ các DLL đi kèm EXE. Bộ cài áp dụng cho tài khoản hiện tại, không yêu cầu quyền admin.
- Chỉ đưa file dự án và sản phẩm vào Git/gói phát hành. Loại trừ lời nhắc cá nhân, hồ sơ web, cookie, token, đường dẫn máy và log riêng.
- Giữ lịch sử GitHub Releases; cây dự án chỉ chứa ZIP hiện hành.

## English

### Scope

Keep the control window on the source display during Smart Mirror projection and give content more space in small windows. Preserve documents, Explorer, browser tabs, VI/EN and existing workspace choices.

### Source-display window containment

- During active Smart Mirror projection, keep the complete QuietShare frame inside the work area of the source display owned by the verified capture session.
- Adjust position and size before Windows applies movement or resizing, including edge dragging, maximizing and restoring. Do not follow the window to a different source display.
- Allow minimizing while a valid session continues; constrain restored bounds before private content becomes visible.
- Retain ordinary window movement when projection is inactive.
- Preserve runtime privacy, capture-exclusion, topology and device checks. Stop safely when the source display or protection conditions become invalid; never restart a failed session automatically.
- Preserve existing Stop, F8, application closure, suspend and interruption-notification behavior.

### Compact workspace

- Reduce navigation rail, icon and control sizes while retaining VI/EN tooltips and consistent DPI scaling.
- Preserve independent bar, Explorer and status visibility and focus mode so users can control the content area.
- On the active browser page, share the top row between web tabs and window controls instead of reserving a separate title row.
- Place Back, Forward, Reload and the address field on one row. Preserve existing tab creation, closure, switching and keyboard shortcuts.
- Place Google, ChatGPT and zoom on a secondary toolbar. **Layout > Toolbar** hides that row to enlarge the page while navigation and zoom shortcuts remain available.
- Preserve per-tab capture verification, tab limits, web permissions and existing privacy protections.

### Version and distribution

- Overview, EXE, installer and user documentation share version **v0.9.2**.
- Portable users must keep bundled DLLs with the EXE. The installer applies to the current account without administrator privileges.
- Git and release packages contain project and product files only. Exclude personal scripts, web profiles, cookies, tokens, machine paths and private logs.
- Preserve older GitHub Releases; keep only the current ZIP in the project tree.
