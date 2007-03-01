//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberAccount.h"

#include "DlgStatus.h"
#include "DlgUtils.h"

#include "..\vs2005\ui\resourceppc.h"


#include "utf8.hpp"


INT_PTR CALLBACK DlgStatus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("Online"));
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("Free for chat"));
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("Away"));
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("Extended Away"));
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("DND"));
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("Invisible"));
            SendDlgItemMessage(hDlg, IDC_C_STATUS, CB_ADDSTRING, 0, (LPARAM) TEXT("Offline"));
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
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return TRUE;
	}
	return (INT_PTR)FALSE;
}

void DialogStatus(HINSTANCE g_hInst, HWND parent, ResourceContextRef rc) {
    /*dlgAccountParam=accnt;*/
    DialogBox(g_hInst, (LPCTSTR)IDD_STATUS, parent, DlgStatus);
}

