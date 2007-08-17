#pragma once

#include <windows.h>
#include <string>

typedef FILETIME PackedTime;

namespace strtime {
    std::string toLocalDate(const PackedTime &utcTime);
    std::string toLocalTime(const PackedTime &utcTime);
    std::string toLocalDateTime(const PackedTime &utcTime);
    std::string toIso8601(const PackedTime &utcTime);
	std::string toXep0080Time(const PackedTime &utcTime);

    PackedTime PackIso8601 (const std::string &time);
    PackedTime getCurrentUtc();
    std::string getLocalZoneOffset();

    std::string getRandom();
}

