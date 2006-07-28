#pragma once
#include <string>
#include <boost/shared_ptr.hpp>
#include "basetypes.h"

class Connection
{
public:
	virtual int read(char * buf, int len)=0;
	virtual int write(const char * buf, int len)=0;

	virtual const std::string getStatistics()=0;

	virtual int write(const StringRef buf);
	virtual int write(std::string &buf);
};

typedef boost::shared_ptr<Connection> ConnectionRef;