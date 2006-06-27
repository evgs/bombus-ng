#pragma once

#include <boost/smart_ptr.hpp>
#include "libxml/xmlwriter.h"
#include "libxml/xmlreader.h"

#include "Socket.h"
#include "JabberDataBlock.h"
#include "JabberListener.h"
#include "JabberStanzaDispatcher.h"

class JabberStream {

static int writeCallback(void * context, const char * buffer, int len);
static int closeCallback(void * context);
static int readCallback(void * context, char * buffer, int len);
static int icloseCallback(void * context);

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

private:
	SocketRef connection;
    xmlTextWriterPtr writer;
	xmlTextReaderPtr reader;

	std::string streamId;

	bool isRunning;

	JabberListenerRef jabberListener;
	JabberStanzaDispatcherRef stanzaDispatcher;

private:
	static void run(JabberStream * _stream);
};

typedef boost::shared_ptr<JabberStream> JabberStreamRef;
