#include "TabCtrl.h"

#include <commctrl.h>
#include <windowsx.h>

extern HINSTANCE			g_hInst;
extern int tabHeight;

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
        hdc = BeginPaint(hWnd, &ps);
        if (p->makeTabLayout) {
            p->tabDoLayout(hdc);
        }

        {
            int offset=0;
            
            for (TabList::const_iterator i=p->tabs.begin(); i!=p->tabs.end(); i++) {
                TabInfoRef tab=*i;
                if (tab.get() != p->activeTab.get()) drawTab(hdc, offset, tab, false);
            }
            HGDIOBJ old=SelectObject(hdc, GetStockObject(BLACK_PEN));
            MoveToEx(hdc, 0, tabHeight-1, NULL);
            LineTo(hdc,p->width, tabHeight-1);
            SelectObject(hdc, old);
            drawTab(hdc, offset, p->activeTab, true);

        }

        // TODO: Add any drawing code here...

        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE: 
        { 
            HDWP hdwp; 
            RECT rc; 

            int height=GET_Y_LPARAM(lParam);
            int width=GET_X_LPARAM(lParam);
            p->width=width;
            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&rc, 0, 0, 
                GET_X_LPARAM(lParam), height ); 

            // Size the tab control to fit the client area. 
            hdwp = BeginDeferWindowPos(p->tabs.size()+1);

            for (TabList::const_iterator i = p->tabs.begin(); i != p->tabs.end(); i++) {
                DeferWindowPos(hdwp, (i->get())->wndChild->getHWnd(), HWND_TOP, 0, tabHeight,  width, height-tabHeight, SWP_NOZORDER);
            }

            //scrollbar
            DeferWindowPos(hdwp, p->tabScrollHWnd, HWND_TOP, width-40, 0,  40, 16, SWP_NOZORDER);

            EndDeferWindowPos(hdwp); 

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
    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("ListView"), WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);
    makeTabLayout=false;
}


void TabsCtrl::addWindow( const WndRef &wnd ) {
    TabInfoRef newTab=TabInfoRef(new TabInfo);
    newTab->wndChild=wnd;
    tabs.push_back(newTab);
    wnd->setParent(thisHWnd);
    if (activeTab.get()==NULL) activeTab=newTab;
    makeTabLayout=true;
    showWindow(true);
}

void TabsCtrl::tabDoLayout(HDC hdc) {

    int x=0;
    int nTabs=tabs.size();
    int maxTabWidth=120;
    if (nTabs>2) maxTabWidth=80;
    if (nTabs>6) maxTabWidth=64;

    for (TabList::const_iterator i=tabs.begin(); i!=tabs.end(); i++) {
        TabInfoRef tab=*i;
        RECT r={0,0,10,10};
        DrawText(hdc, tab->wndChild->getWindowTitle(), -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP);
        tab->tabWidth=r.right+6;
        if (tab->tabWidth>maxTabWidth) tab->tabWidth=maxTabWidth;
        tab->tabXPos=x;
        x+=tab->tabWidth;
    }
    makeTabLayout=false;

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
    r.left+=3; r.right-=3;
    DrawText(hdc, tab->wndChild->getWindowTitle(), -1, &r, DT_LEFT | DT_CENTER );
}
ATOM TabsCtrl::windowClass=0;