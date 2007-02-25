#include "Message.h"
#include <utf8.hpp>

Message::Message(std::string body, std::string fromName, int type) {
    this->body=body;
    this->fromName=fromName;
    this->type=(Message::MsgType)type;
    unread=(type==Message::INCOMING);

    //TODO: xml escaping
    wstr=utf8::utf8_wchar(body);
    init();
}

JabberDataBlockRef Message::constructStanza(const std::string &to) const {
    JabberDataBlockRef out=JabberDataBlockRef(new JabberDataBlock("message"));
    out->setAttribute("type", "chat");
    out->setAttribute("to", to);
    out->addChild("body", body.c_str());
    return out;
}


int Message::getColor() const{
    switch (type) {
        case Message::SENT: return 0x0000ff; //RED
        case Message::INCOMING: return 0xff0000; //BLUE
        case Message::PRESENCE: return 0x00ff00; //Green
        default: return 0x000000;
    }

}