#pragma once
#include "Socket.h"

class CeTLSSocket : public Socket 
{
public:
    CeTLSSocket(const std::string & url, const int port);

    virtual const std::string getStatistics();

public:
    bool startTls();

protected:
    CeTLSSocket(){};
    static int SslValidate (DWORD  dwType, LPVOID pvArg, DWORD  dwChainLen, LPBLOB pCertChain, DWORD  dwFlags); 
};
