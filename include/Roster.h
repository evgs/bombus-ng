#pragma once

#include <map>
#include <string>
#include <windows.h>

#include "Contact.h"
#include "ListViewODR.h"


//////////////////////////////////////////////////////////////////////////
class RosterGroup: public IconTextElement {
public:
    RosterGroup(const std::string &name);
    virtual int getColor() const;

    typedef boost::shared_ptr<RosterGroup> ref;
    bool equals(const std::string &name) const { return groupName==name; }

    bool isExpanded() const { return expanded; }
    
    static bool compare( RosterGroup::ref left, RosterGroup::ref right);

protected:
    virtual const wchar_t * getText() const;
    virtual int getIconIndex() const;
private:
    std::wstring wstr;
    std::string groupName;
    bool expanded;
};
//////////////////////////////////////////////////////////////////////////



class Roster : public JabberDataBlockListener{
private:
    typedef std::map<std::string, Contact::ref> ContactMap;
    typedef std::list<Contact::ref> ContactList;

    typedef std::vector<RosterGroup::ref> GroupList;

    ContactMap bareJidMap;
    ContactList contacts;
    GroupList groups;

public:
    Roster();

    void addContact(Contact::ref contact);
    Contact::ref findContact (const std::string &jid, bool bareJid=true) const;

    RosterGroup::ref findGroup(const std::string &name);
    RosterGroup::ref createGroup(const std::string &name);
    void makeViewList();

    virtual const char * getType() const{ return NULL; /* result/set */ }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    void bindWindow(ListViewODR::ref roster){ this->roster=roster; }

private:
    ListViewODR::ref roster;

};