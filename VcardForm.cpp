#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>

#include "VcardForm.h"
#include "JabberStream.h"

#include "utf8.hpp"

class GetVcard : public JabberDataBlockListener {
public:
    GetVcard(const std::string &jid, VcardForm  * form);
    ~GetVcard(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc);
private:
    std::string jid;
    std::string id;
    ResourceContextRef rc;
    VcardForm *vf;
};

GetVcard::GetVcard(const std::string &jid, VcardForm * form){
    this->jid=jid;
    this->vf=form;
    id="vf#";
    id+=jid;
}

void GetVcard::doRequest(ResourceContextRef rc) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "get");
    req.setAttribute("id", id);

    req.addChild("vCard", NULL)->setAttribute("xmlns","vcard-temp");

    rc->jabberStream->sendStanza(req);
}

void VcardForm::update() {
    std::string xml=*(vcard->toXML());
    const std::string &xml2=XMLStringPrep(xml);
    const std::wstring xml3=utf8::utf8_wchar(xml2);

    //SendMessage(hwndHTML, WM_SETTEXT, 0, (LPARAM)"");

    SendMessage(htmlHWnd, DTM_CLEAR, 0, 0);
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<HTML><TITLE>Test</TITLE>"));
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<BODY><P>"));
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)(xml3.c_str()));
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("</BODY></HTML>"));
    SendMessage(htmlHWnd, DTM_ENDOFSOURCE, 0, (LPARAM)NULL);
}

ProcessResult GetVcard::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    vf->vcardArrivedNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
VcardForm::VcardForm(HWND parent, const std::string &jid, ResourceContextRef rc){
    parentHWnd=parent;
    init();

    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(jid);

    wt=WndTitleRef(new WndTitle(this, icons::ICON_VCARD));

    this->rc=rc;

    GetVcard *getv=new GetVcard(jid, this);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getv));
    getv->doRequest(rc);
}

void VcardForm::vcardArrivedNotify(JabberDataBlockRef vcard){
    this->vcard=vcard;

    PostMessage(getHWnd(), WM_USER, 0, (LPARAM)"");
}