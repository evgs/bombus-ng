//#include "stdafx.h"

#include "jid.h"
#include "JabberDataBlockListener.h"
#include "..\vs2005\ui\resourceppc.h"
#include "Roster.h"

#include <commctrl.h>
#include <windowsx.h>

#include <utf8.hpp>

#include <algorithm>

#include "Image.h"
#include "JabberAccount.h"
#include "JabberStream.h"
#include "Presence.h"
#include "ProcessMUC.h"

#include "wmuser.h"
#include "TabCtrl.h"
#include "ChatView.h"

#include "DlgStatus.h"
#include "DlgAddEditContact.h"
#include "VcardForm.h"
#include "ClientInfoForm.h"
#include "ServiceDiscovery.h"
#include "MucConfigForm.h"

#include "utf8.hpp"

extern TabsCtrlRef tabs;

char *NIL="Not-In-List";

Roster::Roster(ResourceContextRef rc){
    roster=VirtualListView::ref();
    this->rc=rc;
    createGroup("Self-Contact", RosterGroup::SELF_CONTACT);
    createGroup("Transports", RosterGroup::TRANSPORTS);
    createGroup(NIL, RosterGroup::NOT_IN_LIST);

    Contact::ref self=Contact::ref(new Contact(rc->account->getBareJid(), "", ""));
    self->group="Self-Contact";
    bareJidMap[self->jid.getBareJid()]=self;
    addContact(self);
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

    bool rosterPush=(block->getAttribute("type"))=="set";

    JabberDataBlockRef query=block->getChildByName("query");

    if (query.get()==NULL) return BLOCK_REJECTED;
    if (query->getAttribute("xmlns")!="jabber:iq:roster") return BLOCK_REJECTED;

    JabberDataBlockRefList::iterator i=query->getChilds()->begin();
    while (i!=query->getChilds()->end()) {
        JabberDataBlockRef item=*(i++);
        std::string jid=item->getAttribute("jid");
        std::string name=item->getAttribute("name");
        //todo: êîíòàêò â íåñêîëüêèõ ãðóïïàõ
        JabberDataBlockRef jGroup=item->getChildByName("group"); 
        std::string group=(jGroup)? jGroup->getText() : "";

        /*if (group.length()==0)*/ if (jid.find('@')==std::string.npos) 
            group="Transports";

        std::string subscr=item->getAttribute("subscription");
        
        int offlineIcon=presence::OFFLINE;
        if (subscr=="none") offlineIcon=presence::UNKNOWN;
        if (item->hasAttribute("ask")) if (subscr!="remove"){
            subscr+=',';
            subscr+="ask";
            offlineIcon=presence::ASK;
        }

        if (!findGroup(group)) {
            createGroup(group, RosterGroup::ROSTER);
            std::stable_sort(groups.begin(), groups.end(), RosterGroup::compare );
        }

        //ïðè push ìîäèôèöèðîâàòü ÂÑÅ ýêçåìïëÿðû ïî bareJid
        Contact::ref contact;

        if (rosterPush) {
            int i=0;
            bool found=false;
            while (i!=contacts.size()) {
                Contact::ref right=contacts[i];
                if (right->rosterJid==jid) {
                    if (subscr=="remove") {
                        contacts.erase(contacts.begin()+i);
                        //todo: bareJidMap[contact->jid.getBareJid()]=contact; // now it is null
                        continue;
                    } else {
                        found=true;
                        right->subscr=subscr;
                        right->nickname=name;
                        right->group=group;
                        right->offlineIcon=offlineIcon;
                        right->update();
                    }

                }
                i++;
            }

            if (subscr=="remove") break;
            if (found) break;
        } 

        contact=findContact(jid);
        if (contact==NULL) { 
            contact=Contact::ref(new Contact(jid, "", name));
            //std::wstring rjid=utf8::utf8_wchar(contact->rosterJid);
            //roster->addODR(contact, (i==query->getChilds()->end()));
            if (!bareJidMap[contact->jid.getBareJid()]) {
                bareJidMap[contact->jid.getBareJid()]=contact;
                contacts.push_back(contact);
            }
        }   

        contact->subscr=subscr;
        contact->group=group;
        contact->offlineIcon=offlineIcon;

    }
    if (rosterPush) {
        JabberDataBlock result("iq");
        result.setAttribute("type","result");
        result.setAttribute("id",block->getAttribute("id"));
        rc->jabberStream->sendStanza(result);
    }
    //std::stable_sort(contacts.begin(), contacts.end(), Contact::compare);
    makeViewList();
    return BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////

void Roster::deleteContact(Contact::ref contact) {
    int i=0;
    while (i!=contacts.size()) {
        Contact::ref right=contacts[i];
        if (right->rosterJid==contact->rosterJid) {
            if (right->subscr=="NIL") {
                contacts.erase(contacts.begin()+i);
                //todo: bareJidMap[contact->jid.getBareJid()]=contact; // now it is null
                continue;
            } else {
                right->status=(presence::PresenceIndex) icons::ICON_TRASHCAN_INDEX;
                right->update();
            }

        }
        i++;
    }
    makeViewList();

    if (contact->subscr!="NIL") {
        rosterSet(NULL, contact->rosterJid.c_str(), NULL, "remove");
    }
}

void Roster::rosterSet(const char * nick, const char *jid, const char *group, const char *subscr ) {
    JabberDataBlock iqSet("iq");
    iqSet.setAttribute("type","set");
    iqSet.setAttribute("id","roster_set");
    JabberDataBlockRef qry=iqSet.addChildNS("query", "jabber:iq:roster");
    JabberDataBlockRef item=qry->addChild("item", NULL);

    item->setAttribute("jid", jid);
    if (nick) item->setAttribute("name", nick);
    if (group) item->addChild("group", group);
    if (subscr) item->setAttribute("subscription", subscr);

    rc->jabberStream->sendStanza(iqSet);
};
//////////////////////////////////////////////////////////////////////////
Contact::ref Roster::getContactEntry(const std::string & from){

    //first attempt - if we already have this contact in our list
    Contact::ref contact=findContact(from);
    if (!contact) {
        Jid jid(from);
        //second attempt - search for contact without resource
        contact=findContact(jid.getBareJid());
        if (contact) {
            // store resource
            contact->jid.setResource(jid.getResource());
            contact->update();
            needUpdateView=true;
        } else { 
            //third attempt - clone contact from bareJidMap
            if ( bareJidMap.count(jid.getBareJid())!=0 ) {
                contact=bareJidMap[jid.getBareJid()];
                contact=contact->clone();
                contact->jid.setResource(jid.getResource());
                contact->update();
                contacts.push_back(contact);
                needUpdateView=true;
            } else {

                //finally - based on NOT-IN-LIST policy
                contact=Contact::ref(new Contact(jid.getBareJid(), jid.getResource(), ""));

                //std::string group="Not-In-List";

                contact->subscr="NIL";
                contact->offlineIcon=presence::ASK;
                contact->group=NIL;
                //bareJidMap[contact->jid.getBareJid()]=contact;
                contacts.push_back(contact);
                needUpdateView=true;
            }
        }
    }
    return contact;
}
//////////////////////////////////////////////////////////////////////////
void Roster::makeViewList() {

    std::stable_sort(contacts.begin(), contacts.end(), Contact::compare);

    //ODRSet::ref odrlist=ODRSet::ref(new ODRList());
    //ODRList *list=(ODRList *)(odrlist.get());
    
    ODRList *list=new ODRList(); //ÀÕÒÓÍÃ ¹1

    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
        list->push_back(group);

        int elemCount=0;

        if (!group->isExpanded()) continue;

        group->addContacts(list);

        for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
            Contact::ref contact=*ci;
            if (group->equals(contact->group)) {
                list->push_back(contact);
                elemCount++;
            }
        }
        //removing group header if nothing to display
        if (elemCount==0) { list->pop_back();  continue; } 

    }

    needUpdateView=false;
    //roster->bindODRList(odrlist);
    PostMessage(roster->getHWnd(), WM_VIRTUALLIST_REPLACE, 0, (LPARAM)list); //ÀÕÒÓÍÃ ¹2
    //roster->notifyListUpdate(false);
}

