# Dependencies and licenses / Thư viện và giấy phép

QuietShare bundles native components for icons, code editing, PDF rendering and optional browser integration. Exact versions, source references and checksums are recorded in dependency lock files in the source repository.

QuietShare kèm thành phần native cho icon, sửa code, dựng PDF và trình duyệt tùy chọn. Phiên bản, nguồn và checksum chính xác nằm trong các tệp khóa thư viện của kho mã nguồn.

| Component | Purpose / Mục đích |
|---|---|
| Microsoft Codicons | Interface icons / Icon giao diện |
| Scintilla and Lexilla | Code editor and syntax coloring / Trình sửa code và màu cú pháp |
| PDFium | PDF viewing and text access / Xem PDF và đọc lớp chữ |
| Microsoft WebView2 Loader | Optional embedded browser / Trình duyệt nhúng tùy chọn |

The shared [WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) is a separate prerequisite for browsing. Offline file readers do not require it.

[WebView2 Evergreen Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/) dùng chung được cài riêng cho trình duyệt. Trình đọc file offline không cần thành phần này.

- Read [THIRD-PARTY.md](../THIRD-PARTY.md) and the bundled dependency notices for the original license texts.
- Legal notices retain their original language and wording.
- This bilingual guide does not replace or modify those notices.

- Đọc [THIRD-PARTY.md](../THIRD-PARTY.md) và các thông báo thư viện đi kèm để xem giấy phép gốc.
- Thông báo pháp lý giữ nguyên ngôn ngữ và nội dung gốc.
- Hướng dẫn song ngữ này không thay thế hay sửa đổi những thông báo đó.

The original [FreeType notice](../dependencies/licenses/PDFium-third-party/freetype.txt) accompanies the aggregate notice to preserve its original copyright text and encoding.

[Thông báo FreeType gốc](../dependencies/licenses/PDFium-third-party/freetype.txt) đi kèm thông báo tổng hợp để giữ nguyên nội dung bản quyền và mã hóa gốc.

## Optional local speech / Nhận dạng cục bộ tùy chọn

- [whisper.cpp](https://github.com/ggml-org/whisper.cpp) v1.9.1, commit `f049fff95a089aa9969deb009cdd4892b3e74916`, CPU-only static helper, MIT license.
- [Whisper models](https://huggingface.co/ggerganov/whisper.cpp), pinned conversion revision `5359861c739e955e79d9a303bcbc70fb988958b1`. Base is 147,951,465 bytes. Small is 487,601,967 bytes. The default is multilingual Small. OpenAI Whisper model license is included in the add-on.
- These are downloaded only after an explicit install action. All three original license notices accompany the separate speech ZIP. Core portable packages do not include the runtime or models.
- whisper.cpp và mô hình Whisper là thành phần tùy chọn. Small đa ngôn ngữ được chọn mặc định để ưu tiên độ chính xác. Chỉ tải khi chủ động cài. Giấy phép gốc nằm trong ZIP nhận dạng riêng. Gói portable chính không kèm bộ chạy hoặc mô hình.
