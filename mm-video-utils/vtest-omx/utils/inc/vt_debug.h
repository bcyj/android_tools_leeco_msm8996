/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VT_DEBUG_H
#define _VT_DEBUG_H

#include <stdio.h>
#include <string.h>

// undefine this to print results to the display
#define _ANDROID_LOG_

// define these to print the low/medium logs
#define _ANDROID_LOG_DEBUG
#define _ANDROID_LOG_DEBUG_LOW

#ifdef _ANDROID_LOG_
#include <utils/Log.h>
#include <utils/threads.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#undef LOG_NDEBUG
#undef LOG_TAG
#define LOG_NDEBUG 0
#define LOG_TAG "VTEST"

#define VTEST_MSG_HIGH(fmt, ...) ALOGE("VT_HIGH %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_PROFILE(fmt, ...) ALOGE("VT_PROFILE %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_ERROR(fmt, ...) ALOGE("VT_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_FATAL(fmt, ...) ALOGE("VT_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)

#ifdef _ANDROID_LOG_DEBUG
#define VTEST_MSG_MEDIUM(fmt, ...) ALOGE("VT_MED %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#else
#define VTEST_MSG_MEDIUM(fmt, ...)
#endif

#ifdef _ANDROID_LOG_DEBUG_LOW
#define VTEST_MSG_LOW(fmt, ...) ALOGE("VT_LOW %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#else
#define VTEST_MSG_LOW(fmt, ...)
#endif

#else

#define VTEST_MSG_HIGH(fmt, ...) fprintf(stderr, "VT_HIGH %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_PROFILE(fmt, ...) fprintf(stderr, "VT_PROFILE %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_ERROR(fmt, ...) fprintf(stderr, "VT_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_FATAL(fmt, ...) fprintf(stderr, "VT_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)

#ifdef  VTEST_MSG_LOG_DEBUG
#define VTEST_MSG_LOW(fmt, ...) fprintf(stderr, "VT_LOW %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VTEST_MSG_MEDIUM(fmt, ...) fprintf(stderr, "VT_MED %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#else
#define VTEST_MSG_LOW(fmt, ...)
#define VTEST_MSG_MEDIUM(fmt, ...)
#endif

#endif //#ifdef _ANDROID_LOG_

#endif // #ifndef _VT_DEBUG_H
