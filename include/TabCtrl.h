#pragma once

#include "boostheaders.h"

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
    bool switchByODR(ODR * targetWnd);
    WndRef getWindowByODR(ODR * title);
    WndRef getWindowByODR(ODRRef title) { return getWindowByODR(title.get()); }

    void fwdWMCommand(int wmId);

    enum WmCommands {
        CLOSETAB=41000,
        SWITCH_TAB,
        USERCMD=42000
    };

protected:
    HWND tabScrollHWnd;

    virtual void menuUserCmds(HMENU hmenu) {};
    virtual void menuUserActions(int cmdId, DWORD userData) {};

    HMENU getContextMenu();

    void processPopupMenu(bool cmdBar, int posX, int posY);

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


    void init(HWND parent);

    TabsCtrl(){};

private:

    static ATOM windowClass;
    ATOM RegisterWindowClass();
    static void drawTab(HDC hdc, int offset, TabInfoRef tab, bool active);
    /*int width;
    int height;*/
};

typedef boost::shared_ptr<TabsCtrl> TabsCtrlRef;

class MainTabs : public TabsCtrl {

public:
    MainTabs(HWND parent);

protected:
    virtual void menuUserCmds(HMENU hmenu);
    virtual void menuUserActions(int cmdId, DWORD userData);

};