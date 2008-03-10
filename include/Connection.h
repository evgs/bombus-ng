#pragma once
#include <string>
#include "boostheaders.h"
#include "basetypes.h"

class Connection
{
public:
    virtual ~Connection(void){}

	virtual int read(char * buf, int len)=0;
	virtual int write(const char * buf, int len)=0;

	virtual const std::string getStatistics()=0;

    virtual void close()=0;

	virtual int write(const StringRef buf);
	virtual int write(std::string &buf);
};

typedef boost::shared_ptr<Connection> ConnectionRef;
