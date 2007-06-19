#include "History.h"
#include "utf8.hpp"

#include <stdio.h>

History::ref History::getInstance() {
    if (!instance) instance=History::ref(new History());
    return instance;
}

void History::appendHistory( Contact::ref c, Message::ref msg ) {
    if (!writeHistory) return;
    std::wstring filePath=historyPath;
    filePath+='\\';
    //todo: normalize filename
    filePath+=utf8::utf8_wchar(c->jid.getBareJid());
    filePath+=L".txt";

    FILE *f=_wfopen(filePath.c_str(), L"a");
    if (f==NULL) return;
    std::string t=strtime::toDate(msg->time);
    t+=' ';
    t+=strtime::toTime(msg->time);

    fprintf(f, "[%s] (%s)%s\n", t.c_str(), msg->fromName.c_str(), msg->getMessageText().c_str());
    fclose(f);
}

extern std::wstring appRootPath;

History::History() {
    historyPath=appRootPath+L"history";
    //CreateDirectory(historyPath.c_str(), NULL);
    DWORD attrs=GetFileAttributes(historyPath.c_str());
    if (attrs==0xFFFFFFFF) {
        writeHistory=false;
    } else {
        writeHistory=(attrs & FILE_ATTRIBUTE_DIRECTORY)!=0;
    }
}
History::ref History::instance=History::ref();