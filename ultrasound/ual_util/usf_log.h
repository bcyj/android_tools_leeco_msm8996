#ifndef USF_LOG_H
#define USF_LOG_H
/*==========================================================================
                           usf_log.h

DESCRIPTION: Definition of USF common log functions

Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#include <utils/Log.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef LOGE
#define LOGE ALOGE
#endif

#ifndef LOGW
#define LOGW ALOGW
#endif

#ifndef LOGD
#define LOGD ALOGD
#endif

#ifndef LOGV
#define LOGV ALOGV
#endif

#ifndef LOGI
#define LOGI ALOGI
#endif

#ifdef __cplusplus
}
#endif

#endif  /* COMMON_LOG_H */
