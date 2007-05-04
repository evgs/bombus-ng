#pragma once

#include <windows.h>

namespace mru {
    void readMru(const wchar_t * mruName, HWND hComboBox, const wchar_t * defValue);
    void readMru(const wchar_t * mruName, HWND hDlg, int dlgItemId, const wchar_t * defValue);
    void saveMru(const wchar_t * mruName, HWND hComboBox);
    void saveMru(const wchar_t * mruName, HWND hDlg, int dlgItemId);
}