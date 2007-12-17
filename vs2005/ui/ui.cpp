// ui.cpp : Defines the entry point for the application.
//

//#include "stdafx.h"

#include "ui.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "wmuser.h"
#include "Notify.h"

#include "LogPanel.h"
#include "Socket.h"
#include "CETLSSocket.h"
#include <string>
#include "JabberDataBlock.h"
#include "JabberStream.h"
#include "JabberAccount.h"
#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"
#include "HostFeatures.h"

#include "EntityCaps.h"
#include "Roster.h"

#include "ProcessMUC.h"
#include "MucBookmarks.h"

#include "DlgAbout.h"
#include "DlgAccount.h"
#include "DlgConfig.h"
#include "DlgStatus.h"
#include "DlgMucJoin.h"
#include "VirtualListView.h"
#include "ChatView.h"
#include "TabCtrl.h"
#include "XDataForm.h"
#include "VcardForm.h"
#include "ServiceDiscovery.h"
#include "LastActivity.h"

#include "Auth.h"

#include "Sysinfo.h"

#include "Image.h"
#include "Smiles.h"
#include "History.h"

#include "utf8.hpp"

#include "config.h"

#include "dnsquery.h"
#include "boost/format.hpp"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND		mainWnd;
HCURSOR     cursorWait;

//ListViewRef logWnd;
TabsCtrlRef tabs;

VirtualListView::ref odrLog;
RosterListView::ref rosterWnd;
ResourceContextRef rc;

ImgListRef skin;

SmileParser *smileParser;

std::wstring appRootPath;
std::wstring skinRootPath;
std::string appVersion;

int tabHeight;

int prepareAccount();
int initJabber();
void streamShutdown();
void Shell_NotifyIcon(bool show, HWND hwnd);

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UI));

	// Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) {
	//while (true) {
    //    if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {  Sleep(50); continue; }
    //    if (msg.message==WM_QUIT) break;
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
		}
	}
    Shell_NotifyIcon(false, NULL);
    Config::getInstance()->save();
	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BOMBUS));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
