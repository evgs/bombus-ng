#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>



class Wnd {
public:
    virtual Wnd(HWND parent);
    virtual ~Wnd();
    virtual static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND getHWnd() { return thisHWnd; }

    void showWindow(bool show);

protected:
    HWND parentHWnd;
    HWND thisHWnd;

    static ATOM windowClass;
    ATOM RegisterWindowClass();
};

typedef boost::shared_ptr<Wnd> WndRef;