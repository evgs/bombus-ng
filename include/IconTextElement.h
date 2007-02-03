#pragma once

#include "OwnerDrawRect.h"
#include "Image.h"
#include <windows.h>
#include <string>

class IconTextElement : public ODR {
public:
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getColor() const =0;

    virtual void draw(HDC hdc, RECT &rt) const;

    //virtual void eventOk()=0;
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

class IconTextElementContainer : public IconTextElement {
public:
    IconTextElementContainer(std::wstring &wstr, int icon);
    virtual int getColor() const;
    virtual const wchar_t * getText() const;
    virtual int getIconIndex() const;
private:
    std::wstring wstr;
    int iconIndex;
};