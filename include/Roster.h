#pragma once

#include <map>
#include <string>
#include <windows.h>

#include "Contact.h"
#include "ListViewODR.h"

typedef std::map<std::string, ContactRef> ContactRefMap;

class Roster : public JabberDataBlockListener{
private:
    ContactRefMap bareJidMap;
    ContactRefMap jidMap;

public:
    Roster();

    void addContact(ContactRef contact);
    ContactRef findContact (const std::string jid, bool bareJid=true) const;


    virtual const char * getType() const{ return NULL; /* result/set */ }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    void bindWindow(ListViewODR::ref roster){ this->roster=roster; }

private:
    ListViewODR::ref roster;

};