#pragma once

#include "OwnerDrawRect.h"
#include "Image.h"
#include <windows.h>
#include <string>

class ResourceContext;
typedef boost::shared_ptr <ResourceContext> ResourceContextRef;


class IconTextElement : public ODR {
public:
    virtual int getWidth() const;
    virtual int getHeight() const;
    virtual int getColor() const =0;

    virtual void draw(HDC hdc, RECT &rt) const;

    virtual void createContextMenu(HMENU) const;
    virtual void onCommand(int cmdId, ResourceContextRef rc);
    //virtual void eventOk()=0;
    typedef boost::shared_ptr <IconTextElement> ref;


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
    IconTextElementContainer(const std::wstring &wstr, int icon);
    virtual int getColor() const;
    virtual const wchar_t * getText() const;
    virtual int getIconIndex() const;
protected:
    IconTextElementContainer(){};
    std::wstring wstr;
    int iconIndex;
};