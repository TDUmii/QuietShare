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
