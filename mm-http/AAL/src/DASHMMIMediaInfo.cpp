/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
/* =======================================================================
**               Include files for DASHMMIMediaInfo.cpp
** ======================================================================= */

#define LOG_NDEBUG 0
#define LOG_TAG "DASHMMIMediaInfo"

#include "common_log.h"

#include "DASHMMIMediaInfo.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "mmiDeviceApi.h"
#include "OMX_Types.h"
#include "qtv_msg.h"
#include "QCMetaData.h"
#include <QCMediaDefs.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/foundation/ABuffer.h>


#include "DASHMMIInterface.h"
#include "HTTPSourceMMIEntry.h"
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_VideoExtensions.h"
#include "QOMX_StreamingExtensionsPrivate.h"

namespace android {

#define APPROX_CONFIG_HDR_SIZE 128

/** @brief   Constructor of Http Media Extractor.
 *  @return
 */
DASHMMIMediaInfo::DASHMMIMediaInfo(const sp<DASHMMIInterface> &source, OMX_U32 &nReturn)
{
  nReturn = MMI_S_COMPLETE;
  m_mmDASHMMIInterface = (DASHMMIInterface *) source.get();
  bHasAudio = false;
  bHasVideo = false;
  bHasText = false;

  nVideoTrackID = -1;
  nAudioTrackID = -1;
  nTextTrackID  = -1;
}

/** @brief   Destructor of Http Media Extractor.
 *  @return
 */
DASHMMIMediaInfo::~DASHMMIMediaInfo()
{
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "~DASHMMIMediaInfo");
}

DASHMMIInterface *DASHMMIMediaInfo::GetDASHInterface()
{

  return m_mmDASHMMIInterface.get();
}
sp<MetaData> DASHMMIMediaInfo::getVideoMetadata(int &nTrackID)
{
    sp<MetaData> meta = NULL;
    uint32_t assignedTrackID;

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Entering DASHMMIMediaInfo::getVideoMetadata %d",nVideoTrackID);

    if (nVideoTrackID != -1)
    {
      // this is to make sure, when there is a codec change we should reset everything and
      // query dash and get the correct information from DASH
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                            "DASHMMIMediaInfo::getVideoMetadata reset video track metadata");

      m_trackTable.ResetTrackInfo(nVideoTrackID);
      nVideoTrackID = -1;
    }

    for (size_t i = 0; i < countTracks(); ++i)
    {
        meta = getTrackMetaData(i, MMI_HTTP_VIDEO_PORT_INDEX,assignedTrackID);
        if (meta != NULL)
        {
          const char *mime = NULL;
          nTrackID = 0;
          meta->findCString(kKeyMIMEType, &mime);

          if (mime && !strncasecmp(mime, "video/", 6))
          {
             nTrackID = assignedTrackID;
             nVideoTrackID = nTrackID;
             bHasVideo = true;
             //video format found
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DASHMMIMediaInfo::getVideoMetadata -> Video format found");
             return meta;
          }
          else
          {
             meta = NULL;
          }
        }
    }

    return meta;
}


sp<MetaData> DASHMMIMediaInfo::getAudioMetadata(int &nTrackID)
{
    sp<MetaData> meta = NULL;
    uint32_t assignedTrackID;

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Entering DASHMMIMediaInfo::getAudioMetadata %d ",nAudioTrackID);

    //
    if (nAudioTrackID != -1)
    {
       QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                         "DASHMMIMediaInfo::getAudioMetadata reset audio track metadata");

       m_trackTable.ResetTrackInfo(nAudioTrackID);
       nAudioTrackID = -1;
    }

    for (size_t i = 0; i < countTracks(); ++i)
    {
        meta = getTrackMetaData(i, MMI_HTTP_AUDIO_PORT_INDEX,assignedTrackID);
        if (meta != NULL)
        {
          const char *mime = NULL;
          nTrackID = 0;
          meta->findCString(kKeyMIMEType, &mime);

          if (mime && !strncasecmp(mime, "audio/", 6))
          {
             nTrackID = assignedTrackID;
             nAudioTrackID = nTrackID;
             bHasAudio = true;
             //audio format found
             QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                          "DASHMMIMediaInfo::getAudioMetadata -> Audio format found");
            return meta;
          }
          else
          {
            meta = NULL;
          }
        }
    }

    return meta;
}

