//#include "stdafx.h"

#include "ResourceContext.h"
#include "EntityCaps.h"
#include "JabberStream.h"
#include <stdlib.h>

ResourceContext::ResourceContext() {
    status=presence::OFFLINE;
    priority=0;
}

char *statusVals []= { 
    NULL, "chat", "away", "xa",  "dnd", NULL 
};


void ResourceContext::sendPresence(const char *to, presence::PresenceIndex status, const std::string &message, int priority) {
    if (!isLoggedIn()) return;

    JabberDataBlock presenceStanza("presence");

    if (to) presenceStanza.setAttribute("to",to);

    switch (status) {
    case presence::OFFLINE:
        presenceStanza.setAttribute("type", "unavailable"); break;
    case presence::INVISIBLE:
        presenceStanza.setAttribute("type", "invisible"); break;
    case presence::ONLINE:
        break;
    default:
        presenceStanza.addChild("show", statusVals[status]);
    }
    
    presenceStanza.addChild("status", message.c_str());
    presenceStanza.addChild(EntityCaps::presenceEntityCaps());

    char spriority[6];
    _itoa_s(priority, spriority, sizeof(spriority), 10);
    presenceStanza.addChild("priority", spriority);

    //c->setAttribute("ext", "none");

    jabberStream->sendStanza(presenceStanza);
}