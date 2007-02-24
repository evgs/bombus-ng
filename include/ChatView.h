#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <utf8.hpp>

#include "Wnd.h"
#include "VirtualListView.h"

#include "Contact.h"
#include "OwnerDrawRect.h"

class ChatView : public Wnd{
public:
    //ChatView(HWND parent, const std::string & title);
    ChatView(HWND parent, Contact::ref contact);
    virtual ~ChatView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    //virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    typedef boost::shared_ptr<ChatView> ref;

    void addMessage(const std::string & msg);
    void moveUnread();

    virtual void showWindow(bool show);

protected:
    VirtualListView::ref msgList;
    HWND		editWnd;
    int editHeight;

    Contact::ref contact;

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};
//////////////////////////////////////////////////////////////////////////

