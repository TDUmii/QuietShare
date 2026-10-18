#pragma once

#include "Content.h"

#include <commctrl.h>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace quietshare
{

using DocumentId = std::uint64_t;
enum class DocumentType { Script, Text, Code, Image, Pdf };
enum class LoadingState { Unloaded, Ready };

struct ViewState
{
    int scrollX = 0, scrollY = 0;
    int zoom = 100;
    int page = 0;
    int panX = 0, panY = 0;
    int textPoints = 24;
    bool wrap = true;
    bool fitWidth = false;
};

struct Document
{
    DocumentId id = 0;
    DocumentType type = DocumentType::Script;
    std::wstring canonicalPath, displayName, language;
    LoadingState loadingState = LoadingState::Ready;
    LoadedContent content;
    ViewState state;
    bool dirty = false;
    bool draftUnavailable = false;
    std::wstring draft;
    FILETIME lastWrite{};
    std::uint64_t fileSize = 0;
};

enum class OpenStatus { Opened, Existing, ReloadRequired, Rejected };
struct OpenResult
{
    OpenStatus status = OpenStatus::Rejected;
    DocumentId id = 0;
    explicit operator bool() const { return status == OpenStatus::Opened || status == OpenStatus::Existing; }
};

// RichEdit uses CR while disk text may use LF or CRLF. This is not an edit.
inline bool SameDocumentText(const std::wstring& a,const std::wstring& b) {
    size_t i=0,j=0;
    auto next=[](const std::wstring& text,size_t& at){wchar_t c=text[at++];
        if(c==L'\r'){if(at<text.size()&&text[at]==L'\n')++at;c=L'\n';}return c;};
    while(i<a.size()&&j<b.size())if(next(a,i)!=next(b,j))return false;
    return i==a.size()&&j==b.size();
}

// Script id 0 is permanent. Editable file drafts stay in memory for the session.
// Recent paths are stored separately by the encrypted local workspace profile.
// Close the active native viewer before closing/reloading/evicting its document.
class TabManager
{
public:
    static constexpr size_t MaxTabs = 32; // includes the script
    static constexpr std::uint64_t MemoryBudget = 256ULL * 1024 * 1024;

    TabManager();
    Document &Active();
    const Document &Active() const;
    Document *Get(DocumentId id);
    const Document *Get(DocumentId id) const;
    const std::vector<std::unique_ptr<Document>> &Items() const { return documents_; }
    size_t Count() const { return documents_.size(); }
    std::uint64_t MemoryBytes() const;
    OpenResult Open(const std::wstring &path, std::wstring &error);
    bool Activate(DocumentId id, std::wstring &error);
    bool Activate(DocumentId id) { std::wstring error; return Activate(id, error); }
    bool Reload(DocumentId id, std::wstring &error);
    bool Save(DocumentId id, std::wstring &error);
    bool Cycle(int direction, std::wstring &error);
    bool Close(DocumentId id);
    void CloseOthers(DocumentId keep);
    void CloseAllFiles();
    // Call after switching/closing old viewers. Images/PDFs reload only when activated.
    void EvictInactiveHeavy();

private:
    std::vector<std::unique_ptr<Document>> documents_;
    DocumentId active_ = 0, nextId_ = 1;
    bool Load(Document &document, bool allowChanged, std::wstring &error);
};

// Opt-in native lazy tree. Hide the entire HWND via the common privacy gate.
// Handles expansion internally. Double-click/Enter returns a local file to open.
class ProjectTree
{
public:
    static constexpr size_t MaxChildren = 1000;
    static constexpr size_t MaxNodes = 10000;
    ProjectTree() = default;
    ~ProjectTree();
    ProjectTree(const ProjectTree &) = delete;
    ProjectTree &operator=(const ProjectTree &) = delete;
    bool Create(HWND parent, int controlId);
    void SetDpi(UINT dpi);
    HWND Handle() const { return window_; }
    bool OpenFolder(const std::wstring &path, std::wstring &error);
    bool SetIncludeIgnored(bool include, std::wstring &error);
    bool IncludeIgnored() const { return includeIgnored_; }
    const std::wstring &Root() const { return root_; }
    size_t NodeCount() const { return nodes_.size(); }
    std::wstring SelectedFile() const;
    bool HandleNotify(const NMHDR *notification, std::wstring &file, std::wstring &error);
    void Clear();

private:
    struct Node
    {
        std::wstring path;
        bool directory = false, loaded = false;
    };
    HWND window_ = nullptr;
    HIMAGELIST icons_ = nullptr;
    UINT iconDpi_ = 0;
    int iconSize_ = 0;
    std::wstring root_;
    bool includeIgnored_ = false;
    std::unordered_map<HTREEITEM, std::unique_ptr<Node>> nodes_;
    HTREEITEM Insert(HTREEITEM parent, const std::wstring &name, std::wstring path, bool directory);
    bool Expand(HTREEITEM item, std::wstring &error);
    void UpdateFolderIcon(HTREEITEM item);
    static LRESULT CALLBACK TreeProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
};

} // namespace quietshare
