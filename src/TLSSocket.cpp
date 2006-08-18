
#include "Socket.h"

#include "TLSSocket.h"
#include "stdio.h"

#include <openssl/ssl.h>

TLSSocket::TLSSocket(ConnectionRef sock){
    this->sock=sock;

    SOCKET sockfd= ((Socket *)(sock.get()))->getSocket();

    /*sslClient=new yaSSL::Client();
    //sslClient->SetCA("E:\\bombus-ng\\certs\\class3.crt");
    sslClient->SetCA("E:\\bombus-ng\\certs\\root.der");
    int result=sslClient->Connect(sockfd);*/

    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();

    SSL_METHOD* method=TLSv1_client_method();
    SSL_CTX* ctx=SSL_CTX_new(method);
    this->ctx=ctx;

    SSL* ssl=SSL_new(ctx);
    this->ssl=ssl;

    int crt_result=SSL_CTX_use_certificate_file(ctx, "E:\\bombus-ng\\certs\\class3.crt", SSL_FILETYPE_PEM);

    SSL_set_fd(ssl, sockfd);

    int result=SSL_connect(ssl);

    printf ("SSL connection using %s\n", SSL_get_cipher (ssl));


    int err=SSL_get_error(ssl, 0);

    X509*    server_cert=  SSL_get_peer_certificate (ssl);

    printf ("Server certificate:\n");

    char * str = X509_NAME_oneline (X509_get_subject_name (server_cert),0,0);
    printf ("\t subject: %s\n", str);

    OPENSSL_free (str);

    str = X509_NAME_oneline (X509_get_issuer_name  (server_cert),0,0);

    printf ("\t issuer: %s\n", str);
    OPENSSL_free (str);

    /* We could do all sorts of certificate verification stuff here before
    deallocating the certificate. */

    X509_free (server_cert);

}

TLSSocket::~TLSSocket(){
    SSL_free ((SSL*)ssl);
    SSL_CTX_free ((SSL_CTX*)ctx);
}

int TLSSocket::read(char *buf, int len) {
    return SSL_read((SSL*)ssl, buf, len);
};

int TLSSocket::write(const char * buf, int len) {
    return SSL_write((SSL*)ssl, buf, len);
};

const std::string TLSSocket::getStatistics() {
    return std::string("stub");
}