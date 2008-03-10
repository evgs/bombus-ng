#pragma once

#include "boostheaders.h"
#include <windows.h>
#include <list>

class ODR {
public:
    virtual ~ODR(){};

    virtual int getWidth() const =0;
    virtual int getHeight() const =0;
    virtual int getColor() const =0;

    virtual void measure(HDC hdc, RECT &rt) {
        rt.bottom=rt.top+getHeight();
        rt.right=rt.left+getWidth();
    }

    virtual void draw(HDC hdc, RECT &rt) const=0;

    virtual const wchar_t * getText() const =0;
};

typedef boost::shared_ptr<ODR> ODRRef;

