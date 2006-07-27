#pragma once
#include <Winsock2.h>
#include "Connection.h"

class Socket : public Connection 
{
public:
	static Socket * createSocket(const std::string & url, const int port);

public:
	Socket(void);
	~Socket(void);

	virtual int read(char * buf, int len);
	virtual int write(const char * buf, int len);
	const std::string getStatistics();
	//int write(const StringRef buf);
	//int write(std::string &buf);

private:
	SOCKET sock;
	long bytesSent;
	long bytesRecvd;
};

