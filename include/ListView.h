#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>



class ListView {
public:
    ListView(HWND parent);
    virtual ~ListView();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND getHWnd() { return thisHWnd; }
    HWND getListBoxHWnd() {return listBoxHWnd; }

    void showWindow(bool show);

protected:
    HWND parentHWnd;
    HWND thisHWnd;
    HWND listBoxHWnd;
private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};

typedef boost::shared_ptr<ListView> ListViewRef;