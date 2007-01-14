#include "stdafx.h"

#include "JabberAccount.h"

JabberAccount::JabberAccount(){
	password="";

    //useSRV=true;
	hostNameIp="";
	port=5222;

	plainTextPassword=false;
	useSASL=false;
	useEncryption=false;
    legacySSL=false;
	useCompression=false;
	useProxy=false;
	proxy="<none>";
}

JabberAccount::JabberAccount(const std::string &bareJid, const std::string &resource) {
	password="";

    //useSRV=true;
    hostNameIp="";
	port=5222;

	plainTextPassword=false;
	useSASL=false;
	useEncryption=false;
	legacySSL=false;
    useCompression=false;
	useProxy=false;
	setBareJid(bareJid);
	setResource(resource);
}

JabberAccount::~JabberAccount(){}