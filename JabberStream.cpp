#include "stdafx.h"

#include "JabberAccount.h"
#include "JabberStream.h"
//#include <boost/thread.hpp>
//#include <boost/bind.hpp>
#include <stack>
#include <utf8.hpp>
#include <windows.h>

JabberStream::JabberStream(void){}

void JabberStream::run(JabberStream * _stream){
	_stream->rc->log->msg("Reader thread strated");

	_stream->isRunning=true;

	_stream->parser->bindStream( _stream->rc->connection );
	try {
		_stream->parser-> parse();
	} catch (std::exception ex) {
		_stream->jabberListener->endConversation();
	}

}

void JabberStream::tagStart(const std::string & tagname, const StringMap &attr) {

	if (tagname=="xml") return;
	
	if (tagname=="stream:stream") {
		//non-sasl auth
		StringMap::const_iterator i=attr.find("id");
		streamId=i->second;

		//begin conversation
		JabberListener * listener=jabberListener.get();
		if (listener!=NULL) listener->beginConversation( streamId );
		
		return;
	}

	// stanzas
	stanzaStack.push( JabberDataBlockRef( new JabberDataBlock( tagname, attr ) ));
}

void JabberStream::tagEnd(const std::string & tagname) {
	if (stanzaStack.empty()) return;
	JabberDataBlockRef element=stanzaStack.top();
	stanzaStack.pop();
	if (stanzaStack.empty()) {
		//todo: block arrived
		JabberStanzaDispatcher * dispatcher= rc->jabberStanzaDispatcher.get();
		if (dispatcher!=NULL) dispatcher->dispatchDataBlock(element);

		//puts(element->toXML()->c_str());
	} else {
		stanzaStack.top()->addChild(element);
	}
}

void JabberStream::plainTextEncountered(const std::string & body){
	stanzaStack.top()->setText(body);
}

#ifdef _WIN32_WCE
DWORD jabberStreamThread(LPVOID param) {
#else
DWORD WINAPI jabberStreamThread(LPVOID param) {
#endif
	JabberStream::run((JabberStream *)param);
	return 1;
}

JabberStream::JabberStream(ResourceContextRef rc){

	parser=XMLParserRef(new XMLParser(this));

	this->rc=rc;

	CreateThread(NULL, 0, jabberStreamThread, this, 0, NULL);
	//boost::thread test( boost::bind(run, this) );
}

JabberStream::~JabberStream(void){

	printf("JabberStream destructor called \n");
}

void JabberStream::sendStanza(JabberDataBlockRef stanza){
	rc->connection->write( stanza->toXML() );
}

void JabberStream::sendStanza(JabberDataBlock &stanza){
	rc->connection->write( stanza.toXML() );
}

void JabberStream::sendXmlVersion(void){
	rc->connection->write("<?xml version='1.0'?>", 21);
}

void JabberStream::sendXmppBeginHeader(){
	std::string header=
		"<stream:stream "
		"xmlns:stream='http://etherx.jabber.org/streams' "
		"xmlns='jabber:client' "
		"to='";
	header+=rc->account->getServer();
	header+='\'' ;
	if (rc->account->useSASL) header+=" version='1.0'";
	header+=" >";

	rc->connection->write(header);
}

void JabberStream::sendXmppEndHeader(void){
	rc->connection->write(std::string("</stream:stream>"));
}