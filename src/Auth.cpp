//#include "stdafx.h"

#include "Auth.h"
#include "JabberAccount.h"
#include "JabberStream.h"
#include "ResourceContext.h"
#include "CompressedSocket.h"
#include "CETLSSocket.h"
#include "base64.h"
#include "crypto/MD5.h"
#include "crypto/SHA1.h"
#include "utf8.hpp"


typedef boost::shared_ptr<MD5> MD5Ref;
typedef boost::shared_ptr<SHA1> SHA1Ref;

NonSASLAuth::NonSASLAuth(ResourceContextRef rc, JabberDataBlockRef streamHeader) {
    this->rc=rc;
    streamId=streamHeader->getAttribute("id");

	Log::getInstance()->msg("Non-SASL Login: handshake");

	JabberDataBlockRef login=loginStanza(true, false);

	rc->jabberStream->sendStanza(login);
}

ProcessResult NonSASLAuth::blockArrived( JabberDataBlockRef block, const ResourceContextRef rc ) {
    std::string type=block->getAttribute("type");
    std::string id=block->getAttribute("id");
    if (type=="result") {
        if (id=="auth2") {
            //session established
            rc->jabberStream->jabberListener->loginSuccess();
            return LAST_BLOCK_PROCESSED;
        } else {
            JabberDataBlockRef query=block->getChildByName("query");
            bool plain= !(query->getChildByName("digest"));

            if (plain) 
                if (!rc->account->plainTextPassword) {
                    rc->jabberStream->jabberListener->loginFailed("Plain password required");
                    return LAST_BLOCK_PROCESSED;
                }

            JabberDataBlockRef login=loginStanza(false, plain);

            rc->jabberStream->sendStanza(login);

            return BLOCK_PROCESSED;
        }
    }

    if (type=="error") {
        // login failed
        rc->jabberStream->jabberListener->loginFailed("Auth error");
        return LAST_BLOCK_PROCESSED;
    }
    return BLOCK_REJECTED;
}

JabberDataBlockRef NonSASLAuth::loginStanza( bool get, bool sha1 ) {
    JabberDataBlockRef login = JabberDataBlockRef(new JabberDataBlock("iq"));
    login->setAttribute("type", (get)? "get"   : "set" );
    login->setAttribute("id",   (get)? "auth1" : "auth2" );
    JabberDataBlock * qry=login->addChild("query",NULL);
    qry->setAttribute("xmlns","jabber:iq:auth");
    
    qry->addChild("username",rc->account->getUserName().c_str());
    
    if (!get) {
        if (sha1) {
            SHA1Ref digest=SHA1Ref(new SHA1());
            digest->init();
            digest->updateASCII(streamId);
            digest->updateASCII(rc->account->password);
            digest->finish();

            std::string result=digest->getDigestHex();

            qry->addChild("digest", result.c_str());
        } else {
            qry->addChild("password",rc->account->password.c_str());
        }


        qry->addChild("resource",rc->account->getResource().c_str());
    }
    return login;
}
///////////////////////////////////////////////////////////////////////////////

SASLAuth::SASLAuth(ResourceContextRef rc, JabberDataBlockRef streamHeader){
    this->rc=rc;
	//Log::getInstance()->msg("SASL Login: <stream:stream>");
}

SASLAuth::~SASLAuth() {
    //Log::getInstance()->msg("~SASL Login");
}

const std::string responseMd5Digest( const std::string &user, const std::string &pass, const std::string &realm, const std::string &digestUri, const std::string &nonce, const std::string cnonce);