sp<MetaData> DASHMMIMediaInfo::getTextMetadata(int &nTrackID)
{
    sp<MetaData> meta = NULL;
    uint32_t assignedTrackID;

    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                          "Entering DASHMMIMediaInfo::getTextMetadata %d ",nTextTrackID);

    if (nTextTrackID != -1)
    {
      // this is to make sure, when there is a codec change we should reset everything and
      // query dash and get the correct information from DASH
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                            "DASHMMIMediaInfo::getTextFormat reset text track metadata");

      m_trackTable.ResetTrackInfo(nTextTrackID);
      nTextTrackID = -1;
    }

    for (size_t i = 0; i < countTracks(); ++i)
    {
        meta = getTrackMetaData(i, MMI_HTTP_OTHER_PORT_INDEX,assignedTrackID);
        if (meta != NULL)
        {
          const char *mime = NULL;
          nTrackID = 0;
          meta->findCString(kKeyMIMEType, &mime);

          if (mime && !strncasecmp(mime, "text/", 5))
          {
             nTrackID = assignedTrackID;
             nTextTrackID = nTrackID;
             bHasText = true;
             //text format found
              QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DASHMMIMediaInfo::getTextFormat -> Text format found");
             return meta;
          }
          else
          {
             meta = NULL;
          }
        }
    }
    return meta;
}

/** @brief   Count number of tracks in session.
 *  @param[in]
 *  @return numTracks
 */
size_t DASHMMIMediaInfo::countTracks()
{
    if (m_mmDASHMMIInterface == NULL)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "CMMHTTPMediaExtractor::CountTracks DataCache NULL!!");
      return 0;
    }

    return m_mmDASHMMIInterface->GetNumTracks();

/*
  MMI_OmxParamCmdType param;
  param.nParamIndex = OMX_IndexParamNumAvailableStreams;
  OMX_U32 ret = MMI_S_EFAIL;
  OMX_HANDLETYPE handle = NULL;

  if (m_mmDASHMMIInterface == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaInfo::CountTracks DataCache NULL!!");
    return 0;
  }

  handle = m_mmDASHMMIInterface->GetMMIHandle();

  //get audio num tracks
  OMX_PARAM_U32TYPE audioU32;
  QOMX_STRUCT_INIT(audioU32, OMX_PARAM_U32TYPE);
  audioU32.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
  param.pParamStruct = &audioU32;

  ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&param);


  //get video num tracks
  OMX_PARAM_U32TYPE videoU32;
  QOMX_STRUCT_INIT(videoU32, OMX_PARAM_U32TYPE);
  videoU32.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
  param.pParamStruct = &videoU32;

  ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&param);

  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "DASHMMIMediaInfo::CountTracks %d status %d",
                (audioU32.nU32 + videoU32.nU32), ret);

  return (audioU32.nU32 + videoU32.nU32);
  */
}

/** @brief   Gets meta data from http mmi.
 *  @param[in] index track index
 *             flags
 *  @return MetaData smart pointer
 */
sp<MetaData> DASHMMIMediaInfo::getTrackMetaData(size_t index, uint32_t flags, uint32_t &assignedTrackID)
{
  sp<MetaData> metaData = NULL;

  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_LOW,
                "DASHMMIMediaInfo::getTrackMetaData %d", index);

  TrackTable::TrackInfo *track = m_trackTable.FetchTrackInfo(index,flags);
  if (track)
  {
    metaData = GetMetaData(track,index,flags);
    if ( (metaData == NULL) && (track->m_nTrackId != -1))
    {
       m_trackTable.ResetTrackInfo(track->m_nTrackId);
    }
    else
    {
      assignedTrackID = track->m_nTrackId;
    }
  }
  return metaData;
}

status_t DASHMMIMediaInfo::getDuration(int64_t *durationUs)
{

    int64 audDur = 0, vidDur = 0;

    if (m_mmDASHMMIInterface == NULL)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "DASHMMIMediaInfo::getDuration MMI Interface is NULL!!");
          *durationUs = 0; // !Warning : not sure what should be the value here
      return BAD_VALUE;
    }

    audDur = GetDuration(MMI_HTTP_VIDEO_PORT_INDEX);
    vidDur = GetDuration(MMI_HTTP_AUDIO_PORT_INDEX);

    *durationUs = 0;

    if (audDur >= vidDur)
    {
        *durationUs = (int64_t)audDur;
    }
    else
    {
        *durationUs = (int64_t)vidDur;
    }

    return OK;

}



