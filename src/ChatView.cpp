#include "ChatView.h"
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "../vs2005/ui/ui.h"

#include "ResourceContext.h"
#include "JabberStream.h"
#include "TabCtrl.h"
#include "ProcessMUC.h"

#include "Smiles.h"

extern HINSTANCE			g_hInst;
extern int tabHeight;
extern HWND	g_hWndMenuBar;		// menu bar handle
extern ResourceContextRef rc;
extern ImgListRef skin;
extern SmileParser *smileParser;

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



//////////////////////////////////////////////////////////////////////////
// real WndProc for edit box
long (WINAPI *EditWndProc)(HWND w,UINT msg,WPARAM wParam,LPARAM lParam); 

namespace editbox {
    static bool editBoxShifts=false;
}

long WINAPI EditSubClassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { 
    WNDPROC OldWndProc=(WNDPROC) GetWindowLong(hWnd, GWL_USERDATA);
    switch(msg) { 
    case WM_LBUTTONDOWN:
        {

            SHRGINFO    shrg;
            shrg.cbSize = sizeof(shrg);
            shrg.hwndClient = hWnd;
            shrg.ptDown.x = LOWORD(lParam);
            shrg.ptDown.y = HIWORD(lParam);
            shrg.dwFlags = SHRG_RETURNCMD /*| SHRG_NOANIMATION*/;

            if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) {

                DWORD sel=SendMessage(hWnd, EM_GETSEL, 0, 0);

                UINT paste = (IsClipboardFormatAvailable(CF_UNICODETEXT))?  MF_STRING : MF_STRING | MF_GRAYED;
                UINT cut = (LOWORD(sel)!=HIWORD(sel))? MF_STRING : MF_STRING | MF_GRAYED;
                UINT undo= (SendMessage(hWnd, EM_CANUNDO, 0, 0))? MF_STRING : MF_STRING | MF_GRAYED;;

                HMENU hmenu = CreatePopupMenu();
                if (hmenu==NULL) break;

                AppendMenu(hmenu, MF_STRING | MF_GRAYED, 0, TEXT("Add Smile"));
                AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hmenu, cut, WM_CUT, TEXT("Cut") );
                AppendMenu(hmenu, cut, WM_COPY, TEXT("Copy") );
                AppendMenu(hmenu, paste, WM_PASTE, TEXT("Paste") );
                AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hmenu, undo, EM_UNDO, TEXT("Undo") );

                POINT pt={LOWORD(lParam), HIWORD(lParam) };
                ClientToScreen(hWnd, &pt);

                int cmdId=TrackPopupMenuEx(hmenu,
                    TPM_BOTTOMALIGN | TPM_RETURNCMD,
                    pt.x, pt.y,
                    hWnd,
                    NULL);

                DestroyMenu(hmenu);

                if (cmdId>0) PostMessage(hWnd, cmdId, 0, 0);

                return 0;
            }
            break;
        }

    case WM_KEYDOWN:
        if (wParam==VK_CONTROL) editbox::editBoxShifts=true;
        if (wParam==VK_SHIFT)   editbox::editBoxShifts=true;
        break; 
    case WM_KEYUP:
        editbox::editBoxShifts=false;
        break;
    case WM_CHAR:
        if (wParam==VK_RETURN && !editbox::editBoxShifts) {
            PostMessage(GetParent(hWnd), WM_COMMAND, IDS_SEND, 0);
            return 0;
        }
        break;
    } 
    return CallWindowProc(OldWndProc,hWnd,msg,wParam,lParam); 
}

