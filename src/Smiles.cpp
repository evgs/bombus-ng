#include "Smiles.h"
#include <string>
#include <windows.h>

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
    int smileIndex=0;

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
void SmileParser::addSmile(const char *smile, int index) {
    BNode *p=root;
    BNode *p1;

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
