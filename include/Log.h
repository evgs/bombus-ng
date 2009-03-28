#pragma once

#include <string>
#include "boostheaders.h"

class Log {
public:
    typedef boost::shared_ptr<Log> ref;
    static Log::ref getInstance();

    virtual ~Log();

	enum VERBOSITY_LEVEL {
		debug = 0,
		info,
		warning,
		error
	};

protected:
    Log();

    static void setInstance(Log * log);
    //static Log::ref instance;

public:

	void msg(const std::string &message, int level);
	void msg(const char * message, int level);
	void msg(const char * message, const char * message2, int level);

	void setLogLevel(int level);

	int getLogLevel();

protected:
    virtual void addLog(const wchar_t * msg, int level);

	int level;
};