/** @brief  helps to get meta data from http mmi
 *  @param[in] index track index
 *  @return MetaData smart pointer
 */
sp<MetaData> DASHMMIMediaInfo::GetMetaData(TrackTable::TrackInfo *track, size_t index,uint32_t mPortIndex)
{
  MMI_CustomParamCmdType data;
  MMI_OmxParamCmdType stdData;
  MMI_ParamDomainDefType paramStruct;
  OMX_AUDIO_PARAM_PORTFORMATTYPE paramAudioStruct;
  OMX_OTHER_PARAM_PORTFORMATTYPE paramOtherStruct;
  OMX_U32 ret = MMI_S_EFAIL;
  const char *mime = NULL;
  OMX_HANDLETYPE handle = NULL;
  //TrackTable::TrackInfo *track = NULL;
  int64 duration = 0;

  QOMX_STRUCT_INIT(paramAudioStruct, OMX_AUDIO_PARAM_PORTFORMATTYPE);
  QOMX_STRUCT_INIT(paramOtherStruct, OMX_OTHER_PARAM_PORTFORMATTYPE);

  if (m_mmDASHMMIInterface == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaInfo::dataCache NULL!!");
    return NULL;
  }

  HTTPAALAttributeKey nKey = HTTP_AAL_ATTR_METADATA_AVAILABLE;
  HTTPAALAttributeValue nVal;
  nVal.bBoolVal = false;
  if (m_mmDASHMMIInterface->GetAttribute(nKey, nVal) && nVal.bBoolVal)
  {
  handle = m_mmDASHMMIInterface->GetMMIHandle();

  if (track == NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DASHMMIMediaInfo:: track NULL!!");
    return NULL;
  }

  if (track->m_metaData != NULL)
  {
    QTV_MSG_PRIO1( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "DASHMMIMediaInfo::GetMetaData already exists for index(%d)",index);
    return track->m_metaData;
    //track->m_metaData = NULL;    //!Warning : To support Codec change, we need to get the new codec parameters from DASH, hence we need to delete the old track detail and
    //track->m_nPort = 0;
  }
  else
  {
    track->m_metaData = MM_New(MetaData);
  }

  if (!IS_VALID_PORT(track->m_nPort) &&
     !m_trackTable.DoesExist(MMI_HTTP_VIDEO_PORT_INDEX) && (mPortIndex == MMI_HTTP_VIDEO_PORT_INDEX ))
  {
    //video port def
    paramStruct.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
    data.nParamIndex = MMI_IndexDomainDef;
    data.pParamStruct = &paramStruct;

    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_CUSTOM_PARAM,&data);
    if (ret == MMI_S_COMPLETE)
    {
      OMX_VIDEO_CODINGTYPE code= OMX_VIDEO_CodingUnused;
      if ((OMX_VIDEO_CodingUnused != paramStruct.format.video.eCompressionFormat) &&
          (OMX_VIDEO_CodingAutoDetect != paramStruct.format.video.eCompressionFormat))
      {
        track->m_nPort = paramStruct.nPortIndex;
      }
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "DASHMMIMediaInfo::GetMetaData vid format(%d) %d port %d",code,
                paramStruct.format.video.eCompressionFormat,
                track->m_nPort);
      if (mPortIndex != (uint32_t)track->m_nPort)
      {
         track->m_metaData = NULL;
         return NULL;
      }
    }
  }

  if (!IS_VALID_PORT(track->m_nPort) &&
      !m_trackTable.DoesExist(MMI_HTTP_AUDIO_PORT_INDEX) && (mPortIndex == MMI_HTTP_AUDIO_PORT_INDEX ))
  {
    OMX_AUDIO_CODINGTYPE code= OMX_AUDIO_CodingUnused;
    //audio port def
    QOMX_STRUCT_INIT(paramAudioStruct, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    paramAudioStruct.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
    stdData.nParamIndex = OMX_IndexParamAudioPortFormat;
    stdData.pParamStruct = &paramAudioStruct;

    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&stdData);
    if (ret == MMI_S_COMPLETE)
    {
      if ((OMX_AUDIO_CodingUnused != paramAudioStruct.eEncoding) &&
          (OMX_AUDIO_CodingAutoDetect != paramAudioStruct.eEncoding) )
      {
        track->m_nPort = paramAudioStruct.nPortIndex;
      }
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "DASHMMIMediaInfo::GetMetaData aud format(%d) %d port %d",code,
                    paramAudioStruct.eEncoding,
                    track->m_nPort);
      if (mPortIndex != (uint32_t)track->m_nPort)
      {
         track->m_metaData = NULL;
         return NULL;
      }
    }
  } /*if (!m_trackTable.DoesExist(MMI_HTTP_AUDIO_PORT_INDEX))*/

  if (!IS_VALID_PORT(track->m_nPort) &&
      !m_trackTable.DoesExist(MMI_HTTP_OTHER_PORT_INDEX) && (mPortIndex == MMI_HTTP_OTHER_PORT_INDEX ))
  {
    //text port def
    QOMX_STRUCT_INIT(paramOtherStruct, OMX_OTHER_PARAM_PORTFORMATTYPE);
    paramOtherStruct.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX;
    stdData.nParamIndex = OMX_IndexParamOtherPortFormat;
    stdData.pParamStruct = &paramOtherStruct;

    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&stdData);
    if (ret == MMI_S_COMPLETE)
    {
      QOMX_OTHER_CODINGTYPE code = QOMX_OTHER_CodingUnused;

      if (QOMX_OTHER_CodingSMPTETT == (QOMX_OTHER_CODINGTYPE)paramOtherStruct.eFormat)
      {
        track->m_nPort = paramOtherStruct.nPortIndex;
      }
      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "DASHMMIMediaInfo::GetMetaData text format(%d) %d port %d",code,
                paramOtherStruct.eFormat,
                track->m_nPort);
      if (mPortIndex != (uint32_t)track->m_nPort)
      {
        track->m_metaData = NULL;
        return NULL;
      }
    }
  }

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "DASHMMIMediaInfo::GetMetaData trying to set port %d",
                track->m_nPort);

  if (handle && track && (track->m_metaData != 0) &&
      (track->m_nPort == MMI_HTTP_VIDEO_PORT_INDEX))
  {
    track->m_metaData->setInt32(kKeyWidth, paramStruct.format.video.nFrameWidth);
    track->m_metaData->setInt32(kKeyHeight, paramStruct.format.video.nFrameHeight);
    track->m_metaData->setInt32(kKeyBitRate, DASHMMIInterface::BITRATE);
    track->m_metaData->setInt32(kKeyStride, paramStruct.format.video.nStride);
    track->m_metaData->setInt32(kKeySliceHeight, paramStruct.format.video.nSliceHeight);

    duration = GetDuration(track->m_nPort);
    track->m_metaData->setInt64(kKeyDuration, duration);
    mime = VideoEncodingToMIME(paramStruct.format.video.eCompressionFormat);
    if (NULL != mime)
    {
      track->m_metaData->setCString(kKeyMIMEType, mime);
    }
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DASHMMIMediaInfo::GetMetaData video "
                  "width=%u heigth=%u format=%d",
                  (uint32)paramStruct.format.video.nFrameWidth,
                  (uint32)paramStruct.format.video.nFrameHeight,
                  paramStruct.format.video.eCompressionFormat);

    //select audio, video streams
    MMI_OmxParamCmdType activateStream;
    OMX_PARAM_U32TYPE audioStream, videoStream;
    QOMX_STRUCT_INIT(audioStream, OMX_PARAM_U32TYPE);
    audioStream.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
    audioStream.nU32 = 0;
    activateStream.nParamIndex = OMX_IndexParamActiveStream;
    activateStream.pParamStruct = &audioStream;

    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_SET_STD_OMX_PARAM,&activateStream);

    QOMX_STRUCT_INIT(videoStream, OMX_PARAM_U32TYPE);
    videoStream.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
    videoStream.nU32 = 0;
    activateStream.nParamIndex = OMX_IndexParamActiveStream;
    activateStream.pParamStruct = &videoStream;

    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_SET_STD_OMX_PARAM,&activateStream);

  }
  else if (handle && track && (track->m_metaData != 0) &&
           (track->m_nPort == MMI_HTTP_AUDIO_PORT_INDEX))
  {
    mime = AudioEncodingToMIME(paramAudioStruct.eEncoding);
    if (NULL != mime)
    {
      track->m_metaData->setCString(kKeyMIMEType, mime);
    }

    duration = GetDuration(track->m_nPort);
    track->m_metaData->setInt64(kKeyDuration, duration);

    switch(paramAudioStruct.eEncoding)
    {
      case OMX_AUDIO_CodingAAC:
      {
        OMX_AUDIO_PARAM_AACPROFILETYPE aacFmt;
        QOMX_STRUCT_INIT(aacFmt, OMX_AUDIO_PARAM_AACPROFILETYPE);
        stdData.nParamIndex = OMX_IndexParamAudioAac;
        aacFmt.nPortIndex = paramAudioStruct.nPortIndex;
        stdData.pParamStruct = &aacFmt;


    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&stdData);

        track->m_metaData->setInt32(kKeyChannelCount, aacFmt.nChannels);
        track->m_metaData->setInt32(kKeySampleRate, aacFmt.nSampleRate);
        track->m_metaData->setInt32(kKeyBitRate, DASHMMIInterface::BITRATE); //!Warning : Not sure if here the Bit rate needs to be set or some thing else
       //track->m_metaData->setInt32(kKeyBitRate, aacFmt.nBitRate);
        AddAACCodecSpecificData(track->m_metaData);

        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DASHMMIMediaInfo::GetMetaData audio aac "
                      "channels=%u samplerate=%u bitrate=%u",
                      (uint32)aacFmt.nChannels, (uint32)aacFmt.nSampleRate, (uint32)aacFmt.nBitRate);
      }
      break;
      case OMX_AUDIO_CodingMP3:
      {
        OMX_AUDIO_PARAM_MP3TYPE mp3Fmt;
        QOMX_STRUCT_INIT(mp3Fmt, OMX_AUDIO_PARAM_MP3TYPE);
        stdData.nParamIndex = OMX_IndexParamAudioMp3;
        mp3Fmt.nPortIndex = paramAudioStruct.nPortIndex;
        stdData.pParamStruct = &mp3Fmt;

    ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&stdData);

        track->m_metaData->setInt32(kKeyChannelCount, mp3Fmt.nChannels);
        track->m_metaData->setInt32(kKeySampleRate, mp3Fmt.nSampleRate);
        track->m_metaData->setInt32(kKeyBitRate, DASHMMIInterface::BITRATE);
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DASHMMIMediaInfo::GetMetaData audio mp3 "
                      "channels=%u samplerate=%u bitrate=%u",
                      (uint32)mp3Fmt.nChannels, (uint32)mp3Fmt.nSampleRate, (uint32)mp3Fmt.nBitRate);
      }
      break;

      default:
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "DASHMMIMediaInfo::GetMetaData audio unknown encoding");
      }
      break;
    }
  }
  else if (handle && track && (track->m_metaData != 0) &&
           (track->m_nPort == MMI_HTTP_OTHER_PORT_INDEX))
  {
    mime = TextEncodingToMIME(paramOtherStruct.eFormat);
    if (NULL != mime)
    {
      track->m_metaData->setCString(kKeyMIMEType, mime);
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "DASHMMIMediaInfo::GetMetaData unknown media");
  }

  return track->m_metaData;
}
  else
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                 "DASHMMIMediaInfo::GetMetaData Metadata not available yet");
  }

  return NULL;
}

