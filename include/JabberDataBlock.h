#include <string>
#include <map>
#include <list>
#include <boost/smart_ptr.hpp>

class JabberDataBlock;
typedef smart_ptr<JabberDataBlock> JabberDataBlockRef;
typedef list<JabberDataBlockRef>::iterator JabberDataBlockIterator;

class JabberDataBlock {
public:
	JabberDataBlock(void);
	~JabberDataBlock(void);

private:
	string tagName;
	map<string, string> attr;
	list<JabberDataBlockRef> childs;

public:
	string getTagName();
	string getAttribute(string byName);
	void setAttribute(string name, string value);
	list<JabberDataBlockRef> * getChilds();
	void addChild(JabberDataBlockRef child);
	string toXML();
}
