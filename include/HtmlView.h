#pragma once

#include "Wnd.h"
#include "OwnerDrawRect.h"

//////////////////////////////////////////////////////////////////////////


class HtmlView : public Wnd {
public:
    HtmlView();
    HtmlView(HWND parent, const std::string & title);
    virtual ~HtmlView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;


    typedef boost::shared_ptr<HtmlView> ref;


protected:

    std::wstring title;
    WndTitleRef  wt;    

    void init();

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};


