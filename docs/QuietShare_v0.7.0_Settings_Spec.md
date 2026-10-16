# QuietShare 0.7.0 - Settings

## Bổ sung 0.7.1 / 0.7.1 amendments

- Khi ô cỡ chữ/zoom nhận focus hoặc được bấm lần đầu, dấu nháy nằm cuối giá trị, không chọn toàn bộ số. Lần bấm tiếp theo vẫn đặt vị trí/chọn chữ theo hành vi native; Enter áp dụng giá trị và giữ dấu nháy ở cuối.
- Cuộn touchpad trong Cài đặt phản hồi theo delta nhỏ, giữ phần lẻ, đổi chiều và chặn đúng hai đầu. Di chuyển hàng theo lô, không cập nhật lại document/browser hoặc tên các nút mỗi lần cuộn.
- Thanh tab áp dụng delta nhỏ cho cả cuộn dọc và ngang; không đo lại tên tab khi chỉ thay đổi vị trí cuộn.
- Focusing or first-clicking text size/zoom puts an unselected caret at the end. Subsequent clicks retain native positioning/selection; Enter applies the value and keeps the caret at the end.
- Settings responds proportionally to small touchpad deltas, retaining fractional motion with reversal and edge clamping. Batch row movement without refreshing documents, browsers or button text for each wheel packet.
- The tab strip applies small vertical/horizontal deltas without measuring names again during scrolling.

## Tiếng Việt

### Phạm vi

Hoàn thiện trang Cài đặt theo giao diện tối hiện có, gom toàn bộ cấu hình sẵn có vào một nơi và bỏ thanh trượt cỡ chữ. Không thêm tính năng nghiệp vụ, không thay đổi giới hạn định dạng file hoặc cơ chế capture.

### Bố cục và điều khiển

- Form phẳng căn giữa, rộng tối đa 760 logical px, với nhãn và giá trị căn trục thống nhất. Giữ khoảng cách, góc bo và trạng thái hover/focus phù hợp giao diện hiện tại.
- Nhóm **Hiển thị**: ngôn ngữ và ghim trên cùng.
- Nhóm **Đọc nội dung**: ô nhập cỡ chữ/zoom, đơn vị pt/%, Đặt lại, tốc độ lời nhắc và xuống dòng code. Bỏ hoàn toàn slider. Lời nhắc/văn bản dùng 8-200 pt; code/ảnh/PDF dùng 25-400%. Phạm vi/lỗi nằm gần ô nhập và không phụ thuộc thanh trạng thái.
- Nhóm **Bố cục**: bật/tắt riêng thanh điều hướng, Explorer, tab, công cụ, trạng thái và khôi phục mặc định. Giữ tính lưu cục bộ của bố cục hiện có.
- Nhóm **Tiện ích**: hiện thư mục nặng/ẩn và bật/tắt trình duyệt web.
- Nhóm **Trình chiếu**: màn hình nguồn, màn hình đích và con trỏ khi chiếu.
- Nội dung cuộn dọc khi cần, kể cả cửa sổ tối thiểu 430x320 và DPI cao. Tiêu đề/điều hướng nằm ngoài vùng cuộn; control được clip đúng vùng. Con lăn và Tab hoạt động trên các control, focus tự đưa hàng tương ứng vào vùng nhìn thấy. Thanh cuộn native có màu tối.
- Ngôn ngữ VI/EN, nhãn đơn vị và trạng thái hiện tại rõ ràng. Các control không hợp lệ trong ngữ cảnh hiện tại giữ trạng thái disabled đúng quy tắc đang có.

### Luồng sử dụng

**Cài đặt** mở qua icon bánh răng hoặc menu công cụ. Menu công cụ giữ các thao tác làm việc và thêm lối tắt Cài đặt; các mục cấu hình được gom vào form. Menu Bố cục, Ghim và các nút zoom nhanh vẫn hiện ở nơi thao tác chính.

