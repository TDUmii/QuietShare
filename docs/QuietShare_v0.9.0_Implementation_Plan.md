# QuietShare 0.9.0 - Functional specification

## Tiếng Việt

### Phạm vi

Mở rộng trình duyệt tùy chọn thành giao diện nhiều tab và hiển thị phiên bản ứng dụng tại trang Tổng quan. Giữ nguyên trình đọc tài liệu, lời nhắc DPAPI, Explorer, bố cục, chế độ tài nguyên và cơ chế bảo vệ Smart Mirror.

### Mô hình tab web

- Trình duyệt hỗ trợ tối đa 8 tab đồng thời, tách biệt với tối đa 32 tab tài liệu gồm lời nhắc.
- Mỗi tab có trang, lịch sử lùi/tiến và zoom 25-400% riêng trong phiên làm việc. Chuyển tab không tải lại trang đang có hoặc thay đổi trạng thái tab tài liệu.
- Thanh địa chỉ, Lùi, Tiến, Tải lại, Google, ChatGPT và các nút zoom thao tác trên tab web đang chọn. Tiêu đề tab phản ánh trang hiện tại.
- Các tab dùng chung môi trường WebView2 và hồ sơ web cục bộ hiện có, bao gồm trạng thái đăng nhập. Không tạo hồ sơ riêng theo tab.
- Trình duyệt luôn tắt lúc khởi động QuietShare. Không lưu danh sách tab hoặc tự khôi phục URL của phiên trước. Tắt trình duyệt đóng mọi tab và giải phóng controller; hồ sơ web cục bộ vẫn được giữ theo hành vi hiện có.

### Điều khiển và tài nguyên

- Cung cấp nút thêm tab, đóng tab và chọn tab trong thanh tab web. Bố cục giữ các điều khiển có thể dùng được khi thu hẹp cửa sổ và khi thay đổi DPI.
- Tab mới mặc định mở Google. Ctrl+T mở tab, Ctrl+W đóng tab, Ctrl+Tab và Ctrl+Shift+Tab chọn tab tiếp/trước khi ở trang trình duyệt, kể cả khi nội dung web nhận focus. Tại trang nội dung, phím đóng/chuyển tab tiếp tục áp dụng cho tab tài liệu và giữ tab lời nhắc.
- Đóng tab web cuối cùng tắt trình duyệt; không đóng QuietShare hoặc tab tài liệu. Chỉ cho phép một yêu cầu tạo/xác minh tab mới đang chờ tại một thời điểm.
- Khi đã có 8 tab, chặn yêu cầu mở thêm và giữ nguyên các tab đang có. Người dùng cần đóng một tab trước khi mở thêm.
- Chỉ tab đang chọn có thể hiển thị và nhận focus trong vùng trình duyệt. Tab nền và toàn bộ trình duyệt khi ẩn nhận yêu cầu tạm ngưng; tab được chọn nhận yêu cầu tiếp tục hoạt động. WebView2 và nội dung trang quyết định khả năng tạm ngưng thực tế.
- Giữ giới hạn tài nguyên chiếu hiện có; nhiều tab có thể tăng mức dùng RAM theo nội dung web và không có cam kết giảm RAM cố định.

### Bảo vệ capture và điều hướng

- Mỗi tab mới phải xác minh vùng render web bằng nội dung kiểm soát hiển thị và bị loại khỏi WGC trước khi điều hướng đến URL người dùng. Không dùng kết quả xác minh của tab khác thay cho vùng render mới.
- Chuyển giữa các tab đã xác minh không khởi tạo lại kiểm tra vùng web; mỗi tab vẫn chịu cổng bảo vệ chung và phải ẩn khi không được chọn.
- Trong phiên Smart Mirror hợp lệ, quá trình xác minh giữ khung hình công khai gần nhất và chỉ cho phép tiếp tục sau khi kiểm tra thành công và gỡ nội dung kiểm soát. Thất bại, hủy hoặc mất điều kiện bảo vệ vẫn dừng chiếu và khóa nội dung.
- Link yêu cầu cửa sổ mới do người dùng bấm được mở thành tab QuietShare nếu URL hợp lệ và còn chỗ. Không tạo cửa sổ trình duyệt độc lập. Popup tự phát và URL không được phép tiếp tục bị chặn.
- Giữ chính sách vô hiệu hóa tải file lên/xuống, camera, microphone, chia sẻ màn hình, thông báo, DevTools, menu native và các bề mặt không được bảo vệ.
- F8 che toàn bộ vùng web, tiêu đề tab và địa chỉ; không khôi phục nội dung qua callback đang chờ. Thay topology, suspend hoặc lỗi capture tiếp tục đóng/khóa theo cơ chế chung. Duplicate không được mở khóa bởi chức năng tab.

