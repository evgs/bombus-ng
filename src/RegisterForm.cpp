#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "wmuser.h"
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
        req.addChildNS("query", "jabber:iq:register");
    }
    req.setAttribute("id", id);

#ifdef DEBUG
    StringRef out=req.toXML();
#endif

    rc->jabberStream->sendStanza(req);
}

ProcessResult IqRegister::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    RegisterForm::ref regFRef=rf.lock();
    if (!regFRef) return CANCEL;

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

//REG_FILTER "registered" "key" "instructions" "x"
void RegisterForm::constructForm() {
    if (xdata) XDataForm::constructForm();

    else if (iqRegisterData) {
        //instructions
        const std::string &instr=iqRegisterData->getChildText("instructions");
        addText("<i>"); addText(instr); addText("</i><br/>");

        beginForm("jabber:iq:register","send");
        //child fields
        JabberDataBlockRefList *childs=iqRegisterData->getChilds();

        JabberDataBlockRefList::const_iterator i;
        for (i=childs->begin(); i!=childs->end(); i++) {
            JabberDataBlockRef field=*i;
            const std::string &tagname=field->getTagName();
            if (tagname=="registered") continue;
            if (tagname=="key") continue;
            if (tagname=="instructions") continue;
            if (tagname=="x") continue;

            const std::string &value=field->getText();

            if (tagname=="password") 
                passBox(tagname.c_str(), tagname.c_str(), value);
            else
                textBox(tagname.c_str(), tagname.c_str(), value);
        }
        button("Register");
        endForm();
    }
    
    if (iqRegisterData) if (iqRegisterData->getChildByName("registered")) {
        button("unregister","Remove registration");
    }
}

void RegisterForm::RegisterResultNotify(JabberDataBlockRef block) {
    if (block->getAttribute("type")=="error") {
        //todo: error handling
        MessageBox(getHWnd(), L"Unhandled iq:register error", L"Register", MB_OK | MB_ICONEXCLAMATION );
        return;
    }

    //todo: ok result handling
    JabberDataBlockRef qryRegister=block->findChildNamespace("query", "jabber:iq:register");
    if (qryRegister) {
        this->xdata=qryRegister->findChildNamespace("x","jabber:x:data");
    } else {
        xdata.reset();
    }
    this->iqRegisterData=qryRegister;

    PostMessage(getHWnd(), WM_HTML_UPDATE, 0, (LPARAM)"");
    return;
}

void RegisterForm::onSubmit( JabberDataBlockRef replyForm ) {
    JabberDataBlockRef reply=JabberDataBlockRef(new JabberDataBlock(xdata->getTagName().c_str(),NULL));
    reply->setAttribute("xmlns","jabber:x:data");
    reply->addChild(replyForm);

    IqRegister *regListener=new IqRegister(jid, formRef.lock());
    ResourceContextRef rc=this->rc.lock();
    if (rc) {
        rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(regListener));
        regListener->doRequest(rc, reply);
    }
}

void RegisterForm::onCancel() {
    //if (status!="executing") return;
    //sendCommand("cancel", JabberDataBlockRef());
}

RegisterForm::~RegisterForm() {
    onCancel();
}

void RegisterForm::onHotSpot( LPCSTR url, LPCSTR param ) {
    JabberDataBlockRef reply=JabberDataBlockRef(new JabberDataBlock("query",NULL));
    reply->setAttribute("xmlns","jabber:iq:register");

    if (strcmp(url, "unregister")==0) {
        reply->addChild("remove",NULL);

        IqRegister *regListener=new IqRegister(jid, formRef.lock());
        ResourceContextRef rc=this->rc.lock();
        if (rc) {
            rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(regListener));
            regListener->doRequest(rc, reply);
        }

        return;
    }

    if (xdata) {
        XDataForm::onHotSpot(url, param);
        return;
    }


    StringMapRef result=splitHREFtext(param);

    //constructing result
    JabberDataBlockRefList *childs=iqRegisterData->getChilds();

    JabberDataBlockRefList::const_iterator i;
    for (i=childs->begin(); i!=childs->end(); i++) {
        JabberDataBlockRef field=*i;
        const std::string &tagname=field->getTagName();
        const std::string &value=result->operator [](tagname);

        if (tagname=="registered") continue;
        if (tagname=="instructions") continue;
        if (tagname=="x") continue;
        //copy key
        if (tagname=="key") { reply->addChild(field); continue; }

        reply->addChild(tagname.c_str(), value.c_str());
    }

    //send
    IqRegister *regListener=new IqRegister(jid, formRef.lock());
    ResourceContextRef rc=this->rc.lock();
    if (rc) {
        rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(regListener));
        regListener->doRequest(rc, reply);
    }
}