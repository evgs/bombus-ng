#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>

class Image {
public:
    Image(LPCTSTR path);
    virtual ~Image();

    void drawImage(HDC hdc, int x, int y) const;
    void createMask();
protected:
    HBITMAP bmp;
    HBITMAP mask;
    Image();
private:
};
typedef boost::shared_ptr<Image> ImageRef;

class ImgList : private Image {
public:
    ImgList(LPCTSTR path);
    void setGridSize(int nColumns, int nRows);

    virtual void drawElement (HDC hdc, int index, int x, int y) const;
    int getElementWidth() const { return elWidth; }
private:
    int nColumns;
    int nRows;
    int elWidth;
    int elHeight;
};
typedef boost::shared_ptr<ImgList> ImgListRef;

