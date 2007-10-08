//#include "stdafx.h"

#include <CeTLSSocket.h>

#include <boost/assert.hpp>
#include <string>
#include "utf8.hpp"

#include <wincrypt.h>
#include <sslsock.h>
#include <schnlsp.h>

#include "TimeFunc.h"
#include "stringutils.h"
// load SslCrackCertificate and SslFreeCertificate
#define SSL_CRACK_CERTIFICATE_NAME TEXT("SslCrackCertificate")
#define SSL_FREE_CERTIFICATE_NAME TEXT("SslFreeCertificate")

//////////////////////////////////////////////////////////////////////////
SSL_CRACK_CERTIFICATE_FN gSslCrackCertificate=0;
SSL_FREE_CERTIFICATE_FN gSslFreeCertificate=0;
HINSTANCE hSchannelDLL=0;

HRESULT LoadSSL(){

    // already loaded?
    if (hSchannelDLL) return S_OK;
    //if (gSslCrackCertificate && gSslFreeCertificate) return S_OK;

    hSchannelDLL = LoadLibrary(TEXT("schannel.dll"));
    if (!hSchannelDLL) {
        // error logging
        gSslCrackCertificate = NULL;
        gSslFreeCertificate = NULL;
        return E_FAIL;
    }

    gSslCrackCertificate = (SSL_CRACK_CERTIFICATE_FN)GetProcAddress(hSchannelDLL, SSL_CRACK_CERTIFICATE_NAME);
    gSslFreeCertificate = (SSL_FREE_CERTIFICATE_FN)GetProcAddress(hSchannelDLL, SSL_FREE_CERTIFICATE_NAME);

    if (!gSslCrackCertificate || !gSslFreeCertificate) {
        // error logging

        gSslCrackCertificate = NULL;
        gSslFreeCertificate = NULL;
        FreeLibrary(hSchannelDLL);
        hSchannelDLL = NULL;
        return E_FAIL;
    } else {
        return S_OK;
    }
}

HRESULT FreeSSL()
{
    if (hSchannelDLL) {
        FreeLibrary(hSchannelDLL);
        hSchannelDLL = NULL;
        gSslCrackCertificate=NULL;
        gSslFreeCertificate=NULL;
    }
    return S_OK;
}

int CeTLSSocket::SslValidate (
                 DWORD  dwType,
                 LPVOID pvArg,
                 DWORD  dwChainLen,
                 LPBLOB pCertChain,
                 DWORD  dwFlags
                 ) 
{
    CeTLSSocket * s=(CeTLSSocket *)pvArg;

    if (s->ignoreSSLWarnings) return SSL_ERR_OKAY;

    if (dwType!=SSL_CERT_X509) return SSL_ERR_CERT_UNKNOWN;

    if (pCertChain==NULL) return SSL_ERR_CERT_UNKNOWN;

    X509Certificate* pCert = NULL;

    if (dwFlags & SSL_CERT_FLAG_ISSUER_UNKNOWN) {
        //TODO: ask for accept/decline certificate

        //std::wstring url=utf8::utf8_wchar(s->url);

        if (!gSslCrackCertificate || !gSslFreeCertificate) return SSL_ERR_CERT_UNKNOWN;

        // crack X.509 Certificate
        if (!gSslCrackCertificate(pCertChain->pBlobData, pCertChain->cbSize, TRUE, &pCert)) return SSL_ERR_BAD_DATA;

        std::string certInfo="\nCertificate Issuer unknown";
        certInfo+="\nIssuer: "; certInfo+=pCert->pszIssuer;
        certInfo+="\nSubject: "; certInfo+=pCert->pszSubject;
        certInfo+="\nValid from: "; certInfo+=strtime::toLocalDate(pCert->ValidFrom);
        certInfo+="\nValid until: "; certInfo+=strtime::toLocalDate(pCert->ValidUntil);

        certInfo+="\n\nAccept this certificate?";

        gSslFreeCertificate(pCert);

        std::wstring wcertInfo=utf8::utf8_wchar(certInfo);

        int result=MessageBox(NULL, wcertInfo.c_str(), TEXT("SSL Certificate Error"), MB_OKCANCEL | MB_ICONEXCLAMATION );
        if (result!=IDOK) return SSL_ERR_CERT_UNKNOWN;
    }

    return SSL_ERR_OKAY;

};


CeTLSSocket::CeTLSSocket(const long addr, const int port){
    bytesSent=bytesRecvd=0;
    ignoreSSLWarnings=false;

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
    name.sin_addr.S_un.S_addr=addr;
    name.sin_port= htons(port); // internet byte order

    int result=connect(sock, (sockaddr*)(&name), sizeof(name));

    if (result==SOCKET_ERROR) throwSocketError();

};

CeTLSSocket::~CeTLSSocket(){
    FreeSSL();
}

bool CeTLSSocket::startTls(const std::string &url, bool ignoreSSLWarnings){
    this->ignoreSSLWarnings=ignoreSSLWarnings;
    int result=LoadSSL();
    BOOST_ASSERT(result==S_OK);
    int ioctlresult=WSAIoctl(sock, SO_SSL_PERFORM_HANDSHAKE, (LPVOID)url.c_str(), url.length(), 0, 0, 0, 0, 0);
    if (ioctlresult==SOCKET_ERROR) throwSocketError();
    return true;
};


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
    default: std::strAppendInt(stats, SSLConnectionInfo.dwProtocol |0x80);
    }
    stats+="\nChiper: ";

    switch(SSLConnectionInfo.aiCipher) {
        case CALG_RC2: stats+="RC2"; break;
        case CALG_RC4: stats+="RC4"; break;
        case CALG_DES: stats+="DES"; break;
        case CALG_3DES: stats+="Triple DES"; break;
        case CALG_SKIPJACK: stats+="Skipjack"; break;
        default: std::strAppendInt(stats, SSLConnectionInfo.aiCipher | 0x80);
    }
    stats+=", ";
    std::strAppendInt(stats, SSLConnectionInfo.dwCipherStrength);

    stats+="\nHash: ";
    switch(SSLConnectionInfo.aiHash) {
    case CALG_MD5: stats+="MD5"; break;
    case CALG_SHA: stats+="SHA"; break;
    default: std::strAppendInt(stats, SSLConnectionInfo.aiHash);
    }
    stats+=", ";
    std::strAppendInt(stats, SSLConnectionInfo.dwHashStrength);

    stats+="\nKey Exchange: ";

    switch(SSLConnectionInfo.aiExch)
    {
    case CALG_RSA_KEYX: 
    case CALG_RSA_SIGN: stats+="RSA"; break;

    case CALG_KEA_KEYX: stats+="KEA"; break;
    default: std::strAppendInt(stats, SSLConnectionInfo.aiExch);
    }
    stats+=", ";
    std::strAppendInt(stats, SSLConnectionInfo.dwExchStrength);

    stats+="\n";

    stats+=Socket::getStatistics();

    return stats;
}