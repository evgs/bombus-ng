#include "JabberStream.h"
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <stack>

JabberStream::JabberStream(void){}

int writeCallback(void * context, const char * buffer, int len) {
	((JabberStream *) context)->connection->write(buffer, len);

	char *output=new char[len+1];
	memcpy(output, buffer, len);
	output[len]=0;
	printf("out: %s\n", output);
	delete output;
	return len;
}

int closeCallback(void * context){
	printf("close out\n");
	return 0;
};

int readCallback(void * context, char * buffer, int len) {
	len=((JabberStream *) context)->connection->read(buffer, len);

	if (len<0) {
		puts ("in: EOF\n");
		return -1;
	}

	char *input=new char[len+1];
	memcpy(input, buffer, len);
	input[len]=0;
	printf("in: %s\n", input);
	delete input;
	return len;
}

int icloseCallback(void * context){
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
						xmlFree(value);
						//todo: begin conversation
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
							puts(element->toXML()->c_str());
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
					printf("%d %d %s %d %s\n", xmlTextReaderDepth(reader), nodeType, name, empty, value);

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

