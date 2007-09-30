#pragma once

#include <string>
#include <windows.h>

class Serialize {
public:
    Serialize(LPCTSTR fileName, bool read);
    ~Serialize();
    void close();
    void streamBool  (bool &data, bool defValue=false);
    void streamShort (short &data, short defValue=0);
    void streamInt   (int &data, int defValue=0);
    void streamString(std::string &data, const char *defValue=NULL);
    // ***WARNING!!!*** THIS method is not a strong cryptographic algorithm.
    // the simple usage of this method is protecting only from accidental displaying of scrambled data
    void streamScrambledString(std::string &data);

    enum {
        READ=true,
        WRITE=false
    };

    bool isReading() { return read; }
private:
    void streamData (void * data, int len);
    HANDLE file;
    bool read;
};
