#pragma once
#include <string>
#include <map>
#include <list>
#include <boost/smart_ptr.hpp>
#include <libxml/xmlwriter.h>

#include "basetypes.h"

class JabberDataBlock;
typedef boost::shared_ptr<JabberDataBlock> JabberDataBlockRef;
typedef std::list<JabberDataBlockRef>::iterator JabberDataBlockIterator;


class JabberDataBlock {
public:
	JabberDataBlock(void);
	JabberDataBlock(const char * _tagName);
	JabberDataBlock(const std::string & _tagName, const std::map<std::string, std::string> &_attr);
	JabberDataBlock(const char * _tagName, const char *_text);
	~JabberDataBlock(void);

private:
	std::map<std::string, std::string> attr;
	std::list<JabberDataBlockRef> childs;
	std::string tagName;
	std::string text;


public:
	const std::string& getTagName() const{ return tagName; }

	std::string getAttribute(std::string byName);
	void setAttribute(const std::string &name, const std::string &value);
	void setAttribute(const char *name, const char * value);

	const std::string& getText() const{ return text; }
	void setText(const std::string &_text) { text=_text; }
	void setText(const char *_text) { text=_text; }

	std::list<JabberDataBlockRef> * getChilds();
	void addChild(JabberDataBlockRef child);
	JabberDataBlock * addChild(const char *_tagName, const char *_text);

	void JabberDataBlock::constructXML(xmlTextWriter * writer);
	StringRef toXML();
};
