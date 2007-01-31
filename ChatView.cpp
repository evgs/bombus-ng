#include "ChatView.h"
#include <commctrl.h>
#include <windowsx.h>

extern HINSTANCE			g_hInst;
extern int tabHeight;
//////////////////////////////////////////////////////////////////////////
ATOM ChatView::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ChatView::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusCV");

    return RegisterClass(&wc);
}


////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateEditControl(HWND hwndParent) {

    HWND hWndEdit; 
    //TCITEM tie; 

    hWndEdit=CreateWindow(_T("EDIT"), _T("LOG"), 
        WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL
        | ES_MULTILINE , 
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
        hwndParent, NULL, g_hInst, NULL); 
    return hWndEdit;
}



//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ChatView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    HDC hdc;
    ChatView *p=(ChatView *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(ChatView *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            p->msgList=VirtualListView::ref(new VirtualListView(hWnd, std::string("Chat")));
            p->msgList->setParent(hWnd);
            p->msgList->showWindow(true);
            p->editWnd=DoCreateEditControl(hWnd);
            break;
        }

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        /*{

            RECT rc = {0, 0, 100, 100};

            SetBkMode(hdc, TRANSPARENT);
            LPCTSTR t=p->title.c_str();
            DrawText(hdc, t, -1, &rc, DT_CALCRECT | DT_LEFT | DT_TOP);
            DrawText(hdc, t, -1, &rc, DT_LEFT | DT_TOP);
        }*/

        // TODO: Add any drawing code here...

        EndPaint(hWnd, &ps);
        break;

    case WM_SIZE: 
        { 
            HDWP hdwp; 
            RECT rc; 

            int height=GET_Y_LPARAM(lParam);
            int ySplit=height-p->editHeight;
            // Calculate the display rectangle, assuming the 
            // tab control is the size of the client area. 
            SetRect(&rc, 0, 0, 
                GET_X_LPARAM(lParam), ySplit ); 

            // Size the tab control to fit the client area. 
            hdwp = BeginDeferWindowPos(2);

            /*DeferWindowPos(hdwp, dropdownWnd, HWND_TOP, 0, 0, 
            GET_X_LPARAM(lParam), 20, 
            SWP_NOZORDER 
            ); */


            DeferWindowPos(hdwp, p->msgList->getHWnd(), HWND_TOP, 0, tabHeight, 
                GET_X_LPARAM(lParam), ySplit-tabHeight, 
                SWP_NOZORDER 
                );
            /*DeferWindowPos(hdwp, rosterWnd, HWND_TOP, 0, tabHeight, 
            GET_X_LPARAM(lParam), height-tabHeight, 
            SWP_NOZORDER 
            );*/

            DeferWindowPos(hdwp, p->editWnd, NULL, 0, ySplit+1, 
                GET_X_LPARAM(lParam), height-ySplit-1, 
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

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ChatView::ChatView( HWND parent, Contact::ref contact ) 
{
    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;

    TEXTMETRIC txm;
    GetTextMetrics(NULL, &txm);
    editHeight=txm.tmHeight*4;

    this->contact=contact;

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("chat"), WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);

}

//const wchar_t * ChatView::getWindowTitle() const{  return TEXT("stub"); }

ChatView::~ChatView() {}

const OwnerDrawRect * ChatView::getODR() const { return contact.get(); }

void ChatView::addMessage(const std::string & msg) {
    /*std::wstring umsg=utf8::utf8_wchar(msg);
    ODRRef r=ODRRef(new IconTextElementContainer(umsg, -1));*/
    ODRRef r=ODRRef(new MessageElement(msg));
    msgList->addODR(r, true);
}
ATOM ChatView::windowClass=0;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////

void MessageElement::init() {
    HDC tmp=CreateCompatibleDC(NULL);
    RECT r={0,0,240,10};
    DrawText(tmp, getText(), -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP |DT_WORDBREAK);
    width=r.right;
    height=r.bottom;  
    DeleteDC(tmp);
}

void MessageElement::draw(HDC hdc, RECT &rt) const {
    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, getText(), -1, &rt, DT_LEFT | DT_TOP | DT_WORDBREAK);
}

int MessageElement::getWidth() const { return width;}
int MessageElement::getHeight() const { return height; }

MessageElement::MessageElement(const std::string &str) {
    this->wstr=utf8::utf8_wchar(str);
    init();
}

const wchar_t * MessageElement::getText() const { return wstr.c_str(); }

int MessageElement::getColor() const { return 0; }