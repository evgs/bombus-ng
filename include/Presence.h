#pragma once

#include "Image.h"

namespace presence {
    enum PresenceIndex {
        ONLINE=0,
        CHAT=1,
        AWAY=2,
        XA=3,
        DND=4,
        OFFLINE=5,
        ASK=6,
        UNKNOWN=7,
        INVISIBLE=icons::ICON_INVISIBLE_INDEX,
        PRESENCE_ERROR=icons::ICON_ERROR_INDEX,
        TRASH=icons::ICON_TRASHCAN_INDEX,
        NOCHANGE=-1,
        PRESENCE_AUTH=-1,
        PRESENCE_AUTH_ASK=-2,
        PRESENCE_AUTH_REMOVE=-3
    };
}