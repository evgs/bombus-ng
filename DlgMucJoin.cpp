//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberAccount.h"

#include "DlgMucJoin.h"
#include "DlgUtils.h"

#include "ProcessMUC.h"

#include "..\vs2005\ui\resourceppc.h"


#include "utf8.hpp"

extern HINSTANCE	g_hInst;			// current instance
extern RosterView::ref rosterWnd;

INT_PTR CALLBACK DlgMucJoin::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DlgMucJoin *p=(DlgMucJoin *) GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:
		{
            p=(DlgMucJoin *) lParam;
            SetWindowLong(hDlg, GWL_USERDATA, (LONG) p );
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

            std::string room="bombus";
            std::string server="conference.jabber.ru";
            std::string pass="";
            std::string nick="evgs";

            /*for (int i=0; i<6; i++)
                SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) statusNames[i]);
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_SETCURSEL, p->rc->status, 0);*/

            SetDlgItemText(hDlg, IDC_E_ROOM, room);
            SetDlgItemText(hDlg, IDC_E_SERVER, server);
            SetDlgItemText(hDlg, IDC_E_PASSWORD, pass);
            SetDlgItemText(hDlg, IDC_C_NICK, nick);

            SendDlgItemMessage(hDlg, IDC_SPIN_PRIORITY, UDM_SETRANGE32, 0, 20);
            SendDlgItemMessage(hDlg, IDC_SPIN_PRIORITY, UDM_SETPOS, 0, 20 /*p->rc->priority*/);

            /*SetDlgItemText(hDlg, IDC_E_JID, dlgAccountParam->getBareJid());
            SetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
            SetDlgItemText(hDlg, IDC_E_RESOURCE, dlgAccountParam->getResource());
            SetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp);
            SetDlgItemInt(hDlg, IDC_E_PORT, dlgAccountParam->port, false);
            CheckDlgButton(hDlg, IDC_X_SSL, (dlgAccountParam->useEncryption)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_PLAIN, (dlgAccountParam->plainTextPassword)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_SASL, (dlgAccountParam->useSASL)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_ZLIB, (dlgAccountParam->useCompression)?BST_CHECKED:BST_UNCHECKED);
            */
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
            std::string room;  GetDlgItemText(hDlg, IDC_E_ROOM, room);
            std::string server; GetDlgItemText(hDlg, IDC_E_SERVER, server);
            std::string pass; GetDlgItemText(hDlg, IDC_E_PASSWORD, pass);
            std::string nick; GetDlgItemText(hDlg, IDC_C_NICK, nick);

            int histSz=SendDlgItemMessage(hDlg, IDC_SPIN_HIST_SZ, UDM_GETPOS, 0, 0);

            Jid roomNode;
            roomNode.setUserName(room);
            roomNode.setServer(server);
            roomNode.setResource(nick);

            ProcessMuc::initMuc(roomNode.getJid(), pass, p->rc);

            p->rc->sendPresence(roomNode.getJid().c_str(), p->rc->status, p->rc->presenceMessage, p->rc->priority); //TODO

            /*
            dlgAccountParam->setBareJid(GetDlgItemText(hDlg, IDC_E_JID));
            GetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
            dlgAccountParam->setResource(GetDlgItemText(hDlg, IDC_E_RESOURCE));
            GetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp);

            dlgAccountParam->port=GetDlgItemInt(hDlg, IDC_E_PORT, NULL, false);

            dlgAccountParam->useEncryption=IsDlgButtonChecked(hDlg, IDC_X_SSL)==BST_CHECKED;
            dlgAccountParam->plainTextPassword=IsDlgButtonChecked(hDlg, IDC_X_PLAIN)==BST_CHECKED;
            dlgAccountParam->useSASL=IsDlgButtonChecked(hDlg, IDC_X_SASL)==BST_CHECKED;
            dlgAccountParam->useCompression=IsDlgButtonChecked(hDlg, IDC_X_ZLIB)==BST_CHECKED;

            dlgAccountParam->saveAccount(TEXT("defAccount.bin"));
            */

			EndDialog(hDlg, LOWORD(wParam));
            delete p;
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
            delete p;
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, message);
        delete p;
		return TRUE;
	}
	return (INT_PTR)FALSE;
}

void DlgMucJoin::createDialog( HWND parent, ResourceContextRef rc) {
    /*dlgAccountParam=accnt;*/
    DlgMucJoin *p=new DlgMucJoin();
    p->parent=parent;
    p->rc=rc;
    //p->contact=contact;

    DialogBoxParam(g_hInst, (LPCTSTR)IDD_CONFERENCE, parent, dialogProc, (LPARAM)p);
}
