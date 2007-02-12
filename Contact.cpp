//#include "stdafx.h"

#include "Jid.h"
#include "Contact.h"
#include "utf8.hpp"
#include "Image.h"

//////////////////////////////////////////////////////////////////////////
extern ImgListRef skin;

int identifyTransport(const std::string &jid) {
    int serv=jid.find('@')+1;
    int dotsrv=jid.find('.', serv);
    if (dotsrv<0) return 0;

    Skin * il= dynamic_cast<Skin *>(skin.get());
    if (!il) return 0;
    return il->getBaseIndex(jid.substr(serv, dotsrv-serv));
}
//////////////////////////////////////////////////////////////////////////

Contact::Contact(const std::string &jid, const std::string &resource, const std::string &nickname) {
    this->jid=Jid(jid, resource);
    this->rosterJid=jid;
    this->nickname=nickname;
    this->status=presence::OFFLINE;

    transpIndex=identifyTransport(jid);

    update();
    messageList=ODRListRef(new ODRList);
}
bool Contact::hasUnreadMsgs() {
    for ( ODRList::const_iterator i=messageList->begin();   i!=messageList->end();  i++ ) {
        ODRRef r=*i;
        Message *m = dynamic_cast<Message *>(r.get());
        if(m) {
        	if (m->unread) return true;
        }
    }
    return false;
}
int Contact::getColor() const{ return 0; }
int Contact::getIconIndex() const{ return status+transpIndex; }

const wchar_t * Contact::getText() const{ return wjid.c_str(); }
//////////////////////////////////////////////////////////////////////////
bool Contact::compare( Contact::ref left, Contact::ref right ) {
    if (left->status < right->status) return true;
    if (left->status > right->status) return false;
    return (left->wjid < right->wjid);
}
void Contact::update() {
    std::string s=(nickname.empty())? jid.getBareJid():nickname;
    std::string resource=jid.getResource();
    if (resource.length()) { s+='/'; s+=resource; }
    wjid=utf8::utf8_wchar( s );
    init();
}