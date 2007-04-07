//#include "stdafx.h"

#include "XMLParser.h"
#include "XMLEventListener.h"

XMLParser::XMLParser(XMLEventListener *eventListener){
	this->eventListener=eventListener;
	prebuffered=0;
    state=PLAIN_TEXT;
}

XMLParser::~XMLParser(){}

void XMLParser::parse( const char * buf, int size ) {
    while (size>0) {
        size--;
        char c=*buf++;

        switch (state) {
        case PLAIN_TEXT: 
            {
                //parsing plain text
                if (c=='<') {
                    state=TAGNAME;
                    if (!sbuf.empty()) eventListener->plainTextEncountered( sbuf );
                    sbuf.clear();
                    tagname.clear();
                    attr.clear();
                    continue;
                }
                sbuf+=c; continue;
            }

        case ATRNAME:
            {
                if (c=='?') continue;
                if (c==' ') continue;
                if (c=='=') continue;
                if (c=='\'') { state=ATRVALQS; atrname=sbuf; sbuf.clear(); continue; }
                if (c=='\"') { state=ATRVALQS; atrname=sbuf; sbuf.clear(); continue; }

                if (c!='>' && c!='/') { 
                    sbuf+=c;
                    continue;
                } else state=TAGNAME;
            }

        case TAGNAME:
            {
                if (c=='?') continue;
                if (c=='/') { 
                    state=ENDTAGNAME; 
                    sbuf.clear();
                    if (tagname.length()) {
                        eventListener->tagStart(tagname, attr); 
                        sbuf=tagname;
                    }
                    continue; 
                }
                if (c==' ') { state=ATRNAME; continue; }
                if (c=='>') { 
                    state=PLAIN_TEXT; 
                    eventListener->tagStart(tagname, attr); 
                    continue; 
                }
                tagname+=c;
                continue;
            }

        case ENDTAGNAME:
            {
                if (c==' ') continue;
                if (c=='>') {
                    state=PLAIN_TEXT;
                    eventListener->tagEnd(sbuf);
                    sbuf.clear();
                    continue;
                }
                sbuf+=c;
                continue;
            }
            
        case ATRVALQS: 
            {
                if (c=='\'') { state=ATRNAME; attr[atrname]=sbuf; sbuf.clear(); continue; }
                sbuf+=c;
                continue;
            }
        case ATRVALQD: 
            {
                if (c=='\"') { state=ATRNAME; attr[atrname]=sbuf; sbuf.clear(); continue; }
                sbuf+=c;
                continue;
            }
        }
    }
}

void XMLParser::parseStream() {
    while (true) {
        prebuffered=inStream->read(inbuf, XML_PREBUF_SZ);
        if (prebuffered<=0) throw std::exception("Unexpected end of XML");
        parse(inbuf, prebuffered);
    }
}

std::string XMLStringPrep(const std::string & data){
	std::string result;
	for (std::string::const_iterator i=data.begin(); i!=data.end(); i++) {
		unsigned char ch=(unsigned char)*i;
		switch (ch) {
			case '&':   result+="&amp;" ; break;
			case '"':   result+="&quot;" ; break;
			case '<':   result+="&lt;" ; break;
			case '>':   result+="&gt;" ; break;
			case '\'':  result+="&apos;" ; break;
            case 0x09:  result+=(char)0x09; break;
                // not nessesary
            case 0x0d:  /*result+="&#13;"*/ ; break;
            case 0x0a:  result+="&#10;" ; break;
			default: if (ch>0x1f) result+=ch;
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