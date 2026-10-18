# QuietShare 0.12.1

QuietShare là ứng dụng Windows để đọc lời nhắc, mã nguồn, ảnh, PDF và nội dung web trong cửa sổ riêng khi thuyết trình. Smart Mirror chiếu hình ảnh công khai từ màn hình nguồn sang màn hình Extend khác, trong khi cửa sổ QuietShare được loại khỏi đường capture đã xác minh.

QuietShare is a Windows application for scripts, source code, images, PDFs and optional web content during presentations. Smart Mirror projects the public source display onto a separate Extend display while excluding the QuietShare window from the verified capture path.

Tài liệu mô tả nhánh `main`; mục **Unreleased** trong [CHANGELOG](CHANGELOG.md) liệt kê các thay đổi chưa có trong bản phát hành gắn tag.

This guide describes `main`; **Unreleased** in the [CHANGELOG](CHANGELOG.md) lists changes that are not yet in a tagged release.

![Cài đặt QuietShare / QuietShare Settings](docs/quietshare-settings.png)

## Tiếng Việt

### 1. Chức năng

| Thành phần | Khả năng |
|---|---|
| Lời nhắc | Soạn/đọc, cuộn tự động với ba tốc độ, tự lưu cục bộ bằng Windows DPAPI. |
| Tài liệu | Tối đa 32 tab gồm lời nhắc; giữ vị trí cuộn, zoom và trang PDF theo tab trong phiên làm việc. Văn bản/code cho phép sửa và lưu chủ động; ảnh/PDF chỉ xem. Lời nhắc được giữ riêng. |
| Mã nguồn | Màu cú pháp, số dòng, thụt dòng, tìm kiếm, chọn/sao chép và xuống dòng tùy chọn. Không thực thi mã nguồn. |
| Explorer | Cây thư mục cục bộ, icon theo loại file, giữ nhánh mở và lựa chọn khi đổi tab; kéo đường phân cách để chỉnh độ rộng. |
| Ảnh và PDF | Zoom, di chuyển ảnh/trang, PDF từng trang; tìm và sao chép khi PDF có lớp văn bản. |
| Trình duyệt | Bật theo nhu cầu, tối đa 8 tab web và 10 lối tắt trên trang Tab mới; HTTP/HTTPS, tên miền, localhost, tìm Google, lịch sử, zoom 25-400% riêng từng tab và tùy chọn âm thanh/camera/micro. |
| Bố cục | Ẩn/hiện riêng từng thanh, chế độ tập trung, ghim trên cùng, VI/EN, hỗ trợ DPI và cỡ icon 12-24 px. |
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

