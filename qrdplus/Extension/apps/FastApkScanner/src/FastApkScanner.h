/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _FAST_APK_SCANNER_H
#define _FAST_APK_SCANNER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef void* PFilterObject;

PFilterObject RegistFilterObject(int fd);

void UnRegistFilterObject(int fd);

PFilterObject GetFilterObject(int fd);

typedef int (*PNameFilter)(char* name, int length, void* param);

int FilterLibrary(PFilterObject obj, PNameFilter filter, void* param);

int HasRenderScript(PFilterObject obj);

#ifdef __cplusplus
}
#endif

#endif //_FAST_APK_SCANNER_H
