#pragma once

#include "JabberDataBlockListener.h"
#include "TimeFunc.h"

class LastActivity : public JabberDataBlockListener {
public:
    LastActivity() {}
    ~LastActivity(){};
    virtual const char * getType() const{ return "get"; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    static void update();
private:
    static PackedTime lastActivityTime;
    std::string delay();
};
