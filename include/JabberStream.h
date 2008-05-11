#pragma once

#include "Socket.h"
#include "boostheaders.h"
#include <map>
#include <stack>

#include "basetypes.h"
#include "ResourceContext.h"
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
    void parseStream();

	virtual void tagStart(const std::string & tagname, const StringMap &attr);
	virtual bool tagEnd(const std::string & tagname);
	virtual void plainTextEncountered(const std::string & body);

	XMLParserRef parser;

    JabberListenerRef jabberListener;


    ConnectionRef connection;

private:
	ResourceContextRef rc;

	bool isRunning;

	std::stack<JabberDataBlockRef> xmlStack;

public:
	static void run(JabberStream * _stream); 
	// todo: static void run(JabberStreamRef _stream);
};

typedef boost::shared_ptr<JabberStream> JabberStreamRef;
