# QuietShare - bàn giao sau v0.12.2

- Cập nhật: 2026-10-18
- Repository: private `TDUmii/QuietShare`
- Nhánh: `main`
- Bản sản phẩm: `v0.12.2`; xác minh commit bằng tag và Release trực tiếp trước khi tiếp tục.

## Sản phẩm hiện tại

Home được bố trí theo ảnh tham chiếu: tối gọn, xanh lá dịu ở Mở file, Mở dự án, Gần đây lớn và trạng thái bảo vệ riêng. Bỏ dòng mô tả mở tài liệu / khám phá dự án / chia sẻ nội dung. Lịch sử thực lấy từ lần mở thành công, tối đa 32 mục, có mở lại/ghim/bỏ ghim/xóa; Windows DPAPI mã hóa trên máy và không có lịch sử mẫu khi chạy sản phẩm.

Cài đặt > Giao diện có Nhớ kích thước cửa sổ, mặc định bật; chỉ khôi phục kích thước bình thường phù hợp vùng làm việc hiện tại. Cài đặt > Nội dung có Cho phép sửa file văn bản / code, mặc định bật. File được lưu chủ động bằng Lưu hoặc Ctrl+S, giữ mã hóa/BOM, chặn ghi đè khi đã đổi bên ngoài và giữ bản sửa theo tab. Đóng khi chưa lưu có lựa chọn lưu/bỏ/hủy. Ảnh và PDF chỉ xem; lời nhắc vẫn tự lưu riêng.

Bộ kiểm tra UI lúc chưa chiếu dùng 250 ms; nhịp khi chiếu giữ nguyên. Viewer chỉ nhận resize khi vùng hiển thị thực sự đổi. Không thêm dịch vụ, driver hay runtime.

## Bảo vệ và thông tin thực tế

Giữ capture exclusion, privacy gate, topology, vị trí nguồn và thiết bị. Lỗi bảo vệ thực sự dừng đầu ra, không tự khởi động lại. Home phản ánh readback affinity của Windows riêng với trạng thái người xem/đầu nhận vật lý chưa được xác minh; không dùng nhãn An toàn chung.

Sau lượt rà soát v0.10.1, người dùng xác nhận toàn bộ đã kiểm thử thực tế và không còn lỗi. Đây là xác nhận của người dùng; không có thời lượng, build hay checkpoint chi tiết mới để ghi thành measured soak. Nguyên nhân lần mất hình trước đó chưa được xác lập. Không lặp lại TV/timed hardware soak nếu người dùng chưa yêu cầu rõ.

Các thay đổi v0.12.1 được kiểm tra tự động trên tiến trình riêng với hồ sơ và file giả lập, không xác nhận TV vật lý. Bằng chứng QA ở `artifacts/`, không vào payload hay Git. Xem báo cáo release cục bộ để lấy số kiểm tra và hash hiện hành.

## Hoàn tất và quy tắc tiếp tục

Kiểm tra live main/origin/main, working tree, tag và Release. Giữ repository private, Releases cũ và chỉ ZIP hiện tại trong cây dự án. Sau thay đổi sản phẩm, quy trình mặc định là cập nhật phiên bản/docs, build, kiểm thử thích hợp, package, commit/push main, phát hành Release có asset và xác minh hash remote.

Không đưa test case/QA vào specs hay release notes. Không đưa lịch sử, diagnostics, profile/cookie, dữ liệu cá nhân, secrets hay đường dẫn máy vào Git. Dùng `rtk` trước CLI được hỗ trợ qua PowerShell bridge. Không cài driver/runtime/service, xin admin hay thay đổi display/security toàn hệ thống.

Đọc thêm `QUIETSHARE_CONTEXT.md` và `EXTEND_LAPTOP_HANDOFF.md` để giữ các quyết định lịch sử. Protocol phần cứng cũ chỉ là tham khảo.

Bộ chọn file/dự án v0.12.1 dùng layout File Explorer với Desktop, Downloads, Documents, Pictures, This PC, thanh địa chỉ và lịch sử điều hướng trong phiên. Giữ cửa sổ bảo vệ capture và chính sách đường dẫn cục bộ. Người dùng nhắc lại: mặc định kiểm thử, đóng gói, commit/push và phát hành sau mỗi lần chỉnh sửa dự án.

Từ v0.12.1: NAV_SCRIPT (120) là Tệp đang mở, chỉ trở lại content hiện tại, không chọn script hay nạp lại native viewer. HOME_SCRIPT (127) là Soạn lời nhắc chủ động. Script id 0 vẫn giữ trong model để tương thích dữ liệu nhưng không có tab; UI Ctrl+Tab bỏ qua id 0. Home có layout thấp riêng và icon xác nhận capture đúng readback.

Zoom mặc định từ v0.12.1: DefaultZoomPercent=50 dùng chung src/Zoom.h; text/script 12 pt, code 7 pt tương ứng các cỡ chuẩn 24/14. Ctrl+0/Reset về 50; PDF FitPage/FitWidth chủ động vẫn 100% của khung. Giữ canary web ở 1.0 trong probe và áp dụng zoom nội dung sau khi xác minh.


v0.12.2: sửa cặp màn nguồn/đích trùng sau Duplicate → Extend, cập nhật màn trước Verify; không tự khởi động lại. Home Trang chủ, Segoe UI/ClearType và code DirectWrite/LCD, menu nhấp phải Tệp đang mở, ghim xanh lá thống nhất. Kiểm thử tự động không xác nhận hai màn vật lý.