const std::string responseMd5Digest( const std::string &user, const std::string &pass, const std::string &realm, const std::string &digestUri, const std::string &nonce, const std::string cnonce);

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;
    TCHAR szTitle[MAX_LOADSTRING];		// title bar text
    TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name

    g_hInst = hInstance; // Store instance handle in our global variable

    // SHInitExtraControls should be called once during your application's initialization to initialize any
    // of the device specific controls such as CAPEDIT and SIPPREF.
    SHInitExtraControls();

    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
    LoadString(hInstance, IDC_UI, szWindowClass, MAX_LOADSTRING);

    //If it is already running, then focus on the window, and exit
    hWnd = FindWindow(szWindowClass, szTitle);	
    if (hWnd) 
    {
        // set focus to foremost child window
        // The "| 0x00000001" is used to bring any owned windows to the foreground and
        // activate them.
        SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
        return 0;
    } 

    wchar_t wbuf[1024];
    GetModuleFileName(hInstance, wbuf, sizeof(wbuf));
    appRootPath=wbuf;
    int namePos=appRootPath.find_last_of(_T("\\"))+1;
    appRootPath.erase(namePos, appRootPath.length()-namePos);

    wchar_t * skinRelPath;
    if (sysinfo::screenIsVGA()) {
        skinRelPath=TEXT("vga\\");
        tabHeight=34; //TODO: remove hardcode
    } else {
        skinRelPath=TEXT("qvga\\");
        tabHeight=18; //TODO: remove hardcode
    }
    skinRootPath=appRootPath+skinRelPath;

    LoadString(g_hInst, IDS_VERSION, wbuf, sizeof(wbuf));
    appVersion=utf8::wchar_utf8(wbuf);

    if (!MyRegisterClass(hInstance, szWindowClass)) 	return FALSE;

    mainWnd=hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return FALSE;
    }

    // When the main window is created using CW_USEDEFAULT the height of the menubar (if one
    // is created is not taken into account). So we resize the window after creating it
    // if a menubar is present
    if (g_hWndMenuBar)
    {
        RECT rc;
        RECT rcMenuBar;

        GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hWndMenuBar, &rcMenuBar);
        rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
        MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    Shell_NotifyIcon(true, hWnd);
    prepareAccount();

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    static SHACTIVATEINFO s_sai;
	
    switch (message) 
    {
        case WM_COMMAND:
            wmId    = LOWORD(wParam); 
            wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId) {
                case IDM_HELP_ABOUT: {
                    DlgAbout(g_hInst, hWnd);
				    break;
                }

                case ID_JABBER_OPTIONS:
                    DialogConfigMP(g_hInst, hWnd);
                    rc->myCaps=MyCapsRef(new MyCaps());
                    if (!rc->isLoggedIn()) break;
                    rc->roster->makeViewList();
                    break;

                case IDM_JABBER_ACCOUNT:
                    DialogAccountMP(g_hInst, hWnd, rc->account);
                    break;
                case IDM_EXIT:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;

                case IDM_JABBER_STATUS:
                    DlgStatus::createDialog(hWnd, rc);
                    break;

				/*case IDM_JABBER_ONLINE:
					initJabber();
					break;

                case IDM_JABBER_OFFLINE:
                    streamShutdown();
                    break;*/

                case ID_JABBER_JOINCONFERENCE:
                    if (rc->isLoggedIn())
                        DlgMucJoin::createDialog(hWnd, rc, "bombus@conference.jabber.ru");
                    break;

                case ID_TOOLS_MYVCARD:
                    if (rc->isLoggedIn()) {
                        WndRef vc=VcardForm::createVcardForm(tabs->getHWnd(), rc->account->getBareJid(), rc, true);
                        tabs->addWindow(vc);
                        tabs->switchByWndRef(vc);
                        break;
                    }

                case ID_TOOLS_SERVICEDISCOVERY:
                    {
                        ServiceDiscovery::ref disco=ServiceDiscovery::createServiceDiscovery(tabs->getHWnd(), rc, "jabber.ru", "", false);
                        tabs->addWindow(disco);
                        tabs->switchByWndRef(disco);
                    }
                    break;

				case IDM_JABBER_STREAMINFO:
                    Log::getInstance()->msg(
						rc->jabberStream->connection->getStatistics().c_str()
						);
					break;

				case IDM_WINDOWS_LOG:
                    if (!tabs->switchByWndRef(odrLog)) {
                        tabs->addWindow(odrLog);
                        tabs->switchByWndRef(odrLog);
                    }
                    break;


				/*case IDM_WINDOWS_ROSTER:
                    tabs->switchByWndRef(rosterWnd);
					break;*/
                case IDS_WINDOWS:
                    SendMessage(tabs->getHWnd(), WM_COMMAND, IDS_WINDOWS, 0);

                default:
                    if (tabs) tabs->fwdWMCommand(wmId);
                    //return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            cursorWait=LoadCursor(NULL, IDC_WAIT);
            //skin=ImgListRef(new ImgArray(TEXT("skin.png"), 8, 6));
            skin=ImgListRef(new Skin(TEXT("")));

            smileParser=new SmileParser();
            //skin->setGridSize(8, 6);

			//editWnd=DoCreateEditControl(hWnd);
            tabs=TabsCtrlRef(new MainTabs(hWnd));
            //tabs->setParent(hWnd);

            rosterWnd=RosterListView::ref(new RosterListView(tabs->getHWnd(), std::string("Roster")));
            tabs->addWindow(rosterWnd);

            { 
                odrLog = VirtualListView::ref(new VirtualListView(tabs->getHWnd(), std::string("Log")));
                tabs->addWindow(odrLog);
                LogPanel::bindLV(odrLog); 
                Log::getInstance()->msg(responseMd5Digest("juliet", "passwd", "example.com", "xmpp/example.com", "OA6MG9tEQGm2hh", "OA6MHXh6VqTrRk"));
            }

            /*#ifdef DEBUG
            {
                XDataForm::ref testXdata=XDataForm::createXDataForm(tabs->getHWnd(), "", rc);
                tabs->addWindow(testXdata);
                testXdata->formTest();
            }
            #endif*/

			//listWnd=logWnd;
			//dropdownWnd=DoCreateComboControl(hWnd);

            if (!SHCreateMenuBar(&mbi)) 
            {
                g_hWndMenuBar = NULL;
            }
            else
            {
                g_hWndMenuBar = mbi.hwndMB;
            }

            // Initialize the shell activate info structure
            memset(&s_sai, 0, sizeof (s_sai));
            s_sai.cbSize = sizeof (s_sai);
            break;
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);

            /*{
                skin->drawElement(hdc, 0, 0,0);                
            }*/
            
            // TODO: Add any drawing code here...
            
            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            CommandBar_Destroy(g_hWndMenuBar);
            PostQuitMessage(0);
            break;

		case WM_SIZE: 
			{ 
				HDWP hdwp; 
				
				int height=GET_Y_LPARAM(lParam);
								
				// Size the tab control to fit the client area. 
				hdwp = BeginDeferWindowPos(1);

				DeferWindowPos(hdwp, tabs->getHWnd(), HWND_TOP, 0, 0 /*tabHeight*/, 
					GET_X_LPARAM(lParam), height /* -tabHeight */, 
					SWP_NOZORDER 
					);
				
				EndDeferWindowPos(hdwp); 
				
				break; 
			} 

        case WM_ACTIVATE:
            // Notify shell of our activate message
            SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
            break;
        case WM_SETTINGCHANGE:
            SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
            break;

		//case WM_CTLCOLORLISTBOX:
        
        
        /*case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT: 
			{
                HDC hdc=(HDC) wParam;
                HWND hwnd=(HWND) lParam;
				SetBkColor(hdc, 0x000000);
				SetTextColor(hdc, 0xffffff);
				return (BOOL) GetStockObject(BLACK_BRUSH);
				break;
			}*/

        case SHELLNOTIFYICON:
            SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));            
            break;
        case WM_FORWARD_STANZA:
            {
                JabberDataBlockRef *rf=(JabberDataBlockRef *)lParam; //ÀÕÒÓÍÃ
                if (rf==NULL) break; 
                if (rc->jabberStanzaDispatcher2) rc->jabberStanzaDispatcher2->dispatchDataBlock(*rf);
                delete rf;
            }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////
