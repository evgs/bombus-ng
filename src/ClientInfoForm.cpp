#include <windows.h>
#include <sipapi.h>
#include <htmlctrl.h>
#include <commdlg.h>

#include "wmuser.h"
#include "ClientInfoForm.h"
#include "JabberStream.h"

#include "utf8.hpp"
#include "base64.h"
#include "../gsgetfile/include/gsgetlib.h"

#include "TimeFunc.h"

class GetInfo : public JabberDataBlockListener {
public:
    GetInfo(const std::string &jid, ClientInfoForm::ref form);
    ~GetInfo(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc);
private:
    std::string jid;
    std::string idVer;
    std::string idLast;
    std::string idTime;
    int resultCount;
    ResourceContextRef rc;
    boost::weak_ptr<ClientInfoForm> vf;
};

GetInfo::GetInfo(const std::string &jid, ClientInfoForm::ref form){
    this->jid=jid;
    this->vf=form;
}

void GetInfo::doRequest(ResourceContextRef rc) {
    resultCount=3;

    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "get");

    JabberDataBlockRef qry;
    req.setAttribute("id", idVer=strtime::getRandom() );
    qry=req.addChildNS("query", "jabber:iq:version");
    rc->jabberStream->sendStanza(req);

    req.setAttribute("id", idLast=strtime::getRandom() );
    qry->setAttribute("xmlns", "jabber:iq:last");
    rc->jabberStream->sendStanza(req);

    req.setAttribute("id", idTime=strtime::getRandom() );
    qry->setAttribute("xmlns", "jabber:iq:last");
    rc->jabberStream->sendStanza(req);
}

ProcessResult GetInfo::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    ClientInfoForm::ref vfRef=vf.lock();
    if (!vfRef) return CANCEL;

    if (block->getTagName()!="iq") return BLOCK_REJECTED;
    //if (block->getAttribute("from")!=jid) return BLOCK_REJECTED; TODO: stringprep
    const std::string &type=block->getAttribute("type");
    int nType=-1;
    if (type=="result") nType=1;
    if (type=="error") nType=0;
    if (nType<0) return BLOCK_REJECTED;

    const std::string &id=block->getAttribute("id");
    bool idMatch=(id==idVer || id==idLast || id==idTime);
    if (!idMatch) return BLOCK_REJECTED;

    JabberDataBlockRef qry=block->findChildNamespace("query","jabber:iq:version");
    if (qry) {
        if (nType) {
            vfRef->clientName=qry->getChildText("name");
            vfRef->clientVersion=qry->getChildText("version");
            vfRef->clientOS=qry->getChildText("os");
        } else {
            vfRef->clientVersion="unavailable";
        }
    }

    qry=block->findChildNamespace("query","jabber:iq:last");
    if (qry) {
        if (nType) {
            const std::string &seconds=qry->getAttribute("seconds");
            vfRef->lastActivityMessage=qry->getText();
            vfRef->lastActivityTime=seconds; //todo: convert to absolute time
        } /*else {
            vfRef->lastActivityTime="unavailable";
        }*/
    }

    qry=block->findChildNamespace("query","jabber:iq:time");
    if (qry) {
        if (nType) {
            const std::string &display=qry->getChildText("display");
            vfRef->clientTime=display;
        } 
    }

    vfRef->infoArrivedNotify();

    resultCount--;
    return (resultCount==0)? LAST_BLOCK_PROCESSED : BLOCK_PROCESSED;
}

void ClientInfoForm::onWmUserUpdate() {

    startHtml();

    if (clientVersion.length()) {
        addText("<strong>Client info:</strong><br>Name: "); addText(clientName);
        addText("<br>Version: "); addText(clientVersion);
        addText("<br>OS: "); addText(clientOS);
        addText("<br>");
    }

    if (lastActivityTime.length())   {
        unsigned __int64 delay=atol(lastActivityTime.c_str());
        delay*=10000000;
        PackedTime ct=strtime::getCurrentUtc();
        void *i64ct=&ct;
        (*( (__int64*)i64ct ))-=delay;
        
        addText("<br><strong>Last activity:</strong><br>Time: "); addText(strtime::toLocalDateTime(ct));
    }
    if (lastActivityMessage.length())   addText("<br>Status: "); addText(lastActivityMessage);

    if (clientTime.length()) addText("<br>Contact's time: "); addText(clientTime);

    endHtml();
}

//////////////////////////////////////////////////////////////////////////
ClientInfoForm::ref ClientInfoForm::createInfoForm( HWND parent, const std::string &jid, ResourceContextRef rc) {
    ClientInfoForm *vf=new ClientInfoForm();

    vf->parentHWnd=parent;
    vf->title=utf8::utf8_wchar(jid);
    vf->wt=WndTitleRef(new WndTitle(vf, icons::ICON_SEARCH_INDEX));

    vf->init();
    
    SetParent(vf->thisHWnd, parent);
    vf->rc=rc;

    ClientInfoForm::ref vfRef=ClientInfoForm::ref(vf);

    vfRef->infoListener=new GetInfo(jid, vfRef);
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(vfRef->infoListener));
    vfRef->infoListener->doRequest(rc);

    return vfRef;
}

void ClientInfoForm::infoArrivedNotify(){
    PostMessage(getHWnd(), WM_HTML_UPDATE, 0, (LPARAM)"");
}


HBITMAP ClientInfoForm::getImage( LPCTSTR url, DWORD cookie ) { return NULL; }

//extern HINSTANCE g_hInst;

void ClientInfoForm::onHotSpot( LPCSTR url, LPCSTR param ) {
    std::string nurl=std::string(url);
    if (nurl=="ok") {}
}

ClientInfoForm::~ClientInfoForm() {
    ResourceContextRef rc=this->rc.lock();
    if (rc) rc->jabberStanzaDispatcherRT->removeListener(infoListener);
}
