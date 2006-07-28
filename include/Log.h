#pragma once

#include <string>

class Log {
public:
	Log();
	~Log();

public:
	void msg(const std::string &message);
	void msg(const char * message);
	void msg(const char * message, const char * message2);
};
