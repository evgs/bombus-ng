#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>



class Wnd {
public:
    virtual ~Wnd();

    HWND getHWnd() { return thisHWnd; }

    void showWindow(bool show);

protected:
    HWND parentHWnd;
    HWND thisHWnd;
};

typedef boost::shared_ptr<Wnd> WndRef;