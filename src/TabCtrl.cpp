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

            unsigned int activeTab=p->activeTab;

            for (unsigned int i=0; i < p->tabs.size(); i++) {
                if (i != activeTab) drawTab(hdc, p->xOffset, p->tabs[i], false);
            }

            int width=p->clientRect.right;

            HGDIOBJ old=SelectObject(hdc, GetStockObject(BLACK_PEN));
            MoveToEx(hdc, 0, tabHeight-1, NULL);
            LineTo(hdc, width, tabHeight-1);
            SelectObject(hdc, old);
            if (activeTab >= 0  &&  activeTab < p->tabs.size() )
                drawTab(hdc, p->xOffset, p->tabs[activeTab], true);

            RECT b={width-2*tabHeight, 0,  width-tabHeight, tabHeight};
            DrawFrameControl(hdc, &b, DFC_SCROLL, DFCS_SCROLLLEFT);
            b.left+=tabHeight; b.right+=tabHeight;
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

            if (mouseX > width - tabHeight) { 
                p->activeTab++; 
                if (p->activeTab == p->tabs.size()) p->activeTab--;
            } else if (mouseX > width - 2*tabHeight) {
                if (p->activeTab != 0) p->activeTab--;
            } else
            for (unsigned int i=0; i < p->tabs.size(); i++) {
                TabInfoRef tab=p->tabs[i];
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

                HMENU hmenu = p->getContextMenu();
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

    case WM_MEASUREITEM:
        {
            LPMEASUREITEMSTRUCT mi=(LPMEASUREITEMSTRUCT)lParam;
            ODR * odr=(ODR *)(mi->itemData);
            mi->itemHeight=odr->getHeight();
            mi->itemWidth=odr->getWidth();
            return true;
        }
    case WM_DRAWITEM:
        {
            LPDRAWITEMSTRUCT di=(LPDRAWITEMSTRUCT) lParam;
            HBRUSH bgnd=CreateSolidBrush(GetBkColor(di->hDC));
            FillRect(di->hDC, &(di->rcItem), bgnd);
            DeleteObject(bgnd);
            ODR * odr=(ODR *)(di->itemData);
            odr->draw(di->hDC, di->rcItem);
            return TRUE;
        }

    case WM_COMMAND:
        {
            int cmd=LOWORD(wParam);
            if (cmd>=TabsCtrl::TAB_BEGIN_INDEX  && cmd<TabsCtrl::TAB_END_INDEX) {
                //switch to the selected tab
                p->activeTab=cmd-TabsCtrl::TAB_BEGIN_INDEX;
            }
            if (cmd==TabsCtrl::CLOSETAB) {
                //close current tab
                int activeTab=p->activeTab;
                if (activeTab>1) p->tabs.erase(p->tabs.begin()+activeTab);
                if (activeTab>=p->tabs.size()) p->activeTab--;
            }
            p->tabDoLayout();
            InvalidateRect(p->getHWnd(), NULL, true);
            p->showActiveTab();
            return 0;
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
    activeTab=-1;

    hmenu=NULL;
}


void TabsCtrl::addWindow( const WndRef &wnd ) {
    TabInfoRef newTab=TabInfoRef(new TabInfo);
    newTab->wndChild=wnd;
    tabs.push_back(newTab);
    wnd->setParent(thisHWnd);
    if (activeTab<0) activeTab=0;
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
    TabInfoRef tab =tabs[activeTab];
    int tabX1=tab->tabXPos;
    int tabX2=tabX1+tab->tabWidth;

    if (tabX1 < -xOffset) xOffset=-tabX1;
    if (tabX2 > (clientRect.right-32)-xOffset) xOffset=-(tabX2-(clientRect.right-32));

    for (unsigned int i=0; i < tabs.size(); i++) {
        TabInfoRef tab=tabs[i];
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
    if (!targetWnd) return false;
    for (unsigned int i = 0; i < tabs.size(); i++) {
        if (tabs[i]->wndChild->getHWnd()==targetWnd->getHWnd()) {
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
    SendMessage(tabs[activeTab]->wndChild->getHWnd(), WM_COMMAND, wmId, 0);
}

ATOM TabsCtrl::windowClass=0;


HMENU TabsCtrl::getContextMenu() {
    if (hmenu!=NULL) releaseContextMenu();
    //if (!cursorPos) return NULL;

    hmenu=CreatePopupMenu();
    AppendMenu(hmenu, MF_STRING, TabsCtrl::CLOSETAB, TEXT("Close"));
    AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
    int index=TabsCtrl::TAB_BEGIN_INDEX;
    for (TabList::iterator i=tabs.begin(); i!=tabs.end(); i++) {
        const ODR * title=i->get()->wndChild->getODR();
        //LPCTSTR title=i->get()->wndChild->getODR()->getText();
        //AppendMenu(hmenu, MF_STRING, index, title);
        AppendMenu(hmenu, MF_OWNERDRAW, index, (LPCWSTR) title);
        index++;
    }
    //AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);
    //AppendMenu(hmenu, MF_STRING, 49998, TEXT("Close2"));

    //AppendMenu(hmenu, MF_SEPARATOR , 0, NULL);


    return hmenu;
}

void TabsCtrl::releaseContextMenu() {
    if (hmenu!=NULL) DestroyMenu(hmenu);
    hmenu=NULL;
}
