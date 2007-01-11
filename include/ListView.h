#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <utf8.hpp>

#include "Wnd.h"

class ListView : public Wnd{
public:
    ListView(HWND parent, const std::string & title);
    virtual ~ListView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND getListBoxHWnd() {return listBoxHWnd; }
    virtual const wchar_t * getWindowTitle() const;
    virtual const OwnerDrawRect * getODR() const;
protected:
    HWND listBoxHWnd;

    std::wstring title;

    WndTitleRef  wt;    

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};

typedef boost::shared_ptr<ListView> ListViewRef;