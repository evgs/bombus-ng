#include "TimeFunc.h"
#include "stringutils.h"

#include "boostheaders.h"

PackedTime strtime::getCurrentUtc(){
    PackedTime result;
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &result);
    return result;
}

std::string strtime::toLocalDate(const PackedTime &utcTime) {
    PackedTime local;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&utcTime, &local);
    FileTimeToSystemTime(&local, &st);

    boost::format fd("%02d.%02d.%04d");
    fd % st.wDay % st.wMonth % st.wYear;

    return fd.str();
}

std::string strtime::toLocalTime(const PackedTime &utcTime) {
    PackedTime local;
    SYSTEMTIME st;
    FileTimeToLocalFileTime(&utcTime, &local);
    FileTimeToSystemTime(&local, &st);

    boost::format tf("%02d:%02d:%02d");
    tf % st.wHour % st.wMinute % st.wSecond;

    return tf.str();
}

std::string strtime::toLocalDateTime(const PackedTime &utcTime) {
    return toLocalDate(utcTime)+", "+toLocalTime(utcTime);
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
    } 

    // 0123456789abcdef0
	// CCYY-MM-DDThh:mm:ss[.sss]TZD
    if (time[10]=='T') {
		if (time.length()<19) return t;
        st.wYear=extractInt(time, 0, 4);
        st.wMonth=extractInt(time, 5, 7);
        st.wDay=extractInt(time, 8, 10);
        st.wHour=extractInt(time, 11, 13);
        st.wMinute=extractInt(time, 14, 16);
        st.wSecond=extractInt(time, 17, 19);
    } //TODO CCYY-MM-DDThh:mm:ss[.sss]TZD

    SystemTimeToFileTime(&st, &t);
    return t;
}

std::string strtime::toIso8601(const PackedTime &utcTime) {
	SYSTEMTIME st;
	FileTimeToSystemTime(&utcTime, &st);

    boost::format fiso8601("%04d%02d%02dT%02d:%02d:%02d");
	fiso8601 % st.wYear % st.wMonth % st.wDay 
             % st.wHour % st.wMinute % st.wSecond;
	return fiso8601.str();
}

std::string strtime::toXep0080Time(const PackedTime &utcTime) {
	SYSTEMTIME st;
	FileTimeToSystemTime(&utcTime, &st);

    boost::format ftimeXep0080("%04d-%02d-%02dT%02d:%02d:%02dZ");
	ftimeXep0080 % st.wYear % st.wMonth % st.wDay 
                 % st.wHour % st.wMinute % st.wSecond ;

    return ftimeXep0080.str();
}


std::string strtime::getRandom() {
    return boost::str(boost::format("%d") % getCurrentUtc().dwLowDateTime);
}

std::string strtime::getLocalZoneOffset() {
    TIME_ZONE_INFORMATION tinfo;
    DWORD tzi=GetTimeZoneInformation(&tinfo);
    LONG bias=tinfo.Bias;
    switch (tzi) {
        case TIME_ZONE_ID_INVALID: bias=0; break;
        case TIME_ZONE_ID_STANDARD: bias+=tinfo.StandardBias; break;
        case TIME_ZONE_ID_DAYLIGHT: bias+=tinfo.DaylightBias; break;
    }
    if (bias==0) return std::string("Z");

    // because UTC=local+bias, we need invert bias' sign
    char sign='-';
    if (bias<0) {
        sign='+'; bias=-bias;
    }

    boost::format ltzo( "%c%02d:%02d" );
    ltzo % sign % (bias/60) % (bias%60);

    return ltzo.str();
}
