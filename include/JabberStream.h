#pragma once
#include <Socket.h>
#include <JabberDataBlock.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>

class JabberStream {
friend int writeCallback(void * context, const char * buffer, int len);
friend int closeCallback(void * context);
friend int readCallback(void * context, char * buffer, int len);
friend int icloseCallback(void * context);

public:
	JabberStream(void);
	JabberStream(SocketRef _connection);
	~JabberStream(void);

	void sendStanza(JabberDataBlockRef stanza);
	void sendStanza(JabberDataBlock &stanza);
	void sendXmlVersion();
	void sendXmppHeader(const char * serverName);


private:
	SocketRef connection;
    xmlTextWriterPtr writer;
	xmlTextReaderPtr reader;

	std::string streamId;

	bool isRunning;

private:
	static void run(JabberStream * _stream);
};