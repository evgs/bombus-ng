#pragma once

#include "Wnd.h"
#include "OwnerDrawRect.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
class ODRSetIterator {
public:
    typedef boost::shared_ptr<ODRSetIterator> ref;

    virtual ~ODRSetIterator();
    virtual bool isFirstElement()=0;
    virtual bool isLastElement()=0;
    virtual void setFirst()=0;
    virtual void setLast()=0;
    virtual ODRRef get()=0;
    virtual void next()=0;
    virtual void previous()=0;
    virtual bool hasMoreElements()=0;

    bool equals(ref iter2);
    bool operator==(ODRSetIterator &right);
};

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
    virtual const OwnerDrawRect * getODR() const;

    void bindODRList(ODRListRef odr) {  odrlist=odr; }

    void notifyListUpdate( bool redraw);

    virtual void eventOk();

    void addODR(ODRRef odr, bool redraw);

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


