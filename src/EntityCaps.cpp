#include "JabberStream.h"

#include "EntityCaps.h"

#include <string>
#include <functional>
#include <algorithm>
#include "crypto/SHA1.h"
#include "config.h"

extern std::string appVersion;
extern std::string appName;

std::string EntityCaps::capsHash="";

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

    identity->setAttribute("category", rc->myCaps->identityCategory.c_str());
    identity->setAttribute("type", rc->myCaps->identityType.c_str());
    identity->setAttribute("name", rc->myCaps->identityName.c_str());

    rc->myCaps->appendFeatures(query);

    rc->jabberStream->sendStanza(result);

    return BLOCK_PROCESSED;
    
}

JabberDataBlockRef ClientCaps::presenceEntityCaps() {
    JabberDataBlockRef c=JabberDataBlockRef(new JabberDataBlock("c"));
    c->setAttribute("xmlns", "http://jabber.org/protocol/caps");
    c->setAttribute("node", node);

    c->setAttribute("ver", getCapsHash());
    c->setAttribute("hash", alg);
    //c->setAttribute("ver", appVersion.c_str());
    //c->setAttribute("ext", appVersion.c_str());

    return c;
}

void ClientCaps::appendFeatures( JabberDataBlockRef result ) const {
    for (size_t i=0; i<features.size(); i++) {
        result->addChild("feature", NULL)->setAttribute("var", features[i]);
    }

}
typedef boost::shared_ptr<MessageDigest> MDRef;

const std::string & ClientCaps::getCapsHash() {
    if (capsHash.length()) return capsHash;

    //sorting
    std::stable_sort(features.begin(), features.end(), std::less<std::string>() );

    MDRef hashAlg=MDRef(new SHA1());
    hashAlg->init();

    hashAlg->updateASCII(identityCategory.c_str());
    hashAlg->updateByte('/');
    hashAlg->updateASCII(identityType.c_str());
    hashAlg->updateASCII("//");
    hashAlg->updateASCII(identityName.c_str());
    hashAlg->updateASCII("<");

    for (size_t i=0; i<features.size(); i++) {
        hashAlg->updateASCII(features[i]);
        hashAlg->updateASCII("<");
    }

    hashAlg->finish();
    capsHash=hashAlg->getDigestBase64();
    return capsHash;
}

void ClientCaps::addFeature( const std::string feature ) {
    for (size_t i=0; i<features.size(); i++) {
        if (features[i]==feature) return;
    }
    features.push_back(feature);
    capsHash.clear();
}
void ClientCaps::removeFeature( const std::string feature ) {
    for (size_t i=0; i<features.size(); i++) {
        if (features[i]==feature) {
            features.erase(features.begin()+i);
            capsHash.clear();
            return;
        }
    }
}

ClientCaps::ClientCaps() {
    alg="sha-1";
}

MyCaps::MyCaps() {
    identityName=appName+" "+appVersion;
    identityCategory="client";
    identityType="handheld";
    node="http://bombus-im.org/ng#";
    node+=identityName;
    alg="sha-1";

    addFeature("http://jabber.org/protocol/disco#info");
    addFeature("http://jabber.org/protocol/muc");
    addFeature("http://www.xmpp.org/extensions/xep-0199.html#ns");
    addFeature("jabber:iq:last");   // last activity
    addFeature("jabber:iq:time");  //todo: replace with "urn:xmpp:time"
    addFeature("jabber:iq:version");
    addFeature("jabber:x:data");

    if (Config::getInstance()->delivered)
        addFeature("urn:xmpp:receipts");    //xep-0184

    if (Config::getInstance()->composing)
        addFeature("http://jabber.org/protocol/chatstates");

    //addFeature("jabber:x:event");   // xep-0022; deprecated

    addFeature("urn:xmpp:ping");    // xep-0199
    addFeature("urn:xmpp:time");
}