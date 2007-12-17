#include "Log.h"

#include "utf8.hpp"

Log::~Log(){}

void Log::addLog(const wchar_t * msg) {}

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
    if (!instance) 
        setInstance(new Log());

    return instance;
}

void Log::setInstance( Log * log ) {
    instance=Log::ref(log);
}