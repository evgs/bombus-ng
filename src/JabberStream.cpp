#include "JabberStream.h"
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stack>
#include <utf8.hpp>
#include <windows.h>

JabberStream::JabberStream(void){}

void JabberStream::run(JabberStream * _stream){
	puts("thread strated");

	_stream->isRunning=true;

	_stream->parser->bindStream( _stream->connection );
	try {
		_stream->parser-> parse();
	} catch (std::exception ex) {
		_stream->jabberListener->endConversation();
	}

}

void JabberStream::tagStart(const std::string & tagname, const std::map<std::string, std::string> &attr) {

	if (tagname=="xml") return;
	
	if (tagname=="stream:stream") {
		//non-sasl auth
		//TODO: непонятное поведение компилятора.
		std::map<std::string, std::string> attr2=attr;
		streamId=attr2[std::string("id")];
		//streamId=attr[std::string("id")];

		//begin conversation
		JabberListener * listener=jabberListener.get();
		if (listener!=NULL) listener->beginConversation( streamId );
		
		return;
	}

	// stanzas
	stanzaStack.push( JabberDataBlockRef( new JabberDataBlock( tagname, attr ) ));
}

void JabberStream::tagEnd(const std::string & tagname) {
	JabberDataBlockRef element=stanzaStack.top();
	stanzaStack.pop();
	if (stanzaStack.empty()) {
		//todo: block arrived
		JabberStanzaDispatcher * dispatcher= stanzaDispatcher.get();
		if (dispatcher!=NULL) dispatcher->dispatchDataBlock(element);

		//puts(element->toXML()->c_str());
	} else {
		stanzaStack.top()->addChild(element);
	}
}

void JabberStream::plainTextEncountered(const std::string & body){
	stanzaStack.top()->setText(body);
}

JabberStream::JabberStream(SocketRef _connection){

	parser=XMLParserRef(new XMLParser(this));

	connection=_connection;

	boost::thread test( boost::bind(run, this) );
}

JabberStream::~JabberStream(void){
	printf("JabberStream destructor called \n");
}

void JabberStream::sendStanza(JabberDataBlockRef stanza){
	connection->write( stanza->toXML() );
}

void JabberStream::sendStanza(JabberDataBlock &stanza){
	connection->write( stanza.toXML() );
}

void JabberStream::sendXmlVersion(){
	connection->write("<?xml version='1.0'?>", 21);
}

void JabberStream::sendXmppHeader(const char *serverName){
	std::string header=
		"<stream:stream "
		"xmlns:stream='http://etherx.jabber.org/streams' "
		"xmlns='jabber:client' "
		"to='";
	header+=serverName;
	header+="' >";

	connection->write(header);
}

