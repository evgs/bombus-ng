#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>

class Image {
public:
    Image(LPCTSTR path);
    ~Image();

    void drawImage(HDC hdc, int x, int y) const;
    void createMask();
protected:
    HBITMAP bmp;
    HBITMAP mask;
};
typedef boost::shared_ptr<Image> ImageRef;
