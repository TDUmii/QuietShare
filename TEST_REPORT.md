# Kiểm thử QuietShare 0.2.0

Ngày kiểm tra: 13/10/2026, theo múi giờ Asia/Saigon. Bản này chỉ có nội dung nhắc chữ.

## Kết quả của EXE được đóng gói

**94 kiểm tra cục bộ đạt, 0 thất bại**, trên Windows 11 x64 build 26200. Lệnh tái hiện: `./tests/run-local.ps1` trong PowerShell 7. Báo cáo: `artifacts/local-qa-report.json`.

**22 kiểm tra vòng đời production EXE đạt**, gồm bảo toàn bản lưu hỏng, editor thuần text, autosave Unicode, mở lần hai và khôi phục sau khi đóng. Báo cáo riêng: `artifacts/lifecycle-check.json`; script `tests/lifecycle_regression.py` dùng `LOCALAPPDATA` tách biệt và chỉ thao tác cửa sổ theo PID do test mở.

| Hạng mục | Bằng chứng và phạm vi |
|---|---|
| Nhập và đọc | RichEdit thuần text, Unicode tiếng Việt; chuyển Đọc sang chỉ đọc rồi trở lại Sửa lời; nội dung giữ nguyên |
| Cỡ chữ | Thao tác A+ đổi 24 thành 28 pt; kiểm tra định dạng thực tế của control và ảnh render |
| Cuộn và dừng | Timer thật làm vị trí cuộn tăng từ 0 lên 25 pixel trong lượt QA; sau Dừng, vị trí giữ nguyên |
| Che nhanh | Cùng handler F8 ẩn control, dừng cuộn và vẽ màn che; mở lại không đổi nội dung |
| Cửa sổ nhỏ | Cửa sổ 460 x 360 pixel ở DPI hiện tại; các nút và vùng nội dung nằm trong khung; đã xem ảnh render |
| Tự lưu | EN_CHANGE thật kích hoạt lưu sau khoảng 0,9 giây; DPAPI đọc lại đúng tiếng Việt; marker không xuất hiện dạng plaintext trong tệp |
| Giữ nội dung | Tệp hỏng được sao lưu nguyên từng byte trước khi cho nhập lời mới; mở instance thứ hai không tạo cửa sổ sửa cùng tài liệu; đóng/mở production EXE khôi phục đúng lời đã lưu |
| Affinity | Windows chấp nhận và đọc lại `0x11`, Win32 error 0 |
| Capture cục bộ | Ảnh GDI BitBlt trong vùng cửa sổ chỉ thấy nền tím mẫu khi bật affinity; ảnh giao diện dùng nội dung mẫu và tạm tắt affinity để kiểm tra render |
| Bộ cài | Self-test phiên bản, PE x64, payload và đường dẫn theo tài khoản đạt; đã xem trực tiếp giao diện bộ cài 0.2.0 |
| Khớp bản phát hành | 7 kiểm tra resource/phiên bản/PE đạt; EXE nhúng ở resource301 của bộ cài khớp từng byte với EXE chạy trực tiếp |
| Phụ thuộc | EXE ứng dụng chỉ import DLL hệ thống Windows; không có .NET, WebView2 hoặc DLL runtime GCC riêng |
| Chữ ký | Kiểm tra Authenticode thực tế: ứng dụng và bộ cài đều `NotSigned` |

Các lượt QA dùng thư mục dữ liệu riêng. Không mở tài khoản, cài ứng dụng vào máy, hoặc chụp toàn bộ desktop. Đã xem trực tiếp ảnh chế độ đọc, cửa sổ nhỏ, màn che, nền capture và bộ cài.

## Giới hạn

- Capture đạt ở đường GDI `BitBlt` với nền mẫu tại các thời điểm chụp. Chưa kiểm tra đầu nhận Zoom, Teams, Meet hoặc OBS; `remoteReceiverVerified` vẫn là `null`.
- Chưa thử máy chiếu thật, rút/cắm màn hình hoặc đo khung hình chuyển tiếp. Duplicate vật lý phát cùng hình ảnh; cần Extend để có màn hình riêng.
- Chưa kiểm tra Windows 10, ARM64/x86, GPU khác, nhiều DPI hoặc cài/gỡ trên máy sạch. Bản phát hành hiện là x64.
- Native C++ Release đã bỏ symbol; chưa đo sức chống dịch ngược và không cam kết chống dịch ngược tuyệt đối.

Trước buổi chia sẻ, dùng lời mẫu và xem từ phía người nhận với đúng phần mềm/cấu hình sẽ sử dụng. Khi dùng máy chiếu, chuyển Extend trước khi mở lời riêng.

Kích thước và SHA-256 cuối được lưu trong `release-inventory.json` của gói ZIP.
