#include "TimeFunc.h"

PackedTime strtime::getCurrentUtc(){
    PackedTime result;
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &result);
    return result;
}

std::string strtime::toDate(const PackedTime &utcTime) {
    PackedTime local;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&utcTime, &local);
    FileTimeToSystemTime(&local, &st);

    char timeBuf[16];
    sprintf(timeBuf, "%02d.%02d.%04d", st.wDay, st.wMonth, st.wYear);

    return std::string(timeBuf);
}

std::string strtime::toTime(const PackedTime &utcTime) {
    PackedTime local;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&utcTime, &local);
    FileTimeToSystemTime(&local, &st);

    char timeBuf[16];
    sprintf(timeBuf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

    return std::string(timeBuf);
}

int extractInt(const std::string &s, int nbegin, int nend) {
    int result=0;
    for (int index=nbegin; index<nend; index++) {
        result*=10;
        result+=s[index]-'0';
    }
    return result;
}
PackedTime strtime::PackIso8601(const std::string &time) {
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    PackedTime t={0,0};
    if (time.length()<17) return t;
    // 0123456789abcdef0
    // CCYYMMDDThh:mm:ss
    if (time[8]=='T') {
        st.wYear=extractInt(time, 0, 4);
        st.wMonth=extractInt(time, 4, 6);
        st.wDay=extractInt(time, 6, 8);
        st.wHour=extractInt(time, 9, 11);
        st.wMinute=extractInt(time, 12, 14);
        st.wSecond=extractInt(time, 15, 17);
    } //TODO CCYY-MM-DDThh:mm:ss[.sss]TZD
    SystemTimeToFileTime(&st, &t);
    return t;
}