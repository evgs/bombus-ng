#include "Wnd.h"

void Wnd::showWindow( bool show ) {
    ShowWindow(thisHWnd, (show)? SW_SHOW: SW_HIDE );
}

Wnd::~Wnd() { 
    //TODO: release unused windows
}

void Wnd::setParent( HWND parent ) {
    parentHWnd=parent;
    SetParent(thisHWnd, parent);
}
const wchar_t * Wnd::getWindowTitle() const { return TEXT("<name stub>"); }

const OwnerDrawRect * Wnd::getODR() const { return NULL; }

//////////////////////////////////////////////////////////////////////////

WndTitle::WndTitle( Wnd * wnd, int IconIndex ) {
    this->wnd=wnd;
    this->iconIndex=IconIndex;
    init();
}
int WndTitle::getColor() const{ return 0; }
const wchar_t * WndTitle::getText() const{ return wnd->getWindowTitle();}

int WndTitle::getIconIndex() const { return iconIndex;}