#include "Message.h"
#include <utf8.hpp>

Message::Message(std::string body, std::string fromName, int type) {
    this->body=body;
    this->fromName=fromName;
    this->type=(Message::MsgType)type;
    unread=(type==Message::INCOMING);

    wstr=utf8::utf8_wchar(body);
    init();
}


