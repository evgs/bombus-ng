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
    virtual void onWmUserUpdate();

    static VcardForm::ref createVcardForm(HWND parent, const std::string &jid, ResourceContextRef rc, bool edit);

    virtual HBITMAP getImage(LPCTSTR url);

protected:
    void loadPhoto();
    ImageRef img;
    bool editForm;

    enum FieldType {
        TXT=0,
        URL=1,
        TEXTBOX=2,
        MULTILINE=4
    };

    void addHtmlField(const char *ns1, const char *ns2, const char* description, int flags=TXT);

    boost::weak_ptr<VcardForm> formRef;

    JabberDataBlockRef vcard;
    boost::weak_ptr<ResourceContext> rc;
    JabberDataBlockListenerRef listener;
};