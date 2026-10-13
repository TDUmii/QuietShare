#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <algorithm>
#include <windows.h>
#include <commctrl.h>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <iomanip>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr wchar_t kAppName[] = L"QuietShare";
constexpr wchar_t kVersion[] = L"0.3.0";
constexpr wchar_t kUninstallKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\QuietShare";
constexpr int kPayloadResource = 301;
constexpr int kActionButton = 101;
constexpr int kRunCheckbox = 102;
constexpr int kCloseButton = 103;
constexpr COLORREF kBackground = RGB(11, 13, 15);
constexpr COLORREF kSurface = RGB(24, 27, 30);
constexpr COLORREF kElevated = RGB(34, 38, 42);
constexpr COLORREF kHover = RGB(44, 49, 53);
constexpr COLORREF kAccent = RGB(29, 185, 84);
constexpr COLORREF kText = RGB(248, 250, 252);
constexpr COLORREF kMuted = RGB(156, 163, 175);
constexpr COLORREF kDivider = RGB(48, 52, 58);
HINSTANCE gInstance = nullptr;
HWND gWindow = nullptr;
HWND gStatus = nullptr;
HWND gAction = nullptr;
HWND gRun = nullptr;
HWND gClose = nullptr;
HWND gHeading = nullptr;
HWND gIntro = nullptr;
HWND gFolderCaption = nullptr;
HWND gFolderPath = nullptr;
HFONT gFont = nullptr;
HFONT gTitleFont = nullptr;
HFONT gSmallFont = nullptr;
HFONT gButtonFont = nullptr;
HBRUSH gBackground = nullptr;
HBRUSH gSurface = nullptr;
UINT gDpi = 96;
bool gUninstall = false;
bool gComplete = false;
bool gQaVisible = false;
int gQaExitCode = 0;
std::wstring gQaReport;
std::wstring gRoot;

struct Handle
{
    HANDLE value = INVALID_HANDLE_VALUE;
    explicit Handle(HANDLE handle = INVALID_HANDLE_VALUE) : value(handle) {}
    ~Handle()
    {
        if (value != INVALID_HANDLE_VALUE && value != nullptr)
            CloseHandle(value);
    }
    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;
};

std::wstring ErrorText(DWORD code)
{
    wchar_t *message = nullptr;
    const DWORD length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                            FORMAT_MESSAGE_IGNORE_INSERTS,
                                        nullptr, code, 0, reinterpret_cast<wchar_t *>(&message), 0, nullptr);
    std::wstring text = length && message ? message : L"Lỗi hệ thống không xác định.";
    if (message)
        LocalFree(message);
    while (!text.empty() && (text.back() == L'\r' || text.back() == L'\n' || text.back() == L' '))
        text.pop_back();
    return text + L" (" + std::to_wstring(code) + L")";
}

std::wstring ModulePath()
{
    std::vector<wchar_t> buffer(32768);
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    return size && size < buffer.size() ? std::wstring(buffer.data(), size) : L"";
}

std::wstring KnownFolder(REFKNOWNFOLDERID id)
{
    PWSTR folder = nullptr;
    if (FAILED(SHGetKnownFolderPath(id, KF_FLAG_DONT_VERIFY, nullptr, &folder)))
        return L"";
    std::wstring result(folder);
    CoTaskMemFree(folder);
    return result;
}

std::wstring FullPath(const std::wstring &path)
{
    if (path.empty())
        return L"";
    std::vector<wchar_t> buffer(32768);
    const DWORD count =
        GetFullPathNameW(path.c_str(), static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);
    if (!count || count >= buffer.size())
        return L"";
    std::wstring result(buffer.data(), count);
    while (result.size() > 3 && (result.back() == L'\\' || result.back() == L'/'))
        result.pop_back();
    return result;
}

bool SamePath(const std::wstring &left, const std::wstring &right)
{
    const auto a = FullPath(left), b = FullPath(right);
    return !a.empty() && !b.empty() && _wcsicmp(a.c_str(), b.c_str()) == 0;
}

std::wstring ExpectedRoot()
{
    const auto local = KnownFolder(FOLDERID_LocalAppData);
    return local.empty() ? L"" : FullPath(local + L"\\Programs\\QuietShare");
}

bool Exists(const std::wstring &path)
{
    return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool RegularDirectoryOrAbsent(const std::wstring &path, std::wstring &error)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        const DWORD code = GetLastError();
        if (code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND)
            return true;
        error = L"Không kiểm tra được thư mục: " + ErrorText(code);
        return false;
    }
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY) || (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
    {
        error = L"Thư mục cài đặt phải là thư mục thông thường, không phải liên kết hoặc junction.";
        return false;
    }
    return true;
}

bool ValidateRoot(const std::wstring &root, std::wstring &error)
{
    const auto expected = ExpectedRoot();
    if (expected.empty() || !SamePath(root, expected))
    {
        error = L"Đường dẫn không khớp với thư mục cài đặt của QuietShare cho tài khoản này.";
        return false;
    }
    const auto parent = expected.substr(0, expected.find_last_of(L'\\'));
    return RegularDirectoryOrAbsent(parent, error) && RegularDirectoryOrAbsent(expected, error);
}

