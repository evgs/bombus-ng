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

	struct sockaddr_in name;
	name.sin_family=AF_INET;
	name.sin_addr.S_un.S_addr=resolveUrl();
	name.sin_port= htons(port); // internet byte order

	int result=connect(sock, (sockaddr*)(&name), sizeof(name));

	if (result==SOCKET_ERROR) throwSocketError();
}

int Socket::read(char * buf, int len) {
	int rb=recv(sock, buf, len, 0);
    if (rb==SOCKET_ERROR) throwSocketError();
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

const char * errorWSAText(int code);

void Socket::throwSocketError() {
    int lastError=WSAGetLastError();
    std::string err="Socket error: ";
    err+=errorWSAText(lastError);
    err+=" (";
    err+=url;
    err+=")";
    throw std::exception(err.c_str());
}

long Socket::resolveUrl() {
    long inaddr=inet_addr(url.c_str());
    if (inaddr!=INADDR_NONE) return inaddr;

    struct hostent FAR* host=gethostbyname(url.c_str());
    if (host!=NULL) return  *((unsigned long *)host->h_addr_list[0]);
    throwSocketError();
    return NULL;
}

const char * errorWSAText(int code) {
    static char buf[10];
    static struct { int no; const char *msg; } *msgp, msgs[] = {
        { WSAEINTR, "Interrupted system call" },
        { WSAEBADF, "Bad file descriptor" },
        { WSAEACCES, "Permission denied" },
        { WSAEFAULT, "Bad address" },
        { WSAEINVAL, "Invalid argument" },
        { WSAEMFILE, "Too many open files" },
        { WSAEWOULDBLOCK, "Another winsock call while a "
        "blocking function was in progress" },
        { WSAEINPROGRESS, "Operation now in progress" },
        { WSAEALREADY, "Operation already in progress" },
        { WSAENOTSOCK, "Socket operation on non-socket" },
        { WSAEDESTADDRREQ, "Destination address required" },
        { WSAEMSGSIZE, "Message too long" },
        { WSAEPROTOTYPE, "Protocol wrong type for socket" },
        { WSAENOPROTOOPT, "Protocol not available" },
        { WSAEPROTONOSUPPORT, "Protocol not supported" },
        { WSAESOCKTNOSUPPORT, "Socket type not supported" },
        { WSAEOPNOTSUPP, "Operation not supported" },
        { WSAEPFNOSUPPORT, "Protocol family not supported" },
        { WSAEAFNOSUPPORT, "Address family not supported" },
        { WSAEADDRINUSE, "Address already in use" },
        { WSAEADDRNOTAVAIL, "Can't assign requested address" },
        { WSAENETDOWN, "Network is down" },
        { WSAENETUNREACH, "Network is unreachable" },
        { WSAENETRESET, "Network dropped connection on reset" },
        { WSAECONNABORTED, "Software caused connection abort" },
        { WSAECONNRESET, "Connection reset by peer" },
        { WSAENOBUFS, "No buffer space available" },
        { WSAEISCONN, "Socket is already connected" },
        { WSAENOTCONN, "Socket is not connected" },
        { WSAESHUTDOWN, "Can't send after socket shutdown" },
        { WSAETOOMANYREFS, "Too many references: "
        "can't splice" },
        { WSAETIMEDOUT, "Operation timed out" },
        { WSAECONNREFUSED, "Connection refused" },
        { WSAELOOP, "Too many levels of symbolic links" },
        { WSAENAMETOOLONG, "File name too long" },
        { WSAEHOSTDOWN, "Host is down" },
        { WSAEHOSTUNREACH, "No route to host" },
        { WSAENOTEMPTY, "Directory not empty" },
        { WSAEPROCLIM, "Too many processes" },
        { WSAEUSERS, "Too many users" },
        { WSAEDQUOT, "Disc quota exceeded" },
        { WSAESTALE, "Stale NFS file handle" },
        { WSAEREMOTE, "Too many levels of remote in path" },
        { WSASYSNOTREADY, "Network subsystem is unvailable" },
        { WSAVERNOTSUPPORTED, "WinSock version is not "
        "supported" },
        { WSANOTINITIALISED, "Successful WSAStartup() not yet "
        "performed" },
        { WSAEDISCON, "Graceful shutdown in progress" },
        /* Resolver errors */
        { WSAHOST_NOT_FOUND, "No such host is known" },
        { WSATRY_AGAIN, "Host not found, or server failed" },
        { WSANO_RECOVERY, "Unexpected server error "
        "encountered" },
        { WSANO_DATA, "Valid name without requested data" },
        { WSANO_ADDRESS, "No address, look for MX record" },
        { 0, NULL }
    };

    for (msgp=msgs; msgp->msg; msgp++) {
        if (code==msgp->no) return msgp->msg;
    }
    sprintf(buf, "%d", code);
    return buf;
};
