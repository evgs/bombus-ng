// ui.cpp : Defines the entry point for the application.
//

//#define JIVESOFTWARE

//#include "stdafx.h"

#include "ui.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "Log.h"
#include "Socket.h"
#include "CETLSSocket.h"
#include <string>
#include "JabberDataBlock.h"
#include "JabberStream.h"
#include "JabberAccount.h"
#include "JabberListener.h"
#include "JabberDataBlockListener.h"
#include "ResourceContext.h"

#include "Roster.h"

#include "DlgAccount.h"
#include "VirtualListView.h"
#include "ChatView.h"
#include "TabCtrl.h"

#include "Auth.h"

#include "Sysinfo.h"

#include "Image.h"

#include "utf8.hpp"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND		mainWnd;

//ListViewRef logWnd;
TabsCtrlRef tabs;

RosterView::ref rosterWnd;
ChatView::ref chatSample;
ResourceContextRef rc;

ImgListRef skin;

std::wstring appRootPath;
std::wstring skinRootPath;

int tabHeight;

int prepareAccount();
int initJabber();
void streamShutdown();
void Shell_NotifyIcon(bool show, HWND hwnd);

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

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
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
    Shell_NotifyIcon(false, NULL);
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

    wchar_t appName[1024];
    GetModuleFileName(hInstance, appName, sizeof(appName));
    appRootPath=appName;
    int namePos=appRootPath.find_last_of(_T("\\"))+1;
    appRootPath.erase(namePos, appRootPath.length()-namePos);

    wchar_t * skinRelPath;
    if (sysinfo::screenIsVGA()) {
        skinRelPath=TEXT("vga\\");
        tabHeight=32; //TODO: remove hardcode
    } else {
        skinRelPath=TEXT("qvga\\");
        tabHeight=16; //TODO: remove hardcode
    }
    skinRootPath=appRootPath+skinRelPath;
    

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
				case IDM_HELP_ABOUT:
					DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
				break;
				case IDM_JABBER_ACCOUNT:
					DialogAccount(g_hInst, hWnd, rc->account);
					break;
                case IDM_EXIT:
                    SendMessage (hWnd, WM_CLOSE, 0, 0);				
                    break;

				case IDM_JABBER_ONLINE:
					initJabber();
					break;

                case IDM_JABBER_OFFLINE:
                    streamShutdown();

				case IDM_JABBER_STREAMINFO:
                    Log::getInstance()->msg(
						rc->jabberStream->connection->getStatistics().c_str()
						);
					break;

				case IDM_WINDOWS_LOG:
				case IDM_WINDOWS_ROSTER:
					break;

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

            //skin=ImgListRef(new ImgArray(TEXT("skin.png"), 8, 6));
            skin=ImgListRef(new Skin(TEXT("")));
            //skin->setGridSize(8, 6);

			//editWnd=DoCreateEditControl(hWnd);
            tabs=TabsCtrlRef(new TabsCtrl(hWnd));
            //tabs->setParent(hWnd);

            rosterWnd=RosterView::ref(new RosterView(tabs->getHWnd(), std::string("Roster")));
            tabs->addWindow(rosterWnd);

            chatSample=ChatView::ref(new ChatView(tabs->getHWnd(), Contact::ref(new Contact("test@server","resource",""))));
            //tabs->addWindow(chatSample);
            //logWnd=ListViewRef(new ListView(hWnd, std::string("Log")));
            //tabs->addWindow(logWnd);

            { 
                VirtualListView::ref odrLog = VirtualListView::ref(new VirtualListView(tabs->getHWnd(), std::string("Log")));
                tabs->addWindow(odrLog);
                Log::getInstance()->bindLV(odrLog); 
            }

            /*tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 1"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 2"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 3"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window long name"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 4"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 5"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 6"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 7"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window extra long name"))));*/
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

				DeferWindowPos(hdwp, tabs->getHWnd(), HWND_TOP, 0, tabHeight, 
					GET_X_LPARAM(lParam), height-tabHeight, 
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

        case WM_USER:
            SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));            
            break;
        case WM_USER+2:
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                // Create a Done button and size it.  
                SHINITDLGINFO shidi;
                shidi.dwMask = SHIDIM_FLAGS;
                shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
                shidi.hDlg = hDlg;
                SHInitDialog(&shidi);
            }
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;

/*#ifdef _DEVICE_RESOLUTION_AWARE
        case WM_SIZE:
            {
		DRA::RelayoutDialog(
			g_hInst, 
			hDlg, 
			DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
            }
            break;
#endif*/
    }
    return (INT_PTR)FALSE;
}

