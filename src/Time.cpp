#include "Time.h"

PackedTime time::getCurrentUtc(){
    PackedTime result;
    GetSystemTime(result);
    return result;
}

std::string time::toDate(const PackedTime &utcTime) {
    PackedTime local;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&utcTime, &local);
    FileTimeToSystemTime(&local, &st);

    char timeBuf[10];
    sprintf_s(timeBuf, sizeof(timeBuf), "%02d.%02d.%04d", st.wDay, st.wMonth, st.wSecond);

}