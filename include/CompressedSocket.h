#pragma once

#include "Connection.h"
#define ZLIB_WINAPI
#include <zlib.h>

class CompressedSocket : public Connection 
{
public:
	CompressedSocket(ConnectionRef pack);
	~CompressedSocket(void);

	virtual int read(char * buf, int len);
	virtual int write(const char * buf, int len);
	virtual const std::string getStatistics();
	//int write(const StringRef buf);
	//int write(std::string &buf);

private:
	ConnectionRef pack;
	char *zinbuf;
	char *zoutbuf;

	z_stream istr;
	z_stream ostr;

};

