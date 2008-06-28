#pragma once
#include <Winsock2.h>
#include "Connection.h"

class Socket : public Connection 
{
public:
	Socket(const long ip, const int port);
	virtual ~Socket(void);

	virtual int read(char * buf, int len);
	virtual int write(const char * buf, int len);
	virtual const std::string getStatistics();
	//int write(const StringRef buf);
	//int write(std::string &buf);

    virtual void close();

    SOCKET getSocket(){ return sock; }

    //todo: move to another namespace
    static void networkUp();
    static void checkNetworkUp();
    //todo: move to another namespace
    static void initWinsocks();
    //todo: move to another namespace
    static long resolveUrl(const std::string &url);

protected:
    Socket(){};
    SOCKET sock;


    static void throwSocketError();
    static void throwNetworkDown(DWORD status);

    long bytesSent;
    long bytesRecvd;

private:
    static HANDLE hconn;
};

