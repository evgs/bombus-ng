//#include "stdafx.h"

#include "JabberAccount.h"
#include "JabberStream.h"
//#include <boost/thread.hpp>
//#include <boost/bind.hpp>
#include <stack>
#include <exception>

#include <utf8.hpp>
#include <windows.h>

JabberStream::JabberStream(void){}

void JabberStream::run(JabberStream * _stream){
	Log::getInstance()->msg("Reader thread strated");

	_stream->isRunning=true;

	try {
        if (_stream->connection==NULL) {
            _stream->jabberListener->connect();
        }
        _stream->parser->bindStream( _stream->connection );
		_stream->parser-> parse();
        _stream->jabberListener->endConversation(NULL);
	} catch (std::exception ex) {
        _stream->jabberListener->endConversation(&ex);
	}
    Log::getInstance()->msg("Reader thread stopped");
    _stream->rc->jabberStream=JabberStreamRef();
    _stream->isRunning=false;
}

void JabberStream::tagStart(const std::string & tagname, const StringMap &attr) {

	if (tagname=="xml") return;
	
    JabberDataBlockRef blk=JabberDataBlockRef( new JabberDataBlock( tagname, attr ) );

	if (tagname=="stream:stream") {

		//begin conversation
		JabberListener * listener=jabberListener.get();
		if (listener!=NULL) listener->beginConversation( blk );
		
		return;
	}

	// stanzas
	stanzaStack.push( blk);
}

bool JabberStream::tagEnd(const std::string & tagname) {
    if (stanzaStack.empty()) {
        return (tagname=="stream:stream");
    }
	JabberDataBlockRef element=stanzaStack.top();
	stanzaStack.pop();
	if (stanzaStack.empty()) {
		//todo: block arrived
        
        if (element->getTagName()!=tagname) {
            throw std::exception("XML: Tag mismatch");
        }

		JabberStanzaDispatcher * dispatcher= rc->jabberStanzaDispatcherRT.get();
		if (dispatcher!=NULL) dispatcher->dispatchDataBlock(element);

		//puts(element->toXML()->c_str());
	} else {
		stanzaStack.top()->addChild(element);
	}
    return false;
}

void JabberStream::plainTextEncountered(const std::string & body){
	stanzaStack.top()->_setText(body);
}

#ifdef _WIN32_WCE
DWORD jabberStreamThread(LPVOID param) {
#else
DWORD WINAPI jabberStreamThread(LPVOID param) {
#endif
	JabberStream::run((JabberStream *)param);
	return 1;
}

JabberStream::JabberStream(ResourceContextRef rc, JabberListenerRef listener){

	parser=XMLParserRef(new XMLParser(this));

	this->rc=rc;
    this->jabberListener=listener;

	HANDLE thread=CreateThread(NULL, 0, jabberStreamThread, this, 0, NULL);
    SetThreadPriority(thread, THREAD_PRIORITY_IDLE);
	//boost::thread test( boost::bind(run, this) );
}

JabberStream::~JabberStream(void){

	Log::getInstance()->msg("JabberStream destructor called");
}

void JabberStream::sendStanza(JabberDataBlockRef stanza){
    try {
	    connection->write( stanza->toXML() );
    } catch (std::exception ex) {
        jabberListener->endConversation(&ex);
        connection->close();
    }
}

void JabberStream::sendStanza(JabberDataBlock &stanza){
    try {
	    connection->write( stanza.toXML() );
    } catch (std::exception ex) {
        jabberListener->endConversation(&ex);
        connection->close();
    }
}

void JabberStream::sendXmlVersion(void){
	connection->write("<?xml version='1.0'?>", 21);
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

	connection->write(header);
}

void JabberStream::sendXmppEndHeader(void){
	connection->write(std::string("</stream:stream>"));
}