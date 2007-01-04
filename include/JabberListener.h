#pragma once
#include <string>
#include <boost/smart_ptr.hpp>
#include "JabberDataBlock.h"


class JabberListener {
public:
    virtual bool connect()=0;
    virtual void beginConversation(JabberDataBlockRef streamHeader)=0;
    virtual void loginSuccess()=0;
    virtual void loginFailed()=0;
    virtual void endConversation(const std::exception *ex)=0;
};

typedef boost::shared_ptr <JabberListener> JabberListenerRef;
