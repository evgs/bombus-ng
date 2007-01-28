#pragma once

#include "VirtualListView.h"
#include <list>

#include <boost/weak_ptr.hpp>

class ListViewODR : public VirtualListView {
public:
    ListViewODR(HWND parent, const std::string & title);
    virtual ~ListViewODR();

    void addODR(ODRRef odr, bool redraw);

    virtual void eventOk();

    typedef boost::shared_ptr<ListViewODR> ref;
protected:
private:
};


class ODRList : public ODRSet {
public:
    ODRList();
    virtual ODRSetIterator::ref getEnum();
    std::vector<ODRRef> odrVector;    

    typedef boost::shared_ptr<ODRList> ref;
    typedef boost::weak_ptr<ODRList> wref;

    wref selfRef;
};
//////////////////////////////////////////////////////////////////////////
class ODRListIterator : public ODRSetIterator {
public:
    ODRListIterator(ODRList::ref odrlref);
    virtual bool isFirstElement();
    virtual bool isLastElement();
    virtual void setFirst();
    virtual void setLast();
    virtual ODRRef get();
    virtual void next();
    virtual void previous();
    virtual bool hasMoreElements();
private:
    ODRList::wref odrlref;
    int iterator;
};
//////////////////////////////////////////////////////////////////////////
