#pragma once

#include <string>
#include <list>
#include "boostheaders.h"

#include "TimeFunc.h"
#include "OwnerDrawRect.h"
#include "JabberDataBlock.h"
#include "VirtualListView.h"

class MessageElement : public ODR, public VirtualListElement{
public:
    MessageElement(){};
    MessageElement(const std::string &str);
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getColor() const;

    virtual void measure(HDC hdc, RECT &rt);
    virtual void draw(HDC hdc, RECT &rt) const;

    virtual void render(HDC hdc, RECT &rt, bool measure) const;

    virtual HMENU getContextMenu(HMENU menu);
    virtual bool OnMenuCommand(int cmdId, HWND parent);

    virtual const wchar_t * getText() const;

protected:
    std::wstring wstr;
    int width;
    int height;
    bool singleLine;
    bool smiles;
    void init();
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
    Message(std::string body, std::string fromName, bool appendFrom, int type, const PackedTime &time);

    JabberDataBlockRef constructStanza(const std::string &to) const;

    std::string body;
    std::string fromName;

    std::string id;

    PackedTime time;
    
    bool unread;
    bool delivered;

    MsgType type;

    std::string getMessageText();

    virtual void draw(HDC hdc, RECT &rt) const;

    static PackedTime extractXDelay(JabberDataBlockRef stanza);
};


typedef std::list<Message::ref> MessageList;
typedef boost::shared_ptr<MessageList> MessageListRef;