StringVectorRef Roster::getRosterGroups() {
    StringVectorRef result=StringVectorRef(new StringVector());

    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
        if (group->type==RosterGroup::ROSTER) result->push_back(group->getName());
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
RosterGroup::ref Roster::findGroup( const std::string &name ) {
    for (GroupList::const_iterator i=groups.begin(); i!=groups.end(); i++) {
        RosterGroup::ref r=*i;
        if (r->equals(name)) return r;
    }
    return RosterGroup::ref();
}

RosterGroup::ref Roster::createGroup( const std::string &name, RosterGroup::Type type ) {

    RosterGroup::ref newGrp=RosterGroup::ref(new RosterGroup(name, type));
    groups.push_back(newGrp);
    return newGrp;
}

void Roster::addGroup( RosterGroup::ref group ) {
    groups.push_back(group);
}

void Roster::addContact( Contact::ref contact ) {
    contacts.push_back(contact);
}

void Roster::setStatusByFilter( const std::string & bareJid, int status ) {
    int i=0;
    while (i!=contacts.size()) {
        Contact::ref right=contacts[i];
        if (right->jid.getBareJid()==bareJid) {
            right->status=(presence::PresenceIndex)status;
        }
        i++;
    }
}

void Roster::setAllOffline() {
    int i=0;
    while (i!=contacts.size()) {
        contacts[i]->status=presence::OFFLINE;
        i++;
    }
}


Roster::ContactListRef Roster::getHotContacts() {
    ContactListRef hots=ContactListRef(new ContactList());
    for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
        Contact::ref contact=*ci;
        if (contact->nUnread > 0) {
            hots->push_back(contact);
        }
    }

    //muc rooms
    for (GroupList::const_iterator i=groups.begin(); i!=groups.end(); i++) {
        MucGroup::ref r= boost::dynamic_pointer_cast<MucGroup>(*i);
        if (r) {
            MucRoom::ref room=r->room;
            if (room->nUnread>0)
                hots->push_back(room);
        }
    }

    return hots;
}

