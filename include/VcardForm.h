#pragma once

#include "HtmlView.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include <boost/shared_ptr.hpp>

class VcardForm : public HtmlView {
private:
    VcardForm(){};
public:
    void vcardArrivedNotify(JabberDataBlockRef vcard);
    typedef boost::shared_ptr<VcardForm> ref;
    virtual void update();

    static VcardForm::ref createVcardForm(HWND parent, const std::string &jid, ResourceContextRef rc);

    virtual HBITMAP getImage(LPCTSTR url);

protected:
    void loadPhoto();
    ImageRef img;

    enum FieldType {
        URL=1
    };
    void addHtmlField(const char *ns1, const char *ns2, const wchar_t* description, int flags=0);

    boost::weak_ptr<VcardForm> formRef;

    JabberDataBlockRef vcard;
    boost::weak_ptr<ResourceContext> rc;
    JabberDataBlockListenerRef listener;
};