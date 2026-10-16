# QuietShare 0.8.1 - Functional specification

## Tiếng Việt

### Phạm vi

Sửa hành vi thu nhỏ khi đang dùng Smart Mirror, đưa Ghim lên góc trên, làm mịn các điều khiển Cài đặt và chuẩn hóa tài liệu VI/EN. Giữ giới hạn định dạng, quyền trình duyệt, chế độ tài nguyên, DPI, lời nhắc DPAPI và bộ cài theo tài khoản hiện tại.

### Thu nhỏ và khôi phục

- Nút **-** thu nhỏ cửa sổ điều khiển QuietShare mà không tự dừng phiên capture/projector hợp lệ. Đầu chiếu tiếp tục nhận hình ảnh công khai của nguồn đã chọn.
- Cửa sổ riêng, web và metadata không được hiển thị thêm do thu nhỏ. Trình duyệt ẩn tiếp tục nhận yêu cầu tạm ngưng theo cơ chế hiện có.
- Khi khôi phục hoặc gọi lại bằng Ctrl+Alt+Q, trạng thái nội dung vẫn phụ thuộc cổng bảo vệ chung. Khôi phục không bỏ trạng thái che F8, không thay lựa chọn nguồn/đích và không tự khởi động phiên chiếu đã dừng.
- F8, Dừng, đóng ứng dụng, suspend, thay topology/nguồn hoặc mất điều kiện affinity/capture tiếp tục dừng đầu chiếu và khóa theo quy tắc tương ứng. Thu nhỏ không tạo ngoại lệ cho Duplicate hoặc trạng thái an toàn không xác định.

### Ghim và hiển thị Cài đặt

- Nút **Ghim** nằm ở góc trên, có trạng thái bật/tắt và tooltip VI/EN. Cùng trạng thái được phản ánh trong **Cài đặt > Hiển thị**; một thao tác phải cập nhật cả hai vị trí.
- Giữ nút Ghim có thể tiếp cận trong bố cục thường và tập trung. Ghim không thay đổi chức năng thu nhỏ, F8 hoặc Ctrl+Alt+Q.
- Các nút tròn, nút gạt và đường bo trong Cài đặt dùng kết xuất có khử răng cưa theo DPI, giữ nguyên kích thước logic, vùng tương tác, màu và trạng thái hover/focus/disabled.
- Giữ bố cục, cuộn touchpad, kéo thanh cuộn và điều hướng bằng bàn phím hiện có; không thêm tính năng nghiệp vụ.

### Tài liệu và phân phối

- README là tài liệu kỹ thuật người dùng với tính năng, yêu cầu hệ thống, cài đặt/nâng cấp, cách dùng, cài đặt và bảng phím tắt có phạm vi rõ ràng bằng VI/EN.
- Đặc tả chỉ mô tả yêu cầu chức năng, ràng buộc, kiến trúc cần thiết và giới hạn vận hành. Ghi chú phát hành chỉ nêu thay đổi của sản phẩm.
- Áp dụng cùng cấu trúc tài liệu chức năng cho các đặc tả phiên bản cũ còn phân phối.
- Git và gói sản phẩm không chứa lời nhắc cá nhân, URL/đường dẫn riêng, cookie, hồ sơ trình duyệt hoặc log máy. Giữ nguyên mã nguồn và bản phát hành lịch sử; chỉ cập nhật tài liệu và nội dung phân phối trong phạm vi yêu cầu.

## English

### Scope

Correct minimizing during Smart Mirror, move Pin to the upper corner, smooth Settings controls and standardize VI/EN documentation. Preserve file limits, browser permissions, resource modes, DPI support, DPAPI scripts and per-user installation.

### Minimize and restore

- **-** minimizes the QuietShare control window without stopping a valid capture/projector session. The destination continues to receive public source-display frames.
- Minimizing must not reveal private windows, web content or metadata. Hidden browsing retains its existing suspension requests.
- Restore and Ctrl+Alt+Q recall remain subject to the common protection gate. They preserve F8 masking and display selections, and do not automatically restart a stopped projection.
- F8, Stop, application closure, suspend, topology/source changes and invalid affinity/capture continue to stop projection and lock content as required. Minimizing introduces no exception for Duplicate or unknown safety state.

### Pin and Settings rendering

- Place **Pin** in the upper corner with a clear state and VI/EN tooltip. Synchronize it with **Settings > Appearance** so either entry point updates the same state.
- Keep Pin accessible in normal and focus layouts. Pinning does not alter minimize, F8 or Ctrl+Alt+Q behavior.
- Render circular controls, toggles and rounded Settings outlines with DPI-aware antialiasing while preserving logical sizes, hit targets, colors and hover/focus/disabled states.
- Preserve layout, touchpad scrolling, scrollbar dragging and keyboard navigation without adding business features.

### Documentation and distribution

- README is a technical user guide covering capabilities, system requirements, installation/upgrades, operation, settings and scoped shortcut tables in VI/EN.
- Specifications describe functional requirements, constraints, necessary architecture and operating limits. Release notes describe product changes.
- Apply the same functional-document structure to older version specifications that remain in distribution.
- Git and shipping files exclude personal scripts, private URLs/paths, cookies, browser profiles and machine logs. Preserve source history and historical product releases; update documentation and distribution content only within the requested scope.
