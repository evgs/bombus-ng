#include "XMLParser.h"
#include "XMLEventListener.h"

XMLParser::XMLParser(XMLEventListener *eventListener){
	this->eventListener=eventListener;
	prebuffered=0;
	inbufIdx=0;
}

XMLParser::~XMLParser(){}

char XMLParser::getChar(){
	if (inbufIdx>=prebuffered) {
		prebuffered=inStream->read(inbuf, XML_PREBUF_SZ);
		inbufIdx=0;
		if (prebuffered<0) throw std::exception("Unexpected end of XML");
	}
	return inbuf[inbufIdx++];
}

const std::string XMLParser::readTagPortion(){
	char inQuote=0;
	char next=0;
	std::string buf;

	while ( inQuote || (next!='>' && next!=' ') ) {
		next=getChar();
		switch (next) {
			case '>': break;
			case '\'':
			case '\"':
				inQuote=(inQuote)? 0 : next;
			default: buf+=next;
		}
	}
	return buf;
}

void XMLParser::parse(){

	while (true) {

		// parsing text value
		{
			char c;

			std::string body;
			while ( (c=getChar()) != '<' )		body+=c;

			if (!body.empty()) eventListener->plainTextEncountered( body );
		}

		// parsing tag
		bool startTag = true;
		bool emptyTag = false;

		std::string tagName;
		StringMap attributes;

		while (true){

			std::string data=readTagPortion();
			int start=0;
			int end=data.length();

			char last=(end>0)? data[end-1] : 0;
			char first=data[0];

			if (first=='/') { 
				startTag=false;
				start++;
			}

			if (first=='?') start++;
			if (last=='?') end--;

			if (last=='/') {
				emptyTag=true;
				end--;
			}

			bool hasMoreData = ( last==' ' );
			if (hasMoreData) end--;

			if (tagName.empty() ) {
				tagName=data.substr(start, end-start);
				continue;
			}

			int equalSign=data.find_first_of('=');
			if (equalSign==-1) {
				if (hasMoreData) continue; else break;
			}

			std::string attrName=data.substr(start, equalSign-start);
			std::string attrValue=data.substr(equalSign+2, end-equalSign-3);

			attributes[attrName]=attrValue;

			if (!hasMoreData) break;
		}
		if (startTag) { eventListener->tagStart(tagName, attributes);	}
		if (emptyTag || !startTag) { eventListener->tagEnd(tagName); }
	}
}