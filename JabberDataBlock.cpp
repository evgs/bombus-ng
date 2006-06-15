#include "JabberDataBlock.h"

JabberDataBlock::JabberDataBlock(void)
{
}

JabberDataBlock::~JabberDataBlock(void)
{
}

string JabberDataBlock::getAttribute(string byName) {
	return attr[byName];
}

void JabberDataBlock::setAttribute(string name, string value) {
	attr[name]=value;
}

JabberDataBlockIterator JabberDataBlock::getChilds(){
	return childs.begin();
}

void JabberDataBlock::addChild(JabberDataBlock child){
	childs.push_back(child);
}

string JabberDataBlock::toXML(){
	string result("<"+getTagName() );
	if (childs.empty() && attr.empty() && text.empty() ) return result+"/>";

	result+=' ';
	for (map<string, string>::iterator a = attr.begin(); a!=attr.end(); a++) {
		result+= a->first + "='" + a->second + "' ";
	}

	if (childs.empty() && text.empty()) return result+"/>";
	result+='>';

	for (JabberDataBlockIterator c=getChilds(); c!=childs.end(); c++) {
		result+= c->toXML();
	}

	result+= text + "</" + getTagName() + '>';
	
	return result;
}
