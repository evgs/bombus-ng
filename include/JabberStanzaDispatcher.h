#pragma once
#include <list>
#include <boost/smart_ptr.hpp>

#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"

class JabberStanzaDispatcher {
public:
	JabberStanzaDispatcher();
	~JabberStanzaDispatcher();

	void dispatchDataBlock(JabberDataBlockRef block);

	void addListener(JabberDataBlockListenerRef listener);
	void removeListener(const std::string& id);

private:
	std::list<JabberDataBlockListenerRef> listeners;
};

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
