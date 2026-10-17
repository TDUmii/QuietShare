# QuietShare 0.10.0 - Functional specification

## Tiếng Việt

### Phạm vi

Cải thiện truy cập nhanh của trình duyệt, điều khiển trình chiếu và khả năng cá nhân hóa cửa sổ nhỏ. Giữ trình đọc file, Explorer, VI/EN, cơ chế xác minh capture và yêu cầu cài đặt theo tài khoản hiện tại.

### Trình duyệt

- Bỏ các nút Google/ChatGPT cố định. Tab đầu tiên và tab mới mở trang cục bộ có ô tìm kiếm/nhập địa chỉ và tối đa 10 lối tắt do người dùng quản lý.
- Cho thêm tên và URL HTTP/HTTPS, sửa hoặc xóa lối tắt. Lưu lối tắt cùng hồ sơ web cục bộ, dùng chung giữa các tab và giữ khi tắt/bật trình duyệt hoặc khởi động lại ứng dụng.
- Trang Tab mới không yêu cầu tải website ngoài. Chỉ điều hướng hoặc tìm kiếm khi người dùng thực hiện; từ khóa dùng Google Search.
- Căn giữa ô địa chỉ trên hàng điều hướng; bấm vào ô chọn toàn bộ URL để nhập thay thế. Giữ quay lại, tiến tới, tải lại, mở/đóng/chuyển tab và zoom 25-400%.
- Gom thao tác phụ vào menu để dành diện tích cho nội dung trong cửa sổ nhỏ.
- Giữ tối đa 8 tab web với lịch sử và zoom riêng. Không khôi phục danh sách tab sau khi khởi động lại; trình duyệt bắt đầu tắt.
- Giữ xác minh capture từng tab, giới hạn quyền web, chặn popup tự phát và tạm ngưng tab nền. Bật web trong phiên chiếu chỉ tiếp tục cùng phiên sau khi vùng web xác minh đạt; thất bại/hủy dừng an toàn.

### Giao diện và cá nhân hóa

- Dùng icon Microsoft Codicons hiện có với dựng hình mịn theo DPI; giữ kích thước thanh bên gọn và tooltip VI/EN.
- Chia Cài đặt thành năm tab: **Giao diện**, **Nội dung**, **Bố cục**, **Trình chiếu**, **Phím tắt**. Nội dung mỗi tab cuộn độc lập với thanh chọn nhóm.
- Giao diện gồm ngôn ngữ, ghim, độ mờ cửa sổ 40-100% và độ đậm chữ lời nhắc Thường/Vừa/Đậm tương ứng 400/600/700.
- Độ mờ áp dụng bằng alpha đồng đều cho toàn bộ cửa sổ điều khiển, kể cả chữ; không cung cấp nền trong suốt riêng. Đầu ra công khai vẫn đục. Độ đậm lời nhắc không thay đổi font website, code hoặc PDF.
- Lưu thiết lập cục bộ. Khôi phục mặc định tại Giao diện chỉ đặt lại độ mờ/độ đậm; tại Phím tắt chỉ đặt lại các tổ hợp.
- Giữ cỡ chữ/zoom, tốc độ lời nhắc, xuống dòng code, điều khiển bố cục, lọc Explorer và trình duyệt tại tab tương ứng.

### Phím tắt

Cho đổi mười tác vụ sau. Bấm tổ hợp hiện tại rồi nhấn tổ hợp mới, Esc hủy. Từ chối tổ hợp trùng, dành riêng hoặc không hợp lệ. Khi không đăng ký được phím toàn hệ thống, giữ thiết lập trước.

| Tác vụ | Mặc định | Phạm vi |
|---|---|---|
| Che/mở nội dung | F8 | Toàn hệ thống khi đăng ký được, có dự phòng trong ứng dụng |
| Gọi cửa sổ | Ctrl+Alt+Q | Toàn hệ thống khi đăng ký được, giữ focus hiện tại |
| Kiểm tra WGC | F9 | QuietShare |
| Bắt đầu/dừng chiếu | F10 | QuietShare |
| Tập trung | F11 | QuietShare |
| Mở file | Ctrl+O | QuietShare |
| Mở thư mục | Ctrl+Shift+O | QuietShare |
| Ẩn/hiện Explorer | Ctrl+B | QuietShare |
| Soạn/đọc | Ctrl+Enter | QuietShare |
| Cuộn tự động | Space | Vùng lời nhắc ở chế độ đọc |

### Điều khiển và độ ổn định trình chiếu

