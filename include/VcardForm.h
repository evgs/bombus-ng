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


protected:
    std::string jid;

    virtual HBITMAP getImage(LPCTSTR url, DWORD cookie);
    virtual void onHotSpot(LPCTSTR url, LPCTSTR param);

    void decodePhoto();
    void loadPhoto(LPCTSTR path);

    ImageRef img;
    DWORD cookie;

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