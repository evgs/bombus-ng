#pragma once

#include <string>
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"
#include "EntityCaps.h"

class HostCaps : public ClientCaps {
public:
    HostCaps(){};
};

class HostFeatures : public JabberDataBlockListener {
public:
    HostFeatures(){};
    ~HostFeatures(){};

    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return "hf"; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    static void discoverFeatures(ResourceContextRef rc);

private:
};
