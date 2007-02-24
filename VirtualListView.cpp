#include "VirtualListView.h"

#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "utf8.hpp"

extern HINSTANCE			g_hInst;
extern int tabHeight;

ATOM VirtualListView::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = VirtualListView::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = NULL;//(HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusVLV");

    return RegisterClass(&wc);
}

LRESULT CALLBACK VirtualListView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    VirtualListView *p=(VirtualListView *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(VirtualListView *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            //p->listScrollHWND=CreateWindow(_T("SCROLLBAR"), NULL, 
            //    SBS_VERT | WS_VISIBLE | WS_CHILD,
            //    0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
            //    hWnd, NULL, g_hInst, NULL); 

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

            PatBlt(hdc, 0, 0, p->clientRect.right, p->clientRect.bottom, WHITENESS);
            // TODO: Add any drawing code here...

            SetBkMode(hdc, OPAQUE);

            int y=-p->winTop;
            //int index=-1;

            if (p->odrlist.get()) {
                ODRList::const_iterator i=p->odrlist->begin();
                while (i!=p->odrlist->end()) {
                    ODRRef odr=*i; i++;

                    bool focused = (odr.get()==p->cursorPos.get());

                    int iHeight=odr->getHeight();
                    RECT ritem={0, y, p->clientRect.right, y+iHeight} ;
                    y+=iHeight;
                    //index++;

                    if (ritem.bottom < 0) continue;
                    if (ritem.top > p->clientRect.bottom) continue;
                    if (focused) {
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
                        SetTextColor(hdc, odr->getColor());
                        SetBkColor(hdc, 0xffffff);
                    }
                    odr->draw(hdc, ritem);
                }
            }

            RECT rc = {0, 0, 100, 100};

            /*int titleBgnd=0x808080;
            HBRUSH tb=CreateSolidBrush(titleBgnd);
            SetBkColor(hdc, titleBgnd);
            SetTextColor(hdc, 0x000000);

            LPCTSTR t=p->title.c_str();
            DrawText(hdc, t, -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            rc.right=p->clientRect.right;
            FillRect(hdc, &rc, tb);
            DrawText(hdc, t, -1, &rc, DT_LEFT | DT_TOP);
            DeleteObject(tb);*/

            BitBlt(wnd, 0,0,p->clientRect.right, p->clientRect.bottom, hdc, 0,0, SRCCOPY);
            DeleteDC(hdc);
            DeleteObject(buf);

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

            /*hdwp = BeginDeferWindowPos(1);

            DeferWindowPos(hdwp, p->listScrollHWND, HWND_TOP, width-SCROLLWIDTH, tabHeight, 
            SCROLLWIDTH, height-tabHeight, 
            SWP_NOZORDER 
            );
            EndDeferWindowPos(hdwp); */

            p->cursorFit();
            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.fMask=SIF_PAGE |SIF_POS;
            si.nPage=height;
            si.nPos=p->winTop;

            SetScrollInfo(p->thisHWnd, SB_VERT, &si, TRUE);

            break; 
        } 


    case WM_EXITMENULOOP:
        {
            p->releaseContextMenu();
            break;
        }
    case WM_LBUTTONDOWN:
        {
            SetFocus(hWnd);
            if (!(p->moveCursorTo(LOWORD(lParam), HIWORD(lParam)))) break;
            InvalidateRect(p->getHWnd(), NULL, true);

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
        }
    case WM_LBUTTONDBLCLK:
        {
            if (!(p->moveCursorTo(LOWORD(lParam), HIWORD(lParam)))) break;
            InvalidateRect(p->getHWnd(), NULL, true);
            p->eventOk();
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
            if (lkeyData & 0x80000000) break; //keyRelease 
            switch (vKey) {
            case VK_UP:
                p->moveCursor(-1);
                break;

            case VK_DOWN: 
                p->moveCursor(1);
                break;

            case VK_RETURN:
                if (lkeyData &0xc0000000) break;
                p->eventOk();
            }
            p->cursorFit();
            InvalidateRect(p->getHWnd(), NULL, true);

            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.nPos=p->winTop;
            si.fMask=SIF_POS;
            SetScrollInfo(p->thisHWnd, SB_VERT, &si, TRUE);

            break;
        }
    case WM_VSCROLL:
        {
            int scrollCode=(int)LOWORD(wParam);
            int nPos=(int)HIWORD(wParam);

            SCROLLINFO si;
            si.cbSize=sizeof(SCROLLINFO);
            si.fMask=SIF_ALL;

            GetScrollInfo(p->thisHWnd, SB_VERT, &si);

            //TODO: flicker-free scrolling
            switch (scrollCode) {
            case SB_LINEDOWN:   si.nPos+=tabHeight; break;
            case SB_LINEUP:     si.nPos-=tabHeight; break;
            case SB_ENDSCROLL:  break;
            case SB_PAGEUP:     si.nPos-=si.nPage;  break;
            case SB_PAGEDOWN:   si.nPos+=si.nPage;  break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION: si.nPos=si.nTrackPos; break;
            //default:            si.nPos=si.nTrackPos; break;
            }


            if (si.nPos<0) si.nPos=0; 
            if (si.nPos+(int)si.nPage >= si.nMax) si.nPos=si.nMax-si.nPage; 

            p->winTop= si.nPos;

            //TODO: flicker-free scrolling
            InvalidateRect(p->getHWnd(), NULL, true);

            si.fMask=SIF_POS;
            SetScrollInfo(p->thisHWnd, SB_VERT, &si, TRUE);
            return true;

        }

    case WM_COMMAND: 
        {
            int cmdId=LOWORD(wParam);
            p->OnCommand(cmdId, lParam);
            break;
        }

    case WM_USER+1:
        {
            //TODO: create interconnecting message object to avoid pointers
            ODRListRef r=ODRListRef((ODRList *)lParam);
            p->bindODRList(r); //ÀÕÒÓÍÃ ¹5

            //p->bindODRList(ODRSet::ref((ODRSet *)lParam)); //ÀÕÒÓÍÃ ¹3
            p->notifyListUpdate(true);
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


bool VirtualListView::moveCursorTo( int x, int y ) 
{
    y+=winTop;
    if (y<0) return false;

    int yTop=0;

    for (ODRList::const_iterator i = odrlist->begin(); i!=odrlist->end(); i++ ) {
        int yBot=yTop+i->get()->getHeight();

        if (yTop<=y) {
            if (yBot>y) {
                cursorPos=*i;

                cursorFit();
                return true;
            }
        }
        yTop=yBot;
    }
    return false;
}

bool VirtualListView::moveCursorEnd() {
    if (odrlist.get()==NULL) return false;
    cursorPos=odrlist->back();
    cursorFit();
    return true;
}

void VirtualListView::cursorFit() {
    if (!cursorPos) return;

    int yTop=0;
    for (ODRList::const_iterator i = odrlist->begin(); i!=odrlist->end(); i++) {
        int yBot=yTop+ i->get()->getHeight();

        if ( (*i).get()==cursorPos.get() ) {
            if (yBot>winTop+clientRect.bottom) winTop=yBot-(clientRect.bottom);
            if (yTop<winTop) winTop=yTop;
            break;
        }
        yTop=yBot;
    }
}


VirtualListView::VirtualListView() { /*init(); - MUST NOT be called before setting up parentHWnd */ }

void VirtualListView::init() {
    BOOST_ASSERT(parentHWnd);

    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");
    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("ListView"), WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parentHWnd, NULL, g_hInst, (LPVOID)this);
    wrapList=true;
    winTop=0;
}

VirtualListView::VirtualListView( HWND parent, const std::string & title ) {
    parentHWnd=parent;
    init();

    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, 0));
    cursorPos=ODRRef();//odrlist->front();
    odrlist=ODRListRef(new ODRList());
}

