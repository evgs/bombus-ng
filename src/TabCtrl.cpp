#include "TabCtrl.h"

#include <commctrl.h>
#include <windowsx.h>

extern HINSTANCE			g_hInst;
extern int tabHeight;

//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////

ATOM TabsCtrl::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = TabsCtrl::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusTC");

    return RegisterClass(&wc);
}

LRESULT CALLBACK TabsCtrl::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    HDC hdc;
    TabsCtrl *p=(TabsCtrl *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(TabsCtrl *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            p->tabScrollHWnd=CreateWindow(_T("SCROLLBAR"), NULL, 
                /*SBS_TOPALIGN |*/ SBS_HORZ | WS_VISIBLE | WS_CHILD, 
                0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
                hWnd, NULL, g_hInst, NULL); 
            //dropdownWnd=DoCreateComboControl(hWnd);
            //ShowWindow(p->getListBoxHWnd(), SW_SHOW);
            break;

        }

    case WM_PAINT:

        {
            hdc = BeginPaint(hWnd, &ps);
            /*if (p->makeTabLayout) {
                p->tabDoLayout(hdc);
            }*/

            for (TabList::const_iterator i=p->tabs.begin(); i!=p->tabs.end(); i++) {
                if (i != p->activeTab) drawTab(hdc, p->xOffset, *i, false);
            }

            int width=p->clientRect.right;

            HGDIOBJ old=SelectObject(hdc, GetStockObject(BLACK_PEN));
            MoveToEx(hdc, 0, tabHeight-1, NULL);
            LineTo(hdc, width, tabHeight-1);
            SelectObject(hdc, old);
            /*if (p->activeTab.get())*/ drawTab(hdc, p->xOffset, *(p->activeTab), true);

            RECT b={width-32, 0,  width-16, tabHeight};
            DrawFrameControl(hdc, &b, DFC_SCROLL, DFCS_SCROLLLEFT);
            b.left+=16; b.right+=16;
            DrawFrameControl(hdc, &b, DFC_SCROLL, DFCS_SCROLLRIGHT);

            EndPaint(hWnd, &ps);
        }

        break;

    case WM_LBUTTONDOWN:
        {
            int mouseX=GET_X_LPARAM(lParam);
            int mouseY=GET_Y_LPARAM(lParam);

            int width=p->clientRect.right;

            if (mouseY >= tabHeight) break;

            if (mouseX > width - 16) { 
                p->activeTab++; 
                if (p->activeTab == p->tabs.end()) p->activeTab--;
            } else if (mouseX > width - 32) {
                if (p->activeTab != p->tabs.begin()) p->activeTab--;
            } else
            for (TabList::iterator i=p->tabs.begin(); i!=p->tabs.end(); i++) {
                TabInfoRef tab=*i;
                int tabX=p->xOffset+tab->tabXPos;
                if (mouseX < tabX) continue;
                if (mouseX > tabX+tab->tabWidth ) continue;
                p->activeTab=i;
                break;
            }
            InvalidateRect(p->getHWnd(), NULL, true);
            p->showActiveTab();

            SHRGINFO    shrg;
            shrg.cbSize = sizeof(shrg);
            shrg.hwndClient = hWnd;
            shrg.ptDown.x = LOWORD(lParam);
            shrg.ptDown.y = HIWORD(lParam);
            shrg.dwFlags = SHRG_RETURNCMD /*| SHRG_NOANIMATION*/;

            if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) {

                HMENU hmenu = NULL;//p->getWindowMenu();
                if (hmenu==NULL) break;

                POINT pt={LOWORD(lParam), HIWORD(lParam) };
                ClientToScreen(hWnd, &pt);
                TrackPopupMenuEx(hmenu,
                    /*TPM_LEFTALIGN |*/ TPM_TOPALIGN,
                    pt.x, pt.y,
                    hWnd,
                    NULL);
            }
            break;

            break;
        }

    case WM_SIZE: 
        { 
            int height=GET_Y_LPARAM(lParam);
            int width=GET_X_LPARAM(lParam);
            SetRect(&(p->clientRect), 0, 0, width, height);

            p->updateChildsLayout();

            break; 
        } 

        /*case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: 
        {
        //HGDIOBJ brush= GetStockObject(GRAY_BRUSH);
        //HGDIOBJ pen= GetStockObject(WHITE_PEN);
        SetBkColor(hdc, 0x808080);
        SetTextColor(hdc, 0xffffff);
        //SelectObject((HDC)wParam, brush);
        //SelectObject((HDC)wParam, pen);
        return (BOOL) GetStockObject(GRAY_BRUSH);
        break;
        }*/

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

TabsCtrl::TabsCtrl( HWND parent ) {
    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;
    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("ListView"), WS_VISIBLE | WS_CHILD,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);
    makeTabLayout=false;
    xOffset=0;
    activeTab=tabs.end();
}


