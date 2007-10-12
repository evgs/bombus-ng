//#include "stdafx.h"

#include "JabberDataBlock.h"
#include <boost/assert.hpp>

#include "XMLParser.h"

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
	if (_text) setText(_text);
}


JabberDataBlock::~JabberDataBlock(void)
{}

std::string JabberDataBlock::getAttribute( const std::string & byName ) 
{
	StringMap::const_iterator i=attr.find(byName);
    if (i==attr.end()) return std::string();
	return XMLStringExpand(i->second);
	//return XMLStringExpand(attr[byName]);
}

bool JabberDataBlock::hasAttribute(const std::string & byName) {
	StringMap::const_iterator i=attr.find(byName);
	return (i!=attr.end());
}

void JabberDataBlock::setAttribute(const std::string & name,const std::string & value) {
	attr[name]=XMLStringPrep(value);
}
void JabberDataBlock::setAttribute(const char *name, const char *value) {
	setAttribute(string(name), string(value));
}

void JabberDataBlock::addChild(JabberDataBlockRef child){
	childs.push_back(child);
}

JabberDataBlockRef JabberDataBlock::addChild(const char *_tagName, const char *_text){
	JabberDataBlockRef child=JabberDataBlockRef(new JabberDataBlock(_tagName, _text));
	addChild(child);
	return child;
}

JabberDataBlockRef JabberDataBlock::addChildNS(const char *_tagName, const char *xmlns ) {
    JabberDataBlockRef child=addChild(_tagName, NULL);
    child->setAttribute("xmlns", xmlns);
    return child;
}

JabberDataBlockRef JabberDataBlock::getChildByName(const char * tagName) const{
    for (JabberDataBlockRefList::const_iterator c=childs.begin(); c!=childs.end(); c++) {
		if ( (*c)->tagName==tagName) return *c;
	}
	return JabberDataBlockRef();
}

void JabberDataBlock::removeChild( const char * tagName ) {
    for (JabberDataBlockRefList::iterator c=childs.begin(); c!=childs.end(); c++) {
        if ( (*c)->tagName==tagName) {
            childs.erase(c);
            return;
        }
    }
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

        #ifdef DEBUG
        result->append("\n");
        #endif

        return result;
	}

	result->append(">");

    #ifdef DEBUG
    if (!childs.empty()) result->append("\n");
    #endif

    for (JabberDataBlockRefList::const_iterator c=childs.begin(); c!=childs.end(); c++) {
		result->append( *((*c)->toXML()) );
	}

	result->append(text);
	result->append("</");
	result->append(getTagName());
	result->append(">");

    #ifdef DEBUG
    result->append("\n");
    #endif

	return result;

}

std::string JabberDataBlock::getText() const
{
    return XMLStringExpand(text);
}

void JabberDataBlock::setText( const std::string &_text ) {
    text=XMLStringPrep(_text);
}
void JabberDataBlock::setText( const char *_text ) {
    setText(string(_text));
}

JabberDataBlockRef JabberDataBlock::findChildNamespace( const char *tagName, const char *xmlns ) {
    for (JabberDataBlockRefList::const_iterator c=childs.begin(); c!=childs.end(); c++) {
        JabberDataBlockRef child=*c;
        if (tagName)   if ( child->tagName!=tagName) continue;
        std::string a;
        if (child->getAttribute("xmlns").find(xmlns)==0) return child;
    }
    return JabberDataBlockRef();
}

