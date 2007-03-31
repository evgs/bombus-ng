#include "MRU.h"
#include <string>
#include <wchar.h>
#include "Serialize.h"
#include "utf8.hpp"
#include "DlgUtils.h"

extern std::wstring appRootPath;

void mru::readMru(const wchar_t * mruName, HWND hDlg, int itemId, const wchar_t * defValue) {
    std::wstring name(L"config\\mru\\mru-");
    name+=mruName;
    Serialize s(name.c_str(), true);
    std::string value;
    while (true) {
        value.clear();
        s.streamString(value);
        if (value.length()==0) break;
        SendDlgItemMessage(hDlg, itemId, CB_ADDSTRING, 0, (LPARAM) utf8::utf8_wchar(value).c_str());
    }
    SendDlgItemMessage(hDlg, itemId, CB_SETCURSEL, 0, 0);
    s.close();
}

void mru::saveMru(const wchar_t * mruName, HWND hDlg, int itemId) {
    std::wstring name=appRootPath;
    name+=L"config";
    CreateDirectory(name.c_str(), NULL);
    name+=L"\\mru";
    CreateDirectory(name.c_str(), NULL);
    name=L"config\\mru\\mru-";
    name+=mruName;
    Serialize s(name.c_str(), false);

    int nRecords=0;

    std::string item1=GetDlgItemText(hDlg, itemId);
    s.streamString(item1);

    std::string item2;
    int i=0;
    wchar_t buf[1024];
    while (nRecords<10) {
        int result=SendDlgItemMessage(hDlg, itemId, CB_GETLBTEXT, i, (LPARAM) buf);
        if (result==CB_ERR) break;
        item2=utf8::wchar_utf8(buf);
        i++;
        if (item2==item1) continue;
        s.streamString(item2);
        nRecords++;
    }
    s.close();
}
