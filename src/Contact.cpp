#include "stdafx.h"

#include "Jid.h"
#include "Contact.h"

Contact::Contact(const std::string &jid, const std::string &resource, const std::string &nickname) {
    this->jid=Jid(jid, resource);
    this->rosterJid=jid;
    this->nickname="";
    this->status=PRESENCE_OFFLINE;
}