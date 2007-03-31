#pragma once

#include <string>
#include <windows.h>

class Serialize {
public:
    Serialize(LPCTSTR fileName, bool read);
    ~Serialize();
    void close();
    void streamBool  (bool &data);
    void streamShort (short &data);
    void streamInt   (int &data);
    void streamString(std::string &data);
    // ***WARNING!!!*** THIS method is not a strong cryptographic algorithm.
    // the simple usage of this method is protecting only from accidental displaying of scrambled data
    void streamScrambledString(std::string &data);

private:
    void streamData (void * data, int len);
    HANDLE file;
    bool read;
};
