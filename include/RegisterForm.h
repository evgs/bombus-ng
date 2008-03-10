#pragma once

#include "XDataForm.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include "boostheaders.h"

class RegisterForm : public XDataForm {
protected:
    RegisterForm(){};
public:
    virtual ~RegisterForm();
    void RegisterResultNotify(JabberDataBlockRef block);
    typedef boost::shared_ptr<RegisterForm> ref;

    static RegisterForm::ref createRegisterForm(HWND parent, const std::string &jid, ResourceContextRef rc);

protected:
    virtual void onHotSpot(LPCSTR url, LPCSTR param);
    
    virtual void onSubmit(JabberDataBlockRef replyForm);
    virtual void onCancel();

    virtual void constructForm();

    std::string jid;
    std::string pass;

    boost::weak_ptr<RegisterForm> formRef;

    boost::weak_ptr<ResourceContext> rc;

    JabberDataBlockRef iqRegisterData;
    //JabberDataBlockListenerRef listener;
};