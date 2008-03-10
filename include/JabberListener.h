#pragma once
#include <string>
#include "boostheaders.h"
#include "JabberDataBlock.h"


class JabberListener {
public:
    virtual bool connect()=0;
    virtual void beginConversation(JabberDataBlockRef streamHeader)=0;
    virtual void loginSuccess()=0;
    virtual void loginFailed(const char * errMsg)=0;
    virtual void endConversation(const std::exception *ex)=0;
};

typedef boost::shared_ptr <JabberListener> JabberListenerRef;
