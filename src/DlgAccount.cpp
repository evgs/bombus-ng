//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberAccount.h"

#include "DlgAccount.h"
#include "DlgUtils.h"

#include "..\vs2005\ui\resourceppc.h"

#include "utf8.hpp"
#include "stringutils.h"

static JabberAccountRef dlgAccountParam;


void GetDlgItemText(HWND hDlg, int itemId, std::string &dest) {
    wchar_t buf[1024];
    ::GetDlgItemText(hDlg, itemId, buf, sizeof(buf));
    dest=utf8::wchar_utf8(buf);
}
std::string GetDlgItemText(HWND hDlg, int itemId) {
    wchar_t buf[1024];
    ::GetDlgItemText(hDlg, itemId, buf, sizeof(buf));
    return utf8::wchar_utf8(buf);
}

void SetDlgItemText(HWND hDlg, int itemId, const std::string &data) {
    ::SetDlgItemText(hDlg, itemId, utf8::utf8_wchar(data).c_str());
}
void AddComboString(HWND hDlg, int itemId, const std::string &data) {
    SendDlgItemMessage(hDlg, itemId, CB_ADDSTRING, 0, (LPARAM) utf8::utf8_wchar(data).c_str());
}

void DlgAccountItemStates(HWND hDlg) {
    int state=IsDlgButtonChecked(hDlg, IDC_X_NSRV);
    EnableWindow(GetDlgItem(hDlg, IDC_E_HOSTIP), state==BST_CHECKED);
    EnableWindow(GetDlgItem(hDlg, IDC_E_PORT), state==BST_CHECKED);
}

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

            SetDlgItemText(hDlg, IDC_E_JID, dlgAccountParam->getBareJid());
            SetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
            SetDlgItemText(hDlg, IDC_E_RESOURCE, dlgAccountParam->getResource());
            SetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp);
            SetDlgItemInt(hDlg, IDC_E_PORT, dlgAccountParam->port, false);
            CheckDlgButton(hDlg, IDC_X_SSL, (dlgAccountParam->useEncryption)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_SSL_WARNINGS, (dlgAccountParam->ignoreSslWarnings)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_PLAIN, (dlgAccountParam->plainTextPassword)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_SASL, (dlgAccountParam->useSASL)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_ZLIB, (dlgAccountParam->useCompression)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_X_NSRV, (!dlgAccountParam->useSRV)?BST_CHECKED:BST_UNCHECKED);

            //finally
            DlgAccountItemStates(hDlg);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
            std::string myjid=GetDlgItemText(hDlg, IDC_E_JID);
            std::trim(myjid);
            if (!verifyJid(hDlg, myjid)) return TRUE;
            dlgAccountParam->setBareJid(myjid);
            GetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
            dlgAccountParam->setResource(GetDlgItemText(hDlg, IDC_E_RESOURCE));
            GetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp); std::trim(dlgAccountParam->hostNameIp);

            dlgAccountParam->port=GetDlgItemInt(hDlg, IDC_E_PORT, NULL, false);

            dlgAccountParam->useEncryption=IsDlgButtonChecked(hDlg, IDC_X_SSL)==BST_CHECKED;
            dlgAccountParam->ignoreSslWarnings=IsDlgButtonChecked(hDlg, IDC_X_SSL_WARNINGS)==BST_CHECKED;
            dlgAccountParam->plainTextPassword=IsDlgButtonChecked(hDlg, IDC_X_PLAIN)==BST_CHECKED;
            dlgAccountParam->useSASL=IsDlgButtonChecked(hDlg, IDC_X_SASL)==BST_CHECKED;
            dlgAccountParam->useCompression=IsDlgButtonChecked(hDlg, IDC_X_ZLIB)==BST_CHECKED;
            dlgAccountParam->useSRV=!IsDlgButtonChecked(hDlg, IDC_X_NSRV);

            dlgAccountParam->saveAccount(TEXT("defAccount.bin"));

			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}

        if (HIWORD(wParam)==BN_CLICKED) {
            if (LOWORD(wParam)==IDC_X_NSRV) 
                DlgAccountItemStates(hDlg);
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

//////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgAccountP1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void DialogAccountMP(HINSTANCE g_hInst, HWND parent, JabberAccountRef accnt) {
    dlgAccountParam=accnt;

    PROPSHEETPAGE pages[2];
    pages[0].dwSize=sizeof(PROPSHEETPAGE);
    pages[1].dwSize=sizeof(PROPSHEETPAGE);

    pages[0].hInstance=g_hInst;
    pages[1].hInstance=g_hInst;

    pages[0].dwFlags=PSP_DEFAULT;
    pages[1].dwFlags=PSP_DEFAULT;

    pages[0].pszTemplate=(LPCTSTR)IDD_ACCNT1;
    pages[1].pszTemplate=(LPCTSTR)IDD_ACCNT2;

    pages[0].pszTitle=L"Account";
    pages[1].pszTitle=L"Connection";

    pages[0].pfnDlgProc=DlgAccountP1;
    pages[1].pfnDlgProc=DlgAccountP1;

    pages[0].lParam=0;
    pages[1].lParam=1;

    PROPSHEETHEADER psh;
    psh.dwSize=sizeof(PROPSHEETHEADER);
    psh.dwFlags=PSH_PROPSHEETPAGE ;
    psh.hwndParent=parent;
    psh.hInstance=g_hInst;
    psh.pszCaption=L"Account";
    psh.nPages=2;
    psh.nStartPage=0;
    psh.ppsp=pages;


    PropertySheet(&psh);

    dlgAccountParam->saveAccount(TEXT("defAccount.bin"));
}



