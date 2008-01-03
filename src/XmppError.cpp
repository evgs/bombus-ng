#include "XmppError.h"

const std::string XmppError::toString() const{
    if (text.empty()) return getName();
    return getName()+": "+getText();
}

XmppError::XmppError(int condition, const std::string &text) {
    int errorType=TYPE_UNDEFINED;
    textCondition="undefined-condition";
    switch (condition) {
    case NONE:                      
        errorType=TYPE_UNDEFINED; 
        break;
    case CONFLICT:           
        errorType=TYPE_CANCEL; 
        textCondition="conflict"; 
        break;
    case FEATURE_NOT_IMPLEMENTED:   
        errorType=TYPE_CANCEL; 
        textCondition="feature-not-implemented"; 
        break;
    case FORBIDDEN:  
        errorType=TYPE_AUTH; 
        textCondition="forbidden";
        break;
    case GONE:
        errorType=TYPE_MODIFY; 
        textCondition="gone"; 
        break;
    case INTERNAL_SERVER_ERROR:
        errorType=TYPE_WAIT; 
        textCondition="internal-server-error"; 
        break;
    case ITEM_NOT_FOUND: 
        errorType=TYPE_CANCEL; 
        textCondition="item-not-found";
        break;
    case JID_MALFORMED: 
        errorType=TYPE_MODIFY;
        textCondition="jid-malformed"; 
        break;
    case NOT_ACCEPTABLE:
        errorType=TYPE_MODIFY;
        textCondition="not-acceptable";
        break;
    case NOT_ALLOWED:  
        errorType=TYPE_CANCEL;
        textCondition="not-allowed";
        break;
    case NOT_AUTHORIZED:
        errorType=TYPE_AUTH;
        textCondition="not-authorized";
        break;
    case PAYMENT_REQUIRED:  
        errorType=TYPE_AUTH;
        textCondition="payment-required";
        break;
    case RECIPIENT_UNAVAILEBLE: 
        errorType=TYPE_WAIT;
        textCondition="recipient-unavailable";
        break;
    case REDIRECT:  
        errorType=TYPE_MODIFY; 
        textCondition="redirect";
        break;
    case REGISTRATION_REQUIRED:   
        errorType=TYPE_AUTH;
        textCondition="registration-required";
        break;
    case REMOTE_SERVER_NOT_FOUND:  
        errorType=TYPE_CANCEL;
        textCondition="remote-server-not-found";
        break;
    case REMOTE_SERVER_TIMEOUT:
        errorType=TYPE_WAIT;
        textCondition="remote-server-timeout";
        break;
    case RESOURCE_CONSTRAINT:
        errorType=TYPE_WAIT;
        textCondition="resource-constraint";
        break;
    case SERVICE_UNAVAILABLE:  
        errorType=TYPE_CANCEL;
        textCondition="service-unavailable";
        break;
    case SUBSCRIPTION_REQUIRED:    
        errorType=TYPE_AUTH;
        textCondition="subscription-required";
        break;
        //case UNDEFINED_CONDITION:       errorType=TYPE_UNDEFINED; break;
        //case UNDEFINED_CONDITION:       textCondition="undefined-condition"; break;
    case UNEXPECTED_REQUEST: 
        errorType=TYPE_WAIT;
        textCondition="unexpected-request";
        break;
    }
    errCondition=condition;
    this->text=text;
}

JabberDataBlockRef XmppError::construct() const {
    if (errCondition==NONE) return JabberDataBlockRef();

    JabberDataBlockRef error=JabberDataBlockRef(new JabberDataBlock("error"));

    char *type;
    switch (errorType) {
    case TYPE_MODIFY:    type="modify"; break;
    case TYPE_CANCEL:    type="cancel"; break;
    case TYPE_AUTH:      type="auth"; break;
    case TYPE_WAIT:      type="wait"; break;
    default: /*case TYPE_UNDEFINED:*/ type="cancel"; break;
    }
    error->setAttribute("type", type);

    error->addChildNS(textCondition.c_str(), "urn:ietf:params:xml:ns:xmpp-stanzas");
    if (!text.empty()) error->addChildNS("text", "urn:ietf:params:xml:ns:xmpp-stanzas")->setText(text);

    return error;
}

XmppError::ref XmppError::findInStanza(JabberDataBlockRef stanza) {
    return decodeStanzaError(stanza->getChildByName("error"));
}

