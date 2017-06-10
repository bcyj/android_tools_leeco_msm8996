/******************************************************************************
  @file:  xtra.h
  @brief: XTRA daemon main include

  DESCRIPTION

  XTRA Daemon

  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

#ifndef XTRA_H_
#define XTRA_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <xtra_linux.h>


#include <xtra_defines.h>
#include <xtra_config_api.h>
#include <xtra_log_api.h>
#include <xtra_server_api.h>

extern globals_t globals;

int Xtra_Main(void);
void Xtra_EndTask(void);
const char* Xtra_GetVersionString(void);


void*  Xtra_HttpClientTask(void* p);
void*  Xtra_SntpClientTask(void* p);
void*  Xtra_ControlTask(void* p);

#endif


#ifdef __cplusplus
}
#endif

