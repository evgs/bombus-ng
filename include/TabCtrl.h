#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include "Wnd.h"

class TabsCtrl : public Wnd{
public:
    TabsCtrl(HWND parent);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void addWindow (const WndRef &wnd);

protected:
    HWND tabScrollHWnd;

    struct TabInfo {
        int tabWidth;
        int tabDispWidth;
        int tabXPos;
        WndRef wndChild;
    };
    typedef boost::shared_ptr<TabInfo> TabInfoRef;
    typedef std::list<TabInfoRef> TabList;

    TabInfoRef activeTab;
    TabList tabs;

    bool makeTabLayout;
    void tabDoLayout(HDC hdc);

private:

    static ATOM windowClass;
    ATOM RegisterWindowClass();
    static void drawTab(HDC hdc, int offset, TabInfoRef tab, bool active);
    int width;
};

typedef boost::shared_ptr<TabsCtrl> TabsCtrlRef;