Trang **Trình chiếu** hiển thị tóm tắt nguồn/đích cùng **Thiết lập màn hình**, **Kiểm tra WGC**, **Bắt đầu**, **Dừng**. Chọn **Thiết lập màn hình** chuyển thẳng tới nhóm Trình chiếu trong Cài đặt. Đổi màn hình tiếp tục yêu cầu xác minh theo quy tắc hiện tại.

Bật web trong Cài đặt giữ nguyên trang Cài đặt trong quá trình khởi tạo và kiểm tra capture. Người dùng chọn icon quả địa cầu để duyệt sau khi kiểm tra đạt. Trình duyệt vẫn mặc định tắt ở mỗi lần mở app, dùng hồ sơ riêng, đóng engine khi tắt và yêu cầu tạm ngưng khi đọc file.

### Bảo vệ nội dung và phát hành

Duplicate luôn khóa nội dung riêng. Cài đặt không tạo ngoại lệ cho canary, display affinity hoặc thay đổi màn hình. Không cho nội dung web của người dùng chạy trước khi kiểm tra capture thực tế đạt; giữ các hạn chế quyền thiết bị, upload/download, popup và giao diện native hiện tại.

Git và gói phát hành chỉ chứa mã nguồn, tài liệu, ảnh minh họa và binary liên quan tới QuietShare. Không đưa lời nhắc cá nhân, profile trình duyệt, cookie, tài khoản, đường dẫn máy hoặc log riêng vào dự án. Giữ các GitHub Releases cũ, chỉ giữ gói sản phẩm hiện hành trong cây dự án.

## English

### Scope

Refine Settings within the existing dark interface, consolidate existing configuration and remove the text-size slider. Add no business features and preserve file limits and capture behavior.

### Layout and controls

- A flat centered form, up to 760 logical px wide, aligns labels and values consistently. Match existing spacing, rounding and hover/focus states.
- **Appearance**: language and always-on-top pinning.
- **Reading**: manual text size/zoom, pt/% units, Reset, script scrolling and code wrapping. Remove the slider completely. Script/text use 8-200 pt; code/images/PDF use 25-400%. Show range/errors beside the input independently of the status bar.
- **Workspace**: activity bar, Explorer, tabs, toolbar, status bar and reset. Preserve existing local layout persistence.
- **Tools**: include heavy/hidden folders and enable/disable web browsing.
- **Presentation**: source display, destination and pointer visibility.
- Scroll vertically when necessary, including at the 430x320 minimum and high DPI. Keep title/navigation outside the viewport and clip controls correctly. Wheel and Tab work over controls; focus automatically reveals the corresponding row. Use a dark native scrollbar.
- Keep VI/EN, units and current states clear. Preserve existing context-dependent disabled states.

### Usage flow

Open **Settings** from the gear icon or tools menu. The tools menu retains working actions and gains a Settings shortcut; configuration lives in the form. Keep Layout, Pin and quick zoom controls accessible from the workspace.

**Presentation** displays a source/destination summary plus **Display settings**, **Verify WGC privacy**, **Start** and **Stop**. **Display settings** jumps directly to the Presentation group in Settings. Display changes retain current verification requirements.

Enabling browsing from Settings preserves that page during initialization and the capture check. The user selects the globe icon to browse after verification succeeds. Browsing still starts disabled, uses a separate profile, closes the engine on disable and requests suspension while reading files.

### Privacy and publication

Duplicate always locks private content. Settings introduces no exception to the canary, display affinity or display-change checks. User web content cannot run before actual browser capture verification; preserve existing device permissions, upload/download, popup and native-surface restrictions.

Git and shipping archives contain only QuietShare source, documentation, illustrative screenshots and product binaries. Exclude personal scripts, browser profiles, cookies, accounts, machine paths and private logs. Keep older GitHub Releases; keep only the current shipping package in the project tree.