- Thanh bên tích hợp kiểm tra WGC và một nút bắt đầu/dừng theo trạng thái phiên. Cấu hình nguồn/đích/con trỏ/tài nguyên và trạng thái nằm tại **Cài đặt > Trình chiếu**, thay trang Trình chiếu riêng.
- Kiểm tra WGC trong phiên đang chạy xác nhận các điều kiện hiện tại, giữ capture và đầu chiếu hiện có khi hợp lệ.
- Xử lý thông báo màn hình bằng điều kiện thực tế. Thông báo không đổi topology, capture, thiết bị hoặc bảo vệ không tự kết thúc phiên.
- Giữ kiểm tra affinity cho cửa sổ và popup, DWM, topology, source containment, capture và thiết bị. Lỗi an toàn thực sự dừng đầu chiếu; không tự khởi động lại.
- Giữ cảnh báo gián đoạn không lấy focus. Lưu một bản ghi cục bộ được ghi đè cho lần dừng bất ngờ gần nhất, với mã lý do, thời gian và HRESULT; không lưu hình ảnh, nội dung, URL, đường dẫn hay định danh màn hình.
- Giữ cửa sổ trong màn hình nguồn khi chiếu; thu nhỏ giữ phiên hợp lệ. Che nội dung, dừng, đóng ứng dụng hoặc suspend vẫn kết thúc chiếu.

### Phiên bản và phân phối

Tổng quan, EXE, bộ cài và tài liệu dùng **v0.10.0**. Bộ cài theo tài khoản hiện tại không yêu cầu admin; portable cần toàn bộ DLL đi kèm. Chỉ đưa file dự án/sản phẩm vào Git và gói phát hành; loại trừ nội dung riêng, hồ sơ web và chẩn đoán cục bộ. Giữ các GitHub Releases cũ; cây dự án giữ ZIP hiện hành.

## English

### Scope

Improve browser quick access, projection controls and personalization in a compact window. Preserve file readers, Explorer, VI/EN, capture verification and per-user installation.

### Browser

- Remove fixed Google/ChatGPT buttons. Initial and new tabs open a local page with a search/address input and up to 10 user-managed shortcuts.
- Add a name and HTTP/HTTPS URL, edit or remove shortcuts. Persist them with the local browser profile, share across tabs and retain across browser/application restarts.
- New tab does not require an external website request. Navigate or search only after user action; search terms use Google Search.
- Center the address field in the navigation row; clicking selects the complete URL for replacement. Preserve Back, Forward, Reload, tab controls and 25-400% zoom.
- Group secondary actions in a menu to leave more room for content in small windows.
- Retain up to 8 web tabs with independent history and zoom. Do not restore the session tab list after application restart; browsing starts disabled.
- Preserve per-tab capture verification, web permission limits, unsolicited-popup blocking and background suspension. Enabling web during projection continues the same session only after successful web verification; failure/cancellation stops safely.

### Interface and personalization

- Render existing Microsoft Codicons smoothly at the current DPI; retain the compact rail and VI/EN tooltips.
- Organize Settings into **Appearance**, **Reading**, **Workspace**, **Projection** and **Shortcuts**. Scroll category content independently of the category selector.
- Appearance includes language, pinning, 40-100% window opacity and Regular/Semibold/Bold script weight, corresponding to 400/600/700.
- Opacity uses uniform alpha for the entire control window, including text; background-only transparency is not provided. Public output remains opaque. Script weight does not change website, code or PDF fonts.
- Persist choices locally. Appearance defaults reset opacity/weight only; Shortcut defaults reset bindings only.
- Retain size/zoom, script scrolling, code wrapping, workspace controls, Explorer filtering and browser enablement in their corresponding tabs.

### Keyboard shortcuts

Allow ten commands to be remapped. Click a current binding, then press its replacement; Esc cancels. Reject duplicate, reserved or invalid combinations. Failed global registration retains the previous setting.

| Command | Default | Scope |
|---|---|---|
| Mask/reveal | F8 | Global when registered, with application fallback |
| Recall window | Ctrl+Alt+Q | Global when registered, preserving current focus |
| Verify WGC | F9 | QuietShare |
| Start/stop projection | F10 | QuietShare |
| Focus | F11 | QuietShare |
| Open files | Ctrl+O | QuietShare |
| Open folder | Ctrl+Shift+O | QuietShare |
| Toggle Explorer | Ctrl+B | QuietShare |
| Edit/read | Ctrl+Enter | QuietShare |
| Auto-scroll | Space | Script reading area |

### Projection controls and stability

- Place WGC verification and a state-aware start/stop action on the activity bar. Keep source/destination/pointer/resources and status in **Settings > Projection**, replacing the separate Presentation page.
- A WGC check during active projection validates current conditions and retains the existing capture/output while valid.
- Handle display notifications using actual invariants. Notifications that do not change topology, capture, device or privacy conditions do not independently end a session.
- Preserve window/popup affinity, DWM, topology, source containment, capture and device checks. Actual safety failures stop output without automatic restart.
- Preserve non-focus-stealing interruption alerts. Overwrite one local last-unexpected-stop record containing a reason code, timing and HRESULT; exclude images, content, URLs, paths and display identifiers.
- Keep the control window on the source display during projection; minimizing retains a healthy session. Mask, Stop, application closure and suspend retain their projection-stop behavior.

### Version and distribution

Overview, EXE, installer and documentation use **v0.10.0**. The per-user installer needs no administrator privileges; portable use requires all bundled DLLs. Git and release payloads include only project/product files, excluding private content, browser profiles and local diagnostics. Preserve older GitHub Releases and retain the current ZIP in the project tree.
