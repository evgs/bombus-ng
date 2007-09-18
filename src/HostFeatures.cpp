#include "HostFeatures.h"
#include "JabberStream.h"
#include "TimeFunc.h"
#include "Log.h"

ProcessResult HostFeatures::blockArrived( JabberDataBlockRef block, const ResourceContextRef rc ) {
    rc->hostFeatures=HostCapsRef(new HostCaps());

    std::string &type=block->getAttribute("type");
    if (type=="error") {
        return LAST_BLOCK_PROCESSED;
    }

    if (type=="result") {
        JabberDataBlockRef qry=block->findChildNamespace("query","http://jabber.org/protocol/disco#info");
        if (!qry) return LAST_BLOCK_PROCESSED;

        Log::getInstance()->msg("Host features queried");

        JabberDataBlockRefList::iterator i=qry->getChilds()->begin();
        while (i!=qry->getChilds()->end()) {
            JabberDataBlockRef item=*(i++);
            const std::string &tagName=item->getTagName();

            if (tagName=="feature") {
                rc->hostFeatures->addFeature(item->getAttribute("var"));
                //Log::getInstance()->msg(item->getAttribute("var"));
            }
        }
    }
    return LAST_BLOCK_PROCESSED;
}

void HostFeatures::discoverFeatures(ResourceContextRef rc) {
    JabberDataBlock qry("iq");
    qry.setAttribute("type","get");
    qry.setAttribute("id","hf");
    qry.setAttribute("to",rc->myJid.getServer());

    qry.addChildNS("query","http://jabber.org/protocol/disco#info");

    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(new HostFeatures()));
    rc->jabberStream->sendStanza(qry);
}
