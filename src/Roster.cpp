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

#include "TabCtrl.h"
#include "ChatView.h"

#include "DlgStatus.h"

extern TabsCtrlRef tabs;

Roster::Roster(ResourceContextRef rc){
    roster=VirtualListView::ref();
    this->rc=rc;
    createGroup("Self-Contact", RosterGroup::SELF_CONTACT);
    createGroup("Transports", RosterGroup::TRANSPORTS);
    createGroup("Not-In-List", RosterGroup::NOT_IN_LIST);
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
        //todo: ������� � ���������� �������
        JabberDataBlockRef jGroup=item->getChildByName("group"); 
        std::string group=(jGroup)? jGroup->getText() : "";

        /*if (group.length()==0)*/ if (jid.find('@')==std::string.npos) 
            group="Transports";

        std::string subscr=item->getAttribute("subscription");
        if (item->hasAttribute("ask")) {
            subscr+=',';
            subscr+="ask";
        }

        //todo: ������ ��������� ��� roster request � roster push
        //��� push �������������� ��� ���������� �� bareJid
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
            createGroup(group, RosterGroup::ROSTER);
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

    Contact::ref contact=getContactEntry(from);

    contact->status=typeIndex;
    contact->update();
    makeViewList();
}
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
        } else { 
            //todo: third attempt - clone contact from bareJidMap
            if ( bareJidMap.count(jid.getBareJid())!=0 ) {
                contact=bareJidMap[jid.getBareJid()];
                contact=contact->clone();
                contact->jid.setResource(jid.getResource());
                contacts.push_back(contact);
            } else {

                //finally - based on NOT-IN-LIST policy
                contact=Contact::ref(new Contact(jid.getBareJid(), jid.getResource(), ""));

                std::string group="Not-In-List";

                contact->subscr="NIL";
                contact->group=group;
                //bareJidMap[contact->jid.getBareJid()]=contact;
                contacts.push_back(contact);
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
    
    ODRList *list=new ODRList(); //������ �1

    for (GroupList::const_iterator gi=groups.begin(); gi!=groups.end(); gi++) {
        RosterGroup::ref group=*gi;
        list->push_back(group);

        int elemCount=0;

        if (!group->isExpanded()) continue;

        for (ContactList::const_iterator ci=contacts.begin(); ci!=contacts.end(); ci++) {
            Contact::ref contact=*ci;
            if (group->equals(contact->group)) {
                list->push_back(contact);
                elemCount++;
            }
        }

        if (elemCount==0) { list->pop_back();  continue; } 

    }

    //roster->bindODRList(odrlist);
    PostMessage(roster->getHWnd(), WM_USER+1, 0, (LPARAM)list); //������ �2
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

RosterGroup::ref Roster::createGroup( const std::string &name, RosterGroup::Type type ) 
{
    
    RosterGroup::ref newGrp=RosterGroup::ref(new RosterGroup(name, type));
    groups.push_back(newGrp);
    return newGrp;
}


RosterGroup::RosterGroup( const std::string &name, Type type ) {
    groupName=name;
    this->type=type;
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
    if (left->type < right->type) return true;
    if (left->type > right->type) return false;
    return left->groupName < right ->groupName;
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

        if (type==RosterGroup::TRANSPORTS) {
            AppendMenu(hmenu, MF_STRING, RosterView::LOGON,                TEXT("Logon"));
            AppendMenu(hmenu, MF_STRING, RosterView::LOGOFF,               TEXT("Logoff"));
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::RESOLVENICKNAMES,     TEXT("Resolve Nicknames"));
            AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
        }
        AppendMenu(hmenu, MF_STRING , RosterView::OPENCHAT,                TEXT("Open chat"));

        AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

        AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::VCARD,                    TEXT("VCard"));
        AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::CLIENTINFO,               TEXT("Client Info"));
        AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::COMMANDS,                 TEXT("Commands"));

        AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

        if (type==RosterGroup::ROSTER) {
            AppendMenu(hmenu, MF_STRING, RosterView::EDITCONTACT,          TEXT("Edit contact"));
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::SUBSCR,               TEXT("Subscription"));
        }

        if (type==RosterGroup::NOT_IN_LIST)
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::ADDCONTACT,           TEXT("Add contact"));

        AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::DELETECONTACT,            TEXT("Delete"));

        AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);

        AppendMenu(hmenu, MF_STRING, RosterView::SENDSTATUS,               TEXT("Send status"));
        if (type!=RosterGroup::TRANSPORTS) {
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::SENDFILE,             TEXT("Send file"));
            AppendMenu(hmenu, MF_STRING | MF_GRAYED, RosterView::INVITE,               TEXT("Invite"));
        }
    }

    return hmenu;
}

void RosterView::OnCommand( int cmdId, LONG lParam ) {
    Contact::ref focusedContact = boost::dynamic_pointer_cast<Contact>(cursorPos); // <<< Yes, I did it :))
    if (focusedContact) {
    switch (cmdId) {
        case RosterView::OPENCHAT: 
            {
                WndRef chat=tabs->getWindowByODR(cursorPos);
                if (!chat) {
                    //Contact::ref r=roster.lock()->findContact(c->jid.getJid());
                    chat=WndRef(new ChatView(tabs->getHWnd(), focusedContact));
                    tabs->addWindow(chat);
                }
                tabs->switchByWndRef(chat);
                break;
            }
        case RosterView::LOGON: 
        case RosterView::LOGOFF: 
        case RosterView::RESOLVENICKNAMES:
        case RosterView::VCARD: 
        case RosterView::CLIENTINFO: 
        case RosterView::COMMANDS:
        case RosterView::EDITCONTACT:
        case RosterView::SUBSCR: 
        case RosterView::ADDCONTACT:
        case RosterView::DELETECONTACT:
        case RosterView::SENDSTATUS:
            DlgStatus::createDialog(getHWnd(), roster.lock()->rc, focusedContact); break;
        case RosterView::SENDFILE: 
        case RosterView::INVITE:
        //case RosterView::RENAMEGRP:
        default:
            break;
    }
    }
}
void RosterView::showWindow( bool show ) {
    Wnd::showWindow(show);
    if (show) SetFocus(getHWnd());
}
void RosterView::setIcon( int iconIndex ) {
    wt->setIcon(iconIndex);
    InvalidateRect(tabs->getHWnd(), NULL, false);
}