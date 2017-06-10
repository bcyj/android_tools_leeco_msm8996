/************************************************************************* */
/**
 * DEALUtils.cpp
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

/* =======================================================================
**               Include files for DEALUtils.cpp
** ======================================================================= */

#include "OMX_Index.h"
#include "OMX_Other.h"
#include "DashExtractorAdaptationLayer.h"
#include "DEALUtils.h"
#include "OMX_Types.h"

#include "common_log.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMCriticalSection.h"
#include "qtv_msg.h"
#include "OMX_CoreExt.h"
#include "QOMX_StreamingExtensions.h"
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_StreamingExtensionsPrivate.h"
#include "QOMX_VideoExtensions.h"


namespace android {
/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

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

/*
 * Maps MMI to DEAL (stagefright) error codes
 *
 * @param[in] nValue MMI status code
 *
 * @return mapped error code
 */
status_t DEALUtils::MapMMIToDEALStatus(uint32 nValue)
{
  status_t status;

  switch(nValue)
  {
  case MMI_S_COMPLETE:
    status = OK;
    break;
  case MMI_S_PENDING:
    status = WOULD_BLOCK;
    break;
  case MMI_S_ENOTIMPL:
    status = ERROR_UNSUPPORTED;  //!Warnign
    break;
  case MMI_S_EBUFFREQ:
    status = ERROR_BUFFER_TOO_SMALL;
    break;
  case MMI_S_ECMDQFULL:
    status = NOT_ENOUGH_DATA;
    break;
  case MMI_S_EINVALSTATE:
    status = INVALID_OPERATION;
    break;
  case MMI_S_EFAIL:
  case MMI_S_EFATAL:
  default:
    status = UNKNOWN_ERROR;
    break;
  }

  return status;
}


DEALInterface::DASHSourceTrackType DEALUtils::MapPortIndexToTrackId(uint32 nPortIndex)
{
  DEALInterface::DASHSourceTrackType nTrackId = DEALInterface::eTrackNone;

  if(IS_AUDIO_PORT(nPortIndex))
  {
    nTrackId = DEALInterface::eTrackAudio;
  }
  else if(IS_VIDEO_PORT(nPortIndex))
  {
    nTrackId = DEALInterface::eTrackVideo;
  }
  else if(IS_TEXT_PORT(nPortIndex))
  {
    nTrackId = DEALInterface::eTrackText;
  }
  else
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                         "DEALUtils::MapPortIndexToTrackId unknown port %lu", nPortIndex);
  }

  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DEALUtils::MapPortIndexToTrackId portIndex=%lu trackId=%d", nPortIndex, nTrackId);

  return nTrackId;
}

ECALInterface::DASHSourceTrackType DEALUtils::MapPortToTrackId(uint32 nPortIndex)
{
  ECALInterface::DASHSourceTrackType nTrackId = ECALInterface::eTrackNone;

  if(IS_AUDIO_PORT(nPortIndex))
  {
    nTrackId = ECALInterface::eTrackAudio;
  }
  else if(IS_VIDEO_PORT(nPortIndex))
  {
    nTrackId = ECALInterface::eTrackVideo;
  }
  else if(IS_TEXT_PORT(nPortIndex))
  {
    nTrackId = ECALInterface::eTrackText;
  }
  else
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                         "DEALUtils::MapPortIndexToTrackId unknown port %lu", nPortIndex);
  }

  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                           "DEALUtils::MapPortIndexToTrackId portIndex=%lu trackId=%d", nPortIndex, nTrackId);

  return nTrackId;
}



