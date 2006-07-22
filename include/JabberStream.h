#pragma once

#include <boost/smart_ptr.hpp>
#include <map>
#include <stack>
#include "libxml/xmlwriter.h"
#include "libxml/xmlreader.h"

#include "Socket.h"
#include "JabberDataBlock.h"
#include "JabberListener.h"
#include "JabberStanzaDispatcher.h"
#include "XMLEventListener.h"
#include "XMLParser.h"

class JabberStream : public XMLEventListener{

public:
	JabberStream(void);
	JabberStream(SocketRef _connection);
	~JabberStream(void);

	void sendStanza(JabberDataBlockRef stanza);
	void sendStanza(JabberDataBlock &stanza);
	void sendXmlVersion();
	void sendXmppHeader(const char * serverName);

	void setJabberListener(JabberListenerRef listener) { jabberListener=listener; }
	void setJabberStanzaDispatcher(JabberStanzaDispatcherRef dispatcher) {stanzaDispatcher=dispatcher; }

	virtual void tagStart(const std::string & tagname, const std::map<std::string, std::string> &attr);
	virtual void tagEnd(const std::string & tagname);
	virtual void plainTextEncountered(const std::string & body);

private:
	SocketRef connection;
	XMLParserRef parser;

	std::string streamId;

	bool isRunning;

	JabberListenerRef jabberListener;
	JabberStanzaDispatcherRef stanzaDispatcher;

	std::stack<JabberDataBlockRef> stanzaStack;

private:
	static void run(JabberStream * _stream);
};

typedef boost::shared_ptr<JabberStream> JabberStreamRef;
