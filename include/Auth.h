#pragma once

#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

class NonSASLAuth : public JabberListener {
public:
	NonSASLAuth(ResourceContextRef rc) {
		this->rc=rc;
	}
	~NonSASLAuth(){};
	virtual void beginConversation(const std::string & streamId);
	virtual void endConversation();


private:
	ResourceContextRef rc;
};


class SASLAuth : public JabberListener, public JabberDataBlockListener {
public:
	SASLAuth(ResourceContextRef rc) {
		this->rc=rc;
	}
	~SASLAuth(){};
	virtual void beginConversation(const std::string & streamId);
	virtual void endConversation();

	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return NULL; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);


private:
	ResourceContextRef rc;
};
