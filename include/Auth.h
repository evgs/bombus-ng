#pragma once

#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

class NonSASLAuth {
public:
	NonSASLAuth(ResourceContextRef rc, JabberDataBlockRef streamHeader); 
	~NonSASLAuth(){};

private:
	ResourceContextRef rc;
};


class SASLAuth : public JabberDataBlockListener {
public:
	SASLAuth(ResourceContextRef rc, JabberDataBlockRef streamHeader);
	~SASLAuth();

	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return NULL; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);


private:
	ResourceContextRef rc;
};
