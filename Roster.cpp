//#include "stdafx.h"

#include "jid.h"
#include "JabberDataBlockListener.h"
#include "Roster.h"

#include <commctrl.h>
#include <windowsx.h>

#include <utf8.hpp>

#include <algorithm>

#include "Image.h"

Roster::Roster(){
    roster=ListViewODR::ref();
}

void Roster::addContact(Contact::ref contact) {
    //jidMap[contact->jid.getJid()]=contact;
    contacts.push_back(contact);
}

Contact::ref Roster::findContact(const std::string &jid, bool bareJid) const {
    for (ContactList::const_iterator i=contacts.begin(); i!=contacts.end(); i++) {
        Contact::ref r=*i;
        if (r->jid==jid) return r;
    }
    return Contact::ref();
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
        //todo: контакт в нескольких группах
        JabberDataBlockRef jGroup=item->getChildByName("group"); 
        std::string group=(jGroup)? jGroup->getText() : "";

        std::string subscr=item->getAttribute("subscription");
        if (item->hasAttribute("ask")) {
            subscr+=',';
            subscr+="ask";
        }

        //todo: разное поведение для roster request и roster push
        Contact::ref contact;
        if (rosterPush) {
            contact=findContact(jid,true);
        } 
        if (contact==NULL) { 
            contact=Contact::ref(new Contact(jid, "", name));
            std::wstring rjid=utf8::utf8_wchar(contact->rosterJid);
            //roster->addODR(contact, (i==query->getChilds()->end()));
        }   
        if (!findGroup(group)) {
            createGroup(group);
            std::stable_sort(groups.begin(), groups.end(), RosterGroup::compare );
        }

        contact->subscr=subscr;
        contact->group=group;

        addContact(contact);
    }
    makeViewList();
    return BLOCK_PROCESSED;
}

void Roster::makeViewList() {
    std::stable_sort(contacts.begin(), contacts.end(), Contact::compare);

    ODRSet::ref odrlist=ODRSet::ref(new ODRList());

    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
        ODRList *list=(ODRList *)(odrlist.get());
        list->odrVector.push_back(group);

        if (!group->isExpanded()) continue;

        for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
            Contact::ref contact=*ci;
            if (group->equals(contact->group)) list->odrVector.push_back(contact);
        }
    }

    roster->bindODRList(odrlist);
    PostMessage(roster->getHWnd(), WM_USER+1, 0, 0);
    //roster->notifyListUpdate(false);
}
//////////////////////////////////////////////////////////////////////////
RosterGroup::ref Roster::findGroup( const std::string &name ) {
    for (GroupList::const_iterator i=groups.begin(); i!=groups.end(); i++) {
        RosterGroup::ref r=*i;
        if (r->equals(name)) return r;
    }
    return RosterGroup::ref();
}

RosterGroup::ref Roster::createGroup( const std::string &name ) {
    
    RosterGroup::ref newGrp=RosterGroup::ref(new RosterGroup(name));
    groups.push_back(newGrp);
    return newGrp;
}


RosterGroup::RosterGroup( const std::string &name ) {
    groupName=name;
    wstr=utf8::utf8_wchar((name.length()==0)? "General" : name);
    expanded=true;
    init();
}

int RosterGroup::getColor() const { return 0xff0000; }

const wchar_t * RosterGroup::getText() const { return wstr.c_str(); }

int RosterGroup::getIconIndex() const { 
    return (expanded)? 
        icons::ICON_EXPANDED_INDEX : 
        icons::ICON_COLLAPSED_INDEX;
}
bool RosterGroup::compare( RosterGroup::ref left, RosterGroup::ref right ) {
    return left->groupName < right ->groupName;
}