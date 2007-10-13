#include "Notify.h"

#include "config.h"

#include <nled.h>

//#include <Pwinuser.h>
extern "C" { 
    BOOL WINAPI NLedGetDeviceInfo( UINT nInfoId, void *pOutput ); 
    BOOL WINAPI NLedSetDevice( UINT nDeviceId, void *pInput ); 
};

void doSmartPhoneVibra();


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

    nsi.OffOnBlink=1;

    NLedSetDevice(NLED_SETTINGS_INFO_ID, &nsi);
    Sleep(400);

    nsi.OffOnBlink=0;
    NLedSetDevice(NLED_SETTINGS_INFO_ID, &nsi);
    Sleep(200);

    Notify::vibraOn=FALSE;

    return 1;
}

void Notify::PlayNotify() {
    //doSmartPhoneVibra();
    
    if (Notify::vibraOn) return;

    if (!(Config::getInstance()->vibra)) return;

    Notify::vibraOn=TRUE;
    HANDLE thread=CreateThread(NULL, 0, vibraThread, NULL, 0, NULL);
    SetThreadPriority(thread, THREAD_PRIORITY_IDLE);
}

BOOL Notify::vibraOn=FALSE;


typedef struct
{
    WORD wDuration;
    BYTE bAmplitude;  
    BYTE bFrequency;
} VIBRATENOTE; 

void doSmartPhoneVibra() {
    HINSTANCE hInst = LoadLibrary(_T("aygshell.dll"));
    if (hInst) {
        HRESULT (*Vibrate)(DWORD cvn, const VIBRATENOTE * rgvn, BOOL fRepeat, DWORD dwTimeout);
        (FARPROC&)Vibrate = GetProcAddress(hInst, _T("Vibrate"));
        if (Vibrate) {
            HRESULT retval=Vibrate(0, NULL, true, 2000);
            retval++;
        }
        FreeLibrary(hInst);
    }
}