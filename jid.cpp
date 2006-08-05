#include "jid.h"

Jid::Jid() {
	userName=server=resource=jid=bareJid="";
}
Jid::~Jid() {}

Jid::Jid(const std::string &bareJid, const std::string &resource){
	this->resource=resource;
	setBareJid(bareJid);
}
Jid::Jid(const std::string &bareJid){
	//resource="";
	setBareJid(bareJid);
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
	userName=jid.substr(0,at);
	server=jid.substr(at+1,slash);
	resource=jid.substr(slash+1);

	updateJid();
};
void Jid::setBareJid(const std::string &bareJid){
	this->bareJid=bareJid;

	int at=bareJid.find('@');
	int slash=bareJid.find('/');
	userName=bareJid.substr(0,at);
	server=bareJid.substr(at+1,slash);

	updateJid();
};

void Jid::updateJid(void){
	bareJid=userName+'@'+server;
	if (resource.empty()) bareJid=jid; 
	else jid=bareJid+'/'+resource;
};
