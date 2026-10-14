# QuietShare 0.5.0

QuietShare là ứng dụng desktop để đọc lời nhắc, xem code, ảnh và PDF trong một cửa sổ riêng khi thuyết trình. App chạy offline trên Windows 10/11 x64, với giao diện tiếng Việt và tiếng Anh.

QuietShare is an offline Windows 10/11 x64 desktop app for private scripts, code, images and PDFs during presentations. It supports Vietnamese and English.

![Tổng quan / Overview](docs/quietshare-home.png)

## Tiếng Việt

### Tính năng

- **Nhiều tab**: mở nhiều file hoặc kéo thả một nhóm file; giữ scroll, zoom và trang PDF theo tab. Lời nhắc có tab riêng, không bị file xem trước ghi đè. Tối đa 32 tab, gồm lời nhắc.
- **Code có màu cú pháp**: số dòng, monospace, giữ thụt dòng, chọn/sao chép, tìm kiếm và xuống dòng tùy chọn. Lexer hỗ trợ Python, C/C++, C#, Java, JavaScript/TypeScript, HTML, CSS, JSON, XML, YAML, Markdown, SQL, Bash/PowerShell, Rust và Go. Code chỉ được xem, không thực thi.
- **Cây dự án**: duyệt thư mục cục bộ khi cần; mặc định bỏ qua thư mục ẩn và `.git`, `node_modules`, `.venv`, `dist`, `build`. Có thể đổi bộ lọc trong Cài đặt.
- **Trình duyệt tùy chọn**: bật khi cần mở ChatGPT hoặc web HTTP/HTTPS. Có ô địa chỉ, quay lại, tiến tới và tải lại; mặc định tắt khi khởi động. Hồ sơ đăng nhập riêng của QuietShare được giữ trên máy.
- **PDF trong app**: xem từng trang, vừa khung/vừa chiều ngang, zoom 25-400%, chọn/sao chép và tìm chữ khi có lớp văn bản. PDF scan vẫn xem được; PDF có mật khẩu chưa được hỗ trợ.
- **Lời nhắc và ảnh**: tự lưu lời nhắc bằng Windows DPAPI, soạn/đọc và cuộn ở ba tốc độ; ảnh có zoom và kéo để di chuyển.
- **Tập trung**: F11 ẩn sidebar và giữ một thanh tab/lệnh mỏng ở trên. Lệnh bổ sung nằm trong menu **⋯**, ngoài vùng nội dung.
- **Smart Mirror**: đưa hình ảnh công khai từ màn hình nguồn lên màn hình đích ở chế độ Extend, sau khi kiểm tra capture thực tế đạt.
- Giao diện tối, ghim trên cùng, che nhanh F8 và VI/EN trong app và bộ cài.
- Thanh bên chỉ hiện icon; giữ chuột trên icon để xem tên. Menu công cụ có nền tối, icon và phím tắt căn phải; mở menu vẫn giữ được kiểm tra WGC.

Văn bản/code hỗ trợ UTF-8 hoặc UTF-16 có BOM, tối đa 5 MiB. App nhận TXT, MD, LOG, C/C++, PY, JS/TS, HTML/CSS, JSON/XML/YAML, SQL, SH/PS1, RS, GO và một số định dạng code khác. File không có phần mở rộng hoặc định dạng chưa nhận diện được xem dưới dạng văn bản monospace đơn sắc nếu encoding hợp lệ và không có dữ liệu nhị phân; tệp thực thi, gói cài đặt và các định dạng nhị phân như BIN/ZIP bị từ chối. Ảnh nhận PNG, JPG/JPEG, BMP và khung đầu của GIF, tối đa 25 MiB/40 megapixel. PDF tối đa 64 MiB. File và thư mục phải nằm trên ổ đĩa cục bộ.

### Cài đặt