int64 DASHMMIMediaInfo::GetDuration(OMX_U32 nPort) const
{
  OMX_HANDLETYPE handle = NULL;
  MMI_GetExtensionCmdType ext;
  OMX_INDEXTYPE bufDurIndex = OMX_IndexComponentStartUnused;
  int64 duration = 0;
  OMX_U32 ret = MMI_S_EFAIL;

  if (m_mmDASHMMIInterface != NULL)
  {
    handle = m_mmDASHMMIInterface->GetMMIHandle();
  }

  ext.cParamName = (OMX_STRING)OMX_QCOM_INDEX_CONFIG_MEDIAINFO;
  ext.pIndex = &bufDurIndex;

  ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_EXTENSION_INDEX,&ext);


  if (ret == MMI_S_COMPLETE)
  {
    MMI_OmxParamCmdType cmd;
    QOMX_MEDIAINFOTYPE *pExtMediaInfo;
        uint32 size = (uint32)(sizeof(QOMX_MEDIAINFOTYPE) +
                  2 * sizeof(OMX_TICKS));
        pExtMediaInfo = (QOMX_MEDIAINFOTYPE*)MM_Malloc(size * sizeof(OMX_U8));
    if (pExtMediaInfo)
    {
          QOMX_STRUCT_INIT(*pExtMediaInfo, QOMX_MEDIAINFOTYPE);
      pExtMediaInfo->nSize = size;
      pExtMediaInfo->nPortIndex = nPort;
          pExtMediaInfo->eTag = QOMX_MediaInfoDuration;
          pExtMediaInfo->nDataSize = (OMX_U32)(2 * sizeof(OMX_TICKS));
      cmd.nParamIndex = bufDurIndex;
      cmd.pParamStruct = pExtMediaInfo;

          ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&cmd);
          if (ret == MMI_S_COMPLETE)
          {
        duration = *((OMX_TICKS*)((void *)pExtMediaInfo->cData));
        }
        MM_Free(pExtMediaInfo);
    } /*if (pExtMediaInfo)*/
  }

  return duration;
}


