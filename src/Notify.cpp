#include "Notify.h"

#include <nled.h>
//#include <Pwinuser.h>
extern "C" { 
    BOOL WINAPI NLedGetDeviceInfo( UINT nInfoId, void *pOutput ); 
    BOOL WINAPI NLedSetDevice( UINT nDeviceId, void *pInput ); 
};

#ifdef _WIN32_WCE
DWORD vibraThread(LPVOID param) {
#else
DWORD WINAPI vibraThread(LPVOID param) {
#endif

    NLED_SETTINGS_INFO nsi;
    nsi.LedNum=1;
    nsi.OnTime=1000;
    nsi.OffTime=300;
    nsi.TotalCycleTime=1300;
    nsi.MetaCycleOn=2;
    nsi.MetaCycleOff=2;

    nsi.OffOnBlink=2;

    NLedSetDevice(NLED_SETTINGS_INFO_ID, &nsi);
    Sleep(400);

    nsi.OffOnBlink=0;
    NLedSetDevice(NLED_SETTINGS_INFO_ID, &nsi);
    Sleep(200);

    Notify::vibraOn=FALSE;

    return 1;
}

void Notify::PlayNotify() {
    if (Notify::vibraOn) return;

    Notify::vibraOn=TRUE;
    HANDLE thread=CreateThread(NULL, 0, vibraThread, NULL, 0, NULL);
    SetThreadPriority(thread, THREAD_PRIORITY_IDLE);

}

BOOL Notify::vibraOn=FALSE;