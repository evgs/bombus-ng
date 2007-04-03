#pragma once

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>

#include "OwnerDrawRect.h"
#include "JabberDataBlock.h"

class MessageElement : public ODR {
public:
    MessageElement(){};
    MessageElement(const std::string &str);
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getColor() const;

    virtual void measure(HDC hdc, RECT &rt);
    virtual void draw(HDC hdc, RECT &rt) const;

protected:
    std::wstring wstr;
    int width;
    int height;
    void init();
    virtual const wchar_t * getText() const;
};


class Message : public MessageElement{
public:
    virtual int getColor() const;
    enum MsgType : int {
        SENT=0,
        INCOMING_HISTORY=1,
        INCOMING=2,
        PRESENCE=10,
        PRESENCE_ASK_SUBSCR=11,
        PRESENCE_SUBSCRIBED=12,
        PRESENCE_UNSUBSCRIBED=13
    };

    typedef boost::shared_ptr<Message> ref;

public:
    Message(std::string body, std::string fromName, int type);

    JabberDataBlockRef constructStanza(const std::string &to) const;

    std::string body;
    std::string fromName;
    
    bool unread;

    MsgType type;
};


typedef std::list<Message::ref> MessageList;
typedef boost::shared_ptr<MessageList> MessageListRef;