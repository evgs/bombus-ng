#include "lstring.h"

void locale::toLowerCase(std::string &str) {
    for (unsigned int i=0; i<str.length(); i++) {
        char c=str[i];
        if (c>='A' && c<='Z') str[i]=c+'a'-'A';
    }
}