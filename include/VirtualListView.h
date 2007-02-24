#pragma once

#include "Wnd.h"
#include "OwnerDrawRect.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////

typedef std::vector<ODRRef> ODRList;
typedef boost::shared_ptr<ODRList> ODRListRef;

//////////////////////////////////////////////////////////////////////////


class VirtualListView : public Wnd {
public:
    VirtualListView();
    VirtualListView(HWND parent, const std::string & title);
    virtual ~VirtualListView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    void bindODRList(ODRListRef odr) {  odrlist=odr; }

    void notifyListUpdate( bool redraw);

    virtual void eventOk();

    void addODR(ODRRef odr, bool redraw);

    virtual HMENU getContextMenu();
    virtual void releaseContextMenu();
    virtual void OnCommand(int cmdId, LONG lParam);

    bool moveCursorEnd();


    typedef boost::shared_ptr<VirtualListView> ref;


protected:

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

    bool wrapList;
    bool moveCursorTo(int x, int y);
    void moveCursor(int direction);
    void cursorFit();

    static ATOM windowClass;
    ATOM RegisterWindowClass();
};


