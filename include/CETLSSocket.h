#pragma once
#include "Socket.h"

class CeTLSSocket : public Socket 
{
public:
    CeTLSSocket(const long addr, const int port);
    virtual ~CeTLSSocket(void);
    virtual const std::string getStatistics();

    bool ignoreSSLWarnings;
public:
    bool startTls(const std::string &url, bool ignoreSSLWarnings);

    typedef boost::shared_ptr<CeTLSSocket> ref;

protected:
    CeTLSSocket(){};
    static int SslValidate (DWORD  dwType, LPVOID pvArg, DWORD  dwChainLen, LPBLOB pCertChain, DWORD  dwFlags); 
};