Roster::ContactListRef Roster::getGroupContacts( RosterGroup::ref group ) {
    ContactListRef gcl=ContactListRef(new ContactList());
    for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
        Contact::ref contact=*ci;
        if (contact->group==group->getName()) {
            gcl->push_back(contact);
        }
    }
    return gcl;
}

RosterGroup::RosterGroup( const std::string &name, Type type ) {
    groupName=name;
    this->type=type;
    sortKey=utf8::utf8_wchar(name);
    wstr=TEXT("General");    if (name.length()!=0) wstr=sortKey;
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
    if (left->type < right->type) return true;
    if (left->type > right->type) return false;
    return (_wcsicmp(left->sortKey.c_str(), right->sortKey.c_str()) < 0);
}


//////////////////////////////////////////////////////////////////////////
RosterView::RosterView( HWND parent, const std::string & title ){
    parentHWnd=parent;
    init(); 

    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, presence::OFFLINE));
    cursorPos=ODRRef();//odrlist->front();
    odrlist=ODRListRef(new ODRList());

    ////
}

RosterView::~RosterView() {}

void RosterView::eventOk() {
    if (!cursorPos) return;
    RosterGroup *p = dynamic_cast<RosterGroup *>(cursorPos.get());
    if (p) {
    	p->setExpanded(!p->isExpanded());
        roster.lock()->makeViewList();
    } else {
        OnCommand(RosterView::OPENCHAT, 0);
    }
}

