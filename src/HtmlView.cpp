#include "HtmlView.h"

#include <sipapi.h>
#include <htmlctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "utf8.hpp"

extern HINSTANCE			g_hInst;
extern int tabHeight;

ATOM HtmlView::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = HtmlView::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = NULL;//(HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusHTV");

    return RegisterClass(&wc);
}

LRESULT CALLBACK HtmlView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    HtmlView *p=(HtmlView *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(HtmlView *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );


            RECT rc;

            GetClientRect (hWnd, &rc);

            p->htmlHWnd = CreateWindow (
                DISPLAYCLASS, 
                NULL, 
                WS_CHILD | WS_VISIBLE | /*WS_VSCROLL |*/ WS_CLIPSIBLINGS, 
                rc.left, rc.top,
                rc.right - rc.left, rc.bottom - rc.top, 
                hWnd, 
                0, //(HMENU)IDC_HTMLVIEW, 
                g_hInst, 
                NULL);

            SetFocus(p->htmlHWnd);

            HWND hwndHTML=p->htmlHWnd;
            bool   fFitToPage = TRUE;
            PostMessage(hwndHTML, DTM_ENABLESHRINK, 0, fFitToPage);

            SendMessage(hwndHTML, WM_SETTEXT, 0, (LPARAM)"");
            SendMessage(hwndHTML, DTM_ADDTEXTW, FALSE, 
                (LPARAM)TEXT("<HTML><TITLE>Test</TITLE><BODY><P>Loading...<BR></BODY></HTML>"));
            SendMessage(hwndHTML, DTM_ENDOFSOURCE, 0, (LPARAM)NULL);


            /*SendMessage(hwndHTML, WM_SETTEXT, 0, (LPARAM)"");
            SendMessage(hwndHTML, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<HTML><TITLE>Test</TITLE>"));
            SendMessage(hwndHTML, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<BODY><P>"));
            SendMessage(hwndHTML, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<h1>Heading</h1>Normal Text<BR>"));
            SendMessage(hwndHTML, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("<A HREF=\"www.voscorp.com\"><IMG SRC=\"\\Pic.gif\"></A>"));
            SendMessage(hwndHTML, DTM_ADDTEXTW, FALSE, (LPARAM)TEXT("</BODY></HTML>"));
            SendMessage(hwndHTML, DTM_ENDOFSOURCE, 0, (LPARAM)NULL);*/

            break;

        }

    case WM_PAINT:

        {
            PAINTSTRUCT ps;
            HDC wnd;
            wnd = BeginPaint(hWnd, &ps);


            EndPaint(hWnd, &ps);
            break;
        }

    case WM_SIZE: 
        { 
            //HDWP hdwp; 

            int height=GET_Y_LPARAM(lParam);
            int width=GET_X_LPARAM(lParam);
            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&(p->clientRect), 0, 0, width, height ); 

            if(IsWindow(p->htmlHWnd))
                SetWindowPos(p->htmlHWnd, 0, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
            /*hdwp = BeginDeferWindowPos(1);

            DeferWindowPos(hdwp, p->listScrollHWND, HWND_TOP, width-SCROLLWIDTH, tabHeight, 
            SCROLLWIDTH, height-tabHeight, 
            SWP_NOZORDER 
            );
            EndDeferWindowPos(hdwp); */

            break; 
        } 

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HtmlView::HtmlView() { /*init(); - MUST NOT be called before setting up parentHWnd */ }

void HtmlView::init() {
    BOOST_ASSERT(parentHWnd);

    if (htmlViewInstance==0) 
        htmlViewInstance=LoadLibrary(L"htmlview.dll");

    if (htmlViewInstance==0) throw std::exception("Unable to initialize HTML control");

    if(!InitHTMLControl( g_hInst )) throw std::exception("Unable to initialize HTML control");

    if (windowClass==0) 
        windowClass=RegisterWindowClass();

    if (windowClass==0) throw std::exception("Can't create window class");

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("ListView"), WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parentHWnd, NULL, g_hInst, (LPVOID)this);

}

HtmlView::HtmlView( HWND parent, const std::string & title ) {
    parentHWnd=parent;
    init();

    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, 0));
}

const wchar_t * HtmlView::getWindowTitle() const{
    return title.c_str();
}

HtmlView::~HtmlView() {}

const ODR * HtmlView::getODR() const { return wt.get(); }

ATOM HtmlView::windowClass=0;
HINSTANCE HtmlView::htmlViewInstance=0;
