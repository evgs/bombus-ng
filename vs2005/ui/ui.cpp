// ui.cpp : Defines the entry point for the application.
//

//#define JIVESOFTWARE

#include "stdafx.h"

#include "ui.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>

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
#include "ListView.h"
#include "TabCtrl.h"

#include "Auth.h"

#include "Sysinfo.h"

#include "Image.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle
HWND		listWnd;
HWND		editWnd;
HWND		mainWnd;

ListViewRef logWnd;
ListViewRef rosterWnd;
TabsCtrlRef tabs;
ResourceContextRef rc;

ImageRef skin;

int initJabber();

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


    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateEditControl(HWND hwndParent) {

	HWND hWndEdit; 
	TCITEM tie; 

	hWndEdit=CreateWindow(_T("EDIT"), _T("LOG"), 
		WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL
		| ES_MULTILINE , 
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
		hwndParent, NULL, g_hInst, NULL); 
	return hWndEdit;
}
////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateComboControl(HWND hwndParent) {

	HWND hWndCombo; 
	TCITEM tie; 

	/*hWndCombo=CreateWindow(_T("COMBOBOX"), _T("LOG"), 
		WS_BORDER| WS_CHILD | WS_VISIBLE 
		| CBS_DROPDOWNLIST | CBS_HASSTRINGS, 
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
		hwndParent, NULL, g_hInst, NULL); 
	ComboBox_AddString(hWndCombo, _T("TEST1"));
	ComboBox_AddString(hWndCombo, _T("TEST2"));
	ComboBox_SetCurSel(hWndCombo, 0);
	//SendMessage(hWndCombo,CB_SETMINVISIBLE, 10, 0);
	ComboBox_SetItemHeight(hWndCombo, 0, 64);
	*/
	
	return hWndCombo;
}
////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateListControl(HWND hwndParent) {

    HWND hWndList; 
    TCITEM tie; 

	hWndList=CreateWindow(_T("LISTBOX"), NULL, 
				WS_BORDER| WS_CHILD | /*WS_VISIBLE |*/ WS_VSCROLL
				| LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT, 
		        0, 0, CW_USEDEFAULT, CW_USEDEFAULT, 
		        hwndParent, NULL, g_hInst, NULL); 
	
	//ListBox_AddString(hWndList, _T("STR1"));
	//ListBox_AddString(hWndList, _T("STR2"));
	ListBox_SetTopIndex(hWndList, 0);

	int count=ListBox_GetCount(hWndList);
	return hWndList;
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

