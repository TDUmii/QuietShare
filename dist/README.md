# QuietShare 0.3.0

QuietShare là cửa sổ riêng để đọc lời nhắc, xem code hoặc ảnh khi thuyết trình và chia sẻ màn hình. Ứng dụng chạy offline bằng EXE native cho Windows 10/11 x64, không cần .NET hay trình duyệt.

QuietShare is a private window for scripts, code and images during presentations and screen sharing. It runs offline as a native Windows 10/11 x64 EXE, with no .NET or browser runtime required.

![Chuẩn bị nội dung / Prepare content](docs/quietshare-home.png)

## Tiếng Việt

### Tính năng

- **Chuẩn bị**: mở hoặc kéo thả một file văn bản, code hay ảnh vào ứng dụng.
- **Lời nhắc / Đọc**: soạn lời nhắc, đọc với chữ lớn và cuộn tự động ở ba tốc độ.
- **Tùy chỉnh**: kéo thanh chỉnh cỡ chữ 12-72 pt; ảnh phóng/thu 25-400% so với kích thước vừa khung.
- **Tập trung**: F11 ẩn sidebar và toolbar; nút F11 nhỏ trong góc mở lại các điều khiển.
- Giao diện dark, scrollbar tối, ghim cửa sổ, che nhanh bằng F8, tiếng Việt và tiếng Anh.
- Lời nhắc tự lưu trên máy bằng Windows DPAPI. Xem file riêng không ghi đè lời nhắc đang soạn.

Văn bản và code hỗ trợ UTF-8 hoặc UTF-16 có BOM, tối đa 5 MiB. Các định dạng phổ biến gồm TXT, Markdown, LOG, C/C++, Python, JavaScript, TypeScript, HTML, CSS, JSON, XML và YAML. Code được hiển thị bằng font monospace. Ảnh hỗ trợ PNG, JPG/JPEG, BMP và khung hình đầu của GIF; tối đa 25 MiB và 40 megapixel.

### Cài đặt

