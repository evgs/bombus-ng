#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "RegisterForm.h"
#include "JabberStream.h"

#include "utf8.hpp"
#include "base64.h"

class IqRegister : public JabberDataBlockListener {
public:
    IqRegister(const std::string &jid, RegisterForm::ref form);
    ~IqRegister(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc, JabberDataBlockRef childData);

private:
    std::string jid;
    std::string id;
    ResourceContextRef rc;
    boost::weak_ptr<RegisterForm> rf;
};

IqRegister::IqRegister(const std::string &jid, RegisterForm::ref form){
    this->jid=jid;
    this->rf=form;
    id="reg";
}

void IqRegister::doRequest(ResourceContextRef rc, JabberDataBlockRef childData) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);

    if (childData) {
        req.setAttribute("type", "set");
        req.addChild(childData);
    } else {
        req.setAttribute("type", "get");
        id="reg#get";
        req.addChild("query", NULL)->setAttribute("xmlns", "jabber:iq:register");
    }
    req.setAttribute("id", id);

    rc->jabberStream->sendStanza(req);
}

ProcessResult IqRegister::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    RegisterForm::ref regFRef=rf.lock();

    if (regFRef)
        regFRef->RegisterResultNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
RegisterForm::ref RegisterForm::createRegisterForm(HWND parent, const std::string &jid, ResourceContextRef rc){
    RegisterForm *af=new RegisterForm();

    af->parentHWnd=parent;
    af->title=utf8::utf8_wchar(jid);
    af->wt=WndTitleRef(new WndTitle(af, icons::ICON_AD_HOC));

    af->init();
    
    SetParent(af->thisHWnd, parent);

    af->rc=rc;
    af->jid=jid;
    //af->pass=pass;

    RegisterForm::ref afRef=RegisterForm::ref(af);
    af->formRef=afRef;

    IqRegister *regListener=new IqRegister(jid, afRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(regListener));
    regListener->doRequest(rc, JabberDataBlockRef());

    return afRef;
}

void RegisterForm::RegisterResultNotify(JabberDataBlockRef block) {
    if (block->getAttribute("type")=="error") {
        //todo: error handling
        MessageBox(getHWnd(), L"Unhandled iq:register error", L"Register", MB_OK | MB_ICONEXCLAMATION );
        return;
    }


    JabberDataBlockRef qryRegister=block->findChildNamespace("query", "jabber:iq:register");
    if (qryRegister) {
        this->xdata=qryRegister->findChildNamespace("x","jabber:x:data");
    } else {
        plainText="iq:register is not fully supported yet";
        xdata.reset();
    }

    PostMessage(getHWnd(), WM_USER, 0, (LPARAM)"");
    return;
}

void RegisterForm::onSubmit( JabberDataBlockRef replyForm ) {
    //if (status!="executing") return;
    //sendCommand("execute", replyForm);
}

void RegisterForm::onCancel() {
    //if (status!="executing") return;
    //sendCommand("cancel", JabberDataBlockRef());
}

RegisterForm::~RegisterForm() {
    onCancel();
}