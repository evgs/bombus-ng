#pragma once

#include <string>
#include <boost/smart_ptr.hpp>
#include <list>

#include "Message.h"

#include "IconTextElement.h"

#include "Presence.h"
#include "jid.h"

/*enum Status {
    PRESENCE_ONLINE=0,
    PRESENCE_CHAT=1,
    PRESENCE_AWAY=2,
    PRESENCE_XA=3,
    PRESENCE_DND=4,
    PRESENCE_OFFLINE=5,
    PRESENCE_ASK=6,
    PRESENCE_UNKNOWN=7,
    PRESENCE_INVISIBLE=8,
    PRESENCE_ERROR=9,
    PRESENCE_TRASH=10,
    PRESENCE_AUTH=-1
};*/

class Contact : public IconTextElement {
public:
    typedef boost::shared_ptr<Contact> ref;
    Contact(const std::string &jid, const std::string &resource, const std::string &nickname);

    Jid jid;
    std::string nickname;
    std::string rosterJid;
    std::string group;

    std::string subscr;

    presence::PresenceIndex status;

    MessageListRef messageList;

    bool hasUnreadMsgs();
    static bool compare(Contact::ref left, Contact::ref right);

    //////////////////////////////////////////////////////////////////////////

    virtual int getColor() const;
    virtual int getIconIndex() const;
    virtual const wchar_t * getText() const;

    void update();
private:
    std::wstring wjid;
};

