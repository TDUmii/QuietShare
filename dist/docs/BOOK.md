# User manuals / Hướng dẫn sử dụng

Choose a complete manual in your language. Each book covers installation, everyday reading, file editing, scripts, themes, browsing, sharing workflows, projection error codes and recovery.

Chọn quyển hướng dẫn đầy đủ theo ngôn ngữ. Mỗi quyển có cài đặt, cách đọc hằng ngày, sửa file, lời nhắc, giao diện màu, trình duyệt, quy trình chia sẻ, mã lỗi trình chiếu và phục hồi.

## English manual / Quyển tiếng Anh

- [Read the English manual](USER_MANUAL_EN.md)
- [Download the English PDF](QuietShare-User-Manual-EN.pdf)

## Vietnamese manual / Quyển tiếng Việt

- [Đọc hướng dẫn tiếng Việt](USER_MANUAL_VI.md)
- [Tải PDF tiếng Việt](QuietShare-Huong-Dan-Su-Dung-VI.pdf)

## Find a projection error / Tra lỗi trình chiếu

Open chapter 13 of either manual for codes `QS-P001` through `QS-P024`. Chapter 14 explains the local diagnostic record and the information to include in a report. `QS-P000` is an unverified status. `QS-P099` is the unknown-reason fallback.

Mở chương 13 của một trong hai quyển để tra mã `QS-P001` đến `QS-P024`. Chương 14 giải thích bản ghi chẩn đoán cục bộ và thông tin cần gửi khi báo lỗi. `QS-P000` là trạng thái chưa xác minh. `QS-P099` là mã dự phòng cho lý do chưa biết.

A clickable Play button does not establish that verification passed. If projection fails and content hides, read **Settings > Projection** and follow the exact code in the manual.

Nút Play bấm được không chứng minh xác minh đã đạt. Nếu chiếu thất bại và nội dung bị ẩn, đọc **Cài đặt > Trình chiếu** rồi làm theo đúng mã trong hướng dẫn.

## Meeting recording and transcription codes / Mã ghi âm và chép lời cuộc họp

| Code | Meaning / Ý nghĩa | Action / Cách xử lý |
|---|---|---|
| `QS-R001` | WAV recording is missing / Không tìm thấy WAV | Stop and start a new recording. / Dừng rồi ghi phiên mới. |
| `QS-R002` | Windows Speech API could not start / Speech API không khởi động được | Restart QuietShare and check Windows speech services. / Mở lại QuietShare và kiểm tra dịch vụ giọng nói Windows. |
| `QS-R003` | WAV format is not supported / Định dạng WAV không được hỗ trợ | Keep the recording from the default playback device and try again. / Ghi lại từ thiết bị phát mặc định. |
| `QS-R004` | Selected language recognizer is missing / Thiếu bộ nhận dạng ngôn ngữ đã chọn | Install local Whisper or a matching Windows recognizer, then retry. / Cài Whisper cục bộ hoặc bộ nhận dạng Windows đúng ngôn ngữ rồi thử lại. |
| `QS-R005` | Windows could not recognize the audio / Windows không nhận dạng được âm thanh | Check that the meeting audio was playing and choose the correct language. / Kiểm tra âm thanh cuộc họp đang phát và chọn đúng ngôn ngữ. |
| `QS-R006` | Transcription was stopped / Đã dừng chép lời | Start a new recording if a transcript is still needed. / Ghi lại nếu vẫn cần transcript. |
| `QS-R007` | Transcript could not be saved / Không lưu được transcript | Check local storage permission and free space. / Kiểm tra quyền ghi và dung lượng cục bộ. |

The recorder and transcriber use the Windows playback mix only. They do not open a microphone or upload audio.

Bộ ghi và bộ chép lời chỉ dùng âm thanh phát ra từ Windows. Chúng không mở micro và không tải âm thanh lên mạng.

[Documentation index](README.md) | [Mục lục tài liệu](README.md)

## Presentation-set errors / Lỗi bộ thuyết trình