void TabsCtrl::addWindow( const WndRef &wnd ) {
    TabInfoRef newTab=TabInfoRef(new TabInfo);
    newTab->wndChild=wnd;
    tabs.push_back(newTab);
    wnd->setParent(thisHWnd);
    if (activeTab==tabs.end()) activeTab=tabs.begin();
    tabDoLayout();
    updateChildsLayout();
    showWindow(true);
}

void TabsCtrl::tabDoLayout(/*HDC hdc*/) {

    int x=0;
    int nTabs=tabs.size();
    int maxTabWidth=120;
    //if (nTabs>2) maxTabWidth=80;
    //if (nTabs>6) maxTabWidth=64;

    for (TabList::const_iterator i=tabs.begin(); i!=tabs.end(); i++) {
        TabInfoRef tab=*i;
        RECT r={0,0,10,10};
        const ODR * odr=tab->wndChild->getODR();
        BOOST_ASSERT(odr);
        //if (odr) {

        r.bottom=odr->getHeight();
        r.right=odr->getWidth();

        //} else DrawText(hdc, tab->wndChild->getWindowTitle(), -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP);

        tab->tabWidth=r.right+4;
        if (tab->tabWidth>maxTabWidth) tab->tabWidth=maxTabWidth;
        tab->tabXPos=x;
        x+=tab->tabWidth;
    }
    //makeTabLayout=false;

}

void TabsCtrl::drawTab( HDC hdc, int offset, TabInfoRef tab, bool active ) {
    offset+=tab->tabXPos;

    int top=(active)? 0:1;
    RECT r={offset, top, offset+tab->tabWidth, tabHeight};
    FillRect(hdc, &r, (HBRUSH)GetStockObject((active)? WHITE_BRUSH : LTGRAY_BRUSH ));

    HGDIOBJ old=SelectObject(hdc, GetStockObject(BLACK_PEN));
    MoveToEx(hdc, offset, tabHeight, NULL);
    LineTo(hdc,offset,top);
    LineTo(hdc,offset+tab->tabWidth, top);
    LineTo(hdc,offset+tab->tabWidth,tabHeight);
    SelectObject(hdc, old);

    SetBkMode(hdc, TRANSPARENT);
    r.left+=2; r.right-=2;
    const ODR * odr=tab->wndChild->getODR();
    if (odr) {
        odr->draw(hdc, r);
    } else 
    DrawText(hdc, tab->wndChild->getWindowTitle(), -1, &r, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS );
}

void TabsCtrl::showActiveTab() {
    TabInfoRef tab =*activeTab;
    int tabX1=tab->tabXPos;
    int tabX2=tabX1+tab->tabWidth;

    if (tabX1 < -xOffset) xOffset=-tabX1;
    if (tabX2 > (clientRect.right-32)-xOffset) xOffset=-(tabX2-(clientRect.right-32));

    for (TabList::const_iterator i=tabs.begin(); i!=tabs.end(); i++) {
        TabInfoRef tab=*i;
        tab->wndChild->showWindow( i == activeTab ); 
    }
}

void TabsCtrl::updateChildsLayout() {
    HDWP hdwp; 

    // Size the tab control to fit the client area. 
    hdwp = BeginDeferWindowPos(tabs.size()+1);

    for (TabList::const_iterator i = tabs.begin(); i != tabs.end(); i++) {
        DeferWindowPos(hdwp, 
                      (i->get())->wndChild->getHWnd(), 
                      HWND_TOP, 
                      0, tabHeight,  
                      clientRect.right, clientRect.bottom-tabHeight, 
                      SWP_NOZORDER);
    }

    //scrollbar
    //DeferWindowPos(hdwp, tabScrollHWnd, HWND_TOP, width-40, 0,  40, 16, SWP_NOZORDER);

    EndDeferWindowPos(hdwp); 
}

bool TabsCtrl::switchByWndRef( WndRef targetWnd ) {
    for (TabList::iterator i = tabs.begin(); i != tabs.end(); i++) {
        if (i->get()->wndChild->getHWnd()==targetWnd->getHWnd()) {
            activeTab=i;
            InvalidateRect(getHWnd(), NULL, true);
            showActiveTab();
            return true;
        }
    }
    return false;
}

bool TabsCtrl::switchByODR( ODRRef targetWnd ) {
    return switchByWndRef(getWindowByODR(targetWnd));
}

WndRef TabsCtrl::getWindowByODR(ODRRef const &title) {
    for (TabList::iterator i=tabs.begin(); i!=tabs.end(); i++) {
        if (i->get()->wndChild->getODR()==title.get()) {
            return i->get()->wndChild;
        }
    }
    return WndRef();
}

void TabsCtrl::fwdWMCommand( int wmId ) {
    SendMessage(activeTab->get()->wndChild->getHWnd(), WM_COMMAND, wmId, 0);
}

ATOM TabsCtrl::windowClass=0;