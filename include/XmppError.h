#pragma once
#include <string>
#include "JabberDataBlock.h"
#include "boostheaders.h"


class XmppError {
public:
    typedef boost::shared_ptr<XmppError> ref;
    enum ErrorCondition {
        NONE=0,
        BAD_REQUEST,
        CONFLICT,
        FEATURE_NOT_IMPLEMENTED,
        FORBIDDEN,
        GONE,
        INTERNAL_SERVER_ERROR,
        ITEM_NOT_FOUND,
        JID_MALFORMED,
        NOT_ACCEPTABLE,
        NOT_ALLOWED,
        NOT_AUTHORIZED,
        PAYMENT_REQUIRED,
        RECIPIENT_UNAVAILEBLE,
        REDIRECT,
        REGISTRATION_REQUIRED,
        REMOTE_SERVER_NOT_FOUND,
        REMOTE_SERVER_TIMEOUT,
        RESOURCE_CONSTRAINT,
        SERVICE_UNAVAILABLE,
        SUBSCRIPTION_REQUIRED,
        UNDEFINED_CONDITION,
        UNEXPECTED_REQUEST
    };

    enum ErrorType {
        TYPE_UNDEFINED=0,
        TYPE_MODIFY,
        TYPE_CANCEL,
        TYPE_AUTH, 
        TYPE_WAIT
    };

    XmppError(int condition, const std::string &text);
    
    JabberDataBlockRef construct() const;

    static XmppError::ref findInStanza(JabberDataBlockRef stanza);

    static XmppError::ref decodeStreamError(JabberDataBlockRef error);
    static XmppError::ref decodeStanzaError(JabberDataBlockRef error);
    
    const std::string & getText() const { return text; };
    const std::string & getName() const { return textCondition; /*TODO: remove stub*/}
    
    const std::string toString() const;

    int getCondition() const { return errCondition; }
    int getActionType() const { return errorType; }
    
private:
    int errCondition;
    int errorType;

    std::string textCondition;
    std::string text;
    static XmppError::ref decodeError(JabberDataBlockRef error, const char *ns);
};
