#pragma once

#include <boost/smart_ptr.hpp>
#include <map>
#include <stack>

#include "basetypes.h"
#include "ResourceContext.h"
#include "Socket.h"
#include "JabberDataBlock.h"
#include "JabberListener.h"
#include "JabberStanzaDispatcher.h"
#include "XMLEventListener.h"
#include "XMLParser.h"

class JabberStream : public XMLEventListener{

public:
	JabberStream(void);
	JabberStream(ResourceContextRef rc, JabberListenerRef listener);
	~JabberStream(void);

	void sendStanza(JabberDataBlockRef stanza);
	void sendStanza(JabberDataBlock &stanza);
	void sendXmlVersion(void);
	void sendXmppBeginHeader();
	void sendXmppEndHeader(void);

	virtual void tagStart(const std::string & tagname, const StringMap &attr);
	virtual void tagEnd(const std::string & tagname);
	virtual void plainTextEncountered(const std::string & body);

	XMLParserRef parser;

    JabberListenerRef jabberListener;


    ConnectionRef connection;

private:
	ResourceContextRef rc;

	bool isRunning;

	std::stack<JabberDataBlockRef> stanzaStack;

public:
	static void run(JabberStream * _stream); 
	// todo: static void run(JabberStreamRef _stream);
};

typedef boost::shared_ptr<JabberStream> JabberStreamRef;
