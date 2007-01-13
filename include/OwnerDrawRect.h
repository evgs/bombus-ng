#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>

class OwnerDrawRect {
public:
    virtual ~OwnerDrawRect(){};

    virtual int getWidth() const =0;
    virtual int getHeight() const =0;
    virtual int getColor() const =0;

    virtual void draw(HDC hdc, RECT &rt) const=0;
};

typedef boost::shared_ptr<OwnerDrawRect> ODRRef;

