#pragma once
#include <Winsock2.h>
#include "Connection.h"

class Socket : public Connection 
{
public:
	Socket(const std::string & url, const int port);
	~Socket(void);

	virtual int read(char * buf, int len);
	virtual int write(const char * buf, int len);
	const std::string getStatistics();
	//int write(const StringRef buf);
	//int write(std::string &buf);

    virtual void close();

    SOCKET getSocket(){ return sock; }

protected:
    Socket(){};
    std::string url;
    SOCKET sock;

    long resolveUrl();

    static void initWinsocks();

    void throwSocketError();

private:
	long bytesSent;
	long bytesRecvd;
};

