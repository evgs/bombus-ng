#include "Image.h"

#include <aygshell.h>

#define NORASTEROP (0x00AA0029)

Image::Image( LPCTSTR path ) {
    bmp=SHLoadImageFile(path);  
    mask=NULL;
}

Image::Image() {}

Image::~Image() {
    if (bmp) DeleteObject(bmp);
    if (mask) DeleteObject(mask);
}


int getColor(BITMAP &bm, int x, int y) {
    LPCHAR p=(LPCHAR) bm.bmBits+y*bm.bmWidthBytes;
    switch (bm.bmBitsPixel) {
        case 8: p+=x; return *p; 
        case 16: p+=x*2; return *((LPWORD)p); 
        case 24: p+=x*3; return *((LPWORD)p)  | *(p+2) ; 
        case 32: p+=x*4; return *((LPDWORD)p); 
    }
    return 0;
}
//////////////////////////////////////////////////////////////////////////
void Image::drawImage( HDC hdc, int x, int y ) const {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);
    HDC src=CreateCompatibleDC(NULL);
    SelectObject(src, bmp);

    //char *bits=new char()

    MaskBlt(hdc, x,y, bm.bmWidth, bm.bmHeight, 
            src, 0,0, 
            mask, 0, 0, 
            MAKEROP4(SRCCOPY, NORASTEROP));
    //AlphaBlend(hdc, x,y, bm.bmWidth, bm.bmHeight, src, 0,0, bm.bmWidth, bm.bmHeight, )
    DeleteDC(src);
}
//////////////////////////////////////////////////////////////////////////
void ImgList::drawElement( HDC hdc, int index, int x, int y ) const {
    //BITMAP bm;
    //GetObject(bmp, sizeof(bm), &bm);
    HDC src=CreateCompatibleDC(NULL);
    SelectObject(src, bmp);

    //char *bits=new char()

    int xm=(index&&0x0f) * elWidth;
    int ym=(index&&0xf0 >> 8) * elHeight;
    
    MaskBlt(hdc, x,y,  elWidth, elHeight, 
        src, xm, ym, 
        mask, xm, ym, 
        MAKEROP4(SRCCOPY, NORASTEROP));
    //AlphaBlend(hdc, x,y, bm.bmWidth, bm.bmHeight, src, 0,0, bm.bmWidth, bm.bmHeight, )
    DeleteDC(src);


}
//////////////////////////////////////////////////////////////////////////
void Image::createMask() {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);

    int bmaskWB=bm.bmWidth/8;
    char *bmask=new char[bmaskWB*bm.bmHeight];

    int maskColor=getColor(bm, 0, 0);

    LPCHAR p=bmask;
    for (int y=bm.bmHeight; y>0; y--) {
        for (int xb=0; xb<bmaskWB; xb++) {
            char buf;
            for (char b=0; b<8; b++) {
                buf=(buf<<1) | ((getColor(bm, xb*8+b, y-1)==maskColor)? 0:1);
            }
            *(p++)=buf;
        }
    }
    mask=CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, bmask);

    delete bmask;

}


void ImgList::setGridSize( int nColumns, int nRows ) {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);

    this->nColumns=nColumns;
    this->nRows=nRows;
    elWidth=bm.bmWidth/nColumns;
    elHeight=bm.bmHeight/nRows;
}

ImgList::ImgList( LPCTSTR path ) {
    bmp=SHLoadImageFile(path);  
    createMask();
}