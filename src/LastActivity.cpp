#include "LastActivity.h"
#include "JabberStream.h"
#include "stringutils.h"

ProcessResult LastActivity::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef query=block->findChildNamespace("query","jabber:iq:last");
    if (!query) return BLOCK_REJECTED;

    //Log::getInstance()->msg("Last Activity query: ", block->getAttribute("from").c_str());

    JabberDataBlock result("iq");
    result.setAttribute("to", block->getAttribute("from"));
    result.setAttribute("type", "result");
    result.setAttribute("id", block->getAttribute("id"));
    result.addChild(query);
    query->setAttribute("seconds",delay());
    //query->setText("This is stub time");

    rc->jabberStream->sendStanza(result);
    return BLOCK_PROCESSED;
}

void LastActivity::update() {
    lastActivityTime=strtime::getCurrentUtc();
}

std::string LastActivity::delay() {
    PackedTime dl=strtime::getCurrentUtc();

    void *p1=&dl;
    void *p2=&lastActivityTime;

    __int64 delta=(*((__int64 *)p1)) - (*((__int64 *)p2));
    int seconds=(int)(delta/10000000);
    std::string result;
    std::strAppendInt(result, seconds);
    return result;
}
//////////////////////////////////////////////////////////////////////////
PackedTime LastActivity::lastActivityTime;