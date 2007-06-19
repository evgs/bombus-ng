#pragma once

#include <windows.h>
#include <string>

typedef FILETIME PackedTime;

namespace strtime {
    std::string toDate(const PackedTime &utcTime);
    std::string toTime(const PackedTime &utcTime);

    PackedTime PackIso8601 (const std::string &time);
    PackedTime getCurrentUtc();

    std::string getRandom();
}

