//#include "stdafx.h"

#include "JabberAccount.h"

JabberAccount::JabberAccount(){
    init();
}
void JabberAccount::init(){/*deprecated*/}

JabberAccount::JabberAccount(const std::string &bareJid, const std::string &resource) {
    init();
	setBareJid(bareJid);
	setResource(resource);
}

JabberAccount::JabberAccount( LPCTSTR filename ) {
    init();
    Serialize s(filename, Serialize::READ);
    serialize(s);
}

JabberAccount::~JabberAccount(){}

void JabberAccount::saveAccount( LPCTSTR fileName ) {
    Serialize s(fileName, Serialize::WRITE);
    serialize(s);
}

void JabberAccount::serialize( Serialize &s ) 
{
    //////////////////////////////////////////////////////////////////////////
    // Serialization
    int version=3; //for write
    s.streamInt(version, 3);

    setResource("bombus-ng");
    std::string sjid=getJid();
    s.streamString(sjid);
    setJid(sjid);

    s.streamScrambledString(password); 

    // host data
    if (version>=3) s.streamBool(useSRV, true); else useSRV=true;

    s.streamString(hostNameIp);
    s.streamInt(port, 5222);

    // login settings
    s.streamBool(plainTextPassword, false);
    s.streamBool(useSASL, true);
    s.streamBool(useEncryption, false); 
    s.streamBool(legacySSL, false);
    s.streamBool(useCompression, false);

    if (version>=2) s.streamBool(ignoreSslWarnings, false); else ignoreSslWarnings=false;

    // proxy
    s.streamBool(useProxy, false);
    s.streamString(proxy, "<none>");
}

