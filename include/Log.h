#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

#include "VirtualListView.h"

class Log {
public:
    typedef boost::shared_ptr<Log> ref;
    static Log::ref getInstance();

    ~Log();

private:
	Log();

    //static Log::ref instance;

    VirtualListView::ref odrLog;

public:

    void bindLV(VirtualListView::ref odrLog) { this->odrLog=odrLog; }

	void msg(const std::string &message);
	void msg(const char * message);
	void msg(const char * message, const char * message2);

private:
    void Log::addLog(const wchar_t * msg);
};
