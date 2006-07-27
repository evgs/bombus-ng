#include <iostream>

#include "Auth.h"
#include "JabberAccount.h"
#include "JabberStream.h"
#include "ResourceContext.h"
#include "CompressedSocket.h"

void NonSASLAuth::beginConversation(const std::string & streamId) {
	std::cout << "Non-SASL Login: sending password" << std::endl;

	JabberDataBlockRef login=JabberDataBlockRef(new JabberDataBlock("iq"));
	login->setAttribute("type","set");
	login->setAttribute("id","auth");
	JabberDataBlock * qry=login->addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:auth");
	qry->addChild("username",rc->account->getUserName().c_str());
	qry->addChild("password",rc->account->password.c_str());
	qry->addChild("resource",rc->account->getResource().c_str());

	rc->jabberStream->sendStanza(login);
}
void NonSASLAuth::endConversation(){
	std::cout << "end conversation" << std::endl;
};


///////////////////////////////////////////////////////////////////////////////

void SASLAuth::beginConversation(const std::string &streamId){
	std::cout << "SASL Login: <stream:stream>" << std::endl;
}

void SASLAuth::endConversation(){
	std::cout << "end conversation" << std::endl;
};

ProcessResult SASLAuth::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
	std::cout << "SASL Login: stanza  " << *(block->toXML()) << std::endl;

	if (block->getTagName()=="stream:features") {
		JabberDataBlockRef compression=block->getChildByName("compression");
		if (compression.get()!=NULL)
		if (compression->hasChildByValue("zlib")) {
			JabberDataBlock compress=JabberDataBlock("compress");
			compress.setAttribute("xmlns","http://jabber.org/protocol/compress");
			compress.addChild("method","zlib");
			rc->jabberStream->sendStanza(compress);
			return BLOCK_PROCESSED;
		}
	}

	if (block->getTagName()=="compressed") {
		ConnectionRef zsocket=ConnectionRef(new CompressedSocket(rc->connection));
		rc->connection=zsocket;
		rc->jabberStream->parser->bindStream( zsocket );
		rc->jabberStream->sendXmppBeginHeader();
	}
	return BLOCK_PROCESSED;
}

