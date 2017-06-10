/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "WFDMediaEntry"
#include "common_log.h"
#include <utils/Log.h>

#include "OMX_Index.h"
#include "OMX_Other.h"
#include "DashPlayer.h"
#include "WFDSource.h"


namespace android {

extern "C" DashPlayer::WFDSource* CreateWFDSource(const char *uri,
                          const KeyedVector<String8, String8> *headers, bool uidValid, uid_t uid)
{
  OMX_U32 nReturn = UNKNOWN_ERROR;
  DashPlayer::WFDSource *pSource = NULL;
  sp<DashPlayer::WFDSource> spSource = NULL;
  LOGE("Vivek:: Inside CreateWFDSource --- ");
  pSource = new DashPlayer::WFDSource(uri, headers, nReturn, uidValid, uid);

  return pSource;
}


}  // namespace android

