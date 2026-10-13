# Kiểm thử QuietShare 0.3.0

Ngày kiểm tra: 13/10/2026, múi giờ Asia/Saigon. Bản native hỗ trợ lời nhắc, code và ảnh cục bộ.

## Kết quả bản phát hành

Tất cả nhóm kiểm tra dưới đây đạt trên Windows 11 x64 build 26200. Lệnh tái hiện: `./tests/run-local.ps1` trong PowerShell 7.

| Nhóm | Kết quả | Báo cáo cục bộ |
|---|---|---|
| Giao diện, thao tác, capture mẫu, lưu và bộ cài | 179 đạt, 0 thất bại, 0 unavailable | `artifacts/local-qa-report.json` |
| Bộ đọc nội dung và render ảnh | 60 đạt, 0 thất bại | `artifacts/qa-content/content-regression.json` |
| Vòng đời production EXE | 29 đạt, 0 thất bại | `artifacts/lifecycle-check.json` |
| Scrollbar native trên EXE thật | 17 đạt, 0 thất bại | `artifacts/scrollbar-check.json` |
| PE, version resource và payload bộ cài | 7 đạt, 0 thất bại | `artifacts/release-inventory.json` |

| Hạng mục | Bằng chứng và phạm vi |
|---|---|
| Chuẩn bị và Tùy chỉnh | Điều hướng qua nút thật; Home ẩn editor; slider thay cỡ chữ thực tế của control; mở file qua handler `WM_DROPFILES` |
| Focus | Sidebar và toolbar được ẩn; nút F11 mở lại điều khiển; ảnh text và ảnh riêng xác nhận nút không bị canvas vẽ đè |
| Code | Preview chỉ đọc dùng Consolas; giữ nguyên thụt dòng; file không được thực thi; quay về lời nhắc phục hồi đúng draft |
| Ảnh | WIC đọc PNG/JPEG/BMP/GIF khung đầu; kiểm tra alpha, tỷ lệ, orientation, zoom, pan và clip đúng viewport; màu của ảnh fixture được xác nhận trên capture tích hợp |
| Pan regression | 5 kiểm tra zoom 200% bắt được cả 5 lỗi khi chạy với bản mô phỏng cách clip cũ; module sản xuất đạt toàn bộ |
| Lỗi mở file | File không tồn tại, encoding hỏng, binary, định dạng không hỗ trợ, vượt giới hạn và đường dẫn không cục bộ không thay nội dung đang mở |
| Giữ lời nhắc khi xem file | Production EXE mở preview code qua HDROP tổng hợp; encrypted draft không đổi sau autosave; MODE100 phục hồi bản sửa; đóng app khi đang preview rồi mở lại khôi phục đúng draft |
| Typography và resize | 150 tổ hợp: 5 kích thước 430/460/640/840/1080 px, 5 mode Sửa/Đọc/Che/Home/Settings, VI/EN, scale logic 96/120/144 DPI. Control nằm trong khung, không chồng nhau hoặc đè vùng editor thông thường; editor còn tối thiểu 80 px logic |
| Visual review | Đã xem render native Home/Settings/Focus/code/ảnh, cửa sổ tối thiểu, empty, hover, pressed, masked, tiếng Anh và scale logic 150%; reviewer độc lập không thấy visual blocker |
| Scrollbar | Kiểm tra track tại idle, trên 6 giây, hover, rời chuột, theme change, Page Down, phím điều hướng, wheel, kéo thumb và resize: giữ tối; nội dung thực sự cuộn |
| Cuộn và dừng | Timer thật làm vị trí cuộn tăng từ 0 lên 25 pixel trong lượt QA; dừng giữ nguyên vị trí |
| Che nhanh | Handler F8 ẩn editor/canvas, dừng cuộn; mở lại giữ nội dung. Mở file bị chặn khi nội dung đang khóa |
| Tự lưu | EN_CHANGE kích hoạt lưu sau khoảng 0,9 giây; DPAPI đọc lại đúng Unicode; marker không xuất hiện dạng plaintext |
| Bản lưu hỏng và nhiều instance | Tệp hỏng được giữ nguyên từng byte trong bản dự phòng; instance thứ hai không tạo editor cạnh tranh cùng tài liệu |
| Affinity và capture | Windows chấp nhận và đọc lại `0x11`, Win32 error 0; GDI BitBlt chỉ thấy nền tím mẫu khi bật affinity; render giao diện tạm tắt affinity và chỉ dùng nội dung QA |
| Bộ cài | Self-test version, PE x64, payload và đường dẫn theo tài khoản đạt; UI cài/gỡ cùng palette dark; chụp QA không chạy thao tác cài/gỡ |
| Release parity | Resource301 trong Setup khớp từng byte với portable EXE; ZIP được đọc lại để so từng file với payload đã kiểm tra |
| Phụ thuộc và chữ ký | Chỉ import DLL hệ thống Windows, runtime GCC liên kết tĩnh; ứng dụng và bộ cài đều `NotSigned` theo Authenticode |

QA dùng thư mục dữ liệu riêng và file tổng hợp. Không mở tài khoản, dùng clipboard cá nhân, cài app vào máy hoặc chụp toàn bộ desktop. Báo cáo thô, dữ liệu tự lưu và fixture nằm trong `artifacts/`, được loại khỏi Git và ZIP.

## Giới hạn kiểm tra

- Capture đạt ở đường GDI `BitBlt` với nền mẫu tại thời điểm chụp. Chưa kiểm tra đầu nhận Zoom, Teams, Meet hoặc OBS; `remoteReceiverVerified` vẫn là `null`.
- Chưa thử máy chiếu, rút/cắm màn hình hoặc đo khung hình chuyển tiếp. Duplicate vật lý phát cùng hình ảnh; cần Extend để có màn hình riêng.
- Scale DPI trong matrix được mô phỏng bằng scale logic, không phải chuyển monitor hoặc đổi DPI hệ điều hành thực.
- Chưa kiểm tra Windows 10, ARM64/x86, GPU khác hoặc cài/gỡ trên máy sạch. Bản phát hành hiện là x64.
- Drop production dùng HDROP tổng hợp theo PID do test mở; chưa kiểm tra kéo file bằng chuột từ Explorer. Native file picker và chuyển topology khi picker đang mở chưa được kiểm thử tương tác tự động.
- Native Release đã bỏ symbol; chưa đo sức chống dịch ngược và không cam kết chống dịch ngược tuyệt đối.

Trước buổi chia sẻ, dùng nội dung mẫu và xem từ phía người nhận trong đúng phần mềm/cấu hình sẽ dùng. Khi dùng máy chiếu, chuyển Extend trước khi mở nội dung riêng.

Kích thước, SHA-256 và trạng thái chữ ký của EXE cuối được lưu trong `release-inventory.json` của gói ZIP.
