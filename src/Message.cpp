#include "Message.h"
#include <utf8.hpp>
#include "boostheaders.h"
#include "Image.h"

#pragma comment(lib,"regex.lib")

boost::regex e1("((?:(?:ht|f)tps?://|www\\.)[^<\\s\\n]+)(?<![]\\.,:;!\\})<-])");

extern ImgListRef skin;

Message::Message( std::string body, std::string fromName, bool appendFrom, int type, const PackedTime &time ) 
{
    this->body=body;
    this->fromName=fromName;
    this->type=(Message::MsgType)type;
    this->time=time;
    unread=(type==Message::INCOMING);
    delivered=false;

    if (type==Message::SENT) id=strtime::getRandom();


    //TODO: xml escaping
    
    std::string tmp=boost::regex_replace(body, e1, std::string("\x01\\1\x02"));
    if (tmp.find("/me")==0) {
        if (tmp.length()==3 || tmp[3]==' ') {
            tmp.replace(0, 3, fromName);
            tmp.insert(0, "*");
        }
    } else if (appendFrom) {
        tmp.insert(0, "> ");
        tmp.insert(0, fromName);
    }
    tmp.insert(0,"[] ");
    tmp.insert(1, strtime::toLocalTime(time));
    wstr=utf8::utf8_wchar(tmp);
    init();
}

JabberDataBlockRef Message::constructStanza(const std::string &to) const {
    JabberDataBlockRef out=JabberDataBlockRef(new JabberDataBlock("message"));
    out->setAttribute("type", "chat");
    out->setAttribute("to", to);
    out->setAttribute("id", id);
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

PackedTime Message::extractXDelay( JabberDataBlockRef stanza ) {
    JabberDataBlockRef xdelay=stanza->findChildNamespace("x","jabber:x:delay"); // DEPRECATED
	if (!xdelay) xdelay=stanza->findChildNamespace("delay","urn:xmpp:delay");
    if (!xdelay) return strtime::getCurrentUtc();
    return strtime::PackIso8601(xdelay->getAttribute("stamp"));
}

std::string Message::getMessageText() {
    std::string r=utf8::wchar_utf8(getText());
    r.erase(0,10);
    return r;
}

void Message::draw( HDC hdc, RECT &rt ) const{
    MessageElement::draw(hdc, rt);
    if (delivered) skin->drawElement(hdc, icons::ICON_DELIVERED_INDEX, rt.right-skin->getElementWidth()-2, rt.top );
}