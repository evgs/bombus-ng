//#include "stdafx.h"


/*
 *  Started on 14.06.2006
 *
 *
 */

#include <Socket.h>

#ifdef WINCE
#include <connmgr.h>
#endif

#include "boostheaders.h"
#include <memory.h>

static int wsCount=0;


Socket::~Socket(void){
	close();
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

Socket::Socket(const long addr, const int port) {
    bytesSent=bytesRecvd=0;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock==INVALID_SOCKET) throwSocketError();

	struct sockaddr_in name;
	name.sin_family=AF_INET;
	name.sin_addr.S_un.S_addr=addr;
	name.sin_port= htons(port); // internet byte order

    //bool keepAlive=true;
    //int optres=setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (const char *)&keepAlive, 1);
    //if (optres==SOCKET_ERROR) throwSocketError();

	int result=connect(sock, (sockaddr*)(&name), sizeof(name));

	if (result==SOCKET_ERROR) throwSocketError();

}

int Socket::read(char * buf, int len) {
    checkNetworkUp();

    int sel=0;

    //do {
        timeval t={0, 200};
        fd_set fds;
        fds.fd_count=1;
        fds.fd_array[0]=sock;

        sel=select(0, &fds, NULL, NULL, &t);
        if (sel==SOCKET_ERROR) throwSocketError();
    //} while (!sel);
    if (sel==0) return 0;

	int rb=recv(sock, buf, len, 0);
    if (rb==SOCKET_ERROR) throwSocketError();
	bytesRecvd+=rb;

    if (rb<=0) throw std::exception("Socket closed");
	return rb;
}

int Socket::write(const char * buf, int len) {
	//for (int i=0; i<len; i++)	std::cout << buf[i];
	//std::cout<<"("<<len<<")"<<std::endl;
	int sb=send(sock, buf, len, 0);
    if (sb==SOCKET_ERROR) throwSocketError();
	bytesSent+=sb;
	return sb;
}

const std::string Socket::getStatistics(){

    return boost::str( boost::format (
        "--- Socket ---\n"
        "sent=%d\n"
        "recv=%d\n")
        % bytesSent % bytesRecvd 
    );
}

const char * errorWSAText(int code);

void Socket::throwSocketError() {
    int lastError=WSAGetLastError();

    boost::format err("Socket error: %s");
    err % errorWSAText(lastError);// % url;
    throw std::exception(err.str().c_str());
}

long Socket::resolveUrl(const std::string &url) {
    long inaddr=inet_addr(url.c_str());
    if (inaddr!=INADDR_NONE) return inaddr;

    struct hostent FAR* host=gethostbyname(url.c_str());
    if (host!=NULL) return  *((unsigned long *)host->h_addr_list[0]);
    throwSocketError();
    return NULL;
}

void Socket::close() {
    closesocket(sock);
}

void Socket::networkUp() {
#ifdef WINCE
    CONNMGR_CONNECTIONINFO rq;
    memset(&rq, 0, sizeof(rq));
    rq.cbSize=sizeof(rq);
    rq.dwPriority=CONNMGR_PRIORITY_HIPRIBKGND;
    rq.dwParams=CONNMGR_PARAM_GUIDDESTNET;

    ConnMgrMapURL(L"http://bombus-im.org", &rq.guidDestNet, NULL);


    DWORD status;
    if (ConnMgrEstablishConnectionSync(&rq, &hconn, 60000, &status) != S_OK) 
        throwNetworkDown(status);

#endif
}

void Socket::checkNetworkUp() {
    if (hconn==INVALID_HANDLE_VALUE) return;
#ifdef WINCE
    DWORD result;
    ConnMgrConnectionStatus(hconn, &result);
    switch (result) {
        case CONNMGR_STATUS_CONNECTED: return;

        case CONNMGR_STATUS_DISCONNECTED:
        case CONNMGR_STATUS_WAITINGFORPATH:
        case CONNMGR_STATUS_WAITINGFORRESOURCE:
        case CONNMGR_STATUS_WAITINGFORNETWORK:
        case CONNMGR_STATUS_WAITINGFORPHONE:
        case CONNMGR_STATUS_NOPATHTODESTINATION:
        case CONNMGR_STATUS_CONNECTIONFAILED:
        case CONNMGR_STATUS_CONNECTIONCANCELED:
        case CONNMGR_STATUS_CONNECTIONDISABLED:

            throwNetworkDown(result);

        default: return;
    }
#endif
}

const char * errorConnMgr(int code);

void Socket::throwNetworkDown(DWORD status) {
    boost::format err("Network is down: %s");
    err % errorConnMgr(status);// % url;
    throw std::exception(err.str().c_str());
}

const char * errorWSAText(int code) {
    static char buf[10];
    static struct { int no; const char *msg; } *msgp, msgs[] = {
        { 10, "SSL Certificate Issuer unknown" },
        { WSAEINTR, "Interrupted system call" },
        { WSAEBADF, "Bad file descriptor" },
        { WSAEACCES, "Permission denied" },
        { WSAEFAULT, "Bad address" },
        { WSAEINVAL, "Invalid argument" },
        { WSAEMFILE, "Too many open files" },
        { WSAEWOULDBLOCK, "Another winsock call while a blocking function was in progress" },
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
        { WSAETOOMANYREFS, "Too many references: can't splice" },
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
        { WSAVERNOTSUPPORTED, "WinSock version is not supported" },
        { WSANOTINITIALISED, "Successful WSAStartup() not yet performed" },
        { WSAEDISCON, "Graceful shutdown in progress" },
        /* Resolver errors */
        { WSAHOST_NOT_FOUND, "No such host is known" },
        { WSATRY_AGAIN, "Host not found, or server failed" },
        { WSANO_RECOVERY, "Unexpected server error encountered" },
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

const char * errorConnMgr(int code) {
    static char buf[10];
    static struct { int no; const char *msg; } *msgp, msgs[] = {
        { CONNMGR_STATUS_UNKNOWN, "Unknown connection status" },
        { CONNMGR_STATUS_CONNECTED, "Connected" },
        { CONNMGR_STATUS_DISCONNECTED, "Disconnected" },
        { CONNMGR_STATUS_WAITINGFORPATH, "Network temporary unreachable" },
        { CONNMGR_STATUS_WAITINGFORRESOURCE, "Resource limit" },
        { CONNMGR_STATUS_WAITINGFORPHONE, "Voice call" },
        { CONNMGR_STATUS_WAITINGFORNETWORK, "Network is in use" },
        { CONNMGR_STATUS_NOPATHTODESTINATION, "No path for destination" },
        { CONNMGR_STATUS_CONNECTIONFAILED, "Connection failed" },
        { CONNMGR_STATUS_CONNECTIONCANCELED, "Connection canceled" },
        { CONNMGR_STATUS_CONNECTIONDISABLED, "Connection disabled" },
        { CONNMGR_STATUS_WAITINGCONNECTION, "Connecting..." },
        { CONNMGR_STATUS_WAITINGCONNECTION, "Connecting..." },
        { CONNMGR_STATUS_WAITINGCONNECTIONABORT, "Cancelling connection..." },
        { CONNMGR_STATUS_WAITINGDISCONNECTION, "Disconnecting..." },
        { 0, NULL }
    };

    for (msgp=msgs; msgp->msg; msgp++) {
        if (code==msgp->no) return msgp->msg;
    }
    sprintf(buf, "%d", code);
    return buf;
};


HANDLE Socket::hconn=INVALID_HANDLE_VALUE;