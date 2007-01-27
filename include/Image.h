#pragma once

#include <windows.h>
#include <boost/shared_ptr.hpp>

class Image {
public:
    Image(LPCTSTR path);
    virtual ~Image();

    void drawImage(HDC hdc, int x, int y) const;
    void createMask();
protected:
    HBITMAP bmp;
    HBITMAP mask;
    Image();
private:
};
typedef boost::shared_ptr<Image> ImageRef;

class ImgList : private Image {
public:
    ImgList(LPCTSTR path);
    void setGridSize(int nColumns, int nRows);

    virtual void drawElement (HDC hdc, int index, int x, int y) const;
    int getElementWidth() const { return elWidth; }
private:
    int nColumns;
    int nRows;
    int elWidth;
    int elHeight;
};
typedef boost::shared_ptr<ImgList> ImgListRef;

namespace icons {
enum IconDef {
    ICON_INVISIBLE_INDEX = 0x10,
    ICON_ERROR_INDEX = 0x11,
    ICON_TRASHCAN_INDEX = 0x12,
    ICON_PROGRESS_INDEX = 0x13,
    ICON_PRIVACY_ACTIVE = 0x46,
    ICON_PRIVACY_PASSIVE = 0x47,

    ICON_SEARCH_INDEX = 0x14,

    ICON_REGISTER_INDEX = 0x15,

    ICON_MSGCOLLAPSED_INDEX = 0x16,

    ICON_MESSAGE_INDEX = 0x20,

    ICON_AUTHRQ_INDEX = 0x21,

    ICON_COMPOSING_INDEX = 0x22,
    ICON_AD_HOC=0x22,

    ICON_EXPANDED_INDEX = 0x23,

    ICON_COLLAPSED_INDEX = 0x24,

    ICON_MESSAGE_BUTTONS = 0x25,

    ICON_PROFILE_INDEX = 0x30,

    ICON_PRIVACY_ALLOW = 0x36,

    ICON_PRIVACY_BLOCK = 0x37,

    ICON_KEYBLOCK_INDEX = 0x17,

    ICON_MODERATOR_INDEX = 0x50,
    ICON_GROUPCHAT_INDEX = 0x40,
    ICON_GCJOIN_INDEX = 0x41
};

}