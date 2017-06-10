/*******************************************************************************
*   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
*   Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef _OMXLOG_H
#define _OMXGLOG_H

#define OMX_DBG_ERROR_ENABLE 1
#define OMX_DBG_WARNG_ENABLE 0
#define OMX_DBG_HIGH_ENABLE 1
#define OMX_DBG_INFO_ENABLE  0

#ifdef ANDROID
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-still-omx"
  #include <utils/Log.h>
  #ifdef NEW_LOG_API
    #define OMXDBG(fmt, args...) ALOGI(fmt, ##args)
  #else
    #define OMXDBG(fmt, args...) LOGI(fmt, ##args)
  #endif
#endif

#if(OMX_DBG_ERROR_ENABLE)
  #define OMX_DBG_ERROR(...)  OMXDBG(__VA_ARGS__)
#else
  #define OMX_DBG_ERROR(...)  do{}while(0)
#endif

#if(OMX_DBG_WARNG_ENABLE)
  #define OMX_DBG_WARNG(...)  OMXDBG(__VA_ARGS__)
#else
  #define OMX_DBG_WARNG(...)  do{}while(0)
#endif


#if(OMX_DBG_INFO_ENABLE)
  #define OMX_DBG_INFO(...)   OMXDBG(__VA_ARGS__)
#else
  #define OMX_DBG_INFO(...)  do{}while(0)
#endif

#if(OMX_DBG_HIGH_ENABLE)
  #define OMX_DBG_HIGH(...)   OMXDBG(__VA_ARGS__)
#else
  #define OMX_DBG_HIGH(...)  do{}while(0)
#endif


#endif /* _OMXGLOG_H */
