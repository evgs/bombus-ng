#pragma once

#include <string>
#include <boost/smart_ptr.hpp>

class Jid;

enum Status {
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
};


class Contact {
public:
    Contact(const std::string &jid, const std::string &resource, const std::string &nickname);

    Jid jid;
    std::string nickname;
    std::string rosterJid;
    std::string subscr;

    Status status;
    
};

typedef boost::shared_ptr<Contact> ContactRef;