### Phiên bản và tài liệu

- Trang Tổng quan hiển thị **v0.9.0** ở góc trên với độ tương phản phù hợp, dùng cùng nguồn phiên bản với ứng dụng và bộ cài.
- Nhãn phiên bản không nhận focus, không tạo thao tác mới và không che nội dung khi cửa sổ thay đổi kích thước.
- README và tài liệu chức năng có VI/EN, mô tả thao tác tab, giới hạn và phạm vi phím tắt. Đặc tả và ghi chú phát hành chỉ mô tả chức năng, ràng buộc và giới hạn vận hành.
- Nội dung phân phối không bao gồm URL riêng, danh sách tab của người dùng, cookie, hồ sơ trình duyệt hoặc log máy.

## English

### Scope

Extend the optional browser with multiple tabs and identify the application version on Overview. Preserve document readers, DPAPI scripts, Explorer, workspace layout, resource modes and Smart Mirror protection.

### Web tab model

- Support up to 8 simultaneous web tabs, separately from the 32-document-tab limit including Script.
- Each tab retains its page, back/forward history and 25-400% zoom during the session. Switching tabs does not reload an existing page or change document-tab state.
- The address field, Back, Forward, Reload, Google, ChatGPT and zoom controls act on the selected web tab. Tab titles reflect their current pages.
- Tabs share the existing WebView2 environment and local web profile, including login state. Do not create a separate profile for each tab.
- Browsing starts disabled on every QuietShare launch. Do not persist the tab list or automatically restore previous session URLs. Disabling browsing closes every tab and releases its controller while preserving the existing local profile behavior.

### Controls and resource use

- Provide new-tab, close-tab and tab-selection controls in the web tab strip. Keep controls usable at narrow window sizes and across DPI changes.
- New tabs default to Google. Ctrl+T opens a tab, Ctrl+W closes a tab, and Ctrl+Tab/Ctrl+Shift+Tab selects the next/previous tab on the browser page, including when web content has focus. On the content page, close/switch shortcuts continue to control document tabs while preserving Script.
- Closing the final web tab disables browsing without closing QuietShare or document tabs. Allow only one pending tab-creation/verification request at a time.
- Reject requests beyond 8 tabs while preserving existing tabs. A tab must be closed before another can be opened at capacity.
- Only the selected tab can be visible and receive focus inside the browser area. Background tabs and the entire hidden browser request suspension; selected tabs request resumption. WebView2 and page content determine actual suspension support.
- Preserve existing projection resource modes. Multiple tabs can increase memory use according to web content; no fixed RAM reduction is promised.

### Capture protection and navigation

- Every new tab must verify its rendered web area with visible and WGC-excluded control content before navigating to a user URL. Another tab's result cannot substitute for verification of a new rendering surface.
- Switching between verified tabs does not repeat web verification. Each tab remains subject to the common protection gate and must be hidden when it is not selected.
- During a valid Smart Mirror session, verification holds the last public frame and resumes only after success and control-content teardown. Failure, cancellation or invalid protection conditions continue to stop projection and lock content.
- User-initiated new-window links open in QuietShare tabs only when the URL is permitted and capacity is available. Do not create separate browser windows. Continue blocking unsolicited popups and disallowed URLs.
- Preserve restrictions on file uploads/downloads, camera, microphone, screen sharing, notifications, DevTools, native context menus and unprotected surfaces.
- F8 masks all web areas, tab titles and addresses. Pending callbacks cannot restore masked content. Topology changes, suspend and capture failures retain common teardown/locking behavior. Tabs do not override Duplicate restrictions.

### Version identification and documentation

- Display **v0.9.0** in the upper corner of Overview with sufficient contrast, using the same version source as the application and installer.
- The version label is not focusable, introduces no action and does not overlap content when the window resizes.
- Provide VI/EN user and functional documentation covering tab operations, limits and shortcut scope. Specifications and release notes describe functionality, constraints and operating limits.
- Shipping content excludes private URLs, user tab lists, cookies, browser profiles and machine logs.
