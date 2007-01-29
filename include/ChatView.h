#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <utf8.hpp>

#include "Wnd.h"
#include "VirtualListView.h"

class ChatView : public Wnd{
public:
    ChatView(HWND parent, const std::string & title);
    virtual ~ChatView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual const wchar_t * getWindowTitle() const;
    virtual const OwnerDrawRect * getODR() const;

    typedef boost::shared_ptr<ChatView> ref;

    void addMessage(const std::string & msg);

protected:
    VirtualListView::ref msgList;
    HWND		editWnd;
    int editHeight;

    std::wstring title;

    WndTitleRef  wt;    

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};
//////////////////////////////////////////////////////////////////////////
class MessageElement : public OwnerDrawRect {
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