INT_PTR CALLBACK DlgAccountP1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
            //SHInitDialog(&shidi);

            if (lParam==0) {
                SetDlgItemText(hDlg, IDC_E_JID, dlgAccountParam->getBareJid());
                SetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
                SetDlgItemText(hDlg, IDC_E_RESOURCE, dlgAccountParam->getResource());
            } else {
                SetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp);
                SetDlgItemInt(hDlg, IDC_E_PORT, dlgAccountParam->port, false);
                CheckDlgButton(hDlg, IDC_X_SSL, (dlgAccountParam->useEncryption)?BST_CHECKED:BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_X_SSL_WARNINGS, (dlgAccountParam->ignoreSslWarnings)?BST_CHECKED:BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_X_PLAIN, (dlgAccountParam->plainTextPassword)?BST_CHECKED:BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_X_SASL, (dlgAccountParam->useSASL)?BST_CHECKED:BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_X_ZLIB, (dlgAccountParam->useCompression)?BST_CHECKED:BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_X_NSRV, (!dlgAccountParam->useSRV)?BST_CHECKED:BST_UNCHECKED);
                DlgAccountItemStates(hDlg);
            }

            //finally
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            if (lParam==0) {
                std::string myjid=GetDlgItemText(hDlg, IDC_E_JID);
                std::trim(myjid);
                if (!verifyJid(hDlg, myjid)) return TRUE;
                dlgAccountParam->setBareJid(myjid);
                GetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
                dlgAccountParam->setResource(GetDlgItemText(hDlg, IDC_E_RESOURCE));
            } else {
                GetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp); std::trim(dlgAccountParam->hostNameIp);

                dlgAccountParam->port=GetDlgItemInt(hDlg, IDC_E_PORT, NULL, false);

                dlgAccountParam->useEncryption=IsDlgButtonChecked(hDlg, IDC_X_SSL)==BST_CHECKED;
                dlgAccountParam->ignoreSslWarnings=IsDlgButtonChecked(hDlg, IDC_X_SSL_WARNINGS)==BST_CHECKED;
                dlgAccountParam->plainTextPassword=IsDlgButtonChecked(hDlg, IDC_X_PLAIN)==BST_CHECKED;
                dlgAccountParam->useSASL=IsDlgButtonChecked(hDlg, IDC_X_SASL)==BST_CHECKED;
                dlgAccountParam->useCompression=IsDlgButtonChecked(hDlg, IDC_X_ZLIB)==BST_CHECKED;
                dlgAccountParam->useSRV=!IsDlgButtonChecked(hDlg, IDC_X_NSRV);
            }
            return TRUE;
        }

        if (HIWORD(wParam)==BN_CLICKED) {
            if (lParam==1) if (LOWORD(wParam)==IDC_X_NSRV) 
                DlgAccountItemStates(hDlg);
        }

        if (LOWORD(wParam) == IDCANCEL) {
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, message);
        return TRUE;
    }
    return (INT_PTR)FALSE;
}