//////////////////////////////////////////////////////////////////////////
class MTForwarder: public JabberDataBlockListener {
public:
    MTForwarder(ResourceContextRef rc) {  /*this->rc=rc;*/  };
    ~MTForwarder(){};
    virtual const char * getTagName() const { return NULL; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
};
ProcessResult MTForwarder::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc) {
    JabberDataBlockRef *p=new JabberDataBlockRef(block); //ÀÕÒÓÍÃ
    //rc->jabberStanzaDispatcher2->dispatchDataBlock(block);
    PostMessage(mainWnd, WM_USER+2, 0, (LPARAM)p);

    return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class GetRoster : public JabberDataBlockListener {
public:
	GetRoster(ResourceContextRef rc) {
		this->rc=rc;
	}
	~GetRoster(){};
	virtual const char * getType() const{ return "result"; }
	virtual const char * getId() const{ return "roster"; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult GetRoster::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
	Log::getInstance()->msg("Roster arrived");

    rc->roster->blockArrived(block, rc); // forwarding to dispatch roster stanza
    
    //rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(new MTForwarder(rc)));
    //rc->jabberStanzaDispatcher2->addListener(rc->roster);

    rc->jabberStanzaDispatcherRT->addListener(rc->roster);


	JabberDataBlock presence("presence");
	presence.addChild("status", 
		"please, don't send any messages here! \n"
		"they will be dropped because it is debug version" );
	rc->jabberStream->sendStanza(presence);

    /*presence.setAttribute("to","devel@conference.jabber.ru/bng");
    rc->jabberStream->sendStanza(presence);*/

	return LAST_BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class Version : public JabberDataBlockListener {
public:
	Version(ResourceContextRef rc) {
		this->rc=rc;
	}
	~Version(){};
	virtual const char * getType() const{ return "get"; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "iq"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult Version::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    
    Log::getInstance()->msg("version request ", block->getAttribute("from").c_str());

    std::string version=utf8::wchar_utf8(sysinfo::getOsVersion());


	JabberDataBlock reply("iq");
	reply.setAttribute("type","result");
	reply.setAttribute("id", block->getAttribute("id"));
	reply.setAttribute("to", block->getAttribute("from"));

	JabberDataBlock * qry=reply.addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:version");
	qry->addChild("name","Bombus-ng");
	qry->addChild("version","0.0.3-devel");
	qry->addChild("os",version.c_str());

	rc->jabberStream->sendStanza(reply);
	return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class MessageRecv : public JabberDataBlockListener {
public:
	MessageRecv(ResourceContextRef rc) {
		this->rc=rc;
	}
	~MessageRecv(){};
	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "message"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult MessageRecv::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    std::string from=block->getAttribute("from");
    std::string body=block->getChildByName("body")->getText();

    StringRef orig=block->toXML();
	Log::getInstance()->msg("Message from ", from.c_str()); 

    Contact::ref c = rc->roster->getContactEntry(from);

    Message::ref msg=Message::ref(new Message(body, from, Message::INCOMING));

    std::wstring soundName(appRootPath);
    soundName+=TEXT("message.wav");
    PlaySound(soundName.c_str(), NULL, SND_ASYNC | /*SND_NOWAIT |*/SND_FILENAME);

    c->nUnread++;
    c->messageList->push_back(msg);

    ChatView *cv = dynamic_cast<ChatView *>(tabs->getWindowByODR(c).get());
    if(cv) {
        cv->moveUnread();
    }

    tabs->switchByODR(c);

    InvalidateRect(rosterWnd->getHWnd(),NULL, FALSE);

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
}

void JabberStreamEvents::loginSuccess(){
    Log::getInstance()->msg("Login ok");

    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new GetRoster(rc) ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new Version(rc) ));
    rc->jabberStanzaDispatcherRT->addListener( JabberDataBlockListenerRef( new MessageRecv(rc) ));

    JabberDataBlock getRoster("iq");
    getRoster.setAttribute("type","get");
    getRoster.setAttribute("id","roster");

    JabberDataBlock *qry =getRoster.addChild("query", NULL); 
    qry->setAttribute("xmlns","jabber:iq:roster");

    rc->jabberStream->sendStanza(getRoster);
}

void JabberStreamEvents::loginFailed(const char * errMsg){
    Log::getInstance()->msg("Login failed: ", errMsg);
    rc->jabberStream->sendXmppEndHeader();
}

bool JabberStreamEvents::connect(){
    std::string host=(rc->account->hostNameIp.empty())?rc->account->getServer() : rc->account->hostNameIp;

    Log::getInstance()->msg("Connect to ", host.c_str());
    if (rc->account->useEncryption) {
        ConnectionRef tlsCon=ConnectionRef( new CeTLSSocket(host, rc->account->port));
        rc->jabberStream->connection=tlsCon;
        if (rc->account->legacySSL) ((CeTLSSocket*)(tlsCon.get()))->startTls();
    }
    else
        rc->jabberStream->connection=ConnectionRef( new Socket(host, rc->account->port));

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

    //rc->account->useSASL=true;
    //rc->account->useEncryption=true;
    //rc->account->useCompression=true;
    return 0;
}
//////////////////////////////////////////////////////////////
int initJabber()
{
    rc->jabberStanzaDispatcherRT=JabberStanzaDispatcherRef(new JabberStanzaDispatcher(rc));
    rc->jabberStanzaDispatcher2=JabberStanzaDispatcherRef(new JabberStanzaDispatcher(rc));

    //TODO: roster caching
    rc->roster=RosterRef(new Roster());
    rc->roster->bindWindow(rosterWnd);
    rosterWnd->roster=rc->roster;

    rc->jabberStream=JabberStreamRef(new JabberStream(rc, JabberListenerRef(new JabberStreamEvents(rc))));

	return 0;
}
//////////////////////////////////////////////////////////////////////////
void streamShutdown(){
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
        nid.uCallbackMessage=WM_USER;
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
