//#include "stdafx.h"

#include "JabberAccount.h"

JabberAccount::JabberAccount(){
    init();
}
void JabberAccount::init(){
    version=1;

	password="";

    //useSRV=true;
	hostNameIp="";
	port=5222;
    setResource("bombus-ng");

	plainTextPassword=false;
	useSASL=true;
	useEncryption=false;
    legacySSL=false;
	useCompression=false;
	useProxy=false;
	proxy="<none>";
}

JabberAccount::JabberAccount(const std::string &bareJid, const std::string &resource) {
    init();
	setBareJid(bareJid);
	setResource(resource);
}

JabberAccount::JabberAccount( LPCTSTR filename ) {
    init();
    Serialize s(filename, true);
    serialize(s, true);
}

JabberAccount::~JabberAccount(){}

void JabberAccount::saveAccount( LPCTSTR fileName ) {
    Serialize s(fileName, false);
    serialize(s, false);
}

void JabberAccount::serialize( Serialize &s, bool read ) 
{
    //////////////////////////////////////////////////////////////////////////
    // Serialization
    int version=this->version;
    s.streamInt(version);

    std::string sjid=getJid();
    s.streamString(sjid);
    s.streamScrambledString(password); 

    // host data
    s.streamString(hostNameIp);
    s.streamInt(port);

    // login settings
    s.streamBool(plainTextPassword);
    s.streamBool(useSASL);
    s.streamBool(useEncryption);
    s.streamBool(legacySSL);
    s.streamBool(useCompression);

    // proxy
    s.streamBool(useProxy);
    s.streamString(proxy);

    setJid(sjid);
}