/*class MTForwarder: public JabberDataBlockListener {
public:
    MTForwarder(ResourceContextRef rc) {
        //this->rc=rc;
    };
    ~MTForwarder(){};
    virtual const char * getTagName() const { return NULL; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult MTForwarder::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
    JabberDataBlockRef *p=new JabberDataBlockRef(block); //ÀÕÒÓÍÃ
    //rc->jabberStanzaDispatcher2->dispatchDataBlock(block);
    PostMessage(mainWnd, WM_FORWARD_STANZA, 0, (LPARAM)p);

    return BLOCK_PROCESSED;
}*/
//////////////////////////////////////////////////////////////
class GetRoster : public JabberDataBlockListener {
public:
    GetRoster() {}
	~GetRoster(){};
	virtual const char * getType() const{ return "result"; }
	virtual const char * getId() const{ return "roster"; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult GetRoster::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
	Log::getInstance()->msg("Roster arrived");

    rc->roster->blockArrived(block, rc); // forwarding to dispatch roster stanza
    
    //rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(new MTForwarder(rc)));
    //rc->jabberStanzaDispatcher2->addListener(rc->roster);

    rc->jabberStanzaDispatcherRT->addListener(rc->roster);

    rosterWnd->setIcon(rc->status);
    rc->sendPresence();

    //ProcessMuc::initMuc("bombus_im@conference.jabber.ru/evgs-ng","", rc);

	return LAST_BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class Version : public JabberDataBlockListener {
public:
    Version() {}
    ~Version(){};
    virtual const char * getType() const{ return "get"; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult Version::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef query=block->getChildByName("query");
    if (!query) return BLOCK_REJECTED;
    if (query->getAttribute("xmlns")!="jabber:iq:version") return BLOCK_REJECTED;

    Log::getInstance()->msg("version request ", block->getAttribute("from").c_str());

    std::string version=sysinfo::getOsVersion();


    JabberDataBlock result("iq");
    result.setAttribute("to", block->getAttribute("from"));
    result.setAttribute("type", "result");
    result.setAttribute("id", block->getAttribute("id"));
    result.addChild(query);

    query->addChild("name","Bombus-ng");
    query->addChild("version",::appVersion.c_str());
    query->addChild("os",version.c_str());

    rc->jabberStream->sendStanza(result);
    return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class Ping : public JabberDataBlockListener {
public:
    Ping() {}
    ~Ping(){};
    virtual const char * getType() const{ return "get"; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult Ping::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    JabberDataBlockRef ping=block->findChildNamespace("ping","urn:xmpp:ping");
    if (!ping) return BLOCK_REJECTED;

    Log::getInstance()->msg("Ping from ", block->getAttribute("from").c_str());

    JabberDataBlock pong("iq");
    pong.setAttribute("to", block->getAttribute("from"));
    pong.setAttribute("type", "result");
    pong.setAttribute("id", block->getAttribute("id"));

    rc->jabberStream->sendStanza(pong);
    return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class EntityTime : public JabberDataBlockListener {
public:
    EntityTime() {}
    ~EntityTime(){};
    virtual const char * getType() const{ return "get"; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult EntityTime::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){

    int rslt=BLOCK_REJECTED;
    JabberDataBlockRef time=block->findChildNamespace("time","urn:xmpp:time"); 
    if (time) {
        //Log::getInstance()->msg("Time query: ", block->getAttribute("from").c_str());

        PackedTime ct=strtime::getCurrentUtc();
        time->addChild("utc",strtime::toXep0080Time(ct).c_str());
        time->addChild("tzo",strtime::getLocalZoneOffset().c_str());
        rslt=BLOCK_PROCESSED;
    }

    JabberDataBlockRef query=block->findChildNamespace("query","jabber:iq:time"); 
	if (query) {
		//Log::getInstance()->msg("Time query: ", block->getAttribute("from").c_str());

		PackedTime ct=strtime::getCurrentUtc();
		query->addChild("utc",strtime::toIso8601(ct).c_str());
		query->addChild("display",strtime::toLocalDateTime(ct).c_str());
		query->addChild("tz",strtime::getLocalZoneOffset().c_str());
        rslt=BLOCK_PROCESSED;
	}

    if (rslt==BLOCK_REJECTED) return BLOCK_REJECTED;

    JabberDataBlock result("iq");
    result.setAttribute("to", block->getAttribute("from"));
    result.setAttribute("type", "result");
    result.setAttribute("id", block->getAttribute("id"));

    if (time) result.addChild(time);
    if (query) result.addChild(query);
    
    rc->jabberStream->sendStanza(result);
    return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class MessageRecv : public JabberDataBlockListener {
public:
	MessageRecv() {}
	~MessageRecv(){};
	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "message"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult MessageRecv::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    std::string from=block->getAttribute("from");
    const std::string & id=block->getAttribute("id");
    const std::string & body=block->getChildText("body");
    const std::string & subj=block->getChildText("subject");

    //JabberDataBlockRef xfwd=block->findChildNamespace("x","jabber:x:forward");
    //if (xfwd) {
	//  //old method
    //  std::string ofrom=xfwd->getChildText("from");
    //}

	std::string ofrom;
	std::string oto;
    // xep-0033 Extended stanza addressing
    JabberDataBlockRef addresses=block->findChildNamespace("addresses", "http://jabber.org/protocol/address");
    if (addresses && rc->myJid.getBareJid() == Jid(from).getBareJid() ) {
        JabberDataBlockRefList::iterator i=addresses->getChilds()->begin();
        while (i!=addresses->getChilds()->end()) {
            JabberDataBlockRef addr=*(i++);
            if (addr->getAttribute("type")=="ofrom") ofrom=addr->getAttribute("jid");
            if (addr->getAttribute("type")=="oto")   oto=addr->getAttribute("jid");
        }
    }

    if (ofrom.length()) from=ofrom;
    //StringRef orig=block->toXML();

    std::string nick;

    bool mucMessage= block->getAttribute("type")=="groupchat";
    Contact::ref c;
    if (mucMessage) {
        Jid roomNode;
        roomNode.setJid(from);

        MucGroup::ref roomGrp;
        roomGrp=boost::dynamic_pointer_cast<MucGroup> (rc->roster->findGroup(roomNode.getBareJid()));
        BOOST_ASSERT(roomGrp);
        if (!roomGrp) return BLOCK_PROCESSED;
        c=roomGrp->room;

        nick=roomNode.getResource();

    } else {
        c=rc->roster->getContactEntry(from);
        nick=c->getName();
    }

    //xep-085
    if (block->findChildNamespace("active", "http://jabber.org/protocol/chatstates")) {
        c->composing=false;
        c->acceptComposing=true;
    }
    if (block->findChildNamespace("paused", "http://jabber.org/protocol/chatstates")) {
        c->composing=false;
        c->acceptComposing=true;
    }
    if (block->findChildNamespace("composing", "http://jabber.org/protocol/chatstates")) {
        c->composing=true;
        c->acceptComposing=true;
    }
    //end xep-0085

    //xep-0184
    if (Config::getInstance()->delivered) {
        if (block->findChildNamespace("request","urn:xmpp:receipts")) {
            // reply
            JabberDataBlock delivered("message");
            delivered.setAttribute("to", from);
            delivered.setAttribute("id",id);
            delivered.addChildNS("received","urn:xmpp:receipts");
            rc->jabberStream->sendStanza(delivered);
        }

        if (block->findChildNamespace("received","urn:xmpp:receipts")) {
            c->messageDelivered(id);
        }
    }
    //end of xep-0184

    //processing jabber:x:event - deprecated xep-0022
    JabberDataBlockRef x=block->findChildNamespace("x","jabber:x:event");
    if (x) {
        std::string xid=x->getChildText("id");
        //delivery notifications
        if (x->getChildByName("delivered"))   if (Config::getInstance()->delivered) {
            if (xid.empty()) {
				/*if (boost::dynamic_pointer_cast<MucContact>c) {
					if (c->status==presence.OFFLINE) return;
				}*/
                JabberDataBlock delivered("message");
                delivered.setAttribute("to", from);
                JabberDataBlockRef x=delivered.addChildNS("x", "jabber:x:event");
                x->addChild("id", block->getAttribute("id").c_str() );
                x->addChild("delivered", NULL);
                rc->jabberStream->sendStanza(delivered);
            } else {
                c->messageDelivered(xid);
            }
        }
        //composing events
        bool composing = false;
        if (x->getChildByName("composing")) {
            c->acceptComposing=true;
            composing=body.empty();
        }
        c->composing=composing;

        if (composing) {
            //todo: repaint
        }
    }
    // end of xep-0022

    Message::ref msg;

    if (body.length() || subj.length() ) {
        //constructing message and raising message event
        Log::getInstance()->msg("Message from ", from.c_str()); 

        msg=Message::ref(new Message(body, nick, mucMessage, Message::INCOMING, Message::extractXDelay(block) ));

        Notify::PlayNotify();
    }

    ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(c).get());
    bool ascroll=(cv==NULL)? false: cv->autoScroll();

    if (msg) {
        c->nUnread++;
        c->messageList->push_back(msg);
        if (!mucMessage) History::getInstance()->appendHistory(c, msg);

        if (ascroll) /*if (cv)*/ {
            cv->moveEnd();
        }
        //tabs->switchByODR(c); 
    }

    //repainting
    if (rc->roster->needUpdateView) rc->roster->makeViewList();

    if (cv) { 
        if (IsWindowVisible(cv->getHWnd())) cv->redraw();
        InvalidateRect(tabs->getHWnd(), NULL, FALSE);
    }

    InvalidateRect(rosterWnd->getHWnd(),NULL, FALSE);

	return BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////
class PresenceRecv : public JabberDataBlockListener {
public:
    PresenceRecv() {}
    ~PresenceRecv(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return NULL; }
    virtual const char * getTagName() const { return "presence"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult PresenceRecv::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    std::string from=block->getAttribute("from");

    Contact::ref contact=rc->roster->getContactEntry(from);
    ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(contact).get());
    bool ascroll=(cv==NULL)? false: cv->autoScroll();

    contact->processPresence(block);

    rc->roster->makeViewList();


    if (ascroll) /*if(cv)*/ {
        cv->moveEnd();
    }
    if (cv) if (IsWindowVisible(cv->getHWnd())) cv->redraw();

    return BLOCK_PROCESSED;
}


class JabberStreamEvents : public JabberListener{
public:
    JabberStreamEvents(ResourceContextRef rc) {this->rc=rc;}

    virtual bool connect();
    virtual void beginConversation(JabberDataBlockRef streamHeader);
    virtual void endConversation(const std::exception *ex);
    virtual void loginSuccess();
    virtual void loginFailed(const char * errMsg);

private:
    ResourceContextRef rc;

};

void JabberStreamEvents::beginConversation(JabberDataBlockRef streamHeader){
    if (streamHeader->getAttribute("version")=="1.0") {
        rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(new SASLAuth(rc, streamHeader)));
    } else {
        rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(new NonSASLAuth(rc, streamHeader)));
    }
}
void JabberStreamEvents::endConversation(const std::exception *ex){
    if (ex!=NULL)  Log::getInstance()->msg(ex->what());
    Log::getInstance()->msg("End Conversation");
    rc->roster->setAllOffline();
    rc->roster->makeViewList();
    //tabs->

    rosterWnd->setIcon(presence::OFFLINE);
}

void JabberStreamEvents::loginSuccess(){
    Log::getInstance()->msg("Login ok");

    HostFeatures::discoverFeatures(rc);
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new GetRoster() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new ProcessMuc(rc) ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new PresenceRecv() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new MessageRecv() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new Version() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new Ping() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new LastActivity() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new EntityTime() ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new EntityCaps() ));
    LastActivity::update();

    JabberDataBlock getRoster("iq");
    getRoster.setAttribute("type","get");
    getRoster.setAttribute("id","roster");

    getRoster.addChildNS("query", "jabber:iq:roster");

    rc->jabberStream->sendStanza(getRoster);
    rc->bookmarks->doQueryBookmarks(rc);
}

void JabberStreamEvents::loginFailed(const char * errMsg){
    Log::getInstance()->msg("Login failed: ", errMsg);
    rc->jabberStream->sendXmppEndHeader();
    rosterWnd->setIcon(icons::ICON_ERROR_INDEX);
}

bool JabberStreamEvents::connect(){

    Socket::initWinsocks();

    if (rc->account->networkUp) {
        Log::getInstance()->msg("Raising up network");
        Socket::networkUp();
    }

    std::string host;
    int port=5222;

    if (rc->account->useSRV) {
        Log::getInstance()->msg("Searching SRV for ", rc->account->getServer().c_str() );

        dns::DnsSrvQuery d;
        int retries=3;
        do { 
            if (d.doQuery( "_xmpp-client._tcp."+rc->account->getServer())) {
                if (d.getCount()>0) {
                    dns::SRVAnswer::ref a=d.getResult(0);
                    BOOST_ASSERT(a);

                    host=a->target;
                    port=a->port;

                    Log::getInstance()->msg(boost::str(boost::format("Using %s:%d") % host.c_str() % port));
                    break;
                }
            }
        } while (retries--) ;
    } else {
        host=rc->account->hostNameIp;
        port=rc->account->port;
    }

    if (host.empty()) host=rc->account->getServer();

    if (rc->account->legacySSL && rc->account->useEncryption) {
        if (port==5222) port=5223;
    }

    Log::getInstance()->msg("Resolving ", host.c_str());

    long ip=Socket::resolveUrl(host);

    Log::getInstance()->msg(boost::str(boost::format("Connecting to %u.%u.%u.%u:%u") 
        % (ip &0xff) % ((ip>>8) &0xff) % ((ip>>16) &0xff) % ((ip>>24)&0xff) % port));

    if (rc->account->useEncryption) {
        CeTLSSocket::ref tlsCon=CeTLSSocket::ref( new CeTLSSocket(ip, port));
        rc->jabberStream->connection=tlsCon;
        if (!rc->account->useSASL) 
            tlsCon->startTls(rc->account->getServer(), rc->account->ignoreSslWarnings);
    }
    else
        rc->jabberStream->connection=ConnectionRef( new Socket(ip, port));

    /*if (rc->jabberStream->connection==NULL) {
        Log::getInstance()->msg("Failed to open connection");
        return false;
    }
    BOOST_ASSERT(rc->jabberStream->connection);
    */

    rc->jabberStream->sendXmlVersion();
    rc->jabberStream->sendXmppBeginHeader();

    return true;
}
//////////////////////////////////////////////////////////////
int prepareAccount(){
    rc=ResourceContextRef(new ResourceContext());
    //Log::getInstance()=new Log();
    //rc->account=JabberAccountRef(new JabberAccount("bombus_mobilus@jivesoftware.com", "bombus-ng"));
    //rc->account->hostNameIp="213.180.203.19";
    //rc->account->password="l12sx95a";

    rc->account=JabberAccountRef(new JabberAccount(TEXT("defAccount.bin")));
    rc->myJid.setJid( rc->account->getJid() );

    //rc->account->useSASL=true;
    //rc->account->useEncryption=true;
    //rc->account->useCompression=true;
    return 0;
}
//////////////////////////////////////////////////////////////
int initJabber() {
    if (rc->jabberStream) return 1;
    rc->jabberStanzaDispatcherRT=JabberStanzaDispatcherRef(new JabberStanzaDispatcher(rc));
    rc->jabberStanzaDispatcher2=JabberStanzaDispatcherRef(new JabberStanzaDispatcher(rc));

    //TODO: roster caching
    if (!rc->roster)
        rc->roster=RosterRef(new Roster(rc));
    rc->roster->bindWindow(rosterWnd);
    rosterWnd->setIcon(icons::ICON_PROGRESS_INDEX);
    rosterWnd->roster=rc->roster;

    rc->bookmarks=MucBookmarksRef(new MucBookmarks());

    rc->jabberStream=JabberStreamRef(new JabberStream(rc, JabberListenerRef(new JabberStreamEvents(rc))));

	return 0;
}
//////////////////////////////////////////////////////////////////////////
void streamShutdown(){
    if (!rc->jabberStream) return;
    rc->jabberStream->sendXmppEndHeader();
}


NOTIFYICONDATA nid={0};

void Shell_NotifyIcon(bool show, HWND hwnd){
    // This code will add a Shell_NotifyIcon notificaion on PocketPC and Smartphone
    
    if (show) {
        nid.cbSize = sizeof(nid);
        nid.uID = 100;      // Per WinCE SDK docs, values from 0 to 12 are reserved and should not be used.
        nid.uFlags = NIF_ICON | NIF_MESSAGE;
        nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_BOMBUS));
        nid.uCallbackMessage=SHELLNOTIFYICON;
        nid.hWnd=hwnd;

        //Add the notification to the tray
        Shell_NotifyIcon(NIM_ADD, &nid);
    } else {
        //remove the notification from the tray
        Shell_NotifyIcon(NIM_DELETE, &nid);
        DeleteObject(nid.hIcon);
    }

    //Update the icon of the notification
    /*nid.uFlags = NIF_ICON;
    nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_SAMPLEICON2));
    Shell_NotifyIcon(NIM_MODIFY, &nid);*/


    return;
}
