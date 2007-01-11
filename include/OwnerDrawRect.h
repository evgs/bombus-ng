#pragma once

#include <windows.h>

class OwnerDrawRect {
public:
    virtual ~OwnerDrawRect(){};

    virtual int getWidth() const =0;
    virtual int getHeight() const =0;
    virtual int getColor() const =0;

    virtual void draw(HDC hdc, RECT &rt) const=0;
};
