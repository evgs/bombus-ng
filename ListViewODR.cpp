#include "ListViewODR.h"

#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "utf8.hpp"

extern HINSTANCE			g_hInst;
extern int tabHeight;

ATOM ListViewODR::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = ListViewODR::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = NULL;//(HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusLVODR");

    return RegisterClass(&wc);
}

LRESULT CALLBACK ListViewODR::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
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
            PAINTSTRUCT ps;
            HDC hdc;
            HDC wnd;
            wnd = BeginPaint(hWnd, &ps);

            HBITMAP buf=CreateCompatibleBitmap(wnd, p->clientRect.right, p->clientRect.bottom);

            hdc=CreateCompatibleDC(NULL);
            SelectObject(hdc, buf);

            PatBlt(hdc, 0,tabHeight, p->clientRect.right, p->clientRect.bottom-tabHeight, WHITENESS);
            // TODO: Add any drawing code here...

            SetBkMode(hdc, OPAQUE);

            int y=tabHeight-p->winTop;
            //int index=-1;

            for (ItemList::const_iterator i=p->odrList.begin(); i!=p->odrList.end(); i++) {
                ODRRef odr=i->item;
                int iHeight=odr->getHeight();
                RECT ritem={0, y, p->clientRect.right, y+iHeight} ;
                y+=iHeight;
                //index++;


                if (ritem.bottom<tabHeight) continue;
                if (ritem.top>p->clientRect.bottom) continue;
                if (i==p->cursorPos) {
                    // focused item
                    int cursColor=(GetFocus()==hWnd)?0x800000:0x808080;
                    HBRUSH cur=CreateSolidBrush(cursColor);
                    SetTextColor(hdc, 0xffffff);
                    SetBkColor(hdc, cursColor);
                    FillRect(hdc, &ritem, cur);
                    DeleteObject(cur);
                    //DrawFocusRect(hdc, &ritem);
                } else {
                    //usual item
                    SetTextColor(hdc, 0x000000);
                    SetBkColor(hdc, 0xffffff);
                }
                odr->draw(hdc, ritem);
            }

            RECT rc = {0, 0, 100, 100};

            int titleBgnd=0x808080;
            HBRUSH tb=CreateSolidBrush(titleBgnd);
            SetBkColor(hdc, titleBgnd);
            SetTextColor(hdc, 0x000000);

            LPCTSTR t=p->title.c_str();
            DrawText(hdc, t, -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            rc.right=p->clientRect.right;
            FillRect(hdc, &rc, tb);
            DrawText(hdc, t, -1, &rc, DT_LEFT | DT_TOP);
            DeleteObject(tb);

            BitBlt(wnd, 0,0,p->clientRect.right, p->clientRect.bottom, hdc, 0,0, SRCCOPY);
            DeleteDC(hdc);
            DeleteObject(buf);

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
            si.fMask=SIF_PAGE | SIF_RANGE;
            si.nMax=600;
            si.nMin=0;
            si.nPage=height-tabHeight;

            SetScrollInfo(p->listScrollHWND, SB_CTL, &si, TRUE);
            
            break; 
        } 


    case WM_LBUTTONDOWN:
        {
            SHRGINFO    shrg;
            HMENU       hmenu;

            SetFocus(hWnd);
            if (!(p->moveCursorTo(LOWORD(lParam), HIWORD(lParam)))) break;
            InvalidateRect(p->getHWnd(), NULL, true);

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

    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        {
            InvalidateRect(p->getHWnd(), NULL, true);
            break;
        }
    case WM_KEYDOWN:
        {
            int vKey=(int)wParam;
            int lkeyData=lParam;
            if (lkeyData & 0x800000) break; //keyRelease 
            switch (vKey) {
                case VK_UP: if (p->cursorPos!=p->odrList.begin()) p->cursorPos--; break;
                case VK_DOWN: p->cursorPos++; if (p->cursorPos==p->odrList.end()) p->cursorPos--; break;
            }
            p->cursorFit();
            InvalidateRect(p->getHWnd(), NULL, true);

            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.nPos=p->winTop;
            si.fMask=SIF_POS;
            SetScrollInfo(p->listScrollHWND, SB_CTL, &si, TRUE);

            break;
        }
    case WM_VSCROLL:
        {
            int scrollCode=(int)LOWORD(wParam);
            int nPos=(int)HIWORD(wParam);

            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.fMask=SIF_ALL;

            GetScrollInfo(p->listScrollHWND, SB_CTL, &si);

            //TODO: flicker-free scrolling
            switch (scrollCode) {
                case SB_LINEDOWN:   
                    if (si.nPos+si.nPage+tabHeight>=si.nMax) si.nPos=si.nMax-si.nPage; 
                    else si.nPos+=tabHeight; 
                break;
                case SB_LINEUP:     
                    if (si.nPos-tabHeight<0) si.nPos=0; 
                    else si.nPos-=tabHeight; break;
                case SB_ENDSCROLL:  break;
                default:            si.nPos=nPos; break;
            }


            p->winTop=si.nPos;
            //TODO: flicker-free scrolling
            InvalidateRect(p->getHWnd(), NULL, true);

            si.fMask=SIF_POS;
            SetScrollInfo(p->listScrollHWND, SB_CTL, &si, TRUE);
            return true;

        }
    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


bool ListViewODR::moveCursorTo( int x, int y ) 
{
    y+=winTop-tabHeight;
    if (y<0) return false;

    int yTop=0;
    for (ItemList::iterator i=odrList.begin(); i!=odrList.end(); i++) {
        int yBot=i->yPos;

        if (yTop<=y) {
            if (yBot>y) {
                cursorPos=i;

                cursorFit();
                return true;
            }
        }
        yTop=yBot;
    }
    return false;
}

void ListViewODR::cursorFit() {
    if (cursorPos==odrList.end()) return;

    int yBot=cursorPos->yPos;
    int yTop=yBot - cursorPos->item->getHeight();
    // aligning
    if (yBot>winTop+clientRect.bottom-tabHeight) winTop=yBot-(clientRect.bottom-tabHeight);
    if (yTop<winTop) winTop=yTop;
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
    cursorPos=odrList.end();
}

const wchar_t * ListViewODR::getWindowTitle() const{
    return title.c_str();
}

ListViewODR::~ListViewODR() {}

const OwnerDrawRect * ListViewODR::getODR() const { return wt.get(); }

void ListViewODR::addODR( ODRRef odr, bool redraw ) 
{
    int lastY=(odrList.empty())? 0: odrList.back().yPos;
    lastY+=odr->getHeight();

    ItemData item={lastY, odr};
    odrList.push_back(item);

    SCROLLINFO si;
    si.cbSize=sizeof(SCROLLINFO);
    si.fMask=SIF_PAGE | SIF_RANGE;
    si.nMax=lastY;
    si.nMin=0;
    si.nPage=clientRect.bottom-tabHeight;

    if (!redraw) return;
    SetScrollInfo(listScrollHWND, SB_CTL, &si, TRUE);

    InvalidateRect(getHWnd(), NULL, true);
}

ATOM ListViewODR::windowClass=0;