void DASHMMIMediaInfo::AddAACCodecSpecificData(sp<MetaData> &metaData)
{
  OMX_HANDLETYPE handle = NULL;
  MMI_GetExtensionCmdType ext;
  OMX_INDEXTYPE syntaxIndex = OMX_IndexComponentStartUnused;
  OMX_U32 ret = MMI_S_EFAIL;

  if (m_mmDASHMMIInterface != NULL)
  {
    handle = m_mmDASHMMIInterface->GetMMIHandle();
  }

  ext.cParamName = (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_SYNTAXHDR;
  ext.pIndex = &syntaxIndex;

  ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_EXTENSION_INDEX,&ext);

  if (ret == MMI_S_COMPLETE)
  {
    MMI_OmxParamCmdType cmd;
    QOMX_VIDEO_SYNTAXHDRTYPE *pSyntaxHdr;
        uint32 size = (uint32)(sizeof(QOMX_VIDEO_SYNTAXHDRTYPE) +
                  APPROX_CONFIG_HDR_SIZE);
        pSyntaxHdr = (QOMX_VIDEO_SYNTAXHDRTYPE*)MM_Malloc(size * sizeof(OMX_U8));
    if (pSyntaxHdr)
    {
          QOMX_STRUCT_INIT(*pSyntaxHdr, QOMX_VIDEO_SYNTAXHDRTYPE);
      pSyntaxHdr->nSize = size;
      pSyntaxHdr->nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
          pSyntaxHdr->nBytes = APPROX_CONFIG_HDR_SIZE;
      cmd.nParamIndex = syntaxIndex;
      cmd.pParamStruct = pSyntaxHdr;
          ret = m_mmDASHMMIInterface->postCmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&cmd);
          if (ret == MMI_S_COMPLETE)
          {
        metaData->setData(kKeyAacCodecSpecificData, 0, pSyntaxHdr->data, pSyntaxHdr->nBytes);
        }
        MM_Free(pSyntaxHdr);
    } /*if (pSyntaxHdr)*/
  }

  //make ESDS specific info here...

  return;
}

