#include "../src/Documents.h"
#include "../src/FileIcons.h"

#include <winioctl.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace quietshare;
namespace
{
std::wstring root;
std::vector<std::pair<std::string, bool>> checks;
void Check(const char *name, bool passed) { checks.emplace_back(name, passed); }
std::wstring Path(const wchar_t *name) { return root + L"\\" + name; }
void Write(const std::wstring &path, const std::vector<BYTE> &bytes)
{
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) throw std::runtime_error("Fixture write open failed");
    DWORD written = 0;
    bool ok = WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) && written == bytes.size();
    CloseHandle(file);
    if (!ok) throw std::runtime_error("Fixture write failed");
}
void WriteText(const std::wstring &path, const std::string &text) { Write(path, {text.begin(), text.end()}); }
void Folder(const std::wstring &path)
{
    if (!CreateDirectoryW(path.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
        throw std::runtime_error("Fixture directory creation failed");
}
bool Junction(const std::wstring &path, const std::wstring &target)
{
    const DWORD existing = GetFileAttributesW(path.c_str());
    if (existing != INVALID_FILE_ATTRIBUTES && (existing & FILE_ATTRIBUTE_REPARSE_POINT))
        return true;
    Folder(path);
    const std::wstring substitute = L"\\??\\" + target;
    const size_t pathBytes = (substitute.size() + target.size() + 2) * sizeof(wchar_t);
    std::vector<BYTE> buffer(16 + pathBytes);
    const DWORD tag = IO_REPARSE_TAG_MOUNT_POINT;
    const WORD dataLength = static_cast<WORD>(8 + pathBytes);
    std::memcpy(buffer.data(), &tag, sizeof(tag));
    std::memcpy(buffer.data() + 4, &dataLength, sizeof(dataLength));
    WORD fields[] = {0, static_cast<WORD>(substitute.size() * sizeof(wchar_t)),
                     static_cast<WORD>((substitute.size() + 1) * sizeof(wchar_t)),
                     static_cast<WORD>(target.size() * sizeof(wchar_t))};
    std::memcpy(buffer.data() + 8, fields, sizeof(fields));
    std::memcpy(buffer.data() + 16, substitute.c_str(), (substitute.size() + 1) * sizeof(wchar_t));
    std::memcpy(buffer.data() + 16 + fields[2], target.c_str(), (target.size() + 1) * sizeof(wchar_t));
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                              FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD returned = 0;
    bool ok = DeviceIoControl(file, FSCTL_SET_REPARSE_POINT, buffer.data(), static_cast<DWORD>(buffer.size()),
                              nullptr, 0, &returned, nullptr) != FALSE;
    CloseHandle(file);
    return ok;
}

struct Context { ProjectTree *tree = nullptr; std::wstring file, error; };
LRESULT CALLBACK ParentProc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_NCCREATE)
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams));
    auto *context = reinterpret_cast<Context *>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NOTIFY && context && context->tree)
    {
        std::wstring file, error;
        context->tree->HandleNotify(reinterpret_cast<NMHDR *>(lp), file, error);
        if (!file.empty()) context->file = std::move(file);
        if (!error.empty()) context->error = std::move(error);
        return 0;
    }
    return DefWindowProcW(window, message, wp, lp);
}

HTREEITEM FindChild(HWND tree, HTREEITEM parent, const wchar_t *name)
{
    for (auto item = TreeView_GetChild(tree, parent); item; item = TreeView_GetNextSibling(tree, item))
    {
        wchar_t text[512]{};
        TVITEMW value{};
        value.mask = TVIF_TEXT;
        value.hItem = item;
        value.pszText = text;
        value.cchTextMax = 512;
        TreeView_GetItem(tree, &value);
        if (_wcsicmp(name, text) == 0) return item;
    }
    return nullptr;
}

int ItemIcon(HWND tree, HTREEITEM item, bool selected = false)
{
    TVITEMW value{};
    value.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    value.hItem = item;
    return TreeView_GetItem(tree, &value) ? (selected ? value.iSelectedImage : value.iImage) : -1;
}