////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateEditControl(HWND hwndParent) {

    HWND hWndEdit; 
    //TCITEM tie; 

    hWndEdit=CreateWindow(_T("EDIT"), NULL, 
        WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL
        | ES_MULTILINE , 
        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
        hwndParent, NULL, g_hInst, NULL); 

    WNDPROC OldEditWndProc = (WNDPROC)SetWindowLong(hWndEdit, GWL_WNDPROC,  (LONG)EditSubClassProc); 
    SetWindowLong(hWndEdit, GWL_USERDATA, (LONG)OldEditWndProc);

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
            p->msgList->wrapList=false;
            p->msgList->colorInterleaving=true;

            p->editWnd=DoCreateEditControl(hWnd);
            p->calcEditHeight();

            p->msgList->bindODRList(p->contact->messageList);
            break;
        }

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        {
            //p->contact->nUnread=0;
            RECT rc = {0, 0, 200, tabHeight};
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, p->contact->getColor());
            p->contact->draw(hdc, rc);

            skin->drawElement(hdc, icons::ICON_CLOSE, p->width-2-skin->getElementWidth(), 0);

            

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

            int ySplit=height-p->editHeight;

            p->calcEditHeight();

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

    case WM_COMMAND: 
        {
            if (wParam==IDS_SEND) {
                p->sendJabberMessage();
            }
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

    case WM_LBUTTONDOWN:
        SetFocus(hWnd);
        if ((GET_Y_LPARAM(lParam))>tabHeight) break;
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

ChatView::ChatView( HWND parent, Contact::ref contact ) 
{
    BOOST_ASSERT(parent);

    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;

    //TEXTMETRIC txm;
    //GetTextMetrics(NULL, &txm); //TODO - hdc must not be NULL
    //editHeight=txm.tmHeight*4;
    editHeight=60;

    this->contact=contact;

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("chat"), WS_CHILD |WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, g_hInst, (LPVOID)this);

}

//const wchar_t * ChatView::getWindowTitle() const{  return TEXT("stub"); }

ChatView::~ChatView() {
    DestroyWindow(editWnd);
}

const ODR * ChatView::getODR() const { return contact.get(); }

void ChatView::addMessage(const std::string & msg) {
    /*std::wstring umsg=utf8::utf8_wchar(msg);
    ODRRef r=ODRRef(new IconTextElementContainer(umsg, -1));*/
    ODRRef r=ODRRef(new MessageElement(msg));
    msgList->addODR(r, true);
}

void ChatView::showWindow( bool show ) {
    Wnd::showWindow(show);

    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE;
    tbbi.fsState = (show)? TBSTATE_ENABLED : TBSTATE_HIDDEN;

    ::SendMessage (g_hWndMenuBar, TB_SETBUTTONINFO, IDS_SEND, (LPARAM)&tbbi);

    if (show) contact->nUnread=0;

    if (show) msgList->notifyListUpdate(true);
    //if (show) InvalidateRect(msgList->getHWnd(), NULL, false);

    if (show) SetFocus(editWnd);

}

void ChatView::redraw(){
    InvalidateRect(getHWnd(), NULL, TRUE);
    msgList->notifyListUpdate(true);
    //InvalidateRect(msgList->getHWnd(), NULL, false);
}

void ChatView::moveUnread() {
    msgList->moveCursorEnd();
}
ATOM ChatView::windowClass=0;

//////////////////////////////////////////////////////////////////////////
void ChatView::sendJabberMessage() {
    wchar_t buf[1024];
    int len=SendMessage(editWnd, WM_GETTEXT, 1024, (LPARAM) buf);
    if (len==0) return;
    std::string body=utf8::wchar_utf8(buf);

    Message::ref msg=Message::ref(new Message(body, "", Message::SENT));
    bool muc=boost::dynamic_pointer_cast<MucRoom>(contact);

    if (!muc) contact->messageList->push_back(msg);

    //redraw();
    msgList->moveCursorEnd();

    //if (!muc) msgList->moveCursorEnd();

    std::string to=(muc)?contact->jid.getBareJid() : contact->jid.getJid();
    JabberDataBlockRef out=msg->constructStanza(to);
    if (muc) out->setAttribute("type","groupchat");

    //Reset form
    rc->jabberStream->sendStanza(*out);

    buf[0]=0;
    SendMessage(editWnd, WM_SETTEXT, 1024, (LPARAM) buf);
}

void ChatView::calcEditHeight() {
    RECT rect;
    GetWindowRect(editWnd, &rect);
}

