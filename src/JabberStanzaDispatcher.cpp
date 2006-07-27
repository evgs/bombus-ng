#include "JabberStanzaDispatcher.h"

JabberStanzaDispatcher::JabberStanzaDispatcher(ResourceContextRef resourceContext) {
	rc=resourceContext; 
}

JabberStanzaDispatcher::~JabberStanzaDispatcher(){};

void JabberStanzaDispatcher::dispatchDataBlock(JabberDataBlockRef block){

	const std::string & blockId=block->getAttribute("id");
	const std::string & blockType=block->getAttribute("type");
	const std::string & blockTagName=block->getTagName();


	for (std::list <JabberDataBlockListenerRef>::iterator i = listeners.begin();
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

		if (result==BLOCK_PROCESSED) return;
		
		if (result==LAST_BLOCK_PROCESSED) { listeners.erase(i); return; }

		//if (result==BLOCK_REJECTED) continue;
	}
};

void JabberStanzaDispatcher::addListener(JabberDataBlockListenerRef listener){
	listeners.push_back(listener);
}

void JabberStanzaDispatcher::removeListener(const std::string& byId){
	for (std::list <JabberDataBlockListenerRef>::iterator i = listeners.begin();
		 i!=listeners.end();
		 i++) 
	{
		const char * id=i->get()->getId();
		if (id!=NULL) if (!byId.compare(byId)) listeners.erase(i);
	}
}
