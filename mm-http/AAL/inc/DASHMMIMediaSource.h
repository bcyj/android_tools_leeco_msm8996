/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
/* =======================================================================
**               Include files for DASHMMIMediaSource.h
** ======================================================================= */

#ifndef DASH_MMI_MEDIA_SOURCE_H
#define DASH_MMI_MEDIA_SOURCE_H

#include "AEEStdDef.h"
#include "DASHMMIMediaInfo.h"
#include <media/stagefright/foundation/ABuffer.h>
#include <utils/RefBase.h>

#include "OMX_Core.h"
#include "OMX_Types.h"


namespace android
{
/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */
#define IS_VIDEO_PORT(x) ((x)==MMI_HTTP_VIDEO_PORT_INDEX)
#define IS_AUDIO_PORT(x) ((x)==MMI_HTTP_AUDIO_PORT_INDEX)
#define IS_TEXT_PORT(x)  ((x)==MMI_HTTP_OTHER_PORT_INDEX)

/* -----------------------------------------------------------------------
** Type Declarations, forward declarations
** ----------------------------------------------------------------------- */
//forward declarations
class DASHMMIMediaInfo;
/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */
class DASHMMIMediaSource : public RefBase {
public:
  /** @brief   Constructor of Http Media Source.
   *  @return
   */
  DASHMMIMediaSource(const sp<MetaData> &metaData,
                     uint32_t trackId,
                     const sp<DASHMMIMediaInfo> &extractor);

  /** @brief   Http Media Source start, creates mediabuffer
   *  @param[in] params
   *  @return status of operation
   */
  status_t start();


  /** @brief   gets meta data
   *  @param[in] none
   *  @return MetaData smart pointer
   */
  sp<MetaData> getFormat();

  status_t readFrameAsync(sp<ABuffer> mAbuffer, OMX_BUFFERHEADERTYPE &bufHdr);

  OMX_U32 getMaxBufferSize();

  uint32 nPortIndex;

protected:

  /** @brief   Desstructor of Http Media Source.
   *  @return
   */
   ~DASHMMIMediaSource();


private:
    sp<MetaData> m_metaData;

    sp<DASHMMIMediaInfo> m_mmDASHMMIMediaInfo;

    uint32_t m_nTrackId;

    bool m_bPauseRecved;

    bool mMediaStarted;
    uint32 nMaxBufferSize;
};

}

#endif  // DASH_MMI_MEDIA_SOURCE_H