std::uint64_t IconPixels(FileIconKind kind)
{
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = 16;
    info.bmiHeader.biHeight = -16;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *pixels = nullptr;
    HBITMAP image = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!dc || !image || !pixels)
    {
        if (image) DeleteObject(image);
        if (dc) DeleteDC(dc);
        return 0;
    }
    const auto previous = SelectObject(dc, image);
    std::fill_n(static_cast<DWORD *>(pixels), 256, 0x00252526);
    DrawFileTypeIcon(dc, kind, RECT{0, 0, 16, 16});
    GdiFlush();
    std::uint64_t hash = 1469598103934665603ULL;
    for (int index = 0; index < 256; ++index)
        hash = (hash ^ (static_cast<DWORD *>(pixels)[index] & 0x00FFFFFF)) * 1099511628211ULL;
    SelectObject(dc, previous);
    DeleteObject(image);
    DeleteDC(dc);
    return hash;
}

bool ImageListComposes(HIMAGELIST icons, COLORREF background)
{
    int width = 0, height = 0;
    if (!icons || !ImageList_GetIconSize(icons, &width, &height)) return false;
    const int canvasWidth = width + 4, canvasHeight = height + 4;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = canvasWidth;
    info.bmiHeader.biHeight = -canvasHeight;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    void *pixels = nullptr;
    HBITMAP image = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!dc || !image || !pixels)
    {
        if (image) DeleteObject(image);
        if (dc) DeleteDC(dc);
        return false;
    }
    const auto old = SelectObject(dc, image);
    const DWORD backdrop = (GetRValue(background) << 16) | (GetGValue(background) << 8) | GetBValue(background);
    auto *colors = static_cast<DWORD *>(pixels);
    bool all = true;
    for (int icon = 0; icon < ImageList_GetImageCount(icons); ++icon)
    {
        GdiFlush();
        std::fill_n(colors, canvasWidth * canvasHeight, backdrop);
        const bool drawn = ImageList_Draw(icons, icon, dc, 2, 2, ILD_TRANSPARENT) != FALSE;
        GdiFlush();
        int untouched = 0, foreground = 0, black = 0;
        bool outsideUnchanged = true;
        for (int y = 0; y < canvasHeight; ++y)
            for (int x = 0; x < canvasWidth; ++x)
            {
                const auto color = colors[y * canvasWidth + x] & 0x00FFFFFF;
                if (x < 2 || y < 2 || x >= width + 2 || y >= height + 2)
                    outsideUnchanged = outsideUnchanged && color == backdrop;
                else if (color == backdrop) ++untouched;
                else if (color == 0) ++black;
                else ++foreground;
            }
        all = all && drawn && outsideUnchanged && untouched >= width * height / 8 &&
              foreground >= width * height / 16 && black == 0;
    }
    SelectObject(dc, old);
    DeleteObject(image);
    DeleteDC(dc);
    return all;
}