const char *DASHMMIMediaInfo::AudioEncodingToMIME(OMX_AUDIO_CODINGTYPE coding)
{
  const char *mime = NULL;

  switch (coding)
  {
    case OMX_AUDIO_CodingMP3:
      mime = MEDIA_MIMETYPE_AUDIO_MPEG;
      break;
    case OMX_AUDIO_CodingAMR:
      mime = MEDIA_MIMETYPE_AUDIO_AMR_NB;
      break;
    case OMX_AUDIO_CodingAAC:
      mime = MEDIA_MIMETYPE_AUDIO_AAC;
      //mime = MEDIA_MIMETYPE_AUDIO_RAW;
      break;
    //case
    //  mime = MEDIA_MIMETYPE_CONTAINER_MPEG2TS
    default:
      break;
  }

  return mime;
}

const char *DASHMMIMediaInfo::VideoEncodingToMIME(OMX_VIDEO_CODINGTYPE coding)
{
  const char *mime = NULL;

  switch ((int32)coding)
  {
    case OMX_VIDEO_CodingAVC:
      mime = MEDIA_MIMETYPE_VIDEO_AVC;
      break;
    case OMX_VIDEO_CodingH263:
      mime = MEDIA_MIMETYPE_VIDEO_H263;
      break;
    case OMX_VIDEO_CodingMPEG4:
      mime = MEDIA_MIMETYPE_VIDEO_MPEG4;
      break;
    case OMX_VIDEO_CodingMPEG2:
      mime = MEDIA_MIMETYPE_CONTAINER_MPEG2TS;
      break;
     case (OMX_VIDEO_CODINGTYPE) QOMX_OTHER_CodingHevc:
       mime = MEDIA_MIMETYPE_VIDEO_HEVC;
       break;
    default:
      break;
  }

  return mime;
}