HMENU RosterView::getContextMenu() {
    if (!cursorPos) return NULL;

    HMENU hmenu=CreatePopupMenu();

    IconTextElement::ref focused = boost::dynamic_pointer_cast<IconTextElement>(cursorPos);
    if (focused) {
        focused->createContextMenu(hmenu);
    }

    //////////////////////////////////////////////////////////////////////////
    // Group actions
    RosterGroup *rg = dynamic_cast<RosterGroup *>(cursorPos.get());
    if (rg) {
        if (rg->type==RosterGroup::ROSTER)
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::RENAMEGRP,           TEXT("Rename"));

    }
    
    Contact * c = dynamic_cast<Contact *>(cursorPos.get());

    if (c) {
        RosterGroup::Type type=roster.lock()->findGroup(c->group)->type;

        MucRoom * mr= dynamic_cast<MucRoom *>(cursorPos.get());

        if (type==RosterGroup::TRANSPORTS) {
            AppendMenu(hmenu, MF_STRING, RosterView::LOGON,                TEXT("Logon"));
            AppendMenu(hmenu, MF_STRING, RosterView::LOGOFF,               TEXT("Logoff"));
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::RESOLVENICKNAMES,     TEXT("Resolve Nicknames"));
            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
        }
        AppendMenu(hmenu, MF_STRING , RosterView::OPENCHAT,                TEXT("Open chat"));

        AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

        if (!mr) {

            AppendMenu(hmenu, MF_STRING, RosterView::VCARD,                    TEXT("VCard"));
            AppendMenu(hmenu, MF_STRING, RosterView::CLIENTINFO,               TEXT("Client Info"));
            AppendMenu(hmenu, MF_STRING, RosterView::COMMANDS,                 TEXT("Commands"));

            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

            if (type==RosterGroup::ROSTER) {
                AppendMenu(hmenu, MF_STRING, RosterView::EDITCONTACT,          TEXT("Edit contact"));

                HMENU subscrMenu=CreatePopupMenu();
                AppendMenu(subscrMenu, MF_STRING, RosterView::SUBSCRIBE, TEXT("Ask subscription"));
                AppendMenu(subscrMenu, MF_STRING, RosterView::SUBSCRIBED, TEXT("Grant subscription"));
                AppendMenu(subscrMenu, MF_STRING, RosterView::UNSUBSCRIBED, TEXT("Revoke subscription"));

                AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenu,               TEXT("Subscription"));
            }

            if (type==RosterGroup::NOT_IN_LIST)
                AppendMenu(hmenu, MF_STRING, RosterView::ADDCONTACT,           TEXT("Add contact"));

            if (type!=RosterGroup::MUC && type!=RosterGroup::SELF_CONTACT) {
                AppendMenu(hmenu, MF_STRING, RosterView::DELETECONTACT,            TEXT("Delete"));

                AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

                AppendMenu(hmenu, MF_STRING, RosterView::SENDSTATUS,               TEXT("Send status"));
            }
            if (type!=RosterGroup::TRANSPORTS) {
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::SENDFILE,             TEXT("Send file"));
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::INVITE,               TEXT("Invite"));
            }
        }

        if (mr) {
            AppendMenu(hmenu, MF_STRING, RosterView::SENDSTATUS,               TEXT("Send status"));

            MucGroup::ref roomGrp;
            roomGrp=boost::dynamic_pointer_cast<MucGroup> (roster.lock()->findGroup(mr->group));
            MucContact::Role myRole=roomGrp->selfContact->role;
            MucContact::Affiliation myAff=roomGrp->selfContact->affiliation;
            if (myAff==MucContact::OWNER || myAff==MucContact::ADMIN) {
                AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::MLOUTCASTS,           TEXT("Outcasts/Banned"));
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::MLMEMBERS,            TEXT("Members"));
            }
            if (myAff==MucContact::OWNER) {
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::MLADMINS,             TEXT("Admins"));
                AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::MLOWNERS,             TEXT("Owners"));
                AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
                AppendMenu(hmenu, MF_STRING, RosterView::MUCCONFIG,               TEXT("Configure room"));
            }
        }

        MucContact * mc= dynamic_cast<MucContact *>(cursorPos.get());
        if (mc) {
            MucGroup::ref roomGrp;
            roomGrp=boost::dynamic_pointer_cast<MucGroup> (roster.lock()->findGroup(mc->group));
            MucContact::Role myRole=roomGrp->selfContact->role;
            MucContact::Affiliation myAff=roomGrp->selfContact->affiliation;

            bool canKick = (myRole==MucContact::MODERATOR && mc->role !=MucContact::MODERATOR);
            bool canBan = 
                myAff==MucContact::OWNER || 
                (myAff==MucContact::ADMIN 
                && (mc->affiliation==MucContact::PARTICIPANT || mc->affiliation==MucContact::MEMBER));

            if (!canKick && !canBan) return hmenu;
            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
            if (canKick) AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::MUCKICK, L"Kick");
            if (canBan)  AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::MUCBAN,  L"Ban");

            HMENU roleMenu=CreatePopupMenu();
            AppendMenu(roleMenu, MF_STRING | MF_GRAYED| (mc->role==MucContact::VISITOR ? MF_CHECKED : 0), RosterView::MUCVISITOR, L"Visitor");
            AppendMenu(roleMenu, MF_STRING | MF_GRAYED| (mc->role==MucContact::PARTICIPANT ? MF_CHECKED : 0), RosterView::MUCPARTICIPANT, L"Participant");
            AppendMenu(roleMenu, MF_STRING | MF_GRAYED| (mc->role==MucContact::MODERATOR ? MF_CHECKED : 0), RosterView::UNSUBSCRIBED, L"Moderator");

            if (myRole>=MucContact::MODERATOR) AppendMenu(hmenu, MF_POPUP, (LPARAM)roleMenu,               TEXT("Role"));

            HMENU afflMenu=CreatePopupMenu();
            AppendMenu(afflMenu, MF_STRING | MF_GRAYED| (mc->affiliation==MucContact::NONE ? MF_CHECKED : 0), RosterView::MUCNONE, L"None");
            AppendMenu(afflMenu, MF_STRING | MF_GRAYED| (mc->affiliation==MucContact::MEMBER ? MF_CHECKED : 0), RosterView::MUCMEMBER, L"Member");
            AppendMenu(afflMenu, MF_STRING | MF_GRAYED| (mc->affiliation==MucContact::ADMIN ? MF_CHECKED : 0), RosterView::MUCADMIN, L"Admin");
            AppendMenu(afflMenu, MF_STRING | MF_GRAYED| (mc->affiliation==MucContact::OWNER ? MF_CHECKED : 0), RosterView::MUCOWNER, L"Owner");

            if (myAff>=MucContact::ADMIN) AppendMenu(hmenu, MF_POPUP, (LPARAM)afflMenu,               TEXT("Affiliation"));

        }

    }

    return hmenu;
}