1. Cần **Windows 10 phiên bản 2004 (build 19041) trở lên hoặc Windows 11, x64**, với DWM và Windows Graphics Capture hoạt động.
2. Tải [QuietShare 0.5.0](https://github.com/TDUmii/QuietShare/releases/tag/v0.5.0), giải nén toàn bộ gói và chạy `QuietShare-Setup.exe`. Bộ cài dùng tài khoản hiện tại, không cần quyền quản trị.
3. Để dùng portable, chạy `QuietShare.exe` trong thư mục đã giải nén. Giữ `Scintilla.dll`, `Lexilla.dll`, `pdfium.dll` và `WebView2Loader.dll` cạnh EXE; không chỉ sao chép riêng EXE.

Để dùng web, máy cần **Microsoft Edge WebView2 Evergreen Runtime** hiện hành. Nếu trình duyệt không mở được, cài/cập nhật từ [Microsoft WebView2](https://developer.microsoft.com/en-us/microsoft-edge/webview2/), rồi tắt/bật trình duyệt lại. Các chức năng đọc file vẫn dùng được khi không có runtime.

Đóng QuietShare trước khi nâng cấp bằng bộ cài mới. Nâng cấp và gỡ app giữ lại lời nhắc đã tự lưu. Nút **EN / VI** trong bộ cài đổi ngôn ngữ.

### Sử dụng

1. Đợi kiểm tra capture khi mở app. Nếu nội dung đang khóa, bấm **Kiểm tra WGC** tại **Tổng quan**; nếu cần đổi màn hình nguồn, mở **Trình chiếu** và chọn **Nguồn** trước. App kiểm tra một marker thử, không dùng nội dung cá nhân; chỉ hiện nội dung khi kiểm tra đạt.
2. Chọn **Lời nhắc** để soạn. Ctrl+Enter chuyển soạn/đọc; dùng nút chạy hoặc **⋯ > Chạy / Dừng cuộn**. **⋯ > Đổi tốc độ cuộn** chọn Chậm/Vừa/Nhanh. Chỉ lời nhắc được tự lưu và khôi phục khi mở lại app.
3. Dùng **Mở hoặc thả file**, Ctrl+O hoặc kéo thả nhiều file. Ctrl+Tab / Ctrl+Shift+Tab chuyển tab; dấu **×**, chuột giữa hoặc Ctrl+W đóng tab file. **⋯** có lệnh đóng tab khác/mọi file, luôn giữ lời nhắc. Mở lại file đã thay đổi sẽ có lựa chọn **Tải lại** hoặc **Giữ bản đang xem**.
4. Ctrl+Shift+O hoặc **Mở dự án** chọn thư mục. Mở rộng thư mục rồi nhấp đúp file, hoặc dùng phím mũi tên và Enter. Cây dự án và nội dung nằm cạnh nhau khi cửa sổ đủ rộng; mở file hoặc chuyển tab vẫn giữ cây. **Ctrl+B** hoặc **⋯ > Ẩn / Hiện cây thư mục** bật/tắt cây. Ở cửa sổ rất hẹp, cây chiếm vùng nội dung; mở file để đọc, Ctrl+B để trở lại cây. Cây bỏ qua junction/symlink và giới hạn thư mục lớn; mở thư mục con riêng nếu app yêu cầu.
5. Dùng **− / +**, Ctrl+`−` / Ctrl+`+`, Ctrl rồi lăn chuột hoặc **Cài đặt** để đổi cỡ chữ/zoom. Ctrl+`0` đưa chữ/code về cỡ mặc định, ảnh/PDF về vừa khung. Kéo ảnh hoặc dùng phím mũi tên để di chuyển.

Trong **Cài đặt**, nhập trực tiếp cỡ chữ rồi nhấn Enter: lời nhắc/văn bản 8-200 pt; code/ảnh/PDF 25-400% zoom. Thanh trượt hiển thị giá trị và phần màu ngay khi kéo. PDF áp dụng mức zoom khi thả chuột để tránh dựng lại trang nặng ở từng bước.

Hộp chọn file/dự án là cửa sổ native được bảo vệ của QuietShare. Dùng **Ổ đĩa**, **Lên**, nhấp đúp để vào thư mục; Ctrl/Shift chọn nhiều file rồi bấm **Mở**, hoặc bấm **Chọn thư mục** cho dự án. Hộp chọn chỉ duyệt ổ cục bộ, bỏ qua reparse/junction và hiện tối đa 1.000 mục mỗi thư mục.
6. Ctrl+F tìm trong lời nhắc, văn bản, code hoặc PDF có lớp chữ; Enter tìm tiếp, Shift+Enter tìm trước, Esc đóng tìm kiếm. Chọn chữ rồi Ctrl+C để sao chép. **⋯ > Bật/tắt xuống dòng** áp dụng cho code.
7. PDF hiển thị một trang tại một thời điểm. **⋯ > Trang PDF trước / sau** hoặc Page Up / Page Down đổi trang; **⋯ > PDF vừa chiều ngang** đổi fit. Lăn chuột hoặc kéo vùng trống để di chuyển; kéo trên chữ để chọn và Ctrl+C để sao chép.
8. F11 bật/tắt tập trung. Lăn chuột trên thanh tab hoặc dùng mũi tên hai đầu để xem tab bị tràn; rê chuột để xem tooltip. **Ghim** giữ cửa sổ trên cùng. Kéo thanh tiêu đề/cạnh để di chuyển/resize. Trong F11, kéo vùng trống thanh tab để di chuyển; menu ⋯ hoặc Alt+F4 đóng app. **EN / VI** trong Cài đặt hoặc **⋯** đổi ngôn ngữ.
9. F8 che mọi nội dung, tên tab và cây thư mục, đồng thời dừng cuộn và Smart Mirror. Bấm F8 để mở lại; nếu app yêu cầu xác minh, bấm **Kiểm tra WGC** trước khi tiếp tục.

Ctrl+Alt+Q gọi QuietShare lên khi bị che hoặc thu nhỏ, kể cả lúc không ghim. Phím tắt không mở Alt+Tab và giữ focus ở ứng dụng đang dùng. Nếu đang che bằng F8, nội dung vẫn được che. Rê chuột vào nút Ghim để xem trạng thái phím tắt; app khác dùng cùng tổ hợp có thể khiến đăng ký không thành công.

Để dùng web: chọn icon quả địa cầu hoặc vào **Cài đặt > Bật trình duyệt**, bấm **Bật trình duyệt**, chờ kiểm tra vùng WebView2 rồi nhập URL và nhấn Enter hoặc **Mở**. Nút **ChatGPT** mở `https://chatgpt.com`; đăng nhập trong trang web khi cần. **Tắt trình duyệt** đóng engine web, giữ cookie của hồ sơ riêng để lần sau đăng nhập thuận tiện. Chuyển sang đọc file sẽ ẩn và yêu cầu tạm ngưng web; mở lại để tiếp tục. F8 che cả web và ô địa chỉ. Thay đổi màn hình tắt web để yêu cầu kiểm tra mới.

Web trong bản này dành cho đọc và chat bằng chữ. Tải lên/tải xuống file, camera, microphone, chia sẻ màn hình, thông báo, DevTools và menu native bị tắt. Dropdown HTML native có thể chọn bằng phím mũi tên; dropdown do website vẽ vẫn dùng bình thường. Popup tự phát bị chặn; link mở cửa sổ mới do người dùng bấm chuyển vào cùng vùng web. Luồng đăng nhập OAuth cần popup riêng có thể không tương thích.

Để dùng **Smart Mirror**: chuyển **Win+P > Extend**, đặt toàn bộ QuietShare trên màn hình nguồn, vào **Trình chiếu**, chọn **Nguồn** và **Đích** khác nhau, rồi bấm **Kiểm tra WGC** và **Bắt đầu**. Hai màn hình phải thuộc cùng GPU trong phiên bản này. **Con trỏ** chọn có hiện con trỏ; **Dừng** kết thúc chiếu. Một màn hình vẫn có thể kiểm tra WGC để dùng cửa sổ riêng, nhưng không thể bắt đầu Smart Mirror.

**Duplicate luôn khóa nội dung riêng.** Kiểm tra thất bại, thay đổi màn hình hoặc capture mất an toàn sẽ dừng chiếu và khóa nội dung. “Windows đã nhận” xác nhận trạng thái API; “người xem chưa được xác minh” vẫn cần kiểm tra từ phía người nhận trong đúng Zoom/Meet/Teams/OBS hoặc máy chiếu sẽ dùng. Dùng nội dung mẫu trước buổi chia sẻ. Phần mềm không ngăn máy ảnh bên ngoài chụp màn hình.

![Thanh tập trung / Focus strip](docs/quietshare-reading.png)

![Cỡ chữ và thanh trượt / Text size and slider](docs/quietshare-settings.png)

![Cây dự án và code / Project Explorer and code](docs/quietshare-project.png)

![Trình duyệt tùy chọn / Optional browser](docs/quietshare-browser.png)

## English

### Features

- **Multiple tabs**: open/drop several files and keep scroll, zoom and PDF page per tab. The permanent script remains independent of read-only previews. Up to 32 tabs, including the script.
- **Syntax-colored code**: line numbers, monospace indentation, selection/copy, find and optional wrapping. Lexers cover Python, C/C++, C#, Java, JavaScript/TypeScript, HTML, CSS, JSON, XML, YAML, Markdown, SQL, Bash/PowerShell, Rust and Go. Code is never executed.
- **Project tree**: browse local folders lazily. Hidden folders and `.git`, `node_modules`, `.venv`, `dist`, `build` are ignored by default; change the filter in Settings.
- **Optional browser**: enable only when needed for ChatGPT or HTTP/HTTPS pages. Includes address entry, Back, Forward and Reload; starts disabled. QuietShare keeps its own login profile locally.
- **In-app PDFs**: page navigation, fit-page/fit-width, 25-400% zoom, selection/copy and find when a text layer exists. Image-only scans remain viewable; password-protected PDFs are unsupported.
- **Scripts and images**: local Windows DPAPI autosave, editing/reading and three auto-scroll speeds; image zoom and dragging to pan.
- **Focus**: F11 hides the sidebar while retaining one slim top tab/command strip. Extra actions remain accessible through **⋯**, outside the content area.
- **Smart Mirror**: project a public source display onto a separate destination in Extend after the actual capture preflight passes.
- Dark interface, always-on-top pinning, instant F8 hide and VI/EN app/installer interfaces.
- Icon-only navigation with delayed name tooltips. The dark tools menu includes icons and aligned shortcuts; opening it preserves WGC verification.

Text/code support UTF-8 or BOM-marked UTF-16, up to 5 MiB. Supported extensions include TXT, MD, LOG, C/C++, PY, JS/TS, HTML/CSS, JSON/XML/YAML, SQL, SH/PS1, RS, GO and selected other source formats. Extensionless or unrecognized files fall back to monochrome monospace text when their encoding is valid and they contain no binary controls; executables, installer packages and binary formats such as BIN/ZIP are rejected. Images support PNG, JPG/JPEG, BMP and the first GIF frame, up to 25 MiB/40 megapixels. PDFs are limited to 64 MiB. Files/folders must be on local drives.

### Install

1. Use **Windows 10 version 2004 (build 19041) or later, or Windows 11, x64**, with working DWM and Windows Graphics Capture.
2. Download [QuietShare 0.5.0](https://github.com/TDUmii/QuietShare/releases/tag/v0.5.0), extract the complete package and run `QuietShare-Setup.exe`. It installs for the current account without administrator rights.
3. For portable use, run `QuietShare.exe` in the extracted folder. Keep `Scintilla.dll`, `Lexilla.dll`, `pdfium.dll` and `WebView2Loader.dll` beside it; copying only the EXE is insufficient.

Web browsing requires a current **Microsoft Edge WebView2 Evergreen Runtime**. If it cannot start, install/update the runtime from [Microsoft WebView2](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) and toggle the browser off/on. Offline file readers remain available without it.

Close QuietShare before upgrading with a new installer. Upgrading/uninstalling preserve the autosaved script. Use the installer's **EN / VI** button to change its language.

### Use

1. Wait for the capture check at launch. If content is locked, press **Verify WGC privacy** on **Overview**; to change the source display, open **Presentation** and select **Source** first. The app checks a synthetic marker without your personal content and reveals content only after success.
2. Select **Script** to prepare your notes. Ctrl+Enter switches editing/reading; use the play action or **⋯ > Start / stop scrolling**. **⋯ > Change scroll speed** cycles Slow/Medium/Fast. Only the script is autosaved/restored on launch.
3. Use **Open or drop a file**, Ctrl+O or multi-file drag/drop. Ctrl+Tab / Ctrl+Shift+Tab switches tabs; **×**, middle-click or Ctrl+W closes a file tab. **⋯** can close other/all file tabs while retaining the script. Reopening a changed file offers **Reload** or **Keep current preview**.
4. Ctrl+Shift+O or **Open project** selects a folder. Expand folders, then double-click a file or navigate with arrows and Enter. Project Explorer and content sit side by side when the window is wide enough; opening files or switching tabs keeps the tree visible. **Ctrl+B** or **⋯ > Hide / show project tree** toggles Explorer. In very narrow windows, the tree fills the content area; open a file to read it and press Ctrl+B to return to the tree. Junctions/symlinks are skipped and large folders are bounded; open a subfolder separately if requested.
5. Use **− / +**, Ctrl+`−` / Ctrl+`+`, Ctrl plus the wheel, or **Settings** to adjust text size/zoom. Ctrl+`0` restores default script/code size and fits images/PDFs. Drag images or use arrows to pan.

In **Settings**, type a size and press Enter: scripts/text support 8-200 pt; code/images/PDF use 25-400% zoom. The slider value and fill update while dragging. PDFs apply the zoom on release to avoid rendering expensive pages at every step.

The file/project picker is a capture-protected QuietShare native window. Use **Drives**, **Up**, and double-click to enter a folder; Ctrl/Shift selects multiple files, then **Open**, or use **Select folder** for a project. It browses local drives, skips reparse/junction entries and displays up to 1,000 entries per folder.
6. Ctrl+F opens find for scripts, text, code and PDFs with a text layer. Enter finds next, Shift+Enter previous, and Esc closes find. Select text and Ctrl+C to copy. **⋯ > Toggle line wrap** applies to code.
7. The PDF reader displays one page at a time. **⋯ > Previous / Next PDF page** or Page Up / Page Down changes pages; **⋯ > Fit PDF width** changes fit. Scroll or drag a blank area to pan; drag over text to select it, then Ctrl+C to copy.
8. F11 enters/exits focus. Scroll over the tab strip or use its end arrows for overflowing tabs; hover for tooltips. **Pin** keeps the window on top. Drag its title bar/edges to move/resize. In F11, drag blank tab-strip space to move; use ⋯ or Alt+F4 to close the app. Change language through **EN / VI** in Settings or **⋯**.
9. F8 hides every viewer, tab name and project tree, and stops auto-scroll and Smart Mirror. Press F8 to reveal; run **Verify WGC privacy** first if verification is requested.

Ctrl+Alt+Q recalls a covered or minimized QuietShare window, including when unpinned. It preserves the current application's focus and does not open Alt+Tab. F8-masked content remains masked. Hover the Pin button to see shortcut availability; another application's registration can occupy the same combination.

For web: select the globe icon or **Settings > Enable browser**, press **Enable browser**, wait for the rendered WebView2 capture check, then enter an address and press Enter or **Go**. **ChatGPT** opens `https://chatgpt.com`; sign in on the page when needed. **Disable browser** closes the web engine while retaining its separate profile cookies. Switching to file reading hides the browser and requests suspension; return to resume. F8 hides web content and its address. A display change closes web browsing until a fresh check.

This browser is for reading and text chat. File uploads/downloads, camera, microphone, screen sharing, notifications, DevTools and native context menus are disabled. Native HTML selects support arrow-key selection; website-rendered dropdowns remain usable. Unsolicited popups are blocked; user-initiated new-window links navigate in the same protected view. OAuth flows requiring a separate popup may be incompatible.

For **Smart Mirror**, choose **Win+P > Extend**, place the entire QuietShare window on the source display, open **Presentation**, choose different **Source** and **Destination** displays, then press **Verify WGC privacy** and **Start**. Both displays must use the same GPU in this version. **Pointer** controls cursor visibility; **Stop** ends projection. A single display can run the WGC preflight for private-window use but cannot start Smart Mirror.

**Duplicate always locks private content.** Failed checks, changed displays or unsafe capture stop projection and lock the readers. “Windows accepted” confirms the API state; “Viewer unverified” still requires a receiver-side check in the actual Zoom/Meet/Teams/OBS or projector setup. Use sample content before sharing. Software cannot prevent an external camera from filming the screen.
