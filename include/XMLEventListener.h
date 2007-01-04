#pragma once
#include <string>
#include <map>

class XMLEventListener {
public:
	virtual void tagStart(const std::string & tagname, const std::map<std::string, std::string> &attr)=0;
	virtual bool tagEnd(const std::string & tagname)=0;
	virtual void plainTextEncountered(const std::string & body)=0;
};