| Code / Mã | English | Tiếng Việt |
|---|---|---|
| `QS-W001` | Saved library cannot be read. Close QuietShare and keep a copy of presentations.dpapi. Restore a known-good copy under the same Windows account. To start a new library, rename the old file to presentations-backup.dpapi before reopening. The app preserves the unreadable file. | Không đọc được thư viện đã lưu. Đóng QuietShare và giữ một bản sao presentations.dpapi. Khôi phục bản tốt bằng cùng tài khoản Windows. Nếu muốn tạo thư viện mới, đổi tên file cũ thành presentations-backup.dpapi trước khi mở lại. Ứng dụng giữ nguyên file không đọc được. |
| `QS-W002` | Set name is empty or too long. Enter a name containing 1-80 characters. | Tên bộ trống hoặc quá dài. Nhập tên có từ 1 đến 80 ký tự. |
| `QS-W003` | A document has unsaved edits. Return to that document and use Save or Ctrl+S. Then save the set again. Originals are not overwritten by saving a set. | Tài liệu có thay đổi chưa lưu. Quay lại tài liệu và chọn Lưu hoặc Ctrl+S, sau đó lưu bộ lại. Thao tác lưu bộ không ghi đè file gốc. |
| `QS-W004` | Eight sets are already saved. Delete an unneeded set or use an existing name and confirm replacement. | Đã lưu đủ tám bộ. Xóa bộ không cần dùng hoặc nhập tên bộ có sẵn và xác nhận thay thế. |
| `QS-W005` | Library cannot be saved or updated. Check free space and write access to the local QuietShare data folder. Keep the existing library and retry after correcting the condition. | Không lưu hoặc cập nhật được thư viện. Kiểm tra dung lượng trống và quyền ghi thư mục dữ liệu QuietShare cục bộ. Giữ nguyên thư viện cũ và thử lại sau khi khắc phục. |
| `QS-W006` | Previous script backup could not be created. Check access and free space in the local data folder. The current session remains open. Correct the condition before loading the set. | Không tạo được bản dự phòng lời nhắc. Kiểm tra quyền truy cập và dung lượng thư mục dữ liệu cục bộ. Phiên hiện tại vẫn được giữ. Khắc phục trước khi mở bộ. |
| `QS-W007` | One or more saved paths could not be opened. Locate the moved or unavailable files. Open them with Ctrl+O, then save the corrected set. Available documents are still restored. | Không mở được một số đường dẫn đã lưu. Tìm lại file đã di chuyển hoặc tạm không truy cập được. Mở bằng Ctrl+O rồi lưu lại bộ đã sửa. Các tài liệu còn mở được vẫn được khôi phục. |

## Reader, notebook and optional speech errors / Lỗi trình đọc, sổ ghi chép và nhận dạng tùy chọn

