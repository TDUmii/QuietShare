# Sharing handbook and error codes / Sổ tay chia sẻ và mã lỗi

Use this book before a presentation and when projection fails. Codes describe the condition QuietShare detected; they do not prove that a cable, projector or laptop is defective. This catalog covers projection and capture verification. Other file, browser and installation problems are covered in [Troubleshooting](TROUBLESHOOTING.md).

Dùng cuốn book này trước buổi trình chiếu và khi gặp sự cố. Mã lỗi cho biết điều kiện QuietShare phát hiện, không tự chứng minh dây, máy chiếu hay laptop bị hỏng. Danh mục này dành cho trình chiếu và xác minh capture; lỗi file, trình duyệt và cài đặt nằm trong [Xử lý sự cố](TROUBLESHOOTING.md).

## 1. Play does nothing and content disappears / Bấm Play không hiện gì và nội dung bị ẩn

1. Read **Settings > Projection** after the failed start. From v0.14.4, QuietShare opens this page automatically and shows a code such as `QS-P004 [different-adapters]`.
2. Record the code, reason and application version before checking again. Failed verification intentionally hides private content and removes projection output.
3. Look up the exact code below. A clickable Play button means the action can be attempted; it does not mean the capture path has passed verification.
4. Correct the condition, select source/destination again, press **F9** to verify, then **F10** to start projection. Stop if verification fails.
5. If the page is unavailable, open `%LOCALAPPDATA%\QuietShare\projection-stop.json` in a text editor. It is a local last-failure record, overwritten by a later failure. Older versions may record only failures during a running projection.

1. Sau khi khởi động chiếu thất bại, đọc **Cài đặt > Trình chiếu**. Từ v0.14.4, ứng dụng tự mở trang này và hiện mã, ví dụ `QS-P004 [different-adapters]`.
2. Ghi lại mã, lý do và phiên bản trước khi kiểm tra lại. Khi không xác minh được, QuietShare chủ động ẩn nội dung riêng và gỡ đầu ra trình chiếu.
3. Tra đúng mã bên dưới. Nút Play bấm được chỉ có nghĩa có thể thử thao tác, chưa chứng minh đường capture đã đạt kiểm tra.
4. Sửa điều kiện lỗi, chọn lại nguồn/đích, nhấn **F9** để xác minh rồi **F10** để chiếu. Dừng nếu xác minh thất bại.
5. Nếu không xem được trang trạng thái, mở `%LOCALAPPDATA%\QuietShare\projection-stop.json` bằng trình soạn thảo. File lưu lỗi gần nhất ở máy này và bị ghi đè khi có lỗi sau. Bản cũ có thể chỉ lưu lỗi khi đang chiếu.

**Two different Play controls:** F10/sidebar projection Play starts Smart Mirror. The reader's Play starts text scrolling and does not send content to another display. **F8** hides private content and stops the mirror; it does not stop a meeting application's screen share.

**Có hai thao tác Play:** F10/nút Play trình chiếu ở sidebar bật Smart Mirror. Play của trình đọc chỉ cuộn chữ, không gửi nội dung sang màn khác. **F8** ẩn nội dung riêng và dừng mirror; không dừng chia sẻ màn hình trong ứng dụng họp.

## 2. Common preparation / Chuẩn bị chung

1. Save work. Start with a harmless public slide or sample text, without private notes.
2. In Windows display settings, identify the displays. Source is the display containing the entire QuietShare control window and public material behind it. Destination is the separate audience display.
3. For Smart Mirror, use two distinct **Extend** displays on the same GPU. Cross-adapter output is unsupported in this version.
4. Keep the control window entirely on the source. Check Windows capture exclusion, then F9 verification. Windows accepting exclusion, WGC excluding a synthetic marker, and a person checking the receiver are three separate observations.
5. Check the actual projector/TV/meeting receiver using harmless material before opening private notes. Repeat after changing capture software, display mode, cable or source/destination.

1. Lưu công việc. Mở slide hoặc chữ công khai thử nghiệm, chưa dùng lời nhắc riêng.
2. Trong cài đặt màn hình Windows, nhận diện từng màn. Nguồn là màn chứa trọn cửa sổ điều khiển QuietShare và tài liệu công khai phía sau. Đích là màn riêng cho người xem.
3. Smart Mirror cần hai màn **Extend** khác nhau, cùng GPU. Phiên bản này chưa hỗ trợ sao chép giữa hai adapter.
4. Để trọn cửa sổ điều khiển trên nguồn. Xem trạng thái Windows nhận loại trừ rồi kiểm tra F9. Windows nhận yêu cầu, WGC loại marker giả và người dùng kiểm tra đầu nhận là ba bằng chứng khác nhau.
5. Kiểm tra máy chiếu/TV/đầu nhận cuộc họp bằng nội dung không riêng tư trước khi mở lời nhắc. Kiểm tra lại khi đổi phần mềm capture, chế độ màn hình, dây hoặc nguồn/đích.

