#pragma once
#include <string>
#include "boostheaders.h"

#include "JabberDataBlock.h"
#include "ResourceContext.h"

enum ProcessResult{ BLOCK_PROCESSED, BLOCK_REJECTED, LAST_BLOCK_PROCESSED, CANCEL };
class JabberDataBlockListener{
public:
	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return ""; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc)=0;
};

typedef boost::shared_ptr<JabberDataBlockListener> JabberDataBlockListenerRef;
