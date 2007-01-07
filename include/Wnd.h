#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>
#include <list>


class Wnd {
public:
    virtual ~Wnd();

    HWND getHWnd() const{ return thisHWnd; }

    void showWindow(bool show);
    void setParent(HWND parent);

    virtual const wchar_t * getWindowTitle() const;

protected:
    HWND parentHWnd;
    HWND thisHWnd;
};

typedef boost::shared_ptr<Wnd> WndRef;
typedef std::list<WndRef> WndRefList;