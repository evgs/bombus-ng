#include "stdafx.h"

#include "jid.h"
#include "JabberDataBlockListener.h"
#include "Roster.h"

#include <commctrl.h>
#include <windowsx.h>

#include <utf8.hpp>


Roster::Roster(){
    roster=ListViewODR::ref();
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

    bool rosterPush=(block->getAttribute("type"))=="set";

    JabberDataBlockRef query=block->getChildByName("query");

    if (query.get()==NULL) return BLOCK_REJECTED;
    if (query->getAttribute("xmlns")!="jabber:iq:roster") return BLOCK_REJECTED;

    JabberDataBlockRefList::iterator i=query->getChilds()->begin();
    while (i!=query->getChilds()->end()) {
        JabberDataBlockRef item=*(i++);
        std::string jid=item->getAttribute("jid");
        std::string name=item->getAttribute("name");
        std::string subscr=item->getAttribute("subscription");
        if (item->hasAttribute("ask")) {
            subscr+=',';
            subscr+="ask";
        }

        //todo: разное поведение для roster request и roster push
        ContactRef contact;
        if (rosterPush) {
            contact=findContact(jid,true);
        } 
        if (contact==NULL) { 
            contact=ContactRef(new Contact(jid, "", name));
            std::wstring rjid=utf8::utf8_wchar(contact->rosterJid);
            roster->addODR(contact, (i==query->getChilds()->end()));
        }   

        contact->subscr=subscr;
        addContact(contact);
    }
    return BLOCK_PROCESSED;
}