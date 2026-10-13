# QuietShare 0.2.0

QuietShare là cửa sổ nhắc chữ riêng cho lúc bạn thuyết trình, dạy học hoặc chia sẻ màn hình. Ứng dụng chạy offline bằng EXE native cho Windows 10/11 x64, không cần cài .NET hay trình duyệt.

![QuietShare reading view](docs/quietshare-reading.png)

## Tiếng Việt

### Tính năng

- Giao diện dark hiện đại với sidebar **Lời nhắc / Đọc**, điểm nhấn xanh và bố cục tự co theo cửa sổ.
- Chế độ đọc tập trung, chữ phóng to tùy ý, cuộn tự động, ghim cửa sổ và nút che nhanh F8.
- Hỗ trợ tiếng Việt và tiếng Anh; lời nhắc tự lưu cục bộ bằng Windows DPAPI.

### Cài đặt

Tải [QuietShare-v0.2.0-win-x64.zip](QuietShare-v0.2.0-win-x64.zip), giải nén rồi chạy `QuietShare-Setup.exe`. Bộ cài chỉ dùng tài khoản Windows hiện tại, không cần quyền quản trị. Nếu muốn dùng ngay, mở `QuietShare.exe` trong thư mục vừa giải nén.

### Sử dụng

1. Chọn **Lời nhắc** trong sidebar, rồi dán hoặc nhập lời cần nói vào vùng nội dung.
2. Chọn **Đọc** trong sidebar để chuyển sang chế độ đọc, sau đó bấm **Chạy** để cuộn tự động.
3. Dùng **Chậm / Vừa / Nhanh**, **A- / A+** hoặc nút **100%** để điều chỉnh. Bạn cũng có thể dùng Ctrl+`+`, Ctrl+`-`, Ctrl+`0`, hoặc giữ Ctrl rồi lăn chuột.
4. Bấm **Ghim** để giữ cửa sổ trên cùng; bấm lại thành **Thả** để bỏ ghim. Kéo thanh tiêu đề để di chuyển và kéo cạnh cửa sổ để đổi kích thước.
5. Bấm **Che · F8** khi cần giấu lời nhắc ngay lập tức. Nút **EN** chuyển giao diện sang tiếng Anh; nút **VI** chuyển lại tiếng Việt.

Nội dung tự lưu trên máy bằng Windows DPAPI và được khôi phục ở lần mở sau. Ứng dụng chỉ có một cửa sổ đang chỉnh sửa để tránh hai bản lời ghi đè lên nhau.

Khi cửa sổ hẹp, sidebar thu gọn thành icon. Rê chuột lên icon hoặc nút để xem nhãn và phím tắt; dùng Tab để chuyển giữa các điều khiển. Ctrl+Enter chuyển giữa Lời nhắc và Đọc.

Khi chia sẻ online, hãy thử bằng một đoạn văn mẫu trong đúng phần mềm và cấu hình bạn sẽ dùng. Windows có thể loại cửa sổ này khỏi một số đường capture. Nếu dùng máy chiếu hoặc màn hình phụ, chọn **Win+P > Extend**; chế độ **Duplicate** phát cùng một hình ảnh trên cả hai màn hình và không thể giữ lời nhắc riêng trên laptop.

## English

### Features

- A modern dark shell with **Script / Read** sidebar navigation, a green accent and responsive desktop layout.
- Focused reading mode with adjustable type size, auto-scroll, always-on-top pinning and an instant F8 hide action.
- Vietnamese and English UI; the script is autosaved locally with Windows DPAPI.

### Install

Download [QuietShare-v0.2.0-win-x64.zip](QuietShare-v0.2.0-win-x64.zip), extract it, and run `QuietShare-Setup.exe`. The per-user installer does not require administrator rights. For a portable launch, open `QuietShare.exe` in the extracted folder.

### Use

1. Select **Script** in the sidebar, then paste or type your script into the editor.
2. Select **Read** in the sidebar, then press **Start** to begin auto-scrolling.
3. Choose **Slow / Medium / Fast**, use **A- / A+**, or press **100%** to reset. Ctrl+`+`, Ctrl+`-`, Ctrl+`0`, and Ctrl plus the mouse wheel also change the reading size.
4. Use **Pin** to keep the window above other windows; press it again to **Free** the window. Drag the title bar to move it and drag a window edge to resize it.
5. Press **Hide · F8** to cover the script immediately. **EN** switches to English and **VI** switches back to Vietnamese.

The script is saved locally with Windows DPAPI and restored on the next launch. QuietShare keeps one editing window open so two copies cannot overwrite the same script.

In a narrow window, the sidebar collapses to icons. Hover over an icon or button to see its label and shortcut; use Tab to move between controls. Ctrl+Enter switches between Script and Read.

Before a real share, test a sample script with the exact meeting app and capture mode you will use. Windows capture exclusion depends on the receiver and capture path. For a projector or second display, use **Win+P > Extend**; physical **Duplicate** mirrors the same pixels and cannot show a private script on the laptop.

The release is native Windows x64 code and is not digitally signed. Native compilation makes casual decompilation harder, but no executable can promise absolute protection against reverse engineering.
