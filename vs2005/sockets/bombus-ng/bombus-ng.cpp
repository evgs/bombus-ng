// bombus-ng.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Socket.h"
#include <string>
#include "JabberDataBlock.h"
#include "JabberStream.h"
#include "JabberAccount.h"
#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

#include "Auth.h"

//////////////////////////////////////////////////////////////
class Online : public JabberDataBlockListener {
public:
	Online(ResourceContextRef rc) {
		this->rc=rc;
	}
	~Online(){};
	virtual const char * getType() const{ return "result"; }
	virtual const char * getId() const{ return "auth"; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult Online::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
	puts("Login ok");
	JabberDataBlock presence("presence");
	presence.addChild("status", 
		"please, don't send any messages here! \n"
		"they will be dropped because it is debug version" );
	rc->jabberStream->sendStanza(presence);
	return LAST_BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class Version : public JabberDataBlockListener {
public:
	Version(ResourceContextRef rc) {
		this->rc=rc;
	}
	~Version(){};
	virtual const char * getType() const{ return "get"; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult Version::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

	std::cout << "version request " << block->getAttribute("from") << std::endl;

	JabberDataBlock reply("iq");
	reply.setAttribute("type","result");
	reply.setAttribute("id", block->getAttribute("id"));
	reply.setAttribute("to", block->getAttribute("from"));

	JabberDataBlock * qry=reply.addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:version");
	qry->addChild("name","Bombus-ng");
	qry->addChild("version","0.0.1-devel");
	qry->addChild("os","Windows 2000");

	rc->jabberStream->sendStanza(reply);
	return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class MessageFwd : public JabberDataBlockListener {
public:
	MessageFwd(ResourceContextRef rc) {
		this->rc=rc;
	}
	~MessageFwd(){};
	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "message"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult MessageFwd::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
	std::cout << "Message from " << block->getAttribute("from") << std::endl;
	JabberDataBlock reply("message");
	reply.setAttribute("type","chat");
	reply.setAttribute("to", "evgs@jabber.ru/Psi_Home");
	reply.addChild("body", NULL)->setText(XMLStringPrep( *(block->toXML()) ));

	rc->jabberStream->sendStanza(reply);
	return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	ResourceContextRef rc=ResourceContextRef(new ResourceContext());
	rc->account=JabberAccountRef(new JabberAccount("evgs@jabber.ru", "bombus-ng"));
	rc->account->hostNameIp="213.180.203.19";
	rc->account->password=
#include "password"
	;
	rc->account->useSASL=true;

	std::string host=(rc->account->hostNameIp.empty())?rc->account->getServer() : rc->account->hostNameIp;

	rc->connection=ConnectionRef(Socket::createSocket(host, 5222));
	BOOST_ASSERT(rc->connection);

	rc->jabberStream=JabberStreamRef(new JabberStream(rc));

	JabberStanzaDispatcherRef disp= JabberStanzaDispatcherRef(new JabberStanzaDispatcher(rc));
	rc->jabberStanzaDispatcher=disp;

	if (rc->account->useSASL) {
		rc->jabberStream->setJabberListener( JabberListenerRef(new SASLAuth( rc )));
		disp->addListener( JabberDataBlockListenerRef( new SASLAuth(rc) ));
	} else {
		rc->jabberStream->setJabberListener( JabberListenerRef(new NonSASLAuth( rc )));
	}


	disp->addListener( JabberDataBlockListenerRef( new Online(rc) ));
	disp->addListener( JabberDataBlockListenerRef( new Version(rc) ));
	disp->addListener( JabberDataBlockListenerRef( new MessageFwd(rc) ));

	rc->jabberStream->sendXmlVersion();
	rc->jabberStream->sendXmppBeginHeader();


	//jstream.sendStanza(test);
	//printf("%s", test.toXML());

	char tmp[16];
	gets_s(tmp,16);
	//jstream.sendStanza(login);
	rc->jabberStream->sendXmppEndHeader();

	gets_s(tmp,16);

	std::cout << rc->connection->getStatistics().c_str() << std::endl;

	gets_s(tmp,16);

	return 0;
}

