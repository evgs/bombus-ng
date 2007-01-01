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


CeTLSSocket::CeTLSSocket(const std::string & url, const int port){
    initWinsocks();
    this->url=url;

    sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock==INVALID_SOCKET) throwSocketError();

    DWORD dw=SO_SEC_SSL;
    int optres=setsockopt(sock, SOL_SOCKET, SO_SECURE, (const char *)&dw, sizeof(DWORD));
    if (optres==SOCKET_ERROR) throwSocketError();

    //Specify the certificate validation callback function
    SSLVALIDATECERTHOOK hook;
    hook.HookFunc=SslValidate;
    hook.pvArg=0;

    int ioctlresult=WSAIoctl(sock, SO_SSL_SET_VALIDATE_CERT_HOOK , &hook, sizeof(hook), 0, 0, 0, 0, 0);
    if (ioctlresult==SOCKET_ERROR) throwSocketError();

    //Set the socket in deferred handshake mode
    dw=SSL_FLAG_DEFER_HANDSHAKE;
    ioctlresult=WSAIoctl(sock, SO_SSL_SET_FLAGS, &dw, sizeof(DWORD), 0, 0, 0, 0, 0);
    if (ioctlresult==SOCKET_ERROR) throwSocketError();

    struct hostent FAR* host=gethostbyname(url.c_str());

    if (host==NULL) throwSocketError();

    struct sockaddr_in name;
    name.sin_family=AF_INET;
    name.sin_addr.S_un.S_addr=*((unsigned long *)host->h_addr_list[0]);
    name.sin_port= htons(port); // internet byte order

    int result=connect(sock, (sockaddr*)(&name), sizeof(name));

    if (result==SOCKET_ERROR) throwSocketError();

};

bool CeTLSSocket::switchTls(){
    int ioctlresult=WSAIoctl(sock, SO_SSL_PERFORM_HANDSHAKE, (LPVOID)url.c_str(), url.length(), 0, 0, 0, 0, 0);
    return true;
};