void DEALUtils::AddOemHeaders(const KeyedVector<String8, String8> *headers, DEALInterface& dealRef)
{
  OMX_INDEXTYPE protocolHeaderIndex;
  OMX_U32 err = MMI_S_EBADPARAM;
  int numHeaders = 0;

  if (!headers)
  {
    return;
  }

  numHeaders = headers->size();
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                "DEALInterface::AddOemHeaders size=%d",
                numHeaders);

  for (int i=0; i<numHeaders; i++)
  {
    const char* hdrMsgToAdd = "";
    const char* hdrName = headers->keyAt(i).string();
    const char* hdrValue = headers->valueAt(i).string();

    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "DEALInterface::AddOemHeaders Header[\"%s\",\"%s\"]",
                  hdrName, hdrValue);

    MMI_GetExtensionCmdType ext;
    ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_STREAMING_PROTOCOLHEADER;
    ext.pIndex = &protocolHeaderIndex;
    err = HTTPMMIDeviceCommand(dealRef.GetMMIHandle(), MMI_CMD_GET_EXTENSION_INDEX, &ext);
    if (err == MMI_S_COMPLETE)
    {
      MMI_OmxParamCmdType cmd;
      int msgHdrSize = std_strlen(hdrMsgToAdd) + std_strlen(hdrName) +
                       std_strlen(hdrValue) + 1;
      int size = sizeof(QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE) + msgHdrSize;
      QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE* configHdr =
                  (QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE *)MM_Malloc(size);
      if (configHdr)
      {
        QOMX_STRUCT_INIT(*configHdr, QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE);
        configHdr->nSize = size;
        configHdr->eMessageType = QOMX_STREAMING_PROTOCOLMESSAGE_REQUEST;
        configHdr->eActionType = QOMX_STREAMING_PROTOCOLHEADERACTION_ADD;
        configHdr->nMessageClassSize = std_strlen(hdrMsgToAdd);
        configHdr->nHeaderNameSize = std_strlen(hdrName);
        configHdr->nHeaderValueSize = std_strlen(hdrValue);
        std_strlprintf((char *)configHdr->messageHeader, msgHdrSize, "%s%s%s",
                     hdrMsgToAdd, hdrName, hdrValue);
        cmd.nParamIndex = protocolHeaderIndex;
        cmd.pParamStruct = configHdr;
        HTTPMMIDeviceCommand(dealRef.GetMMIHandle(), MMI_CMD_SET_STD_OMX_PARAM, &cmd);
        MM_Free(configHdr);
      } /*if (configHdr)*/
    }
  } /*for (int i=0; i< headers.size(); i++)*/
}

void DEALUtils::QueryStreamType(OMX_U32 nPort, DEALInterface& dealRef)
{
  // Query Dash for the Stream type info and set that with in DASH Interface

  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
               "Query stream type on Port : %lu",nPort);
  OMX_HANDLETYPE handle = NULL;
  MMI_GetExtensionCmdType ext;
  OMX_INDEXTYPE bufDurIndex = OMX_IndexComponentStartUnused;
  int64 duration = 0;
  OMX_U32 ret = MMI_S_EFAIL;

  ext.cParamName = (OMX_STRING)OMX_QCOM_INDEX_CONFIG_MEDIAINFO;
  ext.pIndex = &bufDurIndex;
  handle = dealRef.GetMMIHandle();

  ret = dealRef.postMMICmd(handle,MMI_CMD_GET_EXTENSION_INDEX,&ext);

  if (ret == MMI_S_COMPLETE)
  {
    MMI_OmxParamCmdType cmd;
    QOMX_MEDIAINFOTYPE *pExtMediaInfo;
    uint32 size = sizeof(QOMX_MEDIAINFOTYPE) + sizeof(QOMX_MEDIASTREAMTYPE);
    pExtMediaInfo = (QOMX_MEDIAINFOTYPE*)MM_Malloc(size * sizeof(OMX_U8));

    if (pExtMediaInfo)
    {
      QOMX_STRUCT_INIT(*pExtMediaInfo, QOMX_MEDIAINFOTYPE);
      pExtMediaInfo->nSize = size;
      pExtMediaInfo->nPortIndex = nPort;
      pExtMediaInfo->eTag = QOMX_MediaInfoTagMediaStreamType;
      pExtMediaInfo->nDataSize = sizeof(QOMX_MEDIASTREAMTYPE);
      cmd.nParamIndex = bufDurIndex;
      cmd.pParamStruct = pExtMediaInfo;
      ret = dealRef.postMMICmd(handle,MMI_CMD_GET_STD_OMX_PARAM,&cmd);
      if (ret == MMI_S_COMPLETE)
      {
        QOMX_MEDIASTREAMTYPE *pStreamType = (QOMX_MEDIASTREAMTYPE *)pExtMediaInfo->cData;
        DEALInterface::DashStreamType streamType = DEALInterface::DASH_STREAM_TYPE_VOD;

        if(pStreamType->eStreamType == QOMX_STREAMTYPE_LIVE)
        {
           streamType = DEALInterface::DASH_STREAM_TYPE_LIVE;
        }
        else if (pStreamType->eStreamType == QOMX_STREAMTYPE_VOD)
        {
          streamType = DEALInterface::DASH_STREAM_TYPE_VOD;
        }
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_MEDIUM,
                     "Stream type on Port %lu set to %d ",nPort,streamType);
        dealRef.SetStreamType(streamType);
      }
      MM_Free(pExtMediaInfo);
    } /*if (pExtMediaInfo)*/
  }
}

