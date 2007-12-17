#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

class Log {
public:
    typedef boost::shared_ptr<Log> ref;
    static Log::ref getInstance();

    virtual ~Log();

protected:
    Log(){};

    static void setInstance(Log * log);
    //static Log::ref instance;

public:

	void msg(const std::string &message);
	void msg(const char * message);
	void msg(const char * message, const char * message2);

protected:
    virtual void addLog(const wchar_t * msg);
};
