#include "Smiles.h"
#include <string>
#include <windows.h>
#include <windowsx.h>
#include "utf8.hpp"

extern std::wstring skinRootPath;

BNode::BNode(){ 
    bTrue=bFalse=NULL;
    nodeChar=0;
    sIndex=-1;
}

BNode::~BNode(){
    if (bTrue) delete bTrue;
    if (bFalse) delete bFalse;
}


SmileParser::SmileParser(){
    root=NULL;
    loadSmiles();
}

SmileParser::~SmileParser(){
    delete root;
}

void SmileParser::loadSmiles() {
    icons=ImgListRef(new ImgArray(L"smiles.png", 16, -1));
    if (!icons) return;

    std::wstring path=skinRootPath;
    path+=L"smiles.txt";

    HANDLE file=CreateFile(path.c_str(), 
        GENERIC_READ , 
        FILE_SHARE_READ, NULL, 
        OPEN_EXISTING ,
        0, NULL);

    if (file==INVALID_HANDLE_VALUE) return;
    int size=GetFileSize(file, NULL);
    if (size>64000) {CloseHandle(file); return; }

    char *buf=new char[size+1];
    DWORD rd;
    ReadFile(file, buf, size, &rd, NULL);
    buf[rd+1]=NULL;

    char *p=buf;
    char *smileStart=p;
    uint smileIndex=0;

    root=new BNode();

    char c;
    while (c=*p++) {
        switch (c) {
            case 0x0d: *(p-1)=0; if (*p==0x0a) p++;
            case 0x0a: 
                *(p-1)=0;
                addSmile(smileStart, smileIndex);
                smileIndex++;
                smileStart=p;
                break;

            case 0x09:
                *(p-1)=0;
                addSmile(smileStart, smileIndex);
                smileStart=p;
        }

    }

    CloseHandle(file);
    delete buf;
}
void SmileParser::addSmile(const char *smile, uint index) {
    BNode *p=root;
    BNode *p1;

    if (smileAscii.size()<=index) smileAscii.push_back(std::string(smile));

    char c;
    while (c=*smile++) {
        p1=p;
        if (p->nodeChar==0) { 
            p->nodeChar=c;
            p=p1->bTrue=new BNode();
            continue;
        }
        while (1) {
            if (c==p1->nodeChar) { p=p1->bTrue ; break; }
            if (p1->bFalse) { p1=p1->bFalse; continue; }
            p1->bFalse=new BNode();
            p1=p1->bFalse;
            p1->nodeChar=c;
            p=p1->bTrue=new BNode();
            break;
        }
    }
    p->sIndex=index;
}

int SmileParser::findSmile(LPCTSTR *pstr){
    const wchar_t *p=*pstr;
    BNode *psmile=root;

    wchar_t c;
    while (c=*p) {
        if (psmile->nodeChar==c) {
            psmile=psmile->bTrue; 
            p++;
        } else {
            if (psmile->bFalse==NULL) break; //branch end
            psmile=psmile->bFalse; 
        }
    }
    int result=psmile->sIndex;
    if (result>=0) *pstr=p;
    return result;
}

//////////////////////////////////////////////////////////////////////////
extern HINSTANCE			g_hInst;
extern int tabHeight;

ATOM SmileBox::RegisterWindowClass() {
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = SmileBox::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusSML");

    return RegisterClass(&wc);
}
ATOM SmileBox::windowClass=0;

LRESULT CALLBACK SmileBox::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    SmileBox *p=(SmileBox *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
        {
            p=(SmileBox *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            break;
        }

    case WM_PAINT:

        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            int x=5; int y=5;
            int rowcnt=0;
            int iconWidth=p->parser->icons->getElementWidth() + 4;
            
            for (size_t i=0; i < p->parser->smileAscii.size(); i++){
                p->parser->icons->drawElement(hdc, i, x,y);
                x+=iconWidth; rowcnt++;
                if (rowcnt == p->nwidth ) {
                    rowcnt=0; y+=iconWidth;
                    x=5;
                }
            }

            EndPaint(hWnd, &ps);
            break;
        }

    case WM_SIZE: 
        { 
            //HDWP hdwp; 

            int height=GET_Y_LPARAM(lParam);
            int width=GET_X_LPARAM(lParam);
            SetRect(&(p->clientRect), 0, 0, width, height ); 

            break; 
        } 

    case WM_LBUTTONDOWN: 
    case WM_LBUTTONDBLCLK:
        {
            SetFocus(p->thisHwnd);
            int y=GET_Y_LPARAM(lParam);
            int x=GET_X_LPARAM(lParam);
            if (y<5) break;
            if (x<5) break;
            x-=5;
            y-=5;
            int width=p->parser->icons->getElementWidth()+4;
            int nx=x/width;
            if (nx>=p->nwidth) break;
            uint nsmile=nx + p->nwidth*(y/width);
            if (nsmile>=p->parser->smileAscii.size()) break;

            SendMessage(
                p->editBoxWnd, 
                EM_REPLACESEL, 
                TRUE, (LPARAM) utf8::utf8_wchar(p->parser->smileAscii[nsmile]).c_str());
        }

    case WM_KILLFOCUS:
        {
            SetFocus(p->editBoxWnd);
            DestroyWindow(p->thisHwnd);
            break;
        }

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window
        delete p;
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void SmileBox::showSmileBox(HWND editBoxWnd, int x, int y, SmileParser *parser) {
    SmileBox *b=new SmileBox();
    b->editBoxWnd=editBoxWnd;
    b->parser=parser;

    if (SmileBox::windowClass==0)
        windowClass=RegisterWindowClass();

    //calculate client rect
    RECT parent;
    GetClientRect(editBoxWnd, &parent);

    b->nwidth=(int)sqrt(parser->smileAscii.size());
    int total=parser->smileAscii.size();

    int iconwidth=parser->icons->getElementWidth() + 4;

    int maxwidth=(parent.right-parent.left)/iconwidth;
    if (b->nwidth>maxwidth) b->nwidth=maxwidth;

    int nheight=total/b->nwidth;
    if (total - nheight * b->nwidth) nheight++;

    int width=6 + b->nwidth*iconwidth;
    int height=6 + nheight*iconwidth;

    if (x+width>parent.right) x=parent.right-width-5;
    y-=height;
    if (y<0) y=0;

    if (windowClass==0) throw std::exception("Can't create window class");
    b->thisHwnd=CreateWindow((LPCTSTR)windowClass, _T("SmileBox"), WS_POPUP | WS_BORDER | WS_VISIBLE ,
        x, y, width, height, editBoxWnd, NULL, g_hInst, (LPVOID)b);
}