//#include "stdafx.h"

#include "jid.h"
#include "JabberDataBlockListener.h"
#include "Roster.h"

#include <commctrl.h>
#include <windowsx.h>

#include <utf8.hpp>

#include <algorithm>

#include "Image.h"
#include "Presence.h"

Roster::Roster(){
    roster=VirtualListView::ref();
}

/*void Roster::addContact(Contact::ref contact) {
    bareJidMap[contact->jid.getBareJid()]=contact;
    contacts.push_back(contact);
}*/

Contact::ref Roster::findContact(const std::string &jid) const {
    Jid right(jid);
    for (ContactList::const_iterator i=contacts.begin(); i!=contacts.end(); i++) {
        Contact::ref r=*i;
        if (r->jid==right) return r;
    }
    return Contact::ref();
}

ProcessResult Roster::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {

    const std::string & blockTagName=block->getTagName();

    if (blockTagName=="presence") {
        processPresence(block);
        return BLOCK_PROCESSED;
    }

    if (blockTagName!="iq") return BLOCK_REJECTED;

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

        /*if (group.length()==0)*/ if (jid.find('@')==std::string.npos) 
            group="Transports";

        std::string subscr=item->getAttribute("subscription");
        if (item->hasAttribute("ask")) {
            subscr+=',';
            subscr+="ask";
        }

        //todo: разное поведение для roster request и roster push
        Contact::ref contact;
        if (rosterPush) {
            contact=findContact(jid);
        } 
        if (contact==NULL) { 
            contact=Contact::ref(new Contact(jid, "", name));
            //std::wstring rjid=utf8::utf8_wchar(contact->rosterJid);
            //roster->addODR(contact, (i==query->getChilds()->end()));
        }   
        if (!findGroup(group)) {
            createGroup(group);
            std::stable_sort(groups.begin(), groups.end(), RosterGroup::compare );
        }

        contact->subscr=subscr;
        contact->group=group;

        bareJidMap[contact->jid.getBareJid()]=contact;
        contacts.push_back(contact);
        //todo: subscription=remove
    }
    //std::stable_sort(contacts.begin(), contacts.end(), Contact::compare);
    makeViewList();
    return BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////
void Roster::processPresence( JabberDataBlockRef block ) {
    std::string from=block->getAttribute("from");
    std::string type=block->getAttribute("type");

    presence::PresenceIndex typeIndex=presence::OFFLINE;
    presence::PresenceIndex type2=presence::NOCHANGE; //no change
    if (type=="unavailable") { 
        typeIndex=presence::OFFLINE;
    } else if (type=="subscribe") { 
        //TODO:
    } else if (type=="subscribed") {
        //TODO:
    } else if (type=="unsubscribe") {
        //TODO:
    } else if (type=="unsubscribed") {
        //TODO:
    } else if (type=="error") {
        typeIndex=presence::OFFLINE;
        type2=presence::PRESENCE_ERROR;
        //todo: extract error text here
    } else {
        typeIndex=presence::ONLINE;
        type=block->getChildText("show");
        if (type=="chat") typeIndex=presence::CHAT;
        if (type=="away") typeIndex=presence::AWAY;
        if (type=="xa") typeIndex=presence::XA;
        if (type=="dnd") typeIndex=presence::DND;
    }

    std::string priority=block->getChildText("priority");
    std::string status=block->getChildText("status");

    Contact::ref contact=findContact(from);
    if (!contact) {
        Jid jid(from);
        //first attempt - search for contact without resource
        contact=findContact(jid.getBareJid());
        if (contact) {
            // store resource
            contact->jid.setResource(jid.getResource());
        } else { 
            //todo: second attempt - clone contact from bareJidMap

            //third attempt - based on NOT-IN-LIST policy
            contact=Contact::ref(new Contact(jid.getBareJid(), jid.getResource(), ""));

            std::string group="-Not-In-List-";
            if (!findGroup(group)) {
                createGroup(group);
                std::stable_sort(groups.begin(), groups.end(), RosterGroup::compare );
            }

            contact->subscr="NIL";
            contact->group=group;

            bareJidMap[contact->jid.getBareJid()]=contact;
            contacts.push_back(contact);
        }
    }

    contact->status=typeIndex;
    contact->update();
    makeViewList();
}
//////////////////////////////////////////////////////////////////////////
void Roster::makeViewList() {

    std::stable_sort(contacts.begin(), contacts.end(), Contact::compare);

    //ODRSet::ref odrlist=ODRSet::ref(new ODRList());
    //ODRList *list=(ODRList *)(odrlist.get());
    
    ODRList *list=new ODRList(); //АХТУНГ №1

    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
        list->push_back(group);

        if (!group->isExpanded()) continue;

        for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
            Contact::ref contact=*ci;
            if (group->equals(contact->group)) list->push_back(contact);
        }
    }

    //roster->bindODRList(odrlist);
    PostMessage(roster->getHWnd(), WM_USER+1, 0, (LPARAM)list); //АХТУНГ №2
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


//////////////////////////////////////////////////////////////////////////
RosterView::RosterView( HWND parent, const std::string & title ){
    init();

    parentHWnd=parent;
    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, 0));
    cursorPos=ODRRef();//odrlist->front();
    odrlist=ODRListRef(new ODRList());
}

void RosterView::eventOk() {
    if (!cursorPos) return;
    RosterGroup *p = dynamic_cast<RosterGroup *>(cursorPos.get());
    if (p) {
    	p->setExpanded(!p->isExpanded());
        roster.lock()->makeViewList();
    }
}