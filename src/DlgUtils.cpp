#include "DlgUtils.h"
#include "jid.h"

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "utf8.hpp"

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

bool verifyJid( HWND hwnd, const std::string &jid ) {
    Jid j(jid);

    if (!j.isValid()) {
        MessageBox(hwnd, L"Please enter valid JID, like user@jabber.ru", L"Invalid Jabber ID", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }

    if (!j.getResource().empty()) {
        MessageBox(hwnd, L"JID should not contain resource.\nExample: user@jabber.ru,\nbut not user@jabber.ru/resource", L"Bare Jabber ID wanted", MB_OK | MB_ICONEXCLAMATION);
        return false;
    }
    return true;
}

int CALLBACK PropSheetCallback(HWND hwndDlg, UINT message, LPARAM lParam) {
    switch(message) {
        case PSCB_INITIALIZED:
            {
                HWND hwndChild = GetWindow(hwndDlg, GW_CHILD);
                while (hwndChild) {
                    TCHAR szTemp[32];
                    GetClassName(hwndChild, szTemp, 32);
                    if (_tcscmp(szTemp, _T("SysTabControl32"))==0)
                        break;
                    hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
                }
                if (hwndChild) {
                    DWORD dwStyle = GetWindowLong(hwndChild, GWL_STYLE) | TCS_BOTTOM;
                    ::SetWindowLong(hwndChild, GWL_STYLE, dwStyle);
                }
                break;
            }
        case PSCB_GETVERSION:
            return COMCTL32_VERSION;
    }
    return 0;
}

void SetDlgCheckBox( HWND hDlg, int itemId, const bool flag ) {
    CheckDlgButton(hDlg, itemId, (flag)?BST_CHECKED:BST_UNCHECKED);
}

void GetDlgCheckBox( HWND hDlg, int itemId, bool &flag ) {
    flag=IsDlgButtonChecked(hDlg, itemId)==BST_CHECKED;
}
