#pragma once

#include "Log.h"

#include "VirtualListView.h"

class LogPanel: public Log {
private:
	LogPanel();

    //static Log::ref instance;

    VirtualListView::ref odrLog;

public:
    virtual ~LogPanel();

    static void bindLV(VirtualListView::ref odrLog); 

private:
    virtual void addLog(const wchar_t * msg);
};
