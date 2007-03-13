#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>
#include <aygshell.h>

#include <vector>

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

    enum WmCommands {
        CLOSETAB=41000
    };

protected:
    HWND tabScrollHWnd;

    struct TabInfo {
        int tabWidth;
        //int tabDispWidth;
        int tabXPos;
        WndRef wndChild;
    };
    typedef boost::shared_ptr<TabInfo> TabInfoRef;
    typedef std::vector<TabInfoRef> TabList;

    int xOffset;
    TabList tabs;
    int /*TabList::iterator*/ activeTab;

    bool makeTabLayout;
    void tabDoLayout(/*HDC hdc*/);
    void updateChildsLayout();

    HMENU getContextMenu();


    enum actions {
        TAB_BEGIN_INDEX=40000,
        TAB_END_INDEX=40200
    };

private:

    static ATOM windowClass;
    ATOM RegisterWindowClass();
    static void drawTab(HDC hdc, int offset, TabInfoRef tab, bool active);
    /*int width;
    int height;*/
};

typedef boost::shared_ptr<TabsCtrl> TabsCtrlRef;