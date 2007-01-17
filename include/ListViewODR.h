#pragma once

#include "Wnd.h"
#include <list>

class ListViewODR : public Wnd {
public:
    ListViewODR(HWND parent, const std::string & title);
    virtual ~ListViewODR();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual const wchar_t * getWindowTitle() const;
    virtual const OwnerDrawRect * getODR() const;

    void addODR(ODRRef odr, bool redraw);

    typedef boost::shared_ptr<ListViewODR> ref;
protected:

    struct ItemData {
        int yPos;
        ODRRef item;
    };
    typedef std::list<ItemData> ItemList;

    ItemList odrList;

    int winTop;
    ItemList::iterator cursorPos;

    HWND listScrollHWND;



    std::wstring title;
    WndTitleRef  wt;    

private:
    enum {
        SCROLLWIDTH=14
    };

    bool moveCursorTo(int x, int y);
    void cursorFit();

    static ATOM windowClass;
    ATOM RegisterWindowClass();
};