| Code / Mã | English - recovery | Tiếng Việt - cách xử lý |
|---|---|---|
| `QS-W008` | Preview preference could not be saved. Check local profile write access. The current session choice still applies. | Không lưu được lựa chọn xem trước. Kiểm tra quyền ghi thư mục dữ liệu. Lựa chọn vẫn áp dụng trong phiên hiện tại. |
| `QS-D001` | Reading metadata cannot be read. Close QuietShare, back up reading.dpapi, then restore a known-good copy under the same Windows account or rename it to start fresh. | Không đọc được dữ liệu đọc. Đóng QuietShare, sao lưu reading.dpapi rồi khôi phục bản tốt bằng cùng tài khoản Windows hoặc đổi tên file để tạo mới. |
| `QS-D002` | Reading metadata cannot be saved, or 64 bookmarks are already present. Check write access/free space, or remove an unneeded bookmark. | Không lưu được dữ liệu đọc hoặc đã đủ 64 dấu trang. Kiểm tra quyền ghi/dung lượng hoặc bỏ dấu trang không cần. |
| `QS-D003` | Page number is outside this PDF. Enter an integer from 1 to the displayed page count. | Số trang nằm ngoài PDF. Nhập số nguyên từ 1 đến tổng số trang đang hiện. |
| `QS-N001` | Saved notes cannot be read. Close QuietShare and back up meeting-notes.dpapi. Restore a known-good copy under the same account, or rename it before starting a new notebook. | Không đọc được ghi chép cũ. Đóng QuietShare và sao lưu meeting-notes.dpapi. Khôi phục bản tốt bằng cùng tài khoản hoặc đổi tên file trước khi tạo sổ mới. |
| `QS-N002` | Invalid segment times. Use HH:MM:SS.mmm with end after start, or leave both fields blank. | Giờ của đoạn không hợp lệ. Dùng HH:MM:SS.mmm với giờ cuối sau giờ đầu hoặc để trống cả hai ô. |
| `QS-N003` | VTT needs valid timing for every nonempty segment. Add times or export TXT instead. | VTT cần thời gian hợp lệ cho mọi đoạn có nội dung. Nhập giờ hoặc xuất TXT. |
| `QS-N004` | Transcript format or size is unsupported. Use UTF-8 VTT/TXT, at most 10,000 segments, 32,000 characters per segment and 2 million characters overall. | Transcript không đúng định dạng hoặc vượt giới hạn. Dùng VTT/TXT UTF-8, tối đa 10.000 đoạn, 32.000 ký tự mỗi đoạn và khoảng 2 triệu ký tự tổng. |
| `QS-N005` | Notebook backup/save/export failed. Check the selected local output folder, write permission and free space. Preserve the existing notes and retry. | Không tạo dự phòng/lưu/xuất được ghi chép. Kiểm tra thư mục xuất cục bộ, quyền ghi và dung lượng. Giữ bản cũ rồi thử lại. |
| `QS-R008` | Optional speech setup or integrity check failed. Check internet access and disk space, then use Install / repair local Whisper again. Existing audio remains local. | Cài bộ nhận dạng hoặc kiểm tra toàn vẹn thất bại. Kiểm tra mạng và dung lượng rồi chọn Cài / sửa Whisper cục bộ. Âm thanh cũ vẫn nằm trên máy. |
| `QS-R009` | Whisper could not complete processing. Close memory-heavy apps, verify the selected model is installed, and retry with a shorter recording. Keep the original WAV. | Whisper chưa xử lý xong. Đóng ứng dụng dùng nhiều RAM, kiểm tra mô hình đã cài rồi thử bản ghi ngắn hơn. Giữ WAV gốc. |


## Browser process recovery / Phục hồi tiến trình trình duyệt

Recoverable GPU, utility and subframe events retain the verified controller. A nonresponding page can be waited on or reloaded. This does not override a real WGC, display or privacy failure.

1. If a page stops, read its **QS-B** code and open **Diagnostics**. The current tabs and addresses stay in the session.
2. For **QS-B001** or **QS-B002**, select **Reload tab**. A browser-process exit may need a short wait before retry.
3. QuietShare recreates the required controller/environment and verifies its mounted rendered surface before showing the page.
4. If private content is locked, read the separate **QS-P** record, repair that condition and run F9. If verification still fails, keep content locked and restart QuietShare.
5. Start projection explicitly with F10 only after protection succeeds. Inspect the actual receiver again.

Web process events and projection failures are separate records. Compare their UTC times. An event does not by itself prove that a particular website caused the protection failure. Authenticated Threads and physical HDMI/projector operation require checking on the affected setup.

Sự kiện GPU, tiện ích hoặc khung con có thể phục hồi sẽ giữ controller đã xác minh. Với trang chưa phản hồi, có thể đợi hoặc tải lại. Quy tắc này không bỏ qua lỗi WGC, màn hình hoặc bảo vệ thực tế.

