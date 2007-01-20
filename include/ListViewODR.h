#pragma once

#include "VirtualListView.h"
#include <list>

class ListViewODR : public VirtualListView {
public:
    ListViewODR(HWND parent, const std::string & title);
    virtual ~ListViewODR();

    void addODR(ODRRef odr, bool redraw);

    typedef boost::shared_ptr<ListViewODR> ref;
protected:
private:
};


