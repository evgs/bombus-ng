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


//////////////////////////////////////////////////////////////////////////

ODRListIterator::ODRListIterator( ODRList * odrlref ) {
    this->odrlref=odrlref;
    iterator=0;//this->odrlref->odrlist.begin();
}

bool ODRListIterator::isFirstElement() {
    return iterator==0;//((ODRList)(*odrlref)).odrlist.begin();
}

ODRRef ODRListIterator::get() { return odrlref->odrVector[iterator]; }

void ODRListIterator::next() { iterator++; }

void ODRListIterator::previous() { --iterator; }

bool ODRListIterator::hasMoreElements() { 
    return iterator!=odrlref->odrVector.size();//end();
}

bool ODRListIterator::isLastElement() {
    int sz=odrlref->odrVector.size();
    return (sz==0) || iterator==sz-1;
}

void ODRListIterator::setFirst() { iterator=0; }
void ODRListIterator::setLast() { 
    int sz=odrlref->odrVector.size();
    iterator=(sz==0)? 0 : sz-1; 
}
ODRSetIterator::ref ODRList::getEnum() {
    return ODRListIterator::ref(new ODRListIterator( this ));
}
ODRList::ODRList() {}