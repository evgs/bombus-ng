#pragma once

void GetDlgItemText(HWND hDlg, int itemId, std::string &dest);
std::string GetDlgItemText(HWND hDlg, int itemId);
void SetDlgItemText(HWND hDlg, int itemId, const std::string &data);
void AddComboString(HWND hDlg, int itemId, const std::string &data);
