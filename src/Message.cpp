#include "Message.h"
#include <utf8.hpp>
#include <boost/regex.hpp>

boost::regex e1("((?:(?:ht|f)tps?://|www\\.)[^<\\s\\n]+)(?<![]\\.,:;!\\})<-])");

Message::Message(std::string body, std::string fromName, int type) {
    this->body=body;
    this->fromName=fromName;
    this->type=(Message::MsgType)type;
    unread=(type==Message::INCOMING);

    //TODO: xml escaping
    
    std::string tmp=boost::regex_replace(body, e1, std::string("[\\1]"));
    wstr=utf8::utf8_wchar(tmp);
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
        case Message::SENT: return 0x0000b0; //RED
        case Message::INCOMING: return 0xb00000; //BLUE
        case Message::PRESENCE: return 0x006000; //Green
        default: return 0x000000;
    }

}