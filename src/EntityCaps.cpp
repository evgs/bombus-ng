#include "EntityCaps.h"
#include "JabberStream.h"

#include <string>

extern std::string appVersion;

char *features[]={
    "jabber:iq:version", 
    //"jabber:iq:last", //todo:
    //"jabber:iq:time", //todo:
    "jabber:x:data", 
    "jabber:x:event",
    "http://jabber.org/protocol/disco#info",
    "http://jabber.org/protocol/muc",
    "http://www.xmpp.org/extensions/xep-0199.html#ns"
};

ProcessResult EntityCaps::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
    JabberDataBlockRef query=block->getChildByName("query");
    if (!query) return BLOCK_REJECTED;
    std::string xmlns=query->getAttribute("xmlns");
    if (xmlns=="http://jabber.org/protocol/caps") { 
        if (query->getAttribute("node").find("http://bombus-im.org/ng")<0) return BLOCK_REJECTED;
    } else {
        if (xmlns!="http://jabber.org/protocol/disco#info") return BLOCK_REJECTED;
    }

    JabberDataBlock result ("iq");
    result.setAttribute("to", block->getAttribute("from"));
    result.setAttribute("type", "result");
    result.setAttribute("id", block->getAttribute("id"));
    result.addChild(query);

    JabberDataBlockRef identity=query->addChild("identity",NULL);
    identity->setAttribute("category","client");
    identity->setAttribute("type","handheld");
    identity->setAttribute("name","Bombus-ng");

    for (int i=0; i<sizeof(features)/sizeof(char *); i++) {
        query->addChild("feature", NULL)->setAttribute("var", features[i]);
    }

    rc->jabberStream->sendStanza(result);

    return BLOCK_PROCESSED;
    
}

JabberDataBlockRef EntityCaps::presenceEntityCaps() {
    JabberDataBlockRef c=JabberDataBlockRef(new JabberDataBlock("c"));
    c->setAttribute("xmlns", "http://jabber.org/protocol/caps");
    c->setAttribute("node", "http://bombus-im.org/ng");
    c->setAttribute("ver", appVersion.c_str());
    c->setAttribute("ext", appVersion.c_str());

    return c;
}