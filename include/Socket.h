#pragma once
#include <string>
#include <Winsock2.h>
#include <boost/shared_ptr.hpp>

class Socket
{
public:
	static Socket * createSocket(const std::string & url, const int port);

public:
	Socket(void);
	~Socket(void);

	int read(char * buf, int len);
	int write(const char * buf, int len);
	int write(const std::string &buf);

private:
	SOCKET sock;

};

typedef boost::shared_ptr<Socket> SocketRef;
