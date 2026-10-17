# QuietShare 0.9.1 - Functional specification

## Tiếng Việt

### Phạm vi

Sửa việc Smart Mirror đang chạy ở Extend tự dừng sau khoảng 5 phút và giúp người thuyết trình nhận biết khi phiên chiếu bị gián đoạn. Giữ nguyên trình đọc tài liệu, Explorer, trình duyệt nhiều tab, bố cục và các chế độ tài nguyên hiện có.

### Thời gian phiên chiếu

- Kết quả xác minh capture không tự mất hiệu lực chỉ vì đã qua một khoảng thời gian cố định. Phiên Smart Mirror không có mốc tự dừng 5 phút.
- Phiên tiếp tục khi nguồn và đích Extend, đường capture, thiết bị đồ họa, vùng cửa sổ riêng và trạng thái loại trừ capture vẫn hợp lệ.
- Giữ kiểm tra bảo vệ trong lúc chạy. Thay đổi cấu hình màn hình, suspend, mất loại trừ capture, bề mặt riêng không được bảo vệ hoặc lỗi capture/thiết bị tiếp tục dừng chiếu và thu hồi trạng thái xác minh tương ứng.
- Không bỏ qua kiểm tra capture thực tế khi khởi tạo hoặc khi xác minh lại. Duplicate tiếp tục khóa nội dung riêng; không có tùy chọn bỏ qua.
- Thu nhỏ QuietShare tiếp tục giữ phiên chiếu hợp lệ. Dừng, F8 và đóng ứng dụng giữ hành vi kết thúc chiếu hiện có.
- Khi chiếu đang hoạt động, yêu cầu Windows giữ hệ thống và màn hình thức; khôi phục yêu cầu trước đó của thread khi dừng. Không chặn lệnh ngủ chủ động hoặc thay đổi chính sách đóng nắp máy.
- Sau khi mất điều kiện bảo vệ, không tự khôi phục đầu chiếu. Người dùng kiểm tra trạng thái, xác minh lại khi cần rồi chủ động bắt đầu.

### Nhận biết gián đoạn

- Khi phiên đang chiếu bị gián đoạn ngoài thao tác chủ động, đổi tiêu đề thành **QuietShare - Trình chiếu đã dừng** và duy trì cảnh báo màu hổ phách trong thanh trạng thái khi thanh này đang hiện.
- Nháy nút taskbar và yêu cầu một âm báo Windows; việc nghe được âm báo tùy thiết lập âm thanh hệ thống. Không mở popup, giành focus, mở Alt+Tab hoặc tự khôi phục cửa sổ thu nhỏ.
- Cảnh báo không đưa nội dung riêng lên màn hình đích, không tự bỏ trạng thái che hoặc khôi phục capture. Bắt đầu chiếu thành công hoặc chủ động Dừng/F8 sẽ xóa cảnh báo.
- Thao tác Dừng, F8 hoặc đóng ứng dụng chủ động không tạo cảnh báo gián đoạn.
- Trạng thái tại trang Trình chiếu cung cấp điểm kiểm tra để người dùng xem nguyên nhân và xác minh/bắt đầu lại.

### Phiên bản và phân phối

- Trang Tổng quan, EXE và bộ cài dùng cùng phiên bản **v0.9.1**.
- Hướng dẫn cài đặt, vận hành và khôi phục có tiếng Việt và tiếng Anh. Tài liệu phát hành chỉ mô tả thay đổi chức năng và giới hạn sử dụng.
- Git và gói phát hành chỉ chứa file liên quan đến QuietShare; loại trừ lời nhắc, cookie, hồ sơ web, đường dẫn máy và log riêng. Giữ các GitHub Releases cũ; chỉ giữ ZIP hiện hành trong cây dự án.

## English

### Scope

Fix Smart Mirror stopping after approximately five minutes in Extend and make unexpected projection interruptions apparent to the presenter. Preserve existing readers, Explorer, browser tabs, workspace layout and resource modes.

### Projection lifetime

- Capture verification does not expire solely because a fixed interval has elapsed. Smart Mirror has no five-minute automatic stop boundary.
- A session continues while the Extend source/destination, capture path, graphics device, private-window coverage and capture-exclusion state remain valid.
- Retain runtime protection checks. Display changes, suspend, exclusion loss, unprotected private surfaces and capture/device failures continue to stop projection and invalidate the corresponding verification state.
- Preserve actual capture verification at initialization and when verification is required again. Duplicate continues to lock private content without an override.
- Minimizing QuietShare keeps a valid projection session running. Stop, F8 and application closure retain their existing projection-stop behavior.
- Request system/display wakefulness only during active projection and restore the thread's previous requirements when projection stops. Do not block intentional sleep or change lid-close policy.
- Never restart projection automatically after protection conditions are lost. The user reviews the status, verifies again when needed and explicitly starts projection.

### Interruption awareness

- When an active projection is interrupted without an intentional stop, change the title to **QuietShare - Projection stopped** and retain an amber warning in the status bar when it is visible.
- Flash the taskbar button and request one Windows warning sound, subject to system sound settings. Do not open a popup, take focus, open Alt+Tab or restore a minimized window.
- The warning must not expose private content on the destination, unmask content or restore capture automatically. Clear the warning after a successful Start or intentional Stop/F8.
- Intentional Stop, F8 or application closure does not trigger an interruption warning.
- Presentation status remains the place to review the reason and verify/start projection again.

### Version and distribution

- Overview, the EXE and installer share version **v0.9.1**.
- Provide Vietnamese and English installation, operation and recovery guidance. Release documentation describes functional changes and operating limits.
- Git and release packages contain only QuietShare project files. Exclude personal script content, cookies, web profiles, machine paths and private logs. Preserve older GitHub Releases; keep only the current ZIP in the project tree.