XmppError::ref XmppError::decodeError(JabberDataBlockRef error, const char *ns) {
    int errCond=NONE;
    std::string text;

    JabberDataBlockRefList::iterator i=error->getChilds()->begin();
    while (i!=error->getChilds()->end()) {

        JabberDataBlockRef child=*(i++);

        if (child->getAttribute("xmlns")!=ns) continue;
        std::string tag=child->getTagName();
        if (tag=="text")                    text=child->getText();
        if (tag=="bad-request")             errCond=BAD_REQUEST;
        if (tag=="conflict")                errCond=CONFLICT;
        if (tag=="feature-not-implemented") errCond=FEATURE_NOT_IMPLEMENTED;
        if (tag=="forbidden")               errCond=FORBIDDEN;
        if (tag=="gone")                    errCond=GONE;
        if (tag=="internal-server-error")   errCond=INTERNAL_SERVER_ERROR;
        if (tag=="item-not-found")          errCond=ITEM_NOT_FOUND;
        if (tag=="jid-malformed")           errCond=JID_MALFORMED;
        if (tag=="not-acceptable")          errCond=NOT_ACCEPTABLE;
        if (tag=="not-allowed")             errCond=NOT_ALLOWED;
        if (tag=="not-authorized")          errCond=NOT_AUTHORIZED;
        if (tag=="payment-required")        errCond=PAYMENT_REQUIRED;
        if (tag=="recipient-unavailable")   errCond=RECIPIENT_UNAVAILEBLE;
        if (tag=="redirect")                errCond=REDIRECT;
        if (tag=="registration-required")   errCond=REGISTRATION_REQUIRED;
        if (tag=="remote-server-not-found") errCond=REMOTE_SERVER_NOT_FOUND;
        if (tag=="remote-server-timeout")   errCond=REMOTE_SERVER_TIMEOUT;
        if (tag=="resource-constraint")     errCond=RESOURCE_CONSTRAINT;
        if (tag=="service-unavailable")     errCond=SERVICE_UNAVAILABLE;
        if (tag=="subscription-required")   errCond=SUBSCRIPTION_REQUIRED;
        if (tag=="undefined-condition")     errCond=UNDEFINED_CONDITION;
        if (tag=="unexpected-request")      errCond=UNEXPECTED_REQUEST;
    }

    if (errCond==NONE) {
        int code=atoi(error->getAttribute("code").c_str());
        switch (code) {
            case 302: errCond=REDIRECT; break;
            case 400: errCond=BAD_REQUEST; break;
            case 401: errCond=NOT_AUTHORIZED; break;
            case 402: errCond=PAYMENT_REQUIRED; break;
            case 403: errCond=FORBIDDEN; break;
            case 404: errCond=ITEM_NOT_FOUND; break;
            case 405: errCond=NOT_ALLOWED; break;
            case 406: errCond=NOT_ACCEPTABLE; break;
            case 407: errCond=REGISTRATION_REQUIRED; break;
            case 408: errCond=REMOTE_SERVER_TIMEOUT; break;
            case 409: errCond=CONFLICT; break;
            case 500: errCond=INTERNAL_SERVER_ERROR; break;
            case 501: errCond=FEATURE_NOT_IMPLEMENTED; break;
            case 502: 
            case 503:
            case 510: errCond=SERVICE_UNAVAILABLE; break;
            case 504: errCond=REMOTE_SERVER_TIMEOUT; break;
            default: errCond=UNDEFINED_CONDITION;
        }
    }

    XmppError::ref xe=XmppError::ref(new XmppError(errCond, text));

    const std::string &type=error->getAttribute("type");

    if (type=="auth") xe->errorType=TYPE_AUTH;
    if (type=="cancel") xe->errorType=TYPE_CANCEL;
    if (type=="modify") xe->errorType=TYPE_MODIFY;
    if (type=="wait") xe->errorType=TYPE_WAIT;

    JabberDataBlockRef txt=error->findChildNamespace("text", "urn:ietf:params:xml:ns:xmpp-stanzas");
    if (txt) xe->text=txt->getText();

    return xe;
}

XmppError::ref XmppError::decodeStreamError( JabberDataBlockRef error ) {
    if (error->getTagName()!="stream:error") return XmppError::ref();
    return decodeError(error, "urn:ietf:params:xml:ns:xmpp-streams");
}
XmppError::ref XmppError::decodeStanzaError( JabberDataBlockRef error ) {
    if (error->getTagName()!="error") return XmppError::ref();
    return decodeError(error, "urn:ietf:params:xml:ns:xmpp-stanzas");
}