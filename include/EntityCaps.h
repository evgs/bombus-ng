#pragma once

#include <string>
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

class EntityCaps : public JabberDataBlockListener {
public:
    EntityCaps(){};
    ~EntityCaps(){};

    virtual const char * getType() const{ return "get"; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    static JabberDataBlockRef presenceEntityCaps();

private:
    static const std::string & getCapsHash();
    static std::string capsHash;
};
