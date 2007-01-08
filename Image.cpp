#include "Image.h"

#include <aygshell.h>

Image::Image( LPCTSTR path ) {
    bmp=SHLoadImageFile(path);    
}

Image::~Image() {
    if (bmp) DeleteObject(bmp);
}

void Image::drawImage( HDC hdc, int x, int y ) const {
    BITMAP bm;
    GetObject(bmp, sizeof(bm), &bm);
    HDC src=CreateCompatibleDC(NULL);
    SelectObject(src, bmp);
    BitBlt(hdc, x,y, bm.bmWidth, bm.bmHeight, src, 0,0, SRCCOPY);
    DeleteDC(src);
}