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

class ODRSet {
public:
    ODRSet();
    virtual ~ODRSet();
    virtual ODRSetIterator::ref getEnum()=0;
    typedef boost::shared_ptr<ODRSet> ref;
protected:
};


//////////////////////////////////////////////////////////////////////////

class ODRList : public ODRSet {
public:
    ODRList();
    virtual ODRSetIterator::ref getEnum();
    std::vector<ODRRef> odrVector;    
    typedef boost::shared_ptr<ODRList> ref;
};
//////////////////////////////////////////////////////////////////////////
class ODRListIterator : public ODRSetIterator {
public:
    ODRListIterator(ODRList * odrlref);
    virtual bool isFirstElement();
    virtual bool isLastElement();
    virtual void setFirst();
    virtual void setLast();
    virtual ODRRef get();
    virtual void next();
    virtual void previous();
    virtual bool hasMoreElements();
private:
    ODRList * odrlref;
    int iterator;
};
//////////////////////////////////////////////////////////////////////////

class VirtualListView : public Wnd {
public:
    VirtualListView();
    VirtualListView(HWND parent, const std::string & title);
    virtual ~VirtualListView();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual const wchar_t * getWindowTitle() const;
    virtual const OwnerDrawRect * getODR() const;

    void notifyListUpdate( bool redraw);

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

    bool wrapList;
    bool moveCursorTo(int x, int y);
    void cursorFit();

    static ATOM windowClass;
    ATOM RegisterWindowClass();
};