bool ValidateFile(const std::wstring &path, std::wstring &error)
{
    const DWORD attributes = GetFileAttributesW(path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        const DWORD code = GetLastError();
        if (code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND)
            return true;
        error = L"Không kiểm tra được tệp: " + ErrorText(code);
        return false;
    }
    if (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
    {
        error = L"Tệp ứng dụng không phải là tệp thông thường. Thao tác đã dừng.";
        return false;
    }
    return true;
}

bool AppCanBeChanged(const std::wstring &root, std::wstring &error)
{
    const auto file = root + L"\\QuietShare.exe";
    if (!ValidateFile(file, error))
        return false;
    if (!Exists(file))
        return true;
    Handle check(CreateFileW(file.c_str(), GENERIC_WRITE | DELETE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr));
    if (check.value == INVALID_HANDLE_VALUE)
    {
        error = L"Hãy đóng QuietShare trước khi tiếp tục. Nếu ứng dụng đã đóng, hãy kiểm tra quyền ghi vào "
                L"thư mục cài đặt.\n" +
                ErrorText(GetLastError());
        return false;
    }
    return true;
}

struct Payload
{
    const BYTE *bytes = nullptr;
    DWORD size = 0;
};

Payload ResourcePayload(int id)
{
    HRSRC resource = FindResourceW(gInstance, MAKEINTRESOURCEW(id), RT_RCDATA);
    if (!resource)
        return {};
    HGLOBAL data = LoadResource(gInstance, resource);
    return {static_cast<const BYTE *>(LockResource(data)), SizeofResource(gInstance, resource)};
}

Payload AppPayload()
{
    return ResourcePayload(kPayloadResource);
}

bool ValidPE(const Payload &payload)
{
    if (!payload.bytes || payload.size < sizeof(IMAGE_DOS_HEADER))
        return false;
    const auto dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(payload.bytes);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew < 0)
        return false;
    const auto offset = static_cast<size_t>(dos->e_lfanew);
    if (offset > payload.size || payload.size - offset < sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER))
        return false;
    DWORD signature = 0;
    memcpy(&signature, payload.bytes + offset, sizeof(signature));
    const auto file = reinterpret_cast<const IMAGE_FILE_HEADER *>(payload.bytes + offset + sizeof(DWORD));
    return signature == IMAGE_NT_SIGNATURE && (file->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) &&
           (file->Machine == IMAGE_FILE_MACHINE_AMD64 || file->Machine == IMAGE_FILE_MACHINE_ARM64 ||
            file->Machine == IMAGE_FILE_MACHINE_I386);
}

std::vector<std::wstring> ShortcutPaths()
{
    std::vector<std::wstring> paths;
    const auto desktop = KnownFolder(FOLDERID_Desktop), programs = KnownFolder(FOLDERID_Programs);
    if (!desktop.empty())
        paths.push_back(desktop + L"\\QuietShare.lnk");
    if (!programs.empty())
        paths.push_back(programs + L"\\QuietShare.lnk");
    return paths;
}

bool ShortcutBelongsToApp(const std::wstring &link, const std::wstring &root)
{
    if (!Exists(link))
        return true;
    IShellLinkW *shell = nullptr;
    IPersistFile *persist = nullptr;
    bool matches = false;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shell))))
    {
        if (SUCCEEDED(shell->QueryInterface(IID_PPV_ARGS(&persist))))
        {
            if (SUCCEEDED(persist->Load(link.c_str(), STGM_READ)))
            {
                std::vector<wchar_t> target(32768);
                if (SUCCEEDED(shell->GetPath(target.data(), static_cast<int>(target.size()), nullptr,
                                             SLGP_RAWPATH)))
                    matches = SamePath(target.data(), root + L"\\QuietShare.exe");
            }
            persist->Release();
        }
        shell->Release();
    }
    return matches;
}

bool CreateShortcut(const std::wstring &link, const std::wstring &root, std::wstring &error)
{
    IShellLinkW *shell = nullptr;
    IPersistFile *persist = nullptr;
    HRESULT result = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shell));
    if (SUCCEEDED(result))
    {
        const auto app = root + L"\\QuietShare.exe";
        result = shell->SetPath(app.c_str());
        if (SUCCEEDED(result))
            result = shell->SetWorkingDirectory(root.c_str());
        if (SUCCEEDED(result))
            result = shell->SetDescription(L"QuietShare");
        if (SUCCEEDED(result))
            result = shell->SetIconLocation(app.c_str(), 0);
        if (SUCCEEDED(result))
            result = shell->QueryInterface(IID_PPV_ARGS(&persist));
        if (SUCCEEDED(result))
            result = persist->Save(link.c_str(), TRUE);
        if (persist)
            persist->Release();
        shell->Release();
    }
    if (FAILED(result))
        error = L"Không tạo được lối tắt: " + ErrorText(static_cast<DWORD>(result));
    return SUCCEEDED(result);
}

bool SetRegistryString(HKEY key, const wchar_t *name, const std::wstring &value)
{
    return RegSetValueExW(key, name, 0, REG_SZ, reinterpret_cast<const BYTE *>(value.c_str()),
                          static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t))) == ERROR_SUCCESS;
}

bool RegisterUninstall(const std::wstring &root, DWORD bytes, std::wstring &error)
{
    HKEY key = nullptr;
    LONG status = RegCreateKeyExW(HKEY_CURRENT_USER, kUninstallKey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                  KEY_SET_VALUE, nullptr, &key, nullptr);
    if (status != ERROR_SUCCESS)
    {
        error = L"Không đăng ký được mục gỡ cài đặt: " + ErrorText(status);
        return false;
    }
    const std::wstring command = L"\"" + root + L"\\QuietShare-Setup.exe\" --uninstall";
    bool ok = SetRegistryString(key, L"DisplayName", kAppName) &&
              SetRegistryString(key, L"DisplayVersion", kVersion) &&
              SetRegistryString(key, L"Publisher", L"QuietShare") &&
              SetRegistryString(key, L"InstallLocation", root) &&
              SetRegistryString(key, L"DisplayIcon", root + L"\\QuietShare.exe") &&
              SetRegistryString(key, L"UninstallString", command);
    const DWORD one = 1, kb = (bytes + 1023) / 1024;
    ok = ok && RegSetValueExW(key, L"NoModify", 0, REG_DWORD, reinterpret_cast<const BYTE *>(&one),
                              sizeof(one)) == ERROR_SUCCESS;
    ok = ok && RegSetValueExW(key, L"NoRepair", 0, REG_DWORD, reinterpret_cast<const BYTE *>(&one),
                              sizeof(one)) == ERROR_SUCCESS;
    ok = ok && RegSetValueExW(key, L"EstimatedSize", 0, REG_DWORD, reinterpret_cast<const BYTE *>(&kb),
                              sizeof(kb)) == ERROR_SUCCESS;
    RegCloseKey(key);
    if (!ok)
        error = L"Không ghi đầy đủ thông tin gỡ cài đặt. Bạn có thể chạy lại bộ cài.";
    return ok;
}

bool WritePayload(const std::wstring &target, const Payload &payload, std::wstring &error)
{
    Handle file(CreateFileW(target.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            nullptr));
    if (file.value == INVALID_HANDLE_VALUE)
    {
        error = ErrorText(GetLastError());
        return false;
    }
    DWORD written = 0;
    if (!WriteFile(file.value, payload.bytes, payload.size, &written, nullptr) || written != payload.size ||
        !FlushFileBuffers(file.value))
    {
        error = ErrorText(GetLastError());
        return false;
    }
    return true;
}

