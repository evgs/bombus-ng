#include "stdafx.h"

#include "JabberDataBlock.h"
#include <libxml/xmlwriter.h>
#include <boost/assert.hpp>

using namespace std;

JabberDataBlock::JabberDataBlock(void){}
JabberDataBlock::JabberDataBlock(const char * _tagName){
	tagName=_tagName;
}
JabberDataBlock::JabberDataBlock(const char * _tagName, const char * _text){
	tagName=_tagName;
	if (_text) text=_text;
}


JabberDataBlock::~JabberDataBlock(void)
{}

string JabberDataBlock::getAttribute(string byName) {
	return attr[byName];
}

void JabberDataBlock::setAttribute(string name, string value) {
	attr[name]=value;
}
void JabberDataBlock::setAttribute(const char *name, const char *value) {
	attr[string(name)]=string(value);
}

void JabberDataBlock::addChild(JabberDataBlockRef child){
	childs.push_back(child);
}

JabberDataBlock * JabberDataBlock::addChild(const char *_tagName, const char *_text){
	JabberDataBlockRef child=JabberDataBlockRef(new JabberDataBlock(_tagName, _text));
	addChild(child);
	return child.get();
}

stringRef JabberDataBlock::toXML(){
	stringRef result=stringRef(new string("<"));
	result->append(getTagName());
	if (childs.empty() && attr.empty() && text.empty() ) {
		result->append("/>");
		return result;
	}

	for (map<string, string>::iterator a = attr.begin(); a!=attr.end(); a++) {
	result->append(" ");
		result->append(a->first);
		result->append("='");
		result->append(a->second);
		result->append("'");
	}

	if (childs.empty() && text.empty()) {
		result->append("/>");
		return result;
	}

	result->append(">");

	for (JabberDataBlockIterator c=childs.begin(); c!=childs.end(); c++) {
		result->append( *((*c)->toXML()) );
	}

	result->append(text);
	result->append("</");
	result->append(getTagName());
	result->append(">");
	
	return result;

}

void JabberDataBlock::constructXML(xmlTextWriterPtr writer) {

	int rc=xmlTextWriterStartElement(writer, BAD_CAST tagName.c_str());
	BOOST_ASSERT(rc>=0);

	for (map<string, string>::iterator a = attr.begin(); a!=attr.end(); a++) {
		rc = xmlTextWriterWriteAttribute(writer, 
			BAD_CAST a->first.c_str(),
			BAD_CAST a->second.c_str());
		BOOST_ASSERT(rc>=0);
	}

	for (JabberDataBlockIterator c=childs.begin(); c!=childs.end(); c++) {
		(*c)->constructXML(writer);
	}

	if (!text.empty()) xmlTextWriterWriteString(writer, BAD_CAST text.c_str());

	rc = xmlTextWriterEndElement(writer);
	BOOST_ASSERT(rc>=0);
}