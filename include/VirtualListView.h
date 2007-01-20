#pragma once

#include "Wnd.h"
#include "OwnerDrawRect.h"

//////////////////////////////////////////////////////////////////////////
class ODRSetIterator {
public:
    typedef boost::shared_ptr<ODRSetIterator> ref;

    virtual ~ODRSetIterator();
    virtual bool isFirstElement()=0;
    virtual ODRRef get()=0;
    virtual ODRRef next()=0;
    virtual ODRRef previous()=0;
    virtual bool hasMoreElements()=0;

    bool equals(ref iter2);
    bool operator==(ODRSetIterator &right);
};

class ODRSet {
public:
    virtual ~ODRSet();
    virtual ODRSetIterator::ref getEnum()=0;
    typedef boost::shared_ptr<ODRSet> ref;
};


//////////////////////////////////////////////////////////////////////////

class ODRList : public ODRSet {
public:
    virtual ODRSetIterator::ref getEnum();
    std::list<ODRRef> odrlist;    
    typedef boost::shared_ptr<ODRList> ref;
};
//////////////////////////////////////////////////////////////////////////
class ODRListIterator : public ODRSetIterator {
public:
    ODRListIterator(ODRList::ref odrlref);
    virtual bool isFirstElement();
    virtual ODRRef get();
    virtual ODRRef next();
    virtual ODRRef previous();
    virtual bool hasMoreElements();
private:
    ODRList::ref odrlref;
    std::list<ODRRef>::iterator iterator;
};

class VirtualListView : public Wnd {
public:
    VirtualListView(HWND parent, const std::string & title);
    virtual ~VirtualListView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual const wchar_t * getWindowTitle() const;
    virtual const OwnerDrawRect * getODR() const;

    void addODR( bool redraw);

    typedef boost::shared_ptr<VirtualListView> ref;
protected:

    int winTop;

    ODRSetIterator::ref cursorPos;

    ODRSet::ref odrlist;


    //HWND listScrollHWND;



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


