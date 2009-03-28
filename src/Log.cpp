#include "Log.h"

#include "utf8.hpp"

Log::Log() {
	level=debug;
}

Log::~Log(){}

void Log::addLog(const wchar_t * msg, int level) {}

void Log::msg(const std::string &message, int level){
    addLog(utf8::utf8_wchar(message).c_str(), level);
}

void Log::msg(const char * message, int level){
    msg(std::string(message), level);
}

void Log::msg(const char * message, const char *message2, int level){
    std::string buf(message);
    buf+=" ";
    buf+=message2;
    msg(buf, level);
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

void Log::setLogLevel(int level) {
	this->level=level;
}

int Log::getLogLevel() {
	return level;
}