void RosterView::OnCommand( int cmdId, LONG lParam ) {
    if (roster.expired()) return;
    ResourceContextRef rc=roster.lock()->rc;

    IconTextElement::ref focused = boost::dynamic_pointer_cast<IconTextElement>(cursorPos);
    if (focused) focused->onCommand(cmdId, rc);

    Contact::ref focusedContact = boost::dynamic_pointer_cast<Contact>(cursorPos);

    if (focusedContact) {
        switch (cmdId) {
        case RosterView::OPENCHAT: 
            {
                openChat(focusedContact);
                break;
            }

        case RosterView::LOGON: 
            //
        case RosterView::LOGOFF: 
            rc->sendPresence(
                focusedContact->jid.getJid().c_str(), 
                (cmdId==RosterView::LOGON)? presence::ONLINE: presence::OFFLINE, 
                rc->presenceMessage, rc->priority );
            break;

        case RosterView::RESOLVENICKNAMES:
            break;

        case RosterView::VCARD: 
            {
                if (!rc->isLoggedIn()) break;
                WndRef vc=VcardForm::createVcardForm(tabs->getHWnd(), focusedContact->rosterJid, rc, false);
                tabs->addWindow(vc);
                tabs->switchByWndRef(vc);
                break;
            }
        case RosterView::CLIENTINFO: 
            {
                if (!rc->isLoggedIn()) break;
                const std::string &jid=(focusedContact->status==presence::OFFLINE)?
                    focusedContact->rosterJid : focusedContact->jid.getJid();
                WndRef ci=ClientInfoForm::createInfoForm(tabs->getHWnd(), jid, rc);
                tabs->addWindow(ci);
                tabs->switchByWndRef(ci);
                break;
            }
        case RosterView::COMMANDS:
            {
                ServiceDiscovery::ref disco=ServiceDiscovery::createServiceDiscovery(
                    tabs->getHWnd(), 
                    rc, 
                    focusedContact->jid.getJid(), 
                    "http://jabber.org/protocol/commands", 
                    true);
                tabs->addWindow(disco);
                tabs->switchByWndRef(disco);
            }
            break;
            //case RosterView::SUBSCR: 
        case RosterView::SUBSCRIBE:
        case RosterView::SUBSCRIBED: 
        case RosterView::UNSUBSCRIBED: 
            {
                presence::PresenceIndex subscr=presence::PRESENCE_AUTH_ASK;
                if (cmdId==RosterView::SUBSCRIBED) subscr=presence::PRESENCE_AUTH;
                if (cmdId==RosterView::UNSUBSCRIBED) subscr=presence::PRESENCE_AUTH_REMOVE;
                rc->sendPresence(focusedContact->rosterJid.c_str(),
                    subscr, std::string(), 0);
                break;
            }

        case RosterView::EDITCONTACT:
        case RosterView::ADDCONTACT:
            DlgAddEditContact::createDialog(getHWnd(), rc, focusedContact); break;

        case RosterView::DELETECONTACT:
            {
                std::wstring name=utf8::utf8_wchar(focusedContact->getFullName());
                int result=MessageBox(getHWnd(), name.c_str(), TEXT("Delete contact ?"), MB_YESNO | MB_ICONWARNING);
                if (result==IDYES) {
                    roster.lock()->deleteContact(focusedContact);
                }
                break;
            }
        case RosterView::SENDSTATUS:
            DlgStatus::createDialog(getHWnd(), rc, focusedContact); break;

        case RosterView::SENDFILE: 
        case RosterView::INVITE:
            break;

        case RosterView::MUCCONFIG:
            {
                MucConfigForm::ref mucconf=MucConfigForm::createMucConfigForm(
                    tabs->getHWnd(), 
                    focusedContact->jid.getBareJid(), 
                    rc);
                tabs->addWindow(mucconf);
                tabs->switchByWndRef(mucconf);
                break;
            }
            //case RosterView::RENAMEGRP:
        default:
            break;
        }
    }

    if (cmdId==ID_JABBER_ADDACONTACT){
        DlgAddEditContact::createDialog(getHWnd(), rc, Contact::ref());
    }
}
bool RosterView::showWindow( bool show ) {
    Wnd::showWindow(show);
    if (show) SetFocus(getHWnd());
    return false;
}
void RosterView::setIcon( int iconIndex ) {
    wt->setIcon(iconIndex);
    InvalidateRect(tabs->getHWnd(), NULL, false);
}

void RosterView::openChat(Contact::ref contact) {
    WndRef chat=tabs->getWindowByODR(contact);
    if (!chat) {
        //Contact::ref r=roster.lock()->findContact(c->jid.getJid());
        chat=WndRef(new ChatView(tabs->getHWnd(), contact));
        tabs->addWindow(chat);
    }
    tabs->switchByWndRef(chat);
}