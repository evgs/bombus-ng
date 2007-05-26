
#include "ProcessMUC.h"
#include "utf8.hpp"

#include "TabCtrl.h"
#include "ChatView.h"

extern TabsCtrlRef tabs;
extern RosterView::ref rosterWnd;

//////////////////////////////////////////////////////////////////////////
MucContact::ref getMucContactEntry(const std::string &jid, ResourceContextRef rc) {
    MucContact::ref c=boost::dynamic_pointer_cast<MucContact> (rc->roster->findContact(jid));
    if (!c) {
        c=MucContact::ref(new MucContact(jid));
        rc->roster->addContact(c);
    }
    return c;
}

enum MPA {
    NONE=0,
    ENTER=1,
    LEAVE=2
};

char * roleName[]= { "none", "visitor", "participant", "moderator" };
char * affiliationName[]= { "outcast", "none", "member", "admin", "owner" };

MucContact::Role getRoleIndex(const std::string &role) {
    if (role=="visitor") return MucContact::VISITOR;
    if (role=="participant") return MucContact::PARTICIPANT;
    if (role=="moderator") return MucContact::MODERATOR;
    return MucContact::NONE_ROLE;
}
MucContact::Affiliation getAffiliationIndex(const std::string &role) {
    if (role=="outcast") return MucContact::OUTCAST;
    if (role=="member") return MucContact::MEMBER;
    if (role=="admin") return MucContact::ADMIN;
    if (role=="owner") return MucContact::OWNER;
    return MucContact::NONE;
}
//////////////////////////////////////////////////////////////////////////
ProcessResult ProcessMuc::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef xmuc=block->findChildNamespace("x", "http://jabber.org/protocol/muc");
    if (!xmuc) return BLOCK_REJECTED;

    const std::string &from=block->getAttribute("from");
    const std::string &type=block->getAttribute("type");

    Jid roomNode;
    roomNode.setJid(from);

    std::string message;

    //1. group
    MucGroup::ref roomGrp;
    roomGrp=boost::dynamic_pointer_cast<MucGroup> (rc->roster->findGroup(roomNode.getBareJid()));

    if (!roomGrp) return BLOCK_PROCESSED; //dropped presence

    MucContact::ref c=getMucContactEntry(from, rc);

    MPA action=NONE;

    if (type=="error") {
        JabberDataBlockRef error=block->getChildByName("error");
        int errCode=atoi(error->getAttribute("code").c_str());

        //todo: if (status>=Presence.PRESENCE_OFFLINE) testMeOffline();
        action=LEAVE;
        
        //todo: if (errCode!=409 || status>=Presence.PRESENCE_OFFLINE)  setStatus(presenceType);

        std::string errText=error->getChildText("text");
        if (errText.length()>0) message=errText; // if error description is provided by server
        else // legacy codes
            switch (errCode) {
                case 401: message="Password required";
                case 403: message="You are banned in this room";
                case 404: message="Room does not exists";
                case 405: message="You can't create room on this server";
                case 406: message="Reserved roomnick must be used";
                case 407: message="This room is members-only";
                case 409: message="Nickname is already in use by another occupant";
                case 503: message="Maximum number of users has been reached in this room";
                default: message=*(error->toXML());
            }
    } else {
        JabberDataBlockRef item=xmuc->getChildByName("item");   

        MucContact::Role role = 
            getRoleIndex(item->getAttribute("role"));
        c->sortKey=MucContact::MODERATOR-role;
        
        MucContact::Affiliation affiliation = 
            getAffiliationIndex(item->getAttribute("affiliation"));

        boolean roleChanged= c->role != role;
        boolean affiliationChanged= c->affiliation !=affiliation;
    
        c->role=role;
        c->affiliation=affiliation;

        

        //setSortKey(nick);

        if (role==MucContact::MODERATOR) {
            c->transpIndex=icons::ICON_MODERATOR_INDEX;
        } else {
            c->transpIndex=0;
        }

        JabberDataBlockRef statusBlock=xmuc->getChildByName("status");
        int statusCode=(statusBlock)? atoi(statusBlock->getAttribute("code").c_str()) : 0; 

        message=c->jid.getResource(); // nick

        if (type=="unavailable") {
            action=LEAVE;
            std::string reason=item->getChildText("reason");

            switch (statusCode) {
            case 303:
                message+=" is now known as ";
                message+=item->getAttribute("nick");
                c->jid.setResource(item->getAttribute("nick"));
                c->rosterJid=c->jid.getJid(); //for vCard
                c->update();
                action=NONE;
                break;

            case 307: //kick
            case 301: //ban
                message+=(statusCode==307)?" was kicked " : " was banned ";
                message+="(";
                message+=reason;
                message+=")";

                if (c->realJid.length()>0){
                    message+=" - ";
                    message+=c->realJid;
                }
                break;

            case 321:
                message+=" has been unaffiliated and kicked from members-only room";
                break;

            case 322:
                message+=" has been kicked because room became members-only";
                break;

            default:
                {
                    message+=" has left the channel";
                    const std::string & status=block->getChildText("status");
                    message+="(";
                    message+=status;
                    message+=")";
                }
            }
        } else { //onlines
            action=ENTER;
            if (c->status>=presence::OFFLINE) {
                // first online
                std::string realJid=item->getAttribute("jid");
                if (realJid.length()>0) {
                    c->realJid=realJid;
                    message+=" (";
                    message+=realJid;  //for moderating purposes
                    message+=")";
                }
                message+=" has joined the channel as ";
                message+=roleName[role];

                if (affiliation!=MucContact::NONE) {
                    message+=" and ";
                    message+=affiliationName[affiliation-MucContact::OUTCAST];

                    const std::string & status=block->getChildText("status");
                    message+=" (";
                    message+=status;
                    message+=")";
                }
            } else {
                //change status
                message+=" is now ";

                if ( roleChanged ) message+=roleName[role];
                if (affiliationChanged) {
                    if (roleChanged) message+=" and ";
                    message+=(affiliation==MucContact::NONE)? 
                        "unaffiliated" : affiliationName[affiliation-MucContact::OUTCAST];
                }
                if (!roleChanged && !affiliationChanged) {
                    const std::string &show=block->getChildText("show");
                    if (show.length()==0) message+="online";
                    else message+=show;

                    const std::string & status=block->getChildText("status");
                    message+=" (";
                    message+=status;
                    message+=")";

                }
            }
        }
    }

    if (c.get()==roomGrp->selfContact.get()) {
        switch (action) {
            case ENTER:
                // room contact is online
                roomGrp->room->status=presence::ONLINE;
                break;
            case LEAVE:
                // room contact is offline
                roomGrp->room->status=presence::OFFLINE;
                // make all occupants offline
                rc->roster->setStatusByFilter(roomNode.getBareJid(), presence::OFFLINE);
                break;
        }
    }

    {
        ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(c).get());

        bool ascroll=(cv==NULL)? false: cv->autoScroll();

        c->processPresence(block);

        if (ascroll) {
            cv->moveEnd();
        }
        if (cv) if (IsWindowVisible(cv->getHWnd())) cv->redraw();
    }
    rc->roster->makeViewList();


    {
        Message::ref msg=Message::ref(new Message(message, from, false, Message::PRESENCE, Message::extractXDelay(block) ));

        Contact::ref room=roomGrp->room;

        ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(room).get());

        bool ascroll=(cv==NULL)? false: cv->autoScroll();
        room->messageList->push_back(msg);

        if (ascroll) {
            cv->moveEnd();
        }
        if (cv) if (IsWindowVisible(cv->getHWnd())) cv->redraw();
    }

    return BLOCK_PROCESSED;
}


