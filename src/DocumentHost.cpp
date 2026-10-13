#include "DocumentHost.h"
namespace quietshare {
bool DocumentHost::Create(HWND parent,HWND text,HWND image,std::wstring& error) {
    text_=text; image_=image;
    return code.Create(parent,170,error) && pdf.Create(parent,171,error);
}
void DocumentHost::SetBounds(const RECT& r) {
    for(HWND h:{text_,image_,code.Window(),pdf.Window()})
        if(h) MoveWindow(h,r.left,r.top,r.right-r.left,r.bottom-r.top,TRUE);
}
void DocumentHost::Show(DocumentType type,bool safe,bool contentPage) {
    bool visible=safe && contentPage;
    ShowWindow(text_,visible && (type==DocumentType::Script || type==DocumentType::Text)?SW_SHOW:SW_HIDE);
    ShowWindow(image_,visible && type==DocumentType::Image?SW_SHOW:SW_HIDE);
    code.SetPrivateVisible(visible && type==DocumentType::Code);
    pdf.SetPrivateVisible(visible && type==DocumentType::Pdf);
}
void DocumentHost::Hide() { Show(DocumentType::Script,false,false); }
void DocumentHost::CloseExternal() { code.Close(); pdf.Close(); }
HWND DocumentHost::ActiveWindow(DocumentType type) const {
    return type==DocumentType::Code?code.Window():type==DocumentType::Image?image_:
           type==DocumentType::Pdf?pdf.Window():text_;
}
}
