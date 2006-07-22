/*
 *  Started on 14.06.2006
 *
 *
 */

//#include "stdafx.h"
#include <Socket.h>

static int wsCount=0;

Socket::Socket(void)
{
}

Socket::~Socket(void)
{
	closesocket(sock);
	wsCount--;
	if (wsCount==0) WSACleanup();
}


Socket * Socket::createSocket(const std::string &url, const int port) {
	/* INIT WINSOCKS */
	if (wsCount==0) {
		WSADATA wsaData;
		int err;
		 
		err = WSAStartup( 0x202, &wsaData );
		if ( err != 0 ) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			return NULL; //todo: terminate
		}

		if (wsaData.wVersion!=0x202) {
			WSACleanup();
			return NULL; //todo: terminate
		}
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
	return recv(sock, buf, len, 0);
}

int Socket::write(const char * buf, int len) {
	return send(sock, buf, len, 0);
}

int Socket::write(StringRef buf){
	return write(buf->c_str(), buf->length());
}
int Socket::write(std::string &buf){
	return write(buf.c_str(), buf.length());
}
