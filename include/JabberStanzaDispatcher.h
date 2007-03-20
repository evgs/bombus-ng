#pragma once
#include <list>
#include <boost/smart_ptr.hpp>

#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"

typedef std::list<JabberDataBlockListenerRef> JabberDataBlockListenersList;

class JabberStanzaDispatcher {
public:
	JabberStanzaDispatcher(ResourceContextRef resourceContext);
	~JabberStanzaDispatcher();

	BOOL dispatchDataBlock(JabberDataBlockRef block);

	void addListener(JabberDataBlockListenerRef listener);
	void removeListener(const std::string& id);

private:
	JabberDataBlockListenersList listeners;
	ResourceContextRef rc;
};

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
