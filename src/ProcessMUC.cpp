
#include "ProcessMUC.h"
#include "utf8.hpp"

ProcessResult ProcessMuc::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef ns=block->findChildNamespace("x", "http://jabber.org/protocol/muc");
    if (!ns) return BLOCK_REJECTED;



    //if ()

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
    }

    //3. selfcontact
    MucContact::ref self=boost::dynamic_pointer_cast<MucContact> (rc->roster->findContact(jid));
    if (!self) {
        self=MucContact::ref(new MucContact(jid));
        rc->roster->addContact(self);
    }
    roomGrp->selfContact=self;
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