/** @brief  helps to get meta data from http mmi
 *  @param[in] index track index
 *  @return MetaData smart pointer
 */
void DEALUtils::QueryAndUpdateMetaData
(
 uint32_t mPortIndex,
 sp<MetaData> *metaData,
 bool bIsEnc,
 DEALInterface& dealRef
)
{
  MMI_CustomParamCmdType data;
  MMI_OmxParamCmdType stdData;
  MMI_ParamDomainDefType paramStruct;
  OMX_AUDIO_PARAM_PORTFORMATTYPE paramAudioStruct;
  OMX_OTHER_PARAM_PORTFORMATTYPE paramOtherStruct;
  OMX_U32 ret = MMI_S_EFAIL;
  const char *mime = NULL;
  int64 duration = 0;
  bool isMetadataAvail = false;
  int32_t isDRM = 0;

  QOMX_STRUCT_INIT(paramAudioStruct, OMX_AUDIO_PARAM_PORTFORMATTYPE);
  QOMX_STRUCT_INIT(paramOtherStruct, OMX_OTHER_PARAM_PORTFORMATTYPE);

  if(*metaData != NULL)
  {
    *metaData = NULL;
  }

  if (bIsEnc)
  {
    isDRM = 1;
  }

  *metaData = MM_New(MetaData);

  if(MMI_HTTP_VIDEO_PORT_INDEX == mPortIndex)
  {
    //video port def
    paramStruct.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
    data.nParamIndex = MMI_IndexDomainDef;
    data.pParamStruct = &paramStruct;

    ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_CUSTOM_PARAM, &data);
    if (ret == MMI_S_COMPLETE)
    {
      OMX_VIDEO_CODINGTYPE code= OMX_VIDEO_CodingUnused;

      if (*metaData != NULL)
      {
        (*metaData)->setInt32(kKeyIsDRM, isDRM);
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
             "DEALInterface::GetMetaData Setting kKeyIsDRM");
      }

      if (OMX_VIDEO_CodingAutoDetect == paramStruct.format.video.eCompressionFormat)
      {
        (*metaData)->setCString(kKeyMIMEType, "video/");

        duration = GetDuration(mPortIndex, dealRef);
        (*metaData)->setInt64(kKeyDuration, duration);
      }
      else if (OMX_VIDEO_CodingUnused == paramStruct.format.video.eCompressionFormat)
      {
        *metaData = NULL;
      }
      else
      {
        isMetadataAvail = true;
      }

      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "DEALInterface::GetMetaData vid format(%d) %d port %ld",code,
                paramStruct.format.video.eCompressionFormat,
                paramStruct.nPortIndex);
    }
  }
  else if(MMI_HTTP_AUDIO_PORT_INDEX == mPortIndex)
  {
    //audio port def
    QOMX_STRUCT_INIT(paramAudioStruct, OMX_AUDIO_PARAM_PORTFORMATTYPE);
    paramAudioStruct.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
    stdData.nParamIndex = OMX_IndexParamAudioPortFormat;
    stdData.pParamStruct = &paramAudioStruct;

    ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_STD_OMX_PARAM, &stdData);
    if (ret == MMI_S_COMPLETE)
    {
      OMX_AUDIO_CODINGTYPE code= OMX_AUDIO_CodingUnused;

      if (*metaData != NULL)
      {
        (*metaData)->setInt32(kKeyIsDRM, isDRM);
         QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                    "DEALInterface::GetMetaData Setting kKeyIsDRM");
      }

      if (OMX_AUDIO_CodingAutoDetect == paramAudioStruct.eEncoding)
      {
        (*metaData)->setCString(kKeyMIMEType, "audio/");

        duration = GetDuration(mPortIndex, dealRef);
        (*metaData)->setInt64(kKeyDuration, duration);
      }
      else if (OMX_AUDIO_CodingUnused == paramAudioStruct.eEncoding)
      {
        *metaData = NULL;
      }
      else
      {
        isMetadataAvail = true;
      }

      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                    "DEALInterface::GetMetaData aud format(%d) %d port %ld",code,
                    paramAudioStruct.eEncoding,
                    paramAudioStruct.nPortIndex);
    }
  } /*if (!m_trackTable.DoesExist(MMI_HTTP_AUDIO_PORT_INDEX))*/
  else if(MMI_HTTP_OTHER_PORT_INDEX == mPortIndex)
  {
    //text port def
    QOMX_STRUCT_INIT(paramOtherStruct, OMX_OTHER_PARAM_PORTFORMATTYPE);
    paramOtherStruct.nPortIndex = MMI_HTTP_OTHER_PORT_INDEX;
    stdData.nParamIndex = OMX_IndexParamOtherPortFormat;
    stdData.pParamStruct = &paramOtherStruct;

    ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_STD_OMX_PARAM, &stdData);
    if (ret == MMI_S_COMPLETE)
    {
      QOMX_OTHER_CODINGTYPE code = QOMX_OTHER_CodingUnused;

      if (*metaData != NULL)
      {
        (*metaData)->setInt32(kKeyIsDRM, isDRM);
         QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
              "DEALInterface::GetMetaData Setting kKeyIsDRM");
      }

      if (QOMX_OTHER_CodingAutoDetect == (QOMX_OTHER_CODINGTYPE)paramOtherStruct.eFormat)
      {
        (*metaData)->setCString(kKeyMIMEType, "text/");
      }
      else if (QOMX_OTHER_CodingSMPTETT == (QOMX_OTHER_CODINGTYPE)paramOtherStruct.eFormat)
      {
        isMetadataAvail = true;
      }
      else
      {
        *metaData = NULL;
      }

      QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "DEALInterface::GetMetaData text format(%d) %d port %ld",code,
                paramOtherStruct.eFormat,
                paramOtherStruct.nPortIndex);
    }
  }

  QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_LOW,
                "DEALInterface::GetMetaData trying to set port %u", mPortIndex);

  if(dealRef.GetMMIHandle() && isMetadataAvail && (*metaData != 0) && (MMI_HTTP_VIDEO_PORT_INDEX == mPortIndex))
  {
    (*metaData)->setInt32(kKeyWidth, paramStruct.format.video.nFrameWidth);
    (*metaData)->setInt32(kKeyHeight, paramStruct.format.video.nFrameHeight);
    (*metaData)->setInt32(kKeyBitRate, DEALInterface::BITRATE);
    (*metaData)->setInt32(kKeyStride, paramStruct.format.video.nStride);
    (*metaData)->setInt32(kKeySliceHeight, paramStruct.format.video.nSliceHeight);

    duration = GetDuration(mPortIndex, dealRef);
    (*metaData)->setInt64(kKeyDuration, duration);
    mime = VideoEncodingToMIME(paramStruct.format.video.eCompressionFormat);
    if (NULL != mime)
    {
      (*metaData)->setCString(kKeyMIMEType, mime);
    }
    QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                  "DEALInterface::GetMetaData video "
                  "width=%lu heigth=%lu format=%d",
                  paramStruct.format.video.nFrameWidth,
                  paramStruct.format.video.nFrameHeight,
                  paramStruct.format.video.eCompressionFormat);

    //select audio, video streams
    MMI_OmxParamCmdType activateStream;
    OMX_PARAM_U32TYPE audioStream, videoStream;
    QOMX_STRUCT_INIT(audioStream, OMX_PARAM_U32TYPE);
    audioStream.nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
    audioStream.nU32 = 0;
    activateStream.nParamIndex = OMX_IndexParamActiveStream;
    activateStream.pParamStruct = &audioStream;

    ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_SET_STD_OMX_PARAM, &activateStream);

    QOMX_STRUCT_INIT(videoStream, OMX_PARAM_U32TYPE);
    videoStream.nPortIndex = MMI_HTTP_VIDEO_PORT_INDEX;
    videoStream.nU32 = 0;
    activateStream.nParamIndex = OMX_IndexParamActiveStream;
    activateStream.pParamStruct = &videoStream;

    ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_SET_STD_OMX_PARAM, &activateStream);

  }
  else if (dealRef.GetMMIHandle() && isMetadataAvail && (*metaData != 0) && (MMI_HTTP_AUDIO_PORT_INDEX == mPortIndex))
  {
    mime = AudioEncodingToMIME(paramAudioStruct.eEncoding);
    if (NULL != mime)
    {
      (*metaData)->setCString(kKeyMIMEType, mime);
    }

    duration = GetDuration(mPortIndex, dealRef);
    (*metaData)->setInt64(kKeyDuration, duration);

    switch(paramAudioStruct.eEncoding)
    {
      case OMX_AUDIO_CodingAAC:
      {
        OMX_AUDIO_PARAM_AACPROFILETYPE aacFmt;
        QOMX_STRUCT_INIT(aacFmt, OMX_AUDIO_PARAM_AACPROFILETYPE);
        stdData.nParamIndex = OMX_IndexParamAudioAac;
        aacFmt.nPortIndex = paramAudioStruct.nPortIndex;
        stdData.pParamStruct = &aacFmt;


        ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_STD_OMX_PARAM, &stdData);

        (*metaData)->setInt32(kKeyChannelCount, aacFmt.nChannels);
        (*metaData)->setInt32(kKeySampleRate, aacFmt.nSampleRate);
        (*metaData)->setInt32(kKeyBitRate, DEALInterface::BITRATE); //!Warning : Not sure if here the Bit rate needs to be set or some thing else
       //metaData->setInt32(kKeyBitRate, aacFmt.nBitRate);
        AddAACCodecSpecificData(*metaData, dealRef);

        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DEALInterface::GetMetaData audio aac "
                      "channels=%lu samplerate=%lu bitrate=%lu",
                      aacFmt.nChannels, aacFmt.nSampleRate, aacFmt.nBitRate);
      }
      break;
      case OMX_AUDIO_CodingMP3:
      {
        OMX_AUDIO_PARAM_MP3TYPE mp3Fmt;
        QOMX_STRUCT_INIT(mp3Fmt, OMX_AUDIO_PARAM_MP3TYPE);
        stdData.nParamIndex = OMX_IndexParamAudioMp3;
        mp3Fmt.nPortIndex = paramAudioStruct.nPortIndex;
        stdData.pParamStruct = &mp3Fmt;

        ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_STD_OMX_PARAM, &stdData);

        (*metaData)->setInt32(kKeyChannelCount, mp3Fmt.nChannels);
        (*metaData)->setInt32(kKeySampleRate, mp3Fmt.nSampleRate);
        (*metaData)->setInt32(kKeyBitRate, DEALInterface::BITRATE);
        QTV_MSG_PRIO3(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                      "DEALInterface::GetMetaData audio mp3 "
                      "channels=%lu samplerate=%lu bitrate=%lu",
                      mp3Fmt.nChannels, mp3Fmt.nSampleRate, mp3Fmt.nBitRate);
      }
      break;

      default:
      {
        QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                     "DEALInterface::GetMetaData audio unknown encoding");
      }
      break;
    }
  }
  else if (dealRef.GetMMIHandle() &&  isMetadataAvail && (*metaData != 0) && (MMI_HTTP_OTHER_PORT_INDEX == mPortIndex))
  {
    mime = TextEncodingToMIME(paramOtherStruct.eFormat);
    if (NULL != mime)
    {
      (*metaData)->setCString(kKeyMIMEType, mime);
    }
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "DEALInterface::GetMetaData unknown media");
  }

}

