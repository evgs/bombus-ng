#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <utf8.hpp>

#include "Wnd.h"
#include "VirtualListView.h"

#include "Contact.h"
#include "OwnerDrawRect.h"

class ServiceDiscovery : public Wnd{
public:
    //ChatView(HWND parent, const std::string & title);
    ServiceDiscovery(HWND parent);
    virtual ~ServiceDiscovery();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    //virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    typedef boost::shared_ptr<ServiceDiscovery> ref;

    virtual void showWindow(bool show);

    void redraw();

protected:
    VirtualListView::ref nodeList;
    HWND editWnd;
    int editHeight;
    int width;

    std::string jid;

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};
//////////////////////////////////////////////////////////////////////////

