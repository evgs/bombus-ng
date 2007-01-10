#pragma once

#include <windows.h>

class OwnerDrawRect {
public:
    virtual int getWidth()=0;
    virtual int getHeight()=0;

    virtual void draw(HDC hdc, RECT &rt);
};