1. Khi trang dừng, đọc mã **QS-B** và mở **Chẩn đoán**. Tab và địa chỉ hiện tại vẫn giữ trong phiên.
2. Với **QS-B001** hoặc **QS-B002**, chọn **Tải lại tab**. Tiến trình web thoát có thể cần đợi ít giây rồi thử lại.
3. QuietShare tạo lại controller/môi trường cần thiết và kiểm tra vùng render đang gắn vào cửa sổ trước khi hiện trang.
4. Nếu nội dung riêng bị khóa, đọc bản ghi **QS-P** riêng, khắc phục nguyên nhân rồi nhấn F9. Nếu vẫn không xác minh được, giữ khóa và mở lại QuietShare.
5. Chỉ chủ động nhấn F10 sau khi bảo vệ thành công. Kiểm tra lại đúng đầu nhận.

Sự kiện tiến trình web và lỗi chiếu là hai bản ghi riêng. So thời gian UTC trước khi kết luận nguyên nhân. Một sự kiện không tự chứng minh website cụ thể gây lỗi bảo vệ. Threads đã đăng nhập và HDMI/máy chiếu cần được thử trên đúng thiết bị gặp lỗi.

| Code / Mã | English | Tiếng Việt |
|---|---|---|
| `QS-B001` | Browser process exited. Wait for exit, then Reload tab. Tabs stay in this session. | Tiến trình trình duyệt đã thoát. Đợi thoát xong rồi Tải lại tab. Tab vẫn giữ trong phiên. |
| `QS-B002` | Main renderer exited. Reload tab to recreate and verify it. | Tiến trình render chính đã thoát. Tải lại tab để tạo và xác minh lại. |
| `QS-B003` | Renderer is unresponsive. Wait or reload the page. Close an unused heavy tab if needed. | Trang chưa phản hồi. Đợi hoặc tải lại. Có thể đóng tab nặng không dùng. |
| `QS-B004` | Subframe renderer exited. The controller is retained. Reload if a frame remains blank. | Render khung con thoát. Controller được giữ. Tải lại nếu khung vẫn trắng. |
| `QS-B005` | Utility process exited. WebView2 can recover. Reload if page features remain broken. | Tiến trình tiện ích thoát. WebView2 có thể phục hồi. Tải lại nếu chức năng trang vẫn lỗi. |
| `QS-B006` | Sandbox helper exited. Wait for recovery, then reload if needed. | Tiến trình hỗ trợ sandbox thoát. Đợi phục hồi rồi tải lại nếu cần. |
| `QS-B007` | GPU process exited. WebView2 can recreate it. If repeated, update graphics drivers and the runtime. | Tiến trình GPU thoát. WebView2 có thể tạo lại. Nếu lặp lại, cập nhật driver đồ họa và runtime. |
| `QS-B008` | Plugin process exited. Reload the affected page. Some legacy page features may be unsupported. | Tiến trình plugin thoát. Tải lại trang. Một số chức năng cũ có thể không được hỗ trợ. |
| `QS-B009` | Plugin broker exited. Reload the affected page and update WebView2 if repeated. | Tiến trình môi giới plugin thoát. Tải lại trang và cập nhật WebView2 nếu lặp lại. |
| `QS-B010` | Another WebView2 process exited. Inspect the page and reload if needed. | Tiến trình WebView2 khác thoát. Kiểm tra trang và tải lại nếu cần. |
| `QS-B099` | Unrecognized process event. Reload to recreate and verify. Preserve the numeric reason/exit code. | Sự kiện tiến trình chưa nhận dạng. Tải lại để tạo và xác minh. Giữ mã reason/exit dạng số. |

## Usage profile recovery / Phục hồi hồ sơ sử dụng

- **QS-W009:** saved profiles unreadable. Close the app, back up workspace-profiles.dpapi, restore under the same Windows account or rename it to start fresh.
- **QS-W010:** profile/color save failed. Check local write access and free space, preserve the existing file and retry.

- **QS-W009:** không đọc được hồ sơ. Đóng ứng dụng, sao lưu workspace-profiles.dpapi, khôi phục bằng cùng tài khoản Windows hoặc đổi tên để tạo mới.
- **QS-W010:** không lưu được hồ sơ/màu. Kiểm tra quyền ghi và dung lượng, giữ file cũ rồi thử lại.

