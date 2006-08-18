
#include "Socket.h"
#include <yassl.hpp>
#include "TLSSocket.h"

#include <openssl/ssl.h>

using namespace yaSSL;

TLSSocket::TLSSocket(ConnectionRef sock){
    this->sock=sock;

    SOCKET sockfd= ((Socket *)(sock.get()))->getSocket();

    /*sslClient=new yaSSL::Client();
    //sslClient->SetCA("E:\\bombus-ng\\certs\\class3.crt");
    sslClient->SetCA("E:\\bombus-ng\\certs\\root.der");
    int result=sslClient->Connect(sockfd);*/

    SSL_METHOD* method=TLSv1_client_method();
    SSL_CTX* ctx=SSL_CTX_new(method);

    SSL* ssl=SSL_new(ctx);

    int crt_result=SSL_CTX_use_certificate_file(ctx, "E:\\bombus-ng\\certs\\class3.crt", SSL_FILETYPE_PEM);

    SSL_set_fd(ssl, sockfd);

    int result=SSL_connect(ssl);

    int err=SSL_get_error(ssl, 0);


}

TLSSocket::~TLSSocket(){
    
}

int TLSSocket::read(char *buf, int len) {
    return sslClient->Read(buf, len);
};

int TLSSocket::write(const char * buf, int len) {
    return sslClient->Write(buf, len);
};

const std::string TLSSocket::getStatistics() {
    return std::string("stub");
}