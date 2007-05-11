#pragma once

#include "XDataForm.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include <boost/shared_ptr.hpp>

class AdHocForm : public XDataForm {
protected:
    AdHocForm(){};
public:
    virtual ~AdHocForm();
    void AdHocResultNotify(JabberDataBlockRef block);
    typedef boost::shared_ptr<AdHocForm> ref;

    static AdHocForm::ref createAdHocForm(HWND parent, const std::string &jid, const std::string &node, ResourceContextRef rc);

protected:
    virtual void onSubmit(JabberDataBlockRef replyForm);
    virtual void onCancel();

    void sendCommand(const std::string &command, JabberDataBlockRef childData);

    std::string jid;
    std::string node;

    std::string sessionId;
    std::string status;

    boost::weak_ptr<AdHocForm> formRef;

    boost::weak_ptr<ResourceContext> rc;
    //JabberDataBlockListenerRef listener;
};