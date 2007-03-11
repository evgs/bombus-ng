#pragma once

#include <boost/smart_ptr.hpp>
#include "Log.h"
#include "Presence.h"

class JabberStream;
class JabberStanzaDispatcher;
class Connection;
class JabberAccount;
class Roster;

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
typedef boost::shared_ptr <JabberStream> JabberStreamRef;
typedef boost::shared_ptr <Connection> ConnectionRef;
typedef boost::shared_ptr <JabberAccount> JabberAccountRef;
typedef boost::shared_ptr <Roster> RosterRef;

class ResourceContext {
public:
    ResourceContext();

	JabberAccountRef account;
	JabberStreamRef jabberStream;
	JabberStanzaDispatcherRef jabberStanzaDispatcherRT;
    JabberStanzaDispatcherRef jabberStanzaDispatcher2;
    RosterRef roster;

    presence::PresenceIndex status;
    std::string presenceMessage;

    int priority;

    bool isLoggedIn() { return jabberStream; }

    void sendPresence() { sendPresence(status, presenceMessage, priority); }
    void sendPresence(presence::PresenceIndex status, std::string &message, int priority);
};

typedef boost::shared_ptr <ResourceContext> ResourceContextRef;
