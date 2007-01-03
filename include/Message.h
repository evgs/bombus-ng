#pragma once

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>

class Message {
public:
    enum MsgType {
        SENT=0,
        INCOMING_HISTORY=1,
        INCOMING=2,
        PRESENCE=10,
        PRESENCE_ASK_SUBSCR=11,
        PRESENCE_SUBSCRIBED=12
    };
public:
    Message(std::string body, std::string fromName, int type);

    std::string body;
    std::string fromName;
    
    bool unread;

    MsgType type;
};

typedef boost::shared_ptr<Message> MessageRef;
typedef std::list<MessageRef> MessageList;
typedef boost::shared_ptr<MessageList> MessageListRef;