#pragma once
#include <string>
#include <boost/smart_ptr.hpp>


class JabberListener {
public:
	virtual void beginConversation(const std::string & streamId)=0;
	virtual void endConversation()=0;
};

typedef boost::shared_ptr <JabberListener> JabberListenerRef;
