# Thành phần bên thứ ba - QuietShare 0.2.0

QuietShare được biên dịch thành mã native C++17 bằng MinGW-w64 GCC 13.1.0 x64 (`x86_64-ucrt-posix-seh`). `libgcc`, `libstdc++` và các thành phần runtime MinGW dùng trong bản build được liên kết tĩnh; gói phát hành không yêu cầu cài GCC hoặc mang DLL runtime GCC riêng.

- **GCC runtime libraries:** áp dụng [GCC Runtime Library Exception 3.1](https://www.gnu.org/licenses/gcc-exception-3.1.html) cùng giấy phép của từng thành phần.
- **MinGW-w64:** thông tin giấy phép tại [MinGW-w64 licensing](https://www.mingw-w64.org/license/).
- **Thành phần hệ thống Windows:** ứng dụng và bộ cài gọi các API Win32 cho cửa sổ/control, vẽ chữ, desktop composition, DPAPI, Shell/COM, registry và thông tin phiên bản. Các thư viện hệ thống tương ứng do Windows cung cấp, không được đóng gói lại trong bộ cài QuietShare.

SDK WebView2 của bản cũ không được biên dịch, liên kết hoặc nhúng vào bản 0.2.0. Bản này không dùng .NET hay runtime trình duyệt.