## 3. HDMI projector or wired monitor / Máy chiếu HDMI hoặc màn hình có dây

1. Connect the cable and select the matching input on the projector. First confirm Windows detects a second display. If Windows cannot detect it, troubleshoot input/cable/port detection before starting QuietShare.
2. Press **Win+P > Extend**. Identify the laptop and projector in Windows display settings; do not assume their display numbers.
3. Keep QuietShare on the laptop. Select laptop as source and projector as destination in QuietShare's Projection settings.
4. Put the public slide/browser on the source, behind QuietShare. Run F9, then F10. Observe the projector using sample material.
5. If Play fails and content hides, read the displayed code. `QS-P003` means adapter identification failed; `QS-P004` means the identified GPUs differ. Neither code proves a bad HDMI cable. Follow its specific recovery steps below.
6. To end, stop projection and close private material before switching Windows display mode or unplugging.

1. Cắm dây và chọn đúng đầu vào trên máy chiếu. Trước tiên xác nhận Windows nhận màn thứ hai. Nếu Windows chưa nhận, xử lý đầu vào/dây/cổng trước khi bật QuietShare.
2. Nhấn **Win+P > Extend**. Nhận diện laptop và máy chiếu trong cài đặt Windows, không mặc định số màn.
3. Giữ QuietShare trên laptop. Trong Trình chiếu, chọn laptop làm nguồn và máy chiếu làm đích.
4. Đặt slide/trình duyệt công khai trên nguồn, phía sau QuietShare. F9 rồi F10. Quan sát máy chiếu với nội dung thử.
5. Nếu Play thất bại và chữ bị ẩn, đọc mã. `QS-P003` là không xác định được adapter; `QS-P004` là hai GPU đã nhận diện khác nhau. Hai mã này không chứng minh dây HDMI hỏng. Làm theo mục sửa riêng bên dưới.
6. Khi kết thúc, dừng chiếu và đóng nội dung riêng trước khi đổi chế độ màn hoặc rút dây.

## 4. Wireless TV / TV không dây

1. Connect the TV through the Windows wireless display workflow. Confirm it appears as a separate display in Windows.
2. Select **Extend**, identify the laptop/TV, then choose source and destination in QuietShare.
3. Run F9 and F10 with harmless material. Inspect the TV directly. A wireless connection that Windows exposes through another adapter can be unsupported; use the code rather than assuming compatibility.
4. After reconnection, reselect displays and verify again. Projection never resumes automatically after a safety stop.

1. Kết nối TV qua chức năng màn hình không dây của Windows. Xác nhận TV xuất hiện như màn riêng trong Windows.
2. Chọn **Extend**, nhận diện laptop/TV rồi chọn nguồn và đích trong QuietShare.
3. F9 rồi F10 với tài liệu thử. Quan sát TV trực tiếp. Kết nối không dây được Windows đưa qua adapter khác có thể không được hỗ trợ; tra mã thay vì mặc định tương thích.
4. Sau khi kết nối lại, chọn lại màn và xác minh lại. Chiếu không tự tiếp tục sau khi cơ chế bảo vệ dừng phiên.

## 5. Zoom, Meet or Teams with one display / Zoom, Meet hoặc Teams với một màn hình

1. Open the public document in a separate application window. In the meeting application, select that public window for sharing.
2. Keep QuietShare as the private reader. Smart Mirror is optional here and cannot start without two displays; single-display WGC verification is supported.
3. Run F9 before viewing private notes. Confirm the actual meeting receiver on another device with sample content. A successful local WGC test does not certify the meeting application's capture method.
4. Start the meeting's share through its own controls. Do not select the private QuietShare window. If private material appears at the receiver, stop sharing immediately.
5. End sharing in the meeting application. QuietShare's Stop or F8 does not end the meeting's share.