Tải [QuietShare-v0.3.0-win-x64.zip](https://github.com/TDUmii/QuietShare/releases/tag/v0.3.0), giải nén và chạy `QuietShare-Setup.exe`. Bộ cài dùng tài khoản Windows hiện tại, không cần quyền quản trị. Để dùng bản portable, mở `QuietShare.exe` ngay trong thư mục vừa giải nén.

### Sử dụng

1. Tại **Chuẩn bị**, bấm **Mở hoặc thả file** hoặc kéo một file từ Explorer vào cửa sổ. Muốn soạn lời nói, chọn **Lời nhắc** và nhập hoặc dán nội dung.
2. Chọn **Đọc**, rồi bấm **Chạy** để cuộn lời nhắc. **Chậm / Vừa / Nhanh** chọn tốc độ; **Dừng** tạm dừng. Khi xem code hoặc ảnh, các nút cuộn tự động được ẩn.
3. Dùng **A- / A+**, **Tùy chỉnh** hoặc Ctrl rồi lăn chuột để chỉnh cỡ chữ. Với ảnh, dùng **- / +** hoặc Ctrl rồi lăn chuột để phóng/thu; kéo ảnh hoặc dùng phím mũi tên để di chuyển. Nút **100%** hoặc Ctrl+`0` đưa chữ về cỡ mặc định và ảnh về vừa khung.
4. Bấm **Tập trung** hoặc F11 để dành cửa sổ cho nội dung. Bấm nút **F11** trong góc, nhấn F11 lần nữa hoặc Esc để hiện lại sidebar và toolbar.
5. **Ghim** giữ cửa sổ trên cùng; bấm lại để bỏ ghim. Kéo thanh tiêu đề để di chuyển, kéo cạnh để đổi kích thước. **Che · F8** che nội dung ngay lập tức; F8 lần nữa mở lại.
6. **EN** chuyển sang tiếng Anh và **VI** chuyển lại tiếng Việt. Khi cửa sổ hẹp, sidebar thu thành icon; rê chuột để xem nhãn và phím tắt. Tab chuyển giữa các điều khiển.

Ctrl+Enter chuyển giữa soạn lời nhắc và đọc. Nếu đang xem file, Ctrl+Enter hoặc **Lời nhắc** đưa bạn về bản lời nhắc đã soạn. **Chuẩn bị > Xem file** mở lại file vừa xem. Chỉ lời nhắc được tự lưu và khôi phục khi mở app; file được mở trong phiên hiện tại.

Trước buổi chia sẻ online, thử nội dung mẫu và kiểm tra từ phía người nhận trong đúng phần mềm/cấu hình sẽ dùng. Windows có thể loại cửa sổ QuietShare khỏi một số đường capture. Với máy chiếu hoặc màn hình phụ, dùng **Win+P > Extend** để có màn hình riêng; **Duplicate** phát cùng hình ảnh trên cả hai màn hình và ứng dụng sẽ khóa nội dung trong chế độ này.

![Chế độ tập trung / Focus mode](docs/quietshare-reading.png)

## English

### Features

- **Home**: open or drop a text, source code or image file into the app.
- **Script / Read**: edit a script, read larger text and auto-scroll at three speeds.
- **Settings**: adjust text from 12-72 pt with a slider; zoom images from 25-400% relative to fit-to-window size.
- **Focus**: F11 hides the sidebar and toolbar; a small F11 button restores the controls.
- Dark UI and scrollbar, always-on-top pinning, instant F8 hide, Vietnamese and English UI.
- Local script autosave with Windows DPAPI. File previews keep your existing script intact.

Text and code support UTF-8 or BOM-marked UTF-16, up to 5 MiB. Common formats include TXT, Markdown, LOG, C/C++, Python, JavaScript, TypeScript, HTML, CSS, JSON, XML and YAML. Code uses a monospace font. Images support PNG, JPG/JPEG, BMP and the first frame of GIF, up to 25 MiB and 40 megapixels.

### Install

Download [QuietShare-v0.3.0-win-x64.zip](https://github.com/TDUmii/QuietShare/releases/tag/v0.3.0), extract it and run `QuietShare-Setup.exe`. The per-user installer does not require administrator rights. For portable use, open `QuietShare.exe` in the extracted folder.

### Use

1. On **Home**, click **Open or drop a file**, or drag one file from Explorer into the window. To prepare spoken notes, select **Script** and type or paste your script.
2. Select **Read**, then press **Start** to auto-scroll the script. **Slow / Medium / Fast** changes the speed; **Stop** pauses it. Auto-scroll controls are hidden for code and images.
3. Use **A- / A+**, **Settings**, or Ctrl plus the mouse wheel to change text size. For images, use **- / +** or Ctrl plus the wheel to zoom; drag the image or use the arrow keys to pan. **100%** or Ctrl+`0` restores the default text size or fits the image to the window.
4. Click **Focus** or press F11 to give the content more space. Click the corner **F11** button, press F11 again, or press Esc to restore the sidebar and toolbar.
5. **Pin** keeps the window on top; press it again to unpin. Drag the title bar to move the window and an edge to resize it. **Hide · F8** covers the content immediately; F8 again reveals it.
6. **EN** switches to English and **VI** switches back to Vietnamese. A narrow window collapses the sidebar to icons; hover for labels and shortcuts. Tab moves between controls.

Ctrl+Enter switches between editing and reading your script. From a file preview, Ctrl+Enter or **Script** restores your existing script. **Home > View file** returns to the last opened file. Only the script is autosaved and restored on launch; opened files belong to the current session.

Before a real online share, test sample content from the receiver's side with the meeting app and capture mode you will use. Windows can exclude QuietShare from some capture paths. For a projector or second display, choose **Win+P > Extend** for a private display; **Duplicate** mirrors the same pixels and the app locks its content in that mode.

The release is native Windows x64 code and is not digitally signed. Native compilation makes casual decompilation harder, but no executable can promise absolute protection against reverse engineering.