//////////////////////////////////////////////////////////////////////////
class FontMetricCache {
public: 
    FontMetricCache();
    ~FontMetricCache();
    inline int getWidth(HDC hdc, wchar_t chr);
    int getHeight() {return height; };
private:
    LPINT page[256];
    int height;
};

FontMetricCache::FontMetricCache(){
    memset(page, 0, sizeof(page));
}

FontMetricCache::~FontMetricCache(){
    for (int i=0; i<256; i++) {
        if (page[i]) delete page[i];
    }
}

int FontMetricCache::getWidth(HDC hdc, wchar_t chr){
    byte pn=(chr>>8);
    if (!page[pn]) {
        page[pn]=new int[256];
        GetCharWidth32(hdc, chr & 0xff00, chr | 0xff, page[pn]);

        TEXTMETRIC metric;
        GetTextMetrics(hdc, &metric);
        height=metric.tmHeight;
    }
    return page[pn][chr&0xff];
}

FontMetricCache fmc;
//////////////////////////////////////////////////////////////////////////  
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////

void MessageElement::init() {
    //TODO: recalculate screen and scroller height after rendering, remove this prefetch
    HDC tmp=CreateCompatibleDC(NULL);
    RECT r={0,0,230,10}; //todo: fix width detection
    measure(tmp, r);
    DeleteDC(tmp);
    //height=10; width=10;
}

void MessageElement::measure(HDC hdc, RECT &rt) {
    fmc.getWidth(hdc, ' ');
    if (width==rt.right-rt.left) {rt.bottom=rt.top+height; return; }//already measured
    //if (width>220) { //debug code
    //    SetBkMode(hdc, TRANSPARENT);
    //}

    render(hdc, rt, true);
    width=rt.right-rt.left;
    height=rt.bottom-rt.top;  
}

void MessageElement::draw(HDC hdc, RECT &rt) const {
    render(hdc, rt, false);
}

void MessageElement::render( HDC hdc, RECT &rt, bool measure ) const{
    int ypos=rt.top; //fmc.getHeight();
    
    const wchar_t *end=getText();
    const wchar_t *lineBegin=end;
    const wchar_t *wordBegin=NULL;
    const wchar_t *smileEnd;

    int smileIndex=-1;


    int xpos=rt.left;
    int xbegin=xpos;
    int mw=rt.right;

    wchar_t c;
    do { 
        c=*end;
        switch (c) {
            case 0: break; //newline;
            case 0x0a: end++; break; //newline;

                //some word delimiters
            case ' ':
            case '-':
            case '(':
            case ')':
            case ':':
            case '/':
            case '.':
            case ',':
                wordBegin=end+1;
            default:
                smileEnd=end;
                smileIndex=smileParser->findSmile(&smileEnd);
                if (smileIndex>=0) {
                    if (!measure) ExtTextOut(hdc, xbegin, ypos, ETO_CLIPPED, &rt, lineBegin, end-lineBegin, NULL);
                    lineBegin=end=smileEnd; wordBegin=NULL; xbegin=xpos+smileParser->icons->getElementWidth();
                    if (!measure) smileParser->icons->drawElement(hdc, smileIndex, xpos, ypos);
                    xpos=xbegin;
                    continue;
                }

                xpos+=fmc.getWidth(hdc, c);
                if (xpos<mw) {
                    end++; continue; 
                } else if (wordBegin) end=wordBegin;
        }


        if (!measure) ExtTextOut(hdc, xbegin, ypos, ETO_CLIPPED, &rt, lineBegin, end-lineBegin, NULL);
        xbegin=rt.left;

        ypos+=fmc.getHeight(); xpos=rt.left; lineBegin=end; wordBegin=NULL; //newline
        if (!measure) if (ypos>=rt.bottom) break;
        //if (c) end++;
    } while (c);

    if (measure) rt.bottom=ypos;
}

int MessageElement::getWidth() const { return width;}
int MessageElement::getHeight() const { return height; }

MessageElement::MessageElement(const std::string &str) {
    this->wstr=utf8::utf8_wchar(str);
    init();
}

const wchar_t * MessageElement::getText() const { return wstr.c_str(); }

int MessageElement::getColor() const { return 0; }