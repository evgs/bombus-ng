#pragma once
#include "Socket.h"

class CeTLSSocket : public Socket 
{
public:
    static CeTLSSocket * createTlsSocket(const std::string & url, const int port);

public:
    bool switchTls();

private:
    std::string hostname;
};