int64 DEALUtils::GetDuration(OMX_U32 nPort, DEALInterface& dealRef)
{
  OMX_HANDLETYPE handle = NULL;
  MMI_GetExtensionCmdType ext;
  OMX_INDEXTYPE bufDurIndex = OMX_IndexComponentStartUnused;
  int64 duration = 0;
  OMX_U32 ret = MMI_S_EFAIL;

  ext.cParamName = (OMX_STRING)OMX_QCOM_INDEX_CONFIG_MEDIAINFO;
  ext.pIndex = &bufDurIndex;

  ret = dealRef.postMMICmd(dealRef.GetMMIHandle(),MMI_CMD_GET_EXTENSION_INDEX,&ext);


  if (ret == MMI_S_COMPLETE)
  {
    MMI_OmxParamCmdType cmd;
    QOMX_MEDIAINFOTYPE *pExtMediaInfo;
        uint32 size = sizeof(QOMX_MEDIAINFOTYPE) +
                  2 * sizeof(OMX_TICKS);
        pExtMediaInfo = (QOMX_MEDIAINFOTYPE*)MM_Malloc(size * sizeof(OMX_U8));
    if (pExtMediaInfo)
    {
          QOMX_STRUCT_INIT(*pExtMediaInfo, QOMX_MEDIAINFOTYPE);
      pExtMediaInfo->nSize = size;
      pExtMediaInfo->nPortIndex = nPort;
          pExtMediaInfo->eTag = QOMX_MediaInfoDuration;
          pExtMediaInfo->nDataSize = 2 * sizeof(OMX_TICKS);
      cmd.nParamIndex = bufDurIndex;
      cmd.pParamStruct = pExtMediaInfo;

          ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_STD_OMX_PARAM, &cmd);
          if (ret == MMI_S_COMPLETE)
          {
            duration = *(OMX_TICKS*)pExtMediaInfo->cData;
          }

      MM_Free(pExtMediaInfo);
    } /*if (pExtMediaInfo)*/
  }

  return duration;
}


