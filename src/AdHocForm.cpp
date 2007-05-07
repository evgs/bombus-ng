#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "AdHocForm.h"
#include "JabberStream.h"

#include "utf8.hpp"
#include "base64.h"
#include "../gsgetfile/include/gsgetlib.h"

class GetAdHoc : public JabberDataBlockListener {
public:
    GetAdHoc(const std::string &jid, const std::string &node, AdHocForm::ref form);
    ~GetAdHoc(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc, const std::string &sid, const std::string &action, JabberDataBlockRef childData);

private:
    std::string jid;
    std::string node;
    std::string id;
    ResourceContextRef rc;
    boost::weak_ptr<AdHocForm> ahf;
};

GetAdHoc::GetAdHoc(const std::string &jid, const std::string &node, AdHocForm::ref form){
    this->jid=jid;
    this->node=node;
    this->ahf=form;
    id="adhoc#";
    id+=jid;
    id+='#';
    id+=node;
}

void GetAdHoc::doRequest(ResourceContextRef rc, const std::string &sid, const std::string &action, JabberDataBlockRef childData) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "set");
    req.setAttribute("id", id);

    JabberDataBlockRef cmd=req.addChild("command", NULL);
    cmd->setAttribute("xmlns","http://jabber.org/protocol/commands");
    cmd->setAttribute("node", node);
    if (sid.length()) cmd->setAttribute("sessionid", sid);
    if (action.length()) cmd->setAttribute("action", action);
    if (childData) cmd->addChild(childData);

    rc->jabberStream->sendStanza(req);
}

ProcessResult GetAdHoc::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    AdHocForm::ref ahfRef=ahf.lock();

    if (ahfRef)
        ahfRef->AdHocResultNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
AdHocForm::ref AdHocForm::createAdHocForm(HWND parent, const std::string &jid, const std::string node, ResourceContextRef rc) {
    AdHocForm *af=new AdHocForm();

    af->parentHWnd=parent;
    af->title=utf8::utf8_wchar(jid);
    af->wt=WndTitleRef(new WndTitle(af, icons::ICON_AD_HOC));

    af->init();
    
    SetParent(af->thisHWnd, parent);

    af->rc=rc;
    af->jid=jid;
    af->node=node;

    AdHocForm::ref afRef=AdHocForm::ref(af);
    af->formRef=afRef;

    /*GetAdHoc *getah=new GetAdHoc(jid, node, afRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getah));
    getah->doRequest(rc, "", "execute", JabberDataBlockRef());*/
    af->sendCommand("execute", JabberDataBlockRef());

    return afRef;
}

void AdHocForm::AdHocResultNotify(JabberDataBlockRef block) {
    if (block->getAttribute("type")=="error") {
        //todo: error handling
        MessageBox(getHWnd(), L"Unhandled ad-hoc error", L"AdHoc", MB_OK | MB_ICONEXCLAMATION );
        return;
    }


    JabberDataBlockRef command=block->getChildByName("command");
    plainText=command->getChildText("note"); //todo: multiple 'note' blocks
    //todo: verify session id
    sessionId=command->getAttribute("sessionid");
    status=command->getAttribute("status");

    this->xdata=command->findChildNamespace("x","jabber:x:data");

    //non-descriptive result
    if (!xdata && plainText.empty())
        plainText=status; //todo: make more informative and localizabe

    PostMessage(getHWnd(), WM_USER, 0, (LPARAM)"");
    return;
}

void AdHocForm::onSubmit( JabberDataBlockRef replyForm ) {
    if (status!="executing") return;
    sendCommand("execute", replyForm);
}

void AdHocForm::onCancel() {
    if (status!="executing") return;
    sendCommand("cancel", JabberDataBlockRef());
}

void AdHocForm::sendCommand( const std::string &command, JabberDataBlockRef childData ) {
    ResourceContextRef rc=this->rc.lock();
    if (!rc) return;

    GetAdHoc *getah=new GetAdHoc(jid, node, formRef.lock());
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getah));
    getah->doRequest(rc, sessionId, command, childData);
}

AdHocForm::~AdHocForm() {
    onCancel();
}