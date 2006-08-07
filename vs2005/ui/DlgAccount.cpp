#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>

#include "ResourceContext.h"
#include "JabberAccount.h"

#include "DlgAccount.h"

#include "resourceppc.h"

#include "utf8.hpp"

static JabberAccountRef dlgAccountParam;


INT_PTR CALLBACK DlgAccount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

            SetDlgItemText(hDlg, IDC_E_JID, utf8::utf8_wchar(dlgAccountParam->getBareJid()).c_str());
            SetDlgItemText(hDlg, IDC_E_PASSWORD, utf8::utf8_wchar(dlgAccountParam->password).c_str());
            SetDlgItemText(hDlg, IDC_E_RESOURCE, utf8::utf8_wchar(dlgAccountParam->getResource()).c_str());
            SetDlgItemText(hDlg, IDC_E_HOSTIP, utf8::utf8_wchar(dlgAccountParam->hostNameIp).c_str());
            SetDlgItemInt(hDlg, IDC_E_PORT, dlgAccountParam->port, false);
            CheckDlgButton(hDlg, IDC_X_SSL, (dlgAccountParam->useEncryption)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_PLAIN, (dlgAccountParam->plainTextPassword)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_SASL, (dlgAccountParam->useSASL)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_ZLIB, (dlgAccountParam->useCompression)?BST_CHECKED:BST_UNCHECKED);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
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

void DialogAccount(HINSTANCE g_hInst, HWND parent, JabberAccountRef accnt) {
    dlgAccountParam=accnt;
    DialogBox(g_hInst, (LPCTSTR)IDD_ACCNT, parent, DlgAccount);
}

