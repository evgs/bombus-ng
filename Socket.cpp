#include "stdafx.h"


/*
 *  Started on 14.06.2006
 *
 *
 */

#include <Socket.h>
#include <boost/assert.hpp>

static int wsCount=0;


Socket::~Socket(void){
	closesocket(sock);
	wsCount--;
	if (wsCount==0) WSACleanup();
}

void Socket::initWinsocks(){
    /* INIT WINSOCKS */
    if (wsCount!=0) return; 

    WSADATA wsaData;

    int err=WSAStartup( 0x202, &wsaData );

    BOOST_ASSERT(  err==0  ); 

    if (wsaData.wVersion!=0x202) {
        WSACleanup();
        BOOST_ASSERT(0);
    }

    wsCount++;
}

Socket::Socket(const std::string &url, const int port) {
    bytesSent=bytesRecvd=0;
    initWinsocks();
    this->url=url;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock==INVALID_SOCKET) throwSocketError();

	struct hostent FAR* host=gethostbyname(url.c_str());

	if (host==NULL) throwSocketError();

	struct sockaddr_in name;
	name.sin_family=AF_INET;
	name.sin_addr.S_un.S_addr=*((unsigned long *)host->h_addr_list[0]);
	name.sin_port= htons(port); // internet byte order

	int result=connect(sock, (sockaddr*)(&name), sizeof(name));

	if (result==SOCKET_ERROR) throwSocketError();
}

void Socket::throwSocketError() {
    std::string err="Socket error ";
    err+=WSAGetLastError();
    err+='(';
    err+=url;
    err+=')';
    throw std::exception(err.c_str());
}

int Socket::read(char * buf, int len) {
	int rb=recv(sock, buf, len, 0);
	bytesRecvd+=rb;
	return rb;
}

int Socket::write(const char * buf, int len) {
	//for (int i=0; i<len; i++)	std::cout << buf[i];
	//std::cout<<"("<<len<<")"<<std::endl;
	int sb=send(sock, buf, len, 0);
	bytesSent+=sb;
	return sb;
}

const std::string Socket::getStatistics(){
	char *fmt="--- Socket ---\n"
	"sent=%d\n"
	"recv=%d\n";

	char buf[256];

	//sprintf_s(buf, 256, fmt, bytesSent, bytesRecvd);
	sprintf(buf, fmt, bytesSent, bytesRecvd);

	return std::string(buf);
}