1. Mở tài liệu công khai trong cửa sổ ứng dụng riêng. Trong ứng dụng họp, chọn chia sẻ cửa sổ công khai đó.
2. Dùng QuietShare để đọc riêng. Không bắt buộc Smart Mirror trong trường hợp này; mirror không chạy nếu thiếu hai màn, nhưng xác minh WGC vẫn hỗ trợ một màn.
3. F9 trước khi xem lời nhắc. Dùng tài liệu thử và thiết bị khác để xem đầu nhận cuộc họp. WGC đạt trên máy không chứng nhận phương pháp capture của ứng dụng họp.
4. Bật chia sẻ bằng điều khiển của ứng dụng họp. Không chọn cửa sổ riêng QuietShare. Nếu đầu nhận xuất hiện nội dung riêng, dừng chia sẻ ngay.
5. Kết thúc chia sẻ trong ứng dụng họp. Stop hoặc F8 của QuietShare không tắt luồng chia sẻ cuộc họp.

## 6. Meeting plus Extend, or OBS / Họp kết hợp Extend hoặc OBS

1. For a meeting plus projector, complete the HDMI/wireless workflow first. Prefer sharing the original public application window with online viewers. The Smart Mirror output window also requests capture exclusion; capturing the destination display can omit that output or show a blank area.
2. Select that exact original public surface in the meeting application. If you choose a whole display instead, test the actual result first. Check both the physical receiver and the online receiver using sample content.
3. In OBS, prefer the original public application with Window Capture. If using Display Capture, account for the Smart Mirror output exclusion described above. Do not select QuietShare. Make a short local sample recording and inspect it before recording private notes.
4. Repeat the receiver/recording check after changing the capture backend or source. Local WGC verification does not certify OBS or a meeting application's different capture backend.
5. Stop sharing/recording in those applications as well as stopping QuietShare projection.

1. Nếu vừa họp vừa chiếu, hoàn thành quy trình HDMI/TV trước. Ưu tiên chia sẻ cửa sổ ứng dụng công khai gốc cho người xem trực tuyến. Cửa sổ đầu ra Smart Mirror cũng yêu cầu loại trừ capture; capture màn đích có thể bỏ qua đầu ra này hoặc hiện vùng trống.
2. Chọn đúng bề mặt công khai gốc đó trong ứng dụng họp. Nếu chọn chia sẻ cả màn hình, phải kiểm tra kết quả thực tế trước. Kiểm tra cả máy chiếu/TV lẫn đầu nhận trực tuyến bằng nội dung thử.
3. Với OBS, ưu tiên Window Capture cho ứng dụng công khai gốc. Nếu dùng Display Capture, lưu ý việc loại trừ cửa sổ đầu ra Smart Mirror như trên. Không chọn QuietShare. Ghi thử một đoạn ngắn không chứa thông tin riêng rồi xem lại trước khi dùng lời nhắc.
4. Kiểm tra lại đầu nhận/bản ghi sau khi đổi backend capture hoặc nguồn. WGC cục bộ không chứng nhận backend khác của OBS hay ứng dụng họp.
5. Dừng chia sẻ/ghi hình trong các ứng dụng đó, đồng thời dừng trình chiếu QuietShare.

## 7. Duplicate, reconnect and recovery / Duplicate, kết nối lại và phục hồi

**Duplicate is unsupported for private presenting.** QuietShare locks private content when cloning is detected. To show identical public content, close private material and use the public application. For private notes, return to Extend.

**Duplicate không hỗ trợ thuyết trình có nội dung riêng.** QuietShare khóa khi phát hiện nhân đôi màn. Muốn hiện cùng tài liệu công khai, đóng nội dung riêng và dùng ứng dụng công khai. Muốn dùng lời nhắc riêng, quay lại Extend.

After Duplicate > Extend, cable reconnect, sleep or GPU reset: (1) restore Extend in Windows; (2) wait until the displays appear; (3) move QuietShare entirely onto the source; (4) reselect source/destination; (5) F9; (6) F10; (7) inspect the receiver. If a step fails, record its code and follow the catalog. Do not repeatedly press Play without correcting the condition.

Sau Duplicate > Extend, cắm lại dây, ngủ máy hoặc reset GPU: (1) khôi phục Extend trong Windows; (2) chờ nhận đủ màn; (3) đưa trọn QuietShare lên nguồn; (4) chọn lại nguồn/đích; (5) F9; (6) F10; (7) xem đầu nhận. Nếu bước nào lỗi, ghi mã rồi tra danh mục. Không bấm Play liên tục khi điều kiện lỗi chưa thay đổi.

## 8. Error catalog / Danh mục mã lỗi

