#pragma once

#include <boost/smart_ptr.hpp>
#include "Log.h"

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
	Log *log;
	JabberAccountRef account;
	JabberStreamRef jabberStream;
	JabberStanzaDispatcherRef jabberStanzaDispatcherRT;
    JabberStanzaDispatcherRef jabberStanzaDispatcher2;
    RosterRef roster;
};

typedef boost::shared_ptr <ResourceContext> ResourceContextRef;