bool MakeDirectory(const std::wstring &path, std::wstring &error)
{
    if (CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
        return RegularDirectoryOrAbsent(path, error);
    error = L"Không tạo được thư mục: " + ErrorText(GetLastError());
    return false;
}

bool Install(std::wstring &error)
{
    if (!ValidateRoot(gRoot, error) || !AppCanBeChanged(gRoot, error))
        return false;
    const auto payload = AppPayload();
    if (!ValidPE(payload))
    {
        error = L"Bộ cài thiếu tệp ứng dụng hợp lệ. Hãy lấy lại bộ cài.";
        return false;
    }
    const auto shortcuts = ShortcutPaths();
    if (shortcuts.size() != 2)
    {
        error = L"Không tìm thấy thư mục Desktop hoặc Start Menu cho tài khoản này.";
        return false;
    }
    for (const auto &path : shortcuts)
    {
        if (!ValidateFile(path, error))
            return false;
        if (!ShortcutBelongsToApp(path, gRoot))
        {
            error = L"Đã có lối tắt QuietShare trỏ đến ứng dụng khác: " + path;
            return false;
        }
    }
    const auto parent = gRoot.substr(0, gRoot.find_last_of(L'\\'));
    if (!MakeDirectory(parent, error) || !MakeDirectory(gRoot, error))
        return false;
    const auto app = gRoot + L"\\QuietShare.exe", setup = gRoot + L"\\QuietShare-Setup.exe";
    const auto appTemp = gRoot + L"\\QuietShare.installing.exe",
               setupTemp = gRoot + L"\\QuietShare-Setup.installing.exe";
    if (!ValidateFile(setup, error) || !ValidateFile(appTemp, error) || !ValidateFile(setupTemp, error))
        return false;
    if (!WritePayload(appTemp, payload, error))
    {
        DeleteFileW(appTemp.c_str());
        return false;
    }
    const auto self = ModulePath();
    const bool copySetup = !SamePath(self, setup);
    if (copySetup && !CopyFileW(self.c_str(), setupTemp.c_str(), FALSE))
    {
        error = L"Không sao chép được bộ gỡ cài đặt: " + ErrorText(GetLastError());
        DeleteFileW(appTemp.c_str());
        return false;
    }
    if (!MoveFileExW(appTemp.c_str(), app.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        error =
            L"Không thay thế được ứng dụng. Hãy đóng QuietShare rồi thử lại: " + ErrorText(GetLastError());
        DeleteFileW(appTemp.c_str());
        if (copySetup)
            DeleteFileW(setupTemp.c_str());
        return false;
    }
    if (copySetup &&
        !MoveFileExW(setupTemp.c_str(), setup.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        error = L"Không thay thế được bộ gỡ cài đặt: " + ErrorText(GetLastError());
        DeleteFileW(setupTemp.c_str());
        return false;
    }
    for (const auto &shortcut : shortcuts)
        if (!CreateShortcut(shortcut, gRoot, error))
            return false;
    return RegisterUninstall(gRoot, payload.size, error);
}

bool DeleteExactFile(const std::wstring &file, std::wstring &error)
{
    if (!ValidateFile(file, error))
        return false;
    if (DeleteFileW(file.c_str()))
        return true;
    const auto code = GetLastError();
    if (code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND)
        return true;
    error = L"Không xóa được tệp " + file + L": " + ErrorText(code);
    return false;
}

bool Launch(const std::wstring &executable, const std::wstring &arguments, std::wstring &error)
{
    std::wstring command = L"\"" + executable + L"\"" + (arguments.empty() ? L"" : L" " + arguments);
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr,
                        &startup, &process))
    {
        error = ErrorText(GetLastError());
        return false;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

bool StartCleanup(std::wstring &error)
{
    if (!ValidateRoot(gRoot, error) || !AppCanBeChanged(gRoot, error))
        return false;
    std::vector<wchar_t> tempFolder(32768);
    DWORD length = GetTempPathW(static_cast<DWORD>(tempFolder.size()), tempFolder.data());
    if (!length || length >= tempFolder.size())
    {
        error = L"Không tìm thấy thư mục tạm.";
        return false;
    }
    const auto helper = std::wstring(tempFolder.data(), length) + L"QuietShare-Cleanup-" +
                        std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()) +
                        L".exe";
    if (!CopyFileW(ModulePath().c_str(), helper.c_str(), TRUE))
    {
        error = L"Không tạo được bộ gỡ cài đặt tạm: " + ErrorText(GetLastError());
        return false;
    }
    // Inherit a real process handle. Opening a PID in the child races with this installer exiting.
    Handle waitHandle(
        OpenProcess(SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, TRUE, GetCurrentProcessId()));
    if (waitHandle.value == INVALID_HANDLE_VALUE)
    {
        error = L"Không chuẩn bị được bộ gỡ cài đặt: " + ErrorText(GetLastError());
        DeleteFileW(helper.c_str());
        return false;
    }
    const auto arguments = L"--cleanup --wait-pid " + std::to_wstring(GetCurrentProcessId()) +
                           L" --wait-handle " +
                           std::to_wstring(reinterpret_cast<uintptr_t>(waitHandle.value)) +
                           L" --install-path \"" + gRoot + L"\"";
    std::wstring command = L"\"" + helper + L"\" " + arguments;
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(helper.c_str(), command.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &startup,
                        &process))
    {
        error = L"Không mở được bộ gỡ cài đặt: " + ErrorText(GetLastError());
        DeleteFileW(helper.c_str());
        return false;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

void MarkTemporaryImageForDeletion()
{
    const auto self = ModulePath();
    std::vector<wchar_t> folder(32768);
    const DWORD length = GetTempPathW(static_cast<DWORD>(folder.size()), folder.data());
    if (!length || length >= folder.size())
        return;
    const auto parent = self.substr(0, self.find_last_of(L'\\'));
    const auto filename = self.substr(self.find_last_of(L'\\') + 1);
    if (!SamePath(parent, std::wstring(folder.data(), length)) ||
        filename.rfind(L"QuietShare-Cleanup-", 0) != 0)
        return;
    Handle file(CreateFileW(self.c_str(), DELETE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (file.value == INVALID_HANDLE_VALUE)
        return;
    // FILE_DISPOSITION_INFO_EX: POSIX deletion removes the temporary helper's directory entry on supported
    // Windows.
    struct Disposition
    {
        DWORD Flags;
    } disposition{0x1 | 0x2 | 0x10};
    SetFileInformationByHandle(file.value, static_cast<FILE_INFO_BY_HANDLE_CLASS>(21), &disposition,
                               sizeof(disposition));
}

int Cleanup(DWORD parentPid, HANDLE inheritedParent, const std::wstring &root)
{
    std::wstring error;
    Handle parent(inheritedParent);
    if (!parentPid || parentPid == GetCurrentProcessId() || parent.value == nullptr ||
        parent.value == INVALID_HANDLE_VALUE || GetProcessId(parent.value) != parentPid ||
        !ValidateRoot(root, error))
    {
        if (error.empty())
            error = L"Yêu cầu gỡ cài đặt không hợp lệ.";
        MessageBoxW(nullptr, error.c_str(), L"QuietShare", MB_OK | MB_ICONERROR);
        return 2;
    }
    if (WaitForSingleObject(parent.value, 30000) != WAIT_OBJECT_0)
    {
        MessageBoxW(nullptr,
                    L"Bộ cài chưa đóng. Hãy đóng các cửa sổ cài đặt rồi chạy lại thao tác gỡ cài đặt.",
                    L"QuietShare", MB_OK | MB_ICONERROR);
        return 3;
    }
    if (!ValidateRoot(root, error) || !AppCanBeChanged(root, error))
    {
        MessageBoxW(nullptr, error.c_str(), L"QuietShare", MB_OK | MB_ICONERROR);
        MarkTemporaryImageForDeletion();
        return 4;
    }
    // All mutations below target the fixed per-user installation, never application/user content folders.
    const std::vector<std::wstring> installedFiles{L"QuietShare.exe", L"QuietShare-Setup.exe"};
    for (const auto &name : installedFiles)
        if (!ValidateFile(root + L"\\" + name, error))
        {
            MessageBoxW(nullptr, error.c_str(), L"QuietShare", MB_OK | MB_ICONERROR);
            MarkTemporaryImageForDeletion();
            return 5;
        }
    for (const auto &name : installedFiles)
        if (!DeleteExactFile(root + L"\\" + name, error))
        {
            MessageBoxW(nullptr, error.c_str(), L"QuietShare", MB_OK | MB_ICONERROR);
            MarkTemporaryImageForDeletion();
            return 5;
        }
    for (const auto &shortcut : ShortcutPaths())
    {
        if (ShortcutBelongsToApp(shortcut, root) && !DeleteExactFile(shortcut, error))
        {
            MessageBoxW(nullptr, error.c_str(), L"QuietShare", MB_OK | MB_ICONERROR);
            MarkTemporaryImageForDeletion();
            return 6;
        }
    }
    const LONG registry = RegDeleteKeyW(HKEY_CURRENT_USER, kUninstallKey);
    if (registry != ERROR_SUCCESS && registry != ERROR_FILE_NOT_FOUND)
    {
        MessageBoxW(nullptr,
                    (L"Đã xóa ứng dụng nhưng không xóa được mục gỡ cài đặt: " + ErrorText(registry)).c_str(),
                    L"QuietShare", MB_OK | MB_ICONERROR);
        MarkTemporaryImageForDeletion();
        return 7;
    }
    // RemoveDirectory only removes an empty app installation directory. It never recurses.
    RemoveDirectoryW(root.c_str());
    MarkTemporaryImageForDeletion();
    MessageBoxW(nullptr, L"Đã gỡ QuietShare. Ghi chú và cài đặt cá nhân của bạn được giữ lại.", L"QuietShare",
                MB_OK | MB_ICONINFORMATION);
    return 0;
}

std::string Utf8(const std::wstring &value)
{
    if (value.empty())
        return "";
    const auto length = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr,
                                            0, nullptr, nullptr);
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), length,
                        nullptr, nullptr);
    return result;
}

std::string JsonString(const std::wstring &value)
{
    std::string result = "\"";
    for (unsigned char ch : Utf8(value))
    {
        switch (ch)
        {
        case '\\':
            result += "\\\\";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            if (ch < 32)
            {
                const char hex[] = "0123456789abcdef";
                result += "\\u00";
                result += hex[ch >> 4];
                result += hex[ch & 15];
            }
            else
                result += static_cast<char>(ch);
        }
    }
    return result + "\"";
}

int SelfTest(const std::wstring &reportPath)
{
    std::wstring error;
    const auto payload = AppPayload();
    const auto root = ExpectedRoot();
    const bool payloadOk = ValidPE(payload), pathOk = ValidateRoot(root, error);
    const auto self = ModulePath();
    DWORD ignored = 0;
    const DWORD versionSize = GetFileVersionInfoSizeW(self.c_str(), &ignored);
    std::vector<BYTE> versionData(versionSize);
    VS_FIXEDFILEINFO *info = nullptr;
    UINT infoSize = 0;
    bool versionOk = versionSize && GetFileVersionInfoW(self.c_str(), 0, versionSize, versionData.data()) &&
                     VerQueryValueW(versionData.data(), L"\\", reinterpret_cast<void **>(&info), &infoSize) &&
                     infoSize >= sizeof(VS_FIXEDFILEINFO) && info->dwSignature == 0xFEEF04BD &&
                     HIWORD(info->dwFileVersionMS) == 0 && LOWORD(info->dwFileVersionMS) == 3 &&
                     info->dwFileVersionLS == 0;
    const bool ok = payloadOk && pathOk && versionOk;
    std::ostringstream report;
    report << "{\n  \"selfTestPassed\": " << (ok ? "true" : "false")
           << ",\n  \"version\": \"0.3.0\",\n  \"versionResourceVerified\": "
           << (versionOk ? "true" : "false")
           << ",\n  \"payloadResourceId\": 301,\n  \"payloadBytes\": " << payload.size
           << ",\n  \"payloadPortableExecutable\": " << (payloadOk ? "true" : "false")
           << ",\n  \"installRoot\": " << JsonString(root)
           << ",\n  \"fixedInstallPathVerified\": " << (pathOk ? "true" : "false")
           << ",\n  \"installationWrites\": false,\n  \"error\": " << JsonString(error) << "\n}\n";
    const auto bytes = report.str();
    if (!reportPath.empty())
    {
        const auto destination = FullPath(reportPath);
        // Diagnostics may be written only when explicitly requested and outside the installed application
        // directory.
        const auto normalizedRoot = FullPath(root) + L"\\";
        if (destination.empty() || SamePath(destination, root) ||
            _wcsnicmp(destination.c_str(), normalizedRoot.c_str(), normalizedRoot.size()) == 0)
            return 10;
        Handle file(CreateFileW(destination.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL, nullptr));
        if (file.value == INVALID_HANDLE_VALUE)
            return 11;
        DWORD written = 0;
        if (!WriteFile(file.value, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) ||
            written != bytes.size())
            return 12;
    }
    else
    {
        AttachConsole(ATTACH_PARENT_PROCESS);
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        if (output && output != INVALID_HANDLE_VALUE)
        {
            DWORD written = 0;
            WriteFile(output, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr);
        }
    }
    return ok ? 0 : 1;
}

bool SaveOwnWindowBitmap(HWND window, const std::wstring &path)
{
    RECT bounds{};
    if (!GetWindowRect(window, &bounds))
        return false;
    const LONG width = bounds.right - bounds.left, height = bounds.bottom - bounds.top;
    if (width <= 0 || height <= 0 || width > 10000 || height > 10000)
        return false;
    HDC windowDC = GetWindowDC(window);
    if (!windowDC)
        return false;
    HDC memory = CreateCompatibleDC(windowDC);
    BITMAPINFO bitmap{};
    bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap.bmiHeader.biWidth = width;
    bitmap.bmiHeader.biHeight = -height;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = 32;
    bitmap.bmiHeader.biCompression = BI_RGB;
    const DWORD byteSize = static_cast<DWORD>(width * height * 4);
    bitmap.bmiHeader.biSizeImage = byteSize;
    void *pixels = nullptr;
    HBITMAP image =
        memory ? CreateDIBSection(windowDC, &bitmap, DIB_RGB_COLORS, &pixels, nullptr, 0) : nullptr;
    bool ok = false;
    if (image && pixels)
    {
        HGDIOBJ previous = SelectObject(memory, image);
        // PrintWindow renders only this HWND into a clean buffer. No desktop pixels are copied.
        memset(pixels, 0, byteSize);
        RECT area{0, 0, width, height};
        FillRect(memory, &area, gBackground);
        if (PrintWindow(window, memory, 0x00000002))
        {
            // BI_RGB ignores alpha, but opaque bytes also aid bitmap viewers that inspect it.
            auto *bytes = static_cast<BYTE *>(pixels);
            for (DWORD offset = 3; offset < byteSize; offset += 4)
                bytes[offset] = 255;
            BITMAPFILEHEADER header{};
            header.bfType = 0x4D42;
            header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            header.bfSize = header.bfOffBits + byteSize;
            Handle file(CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr));
            if (file.value != INVALID_HANDLE_VALUE)
            {
                DWORD written = 0;
                ok = WriteFile(file.value, &header, sizeof(header), &written, nullptr) &&
                     written == sizeof(header) &&
                     WriteFile(file.value, &bitmap.bmiHeader, sizeof(BITMAPINFOHEADER), &written, nullptr) &&
                     written == sizeof(BITMAPINFOHEADER) &&
                     WriteFile(file.value, pixels, byteSize, &written, nullptr) && written == byteSize;
            }
        }
        SelectObject(memory, previous);
        DeleteObject(image);
    }
    if (memory)
        DeleteDC(memory);
    ReleaseDC(window, windowDC);
    return ok;
}

bool ValidQaReportPath(const std::wstring &path)
{
    const bool absolute = (path.size() >= 3 && path[1] == L':' && (path[2] == L'\\' || path[2] == L'/')) ||
                          path.rfind(L"\\\\", 0) == 0;
    if (!absolute || path.size() < 4 || _wcsicmp(path.substr(path.size() - 4).c_str(), L".bmp") != 0)
        return false;
    const auto destination = FullPath(path), root = ExpectedRoot();
    const auto rootPrefix = root + L"\\";
    std::wstring error;
    return !destination.empty() && !SamePath(destination, root) &&
           _wcsnicmp(destination.c_str(), rootPrefix.c_str(), rootPrefix.size()) != 0 &&
           ValidateFile(destination, error);
}

int S(int value)
{
    return MulDiv(value, gDpi, 96);
}

void FillRounded(HDC dc, const RECT &rect, COLORREF color, int radius)
{
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ previousBrush = SelectObject(dc, brush);
    HGDIOBJ previousPen = SelectObject(dc, GetStockObject(NULL_PEN));
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    SelectObject(dc, previousPen);
    SelectObject(dc, previousBrush);
    DeleteObject(brush);
}

void DrawInstallerText(HDC dc, const wchar_t *text, RECT rect, COLORREF color, HFONT font, UINT flags)
{
    HGDIOBJ previous = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    DrawTextW(dc, text, -1, &rect, flags);
    SelectObject(dc, previous);
}

void PaintInstaller(HDC dc)
{
    RECT client{};
    GetClientRect(gWindow, &client);
    FillRect(dc, &client, gBackground);
    RECT logo{S(32), S(24), S(64), S(56)};
    FillRounded(dc, logo, kAccent, S(8));
    HPEN stroke = CreatePen(PS_SOLID, std::max(1, S(2)), kBackground);
    HGDIOBJ previousPen = SelectObject(dc, stroke);
    HGDIOBJ previousBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    MoveToEx(dc, S(45), S(37), nullptr);
    LineTo(dc, S(53), S(33));
    MoveToEx(dc, S(44), S(40), nullptr);
    LineTo(dc, S(54), S(46));
    Ellipse(dc, S(37), S(34), S(45), S(42));
    Ellipse(dc, S(53), S(28), S(61), S(36));
    Ellipse(dc, S(53), S(44), S(61), S(52));
    SelectObject(dc, previousBrush);
    SelectObject(dc, previousPen);
    DeleteObject(stroke);
    DrawInstallerText(dc, L"QuietShare", {S(76), S(24), S(256), S(56)}, kText, gButtonFont,
                      DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawInstallerText(dc, L"0.3.0", {S(504), S(24), S(588), S(56)}, kMuted, gSmallFont,
                      DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    FillRounded(dc, {S(32), S(192), S(588), S(280)}, kSurface, S(12));
    RECT footer{0, S(400), client.right, client.bottom};
    FillRect(dc, &footer, gSurface);
    HBRUSH divider = CreateSolidBrush(kDivider);
    RECT rule{0, S(400), client.right, S(401)};
    FillRect(dc, &rule, divider);
    DeleteObject(divider);
}

void DrawInstallerControl(HWND control, HDC dc)
{
    RECT rect{};
    GetClientRect(control, &rect);
    const bool checkbox = GetDlgCtrlID(control) == kRunCheckbox;
    const bool primary = GetDlgCtrlID(control) == kActionButton;
    const bool enabled = IsWindowEnabled(control) != FALSE;
    const bool hot = GetPropW(control, L"QuietShareHover") != nullptr;
    const LRESULT state = SendMessageW(control, BM_GETSTATE, 0, 0);
    const bool pressed = (state & BST_PUSHED) != 0;
    const bool focused = GetFocus() == control;
    FillRect(dc, &rect, checkbox ? gBackground : gSurface);
    const COLORREF color = !enabled  ? kSurface
                           : pressed ? (primary ? RGB(24, 149, 68) : kElevated)
                           : primary ? (hot ? RGB(35, 207, 98) : kAccent)
                                     : (hot ? kHover : kElevated);
    RECT shape = rect;
    InflateRect(&shape, -1, -1);
    if (checkbox)
        shape = {S(1), (rect.bottom - S(18)) / 2, S(19), (rect.bottom + S(18)) / 2};
    const bool checked = checkbox && SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
    const COLORREF checkedColor = pressed ? RGB(24, 149, 68) : hot ? RGB(35, 207, 98) : kAccent;
    FillRounded(dc, shape, checked && enabled ? checkedColor : color, checkbox ? S(4) : S(8));
    if (checkbox || !primary)
    {
        HPEN border = CreatePen(PS_SOLID, std::max(1, S(1)), enabled ? RGB(99, 107, 116) : kDivider);
        HGDIOBJ previousPen = SelectObject(dc, border);
        HGDIOBJ previousBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        RoundRect(dc, shape.left, shape.top, shape.right, shape.bottom, checkbox ? S(4) : S(8),
                  checkbox ? S(4) : S(8));
        SelectObject(dc, previousBrush);
        SelectObject(dc, previousPen);
        DeleteObject(border);
    }
    if (checked)
    {
        HPEN check = CreatePen(PS_SOLID, std::max(1, S(2)), enabled ? kBackground : kMuted);
        HGDIOBJ previous = SelectObject(dc, check);
        MoveToEx(dc, shape.left + S(4), shape.top + S(9), nullptr);
        LineTo(dc, shape.left + S(7), shape.top + S(12));
        LineTo(dc, shape.left + S(13), shape.top + S(5));
        SelectObject(dc, previous);
        DeleteObject(check);
    }
    wchar_t text[128]{};
    GetWindowTextW(control, text, static_cast<int>(sizeof(text) / sizeof(text[0])));
    RECT label = rect;
    if (checkbox)
        label.left = S(30);
    DrawInstallerText(dc, text, label,
                      !enabled  ? kMuted
                      : primary ? kBackground
                                : kText,
                      checkbox ? gFont : gButtonFont,
                      DT_VCENTER | DT_SINGLELINE | (checkbox ? DT_LEFT : DT_CENTER));
    if (focused)
    {
        RECT focus = checkbox ? RECT{S(26), 0, rect.right, rect.bottom} : shape;
        InflateRect(&focus, -S(3), -S(3));
        HPEN pen = CreatePen(PS_SOLID, std::max(1, S(2)), primary ? kBackground : kAccent);
        HGDIOBJ previousPen = SelectObject(dc, pen);
        HGDIOBJ previousBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        RoundRect(dc, focus.left, focus.top, focus.right, focus.bottom, S(6), S(6));
        SelectObject(dc, previousBrush);
        SelectObject(dc, previousPen);
        DeleteObject(pen);
    }
}

LRESULT CALLBACK InstallerControlProc(HWND control, UINT message, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
    // Keep the native BUTTON styles, accessibility and Enter/Space/check semantics; replace painting only.
    switch (message)
    {
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(control, &paint);
        DrawInstallerControl(control, dc);
        EndPaint(control, &paint);
        return 0;
    }
    case WM_PRINTCLIENT:
        DrawInstallerControl(control, reinterpret_cast<HDC>(wp));
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_MOUSEMOVE:
        if (!GetPropW(control, L"QuietShareHover"))
        {
            SetPropW(control, L"QuietShareHover", reinterpret_cast<HANDLE>(1));
            TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, control, 0};
            TrackMouseEvent(&track);
            InvalidateRect(control, nullptr, FALSE);
        }
        break;
    case WM_MOUSELEAVE:
        RemovePropW(control, L"QuietShareHover");
        InvalidateRect(control, nullptr, FALSE);
        break;
    case WM_NCDESTROY:
        RemovePropW(control, L"QuietShareHover");
        RemoveWindowSubclass(control, InstallerControlProc, 1);
        break;
    }
    const LRESULT result = DefSubclassProc(control, message, wp, lp);
    if (message == WM_SETFOCUS || message == WM_KILLFOCUS || message == WM_ENABLE || message == BM_SETCHECK ||
        message == BM_SETSTATE || message == WM_SETTEXT || message == WM_KEYDOWN || message == WM_KEYUP ||
        message == WM_LBUTTONDOWN || message == WM_LBUTTONUP)
        InvalidateRect(control, nullptr, FALSE);
    return result;
}

void SetInstallerFonts()
{
    for (HFONT font : {gFont, gTitleFont, gSmallFont, gButtonFont})
        if (font)
            DeleteObject(font);
    auto make = [](int size, int weight) {
        return CreateFontW(-S(size), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH,
                           L"Segoe UI");
    };
    gFont = make(15, FW_NORMAL);
    gTitleFont = make(24, FW_SEMIBOLD);
    gSmallFont = make(12, FW_SEMIBOLD);
    gButtonFont = make(15, FW_SEMIBOLD);
    for (HWND item : {gIntro, gFolderPath, gStatus, gRun})
        if (item)
            SendMessageW(item, WM_SETFONT, reinterpret_cast<WPARAM>(gFont), FALSE);
    for (HWND item : {gAction, gClose})
        if (item)
            SendMessageW(item, WM_SETFONT, reinterpret_cast<WPARAM>(gButtonFont), FALSE);
    if (gHeading)
        SendMessageW(gHeading, WM_SETFONT, reinterpret_cast<WPARAM>(gTitleFont), FALSE);
    if (gFolderCaption)
        SendMessageW(gFolderCaption, WM_SETFONT, reinterpret_cast<WPARAM>(gSmallFont), FALSE);
}

void LayoutInstaller()
{
    auto place = [](HWND item, int x, int y, int width, int height) {
        if (item)
            MoveWindow(item, S(x), S(y), S(width), S(height), TRUE);
    };
    place(gHeading, 32, 80, 556, 34);
    place(gIntro, 32, 124, 556, 48);
    place(gFolderCaption, 48, 208, 524, 18);
    place(gFolderPath, 48, 232, 524, 36);
    place(gStatus, 32, 300, 556, 44);
    place(gRun, 32, 352, 424, 32);
    place(gClose, 332, 420, 112, 40);
    place(gAction, 456, 420, 132, 40);
}

HWND Label(HWND parent, const wchar_t *text, HFONT font)
{
    HWND item = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, parent,
                                nullptr, gInstance, nullptr);
    SendMessageW(item, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return item;
}

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        gWindow = window;
        gDpi = GetDpiForWindow(window);
        SetInstallerFonts();
        gHeading = Label(window, gUninstall ? L"Gỡ cài đặt QuietShare" : L"Cài đặt QuietShare", gTitleFont);
        gIntro = Label(window,
                       gUninstall
                           ? L"Gỡ ứng dụng khỏi tài khoản Windows hiện tại.\nGhi chú và cài đặt cá nhân của "
                             L"bạn được giữ lại."
                           : L"Cửa sổ nhắc chữ riêng khi bạn chia sẻ màn hình.\nCài đặt cho tài "
                             L"khoản hiện tại, không cần quyền quản trị.",
                       gFont);
        gFolderCaption = Label(window, L"THƯ MỤC ỨNG DỤNG", gSmallFont);
        gFolderPath = Label(window, gRoot.c_str(), gFont);
        gStatus = Label(window,
                        gUninstall ? L"Hãy đóng QuietShare trước khi gỡ cài đặt."
                                   : L"Thêm lối tắt vào Desktop và Start Menu. Phiên bản 0.3.0.",
                        gFont);
        if (!gUninstall)
        {
            gRun = CreateWindowExW(0, L"BUTTON", L"Mở QuietShare sau khi cài đặt",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, window,
                                   reinterpret_cast<HMENU>(kRunCheckbox), gInstance, nullptr);
            SendMessageW(gRun, WM_SETFONT, reinterpret_cast<WPARAM>(gFont), TRUE);
            SendMessageW(gRun, BM_SETCHECK, BST_CHECKED, 0);
            SetWindowSubclass(gRun, InstallerControlProc, 1, 0);
        }
        gClose = CreateWindowExW(0, L"BUTTON", L"Hủy", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0,
                                 0, 0, 0, window, reinterpret_cast<HMENU>(kCloseButton), gInstance, nullptr);
        gAction = CreateWindowExW(0, L"BUTTON", gUninstall ? L"Gỡ cài đặt" : L"Cài đặt",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 0, 0, 0, 0, window,
                                  reinterpret_cast<HMENU>(kActionButton), gInstance, nullptr);
        SendMessageW(gClose, WM_SETFONT, reinterpret_cast<WPARAM>(gButtonFont), TRUE);
        SendMessageW(gAction, WM_SETFONT, reinterpret_cast<WPARAM>(gButtonFont), TRUE);
        SetWindowSubclass(gClose, InstallerControlProc, 1, 0);
        SetWindowSubclass(gAction, InstallerControlProc, 1, 0);
        LayoutInstaller();
        if (gRoot.empty())
        {
            EnableWindow(gAction, FALSE);
            SetWindowTextW(gStatus, L"Không tìm được thư mục ứng dụng của tài khoản này.");
        }
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == kCloseButton)
        {
            DestroyWindow(window);
            return 0;
        }
        if (LOWORD(wParam) == kActionButton)
        {
            if (gQaVisible)
                return 0;
            if (gComplete)
            {
                DestroyWindow(window);
                return 0;
            }
            EnableWindow(gAction, FALSE);
            std::wstring error;
            if (gUninstall)
            {
                if (StartCleanup(error))
                    DestroyWindow(window);
                else
                {
                    MessageBoxW(window, error.c_str(), L"Chưa gỡ được QuietShare", MB_OK | MB_ICONERROR);
                    EnableWindow(gAction, TRUE);
                }
                return 0;
            }
            SetWindowTextW(gStatus, L"Đang cài đặt...");
            UpdateWindow(window);
            if (!Install(error))
            {
                SetWindowTextW(gStatus, L"Cài đặt chưa hoàn tất. Hãy kiểm tra lỗi và chạy lại bộ cài.");
                MessageBoxW(window, error.c_str(), L"Chưa cài được QuietShare", MB_OK | MB_ICONERROR);
                EnableWindow(gAction, TRUE);
                return 0;
            }
            gComplete = true;
            SetWindowTextW(gStatus,
                           L"Đã cài đặt QuietShare. Bạn có thể mở ứng dụng từ Desktop hoặc Start Menu.");
            SetWindowTextW(gAction, L"Hoàn tất");
            EnableWindow(gAction, TRUE);
            if (SendMessageW(gRun, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                if (!Launch(gRoot + L"\\QuietShare.exe", L"", error))
                    MessageBoxW(window, (L"Đã cài đặt nhưng chưa mở được ứng dụng: " + error).c_str(),
                                L"QuietShare", MB_OK | MB_ICONINFORMATION);
            }
            EnableWindow(gRun, FALSE);
            SetWindowTextW(GetDlgItem(window, kCloseButton), L"Đóng");
            return 0;
        }
        break;
    case WM_TIMER:
        if (gQaVisible && wParam == 901)
        {
            KillTimer(window, 901);
            RedrawWindow(window, nullptr, nullptr,
                         RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME);
            gQaExitCode = SaveOwnWindowBitmap(window, gQaReport) ? 0 : 30;
            DestroyWindow(window);
            return 0;
        }
        break;
    case WM_CTLCOLORSTATIC: {
        const HWND label = reinterpret_cast<HWND>(lParam);
        const bool card = label == gFolderCaption || label == gFolderPath;
        SetBkColor(reinterpret_cast<HDC>(wParam), card ? kSurface : kBackground);
        SetTextColor(reinterpret_cast<HDC>(wParam),
                     label == gHeading || label == gFolderPath ? kText : kMuted);
        return reinterpret_cast<LRESULT>(card ? gSurface : gBackground);
    }
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        PaintInstaller(dc);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_ERASEBKGND:
        PaintInstaller(reinterpret_cast<HDC>(wParam));
        return 1;
    case WM_DPICHANGED: {
        gDpi = HIWORD(wParam);
        SetInstallerFonts();
        const auto suggested = reinterpret_cast<RECT *>(lParam);
        SetWindowPos(window, nullptr, suggested->left, suggested->top, suggested->right - suggested->left,
                     suggested->bottom - suggested->top, SWP_NOACTIVATE | SWP_NOZORDER);
        LayoutInstaller();
        RedrawWindow(window, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(gQaVisible ? gQaExitCode : 0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    gInstance = instance;
    int count = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &count);
    bool selfTest = false, cleanup = false;
    std::wstring reportPath, cleanupRoot;
    DWORD waitPid = 0;
    HANDLE waitHandle = nullptr;
    for (int i = 1; argv && i < count; ++i)
    {
        const std::wstring arg(argv[i]);
        if (arg == L"--uninstall")
            gUninstall = true;
        else if (arg == L"--self-test")
            selfTest = true;
        else if (arg == L"--qa-visible")
            gQaVisible = true;
        else if (arg == L"--cleanup")
            cleanup = true;
        else if (arg == L"--report" && i + 1 < count)
            reportPath = argv[++i];
        else if (arg == L"--install-path" && i + 1 < count)
            cleanupRoot = argv[++i];
        else if (arg == L"--wait-pid" && i + 1 < count)
        {
            wchar_t *end = nullptr;
            const unsigned long parsed = wcstoul(argv[++i], &end, 10);
            if (end && !*end)
                waitPid = static_cast<DWORD>(parsed);
        }
        else if (arg == L"--wait-handle" && i + 1 < count)
        {
            wchar_t *end = nullptr;
            const unsigned long long parsed = _wcstoui64(argv[++i], &end, 10);
            if (end && !*end)
                waitHandle = reinterpret_cast<HANDLE>(static_cast<uintptr_t>(parsed));
        }
        else
        {
            if (argv)
                LocalFree(argv);
            MessageBoxW(nullptr, L"Tham số không hợp lệ. Chạy bộ cài không có tham số để mở cửa sổ cài đặt.",
                        L"QuietShare", MB_OK | MB_ICONERROR);
            return 20;
        }
    }
    if (argv)
        LocalFree(argv);
    if (gQaVisible)
    {
        if (selfTest || cleanup || !ValidQaReportPath(reportPath))
            return 31;
        gQaReport = FullPath(reportPath);
        gQaExitCode = 32;
    }
    if (selfTest)
        return SelfTest(reportPath);
    const HRESULT com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(com))
    {
        MessageBoxW(nullptr, L"Không khởi tạo được dịch vụ Windows để tạo lối tắt.", L"QuietShare",
                    MB_OK | MB_ICONERROR);
        return 21;
    }
    if (cleanup)
    {
        const int result = Cleanup(waitPid, waitHandle, cleanupRoot);
        CoUninitialize();
        return result;
    }
    gRoot = ExpectedRoot();
    SetProcessDPIAware();
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    HDC screen = GetDC(nullptr);
    if (screen)
    {
        gDpi = static_cast<UINT>(GetDeviceCaps(screen, LOGPIXELSX));
        ReleaseDC(nullptr, screen);
    }
    gBackground = CreateSolidBrush(kBackground);
    gSurface = CreateSolidBrush(kSurface);
    WNDCLASSEXW type{};
    type.cbSize = sizeof(type);
    type.lpfnWndProc = WindowProc;
    type.hInstance = instance;
    type.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    type.hbrBackground = gBackground;
    type.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(101));
    type.hIconSm = reinterpret_cast<HICON>(LoadImageW(instance, MAKEINTRESOURCEW(101), IMAGE_ICON,
                                                      GetSystemMetrics(SM_CXSMICON),
                                                      GetSystemMetrics(SM_CYSMICON), LR_SHARED));
    type.lpszClassName = L"QuietShareSetupWindow";
    RegisterClassExW(&type);
    RECT rect{0, 0, S(620), S(480)};
    constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rect, style, FALSE, 0);
    const int width = rect.right - rect.left, height = rect.bottom - rect.top;
    gWindow = CreateWindowExW(
        0, type.lpszClassName, gUninstall ? L"QuietShare - Gỡ cài đặt" : L"QuietShare - Cài đặt", style,
        (GetSystemMetrics(SM_CXSCREEN) - width) / 2, (GetSystemMetrics(SM_CYSCREEN) - height) / 2, width,
        height, nullptr, nullptr, instance, nullptr);
    if (!gWindow)
    {
        DeleteObject(gFont);
        DeleteObject(gTitleFont);
        DeleteObject(gSmallFont);
        DeleteObject(gButtonFont);
        DeleteObject(gBackground);
        DeleteObject(gSurface);
        CoUninitialize();
        return 22;
    }
    HMODULE dwm = LoadLibraryExW(L"dwmapi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (dwm)
    {
        using SetAttribute = HRESULT(WINAPI *)(HWND, DWORD, LPCVOID, DWORD);
        auto setAttribute = reinterpret_cast<SetAttribute>(GetProcAddress(dwm, "DwmSetWindowAttribute"));
        if (setAttribute)
        {
            const BOOL dark = TRUE;
            const DWORD rounded = 2;
            setAttribute(gWindow, 20, &dark, sizeof(dark));
            setAttribute(gWindow, 33, &rounded, sizeof(rounded));
            setAttribute(gWindow, 34, &kDivider, sizeof(kDivider));
        }
        FreeLibrary(dwm);
    }
    ShowWindow(gWindow, show);
    UpdateWindow(gWindow);
    if (gQaVisible)
    {
        ShowWindow(gWindow, SW_SHOWNORMAL);
        SetForegroundWindow(gWindow);
        if (!SetTimer(gWindow, 901, 1000, nullptr))
        {
            DestroyWindow(gWindow);
        }
    }
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        if (!IsDialogMessageW(gWindow, &message))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    DeleteObject(gFont);
    DeleteObject(gTitleFont);
    DeleteObject(gSmallFont);
    DeleteObject(gButtonFont);
    DeleteObject(gBackground);
    DeleteObject(gSurface);
    CoUninitialize();
    return static_cast<int>(message.wParam);
}