// tab variables here
int tabHeight=16;
int editHeight=64;


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

				case IDM_JABBER_STREAMINFO:
					rc->log->msg(
						rc->jabberStream->connection->getStatistics().c_str()
						);
					break;

				case IDM_WINDOWS_LOG:
					//listWnd=logWnd;
					//SetWindowLong(rosterWnd, GWL_STYLE, WS_BORDER| WS_CHILD | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT);
					//SetWindowLong(logWnd, GWL_STYLE, WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT);
					//SetWindowPos(logWnd, NULL, 0,0, 0,0, SWP_SHOWWINDOW| SWP_NOSIZE | SWP_NOMOVE| SWP_NOZORDER);
					//SetWindowPos(rosterWnd, NULL, 0,0, 0,0, SWP_HIDEWINDOW| SWP_NOSIZE | SWP_NOMOVE| SWP_NOZORDER);

                    //ShowWindow(rosterWnd, SW_HIDE);
                    //logWnd->showWindow(true);
                    //ShowWindow(editWnd, SW_SHOW);
					break;
				case IDM_WINDOWS_ROSTER:
					//listWnd=rosterWnd;
					//SetWindowLong(logWnd, GWL_STYLE, WS_BORDER| WS_CHILD | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT);
					//SetWindowLong(rosterWnd, GWL_STYLE, WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT);
					//SetWindowPos(rosterWnd, NULL, 0,0, 0,0, SWP_SHOWWINDOW| SWP_NOSIZE | SWP_NOMOVE| SWP_NOZORDER);
					//SetWindowPos(logWnd, NULL, 0,0, 0,0, SWP_HIDEWINDOW| SWP_NOSIZE | SWP_NOMOVE| SWP_NOZORDER);

                    //ShowWindow(rosterWnd, SW_SHOW);
                    //logWnd->showWindow(false);
                    //ShowWindow(editWnd, SW_HIDE);
					break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        case WM_CREATE:
            SHMENUBARINFO mbi;

            memset(&mbi, 0, sizeof(SHMENUBARINFO));
            mbi.cbSize     = sizeof(SHMENUBARINFO);
            mbi.hwndParent = hWnd;
            mbi.nToolBarId = IDR_MENU;
            mbi.hInstRes   = g_hInst;

            skin=ImageRef(new Image(TEXT("\\Program Files\\ui\\skin.png")));
            skin->createMask();

			editWnd=DoCreateEditControl(hWnd);
            tabs=TabsCtrlRef(new TabsCtrl(hWnd));
            rosterWnd=ListViewRef(new ListView(hWnd, std::string("Roster")));
            tabs->addWindow(rosterWnd);
            logWnd=ListViewRef(new ListView(hWnd, std::string("Log")));
            tabs->addWindow(logWnd);
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 1"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 2"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 3"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window long name"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 4"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 5"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 6"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window 7"))));
            tabs->addWindow(ListViewRef(new ListView(hWnd, std::string("Window extra long name"))));
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

            {
                skin->drawImage(hdc, 0,0);                
            }
            
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
				RECT rc; 
				
				int height=GET_Y_LPARAM(lParam);
				int ySplit=height-editHeight;
				// Calculate the display rectangle, assuming the 
				// tab control is the size of the client area. 
				SetRect(&rc, 0, 0, 
					GET_X_LPARAM(lParam), ySplit ); 
								
				// Size the tab control to fit the client area. 
				hdwp = BeginDeferWindowPos(3);

				/*DeferWindowPos(hdwp, dropdownWnd, HWND_TOP, 0, 0, 
					GET_X_LPARAM(lParam), 20, 
					SWP_NOZORDER 
					); */


				DeferWindowPos(hdwp, tabs->getHWnd(), HWND_TOP, 0, tabHeight, 
					GET_X_LPARAM(lParam), ySplit-tabHeight, 
					SWP_NOZORDER 
					);
				/*DeferWindowPos(hdwp, rosterWnd, HWND_TOP, 0, tabHeight, 
					GET_X_LPARAM(lParam), height-tabHeight, 
					SWP_NOZORDER 
					);*/

				DeferWindowPos(hdwp, editWnd, NULL, 0, ySplit+1, 
					GET_X_LPARAM(lParam), height-ySplit-1, 
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

		case WM_CTLCOLORSTATIC:
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

/**************************************************************************************************************************/
Log::Log(){
}

Log::~Log(){
}

wchar_t buf[256];

const wchar_t * charToWchar(const char * src, const char *src2 = NULL) {
	wchar_t *b=buf;

	int i;
	for (i=0; i<255; i++) {
		if (*src ==0 ) break;
		*(b++)=*(src++);
	}

	//*(b++)=0x20;
	if (src2!=0)
		for (; i<255; i++) {
		if (*src2 ==0 ) break;
		*(b++)=*(src2++);
	}
	*b=0;

	return buf;
}

void Log::msg(const std::string &message){

	ListBox_AddString( logWnd->getListBoxHWnd(), charToWchar(message.c_str()));
}

void Log::msg(const char * message){
	ListBox_AddString( logWnd->getListBoxHWnd(), charToWchar(message));
}

void Log::msg(const char * message, const char *message2){
	ListBox_AddString( logWnd->getListBoxHWnd(), charToWchar(message, message2));
}


/**********************************************************************************/
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
	rc->log->msg("Roster arrived");

    rc->roster->blockArrived(block, rc); // forwarding to dispatch roster stanza
    rc->jabberStanzaDispatcher->addListener(rc->roster);

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
    
    rc->log->msg("version request", block->getAttribute("from").c_str());

    std::string version=utf8::wchar_utf8(sysinfo::getOsVersion());


	JabberDataBlock reply("iq");
	reply.setAttribute("type","result");
	reply.setAttribute("id", block->getAttribute("id"));
	reply.setAttribute("to", block->getAttribute("from"));

	JabberDataBlock * qry=reply.addChild("query",NULL);
	qry->setAttribute("xmlns","jabber:iq:version");
	qry->addChild("name","Bombus-ng");
	qry->addChild("version","0.0.2-devel");
	qry->addChild("os",version.c_str());

	rc->jabberStream->sendStanza(reply);
	return BLOCK_PROCESSED;
}
//////////////////////////////////////////////////////////////
class MessageFwd : public JabberDataBlockListener {
public:
	MessageFwd(ResourceContextRef rc) {
		this->rc=rc;
	}
	~MessageFwd(){};
	virtual const char * getType() const{ return NULL; }
	virtual const char * getId() const{ return NULL; }
	virtual const char * getTagName() const { return "message"; }
	virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
private:
	ResourceContextRef rc;
};
ProcessResult MessageFwd::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
	rc->log->msg("Message from ", block->getAttribute("from").c_str()); 
	JabberDataBlock reply("message");
	reply.setAttribute("type","chat");
	reply.setAttribute("to", "evgs@jabber.ru/Psi_Home");
	reply.addChild("body", NULL)->setText(XMLStringPrep( *(block->toXML()) ));

	rc->jabberStream->sendStanza(reply);
	return BLOCK_PROCESSED;
}

