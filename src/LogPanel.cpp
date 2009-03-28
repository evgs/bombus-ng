#include "LogPanel.h"

#include "Message.h"
#include "utf8.hpp"


class LogMessage : public MessageElement {
public:
    virtual int getColor() const;
    LogMessage(const wchar_t * text, int level);
private:
	int level;
};

LogMessage::LogMessage(const wchar_t * text, int level) {
    wstr=std::wstring(text);
    init();
	this->level=level;
	smiles=false;
	if (level == Log::debug) singleLine=true;
}

int LogMessage::getColor() const {
	switch (level) {
		case Log::debug: return 0x404040;
		case Log::info: return 0xd09000;
		case Log::warning: return 0x000080;
		case Log::error: return 0x000090;
	}
    return 0;
}

LogPanel::LogPanel(){}

LogPanel::~LogPanel(){}

void LogPanel::addLog(const wchar_t * msg, int level) {
	if (level < this->level) return;
	Log::addLog(msg, level);
    //ListBox_AddString( logWnd->getListBoxHWnd(), msg);
    //ODRRef r=ODRRef(new IconTextElementContainer(std::wstring(msg), -1));
    ODRRef r=ODRRef(new LogMessage(msg, level));
    odrLog->addODR(r, true);
}

void LogPanel::bindLV( VirtualListView::ref odrLog ) {
    LogPanel *lp=new LogPanel();
    setInstance(lp);
    lp->odrLog=odrLog;
}