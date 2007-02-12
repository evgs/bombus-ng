#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>
#include <aygshell.h>

#include "Wnd.h"

class TabsCtrl : public Wnd{
public:
    TabsCtrl(HWND parent);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void addWindow (const WndRef &wnd);
    void showActiveTab();
    bool switchByWndRef(WndRef targetWnd);
    bool switchByODR(ODRRef targetWnd);
    WndRef getWindowByODR(ODRRef const &title);

    void fwdWMCommand(int wmId);
protected:
    HWND tabScrollHWnd;

    struct TabInfo {
        int tabWidth;
        //int tabDispWidth;
        int tabXPos;
        WndRef wndChild;
    };
    typedef boost::shared_ptr<TabInfo> TabInfoRef;
    typedef std::list<TabInfoRef> TabList;

    int xOffset;
    TabList tabs;
    TabList::iterator activeTab;

    bool makeTabLayout;
    void tabDoLayout(/*HDC hdc*/);
    void updateChildsLayout();

    HMENU getContextMenu();
    void releaseContextMenu();

private:

    static ATOM windowClass;
    ATOM RegisterWindowClass();
    static void drawTab(HDC hdc, int offset, TabInfoRef tab, bool active);
    /*int width;
    int height;*/

    HMENU hmenu;
};

typedef boost::shared_ptr<TabsCtrl> TabsCtrlRef;