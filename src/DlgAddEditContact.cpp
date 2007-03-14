//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"

#include "DlgAddEditContact.h"
#include "DlgUtils.h"

#include "JabberDataBlockListener.h"
#include "Roster.h"

#include "..\vs2005\ui\resourceppc.h"

#include "basetypes.h"
#include "utf8.hpp"

extern HINSTANCE	g_hInst;			// current instance
extern RosterView::ref rosterWnd;


void streamShutdown();
int initJabber();

INT_PTR CALLBACK DlgAddEditContact::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DlgAddEditContact *p=(DlgAddEditContact *) GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:
		{
            p=(DlgAddEditContact *) lParam;
            SetWindowLong(hDlg, GWL_USERDATA, (LONG) p );
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

            if (p->contact) { 
                SetDlgItemText(hDlg, IDC_E_JID, p->contact->rosterJid);
                SetDlgItemText(hDlg, IDC_E_NICK, p->contact->nickname);
                SetDlgItemText(hDlg, IDC_C_GROUP, p->contact->group);
            }

            StringVectorRef groups=p->rc->roster->getRosterGroups();

            for (int i=0; i<groups->size(); i++)
                AddComboString(hDlg, IDC_C_GROUP, (*groups)[i]);


            CheckDlgButton(hDlg, IDC_X_SUBSCR, BST_CHECKED);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{

            /*presence::PresenceIndex status=(presence::PresenceIndex) SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_GETCURSEL, 0,0);
            int priority=SendDlgItemMessage(hDlg, IDC_SPIN_PRIORITY, UDM_GETPOS, 0, 0);
            std::string pmessage;
            GetDlgItemText(hDlg, IDC_E_STATUS, pmessage);
            
            //direct presences
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

void DlgAddEditContact::createDialog(HWND parent, ResourceContextRef rc) {
    createDialog(parent, rc, Contact::ref());
}

void DlgAddEditContact::createDialog( HWND parent, ResourceContextRef rc, Contact::ref contact ) {
    /*dlgAccountParam=accnt;*/
    DlgAddEditContact *p=new DlgAddEditContact();
    p->parent=parent;
    p->rc=rc;
    p->contact=contact;

    bool edit=false;
    if (contact) {
        if (contact->subscr!="NIL") edit=true;
    }
    DialogBoxParam(g_hInst, 
        (edit)? (LPCTSTR)IDD_EDIT_CONTACT : (LPCTSTR)IDD_ADD_CONTACT ,
        parent, dialogProc, (LPARAM)p);
}
