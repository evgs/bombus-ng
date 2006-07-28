#include "Log.h"
#include <stdio.h>

Log::Log(){
}

Log::~Log(){
}

void Log::msg(const std::string &message){
	puts(message.c_str());
}

void Log::msg(const char * message){
	puts(message);
}

void Log::msg(const char * message, const char *message2){
	printf ("%s %s\n", message, message2);
}
