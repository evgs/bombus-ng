#include "JabberStream.h"
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stack>
#include <utf8.hpp>
#include <windows.h>

JabberStream::JabberStream(void){}

int JabberStream::writeCallback(void * context, const char * buffer, int len) {
	((JabberStream *) context)->connection->write(buffer, len);

	char *output=new char[len+1];
	memcpy(output, buffer, len);
	output[len]=0;
	std::cout << "out: " << output << std::endl;
	delete output;
	return len;
}

int JabberStream::closeCallback(void * context){
	std::cout << "close out\n";
	return 0;
};

int JabberStream::readCallback(void * context, char * buffer, int len) {
	len=((JabberStream *) context)->connection->read(buffer, len);

	if (len<0) {
		std::cout << "in: EOF\n";
		return -1;
	}

	char *input=new char[2*len+1];
	char *oem=new char[len+1];
	memcpy(input, buffer, len);
	input[len]=0;

	std::wstring uni=utf8::utf8_wchar(input);
	WideCharToMultiByte(CP_OEMCP,0, uni.c_str(), -1, oem, len+1, NULL, NULL);

	std::cout << "in: "<< oem << std::endl;
	delete input;
	delete oem;
	return len;
}

int JabberStream::icloseCallback(void * context){
	printf("close in\n");
	return 0;
};

void JabberStream::run(JabberStream * _stream){
	puts("thread strated");
    xmlBufferPtr buf = xmlBufferCreate();
	_stream->reader=xmlReaderForIO( readCallback, icloseCallback, _stream , NULL, NULL, 0);
	puts("startup ok");

	_stream->isRunning=true;

	std::stack<JabberDataBlockRef> stanzaStack;

	int ret=xmlTextReaderRead(_stream->reader);
	while (ret==1) {
		{	// processing node
			xmlTextReaderPtr reader=_stream->reader;
			xmlChar *name=xmlTextReaderName(reader);
			xmlChar *value=xmlTextReaderValue(reader);
			int nodeType=xmlTextReaderNodeType(reader);
			bool empty=xmlTextReaderIsEmptyElement(reader);

			switch (nodeType){
				case XML_READER_TYPE_ELEMENT:
					// stream initiated
					if (!strcmp((char *) name,"stream:stream")) {
						xmlChar *value=xmlTextReaderGetAttribute(reader, BAD_CAST "id");

						_stream->streamId=(char *)value;

						//begin conversation
						JabberListener * listener=_stream->jabberListener.get();
						if (listener!=NULL) listener->beginConversation( (char *)value);

						xmlFree(value);

						break;
					}

					// stanzas
					{
						stanzaStack.push( JabberDataBlockRef( new JabberDataBlock( (char *)name)) );
						while (xmlTextReaderMoveToNextAttribute(reader)) {
							xmlChar *name=xmlTextReaderName(reader);
							xmlChar *value=xmlTextReaderValue(reader);

							stanzaStack.top()->setAttribute( (char *) name, (char *) value );

							xmlFree(value);
							xmlFree(name);
						}
						if (!empty) break;
					}
				case XML_READER_TYPE_END_ELEMENT:
					{
						JabberDataBlockRef element=stanzaStack.top();
						stanzaStack.pop();
						if (stanzaStack.empty()) {
							//todo: block arrived
							JabberStanzaDispatcher * dispatcher=_stream->stanzaDispatcher.get();
							if (dispatcher!=NULL) dispatcher->dispatchDataBlock(element);

							//puts(element->toXML()->c_str());
						} else {
							stanzaStack.top()->addChild(element);
						}
					}
					break;
				case XML_READER_TYPE_TEXT:
					{
						stanzaStack.top()->setText((char *)value);
						break;
					}
				default:
					std::cout 
						<< "##### depth=" << xmlTextReaderDepth(reader) 
						<< " nodetype=" << nodeType 
						<< " name=" << name 
						<< " isempty=" << empty 
						<< " val=" << value << std::endl;

					/*if (nodeType==1) while (xmlTextReaderMoveToNextAttribute(reader)) {
						xmlChar *name=xmlTextReaderName(reader);
						xmlChar *value=xmlTextReaderValue(reader);
						printf( "-- %d %d %s='%s'\n", xmlTextReaderDepth(reader), xmlTextReaderNodeType(reader), name, value);
						xmlFree(value);
						xmlFree(name);
					}*/
			}
			xmlFree(value);
			xmlFree(name);
		}
		ret=xmlTextReaderRead(_stream->reader);
	}
	xmlFreeTextReader(_stream->reader);
}

JabberStream::JabberStream(SocketRef _connection){
	connection=_connection;

    xmlBufferPtr buf = xmlBufferCreate();
	BOOST_ASSERT(buf);

	xmlOutputBufferPtr outBuf=xmlOutputBufferCreateBuffer(buf, NULL);
	outBuf->context=this;
	outBuf->writecallback=(xmlOutputWriteCallback) writeCallback;
	outBuf->closecallback=(xmlOutputCloseCallback) closeCallback;

	writer = xmlNewTextWriter(outBuf);

    BOOST_ASSERT(writer);

	boost::thread test( boost::bind(run, this) );
}

JabberStream::~JabberStream(void){
	printf("Closing...\n");
	xmlTextWriterEndDocument(writer);
	xmlTextWriterFlush(writer);
	xmlFreeTextWriter(writer);
	//outBuf=NULL; //released by xmlFreeTextWriter(writer);
	writer=NULL;
}

void JabberStream::sendStanza(JabberDataBlockRef stanza){
	stanza->constructXML(writer);
	int res=xmlTextWriterFlush(writer);
	BOOST_ASSERT(res>=0);
}
void JabberStream::sendStanza(JabberDataBlock &stanza){
	stanza.constructXML(writer);
	int res=xmlTextWriterFlush(writer);
	BOOST_ASSERT(res>=0);
}

void JabberStream::sendXmlVersion(){
	xmlTextWriterStartDocument(writer, NULL, "utf-8", NULL);
}

void JabberStream::sendXmppHeader(const char *serverName){
	xmlTextWriterStartElement(writer, BAD_CAST "stream:stream");
	xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns:stream", BAD_CAST "http://etherx.jabber.org/streams");
	xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns", BAD_CAST "jabber:client");
	xmlTextWriterWriteAttribute(writer, BAD_CAST "to", BAD_CAST serverName);
	xmlTextWriterWriteString(writer, BAD_CAST "");

	int res=xmlTextWriterFlush(writer);
	BOOST_ASSERT(res>=0);
}

