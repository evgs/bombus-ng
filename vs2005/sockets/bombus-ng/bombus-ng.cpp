// bombus-ng.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Socket.h"
#include <string>
#include "JabberDataBlock.h"
#include "JabberStream.h"
#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

//////////////////////////////////////////////////////////////
class Login : public JabberListener {
public:
	Login(JabberStream * stream) {
		_stream=stream;
	}
	~Login(){};
	virtual void beginConversation(const std::string & streamId);
	virtual void endConversation();


private:
	JabberStream * _stream;
};
void Login::beginConversation(const std::string & streamId) {
	puts ("begin conversation");

	JabberDataBlockRef login=JabberDataBlockRef(new JabberDataBlock("iq"));
	login->setAttribute("type","set");
	login->setAttribute("id","auth");
	JabberDataBlock * qry=login->addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:auth");
	qry->addChild("username","evgs");
	qry->addChild("password",
		//"secret"
#include "password"
		);
	qry->addChild("resource","bombus-ng");

	_stream->sendStanza(login);
}
void Login::endConversation(){};
//////////////////////////////////////////////////////////////
class Online : public JabberDataBlockListener {
public:
	Online(JabberStream * stream) {
		_stream=stream;
	}
	~Online(){};
	virtual const char * getType() const{ return "result"; }
	virtual const char * getId() const{ return "auth"; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContext * rc);
private:
	JabberStream * _stream;
};
ProcessResult Online::blockArrived(JabberDataBlockRef block, const ResourceContext * rc){
	puts("Login ok");
	JabberDataBlock presence("presence");
	_stream->sendStanza(presence);
	return LAST_BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class Version : public JabberDataBlockListener {
public:
	Version(JabberStream * stream) {
		_stream=stream;
	}
	~Version(){};
	virtual const char * getType() const{ return "get"; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContext * rc);
private:
	JabberStream * _stream;
};
ProcessResult Version::blockArrived(JabberDataBlockRef block, const ResourceContext * rc){
	puts("version request");
	JabberDataBlock reply("iq");
	reply.setAttribute("type","result");
	reply.setAttribute("id", block->getAttribute("id"));
	reply.setAttribute("to", block->getAttribute("from"));

	JabberDataBlock * qry=reply.addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:version");
	qry->addChild("name","Bombus-ng");
	qry->addChild("version","0.0.1");
	qry->addChild("os","Windows 2000");

	_stream->sendStanza(reply);
	return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////

int _tmain(int argc, _TCHAR* argv[])
{
	ResourceContext rc;

	std::string url ("jabber.ru");
	SocketRef s=SocketRef(Socket::createSocket(url, 5222));
	BOOST_ASSERT(s);

	JabberStreamRef jstream=JabberStreamRef(new JabberStream(s));
	rc.jabberStream=jstream;
	
	jstream->setJabberListener( JabberListenerRef(new Login( jstream.get() )));

	JabberStanzaDispatcherRef disp= JabberStanzaDispatcherRef(new JabberStanzaDispatcher(&rc));
	jstream->setJabberStanzaDispatcher(disp);
	disp->addListener( JabberDataBlockListenerRef( new Online(jstream.get() )));
	disp->addListener( JabberDataBlockListenerRef( new Version(jstream.get() )));

	jstream->sendXmlVersion();
 
	jstream->sendXmppHeader("jabber.ru");


	//jstream.sendStanza(test);
	//printf("%s", test.toXML());

	char tmp[16];
	gets_s(tmp,16);
	//jstream.sendStanza(login);
	gets_s(tmp,16);
	return 0;
}

