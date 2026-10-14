# QuietShare 0.6.0

## Tiếng Việt

### Phạm vi

Hoàn thiện không gian làm việc theo hướng VS Code: Explorer ổn định, bố cục tùy chỉnh, trình duyệt nhận cả địa chỉ và tìm kiếm, góc bo nhẹ và thao tác mượt. Giữ nguyên trình đọc file, lời nhắc, Smart Mirror và quy tắc bảo vệ nội dung.

### Yêu cầu

- Mở một thư mục rồi chọn file không làm cây biến mất. Các nhánh mở và lựa chọn được giữ khi đổi tab, chuyển giữa lời nhắc và file, hoặc resize cửa sổ.
- Explorer và trình đọc nằm cạnh nhau cả trong cửa sổ hẹp. Người dùng kéo đường phân cách để chỉnh độ rộng; giới hạn kích thước để cả hai vùng vẫn sử dụng được.
- Ctrl+B, nút **×** trên tiêu đề Explorer và menu **Bố cục** điều khiển ẩn/hiện. Hiện lại giữ nguyên cây, không phải chọn folder lần nữa.
- Nút **Bố cục** ở góc trên bên phải bật/tắt độc lập thanh icon, Explorer, tab, công cụ và thanh trạng thái. Trong chế độ thường, nút này vẫn hiện khi các thanh khác đã ẩn; có lệnh khôi phục mặc định.
- Chỉ lưu lựa chọn bố cục và độ rộng trên máy. Không tự lưu đường dẫn dự án hoặc file để mở lại.
- F11 chuyển sang chế độ tập trung, ẩn tab và các thanh phụ, giữ các nút cần thiết. Thoát F11 phục hồi thanh tiêu đề cùng các lựa chọn bố cục. Kéo vùng trống ở trên để di chuyển cửa sổ.
- Ô trình duyệt nhận HTTP/HTTPS, tên miền, localhost có cổng và từ khóa Google, kể cả tiếng Việt. Có nút Google và ChatGPT; Google là trang mặc định khi bật trình duyệt.
- Trình duyệt vẫn mặc định tắt ở mỗi lần mở app. Chỉ tải engine khi người dùng bật, đóng engine khi tắt và yêu cầu tạm ngưng khi chuyển sang đọc file.
- Dùng hệ icon và màu tối hiện có, bo góc nhẹ và spacing nhất quán. Tránh repaint lặp khi con trỏ vẫn ở cùng tab hoặc khi splitter chưa đổi vị trí.
- App và hướng dẫn có tiếng Việt/tiếng Anh. Cài đặt, portable và nâng cấp tiếp tục hoạt động trên Windows 10/11 x64 được hỗ trợ.

### Nghiệm thu

1. Mở folder có nhiều cấp, mở file bằng chuột và Enter, đổi tab/lời nhắc rồi resize. Cây, lựa chọn và nhánh mở không tự ẩn hoặc bị dựng lại.
2. Kéo splitter ở cửa sổ rộng và hẹp; Explorer và viewer không chồng nhau. Thu hẹp rồi mở rộng giữ độ rộng người dùng đã chọn khi còn đủ chỗ.
3. Ẩn/hiện từng thanh, kiểm tra khôi phục mặc định, mở lại app và F11. Nội dung vẫn sử dụng được; thoát F11 hiện lại nút Bố cục để chỉnh các thanh.
4. Kiểm tra địa chỉ HTTP/HTTPS, tên miền, localhost có cổng, từ khóa có khoảng trắng và tiếng Việt; các từ khóa được mã hóa đúng khi tìm Google. Scheme nguy hiểm không được thực thi.
5. Kiểm tra giao diện VI/EN, cửa sổ nhỏ, DPI khác nhau và trạng thái hover/focus. Không có nút chồng nhau, chữ bị cắt bất thường hoặc khoảng trống dành cho thanh đã ẩn.
6. Build sản phẩm, chạy kiểm thử native và kiểm thử cài/nâng cấp/gỡ trong vùng dữ liệu thử riêng. Gói phát hành chứa đúng binary, thư viện và tài liệu liên quan.

