#pragma once
#include <list>
#include <boost/smart_ptr.hpp>

#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"

class JabberStanzaDispatcher {
public:
	JabberStanzaDispatcher(ResourceContextRef resourceContext);
	~JabberStanzaDispatcher();

	void dispatchDataBlock(JabberDataBlockRef block);

	void addListener(JabberDataBlockListenerRef listener);
	void removeListener(const std::string& id);

private:
	std::list<JabberDataBlockListenerRef> listeners;
	ResourceContextRef rc;
};

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
