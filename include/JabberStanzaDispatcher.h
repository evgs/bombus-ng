#pragma once
#include <list>
#include <boost/smart_ptr.hpp>

#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"

class JabberStanzaDispatcher {
public:
	JabberStanzaDispatcher(ResourceContext * resourceContext);
	~JabberStanzaDispatcher();

	void dispatchDataBlock(JabberDataBlockRef block);

	void addListener(JabberDataBlockListenerRef listener);
	void removeListener(const std::string& id);

private:
	std::list<JabberDataBlockListenerRef> listeners;
	ResourceContext * rc;
};

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
