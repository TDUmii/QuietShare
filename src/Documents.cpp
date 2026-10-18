#include "Documents.h"
#include "FileIcons.h"

#include <uxtheme.h>

#include <algorithm>
#include <cwctype>
#include <utility>

namespace quietshare
{
namespace
{
bool SamePath(const std::wstring &a, const std::wstring &b)
{
    return CompareStringOrdinal(a.c_str(), static_cast<int>(a.size()), b.c_str(),
                                static_cast<int>(b.size()), TRUE) == CSTR_EQUAL;
}

bool Stamp(const std::wstring &path, FILETIME &write, std::uint64_t &size)
{
    HANDLE file = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    BY_HANDLE_FILE_INFORMATION info{};
    const bool ok = GetFileInformationByHandle(file, &info) && GetFileType(file) == FILE_TYPE_DISK &&
                    !(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    CloseHandle(file);
    if (ok)
    {
        write = info.ftLastWriteTime;
        size = (static_cast<std::uint64_t>(info.nFileSizeHigh) << 32) | info.nFileSizeLow;
    }
    return ok;
}

bool SameStamp(const FILETIME &a, std::uint64_t as, const FILETIME &b, std::uint64_t bs)
{
    return as == bs && CompareFileTime(&a, &b) == 0;
}

std::uint64_t Bytes(const LoadedContent &content)
{
    return content.text.size() * sizeof(wchar_t) + content.bytes.size() +
            (content.image ? static_cast<std::uint64_t>(content.width) * content.height * 4 : 0);
}

DocumentType Type(ContentKind kind)
{
    switch (kind)
    {
    case ContentKind::Text: return DocumentType::Text;
    case ContentKind::Code: return DocumentType::Code;
    case ContentKind::Image: return DocumentType::Image;
    case ContentKind::Pdf: return DocumentType::Pdf;
    }
    return DocumentType::Text;
}

std::wstring Lower(std::wstring value)
{
    std::transform(value.begin(), value.end(), value.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return value;
}

std::wstring Basename(const std::wstring &path)
{
    const auto separator = path.find_last_of(L"\\/");
    return path.substr(separator == std::wstring::npos ? 0 : separator + 1);
}

std::wstring Language(const std::wstring &name)
{
    const auto lower = Lower(name);
    const auto dot = lower.find_last_of(L'.');
    return dot == std::wstring::npos ? lower : lower.substr(dot + 1);
}

bool Fail(std::wstring &error, const wchar_t *message)
{
    error = message;
    return false;
}

bool Ignored(const WIN32_FIND_DATAW &entry)
{
    if (entry.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        return true;
    const auto name = Lower(entry.cFileName);
    return name == L".git" || name == L"node_modules" || name == L".venv" ||
            name == L"dist" || name == L"build";
}
} // namespace

TabManager::TabManager()
{
    documents_.reserve(MaxTabs);
    auto script = std::make_unique<Document>();
    script->displayName = L"Lời nhắc / Script";
    documents_.push_back(std::move(script));
}

Document *TabManager::Get(DocumentId id)
{
    for (auto &document : documents_)
        if (document->id == id)
            return document.get();
    return nullptr;
}

const Document *TabManager::Get(DocumentId id) const
{
    for (const auto &document : documents_)
        if (document->id == id)
            return document.get();
    return nullptr;
}

Document &TabManager::Active() { return *Get(active_); }
const Document &TabManager::Active() const { return *Get(active_); }

std::uint64_t TabManager::MemoryBytes() const
{
    std::uint64_t bytes = 0;
    for (const auto &document : documents_)
        bytes += Bytes(document->content)+document->draft.size()*sizeof(wchar_t);
    return bytes;
}

bool TabManager::Save(DocumentId id,std::wstring &error)
{
    auto* doc=Get(id);
    if(!doc||!id||(doc->type!=DocumentType::Text&&doc->type!=DocumentType::Code))
        return Fail(error,L"Chỉ sửa được văn bản và code. / Only text and code can be edited.");
    if(doc->draftUnavailable)return Fail(error,L"Code vượt giới hạn 5 MiB. Giảm nội dung hoặc bỏ bản sửa; file gốc được giữ nguyên. / Code exceeds 5 MiB. Reduce it or discard edits; the original file is preserved.");
    if(!doc->dirty){error.clear();return true;}
    FILETIME write{};std::uint64_t size=0;std::wstring canonical;
    if(!CanonicalLocalPath(doc->canonicalPath,false,canonical,error)||!SamePath(canonical,doc->canonicalPath)||
       !Stamp(canonical,write,size)||!SameStamp(doc->lastWrite,doc->fileSize,write,size))
        return Fail(error,L"File đã thay đổi bên ngoài hoặc không còn truy cập được. Bản sửa vẫn được giữ trong tab. / File changed externally or is unavailable. Your edits remain in the tab.");
    HANDLE original=CreateFileW(canonical.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(original==INVALID_HANDLE_VALUE)return Fail(error,L"Không đọc được file gốc. / Cannot read the original file.");
    if(size>5*1024*1024){CloseHandle(original);return Fail(error,L"File vượt giới hạn 5 MiB. / File exceeds 5 MiB.");}
    std::vector<unsigned char> bytes(static_cast<size_t>(size));DWORD read=0;
    bool ok=size<=5*1024*1024&&ReadFile(original,bytes.data(),static_cast<DWORD>(bytes.size()),&read,nullptr)&&read==size;
    CloseHandle(original);
    if(!ok)return Fail(error,L"Không đọc được file gốc. / Cannot read the original file.");
    const bool little=bytes.size()>=2&&bytes[0]==0xff&&bytes[1]==0xfe;
    const bool big=bytes.size()>=2&&bytes[0]==0xfe&&bytes[1]==0xff;
    const bool bom=bytes.size()>=3&&bytes[0]==0xef&&bytes[1]==0xbb&&bytes[2]==0xbf;
    // RichEdit returns CR line endings; keep the original document's convention.
    const auto& baseline=doc->content.text;
    const std::wstring newline=baseline.find(L"\r\n")!=std::wstring::npos?L"\r\n":baseline.find(L'\n')!=std::wstring::npos?L"\n":baseline.find(L'\r')!=std::wstring::npos?L"\r":L"\r\n";
    // The editor's draft is stored separately from the last disk content.
    std::wstring text=doc->draft,normalized;
    for(size_t i=0;i<text.size();++i){if(text[i]==L'\r'||text[i]==L'\n'){
        if(text[i]==L'\r'&&i+1<text.size()&&text[i+1]==L'\n')++i;
        normalized+=newline;
    }else normalized+=text[i];}
    const int length=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,normalized.data(),static_cast<int>(normalized.size()),nullptr,0,nullptr,nullptr);
    if(!length&&!normalized.empty())return Fail(error,L"Văn bản không hợp lệ. / Invalid text.");
    bytes.clear();
    if(little||big){bytes={static_cast<unsigned char>(little?0xff:0xfe),static_cast<unsigned char>(little?0xfe:0xff)};
        for(wchar_t c:normalized){bytes.push_back(static_cast<unsigned char>(little?c:c>>8));bytes.push_back(static_cast<unsigned char>(little?c>>8:c));}}
    else {
        if(bom)bytes={0xef,0xbb,0xbf};
        const size_t offset=bytes.size();bytes.resize(offset+length);
        if(length)WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,normalized.data(),static_cast<int>(normalized.size()),reinterpret_cast<char*>(bytes.data()+offset),length,nullptr,nullptr);
    }
    if(bytes.size()>5*1024*1024)return Fail(error,L"File vượt giới hạn 5 MiB. / File exceeds 5 MiB.");
    const auto temporary=canonical+L".quietshare-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64())+L".tmp";
    HANDLE file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE)return Fail(error,L"Không tạo được bản lưu tạm. / Cannot create a temporary save.");
    DWORD written=0;ok=WriteFile(file,bytes.data(),static_cast<DWORD>(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(file);CloseHandle(file);
    if(ok)ok=Stamp(canonical,write,size)&&SameStamp(doc->lastWrite,doc->fileSize,write,size)&&ReplaceFileW(canonical.c_str(),temporary.c_str(),nullptr,0,nullptr,nullptr);
    if(!ok){DeleteFileW(temporary.c_str());return Fail(error,L"Không lưu được file hoặc file đã thay đổi. Bản sửa vẫn ở trong tab. / Save failed or file changed. Your edits remain in the tab.");}
    doc->content.text=std::move(normalized);doc->draft.clear();doc->dirty=false;
    Stamp(canonical,doc->lastWrite,doc->fileSize);error.clear();return true;
}

bool TabManager::Load(Document &document, bool allowChanged, std::wstring &error)
{
    FILETIME before{}, after{};
    std::uint64_t beforeSize = 0, afterSize = 0;
    if (!Stamp(document.canonicalPath, before, beforeSize))
        return Fail(error, L"Không đọc được tệp. Kiểm tra quyền truy cập. / Cannot read the file. Check access permissions.");
    if (!allowChanged && !SameStamp(document.lastWrite, document.fileSize, before, beforeSize))
        return Fail(error, L"Tệp đã thay đổi. Mở lại tệp để chọn tải lại. / The file has changed. Reopen it to choose reload.");
    LoadedContent loaded;
    if (!LoadLocalContent(document.canonicalPath, loaded, error))
        return false;
    if (!Stamp(loaded.path, after, afterSize) || !SameStamp(before, beforeSize, after, afterSize))
        return Fail(error, L"Tệp đang thay đổi. Hãy thử mở lại. / The file is changing. Try reopening it.");
    if (MemoryBytes() - Bytes(document.content) + Bytes(loaded) > MemoryBudget)
        return Fail(error, L"Đã dùng hết 256 MiB bộ nhớ tài liệu. Đóng bớt tab rồi thử lại. / The 256 MiB document budget is full. Close some tabs and retry.");
    document.content = std::move(loaded);
    document.canonicalPath = document.content.path;
    document.displayName = document.content.displayName;
    document.language = Language(document.displayName);
    document.type = Type(document.content.kind);
    document.loadingState = LoadingState::Ready;
    document.lastWrite = after;
    document.fileSize = afterSize;
    document.draft.clear();document.dirty=false;document.draftUnavailable=false;
    error.clear();
    return true;
}

OpenResult TabManager::Open(const std::wstring &path, std::wstring &error)
{
    std::wstring canonical;
    if (!CanonicalLocalPath(path, false, canonical, error))
        return {};
    for (auto &document : documents_)
    {
        if (document->id == 0 || !SamePath(document->canonicalPath, canonical))
            continue;
        FILETIME write{};
        std::uint64_t size = 0;
        if (!Stamp(canonical, write, size))
        {
            Fail(error, L"Không đọc được tệp. Kiểm tra quyền truy cập. / Cannot read the file. Check access permissions.");
            return {};
        }
        if (!SameStamp(document->lastWrite, document->fileSize, write, size))
        {
            error.clear();
            return {OpenStatus::ReloadRequired, document->id};
        }
        if (!Activate(document->id, error))
            return {};
        return {OpenStatus::Existing, document->id};
    }
    if (Count() >= MaxTabs)
    {
        Fail(error, L"Tối đa 32 tab, gồm lời nhắc. Đóng bớt tab để mở tệp. / Up to 32 tabs, including the script. Close a tab to open another file.");
        return {};
    }
    auto document = std::make_unique<Document>();
    document->id = nextId_;
    document->canonicalPath = std::move(canonical);
    document->loadingState = LoadingState::Unloaded;
    if (!Load(*document, true, error))
        return {};
    document->state.textPoints = document->type == DocumentType::Code ? DefaultCodePoints : DefaultTextPoints;
    document->state.wrap = document->type != DocumentType::Code;
    ++nextId_;
    active_ = document->id;
    documents_.push_back(std::move(document));
    return {OpenStatus::Opened, active_};
}

bool TabManager::Activate(DocumentId id, std::wstring &error)
{
    Document *document = Get(id);
    if (!document)
        return Fail(error, L"Tab không còn tồn tại. / This tab is no longer open.");
    if (document->loadingState == LoadingState::Unloaded && !Load(*document, false, error))
        return false;
    active_ = id;
    error.clear();
    return true;
}

bool TabManager::Reload(DocumentId id, std::wstring &error)
{
    Document *document = Get(id);
    if (!document || id == 0)
        return Fail(error, L"Chọn tab tệp để tải lại. / Select a file tab to reload.");
    if (!Load(*document, true, error))
        return false;
    active_ = id;
    return true;
}

bool TabManager::Cycle(int direction, std::wstring &error)
{
    size_t index = 0;
    while (index < documents_.size() && documents_[index]->id != active_)
        ++index;
    const size_t next = direction < 0 ? (index + documents_.size() - 1) % documents_.size()
                                     : (index + 1) % documents_.size();
    return Activate(documents_[next]->id, error);
}

bool TabManager::Close(DocumentId id)
{
    if (id == 0)
        return false;
    const auto found = std::find_if(documents_.begin(), documents_.end(),
                                    [id](const auto &document) { return document->id == id; });
    if (found == documents_.end())
        return false;
    // Script is always ready, so closing cannot fail or accidentally reload a modified file.
    if (active_ == id)
        active_ = 0;
    documents_.erase(found);
    return true;
}

void TabManager::CloseOthers(DocumentId keep)
{
    if (!Get(keep))
        keep = 0;
    documents_.erase(std::remove_if(documents_.begin(), documents_.end(),
                                    [keep](const auto &document) { return document->id != 0 && document->id != keep; }),
                     documents_.end());
    if (!Get(active_))
        active_ = keep;
}

void TabManager::CloseAllFiles() { CloseOthers(0); }

void TabManager::EvictInactiveHeavy()
{
    for (auto &document : documents_)
        if (document->id != active_ && (document->type == DocumentType::Image || document->type == DocumentType::Pdf))
        {
            document->content.Clear();
            document->content.path = document->canonicalPath;
            document->content.displayName = document->displayName;
            document->content.kind = document->type == DocumentType::Image ? ContentKind::Image : ContentKind::Pdf;
            document->loadingState = LoadingState::Unloaded;
        }
}

ProjectTree::~ProjectTree()
{
    if (IsWindow(window_))
    {
        TreeView_SetImageList(window_, nullptr, TVSIL_NORMAL);
        DestroyWindow(window_);
    }
    if (icons_) ImageList_Destroy(icons_);
}

bool ProjectTree::Create(HWND parent, int controlId)
{
    if (IsWindow(window_)) return true;
    INITCOMMONCONTROLSEX common{sizeof(common), ICC_TREEVIEW_CLASSES};
    InitCommonControlsEx(&common);
    window_ = CreateWindowExW(0, WC_TREEVIEWW, L"", WS_CHILD | WS_TABSTOP | TVS_HASBUTTONS | TVS_LINESATROOT |
                              TVS_SHOWSELALWAYS | TVS_NOTOOLTIPS | TVS_DISABLEDRAGDROP | TVS_HASLINES,
                              0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId)),
                              GetModuleHandleW(nullptr), nullptr);
    if (!window_)
        return false;
    TreeView_SetBkColor(window_, RGB(37, 37, 38));
    TreeView_SetTextColor(window_, RGB(242, 245, 247));
    TreeView_SetLineColor(window_, RGB(52, 59, 66));
    SetWindowTheme(window_, L"DarkMode_Explorer", nullptr);
    SetWindowSubclass(window_, TreeProc, 1, reinterpret_cast<DWORD_PTR>(this));
    SetDpi(GetDpiForWindow(window_));
    return true;
}

void ProjectTree::SetDpi(UINT dpi)
{
    dpi = std::clamp(dpi ? dpi : 96, 48u, 768u);
    if (!window_) return;
    if (icons_ && dpi == iconDpi_ && iconSize_ == interfaceIconSize)
    {
        TreeView_SetImageList(window_, icons_, TVSIL_NORMAL);
        return;
    }
    const int size = MulDiv(interfaceIconSize, dpi, 96);
    const int count = static_cast<int>(FileIconKind::Count);
    // Explicit RGB + color-key mask also works with the classic common-control
    // image list; its 32-bit path does not reliably retain GDI-authored alpha.
    HIMAGELIST icons = ImageList_Create(size, size, ILC_COLOR24 | ILC_MASK, count, 0);
    if (icons) ImageList_SetBkColor(icons, CLR_NONE);
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bitmap{};
    bitmap.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap.bmiHeader.biWidth = size;
    bitmap.bmiHeader.biHeight = -size;
    bitmap.bmiHeader.biPlanes = 1;
    bitmap.bmiHeader.biBitCount = 32;
    void *pixels = nullptr;
    HBITMAP image = CreateDIBSection(dc, &bitmap, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!icons || !dc || !image || !pixels)
    {
        if (image) DeleteObject(image);
        if (dc) DeleteDC(dc);
        if (icons) ImageList_Destroy(icons);
        return;
    }
    const auto old = SelectObject(dc, image);
    for (int index = 0; index < count; ++index)
    {
        GdiFlush();
        std::fill_n(static_cast<DWORD *>(pixels), size * size, 0x00252526);
        DrawFileTypeIcon(dc, static_cast<FileIconKind>(index), RECT{0, 0, size, size}, false, false);
        GdiFlush();
        // Image-list import may read bitmap bits; do not leave it selected in a DC.
        SelectObject(dc, old);
        const int added = ImageList_AddMasked(icons, image, RGB(37,37,38));
        SelectObject(dc, image);
        if (added < 0)
        {
            ImageList_Destroy(icons);
            icons = nullptr;
            break;
        }
    }
    SelectObject(dc, old);
    DeleteObject(image);
    DeleteDC(dc);
    if (!icons) return;
    TreeView_SetImageList(window_, icons, TVSIL_NORMAL);
    if (icons_) ImageList_Destroy(icons_);
    icons_ = icons;
    iconDpi_ = dpi;
    iconSize_ = interfaceIconSize;
}

LRESULT CALLBACK ProjectTree::TreeProc(HWND window, UINT message, WPARAM wp, LPARAM lp, UINT_PTR,
                                      DWORD_PTR data)
{
    auto *tree = reinterpret_cast<ProjectTree *>(data);
    if (message == WM_DPICHANGED_AFTERPARENT) tree->SetDpi(GetDpiForWindow(window));
    if (message == WM_NCDESTROY)
    {
        RemoveWindowSubclass(window, TreeProc, 1);
        tree->window_ = nullptr;
    }
    const auto result = DefSubclassProc(window, message, wp, lp);
    // TVM_EXPAND can suppress repeat notifications after TVIS_EXPANDEDONCE.
    // Read the actual state after the native control handles the request.
    if (message == TVM_EXPAND) tree->UpdateFolderIcon(reinterpret_cast<HTREEITEM>(lp));
    return result;
}

void ProjectTree::UpdateFolderIcon(HTREEITEM handle)
{
    const auto found = nodes_.find(handle);
    if (found == nodes_.end() || !found->second->directory) return;
    const bool expanded = (TreeView_GetItemState(window_, handle, TVIS_EXPANDED) & TVIS_EXPANDED) != 0;
    TVITEMW item{};
    item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    item.hItem = handle;
    item.iImage = static_cast<int>(FileIconForName(L"", true, expanded));
    item.iSelectedImage = item.iImage;
    TreeView_SetItem(window_, &item);
}

void ProjectTree::Clear()
{
    if (IsWindow(window_))
        TreeView_DeleteAllItems(window_);
    nodes_.clear();
    root_.clear();
}

HTREEITEM ProjectTree::Insert(HTREEITEM parent, const std::wstring &name, std::wstring path, bool directory)
{
    if (nodes_.size() >= MaxNodes)
        return nullptr;
    auto node = std::make_unique<Node>();
    node->path = std::move(path);
    node->directory = directory;
    TVINSERTSTRUCTW insert{};
    insert.hParent = parent;
    insert.hInsertAfter = TVI_LAST;
    insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    insert.item.pszText = const_cast<wchar_t *>(name.c_str());
    insert.item.lParam = reinterpret_cast<LPARAM>(node.get());
    insert.item.cChildren = directory ? 1 : 0;
    insert.item.iImage = static_cast<int>(FileIconForName(name, directory));
    insert.item.iSelectedImage = insert.item.iImage;
    HTREEITEM item = TreeView_InsertItem(window_, &insert);
    if (item)
        nodes_.emplace(item, std::move(node));
    return item;
}

bool ProjectTree::OpenFolder(const std::wstring &path, std::wstring &error)
{
    std::wstring canonical;
    if (!window_ || !CanonicalLocalPath(path, true, canonical, error))
        return false;
    // Keep the old project intact until validation succeeds.
    Clear();
    root_ = std::move(canonical);
    const auto item = Insert(TVI_ROOT, Basename(root_).empty() ? root_ : Basename(root_), root_, true);
    if (!item)
    {
        Clear();
        return Fail(error, L"Không tạo được cây thư mục. / Cannot create the folder tree.");
    }
    TreeView_SelectItem(window_, item);
    error.clear();
    return true;
}

bool ProjectTree::SetIncludeIgnored(bool include, std::wstring &error)
{
    if (include == includeIgnored_)
    {
        error.clear();
        return true;
    }
    includeIgnored_ = include;
    const std::wstring root = root_;
    return root.empty() ? (error.clear(), true) : OpenFolder(root, error);
}

bool ProjectTree::Expand(HTREEITEM item, std::wstring &error)
{
    const auto found = nodes_.find(item);
    if (found == nodes_.end() || !found->second->directory || found->second->loaded)
        return true;
    Node *node = found->second.get();
    const DWORD attributes = GetFileAttributesW(node->path.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
        return Fail(error, L"Thư mục không còn khả dụng hoặc là liên kết. / The folder is unavailable or is a link.");
    WIN32_FIND_DATAW entry{};
    HANDLE search = FindFirstFileW((node->path + L"\\*").c_str(), &entry);
    if (search == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            node->loaded = true;
            return true;
        }
        return Fail(error, L"Không đọc được thư mục. Kiểm tra quyền truy cập. / Cannot read the folder. Check access permissions.");
    }
    struct Entry { std::wstring name; bool directory; };
    std::vector<Entry> entries;
    entries.reserve(MaxChildren);
    size_t examined = 0;
    bool limited = false;
    const ULONGLONG start = GetTickCount64();
    do
    {
        if (++examined > MaxChildren * 4 || entries.size() >= MaxChildren ||
            nodes_.size() + entries.size() >= MaxNodes || GetTickCount64() - start > 250)
        {
            limited = true;
            break;
        }
        if (wcscmp(entry.cFileName, L".") == 0 || wcscmp(entry.cFileName, L"..") == 0 ||
            (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) || (!includeIgnored_ && Ignored(entry)))
            continue;
        entries.push_back({entry.cFileName, (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0});
    } while (FindNextFileW(search, &entry));
    const DWORD searchError = GetLastError();
    FindClose(search);
    if (!limited && searchError != ERROR_NO_MORE_FILES)
        return Fail(error, L"Đọc thư mục bị gián đoạn. Hãy mở lại thư mục. / Folder enumeration was interrupted. Reopen the folder.");
    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
        if (a.directory != b.directory)
            return a.directory > b.directory;
        return CompareStringOrdinal(a.name.c_str(), -1, b.name.c_str(), -1, TRUE) == CSTR_LESS_THAN;
    });
    SendMessageW(window_, WM_SETREDRAW, FALSE, 0);
    for (const auto &entry : entries)
        Insert(item, entry.name, node->path + L"\\" + entry.name, entry.directory);
    node->loaded = true;
    SendMessageW(window_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(window_, nullptr, TRUE);
    if (limited)
        return Fail(error, L"Chỉ hiển thị tối đa 1.000 mục mỗi thư mục và 10.000 mục mỗi dự án. Mở thư mục con riêng nếu cần. / At most 1,000 entries per folder and 10,000 per project. Open a subfolder separately if needed.");
    error.clear();
    return true;
}

std::wstring ProjectTree::SelectedFile() const
{
    if (!IsWindow(window_))
        return {};
    const auto found = nodes_.find(TreeView_GetSelection(window_));
    return found != nodes_.end() && !found->second->directory ? found->second->path : L"";
}

bool ProjectTree::HandleNotify(const NMHDR *notification, std::wstring &file, std::wstring &error)
{
    file.clear();
    error.clear();
    if (!notification || notification->hwndFrom != window_)
        return false;
    if (notification->code == TVN_ITEMEXPANDINGW)
    {
        const auto *expanded = reinterpret_cast<const NMTREEVIEWW *>(notification);
        if (expanded->action == TVE_EXPAND)
            Expand(expanded->itemNew.hItem, error);
    }
    else if (notification->code == TVN_ITEMEXPANDEDW)
    {
        const auto *expanded = reinterpret_cast<const NMTREEVIEWW *>(notification);
        UpdateFolderIcon(expanded->itemNew.hItem);
    }
    else if (notification->code == NM_DBLCLK)
        file = SelectedFile();
    else if (notification->code == TVN_KEYDOWN &&
             reinterpret_cast<const NMTVKEYDOWN *>(notification)->wVKey == VK_RETURN)
        file = SelectedFile();
    return true;
}

} // namespace quietshare
