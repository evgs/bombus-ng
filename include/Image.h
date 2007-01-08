#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>

class Image {
public:
    Image(LPCTSTR path);
    ~Image();

    void drawImage(HDC hdc, int x, int y) const;
protected:
    HBITMAP bmp;
};
typedef boost::shared_ptr<Image> ImageRef;
