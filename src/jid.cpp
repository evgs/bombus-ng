#include "jid.h"
//#include <boost/algorithm/string.hpp>
#include "lstring.h"

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
    const char *pJid=jid.c_str();
    if (slash<0) {
        slash=jid.length();
        resource="";
    } else resource.assign(pJid+slash+1);
	server.assign(pJid+at+1,slash-at-1);
    if (at==-1) at=0;
    userName.assign(pJid,at);

	updateJid();
};
void Jid::setBareJid(const std::string &bareJid){
	this->bareJid=bareJid;

	int at=bareJid.find('@');
	int slash=bareJid.find('/');
    if (slash<0) slash=bareJid.length();

    const char *pBareJid=bareJid.c_str();
    server.assign(pBareJid+at+1,slash-at-1);

    if (at==-1) at=0;
    userName.assign(pBareJid,at);

	updateJid();
};

void Jid::updateJid(void){
    locale::toLowerCase(userName);
    locale::toLowerCase(server);

	bareJid=userName;
    if (userName.length()!=0) bareJid+='@';
    bareJid+=server;

	if (resource.empty()) jid=bareJid; 
	else jid=bareJid+'/'+resource;
};

bool Jid::operator==(const Jid &right) const {
    return this->jid==right.jid;
};

bool Jid::isValid() {
    if (getUserName().length()==0) return false;
    if (getServer().length()==0) return false;
    return true;
}