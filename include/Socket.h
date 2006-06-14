#pragma once
#include <string>
#include <Winsock2.h>

class Socket
{
public:
	static Socket * createSocket(const std::string & url, const int port);

public:
	Socket(void);
	~Socket(void);

private:
	SOCKET sock;

};
