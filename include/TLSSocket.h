#pragma once

#include "Connection.h"

namespace yaSSL {
    class Client;
}

class TLSSocket : public Connection 
{
public:
	TLSSocket(ConnectionRef sock);
	~TLSSocket(void);

	virtual int read(char * buf, int len);
	virtual int write(const char * buf, int len);
	virtual const std::string getStatistics();
	//int write(const StringRef buf);
	//int write(std::string &buf);

private:
	ConnectionRef sock;

    yaSSL::Client * sslClient;
};