Each row gives the detected condition and ordered recovery steps. Codes stay stable; the internal reason is also shown for technical diagnosis. Recheck the receiver after recovery. If the condition persists, keep private content closed and report the code/version instead of disabling protection.

Mỗi dòng gồm điều kiện phát hiện và các bước sửa theo thứ tự. Mã được giữ ổn định; ứng dụng vẫn hiện lý do nội bộ để chẩn đoán. Kiểm tra lại đầu nhận sau khi phục hồi. Nếu vẫn lỗi, giữ nội dung riêng đóng và gửi mã/phiên bản, không tắt bảo vệ.

| Code / Mã | Condition and recovery / Điều kiện và cách sửa |
|---|---|
| `QS-P001` `need-two-monitors` | Fewer than two displays: 1. Connect the receiver. 2. Confirm Windows detects two displays. 3. Select Extend and reselect the pair. / Chưa đủ hai màn: 1. Kết nối đầu nhận. 2. Kiểm tra Windows nhận hai màn. 3. Chọn Extend rồi chọn lại cặp màn. |
| `QS-P002` `same-monitor` | Same source and destination: 1. Identify the displays. 2. Select different source/destination. 3. F9 then F10. / Nguồn trùng đích: 1. Nhận diện màn. 2. Chọn hai màn khác nhau. 3. F9 rồi F10. |
| `QS-P003` `adapter-unknown` | GPU identification failed: 1. Reconnect the display and reselect it. 2. Check the laptop/display adapter driver. 3. Retry after restarting the app; report the code if identification still fails. / Không nhận diện GPU: 1. Kết nối và chọn lại màn. 2. Kiểm tra driver đồ họa/màn hình của laptop. 3. Mở lại ứng dụng rồi thử; gửi mã nếu vẫn không nhận diện được. |
| `QS-P004` `different-adapters` | Identified GPUs differ; cross-adapter copy is unsupported: 1. Check which GPU drives each output. 2. Use a supported port/configuration where both displays belong to one adapter, if available. 3. Otherwise use a public application directly without private notes. A Windows graphics preference does not guarantee changing the physical port's GPU. / Hai GPU khác nhau, chưa hỗ trợ sao chép giữa adapter: 1. Kiểm tra GPU điều khiển từng đầu ra. 2. Dùng cổng/cấu hình có hai màn cùng adapter nếu máy hỗ trợ. 3. Nếu không có, dùng ứng dụng công khai trực tiếp, không mở lời nhắc riêng. Chọn ưu tiên GPU trong Windows không bảo đảm đổi GPU nối với cổng vật lý. |
| `QS-P005` `control-location` | Private window outside source: 1. Move the entire QuietShare window onto the source. 2. Reduce/restore the window if it spans displays. 3. Verify again. / Cửa sổ riêng ngoài nguồn: 1. Đưa trọn QuietShare lên nguồn. 2. Thu nhỏ/khôi phục nếu vắt qua hai màn. 3. Xác minh lại. |
| `QS-P006` `windows-unsupported` | Windows build below 19041: 1. Check Windows version. 2. Upgrade to supported Windows. 3. Reopen and verify. / Windows dưới build 19041: 1. Xem phiên bản Windows. 2. Nâng cấp Windows được hỗ trợ. 3. Mở lại và xác minh. |
| `QS-P007` `dwm-unavailable` | Desktop composition unavailable: 1. Return to a normal Windows desktop session. 2. Check graphics driver/desktop composition. 3. Restart Windows if necessary and retry. / DWM không khả dụng: 1. Về phiên desktop Windows bình thường. 2. Kiểm tra driver và bộ dựng desktop. 3. Khởi động Windows lại nếu cần rồi thử. |
| `QS-P008` `control-affinity` | Private window exclusion not accepted/retained: 1. Close and reopen QuietShare. 2. Check supported Windows and graphics driver. 3. Retry F9; do not expose notes while locked. / Không nhận/giữ loại trừ cửa sổ riêng: 1. Mở lại QuietShare. 2. Kiểm tra Windows được hỗ trợ và driver. 3. F9 lại; không mở nội dung riêng khi đang khóa. |
| `QS-P009` `private-popup-affinity` | A private popup lacks exclusion: 1. Close private dialogs/menus. 2. Verify again. 3. If reproducible, report which action opened the popup without its private contents. / Popup riêng thiếu loại trừ: 1. Đóng hộp thoại/menu riêng. 2. Xác minh lại. 3. Nếu lặp lại, gửi thao tác mở popup, không gửi nội dung riêng. |
| `QS-P010` `topology-unavailable` | Windows display topology cannot be read: 1. Wait for connection to finish. 2. Confirm Extend in Windows and reselect displays. 3. Check drivers/restart if persistent. / Không đọc được cấu hình màn: 1. Chờ kết nối xong. 2. Kiểm tra Extend và chọn lại màn. 3. Kiểm tra driver/khởi động lại nếu còn lỗi. |
| `QS-P011` `display-cloned` | Duplicate detected: 1. Close private content. 2. Win+P > Extend. 3. Reselect displays, F9, F10. / Phát hiện Duplicate: 1. Đóng nội dung riêng. 2. Win+P > Extend. 3. Chọn lại màn, F9, F10. |
| `QS-P012` `capture-frame` | Capture initialization/frame or capture setup failed: 1. Restore the source desktop after sleep/disconnect. 2. Reopen and verify. 3. Check graphics drivers and report HRESULT if persistent. / Lỗi khởi tạo/frame hoặc thiết lập capture: 1. Khôi phục desktop nguồn sau ngủ/ngắt kết nối. 2. Mở lại và xác minh. 3. Kiểm tra driver, gửi HRESULT nếu còn lỗi. |
| `QS-P013` `canary-failed` | Synthetic marker did not prove exclusion: 1. Close overlapping private dialogs. 2. Verify again on the selected source. 3. Keep notes locked if it fails; report version/code. / Marker giả không chứng minh được loại trừ: 1. Đóng hộp thoại riêng đang phủ. 2. Xác minh lại đúng nguồn. 3. Giữ khóa nếu không đạt, gửi phiên bản/mã. |
| `QS-P014` `render-frame` | D3D11 audience output failed: 1. Confirm destination is connected. 2. Reselect source/destination and retry. 3. Restart app/check graphics driver if persistent. / Không dựng được đầu ra D3D11: 1. Kiểm tra đích còn kết nối. 2. Chọn lại nguồn/đích rồi thử. 3. Mở lại ứng dụng/kiểm tra driver nếu còn lỗi. |
| `QS-P015` `display-topology` | Selected capture/display configuration changed: 1. Restore Extend. 2. Reselect the current pair. 3. F9 then F10. / Cấu hình màn/capture đã đổi: 1. Khôi phục Extend. 2. Chọn lại cặp màn hiện tại. 3. F9 rồi F10. |
| `QS-P016` `device-removed` | Capture GPU device unavailable/reset: 1. Save work and restore displays. 2. Reopen QuietShare. 3. Check driver and include HRESULT in the report if repeated. / Thiết bị GPU capture mất/reset: 1. Lưu việc và khôi phục màn. 2. Mở lại QuietShare. 3. Kiểm tra driver, kèm HRESULT nếu lặp lại. |
| `QS-P017` `projector-affinity` | Public output exclusion invariant lost: 1. Stop projection. 2. Restart QuietShare and verify. 3. Report if repeated; do not bypass the guard. / Đầu ra mất điều kiện loại trừ: 1. Dừng chiếu. 2. Mở lại QuietShare và xác minh. 3. Gửi lỗi nếu lặp lại, không bỏ kiểm tra. |
| `QS-P018` `projector-closed` | Audience output window closed: 1. Confirm closure was intended. 2. Reselect displays if needed. 3. Verify/start explicitly to reopen it. / Cửa sổ đầu ra đã đóng: 1. Kiểm tra có chủ động đóng không. 2. Chọn lại màn nếu cần. 3. Xác minh/bật chiếu lại rõ ràng. |
| `QS-P019` `projector-escape` | Escape stopped audience output: 1. Confirm the key action. 2. Return to the control window. 3. Verify/start when ready. / Escape dừng đầu ra: 1. Kiểm tra thao tác phím. 2. Về cửa sổ điều khiển. 3. Xác minh/bật lại khi sẵn sàng. |
| `QS-P020` `system-suspend` | Sleep/suspend interrupted projection: 1. Wake the machine. 2. Wait for displays and restore Extend. 3. Reselect, verify and start. / Máy ngủ/tạm ngưng: 1. Đánh thức máy. 2. Chờ nhận màn và khôi phục Extend. 3. Chọn lại, xác minh rồi bật. |
| `QS-P021` `winrt-unavailable` | WinRT capture support unavailable: 1. Check supported Windows. 2. Restart the app/Windows. 3. Report if it persists on supported Windows. / WinRT không khả dụng: 1. Kiểm tra Windows được hỗ trợ. 2. Mở lại ứng dụng/Windows. 3. Gửi lỗi nếu vẫn xảy ra trên Windows phù hợp. |
| `QS-P022` `privacy-unsafe` | Privacy guard failed without a more specific reason: 1. Confirm Extend and Windows exclusion status. 2. Close dialogs and verify again. 3. Keep notes closed and report if repeated. / Bảo vệ không đạt nhưng chưa có lý do chi tiết hơn: 1. Kiểm tra Extend và trạng thái loại trừ Windows. 2. Đóng hộp thoại rồi xác minh. 3. Giữ nội dung riêng đóng và gửi lỗi nếu lặp lại. |
| `QS-P023` `environment-invalidated` | Capture environment invalidated: 1. Restore the intended display setup. 2. Reselect and verify. 3. Report the preceding action if repeated. / Môi trường capture bị hủy xác minh: 1. Khôi phục cấu hình cần dùng. 2. Chọn lại và xác minh. 3. Gửi thao tác ngay trước lỗi nếu lặp lại. |
| `QS-P024` `source-monitor-changed` | Source monitor disappeared or changed during window movement: 1. Restore the source. 2. Keep the entire window on it. 3. Reselect and verify/start. / Nguồn mất hoặc đổi khi di chuyển cửa sổ: 1. Khôi phục nguồn. 2. Giữ trọn cửa sổ trên nguồn. 3. Chọn lại, xác minh/bật lại. |
| `QS-P000` | No failure reason; unverified is a status, not a hardware diagnosis. Choose the source and run F9. / Chưa có lý do lỗi; chưa xác minh là trạng thái, không phải kết luận hỏng phần cứng. Chọn nguồn và F9. |
| `QS-P099` | Unknown reason fallback: preserve the internal reason and app version, stop using private notes and report it. / Mã dự phòng cho lý do chưa biết: giữ lý do nội bộ và phiên bản, dừng dùng lời nhắc riêng rồi gửi lỗi. |

