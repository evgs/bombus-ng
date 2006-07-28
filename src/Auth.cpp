#include <iostream>

#include "Auth.h"
#include "JabberAccount.h"
#include "JabberStream.h"
#include "ResourceContext.h"
#include "CompressedSocket.h"
#include "base64.h"

void NonSASLAuth::beginConversation(const std::string & streamId) {
	std::cout << "Non-SASL Login: sending password" << std::endl;

	JabberDataBlockRef login=JabberDataBlockRef(new JabberDataBlock("iq"));
	login->setAttribute("type","set");
	login->setAttribute("id","sessionInit");
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
		if (rc->account->useCompression) {
			JabberDataBlockRef compression=block->getChildByName("compression");
			if (compression.get()!=NULL) {
				if (compression->hasChildByValue("zlib")) {
					JabberDataBlock compress("compress");
					compress.setAttribute("xmlns","http://jabber.org/protocol/compress");
					compress.addChild("method","zlib");
					rc->jabberStream->sendStanza(compress);
					return BLOCK_PROCESSED;
				}
			}
		}

		JabberDataBlockRef mechanisms=block->getChildByName("mechanisms");
		if (mechanisms.get()!=NULL) {

			JabberDataBlock auth("auth");
			auth.setAttribute("xmlns","urn:ietf:params:xml:ns:xmpp-sasl");

			if (mechanisms->hasChildByValue("PLAIN")) {
				auth.setAttribute("mechanism", "PLAIN");

				std::string plain(rc->account->getBareJid());
				plain+=(char)0x00;
				plain+=rc->account->getUserName();
				plain+=(char)0x00;
				plain+=rc->account->password;

				auth.setText(base64::base64Encode(plain));

				rc->jabberStream->sendStanza(auth);
				return BLOCK_PROCESSED;
			}
		}

		// resource binding session
		JabberDataBlockRef bindsess=block->getChildByName("bind");
		if (bindsess.get()!=NULL) {
			JabberDataBlock bindIq("iq");
			bindIq.setAttribute("type", "set");
			bindIq.setAttribute("id", "bind");
			JabberDataBlock *bind=bindIq.addChild("bind", NULL);
			bind->setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-bind");
			bind->addChild("resource", rc->account->getResource().c_str() );
			rc->jabberStream->sendStanza(bindIq);
			return BLOCK_PROCESSED;
		}
	}

	if (block->getTagName()=="compressed") {
		// switching to compressed stream
		ConnectionRef zsocket=ConnectionRef(new CompressedSocket(rc->connection));
		rc->connection=zsocket;
		rc->jabberStream->parser->bindStream( zsocket );
		rc->jabberStream->sendXmppBeginHeader();
		return BLOCK_PROCESSED;
	}

	if (block->getTagName()=="success") { 
		// initiating bind session
		rc->jabberStream->sendXmppBeginHeader();
		return BLOCK_PROCESSED;
	}

	if (block->getTagName()=="iq") {
		if (block->getAttribute("id")=="bind") {
			// TODO: get assigned jid/resource
			//openung session
			JabberDataBlock session("iq");
			session.setAttribute("type", "set");
			session.setAttribute("id", "sessionInit");
			session.addChild("session", NULL)
				->setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-session");
			rc->jabberStream->sendStanza(session);
			return LAST_BLOCK_PROCESSED;
		}
		/*if (block->getAttribute("id")=="sessionInit") {
			//TODO: session established, now sending presence
		}*/
	}

	return BLOCK_REJECTED;
}

