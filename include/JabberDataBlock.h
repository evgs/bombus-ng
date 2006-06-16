#include <string>
#include <map>
#include <list>

class JabberDataBlock {
public:
	JabberDataBlock(void);
	~JabberDataBlock(void);

private:
	string tagName;
	map<string, string> attr;
	list<JabberDataBlock> childs;

public:
	string getTagName();
	string getAttribute(string byName);
	void setAttribute(string name, string value);
	list<JabberDataBlock> * getChilds();
	void addChild(JabberDataBlock child);
	string toXML();
}
typedef list<JabberDataBlock>::iterator JabberDataBlockIterator;
