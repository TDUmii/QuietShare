#pragma once
#include "Viewers.h"
#include "Documents.h"
namespace quietshare {
// One host owns visibility for every private child window. The caller supplies
// the result of the common capture/privacy gate; viewers never override it.
class DocumentHost {
public:
    CodeViewer code;
    PdfViewer pdf;
    bool Create(HWND parent, HWND text, HWND image, std::wstring& error);
    void SetBounds(const RECT& rect);
    void Show(DocumentType type, bool privateSafe, bool contentPage);
    void Hide();
    void CloseExternal();
    HWND ActiveWindow(DocumentType type) const;
private:
    HWND text_ = nullptr, image_ = nullptr;
};
}
