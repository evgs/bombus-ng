#pragma once

#include <windows.h>
#include <string>

void GetDlgItemText(HWND hDlg, int itemId, std::string &dest);
std::string GetDlgItemText(HWND hDlg, int itemId);
void SetDlgItemText(HWND hDlg, int itemId, const std::string &data);
void AddComboString(HWND hDlg, int itemId, const std::string &data);

void SetDlgCheckBox(HWND hDlg, int itemId, const bool flag);
void GetDlgCheckBox(HWND hDlg, int itemId, bool &flag);

bool verifyJid(HWND hwnd, const std::string &jid);
int CALLBACK PropSheetCallback(HWND hwndDlg, UINT message, LPARAM lParam);
