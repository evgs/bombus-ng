//#include "stdafx.h"
#include "JabberStream.h"

#include "ResourceContext.h"
#include "EntityCaps.h"
#include "HostFeatures.h"

#include <Presence.h>
#include <stdlib.h>

ResourceContext::ResourceContext() {
    status=presence::OFFLINE;
    priority=0;
    myCaps=MyCapsRef(new MyCaps());
    hostFeatures=HostCapsRef(new HostCaps());
}

char *statusVals []= { 
    NULL, "chat", "away", "xa",  "dnd", NULL 
};


void ResourceContext::sendPresence(const char *to, presence::PresenceIndex status, const std::string &message, int priority) {
    if (!isLoggedIn()) return;

    JabberDataBlockRef prs=constructPresence(to, status, message, priority);

    prs->addChild(myCaps->presenceEntityCaps());

    jabberStream->sendStanza(prs);
    if (to) return;
    prs->setAttribute("from", myJid.getJid());
    jabberStanzaDispatcherRT->dispatchDataBlock(prs);
}

JabberDataBlockRef presence::constructPresence(const char *to, presence::PresenceIndex status, const std::string &message, int priority) {
    JabberDataBlockRef presenceStanza=JabberDataBlockRef(new JabberDataBlock("presence"));

    if (to) presenceStanza->setAttribute("to",to);

    switch (status) {
    case presence::PRESENCE_AUTH_ASK: 
        presenceStanza->setAttribute("type", "subscribe");
        return presenceStanza;
    case presence::PRESENCE_AUTH: 
        presenceStanza->setAttribute("type", "subscribed");
        return presenceStanza;
        //TODO: unsubscribe, unsubscribed

    case presence::OFFLINE:
        presenceStanza->setAttribute("type", "unavailable"); break;
    case presence::INVISIBLE:
        presenceStanza->setAttribute("type", "invisible"); break;
    case presence::ONLINE:
        break;
    default:
        presenceStanza->addChild("show", statusVals[status]);
    }

    presenceStanza->addChild("status", message.c_str());

    char spriority[6];
    _itoa_s(priority, spriority, sizeof(spriority), 10);
    presenceStanza->addChild("priority", spriority);

    //TODO:
    //presenceStanza->addChildNS("nick", "http://jabber.org/protocol/nick")->setText(nick);

    return presenceStanza;
}