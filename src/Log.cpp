#include "Log.h"

#include "Message.h"
#include "utf8.hpp"

class LogMessage : public MessageElement {
public:
    virtual int getColor() const;
    LogMessage(const wchar_t * text);
};

LogMessage::LogMessage(const wchar_t * text) {
    wstr=std::wstring(text);
    init();
}

int LogMessage::getColor() const {
    return 0;
}

Log::Log(){}

Log::~Log(){}

void Log::addLog(const wchar_t * msg) {
    //ListBox_AddString( logWnd->getListBoxHWnd(), msg);
    //ODRRef r=ODRRef(new IconTextElementContainer(std::wstring(msg), -1));
    ODRRef r=ODRRef(new LogMessage(msg));
    odrLog->addODR(r, true);
}

void Log::msg(const std::string &message){
    addLog(utf8::utf8_wchar(message).c_str());
}

void Log::msg(const char * message){
    msg(std::string(message));
}

void Log::msg(const char * message, const char *message2){
    std::string buf(message);
    buf+=" ";
    buf+=message2;
    msg(buf);
}

Log::ref instance;

Log::ref Log::getInstance() {
    if (!instance) { 
        instance=Log::ref(new Log());
    }
    return instance;
}