#include "stdafx.h"

#include "jid.h"
#include "JabberDataBlockListener.h"
#include "Roster.h"


Roster::Roster(){

}

void Roster::addContact(ContactRef contact) {
    jidMap[contact->jid.getJid()]=contact;
}

ContactRef Roster::findContact(const std::string jid, bool bareJid) const {
    ContactRefMap::const_iterator i=jidMap.find(jid);
    if (i==jidMap.end()) return ContactRef();
    return i->second;
}

ProcessResult Roster::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
    JabberDataBlockRef query=block->getChildByName("query");

    if (query.get()==NULL) return BLOCK_REJECTED;
    if (!query->hasAttribute("jabber:iq:roster")) return BLOCK_REJECTED;

    JabberDataBlockIterator i=query->getChilds()->begin();
    while (i!=query->getChilds()->end()) {
        JabberDataBlockRef item=*(i++);
        std::string jid=item->getAttribute("jid");
        std::string name=item->getAttribute("name");
        std::string subscr=item->getAttribute("subscription");
        if (item->hasAttribute("ask")) {
            subscr+=',';
            subscr+="ask";
        }

        ContactRef contact=ContactRef(new Contact(jid, "", name));
        contact->subscr=subscr;
    }
    return BLOCK_PROCESSED;
}