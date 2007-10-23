//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberStream.h"

#include "DlgAddEditContact.h"
#include "DlgUtils.h"

#include "JabberDataBlockListener.h"
#include "Roster.h"

#include "..\vs2005\ui\resourceppc.h"

#include "basetypes.h"
#include "utf8.hpp"
#include "stringutils.h"

extern HINSTANCE	g_hInst;			// current instance
extern RosterListView::ref rosterWnd;


//////////////////////////////////////////////////////////////////////////
class GetVcardNick : public JabberDataBlockListener {
public:
    GetVcardNick(std::string &jid, HWND targetDlg, int targetCtrl);
    ~GetVcardNick(){};
    virtual const char * getType() const{ return NULL; }
    virtual const char * getId() const{ return id.c_str(); }
    virtual const char * getTagName() const { return "iq"; }
    virtual ProcessResult blockArrived(JabberDataBlockRef block, const ResourceContextRef rc);
    void doRequest(ResourceContextRef rc);
private:
    std::string jid;
    std::string id;
    HWND targetDlg;
    int targetCtrl;
    ResourceContextRef rc;
};

GetVcardNick::GetVcardNick(std::string &jid, HWND targetDlg, int targetCtrl) {
    this->jid=jid;
    this->targetDlg=targetDlg;
    this->targetCtrl=targetCtrl;
    id="cf#";
    id+=jid;
}

void GetVcardNick::doRequest(ResourceContextRef rc) {
    JabberDataBlock req("iq");
    req.setAttribute("to", jid);
    req.setAttribute("type", "get");
    req.setAttribute("id", id);

    req.addChildNS("vCard", "vcard-temp");

    rc->jabberStream->sendStanza(req);
}

ProcessResult GetVcardNick::blockArrived(JabberDataBlockRef block, const ResourceContextRef rc){
    std::string &type=block->getAttribute("type");
    std::string nick;
    if (type=="result") {
        JabberDataBlockRef vcard=block->getChildByName("vCard");
        if (vcard) {
            nick=vcard->getChildText("NICKNAME");
            if (nick.length()==0)
                nick=vcard->getChildText("FN");
        }

        SetDlgItemText(targetDlg, targetCtrl, nick);
    }

    return LAST_BLOCK_PROCESSED;
}

//////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgAddEditContact::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DlgAddEditContact *p=(DlgAddEditContact *) GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:
		{
            p=(DlgAddEditContact *) lParam;
            SetWindowLong(hDlg, GWL_USERDATA, (LONG) p );
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

            if (p->contact) { 
                SetDlgItemText(hDlg, IDC_E_JID, p->contact->rosterJid);
                SetDlgItemText(hDlg, IDC_E_NICK, p->contact->nickname);
                SetDlgItemText(hDlg, IDC_C_GROUP, p->contact->group);
            }

            StringVectorRef groups=p->rc->roster->getRosterGroups();

            for (size_t i=0; i<groups->size(); i++)
                AddComboString(hDlg, IDC_C_GROUP, (*groups)[i]);


            CheckDlgButton(hDlg, IDC_X_SUBSCR, BST_CHECKED);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
        if (LOWORD(wParam==ID_VCARD_NICK)) {
            std::string jid; GetDlgItemText(hDlg, IDC_E_JID, jid);
            if (!verifyJid(hDlg, jid)) return TRUE;

            GetVcardNick *getNick=new GetVcardNick(jid, hDlg, IDC_E_NICK);
            p->rc->jabberStanzaDispatcherRT->addListener(JabberDataBlockListenerRef(getNick));
            getNick->doRequest(p->rc);
            return TRUE;
        }

		if (LOWORD(wParam) == IDOK)
		{
            std::string jid; GetDlgItemText(hDlg, IDC_E_JID, jid);
            std::trim(jid);
            if (!verifyJid(hDlg, jid)) return TRUE;
            std::string group; GetDlgItemText(hDlg, IDC_C_GROUP, group);
            std::string nick; GetDlgItemText(hDlg, IDC_E_NICK, nick);

            p->rc->roster->rosterSet(
                (nick.length()==0)? NULL : nick.c_str(), 
                jid.c_str(), 
                (group.length()==0)? NULL: group.c_str(), 
                NULL);

            if (p->edit) 
                if (IsDlgButtonChecked(hDlg, IDC_X_SUBSCR)==BST_CHECKED)
                    p->rc->sendPresence(jid.c_str(), presence::PRESENCE_AUTH_ASK, std::string(), 0);

			EndDialog(hDlg, LOWORD(wParam));
            delete p;
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
            delete p;
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, message);
        delete p;
		return TRUE;
	}
	return (INT_PTR)FALSE;
}

void DlgAddEditContact::createDialog(HWND parent, ResourceContextRef rc) {
    createDialog(parent, rc, Contact::ref());
}

void DlgAddEditContact::createDialog( HWND parent, ResourceContextRef rc, Contact::ref contact ) {
    /*dlgAccountParam=accnt;*/
    DlgAddEditContact *p=new DlgAddEditContact();
    p->parent=parent;
    p->rc=rc;
    p->contact=contact;

    p->edit=false;
    if (contact) {
        if (contact->subscr!="NIL") p->edit=true;
    }
    DialogBoxParam(g_hInst, 
        (p->edit)? (LPCTSTR)IDD_EDIT_CONTACT : (LPCTSTR)IDD_ADD_CONTACT ,
        parent, dialogProc, (LPARAM)p);
}
