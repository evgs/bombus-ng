#pragma once

#include "HtmlView.h"
#include "JabberDataBlock.h"
#include "JabberDataBlockListener.h"
#include <boost/shared_ptr.hpp>

class GetInfo;

class ClientInfoForm : public HtmlView {
private:
    ClientInfoForm(){};
public:
    virtual ~ClientInfoForm();

    void infoArrivedNotify();
    typedef boost::shared_ptr<ClientInfoForm> ref;
    virtual void onWmUserUpdate();

    static ClientInfoForm::ref createInfoForm(HWND parent, const std::string &jid, ResourceContextRef rc);

    std::string clientName;
    std::string clientVersion;
    std::string clientOS;

    std::string lastActivityTime;
    std::string lastActivityMessage;

    std::string clientTime;

protected:
    std::string jid;

    virtual HBITMAP getImage(LPCTSTR url, DWORD cookie);
    virtual void onHotSpot(LPCSTR url, LPCSTR param);

    int detectMime(char *buf);

    boost::weak_ptr<ClientInfoForm> formRef;

    boost::weak_ptr<ResourceContext> rc;
    JabberDataBlockListenerRef listener;

    GetInfo *infoListener;
};