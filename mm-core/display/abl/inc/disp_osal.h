/*=============================================================================

  A D A P T I V E   B A C K L I G H T   C O N T R O L   H E A D E R   F I L E

DESCRIPTION:
  This file contains interface specifications of the adaptive backlight control
  feature for display enhancements of QUALCOMM MSM display drivers. It reduces
  display power consumption by applying only the minimum required backlight for
  any given full video frame. The change in backlight levels is coupled with
  corresponding gamma correction to compensate for changing brightness levels.

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#ifndef __DISP_OSAL_H__
#define __DISP_OSAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _ANDROID_
#include <utils/Log.h>
#define LOG_TAG "CABL"
#endif
/* -----------------------------------------------------------------------
** Platform dependent #define
** ----------------------------------------------------------------------- */
#define HW_ABL

#ifndef TRUE
#define TRUE   1                    /* Boolean true value. */
#endif

#ifndef FALSE
#define FALSE  0                    /* Boolean false value. */
#endif


/* -----------------------------------------------------------------------
** MACROS
** ----------------------------------------------------------------------- */
#ifdef _ANDROID_
#define DISP_OSAL(msg, arg1)                     {LOGE((msg), arg1);}
#define DISP_OSAL_LOG(msg)                      {LOGE((msg));}
#define DISP_OSAL_LOG1(msg, arg1)               {LOGE((msg), arg1);}
#define DISP_OSAL_LOG2(msg, arg1, arg2)         {LOGE((msg), arg1, arg2);}
#define DISP_OSAL_LOG3(msg, arg1, arg2, arg3)   {LOGE((msg), arg1, arg2, arg3);}

#else

#define DISP_OSAL(msg, arg1)                    {printf((msg), arg1);}
#define DISP_OSAL_LOG(msg)                      {printf (msg); printf("\n");}
#define DISP_OSAL_LOG1(msg, arg1)               {printf((msg), arg1 ); printf("\n");}
#define DISP_OSAL_LOG2(msg, arg1, arg2)         {printf((msg), arg1, arg2 );printf("\n");}
#define DISP_OSAL_LOG3(msg, arg1, arg2, arg3)   {printf((msg), arg1, arg2, arg3 );printf("\n");}
#endif

#endif //__DISP_OSAL_H__
