#pragma once
#include <string>
#include <map>
#include <list>
#include <boost/smart_ptr.hpp>

#include "basetypes.h"

class JabberDataBlock;
typedef boost::shared_ptr<JabberDataBlock> JabberDataBlockRef;
typedef std::list<JabberDataBlockRef> JabberDataBlockRefList;
//typedef std::list<JabberDataBlockRef>::iterator JabberDataBlockIterator;

class JabberDataBlock {
public:
	JabberDataBlock(void);
	JabberDataBlock(const char * _tagName);
	JabberDataBlock(const std::string & _tagName, const StringMap &_attr);
	JabberDataBlock(const char * _tagName, const char *_text);
	~JabberDataBlock(void);

private:
	std::map<std::string, std::string> attr;
	JabberDataBlockRefList childs;
	std::string tagName;
	std::string text;


public:
	const std::string& getTagName() const{ return tagName; }

	bool hasAttribute(const std::string & byName);
    std::string getAttribute(const std::string & byName);

	void setAttribute(const std::string &name, const std::string &value);
	void setAttribute(const char *name, const char * value);

    std::string getText() const; 
    void _setText(const std::string &_text) { text=_text; }
    void setText(const std::string &_text);
	void setText(const char *_text);

    JabberDataBlockRefList * getChilds() {return &childs;};
	void addChild(JabberDataBlockRef child);
	JabberDataBlockRef addChild(const char *_tagName, const char *_text);

    JabberDataBlockRef getChildByName(const char * tagName) const;
    void removeChild(const char * tagName);
    JabberDataBlockRef findChildNamespace(const char *tagName, const char *xmlns);

    const std::string getChildText(const char * tagName) const;

	bool hasChildByValue(const char * text);

	StringRef toXML();
};
