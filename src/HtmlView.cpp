#include "HtmlView.h"

#include <sipapi.h>
#include <htmlctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "utf8.hpp"
#include "basetypes.h"

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
            SendMessage(hwndHTML, DTM_ADDTEXT, FALSE, 
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

    case WM_NOTIFY:
        //if (wParam!=IDC_HTMLVIEW) break;
        {
            NM_HTMLVIEW* pnm = (NM_HTMLVIEW*)lParam;

            switch(pnm->hdr.code) 
            {
            case NM_INLINE_IMAGE:
                {
                    DWORD cookie=pnm->dwCookie;
                    HBITMAP bmp=p->getImage(pnm->szTarget, pnm->dwCookie);
                    p->setImage(bmp, cookie);
                    return TRUE;
                } // end case NM_INLINE_IMAGE:
            case NM_HOTSPOT:
                {
                    if (pnm->szTarget!=NULL)
                        p->onHotSpot((LPCSTR)pnm->szTarget, (LPCSTR)pnm->szData);
                    return TRUE;
                }
            } // end switch(pnm->hdr.code)
        } // End case IDC_HTMLVIEW:
        break;

    case WM_USER:
        {
            p->onWmUserUpdate();
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

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("HtmlView"), WS_CHILD | WS_VISIBLE,
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

void HtmlView::onWmUserUpdate() {}

HBITMAP HtmlView::getImage( LPCTSTR url, DWORD cookie ) 
{
    return NULL;
}

StringMapRef HtmlView::splitHREFtext( LPCSTR ht ) {
    std::string key;

    std::string buf;

    StringMap *m=new StringMap();

    char c;
    if (ht){
        while ((c=(char)(*ht++))) {
            switch (c) {
            case '+': buf+=' '; break;
            case '=': key=buf; buf.clear(); break;
            case '&': m->operator [](key)=buf; buf.clear(); break; 
            case '%': { 
                char c1=(char)(*ht++)-'0'; if (c1>9) c1+='0'-'A'+10;
                char c2=(char)(*ht++)-'0'; if (c2>9) c2+='0'-'A'+10;
                char c=c1<<4 | c2;
                if (c!=0x0d) buf+=c;
                break;
                      }
            default:
                buf+=c;
            }
        }
        m->operator [](key)=buf; 
    }
    return StringMapRef(m);
}

void HtmlView::addText( const char *text ) {  
    SendMessage(htmlHWnd, DTM_ADDTEXT, FALSE, (LPARAM) text);
}
void HtmlView::addText( const std::string &text ) { addText( (text.c_str()) ); }
void HtmlView::addText( const wchar_t *text ) {
    SendMessage(htmlHWnd, DTM_ADDTEXT, FALSE, (LPARAM) (utf8::wchar_utf8(std::wstring(text))).c_str());
}


void HtmlView::startHtml() {
    SendMessage(htmlHWnd, DTM_CLEAR, 0, 0);
    addText("<HTML>"
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">"
        "<TITLE>form</TITLE><BODY><P>");
}


void HtmlView::addImg( const wchar_t *src ) {
    addText("<IMG SRC=\"");
    addText(src);
    addText("\"><BR>");
}

void HtmlView::endForm() { addText("</form>"); }

void HtmlView::endHtml() {
    addText("</BODY></HTML>");
    SendMessage(htmlHWnd, DTM_ENDOFSOURCE, 0, (LPARAM)NULL);
}

void HtmlView::button( const std::string &label ) {
    addText("<input type=\"submit\" value=\"");
    addText(label);
    addText("\"/>");
}

void HtmlView::button( const char *name, const std::string &label ) {
    addText("<input type=\"button\" name=\"");
    addText(name);
    addText("\" value=\"");
    addText(label);
    addText("\">");
}
void HtmlView::textBox( const char *name, const std::string &label, const std::string &value ) {
    addText(label);
    addText(": <BR><input type=\"text\" name=\"");
    addText(name);
    addText("\" value=\"");
    addText(value);
    addText("\"><BR>");
}

void HtmlView::url( const std::string &label, const std::string &url ) {
    addText(label);
    addText(": <A HREF=\"");
    addText(url);
    addText("\">");
    addText(url);
    addText("</A><BR>");
}

void HtmlView::textConst( const std::string &label, const std::string &value ) {
    addText(label);
    addText(": ");
    addText(value);
    addText("<BR>");
}

void HtmlView::textML( const char *name, const std::string &label, const std::string &value ) {
    addText(label);
    addText(": <BR><textarea rows=\"6\" cols=\"20\" name=\"");
    addText(name);
    addText("\">");
    addText(value);
    addText("</textarea><BR>");
}

void HtmlView::beginForm( const char *name, const char *action ) {
    addText("<form name=\"");
    addText(name);
    addText("\" action=\"");
    addText(action);
    addText("\" method=\"post\" accept-charset=\"UTF-8\">");
}

void HtmlView::setImage( HBITMAP bmp, DWORD cookie ) {
    if (!bmp) {
        SendMessage(htmlHWnd, DTM_IMAGEFAIL, 0, cookie);
        return;
    }

    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);

    INLINEIMAGEINFO      imgInfo;
    imgInfo.dwCookie = cookie;
    imgInfo.iOrigHeight = bm.bmHeight;
    imgInfo.iOrigWidth = bm.bmWidth;
    imgInfo.hbm = bmp;    
    imgInfo.bOwnBitmap = FALSE;
    SendMessage(htmlHWnd, DTM_SETIMAGE, 0, (LPARAM)&imgInfo);   
}

ATOM HtmlView::windowClass=0;
HINSTANCE HtmlView::htmlViewInstance=0;
