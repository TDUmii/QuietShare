# Build from source / Biên dịch từ mã nguồn

QuietShare is a native C++17 Windows x64 application. Development requires Windows, Python 3 and a compatible MinGW-w64 toolchain; the existing build uses GCC 13.1.0. Dependency bootstrap needs internet and checks pinned hashes. End users can use the [prebuilt releases](https://github.com/TDUmii/QuietShare/releases/latest).

QuietShare là ứng dụng Windows x64 viết bằng C++17 native. Phát triển cần Windows, Python 3 và MinGW-w64 tương thích; quy trình hiện tại dùng GCC 13.1.0. Chuẩn bị thư viện cần mạng và kiểm tra hash đã ghim. Người dùng có thể tải [bản dựng sẵn](https://github.com/TDUmii/QuietShare/releases/latest).

## Build and validate / Dựng và kiểm tra

Use a Git checkout with release tags. From the repository root in PowerShell, set the compiler folder to your installation, then run:

Dùng bản Git checkout có các tag phát hành. Tại thư mục gốc dự án trong PowerShell, sửa đường dẫn compiler theo máy rồi chạy:

```powershell
.\build.ps1 -CompilerRoot 'C:\path\to\mingw64\bin'
.\tests\run-all.ps1 -CompilerRoot 'C:\path\to\mingw64\bin'
python -B tests\docs_regression.py
.\package.ps1
python -B tests\portable_regression.py
```

The build produces the application, installer and a separate QA executable. GUI checks run sequentially against owned processes and isolated profiles. Packaging requires current validation evidence and creates the portable ZIP. Test reports and profiles remain in ignored local build/artifact folders, outside release payloads.

Quy trình tạo ứng dụng, bộ cài và EXE QA riêng. Kiểm tra GUI chạy tuần tự trên tiến trình tự tạo và hồ sơ tách biệt. Đóng gói yêu cầu bằng chứng kiểm tra còn khớp và tạo ZIP portable. Báo cáo/hồ sơ kiểm thử giữ trong thư mục build/artifact cục bộ được bỏ qua, không đưa vào bản phát hành.

## Scope of verification / Phạm vi kiểm chứng

Automated WGC and renderer checks do not establish what a physical receiver or meeting app displays. Validate the actual audience output separately before presenting. Do not describe synthetic monitor checks as physical Duplicate/Extend validation.

Kiểm tra WGC và renderer tự động không xác lập hình ảnh ở đầu nhận vật lý hoặc phần mềm họp. Cần kiểm tra riêng đầu ra thực tế trước buổi trình bày. Không coi kiểm tra màn hình mô phỏng là xác minh Duplicate/Extend vật lý.

Use concise English commit messages when contributing. Keep runtime files and current guides in the portable archive; installer, development records and validation data do not belong in it.

Dùng commit message ngắn gọn bằng tiếng Anh khi đóng góp. Gói portable chỉ giữ thành phần chạy và hướng dẫn hiện tại; bộ cài, tư liệu phát triển và dữ liệu kiểm thử không nằm trong gói này.