## Export error history / Xuất lịch sử lỗi

Open **Settings > Projection or Browser > Last-stop diagnostics > Export error history** (JSON). The newest 100 mapped events survive restarts. Default output is **Downloads / QuietShare-Diagnostics**. Choose another existing local folder in **Settings > Data**. Send the JSON with reproduction steps. Nothing is uploaded automatically.

Mở **Cài đặt > Trình chiếu hoặc Trình duyệt > Chẩn đoán lần dừng > Xuất lịch sử lỗi** (JSON). 100 sự kiện có mã gần nhất được giữ qua lần mở lại. Mặc định lưu ở **Downloads / QuietShare-Diagnostics**. Đổi thư mục cục bộ có sẵn tại **Cài đặt > Dữ liệu**. Gửi JSON kèm các bước gây lỗi. Ứng dụng không tự tải báo cáo lên mạng.

The report includes UTC, version, fixed codes/numeric results, Windows build, display/protection flags and a current memory/tab snapshot. It excludes content, paths, URLs, titles and recordings. It starts in v0.23.0 and cannot recreate past or unwritten events.

Báo cáo có UTC, phiên bản, mã cố định/kết quả dạng số, bản Windows, cờ màn hình/bảo vệ và bộ nhớ/số tab hiện tại. Không chứa nội dung, đường dẫn, URL, tên tab hoặc bản ghi âm. Bắt đầu từ v0.23.0, không dựng lại sự kiện cũ hoặc chưa ghi được.

| Code / Mã | English - recovery | Tiếng Việt - cách xử lý |
|---|---|---|
| `QS-D004` | PDF rendering reached its memory/time budget. Reduce zoom or close another PDF, then retry. A complex page may need a simpler copy. | Dựng PDF chạm giới hạn RAM/thời gian. Giảm zoom hoặc đóng PDF khác rồi thử lại. Trang quá phức tạp có thể cần bản đơn giản hơn. |
| `QS-D005` | PDF page or bitmap could not be loaded. Reopen the file and check it in another reader. Keep the original. | Không tải được trang PDF hoặc bitmap. Mở lại file và kiểm tra bằng trình đọc khác. Giữ file gốc. |
| `QS-F001` | Local file/path/format was rejected. Check chapter 4 limits and open a supported local copy. | File, đường dẫn hoặc định dạng bị từ chối. Kiểm tra giới hạn ở chương 4 rồi mở bản cục bộ được hỗ trợ. |
| `QS-F002` | File could not be saved. Preserve your draft, check write access, free space and external changes before retrying. | Không lưu được file. Giữ bản nháp, kiểm tra quyền ghi, dung lượng và thay đổi từ ứng dụng khác trước khi thử lại. |
| `QS-F003` | The viewer could not open the file. Check that the file is valid, supported and not password-protected. | Trình xem không mở được file. Kiểm tra file hợp lệ, được hỗ trợ và không có mật khẩu. |
| `QS-J001` | History is unreadable. Close QuietShare and back up error-history.dat. Restore a good copy or rename it before reopening to start a new journal. The app preserves the bad file. | Không đọc được lịch sử. Đóng QuietShare, sao lưu error-history.dat. Khôi phục bản tốt hoặc đổi tên file trước khi mở lại để tạo nhật ký mới. Ứng dụng giữ file lỗi. |
| `QS-J002` | Some events were not saved. Check local data-folder write access and free space. Missing events cannot be recreated. | Có sự kiện chưa lưu được. Kiểm tra quyền ghi và dung lượng thư mục dữ liệu cục bộ. Không dựng lại được sự kiện đã mất. |
| `QS-J003` | JSON export or its folder preference failed. Select an existing writable local folder, check free space and retry. | Xuất JSON hoặc lưu thư mục thất bại. Chọn thư mục cục bộ có sẵn, có quyền ghi, kiểm tra dung lượng rồi thử lại. |
