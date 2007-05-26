#include "stringutils.h"
#include "locale.h"

void std::trim(std::string &str) {
    //leading whitespaces
    while (str.length()) {
        if (iswspace(str[0])) str.erase(0, 1); else break;
    }
    
    if (str.empty()) return;

    //trailing whitespaces
    while (str.length()) {
        int pos=str.length()-1;
        if (iswspace(str[pos])) str.erase(pos, pos+1); else break;
    }
}

void std::strAppendInt(std::string &s, int n){
    char tmpbuf[10];
    sprintf(tmpbuf, "%d", n);
    s+=tmpbuf;
}
