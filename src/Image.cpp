#include "Image.h"

#include <aygshell.h>

#include <string>
#include <utf8.hpp>
#include "lstring.h"

#define NORASTEROP (0x00AA0029)

extern std::wstring skinRootPath;

Image::Image( LPCTSTR path ) {
    //std::wstring bmpPath=skinRootPath+path;
    //bmp=SHLoadImageFile(bmpPath.c_str());  
    bmp=SHLoadImageFile(path);  

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
    std::wstring bmpPath=skinRootPath+path;
    bmp=SHLoadImageFile(bmpPath.c_str()); 
    //if (bmp==NULL) throw

    HDC hdcImage=CreateCompatibleDC(NULL);
    SelectObject(hdcImage, bmp);
    transparentColor=GetPixel(hdcImage, 0, 0);
    DeleteDC(hdcImage);

    setGridSize(nColumns, nRows);
}

ImgArray::~ImgArray(){
    if (bmp) DeleteObject(bmp);
}

//////////////////////////////////////////////////////////////////////////
Skin::Skin( LPCTSTR path )  {
    names.push_back("");
    iconset.push_back(ImgListRef(new ImgArray(TEXT("skin.png"), 8, 6)));

    std::wstring srcPath=skinRootPath+path+TEXT("transports\\*.*");

    WIN32_FIND_DATA wfd;

    HANDLE sf=FindFirstFile(srcPath.c_str(), &wfd);
    if (sf==INVALID_HANDLE_VALUE) return;
    do {
        std::wstring wname(path);
        wname+=TEXT("transports\\");
        wname+=wfd.cFileName;
        iconset.push_back(ImgListRef(new ImgArray(wname.c_str(), 8, 1)));

        std::string name=utf8::wchar_utf8(wfd.cFileName);
        int dot=name.find('.');
        name.erase(dot, name.length()-dot);
        locale::toLowerCase(name);
        names.push_back(name);
    } while (FindNextFile(sf, &wfd));
    FindClose(sf);
}

void Skin::drawElement( HDC hdc, int index, int x, int y ) const {
    unsigned int sIndex=index>>8;
    if (sIndex>=iconset.size()) return;
    iconset[sIndex]->drawElement(hdc, index &0xff, x, y);
}

int Skin::getElementWidth() const{
    return iconset.front()->getElementWidth();
}

int Skin::getElementHeight() const {
    return iconset.front()->getElementHeight();
}
Skin::~Skin() {}

int Skin::getBaseIndex( const std::string &setName ) {
    for (int i=names.size()-1; i>0; i--) {
        if (setName==names[i]) return i<<8;
    }
    return 0;
}