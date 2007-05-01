#pragma once
#include "ResourceContext.h"
#include <windows.h>
#include "jid.h"

class DlgMucJoin {
private:
    DlgMucJoin(){};
public:
    static void createDialog(HWND parent, ResourceContextRef rc, const std::string &jid);
    static INT_PTR CALLBACK dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    Jid jid;
    std::string pass;
    HWND parent;
    ResourceContextRef rc;
};

