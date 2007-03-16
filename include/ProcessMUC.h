#pragma once

#include "ResourceContext.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"

#include "Roster.h"
#include "Contact.h"

class MucContact : public Contact {
public:
    MucContact(const std::string &jid);
    std::string realJid;
    std::string role;
    std::string affiliation;

    virtual void update();

    typedef boost::shared_ptr<MucContact> ref;
};

class MucRoom : public Contact {
public:
    MucRoom(const std::string &jid);
    typedef boost::shared_ptr<MucRoom> ref;
};

class MucGroup : public RosterGroup {
public:
    MucGroup( const std::string &jid, const std::string &name );
    MucContact::ref selfContact;
    MucRoom::ref room;
    std::string password;

    void join(ResourceContextRef rc);

    virtual void addContacts(ODRList *list);

    typedef boost::shared_ptr<MucGroup> ref;
};

class ProcessMuc : public JabberDataBlockListener {
public:
    ProcessMuc(ResourceContextRef rc) { this->rc=rc; }
    ~ProcessMuc(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "presence"; } //presence
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    static void initMuc(const std::string &jid, const std::string &password, ResourceContextRef rc);

private:
    ResourceContextRef rc;
};
