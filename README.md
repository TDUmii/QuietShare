# QuietShare 0.2.0

QuietShare là cửa sổ nhắc chữ riêng cho lúc bạn thuyết trình, dạy học hoặc chia sẻ màn hình. Ứng dụng chạy offline bằng EXE native cho Windows 10/11 x64, không cần cài .NET hay trình duyệt.

![QuietShare reading view](docs/quietshare-reading.png)

## Tiếng Việt

### Cài đặt

Tải [QuietShare-v0.2.0-win-x64.zip](QuietShare-v0.2.0-win-x64.zip), giải nén rồi chạy `QuietShare-Setup.exe`. Bộ cài chỉ dùng tài khoản Windows hiện tại, không cần quyền quản trị. Nếu muốn dùng ngay, mở `dist/QuietShare.exe`.

### Sử dụng

1. Dán lời cần nói vào vùng nội dung.
2. Bấm **Đọc** để chuyển sang khung chữ lớn, sau đó bấm **Chạy** để cuộn tự động.
3. Dùng **Chậm / Vừa / Nhanh**, **A- / A+** hoặc nút **100%** để điều chỉnh. Bạn cũng có thể dùng Ctrl+`+`, Ctrl+`-`, Ctrl+`0`, hoặc giữ Ctrl rồi lăn chuột.
4. Bấm **Ghim** để giữ cửa sổ trên cùng; bấm lại thành **Thả** khi muốn di chuyển hoặc làm việc với cửa sổ khác. Kéo thanh tiêu đề để đặt cửa sổ ở vị trí mong muốn.
5. Bấm **Che · F8** khi cần giấu lời nhắc ngay lập tức. Nút **EN** chuyển giao diện sang tiếng Anh; nút **VI** chuyển lại tiếng Việt.

Nội dung tự lưu trên máy bằng Windows DPAPI và được khôi phục ở lần mở sau. Ứng dụng chỉ có một cửa sổ đang chỉnh sửa để tránh hai bản lời ghi đè lên nhau.

Khi chia sẻ online, hãy thử bằng một đoạn văn mẫu trong đúng phần mềm và cấu hình bạn sẽ dùng. Windows có thể loại cửa sổ này khỏi một số đường capture. Nếu dùng máy chiếu hoặc màn hình phụ, chọn **Win+P > Extend**; chế độ **Duplicate** phát cùng một hình ảnh trên cả hai màn hình và không thể giữ lời nhắc riêng trên laptop.

## English

### Install

Download [QuietShare-v0.2.0-win-x64.zip](QuietShare-v0.2.0-win-x64.zip), extract it, and run `QuietShare-Setup.exe`. The per-user installer does not require administrator rights. For a portable launch, open `dist/QuietShare.exe`.

### Use

1. Paste your script into the editor.
2. Select **Read**, then press **Start** to begin auto-scrolling.
3. Choose **Slow / Medium / Fast**, use **A- / A+**, or press **100%** to reset. Ctrl+`+`, Ctrl+`-`, Ctrl+`0`, and Ctrl plus the mouse wheel also change the reading size.
4. Use **Pin** to keep the window above other windows; press it again to **Free** the window. Drag the title bar to move or resize it.
5. Press **Hide · F8** to cover the script immediately. **EN** switches to English and **VI** switches back to Vietnamese.

The script is saved locally with Windows DPAPI and restored on the next launch. QuietShare keeps one editing window open so two copies cannot overwrite the same script.

Before a real share, test a sample script with the exact meeting app and capture mode you will use. Windows capture exclusion depends on the receiver and capture path. For a projector or second display, use **Win+P > Extend**; physical **Duplicate** mirrors the same pixels and cannot show a private script on the laptop.

The release is native Windows x64 code and is not digitally signed. Native compilation makes casual decompilation harder, but no executable can promise absolute protection against reverse engineering.
