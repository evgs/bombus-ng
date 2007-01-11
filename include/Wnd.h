#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>
#include "OwnerDrawRect.h"
#include "IconTextElement.h"
#include <list>


class Wnd {
public:
    virtual ~Wnd();

    HWND getHWnd() const{ return thisHWnd; }

    void showWindow(bool show);
    void setParent(HWND parent);

    virtual const wchar_t * getWindowTitle() const;
    virtual const OwnerDrawRect * getODR() const;

protected:
    HWND parentHWnd;
    HWND thisHWnd;
};

typedef boost::shared_ptr<Wnd> WndRef;
typedef std::list<WndRef> WndRefList;

class WndTitle : public IconTextElement {
public:
    WndTitle(Wnd* wnd, int iconIndex);

    virtual int getColor() const;
    virtual const wchar_t * getText() const;
    virtual int getIconIndex() const;

private:
    int iconIndex;
    Wnd *wnd;
};

typedef boost::shared_ptr<WndTitle> WndTitleRef;
