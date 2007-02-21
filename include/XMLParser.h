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
	int inbufIdx;


public:
	XMLParser(XMLEventListener * eventListener);
	~XMLParser();

	void bindStream(ConnectionRef s) {inStream=s; };

	void parse();

private:
	char getChar();
	const std::string XMLParser::readTagPortion();
};

typedef boost::shared_ptr<XMLParser> XMLParserRef;

std::string XMLStringExpand(const std::string & data);
std::string XMLStringPrep(const std::string & data);