void Run()
{
    std::wstring error;
    struct PrivateFont
    {
        std::wstring path;
        int added = 0;
        ~PrivateFont() { if (added) RemoveFontResourceExW(path.c_str(), FR_PRIVATE, nullptr); }
    } font{Path(L"..\\..\\assets\\codicons\\codicon.ttf")};
    font.added = AddFontResourceExW(font.path.c_str(), FR_PRIVATE, nullptr);
    Check("shared_file_icons_use_existing_private_codicons_font", font.added > 0);
    Check("file_icons_classify_case_insensitive_basename", FileIconForName(L"C:\\fixture\\MAIN.PY") == FileIconKind::Python &&
                                                            FileIconForName(L"fixture/main.tsx") == FileIconKind::TypeScript);
    Check("file_icons_distinguish_common_languages", FileIconForName(L"main.js") == FileIconKind::JavaScript &&
                                                       FileIconForName(L"main.cpp") == FileIconKind::Cpp &&
                                                       FileIconForName(L"main.c") == FileIconKind::C &&
                                                       FileIconForName(L"main.cs") == FileIconKind::CSharp &&
                                                       FileIconForName(L"main.java") == FileIconKind::Java &&
                                                       FileIconForName(L"main.rs") == FileIconKind::Rust &&
                                                       FileIconForName(L"main.go") == FileIconKind::Go);
    Check("file_icons_distinguish_documents_and_media", FileIconForName(L"settings.json") == FileIconKind::Json &&
                                                        FileIconForName(L"layout.xml") == FileIconKind::Xml &&
                                                        FileIconForName(L"README.md") == FileIconKind::Markdown &&
                                                        FileIconForName(L"notes.txt") == FileIconKind::Text &&
                                                        FileIconForName(L"photo.png") == FileIconKind::Image &&
                                                        FileIconForName(L"guide.pdf") == FileIconKind::Pdf);
    Check("file_icons_config_web_shell_and_fallback", FileIconForName(L".gitignore") == FileIconKind::Config &&
                                                      FileIconForName(L".env.local") == FileIconKind::Config &&
                                                      FileIconForName(L"settings.yaml") == FileIconKind::Config &&
                                                      FileIconForName(L"page.html") == FileIconKind::Html &&
                                                      FileIconForName(L"styles.css") == FileIconKind::Stylesheet &&
                                                      FileIconForName(L"script.ps1") == FileIconKind::Shell &&
                                                      FileIconForName(L"something.custom") == FileIconKind::Generic);
    Check("file_icons_folder_state_overrides_extension", FileIconForName(L"directory.py", true) == FileIconKind::Folder &&
                                                          FileIconForName(L"directory.py", true, true) == FileIconKind::FolderOpen);
    std::vector<std::uint64_t> iconPixels;
    for (const auto kind : {FileIconKind::Folder, FileIconKind::FolderOpen, FileIconKind::Python,
                            FileIconKind::JavaScript, FileIconKind::TypeScript, FileIconKind::Cpp,
                            FileIconKind::CSharp, FileIconKind::Json, FileIconKind::Xml,
                            FileIconKind::Markdown, FileIconKind::Config, FileIconKind::Image, FileIconKind::Pdf})
        iconPixels.push_back(IconPixels(kind));
    std::sort(iconPixels.begin(), iconPixels.end());
    Check("typed_icon_rasters_visually_distinct", iconPixels.front() != 0 &&
          std::adjacent_find(iconPixels.begin(), iconPixels.end()) == iconPixels.end());
    TabManager tabs;
    Check("permanent_script_initial_active", tabs.Count() == 1 && tabs.Active().id == 0 && tabs.Active().type == DocumentType::Script);
    tabs.Get(0)->content.text = L"Lời nhắc độc lập: Xin chào";
    tabs.Get(0)->state = {12, 345, 150, 0, 0, 0, 32, true};
    for (int i = 0; i < 8; ++i)
        WriteText(Path((L"file" + std::to_wstring(i) + L".cpp").c_str()), "// Fixture\nint main() { return 0; }\n");
    std::vector<DocumentId> ids;
    for (int i = 0; i < 8; ++i)
    {
        const auto opened = tabs.Open(Path((L"file" + std::to_wstring(i) + L".cpp").c_str()), error);
        if (opened) ids.push_back(opened.id);
    }
    Check("eight_readonly_files_open_alongside_script", ids.size() == 8 && tabs.Count() == 9);
    Check("code_type_and_language", tabs.Active().type == DocumentType::Code && tabs.Active().language == L"cpp" && tabs.Active().state.textPoints == 14);
    tabs.Get(ids[2])->state = {31, 704, 120, 0, 0, 0, 18, false};
    Check("activate_preserves_scroll_zoom_wrap", tabs.Activate(ids[2], error) && tabs.Active().state.scrollY == 704 && tabs.Active().state.scrollX == 31 && tabs.Active().state.textPoints == 18 && !tabs.Active().state.wrap);
    Check("cycle_forward", tabs.Cycle(1, error) && tabs.Active().id == ids[3]);
    Check("cycle_backward", tabs.Cycle(-1, error) && tabs.Active().id == ids[2]);
    auto duplicate = tabs.Open(Path(L".\\FILE2.CPP"), error);
    Check("canonical_case_insensitive_duplicate", duplicate.status == OpenStatus::Existing && duplicate.id == ids[2] && tabs.Count() == 9);
    Check("content_path_is_canonical", tabs.Active().content.path == tabs.Active().canonicalPath && tabs.Active().canonicalPath.find(L".\\") == std::wstring::npos);
    const auto oldText = tabs.Active().content.text;
    WriteText(Path(L"file2.cpp"), "// Changed externally\nint answer = 42;\n");
    duplicate = tabs.Open(Path(L"file2.cpp"), error);
    Check("modified_file_offers_reload_without_clobber", duplicate.status == OpenStatus::ReloadRequired && duplicate.id == ids[2] && tabs.Get(ids[2])->content.text == oldText);
    Check("explicit_reload_updates_only_file", tabs.Reload(ids[2], error) && tabs.Active().content.text.find(L"42") != std::wstring::npos && tabs.Get(0)->content.text == L"Lời nhắc độc lập: Xin chào");
    Check("reload_preserves_view_state", tabs.Active().state.scrollY == 704 && tabs.Active().state.textPoints == 18);
    const auto count = tabs.Count(), active = tabs.Active().id;
    Check("failed_open_retains_all_tabs_and_active", !tabs.Open(Path(L"missing.cpp"), error) && tabs.Count() == count && tabs.Active().id == active);
    Check("unsupported_binary_does_not_clear_tabs", !tabs.Open(Path(L"..\\qa-content\\must-never-run.exe"), error) && tabs.Count() == count);
    Check("script_cannot_close", !tabs.Close(0) && tabs.Get(0) != nullptr);
    Check("close_inactive_releases_document", tabs.Close(ids[0]) && !tabs.Get(ids[0]) && tabs.Active().id == active);
    Check("close_active_returns_ready_script", tabs.Close(active) && tabs.Active().id == 0 && tabs.Active().state.scrollY == 345 && tabs.Active().content.text == L"Lời nhắc độc lập: Xin chào");
    tabs.CloseOthers(ids[1]);
    Check("close_others_always_keeps_script", tabs.Count() == 2 && tabs.Get(0) && tabs.Get(ids[1]));
    tabs.CloseAllFiles();
    Check("close_all_files_preserves_script", tabs.Count() == 1 && tabs.Active().id == 0 && tabs.Active().state.textPoints == 32);

    WriteText(Path(L"sample.pdf"), "%PDF-1.7\n% Native PDF loader fixture; full validation is PDFium's responsibility\n%%EOF\n");
    LoadedContent content;
    Check("pdf_bytes_retained_without_text_or_bitmap", LoadLocalContent(Path(L"sample.pdf"), content, error) && content.kind == ContentKind::Pdf && content.bytes.size() > 8 && content.text.empty() && !content.image);
    const auto savedBytes = content.bytes;
    WriteText(Path(L"invalid.pdf"), "invalid pdf");
    Check("invalid_pdf_header_transactional", !LoadLocalContent(Path(L"invalid.pdf"), content, error) && content.bytes == savedBytes);
    LoadedContent moved(std::move(content));
    Check("pdf_move_transfers_buffer", moved.bytes == savedBytes && content.bytes.empty());
    moved.Clear();
    Check("pdf_clear_releases_buffer", moved.bytes.empty() && moved.bytes.capacity() == 0);
    std::vector<BYTE> large(64 * 1024 * 1024 + 1, 0);
    std::copy_n(reinterpret_cast<const BYTE *>("%PDF-1.7"), 8, large.begin());
    Write(Path(L"oversized.pdf"), large);
    Check("pdf_cap_64mib", !LoadLocalContent(Path(L"oversized.pdf"), content, error) && error.find(L"64") != std::wstring::npos);
    std::vector<BYTE>().swap(large);
    auto pdf = tabs.Open(Path(L"sample.pdf"), error);
    tabs.Get(pdf.id)->state = {0, 25, 175, 7, -42, 83, 24, true};
    tabs.Activate(0, error);
    tabs.EvictInactiveHeavy();
    Check("inactive_pdf_buffer_evicted", tabs.Get(pdf.id)->loadingState == LoadingState::Unloaded && tabs.Get(pdf.id)->content.bytes.empty());
    Check("lazy_pdf_reload_keeps_page_zoom", tabs.Activate(pdf.id, error) && tabs.Active().state.page == 7 && tabs.Active().state.zoom == 175 && tabs.Active().content.bytes == savedBytes);
    tabs.Activate(0, error);
    tabs.EvictInactiveHeavy();
    WriteText(Path(L"sample.pdf"), "%PDF-1.7\nchanged PDF bytes\n%%EOF\n");
    Check("evicted_modified_document_requires_explicit_reload", !tabs.Activate(pdf.id, error) && tabs.Active().id == 0 && tabs.Get(pdf.id)->content.bytes.empty());
    Check("explicit_reload_of_evicted_document", tabs.Reload(pdf.id, error) && tabs.Active().content.bytes != savedBytes);
    auto image = tabs.Open(Path(L"..\\qa-content\\transparent.png"), error);
    Check("image_tab_loaded", image && tabs.Active().type == DocumentType::Image && tabs.Active().content.image);
    tabs.Get(image.id)->state.panX = 123;
    tabs.Get(image.id)->state.panY = -456;
    tabs.Activate(0, error);
    tabs.EvictInactiveHeavy();
    Check("inactive_image_bitmap_released", !tabs.Get(image.id)->content.image && tabs.Get(image.id)->loadingState == LoadingState::Unloaded);
    Check("image_lazy_reopen_keeps_pan", tabs.Activate(image.id, error) && tabs.Active().content.image && tabs.Active().state.panX == 123 && tabs.Active().state.panY == -456);
    tabs.CloseAllFiles();
    for (int i = 0; i < 31; ++i)
    {
        const auto name = Path((L"cap" + std::to_wstring(i) + L".txt").c_str());
        WriteText(name, "fixture");
        tabs.Open(name, error);
    }
    WriteText(Path(L"cap-extra.txt"), "fixture");
    Check("tabs_cap_32_including_script", tabs.Count() == 32 && !tabs.Open(Path(L"cap-extra.txt"), error) && error.find(L"32") != std::wstring::npos);
    Check("duplicate_allowed_at_tab_cap", tabs.Open(Path(L"cap0.txt"), error).status == OpenStatus::Existing && tabs.Count() == 32);
    tabs.CloseAllFiles();
    Check("closing_releases_aggregate_memory", tabs.MemoryBytes() == tabs.Get(0)->content.text.size() * sizeof(wchar_t));
    Write(Path(L"budget0.txt"), std::vector<BYTE>(5 * 1024 * 1024, 'a'));
    for (int i = 1; i < 26; ++i)
        if (!CopyFileW(Path(L"budget0.txt").c_str(), Path((L"budget" + std::to_wstring(i) + L".txt").c_str()).c_str(), FALSE))
            throw std::runtime_error("Memory-budget fixture copy failed");
    int budgetOpened = 0;
    for (int i = 0; i < 26; ++i)
        if (tabs.Open(Path((L"budget" + std::to_wstring(i) + L".txt").c_str()), error)) ++budgetOpened;
    Check("aggregate_document_budget_256mib_transactional", budgetOpened == 25 && tabs.Count() == 26 &&
                                                            tabs.MemoryBytes() < TabManager::MemoryBudget && error.find(L"256") != std::wstring::npos);
    const auto retainedId = tabs.Active().id;
    Check("memory_budget_reject_preserves_active_text", retainedId != 0 && tabs.Active().content.text.size() == 5 * 1024 * 1024);
    tabs.CloseAllFiles();
    Check("budget_payloads_released_on_close", tabs.MemoryBytes() == tabs.Get(0)->content.text.size() * sizeof(wchar_t));
    std::wstring canonical;
    Check("canonical_rejects_unc", !CanonicalLocalPath(L"\\\\localhost\\share", true, canonical, error));
    Check("canonical_rejects_ads", !CanonicalLocalPath(Path(L"file0.cpp:stream"), false, canonical, error));
    Check("canonical_rejects_file_as_folder", !CanonicalLocalPath(Path(L"file0.cpp"), true, canonical, error));

    const auto project = Path(L"project");
    Folder(project);
    for (const auto *name : {L"src", L".git", L"node_modules", L".venv", L"dist", L"build"})
        Folder(project + L"\\" + name);
    Folder(project + L"\\src\\nested");
    WriteText(project + L"\\src\\main.py", "# Xin chao\nprint('safe')\n");
    SetFileAttributesW((project + L"\\hidden.txt").c_str(), FILE_ATTRIBUTE_NORMAL);
    WriteText(project + L"\\hidden.txt", "fixture");
    SetFileAttributesW((project + L"\\hidden.txt").c_str(), FILE_ATTRIBUTE_HIDDEN);
    const bool junction = Junction(project + L"\\link", project + L"\\src");
    Check("junction_fixture_created", junction);
    WNDCLASSW wc{};
    wc.lpfnWndProc = ParentProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"QuietShare.DocumentRegression";
    RegisterClassW(&wc);
    Context context;
    HWND parent = CreateWindowW(wc.lpszClassName, L"QuietShare generated-fixture QA", WS_OVERLAPPEDWINDOW,
                                 0, 0, 640, 480, nullptr, nullptr, wc.hInstance, &context);
    ProjectTree tree;
    context.tree = &tree;
    Check("native_tree_creation", parent && tree.Create(parent, 1));
    const auto imageList = TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL);
    int iconWidth = 0, iconHeight = 0;
    Check("project_icons_native_cached_image_list", imageList &&
          ImageList_GetImageCount(imageList) == static_cast<int>(FileIconKind::Count) &&
          ImageList_GetIconSize(imageList, &iconWidth, &iconHeight) && iconWidth == 16 && iconHeight == 16);
    Check("cached_icons_compose_on_explorer_card_without_black_squares", ImageListComposes(imageList, RGB(37,37,38)));
    Check("cached_icons_preserve_transparency_on_alternate_background", ImageListComposes(imageList, RGB(18,44,66)));
    tree.SetDpi(96);
    Check("project_icon_list_reused_for_unchanged_dpi", TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL) == imageList);
    tree.SetDpi(144);
    Check("project_icon_list_scales_at_150_percent", ImageList_GetIconSize(TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL),
          &iconWidth, &iconHeight) && iconWidth == 24 && iconHeight == 24);
    Check("cached_scaled_icons_compose_without_black_squares", ImageListComposes(TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL), RGB(37,37,38)));
    tree.SetDpi(192);
    Check("project_icon_list_scales_at_200_percent", ImageList_GetIconSize(TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL),
          &iconWidth, &iconHeight) && iconWidth == 32 && iconHeight == 32);
    tree.SetDpi(96);
    Check("project_root_is_lazy", tree.OpenFolder(project, error) && tree.NodeCount() == 1);
    HTREEITEM top = TreeView_GetRoot(tree.Handle());
    Check("project_closed_root_has_folder_icon", ItemIcon(tree.Handle(), top) == static_cast<int>(FileIconKind::Folder));
    TreeView_Expand(tree.Handle(), top, TVE_EXPAND);
    Check("project_expanded_root_has_open_folder_icon", ItemIcon(tree.Handle(), top) == static_cast<int>(FileIconKind::FolderOpen) &&
          ItemIcon(tree.Handle(), top, true) == static_cast<int>(FileIconKind::FolderOpen));
    Check("ignored_heavy_hidden_folders_omitted", tree.NodeCount() == 2 && FindChild(tree.Handle(), top, L"src") && !FindChild(tree.Handle(), top, L".git") && !FindChild(tree.Handle(), top, L"hidden.txt"));
    Check("junction_never_recursed", !FindChild(tree.Handle(), top, L"link"));
    const auto src = FindChild(tree.Handle(), top, L"src");
    Check("nested_folder_not_eagerly_loaded", !TreeView_GetChild(tree.Handle(), src));
    TreeView_Expand(tree.Handle(), src, TVE_EXPAND);
    Check("lazy_folder_expands_native", tree.NodeCount() == 4 && FindChild(tree.Handle(), src, L"main.py") && FindChild(tree.Handle(), src, L"nested"));
    Check("project_python_file_has_typed_icon", ItemIcon(tree.Handle(), FindChild(tree.Handle(), src, L"main.py")) == static_cast<int>(FileIconKind::Python));
    TreeView_Expand(tree.Handle(), src, TVE_COLLAPSE);
    Check("project_collapse_restores_folder_icon", ItemIcon(tree.Handle(), src) == static_cast<int>(FileIconKind::Folder));
    TreeView_Expand(tree.Handle(), src, TVE_EXPAND);
    TreeView_SelectItem(tree.Handle(), FindChild(tree.Handle(), src, L"main.py"));
    const auto retainedSelection = TreeView_GetSelection(tree.Handle());
    const auto retainedNodes = tree.NodeCount();
    for (const int size : {12, 24, 16})
    {
        const auto previousIcons = TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL);
        SetInterfaceIconSize(size);
        tree.SetDpi(96);
        const auto resizedIcons = TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL);
        const auto prefix = "project_icon_size_" + std::to_string(size);
        Check((prefix + "_rebuilds_at_same_dpi").c_str(), resizedIcons && resizedIcons != previousIcons &&
              ImageList_GetIconSize(resizedIcons, &iconWidth, &iconHeight) && iconWidth == size && iconHeight == size &&
              ImageList_GetImageCount(resizedIcons) == static_cast<int>(FileIconKind::Count));
        Check((prefix + "_preserves_branches_and_selection").c_str(), tree.NodeCount() == retainedNodes &&
              TreeView_GetRoot(tree.Handle()) == top && TreeView_GetSelection(tree.Handle()) == retainedSelection &&
              TreeView_GetItemState(tree.Handle(), top, TVIS_EXPANDED) && TreeView_GetItemState(tree.Handle(), src, TVIS_EXPANDED) &&
              tree.SelectedFile() == project + L"\\src\\main.py" &&
              ItemIcon(tree.Handle(), src) == static_cast<int>(FileIconKind::FolderOpen) &&
              ItemIcon(tree.Handle(), retainedSelection) == static_cast<int>(FileIconKind::Python));
        tree.SetDpi(96);
        Check((prefix + "_reuses_unchanged_image_list").c_str(), TreeView_GetImageList(tree.Handle(), TVSIL_NORMAL) == resizedIcons);
    }
    NMTVKEYDOWN key{};
    key.hdr.hwndFrom = tree.Handle();
    key.hdr.code = TVN_KEYDOWN;
    key.wVKey = VK_RETURN;
    std::wstring selected;
    tree.HandleNotify(&key.hdr, selected, error);
    Check("keyboard_enter_returns_selected_local_file", selected == project + L"\\src\\main.py");
    Check("include_ignored_opt_in_rebuilds_lazily", tree.SetIncludeIgnored(true, error) && tree.NodeCount() == 1);
    top = TreeView_GetRoot(tree.Handle());
    TreeView_Expand(tree.Handle(), top, TVE_EXPAND);
    Check("include_ignored_shows_hidden_and_heavy", FindChild(tree.Handle(), top, L".git") && FindChild(tree.Handle(), top, L"hidden.txt") && !FindChild(tree.Handle(), top, L"link"));
    const auto oldRoot = tree.Root();
    Check("failed_folder_open_keeps_existing_project", !tree.OpenFolder(Path(L"nonexistent-project"), error) && tree.Root() == oldRoot);
    Check("project_rejects_junction_root", !tree.OpenFolder(project + L"\\link", error));
    Folder(Path(L"many"));
    for (int i = 0; i < 1005; ++i)
        WriteText(Path((L"many\\entry" + std::to_wstring(i) + L".txt").c_str()), "fixture");
    tree.OpenFolder(Path(L"many"), error);
    top = TreeView_GetRoot(tree.Handle());
    TreeView_Expand(tree.Handle(), top, TVE_EXPAND);
    Check("large_folder_bounded_with_actionable_error", tree.NodeCount() == 1001 && context.error.find(L"1.000") != std::wstring::npos);
    tree.Clear();
    Check("project_clear_releases_nodes_and_metadata", tree.NodeCount() == 0 && tree.Root().empty() && !TreeView_GetRoot(tree.Handle()));
    context.tree = nullptr;
    DestroyWindow(parent);
}
} // namespace

