#include "ServiceDiscovery.h"
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include "../vs2005/ui/ui.h"

#include "ResourceContext.h"
#include "JabberStream.h"
#include "TabCtrl.h"
#include "Presence.h"
#include "VcardForm.h"

#include "DlgMucJoin.h"
//#include "Contact.h"
int identifyTransport(const std::string &jid);

extern HINSTANCE			g_hInst;
extern int tabHeight;
extern HWND	g_hWndMenuBar;		// menu bar handle
extern ResourceContextRef rc;
extern ImgListRef skin;
extern TabsCtrlRef tabs;

long WINAPI ComboSubClassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) { 
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

                //AppendMenu(hmenu, (smileParser->hasSmiles())? MF_STRING : MF_STRING | MF_GRAYED, WM_USER, TEXT("Add Smile"));
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

                //if (cmdId==WM_USER) SmileBox::showSmileBox(hWnd, pt.x, pt.y, smileParser);

                DestroyMenu(hmenu);

                if (cmdId>0) PostMessage(hWnd, cmdId, 0, 0);

                return 0;
            }
            break;
        }

    /*case WM_KEYDOWN:
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
        */
    } 
    return CallWindowProc(OldWndProc,hWnd,msg,wParam,lParam); 
}

////////////////////////////////////////////////////////////////////////////////

HWND WINAPI DoCreateComboControl(HWND hwndParent) {

    HWND hwndCombo; 
    //TCITEM tie; 

    hwndCombo=CreateWindow(_T("COMBOBOX"), NULL, 
        WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL
        | CBS_DROPDOWN | CBS_LOWERCASE , 
        0, 0, 32 /*CW_USEDEFAULT*/, CW_USEDEFAULT, 
        hwndParent, NULL, g_hInst, NULL); 

    POINT pt;
    pt.x=6; pt.y=6;
    HWND hWndEdit = ChildWindowFromPoint(hwndCombo, pt); 

    WNDPROC OldEditWndProc = (WNDPROC)SetWindowLong(hWndEdit, GWL_WNDPROC,  (LONG)ComboSubClassProc); 
    SetWindowLong(hWndEdit, GWL_USERDATA, (LONG)OldEditWndProc);

    return hwndCombo;
}

class DiscoListView : public VirtualListView {
public:
    DiscoListView( HWND parent, const std::string & title );
    void eventOk();
    boost::weak_ptr<ServiceDiscovery> serviceDiscovery;
};

void DiscoListView::eventOk() {
    ServiceDiscovery::ref sd=serviceDiscovery.lock();
    if (!sd) return;
    
    DiscoItem::ref c=boost::dynamic_pointer_cast<DiscoItem> (cursorPos);
    if (c) {
        sd->discoverJid(c->jid, c->node);
        sd->go();
        return;
    }

    DiscoCommand::ref dc=boost::dynamic_pointer_cast<DiscoCommand> (cursorPos);
    if (dc) {
        switch (dc->cmdId) {
        case DiscoCommand::BACK: 
            sd->back();
            return;
        case DiscoCommand::VCARD:
            sd->vcard();
            return;
        case DiscoCommand::JOINGC:
            sd->joingc();
        }
    }
    VirtualListView::eventOk();
}

DiscoListView::DiscoListView( HWND parent, const std::string & title ) {
    parentHWnd=parent;
    init();

    SetParent(thisHWnd, parent);

    this->title=utf8::utf8_wchar(title);

    wt=WndTitleRef(new WndTitle(this, 0));
    cursorPos=ODRRef();//odrlist->front();
    odrlist=ODRListRef(new ODRList());
}

////////////////////////////////////////////////////////////////////////////////
DiscoItem::DiscoItem( const std::string &jid, const std::string &node, const std::string &name ) 
{
    this->jid=jid;
    this->node=node;
    this->name=name;

    this->iconIndex=(node.empty())? 
        identifyTransport(jid) : 
        icons::ICON_COLLAPSED_INDEX;

    this->wstr=utf8::utf8_wchar((name.empty())? jid : name);
    init();
}
////////////////////////////////////////////////////////////////////////////////
DiscoCommand::DiscoCommand(std::wstring cmdName, int icon, int cmdId) {
    this->iconIndex=icon;
    this->wstr=cmdName;
    this->cmdId=(DiscoCmds)cmdId;
    init();
}

int DiscoCommand::getColor() const { return 0xff0000; }
//TODO: subclass combo box: clipboard, enter redirection

//////////////////////////////////////////////////////////////////////////
class GetDisco : public JabberDataBlockListener {
public:
    GetDisco(const std::string &jid, const std::string &node, ServiceDiscovery::ref form);
    ~GetDisco(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc);

private:
    std::string jid;
    std::string node;
    std::string idinfo;
    std::string iditems;
    ResourceContextRef rc;
    boost::weak_ptr<ServiceDiscovery> vf;
};

GetDisco::GetDisco( const std::string &jid, const std::string &node, ServiceDiscovery::ref form ) 
{
    this->jid=jid;
    this->vf=form;
    this->node=node;

    idinfo="info#"; idinfo+=jid;
    iditems="items#"; iditems+=jid;
}

