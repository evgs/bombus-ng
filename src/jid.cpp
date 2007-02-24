#include "jid.h"
//#include <boost/algorithm/string.hpp>

Jid::Jid() {
	userName=server=resource=jid=bareJid="";
}
Jid::~Jid() {}

Jid::Jid(const std::string &bareJid, const std::string &resource){
	this->resource=resource;
	setBareJid(bareJid);
}
Jid::Jid(const std::string &jid){
	//resource="";
	setJid(jid);
}
void Jid::setUserName(const std::string &userName) {
	this->userName=userName;
	updateJid();
};
void Jid::setServer(const std::string &server){
	this->server=server;
	updateJid();
};
void Jid::setResource(const std::string &resource){
	this->resource=resource;
	updateJid();
};
void Jid::setJid(const std::string &jid){
	this->jid=jid;

	int at=jid.find('@');
	int slash=jid.find('/');
    if (slash<0) {
        slash=jid.length();
        resource="";
    } else resource=jid.substr(slash+1);
	server=jid.substr(at+1,slash-at-1);
    if (at==-1) at=0;
    userName=jid.substr(0,at);

	updateJid();
};
void Jid::setBareJid(const std::string &bareJid){
	this->bareJid=bareJid;

	int at=bareJid.find('@');
	int slash=bareJid.find('/');
	server=bareJid.substr(at+1,slash-at-1);
    if (at==-1) at=0;
    userName=bareJid.substr(0,at);

	updateJid();
};

void Jid::updateJid(void){
	bareJid=userName;
    if (userName.length()!=0) bareJid+='@';
    bareJid+=server;

	if (resource.empty()) jid=bareJid; 
	else jid=bareJid+'/'+resource;
};

bool Jid::operator==(const Jid &right) const {
    return this->jid==right.jid;
};