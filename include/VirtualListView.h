#pragma once

#include "Wnd.h"
#include "OwnerDrawRect.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////

typedef std::vector<ODRRef> ODRList;
typedef boost::shared_ptr<ODRList> ODRListRef;

//////////////////////////////////////////////////////////////////////////
class VirtualListElement {
public:
    typedef boost::shared_ptr<VirtualListElement> ref;
    virtual HMENU getContextMenu(HMENU menu){ return menu; }
    virtual bool OnMenuCommand(int cmdId, HWND parent) { return false; }
};
//////////////////////////////////////////////////////////////////////////

class VirtualListView : public Wnd {
public:
    VirtualListView();
    VirtualListView(HWND parent, const std::string & title);
    virtual ~VirtualListView();

    virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    void bindODRList(ODRListRef odr) {  odrlist=odr; }
    ODRListRef getODRList() { return odrlist; }

    void notifyListUpdate( bool redraw);

    virtual void eventOk();

    void addODR(ODRRef odr, bool redraw);

    virtual HMENU getContextMenu();
    virtual void OnCommand(int cmdId, LONG lParam);

    ODRRef getCursorPos() { return cursorPos; }
    void setCursorPos(ODRRef newPos);
    bool moveCursorEnd();
    bool cursorAtEnd();

    typedef boost::shared_ptr<VirtualListView> ref;


    bool wrapList;
    // enable different background colors for odd and even list elements
    bool colorInterleaving;

protected:

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    int winTop;

    ODRRef cursorPos;

    ODRListRef odrlist;

    std::wstring title;
    WndTitleRef  wt;    

    void init();

private:
    enum {
        SCROLLWIDTH=14
    };

    ODRRef moveCursorTo(int x, int y);
    void moveCursor(int direction);
    void cursorFit();

    static ATOM windowClass;
    ATOM RegisterWindowClass();
};


