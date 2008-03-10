#pragma once
#include <vector>
#include "boostheaders.h"

#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"

typedef std::vector<JabberDataBlockListenerRef> JabberDataBlockListenersList;

class JabberStanzaDispatcher {
public:
	JabberStanzaDispatcher(ResourceContextRef resourceContext);
	~JabberStanzaDispatcher();

	BOOL dispatchDataBlock(JabberDataBlockRef block);

	void addListener(JabberDataBlockListenerRef listener);
    void removeListener(const std::string& id);
    void removeListener(JabberDataBlockListener *ptr);

private:
	JabberDataBlockListenersList listeners;
	ResourceContextRef rc;
};

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
