#pragma once

#include <boost/smart_ptr.hpp>

class JabberStream;
class JabberStanzaDispatcher;
typedef boost::shared_ptr <JabberStanzaDispatcher> JabberStanzaDispatcherRef;
typedef boost::shared_ptr <JabberStream> JabberStreamRef;

class ResourceContext {
public:
	JabberStreamRef jabberStream;
	JabberStanzaDispatcherRef jabberStanzaDispatcher;
};
