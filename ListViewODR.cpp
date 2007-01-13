#include "ListViewODR.h"

#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "utf8.hpp"

extern HINSTANCE			g_hInst;
extern int tabHeight;

ATOM ListViewODR::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ListViewODR::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusLVODR");

    return RegisterClass(&wc);
}

LRESULT CALLBACK ListViewODR::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    ListViewODR *p=(ListViewODR *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(ListViewODR *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            p->listScrollHWND=CreateWindow(_T("SCROLLBAR"), NULL, 
                SBS_VERT | WS_VISIBLE | WS_CHILD,
                0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
                hWnd, NULL, g_hInst, NULL); 
            //dropdownWnd=DoCreateComboControl(hWnd);
            //ShowWindow(p->getListBoxHWnd(), SW_SHOW);
            break;

        }

    case WM_PAINT:

        {
            HDC hdc;
            hdc = BeginPaint(hWnd, &ps);

            // TODO: Add any drawing code here...

            RECT rc = {0, 0, 100, 100};

            SetBkMode(hdc, TRANSPARENT);
            LPCTSTR t=p->title.c_str();
            DrawText(hdc, t, -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            DrawText(hdc, t, -1, &rc, DT_LEFT | DT_TOP);

            int y=tabHeight;

            for (ItemList::const_iterator i=p->odrList.begin(); i!=p->odrList.end(); i++) {
                ODRRef odr=i->item;
                int iHeight=odr->getHeight();
                RECT ritem={0, y, p->clientRect.right, y+iHeight} ;
                odr->draw(hdc, ritem);
                y+=iHeight;
            }
            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.fMask=SIF_ALL;
            GetScrollInfo(p->listScrollHWND, SB_CTL, &si);

            EndPaint(hWnd, &ps);
            break;
        }



    case WM_SIZE: 
        { 
            HDWP hdwp; 

            int height=GET_Y_LPARAM(lParam);
            int width=GET_X_LPARAM(lParam);
            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&(p->clientRect), 0, 0, width, height ); 

            hdwp = BeginDeferWindowPos(1);

            DeferWindowPos(hdwp, p->listScrollHWND, HWND_TOP, width-SCROLLWIDTH, tabHeight, 
                SCROLLWIDTH, height-tabHeight, 
                SWP_NOZORDER 
                );
            EndDeferWindowPos(hdwp); 

            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.fMask=SIF_ALL;
            si.nMax=0;
            si.nMin=0;
            si.nPage=height-tabHeight;
            si.nPos=0;
            si.nTrackPos=0;

            SetScrollInfo(p->listScrollHWND, SB_CTL, &si, TRUE);
            
            break; 
        } 


    case WM_LBUTTONDOWN:
        {
            SHRGINFO    shrg;
            HMENU       hmenu;

            shrg.cbSize = sizeof(shrg);
            shrg.hwndClient = hWnd;
            shrg.ptDown.x = LOWORD(lParam);
            shrg.ptDown.y = HIWORD(lParam);
            shrg.dwFlags = SHRG_RETURNCMD /*| SHRG_NOANIMATION*/;

            if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) {
                /*hmenu = GetSubMenu(g_hMainMenu, 0);
                TrackPopupMenuEx(hmenu,
                    TPM_LEFTALIGN,
                    LOWORD(lParam),
                    HIWORD(lParam),
                    hWnd,
                    NULL);*/
            }
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

ListViewODR::ListViewODR( HWND parent, const std::string & title ) {
    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;

    this->title=utf8::utf8_wchar(title);

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("ListView"), WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);

    wt=WndTitleRef(new WndTitle(this, 0));
}

const wchar_t * ListViewODR::getWindowTitle() const{
    return title.c_str();
}

ListViewODR::~ListViewODR() {}

const OwnerDrawRect * ListViewODR::getODR() const { return wt.get(); }

void ListViewODR::addODR(ODRRef odr){
    int lastY=(odrList.empty())? 0: odrList.back().yPos;
    lastY+=odr->getHeight();

    ItemData item={lastY, odr};
    odrList.push_back(item);
    SetScrollRange(listScrollHWND, SB_CTL, 0, lastY, TRUE);
    InvalidateRect(getHWnd(), NULL, true);
}

ATOM ListViewODR::windowClass=0;