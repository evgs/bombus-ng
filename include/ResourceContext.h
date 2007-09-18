#pragma once

#include <boost/smart_ptr.hpp>
#include "Log.h"
#include "Presence.h"
#include "jid.h"

#include "map"

class JabberStream;
class JabberStanzaDispatcher;
class Connection;
class JabberAccount;
class Roster;
class MucBookmarks;
class MyCaps;
class HostCaps;

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
typedef boost::shared_ptr <JabberStream> JabberStreamRef;
typedef boost::shared_ptr <Connection> ConnectionRef;
typedef boost::shared_ptr <JabberAccount> JabberAccountRef;
typedef boost::shared_ptr <Roster> RosterRef;
typedef boost::shared_ptr <MucBookmarks> MucBookmarksRef;
typedef boost::shared_ptr <MyCaps> MyCapsRef;
typedef boost::shared_ptr <HostCaps> HostCapsRef;

class ResourceContext {
public:
    ResourceContext();

	JabberAccountRef account;
    Jid myJid;
	JabberStreamRef jabberStream;
	JabberStanzaDispatcherRef jabberStanzaDispatcherRT;
    JabberStanzaDispatcherRef jabberStanzaDispatcher2;
    RosterRef roster;
    MucBookmarksRef bookmarks;

    MyCapsRef myCaps;

    HostCapsRef hostFeatures;

    presence::PresenceIndex status;
    std::string presenceMessage;

    int priority;

    bool isLoggedIn() { return jabberStream; }

    void sendPresence() { sendPresence(NULL, status, presenceMessage, priority); }
    void sendPresence(const char *to, presence::PresenceIndex status, const std::string &message, int priority);
};

typedef boost::shared_ptr <ResourceContext> ResourceContextRef;
