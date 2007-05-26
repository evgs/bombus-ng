#pragma once
#include <windows.h>
enum {
    ADD_SMILE=WM_USER,
    SHELLNOTIFYICON,
    WM_FORWARD_STANZA,  //TODO: remove
    WM_NOTIFY_BLOCKARRIVED, // TODO: remove after single-threading
    WM_HTML_UPDATE,
    WM_VIRTUALLIST_REPLACE
};