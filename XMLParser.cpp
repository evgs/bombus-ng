//#include "stdafx.h"

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
		if (prebuffered<=0) throw std::exception("Unexpected end of XML");
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
				if (hasMoreData) continue; else break;
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
		if (emptyTag || !startTag) { 
            bool lastTag=eventListener->tagEnd(tagName); 
            if (lastTag) return; // normal end 
        }
	}
}

std::string XMLStringPrep(const std::string & data){
	std::string result;
	for (std::string::const_iterator i=data.begin(); i!=data.end(); i++) {
		char ch=*i;
		switch (ch) {
			case '&':   result+="&amp;" ; break;
			case '"':   result+="&quot;" ; break;
			case '<':   result+="&lt;" ; break;
			case '>':   result+="&gt;" ; break;
			case '\'':  result+="&apos;" ; break;
                // not nessesary
            case 0x0d:  result+="&#13;" ; break;
            case 0x0a:  result+="&#13;" ; break;
			default: result+=ch;
		}
	}
	return result;
}

std::string XMLStringExpand(const std::string & data){
    std::string result;
    std::string escapedChar;
    bool inEsc=false;
    for (std::string::const_iterator i=data.begin(); i!=data.end(); i++) {
        char ch=*i;
        switch (ch) {
            case '&':
                if (inEsc) throw std::exception("Malformed XML escape sequence");
                inEsc=true; 
                escapedChar.clear();
                break;
            case ';':   
                if (inEsc) {
                    ch=0;
                    inEsc=false;
                    if (escapedChar=="amp")  ch='&';
                    if (escapedChar=="quot") ch='"'; 
                    if (escapedChar=="lt")   ch='<'; 
                    if (escapedChar=="gt")   ch='>';
                    if (escapedChar=="apos") ch='\'';
                    if (escapedChar[0]=='#') { 
                        escapedChar[0]=' ';
                        ch=atoi(escapedChar.c_str());
                    }
                    if (ch==0) throw std::exception("Malformed XML escape sequence");
                }
            default: if (inEsc) escapedChar+=ch; else result+=ch;
        }
    }

    if (inEsc) throw std::exception("Malformed XML escape sequence");
    return result;
}