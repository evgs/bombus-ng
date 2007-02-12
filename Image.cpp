#include "Image.h"

#include <aygshell.h>

#include <string>

#define NORASTEROP (0x00AA0029)

extern std::wstring appRootPath;

Image::Image( LPCTSTR path ) {
    std::wstring bmpPath=appRootPath+path;
    bmp=SHLoadImageFile(bmpPath.c_str());  

    HDC hdcImage=CreateCompatibleDC(NULL);
    SelectObject(hdcImage, bmp);
    transparentColor=GetPixel(hdcImage, 0, 0);
    DeleteDC(hdcImage);
}

Image::Image() {}

Image::~Image() {
    if (bmp) DeleteObject(bmp);
    //if (mask) DeleteObject(mask);
}

void Image::drawImage( HDC hdc, int x, int y ) const {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);

    TransparentImage(hdc, x, y,  bm.bmWidth, bm.bmHeight, 
        bmp, 0, 0,  bm.bmWidth, bm.bmHeight, 
        transparentColor);
}
//////////////////////////////////////////////////////////////////////////
void ImgArray::drawElement( HDC hdc, int index, int x, int y ) const {

    int xm=(index&0x0f) * elWidth;
    int ym=((index&0xf0) >> 4) * elHeight;
    
    TransparentImage(hdc, x,y, elWidth,elHeight, 
                     bmp, xm,ym, elWidth,elHeight,
                     transparentColor);
}

void ImgArray::setGridSize( int nColumns, int nRows ) {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);

    this->nColumns=nColumns;
    this->nRows=nRows;
    elWidth=bm.bmWidth/nColumns;
    elHeight=bm.bmHeight/nRows;
}

ImgArray::ImgArray( LPCTSTR path, int nColumns, int nRows ) {
    std::wstring bmpPath=appRootPath+path;
    bmp=SHLoadImageFile(bmpPath.c_str()); 

    HDC hdcImage=CreateCompatibleDC(NULL);
    SelectObject(hdcImage, bmp);
    transparentColor=GetPixel(hdcImage, 0, 0);
    DeleteDC(hdcImage);

    setGridSize(nColumns, nRows);
}

ImgArray::~ImgArray(){
    if (bmp) DeleteObject(bmp);
}