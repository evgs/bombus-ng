#pragma once

#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

class NonSASLAuth : public JabberDataBlockListener{
public:
	NonSASLAuth(ResourceContextRef rc, JabberDataBlockRef streamHeader); 
	~NonSASLAuth(){};
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

private:
    JabberDataBlockRef loginStanza(bool get, bool sha1);
	ResourceContextRef rc;
    std::string streamId;
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
