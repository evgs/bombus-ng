#pragma once

#include <map>
#include <string>
#include <windows.h>

#include "Contact.h"
#include "VirtualListView.h"

#include "JabberDataBlockListener.h"

#include "basetypes.h"

//////////////////////////////////////////////////////////////////////////
class RosterGroup: public IconTextElement {
public:
    enum Type {
        SELF_CONTACT=0,
        ROSTER,
        NOT_IN_LIST,
        TRANSPORTS,
        MUC
    };

    RosterGroup(){};
    RosterGroup(const std::string &name, Type type);
    virtual int getColor() const;

    typedef boost::shared_ptr<RosterGroup> ref;
    bool equals(const std::string &name) const { return groupName==name; }

    void setExpanded(bool state) { expanded=state; }
    bool isExpanded() const { return expanded; }
    
    static bool compare( RosterGroup::ref left, RosterGroup::ref right);

    std::string & getName(){ return groupName; }

    Type type;

    virtual void addContacts(ODRList *list){};

protected:
    virtual const wchar_t * getText() const;
    virtual int getIconIndex() const;
    std::wstring wstr;
    std::wstring sortKey;
    std::string groupName;
    bool expanded;
private:
};

//////////////////////////////////////////////////////////////////////////



class Roster : public JabberDataBlockListener{
public:
    typedef std::vector<Contact::ref> ContactList;
    typedef boost::shared_ptr<ContactList> ContactListRef;

private:
    typedef std::map<std::string, Contact::ref> ContactMap;

    typedef std::vector<RosterGroup::ref> GroupList;

    ContactMap bareJidMap;
    ContactList contacts;
    GroupList groups;

public:

    Roster(ResourceContextRef rc);

    //void addContact(Contact::ref contact);
    Contact::ref findContact (const std::string &jid) const;
    Contact::ref getContactEntry(const std::string & jid);

    void addGroup( RosterGroup::ref group );
    RosterGroup::ref findGroup(const std::string &name);
    RosterGroup::ref createGroup(const std::string &name, RosterGroup::Type type);

    bool needUpdateView;
    void makeViewList();

    virtual const char * getType() const{ return NULL; /* result/set */ }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);

    void bindWindow(VirtualListView::ref roster){ this->roster=roster; }

    StringVectorRef getRosterGroups();

    void addContact(Contact::ref contact);
    void deleteContact(Contact::ref contact);
    void setStatusByFilter(const std::string & bareJid, int status);
    Roster::ContactListRef getHotContacts();

    void rosterSet(const char * nick, const char *jid, const char *group, const char *subscr );
    typedef boost::shared_ptr<Roster> ref;

    ResourceContextRef rc;
private:
    VirtualListView::ref roster;

};


class RosterView: public VirtualListView {
public:
    RosterView(HWND parent, const std::string & title);
    virtual ~RosterView();

    virtual void eventOk();
    boost::weak_ptr<Roster> roster;

    typedef boost::shared_ptr<RosterView> ref;

    virtual HMENU getContextMenu();
    virtual void OnCommand(int cmdId, LONG lParam);

    virtual void showWindow(bool show);

    void setIcon(int iconIndex);

    enum actions {
        OPENCHAT=50000,
        LOGON, LOGOFF, RESOLVENICKNAMES,
        VCARD, CLIENTINFO, COMMANDS,
        EDITCONTACT, SUBSCR, ADDCONTACT, DELETECONTACT, 
        SENDSTATUS, SENDFILE, INVITE, 
        RENAMEGRP, SUBSCRIBE, SUBSCRIBED, UNSUBSCRIBED,
        MUCKICK, MUCBAN, 
        MUCVISITOR, MUCPARTICIPANT, MUCMODERATOR,
        MUCNONE, MUCMEMBER, MUCADMIN, MUCOWNER
    };
private:
};
