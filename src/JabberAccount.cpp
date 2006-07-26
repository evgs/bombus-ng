#include "JabberAccount.h"

void JabberAccount::setUserName(const std::string &userName) {
	this->userName=userName;
	updateJid();
};
void JabberAccount::setServer(const std::string &server){
	this->server=server;
	updateJid();
};
void JabberAccount::setResource(const std::string &resource){
	this->resource=resource;
	updateJid();
};
void JabberAccount::setJid(const std::string &jid){
	this->jid=jid;

	int at=jid.find('@');
	int slash=jid.find('/');
	userName=jid.substr(0,at);
	server=jid.substr(at+1,slash);
	resource=jid.substr(slash+1);

	updateJid();
};
void JabberAccount::setBareJid(const std::string &bareJid){
	this->bareJid=bareJid;

	int at=bareJid.find('@');
	int slash=bareJid.find('/');
	userName=bareJid.substr(0,at);
	server=bareJid.substr(at+1,slash);

	updateJid();
};

void JabberAccount::updateJid(void){
	bareJid=userName+'@'+server;
	if (resource.empty()) bareJid=jid; 
	else jid=bareJid+'/'+resource;
};

JabberAccount::JabberAccount(){
	userName=server=password=resource=jid=bareJid="";
	hostNameIp="";
	port=5222;

	plainTextPassword=false;
	useSASL=false;
	useEncryption=false;
	useCompression=false;
	useProxy=false;
	proxy="<none>";
}

JabberAccount::JabberAccount(const std::string &bareJid, const std::string &resource) {
	password="";
	hostNameIp="";
	port=5222;

	plainTextPassword=false;
	useSASL=false;
	useEncryption=false;
	useCompression=false;
	useProxy=false;
	setBareJid(bareJid);
	setResource(resource);
}

JabberAccount::~JabberAccount(){}