void ProcessMuc::initMuc( const std::string &jid, const std::string &password, ResourceContextRef rc) {
    Jid roomNode;
    roomNode.setJid(jid);

    //1. group
    MucGroup::ref roomGrp;
    roomGrp=boost::dynamic_pointer_cast<MucGroup> (rc->roster->findGroup(roomNode.getBareJid()));
    if (!roomGrp) {
        roomGrp=MucGroup::ref(new MucGroup(roomNode.getBareJid(), roomNode.getUserName()));
        rc->roster->addGroup(roomGrp);
    }
    roomGrp->password=password;

    //2. room contact
    MucRoom::ref room=roomGrp->room;
    if (!room) {
        room=MucRoom::ref(new MucRoom(jid));
        roomGrp->room=room;
        room->group=roomGrp->getName();
    }

    rosterWnd->openChat(room);
    

    //3. selfcontact
    roomGrp->selfContact=getMucContactEntry(jid, rc);
}

//////////////////////////////////////////////////////////////////////////

MucGroup::MucGroup( const std::string &jid, const std::string &name ) {
    groupName=jid;
    this->type=MUC;
    sortKey=utf8::utf8_wchar(name);
    wstr=sortKey;
    expanded=true;
    init();
}

void MucGroup::addContacts( ODRList *list ) {
    list->push_back(room); //there are no MucRoom contact in roster data, 
    //room messages will be count separately ;)
}


//////////////////////////////////////////////////////////////////////////
MucRoom::MucRoom( const std::string &jid ) {
    this->jid=Jid(jid);
    this->rosterJid=jid;
    //this->nickname=nickname;
    this->status=presence::OFFLINE;
    offlineIcon=presence::OFFLINE;

    nUnread=0;

    transpIndex=icons::ICON_GROUPCHAT_INDEX;

    update();
    messageList=ODRListRef(new ODRList);
}

void MucRoom::update() {
    wjid=utf8::utf8_wchar( jid.getBareJid() );
    init();
}
//////////////////////////////////////////////////////////////////////////
MucContact::MucContact( const std::string &jid ) 
{
    this->jid=Jid(jid);
    this->rosterJid=jid;

    this->group=this->jid.getBareJid();

    this->status=presence::OFFLINE;
    offlineIcon=presence::OFFLINE;

    nUnread=0;

    transpIndex=0;

    update();
    messageList=ODRListRef(new ODRList);
}

void MucContact::update() {
    wjid=utf8::utf8_wchar( jid.getResource() );
    init();
}
