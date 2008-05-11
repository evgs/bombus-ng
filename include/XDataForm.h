#pragma once

#include "HtmlView.h"
#include "JabberDataBlock.h"
#include "ResourceContext.h"
#include "boostheaders.h"

class XDataForm : public HtmlView {
protected:
    XDataForm(){};
public:
    typedef boost::shared_ptr<XDataForm> ref;
    virtual void onWmUserUpdate();
#ifdef DEBUG
    static XDataForm::ref createXDataForm(HWND parent, const std::string &title, ResourceContextRef rc);
    void formTest();
#endif

protected:
    virtual HBITMAP getImage(LPCTSTR url, DWORD cookie);

    virtual void onSubmit(JabberDataBlockRef replyForm);

    virtual void onHotSpot(LPCSTR url, LPCSTR param);

    virtual void constructForm();

    boost::weak_ptr<XDataForm> formRef;

    JabberDataBlockRef xdata;
    std::string plainText;

    ImageRef img;

    boost::weak_ptr<ResourceContext> rc;
};
