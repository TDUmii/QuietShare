# Build from source / Biên dịch từ mã nguồn

QuietShare is a native C++17 Windows x64 application. End users can use the [prebuilt releases](https://github.com/TDUmii/QuietShare/releases/latest).

- Development requires Windows, Python 3 and a compatible MinGW-w64 toolchain.
- The existing build uses GCC 13.1.0.
- Dependency bootstrap needs internet and checks pinned hashes.

QuietShare là ứng dụng Windows x64 viết bằng C++17 native. Người dùng có thể tải [bản dựng sẵn](https://github.com/TDUmii/QuietShare/releases/latest).

- Phát triển cần Windows, Python 3 và MinGW-w64 tương thích.
- Quy trình hiện tại dùng GCC 13.1.0.
- Chuẩn bị thư viện cần mạng và kiểm tra hash đã ghim.

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

- The build produces the application, installer and a separate QA executable.
- GUI checks run sequentially against owned processes and isolated profiles.
- Packaging requires current validation evidence and creates the portable ZIP.
- Test reports and profiles remain in ignored local build/artifact folders, outside release payloads.

- Quy trình tạo ứng dụng, bộ cài và EXE QA riêng.
- Kiểm tra GUI chạy tuần tự trên tiến trình tự tạo và hồ sơ tách biệt.
- Đóng gói yêu cầu bằng chứng kiểm tra còn khớp và tạo ZIP portable.
- Báo cáo/hồ sơ kiểm thử giữ trong thư mục build/artifact cục bộ được bỏ qua, không đưa vào bản phát hành.

## Regenerate the PDF manuals / Tạo lại sách PDF

The English and Vietnamese manuals are maintained separately. PDF generation is an optional documentation task.

Hai quyển hướng dẫn tiếng Anh và tiếng Việt có nguồn riêng. Tạo PDF là bước tùy chọn khi biên soạn tài liệu.

1. Update [USER_MANUAL_EN.md](USER_MANUAL_EN.md) and [USER_MANUAL_VI.md](USER_MANUAL_VI.md).
2. Install the pinned documentation dependencies, generate the PDFs and run the manual checks:

   Cập nhật [USER_MANUAL_EN.md](USER_MANUAL_EN.md) và [USER_MANUAL_VI.md](USER_MANUAL_VI.md). Cài các thư viện tài liệu đã ghim phiên bản, tạo PDF rồi kiểm tra:

```powershell
python -m pip install -r tools/requirements-manuals.txt
python tools/build_manuals.py
python tests/manuals_regression.py
```

- The renderer uses Arial, Arial Bold, Arial Italic and Consolas from the Windows fonts folder by default. Use `--font-dir` to select another folder containing those fonts.
- Fonts are not copied into the repository.
- Poppler is needed only for rendering PDF pages during visual QA. It is not an application dependency.

- Trình tạo PDF mặc định dùng Arial, Arial Bold, Arial Italic và Consolas trong thư mục font Windows. Dùng `--font-dir` để chọn thư mục khác chứa các font này.
- Font không được chép vào kho mã nguồn.
- Poppler chỉ cần khi dựng ảnh trang PDF để kiểm tra bố cục. Ứng dụng không phụ thuộc Poppler.

## Scope of verification / Phạm vi kiểm chứng

Automated WGC and renderer checks do not establish what a physical receiver or meeting app displays. Validate the actual audience output separately before presenting. Do not describe synthetic monitor checks as physical Duplicate/Extend validation.

Kiểm tra WGC và renderer tự động không xác lập hình ảnh ở đầu nhận vật lý hoặc phần mềm họp. Cần kiểm tra riêng đầu ra thực tế trước buổi trình bày. Không coi kiểm tra màn hình mô phỏng là xác minh Duplicate/Extend vật lý.

- Use concise English commit messages when contributing.
- Keep runtime files and current guides in the portable archive.
- Keep the installer, development records and validation data outside that archive.

- Dùng commit message ngắn gọn bằng tiếng Anh khi đóng góp.
- Gói portable chỉ giữ thành phần chạy và hướng dẫn hiện tại.
- Để bộ cài, tư liệu phát triển và dữ liệu kiểm thử bên ngoài gói portable.

## Refresh Project Status / Cập nhật thống kê

1. Install Git and the GitHub CLI. Authenticate for this repository.
2. Run `python tools/update_readme_status.py` from the repository root.
3. Review and commit the refreshed SVG with the README.

The SVG records 30 days of main-branch commits and GitHub issues/PRs opened, using Vietnam time (UTC+07). It includes its source commit and update time. Viewing the image does not refresh its data.

1. Cài Git và GitHub CLI. Đăng nhập và cấp quyền đọc repo.
2. Chạy `python tools/update_readme_status.py` tại thư mục gốc.
3. Xem lại và commit SVG đã cập nhật cùng README.

SVG ghi commit nhánh main và issue/PR được mở trong 30 ngày, theo giờ Việt Nam (UTC+07). Hình có commit nguồn và thời điểm cập nhật. Mở xem hình không cập nhật dữ liệu.

## Browser resource measurements / Đo tài nguyên trình duyệt

For a bounded local browser resource workload, run `python -B tests/browser_resource_benchmark.py --check --output artifacts/browser-resource-regression.json`. The shipping EXE uses real WGC verification and an isolated profile. No personal tabs are inspected. CPU is reported as percent of one logical core. Private bytes measure committed process memory, while summed working sets can include shared pages. Neither metric measures GPU memory. Compare the same script, host and workload across binaries. A synthetic feed is not an authenticated Threads benchmark.

Đo trình duyệt cục bộ bằng lệnh trên. Bản chạy dùng WGC thực và hồ sơ riêng, không đọc tab cá nhân. CPU tính theo một lõi logic. Private bytes là bộ nhớ tiến trình đã cấp, tổng working set có thể đếm trùng trang dùng chung. Hai số này không đo bộ nhớ GPU. So cùng script, máy và dữ liệu giữa các bản. Feed giả lập không đại diện phiên Threads đã đăng nhập.

## Optional speech runtime / Bộ nhận dạng tùy chọn

Build the separate CPU add-on with MinGW-w64 GCC 13.1.0, CMake and Ninja:

```powershell
python -B tools/prepare_speech.py --compiler-root "C:\path\to\mingw64\bin"
```

`tools/prepare_speech.py` pins the whisper.cpp source ZIP, toolchain flags, stripped EXE digest, licenses and deterministic add-on archive. `src/SpeechAssets.h` pins the shipped archive, CPU executable and official model revision/digests. Rebuilding with another toolchain requires an explicit reviewed pin update. Models stay outside Git and portable packages. The add-on is a separate release asset.

Tạo gói CPU riêng bằng lệnh trên. Script ghim ZIP mã nguồn whisper.cpp, cờ biên dịch, mã băm EXE đã bỏ symbol, giấy phép và ZIP kết quả. Header ghim bộ chạy và mô hình chính thức. Đổi toolchain cần duyệt và cập nhật mã băm. Mô hình không vào Git hoặc gói portable. Bộ nhận dạng là tài sản phát hành riêng.

Optional inference QA uses the English whisper.cpp JFK sample and one Vietnamese [Google FLEURS](https://huggingface.co/datasets/google/fleurs) validation clip (CC BY 4.0, vi_vn row 0). Keep attribution when sharing fixtures. The corpus/model files stay under ignored artifacts. Without them the lightweight process/cancellation checks still run and inference is marked not tested. A single clip is not a general accuracy benchmark.

Kiểm thử nhận dạng tùy chọn dùng mẫu Anh JFK whisper.cpp và một đoạn kiểm định tiếng Việt Google FLEURS (CC BY 4.0, vi_vn dòng 0). Giữ ghi nguồn khi chia sẻ mẫu. Dữ liệu và mô hình chỉ nằm trong artifacts bị bỏ qua. Nếu không có mẫu, vẫn chạy kiểm tra tiến trình/dừng và ghi rõ phần nhận dạng chưa thử. Một đoạn âm thanh không đại diện độ chính xác chung.
