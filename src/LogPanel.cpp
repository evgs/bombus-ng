#include "LogPanel.h"

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

LogPanel::LogPanel(){}

LogPanel::~LogPanel(){}

void LogPanel::addLog(const wchar_t * msg, int level) {
	if (level < this->level) return;
    //ListBox_AddString( logWnd->getListBoxHWnd(), msg);
    //ODRRef r=ODRRef(new IconTextElementContainer(std::wstring(msg), -1));
    ODRRef r=ODRRef(new LogMessage(msg));
    odrLog->addODR(r, true);
}

void LogPanel::bindLV( VirtualListView::ref odrLog ) {
    LogPanel *lp=new LogPanel();
    setInstance(lp);
    lp->odrLog=odrLog;
}