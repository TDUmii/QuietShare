# QuietShare 0.7.0 - Settings

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

### Nghiệm thu

1. Không còn slider cỡ chữ trong UI hay luồng điều khiển. Nhập cỡ hợp lệ, ngoài phạm vi và dùng Đặt lại với script/text/code/ảnh/PDF cho kết quả đúng; lỗi vẫn đọc được khi thanh trạng thái đã ẩn.
2. Mọi cấu hình hiện có xuất hiện trong đúng nhóm, cập nhật cùng state với các lối tắt nhanh. Bật/tắt thanh, đổi ngôn ngữ và reset không làm hỏng form hoặc vùng cuộn.
3. Kiểm tra VI/EN, cửa sổ 430x320 và các mức DPI 96/120/144/192. Không có nhãn/control chồng nhau hoặc tràn lên tiêu đề/footer. Cuộn tới cuối rồi về đầu không để sót region clip trên màn hình khác.
4. Tab/Shift+Tab tới từng control đưa đúng hàng vào vùng nhìn thấy; con lăn, kéo thumb và phím cuộn không ảnh hưởng nội dung file phía sau.
5. Chọn nguồn/đích trong Cài đặt, mở lại Trình chiếu và kiểm tra tóm tắt. Lối tắt Thiết lập màn hình tới đúng nhóm; Verify/Start/Stop giữ hành vi hiện tại.
6. Bật/tắt browser từ Cài đặt không tự chuyển trang và không bỏ qua kiểm tra vùng web. F8, Ctrl+Alt+Q, picker, viewer, Explorer và Smart Mirror tiếp tục hoạt động qua cùng privacy gate.
7. Build, kiểm thử native, kiểm thử cài/nâng cấp/gỡ trong vùng dữ liệu thử riêng, đóng gói và kiểm tra tính nhất quán phiên bản 0.7.0 của sản phẩm/tài liệu/Release.

### Bảo vệ nội dung và phát hành

Duplicate luôn khóa nội dung riêng. Cài đặt không tạo ngoại lệ cho canary, display affinity hoặc thay đổi màn hình. Không cho nội dung web của người dùng chạy trước khi kiểm tra capture thực tế đạt; giữ các hạn chế quyền thiết bị, upload/download, popup và giao diện native hiện tại.

Git và gói phát hành chỉ chứa mã nguồn, tài liệu, ảnh minh họa và binary liên quan tới QuietShare. Không đưa lời nhắc cá nhân, profile trình duyệt, cookie, tài khoản, đường dẫn máy hoặc log riêng vào dự án. Giữ các GitHub Releases cũ, thay ZIP và bằng chứng kiểm thử đã cũ trong cây hiện tại bằng bản 0.7.0.

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

### Acceptance

1. No text-size slider remains in the interface or control flow. Valid/out-of-range manual input and Reset work for script/text/code/images/PDF. Errors remain visible with the status bar hidden.
2. Every existing setting appears in its group and shares state with quick controls. Bar toggles, language changes and reset preserve the form and scrolling.
3. Check VI/EN, 430x320 windows and DPI 96/120/144/192. No labels/controls overlap or cross the title/footer. Scrolling and leaving Settings do not leave clipping regions on other pages.
4. Tab/Shift+Tab reveal each focused row; wheel, scrollbar dragging and scroll keys do not move file content behind Settings.
5. Set source/destination in Settings, return to Presentation and confirm its summary. Display settings reaches the correct group; Verify/Start/Stop retain their behavior.
6. Browser toggling in Settings preserves the page and browser capture check. F8, Ctrl+Alt+Q, pickers, viewers, Explorer and Smart Mirror keep the shared privacy gate.
7. Build, run native and isolated install/upgrade/uninstall checks, package the product, and verify matching 0.7.0 binaries, documentation and Release.

### Privacy and publication

Duplicate always locks private content. Settings introduces no exception to the canary, display affinity or display-change checks. User web content cannot run before actual browser capture verification; preserve existing device permissions, upload/download, popup and native-surface restrictions.

Git and shipping archives contain only QuietShare source, documentation, illustrative screenshots and product binaries. Exclude personal scripts, browser profiles, cookies, accounts, machine paths and private logs. Keep older GitHub Releases; replace superseded ZIPs and test evidence in the current tree with version 0.7.0.
