#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <utf8.hpp>

class ListView : public Wnd{
public:
    ListView(HWND parent, const std::string & title);
    virtual ~ListView();
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND getHWnd() { return thisHWnd; }
    HWND getListBoxHWnd() {return listBoxHWnd; }

protected:
    HWND parentHWnd;
    HWND thisHWnd;
    HWND listBoxHWnd;

    std::wstring title;

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};

typedef boost::shared_ptr<ListView> ListViewRef;