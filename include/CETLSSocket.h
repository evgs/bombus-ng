#pragma once
#include "Socket.h"

class CeTLSSocket : public Socket 
{
public:
    CeTLSSocket(const std::string & url, const int port);

public:
    bool startTls();

protected:
    CeTLSSocket(){};
};
