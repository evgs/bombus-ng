#pragma once

#include <string>

class JabberAccount {

public:
	JabberAccount();
	JabberAccount(const std::string &jid, const std::string &resource);

	~JabberAccount();

	const std::string & getUserName(void) {return userName; };
	const std::string & getServer(void) {return server; };
	const std::string & getResource(void) {return resource; };
	const std::string & getJid(void) {return jid; };
	const std::string & getBareJid(void) {return bareJid;};

	void setUserName(const std::string &userName);
	void setServer(const std::string &server);
	void setResource(const std::string &resource);
	void setJid(const std::string &Jid);
	void setBareJid(const std::string &bareJid);

private:
	std::string userName;
	std::string server;
	std::string resource;

	std::string jid;
	std::string bareJid;

public:

	std::string password;

	std::string hostNameIp;
	int port;

	bool plainTextPassword;
	bool useSASL;
	bool useEncryption;
	bool useCompression;
	bool useProxy;
	std::string proxy;

private:
	void updateJid(void);
};