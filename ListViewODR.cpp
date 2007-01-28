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

    ODRList::ref r=ODRList::ref(new ODRList());
    r->selfRef=r;
    bindODRList(r);
    cursorPos=r->getEnum();
}

ListViewODR::~ListViewODR() {}

void ListViewODR::addODR( ODRRef odr, bool redraw ) 
{
    ((ODRList *)(odrlist.get()))->odrVector.push_back(odr);

    notifyListUpdate(redraw);
}

void ListViewODR::eventOk() {
    if (!cursorPos) return;
    if (!cursorPos->hasMoreElements()) return;

    //cursorPos->get()->eventOk();
}

//////////////////////////////////////////////////////////////////////////

ODRListIterator::ODRListIterator( ODRList::ref odrlref ) {
    this->odrlref=odrlref;
    iterator=0;//this->odrlref->odrlist.begin();
}

bool ODRListIterator::isFirstElement() {
    return iterator==0;//((ODRList)(*odrlref)).odrlist.begin();
}

ODRRef ODRListIterator::get() { return odrlref.lock()->odrVector[iterator]; }

void ODRListIterator::next() { iterator++; }

void ODRListIterator::previous() { --iterator; }

bool ODRListIterator::hasMoreElements() { 
    return iterator < /* != */ odrlref.lock()->odrVector.size();//end();
}

bool ODRListIterator::isLastElement() {
    int sz=odrlref.lock()->odrVector.size();
    return (sz==0) || iterator==sz-1;
}

void ODRListIterator::setFirst() { iterator=0; }
void ODRListIterator::setLast() { 
    int sz=odrlref.lock()->odrVector.size();
    iterator=(sz==0)? 0 : sz-1; 
}
ODRSetIterator::ref ODRList::getEnum() {
    return ODRListIterator::ref(new ODRListIterator( selfRef.lock() ));
}
ODRList::ODRList() {}