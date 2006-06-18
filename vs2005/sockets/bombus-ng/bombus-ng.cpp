// bombus-ng.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Socket.h"
#include <string>
#include "JabberDataBlock.h"
#include "JabberStream.h"

int _tmain(int argc, _TCHAR* argv[])
{
	JabberDataBlock login("iq");
	login.setAttribute("type","set");
	login.setAttribute("id","auth");
	JabberDataBlock * qry=login.addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:auth");
	qry->addChild("username","evgs@jabber.ru");
	qry->addChild("password","secret");


	std::string url ("jabber.ru");
	SocketRef s=SocketRef(Socket::createSocket(url, 5222));
	BOOST_ASSERT(s);
	JabberStream jstream(s);
	jstream.sendXmlVersion();
	jstream.sendXmppHeader("jabber.ru");


	//jstream.sendStanza(test);
	//printf("%s", test.toXML());

	char tmp[16];
	gets(tmp);
	jstream.sendStanza(login);
	gets(tmp);
	return 0;
}

