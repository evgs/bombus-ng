#include "stdafx.h"

#include "Jid.h"
#include "Contact.h"
#include "utf8.hpp"

Contact::Contact(const std::string &jid, const std::string &resource, const std::string &nickname) {
    this->jid=Jid(jid, resource);
    this->rosterJid=jid;
    this->nickname=nickname;
    this->status=PRESENCE_OFFLINE;

    wjid=utf8::utf8_wchar( (nickname.empty())? jid:nickname);
    init();
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