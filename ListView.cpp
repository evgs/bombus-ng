#include "ListView.h"

#include <commctrl.h>
#include <windowsx.h>

extern HINSTANCE			g_hInst;
extern int tabHeight;

HWND tbd;

ATOM ListView::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ListView::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusLV");

    return RegisterClass(&wc);
}

LRESULT CALLBACK ListView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    HDC hdc;


    switch (message) {
    case WM_CREATE:
        {
            ListView *p=(ListView *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            tbd=p->listBoxHWnd=CreateWindow(_T("LISTBOX"), NULL, 
                WS_BORDER| WS_CHILD | /*WS_VISIBLE |*/ WS_VSCROLL
                | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT, 
                0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
                hWnd, NULL, g_hInst, NULL); 
            //dropdownWnd=DoCreateComboControl(hWnd);
            ShowWindow(p->getListBoxHWnd(), SW_SHOW);
            break;

        }

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        {

            RECT rc = {0, 0, 100, 100};

            DrawText(hdc, TEXT("This is listview"), -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            DrawText(hdc, TEXT("This is listview"), -1, &rc, DT_LEFT | DT_TOP);
        }

        // TODO: Add any drawing code here...

        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE: 
        { 
            HDWP hdwp; 
            RECT rc; 

            int height=GET_Y_LPARAM(lParam);
            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&rc, 0, 0, 
                GET_X_LPARAM(lParam), height ); 

            // Size the tab control to fit the client area. 
            hdwp = BeginDeferWindowPos(1);

            /*DeferWindowPos(hdwp, dropdownWnd, HWND_TOP, 0, 0, 
            GET_X_LPARAM(lParam), 20, 
            SWP_NOZORDER 
            ); */


            DeferWindowPos(hdwp, tbd, HWND_TOP, 0, tabHeight, 
                GET_X_LPARAM(lParam), height-tabHeight, 
                SWP_NOZORDER 
                );
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

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ListView::ListView( HWND parent ) {
    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;
    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("ListView"), WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);

}

void ListView::showWindow( bool show ) {
    ::ShowWindow(thisHWnd, (show)? SW_SHOW: SW_HIDE );
}

ListView::~ListView() { 
    //TODO: release unused windows
}

ATOM ListView::windowClass=0;