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

void DlgAccountItemStates(HWND hDlg) {
    int state=IsDlgButtonChecked(hDlg, IDC_X_NSRV);
    EnableWindow(GetDlgItem(hDlg, IDC_E_HOSTIP), state==BST_CHECKED);
    EnableWindow(GetDlgItem(hDlg, IDC_E_PORT), state==BST_CHECKED);
}

//////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK DlgProcAccount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int npage);
INT_PTR CALLBACK DlgProcAccountP1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    return DlgProcAccount(hDlg, message, wParam, lParam, 0);
}
INT_PTR CALLBACK DlgProcAccountP2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    return DlgProcAccount(hDlg, message, wParam, lParam, 1);
}
INT_PTR CALLBACK DlgProcAccountP3(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    return DlgProcAccount(hDlg, message, wParam, lParam, 2);
}

void DialogAccountMP(HINSTANCE g_hInst, HWND parent, JabberAccountRef accnt) {
    dlgAccountParam=accnt;

    PROPSHEETPAGE pages[3];
    pages[0].dwSize=sizeof(PROPSHEETPAGE);
    pages[1].dwSize=sizeof(PROPSHEETPAGE);
    pages[2].dwSize=sizeof(PROPSHEETPAGE);

    pages[0].hInstance=g_hInst;
    pages[1].hInstance=g_hInst;
    pages[2].hInstance=g_hInst;

    pages[0].dwFlags=PSP_DEFAULT;
    pages[1].dwFlags=PSP_DEFAULT;
    pages[2].dwFlags=PSP_DEFAULT;

    pages[0].pszTemplate=(LPCTSTR)IDD_ACCNT1;
    pages[1].pszTemplate=(LPCTSTR)IDD_ACCNT2;
    pages[2].pszTemplate=(LPCTSTR)IDD_ACCNT3;

    pages[0].pfnDlgProc=DlgProcAccountP1;
    pages[1].pfnDlgProc=DlgProcAccountP2;
    pages[2].pfnDlgProc=DlgProcAccountP3;

    pages[0].lParam=0;
    pages[1].lParam=1;
    pages[2].lParam=2;

    PROPSHEETHEADER psh;
    psh.dwSize=sizeof(PROPSHEETHEADER);
    psh.dwFlags=PSH_MAXIMIZE | PSH_PROPSHEETPAGE | PSH_USECALLBACK;
    psh.hwndParent=parent;
    psh.hInstance=g_hInst;
    psh.pszCaption=L"Account";
    psh.nPages=3;
    psh.nStartPage=0;
    psh.ppsp=pages;
	psh.pfnCallback = PropSheetCallback;

    PropertySheet(&psh);

    dlgAccountParam->saveAccount(TEXT("defAccount.bin"));
}


INT_PTR CALLBACK DlgProcAccount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int npage) {
    switch (message) {
    case WM_INITDIALOG:
        {
            // Create a Done button and size it.  
            SHINITDLGINFO shidi;
            shidi.dwMask = SHIDIM_FLAGS;
            shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
            shidi.hDlg = hDlg;
            //SHInitDialog(&shidi);

            if (npage==0) {
                SetDlgItemText(hDlg, IDC_E_JID, dlgAccountParam->getBareJid());
                SetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
                SetDlgItemText(hDlg, IDC_E_RESOURCE, dlgAccountParam->getResource());
            } 
            if (npage==1) {
                SetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp);
                SetDlgItemInt(hDlg, IDC_E_PORT, dlgAccountParam->port, false);
                SetDlgCheckBox(hDlg, IDC_X_SSL, dlgAccountParam->useEncryption);
                SetDlgCheckBox(hDlg, IDC_X_SSL_WARNINGS, dlgAccountParam->ignoreSslWarnings);
                SetDlgCheckBox(hDlg, IDC_X_PLAIN, dlgAccountParam->plainTextPassword);
                SetDlgCheckBox(hDlg, IDC_X_SASL, !dlgAccountParam->useSASL);
                SetDlgCheckBox(hDlg, IDC_X_ZLIB, dlgAccountParam->useCompression);
                SetDlgCheckBox(hDlg, IDC_X_NSRV, !dlgAccountParam->useSRV);
                DlgAccountItemStates(hDlg);
            }
            if (npage==2) {
                SetDlgCheckBox(hDlg, IDC_X_DIALUP, dlgAccountParam->networkUp);
            } 

            //finally
        }
        return (INT_PTR)TRUE;

    case WM_NOTIFY:
        {
            NMHDR* pnmh = (NMHDR*)lParam; 
            if (pnmh->code == PSN_APPLY) {

                if (npage==0) {
                    std::string myjid=GetDlgItemText(hDlg, IDC_E_JID);
                    std::trim(myjid);
                    if (!verifyJid(hDlg, myjid)) return TRUE;
                    dlgAccountParam->setBareJid(myjid);
                    GetDlgItemText(hDlg, IDC_E_PASSWORD, dlgAccountParam->password);
                    dlgAccountParam->setResource(GetDlgItemText(hDlg, IDC_E_RESOURCE));
                } 
                if (npage==1) {
                    GetDlgItemText(hDlg, IDC_E_HOSTIP, dlgAccountParam->hostNameIp); std::trim(dlgAccountParam->hostNameIp);

                    GetDlgCheckBox(hDlg, IDC_X_SSL, dlgAccountParam->useEncryption);
                    GetDlgCheckBox(hDlg, IDC_X_SSL_WARNINGS, dlgAccountParam->ignoreSslWarnings);
                    GetDlgCheckBox(hDlg, IDC_X_PLAIN, dlgAccountParam->plainTextPassword);
                    GetDlgCheckBox(hDlg, IDC_X_ZLIB, dlgAccountParam->useCompression);

                    dlgAccountParam->useSASL=!IsDlgButtonChecked(hDlg, IDC_X_SASL)==BST_CHECKED;
                    dlgAccountParam->useSRV=!IsDlgButtonChecked(hDlg, IDC_X_NSRV);
                }
                if (npage==2) {
                    GetDlgCheckBox(hDlg, IDC_X_DIALUP, dlgAccountParam->networkUp);
                }

                return TRUE;
            }
            return FALSE;

            return TRUE;
        }

    case WM_COMMAND:
        if (HIWORD(wParam)==BN_CLICKED) {
            if (npage==1) if (LOWORD(wParam)==IDC_X_NSRV) 
                DlgAccountItemStates(hDlg);
        }

        if (LOWORD(wParam) == IDCANCEL) {
            return TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