class JabberStreamEvents : public JabberListener{
public:
    JabberStreamEvents(ResourceContextRef rc) {this->rc=rc;}

    virtual bool connect();
    virtual void beginConversation(JabberDataBlockRef streamHeader);
    virtual void endConversation(const std::exception *ex);
    virtual void loginSuccess();
    virtual void loginFailed();

private:
    ResourceContextRef rc;

};

void JabberStreamEvents::beginConversation(JabberDataBlockRef streamHeader){
    if (streamHeader->getAttribute("version")=="1.0") {
        rc->jabberStanzaDispatcher->addListener(JabberDataBlockListenerRef(new SASLAuth(rc, streamHeader)));
    } else {
        //JabberDataBlockListenerRef(new NonSASLAuth(rc, streamHeader));
    }
}
void JabberStreamEvents::endConversation(const std::exception *ex){
    if (ex!=NULL)  rc->log->msg(ex->what());
    rc->log->msg("End Conversation");
}

void JabberStreamEvents::loginSuccess(){
    rc->log->msg("Login ok");

    rc->jabberStanzaDispatcher->addListener( JabberDataBlockListenerRef( new GetRoster(rc) ));
    rc->jabberStanzaDispatcher->addListener( JabberDataBlockListenerRef( new Version(rc) ));
    rc->jabberStanzaDispatcher->addListener( JabberDataBlockListenerRef( new MessageFwd(rc) ));

    JabberDataBlock getRoster("iq");
    getRoster.setAttribute("type","get");
    getRoster.setAttribute("id","roster");

    JabberDataBlock *qry =getRoster.addChild("query", NULL); 
    qry->setAttribute("xmlns","jabber:iq:roster");

    rc->jabberStream->sendStanza(getRoster);
}

void JabberStreamEvents::loginFailed(){
    rc->log->msg("Login failed");
    rc->jabberStream->sendXmppEndHeader();
}

bool JabberStreamEvents::connect(){
    std::string host=(rc->account->hostNameIp.empty())?rc->account->getServer() : rc->account->hostNameIp;

    rc->log->msg("Connect to ", host.c_str());
    if (rc->account->useEncryption) {
        ConnectionRef tlsCon=ConnectionRef( new CeTLSSocket(host, rc->account->port));
        rc->jabberStream->connection=tlsCon;
        if (rc->account->legacySSL) ((CeTLSSocket*)(tlsCon.get()))->startTls();
    }
    else
        rc->jabberStream->connection=ConnectionRef( new Socket(host, rc->account->port));

    /*if (rc->jabberStream->connection==NULL) {
        rc->log->msg("Failed to open connection");
        return false;
    }
    BOOST_ASSERT(rc->jabberStream->connection);
    */

    rc->jabberStream->sendXmlVersion();
    rc->jabberStream->sendXmppBeginHeader();

    return true;
}
//////////////////////////////////////////////////////////////

int initJabber()
{{
	rc=ResourceContextRef(new ResourceContext());
	rc->log=new Log();
#ifdef JIVESOFTWARE
    rc->account=JabberAccountRef(new JabberAccount("bombus_mobilus@jivesoftware.com", "bombus-ng"));
    //rc->account->hostNameIp="213.180.203.19";
    rc->account->password="l12sx95a";
#else
	rc->account=JabberAccountRef(new JabberAccount("evgs@jabber.ru", "bombus-ng"));
	//rc->account->hostNameIp="213.180.203.19";
    //rc->account->port=5222;
	rc->account->password=
#include "password"
	;
#endif
    rc->account->useSASL=true;
    //rc->account->useEncryption=true;
	rc->account->useCompression=true;

    rc->jabberStanzaDispatcher=JabberStanzaDispatcherRef(new JabberStanzaDispatcher(rc));

    //TODO: roster caching
    rc->roster=RosterRef(new Roster());
    rc->roster->bindWindow(rosterWnd);

    rc->jabberStream=JabberStreamRef(new JabberStream(rc, JabberListenerRef(new JabberStreamEvents(rc))));


    //jstream.sendStanza(test);
	//printf("%s", test.toXML());

	/*char tmp[16];
	gets_s(tmp,16);
	//jstream.sendStanza(login);
	rc->jabberStream->sendXmppEndHeader();

	gets_s(tmp,16);

	rc->log->msg(
		rc->connection->getStatistics().c_str()
		);

	gets_s(tmp,16);*/
}
	return 0;
}

