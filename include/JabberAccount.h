#pragma once

#include <string>
#include "jid.h"
#include "Serialize.h"

class JabberAccount {

public:
	JabberAccount();
	JabberAccount(const std::string &jid, const std::string &resource);
    JabberAccount(LPCTSTR filename);

    virtual ~JabberAccount();

	const std::string & getUserName(void) const {return jid.getUserName(); };
	const std::string & getServer(void) const {return jid.getServer(); };
	const std::string & getResource(void) const {return jid.getResource(); };
	const std::string & getJid(void) const {return jid.getJid(); };
	const std::string & getBareJid(void) const {return jid.getBareJid(); };

    const std::string & getNickname(void) const {return jid.getUserName(); /*stub*/}

	void setUserName(const std::string &userName) { jid.setUserName(userName); };
	void setServer(const std::string &server){ jid.setServer(server); };
	void setResource(const std::string &resource){ jid.setResource(resource); };
	void setJid(const std::string &jid){ this->jid.setJid(jid); };
	void setBareJid(const std::string &bareJid){ jid.setBareJid(bareJid); };

    void saveAccount(LPCTSTR fileName);

    void serialize(Serialize &s);

private:
	Jid jid;
public:

	std::string password;

    bool useSRV;
	std::string hostNameIp;
	int port;

	bool plainTextPassword;
	bool useSASL;
	bool useEncryption;
    bool ignoreSslWarnings;
    bool legacySSL;
	bool useCompression;
	bool useProxy;
	std::string proxy;

private:
    void init();
};