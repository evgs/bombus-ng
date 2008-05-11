#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "wmuser.h"
#include "TabCtrl.h"
#include "Captcha.h"
#include "JabberStream.h"
#include "XmppError.h"

#include "utf8.hpp"
#include "base64.h"

extern TabsCtrlRef tabs;


ProcessResult CaptchaListener::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef challenge=block->findChildNamespace("challenge", "urn:xmpp:tmp:challenge");

    if (!challenge) return BLOCK_REJECTED;

    JabberDataBlockRef xdata=challenge->findChildNamespace("x", "jabber:x:data");

    const std::string &jid=block->getAttribute("from");
    const std::string &id=block->getAttribute("id");

    CaptchaForm::ref rform=CaptchaForm::createCaptchaForm(tabs->getHWnd(), jid, id, rc);
    
    tabs->addWindow(rform);
    tabs->switchByWndRef(rform);

    rform->bindXData(xdata);

    return BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
CaptchaForm::ref CaptchaForm::createCaptchaForm(HWND parent, const std::string &jid, const std::string &id, ResourceContextRef rc){
    CaptchaForm *af=new CaptchaForm();

    af->parentHWnd=parent;
    af->title=utf8::utf8_wchar(jid);
    af->wt=WndTitleRef(new WndTitle(af, icons::ICON_AD_HOC));

    af->init();
    
    SetParent(af->thisHWnd, parent);

    af->rc=rc;
    af->jid=jid;
    af->id=id;

    CaptchaForm::ref afRef=CaptchaForm::ref(af);
    af->formRef=afRef;

    /*IqRegister *regListener=new IqRegister(jid, afRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(regListener));
    regListener->doRequest(rc, JabberDataBlockRef());*/

    return afRef;
}

void CaptchaForm::bindXData( JabberDataBlockRef xdata ) {
    this->xdata=xdata;
    constructForm();
}

/*void RegisterForm::RegisterResultNotify(JabberDataBlockRef block) {
    if (block->getAttribute("type")=="error") {
        XmppError::ref xe= XmppError::findInStanza(block);
        //todo: error handling
        MessageBox(getHWnd(), utf8::utf8_wchar(xe->toString()).c_str() , L"Registering error", MB_OK | MB_ICONEXCLAMATION );
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
}*/

void CaptchaForm::onSubmit( JabberDataBlockRef replyForm ) {

    JabberDataBlock answer("iq", NULL);
    answer.setAttribute("type", "set");
    answer.setAttribute("id", id);
    answer.setAttribute("to", jid);

    answer.addChildNS("challenge", "urn:xmpp:tmp:challenge")->addChild(replyForm);
    
    ResourceContextRef rc=this->rc.lock();
    if (rc) {
        rc->jabberStream->sendStanza(answer);

        startHtml();
        addText("Done");
        endForm();

    }
}

void CaptchaForm::onCancel() {
    //if (status!="executing") return;
    //sendCommand("cancel", JabberDataBlockRef());
}

CaptchaForm::~CaptchaForm() {
    onCancel();
}
