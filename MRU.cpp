#include "MRU.h"
#include <string>
#include <wchar.h>

void mru::processMruKey(LPCWSTR mruName, int keyIndex, LPWSTR buf, int bufSize, bool write) {
    HKEY hkey;
    DWORD op;
    std::wstring name(L"Software\\Bombus\\Mru\\");
    name+=mruName;

    wchar_t valName[16];
    _itow_s(keyIndex, valName, sizeof(valName), 10);

    LONG result=RegCreateKeyEx(HKEY_CURRENT_USER, name.c_str(), NULL, NULL, REG_OPTION_VOLATILE, 0, NULL, &hkey, &op);
    if (write) {
        RegSetValueEx(hkey, valName, NULL, REG_SZ, (BYTE *)buf, (wcslen(buf)+1)*2);
    } else {
        DWORD bufsz=bufSize *2;
        DWORD type=REG_SZ;
        RegQueryValueEx(hkey, valName, NULL, &type, (BYTE *)buf, &bufsz );
    }
}

void mru::readMru(const wchar_t * mruName, HWND hDlg, int itemId, const wchar_t * defValue) {
}

void mru::saveMru(const wchar_t * mruName, HWND hDlg, int itemId) {

}
