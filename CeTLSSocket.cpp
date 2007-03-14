//#include "stdafx.h"

#include <CeTLSSocket.h>
#include <sslsock.h>
#include <boost/assert.hpp>
#include <string>
#include "utf8.hpp"

static int wsCount;

int CeTLSSocket::SslValidate (
                 DWORD  dwType,
                 LPVOID pvArg,
                 DWORD  dwChainLen,
                 LPBLOB pCertChain,
                 DWORD  dwFlags
                 ) 
{
    if (dwFlags==SSL_CERT_FLAG_ISSUER_UNKNOWN) {
        //TODO: ask for accept/decline certificate
        CeTLSSocket * s=(CeTLSSocket *)pvArg;
        std::wstring url=utf8::utf8_wchar(s->url);

        //int result=
    }

    return SSL_ERR_OKAY;

};


CeTLSSocket::CeTLSSocket(const std::string & url, const int port){
    bytesSent=bytesRecvd=0;

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
    hook.pvArg=(LPVOID) this;

    int ioctlresult=WSAIoctl(sock, SO_SSL_SET_VALIDATE_CERT_HOOK , &hook, sizeof(hook), 0, 0, 0, 0, 0);
    if (ioctlresult==SOCKET_ERROR) throwSocketError();

    //Set the socket in deferred handshake mode
    dw=SSL_FLAG_DEFER_HANDSHAKE;
    ioctlresult=WSAIoctl(sock, SO_SSL_SET_FLAGS, &dw, sizeof(DWORD), 0, 0, 0, 0, 0);
    if (ioctlresult==SOCKET_ERROR) throwSocketError();

    struct sockaddr_in name;
    name.sin_family=AF_INET;
    name.sin_addr.S_un.S_addr=resolveUrl();
    name.sin_port= htons(port); // internet byte order

    int result=connect(sock, (sockaddr*)(&name), sizeof(name));

    if (result==SOCKET_ERROR) throwSocketError();

};

bool CeTLSSocket::startTls(){
    int ioctlresult=WSAIoctl(sock, SO_SSL_PERFORM_HANDSHAKE, (LPVOID)url.c_str(), url.length(), 0, 0, 0, 0, 0);
    return true;
};


void strAppendInt(std::string &s, int n){
    char tmpbuf[10];
    sprintf(tmpbuf, "%d", n);
    s+=tmpbuf;
}

//////////////////////////////////////////////////////////////////////////
const std::string CeTLSSocket::getStatistics(){

    std::string stats="--- Stream Encryption ---";
    // show SSLCONNECTIONINFO
    SSLCONNECTIONINFO SSLConnectionInfo;
    DWORD dwBytes = 0;

    int sockerror = WSAIoctl(sock, SO_SSL_GET_CONNECTION_INFO,
        NULL, 0, &SSLConnectionInfo, sizeof(SSLConnectionInfo), &dwBytes, NULL,NULL);
    if (sockerror == SOCKET_ERROR) throwSocketError();

    stats+="\nProtocol: ";
    switch(SSLConnectionInfo.dwProtocol) {
    case SSL_PROT_PCT1_CLIENT: stats+="PCT"; break;
    case SSL_PROT_SSL2_CLIENT: stats+="SSL2"; break;
    case SSL_PROT_SSL3_CLIENT: stats+="SSL3"; break;
    case SSL_PROT_TLS1_CLIENT: stats+="TLS1"; break;
    default: strAppendInt(stats, SSLConnectionInfo.dwProtocol |0x80);
    }
    stats+="\nChiper: ";

    switch(SSLConnectionInfo.aiCipher) {
        case CALG_RC2: stats+="RC2"; break;
        case CALG_RC4: stats+="RC4"; break;
        case CALG_DES: stats+="DES"; break;
        case CALG_3DES: stats+="Triple DES"; break;
        case CALG_SKIPJACK: stats+="Skipjack"; break;
        default: strAppendInt(stats, SSLConnectionInfo.aiCipher | 0x80);
    }
    stats+=", ";
    strAppendInt(stats, SSLConnectionInfo.dwCipherStrength);

    stats+="\nHash: ";
    switch(SSLConnectionInfo.aiHash) {
    case CALG_MD5: stats+="MD5"; break;
    case CALG_SHA: stats+="SHA"; break;
    default: strAppendInt(stats, SSLConnectionInfo.aiHash);
    }
    stats+=", ";
    strAppendInt(stats, SSLConnectionInfo.dwHashStrength);

    stats+="\nKey Exchange: ";

    switch(SSLConnectionInfo.aiExch)
    {
    case CALG_RSA_KEYX: 
    case CALG_RSA_SIGN: stats+="RSA"; break;

    case CALG_KEA_KEYX: stats+="KEA"; break;
    default: strAppendInt(stats, SSLConnectionInfo.aiExch);
    }
    stats+=", ";
    strAppendInt(stats, SSLConnectionInfo.dwExchStrength);

    stats+="\n";

    stats+=Socket::getStatistics();

    return stats;
}