void GetDisco::doRequest(ResourceContextRef rc) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "get");
    JabberDataBlockRef qry=req.addChild("query", NULL);

    qry->setAttribute("xmlns","http://jabber.org/protocol/disco#info");
    if (node.length()) qry->setAttribute("node", node);

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

            //p->nodeList=VirtualListView::ref(new VirtualListView(hWnd, std::string("disco")));
            DiscoListView * dlv = new DiscoListView(hWnd, std::string("disco"));
            //dlv->serviceDiscovery=p->thisRef;

            p->nodeList=VirtualListView::ref(dlv);
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
            break;
        }
        if (GET_X_LPARAM(lParam) > p->width-2-2*skin->getElementWidth()) {
            //reset back-history
            while (!p->nodes.empty()) p->nodes.pop();
            p->nodeList->bindODRList(ODRListRef());

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
    (boost::dynamic_pointer_cast<DiscoListView> (sd->nodeList))->serviceDiscovery=sd;
    sd->discoverJid(jid, "");
    sd->rc=rc;
    return sd;
}

void ServiceDiscovery::discoverJid( const std::string &jid, const std::string &node ) 
{
    this->newNode=node;
    SendMessage(editWnd, WM_SETTEXT, 0, (LPARAM)utf8::utf8_wchar(jid).c_str());
}

void ServiceDiscovery::go() {
    wchar_t buf[1024];
    SendMessage(editWnd, WM_GETTEXT, 1024, (LPARAM) buf);
    std::string newJid=utf8::wchar_utf8(std::wstring(buf));

    GetDisco *gd=new GetDisco(newJid, this->newNode, thisRef.lock());
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(gd));

    DiscoNode node;
    node.jid=this->jid;
    node.node=this->node;
    node.subnodes=nodeList->getODRList();
    if (node.subnodes) if (node.subnodes->size()>0)
        nodes.push(node);

    this->jid=newJid;
    this->node=newNode;
    infoReply.reset();
    itemReply.reset();
    gd->doRequest(rc);
}

void ServiceDiscovery::back() {
    if (nodes.empty()) return;

    DiscoNode &node=nodes.top();
    nodeList->bindODRList(node.subnodes);
    this->jid=node.jid;
    //this->node=node.node;
    nodes.pop();
    discoverJid(this->jid, this->node);

    /*GetDisco *gd=new GetDisco(jid, thisRef.lock());
    rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(gd));
    gd->doRequest(rc);*/
    nodeList->notifyListUpdate(true);
}

void ServiceDiscovery::parseResult() {
    //parsing items
    ODRList *list=new ODRList();

    if (!nodes.empty()) list->push_back(DiscoCommand::ref(new DiscoCommand(L"..", icons::ICON_DISCO_BACK, DiscoCommand::BACK)));
    if (infoReply) {
        JabberDataBlockRefList::iterator i=infoReply->getChilds()->begin();
        while (i!=infoReply->getChilds()->end()) {
            JabberDataBlockRef feature=*(i++);
            if (feature->getTagName()!="feature") continue;
            std::string &var=feature->getAttribute("var");
            DiscoCommand *cmd=NULL;
            if (var=="jabber:iq:register") cmd=new DiscoCommand(L"Register", icons::ICON_REGISTER_INDEX, DiscoCommand::REGISTER);
            if (var=="jabber:iq:search") cmd=new DiscoCommand(L"Search", icons::ICON_SEARCH_INDEX, DiscoCommand::SEARCH);
            if (var=="vcard-temp") cmd=new DiscoCommand(L"vCard", icons::ICON_VCARD, DiscoCommand::VCARD);
            if (var=="http://jabber.org/protocol/muc") cmd=new DiscoCommand(L"Join conference", icons::ICON_GCJOIN_INDEX, DiscoCommand::JOINGC);
            //    ADD=6
            if (cmd) list->push_back(DiscoCommand::ref(cmd));
        }
        JabberDataBlockRef identity=infoReply->getChildByName("identity");
        if (identity) {
            if (identity->getAttribute("type")=="command-node" && 
                identity->getAttribute("category")=="automation") {
                    list->push_back(DiscoCommand::ref(new DiscoCommand(L"Execute", icons::ICON_AD_HOC, DiscoCommand::EXECUTE)));
            }
        }
    }

    if (itemReply) {
        JabberDataBlockRefList::iterator i=itemReply->getChilds()->begin();
        while (i!=itemReply->getChilds()->end()) {
            JabberDataBlockRef item=*(i++);
            std::string &jid=item->getAttribute("jid");
            std::string &name=item->getAttribute("name");
            std::string &node=item->getAttribute("node");
            DiscoItem::ref contact=DiscoItem::ref(new DiscoItem(jid, node, name));
            list->push_back(contact);
        }
    }

    nodeList->bindODRList(ODRListRef(list));
    nodeList->notifyListUpdate(true);
}

void ServiceDiscovery::vcard() {
    WndRef vc=VcardForm::createVcardForm(tabs->getHWnd(), jid, rc, false);
    tabs->addWindow(vc);
    tabs->switchByWndRef(vc);
}
void ServiceDiscovery::joingc() {
    DlgMucJoin::createDialog(tabs->getHWnd(), rc, jid);
}

ATOM ServiceDiscovery::windowClass=0;