### Bảo vệ nội dung và phát hành

Duplicate luôn khóa nội dung riêng. Explorer, tên tab, địa chỉ và web đi qua cùng kiểm tra capture; thay đổi bố cục không được bỏ qua gate hoặc làm mất xác minh. F8 và Ctrl+Alt+Q giữ hành vi hiện tại. Web chỉ mở trang sau khi kiểm tra vùng trình duyệt thực tế đạt; các hạn chế upload/download, quyền thiết bị, popup và giao diện native vẫn áp dụng.

Chỉ đưa mã nguồn, tài liệu và sản phẩm dự án lên repository private. Không đưa lời nhắc cá nhân, profile trình duyệt, cookie, tài khoản, đường dẫn máy hoặc log riêng vào Git hay ZIP. Bản phát hành trước tiếp tục được giữ trong Releases; thay thế ZIP và bằng chứng kiểm thử cũ trong cây dự án hiện tại bằng bản 0.6.0.

## English

### Scope

Refine the VS Code-style workspace with stable Explorer navigation, configurable bars, URL/search entry, modest rounding and smooth interactions. Preserve existing file readers, scripts, Smart Mirror and content protection rules.

### Requirements

- Selecting a file after opening a folder keeps Explorer visible. Preserve expanded branches and selection when switching tabs, returning to Script or resizing.
- Explorer and the reader remain side by side in narrow windows. A draggable divider adjusts width while maintaining usable minimum widths for both panes.
- Ctrl+B, the Explorer header's **×** and **Layout** control visibility. Showing Explorer restores the same tree without another folder selection.
- A top-right **Layout** button independently toggles the activity bar, Explorer, tabs, toolbar and status bar. Outside focus mode, keep this entry point visible when other bars are hidden; provide a reset command.
- Save layout visibility and width locally. Do not automatically save project or file paths for reopening.
- F11 hides tabs and supporting bars while keeping essential actions. Exiting restores the title bar and chosen layout. Blank top space allows window dragging.
- Browser input accepts HTTP/HTTPS URLs, domains, localhost with ports and Google queries including Vietnamese. Provide Google and ChatGPT shortcuts; use Google as the default destination when enabling browsing.
- Browsing remains disabled at every app startup. Load the engine on demand, close it on disable, and request suspension when returning to file reading.
- Retain existing icons and dark colors, with subtle rounding and consistent spacing. Avoid repeated repaints while hovering the same tab or moving a divider to the same position.
- Keep the app and guidance bilingual. Installation, portable use and upgrades continue to support the documented Windows 10/11 x64 versions.

### Acceptance

1. Open a nested folder, open files with mouse and Enter, switch tabs/Script, and resize. Preserve the visible tree, selection and expanded branches.
2. Drag the divider in wide and narrow windows without overlapping the reader. Narrowing and widening restore the user's preferred width when space permits.
3. Toggle every bar, reset the layout, restart the app and enter/exit F11. Content remains usable; exiting F11 restores Layout for adjusting the bars.
4. Check HTTP/HTTPS addresses, domains, localhost ports, multi-word searches and Vietnamese. Encode Google queries correctly; never execute unsafe schemes.
5. Check VI/EN, small windows, different DPI settings and hover/focus states. No overlapping controls, unintended clipping or space reserved for hidden bars.
6. Build the shipping product, run native and isolated install/upgrade/uninstall checks, and package only the relevant binaries, libraries and documentation.

### Privacy and release

Duplicate always locks private content. Explorer, tab names, addresses and web pages share the existing capture gate; layout changes cannot bypass it or invalidate verification. Preserve F8 masking and Ctrl+Alt+Q recall behavior. User pages open only after the browser-rendered capture check succeeds. Existing restrictions on uploads/downloads, device permissions, popups and native surfaces remain active.

Publish only project source, documentation and shipping files to the private repository. Exclude personal scripts, browser profiles, cookies, accounts, machine paths and private logs from Git and ZIP archives. Keep older GitHub Releases; replace superseded ZIPs and test evidence in the current project tree with version 0.6.0.
