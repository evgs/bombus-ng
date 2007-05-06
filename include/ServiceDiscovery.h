#pragma once

#include <boost/shared_ptr.hpp>

#include <windows.h>

#include <string>
#include <stack>
#include <utf8.hpp>

#include "Wnd.h"
#include "VirtualListView.h"

#include "OwnerDrawRect.h"

#include "ResourceContext.h"

class DiscoCommand: public IconTextElementContainer {
public:
    enum DiscoCmds {
        ERR=-1,   //[v]
        BACK=0,     //[v]
        REGISTER=1, //[ ]
        SEARCH=2,   //[ ]
        EXECUTE=3,  //[ ]
        VCARD=4,    //[v]
        JOINGC=5,   //[v]
        ADD=6       //[ ]
    };
    DiscoCommand(std::wstring cmdName, int icon, int cmdId);
    DiscoCmds cmdId;

    typedef boost::shared_ptr<DiscoCommand> ref;
    virtual int getColor() const;
};

class DiscoItem: public IconTextElementContainer {
public:
    DiscoItem(const std::string &jid, const std::string &node, const std::string &name);

    std::string jid;
    std::string node;
    std::string name;

    typedef boost::shared_ptr<DiscoItem> ref;
};

struct DiscoNode {
    std::string jid;
    std::string node;
    ODRListRef subnodes;
    ODRRef cursorPos;
};
//////////////////////////////////////////////////////////////////////////

class ServiceDiscovery : public Wnd{
protected:
    ServiceDiscovery(HWND parent);
public:
    //ChatView(HWND parent, const std::string & title);
    virtual ~ServiceDiscovery();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    //virtual const wchar_t * getWindowTitle() const;
    virtual const ODR * getODR() const;

    typedef boost::shared_ptr<ServiceDiscovery> ref;

    virtual void showWindow(bool show);

    void redraw();

    static ServiceDiscovery::ref createServiceDiscovery(
        HWND parent, 
        ResourceContextRef rc, 
        const std::string &jid, 
        const std::string &node, 
        bool go);

    JabberDataBlockRef itemReply;
    JabberDataBlockRef infoReply;

    void go();
    void discoverJid(const std::string &jid, const std::string &node);
    void back();
    void vcard();
    void joingc();
    void execute();

protected:
    std::stack<DiscoNode> nodes;

    VirtualListView::ref nodeList;
    HWND editWnd;
    int editHeight;
    int width;

    std::string jid;
    std::string node;

    std::string newNode;

    boost::weak_ptr<ServiceDiscovery> thisRef;

    ResourceContextRef rc;

    void parseResult();

private:
    static ATOM windowClass;
    ATOM RegisterWindowClass();
};
//////////////////////////////////////////////////////////////////////////

