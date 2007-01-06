#include "Window.h"

void Wnd::showWindow( bool show ) {
    ::ShowWindow(thisHWnd, (show)? SW_SHOW: SW_HIDE );
}
