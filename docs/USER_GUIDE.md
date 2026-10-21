# User guide / Hướng dẫn sử dụng

[Overview](../README.md) · [Privacy](PRIVACY.md) · [Troubleshooting](TROUBLESHOOTING.md)

[Giới thiệu](../README.md) · [Quyền riêng tư](PRIVACY.md) · [Xử lý sự cố](TROUBLESHOOTING.md)

## Install and update / Cài đặt và nâng cấp

**English**

1. Download from the [release page](https://github.com/TDUmii/QuietShare/releases/latest).
2. Run **QuietShare-Setup.exe** for a per-user installation without administrator access. The installer supports English and Vietnamese.
3. For portable use, extract the complete ZIP. Keep Scintilla.dll, Lexilla.dll, pdfium.dll and WebView2Loader.dll beside QuietShare.exe.
4. Close the app before updating. Upgrading or uninstalling preserves saved scripts.

**Tiếng Việt**

1. Tải từ [trang phát hành](https://github.com/TDUmii/QuietShare/releases/latest).
2. Chạy **QuietShare-Setup.exe** để cài cho tài khoản hiện tại, không cần quyền quản trị. Bộ cài có tiếng Anh và tiếng Việt.
3. Nếu dùng portable, giải nén toàn bộ ZIP. Giữ Scintilla.dll, Lexilla.dll, pdfium.dll và WebView2Loader.dll cạnh QuietShare.exe.
4. Đóng ứng dụng trước khi nâng cấp. Nâng cấp hoặc gỡ cài đặt vẫn giữ lời nhắc đã lưu.

## Files and projects / File và dự án

- **Open files:** choose **Open file / Ctrl+O** or drag local files into the app.
- **Open a project folder:** press **Ctrl+Shift+O**.
- **Find common folders:** the protected picker provides Desktop, Downloads, Documents, Pictures and drive shortcuts.
- **Use Explorer:** press **Ctrl+B** to show or hide it. Double-click or press Enter to open a selected file.
- Explorer skips junctions/symlinks and, by default, hidden/heavy development folders.

- **Mở file:** chọn **Mở file / Ctrl+O** hoặc kéo file cục bộ vào ứng dụng.
- **Mở thư mục dự án:** nhấn **Ctrl+Shift+O**.
- **Tìm thư mục quen thuộc:** hộp chọn được bảo vệ có lối tắt Desktop, Downloads, Documents, Pictures và ổ đĩa.
- **Dùng Explorer:** nhấn **Ctrl+B** để ẩn hoặc hiện. Nhấp đúp hoặc Enter để mở file đã chọn.
- Explorer bỏ qua junction/symlink và mặc định lọc thư mục ẩn hoặc thư mục phát triển nặng.

- **Recent history:** Home lists files and projects you actually opened.
- Pin favorites, remove one entry or clear all history, including pins. Removing history never deletes the original file.
- **Switch documents:** right-click **Open files** on the sidebar.
- Up to 32 documents, including the optional script, can be open. View positions are retained during the session.

- **Lịch sử gần đây:** Home hiển thị file và dự án bạn thực sự đã mở.
- Có thể ghim, xóa một mục hoặc xóa toàn bộ lịch sử gồm cả ghim. Xóa lịch sử không xóa file gốc.
- **Chuyển tài liệu:** nhấp phải **Tệp đang mở** ở thanh bên.
- Có thể mở tối đa 32 tài liệu tính cả lời nhắc tùy chọn. Vị trí xem được giữ trong phiên.

- **Edit:** text/code editing is enabled by default. Disable editing in **Settings > Reading** for viewing only.
- **Save:** use **Save / Ctrl+S**. Saving preserves supported encoding/BOM.
- **External changes:** saving refuses to overwrite a file changed externally. Reload or keep your current draft when prompted.
- **Close a modified file:** choose save, discard or cancel.
- Images and PDFs remain read-only. Source code is never executed.

- **Sửa:** văn bản/code mặc định cho phép sửa. Tắt sửa tại **Cài đặt > Nội dung** để chỉ xem.
- **Lưu:** dùng **Lưu / Ctrl+S**. Ứng dụng giữ mã hóa/BOM được hỗ trợ.
- **Thay đổi bên ngoài:** ứng dụng chặn ghi đè nếu file đã đổi trong ứng dụng khác. Chọn tải lại hoặc giữ bản sửa khi được hỏi.
- **Đóng file đang sửa:** chọn lưu, bỏ thay đổi hoặc hủy.
- Ảnh và PDF chỉ đọc. Code không được thực thi.

## Scripts and reading / Lời nhắc và cách đọc

**English**

1. Right-click **Open files > Compose script** to prepare a script.
2. Press **Ctrl+Enter** to switch editing/reading.
3. Press **Space** in the reading area to start or pause auto-scroll.

Scripts autosave locally, separately from imported files.

**Tiếng Việt**

1. Nhấp phải **Tệp đang mở > Soạn lời nhắc** để chuẩn bị nội dung.
2. Nhấn **Ctrl+Enter** để chuyển soạn/đọc.
3. Nhấn **Space** trong vùng đọc để chạy hoặc dừng cuộn tự động.

Lời nhắc tự lưu trên máy, tách biệt với file đã mở.

- **Text/scripts:** use toolbar controls for size, regular/semibold/bold weight and word wrap. Text supports 8-200 pt.
- **Code:** use its wrap control. Code, images, PDFs and web pages support 25-400% zoom.
- **Defaults:** 50% zoom and 12 pt for text/scripts. Reset or **Ctrl+0** restores defaults.
- Formatting/theme changes preserve the visible reading anchor and selection. Formatting is not written into plain-text files.

- **Văn bản/lời nhắc:** dùng thanh xem để chỉnh cỡ chữ, độ đậm thường/vừa/đậm và xuống dòng. Cỡ chữ hỗ trợ 8-200 pt.
- **Code:** có nút xuống dòng riêng. Code, ảnh, PDF và web hỗ trợ zoom 25-400%.
- **Mặc định:** zoom 50% và 12 pt cho văn bản/lời nhắc. Đặt lại hoặc **Ctrl+0** để khôi phục.
- Đổi định dạng/theme giữ vùng đang đọc và vùng chọn. Định dạng không được ghi vào file text.

- **Search:** press **Ctrl+F**, then Enter/Shift+Enter for next/previous. Press Esc to close search.
- **PDF text:** search/copy requires a text layer.
- **PDF pages:** use Page Up/Page Down. The overflow menu includes page navigation and Fit Width.
- **PDF movement:** drag blank space to pan or drag text to select it.

- **Tìm kiếm:** nhấn **Ctrl+F**, sau đó Enter/Shift+Enter để tìm tiếp/trước. Nhấn Esc để đóng.
- **Chữ PDF:** cần lớp chữ để tìm/sao chép.
- **Trang PDF:** dùng Page Up/Page Down. Menu mở rộng có chuyển trang và vừa chiều ngang.
- **Di chuyển trong PDF:** kéo vùng trống để di chuyển hoặc kéo trên chữ để chọn.

## Appearance / Giao diện

- Choose **Settings > Appearance > Color theme**, or the theme button in the viewer toolbar.
- Choices are Classic dark, Light, Warm paper, Midnight blue, Forest dark, Graphite, Soft mint, Daylight blue, High contrast black and High contrast white.
- Themes persist across launches. Image pixels, PDF pages and website colors remain unchanged.

- Chọn **Cài đặt > Giao diện > Chủ đề màu** hoặc nút theme trên thanh xem.
- Có Tối cổ điển, Sáng, Giấy ấm, Xanh đêm, Rừng đêm, Than chì, Sương xanh, Xanh ban ngày, Đen tương phản cao và Trắng tương phản cao.
- Theme được nhớ qua các lần mở. Màu ảnh, trang PDF và website giữ nguyên.

- **Appearance settings:** language, always-on-top, window opacity (40-100%) and icon size (12-24 px).
- **Window size:** remembering the last size is enabled by default.
- **Opacity:** affects controller text as well. Public projection remains opaque.
- **Layout:** toggle bars and Explorer. Press **F11** for focus mode.
- **Resources:** presets limit projection processing to 15/30/60 fps. Actual performance depends on the display, GPU and content.

- **Cài đặt giao diện:** ngôn ngữ, ghim trên cùng, độ mờ cửa sổ (40-100%) và icon (12-24 px).
- **Kích thước cửa sổ:** nhớ kích thước lần cuối được bật mặc định.
- **Độ mờ:** tác động cả chữ cửa sổ điều khiển. Hình chiếu công khai vẫn đục.
- **Bố cục:** ẩn/hiện các thanh và Explorer. Nhấn **F11** vào chế độ tập trung.
- **Tài nguyên:** các chế độ giới hạn xử lý chiếu ở 15/30/60 fps. Hiệu năng thực tế tùy màn hình, GPU và nội dung.

## Optional browser / Trình duyệt tùy chọn

- **Enable:** use **Settings > Browser** or the browser page. Browsing requires WebView2 Runtime and starts disabled at each app launch.
- **Navigate:** the local New tab page accepts HTTP/HTTPS URLs, domains, localhost or search terms.
- **Tabs:** up to 8 share the local sign-in profile, with independent history/zoom during the session.
- **Shortcuts:** up to 10 saved shortcuts persist. Open tabs are not restored next launch.

- **Bật:** dùng **Cài đặt > Trình duyệt** hoặc trang trình duyệt. Tính năng cần WebView2 Runtime và luôn tắt khi mới mở ứng dụng.
- **Điều hướng:** trang Tab mới cục bộ nhận URL HTTP/HTTPS, tên miền, localhost hoặc từ khóa.
- **Tab:** tối đa 8 tab dùng chung hồ sơ đăng nhập cục bộ, có lịch sử/zoom riêng trong phiên.
- **Lối tắt:** tối đa 10 lối tắt được lưu. Danh sách tab mở không khôi phục ở lần chạy sau.

- **Ctrl+T** opens a tab. **Ctrl+W** closes it. Closing the last web tab disables browsing.
- New tabs must pass capture verification before user pages load.
- Background tabs receive suspension requests. Actual suspension depends on WebView2 and the page.
- Enabling web during projection holds the last public frame during verification, for up to about 5 seconds. Failure/cancellation stops projection.

- **Ctrl+T** mở tab. **Ctrl+W** đóng tab. Đóng tab web cuối cùng sẽ tắt trình duyệt.
- Tab mới phải qua xác minh capture trước khi tải trang người dùng.
- Tab nền nhận yêu cầu tạm ngưng. Việc tạm ngưng thực tế tùy WebView2 và trang web.
- Khi bật web lúc đang chiếu, ứng dụng giữ khung hình công khai gần nhất trong lúc xác minh, tối đa khoảng 5 giây. Thất bại/hủy sẽ dừng chiếu.

- Explicit **Ctrl+V** can paste raster images into compatible web editors.
- Web file pickers, file drops, downloads, screen sharing, notifications, DevTools and native context menus are blocked.
- Separate-popup sign-in flows may not work.
- Microphone/camera access is off by default. See [privacy](PRIVACY.md) for grant conditions.

## Local meeting recording / Ghi âm cuộc họp cục bộ

- **English:** Select **Record meeting audio** on the sidebar when you want to capture the Windows playback mix. The red record icon and footer show the active state. Select it again to stop. QuietShare saves a WAV file under `%LOCALAPPDATA%\QuietShare\Meetings`.
- The recorder captures system playback only. It does not open the microphone, does not begin at launch and does not continue after QuietShare closes.
- Use the script editor beside the recording control for manual notes. This release stores audio and notes separately. It does not claim automatic speech-to-text.

- **Tiếng Việt:** Chọn **Ghi âm cuộc họp** ở thanh bên khi muốn lưu âm thanh phát ra từ Windows. Icon ghi màu đỏ và thanh trạng thái cho biết đang ghi. Chọn lại nút đó để dừng. QuietShare lưu file WAV tại `%LOCALAPPDATA%\QuietShare\Meetings`.
- Bộ ghi chỉ lấy âm thanh phát ra từ máy. Nó không mở micro, không tự ghi khi khởi động và dừng khi QuietShare đóng.
- Có thể dùng vùng soạn lời nhắc cạnh nút ghi để ghi chú thủ công. Phiên bản này lưu âm thanh và lời nhắc riêng, chưa tự nhận dạng lời nói.

- Người dùng có thể chủ động **Ctrl+V** ảnh raster vào ô nhập web tương thích.
- Hộp chọn file web, thả file, tải xuống, chia sẻ màn hình, thông báo, DevTools và menu native bị chặn.
- Đăng nhập cần popup riêng có thể không hoạt động.
- Camera/micro mặc định tắt. Xem điều kiện cấp quyền tại [quyền riêng tư](PRIVACY.md).

## Supported content / Nội dung hỗ trợ

| Type | Limits / Giới hạn |
|---|---|
| Text and code / Văn bản và code | UTF-8 or UTF-16 with BOM, up to 5 MiB. Unknown extensions need valid text encoding and no binary content.<br>UTF-8 hoặc UTF-16 có BOM, tối đa 5 MiB. Đuôi lạ phải là văn bản hợp lệ, không chứa dữ liệu nhị phân. |
| Images / Ảnh | PNG, JPEG, BMP and the first GIF frame. Up to 25 MiB and 40 megapixels.<br>PNG, JPEG, BMP và khung đầu GIF. Tối đa 25 MiB, 40 megapixel. |
| PDF | Up to 64 MiB and 10,000 pages. One rendered page at a time, no password support.<br>Tối đa 64 MiB và 10.000 trang. Dựng từng trang, chưa hỗ trợ mật khẩu. |
| Local paths / Đường dẫn cục bộ | Local files/folders only. Executables, installers, archives and binaries are rejected.<br>Chỉ file/thư mục cục bộ. Từ chối tệp thực thi, bộ cài, tệp nén và nhị phân. |

## Default shortcuts / Phím tắt mặc định

| Shortcut | Action / Thao tác |
|---|---|
| F8 | Mask content and stop projection/scrolling. Press again to reveal when protection permits.<br>Che nội dung, dừng chiếu/cuộn. Nhấn lại để mở khi điều kiện bảo vệ cho phép. |
| Ctrl+Alt+Q | Recall QuietShare while preserving foreground focus and masking state.<br>Gọi lại QuietShare, giữ focus hiện tại và trạng thái che. |
| F9 / F10 | Verify WGC / start or stop projection.<br>Kiểm tra WGC / bắt đầu hoặc dừng chiếu. |
| F11 | Toggle focus mode.<br>Bật/tắt chế độ tập trung. |
| Ctrl+O / Ctrl+Shift+O | Open files / a folder.<br>Mở file / thư mục. |
| Ctrl+S / Ctrl+B | Save text/code / toggle Explorer.<br>Lưu văn bản/code / ẩn hiện Explorer. |
| Ctrl+Tab / Ctrl+Shift+Tab | Next/previous document or web tab on its active page.<br>Tab tài liệu hoặc web tiếp/trước trên trang đang dùng. |
| Ctrl+Enter / Space | Script edit/read / auto-scroll in reading mode.<br>Soạn/đọc lời nhắc / cuộn tự động trong chế độ đọc. |
| Ctrl+F / Ctrl+0 | Find / reset content zoom or size.<br>Tìm kiếm / đặt lại zoom hoặc cỡ chữ. |

- F8 and Ctrl+Alt+Q are global when registration succeeds.
- Other shortcuts depend on the focused area.
- Change ten shortcuts in **Settings > Shortcuts**. Reserved or duplicate bindings are rejected.

- F8 và Ctrl+Alt+Q dùng toàn hệ thống khi đăng ký thành công.
- Phím khác tùy vùng đang nhận focus.
- Đổi mười phím tắt tại **Cài đặt > Phím tắt**. Tổ hợp trùng hoặc dành riêng bị từ chối.