void DEALUtils::AddAACCodecSpecificData(sp<MetaData> &metaData, DEALInterface& dealRef)
{
  OMX_HANDLETYPE handle = NULL;
  MMI_GetExtensionCmdType ext;
  OMX_INDEXTYPE syntaxIndex = OMX_IndexComponentStartUnused;
  OMX_U32 ret = MMI_S_EFAIL;

  ext.cParamName = (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_SYNTAXHDR;
  ext.pIndex = &syntaxIndex;

  ret = dealRef.postMMICmd(dealRef.GetMMIHandle(),MMI_CMD_GET_EXTENSION_INDEX,&ext);

  if (ret == MMI_S_COMPLETE)
  {
    MMI_OmxParamCmdType cmd;
    QOMX_VIDEO_SYNTAXHDRTYPE *pSyntaxHdr;
        uint32 size = sizeof(QOMX_VIDEO_SYNTAXHDRTYPE) +
                  APPROX_CONFIG_HDR_SIZE;
        pSyntaxHdr = (QOMX_VIDEO_SYNTAXHDRTYPE*)MM_Malloc(size * sizeof(OMX_U8));
    if (pSyntaxHdr)
    {
          QOMX_STRUCT_INIT(*pSyntaxHdr, QOMX_VIDEO_SYNTAXHDRTYPE);
      pSyntaxHdr->nSize = size;
      pSyntaxHdr->nPortIndex = MMI_HTTP_AUDIO_PORT_INDEX;
          pSyntaxHdr->nBytes = APPROX_CONFIG_HDR_SIZE;
      cmd.nParamIndex = syntaxIndex;
      cmd.pParamStruct = pSyntaxHdr;
          ret = dealRef.postMMICmd(dealRef.GetMMIHandle(),MMI_CMD_GET_STD_OMX_PARAM,&cmd);
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

const char *DEALUtils::AudioEncodingToMIME(OMX_AUDIO_CODINGTYPE coding)
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

const char *DEALUtils::VideoEncodingToMIME(OMX_VIDEO_CODINGTYPE coding)
{
  const char *mime = NULL;

  switch (coding)
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
    default:
      break;
  }

  return mime;
}

const char *DEALUtils::TextEncodingToMIME(OMX_OTHER_FORMATTYPE coding)
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

/** @brief   Http Media Source start, creates mediabuffer
 *  @param[in] params
 *  @return status of operation
 */
status_t DEALUtils::UpdateBufReq(DEALInterface::TrackInfo& info, uint32_t index, DEALInterface& dealRef)
{
  const char *mime = NULL;
  status_t err = UNKNOWN_ERROR;
  OMX_U32 ret = MMI_S_EFAIL;

  MMI_CustomParamCmdType data;
  MMI_ParamBuffersReqType paramStruct;

  // Get max buffer size
  paramStruct.nPortIndex = index;
  data.nParamIndex = MMI_IndexBuffersReq;
  data.pParamStruct = &paramStruct;

  ret = dealRef.postMMICmd(dealRef.GetMMIHandle(), MMI_CMD_GET_CUSTOM_PARAM, &data);

  if (IS_SUCCESS(ret))
  {
    info.setMaxBufferSize(paramStruct.nDataSize);
    info.setMaxBufferCount(paramStruct.nCount);

    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "DEALInterface::UpdateBufReq port found");
  }
  else
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                  "DEALInterface::UpdateBufReq failed %d", (int)ret);
  }

  return err;
}


}/* namespace android */
