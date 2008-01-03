#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "MucConfigForm.h"
#include "JabberStream.h"
#include "XmppError.h"

#include "wmuser.h"
#include "utf8.hpp"
#include "base64.h"
#include "../gsgetfile/include/gsgetlib.h"

class GetMucConfig : public JabberDataBlockListener {
public:
    GetMucConfig(const std::string &jid, MucConfigForm::ref form);
    ~GetMucConfig(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc, JabberDataBlockRef childData);

private:
    std::string jid;
    std::string id;
    ResourceContextRef rc;
    boost::weak_ptr<MucConfigForm> ahf;
};

GetMucConfig::GetMucConfig(const std::string &jid, MucConfigForm::ref form){
    this->jid=jid;
    this->ahf=form;
}

void GetMucConfig::doRequest( ResourceContextRef rc, JabberDataBlockRef childData ) 
{
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);

    JabberDataBlockRef qry=req.addChildNS("query", "http://jabber.org/protocol/muc#owner");

    if (childData) { 
        qry->addChild(childData);
        id="save#";
        req.setAttribute("type", "set");
    } else {
        id="get#";
        req.setAttribute("type", "get");
    }

    req.setAttribute("id", id);

    StringRef s=req.toXML();
    rc->jabberStream->sendStanza(req);
}

ProcessResult GetMucConfig::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    MucConfigForm::ref ahfRef=ahf.lock();
    if (!ahfRef) return CANCEL;

    //StringRef s=block->toXML();
    ahfRef->MucConfigResultNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
MucConfigForm::ref MucConfigForm::createMucConfigForm(HWND parent, const std::string &roomJid, ResourceContextRef rc) {
    MucConfigForm *af=new MucConfigForm();

    af->parentHWnd=parent;
    af->title=utf8::utf8_wchar(roomJid);
    af->wt=WndTitleRef(new WndTitle(af, icons::ICON_AD_HOC));

    af->init();
    
    SetParent(af->thisHWnd, parent);

    af->rc=rc;
    af->roomJid=roomJid;

    MucConfigForm::ref afRef=MucConfigForm::ref(af);
    af->formRef=afRef;

    /*GetMucConfig *getah=new GetMucConfig(jid, node, afRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getah));
    getah->doRequest(rc, "", "execute", JabberDataBlockRef());*/
    af->sendIq(JabberDataBlockRef());

    return afRef;
}

void MucConfigForm::MucConfigResultNotify(JabberDataBlockRef block) {
    if (block->getAttribute("type")=="error") {
        XmppError::ref xe= XmppError::findInStanza(block);
        //todo: error handling
        MessageBox(getHWnd(), utf8::utf8_wchar(xe->toString()).c_str() , L"MucConfig", MB_OK | MB_ICONEXCLAMATION );
        return;
    }


    JabberDataBlockRef query=block->findChildNamespace("query","http://jabber.org/protocol/muc#owner");
    this->xdata=query->findChildNamespace("x","jabber:x:data");

    //non-descriptive result
    if (!xdata && plainText.empty())
        plainText="done"; //todo: make more informative and localizabe

    PostMessage(getHWnd(), WM_HTML_UPDATE, 0, (LPARAM)"");
    return;
}

void MucConfigForm::onSubmit( JabberDataBlockRef replyForm ) {
    sendIq(replyForm);
}

void MucConfigForm::onCancel() {}

void MucConfigForm::sendIq(JabberDataBlockRef childData ) {
    ResourceContextRef rc=this->rc.lock();
    if (!rc) return;

    GetMucConfig *getah=new GetMucConfig(roomJid, formRef.lock());
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getah));
    getah->doRequest(rc, childData);
}

MucConfigForm::~MucConfigForm() {
    onCancel();
}