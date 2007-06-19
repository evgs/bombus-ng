#pragma once

#include <string>
#include <boost/smart_ptr.hpp>
#include <list>

#include "Message.h"

#include "IconTextElement.h"

#include "Presence.h"
#include "jid.h"

#include "VirtualListView.h"

class Contact : public IconTextElement {
public:
    typedef boost::shared_ptr<Contact> ref;
    Contact(const std::string &jid, const std::string &resource, const std::string &nickname);
    Contact(){}

    Contact::ref clone();

    Jid jid;
    std::string nickname;
    std::string rosterJid;
    std::string group;

    std::string subscr;

    int sortKey;
    presence::PresenceIndex status;
    int offlineIcon;

    ODRListRef messageList;

    bool composing;
    bool acceptComposing;

    //bool hasUnreadMsgs();

    int nUnread;

    static bool compare(Contact::ref left, Contact::ref right);

    const std::string getFullName() const;
    const std::string getName() const;

    virtual void processPresence(JabberDataBlockRef block);
    //////////////////////////////////////////////////////////////////////////

    virtual int getColor() const;
    virtual int getIconIndex() const;
    virtual const wchar_t * getText() const;

    virtual void update();

    std::wstring wjid;
    int transpIndex;
private:
};

