//#include "stdafx.h"

#include "Jid.h"
#include "Contact.h"
#include "utf8.hpp"
#include "Image.h"
#include "TimeFunc.h"
//////////////////////////////////////////////////////////////////////////
extern ImgListRef skin;

int identifyTransport(const std::string &jid) {
    int serv=jid.find('@')+1;
    int dotsrv=jid.find('.', serv);
    if (dotsrv<0) return 0;

    Skin * il= dynamic_cast<Skin *>(skin.get());
    if (!il) return 0;
    return il->getBaseIndex(jid.substr(serv, dotsrv-serv));
}
//////////////////////////////////////////////////////////////////////////

Contact::Contact(const std::string &jid, const std::string &resource, const std::string &nickname) {
    this->jid=Jid(jid, resource);
    this->rosterJid=jid;
    this->nickname=nickname;
    this->status=presence::OFFLINE;
    offlineIcon=presence::OFFLINE;

    nUnread=0;
    sortKey=0;

    transpIndex=identifyTransport(jid);

    update();
    messageList=ODRListRef(new ODRList);
}

Contact::ref Contact::clone() {
    Contact::ref c=Contact::ref(new Contact(jid.getBareJid(), jid.getResource(), nickname));
    c->subscr=subscr;
    c->group=group;
    c->offlineIcon=offlineIcon;
    return c;
}

/*bool Contact::hasUnreadMsgs() {
    for ( ODRList::const_iterator i=messageList->begin();   i!=messageList->end();  i++ ) {
        ODRRef r=*i;
        Message *m = dynamic_cast<Message *>(r.get());
        if(m) {
        	if (m->unread) return true;
        }
    }
    return false;
}*/

int Contact::getColor() const{ return 0; }
int Contact::getIconIndex() const{ 
    if (nUnread>0) return icons::ICON_MESSAGE_INDEX;
    int icon=(status==presence::OFFLINE)? offlineIcon: status;
    if (icon<=presence::UNKNOWN) return icon+transpIndex; 
    return icon;
}

const wchar_t * Contact::getText() const{ return wjid.c_str(); }
const std::string Contact::getFullName() const{
    if (nickname.length()==0) return rosterJid;
    return nickname+" <"+rosterJid+">";
}
//////////////////////////////////////////////////////////////////////////
bool Contact::compare( Contact::ref left, Contact::ref right ) {
    Contact *l=left.get();
    Contact *r=right.get();
    if (l->sortKey < r->sortKey) return true;
    if (l->sortKey > r->sortKey) return false;
    if (l->status < r->status) return true;
    if (l->status > r->status) return false;
    return (_wcsicmp(left->getText(), right->getText()) < 0);
}
void Contact::update() {
    std::string s=(nickname.empty())? jid.getBareJid():nickname;
    std::string resource=jid.getResource();
    if (resource.length()) { s+='/'; s+=resource; }
    wjid=utf8::utf8_wchar( s );
    init();
}

void Contact::processPresence( JabberDataBlockRef block ) {

    std::string type=block->getAttribute("type");
    std::string priority=block->getChildText("priority");
    std::string status=block->getChildText("status");

    presence::PresenceIndex typeIndex=presence::OFFLINE;
    presence::PresenceIndex type2=presence::NOCHANGE; //no change
    Message::MsgType msgType=Message::PRESENCE;

    if (type=="unavailable") { 
        typeIndex=presence::OFFLINE;
        type="offline";
    } else if (type=="subscribe") { 
        msgType=Message::PRESENCE_ASK_SUBSCR;
        //TODO:
    } else if (type=="subscribed") {
        msgType=Message::PRESENCE_SUBSCRIBED;
        //TODO:
    } else if (type=="unsubscribe") {
        //TODO:
    } else if (type=="unsubscribed") {
        msgType=Message::PRESENCE_UNSUBSCRIBED;
        //TODO:
    } else if (type=="error") {
        typeIndex=presence::OFFLINE;
        type2=presence::PRESENCE_ERROR;
        //todo: extract error text here
    } else {
        type=block->getChildText("show");
        if (type=="chat") typeIndex=presence::CHAT; else
            if (type=="away") typeIndex=presence::AWAY; else
                if (type=="xa") typeIndex=presence::XA; else
                    if (type=="dnd") typeIndex=presence::DND; else {
                        typeIndex=presence::ONLINE;
                        type="online";
                    }
    }



    this->status=typeIndex;
    if (type2!=presence::NOCHANGE) this->offlineIcon=type2;
    update();

    std::string body=type;
    body+=" (";
    body+=status;
    body+=") [#]"; //todo: priority

    Message::ref msg=Message::ref(new Message(body, block->getAttribute("from"), msgType, Message::extractXDelay(block) ));

    if (messageList->size()==1) {
        //verify if it is presence;
        Message::ref mfirst=boost::dynamic_pointer_cast<Message>(messageList->front());
        if (mfirst) if (mfirst->type==Message::PRESENCE)
            this->messageList->erase( this->messageList->begin());
    }
    this->messageList->push_back(msg);
}