ProcessResult SASLAuth::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
	//Log::getInstance()->msg("SASL Login: stanza  ", (*(block->toXML())).c_str() );

	if (block->getTagName()=="stream:features") {

#ifndef NOSTARTTLS
        if (rc->account->useEncryption) {
            JabberDataBlockRef starttls=block->getChildByName("starttls");
            if (starttls.get()!=NULL) {
                JabberDataBlock tls("starttls");
                tls.setAttribute("xmlns","urn:ietf:params:xml:ns:xmpp-tls");
                rc->jabberStream->sendStanza(tls);
                return BLOCK_PROCESSED;
            }
        }
#endif
#ifndef NOZLIB
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
#endif

		JabberDataBlockRef mechanisms=block->getChildByName("mechanisms");
		if (mechanisms.get()!=NULL) {

			JabberDataBlock auth("auth");
			auth.setAttribute("xmlns","urn:ietf:params:xml:ns:xmpp-sasl");

            //DIGEST-MD5 mechanism
            if (mechanisms->hasChildByValue("DIGEST-MD5")) {
                Log::getInstance()->msg("Init DIGEST-MD5");

                auth.setAttribute("mechanism", "DIGEST-MD5");

                rc->jabberStream->sendStanza(auth);
                return BLOCK_PROCESSED;
            }
            

            //PLAIN mechanism
            if (mechanisms->hasChildByValue("PLAIN")) {

                if (!rc->account->plainTextPassword) {
                    rc->jabberStream->jabberListener->loginFailed("Plain password required");
                    return LAST_BLOCK_PROCESSED;
                }

				Log::getInstance()->msg("Sending PLAIN password");
					
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
			Log::getInstance()->msg("Binding resource");
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

    if (block->getTagName()=="challenge") {
        std::string challenge= base64::base64Decode(block->getText());
        
        JabberDataBlock resp("response");
        resp.setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-sasl");
        int nonceIndex=challenge.find("nonce=");
        // first stream - step 2. generating DIGEST-MD5 response due to challenge

        if (nonceIndex>=0) {
            nonceIndex+=7; //length("nonce=\"");
            std::string nonce=challenge.substr(nonceIndex, challenge.find('\"', nonceIndex)-nonceIndex);
            std::string cnonce("123456789abcd");

            resp.setText(responseMd5Digest(
                rc->account->getUserName(), 
                rc->account->password, 
                rc->account->getServer(),
                std::string("xmpp/")+rc->account->getServer(),
                nonce,
                cnonce ));
            //System.out.println(resp.toString());
        }
        // first stream - step 3. sending second empty response due to second challenge
        //if (challenge.startsWith("rspauth")) {}
        rc->jabberStream->sendStanza(resp);
        return BLOCK_PROCESSED;
    }

    if (block->getTagName()=="proceed") {
        Log::getInstance()->msg("Starting TLS connection");
#ifndef NOSTARTTLS
        //starting tls layer socket
        /*ConnectionRef tlssocket=ConnectionRef(new TLSSocket(rc->connection));
        rc->connection=tlssocket;**/
        ((CeTLSSocket *)(rc->jabberStream->connection.get()))->startTls(rc->account->ignoreSslWarnings);
        //rc->jabberStream->parser->bindStream(tlssocket);
        rc->jabberStream->sendXmppBeginHeader();
        return LAST_BLOCK_PROCESSED;
#endif
    }

    if (block->getTagName()=="compressed") {
		Log::getInstance()->msg("Opening compressed stream");

#ifndef NOZLIB
		// switching to compressed stream
		ConnectionRef zsocket=ConnectionRef(new CompressedSocket(rc->jabberStream->connection));
		rc->jabberStream->connection=zsocket;
		rc->jabberStream->parser->bindStream( zsocket );
		rc->jabberStream->sendXmppBeginHeader();
#endif
		return LAST_BLOCK_PROCESSED;
	}


    if (block->getTagName()=="success") { 
        // initiating bind session
        rc->jabberStream->sendXmppBeginHeader();
        return LAST_BLOCK_PROCESSED;
    }

    if (block->getTagName()=="failure") { 
        rc->jabberStream->jabberListener->loginFailed("Auth error");
        return LAST_BLOCK_PROCESSED;
    }

	if (block->getTagName()=="iq") {
		if (block->getAttribute("id")=="bind") {
			//get assigned jid/resource
            JabberDataBlockRef bind=block->getChildByName("bind");
            JabberDataBlockRef resource=bind->getChildByName("jid");
            std::string res=resource->getText();

			Log::getInstance()->msg("Resource: ", res.c_str());
			//openung session
			JabberDataBlock session("iq");
			session.setAttribute("type", "set");
			session.setAttribute("id", "sessionInit");
			session.addChild("session", NULL)
				->setAttribute("xmlns", "urn:ietf:params:xml:ns:xmpp-session");
			rc->jabberStream->sendStanza(session);
			return BLOCK_PROCESSED;
		}
		if (block->getAttribute("id")=="sessionInit") {
            //session established
            rc->jabberStream->jabberListener->loginSuccess();
            return LAST_BLOCK_PROCESSED;
		}
	}

	return BLOCK_REJECTED;
}


const std::string responseMd5Digest(
    const std::string &user, 
    const std::string &pass, 
    const std::string &realm, 
    const std::string &digestUri, 
    const std::string &nonce, 
    const std::string cnonce) 
{
    

    MD5Ref hUserRealmPass=MD5Ref(new MD5());
    hUserRealmPass->init();
    hUserRealmPass->updateASCII(user);
    hUserRealmPass->updateByte(':');
    hUserRealmPass->updateASCII(realm);
    hUserRealmPass->updateByte(':');
    hUserRealmPass->updateASCII(pass);
    hUserRealmPass->finish();

    MD5Ref hA1=MD5Ref(new MD5());
    hA1->init();
    hA1->updateArray(hUserRealmPass->getDigestBits(), hUserRealmPass->digestBitsLen, 0);
    hA1->updateByte(':');
    hA1->updateASCII(nonce);
    hA1->updateByte(':');
    hA1->updateASCII(cnonce);
    hA1->finish();

    MD5Ref hA2=MD5Ref(new MD5());
    hA2->init();
    hA2->updateASCII("AUTHENTICATE:");
    hA2->updateASCII(digestUri);
    hA2->finish();

    MD5Ref hResp=MD5Ref(new MD5());
    hResp->init();
    hResp->updateASCII(hA1->getDigestHex());
    hResp->updateByte(':');
    hResp->updateASCII(nonce);
    hResp->updateASCII(":00000001:");
    hResp->updateASCII(cnonce);
    hResp->updateASCII(":auth:");
    hResp->updateASCII(hA2->getDigestHex());
    hResp->finish();

    std::string out = "username=\""+user+"\",realm=\""+realm+"\"," +
        "nonce=\""+nonce+"\",nc=00000001,cnonce=\""+cnonce+"\"," +
        "qop=auth,digest-uri=\""+digestUri+"\"," +
        "response=\""+hResp->getDigestHex()+"\",charset=utf-8";
    std::string resp = base64::base64Encode(out);   
    //System.out.println(decodeBase64(resp));

    return resp;
}

