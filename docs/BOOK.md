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
| `QS-R004` | Selected language recognizer is missing / Thiếu bộ nhận dạng ngôn ngữ đã chọn | Install the matching Windows speech language pack, then retry. / Cài gói ngôn ngữ giọng nói Windows tương ứng rồi thử lại. |
| `QS-R005` | Windows could not recognize the audio / Windows không nhận dạng được âm thanh | Check that the meeting audio was playing and choose the correct language. / Kiểm tra âm thanh cuộc họp đang phát và chọn đúng ngôn ngữ. |
| `QS-R006` | Transcription was stopped / Đã dừng chép lời | Start a new recording if a transcript is still needed. / Ghi lại nếu vẫn cần transcript. |
| `QS-R007` | Transcript could not be saved / Không lưu được transcript | Check local storage permission and free space. / Kiểm tra quyền ghi và dung lượng cục bộ. |

The recorder and transcriber use the Windows playback mix only. They do not open a microphone or upload audio.

Bộ ghi và bộ chép lời chỉ dùng âm thanh phát ra từ Windows. Chúng không mở micro và không tải âm thanh lên mạng.

[Documentation index](README.md) | [Mục lục tài liệu](README.md)
