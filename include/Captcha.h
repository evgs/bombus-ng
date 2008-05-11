#pragma once

#include "XDataForm.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include "boostheaders.h"

class CaptchaListener : public JabberDataBlockListener {
public:
    CaptchaListener(){};
    ~CaptchaListener(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "message"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

private:
    ResourceContextRef rc;
};


class CaptchaForm : public XDataForm {
protected:
    CaptchaForm(){};
public:
    virtual ~CaptchaForm();
    void CaptchaResultNotify(JabberDataBlockRef block);
    typedef boost::shared_ptr<CaptchaForm> ref;

    static CaptchaForm::ref createCaptchaForm(
        HWND parent, 
        const std::string &jid, 
        const std::string &id, 
        ResourceContextRef rc);

    void bindXData(JabberDataBlockRef xdata);

protected:
    virtual void onSubmit(JabberDataBlockRef replyForm);
    virtual void onCancel();

    std::string jid;
    std::string id;

    boost::weak_ptr<CaptchaForm> formRef;

    boost::weak_ptr<ResourceContext> rc;

    JabberDataBlockRef captchaData;
    //JabberDataBlockListenerRef listener;
};