#include "ServiceDiscovery.h"
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "../vs2005/ui/ui.h"

#include "ResourceContext.h"
#include "JabberStream.h"
#include "TabCtrl.h"
#include "Presence.h"

extern HINSTANCE			g_hInst;
extern int tabHeight;
extern HWND	g_hWndMenuBar;		// menu bar handle
extern ResourceContextRef rc;
extern ImgListRef skin;

//TODO: subclass combo box: clipboard, enter redirection

//////////////////////////////////////////////////////////////////////////
class GetDisco : public JabberDataBlockListener {
public:
    GetDisco(const std::string &jid, ServiceDiscovery::ref form);
    ~GetDisco(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc);

private:
    std::string jid;
    std::string idinfo;
    std::string iditems;
    ResourceContextRef rc;
    boost::weak_ptr<ServiceDiscovery> vf;
};

GetDisco::GetDisco(const std::string &jid, ServiceDiscovery::ref form){
    this->jid=jid;
    this->vf=form;
    idinfo="info#"; idinfo+=jid;
    iditems="items#"; iditems+=jid;
}

void GetDisco::doRequest(ResourceContextRef rc) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "get");
    JabberDataBlockRef qry=req.addChild("query", NULL);

    qry->setAttribute("xmlns","http://jabber.org/protocol/disco#info");
    req.setAttribute("id", idinfo);
    rc->jabberStream->sendStanza(req);

    qry->setAttribute("xmlns","http://jabber.org/protocol/disco#items");
    req.setAttribute("id", iditems);
    rc->jabberStream->sendStanza(req);
}

ProcessResult GetDisco::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    //VcardForm::ref vfRef=vf.lock();

    ServiceDiscovery::ref sd=vf.lock();

    if (block->getAttribute("id")==idinfo) {
        if (block->getAttribute("type")=="result") {
            if (sd) {
                sd->infoReply=block->findChildNamespace("query", "http://jabber.org/protocol/disco#info");
                SendMessage(sd->getHWnd(), WM_USER, 0,0);
            }
        } else {
            //TODO: error
        }
        return BLOCK_PROCESSED;
    }
    if (block->getAttribute("id")==iditems) {
        if (block->getAttribute("type")=="result") {
            if (sd) {
                sd->itemReply=block->findChildNamespace("query", "http://jabber.org/protocol/disco#items");
                SendMessage(sd->getHWnd(), WM_USER, 0,0);
            }
        } else {
            //TODO: error
        }
        return LAST_BLOCK_PROCESSED;
    }

    return BLOCK_REJECTED;
}

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
        if (GET_X_LPARAM(lParam) > p->width-2-2*skin->getElementWidth()) {
            p->go();
        }
        break;

    case WM_USER:
        p->parseResult();
        return 0;

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

ServiceDiscovery::ref ServiceDiscovery::createServiceDiscovery( HWND parent, ResourceContextRef rc, const std::string &jid ) {
    ServiceDiscovery::ref sd=ServiceDiscovery::ref(new ServiceDiscovery(parent));
    sd->thisRef=sd;
    sd->discoverJid(jid);
    sd->rc=rc;
    return sd;
}

void ServiceDiscovery::discoverJid( const std::string &jid ) {
    SendMessage(editWnd, WM_SETTEXT, 0, (LPARAM)utf8::utf8_wchar(jid).c_str());
}

void ServiceDiscovery::go() {
    wchar_t buf[1024];
    SendMessage(editWnd, WM_GETTEXT, 1024, (LPARAM) buf);
    std::string jid=utf8::wchar_utf8(std::wstring(buf));

    GetDisco *gd=new GetDisco(jid, thisRef.lock());
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(gd));

    gd->doRequest(rc);
}

void ServiceDiscovery::parseResult() {
    //parsing items
    if (!itemReply) return;
    ODRList *list=new ODRList();

    JabberDataBlockRefList::iterator i=itemReply->getChilds()->begin();
    while (i!=itemReply->getChilds()->end()) {
        JabberDataBlockRef item=*(i++);
        std::string &jid=item->getAttribute("jid");
        std::string &name=item->getAttribute("name");
        Contact::ref contact=Contact::ref(new Contact(jid, "", name));
        contact->status=presence::ONLINE;
        list->push_back(contact);
    }
    nodeList->bindODRList(ODRListRef(list));
    nodeList->notifyListUpdate(true);
}

ATOM ServiceDiscovery::windowClass=0;

