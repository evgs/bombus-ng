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


    for (JabberDataBlockListenersList::iterator i = listeners.begin();
		 i!=listeners.end();
		 i++) 
	{
		const char * tag=i->get()->getTagName();
		if (tag!=NULL) if ( blockTagName.compare(tag) ) continue;

		const char * type=i->get()->getType();
		if (type!=NULL) if (blockType.compare(type)) continue;

		const char * id=i->get()->getId();
		if (id!=NULL) if (blockId.compare(id)) continue;

		ProcessResult result=i->get()->blockArrived(block, rc);

		if (result==BLOCK_PROCESSED) return true;
		
		if (result==LAST_BLOCK_PROCESSED) { listeners.erase(i); return true; }

		//if (result==BLOCK_REJECTED) continue;
	}

    return false;
};

void JabberStanzaDispatcher::addListener(JabberDataBlockListenerRef listener){
	listeners.push_back(listener);
}

void JabberStanzaDispatcher::removeListener(const std::string& byId){
	for (JabberDataBlockListenersList::iterator i = listeners.begin();
		 i!=listeners.end();
		 i++) 
	{
		const char * id=i->get()->getId();
		if (id!=NULL) if (!byId.compare(byId)) listeners.erase(i);
	}
}
