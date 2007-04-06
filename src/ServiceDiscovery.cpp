#include "ServiceDiscovery.h"
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "../vs2005/ui/ui.h"

#include "ResourceContext.h"
#include "JabberStream.h"
#include "TabCtrl.h"

extern HINSTANCE			g_hInst;
extern int tabHeight;
extern HWND	g_hWndMenuBar;		// menu bar handle
extern ResourceContextRef rc;
extern ImgListRef skin;

//////////////////////////////////////////////////////////////////////////
ATOM ServiceDiscovery::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ServiceDiscovery::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusDisco");

    return RegisterClass(&wc);
}

////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateComboControl(HWND hwndParent) {

    HWND hWndEdit; 
    //TCITEM tie; 

    hWndEdit=CreateWindow(_T("COMBOBOX"), NULL, 
        WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL
        | CBS_DROPDOWN | CBS_LOWERCASE , 
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
        hwndParent, NULL, g_hInst, NULL); 

    //WNDPROC OldEditWndProc = (WNDPROC)SetWindowLong(hWndEdit, GWL_WNDPROC,  (LONG)EditSubClassProc); 
    //SetWindowLong(hWndEdit, GWL_USERDATA, (LONG)OldEditWndProc);

    return hWndEdit;
}



//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ServiceDiscovery::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    HDC hdc;
    ServiceDiscovery *p=(ServiceDiscovery *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(ServiceDiscovery *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            p->nodeList=VirtualListView::ref(new VirtualListView(hWnd, std::string("disco")));
            p->nodeList->setParent(hWnd);
            p->nodeList->showWindow(true);
            //p->nodeList->wrapList=false;
            //p->nodeList->colorInterleaving=true;

            RECT rect;
            p->editWnd=DoCreateComboControl(hWnd);
            GetWindowRect(p->editWnd, &rect);
            p->editHeight=rect.bottom-rect.top+2;

            //p->msgList->bindODRList(p->contact->messageList);
            break;
        }

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        {
            //p->contact->nUnread=0;
            RECT rc = {0, 0, 200, tabHeight};
            SetBkMode(hdc, TRANSPARENT);
            //SetTextColor(hdc, p->contact->getColor());
            //p->contact->draw(hdc, rc);

            skin->drawElement(hdc, icons::ICON_CLOSE, p->width-2-skin->getElementWidth(), 0);
            skin->drawElement(hdc, icons::ICON_SEARCH_INDEX, p->width-4-skin->getElementWidth()*2, 0);

            

            /*SetBkMode(hdc, TRANSPARENT);
            LPCTSTR t=p->title.c_str();
            DrawText(hdc, t, -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            DrawText(hdc, t, -1, &rc, DT_LEFT | DT_TOP);*/
        }

        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE: 
        { 
            HDWP hdwp; 
            RECT rc; 

            int height=GET_Y_LPARAM(lParam);
            p->width=GET_X_LPARAM(lParam);

            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&rc, 0, 0, 
                GET_X_LPARAM(lParam), height ); 

            // Size the tab control to fit the client area. 
            hdwp = BeginDeferWindowPos(2);

            DeferWindowPos(hdwp, p->editWnd, NULL, 1, 1, 
                GET_X_LPARAM(lParam)-(tabHeight*2+2), p->editHeight, 
                SWP_NOZORDER 
                ); 


            DeferWindowPos(hdwp, p->nodeList->getHWnd(), HWND_TOP, 0, p->editHeight, 
                GET_X_LPARAM(lParam), height - p->editHeight, 
                SWP_NOZORDER 
                );


            EndDeferWindowPos(hdwp); 

            break; 
        } 

    case WM_COMMAND: 
        {
            break;             
        }

    case WM_LBUTTONDOWN:
        SetFocus(hWnd);
        if ((GET_Y_LPARAM(lParam)) > p->editHeight) break;
        if (GET_X_LPARAM(lParam) > p->width-2-skin->getElementWidth()) {
            PostMessage(GetParent(hWnd), WM_COMMAND, TabsCtrl::CLOSETAB, 0);
        }
        break;

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ServiceDiscovery::ServiceDiscovery( HWND parent ) {
    BOOST_ASSERT(parent);

    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;

    //TEXTMETRIC txm;
    //GetTextMetrics(NULL, &txm); //TODO - hdc must not be NULL
    //editHeight=txm.tmHeight*4;
    //editHeight=60;

    //this->contact=contact;

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("disco"), WS_CHILD |WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);

}

//const wchar_t * ChatView::getWindowTitle() const{  return TEXT("stub"); }

ServiceDiscovery::~ServiceDiscovery() {
    DestroyWindow(editWnd);
}

const ODR * ServiceDiscovery::getODR() const { return nodeList->getODR(); }

void ServiceDiscovery::showWindow( bool show ) {
    Wnd::showWindow(show);

    /*TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE;
    tbbi.fsState = (show)? TBSTATE_ENABLED : TBSTATE_HIDDEN;

    ::SendMessage (g_hWndMenuBar, TB_SETBUTTONINFO, IDS_SEND, (LPARAM)&tbbi);*/

    //if (show) contact->nUnread=0;

    if (show) nodeList->notifyListUpdate(true);
    //if (show) InvalidateRect(msgList->getHWnd(), NULL, false);

    if (show) SetFocus(nodeList->getHWnd());

}

void ServiceDiscovery::redraw(){
    InvalidateRect(getHWnd(), NULL, TRUE);
    nodeList->notifyListUpdate(true);
    //InvalidateRect(msgList->getHWnd(), NULL, false);
}

ATOM ServiceDiscovery::windowClass=0;

//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////
