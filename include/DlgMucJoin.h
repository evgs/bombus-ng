#pragma once
#include "ResourceContext.h"
#include <windows.h>

class DlgMucJoin {
private:
    DlgMucJoin(){};
public:
    static void createDialog(HWND parent, ResourceContextRef rc);
    static INT_PTR CALLBACK dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND parent;
    ResourceContextRef rc;
};

