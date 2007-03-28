#pragma once

#include <windows.h>

namespace mru {
    void readMru(const wchar_t * mruName, HWND hDlg, int itemId, const wchar_t * defValue);
    void saveMru(const wchar_t * mruName, HWND hDlg, int itemId);
    void processMruKey(LPCWSTR mruName, int keyIndex, LPWSTR buf, int bufSize, bool write);

}