int wmain()
{
    wchar_t module[32768]{};
    GetModuleFileNameW(nullptr, module, 32768);
    root = std::wstring(module).substr(0, std::wstring(module).find_last_of(L"\\/"));
    try { Run(); }
    catch (const std::exception &error) { std::fprintf(stderr, "%s\n", error.what()); Check("fixture_or_runtime_setup", false); }
    const size_t passed = std::count_if(checks.begin(), checks.end(), [](const auto &check) { return check.second; });
    std::ostringstream report;
    report << "{\n  \"passedChecks\": " << passed << ",\n  \"failedChecks\": " << checks.size() - passed
           << ",\n  \"scope\": \"Local generated-file and native tree fixtures; no physical display or capture claims\",\n  \"checks\": [\n";
    for (size_t index = 0; index < checks.size(); ++index)
        report << "    {\"name\": \"" << checks[index].first << "\", \"passed\": " << (checks[index].second ? "true" : "false") << "}" << (index + 1 == checks.size() ? "\n" : ",\n");
    report << "  ]\n}\n";
    const auto text = report.str();
    Write(Path(L"documents-regression.json"), {text.begin(), text.end()});
    std::printf("Documents regression: %zu/%zu passed, %zu failed\n", passed, checks.size(), checks.size() - passed);
    return passed == checks.size() ? 0 : 1;
}