const char *DASHMMIMediaInfo::TextEncodingToMIME(OMX_OTHER_FORMATTYPE coding)
{
  const char *mime = NULL;

  switch ((QOMX_OTHER_CODINGTYPE)coding)
  {
    case QOMX_OTHER_CodingSMPTETT:
      mime = MEDIA_MIMETYPE_TEXT_3GPP;
      break;
    default:
      break;
  }

  return mime;
}

/** @brief  TrackTable CTor
 *  @param[in] none
 *  @return none *
 */
DASHMMIMediaInfo::TrackTable::TrackTable()
{
  for (int i=0; i<MAX_TRACKS; i++)
  {
    m_trackInfo[i].m_nPort = 0;
    m_trackInfo[i].m_nTrackId = -1;
    m_trackInfo[i].m_metaData = NULL;
  }
}

void DASHMMIMediaInfo::TrackTable::ResetTrackInfo(uint32_t nTrackID)
{
  for (int i=0; i<MAX_TRACKS; i++)
  {
    if ((uint32_t)m_trackInfo[i].m_nTrackId == nTrackID)
    {
        m_trackInfo[i].m_nPort = 0;
        m_trackInfo[i].m_nTrackId = -1;
        m_trackInfo[i].m_metaData = NULL;
        break;
    }
  }
}


/** @brief  TrackTable DTor
 *  @param[in] none
 *  @return none *
 */
DASHMMIMediaInfo::TrackTable::~TrackTable()
{
}

/** @brief  FetchTrackInfo
 *  @param[in] index trackid
 *  @return TrackTable::TrackInfo *
 */
DASHMMIMediaInfo::TrackTable::TrackInfo *
DASHMMIMediaInfo::TrackTable::FetchTrackInfo(size_t index,uint32_t mPortIndex)
{
  TrackTable::TrackInfo *tmp = NULL;

  for (int i=0; i<MAX_TRACKS; i++)
  {
    //does this track exist in table
    if ( ((uint32_t)m_trackInfo[i].m_nTrackId == index) && (mPortIndex == (uint32_t)m_trackInfo[i].m_nPort))
    {
      tmp = &m_trackInfo[i];
      break;
    }
  }
  //if not found then return the free entry
  if(tmp == NULL)
  {
    for (int i=0; i<MAX_TRACKS; i++)
    {
      if (m_trackInfo[i].m_nTrackId == -1)
      {
        m_trackInfo[i].m_nTrackId = i;
        tmp = &m_trackInfo[i];
        break;
      }
    }
  }

  return tmp;
}

bool DASHMMIMediaInfo::TrackTable::DoesExist(OMX_U32 idx)
{
  bool bExist = false;
  TrackTable::TrackInfo *tmp = NULL;

  for (int i=0; i<MAX_TRACKS; i++)
  {
    //does this track exist in table
    if ((uint32_t)m_trackInfo[i].m_nPort == idx)
    {
      bExist = true;
      break;
    }
  }

  return bExist;
}

status_t DASHMMIMediaInfo::setMediaPresence(int kWhome, bool mute)
{
   if (kWhome == 1)
   {
      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "bHasAudio set to %d from %d",mute,bHasAudio);
      bHasAudio = mute;
   }
   else if (kWhome == 0)
   {
     bHasVideo = mute;
     QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "bHasVideo set to %d",mute);
   }
   else if (kWhome == 2)
   {
     bHasText = mute;
     QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "bHasText set to %d",mute);
   }

   return OK;
}

}  // namespace android

