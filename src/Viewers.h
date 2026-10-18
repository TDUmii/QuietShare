#pragma once
#include "Content.h"
#include <memory>
#include <string>

namespace quietshare
{

// State is kept by the document model for this session only.
struct ViewerState
{
    int scrollX = 0, scrollY = 0, zoom = 100, page = 0, panX = 0, panY = 0;
    bool wrap = false;
    bool fitWidth = false;
};

class CodeViewer
{
  public:
    CodeViewer();
    ~CodeViewer();
    CodeViewer(const CodeViewer &) = delete;
    CodeViewer &operator=(const CodeViewer &) = delete;
    bool Create(HWND parent, int id, std::wstring &error);
    HWND Window() const;
    bool Open(const LoadedContent &content, std::wstring &error);
    void Close();
    void SetPrivateVisible(bool safe);
    ViewerState GetState() const;
    void SetState(const ViewerState &state);
    void SetZoom(int percent);
    void SetWrap(bool enabled);
    void SetEditable(bool enabled);
    bool ReadText(std::wstring& text) const;
    bool Modified() const;
    void MarkSaved();
    bool Find(const std::wstring &query, bool forward = true);
    int TokenStyleAt(int byteOffset) const;
    std::wstring Language() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

class PdfViewer
{
  public:
    PdfViewer();
    ~PdfViewer();
    PdfViewer(const PdfViewer &) = delete;
    PdfViewer &operator=(const PdfViewer &) = delete;
    bool Create(HWND parent, int id, std::wstring &error);
    HWND Window() const;
    bool Open(const LoadedContent &content, std::wstring &error);
    void Close();
    void SetPrivateVisible(bool safe);
    ViewerState GetState() const;
    void SetState(const ViewerState &state);
    void SetZoom(int percent);
    void FitPage();
    void FitWidth();
    bool GoToPage(int zeroBasedPage);
    int PageCount() const;
    bool HasText() const;
    bool Find(const std::wstring &query, bool forward = true);
    bool CopySelection(); // Only call in response to an explicit copy command.
    std::wstring SelectedText() const;
    size_t CachedBytes() const;
    bool RenderCurrentPage(); // Also used by native regression tests.
  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace quietshare
