//#include "stdafx.h"

#include "Jid.h"
#include "Contact.h"
#include "utf8.hpp"

Contact::Contact(const std::string &jid, const std::string &resource, const std::string &nickname) {
    this->jid=Jid(jid, resource);
    this->rosterJid=jid;
    this->nickname=nickname;
    this->status=presence::OFFLINE;

    update();
    messageList=MessageListRef(new MessageList());
}
bool Contact::hasUnreadMsgs() {
    for ( MessageList::const_iterator i=messageList->begin();  
          i!=messageList->end();
          i++ ) {
        if (i->get()->unread) return true;
    }
    return false;
}
int Contact::getColor() const{ return 0; }
int Contact::getIconIndex() const{ return status; }

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