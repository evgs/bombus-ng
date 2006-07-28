/*
 *  Started on 14.06.2006
 *
 *
 */

//#include "stdafx.h"
#include <Socket.h>
#include <boost/assert.hpp>
#include <iostream>

static int wsCount=0;

Socket::Socket(void){	
	bytesSent=bytesRecvd=0;
}

Socket::~Socket(void){
	closesocket(sock);
	wsCount--;
	if (wsCount==0) WSACleanup();
}


Socket * Socket::createSocket(const std::string &url, const int port) {
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

	struct hostent FAR* host=gethostbyname(url.c_str());

	struct sockaddr_in name;
	name.sin_family=AF_INET;
	name.sin_addr.S_un.S_addr=*((unsigned long *)host->h_addr_list[0]);
	name.sin_port= htons(port); // internet byte order

	int result=connect(s, (sockaddr*)(&name), sizeof(name));

	if (result==SOCKET_ERROR) return NULL;

	Socket *res=new Socket();
	res->sock=s;

	return res;
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

	sprintf_s(buf, 256, fmt, bytesSent, bytesRecvd);

	return std::string(buf);
}
