#include "Image.h"

#include <aygshell.h>

#include <string>

#define NORASTEROP (0x00AA0029)

extern std::wstring appRootPath;

Image::Image( LPCTSTR path ) {
    std::wstring bmpPath=appRootPath+path;
    bmp=SHLoadImageFile(bmpPath.c_str());  
}

Image::Image() {}

Image::~Image() {
    if (bmp) DeleteObject(bmp);
    //if (mask) DeleteObject(mask);
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

    TransparentImage(hdc, x, y,  bm.bmWidth, bm.bmHeight, 
        bmp, 0, 0,  bm.bmWidth, bm.bmHeight, 
        transparentColor);
}
//////////////////////////////////////////////////////////////////////////
void ImgList::drawElement( HDC hdc, int index, int x, int y ) const {

    int xm=(index&0x0f) * elWidth;
    int ym=((index&0xf0) >> 4) * elHeight;
    
    TransparentImage(hdc, x,y, elWidth,elHeight, 
                     bmp, xm,ym, elWidth,elHeight,
                     transparentColor);
}
//////////////////////////////////////////////////////////////////////////
void Image::createMask() {


    HDC hdcImage=CreateCompatibleDC(NULL);
    SelectObject(hdcImage, bmp);

    transparentColor=GetPixel(hdcImage, 0, 0);

    DeleteDC(hdcImage);
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
    std::wstring bmpPath=appRootPath+path;
    bmp=SHLoadImageFile(bmpPath.c_str());  
    createMask();
}