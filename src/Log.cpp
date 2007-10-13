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

wchar_t buf[256];

const wchar_t * charToWchar(const char * src, const char *src2 = NULL) {
    wchar_t *b=buf;

    int i;
    for (i=0; i<255; i++) {
        if (*src ==0 ) break;
        *(b++)=*(src++);
    }

    //*(b++)=0x20;
    if (src2!=0)
        for (; i<255; i++) {
            if (*src2 ==0 ) break;
            *(b++)=*(src2++);
        }
        *b=0;

        return buf;
}


void Log::addLog(const wchar_t * msg) {
    //ListBox_AddString( logWnd->getListBoxHWnd(), msg);
    //ODRRef r=ODRRef(new IconTextElementContainer(std::wstring(msg), -1));
    ODRRef r=ODRRef(new LogMessage(msg));
    odrLog->addODR(r, true);
}

void Log::msg(const std::string &message){
    addLog(charToWchar(message.c_str()));
}

void Log::msg(const char * message){
    addLog(charToWchar(message));
}

void Log::msg(const char * message, const char *message2){
    addLog(charToWchar(message, message2));
}

Log::ref instance;

Log::ref Log::getInstance() {
    if (!instance) { 
        instance=Log::ref(new Log());
    }
    return instance;
}