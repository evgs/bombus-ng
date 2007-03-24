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

void VcardForm::onWmUserUpdate() {
    //std::string xml=*(vcard->toXML());
    //const std::string &xml2=XMLStringPrep(xml);
    //const std::wstring xml3=utf8::utf8_wchar(xml2);

    //SendMessage(hwndHTML, WM_SETTEXT, 0, (LPARAM)"");

    loadPhoto();

    startHtml();
    addImg(L"\\vcard");
    beginForm("vcard-photo", "photo");
    if (img) if (img->getHBmp()) {
        button("save",std::string("Save")); 
        if (editForm) button("clear",std::string("Clear")); 
    }
    if (editForm) button("load",std::string("Load")); 
    addText("<BR>");
    endForm(); 
    

    beginForm("vcard-fields", "publish");

    addHtmlField("FN", NULL,        "Full Name", TXT);
    addHtmlField("NICKNAME", NULL,  "Nickname", TXT);
    addHtmlField("BDAY", NULL,      "Birthday", TXT);
    addHtmlField("ADR", "STREET",   "Street", TXT);
    addHtmlField("ADR", "EXTADR",   "Street2", TXT);
    addHtmlField("ADR", "LOCALITY", "City", TXT);
    addHtmlField("ADR", "REGION",   "State", TXT);
    addHtmlField("ADR", "PCODE",    "Post code", TXT);
    addHtmlField("ADR", "CTRY",     "Country", TXT);
    addHtmlField("TEL", "HOME",     "Home Phone Number", TXT);
    addHtmlField("TEL", "NUMBER",   "Phone Number", TXT);
    addHtmlField("EMAIL", "USERID", "E-Mail", URL);
    addHtmlField("TITLE", NULL,     "Position", TXT);
    addHtmlField("ROLE", NULL,      "Role", TXT);
    addHtmlField("ORG", "ORGNAME",  "Organization", TXT);
    addHtmlField("ORG", "ORGUNIT",  "Dept", TXT);
    addHtmlField("URL", NULL,       "Url", URL);
    addHtmlField("DESC", NULL,      "About", MULTILINE);

    if (editForm) button(std::string("Publish"));
    button("reload",std::string("Reload")); 

    endHtml();
}

ProcessResult GetVcard::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    VcardForm::ref vfRef=vf.lock();

    if (vfRef)
        vfRef->vcardArrivedNotify(block);

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
VcardForm::ref VcardForm::createVcardForm( HWND parent, const std::string &jid, ResourceContextRef rc, bool edit ) {
    VcardForm *vf=new VcardForm();

    vf->editForm=edit;
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

void VcardForm::addHtmlField( const char *ns1, const char *ns2, const char *description, int flags ) {
    if (!vcard) return;
    JabberDataBlockRef vcardTemp=vcard->findChildNamespace("vCard", "vcard-temp");      if (!vcardTemp) return;
    JabberDataBlockRef field=vcardTemp->getChildByName(ns1);     if (!field) return;
    if (ns2) field=field->getChildByName(ns2);     if (!field) return;
    const std::string value=XMLStringPrep(field->getText());

    std::string name(ns1);
    if (ns2) {
        name+="#";
        name+=ns2;
    }
    const std::wstring wname=utf8::utf8_wchar(name);

    if (editForm) {
        if (flags==TXT) flags=TEXTBOX;
        if (flags==URL) flags=TEXTBOX;
    } else {
        if (flags==MULTILINE) flags=TXT;
    }

    if (flags & TEXTBOX) {
        textBox(name.c_str(), std::string(description), value);
        return;
    }
    if (flags & MULTILINE) {
        textML(name.c_str(), std::string(description), value);
        return;
    }

    if (flags & URL)   {
        url(std::string(description), value);
        return;
    }

    if (!editForm) if (value.length()==0) return;

    textConst(std::string(description), value);
}

HBITMAP VcardForm::getImage( LPCTSTR url, DWORD cookie ) {
    this->cookie=cookie;
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

void VcardForm::onHotSpot( LPCTSTR url, LPCTSTR param ) {
    std::string nurl=utf8::wchar_utf8(std::wstring(url));
    if (nurl=="load") {
        //todo: load photo
        //loadPhoto();
        //setImage(img->getHBmp(), cookie);
        return;
    }
    if (nurl=="save") {
        //todo: save photo
        return;
    }
    if (nurl=="clear") {
        //todo: clear photo
        img.reset();
        setImage(0, cookie);
        return;
    }
    if (nurl=="reload") {
        //todo: re-request vcard
        return;
    }
    if (nurl=="publish") {
        //todo: publish vcard

        JabberDataBlock viq("iq");
        viq.setAttribute("to",jid.c_str());
        viq.setAttribute("type","set");
        viq.setAttribute("id",jid.c_str());
        JabberDataBlock *vcardTemp=viq.addChild("vCard",NULL);
        vcardTemp->setAttribute("xmlns","vcard-temp");

        StringMapRef m=HtmlView::splitHREFtext(param);
        for (StringMap::iterator i=m->begin(); i!=m->end(); i++) {
            std::string key=i->first;
            std::string value=i->second;
            std::string key2;
            int k2=key.find('#');
            if (k2>0) {
                key2=key.substr(k2+1, key.length()-k2-1);
                key=key.substr(0, k2);
            }

            JabberDataBlock *child=vcardTemp->addChild(key.c_str(), NULL);
            if (k2>0) child=vcardTemp->addChild(key2.c_str(), NULL);
            child->setText(value);
        }

        StringRef result=viq.toXML();
        
        return;
    }

}