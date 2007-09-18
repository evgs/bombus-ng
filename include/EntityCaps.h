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

private:
    static const std::string & getCapsHash();
    static std::string capsHash;
};

class ClientCaps {
public:
    ClientCaps();
    virtual ~ClientCaps(){};

    JabberDataBlockRef presenceEntityCaps();

    const std::string & getCapsHash();
    
    bool probeFeature(const std:: string) const;

    void addFeature(const std::string feature);
    void removeFeature(const std::string feature);

    void appendFeatures(JabberDataBlockRef result) const;

protected:
    std::string node;
    std::string capsHash;
    std::string alg;
    std::vector<std::string> features;
};

class MyCaps : public ClientCaps {
public:
    MyCaps();
};