# QuietShare 0.4.1

QuietShare là ứng dụng desktop để đọc lời nhắc, xem code, ảnh và PDF trong một cửa sổ riêng khi thuyết trình. App chạy offline trên Windows 10/11 x64, với giao diện tiếng Việt và tiếng Anh.

QuietShare is an offline Windows 10/11 x64 desktop app for private scripts, code, images and PDFs during presentations. It supports Vietnamese and English.

![Tổng quan / Overview](docs/quietshare-home.png)

## Tiếng Việt

### Tính năng

- **Nhiều tab**: mở nhiều file hoặc kéo thả một nhóm file; giữ scroll, zoom và trang PDF theo tab. Lời nhắc có tab riêng, không bị file xem trước ghi đè. Tối đa 32 tab, gồm lời nhắc.
- **Code có màu cú pháp**: số dòng, monospace, giữ thụt dòng, chọn/sao chép, tìm kiếm và xuống dòng tùy chọn. Lexer hỗ trợ Python, C/C++, C#, Java, JavaScript/TypeScript, HTML, CSS, JSON, XML, YAML, Markdown, SQL, Bash/PowerShell, Rust và Go. Code chỉ được xem, không thực thi.
- **Cây dự án**: duyệt thư mục cục bộ khi cần; mặc định bỏ qua thư mục ẩn và `.git`, `node_modules`, `.venv`, `dist`, `build`. Có thể đổi bộ lọc trong Cài đặt.
- **PDF trong app**: xem từng trang, vừa khung/vừa chiều ngang, zoom 25-400%, chọn/sao chép và tìm chữ khi có lớp văn bản. PDF scan vẫn xem được; PDF có mật khẩu chưa được hỗ trợ.
- **Lời nhắc và ảnh**: tự lưu lời nhắc bằng Windows DPAPI, soạn/đọc và cuộn ở ba tốc độ; ảnh có zoom và kéo để di chuyển.
- **Tập trung**: F11 ẩn sidebar và giữ một thanh tab/lệnh mỏng ở trên. Lệnh bổ sung nằm trong menu **⋯**, ngoài vùng nội dung.
- **Smart Mirror**: đưa hình ảnh công khai từ màn hình nguồn lên màn hình đích ở chế độ Extend, sau khi kiểm tra capture thực tế đạt.
- Giao diện tối, ghim trên cùng, che nhanh F8 và VI/EN trong app và bộ cài.
- Thanh bên chỉ hiện icon; giữ chuột trên icon để xem tên. Menu công cụ có nền tối, icon và phím tắt căn phải; mở menu vẫn giữ được kiểm tra WGC.

Văn bản/code hỗ trợ UTF-8 hoặc UTF-16 có BOM, tối đa 5 MiB. App nhận TXT, MD, LOG, C/C++, PY, JS/TS, HTML/CSS, JSON/XML/YAML, SQL, SH/PS1, RS, GO và một số định dạng code khác. File không có phần mở rộng hoặc định dạng chưa nhận diện được xem dưới dạng văn bản monospace đơn sắc nếu encoding hợp lệ và không có dữ liệu nhị phân; tệp thực thi, gói cài đặt và các định dạng nhị phân như BIN/ZIP bị từ chối. Ảnh nhận PNG, JPG/JPEG, BMP và khung đầu của GIF, tối đa 25 MiB/40 megapixel. PDF tối đa 64 MiB. File và thư mục phải nằm trên ổ đĩa cục bộ.

### Cài đặt

