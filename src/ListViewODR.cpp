#include "ListViewODR.h"

#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "utf8.hpp"

//extern HINSTANCE			g_hInst;
extern int tabHeight;

ListViewODR::ListViewODR( HWND parent, const std::string & title ) {
    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, 0));

    odrlist=ODRSet::ref(new ODRList());
    cursorPos=odrlist->getEnum();
}

ListViewODR::~ListViewODR() {}

void ListViewODR::addODR( ODRRef odr, bool redraw ) 
{
    ((ODRList *)(odrlist.get()))->odrVector.push_back(odr);

    notifyListUpdate(redraw);
}

