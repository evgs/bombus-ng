#include "Serialize.h"

extern std::wstring appRootPath;

Serialize::Serialize( LPCTSTR fileName, bool read ) {
    std::wstring path=appRootPath+fileName;
    file=CreateFile(path.c_str(), 
                    (read)? GENERIC_READ : GENERIC_WRITE, 
                    FILE_SHARE_READ, NULL, 
                    (read)? OPEN_EXISTING : OPEN_ALWAYS,
                    0, NULL);
    this->read=read;
}
Serialize::~Serialize() { close(); }

void Serialize::close() {
    if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
    file=INVALID_HANDLE_VALUE;
}

void Serialize::streamData( void * data, int len ) {
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD dwProcessed;
    if (read) ReadFile(file, data, len, &dwProcessed, NULL);
    else WriteFile(file, data, len, &dwProcessed, NULL);
}

void Serialize::streamInt( int &data ) {
    int *pdata=&data;
    streamData((LPVOID)(pdata), sizeof(int));
}

void Serialize::streamShort( short &data ) {
    short *pdata=&data;
    streamData((LPVOID)(pdata), sizeof(short));
}

void Serialize::streamBool( bool &data ) {
    bool *pdata=&data;
    streamData((LPVOID)(pdata), sizeof(bool));
}

void Serialize::streamString( std::string &data ) {
    if (file == INVALID_HANDLE_VALUE) return;
    short len;
    len=data.length();
    streamShort(len);
    char *buf=new char[len+1];

#pragma warning(push)
#pragma warning(disable : 4996)
    if (!read) data.copy(buf, len, 0);
#pragma warning(pop)


    streamData((LPVOID) buf, len);
    buf[len]=0;
    if (read) data=buf;
    delete buf;
}

void eor(char *buf, int len) {
    int c=((len*len)^0x5A6B7C8D);
    for (int i=0; i<len; i++)
        buf[i]=buf[i]^ (0xff & (c>> (i % 24))); 
}


void Serialize::streamScrambledString( std::string &data ) {
    if (file == INVALID_HANDLE_VALUE) return;
    short len;
    len=data.length();
    streamShort(len);
    char *buf=new char[len+1];

    short method=1; // if some stronger crypto method will be used here
    streamShort(method);

#pragma warning(push)
#pragma warning(disable : 4996)
    if (!read) data.copy(buf, len, 0);
#pragma warning(pop)

    if (method==1) eor(buf, len);
    streamData((LPVOID) buf, len);
    if (method==1) eor(buf, len);
    buf[len]=0;
    if (read) data=buf;
    delete buf;
}