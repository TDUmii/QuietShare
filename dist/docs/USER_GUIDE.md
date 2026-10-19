# User guide / Hướng dẫn sử dụng

[Overview](../README.md) · [Privacy](PRIVACY.md) · [Troubleshooting](TROUBLESHOOTING.md)

[Giới thiệu](../README.md) · [Quyền riêng tư](PRIVACY.md) · [Xử lý sự cố](TROUBLESHOOTING.md)

## Install and update / Cài đặt và nâng cấp

Download from the [release page](https://github.com/TDUmii/QuietShare/releases/latest). Run **QuietShare-Setup.exe** for a per-user installation without administrator access. The installer supports English and Vietnamese. For portable use, extract the complete ZIP; keep Scintilla.dll, Lexilla.dll, pdfium.dll and WebView2Loader.dll beside QuietShare.exe. Close the app before updating. Upgrading or uninstalling preserves saved scripts.

Tải từ [trang phát hành](https://github.com/TDUmii/QuietShare/releases/latest). Chạy **QuietShare-Setup.exe** để cài cho tài khoản hiện tại, không cần quyền quản trị. Bộ cài có tiếng Anh và tiếng Việt. Nếu dùng portable, giải nén toàn bộ ZIP; giữ Scintilla.dll, Lexilla.dll, pdfium.dll và WebView2Loader.dll cạnh QuietShare.exe. Đóng ứng dụng trước khi nâng cấp. Nâng cấp hoặc gỡ cài đặt vẫn giữ lời nhắc đã lưu.

## Files and projects / File và dự án

Use **Open file / Ctrl+O**, drag local files into the app, or open a project folder with **Ctrl+Shift+O**. The protected picker provides Desktop, Downloads, Documents, Pictures and drive shortcuts. **Ctrl+B** shows/hides Explorer; double-click or press Enter to open a selected file. Explorer skips junctions/symlinks and, by default, hidden/heavy development folders.

Dùng **Mở file / Ctrl+O**, kéo file cục bộ vào ứng dụng hoặc mở thư mục dự án bằng **Ctrl+Shift+O**. Hộp chọn được bảo vệ có lối tắt Desktop, Downloads, Documents, Pictures và ổ đĩa. **Ctrl+B** ẩn/hiện Explorer; nhấp đúp hoặc Enter để mở file đã chọn. Explorer bỏ qua junction/symlink và mặc định lọc thư mục ẩn hoặc thư mục phát triển nặng.

Home lists files and projects you actually opened. Pin favorites, remove one entry or clear all history, including pins. Removing history never deletes the original file. Right-click **Open files** on the sidebar to switch documents. Up to 32 documents, including the optional script, can be open; view positions are retained during the session.

Home hiển thị file và dự án bạn thực sự đã mở. Có thể ghim, xóa một mục hoặc xóa toàn bộ lịch sử gồm cả ghim. Xóa lịch sử không xóa file gốc. Nhấp phải **Tệp đang mở** ở thanh bên để chuyển tài liệu. Có thể mở tối đa 32 tài liệu tính cả lời nhắc tùy chọn; vị trí xem được giữ trong phiên.

Text/code editing is enabled by default. Use **Save / Ctrl+S** to write changes. Saving preserves supported encoding/BOM and refuses to overwrite a file changed externally. Reload or keep your current draft when prompted. Closing a modified file offers save, discard or cancel. Disable editing in **Settings > Reading** for viewing only. Images and PDFs remain read-only; source code is never executed.

Văn bản/code mặc định cho phép sửa. Dùng **Lưu / Ctrl+S** để ghi thay đổi. Khi lưu, ứng dụng giữ mã hóa/BOM được hỗ trợ và chặn ghi đè nếu file đã đổi bên ngoài. Chọn tải lại hoặc giữ bản sửa khi được hỏi. Đóng file đang sửa có lựa chọn lưu, bỏ thay đổi hoặc hủy. Tắt sửa tại **Cài đặt > Nội dung** để chỉ xem. Ảnh và PDF chỉ đọc; code không được thực thi.

## Scripts and reading / Lời nhắc và cách đọc

Right-click **Open files > Compose script** to prepare a script. **Ctrl+Enter** switches editing/reading; **Space** starts or pauses auto-scroll in the reading area. Scripts autosave locally, separately from imported files.

Nhấp phải **Tệp đang mở > Soạn lời nhắc** để chuẩn bị nội dung. **Ctrl+Enter** chuyển soạn/đọc; **Space** chạy hoặc dừng cuộn tự động trong vùng đọc. Lời nhắc tự lưu trên máy, tách biệt với file đã mở.

The toolbar exposes text size, regular/semibold/bold weight and word wrap for text/scripts. Code has a wrap control. Text supports 8-200 pt; code, images, PDFs and web pages support 25-400% zoom. Defaults are 50% zoom and 12 pt for text/scripts. Reset or **Ctrl+0** restores defaults. Formatting/theme changes preserve the visible reading anchor and selection without writing formatting into plain-text files.

Thanh xem có cỡ chữ, độ đậm thường/vừa/đậm và xuống dòng cho văn bản/lời nhắc. Code có nút xuống dòng. Văn bản hỗ trợ 8-200 pt; code, ảnh, PDF và web hỗ trợ zoom 25-400%. Mặc định là zoom 50% và 12 pt cho văn bản/lời nhắc. Đặt lại hoặc **Ctrl+0** trở về mặc định. Đổi định dạng/theme giữ vùng đang đọc và vùng chọn, không ghi định dạng vào file text.

Use **Ctrl+F** to find, Enter/Shift+Enter for next/previous and Esc to close search. PDF search/copy requires a text layer. Page Up/Page Down changes PDF pages; the overflow menu includes page navigation and Fit Width. Drag blank PDF space to pan or drag text to select it.

Dùng **Ctrl+F** để tìm, Enter/Shift+Enter tìm tiếp/trước và Esc để đóng tìm kiếm. PDF cần lớp chữ để tìm/sao chép. Page Up/Page Down đổi trang PDF; menu mở rộng có chuyển trang và vừa chiều ngang. Kéo vùng trống PDF để di chuyển hoặc kéo trên chữ để chọn.

## Appearance / Giao diện

Choose **Settings > Appearance > Color theme**, or the theme button in the viewer toolbar. Choices are Classic dark, Light, Warm paper, Midnight blue, Forest dark, Graphite, Soft mint, Daylight blue, High contrast black and High contrast white. Themes persist across launches. Image pixels, PDF pages and website colors remain unchanged.

Chọn **Cài đặt > Giao diện > Chủ đề màu** hoặc nút theme trên thanh xem. Có Tối cổ điển, Sáng, Giấy ấm, Xanh đêm, Rừng đêm, Than chì, Sương xanh, Xanh ban ngày, Đen tương phản cao và Trắng tương phản cao. Theme được nhớ qua các lần mở. Màu ảnh, trang PDF và website giữ nguyên.

Appearance includes language, always-on-top, window opacity (40-100%), icon size (12-24 px) and remembering the last window size, enabled by default. Opacity includes controller text; public projection remains opaque. **Layout** toggles bars and Explorer; **F11** enters focus mode. Resource presets limit projection processing to 15/30/60 fps; actual performance depends on the display, GPU and content.

Giao diện có ngôn ngữ, ghim trên cùng, độ mờ cửa sổ (40-100%), icon (12-24 px) và nhớ kích thước lần cuối, mặc định bật. Độ mờ tác động cả chữ cửa sổ điều khiển; hình chiếu công khai vẫn đục. **Bố cục** ẩn/hiện các thanh và Explorer; **F11** vào chế độ tập trung. Chế độ tài nguyên giới hạn xử lý chiếu ở 15/30/60 fps; hiệu năng thực tế tùy màn hình, GPU và nội dung.

## Optional browser / Trình duyệt tùy chọn

Enable it in **Settings > Browser** or on the browser page. It requires WebView2 Runtime and starts disabled at each app launch. A local New tab page accepts HTTP/HTTPS URLs, domains, localhost or search terms. Up to 8 tabs share the local sign-in profile, with independent history/zoom during the session. Up to 10 saved shortcuts persist; open tabs are not restored next launch.

Bật tại **Cài đặt > Trình duyệt** hoặc trang trình duyệt. Tính năng cần WebView2 Runtime và luôn tắt khi mới mở ứng dụng. Trang Tab mới cục bộ nhận URL HTTP/HTTPS, tên miền, localhost hoặc từ khóa. Tối đa 8 tab dùng chung hồ sơ đăng nhập cục bộ, có lịch sử/zoom riêng trong phiên. Tối đa 10 lối tắt được lưu; danh sách tab mở không khôi phục ở lần chạy sau.

**Ctrl+T** opens a tab; **Ctrl+W** closes it. Closing the last web tab disables browsing. New tabs must pass capture verification before user pages load. Background tabs receive suspension requests; actual suspension depends on WebView2 and the page. Enabling web during projection holds the last public frame during verification, for up to about 5 seconds; failure/cancellation stops projection.

**Ctrl+T** mở tab; **Ctrl+W** đóng tab. Đóng tab web cuối cùng sẽ tắt trình duyệt. Tab mới phải qua xác minh capture trước khi tải trang người dùng. Tab nền nhận yêu cầu tạm ngưng; việc tạm ngưng thực tế tùy WebView2 và trang web. Khi bật web lúc đang chiếu, ứng dụng giữ khung hình công khai gần nhất trong lúc xác minh, tối đa khoảng 5 giây; thất bại/hủy sẽ dừng chiếu.

Explicit **Ctrl+V** can paste raster images into compatible web editors. Web file pickers, file drops, downloads, screen sharing, notifications, DevTools and native context menus are blocked. Separate-popup sign-in flows may not work. Microphone/camera access is off by default; see [privacy](PRIVACY.md) for grant conditions.

Người dùng có thể chủ động **Ctrl+V** ảnh raster vào ô nhập web tương thích. Hộp chọn file web, thả file, tải xuống, chia sẻ màn hình, thông báo, DevTools và menu native bị chặn. Đăng nhập cần popup riêng có thể không hoạt động. Camera/micro mặc định tắt; xem điều kiện cấp quyền tại [quyền riêng tư](PRIVACY.md).

## Supported content / Nội dung hỗ trợ

| Type | Limits / Giới hạn |
|---|---|
| Text and code / Văn bản và code | UTF-8 or UTF-16 with BOM, up to 5 MiB. Unknown extensions need valid text encoding and no binary content.<br>UTF-8 hoặc UTF-16 có BOM, tối đa 5 MiB. Đuôi lạ phải là văn bản hợp lệ, không chứa dữ liệu nhị phân. |
| Images / Ảnh | PNG, JPEG, BMP and the first GIF frame; up to 25 MiB and 40 megapixels.<br>PNG, JPEG, BMP và khung đầu GIF; tối đa 25 MiB, 40 megapixel. |
| PDF | Up to 64 MiB and 10,000 pages; one rendered page at a time, no password support.<br>Tối đa 64 MiB và 10.000 trang; dựng từng trang, chưa hỗ trợ mật khẩu. |
| Local paths / Đường dẫn cục bộ | Local files/folders only; executables, installers, archives and binaries are rejected.<br>Chỉ file/thư mục cục bộ; từ chối tệp thực thi, bộ cài, tệp nén và nhị phân. |

## Default shortcuts / Phím tắt mặc định

| Shortcut | Action / Thao tác |
|---|---|
| F8 | Mask content and stop projection/scrolling; press again to reveal when protection permits.<br>Che nội dung, dừng chiếu/cuộn; nhấn lại để mở khi điều kiện bảo vệ cho phép. |
| Ctrl+Alt+Q | Recall QuietShare while preserving foreground focus and masking state.<br>Gọi lại QuietShare, giữ focus hiện tại và trạng thái che. |
| F9 / F10 | Verify WGC / start or stop projection.<br>Kiểm tra WGC / bắt đầu hoặc dừng chiếu. |
| F11 | Toggle focus mode.<br>Bật/tắt chế độ tập trung. |
| Ctrl+O / Ctrl+Shift+O | Open files / a folder.<br>Mở file / thư mục. |
| Ctrl+S / Ctrl+B | Save text/code / toggle Explorer.<br>Lưu văn bản/code / ẩn hiện Explorer. |
| Ctrl+Tab / Ctrl+Shift+Tab | Next/previous document or web tab on its active page.<br>Tab tài liệu hoặc web tiếp/trước trên trang đang dùng. |
| Ctrl+Enter / Space | Script edit/read / auto-scroll in reading mode.<br>Soạn/đọc lời nhắc / cuộn tự động trong chế độ đọc. |
| Ctrl+F / Ctrl+0 | Find / reset content zoom or size.<br>Tìm kiếm / đặt lại zoom hoặc cỡ chữ. |

F8 and Ctrl+Alt+Q are global when registration succeeds. Other shortcuts depend on the focused area. Ten shortcuts can be changed in **Settings > Shortcuts**; reserved or duplicate bindings are rejected.

F8 và Ctrl+Alt+Q dùng toàn hệ thống khi đăng ký thành công. Phím khác tùy vùng đang nhận focus. Có thể đổi mười phím tắt tại **Cài đặt > Phím tắt**; tổ hợp trùng hoặc dành riêng bị từ chối.