## 9. Diagnostic record and support / Bản ghi chẩn đoán và hỗ trợ

The record contains `code`, `reason`, `version`, UTC time, `hresult`, `sessionMs`, `resourceMode` and `phase`. `phase` is `start` (starting output), `verification` (checking capture) or `running` (a running session stopped). `sessionMs` is zero before projection starts. `0x00000000` means no failing HRESULT was attached; it does not mean projection succeeded. No screenshots, private text, file paths, URLs or display identifiers are recorded. Diagnostic write failure never permits projection.

Bản ghi gồm `code`, `reason`, `version`, giờ UTC, `hresult`, `sessionMs`, `resourceMode` và `phase`. `phase` là `start` (khởi động đầu ra), `verification` (kiểm tra capture) hoặc `running` (phiên đang chạy bị dừng). `sessionMs` bằng không trước khi chiếu chạy. `0x00000000` nghĩa không đính kèm HRESULT thất bại, không có nghĩa chiếu thành công. Không ghi ảnh, chữ riêng, đường dẫn file, URL hay định danh màn hình. Lỗi ghi chẩn đoán không cho phép bỏ khóa.

When reporting, provide: app version, exact code/reason, phase/HRESULT if present, Windows version, connection type (HDMI/wireless/dock), Extend or Duplicate, which step failed, and whether the other screen is visible to Windows. Share only the small diagnostic record or a cropped status image; do not send private notes or browser content.

Khi báo lỗi, gửi: phiên bản app, mã/lý do chính xác, phase/HRESULT nếu có, phiên bản Windows, kiểu kết nối (HDMI/không dây/dock), Extend hay Duplicate, bước bị lỗi và Windows có nhận màn kia không. Chỉ gửi bản ghi nhỏ hoặc ảnh cắt vùng trạng thái; không gửi lời nhắc hay nội dung trình duyệt riêng.

New projection reasons must be assigned a stable code and documented here with recovery steps before release. Documentation checks enforce correspondence between the code mapping and this catalog. New non-projection errors should receive their own documented category when implemented.

Lý do lỗi trình chiếu mới phải có mã ổn định và hướng dẫn sửa tại đây trước khi phát hành. Kiểm thử tài liệu đối chiếu bảng mã trong chương trình với danh mục này. Lỗi ngoài trình chiếu mới cần có nhóm mã riêng được ghi lại khi hiện thực.
