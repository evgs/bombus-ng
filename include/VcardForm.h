#pragma once

#include "HtmlView.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include <boost/shared_ptr.hpp>

class VcardForm : public HtmlView {
private:
    VcardForm(){};
public:
    VcardForm(HWND parent, const std::string &jid, ResourceContextRef rc);

    void vcardArrivedNotify(JabberDataBlockRef vcard);

    typedef boost::shared_ptr<VcardForm> ref;

    virtual void update();

protected:
    JabberDataBlockRef vcard;
    boost::weak_ptr<ResourceContext> rc;
    JabberDataBlockListenerRef listener;
};