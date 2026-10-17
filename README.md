# QuietShare 0.9.1

QuietShare là ứng dụng Windows để đọc lời nhắc, mã nguồn, ảnh, PDF và nội dung web trong cửa sổ riêng khi thuyết trình. Smart Mirror chiếu hình ảnh công khai từ màn hình nguồn sang màn hình Extend khác, trong khi cửa sổ QuietShare được loại khỏi đường capture đã xác minh.

QuietShare is a Windows application for scripts, source code, images, PDFs and optional web content during presentations. Smart Mirror projects the public source display onto a separate Extend display while excluding the QuietShare window from the verified capture path.

![Cài đặt và nút Ghim / Settings and Pin control](docs/quietshare-settings.png)

## Tiếng Việt

### 1. Chức năng

| Thành phần | Khả năng |
|---|---|
| Lời nhắc | Soạn/đọc, cuộn tự động với ba tốc độ, tự lưu cục bộ bằng Windows DPAPI. |
| Tài liệu | Tối đa 32 tab gồm lời nhắc; giữ vị trí cuộn, zoom và trang PDF theo tab trong phiên làm việc. File mở để xem chỉ đọc, không ghi đè lời nhắc. |
| Mã nguồn | Màu cú pháp, số dòng, thụt dòng, tìm kiếm, chọn/sao chép và xuống dòng tùy chọn. Không thực thi mã nguồn. |
| Explorer | Cây thư mục cục bộ, icon theo loại file, giữ nhánh mở và lựa chọn khi đổi tab; kéo đường phân cách để chỉnh độ rộng. |
| Ảnh và PDF | Zoom, di chuyển ảnh/trang, PDF từng trang; tìm và sao chép khi PDF có lớp văn bản. |
| Trình duyệt | Bật theo nhu cầu, tối đa 8 tab web; địa chỉ HTTP/HTTPS, tên miền, localhost và tìm Google; lịch sử điều hướng và zoom 25-400% riêng từng tab. |
| Bố cục | Ẩn/hiện riêng từng thanh, chế độ tập trung, ghim trên cùng, VI/EN và hỗ trợ DPI. |
| Smart Mirror | Chọn màn hình nguồn/đích, con trỏ và chế độ tài nguyên; đầu chiếu chỉ khởi động sau khi xác minh capture đạt. |

### 2. Yêu cầu hệ thống và định dạng

- Windows 10 phiên bản 2004 (build 19041) trở lên hoặc Windows 11, kiến trúc x64. DWM và Windows Graphics Capture phải hoạt động.
- Smart Mirror yêu cầu ít nhất hai màn hình khác nhau trong **Extend**, cùng GPU. QuietShare không tự thay đổi chế độ màn hình của Windows.
- Trình đọc file hoạt động offline. Trình duyệt cần [Microsoft Edge WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) hiện hành; các trình đọc file vẫn dùng được khi chưa cài runtime.

| Nội dung | Định dạng và giới hạn |
|---|---|
| Văn bản/mã nguồn | UTF-8 hoặc UTF-16 có BOM; tối đa 5 MiB. Nhận TXT, MD, LOG, C/C++, C#, Java, Python, JS/TS, HTML/CSS, JSON/XML/YAML, SQL, SH/PS1, Rust, Go và định dạng code liên quan. |
| File chưa nhận diện | Xem dưới dạng văn bản monospace nếu encoding hợp lệ và không có dữ liệu nhị phân. Tệp thực thi, gói cài đặt và file nhị phân như BIN/ZIP bị từ chối. |
| Ảnh | PNG, JPG/JPEG, BMP, khung đầu GIF; tối đa 25 MiB và 40 megapixel. |
| PDF | Tối đa 64 MiB; PDF scan xem được nhưng tìm/sao chép cần lớp chữ. PDF có mật khẩu chưa được hỗ trợ. |
| Đường dẫn | File và thư mục trên ổ cục bộ. Explorer bỏ qua junction/symlink và giới hạn duyệt thư mục lớn. |

