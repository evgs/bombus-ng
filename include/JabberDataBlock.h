#pragma once
#include <string>
#include <map>
#include <list>
#include <boost/smart_ptr.hpp>
#include <libxml/xmlwriter.h>

class JabberDataBlock;
typedef boost::shared_ptr<JabberDataBlock> JabberDataBlockRef;
typedef std::list<JabberDataBlockRef>::iterator JabberDataBlockIterator;
typedef boost::shared_ptr<std::string> stringRef;

class JabberDataBlock {
public:
	JabberDataBlock(void);
	JabberDataBlock(const char * _tagName);
	JabberDataBlock(const char * _tagName, const char *_text);
	~JabberDataBlock(void);

private:
	std::map<std::string, std::string> attr;
	std::list<JabberDataBlockRef> childs;
	std::string tagName;
	std::string text;


public:
	const std::string& getTagName() { return tagName; }

	std::string getAttribute(std::string byName);
	void setAttribute(std::string &name, std::string &value);
	void setAttribute(const char *name, const char * value);

	std::string getText() { return text; }
	void setText(std::string &_text) { text=_text; }
	void setText(const char *_text) { text=_text; }

	std::list<JabberDataBlockRef> * getChilds();
	void addChild(JabberDataBlockRef child);
	JabberDataBlock * addChild(const char *_tagName, const char *_text);

	void JabberDataBlock::constructXML(xmlTextWriter * writer);
	stringRef toXML();
};
