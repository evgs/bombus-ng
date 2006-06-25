#pragma once
#include <string>
#include <boost/smart_ptr.hpp>

#include "JabberDataBlock.h"

enum ProcessResult{ BLOCK_PROCESSED, BLOCK_REJECTED, LAST_BLOCK_PROCESSED };
class JabberDataBlockListener{
public:
	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return ""; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block)=0;
};

typedef boost::shared_ptr<JabberDataBlockListener> JabberDataBlockListenerRef;
