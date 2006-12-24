#include "stdafx.h"

#include <CeTLSSocket.h>
#include <sslsock.h>
#include <boost/assert.hpp>

static int wsCount;

int SslValidate (
                 DWORD  dwType,
                 LPVOID pvArg,
                 DWORD  dwChainLen,
                 LPBLOB pCertChain,
                 DWORD  dwFlags
                 ) 
{
    return SSL_ERR_OKAY;

};


CeTLSSocket * CeTLSSocket::createTlsSocket(const std::string & url, const int port){
    /* INIT WINSOCKS */
    if (wsCount==0) {
        WSADATA wsaData;

        int err=WSAStartup( 0x202, &wsaData );

        BOOST_ASSERT(  err==0  ); 

        if (wsaData.wVersion!=0x202) {
            WSACleanup();
            BOOST_ASSERT(0);
        }

        wsCount++;
    }

    SOCKET s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s==INVALID_SOCKET) return NULL;

    DWORD dw=SO_SEC_SSL;
    int optres=setsockopt(s, SOL_SOCKET, SO_SECURE, (const char *)&dw, sizeof(DWORD));

    //Specify the certificate validation callback function
    SSLVALIDATECERTHOOK hook;
    hook.HookFunc=SslValidate;
    hook.pvArg=0;

    int ioctlresult=WSAIoctl(s, SO_SSL_SET_VALIDATE_CERT_HOOK , &hook, sizeof(hook), 0, 0, 0, 0, 0);

    //Set the socket in deferred handshake mode
    dw=SSL_FLAG_DEFER_HANDSHAKE;
    ioctlresult=WSAIoctl(s, SO_SSL_SET_FLAGS, &dw, sizeof(DWORD), 0, 0, 0, 0, 0);


    struct hostent FAR* host=gethostbyname(url.c_str());

    if (host==NULL) return NULL;

    struct sockaddr_in name;
    name.sin_family=AF_INET;
    name.sin_addr.S_un.S_addr=*((unsigned long *)host->h_addr_list[0]);
    name.sin_port= htons(port); // internet byte order

    int result=connect(s, (sockaddr*)(&name), sizeof(name));

    if (result==SOCKET_ERROR) return NULL;

    CeTLSSocket *res=new CeTLSSocket();
    res->sock=s;
    res->hostname=url;

    return res;
};

bool CeTLSSocket::switchTls(){
    int ioctlresult=WSAIoctl(sock, SO_SSL_PERFORM_HANDSHAKE, (LPVOID)hostname.c_str(), hostname.length(), 0, 0, 0, 0, 0);
    return true;
};
