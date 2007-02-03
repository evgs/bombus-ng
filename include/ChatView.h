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
    ChatView(HWND parent, ODRRef contact);
    virtual ~ChatView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    //virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    typedef boost::shared_ptr<ChatView> ref;

    void addMessage(const std::string & msg);

protected:
    VirtualListView::ref msgList;
    HWND		editWnd;
    int editHeight;

    ODRRef contact;

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};
//////////////////////////////////////////////////////////////////////////
class MessageElement : public ODR {
public:
    MessageElement(const std::string &str);
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getColor() const;

    virtual void draw(HDC hdc, RECT &rt) const;

protected:
    std::wstring wstr;
    int width;
    int height;
    void init();
    virtual const wchar_t * getText() const;
};

