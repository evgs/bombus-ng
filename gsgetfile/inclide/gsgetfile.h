/*
gsgetfile.h
export function definitions

Copyright(C) 2002 Y.Nagamidori
*/

#ifndef __GSGETFILE_H__
#define __GSGETFILE_H__

#ifdef GSGETFILE_EXPORTS
#define GSGETFILE_EXPORT __declspec(dllexport)
#else
#define GSGETFILE_EXPORT
#endif

#include <windows.h>
#include <tchar.h>
#include <commdlg.h>

#ifdef __cplusplus
extern "C" {
#endif

GSGETFILE_EXPORT BOOL WINAPI gsGetOpenFileName(LPOPENFILENAME pofn);
GSGETFILE_EXPORT BOOL WINAPI gsGetSaveFileName(LPOPENFILENAME pofn);

#ifdef __cplusplus
};
#endif

#endif // __GSGETFILE_H__