/* =======================================================================
                              WFDMMSourceDebug.h
DESCRIPTION

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header://depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/WFDMMSourceDebug.h

========================================================================== */

#ifndef _VENC_TEST_DEBUG_H
#define _VENC_TEST_DEBUG_H

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include <stdio.h>
#include <string.h>
#ifndef WFD_ICS
#include "common_log.h"
#endif
#define LOG_NDEBUG 0

#ifdef _ANDROID_LOG_
//#ifndef LOG_TAG
//#define LOG_TAG "VENC_TEST"
#include <utils/Log.h>
//#endif

#define VENC_TEST_MSG_HIGH(fmt, ...) LOGE("VENC_HIGH %s::%d "fmt"\n",__FUNCTION__, __LINE__, ## __VA_ARGS__)
#define VENC_TEST_MSG_PROFILE(fmt, ...) LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_ERROR(fmt, ...) LOGE("VENC_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__, ## __VA_ARGS__)
#define VENC_TEST_MSG_FATAL(fmt, ...)LOGE("VENC_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)

// #ifdef _ANDROID_LOG_DEBUG
#if 0
#define VENC_TEST_MSG_MEDIUM(fmt, ...) LOGE("(TIME:%lld) %s::%d "fmt"\n",(unsigned long long)(Time::GetTimeMicrosec()/1000), __FUNCTION__, __LINE__,  ## __VA_ARGS__)
//#define VENC_TEST_MSG_MEDIUM(fmt, ...) LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__, ## __VA_ARGS__)
//#define VENC_TEST_MSG_LOW(fmt, ...) LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
//#define VENC_TEST_MSG_LOW(fmt, ...) LOGE("(TIME:%lld) %s::%d "fmt"\n",(unsigned long long)(Time::GetTimeMicrosec()/1000), __FUNCTION__, __LINE__,  ## __VA_ARGS__)
//#define VENC_TEST_MSG_VPE(fmt, ...) LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__, ## __VA_ARGS__)
#define VENC_TEST_MSG_VPE(fmt, ...)
#define VENC_TEST_MSG_LOW(fmt, ...)
#else
#define VENC_TEST_MSG_VPE(fmt, ...) //LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_LOW(fmt, ...) //LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_MEDIUM(fmt, ...) //LOGE("%s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#endif

#else

#define VENC_TEST_REMOVE_SLASH(x) strrchr(x, '/') != NULL ? strrchr(x, '/') + 1 : x

#define VENC_TEST_MSG_HIGH(fmt, ...) fprintf(stderr, "VENC_TEST_HIGH %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_PROFILE(fmt, ...) fprintf(stderr, "VENC_TEST_PROFILE %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_ERROR(fmt, ...) fprintf(stderr, "VENC_TEST_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_FATAL(fmt, ...) fprintf(stderr, "VENC_TEST_ERROR %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)

#ifdef  VENC_MSG_LOG_DEBUG
#define VENC_TEST_MSG_LOW(fmt, ...) fprintf(stderr, "VENC_TEST_LOW %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#define VENC_TEST_MSG_MEDIUM(fmt, ...) fprintf(stderr, "VENC_TEST_MEDIUM %s::%d "fmt"\n",__FUNCTION__, __LINE__,## __VA_ARGS__)
#else
#define VENC_TEST_MSG_LOW(fmt, ...)
#define VENC_TEST_MSG_MEDIUM(fmt, ...)
#endif

#endif //#ifdef _ANDROID_LOG_

#endif // #ifndef _VENC_TEST_DEBUG_H
