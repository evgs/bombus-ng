#pragma once

#include "HtmlView.h"
#include "JabberDataBlock.h"
#include "ResourceContext.h"
#include <boost/shared_ptr.hpp>

class XDataForm : public HtmlView {
private:
    XDataForm(){};
public:
    void vcardArrivedNotify(JabberDataBlockRef vcard);
    typedef boost::shared_ptr<XDataForm> ref;
    virtual void onWmUserUpdate();

    static XDataForm::ref createXDataForm(HWND parent, const std::string &jid, ResourceContextRef rc);

    void formTest();

protected:
    std::string jid;

    virtual void onHotSpot(LPCSTR url, LPCSTR param);

    boost::weak_ptr<XDataForm> formRef;

    JabberDataBlockRef xdata;
    boost::weak_ptr<ResourceContext> rc;
};