const wchar_t * VirtualListView::getWindowTitle() const{
    return title.c_str();
}

VirtualListView::~VirtualListView() {}

const ODR * VirtualListView::getODR() const { return wt.get(); }

void VirtualListView::notifyListUpdate( bool redraw ) {
    if (!odrlist) return;
    int lastY=0;
    for (ODRList::const_iterator i = odrlist->begin(); i!=odrlist->end(); i++) {
        lastY+= i->get()->getHeight();
    }

    SCROLLINFO si;
    si.cbSize=sizeof(SCROLLINFO);
    si.fMask=SIF_PAGE | SIF_RANGE;
    si.nPage=clientRect.bottom;
    si.nMin=0;
    si.nMax=lastY;

    if (!redraw) return;
    SetScrollInfo(thisHWnd, SB_VERT, &si, TRUE);

    InvalidateRect(getHWnd(), NULL, true);
}

void VirtualListView::eventOk() { }

void VirtualListView::moveCursor( int direction ) {
    for (ODRList::const_iterator i=odrlist->begin(); i!=odrlist->end(); i++) {
        ODRRef r=*i;
        if (r.get()==cursorPos.get()) {
            if (direction>0) {
                i++;
                if (i==odrlist->end()) i=odrlist->begin();
            } else {
                if (i==odrlist->begin()) i=odrlist->end();
                i--;
            }
            cursorPos=*i;
            return;
        }
    }
}

void VirtualListView::addODR( ODRRef odr, bool redraw ) {
    odrlist->push_back(odr);
    notifyListUpdate(redraw);
}

HMENU VirtualListView::getContextMenu() { return NULL; }
void VirtualListView::releaseContextMenu() {};
void VirtualListView::OnCommand( int cmdId, LONG lParam ) {};

ATOM VirtualListView::windowClass=0;

