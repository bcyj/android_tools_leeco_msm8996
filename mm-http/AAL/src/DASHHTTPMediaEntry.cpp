/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "DASHHTTPMediaEntry"
#include "common_log.h"
#include <utils/Log.h>

#include "OMX_Index.h"
#include "OMX_Other.h"
#include "DashPlayer.h"
#include "DASHHTTPLiveSource.h"


namespace android {

extern "C" DashPlayer::DASHHTTPLiveSource* CreateDashHttpLiveSource(const char *uri,
                          const KeyedVector<String8, String8> *headers, bool uidValid, uid_t uid)
{
  OMX_U32 nReturn = MMI_S_EFAIL;
  DashPlayer::DASHHTTPLiveSource *cache = NULL;
  sp<DashPlayer::DASHHTTPLiveSource> tCache = NULL;
  cache = new DashPlayer::DASHHTTPLiveSource(uri, headers, nReturn, uidValid, uid);


  if (!IS_SUCCESS(nReturn))
  {
    if (cache)
    {
      //smart pointer deletes the object we just return NULL
      tCache = cache;
      cache = NULL;
    }
  }

  return cache;
}


}  // namespace android

