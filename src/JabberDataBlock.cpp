//#include "stdafx.h"

#include "JabberDataBlock.h"
#include <boost/assert.hpp>

using namespace std;

JabberDataBlock::JabberDataBlock(void){}
JabberDataBlock::JabberDataBlock(const char * _tagName){
	tagName=_tagName;
}

JabberDataBlock::JabberDataBlock(const std::string & _tagName, const StringMap &_attr){
	tagName=_tagName;
	attr=_attr;
}

JabberDataBlock::JabberDataBlock(const char * _tagName, const char * _text){
	tagName=_tagName;
	if (_text) text=_text;
}


JabberDataBlock::~JabberDataBlock(void)
{}

const std::string& JabberDataBlock::getAttribute(const std::string &byName) {
	// TODO:
	//StringMap::const_iterator i=attr.find(byName);
	//return i->second;
	return attr[byName];
}

bool JabberDataBlock::hasAttribute(const std::string & byName) {
	StringMap::const_iterator i=attr.find(byName);
	return (i!=attr.end());
}

void JabberDataBlock::setAttribute(const std::string & name,const std::string & value) {
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


JabberDataBlockRef JabberDataBlock::getChildByName(const char * tagName) const{
    for (JabberDataBlockRefList::const_iterator c=childs.begin(); c!=childs.end(); c++) {
		if ( (*c)->tagName==tagName) return *c;
	}
	return JabberDataBlockRef();
}

const std::string JabberDataBlock::getChildText( const char * tagName ) const{
    JabberDataBlockRef child=getChildByName(tagName);
    if (child) return child->getText();
    return "";
}

bool JabberDataBlock::hasChildByValue(const char * text){
	for (JabberDataBlockRefList::const_iterator c=childs.begin(); c!=childs.end(); c++) {
		if ( (*c)->text==text) return true;
	}
	return false;
}

StringRef JabberDataBlock::toXML(){
	StringRef result=StringRef(new string("<"));
	result->append(getTagName());
	if (childs.empty() && attr.empty() && text.empty() ) {
		result->append("/>");
		return result;
	}

	for (StringMap::iterator a = attr.begin(); a!=attr.end(); a++) {
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

    for (JabberDataBlockRefList::const_iterator c=childs.begin(); c!=childs.end(); c++) {
		result->append( *((*c)->toXML()) );
	}

	result->append(text);
	result->append("</");
	result->append(getTagName());
	result->append(">");
	
	return result;

}

