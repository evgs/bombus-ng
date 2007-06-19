#include "MRU.h"
#include <string>
#include <wchar.h>
#include "Serialize.h"
#include "utf8.hpp"
#include "DlgUtils.h"

extern std::wstring appRootPath;

void mru::readMru(const wchar_t * mruName, HWND hDlg, int itemId, const wchar_t * defValue) {
    mru::readMru(mruName, GetDlgItem(hDlg, itemId), defValue);
}

void mru::readMru(const wchar_t * mruName, HWND hComboBox, const wchar_t * defValue) {
    std::wstring name(L"config\\mru\\mru-");
    name+=mruName;
    Serialize s(name.c_str(), true);
    std::string value;
    while (true) {
        value.clear();
        s.streamString(value);
        if (value.length()==0) break;
        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM) utf8::utf8_wchar(value).c_str());
    }
    SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
    s.close();
}

void mru::saveMru(const wchar_t * mruName, HWND hDlg, int itemId) {
    mru::saveMru(mruName, GetDlgItem(hDlg, itemId));
}

void mru::saveMru(const wchar_t * mruName, HWND hComboBox) {
    //using absolute path;
    std::wstring name=appRootPath;
    name+=L"config";
    CreateDirectory(name.c_str(), NULL);
    name+=L"\\mru";
    CreateDirectory(name.c_str(), NULL);
    //using relative path;
    name=L"config\\mru\\mru-";
    name+=mruName;
    Serialize s(name.c_str(), false);

    int nRecords=0;

    wchar_t buf[1024];
    SendMessage(hComboBox, WM_GETTEXT, 1024, (LPARAM)buf);
    std::string item1=utf8::wchar_utf8(buf);
    s.streamString(item1);

    std::string item2;
    int i=0;
    while (nRecords<10) {
        int result=SendMessage(hComboBox, CB_GETLBTEXT, i, (LPARAM) buf);
        if (result==CB_ERR) break;
        item2=utf8::wchar_utf8(buf);
        i++;
        if (item2==item1) continue;
        s.streamString(item2);
        nRecords++;
    }
    s.close();
}
