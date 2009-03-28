//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberAccount.h"
#include "JabberStream.h"

#include "DlgMucJoin.h"
#include "DlgUtils.h"
#include "MRU.h"

#include "ProcessMUC.h"
#include "MucBookmarks.h"

#include "..\vs2005\ui\resourceppc.h"

#include "utf8.hpp"
#include "stringutils.h"

extern HINSTANCE	g_hInst;			// current instance
extern RosterListView::ref rosterWnd;

#define MRU_MUC_NICK L"MucJoinNick"

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

            SetDlgItemText(hDlg, IDC_E_ROOM, p->jid.getUserName());
            SetDlgItemText(hDlg, IDC_E_SERVER, p->jid.getServer());
            SetDlgItemText(hDlg, IDC_E_PASSWORD, p->pass);
            mru::readMru(MRU_MUC_NICK, hDlg, IDC_C_NICK, NULL);
            if (p->jid.getResource().length())
                SetDlgItemText(hDlg, IDC_C_NICK, p->jid.getResource());

            SendDlgItemMessage(hDlg, IDC_SPIN_HIST_SZ, UDM_SETRANGE32, 0, 20);
            SendDlgItemMessage(hDlg, IDC_SPIN_HIST_SZ, UDM_SETPOS, 0, 20 /*p->rc->priority*/);

            if ( p->rc->bookmarks->isBookmarksAvailable() ) {
                // enabling grayed items
                EnableWindow(GetDlgItem(hDlg, IDC_C_BOOKMARK), TRUE);
                //EnableWindow(GetDlgItem(hDlg, IDC_SAVE), TRUE);
                //EnableWindow(GetDlgItem(hDlg, IDC_DELETE), TRUE);
                //EnableWindow(GetDlgItem(hDlg, IDC_X_AUTOJOIN), TRUE);

                // filling up combo box
                for (int i=0; i< (p->rc->bookmarks->getBookmarkCount()); i++) {
                    std::wstring bn=utf8::utf8_wchar(p->rc->bookmarks->get(i)->name);
                    SendDlgItemMessage(hDlg, IDC_C_BOOKMARK, CB_ADDSTRING, 0, (LPARAM) bn.c_str());
                }
                SendDlgItemMessage(hDlg, IDC_C_BOOKMARK, CB_SETCURSEL, 0, 0);
            }

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
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            int bmi=SendDlgItemMessage(hDlg, IDC_C_BOOKMARK, CB_GETCURSEL, 0, 0);
            if (bmi==CB_ERR) return TRUE;
            MucBookmarkItem::ref bm=p->rc->bookmarks->get(bmi);
            Jid roomJid(bm->jid);
            SetDlgItemText(hDlg, IDC_E_ROOM, roomJid.getUserName());
            SetDlgItemText(hDlg, IDC_E_SERVER, roomJid.getServer());
            SetDlgItemText(hDlg, IDC_E_PASSWORD, bm->password);
        }

		if (LOWORD(wParam) == IDOK)
		{
            std::string room;  GetDlgItemText(hDlg, IDC_E_ROOM, room); std::trim(room);
            std::string server; GetDlgItemText(hDlg, IDC_E_SERVER, server); std::trim(server);
            std::string pass; GetDlgItemText(hDlg, IDC_E_PASSWORD, pass);
            std::string nick; GetDlgItemText(hDlg, IDC_C_NICK, nick); std::trim(nick);
            mru::saveMru(MRU_MUC_NICK, hDlg, IDC_C_NICK);

            int histSz=SendDlgItemMessage(hDlg, IDC_SPIN_HIST_SZ, UDM_GETPOS, 0, 0);

            Jid roomNode;
            roomNode.setUserName(room);
            roomNode.setServer(server);
            roomNode.setResource(nick);

            if (room.length()==0 || server.length()==0 || nick.length()==0) return TRUE;

            ProcessMuc::initMuc(roomNode.getJid(), pass, p->rc);



            JabberDataBlockRef joinPresence=constructPresence(
                roomNode.getJid().c_str(), 
                p->rc->status, 
                p->rc->presenceMessage, 
                p->rc->priority); 
            JabberDataBlockRef xMuc=joinPresence->addChildNS("x", "http://jabber.org/protocol/muc");
            if (!pass.empty()) xMuc->addChild("password",pass.c_str());

            if (p->rc->isLoggedIn())
                p->rc->jabberStream->sendStanza(joinPresence);

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

void DlgMucJoin::createDialog( HWND parent, ResourceContextRef rc, const std::string &jid) {
    /*dlgAccountParam=accnt;*/
    DlgMucJoin *p=new DlgMucJoin();
    p->parent=parent;
    p->rc=rc;
    p->jid.setJid(jid);
    //p->contact=contact;

    DialogBoxParam(g_hInst, (LPCTSTR)IDD_CONFERENCE, parent, dialogProc, (LPARAM)p);
}
