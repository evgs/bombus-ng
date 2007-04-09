#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <utf8.hpp>

#include "Wnd.h"
#include "VirtualListView.h"

#include "Contact.h"
#include "OwnerDrawRect.h"

#include "ResourceContext.h"

class ServiceDiscovery : public Wnd{
protected:
    ServiceDiscovery(HWND parent);
public:
    //ChatView(HWND parent, const std::string & title);
    virtual ~ServiceDiscovery();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    //virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    typedef boost::shared_ptr<ServiceDiscovery> ref;

    virtual void showWindow(bool show);

    void redraw();

    static ServiceDiscovery::ref createServiceDiscovery(HWND parent, ResourceContextRef rc, const std::string &jid);

protected:
    

    VirtualListView::ref nodeList;
    HWND editWnd;
    int editHeight;
    int width;

    std::string jid;

    boost::weak_ptr<ServiceDiscovery> thisRef;

    ResourceContextRef rc;
    void discoverJid(const std::string &jid);

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};
//////////////////////////////////////////////////////////////////////////

