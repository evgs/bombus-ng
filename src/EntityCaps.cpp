#include "EntityCaps.h"
#include "JabberStream.h"

#include <string>
#include "crypto/SHA1.h"

extern std::string appVersion;

std::string EntityCaps::capsHash="";

char *features[]={
    "http://jabber.org/protocol/disco#info",
    "http://jabber.org/protocol/muc",
    "http://www.xmpp.org/extensions/xep-0199.html#ns",
    "jabber:iq:last",   // last activity
    "jabber:iq:time",  //todo: replace with "urn:xmpp:time"
    "jabber:iq:version", 
    "jabber:x:data", 
    "jabber:x:event",   // composing, delivered
    "urn:xmpp:time"
};
#define featuresSize (sizeof(features)/sizeof(features[0]))

ProcessResult EntityCaps::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
    JabberDataBlockRef query=block->getChildByName("query");
    if (!query) return BLOCK_REJECTED;
    std::string xmlns=query->getAttribute("xmlns");
    if (xmlns!="http://jabber.org/protocol/disco#info") return BLOCK_REJECTED;
    std::string node=query->getAttribute("node");
    if (node.length()>0) 
      if (node.find("http://bombus-im.org/ng")<0) return BLOCK_REJECTED;

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
    std::string node="http://bombus-im.org/ng#"; node+=appVersion.c_str();
    c->setAttribute("node", node);

    c->setAttribute("ver", getCapsHash());
    c->setAttribute("hash", "sha-1");
    //c->setAttribute("ver", appVersion.c_str());
    //c->setAttribute("ext", appVersion.c_str());

    return c;
}

typedef boost::shared_ptr<SHA1> SHA1Ref;

const std::string & EntityCaps::getCapsHash() {
    if (capsHash.length()) return capsHash;

    SHA1Ref sha1=SHA1Ref(new SHA1());
    sha1->init();

    sha1->updateASCII("client/handheld");
    sha1->updateASCII("<");

    for (int i=0; i<featuresSize; i++) {
        sha1->updateASCII(features[i]);
        sha1->updateASCII("<");
    }

    sha1->finish();
    capsHash=sha1->getDigestBase64();
    return capsHash;
}