### 3. Cài đặt và nâng cấp

1. Tải gói tại [QuietShare v0.9.1](https://github.com/TDUmii/QuietShare/releases/tag/v0.9.1), giải nén toàn bộ và chạy `QuietShare-Setup.exe`.
2. Bộ cài áp dụng cho tài khoản Windows hiện tại, không cần quyền quản trị. Nút **EN / VI** đổi ngôn ngữ bộ cài.
3. Để dùng portable, chạy `QuietShare.exe` trong thư mục đã giải nén. Giữ `Scintilla.dll`, `Lexilla.dll`, `pdfium.dll` và `WebView2Loader.dll` cạnh EXE; không chỉ sao chép EXE.
4. Đóng QuietShare trước khi nâng cấp bằng bộ cài mới. Nâng cấp và gỡ ứng dụng giữ lại lời nhắc đã lưu.

Nếu trình duyệt không khởi tạo được, cài/cập nhật WebView2 Runtime rồi tắt và bật lại trình duyệt trong QuietShare.

### 4. Sử dụng tài liệu và không gian làm việc

Khi mở ứng dụng, đợi quá trình xác minh capture. Góc trên trang **Tổng quan** hiển thị **v0.9.1** để nhận diện phiên bản đang chạy. Nếu nội dung bị khóa và cần xác minh lại, chọn **Kiểm tra WGC** tại **Tổng quan** hoặc **Trình chiếu**. Nội dung chỉ được hiển thị khi các điều kiện bảo vệ đạt.

- **Lời nhắc:** mở trang Lời nhắc để nhập hoặc dán nội dung. **Ctrl+Enter** chuyển soạn/đọc; **Space** trong vùng đọc bắt đầu/dừng cuộn. Lời nhắc được tự lưu và khôi phục ở lần mở tiếp theo.
- **File:** chọn **Mở hoặc thả file**, **Ctrl+O** hoặc kéo thả nhiều file. Dấu **×**, chuột giữa hoặc **Ctrl+W** đóng tab file. Menu **⋯** có lệnh đóng các tab khác/mọi file; tab lời nhắc được giữ. Khi mở lại file đã thay đổi, chọn **Tải lại** hoặc **Giữ bản đang xem**.
- **Dự án:** dùng **Ctrl+Shift+O** để chọn thư mục. Nhấp đúp file hoặc dùng mũi tên và Enter trong Explorer. **Ctrl+B** hoặc **Bố cục > Explorer** ẩn/hiện cùng cây đã mở. Mặc định bỏ qua thư mục ẩn và `.git`, `node_modules`, `.venv`, `dist`, `build`; đổi bộ lọc trong Cài đặt.
- **Cỡ chữ/zoom:** dùng các nút thu/phóng, phím tắt hoặc **Cài đặt > Đọc nội dung**. Nhập giá trị rồi Enter; **Đặt lại** khôi phục mặc định. Ô nhập đặt dấu nháy ở cuối khi nhận focus/bấm lần đầu.
- **Tìm kiếm:** **Ctrl+F** mở thanh tìm. Enter tìm tiếp, Shift+Enter tìm trước, Esc đóng thanh. Áp dụng cho lời nhắc, văn bản, code và PDF có lớp chữ.
- **PDF:** dùng menu **⋯** để đổi trang hoặc chọn vừa chiều ngang. Khi vùng PDF nhận focus, Page Up/Page Down đổi trang; kéo vùng trống để di chuyển, kéo trên chữ để chọn và Ctrl+C để sao chép.
- **Bố cục:** nút **Bố cục** ở phía trên bật/tắt thanh icon, Explorer, tab, công cụ và trạng thái. **Khôi phục bố cục** khôi phục các thanh và độ rộng Explorer. Lựa chọn được lưu trên máy. **F11** vào/thoát chế độ tập trung.

Nút **Ghim** ở góc trên điều khiển trạng thái luôn trên cùng; cùng thiết lập có tại **Cài đặt > Hiển thị**. Kéo thanh tiêu đề hoặc vùng trống phía trên để di chuyển, kéo cạnh cửa sổ để đổi kích thước. Nút **-** thu nhỏ cửa sổ QuietShare và giữ phiên Smart Mirror đang chạy. **F8** là thao tác che nội dung và dừng chiếu; **Dừng** kết thúc phiên chiếu; đóng ứng dụng giải phóng đầu chiếu.

**Ctrl+Alt+Q** gọi lại cửa sổ bị che hoặc thu nhỏ, kể cả khi không ghim, và giữ focus ở ứng dụng đang dùng. Phím tắt không mở Alt+Tab và không bỏ trạng thái che F8. Rê chuột vào Ghim để xem khả năng đăng ký phím tắt.

### 5. Cài đặt

Mở bằng icon bánh răng hoặc **⋯ > Cài đặt**. Dùng con lăn, touchpad, thanh cuộn hoặc Tab/Shift+Tab; hàng nhận focus tự đi vào vùng nhìn thấy.

| Nhóm | Thiết lập |
|---|---|
| Hiển thị | Ngôn ngữ VI/EN, ghim cửa sổ trên cùng. |
| Đọc nội dung | Cỡ chữ lời nhắc/văn bản 8-200 pt; zoom code/ảnh/PDF 25-400%; đặt lại, tốc độ cuộn lời nhắc, xuống dòng code. |
| Bố cục | Bật/tắt năm thanh và khôi phục bố cục. |
| Tiện ích | Hiện thư mục nặng/ẩn; bật/tắt trình duyệt. |
| Hiệu suất | Tài nguyên: Tiết kiệm 15 FPS, Cân bằng 30 FPS, Hiệu năng 60 FPS mặc định. |
| Trình chiếu | Màn hình nguồn, màn hình đích, hiển thị con trỏ. |

Chế độ tài nguyên giới hạn tốc độ xử lý chiếu của ứng dụng; FPS thực tế phụ thuộc màn hình, GPU và nội dung. Không có mức giảm RAM cố định. Trình duyệt ẩn vẫn nhận yêu cầu tạm ngưng; tắt trình duyệt đóng engine và giữ hồ sơ đăng nhập riêng trên máy.

### 6. Trình duyệt

Bật tại **Cài đặt > Tiện ích > Trình duyệt web**, đợi xác minh vùng web rồi chọn icon quả địa cầu. Có thể bật/tắt trực tiếp trên trang trình duyệt. Trình duyệt luôn tắt ở mỗi lần khởi động ứng dụng.

Nhập URL, tên miền, địa chỉ như `localhost:3000` hoặc từ khóa rồi Enter/**Mở**. Khi cửa sổ đủ rộng, nút **Google** và **ChatGPT** mở trang tương ứng; ở bố cục hẹp dùng thanh địa chỉ. **- / phần trăm / +** thay đổi zoom 25-400%; bấm phần trăm để về 100%. Zoom web độc lập với tab file.

Dùng **+** hoặc **Ctrl+T** để mở tab web mới tại Google; chọn tiêu đề tab để chuyển trang. Dấu **×** hoặc **Ctrl+W** đóng tab đang chọn. Đóng tab web cuối cùng sẽ tắt trình duyệt, giữ QuietShare và các tab tài liệu đang mở. **Ctrl+Tab / Ctrl+Shift+Tab** chuyển tab web tiếp/trước khi ở trang trình duyệt, kể cả lúc đang nhập trong trang web.

Mỗi tab giữ trang đang mở, lịch sử lùi/tiến và mức zoom riêng trong phiên; thanh địa chỉ và nút điều hướng luôn áp dụng cho tab đang chọn. Các tab dùng chung hồ sơ web cục bộ nên cùng dùng trạng thái đăng nhập. Danh sách tab không được khôi phục ở lần khởi động tiếp theo. Tab nền và trình duyệt ẩn nhận yêu cầu tạm ngưng; việc tạm ngưng thực tế phụ thuộc trang web và WebView2.

Mỗi tab mới phải hoàn tất xác minh capture của vùng web trước khi mở trang người dùng; chỉ tạo/xác minh một tab mới tại một thời điểm. Chuyển giữa các tab đã xác minh không yêu cầu kiểm tra lại. Giới hạn **8 tab** áp dụng cho cả nút mở tab và link yêu cầu tab mới; đóng một tab trước khi mở thêm khi đã đạt giới hạn.

Khi bật web trong phiên Smart Mirror, đầu chiếu giữ khung hình công khai gần nhất trong lúc xác minh, tối đa khoảng 5 giây. Xác minh đạt thì tiếp tục cùng phiên; thất bại hoặc hủy thì dừng chiếu và khóa nội dung.

Trình duyệt dành cho đọc và chat bằng chữ. Tải lên/tải xuống file, camera, microphone, chia sẻ màn hình, thông báo, DevTools và menu native bị tắt. Popup tự phát bị chặn; link mở cửa sổ mới do người dùng bấm được mở thành tab trong QuietShare nếu còn chỗ. Đăng nhập OAuth cần popup riêng có thể không tương thích. Dropdown HTML native có thể chọn bằng phím mũi tên. Thay đổi màn hình tắt trình duyệt và yêu cầu xác minh lại.

### 7. Smart Mirror và giới hạn bảo vệ

1. Chọn **Win+P > Extend** trong Windows và đặt toàn bộ QuietShare trên màn hình nguồn.
2. Mở **Cài đặt > Trình chiếu**. Chọn nguồn và đích khác nhau, cùng tùy chọn con trỏ.
3. Dùng nhãn **Màn N · Chính · Cửa sổ này · độ phân giải** để nhận biết màn hình. Nhãn vai trò chỉ hiện khi áp dụng; số 1 không nhất thiết là màn chính. Di chuyển QuietShare cập nhật nhãn mà không tự đổi lựa chọn nguồn/đích.
4. Mở **Trình chiếu**, chọn **Kiểm tra WGC**, sau đó **Bắt đầu**. **Thiết lập màn hình** quay thẳng tới nhóm cài đặt tương ứng. Nút **-** có thể thu nhỏ QuietShare trong khi tiếp tục chiếu.
5. Chọn **Dừng** để kết thúc. **F8** che mọi nội dung, tên tab và Explorer, đồng thời dừng cuộn và chiếu. F8 lần nữa mở lại nội dung khi các điều kiện bảo vệ cho phép; cần bắt đầu lại nếu muốn tiếp tục chiếu.

Phiên Smart Mirror không còn tự hết hạn theo mốc 5 phút. Phiên tiếp tục khi đường capture, cấu hình màn hình và các điều kiện bảo vệ vẫn hợp lệ; thu nhỏ QuietShare không kết thúc phiên. Khi đang chiếu, QuietShare yêu cầu Windows giữ máy và màn hình hoạt động để tránh ngủ do không có thao tác. Lệnh ngủ chủ động hoặc đóng nắp máy vẫn được xử lý theo Windows và dừng chiếu khi hệ thống suspend.

Nếu phiên chiếu bị gián đoạn ngoài thao tác chủ động, tiêu đề cửa sổ chuyển thành **QuietShare - Trình chiếu đã dừng**, thanh trạng thái hiển thị cảnh báo màu hổ phách khi đang hiện, đồng thời ứng dụng nháy taskbar và phát một âm báo Windows nếu âm thanh hệ thống cho phép. Cảnh báo không giành focus, mở cửa sổ mới hoặc tự khôi phục cửa sổ đang thu nhỏ. Mở **Trình chiếu** để xem trạng thái; khi cần, dùng **Kiểm tra WGC** rồi **Bắt đầu** để tiếp tục. Ứng dụng không tự khởi động lại đầu chiếu sau khi mất điều kiện bảo vệ. Cảnh báo được xóa khi bắt đầu chiếu thành công hoặc chủ động dùng **Dừng/F8**; các thao tác chủ động này không phát cảnh báo gián đoạn.

**Duplicate luôn khóa nội dung riêng.** Xác minh thất bại, thay đổi màn hình, suspend hoặc capture mất an toàn sẽ dừng chiếu và khóa nội dung. Trạng thái **Windows đã nhận** xác nhận API loại trừ capture; **người xem chưa được xác minh** yêu cầu quan sát phía người nhận trong đúng Zoom/Meet/Teams/OBS hoặc máy chiếu sử dụng. Xác nhận bằng nội dung mẫu trước buổi trình bày. Phần mềm không ngăn máy ảnh bên ngoài quay màn hình.

### 8. Phím tắt

Phím **toàn hệ thống** dùng được khi ứng dụng khác đang nhận focus nếu QuietShare đăng ký thành công. Các phím còn lại áp dụng khi vùng tương ứng trong QuietShare nhận focus. Các phím quản lý tab web trong bảng được QuietShare xử lý cả khi nội dung website nhận focus.

| Phím | Thao tác | Phạm vi |
|---|---|---|
| F8 | Che/mở lại nội dung; khi che sẽ dừng cuộn và Smart Mirror | Toàn hệ thống khi đăng ký được; dự phòng trong QuietShare |
| Ctrl+Alt+Q | Gọi lại cửa sổ, giữ focus hiện tại và trạng thái F8 | Toàn hệ thống khi đăng ký được |
| F11 | Vào/thoát tập trung | QuietShare |
| Esc | Đóng tìm kiếm; nếu không tìm kiếm thì thoát tập trung | QuietShare |
| Ctrl+O | Chọn nhiều file | QuietShare |
| Ctrl+Shift+O | Chọn thư mục dự án | QuietShare |
| Ctrl+B | Ẩn/hiện Explorer | QuietShare |
| Ctrl+Tab / Ctrl+Shift+Tab | Tab tài liệu tiếp/trước | Trang nội dung |
| Ctrl+W | Đóng tab file đang chọn; giữ tab lời nhắc | Trang nội dung |
| Ctrl+T | Mở tab web tại Google, tối đa 8 tab | Trang trình duyệt, gồm nội dung web |
| Ctrl+Tab / Ctrl+Shift+Tab | Tab web tiếp/trước | Trang trình duyệt, gồm nội dung web |
| Ctrl+W | Đóng tab web; tab cuối cùng sẽ tắt trình duyệt | Trang trình duyệt, gồm nội dung web |
| Ctrl+Enter | Chuyển soạn/đọc lời nhắc | QuietShare |
| Space | Chạy/dừng cuộn lời nhắc | Vùng lời nhắc ở chế độ đọc |
| Ctrl+F | Mở tìm kiếm | Lời nhắc, văn bản, code, PDF có lớp chữ |
| Enter / Shift+Enter | Tìm tiếp/trước | Ô tìm kiếm |
| Ctrl+C | Sao chép phần được chọn | Nội dung có thể chọn |
| Ctrl + `+` / `-` | Tăng/giảm cỡ chữ hoặc zoom | Nội dung đang dùng; web có zoom riêng |
| Ctrl+0 | Về cỡ mặc định; ảnh/PDF vừa khung; web 100% | Nội dung đang dùng |
| Ctrl + lăn chuột | Đổi cỡ chữ/zoom | Vùng nội dung hoặc web dưới con trỏ |
| Page Up / Page Down | Trang PDF trước/sau | Vùng PDF |
| Mũi tên / Enter | Điều hướng/mở file hoặc mở/đóng nhánh | Explorer |
| Tab / Shift+Tab | Control tiếp/trước; tự cuộn hàng vào vùng nhìn thấy | Cài đặt |
| Alt+F4 | Đóng ứng dụng và dừng chiếu | Cửa sổ QuietShare đang hoạt động |


## English

### 1. Capabilities

| Component | Capability |
|---|---|
| Script | Editing/reading, three auto-scroll speeds and local Windows DPAPI autosave. |
| Documents | Up to 32 tabs including the script; scroll, zoom and PDF page retained per tab during the session. File previews are read-only and do not replace the script. |
| Source code | Syntax coloring, line numbers, indentation, find, selection/copy and optional wrapping. Source code is never executed. |
| Explorer | Local folder tree with file-type icons; expanded branches and selection survive tab changes. Drag the divider to resize. |
| Images and PDFs | Zoom and pan, single-page PDF navigation, search/copy when a text layer exists. |
| Browser | Up to 8 on-demand web tabs; HTTP/HTTPS, domains, localhost and Google search; independent navigation history and 25-400% zoom per tab. |
| Workspace | Independent bar visibility, focus mode, always-on-top pinning, VI/EN and DPI support. |
| Smart Mirror | Source/destination display selection, pointer option and resource modes; projection starts only after capture verification succeeds. |

### 2. System requirements and formats

- Windows 10 version 2004 (build 19041) or later, or Windows 11, x64. DWM and Windows Graphics Capture must be available.
- Smart Mirror requires at least two different **Extend** displays on the same GPU. QuietShare does not change Windows display topology automatically.
- File readers work offline. Browsing requires the current [Microsoft Edge WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/); file readers remain available without it.

| Content | Formats and limits |
|---|---|
| Text/source code | UTF-8 or BOM-marked UTF-16; up to 5 MiB. Includes TXT, MD, LOG, C/C++, C#, Java, Python, JS/TS, HTML/CSS, JSON/XML/YAML, SQL, SH/PS1, Rust, Go and related source formats. |
| Unrecognized files | Monospace text when encoding is valid and no binary data is present. Executables, installers and binary formats such as BIN/ZIP are rejected. |
| Images | PNG, JPG/JPEG, BMP and the first GIF frame; up to 25 MiB and 40 megapixels. |
| PDF | Up to 64 MiB. Scans can be displayed; search/copy requires a text layer. Password-protected PDFs are unsupported. |
| Paths | Files/folders on local drives. Explorer skips junctions/symlinks and bounds large folder enumeration. |

### 3. Installation and upgrades

1. Download [QuietShare v0.9.1](https://github.com/TDUmii/QuietShare/releases/tag/v0.9.1), extract the complete package and run `QuietShare-Setup.exe`.
2. Installation applies to the current Windows account without administrator privileges. **EN / VI** switches the installer language.
3. For portable use, run `QuietShare.exe` in the extracted folder. Keep `Scintilla.dll`, `Lexilla.dll`, `pdfium.dll` and `WebView2Loader.dll` beside it; do not copy only the EXE.
4. Close QuietShare before upgrading. Upgrades and uninstallation retain the saved script.

If browsing cannot initialize, install/update WebView2 Runtime, then disable and re-enable browsing in QuietShare.

### 4. Documents and workspace

Wait for capture verification at startup. The upper corner of **Overview** displays **v0.9.1** to identify the running version. If content is locked and verification is requested, select **Verify WGC privacy** on **Overview** or **Presentation**. Readers become available only after the protection conditions pass.

- **Script:** open Script to type or paste content. **Ctrl+Enter** switches editing/reading; **Space** in the reading area starts/stops auto-scroll. The script is saved automatically and restored on the next launch.
- **Files:** use **Open or drop files**, **Ctrl+O** or a multi-file drop. **×**, middle-click and **Ctrl+W** close file tabs. The **⋯** menu closes other/all files while keeping the script. Reopening a changed file offers **Reload** or **Keep current preview**.
- **Projects:** use **Ctrl+Shift+O** to select a folder. Double-click a file, or use arrows and Enter in Explorer. **Ctrl+B** or **Layout > Explorer** hides/shows the same tree. Hidden folders and `.git`, `node_modules`, `.venv`, `dist`, `build` are ignored by default; change the filter in Settings.
- **Text size/zoom:** use quick controls, shortcuts or **Settings > Reading**. Enter a value and press Enter; **Reset** restores the default. The field places an unselected caret at the end on first focus/click.
- **Find:** **Ctrl+F** opens search; Enter finds next, Shift+Enter previous, Esc closes it. Supports scripts, text, code and PDFs with text layers.
- **PDF:** use **⋯** for page navigation or fit-width. With the PDF area focused, Page Up/Page Down changes pages; drag blank space to pan, or drag text to select and Ctrl+C to copy.
- **Layout:** the upper **Layout** control toggles the activity bar, Explorer, tabs, toolbar and status bar independently. **Reset layout** restores default bars and Explorer width. Choices are saved locally. **F11** enters/exits focus mode.

The **Pin** button in the upper corner toggles always-on-top; the same setting is available under **Settings > Appearance**. Drag the title bar or blank top space to move, and window edges to resize. **-** minimizes QuietShare while keeping an active Smart Mirror projection running. **F8** masks content and stops projection; **Stop** ends the projection; closing the application releases the projector.

**Ctrl+Alt+Q** recalls a covered or minimized window, including when unpinned, while preserving the current application's focus. It does not open Alt+Tab or remove F8 masking. Hover Pin to see shortcut registration availability.

### 5. Settings

Open the gear icon or **⋯ > Settings**. Navigate with the wheel, touchpad, scrollbar or Tab/Shift+Tab; focused rows automatically scroll into view.

| Group | Controls |
|---|---|
| Appearance | VI/EN language and always-on-top pinning. |
| Reading | Script/text 8-200 pt; code/image/PDF zoom 25-400%; reset, script scrolling speed and code wrapping. |
| Workspace | Five bar visibility controls and layout reset. |
| Tools | Include heavy/hidden folders; enable/disable browsing. |
| Performance | Resources: Saver 15 FPS, Balanced 30 FPS and default Performance 60 FPS. |
| Presentation | Source display, destination display and pointer visibility. |

Resource modes cap application projection processing. Achieved FPS depends on displays, GPU and source activity; no fixed RAM reduction is promised. Hidden browsing still requests suspension. Disabling browsing closes the engine while retaining its separate local login profile.

### 6. Browser

Enable **Settings > Tools > Web browser**, wait for web capture verification and select the globe icon. Browsing can also be enabled/disabled on its own page. Every application launch starts with browsing disabled.

Enter a URL, domain, address such as `localhost:3000`, or search terms, then press Enter/**Go**. Wider windows show **Google** and **ChatGPT** shortcuts; use the address field in compact layouts. **- / percentage / +** controls 25-400% zoom; click the percentage to reset to 100%. Browser zoom is independent of file tabs.

Use **+** or **Ctrl+T** to open a web tab at Google; select a tab title to switch pages. **×** or **Ctrl+W** closes the selected tab. Closing the last web tab disables browsing while keeping QuietShare and its document tabs open. **Ctrl+Tab / Ctrl+Shift+Tab** selects the next/previous web tab on the browser page, including while typing in web content.

Each tab retains its page, back/forward history and zoom during the session. The address field and navigation controls always apply to the selected tab. Tabs share the local web profile, including login state. The tab list is not restored on the next application launch. Background tabs and hidden browsing request suspension; actual suspension depends on the page and WebView2.

Each new tab must complete web capture verification before opening a user page; only one new tab can be created/verified at a time. Switching between verified tabs does not require a fresh check. The **8-tab limit** applies to both the new-tab control and links requesting a new tab. Close an existing tab before opening another when the limit is reached.

Enabling web during Smart Mirror holds the most recent public frame while verification runs, for up to about five seconds. Success resumes the same projection; failure or cancellation stops projection and locks content.

The browser supports reading and text chat. File uploads/downloads, camera, microphone, screen sharing, notifications, DevTools and native context menus are disabled. Unsolicited popups are blocked; user-initiated new-window links open in a QuietShare tab if capacity is available. OAuth flows requiring separate popups may be incompatible. Native HTML selects support arrow-key selection. Display changes disable browsing until fresh verification.

### 7. Smart Mirror and protection limits

1. Select **Win+P > Extend** in Windows and place the entire QuietShare window on the source display.
2. Open **Settings > Presentation**. Select different source/destination displays and the pointer option.
3. Identify displays with **Display N · Main · This window · resolution**. Role labels appear only when applicable; Display 1 is not necessarily primary. Moving QuietShare updates labels without automatically changing the selected source/destination.
4. Open **Presentation**, select **Verify WGC privacy**, then **Start**. **Display settings** jumps to its Settings group. **-** can minimize QuietShare while projection continues.
5. Select **Stop** to finish. **F8** masks viewers, tab names and Explorer, and stops auto-scroll and projection. Pressing F8 again reveals content when protection conditions allow; restart projection explicitly if needed.

Smart Mirror sessions no longer expire at a fixed five-minute boundary. A session continues while the capture path, display configuration and protection conditions remain valid; minimizing QuietShare does not end it. During projection, QuietShare requests that Windows keep the system and display awake to avoid idle sleep. Intentional sleep commands and lid closure retain Windows behavior and stop projection when the system suspends.

If projection is interrupted without an intentional stop, the window title changes to **QuietShare - Projection stopped** and the status bar shows an amber warning when visible. QuietShare also flashes its taskbar button and requests one Windows warning sound, subject to system sound settings. The warning does not take focus, open another window or restore a minimized window. Open **Presentation** to inspect the status; use **Verify WGC privacy**, then **Start**, when required to resume. The application does not restart projection automatically after protection conditions are lost. A successful Start or intentional **Stop/F8** clears the warning; intentional stop actions do not trigger an interruption alert.

**Duplicate always locks private content.** Failed verification, display changes, suspend or unsafe capture stop projection and lock readers. **Windows accepted** confirms the capture-exclusion API state; **Viewer unverified** requires receiver-side observation in the actual Zoom/Meet/Teams/OBS or projector setup. Confirm with sample content before a presentation. Software cannot prevent an external camera from filming a display.

### 8. Keyboard shortcuts

**Global** shortcuts work while another application has focus if QuietShare registers them successfully. Other shortcuts require the relevant QuietShare area to have focus. QuietShare handles the web tab shortcuts listed below even when web content has focus.

| Shortcut | Action | Scope |
|---|---|---|
| F8 | Mask/reveal content; masking stops auto-scroll and Smart Mirror | Global when registered; local fallback |
| Ctrl+Alt+Q | Recall the window; preserve focus and F8 masking | Global when registered |
| F11 | Enter/exit focus mode | QuietShare |
| Esc | Close search; otherwise exit focus mode | QuietShare |
| Ctrl+O | Open multiple files | QuietShare |
| Ctrl+Shift+O | Open a project folder | QuietShare |
| Ctrl+B | Show/hide Explorer | QuietShare |
| Ctrl+Tab / Ctrl+Shift+Tab | Next/previous document tab | Content page |
| Ctrl+W | Close the selected file tab; preserve Script | Content page |
| Ctrl+T | Open a Google web tab, up to 8 tabs | Browser page, including web content |
| Ctrl+Tab / Ctrl+Shift+Tab | Next/previous web tab | Browser page, including web content |
| Ctrl+W | Close a web tab; closing the last disables browsing | Browser page, including web content |
| Ctrl+Enter | Switch script editing/reading | QuietShare |
| Space | Start/stop script auto-scroll | Script reading area |
| Ctrl+F | Open find | Script, text, code, searchable PDF |
| Enter / Shift+Enter | Find next/previous | Search field |
| Ctrl+C | Copy selection | Selectable content |
| Ctrl + `+` / `-` | Increase/decrease text size or zoom | Current content; independent web zoom |
| Ctrl+0 | Default text size; fit images/PDFs; web 100% | Current content |
| Ctrl + wheel | Change text size/zoom | Content area or browser under the pointer |
| Page Up / Page Down | Previous/next PDF page | PDF area |
| Arrows / Enter | Navigate/open a file or expand/collapse a folder | Explorer |
| Tab / Shift+Tab | Next/previous control; reveal the focused row | Settings |
| Alt+F4 | Close the application and stop projection | Active QuietShare window |
