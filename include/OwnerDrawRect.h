#pragma once

#include <boost/shared_ptr.hpp>
#include <windows.h>
#include <list>

class ODR {
public:
    virtual ~ODR(){};

    virtual int getWidth() const =0;
    virtual int getHeight() const =0;
    virtual int getColor() const =0;

    virtual void draw(HDC hdc, RECT &rt) const=0;

    virtual const wchar_t * getText() const =0;
};

typedef boost::shared_ptr<ODR> ODRRef;

