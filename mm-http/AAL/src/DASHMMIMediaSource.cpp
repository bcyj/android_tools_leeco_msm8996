/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
/* =======================================================================
**               Include files for DASHMMIMediaSource.cpp
** ======================================================================= */

#define LOG_NDEBUG 0
#define LOG_TAG "DASHMMIMediaSource"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MetaData.h>

#include "common_log.h"

#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMCriticalSection.h"
#include "mmiDeviceApi.h"
#include "OMX_Types.h"
#include "qtv_msg.h"
#include "DASHMMIMediaSource.h"
#include "DASHMMIInterface.h"
#include "DASHMMIMediaInfo.h"
#include "HTTPSourceMMIEntry.h"

namespace android {

const char *HTTP_DASH_MEDIA_MIMETYPE_AUDIO_AAC = "audio/mp4a-latm";

/** @brief   Constructor of Http Media Source.
 *  @return
 */
DASHMMIMediaSource::DASHMMIMediaSource(const sp<MetaData> &metaData,
                                       uint32_t trackId,
                                       const sp<DASHMMIMediaInfo> &extractor)
                                       :m_metaData(metaData),
                                        m_mmDASHMMIMediaInfo(extractor),
                                        m_nTrackId(trackId)
{
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "DASHMMIMediaSource::DASHMMIMediaSource");

  m_bPauseRecved = false;
  mMediaStarted = false;
  nMaxBufferSize = 0;
  nPortIndex = 0;
}

/** @brief   Desstructor of Http Media Source.
 *  @return
 */
DASHMMIMediaSource::~DASHMMIMediaSource()
{
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "DASHMMIMediaSource::~DASHMMIMediaSource called");
    mMediaStarted = false;
    nPortIndex = 0;
}

/** @brief   Http Media Source start, creates mediabuffer
 *  @param[in] params
 *  @return status of operation
 */
status_t DASHMMIMediaSource::start()
{
  const char *mime = NULL;
  uint32 nPort = 0;
  status_t err = (status_t)UNKNOWN_ERROR;
  OMX_U32 ret = MMI_S_EFAIL;
  DASHMMIInterface *pMMIHandle = NULL;

  MMI_CustomParamCmdType data;
  MMI_ParamBuffersReqType paramStruct;

  if (mMediaStarted == true)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "DASHMMIMediaSource::start already started");
    //already started
    return OK;
  }

  if (m_mmDASHMMIMediaInfo != NULL)
  {
    pMMIHandle = m_mmDASHMMIMediaInfo->GetDASHInterface();
  }

  if (NULL == pMMIHandle)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaSource::start  DASHMMI Interface is NULL!");
    return err;
  }

  OMX_HANDLETYPE handle = pMMIHandle->GetMMIHandle();

  if (handle == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaSource::start  GetMMIHandle is NULL!");
    return err;
  }

  if (m_metaData != NULL)
  {
    m_metaData->findCString(kKeyMIMEType, &mime);
  }

  if (mime && !std_strnicmp(mime, "video/", 6))
  {
    nPort = MMI_HTTP_VIDEO_PORT_INDEX;
    pMMIHandle->m_bVidEos = false;
  }
  else if (mime && !std_strnicmp(mime, "audio/", 6))
  {
    nPort = MMI_HTTP_AUDIO_PORT_INDEX;
    pMMIHandle->m_bAudEos = false;
  }
  else if (mime && !std_strnicmp(mime, "text/", 5))
  {
    nPort = MMI_HTTP_OTHER_PORT_INDEX;
    pMMIHandle->m_bTextEos = false;
  }
  nPortIndex = nPort;

  if (nPort)
  {
    // Get max buffer size
    paramStruct.nPortIndex = nPort;
    data.nParamIndex = MMI_IndexBuffersReq;
    data.pParamStruct = &paramStruct;

    ret = pMMIHandle->postCmd(handle, MMI_CMD_GET_CUSTOM_PARAM, &data);
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "DASHMMIMediaSource::start port found");
  }
  else
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "DASHMMIMediaSource::start failed, port not found");
  }

  if (IS_SUCCESS(ret))
  {
    nMaxBufferSize =  paramStruct.nDataSize;
    mMediaStarted = true;
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "Port [%d] Allocated Memory %u", (int)nPort, nMaxBufferSize);
  }
  else
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "DASHMMIMediaSource::start failed %d", (int)ret);
    mMediaStarted = false;
  }

  return err;
}

OMX_U32 DASHMMIMediaSource::getMaxBufferSize()
{
   return (OMX_U32)nMaxBufferSize;
}

/** @brief   gets meta data
 *  @param[in] none
 *  @return MetaData smart pointer
 */
sp<MetaData> DASHMMIMediaSource::getFormat()
{
  return m_metaData;
}

status_t DASHMMIMediaSource::readFrameAsync(sp<ABuffer> mAbuffer, OMX_BUFFERHEADERTYPE &bufHdr)
{

  OMX_U32 nReturn = MMI_S_EFAIL;
  status_t ret = OK;

  DASHMMIInterface *pMMIHandle = NULL;
  DASHMMIMediaInfo::TrackTable::TrackInfo *tInfo = NULL;
  OMX_U32 status = MMI_S_COMPLETE;

  if (m_mmDASHMMIMediaInfo != NULL)
  {
    pMMIHandle = m_mmDASHMMIMediaInfo->GetDASHInterface();
  }

  if (NULL == pMMIHandle || mAbuffer == NULL )
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaSource::read  MMI Handle or ABuffer NULL!");
    return (status_t)UNKNOWN_ERROR;
  }


  OMX_HANDLETYPE handle = pMMIHandle->GetMMIHandle();

  if (NULL == handle)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaSource::GetMMIHandle is NULL!");
    return UNKNOWN_ERROR;
  }

  if (m_mmDASHMMIMediaInfo != NULL)
  {
    tInfo = m_mmDASHMMIMediaInfo->m_trackTable.FetchTrackInfo(m_nTrackId,nPortIndex);
  }

  if (NULL == tInfo)
  {
    return ERROR_IO;
  }

  QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "DASHMMIMediaSource::readFrameAsync  port %d track %d buffer[%p]",
                (int)tInfo->m_nPort,(int)m_nTrackId, mAbuffer->data());


  mAbuffer->setRange(0,nMaxBufferSize);

  QOMX_STRUCT_INIT(bufHdr, OMX_BUFFERHEADERTYPE);
  MMI_BufferCmdType dataCmd;
  dataCmd.nPortIndex = tInfo->m_nPort;
  bufHdr.nOutputPortIndex = tInfo->m_nPort;
  bufHdr.nAllocLen = (OMX_U32) mAbuffer->size();
  bufHdr.pBuffer = (OMX_U8 *) mAbuffer->data();
  dataCmd.pBufferHdr = &bufHdr;


  nReturn = pMMIHandle->postCmd(handle, MMI_CMD_FILL_THIS_BUFFER, &dataCmd);

  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "DASHMMIMediaSource::readFrameAsync - about to post FTB to MMI Device %d with Size %lu",
                   (int)nReturn, bufHdr.nAllocLen);


  ret = pMMIHandle->MapMMIToAALStatus(nReturn);

  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "DASHMMIMediaSource::readFrameAsync - FILL THIS BUFFER Posted to MMI Device %d with Size %lu",
                    ret, bufHdr.nAllocLen);

  if (nReturn == MMI_S_EFAIL)
  {
      ret = ERROR_IO;
  }
  else if (ret == WOULD_BLOCK)
  {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "DASHMMIMediaSource::readFrameAsync  pending - return OK..");
      ret = OK;
  }
  return ret;
}

}  // namespace android



