#pragma once

#include <boost/smart_ptr.hpp>
#include "XMLEventListener.h"
#include "Socket.h"
#include <exception>

#define XML_PREBUF_SZ 1024

class XMLParser {
private:
	XMLEventListener * eventListener;
	ConnectionRef inStream;

	char inbuf[XML_PREBUF_SZ];
	int prebuffered;

    std::string sbuf;
    std::string tagname;
    std::string atrname;
    StringMap attr;

    enum ParseState {
        PLAIN_TEXT=0,
        TAGNAME,
        ENDTAGNAME,
        ATRNAME,
        ATRVALQS,
        ATRVALQD
    };
    ParseState state;
public:
	XMLParser(XMLEventListener * eventListener);
	~XMLParser();

	void bindStream(ConnectionRef s) {inStream=s; };

    void parseStream();
    void parse(const char * buf, int size);
};

typedef boost::shared_ptr<XMLParser> XMLParserRef;

std::string XMLStringExpand(const std::string & data);
std::string XMLStringPrep(const std::string & data);
