#include "StdAfx.h"
#include "config.h"

static Config * instance=0;

Config * Config::getInstance() {
	if (!instance) {
		instance=new Config();
	}
	return instance;
}

Config::Config(void)
{
}

Config::~Config(void)
{
}
