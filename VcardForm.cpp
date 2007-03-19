#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>

#include "VcardForm.h"
#include "JabberStream.h"

#include "utf8.hpp"
#include "base64.h"

extern std::wstring appRootPath;


class GetVcard : public JabberDataBlockListener {
public:
    GetVcard(const std::string &jid, VcardForm::ref form);
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
    boost::weak_ptr<VcardForm> vf;
};

GetVcard::GetVcard(const std::string &jid, VcardForm::ref form){
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
    //std::string xml=*(vcard->toXML());
    //const std::string &xml2=XMLStringPrep(xml);
    //const std::wstring xml3=utf8::utf8_wchar(xml2);

    //SendMessage(hwndHTML, WM_SETTEXT, 0, (LPARAM)"");

    loadPhoto();

    SendMessage(htmlHWnd, DTM_CLEAR, 0, 0);
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<HTML><TITLE>Test</TITLE>"));
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<BODY><P><IMG SRC=\"\\vcard\"><BR>"));

    addHtmlField("FN", NULL,        L"Full Name");
    addHtmlField("NICKNAME", NULL,  L"Nickname");
    addHtmlField("BDAY", NULL,      L"Birthday");
    addHtmlField("ADR", "STREET",   L"Street");
    addHtmlField("ADR", "EXTADR",   L"Street2");
    addHtmlField("ADR", "LOCALITY", L"City");
    addHtmlField("ADR", "REGION",   L"State");
    addHtmlField("ADR", "PCODE",    L"Post code");
    addHtmlField("ADR", "CTRY",     L"Country");
    addHtmlField("TEL", "HOME",     L"Home Phone Number");
    addHtmlField("TEL", "NUMBER",   L"Phone Number");
    addHtmlField("EMAIL", "USERID", L"E-Mail", URL);
    addHtmlField("TITLE", NULL,     L"Position");
    addHtmlField("ROLE", NULL,      L"Role");
    addHtmlField("ORG", "ORGNAME",  L"Organization");
    addHtmlField("ORG", "ORGUNIT",  L"Dept");
    addHtmlField("URL", NULL,       L"Url", URL);
    addHtmlField("DESC", NULL,      L"About");

    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("</BODY></HTML>"));
    SendMessage(htmlHWnd, DTM_ENDOFSOURCE, 0, (LPARAM)NULL);
}

ProcessResult GetVcard::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    VcardForm::ref vfRef=vf.lock();

    if (vfRef)
        vfRef->vcardArrivedNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
VcardForm::ref VcardForm::createVcardForm(HWND parent, const std::string &jid, ResourceContextRef rc){
    VcardForm *vf=new VcardForm();

    vf->parentHWnd=parent;
    vf->init();
    
    SetParent(vf->thisHWnd, parent);

    vf->title=utf8::utf8_wchar(jid);

    vf->wt=WndTitleRef(new WndTitle(vf, icons::ICON_VCARD));

    vf->rc=rc;

    VcardForm::ref vfRef=VcardForm::ref(vf);

    GetVcard *getv=new GetVcard(jid, vfRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getv));
    getv->doRequest(rc);

    return vfRef;
}

void VcardForm::vcardArrivedNotify(JabberDataBlockRef vcard){
    this->vcard=vcard;

    PostMessage(getHWnd(), WM_USER, 0, (LPARAM)"");
}

void VcardForm::addHtmlField( const char *ns1, const char *ns2, const wchar_t* description, int flags ) 
{
    if (!vcard) return;
    JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard", "vcard-temp");      if (!vcardTemp) return;
    JabberDataBlockRef field=vcardTemp->getChildByName(ns1);     if (!field) return;
    if (ns2) field=field->getChildByName(ns2);     if (!field) return;
    const std::wstring value=utf8::utf8_wchar(XMLStringPrep(field->getText()));
    if (value.length()==0) return;

    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) description);
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) L": ");
    if (flags & URL)   {
        SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) L"<A HREF=\"");
        //SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) value.c_str()); 
        SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) L"\">");
    }
    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) value.c_str());
    if (flags & URL)   SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) L"</A>");

    SendMessage(htmlHWnd, DTM_ADDTEXTW, FALSE, (LPARAM) L"<BR>");
}

HBITMAP VcardForm::getImage( LPCTSTR url ) {
    if (img) return img->getHBmp();
    return NULL;
}

void VcardForm::loadPhoto() {
    if (!vcard) return;
    JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard", "vcard-temp");      if (!vcardTemp) return;
    JabberDataBlockRef photo=vcardTemp->getChildByName("PHOTO");    if (!photo) return;
    JabberDataBlockRef binval=photo->getChildByName("BINVAL"); if (!binval) return;
    const std::string &data=binval->getText();

    int dstLen=base64::base64DecodeGetLength(data.length());
    char *dst=new char[dstLen];

    dstLen=base64::base64Decode2(dst, data.c_str(), data.length());

    std::wstring imgFile=appRootPath+L"$tmpimg.jpg";


    HANDLE file=CreateFile(imgFile.c_str(), 
        GENERIC_WRITE, 
        FILE_SHARE_READ, NULL, 
        CREATE_ALWAYS,
        0, NULL);

    DWORD dwProcessed;
    if (file==INVALID_HANDLE_VALUE) {
        delete dst; return;
    }
    WriteFile(file, dst, dstLen, &dwProcessed, NULL);
    CloseHandle(file);

    img=ImageRef(new Image(imgFile.c_str()));

    delete dst;
}