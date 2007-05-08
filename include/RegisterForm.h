#pragma once

#include "XDataForm.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include <boost/shared_ptr.hpp>

class RegisterForm : public XDataForm {
protected:
    RegisterForm(){};
public:
    virtual ~RegisterForm();
    void RegisterResultNotify(JabberDataBlockRef block);
    typedef boost::shared_ptr<RegisterForm> ref;

    static RegisterForm::ref createRegisterForm(HWND parent, const std::string &jid, ResourceContextRef rc);

protected:
    virtual void onSubmit(JabberDataBlockRef replyForm);
    virtual void onCancel();

    void sendCommand(const std::string &command, JabberDataBlockRef childData);

    std::string jid;
    std::string pass;

    boost::weak_ptr<RegisterForm> formRef;

    boost::weak_ptr<ResourceContext> rc;
    //JabberDataBlockListenerRef listener;
};