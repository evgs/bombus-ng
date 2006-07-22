#pragma once
#include <string>
#include <Winsock2.h>
#include <boost/shared_ptr.hpp>

#include "basetypes.h"

class Socket
{
public:
	static Socket * createSocket(const std::string & url, const int port);

public:
	Socket(void);
	~Socket(void);

	int read(char * buf, int len);
	int write(const char * buf, int len);
	int write(const StringRef buf);
	int Socket::write(std::string &buf);

private:
	SOCKET sock;

};

typedef boost::shared_ptr<Socket> SocketRef;
