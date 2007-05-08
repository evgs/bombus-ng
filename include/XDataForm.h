#pragma once

#include "HtmlView.h"
#include "JabberDataBlock.h"
#include "ResourceContext.h"
#include <boost/shared_ptr.hpp>

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
    virtual void onSubmit(JabberDataBlockRef replyForm);

    virtual void onHotSpot(LPCSTR url, LPCSTR param);

    boost::weak_ptr<XDataForm> formRef;

    JabberDataBlockRef xdata;
    std::string plainText;

    boost::weak_ptr<ResourceContext> rc;
};
