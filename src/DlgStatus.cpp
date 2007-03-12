//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberAccount.h"

#include "DlgStatus.h"
#include "DlgUtils.h"

#include "JabberDataBlockListener.h"
#include "Roster.h"

#include "..\vs2005\ui\resourceppc.h"


#include "utf8.hpp"

extern HINSTANCE	g_hInst;			// current instance
extern RosterView::ref rosterWnd;


void streamShutdown();
int initJabber();

wchar_t *statusNames []= { 
    TEXT("Online"),         TEXT("Free for chat"),  TEXT("Away"), 
    TEXT("Extended Away"),  TEXT("DND"),            TEXT("Offline") 
};


INT_PTR CALLBACK DlgStatus::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DlgStatus *p=(DlgStatus *) GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:
		{
            p=(DlgStatus *) lParam;
            SetWindowLong(hDlg, GWL_USERDATA, (LONG) p );
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

            for (int i=0; i<6; i++)
                SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) statusNames[i]);
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_SETCURSEL, p->rc->status, 0);

            if (p->contact) SetDlgItemText(hDlg, IDC_E_JID, p->contact->jid.getJid());

            SetDlgItemText(hDlg, IDC_E_STATUS, p->rc->presenceMessage);

            SendDlgItemMessage(hDlg, IDC_SPIN_PRIORITY, UDM_SETRANGE32, -128, 128);
            SendDlgItemMessage(hDlg, IDC_SPIN_PRIORITY, UDM_SETPOS, 0, p->rc->priority);

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

            presence::PresenceIndex status=(presence::PresenceIndex) SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_GETCURSEL, 0,0);
            int priority=SendDlgItemMessage(hDlg, IDC_SPIN_PRIORITY, UDM_GETPOS, 0, 0);
            std::string pmessage;
            GetDlgItemText(hDlg, IDC_E_STATUS, pmessage);
            
            //TODO: direct presences
            if (p->contact) {
                std::string to;
                GetDlgItemText(hDlg, IDC_E_JID, to);
                p->rc->sendPresence(to.c_str(), status, pmessage, priority);
            } else {
                //store selected presence
                p->rc->status=status;
                p->rc->presenceMessage=pmessage;
                p->rc->priority=priority;

                // Broadcast presence
                rosterWnd->setIcon(p->rc->status);
                p->rc->sendPresence();
                if (status==presence::OFFLINE) {
                    streamShutdown();
                } else {
                    initJabber();
                }
            }

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

void DlgStatus::createDialog(HWND parent, ResourceContextRef rc) {
    createDialog(parent, rc, Contact::ref());
}

void DlgStatus::createDialog( HWND parent, ResourceContextRef rc, Contact::ref contact ) {
    /*dlgAccountParam=accnt;*/
    DlgStatus *p=new DlgStatus();
    p->parent=parent;
    p->rc=rc;
    p->contact=contact;

    DialogBoxParam(g_hInst, 
        (contact)? (LPCTSTR)IDD_DIRECT_PRESENCE : (LPCTSTR)IDD_STATUS, parent, dialogProc, (LPARAM)p);
}
