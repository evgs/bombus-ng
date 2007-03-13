#pragma once
#include "Contact.h"

class DlgAddEditContact {
private:
    DlgAddEditContact(){};
public:
    static void createDialog(HWND parent, ResourceContextRef rc);
    static void createDialog(HWND parent, ResourceContextRef rc, Contact::ref contact);
    static INT_PTR CALLBACK dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND parent;
    ResourceContextRef rc;
    Contact::ref contact;
};

