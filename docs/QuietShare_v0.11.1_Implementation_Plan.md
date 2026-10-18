# QuietShare v0.11.1 - Bộ chọn file và dự án

## Mục tiêu

Bộ chọn file/thư mục có bố cục quen thuộc của File Explorer, đồng bộ giao diện tối và điểm nhấn xanh lá của QuietShare.

## Bố cục và thao tác

- Truy cập nhanh Desktop, Downloads, Tài liệu, Hình ảnh và Máy tính này ở bên trái.
- Vị trí thư mục lấy từ Windows Known Folders, hỗ trợ thư mục được chuyển vị trí trên ổ đĩa cục bộ.
- Thanh địa chỉ cho nhập đường dẫn thư mục; Ctrl+L chọn địa chỉ, Enter mở thư mục.
- Quay lại, tiến tới và lên thư mục cha; Alt+Left/Right/Up điều hướng.
- Danh sách có tên, ngày sửa đổi và kích thước file; Ctrl/Shift chọn nhiều file, nhấp đúp để vào thư mục.
- Dùng chung cho Mở file và Mở dự án; chọn dự án chỉ hiển thị thư mục.

## Bảo vệ và tài nguyên

Bộ chọn được áp dụng capture exclusion trước khi hiển thị. Không mở shell menu hoặc Explorer broker riêng. Giữ kiểm tra đường dẫn cục bộ, loại mục reparse và không cho mở đường dẫn mạng/thiết bị ngoài chính sách. Lịch sử điều hướng chỉ tồn tại trong phiên bộ chọn, tối đa 64 vị trí; danh sách giữ giới hạn 1.000 mục hiện có. Không thêm thư viện, dịch vụ hay tác vụ chạy nền.
