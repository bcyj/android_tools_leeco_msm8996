/************************************************************************* */
/**
 * DEALUtils.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

#ifndef DEAL_UTILS_H_

#define DEAL_UTILS_H_

/* =======================================================================
**               Include files for DEALUtils.h
** ======================================================================= */

#include "DashExtractorAdaptationLayer.h"

namespace android {

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */

class  DEALUtils {

public:
static status_t MapMMIToDEALStatus(uint32 nValue);
static void AddOemHeaders(const KeyedVector<String8, String8> *headers, DEALInterface& dealRef);
static void QueryStreamType(OMX_U32 nPort, DEALInterface& dealRef);
static const char* TextEncodingToMIME(OMX_OTHER_FORMATTYPE coding);
static const char* VideoEncodingToMIME(OMX_VIDEO_CODINGTYPE coding);
static const char* AudioEncodingToMIME(OMX_AUDIO_CODINGTYPE coding);
static void AddAACCodecSpecificData(sp<MetaData> &metaData, DEALInterface& dealRef);
static int64 GetDuration(OMX_U32 nPort, DEALInterface& dealRef);
static void QueryAndUpdateMetaData(uint32_t mPortIndex, sp<MetaData> *metaData, bool bIsEnc, DEALInterface& dealRef);
static status_t UpdateBufReq(DEALInterface::TrackInfo &info, uint32_t index, DEALInterface& dealRef);
static DEALInterface::DASHSourceTrackType MapPortIndexToTrackId(uint32 nPortIndex);
static ECALInterface::DASHSourceTrackType MapPortToTrackId(uint32 nPortIndex);


};

}  // namespace android

#endif  // DEAL_UTILS_H_
