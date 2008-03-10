#pragma once

#include "XDataForm.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include "boostheaders.h"

class MucConfigForm : public XDataForm {
protected:
    MucConfigForm(){};
public:
    virtual ~MucConfigForm();
    void MucConfigResultNotify(JabberDataBlockRef block);
    typedef boost::shared_ptr<MucConfigForm> ref;

    static MucConfigForm::ref createMucConfigForm(HWND parent, const std::string &roomJid, ResourceContextRef rc);

protected:
    virtual void onSubmit(JabberDataBlockRef replyForm);
    virtual void onCancel();

    void sendIq(JabberDataBlockRef childData);

    std::string roomJid;

    boost::weak_ptr<MucConfigForm> formRef;

    boost::weak_ptr<ResourceContext> rc;
    //JabberDataBlockListenerRef listener;
};