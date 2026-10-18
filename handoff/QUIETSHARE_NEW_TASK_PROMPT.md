# Prompt tiếp tục QuietShare

Tiếp tục private repository `TDUmii/QuietShare` từ bản `v0.12.2`. Đọc `handoff/QUIETSHARE_NEXT_CHAT_HANDOFF.md`, `handoff/QUIETSHARE_CONTEXT.md` và `handoff/EXTEND_LAPTOP_HANDOFF.md`; xác minh live main/origin/main, working tree, tag/Release trước khi thay đổi.

Home dùng lịch sử file/dự án thực, mã hóa DPAPI cục bộ, có mở lại/ghim/xóa. Nhớ kích thước cửa sổ và cho phép sửa văn bản/code mặc định bật; file dùng lưu chủ động và kiểm tra xung đột bên ngoài. Giữ nguyên toàn bộ capture/privacy/topology/source/device guards. Lỗi bảo vệ dừng đầu ra và không tự khởi động lại.

Người dùng xác nhận kiểm thử thực tế hiện không còn lỗi; không suy ra measured soak hay root cause cũ. Không lặp TV/hardware soak nếu chưa được yêu cầu. Không cài driver/runtime/service, xin admin hoặc đổi hệ thống display/security. Bảo toàn repository private và Releases lịch sử.

Sau thay đổi sản phẩm: cập nhật phiên bản/docs, kiểm thử phù hợp, build/package Windows, commit/push main, tạo versioned GitHub Release và đối chiếu remote branch/tag/asset hashes. Không đưa QA/test case vào specs/release notes; không commit profile, cookies, logs, secrets, đường dẫn máy hay dữ liệu cá nhân. Dùng RTK cho CLI được hỗ trợ và báo cáo bằng tiếng Việt.

Bộ chọn file/dự án v0.12.1 dùng layout File Explorer với Desktop, Downloads, Documents, Pictures, This PC, thanh địa chỉ và lịch sử điều hướng trong phiên. Giữ cửa sổ bảo vệ capture và chính sách đường dẫn cục bộ. Người dùng nhắc lại: mặc định kiểm thử, đóng gói, commit/push và phát hành sau mỗi lần chỉnh sửa dự án.

Từ v0.12.1: NAV_SCRIPT (120) là Tệp đang mở, chỉ trở lại content hiện tại, không chọn script hay nạp lại native viewer. HOME_SCRIPT (127) là Soạn lời nhắc chủ động. Script id 0 vẫn giữ trong model để tương thích dữ liệu nhưng không có tab; UI Ctrl+Tab bỏ qua id 0. Home có layout thấp riêng và icon xác nhận capture đúng readback.

Zoom mặc định từ v0.12.1: DefaultZoomPercent=50 dùng chung src/Zoom.h; text/script 12 pt, code 7 pt tương ứng các cỡ chuẩn 24/14. Ctrl+0/Reset về 50; PDF FitPage/FitWidth chủ động vẫn 100% của khung. Giữ canary web ở 1.0 trong probe và áp dụng zoom nội dung sau khi xác minh.


v0.12.2: sửa cặp màn nguồn/đích trùng sau Duplicate → Extend, cập nhật màn trước Verify; không tự khởi động lại. Home Trang chủ, Segoe UI/ClearType và code DirectWrite/LCD, menu nhấp phải Tệp đang mở, ghim xanh lá thống nhất. Kiểm thử tự động không xác nhận hai màn vật lý.
