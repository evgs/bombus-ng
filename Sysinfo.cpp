#include "Sysinfo.h"

#include <windows.h>
#include <sstream>

const std::wstring sysinfo::getOsVersion() {

    std::wstringstream os;
    wchar_t buf[64];

    //SystemParametersInfo(SPI_GETPLATFORMTYPE, 128, buf, 0);  
    //os << buf << " ";

    OSVERSIONINFO osv;
    GetVersionEx(&osv);

    os << "WinCE " << osv.dwMajorVersion << "." << osv.dwMinorVersion << "." << osv.dwBuildNumber;

    SystemParametersInfo(SPI_GETOEMINFO, 64, buf, 0);  
    os << "/" << buf;

    return os.str();

}

bool sysinfo::screenIsVGA() {
    RECT ws;
    SystemParametersInfo(SPI_GETWORKAREA, sizeof(ws), &ws, 0);

    return (ws.right - ws.left > 240);
}
