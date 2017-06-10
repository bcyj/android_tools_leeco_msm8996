/*****************************************************************************
* Copyright (c) 2009-2011,2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/23/09   zhiminl Replaced #ifdef _ANDROID_ with #ifdef ANDROID.
06/02/09   vma     Created file.

========================================================================== */

#ifndef _JPEGLOG_H
#define _JPEGLOG_H

#include <stdio.h>

#define LOG_DEBUG

#define JPEG_DBG_ERROR_ENABLE 1
#define JPEG_DBG_HIGH_ENABLE  0
#define JPEG_DBG_MED_ENABLE   0
#define JPEG_DBG_LOW_ENABLE   0

#undef JDBG
#ifdef LOG_DEBUG
    #ifdef ANDROID
        #define LOG_NIDEBUG 0
        #define LOG_TAG "mm-still"
        #include <utils/Log.h>
	#ifdef NEW_LOG_API
            #define JDBG(fmt, args...) ALOGI(fmt, ##args)
	#else
            #define JDBG(fmt, args...) LOGI(fmt, ##args)
	#endif
    #else
        #define JDBG(fmt, args...) fprintf(stderr, fmt, ##args)
    #endif
#else
    #define JDBG(...) do{}while(0)
#endif

#if(JPEG_DBG_ERROR_ENABLE)
  #define JPEG_DBG_ERROR(...)   JDBG(__VA_ARGS__)
#else
  #define JPEG_DBG_ERROR(...)   do{}while(0)
#endif

#if(JPEG_DBG_HIGH_ENABLE)
  #define JPEG_DBG_HIGH(...)   JDBG(__VA_ARGS__)
#else
  #define JPEG_DBG_HIGH(...)   do{}while(0)
#endif

#if(JPEG_DBG_MED_ENABLE)
  #define JPEG_DBG_MED(...)   JDBG(__VA_ARGS__)
#else
  #define JPEG_DBG_MED(...)   do{}while(0)
#endif

#if(JPEG_DBG_LOW_ENABLE)
  #define JPEG_DBG_LOW(...)   JDBG(__VA_ARGS__)
#else
  #define JPEG_DBG_LOW(...)   do{}while(0)
#endif

#endif /* _JPEGLOG_H */
