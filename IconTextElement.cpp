#include "IconTextElement.h"

extern ImgListRef skin;


//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////

void IconTextElement::init() {
    HDC tmp=CreateCompatibleDC(NULL);
    RECT r={0,0,10,10};
    DrawText(tmp, getText(), -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP);
    width=r.right+skin->getElementWidth()+ICON_SPACING;
    height=r.bottom;  
    DeleteDC(tmp);
}

void IconTextElement::draw(HDC hdc, RECT &rt) const {
    SetBkMode(hdc, TRANSPARENT);
    skin->drawElement(hdc, getIconIndex(), rt.left, rt.top);
    rt.left+=skin->getElementWidth()+ICON_SPACING;
    DrawText(hdc, getText(), -1, &rt, DT_LEFT | DT_TOP | DT_END_ELLIPSIS);
}

int IconTextElement::getWidth() const { return width;}
int IconTextElement::getHeight() const { return height; }

IconTextElementContainer::IconTextElementContainer( std::wstring &wstr, int icon ) {
    this->iconIndex=icon;
    this->wstr=wstr;
    init();
}
int IconTextElementContainer::getIconIndex() const { return iconIndex; }

const wchar_t * IconTextElementContainer::getText() const { return wstr.c_str(); }

int IconTextElementContainer::getColor() const { return 0; }