1. Cần **Windows 10 phiên bản 2004 (build 19041) trở lên hoặc Windows 11, x64**, với DWM và Windows Graphics Capture hoạt động.
2. Tải [QuietShare 0.4.1](https://github.com/TDUmii/QuietShare/releases/tag/v0.4.1), giải nén toàn bộ gói và chạy `QuietShare-Setup.exe`. Bộ cài dùng tài khoản hiện tại, không cần quyền quản trị.
3. Để dùng portable, chạy `QuietShare.exe` trong thư mục đã giải nén. Giữ `Scintilla.dll`, `Lexilla.dll` và `pdfium.dll` cạnh EXE; không chỉ sao chép riêng EXE.

Đóng QuietShare trước khi nâng cấp bằng bộ cài mới. Nâng cấp và gỡ app giữ lại lời nhắc đã tự lưu. Nút **EN / VI** trong bộ cài đổi ngôn ngữ.

### Sử dụng

1. Đợi kiểm tra capture khi mở app. Nếu nội dung đang khóa, bấm **Kiểm tra WGC** tại **Tổng quan**; nếu cần đổi màn hình nguồn, mở **Trình chiếu** và chọn **Nguồn** trước. App kiểm tra một marker thử, không dùng nội dung cá nhân; chỉ hiện nội dung khi kiểm tra đạt.
2. Chọn **Lời nhắc** để soạn. Ctrl+Enter chuyển soạn/đọc; dùng nút chạy hoặc **⋯ > Chạy / Dừng cuộn**. **⋯ > Đổi tốc độ cuộn** chọn Chậm/Vừa/Nhanh. Chỉ lời nhắc được tự lưu và khôi phục khi mở lại app.
3. Dùng **Mở hoặc thả file**, Ctrl+O hoặc kéo thả nhiều file. Ctrl+Tab / Ctrl+Shift+Tab chuyển tab; dấu **×**, chuột giữa hoặc Ctrl+W đóng tab file. **⋯** có lệnh đóng tab khác/mọi file, luôn giữ lời nhắc. Mở lại file đã thay đổi sẽ có lựa chọn **Tải lại** hoặc **Giữ bản đang xem**.
4. Ctrl+Shift+O hoặc **Mở dự án** chọn thư mục. Mở rộng thư mục rồi nhấp đúp file, hoặc dùng phím mũi tên và Enter. **⋯ > Ẩn / Hiện cây thư mục** đổi giữa cây và nội dung. Cây bỏ qua junction/symlink và giới hạn thư mục lớn; mở thư mục con riêng nếu app yêu cầu.
5. Dùng **− / +**, Ctrl+`−` / Ctrl+`+`, Ctrl rồi lăn chuột hoặc **Cài đặt** để đổi cỡ chữ/zoom. Ctrl+`0` đưa chữ/code về cỡ mặc định, ảnh/PDF về vừa khung. Kéo ảnh hoặc dùng phím mũi tên để di chuyển.
6. Ctrl+F tìm trong lời nhắc, văn bản, code hoặc PDF có lớp chữ; Enter tìm tiếp, Shift+Enter tìm trước, Esc đóng tìm kiếm. Chọn chữ rồi Ctrl+C để sao chép. **⋯ > Bật/tắt xuống dòng** áp dụng cho code.
7. PDF hiển thị một trang tại một thời điểm. **⋯ > Trang PDF trước / sau** hoặc Page Up / Page Down đổi trang; **⋯ > PDF vừa chiều ngang** đổi fit. Lăn chuột hoặc kéo vùng trống để di chuyển; kéo trên chữ để chọn và Ctrl+C để sao chép.
8. F11 bật/tắt tập trung. Lăn chuột trên thanh tab hoặc dùng mũi tên hai đầu để xem tab bị tràn; rê chuột để xem tooltip. **Ghim** giữ cửa sổ trên cùng. Kéo thanh tiêu đề/cạnh để di chuyển/resize. Trong F11, kéo vùng trống thanh tab để di chuyển; menu ⋯ hoặc Alt+F4 đóng app. **EN / VI** trong Cài đặt hoặc **⋯** đổi ngôn ngữ.
9. F8 che mọi nội dung, tên tab và cây thư mục, đồng thời dừng cuộn và Smart Mirror. Bấm F8 để mở lại; nếu app yêu cầu xác minh, bấm **Kiểm tra WGC** trước khi tiếp tục.

Để dùng **Smart Mirror**: chuyển **Win+P > Extend**, đặt toàn bộ QuietShare trên màn hình nguồn, vào **Trình chiếu**, chọn **Nguồn** và **Đích** khác nhau, rồi bấm **Kiểm tra WGC** và **Bắt đầu**. Hai màn hình phải thuộc cùng GPU trong phiên bản này. **Con trỏ** chọn có hiện con trỏ; **Dừng** kết thúc chiếu. Một màn hình vẫn có thể kiểm tra WGC để dùng cửa sổ riêng, nhưng không thể bắt đầu Smart Mirror.

**Duplicate luôn khóa nội dung riêng.** Kiểm tra thất bại, thay đổi màn hình hoặc capture mất an toàn sẽ dừng chiếu và khóa nội dung. “Windows đã nhận” xác nhận trạng thái API; “người xem chưa được xác minh” vẫn cần kiểm tra từ phía người nhận trong đúng Zoom/Meet/Teams/OBS hoặc máy chiếu sẽ dùng. Dùng nội dung mẫu trước buổi chia sẻ. Phần mềm không ngăn máy ảnh bên ngoài chụp màn hình.

![Thanh tập trung / Focus strip](docs/quietshare-reading.png)

## English

### Features

- **Multiple tabs**: open/drop several files and keep scroll, zoom and PDF page per tab. The permanent script remains independent of read-only previews. Up to 32 tabs, including the script.
- **Syntax-colored code**: line numbers, monospace indentation, selection/copy, find and optional wrapping. Lexers cover Python, C/C++, C#, Java, JavaScript/TypeScript, HTML, CSS, JSON, XML, YAML, Markdown, SQL, Bash/PowerShell, Rust and Go. Code is never executed.
- **Project tree**: browse local folders lazily. Hidden folders and `.git`, `node_modules`, `.venv`, `dist`, `build` are ignored by default; change the filter in Settings.
- **In-app PDFs**: page navigation, fit-page/fit-width, 25-400% zoom, selection/copy and find when a text layer exists. Image-only scans remain viewable; password-protected PDFs are unsupported.
- **Scripts and images**: local Windows DPAPI autosave, editing/reading and three auto-scroll speeds; image zoom and dragging to pan.
- **Focus**: F11 hides the sidebar while retaining one slim top tab/command strip. Extra actions remain accessible through **⋯**, outside the content area.
- **Smart Mirror**: project a public source display onto a separate destination in Extend after the actual capture preflight passes.
- Dark interface, always-on-top pinning, instant F8 hide and VI/EN app/installer interfaces.
- Icon-only navigation with delayed name tooltips. The dark tools menu includes icons and aligned shortcuts; opening it preserves WGC verification.

Text/code support UTF-8 or BOM-marked UTF-16, up to 5 MiB. Supported extensions include TXT, MD, LOG, C/C++, PY, JS/TS, HTML/CSS, JSON/XML/YAML, SQL, SH/PS1, RS, GO and selected other source formats. Extensionless or unrecognized files fall back to monochrome monospace text when their encoding is valid and they contain no binary controls; executables, installer packages and binary formats such as BIN/ZIP are rejected. Images support PNG, JPG/JPEG, BMP and the first GIF frame, up to 25 MiB/40 megapixels. PDFs are limited to 64 MiB. Files/folders must be on local drives.

### Install

1. Use **Windows 10 version 2004 (build 19041) or later, or Windows 11, x64**, with working DWM and Windows Graphics Capture.
2. Download [QuietShare 0.4.1](https://github.com/TDUmii/QuietShare/releases/tag/v0.4.1), extract the complete package and run `QuietShare-Setup.exe`. It installs for the current account without administrator rights.
3. For portable use, run `QuietShare.exe` in the extracted folder. Keep `Scintilla.dll`, `Lexilla.dll` and `pdfium.dll` beside it; copying only the EXE is insufficient.

Close QuietShare before upgrading with a new installer. Upgrading/uninstalling preserve the autosaved script. Use the installer's **EN / VI** button to change its language.

### Use

1. Wait for the capture check at launch. If content is locked, press **Verify WGC privacy** on **Overview**; to change the source display, open **Presentation** and select **Source** first. The app checks a synthetic marker without your personal content and reveals content only after success.
2. Select **Script** to prepare your notes. Ctrl+Enter switches editing/reading; use the play action or **⋯ > Start / stop scrolling**. **⋯ > Change scroll speed** cycles Slow/Medium/Fast. Only the script is autosaved/restored on launch.
3. Use **Open or drop a file**, Ctrl+O or multi-file drag/drop. Ctrl+Tab / Ctrl+Shift+Tab switches tabs; **×**, middle-click or Ctrl+W closes a file tab. **⋯** can close other/all file tabs while retaining the script. Reopening a changed file offers **Reload** or **Keep current preview**.
4. Ctrl+Shift+O or **Open project** selects a folder. Expand folders, then double-click a file or navigate with arrows and Enter. **⋯ > Hide / show project tree** switches between tree and content. Junctions/symlinks are skipped and large folders are bounded; open a subfolder separately if requested.
5. Use **− / +**, Ctrl+`−` / Ctrl+`+`, Ctrl plus the wheel, or **Settings** to adjust text size/zoom. Ctrl+`0` restores default script/code size and fits images/PDFs. Drag images or use arrows to pan.
6. Ctrl+F opens find for scripts, text, code and PDFs with a text layer. Enter finds next, Shift+Enter previous, and Esc closes find. Select text and Ctrl+C to copy. **⋯ > Toggle line wrap** applies to code.
7. The PDF reader displays one page at a time. **⋯ > Previous / Next PDF page** or Page Up / Page Down changes pages; **⋯ > Fit PDF width** changes fit. Scroll or drag a blank area to pan; drag over text to select it, then Ctrl+C to copy.
8. F11 enters/exits focus. Scroll over the tab strip or use its end arrows for overflowing tabs; hover for tooltips. **Pin** keeps the window on top. Drag its title bar/edges to move/resize. In F11, drag blank tab-strip space to move; use ⋯ or Alt+F4 to close the app. Change language through **EN / VI** in Settings or **⋯**.
9. F8 hides every viewer, tab name and project tree, and stops auto-scroll and Smart Mirror. Press F8 to reveal; run **Verify WGC privacy** first if verification is requested.

For **Smart Mirror**, choose **Win+P > Extend**, place the entire QuietShare window on the source display, open **Presentation**, choose different **Source** and **Destination** displays, then press **Verify WGC privacy** and **Start**. Both displays must use the same GPU in this version. **Pointer** controls cursor visibility; **Stop** ends projection. A single display can run the WGC preflight for private-window use but cannot start Smart Mirror.

**Duplicate always locks private content.** Failed checks, changed displays or unsafe capture stop projection and lock the readers. “Windows accepted” confirms the API state; “Viewer unverified” still requires a receiver-side check in the actual Zoom/Meet/Teams/OBS or projector setup. Use sample content before sharing. Software cannot prevent an external camera from filming the screen.
