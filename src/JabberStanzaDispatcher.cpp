//#include "stdafx.h"

#include "JabberStanzaDispatcher.h"

JabberStanzaDispatcher::JabberStanzaDispatcher(ResourceContextRef resourceContext) {
	rc=resourceContext; 
}

JabberStanzaDispatcher::~JabberStanzaDispatcher(){};

BOOL JabberStanzaDispatcher::dispatchDataBlock(JabberDataBlockRef block){

    //StringRef sdebug=block->toXML();

	const std::string & blockId=block->getAttribute("id");
	const std::string & blockType=block->getAttribute("type");
	const std::string & blockTagName=block->getTagName();


    int i=0;
    while (i<listeners.size()) 
	{
        JabberDataBlockListener *p=listeners[i].get();
        i++;

		const char * tag=p->getTagName();
		if (tag!=NULL) if ( blockTagName.compare(tag) ) continue;

		const char * type=p->getType();
		if (type!=NULL) if (blockType.compare(type)) continue;

		const char * id=p->getId();
		if (id!=NULL) if (blockId.compare(id)) continue;

		ProcessResult result=p->blockArrived(block, rc);

		if (result==BLOCK_PROCESSED) return true;
		
		if (result==LAST_BLOCK_PROCESSED) { listeners.erase(listeners.begin()+i-1); return true; }

        if (result==CANCEL) listeners.erase(listeners.begin()+i-1);

		//if (result==BLOCK_REJECTED) continue;
	}

    return false;
};

void JabberStanzaDispatcher::addListener(JabberDataBlockListenerRef listener){
	listeners.push_back(listener);
}

void JabberStanzaDispatcher::removeListener(const std::string& byId){
    int index=0;
    while (index<listeners.size()) {
		const char * id=listeners[index].get()->getId();
        if (id!=NULL) if (!byId.compare(byId)) {
            listeners.erase(listeners.begin()+index); 
        } else index++;
	}
}

void JabberStanzaDispatcher::removeListener(JabberDataBlockListener *ptr){
    for (JabberDataBlockListenersList::iterator i = listeners.begin();
        i!=listeners.end();
        i++) 
    {
        if (i->get()==ptr) {
            listeners.erase(i); return;
        }
    }
}
