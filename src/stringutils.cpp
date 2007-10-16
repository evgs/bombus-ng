#include "stringutils.h"
#include "locale.h"

void std::trimHead(std::string &str){
    //leading whitespaces
    while (str.length()) {
        if (iswspace(str[0])) str.erase(0, 1); else break;
    }
}
void std::trimTail(std::string &str){
    //trailing whitespaces
    while (str.length()) {
        int pos=str.length()-1;
        if (iswspace(str[pos])) str.erase(pos, pos+1); else break;
    }
}

void std::trim(std::string &str) {
    trimHead(str);
    trimTail(str);
}

void std::strAppendInt(std::string &s, int n){
    char tmpbuf[10];
    sprintf(tmpbuf, "%d", n);
    s+=tmpbuf;
}
