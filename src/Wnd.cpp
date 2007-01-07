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
