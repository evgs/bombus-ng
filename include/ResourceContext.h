#pragma once

#include <boost/smart_ptr.hpp>

class JabberStream;
class JabberStanzaDispatcher;
class Socket;
class JabberAccount;

typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
typedef boost::shared_ptr <JabberStream> JabberStreamRef;
typedef boost::shared_ptr <Socket> SocketRef;
typedef boost::shared_ptr <JabberAccount> JabberAccountRef;

class ResourceContext {
public:
	JabberAccountRef account;
	SocketRef connection;
	JabberStreamRef jabberStream;
	JabberStanzaDispatcherRef jabberStanzaDispatcher;
};

typedef boost::shared_ptr <ResourceContext> ResourceContextRef;
