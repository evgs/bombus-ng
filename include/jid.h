#pragma once

#include <string>

class Jid {

public:
	Jid();
	Jid(const std::string &bareJid);
	Jid(const std::string &jid, const std::string &resource);

	~Jid();

	const std::string & getUserName(void) const {return userName; };
	const std::string & getServer(void) const {return server; };
	const std::string & getResource(void) const {return resource; };
	const std::string & getJid(void) const {return jid; };
	const std::string & getBareJid(void) const {return bareJid;};

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

private:
	void updateJid(void);
};