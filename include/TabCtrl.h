#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>



class TabsCtrl {
public:
    TabsCtrl(HWND parent);
    virtual ~TabsCtrl();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND getHWnd() { return thisHWnd; }

    //void showWindow(bool show);

protected:
    HWND parentHWnd;
    HWND thisHWnd;
private:
    HWND tabScrollHWnd;

    static ATOM windowClass;
    ATOM RegisterWindowClass();
};

typedef boost::shared_ptr<TabsCtrl> TabsCtrlRef;