#pragma once

#include "OwnerDrawRect.h"
#include "Image.h"
#include <windows.h>

class IconTextElement : public OwnerDrawRect {
public:
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getColor() const =0;

    virtual void draw(HDC hdc, RECT &rt) const;
protected:
    enum {
        ICON_SPACING=4
    };
    int width;
    int height;
    void init();
    virtual const wchar_t * getText() const=0;
    virtual int getIconIndex() const=0;
    //ImgListRef il;

};