1. Tải `QuietShare-Setup.exe` tại [QuietShare v0.13.0](https://github.com/TDUmii/QuietShare/releases/tag/v0.13.0) và chạy bộ cài. Nếu dùng ZIP, giải nén toàn bộ trước khi chạy ứng dụng.
2. Bộ cài áp dụng cho tài khoản Windows hiện tại, không cần quyền quản trị. Nút **EN / VI** đổi ngôn ngữ bộ cài.
3. Để dùng portable, chạy `QuietShare.exe` trong thư mục đã giải nén. Giữ `Scintilla.dll`, `Lexilla.dll`, `pdfium.dll` và `WebView2Loader.dll` cạnh EXE; không chỉ sao chép EXE.
4. Đóng QuietShare trước khi nâng cấp bằng bộ cài mới. Nâng cấp và gỡ ứng dụng giữ lại lời nhắc đã lưu.

Nếu trình duyệt không khởi tạo được, cài/cập nhật WebView2 Runtime rồi tắt và bật lại trình duyệt trong QuietShare.

### 4. Sử dụng tài liệu và không gian làm việc

Khi mở ứng dụng, đợi quá trình xác minh capture. Chân trang **Trang chủ** hiển thị **v0.13.0** để nhận diện phiên bản đang chạy. Nếu nội dung bị khóa và cần xác minh lại, chọn icon **Kiểm tra WGC** tại thanh bên trái. Nội dung chỉ được hiển thị khi các điều kiện bảo vệ đạt.

- **Home / Gần đây:** mở lại file hoặc dự án đã mở thành công, ghim mục yêu thích hoặc xóa từng mục bằng nút ×. Các nút trước/sau duyệt tối đa 32 mục. **Xóa lịch sử** xóa cả ghim, không xóa file gốc. Lịch sử được Windows DPAPI mã hóa trên máy và tên file được che khi nội dung đang khóa. Lần chạy đầu không có dữ liệu mẫu.
- **Sửa file:** văn bản và code mặc định cho phép sửa. Nhấn **Lưu** hoặc **Ctrl+S** để ghi file; lời nhắc vẫn tự lưu riêng. Khi đóng tab có thay đổi, chọn lưu, bỏ thay đổi hoặc hủy. QuietShare giữ mã hóa UTF-8/UTF-16 và BOM; nếu file đã đổi bên ngoài, thao tác lưu bị chặn và bản sửa vẫn ở trong tab. Tắt tại **Cài đặt > Nội dung > Cho phép sửa file văn bản / code** để chỉ xem; bản sửa chưa lưu được giữ trong phiên.
- **Kích thước cửa sổ:** **Cài đặt > Giao diện > Nhớ kích thước cửa sổ** mặc định bật. Ứng dụng nhớ kích thước cửa sổ bình thường khi đóng và điều chỉnh để vừa vùng làm việc hiện tại; khi tắt, lần mở sau dùng kích thước mặc định.
- **Lời nhắc:** chọn Soạn lời nhắc trong menu nhấp phải Tệp đang mở để nhập hoặc dán nội dung. **Ctrl+Enter** chuyển soạn/đọc; **Space** trong vùng đọc bắt đầu/dừng cuộn. Lời nhắc được tự lưu và khôi phục ở lần mở tiếp theo.
- **File:** chọn **Mở hoặc thả file**, **Ctrl+O** hoặc kéo thả nhiều file. Dấu **×**, chuột giữa hoặc **Ctrl+W** đóng tab file. Menu **⋯** có lệnh đóng các tab khác/mọi file; tab lời nhắc được giữ. Khi mở lại file đã thay đổi, chọn **Tải lại** hoặc **Giữ bản đang xem**.
- **Dự án:** dùng **Ctrl+Shift+O** để chọn thư mục. Nhấp đúp file hoặc dùng mũi tên và Enter trong Explorer. **Ctrl+B** hoặc **Bố cục > Explorer** ẩn/hiện cùng cây đã mở. Mặc định bỏ qua thư mục ẩn và `.git`, `node_modules`, `.venv`, `dist`, `build`; đổi bộ lọc tại **Cài đặt > Bố cục**.
- **Cỡ chữ/zoom:** dùng các nút thu/phóng, phím tắt hoặc **Cài đặt > Nội dung**. Nhập giá trị rồi Enter; **Đặt lại** khôi phục mặc định. Ô nhập đặt dấu nháy ở cuối khi nhận focus/bấm lần đầu.
- **Tìm kiếm:** **Ctrl+F** mở thanh tìm. Enter tìm tiếp, Shift+Enter tìm trước, Esc đóng thanh. Áp dụng cho lời nhắc, văn bản, code và PDF có lớp chữ.
- **PDF:** dùng menu **⋯** để đổi trang hoặc chọn vừa chiều ngang. Khi vùng PDF nhận focus, Page Up/Page Down đổi trang; kéo vùng trống để di chuyển, kéo trên chữ để chọn và Ctrl+C để sao chép.
- **Bố cục:** nút **Bố cục** ở phía trên bật/tắt thanh icon, Explorer, tab, công cụ và trạng thái. **Khôi phục bố cục** khôi phục các thanh và độ rộng Explorer. Lựa chọn được lưu trên máy. **F11** vào/thoát chế độ tập trung.

Nút **Ghim** ở góc trên điều khiển trạng thái luôn trên cùng; cùng thiết lập có tại **Cài đặt > Giao diện**. Kéo thanh tiêu đề hoặc vùng trống phía trên để di chuyển, kéo cạnh cửa sổ để đổi kích thước. Nút **-** thu nhỏ cửa sổ QuietShare và giữ phiên Smart Mirror đang chạy. **F8** là thao tác che nội dung và dừng chiếu; **Dừng** kết thúc phiên chiếu; đóng ứng dụng giải phóng đầu chiếu.

**Ctrl+Alt+Q** gọi lại cửa sổ bị che hoặc thu nhỏ, kể cả khi không ghim, và giữ focus ở ứng dụng đang dùng. Phím tắt không mở Alt+Tab và không bỏ trạng thái che F8. Rê chuột vào Ghim để xem khả năng đăng ký phím tắt.

### 5. Cài đặt

Mở bằng icon bánh răng hoặc **⋯ > Cài đặt**, sau đó chọn một trong sáu tab. Dùng con lăn, touchpad, thanh cuộn hoặc Tab/Shift+Tab; hàng nhận focus tự đi vào vùng nhìn thấy. Độ mờ, cỡ chữ/zoom và cỡ icon có nút **-**, ô nhập và nút **+**; giá trị được căn giữa theo cả hai chiều.

| Nhóm | Thiết lập |
|---|---|
| Giao diện | Ngôn ngữ VI/EN, ghim, độ mờ cửa sổ 40-100%, cỡ icon 12-24 px (mặc định 16 px), độ đậm chữ lời nhắc Thường/Vừa/Đậm (400/600/700). |
| Nội dung | Cỡ chữ lời nhắc/văn bản 8-200 pt; zoom code/ảnh/PDF 25-400%; đặt lại, tốc độ cuộn lời nhắc, xuống dòng code. |
| Bố cục | Bật/tắt năm thanh, khôi phục bố cục và thư mục nặng/ẩn. |
| Trình chiếu | Nguồn, đích, con trỏ, tài nguyên Tiết kiệm 15 FPS / Cân bằng 30 FPS / Hiệu năng 60 FPS mặc định và trạng thái WGC/chiếu. |
| Phím tắt | Đổi mười tác vụ, kiểm tra tổ hợp trùng/dành riêng và khôi phục phím mặc định. |
| Trình duyệt | Bật/tắt trình duyệt, tắt âm thanh, cho phép camera và cho phép micro. Ba tùy chọn âm thanh/camera/micro mặc định tắt và được lưu cục bộ. |

**Độ mờ áp dụng đồng đều cho toàn bộ cửa sổ điều khiển, gồm nền và chữ.** 100% là rõ hoàn toàn; giá trị thấp hơn giúp nhìn phía sau. Chưa có độ trong suốt chỉ riêng nền. Độ đậm chữ lời nhắc được điều chỉnh riêng, không thay đổi font của website, code hay PDF. Hình chiếu công khai vẫn hoàn toàn đục.

Trong **Phím tắt**, bấm tổ hợp hiện tại rồi nhấn tổ hợp mới; Esc hủy. Hai tác vụ **Che/mở nội dung** và **Gọi cửa sổ** đăng ký toàn hệ thống; các tác vụ còn lại áp dụng trong QuietShare. Nếu trùng, dành riêng hoặc đăng ký toàn hệ thống thất bại, ứng dụng giữ thiết lập trước. Lựa chọn được lưu cục bộ. **Khôi phục mặc định** tại Giao diện đặt lại độ mờ/độ đậm/cỡ icon; tại Phím tắt chỉ đặt lại các phím. Cỡ icon được giới hạn theo vùng nút ở những điều khiển nhỏ để không tràn hoặc bị cắt.

Chế độ tài nguyên giới hạn tốc độ xử lý chiếu của ứng dụng; FPS thực tế phụ thuộc màn hình, GPU và nội dung. Không có mức giảm RAM cố định. Trình duyệt ẩn vẫn nhận yêu cầu tạm ngưng; tắt trình duyệt đóng engine và giữ hồ sơ đăng nhập riêng trên máy.

### 6. Trình duyệt

Bật tại **Cài đặt > Trình duyệt > Bật trình duyệt**, đợi xác minh vùng web rồi chọn icon quả địa cầu. Có thể bật/tắt trực tiếp trên trang trình duyệt. Trình duyệt luôn tắt ở mỗi lần khởi động ứng dụng.

Tab đầu tiên và các tab mới mở trang **Tab mới** cục bộ với ô tìm kiếm/nhập địa chỉ. Chọn **Thêm lối tắt**, nhập tên và URL HTTP/HTTPS rồi **Lưu**; nút **⋯** trên ô cho sửa hoặc xóa. Tối đa 10 lối tắt được dùng chung giữa các tab và lưu cùng hồ sơ web trên máy. Không có nút Google/ChatGPT cố định; người dùng có thể tự lưu trang cần dùng. Trang Tab mới không tải website bên ngoài cho tới khi người dùng mở địa chỉ hoặc tìm kiếm.

Nhập URL, tên miền, địa chỉ như `localhost:3000` hoặc từ khóa rồi Enter/nút mũi tên; từ khóa được tìm bằng Google Search. Ô địa chỉ nằm giữa hàng điều hướng; nội dung và dấu nháy được căn giữa theo chiều dọc trên một dòng. Bấm lần đầu vào ô vẫn chọn toàn bộ URL để nhập thay thế. Quay lại, tiến tới và tải lại áp dụng cho tab đang chọn. Các nút **-**, mức zoom và **+** nằm trực tiếp trên hàng điều hướng; phạm vi zoom là 25-400%. Nút **⋯** gom các thao tác phụ và mở thiết lập trình duyệt. **Ctrl + `+` / `-` / `0`** điều chỉnh zoom web, độc lập với tab file.

Dùng **+** hoặc **Ctrl+T** để mở trang **Tab mới** cục bộ; chọn tiêu đề tab để chuyển trang. Dấu **×** hoặc **Ctrl+W** đóng tab đang chọn. Đóng tab web cuối cùng sẽ tắt trình duyệt, giữ QuietShare và các tab tài liệu đang mở. **Ctrl+Tab / Ctrl+Shift+Tab** chuyển tab web tiếp/trước khi ở trang trình duyệt, kể cả lúc đang nhập trong trang web.

Mỗi tab giữ trang đang mở, lịch sử lùi/tiến và mức zoom riêng trong phiên; thanh địa chỉ và nút điều hướng luôn áp dụng cho tab đang chọn. Các tab dùng chung hồ sơ web cục bộ nên cùng dùng trạng thái đăng nhập. Danh sách tab không được khôi phục ở lần khởi động tiếp theo; lối tắt đã lưu được giữ. Tab nền và trình duyệt ẩn nhận yêu cầu tạm ngưng; việc tạm ngưng thực tế phụ thuộc trang web và WebView2.

Mỗi tab mới phải hoàn tất xác minh capture của vùng web trước khi mở trang người dùng; chỉ tạo/xác minh một tab mới tại một thời điểm. Chuyển giữa các tab đã xác minh không yêu cầu kiểm tra lại. Giới hạn **8 tab** áp dụng cho cả nút mở tab và link yêu cầu tab mới; đóng một tab trước khi mở thêm khi đã đạt giới hạn.

Khi bật web trong phiên Smart Mirror, đầu chiếu giữ khung hình công khai gần nhất trong lúc xác minh, tối đa khoảng 5 giây. Xác minh đạt thì tiếp tục cùng phiên; thất bại hoặc hủy thì dừng chiếu và khóa nội dung.

Trong **Cài đặt > Trình duyệt**, **Tắt âm thanh** áp dụng cho các tab web. **Cho phép camera** và **Cho phép micro** mặc định tắt. Khi bật từng quyền, chỉ tab đang hiển thị, đã xác minh capture và mở trang HTTPS mới có thể được cấp quyền native; nguồn yêu cầu phải cùng origin với trang chính (giao thức, tên miền và cổng). Quyền từng website không được lưu vào hồ sơ web; quyền thiết bị của Windows vẫn áp dụng. Tắt một quyền đã được cấp sẽ thu hồi quyền và có thể tải lại tab để dừng các luồng thiết bị đang dùng.

Hộp chọn file web, kéo/thả file, tải xuống, chia sẻ màn hình, thông báo, DevTools và menu native vẫn bị tắt. Có thể chủ động Ctrl+V ảnh raster vào ô nhập web. Popup tự phát bị chặn; link mở cửa sổ mới do người dùng bấm được mở thành tab trong QuietShare nếu còn chỗ. Đăng nhập OAuth cần popup riêng có thể không tương thích. Dropdown HTML native có thể chọn bằng phím mũi tên. Thay đổi màn hình thực sự tắt trình duyệt và yêu cầu xác minh lại.

### 7. Smart Mirror và giới hạn bảo vệ

1. Chọn **Win+P > Extend** trong Windows và đặt toàn bộ QuietShare trên màn hình nguồn.
2. Mở **Cài đặt > Trình chiếu**. Chọn nguồn và đích khác nhau, cùng tùy chọn con trỏ.
3. Dùng nhãn **Màn N · Chính · Cửa sổ này · độ phân giải** để nhận biết màn hình. Nhãn vai trò chỉ hiện khi áp dụng; số 1 không nhất thiết là màn chính. Di chuyển QuietShare cập nhật nhãn mà không tự đổi lựa chọn nguồn/đích.
4. Dùng icon **Kiểm tra WGC** trên thanh bên, sau đó icon **Bắt đầu/dừng chiếu**; phím mặc định tương ứng là **F9 / F10**. Thiết lập và trạng thái đầy đủ nằm trong **Cài đặt > Trình chiếu**, không cần trang Trình chiếu riêng. Khi đang chiếu, toàn bộ cửa sổ điều khiển được giữ trong màn hình nguồn, kể cả khi kéo hoặc đổi kích thước sát mép. Nút **-** có thể thu nhỏ QuietShare trong khi tiếp tục chiếu.
5. Chọn **Dừng** để kết thúc. **F8** che mọi nội dung, tên tab và Explorer, đồng thời dừng cuộn và chiếu. F8 lần nữa mở lại nội dung khi các điều kiện bảo vệ cho phép; cần bắt đầu lại nếu muốn tiếp tục chiếu.

Kiểm tra WGC trong phiên đang chiếu xác nhận các điều kiện hiện tại và giữ cùng phiên capture. Thông báo hiển thị không đổi cấu hình thực tế không tự kết thúc phiên. Mất affinity, topology, capture, DWM hoặc thiết bị vẫn dừng an toàn, không tự khởi động lại. Ứng dụng lưu một bản ghi chẩn đoán lần dừng gần nhất trên máy, chỉ có mã lý do, thời gian và trạng thái kỹ thuật; không chứa hình ảnh, nội dung, URL hay tên màn hình.

Phiên Smart Mirror không còn tự hết hạn theo mốc 5 phút. Phiên tiếp tục khi đường capture, cấu hình màn hình và các điều kiện bảo vệ vẫn hợp lệ; thu nhỏ QuietShare không kết thúc phiên. Khi đang chiếu, QuietShare yêu cầu Windows giữ máy và màn hình hoạt động để tránh ngủ do không có thao tác. Lệnh ngủ chủ động hoặc đóng nắp máy vẫn được xử lý theo Windows và dừng chiếu khi hệ thống suspend.

Nếu phiên chiếu bị gián đoạn ngoài thao tác chủ động, tiêu đề cửa sổ chuyển thành **QuietShare - Trình chiếu đã dừng**, thanh trạng thái hiển thị cảnh báo màu hổ phách khi đang hiện, đồng thời ứng dụng nháy taskbar và phát một âm báo Windows nếu âm thanh hệ thống cho phép. Cảnh báo không giành focus, mở cửa sổ mới hoặc tự khôi phục cửa sổ đang thu nhỏ. Mở **Cài đặt > Trình chiếu** để xem trạng thái và mã lý do; khi cần, dùng **Kiểm tra WGC** rồi **Bắt đầu** để tiếp tục. Ứng dụng không tự khởi động lại đầu chiếu sau khi mất điều kiện bảo vệ. Cảnh báo được xóa khi bắt đầu chiếu thành công hoặc chủ động dùng **Dừng/F8**; các thao tác chủ động này không phát cảnh báo gián đoạn.

**Duplicate luôn khóa nội dung riêng.** Xác minh thất bại, thay đổi màn hình, suspend hoặc capture mất an toàn sẽ dừng chiếu và khóa nội dung. Trạng thái **Windows đã nhận** xác nhận API loại trừ capture; **người xem chưa được xác minh** yêu cầu quan sát phía người nhận trong đúng Zoom/Meet/Teams/OBS hoặc máy chiếu sử dụng. Xác nhận bằng nội dung mẫu trước buổi trình bày. Phần mềm không ngăn máy ảnh bên ngoài quay màn hình.

### 8. Phím tắt

Bảng dưới là mặc định. Có thể đổi F8, Ctrl+Alt+Q, F9, F10, F11, Ctrl+O, Ctrl+Shift+O, Ctrl+B, Ctrl+Enter và Space tại **Cài đặt > Phím tắt**.

Phím **toàn hệ thống** dùng được khi ứng dụng khác đang nhận focus nếu QuietShare đăng ký thành công. Các phím còn lại áp dụng khi vùng tương ứng trong QuietShare nhận focus. Các phím quản lý tab web trong bảng được QuietShare xử lý cả khi nội dung website nhận focus.

| Phím | Thao tác | Phạm vi |
|---|---|---|
| F8 | Che/mở lại nội dung; khi che sẽ dừng cuộn và Smart Mirror | Toàn hệ thống khi đăng ký được; dự phòng trong QuietShare |
| Ctrl+Alt+Q | Gọi lại cửa sổ, giữ focus hiện tại và trạng thái F8 | Toàn hệ thống khi đăng ký được |
| F9 | Kiểm tra WGC; giữ phiên đang chiếu khi điều kiện bảo vệ còn hợp lệ | QuietShare |
| F10 | Bắt đầu/dừng chiếu | QuietShare |
| F11 | Vào/thoát tập trung | QuietShare |
| Esc | Đóng tìm kiếm; nếu không tìm kiếm thì thoát tập trung | QuietShare |
| Ctrl+O | Chọn nhiều file | QuietShare |
| Ctrl+Shift+O | Chọn thư mục dự án | QuietShare |
| Ctrl+B | Ẩn/hiện Explorer | QuietShare |
| Ctrl+Tab / Ctrl+Shift+Tab | Tab tài liệu tiếp/trước | Trang nội dung |
| Ctrl+W | Đóng tab file đang chọn; giữ tab lời nhắc | Trang nội dung |
| Ctrl+T | Mở trang Tab mới, tối đa 8 tab | Trang trình duyệt, gồm nội dung web |
| Ctrl+Tab / Ctrl+Shift+Tab | Tab web tiếp/trước | Trang trình duyệt, gồm nội dung web |
| Ctrl+W | Đóng tab web; tab cuối cùng sẽ tắt trình duyệt | Trang trình duyệt, gồm nội dung web |
| Ctrl+Enter | Chuyển soạn/đọc lời nhắc | QuietShare |
| Space | Chạy/dừng cuộn lời nhắc | Vùng lời nhắc ở chế độ đọc |
| Ctrl+F | Mở tìm kiếm | Lời nhắc, văn bản, code, PDF có lớp chữ |
| Enter / Shift+Enter | Tìm tiếp/trước | Ô tìm kiếm |
| Ctrl+C | Sao chép phần được chọn | Nội dung có thể chọn |
| Ctrl + `+` / `-` | Tăng/giảm cỡ chữ hoặc zoom | Nội dung đang dùng; web có zoom riêng |
| Ctrl+0 | Về zoom mặc định 50%; văn bản/lời nhắc 12 pt | Nội dung đang dùng |
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
| Documents | Up to 32 tabs including the script; scroll, zoom and PDF page retained per tab during the session. Text/code support explicit editing and saving; images/PDFs remain viewers. The script stays separate. |
| Source code | Syntax coloring, line numbers, indentation, find, selection/copy and optional wrapping. Source code is never executed. |
| Explorer | Local folder tree with file-type icons; expanded branches and selection survive tab changes. Drag the divider to resize. |
| Images and PDFs | Zoom and pan, single-page PDF navigation, search/copy when a text layer exists. |
| Browser | Up to 8 on-demand tabs and 10 saved New tab shortcuts; HTTP/HTTPS, domains, localhost, Google search, independent history/25-400% zoom per tab and audio/camera/microphone options. |
| Workspace | Independent bar visibility, focus mode, always-on-top pinning, VI/EN, DPI support and 12-24 px icons. |
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

1. Download `QuietShare-Setup.exe` from [QuietShare v0.13.0](https://github.com/TDUmii/QuietShare/releases/tag/v0.13.0) and run it. For ZIP use, extract the complete archive before running the application.
2. Installation applies to the current Windows account without administrator privileges. **EN / VI** switches the installer language.
3. For portable use, run `QuietShare.exe` in the extracted folder. Keep `Scintilla.dll`, `Lexilla.dll`, `pdfium.dll` and `WebView2Loader.dll` beside it; do not copy only the EXE.
4. Close QuietShare before upgrading. Upgrades and uninstallation retain the saved script.

If browsing cannot initialize, install/update WebView2 Runtime, then disable and re-enable browsing in QuietShare.

### 4. Documents and workspace

Wait for capture verification at startup. The **Home** footer displays **v0.13.0** to identify the running version. If content is locked and verification is requested, select **Verify WGC privacy** on the left activity bar. Readers become available only after the protection conditions pass.

- **Home / Recent:** reopen successfully opened files/projects, pin favorites or remove an entry with ×. Previous/next browse up to 32 entries. **Clear history** removes pins as well, without deleting source files. Windows DPAPI encrypts this local history; filenames are concealed while content is locked. No sample history appears at first launch.
- **File editing:** text and code editing is enabled by default. Use **Save** or **Ctrl+S** to write the file; script autosave remains separate. Closing a modified tab offers save, discard or cancel. QuietShare retains UTF-8/UTF-16 encoding and BOM; an external change blocks saving while your draft remains in the tab. Disable **Settings > Reading > Allow text / code editing** for viewing only; unsaved drafts remain in the session.
- **Window size:** **Settings > Appearance > Remember window size** is on by default. It restores normal window dimensions fitted to the current work area. Turning it off uses the default dimensions on the next launch.
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

Open the gear icon or **⋯ > Settings**, then select one of six tabs. Navigate with the wheel, touchpad, scrollbar or Tab/Shift+Tab; focused rows automatically scroll into view. Opacity, text size/zoom and icon size use **-**, an editable value and **+**, with values centered horizontally and vertically.

| Group | Controls |
|---|---|
| Appearance | VI/EN, pinning, 40-100% window opacity, 12-24 px icons (default 16 px) and Regular/Semibold/Bold script text (400/600/700). |
| Reading | Script/text 8-200 pt; code/image/PDF zoom 25-400%; reset, script scrolling speed and code wrapping. |
| Workspace | Five bar visibility controls, layout reset and heavy/hidden folders. |
| Projection | Source, destination, pointer, resource modes Saver 15 FPS / Balanced 30 FPS / default Performance 60 FPS and WGC/projection status. |
| Shortcuts | Ten configurable commands, duplicate/reserved-binding validation and restoring shortcut defaults. |
| Browser | Enable/disable browsing, mute audio, allow camera and allow microphone. Audio mute, camera and microphone options default to off and persist locally. |

**Opacity applies uniformly to the entire control window, including its background and text.** 100% is fully opaque; lower values reveal what is behind it. Background-only transparency is not provided. Script text weight is separate and does not change website, code or PDF fonts. Public projection remains opaque.

In **Shortcuts**, click the current binding and press a new combination; Esc cancels. **Mask/reveal** and **Recall window** register globally; other commands apply within QuietShare. Duplicate/reserved combinations and failed global registrations retain the previous setting. Choices are stored locally. **Restore defaults** in Appearance resets opacity/weight/icon size; in Shortcuts it resets bindings only. Small controls cap the icon glyph to fit their available space without clipping.

Resource modes cap application projection processing. Achieved FPS depends on displays, GPU and source activity; no fixed RAM reduction is promised. Hidden browsing still requests suspension. Disabling browsing closes the engine while retaining its separate local login profile.

### 6. Browser

Enable **Settings > Browser > Enable browser**, wait for web capture verification and select the globe icon. Browsing can also be enabled/disabled on its own page. Every application launch starts with browsing disabled.

The first tab and new tabs open a local **New tab** page with a search/address input. Select **Add shortcut**, enter a name and HTTP/HTTPS URL, then **Save**. A tile's **⋯** edits or removes it. Up to 10 shortcuts are shared across tabs and saved with the local web profile. Fixed Google/ChatGPT buttons are removed; users can save their own destinations. This local page makes no external website request until the user opens an address or searches.

Enter a URL, domain, address such as `localhost:3000`, or search terms, then press Enter/the arrow button; search terms use Google Search. The address container is centered in the navigation row; text and the caret remain vertically centered on one line. The first click still selects the complete URL for replacement. Back, Forward and Reload apply to the selected tab. **-**, the zoom level and **+** are directly available in the navigation row, with a 25-400% range. The browser **⋯** menu groups secondary actions and opens browser settings. **Ctrl + `+` / `-` / `0`** controls web zoom independently of file tabs.

Use **+** or **Ctrl+T** to open the local **New tab** page; select a tab title to switch pages. **×** or **Ctrl+W** closes the selected tab. Closing the last web tab disables browsing while keeping QuietShare and its document tabs open. **Ctrl+Tab / Ctrl+Shift+Tab** selects the next/previous web tab on the browser page, including while typing in web content.

Each tab retains its page, back/forward history and zoom during the session. The address field and navigation controls always apply to the selected tab. Tabs share the local web profile, including login state. The tab list is not restored on the next application launch; saved shortcuts persist. Background tabs and hidden browsing request suspension; actual suspension depends on the page and WebView2.

Each new tab must complete web capture verification before opening a user page; only one new tab can be created/verified at a time. Switching between verified tabs does not require a fresh check. The **8-tab limit** applies to both the new-tab control and links requesting a new tab. Close an existing tab before opening another when the limit is reached.

Enabling web during Smart Mirror holds the most recent public frame while verification runs, for up to about five seconds. Success resumes the same projection; failure or cancellation stops projection and locks content.

In **Settings > Browser**, **Mute audio** applies to web tabs. **Allow camera** and **Allow microphone** are off by default. When individually enabled, native access is eligible only in a visible, capture-verified tab displaying an HTTPS page; the requesting origin must match the main page's origin (scheme, host and port). Website grants are not saved to the web profile, and Windows device permissions still apply. Disabling a previously granted permission revokes it and may reload the affected tab to stop active device streams.

Native web file pickers, file drops, downloads, screen sharing, notifications, DevTools and native context menus remain disabled. Explicit Ctrl+V raster image paste into web editors is supported. Unsolicited popups are blocked; user-initiated new-window links open in a QuietShare tab if capacity is available. OAuth flows requiring separate popups may be incompatible. Native HTML selects support arrow-key selection. Actual display changes disable browsing until fresh verification.

### 7. Smart Mirror and protection limits

1. Select **Win+P > Extend** in Windows and place the entire QuietShare window on the source display.
2. Open **Settings > Projection**. Select different source/destination displays and the pointer option.
3. Identify displays with **Display N · Main · This window · resolution**. Role labels appear only when applicable; Display 1 is not necessarily primary. Moving QuietShare updates labels without automatically changing the selected source/destination.
4. Use **Verify WGC** on the activity bar, then **Start/stop projection**; default shortcuts are **F9 / F10**. Full configuration and status are in **Settings > Projection**; a separate Presentation page is no longer needed. During projection, the entire control window stays within the source display, including when dragging or resizing at its edges. **-** can minimize QuietShare while projection continues.
5. Select **Stop** to finish. **F8** masks viewers, tab names and Explorer, and stops auto-scroll and projection. Pressing F8 again reveals content when protection conditions allow; restart projection explicitly if needed.

A WGC check during projection validates current conditions while retaining the same capture session. Display notifications without a real configuration change do not end the session by themselves. Lost affinity, topology, capture, DWM or device safety still stops output without automatic restart. One local last-stop diagnostic stores a reason code, timing and technical state; it contains no images, content, URLs or display names.

Smart Mirror sessions no longer expire at a fixed five-minute boundary. A session continues while the capture path, display configuration and protection conditions remain valid; minimizing QuietShare does not end it. During projection, QuietShare requests that Windows keep the system and display awake to avoid idle sleep. Intentional sleep commands and lid closure retain Windows behavior and stop projection when the system suspends.

If projection is interrupted without an intentional stop, the window title changes to **QuietShare - Projection stopped** and the status bar shows an amber warning when visible. QuietShare also flashes its taskbar button and requests one Windows warning sound, subject to system sound settings. The warning does not take focus, open another window or restore a minimized window. Open **Settings > Projection** to inspect the status and reason code; use **Verify WGC privacy**, then **Start**, when required to resume. The application does not restart projection automatically after protection conditions are lost. A successful Start or intentional **Stop/F8** clears the warning; intentional stop actions do not trigger an interruption alert.

**Duplicate always locks private content.** Failed verification, display changes, suspend or unsafe capture stop projection and lock readers. **Windows accepted** confirms the capture-exclusion API state; **Viewer unverified** requires receiver-side observation in the actual Zoom/Meet/Teams/OBS or projector setup. Confirm with sample content before a presentation. Software cannot prevent an external camera from filming a display.

### 8. Keyboard shortcuts

The table lists defaults. Change F8, Ctrl+Alt+Q, F9, F10, F11, Ctrl+O, Ctrl+Shift+O, Ctrl+B, Ctrl+Enter and Space in **Settings > Shortcuts**.

**Global** shortcuts work while another application has focus if QuietShare registers them successfully. Other shortcuts require the relevant QuietShare area to have focus. QuietShare handles the web tab shortcuts listed below even when web content has focus.

| Shortcut | Action | Scope |
|---|---|---|
| F8 | Mask/reveal content; masking stops auto-scroll and Smart Mirror | Global when registered; local fallback |
| Ctrl+Alt+Q | Recall the window; preserve focus and F8 masking | Global when registered |
| F9 | Verify WGC; retain an active session while protection conditions remain valid | QuietShare |
| F10 | Start/stop projection | QuietShare |
| F11 | Enter/exit focus mode | QuietShare |
| Esc | Close search; otherwise exit focus mode | QuietShare |
| Ctrl+O | Open multiple files | QuietShare |
| Ctrl+Shift+O | Open a project folder | QuietShare |
| Ctrl+B | Show/hide Explorer | QuietShare |
| Ctrl+Tab / Ctrl+Shift+Tab | Next/previous document tab | Content page |
| Ctrl+W | Close the selected file tab; preserve Script | Content page |
| Ctrl+T | Open the local New tab page, up to 8 tabs | Browser page, including web content |
| Ctrl+Tab / Ctrl+Shift+Tab | Next/previous web tab | Browser page, including web content |
| Ctrl+W | Close a web tab; closing the last disables browsing | Browser page, including web content |
| Ctrl+Enter | Switch script editing/reading | QuietShare |
| Space | Start/stop script auto-scroll | Script reading area |
| Ctrl+F | Open find | Script, text, code, searchable PDF |
| Enter / Shift+Enter | Find next/previous | Search field |
| Ctrl+C | Copy selection | Selectable content |
| Ctrl + `+` / `-` | Increase/decrease text size or zoom | Current content; independent web zoom |
| Ctrl+0 | Reset to 50% zoom; text/script 12 pt | Current content |
| Ctrl + wheel | Change text size/zoom | Content area or browser under the pointer |
| Page Up / Page Down | Previous/next PDF page | PDF area |
| Arrows / Enter | Navigate/open a file or expand/collapse a folder | Explorer |
| Tab / Shift+Tab | Next/previous control; reveal the focused row | Settings |
| Alt+F4 | Close the application and stop projection | Active QuietShare window |

## Bộ chọn file và dự án

Bộ chọn có cột truy cập nhanh Desktop, Downloads, Tài liệu, Hình ảnh và Máy tính này; lấy vị trí thư mục hiện tại từ Windows. Dùng thanh địa chỉ để nhập đường dẫn thư mục, Ctrl+L để chọn địa chỉ, Alt+Left/Right để quay lại/tiến tới và Alt+Up để lên thư mục cha. Danh sách hiển thị tên, ngày sửa đổi và kích thước; Ctrl/Shift chọn nhiều file. Bộ chọn vẫn được bảo vệ capture và chỉ mở đường dẫn cục bộ phù hợp chính sách bảo vệ.

The protected file/project picker follows the familiar File Explorer layout, with Windows known-folder shortcuts, local drives, an editable folder address, back/forward/up navigation, and name/date/size details. Ctrl+L selects the address; Alt+Left/Right/Up navigates. Existing local-path restrictions remain enforced.

## Chuyển file và lời nhắc

Nút **Tệp đang mở** trở lại đúng file đang xem sau khi qua Home, trình duyệt hoặc cài đặt, giữ bản sửa chưa lưu và vị trí xem. Thanh tab chỉ chứa các file đã mở; Ctrl+Tab / Ctrl+Shift+Tab chuyển giữa những file này. **Soạn lời nhắc** mở chủ động từ menu dấu ba chấm hoặc menu bố cục. Bản lời nhắc đã lưu vẫn được giữ; hồ sơ mới không có nội dung mẫu.

The Open files activity button resumes the current file without resetting its native buffer or view. File tabs and Ctrl+Tab cycling exclude the optional script. Open Compose script explicitly from the overflow or layout menu. Existing saved scripts are retained; new profiles start empty.

## Zoom mặc định

Web, tab web mới, code, ảnh và PDF mở ở 50%; văn bản/lời nhắc dùng 12 pt, tương ứng 50% của cỡ chuẩn 24 pt. Nút đặt lại zoom và Ctrl+0 trở về mức mặc định này. Zoom tự chọn vẫn được giữ riêng theo tab trong phiên; các lệnh PDF Vừa trang / Vừa ngang vẫn căn theo khung khi chọn chủ động. Kích thước giao diện điều khiển theo DPI Windows như hiện tại.

Web pages, new browser tabs, code, images and PDFs default to 50%; text/scripts default to 12 pt. Reset and Ctrl+0 restore these defaults. Per-tab user zoom remains independent for the session. Explicit PDF Fit Page / Fit Width still fit the viewport.


### Khôi phục sau khi đổi chế độ màn hình

Khi chuyển Duplicate về Extend, QuietShare chọn lại nguồn và đích khác nhau. Chọn **Kiểm tra WGC**, rồi **Bắt đầu trình chiếu** để tạo phiên mới; ứng dụng không tự chiếu lại sau khi bị khóa. Nhấp phải biểu tượng **Tệp đang mở** ở thanh trái để chọn nhanh tệp hoặc soạn lời nhắc. Nút ghim dùng xanh lá khi bật và xám khi tắt.

### Returning from Duplicate to Extend

QuietShare selects distinct source and destination displays again. Select **Verify WGC**, then **Start projection** for a fresh session; output never resumes automatically after a safety stop. Right-click **Open files** on the activity bar to switch files or compose a script. Pin controls use green when enabled and gray when disabled.


### Dán ảnh và đọc văn bản

Ctrl+V có thể dán ảnh raster trong clipboard vào ô nhập trên trang web hỗ trợ ảnh. Thao tác dán do người dùng thực hiện; QuietShare không cấp quyền đọc clipboard tự động. Hộp chọn file web và kéo/thả tệp vẫn bị chặn.

Màn xem văn bản/lời nhắc có ô nhập cỡ chữ 8-200 pt, nút chọn Thường / Vừa / Đậm và nút Xuống dòng; màn code có nút Xuống dòng. Thanh công cụ tự chia hàng khi hẹp. Điều chỉnh cách xem không ghi định dạng vào file text.

### Image paste and reading controls

Ctrl+V can paste clipboard raster images into web editors that support images. QuietShare retains denial of automatic clipboard reads, native web file pickers and file drops. Text/script viewers expose an 8-200 pt size field, regular/semibold/bold weight and word wrap; code has a word-wrap control. The toolbar reflows in narrow windows. Viewing preferences do not write formatting into plain-text files.
