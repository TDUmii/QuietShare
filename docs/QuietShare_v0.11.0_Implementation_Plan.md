# QuietShare v0.11.0 - Home và không gian làm việc

Home dùng bố cục tối gọn, màu xanh lá dịu cho Mở file, nút Mở dự án và danh sách Gần đây lớn. Không dùng hình nền trang trí hay lịch sử mẫu. Trang hỗ trợ tiếng Việt và tiếng Anh.

## Lịch sử cục bộ

File và dự án được thêm sau khi mở thành công. Tối đa 32 mục, không trùng đường dẫn theo chữ hoa/thường; mục ghim được xếp trước. Có thao tác mở lại, ghim/bỏ ghim, xóa một mục và xóa toàn bộ kể cả ghim. Việc xóa lịch sử không xóa file gốc. Lịch sử nằm trong hồ sơ ứng dụng cục bộ, mã hóa bằng Windows DPAPI. Tên mục được che khi cổng bảo vệ nội dung đang khóa.

## Kích thước cửa sổ

Nhớ kích thước cửa sổ nằm trong Cài đặt > Giao diện, mặc định bật. Lưu kích thước cửa sổ bình thường khi đóng, theo đơn vị độc lập DPI; lần mở sau điều chỉnh để vừa vùng làm việc hiện tại. Không khôi phục vị trí trên màn hình cũ hay tự bắt đầu trình chiếu. Tắt tùy chọn để dùng kích thước mặc định ở lần mở sau.

## Chỉnh sửa file

Cho phép sửa file văn bản / code nằm trong Cài đặt > Nội dung, mặc định bật. Văn bản và code hỗ trợ chỉnh sửa, Lưu và Ctrl+S; ảnh và PDF tiếp tục chỉ xem. File nhập vào không dùng cơ chế tự lưu lời nhắc. Bản sửa được giữ theo tab trong phiên. Đóng tab hoặc ứng dụng với file chưa lưu có lựa chọn lưu, bỏ thay đổi hoặc hủy.

Lưu file giữ mã hóa UTF-8/UTF-16 và BOM của file gốc, dùng file tạm và thay thế sau khi ghi hoàn tất. Nếu file đã thay đổi bên ngoài hoặc không truy cập được, không ghi đè; giữ bản sửa trong tab để người dùng xử lý. Giới hạn văn bản/code vẫn là 5 MiB. Tắt chỉnh sửa giữ lại bản sửa chưa lưu trong phiên và chặn thao tác nhập tiếp.

## Bảo vệ và tài nguyên

Home hiển thị riêng việc Windows nhận loại trừ capture và trạng thái người xem/đầu nhận vật lý chưa được xác minh. Không gộp thành một nhãn An toàn. Capture exclusion, cổng xác minh, containment màn hình nguồn, topology và thiết bị tiếp tục quyết định khả năng hiển thị và trình chiếu. Lỗi bảo vệ thực sự dừng đầu ra; không tự khởi động lại.

Khi chưa trình chiếu, bộ kiểm tra UI hoạt động mỗi 250 ms; khi chiếu vẫn dùng nhịp tài nguyên đã chọn. Viewer chỉ nhận resize khi vùng hiển thị thực sự thay đổi. Không thêm vòng lặp nền, dịch vụ hay runtime mới.
