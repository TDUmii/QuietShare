# Hướng dẫn sử dụng QuietShare

Ấn bản tiếng Việt - QuietShare v0.15.0

Đọc, chuẩn bị và thuyết trình với lời nhắc riêng trên Windows.

[Bản PDF](QuietShare-Huong-Dan-Su-Dung-VI.pdf) | [Ấn bản tiếng Anh](USER_MANUAL_EN.md) | [Bản phát hành mới nhất](https://github.com/TDUmii/QuietShare/releases/latest)

## 1. Bắt đầu từ đây

QuietShare hỗ trợ đọc lời nhắc, tài liệu cục bộ và trang web tùy chọn khi thuyết trình. Cửa sổ điều khiển riêng của ứng dụng tách biệt với tài liệu bạn muốn đưa cho người xem.

Smart Mirror thu màn hình nguồn công khai đã chọn rồi dựng hình lên màn hình thứ hai. Ứng dụng kiểm tra QuietShare được loại khỏi đường thu hình đã xác minh trước khi cho hiện nội dung riêng. Kiểm tra đạt chỉ mô tả đường thu hình cục bộ đó. Bạn vẫn cần quan sát máy chiếu, TV, đầu nhận cuộc họp hoặc bản ghi thực tế.

### Chọn tình huống sử dụng

- **Đọc trên một màn hình:** mở file cục bộ hoặc soạn lời nhắc sau khi xác minh capture. Không bắt buộc dùng Smart Mirror.
- **Chiếu qua máy chiếu hoặc màn hình có dây:** dùng Extend của Windows với hai màn khác nhau cùng GPU. Làm theo chương 10.
- **Chiếu qua TV không dây:** trước tiên cần tạo màn hình Windows riêng cho TV. Làm theo chương 11.
- **Chia sẻ trong cuộc họp:** dùng điều khiển của ứng dụng họp để chia sẻ cửa sổ ứng dụng công khai gốc. Làm theo chương 12.
- **Ghi hình bằng OBS:** chọn và kiểm tra đúng nguồn công khai được thu. Làm theo chương 12.

Không chọn cửa sổ QuietShare riêng làm tài liệu công khai. F8 che nội dung riêng và dừng trình chiếu QuietShare. Phím này không tắt chia sẻ màn hình của ứng dụng họp hoặc dừng ghi OBS.

### Yêu cầu

- Windows 10 phiên bản 2004, build 19041 trở lên hoặc Windows 11.
- Windows 64 bit.
- Desktop Window Manager (DWM) và Windows Graphics Capture (WGC) hoạt động.
- Smart Mirror cần hai màn Extend khác nhau, được nhận diện trên cùng adapter đồ họa.
- Trình duyệt tùy chọn cần Microsoft Edge WebView2 Runtime.

Trình đọc file cục bộ hoạt động ngoại tuyến và không cần bộ máy trình duyệt. Chưa hỗ trợ trình chiếu giữa hai adapter khác nhau. Chế độ Duplicate khóa nội dung riêng.

### Buổi dùng thử đầu tiên

1. Cài QuietShare hoặc giải nén toàn bộ gói portable.
2. Mở ứng dụng và đợi xác minh capture.
3. Đọc trạng thái bảo vệ. Nếu bị khóa, xử lý điều kiện hiển thị trước khi mở lời nhắc riêng.
4. Mở một file text không chứa thông tin nhạy cảm bằng Ctrl+O.
5. Chỉnh cỡ chữ, độ đậm và xuống dòng cho phù hợp laptop.
6. Thử F8 để làm quen thao tác che trước khi thuyết trình.
7. Nếu có người xem, hoàn thành quy trình tương ứng và kiểm tra đầu nhận bằng tài liệu mẫu.

Nút Play trình chiếu và nút chạy cuộn của trình đọc có tác dụng khác nhau. F10 bắt đầu hoặc dừng Smart Mirror. Space chạy hoặc tạm dừng cuộn trong vùng đọc.

## 2. Cài đặt, cập nhật và gỡ ứng dụng

Tải từ [trang phát hành QuietShare](https://github.com/TDUmii/QuietShare/releases/latest). Dùng các thành phần thuộc cùng một bản phát hành. Không ghép EXE của bản này với thư viện của bản khác.

### Cài cho tài khoản hiện tại

1. Đóng bản QuietShare cũ đang chạy.
2. Tải **QuietShare-Setup.exe**.
3. Chạy bộ cài và chọn tiếng Anh hoặc tiếng Việt.
4. Hoàn thành cài đặt cho tài khoản Windows đang dùng. Không cần quyền quản trị.
5. Mở QuietShare và kiểm tra phiên bản trên Home.

Windows có thể áp dụng kiểm tra thông thường cho ứng dụng tải xuống. Dùng bản phát hành từ kho của dự án và tuân theo quy định cài phần mềm của tổ chức bạn.

### Dùng gói portable

1. Tải toàn bộ ZIP Windows x64 có số phiên bản.
2. Giải nén vào thư mục cục bộ bình thường trước khi chạy.
3. Giữ các file sau cạnh nhau:
   - QuietShare.exe
   - Scintilla.dll
   - Lexilla.dll
   - pdfium.dll
   - WebView2Loader.dll
4. Giữ tài liệu và giấy phép đi kèm trong gói.
5. Chạy QuietShare.exe từ thư mục đã giải nén.

Chỉ tải EXE hoặc chạy trực tiếp trong cửa sổ xem ZIP không tạo thành bản portable đầy đủ.

### Cập nhật

1. Lưu các file văn bản hoặc code đang sửa.
2. Dừng trình chiếu và đóng QuietShare.
3. Nếu đã cài ứng dụng, chạy bộ cài mới.
4. Nếu dùng portable, giải nén toàn bộ gói mới vào thư mục riêng rồi chạy bản đó.
5. Xác nhận phiên bản mới trên Home.
6. Kiểm tra lại đầu ra thực tế phía người xem trước buổi thuyết trình có lời nhắc riêng tiếp theo.

Nâng cấp bằng bộ cài giữ lời nhắc đã lưu. Với tài liệu quan trọng, duy trì bản sao riêng theo cách sao lưu thông thường của bạn.

### Gỡ ứng dụng

1. Lưu công việc và đóng QuietShare.
2. Với bản cài đặt, mở danh sách ứng dụng đã cài của Windows rồi gỡ QuietShare.
3. Với bản portable, xóa thư mục ứng dụng đã giải nén sau khi đóng chương trình.

Gỡ ứng dụng không xóa lời nhắc đã lưu hoặc toàn bộ dữ liệu người dùng cục bộ. Bản portable cũng lưu cấu hình và lời nhắc trong hồ sơ người dùng Windows. Xóa thư mục ứng dụng portable không xóa hồ sơ đó. Nếu muốn xóa lịch sử gần đây, dùng chức năng xóa lịch sử riêng trong QuietShare.

## 3. Home và không gian làm việc

Home giúp mở nhanh file và thư mục dự án bạn thực sự đã dùng. Ứng dụng không chèn các file minh họa vào lịch sử gần đây.

![Trang chủ QuietShare](quietshare-home.png)

Hình minh họa bố cục Home. Phiên bản, ngôn ngữ, lịch sử và kết quả bảo vệ của bạn có thể khác.

### Mở lại công việc gần đây

1. Chọn **Tổng quan** ở thanh bên để về Home.
2. Tìm file hoặc dự án trong danh sách gần đây.
3. Chọn mục để mở lại.
4. Dùng nút trang trước hoặc trang sau khi có nhiều mục.

File hoặc thư mục gốc vẫn phải còn ở vị trí cục bộ tương ứng. Mục gần đây là lối tắt đến vị trí đó, không phải bản sao lưu được nhúng trong ứng dụng.

### Ghim hoặc xóa một mục

- Chọn nút ghim cạnh mục để giữ trong nhóm yêu thích.
- Chọn lại để bỏ ghim.
- Dùng nút xóa cạnh mục để xóa riêng mục lịch sử đó.
- Chọn **Xóa lịch sử** để xóa toàn bộ mục gần đây, gồm cả ghim.

Các thao tác này không xóa file hoặc thư mục gốc. Xóa lịch sử không đóng tài liệu đang mở.

### Dùng thanh bên

- **Tổng quan** trở về Home.
- **Tệp đang mở** trở về tài liệu hiện tại.
- Nhấp phải **Tệp đang mở** để chọn file khác hoặc soạn lời nhắc.
- **Trình duyệt** mở không gian duyệt web tùy chọn.
- **Kiểm tra WGC** xác minh capture. Nút Play hoặc Dừng trình chiếu bắt đầu hoặc dừng Smart Mirror.
- **Cài đặt** chia thành giao diện, nội dung, bố cục, trình chiếu, phím tắt và trình duyệt.

Di chuột để đọc chú giải các nút biểu tượng nhỏ. Với màn hình laptop nhỏ, ẩn thanh không cần thiết trong Bố cục hoặc dùng F11 để tập trung.

### Mở thư mục dự án

1. Chọn **Mở dự án** hoặc nhấn Ctrl+Shift+O.
2. Chọn thư mục cục bộ trong hộp chọn được bảo vệ.
3. Mở rộng các thư mục trong Explorer.
4. Nhấp đúp file hoặc chọn file rồi nhấn Enter.
5. Dùng Ctrl+B để ẩn hoặc hiện Explorer.

Explorer bỏ qua junction và liên kết tượng trưng. Thư mục ẩn và thư mục phát triển nặng được lọc theo mặc định. Bật tùy chọn hiện các thư mục này khi cần thêm mục. Mở dự án không thực thi code hoặc cài thư viện của dự án đó.

## 4. Mở, chỉnh sửa và lưu file

Dùng **Mở file**, Ctrl+O hoặc thả file cục bộ. Hộp chọn được bảo vệ có các vị trí quen thuộc như Desktop, Downloads, Documents, Pictures và các ổ đĩa.

### Chọn và chuyển file

1. Mở hộp chọn bằng Ctrl+O.
2. Chọn vị trí nhanh hoặc đi đến thư mục chứa file.
3. Chọn file và xác nhận.
4. Dùng tab tài liệu hoặc nhấp phải **Tệp đang mở** để chuyển tài liệu.
5. Dùng Ctrl+Tab và Ctrl+Shift+Tab để chuyển tài liệu tiếp hoặc trước trên trang tài liệu đang hoạt động.

Có thể mở tối đa 32 tài liệu, tính cả lời nhắc tùy chọn. Một lần thả file nhận tối đa 31 file. Vị trí xem được giữ khi chuyển trong cùng phiên. Không có cam kết khôi phục danh sách tài liệu đang mở sau khi khởi động lại ứng dụng.

### Nội dung cục bộ được hỗ trợ

- **Văn bản và code:** tối đa 5 MiB. Hỗ trợ UTF-8 và UTF-16 có dấu thứ tự byte. Đuôi file lạ phải chứa văn bản hợp lệ được hỗ trợ, không phải dữ liệu nhị phân.
- **Ảnh:** PNG, JPEG, BMP và khung đầu GIF. Tối đa 25 MiB và 40 megapixel.
- **PDF:** tối đa 64 MiB và 10.000 trang. Chưa hỗ trợ PDF có mật khẩu.
- **Đường dẫn:** file và thư mục cục bộ. Chưa hỗ trợ đường dẫn mạng, tệp thực thi, bộ cài, tệp nén hoặc nhị phân tùy ý.

Nếu bản gốc không mở được, dùng một bản đã chuyển sang định dạng phù hợp. Đổi đuôi file nhị phân thành đuôi text không chuyển đổi nội dung.

### Sửa văn bản hoặc code

Mặc định cho phép chỉnh sửa. Ảnh và PDF vẫn chỉ đọc. Code được xem và sửa như văn bản, không bao giờ được thực thi.

1. Mở file văn bản hoặc code.
2. Thực hiện thay đổi cần thiết.
3. Xem dấu thay đổi cạnh tên tài liệu.
4. Chọn **Lưu** hoặc nhấn Ctrl+S.
5. Đọc thông báo lỗi lưu nếu có trước khi đóng tài liệu.

Khi lưu, ứng dụng giữ mã hóa và dấu thứ tự byte được hỗ trợ. Cỡ chữ, độ đậm và xuống dòng khi đọc không ghi định dạng vào file văn bản thuần.

Để chỉ xem, tắt **Cài đặt > Nội dung > Cho phép sửa file**. Lưu các thay đổi đang có trước khi đổi chế độ.

### File thay đổi bên ngoài và đóng tài liệu

QuietShare từ chối ghi đè khi file đã thay đổi trong ứng dụng khác.

1. Giữ bản đang sửa nếu vẫn cần các thay đổi đó.
2. Xem file đã bị ứng dụng khác thay đổi.
3. Lưu hoặc sao chép phần bản nháp cần giữ.
4. Chỉ dùng **Tải lại** khi sẵn sàng thay bản đang xem bằng nội dung trên ổ đĩa.

Đóng file đang sửa có lựa chọn lưu, bỏ thay đổi hoặc hủy. Bản sửa file giữ trong bộ nhớ đến khi bạn chủ động lưu. Nó không tự ghi đè lời nhắc hoặc file gốc.

## 5. Chuẩn bị lời nhắc và đọc dễ hơn

Lời nhắc là bản nháp cục bộ tùy chọn để chuẩn bị lời nói. Nó tách biệt với file nhập vào và không cần chiếm một tab tài liệu thường trực.

### Soạn lời nhắc

1. Nhấp phải **Tệp đang mở** ở thanh bên.
2. Chọn **Soạn lời nhắc**.
3. Nhập hoặc dán nội dung cần dùng.
4. Đợi tự lưu cục bộ hoàn thành.
5. Nhấn Ctrl+Enter để chuyển giữa soạn và đọc.

Lời nhắc tự lưu trên máy. Vùng soạn lời nhắc nhận tối đa 500.000 ký tự. Đây là giới hạn riêng, khác với mức 5 MiB của file text nhập vào.

Văn bản và code nhập từ file dùng thao tác Lưu chủ động. Chuyển tài liệu không thay lời nhắc đã lưu bằng file vừa mở.

### Cuộn khi trình bày

1. Chuyển lời nhắc sang chế độ đọc.
2. Chọn tốc độ cuộn chậm, vừa hoặc nhanh.
3. Nhấn Space trong vùng đọc hoặc dùng nút Chạy của vùng đó.
4. Nhấn Space lần nữa để tạm dừng.
5. Dừng cuộn trước khi chuyển tài liệu hoặc tab trình duyệt.

Cuộn tự động dành cho đọc văn bản. Nó không lật trang PDF, cuộn ảnh hoặc bật Smart Mirror. F8 dừng cuộn, đồng thời che nội dung riêng và dừng trình chiếu.

### Cỡ chữ, độ đậm và xuống dòng

- Dùng thanh công cụ xem để tăng hoặc giảm cỡ chữ.
- Nhập cỡ chính xác theo pt rồi nhấn Enter để áp dụng.
- Văn bản và lời nhắc hỗ trợ 8-200 pt.
- Chọn Thường, Vừa hoặc Đậm cho văn bản và lời nhắc.
- Bật Xuống dòng để đưa phần dài của dòng xuống hàng hiển thị tiếp theo.
- Tắt xuống dòng khi cần giữ bố cục dòng gốc.

Mặc định văn bản và lời nhắc là 12 pt, tương ứng tỷ lệ ban đầu 50%. Ctrl+0 trở về mặc định của nội dung đang xem. Code, ảnh, PDF và web có điều khiển zoom riêng.

Đổi cỡ, độ đậm, xuống dòng và theme giữ điểm đọc và vùng chọn trong các trường hợp được hỗ trợ. Điều này giúp chỉnh một lời nhắc dài khi đang đọc dở.

### Chuyển giữa lời nhắc, file và web

1. Tạm dừng cuộn tự động.
2. Chọn tài liệu cần dùng từ **Tệp đang mở** hoặc tab của nó.
3. Với web, chọn Trình duyệt rồi chọn tab cần xem.
4. Quay về tài liệu hiện tại qua **Tệp đang mở**.
5. Kiểm tra đoạn đang hiện trước khi tiếp tục cuộn.

Không gian trình duyệt không yêu cầu quay lại một tab Lời nhắc thường trực. Chỉ giữ tài liệu và tab web thực sự cần cho phiên làm việc.

## 6. Xem tài liệu và chọn giao diện màu

QuietShare bắt đầu zoom nội dung ở 50%. Đây là cài đặt vùng xem nội dung, không phải yêu cầu đổi tỷ lệ màn hình Windows. Chọn kích thước dễ đọc trên màn của bạn.

### Code và ảnh

- Code có trình xem nhận diện cú pháp và nút xuống dòng.
- Code, ảnh và PDF có zoom 25-400%.
- Dùng nút zoom hoặc Ctrl+- và Ctrl++ trong trình xem tương ứng.
- Ctrl+0 trở về tỷ lệ nội dung mặc định.
- Trình xem ảnh giữ màu điểm ảnh. Theme không đổi màu bức ảnh.
- GIF chỉ hiện khung hình đầu tiên.

Nếu dòng code quá dài, bật xuống dòng. Khi cần xem thụt lề hoặc căn hàng, giữ bố cục dòng gốc và di chuyển ngang nếu cần.

### Đọc PDF

1. Mở PDF cục bộ được hỗ trợ.
2. Dùng Page Up và Page Down để đổi trang.
3. Dùng menu mở rộng để chuyển trang hoặc chọn vừa chiều ngang.
4. Kéo phần trống của trang để di chuyển.
5. Kéo trên chữ để chọn nếu PDF có lớp văn bản.
6. Dùng Ctrl+F để tìm trong phần chữ có thể tìm kiếm.

Trang quét không có lớp chữ sẽ không cung cấp tìm hoặc sao chép văn bản thông thường. QuietShare dựng từng trang PDF. Ứng dụng không sửa nội dung PDF hoặc gỡ mật khẩu.

### Tìm chữ

1. Nhấn Ctrl+F.
2. Nhập từ cần tìm.
3. Nhấn Enter để đến kết quả tiếp.
4. Nhấn Shift+Enter để về kết quả trước.
5. Nhấn Esc để đóng vùng tìm kiếm.

Cách tìm tùy trình xem đang hoạt động. Tìm chữ trong PDF cần lớp văn bản.

### Chọn một trong mười theme

Mở **Cài đặt > Giao diện > Chủ đề màu** hoặc dùng nút theme trên thanh công cụ xem.

- Tối cổ điển
- Sáng
- Giấy ấm
- Xanh đêm
- Rừng đêm
- Than chì
- Sương xanh
- Xanh ban ngày
- Đen tương phản cao
- Trắng tương phản cao

Thử theme sáng hoặc tương phản cao trong phòng nhiều ánh sáng. Thử Giấy ấm nếu muốn nền đọc dịu hơn. Lựa chọn được nhớ qua các lần mở ứng dụng. Theme đổi giao diện QuietShare và phần trình bày chữ. Nó không đổi màu nội dung website, điểm ảnh hoặc trang PDF.

Nếu chữ nhạt, tăng độ mờ cửa sổ về gần 100%, sau đó chỉnh cỡ và độ đậm. Hình thức phông chữ còn phụ thuộc tỷ lệ Windows, màn hình và cài đặt đồ họa.

## 7. Cài đặt, bố cục và phím tắt

Cài đặt chia thành **Giao diện**, **Nội dung**, **Bố cục**, **Trình chiếu**, **Phím tắt** và **Trình duyệt**. Cuộn trong từng nhóm nếu cửa sổ nhỏ không hiện đủ điều khiển.

### Chỉnh cửa sổ cho thuận tiện

- Đổi ngôn ngữ giao diện trong Giao diện.
- Bật hoặc tắt nhớ kích thước cửa sổ lần cuối. Mặc định bật.
- Ghim cửa sổ điều khiển trên cùng khi cần nhìn lời nhắc liên tục.
- Chỉnh độ mờ 40-100%. Thao tác tác động cả chữ của cửa sổ điều khiển. Hình chiếu công khai vẫn đục.
- Chỉnh icon 12-24 px.
- Dùng Bố cục để ẩn hoặc hiện thanh hoạt động, tab, công cụ, trạng thái và Explorer.
- Kéo đường chia Explorer để đổi chiều rộng.
- Dùng F11 để vào hoặc thoát chế độ tập trung.

Với màn hình nhỏ, trước tiên ẩn bảng không cần dùng. Sau đó chỉnh cỡ chữ. Giảm độ mờ có thể làm chữ khó đọc ngay cả khi theme đang có độ tương phản cao.

### Chế độ tài nguyên

Nút tài nguyên chuyển giữa Tiết kiệm 15 FPS, Cân bằng 30 FPS và Hiệu năng 60 FPS. Đây là giới hạn xử lý trình chiếu. Nó không bảo đảm màn hình dựng đúng số khung hình đó.

Bắt đầu với Cân bằng cho slide thông thường. Tiết kiệm có thể đủ với nội dung chủ yếu đứng yên. Hiệu năng có thể hữu ích với chuyển động khi GPU và màn đáp ứng được. Đóng tab web không dùng nếu website tốn tài nguyên. Việc nghỉ của tab nền vẫn phụ thuộc WebView2 và trang web.

### Phím tắt mặc định

| Phím | Thao tác |
|---|---|
| F8 | Che hoặc mở nội dung riêng khi bảo vệ cho phép. Che sẽ dừng chiếu và cuộn. |
| Ctrl+Alt+Q | Gọi QuietShare, giữ focus tiền cảnh và trạng thái che. |
| F9 | Xác minh đường thu hình WGC. |
| F10 | Bắt đầu hoặc dừng trình chiếu Smart Mirror. |
| F11 | Vào hoặc thoát chế độ tập trung. |
| Ctrl+O | Mở file. |
| Ctrl+Shift+O | Mở thư mục dự án. |
| Ctrl+B | Ẩn hoặc hiện Explorer. |
| Ctrl+S | Lưu file văn bản hoặc code đang cho phép sửa. |
| Ctrl+Tab | Đến tài liệu hoặc tab web tiếp trên trang đang dùng. |
| Ctrl+Shift+Tab | Đến tài liệu hoặc tab web trước. |
| Ctrl+Enter | Chuyển soạn và đọc lời nhắc. |
| Space | Chạy hoặc tạm dừng cuộn tự động trong vùng đọc. |
| Ctrl+F | Tìm trong trình xem được hỗ trợ đang hoạt động. |
| Ctrl+- / Ctrl++ | Giảm hoặc tăng cỡ chữ hay zoom nội dung hiện tại. |
| Ctrl+0 | Khôi phục cỡ hoặc zoom mặc định của nội dung hiện tại. |
| Ctrl+T / Ctrl+W | Mở hoặc đóng tab web trong ngữ cảnh trình duyệt. |

F8 và Ctrl+Alt+Q chỉ dùng toàn hệ thống khi đăng ký với Windows thành công. Các phím khác tùy vùng nhận focus và trang đang hoạt động.

Để đổi một trong mười phím tắt tùy chỉnh:

1. Mở **Cài đặt > Phím tắt**.
2. Chọn tổ hợp cần đổi.
3. Nhấn tổ hợp mới.
4. Đọc thông báo trùng hoặc phím dành riêng nếu có.
5. Nhấn Esc để hủy thay đổi chưa hoàn thành.

Tổ hợp trùng hoặc dành riêng sẽ bị từ chối. Ứng dụng khác giữ phím có thể khiến đăng ký toàn hệ thống thất bại. Khi đó dùng điều khiển trên màn hình hoặc phím dự phòng cục bộ.

## 8. Dùng trình duyệt tùy chọn

Trình duyệt tắt ở mỗi lần mở ứng dụng. Tính năng cần Microsoft Edge WebView2 Runtime. Bật từ không gian Trình duyệt hoặc **Cài đặt > Trình duyệt** khi cần.

### Mở trang

1. Bật trình duyệt.
2. Đợi xác minh capture trước khi tải trang người dùng.
3. Nhập địa chỉ HTTP hoặc HTTPS, tên miền, địa chỉ localhost hoặc từ khóa trên trang Tab mới cục bộ.
4. Xác nhận địa chỉ hoặc tìm kiếm.
5. Dùng Quay lại, Tiến tới và Tải lại khi cần.

Từ khóa được gửi tới Google. Website bạn mở nhận các yêu cầu mạng thông thường. Trình đọc tài liệu cục bộ không cần các kết nối này.

### Tab, lối tắt và zoom

- Dùng Ctrl+T để mở tab.
- Dùng Ctrl+W trong trình duyệt để đóng tab hiện tại.
- Dùng Ctrl+Tab hoặc Ctrl+Shift+Tab để chuyển tab.
- Có thể mở tối đa tám tab.
- Các tab dùng chung hồ sơ đăng nhập cục bộ.
- Mỗi tab giữ lịch sử điều hướng và zoom riêng trong phiên.
- Tối đa mười lối tắt website được lưu cục bộ.
- Tab đang mở không được khôi phục ở lần chạy tiếp theo.
- Đóng tab cuối cùng sẽ tắt trình duyệt.

Tab mới phải qua xác minh capture. Tab nền nhận yêu cầu tạm ngưng. Trang hoặc bộ máy trình duyệt vẫn có thể tiếp tục một số hoạt động dù đã có yêu cầu đó.

Nếu bật trình duyệt khi đang chiếu, QuietShare giữ khung hình công khai gần nhất trong lúc xác minh. Thời gian này có thể tới khoảng năm giây. Thất bại hoặc hủy sẽ dừng chiếu. Kiểm tra kết quả trình chiếu trước khi tiếp tục.

### Dán ảnh vào ô soạn web

1. Sao chép ảnh raster vào clipboard Windows.
2. Mở ô soạn tương thích trên website cần dùng.
3. Nhấp vào ô soạn để nó nhận phím.
4. Tự nhấn Ctrl+V.
5. Kiểm tra ảnh xem trước của website trước khi gửi nội dung.

Khả năng tương thích tùy ô soạn của website. Đọc clipboard tự động bị từ chối. Chỉ sao chép URL ảnh hoặc tham chiếu file không giống sao chép dữ liệu điểm ảnh.

### Giới hạn và quyền thiết bị

Trình duyệt nhúng chặn hộp chọn file web native, thả file, tải xuống, chia sẻ màn hình, thông báo, DevTools và menu native. Luồng đăng nhập cần popup riêng có thể không hoạt động.

Camera và micro mặc định tắt. Khi bật tùy chọn thiết bị, quyền native vẫn yêu cầu:

- Tab đang hiển thị và hoạt động.
- Trang đã được xác minh capture.
- HTTPS.
- Yêu cầu cùng origin với trang chính.
- Windows cho phép dùng thiết bị.

Quyền thiết bị của website không được lưu. Thu hồi quyền có thể tải lại tab để dừng luồng thiết bị. Dùng ứng dụng họp thông thường khi cần chia sẻ màn hình công khai trong cuộc họp.

## 9. Hiểu bảo vệ trước khi chia sẻ

QuietShare phân biệt yêu cầu loại trừ Windows, xác minh WGC cục bộ và hình ảnh người xem thực sự nhận được. Duy trì ba kiểm tra riêng trong suốt buổi trình bày.

### Ý nghĩa từng trạng thái

- **Windows đã nhận loại trừ capture:** Windows đọc lại đúng yêu cầu loại trừ cho cửa sổ riêng.
- **WGC đã xác minh:** kiểm tra cục bộ thấy hành vi loại trừ như mong đợi bằng marker tổng hợp trên đường thu đã chọn.
- **Người xem hoặc đầu nhận vật lý chưa được xác minh:** ứng dụng chưa chứng minh hình ảnh thực tế phía người xem. Bạn cần tự quan sát đầu nhận.
- **Bị khóa hoặc xác minh thất bại:** nội dung riêng bị ẩn vì điều kiện bắt buộc chưa đạt.

Windows nhận yêu cầu không phải bảo đảm an toàn chung. WGC đạt cục bộ không chứng nhận mọi phương thức capture của ứng dụng họp, capture card hoặc nguồn OBS.

### Chuẩn bị bề mặt công khai và riêng

1. Mở slide, tài liệu hoặc trình duyệt công khai trong ứng dụng riêng.
2. Giữ trọn cửa sổ điều khiển QuietShare trên màn nguồn.
3. Với Smart Mirror, chọn màn riêng cho người xem trong Extend và cùng GPU.
4. Chạy F9 rồi đọc kết quả.
5. Bắt đầu chiếu bằng F10 khi cấu hình cho phép.
6. Quan sát đầu nhận với tài liệu thử không nhạy cảm.
7. Chỉ mở lời nhắc riêng sau khi các kiểm tra cần thiết đạt.

Nguồn là màn chứa QuietShare và ứng dụng công khai phía sau. Smart Mirror chuyển bản thu công khai của nguồn sang đích. Chỉ mở file trong trình đọc riêng QuietShare không tự biến file đó thành tài liệu công khai được chiếu.

### Những điều kiện gây dừng bảo vệ

Duplicate, mất loại trừ, cấu hình màn không hợp lệ, lỗi thu hoặc dựng hình, ngủ máy và bề mặt riêng không còn an toàn có thể dừng chiếu và khóa nội dung.

Dừng ngoài dự kiến có thể đổi tiêu đề và trạng thái, nháy taskbar và phát âm báo Windows. Ứng dụng không giành focus hoặc mở lại cửa sổ điều khiển đang thu nhỏ. Đọc lý do tại **Cài đặt > Trình chiếu**.

Đầu ra không tự chạy lại sau khi dừng bảo vệ. Sửa điều kiện, xác minh lại và chủ động mở phiên mới. Có thể thu nhỏ cửa sổ điều khiển khi đang chiếu. Trong phiên hoạt động, QuietShare giữ cửa sổ trong vùng làm việc nguồn đã xác minh.

QuietShare không tự đổi chế độ màn Windows. Ứng dụng cũng không ngăn được máy ảnh bên ngoài chụp màn laptop.

## 10. Chiếu qua HDMI hoặc màn hình có dây

Dùng quy trình này cho máy chiếu, TV hoặc màn hình qua HDMI, DisplayPort hay dock. Windows trước tiên phải nhận được màn hình thứ hai có thể dùng.

### Kết nối và nhận diện

1. Lưu công việc và đóng nội dung riêng.
2. Cắm dây màn hình.
3. Chọn đúng đầu vào trên máy chiếu hoặc màn hình.
4. Mở cài đặt màn hình Windows và xác nhận có màn thứ hai.
5. Nhấn Win+P rồi chọn **Extend**.
6. Nhận diện hình chữ nhật nào là laptop, hình nào là màn của người xem.

Nếu Windows chưa nhận màn, xử lý đầu vào, nguồn điện, dây, cổng hoặc driver trước khi chạy QuietShare. TV không dây từng chạy được không chứng minh đầu ra có dây dùng cùng GPU hoặc đường driver.

### Chọn và kiểm tra trong QuietShare

1. Giữ trọn cửa sổ điều khiển trên màn laptop.
2. Mở **Cài đặt > Trình chiếu**.
3. Chọn laptop làm nguồn và máy chiếu làm đích.
4. Xem nhãn vai trò và độ phân giải. Không suy ra vai trò chỉ từ số màn.
5. Đặt tài liệu mẫu công khai trên nguồn, phía sau QuietShare.
6. Nhấn F9 và đợi kết quả.
7. Nhấn F10 hoặc nút Play trình chiếu.
8. Nhìn màn của người xem và xác nhận hiện tài liệu mẫu mà không có lời nhắc riêng.

Nếu Play thất bại, ứng dụng mở cài đặt Trình chiếu kèm mã. Ghi lại mã trước khi thử tiếp. Nội dung riêng biến mất cho biết cổng bảo vệ chưa đạt. Bấm Play liên tục không sửa điều kiện đang lỗi.

### Một số kết quả thường gặp với HDMI

- `QS-P001`: Windows chưa cung cấp đủ hai màn cho quy trình mirror.
- `QS-P003`: ứng dụng không nhận diện được adapter của một màn.
- `QS-P004`: adapter nguồn và đích đã nhận diện khác nhau.
- `QS-P005`: cửa sổ điều khiển riêng chưa nằm trọn trên nguồn đã chọn.
- `QS-P013`: xác minh chưa chứng minh loại trừ trên đường thu được thử.

Các điều kiện này không chứng minh dây hoặc máy chiếu bị hỏng. Tra đúng mục sửa tại chương 13.

### Kết thúc

1. Dừng trình chiếu QuietShare bằng F10.
2. Dừng chia sẻ cuộc họp hoặc ghi hình trong ứng dụng tương ứng.
3. Đóng hoặc che nội dung riêng.
4. Chỉ đổi chế độ Windows hoặc rút dây sau khi kết thúc phiên.

## 11. Chiếu qua TV không dây

Hỗ trợ không dây phụ thuộc cách Windows và driver cung cấp kết nối. QuietShare không tạo liên kết không dây và không mặc định nó tương đương HDMI.

### Tạo kết nối

1. Chuẩn bị TV ở chế độ màn hình không dây mà TV hỗ trợ.
2. Dùng quy trình kết nối màn hình không dây của Windows.
3. Đợi kết nối hoàn thành.
4. Mở cài đặt màn hình Windows và xác nhận TV hiện như màn riêng.
5. Chọn **Extend**, không chọn Duplicate.
6. Nhận diện laptop và TV trước khi mở lời nhắc riêng.

Nếu đầu nhận chỉ là luồng từ xa, không xuất hiện như màn Windows riêng, nó không phải đích Smart Mirror theo quy trình này.

### Bật phiên đã xác minh

1. Đặt trọn QuietShare trên màn laptop.
2. Mở **Cài đặt > Trình chiếu**.
3. Chọn laptop làm nguồn và TV làm đích.
4. Mở tài liệu mẫu công khai trên nguồn.
5. Chạy F9.
6. Bắt đầu bằng F10 nếu xác minh đạt.
7. Quan sát TV trực tiếp.

Màn không dây được cung cấp qua adapter khác có thể không được hỗ trợ. Đọc `QS-P003` hoặc `QS-P004` nếu xuất hiện. Phiên từng thành công là thông tin hữu ích, nhưng cấu hình màn hiện tại vẫn cần xác minh.

### Kết nối lại và hiệu năng

- Sau ngắt kết nối, cần chờ trình chiếu dừng.
- Kết nối lại trong Windows rồi khôi phục Extend.
- Chọn lại cặp màn hiện tại nếu định danh hoặc số màn đã đổi.
- Chạy F9 và chủ động bắt đầu lại.
- Dùng Cân bằng hoặc Tiết kiệm trước nếu tài liệu chủ yếu đứng yên.
- Độ trễ mạng và hành vi đầu nhận vẫn có thể ảnh hưởng hình chiếu.

Tăng FPS không sửa đường thu chưa xác minh. Trước tiên sửa điều kiện được báo, sau đó đánh giá chuyển động và tài nguyên.

## 12. Cuộc họp, OBS và phục hồi phiên

Dùng điều khiển của ứng dụng thực sự chia sẻ hoặc ghi hình. Nút che và dừng của QuietShare tác động QuietShare. Chúng không điều khiển luồng trực tiếp của ứng dụng khác.

### Ghi âm âm thanh cuộc họp trên máy này

QuietShare có thể ghi âm luồng phát mặc định của Windows khi bạn chủ động bật nút ghi. Tính năng phù hợp khi âm thanh cuộc họp, video thuyết trình hoặc người tham dự từ xa đang phát qua chính máy này.

1. Chọn **Ghi âm cuộc họp** trên thanh bên.
2. Xác nhận icon ghi màu đỏ và thanh trạng thái cho biết đang ghi.
3. Giữ âm thanh cuộc họp trên đúng thiết bị phát Windows cần dùng.
4. Dùng vùng soạn lời nhắc để ghi chú thủ công trong lúc bản ghi đang chạy.
5. Chọn **Dừng ghi âm** khi kết thúc hoặc đóng QuietShare để dừng và hoàn tất file.
6. Tìm file WAV tại `%LOCALAPPDATA%\QuietShare\Meetings`.

Bộ ghi chỉ lấy âm thanh phát ra từ máy. Nó không mở micro, không tự ghi khi khởi động và không đưa âm thanh vào đường trình chiếu của QuietShare. Nếu thiết bị phát thay đổi, bản ghi có thể dừng và để lại file WAV một phần nhưng đã hoàn tất. Phiên bản hiện tại lưu âm thanh và ghi chú thủ công riêng, chưa tự nhận dạng lời nói.

### Zoom, Meet hoặc Teams với một màn

1. Mở tài liệu công khai trong cửa sổ ứng dụng riêng.
2. Dùng QuietShare làm trình đọc riêng.
3. Chạy F9. Xác minh WGC hỗ trợ một màn dù Smart Mirror cần hai màn.
4. Trong ứng dụng họp, chọn chia sẻ cửa sổ ứng dụng công khai gốc.
5. Không chọn cửa sổ QuietShare riêng.
6. Quan sát đầu nhận cuộc họp trên thiết bị khác bằng tài liệu mẫu không nhạy cảm.
7. Chỉ tiếp tục khi đầu nhận có đúng tài liệu công khai và không có lời nhắc riêng.
8. Kết thúc bằng nút dừng chia sẻ của ứng dụng họp.

Nếu nội dung riêng xuất hiện, dừng chia sẻ cuộc họp ngay. Kiểm tra cục bộ đạt không chứng minh ứng dụng họp dùng cùng phương pháp thu hình.

### Vừa họp trực tuyến vừa dùng máy chiếu

1. Hoàn thành chương 10 hoặc 11 cho màn vật lý.
2. Ưu tiên cửa sổ ứng dụng công khai gốc làm nguồn cuộc họp trực tuyến.
3. Kiểm tra cả đầu nhận vật lý lẫn đầu nhận cuộc họp bằng nội dung mẫu.
4. Kiểm tra lại sau khi đổi chế độ màn hoặc nguồn capture của cuộc họp.
5. Dừng cả chia sẻ cuộc họp và trình chiếu QuietShare khi kết thúc.

Cửa sổ đầu ra Smart Mirror cũng yêu cầu loại trừ capture. Thu màn đích có thể bỏ qua đầu ra đó hoặc hiện vùng trống. Chia sẻ cửa sổ công khai gốc giúp tránh phụ thuộc vào việc thu cửa sổ đầu ra đang được loại trừ.

### Ghi hoặc phát trực tiếp bằng OBS

1. Tạo nguồn OBS cho ứng dụng công khai gốc.
2. Ưu tiên Window Capture khi phù hợp với ứng dụng đó.
3. Nếu dùng Display Capture, chọn đúng màn công khai và lưu ý các cửa sổ đầu ra bị loại trừ.
4. Không chọn cửa sổ QuietShare riêng làm nguồn OBS.
5. Ghi thử một đoạn ngắn không chứa thông tin bí mật.
6. Phát lại và xem nội dung thực sự được ghi.
7. Kiểm tra lại sau khi đổi nguồn, backend capture hoặc bố trí màn hình.
8. Dừng ghi hoặc phát trong OBS khi kết thúc.

WGC đạt không chứng nhận mọi backend capture của OBS. Capture card hoặc thiết bị ghi bên ngoài có thể đi theo đường khác nữa.

### Duplicate và phục hồi

Duplicate không hỗ trợ thuyết trình có nội dung riêng. QuietShare khóa khi phát hiện nhân đôi màn. Nếu cần hiện cùng tài liệu công khai, đóng lời nhắc riêng và dùng ứng dụng công khai trực tiếp.

Sau Duplicate về Extend, cắm lại dây, ngủ máy hoặc GPU reset:

1. Khôi phục Extend trong Windows.
2. Đợi đủ màn cần dùng xuất hiện.
3. Đưa trọn QuietShare lên nguồn cần dùng.
4. Chọn nguồn và đích hiện tại.
5. Chạy F9 rồi đọc kết quả.
6. Chủ động bắt đầu chiếu bằng F10.
7. Kiểm tra lại mọi đầu nhận cần dùng.

Nếu Play không hiện gì và nội dung riêng bị ẩn, đọc mã trong **Cài đặt > Trình chiếu**. Không bỏ kiểm tra bảo vệ. Chương 13 có các bước sửa theo thứ tự.

## 13. Danh mục mã lỗi trình chiếu

Mã cho biết điều kiện phát hiện. Chúng không tự kết luận phần cứng hỏng. Lý do nội bộ hiện cạnh mã ổn định để hỗ trợ kỹ thuật.

Sau khi sửa, xác minh lại và quan sát đầu nhận bằng nội dung không nhạy cảm. Nếu điều kiện vẫn còn, giữ lời nhắc riêng đóng và giữ bản ghi chẩn đoán theo chương 14.

### QS-P001 `need-two-monitors`

**Điều kiện:** Smart Mirror nhận ít hơn hai màn.

1. Kết nối, bật đầu nhận và chọn đúng đầu vào.
2. Xác nhận cài đặt màn Windows hiện hai màn riêng.
3. Chọn Extend rồi chọn lại nguồn và đích trong QuietShare.

Đọc trên một màn và xác minh WGC cục bộ không bắt buộc Smart Mirror.

### QS-P002 `same-monitor`

**Điều kiện:** Nguồn và đích là cùng một màn.

1. Nhận diện laptop và đầu nhận trong cài đặt màn Windows.
2. Chọn hai màn khác nhau làm nguồn và đích.
3. Giữ QuietShare trên nguồn, chạy F9 rồi F10.

### QS-P003 `adapter-unknown`

**Điều kiện:** QuietShare không nhận diện được adapter GPU của một màn đã chọn.

1. Chờ kết nối màn hoàn thành rồi chọn lại màn.
2. Kiểm tra driver đồ họa và adapter màn của laptop. Kết nối lại đầu nhận nếu cần.
3. Mở lại QuietShare rồi thử. Nếu vẫn không nhận diện được, gửi mã và kiểu kết nối.

Mã này chưa xác lập rằng hai màn dùng hai GPU khác nhau.

### QS-P004 `different-adapters`

**Điều kiện:** Nguồn và đích đã nhận diện thuộc hai adapter GPU khác nhau. Chưa hỗ trợ sao chép giữa adapter.

1. Xem GPU điều khiển từng màn trong thông tin hiển thị của laptop hoặc nhà sản xuất đồ họa.
2. Nếu phần cứng hỗ trợ, dùng cổng hoặc cấu hình mà cả hai màn được một adapter điều khiển.
3. Nếu không có cấu hình cùng adapter phù hợp, dùng ứng dụng công khai trực tiếp, không dùng lời nhắc riêng theo quy trình chiếu này.

Ưu tiên đồ họa trong Windows không bảo đảm đổi GPU nối vật lý với cổng xuất hình.

### QS-P005 `control-location`

**Điều kiện:** Cửa sổ điều khiển riêng không nằm trọn trong màn nguồn đã chọn.

1. Đưa toàn bộ cửa sổ QuietShare lên nguồn đã chọn.
2. Khôi phục hoặc giảm kích thước nếu cửa sổ vắt qua hai màn.
3. Kiểm tra nguồn được chọn rồi xác minh lại.

### QS-P006 `windows-unsupported`

**Điều kiện:** Windows dưới build 19041.

1. Xem phiên bản và build Windows.
2. Cập nhật Windows được hỗ trợ, phù hợp quy định của tổ chức.
3. Mở lại QuietShare rồi xác minh.

### QS-P007 `dwm-unavailable`

**Điều kiện:** Bộ dựng desktop không khả dụng cho quy trình thu hình bắt buộc.

1. Trở về phiên desktop Windows bình thường.
2. Kiểm tra driver đồ họa và khả năng dựng desktop.
3. Khởi động lại Windows nếu cần, sau đó mở QuietShare và thử.

### QS-P008 `control-affinity`

**Điều kiện:** Không nhận hoặc không giữ được loại trừ capture của cửa sổ riêng.

1. Đóng rồi mở lại QuietShare.
2. Xác nhận Windows được hỗ trợ và driver đồ họa hoạt động.
3. Chạy F9. Giữ nội dung riêng đóng nếu vẫn không nhận loại trừ.

### QS-P009 `private-popup-affinity`

**Điều kiện:** Hộp thoại hoặc popup riêng thiếu loại trừ capture bắt buộc.

1. Đóng hộp thoại và menu riêng.
2. Trở về cửa sổ điều khiển rồi xác minh lại.
3. Nếu cùng thao tác gây lặp lại, gửi thao tác mở popup, không gửi nội dung riêng trong đó.

### QS-P010 `topology-unavailable`

**Điều kiện:** Không đọc được cấu hình màn Windows.

1. Chờ kết nối, gắn dock hoặc đổi chế độ màn hoàn thành.
2. Xác nhận Extend trong Windows và chọn lại các màn hiện tại.
3. Kiểm tra driver hoặc khởi động Windows lại nếu vẫn còn điều kiện lỗi.

### QS-P011 `display-cloned`

**Điều kiện:** Phát hiện Duplicate hoặc nhân đôi màn.

1. Đóng hoặc che nội dung riêng.
2. Nhấn Win+P rồi chọn Extend.
3. Chọn lại cặp màn, chạy F9 và chủ động bật bằng F10.

Nếu phải giữ Duplicate, dùng tài liệu công khai trực tiếp mà không mở lời nhắc riêng.

### QS-P012 `capture-frame`

**Điều kiện:** Khởi tạo, thiết lập hoặc lấy khung hình capture thất bại.

1. Khôi phục desktop nguồn sau ngủ máy, ngắt kết nối hoặc chuyển trạng thái đồ họa.
2. Mở lại QuietShare, chọn nguồn rồi xác minh lại.
3. Nếu lặp lại, kiểm tra driver đồ họa và kèm HRESULT được ghi khi báo lỗi.

### QS-P013 `canary-failed`

**Điều kiện:** Marker xác minh tổng hợp chưa chứng minh loại trừ capture trên đường được thử.

1. Đóng hộp thoại riêng đang phủ và hoàn thành các thay đổi màn.
2. Giữ QuietShare trên nguồn đã chọn rồi chạy F9 lại.
3. Nếu vẫn thất bại, giữ lời nhắc bị khóa và gửi mã cùng phiên bản ứng dụng.

Windows nhận loại trừ không thay thế được kết quả xác minh thất bại này.

### QS-P014 `render-frame`

**Điều kiện:** Không tạo hoặc dựng đúng đầu ra D3D11 cho người xem.

1. Xác nhận màn đích vẫn kết nối và sẵn sàng.
2. Chọn lại nguồn và đích, xác minh rồi thử.
3. Mở lại QuietShare và kiểm tra driver đồ họa nếu vẫn lỗi.

### QS-P015 `display-topology`

**Điều kiện:** Cấu hình màn hoặc capture đã chọn thay đổi.

1. Khôi phục cấu hình Extend cần dùng.
2. Chọn nguồn và đích hiện tại, không dựa vào số màn cũ.
3. Chạy F9, bật F10 và xem đầu nhận.

### QS-P016 `device-removed`

**Điều kiện:** Thiết bị GPU dùng capture không còn khả dụng hoặc đã reset.

1. Lưu công việc và khôi phục các màn cần dùng.
2. Đóng rồi mở lại QuietShare, sau đó xác minh lại.
3. Nếu lặp lại, kiểm tra driver đồ họa và gửi HRESULT cùng thao tác ngay trước lỗi.

### QS-P017 `projector-affinity`

**Điều kiện:** Cửa sổ đầu ra công khai mất điều kiện loại trừ capture bắt buộc.

1. Dừng phiên và giữ tài liệu riêng đóng.
2. Mở lại QuietShare, khôi phục cặp màn và xác minh.
3. Báo lỗi nếu lặp lại. Không tắt cơ chế bảo vệ để tiếp tục.

### QS-P018 `projector-closed`

**Điều kiện:** Cửa sổ đầu ra cho người xem đã bị đóng.

1. Xác nhận bạn có chủ động đóng cửa sổ không.
2. Quay về QuietShare và kiểm tra các màn đang chọn.
3. Xác minh và chủ động bắt đầu lại khi sẵn sàng mở lại đầu ra.

### QS-P019 `projector-escape`

**Điều kiện:** Phím Escape đã dừng đầu ra cho người xem.

1. Kiểm tra có chủ động nhấn phím trong cửa sổ đầu ra đó không.
2. Quay về cửa sổ điều khiển.
3. Xác minh và chủ động bắt đầu lại khi sẵn sàng.

### QS-P020 `system-suspend`

**Điều kiện:** Ngủ hoặc tạm ngưng máy đã ngắt trình chiếu.

1. Đánh thức laptop và đầu nhận.
2. Chờ màn kết nối lại rồi khôi phục Extend.
3. Chọn lại cặp màn, xác minh và chủ động mở phiên mới.

### QS-P021 `winrt-unavailable`

**Điều kiện:** Hỗ trợ capture Windows Runtime cần thiết không khả dụng.

1. Xác nhận Windows đáp ứng phiên bản được hỗ trợ.
2. Mở lại QuietShare và khởi động Windows lại nếu cần.
3. Báo lỗi nếu vẫn xảy ra trên Windows phù hợp.

### QS-P022 `privacy-unsafe`

**Điều kiện:** Một kiểm tra bảo vệ thất bại nhưng chưa ghi được lý do cụ thể hơn.

1. Kiểm tra Extend và trạng thái loại trừ capture Windows.
2. Đóng hộp thoại riêng, giữ cửa sổ điều khiển trên nguồn rồi xác minh lại.
3. Giữ lời nhắc đóng và gửi phiên bản ứng dụng nếu lỗi lặp lại.

### QS-P023 `environment-invalidated`

**Điều kiện:** Môi trường capture không còn được xác minh.

1. Khôi phục cấu hình màn cần dùng.
2. Chọn lại nguồn, đích rồi chạy F9.
3. Nếu lặp lại, ghi thao tác ngay trước khi mất xác minh rồi gửi lỗi.

### QS-P024 `source-monitor-changed`

**Điều kiện:** Màn nguồn biến mất hoặc thay đổi khi di chuyển cửa sổ điều khiển.

1. Khôi phục kết nối nguồn và Extend cần dùng.
2. Đặt toàn bộ cửa sổ điều khiển trên nguồn đó.
3. Chọn lại, xác minh và chủ động bắt đầu lại.

### QS-P000 - chưa có lý do lỗi

Đây là trạng thái chưa xác minh, không phải chẩn đoán phần cứng.

1. Chọn nguồn cần dùng.
2. Chạy F9 rồi đợi kết quả.
3. Đọc mã cụ thể xuất hiện sau đó nếu có.

### QS-P099 - lý do chưa biết

Mã dự phòng này biểu thị lý do chưa có ánh xạ công khai được nhận diện.

1. Giữ lời nhắc riêng đóng.
2. Giữ lại lý do nội bộ và phiên bản ứng dụng.
3. Gửi bản ghi theo chương 14.

## 14. Chẩn đoán và kiểm tra trước buổi trình bày

### Tìm bản ghi trình chiếu gần nhất

Sau lần bắt đầu thất bại, đọc **Cài đặt > Trình chiếu**. Từ v0.14.4, QuietShare mở nhóm này khi khởi động trình chiếu thất bại.

Nếu cần, mở `%LOCALAPPDATA%\QuietShare\projection-stop.json` bằng trình soạn thảo. Đây là một bản ghi lỗi cục bộ gần nhất, bị ghi đè khi có lỗi sau. Không có file không chứng minh chưa từng có lỗi. Bản cũ có thể chỉ ghi lỗi trong phiên đang chạy.

Bản ghi gồm:

- `code`: mã công khai trong sổ tay.
- `reason`: lý do kỹ thuật nội bộ.
- `version`: phiên bản ứng dụng.
- Giờ UTC: thời điểm ghi lỗi.
- `phase`: `start`, `verification` hoặc `running`.
- `hresult`: kết quả kỹ thuật đính kèm khi có.
- `sessionMs`: thời gian phiên, bằng không trước khi trình chiếu bắt đầu.
- `resourceMode`: chế độ xử lý được chọn.

`0x00000000` nghĩa là không đính kèm HRESULT thất bại. Nó không có nghĩa trình chiếu thành công. Bản ghi không chứa ảnh, chữ riêng, đường dẫn file, URL hoặc định danh màn hình. Ghi chẩn đoán thất bại không cho phép trình chiếu.

### Báo lỗi có thể lặp lại

1. Ghi phiên bản ứng dụng trên Home.
2. Sao chép mã chính xác và lý do nội bộ.
3. Kèm phase và HRESULT nếu có.
4. Nêu phiên bản Windows và kiểu kết nối như HDMI, không dây hoặc dock.
5. Nêu Windows có nhận đầu nhận không và đang Extend hay Duplicate.
6. Liệt kê thao tác ngay trước lỗi cùng kết quả mong đợi.
7. Xem lại bản ghi nhỏ hoặc cắt ảnh về vùng trạng thái trước khi chia sẻ.

Không gửi lời nhắc riêng, thông tin đăng nhập hoặc nội dung tài liệu cá nhân để minh họa lỗi. Mã trên laptop bị ảnh hưởng hữu ích hơn nhật ký không liên quan từ máy khác.

### Một số sự cố khác

- **Không mở được file:** kiểm tra đường dẫn cục bộ, định dạng và dung lượng ở chương 4. PDF có mật khẩu hoặc mã hóa không hỗ trợ cần một bản tương thích.
- **Không lưu được:** kiểm tra file có bị ứng dụng khác thay đổi không. Giữ bản nháp trước khi tải lại.
- **Không dùng được trình duyệt:** cài hoặc cập nhật [WebView2 Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) chính thức, rồi tắt và bật lại trình duyệt.
- **Không dán được ảnh:** chọn ô soạn website tương thích rồi tự nhấn Ctrl+V với dữ liệu ảnh raster trong clipboard. Hộp tải file web vẫn bị chặn.
- **Chữ nhạt:** dùng độ mờ 100%, theme phù hợp, chữ lớn hơn và độ đậm được hỗ trợ.
- **Phím tắt không chạy:** kiểm tra trang hoạt động, focus bàn phím và xung đột đăng ký toàn hệ thống.

### Trước mỗi buổi trình bày

1. Lưu công việc và kết nối đầu nhận cần dùng.
2. Xác nhận Extend và đúng nguồn, đích nếu dùng Smart Mirror.
3. Xác nhận toàn bộ cửa sổ riêng nằm trên nguồn.
4. Chạy xác minh WGC.
5. Kiểm tra đầu nhận hoặc bản ghi thử bằng tài liệu không nhạy cảm.
6. Kiểm tra F8 để che và nút dừng riêng của từng ứng dụng chia sẻ.

Sau khi đổi màn hoặc đường capture, lặp lại xác minh và quan sát đầu nhận. Phần mềm không bảo đảm chống mọi lỗi hệ điều hành, driver, phương pháp thu hình hoặc máy ảnh bên ngoài.

Lời nhắc và lịch sử gần đây được Windows DPAPI mã hóa cho người dùng cục bộ. Cấu hình và bố cục lưu trên máy. Cookie và đăng nhập trình duyệt dùng hồ sơ WebView2 cục bộ riêng. Đây là các cơ chế lưu trữ khác nhau. Gỡ ứng dụng vẫn giữ lời nhắc đã lưu.

Lý do lỗi trình chiếu mới cần có mã ổn định và các bước phục hồi theo thứ tự trong cả hai ấn bản trước khi phát hành. Danh mục dành cho điều kiện trình chiếu đã hiện thực. Không gán mã giả cho lỗi website hoặc file không liên quan.

[Tải bản phát hành](https://github.com/TDUmii/QuietShare/releases/latest) | [Mục lục tài liệu](README.md) | [Chi tiết quyền riêng tư](PRIVACY.md) | [Xử lý sự cố](TROUBLESHOOTING.md)
