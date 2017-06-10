/************************************************************************* */
/**
 * DashExtractorCryptoAdaptionLayer.cpp
 * @brief Provides implementation for general DRM functionlities
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */


/* =======================================================================
**       Include files for ExtractorCryptoAdaptationLayer.cpp
** ======================================================================= */
#include "ExtractorCryptoAdaptationLayer.h"
#include "DEALUtils.h"
#include "MMMemory.h"
#include "DashCryptoInterface.h"

using namespace android;

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

//OMX_HANDLETYPE ECALInterface::m_MMISourceHandle = NULL;



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


ECALInterface::ECALInterface():
mVideoTrackCryptoObj(NULL),
mAudioTrackCryptoObj(NULL),
m_MMISourceHandle(NULL)
{
}


ECALInterface::~ECALInterface()
{
  if (mVideoTrackCryptoObj)
  {
    MM_Free(mVideoTrackCryptoObj);
    mVideoTrackCryptoObj = NULL;
  }
  if (mAudioTrackCryptoObj)
  {
    MM_Free(mAudioTrackCryptoObj);
    mAudioTrackCryptoObj = NULL;
  }
}


void ECALInterface::SetSourceMMIHandle(OMX_HANDLETYPE mmiHandle)
{
  if (mmiHandle != NULL)
  {
    m_MMISourceHandle = mmiHandle;
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "ECALInterface::Setting mmiHandle");
  }
}

void ECALInterface::SetServiceHandle()
{
  sp<IServiceManager> sm = defaultServiceManager();
  if (sm != NULL)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Success:ServiceManager");
    sp<IBinder> binder = sm->getService(String16("media.player"));
    if (binder != NULL)
    {
      mMediaPlayerServiceHandle = interface_cast<IMediaPlayerService>(binder);
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Success:ServiceHandle");
    }
  }
  return;
}

status_t ECALInterface::CreateCryptoHandle(TrackCryptoObj *trackCryptoObj)
{
  status_t drmErr = DRM_ERROR_UNKNOWN;
  if (mMediaPlayerServiceHandle != NULL)
  {
    sp<ICrypto> cryptoObj = mMediaPlayerServiceHandle->makeCrypto();
    if (cryptoObj != NULL && cryptoObj->initCheck() != OK)
    {
      trackCryptoObj->SetCryptoHandle(cryptoObj);
      drmErr = DRM_NO_ERROR;
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Success:CreateCryptoHandle");
    }
  }
  return drmErr;
}


ECALInterface::DASHEncryptionStatus
ECALInterface::QueryTrackEncInfo(uint32 portIndex, int &drmType)
{
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "PortIndex [%d]: QueryTrackEncInfo", (int)portIndex );

  DASHEncryptionStatus ret = NOT_ENCRYPTED;
  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);
  if (nTrackId == ECALInterface::eTrackVideo)
  {
    if (mVideoTrackCryptoObj)
    {
      ret = mVideoTrackCryptoObj->QueryTrackEncInfo(portIndex, drmType);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio)
  {
    if(mAudioTrackCryptoObj)
    {
      ret = mAudioTrackCryptoObj->QueryTrackEncInfo(portIndex, drmType);
    }
  }
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "PortIndex [%d]: QueryTrackEncInfo returns %d",
                   (int)portIndex, ret);
  return ret;
}


ECALInterface::DASHEncryptionStatus
ECALInterface::QueryDrmInfo(uint32 portIndex, int &drmType)
{
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "PortIndex [%d]: QueryContentProtectionInfo", (int)portIndex );

  DASHEncryptionStatus ret = NOT_ENCRYPTED;
  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);
  if (nTrackId == ECALInterface::eTrackVideo)
  {
    mVideoTrackCryptoObj = MM_New_Args(TrackCryptoObj,
                     (portIndex, nTrackId, ECALInterface::m_MMISourceHandle));
    if (mVideoTrackCryptoObj)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "Success : VideoTrackCryptoObj");
      ret = mVideoTrackCryptoObj->QueryContentProtectionInfo(portIndex,
                                                             drmType);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio)
  {
    mAudioTrackCryptoObj = MM_New_Args(TrackCryptoObj,
                     (portIndex, nTrackId, ECALInterface::m_MMISourceHandle));
    if(mAudioTrackCryptoObj )
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "Success : AudioTrackCryptoObj");
      ret = mAudioTrackCryptoObj->QueryContentProtectionInfo(portIndex,
                                                             drmType);
    }
  }
  return ret;
}


/*


ECALInterface::DASHEncryptionStatus
ECALInterface::QueryPsshInfo(uint32 portIndex, int &drmType)
{
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "PortIndex [%d]: QueryContentProtectionInfo", (int)portIndex );

  DASHEncryptionStatus ret = NOT_ENCRYPTED;
  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);
  if (nTrackId == ECALInterface::eTrackVideo)
  {
    mVideoTrackCryptoObj = MM_New_Args(TrackCryptoObj, (portIndex, nTrackId));
    if (mVideoTrackCryptoObj)
    {
      ret = mVideoTrackCryptoObj->QueryPsshInfo(portIndex, drmType);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio)
  {
    mAudioTrackCryptoObj = MM_New_Args(TrackCryptoObj,(portIndex, nTrackId));
    if(mAudioTrackCryptoObj )
    {
      ret = mAudioTrackCryptoObj->QueryPsshInfo(portIndex, drmType);
    }
  }
  return ret;
}

*/


status_t ECALInterface::CreateCryptoSession(uint32 portIndex)
{
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "PortIndex [%d]: CreateCryptoSession", (int)portIndex );

  status_t drmErr = DRM_ERROR_UNKNOWN;

  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);
  SetServiceHandle();
  if (nTrackId == ECALInterface::eTrackVideo && mVideoTrackCryptoObj)
  {
    drmErr = CreateCryptoHandle(mVideoTrackCryptoObj);
    if (DRM_NO_ERROR == drmErr &&
        QOMX_MARLIN_DRM == mVideoTrackCryptoObj->GetDRMTtype())
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "PortIndex [%d] : CreatePlugin for Marlin",
                 (int)portIndex);
      drmErr = mVideoTrackCryptoObj->CreatePlugin(kUUIDMarlin, sessionID);
    }
    else if (DRM_NO_ERROR == drmErr &&
             QOMX_PLAYREADY_DRM == mVideoTrackCryptoObj->GetDRMTtype())
    {
       QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                    "PortIndex [%d] : CreatePlugin for PlayReady",
                    (int)portIndex);
      drmErr = mVideoTrackCryptoObj->CreatePlugin(kUUIDPlayready, sessionID);
    }
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "PortIndex [%d]:createPlugin Returns %d",
                  (int)portIndex, drmErr);
    if (DRM_NO_ERROR == drmErr)
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Success:PortIndex [%d] CreatePlugin", (int)portIndex);
      drmErr = mVideoTrackCryptoObj->InitializeSession();
    }
    else
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                 "Error:PortIndex [%d] CreatePlugin", (int)portIndex);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio && mAudioTrackCryptoObj)
  {
    drmErr = CreateCryptoHandle(mAudioTrackCryptoObj);
    if (DRM_NO_ERROR == drmErr &&
        QOMX_MARLIN_DRM == mAudioTrackCryptoObj->GetDRMTtype())
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "PortIndex [%d] : CreatePlugin for Marlin",
                   (int)portIndex);
      drmErr = mAudioTrackCryptoObj->CreatePlugin(kUUIDMarlin, sessionID);
    }
    else if (DRM_NO_ERROR == drmErr &&
             QOMX_PLAYREADY_DRM == mAudioTrackCryptoObj->GetDRMTtype())
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                    "PortIndex [%d] : CreatePlugin for PlayReady",
                    (int)portIndex);
      drmErr = mAudioTrackCryptoObj->CreatePlugin(kUUIDPlayready, sessionID);
    }
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "PortIndex [%d]:createPlugin Returns %d",
                  (int)portIndex, drmErr);;
    if (DRM_NO_ERROR == drmErr)
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                     "Success:PortIndex [%d] CreatePlugin", (int)portIndex);
      drmErr = mAudioTrackCryptoObj->InitializeSession();
    }
    else
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                    "Error:PortIndex [%d] CreatePlugin", (int)portIndex);

    }
  }
  return drmErr;
}


void
ECALInterface::ParseExtraData
(
  uint32 portIndex,MMI_BufferCmdType *pCmdBuf,
  QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
  QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo
)
{
  QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "ECALInterface::ParserExtraData");
  DASHEncryptionStatus ret = NOT_ENCRYPTED;
  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);
  if (nTrackId == ECALInterface::eTrackVideo)
  {
    if (mVideoTrackCryptoObj)
    {
      mVideoTrackCryptoObj->ParseExtraData(pCmdBuf,
                                           psshInfo,
                                           extraSampleInfo);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio)
  {
    if(mAudioTrackCryptoObj )
    {
      mAudioTrackCryptoObj->ParseExtraData(pCmdBuf,
                                           psshInfo,
                                           extraSampleInfo);
    }
  }
  return;
}

status_t
ECALInterface::HasDrmMetaInfoChanged
(
  uint32 portIndex,
  uint32 uniqueId
)
{
  status_t drmErr = DRM_NO_ERROR;
  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);

  if (nTrackId == ECALInterface::eTrackVideo)
  {
    if (mVideoTrackCryptoObj)
    {
      drmErr= mVideoTrackCryptoObj->HasDrmMetaInfoChanged(portIndex, uniqueId);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio)
  {
    if(mAudioTrackCryptoObj )
    {
      drmErr = mAudioTrackCryptoObj->HasDrmMetaInfoChanged(portIndex, uniqueId);
    }
  }
  return drmErr;
}


status_t
ECALInterface::DoDecyrpt
(
  uint32 portIndex,
  int encBufferSize,
  int subsampleCount,
  int sampleIVSize,
  const char* sampleIV,
  const char* sampleKID,
  QOMX_ENCRYPTED_SUBSAMPLE_INFO *sEncSubsampleInfo,
  const char *bufferEncData,
  int Fd
)
{
  status_t drmErr = DRM_NO_ERROR;
  DASHSourceTrackType nTrackId = DEALUtils::MapPortToTrackId(portIndex);

  if (nTrackId == ECALInterface::eTrackVideo)
  {
    if (mVideoTrackCryptoObj)
    {
      drmErr= mVideoTrackCryptoObj->DoDecyrpt(portIndex, encBufferSize,
                                              subsampleCount, sampleIVSize,
                                              sampleIV, sampleKID,
                                              sEncSubsampleInfo, bufferEncData,
                                              Fd);
    }
  }
  if (nTrackId == ECALInterface::eTrackAudio)
  {
    if(mAudioTrackCryptoObj )
    {
      drmErr = mAudioTrackCryptoObj->DoDecyrpt(portIndex, encBufferSize,
                                              subsampleCount, sampleIVSize,
                                              sampleIV, sampleKID,
                                              sEncSubsampleInfo, bufferEncData,
                                              Fd);
    }
  }
  return drmErr;

}


ECALInterface::TrackCryptoObj::TrackCryptoObj
(
  uint32 portIndex,
  DASHSourceTrackType trackId,
  OMX_HANDLETYPE mmiHandle
):mICryptoHandle(NULL),
  mPortIndex(portIndex),
  mTrackType(trackId),
  mTrackPSSHDataSize(0),
  mLastUniqueID(0),
  mStreamType(INVALID_STREAM),
  mTrackEncType(NOT_ENCRYPTED),
  mDRMType(QOMX_NO_DRM),
  //m_pInitSessionConfData(NULL),
  m_pCryptoSourceInterface(NULL)
{

  m_pCryptoSourceInterface = MM_New_Args(DashCryptoInterface, (mmiHandle));

  if (m_pCryptoSourceInterface)
  {
     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                 "Success: Create TrackCryptoObj");
  }
  else
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                 "Error:CreateDASHCryptoInterface");
  }
  return ;
}


ECALInterface::TrackCryptoObj::~TrackCryptoObj()
{
  status_t drmErr = DRM_ERROR_UNKNOWN;
  if (m_pCryptoSourceInterface)
  {
    MM_Free(m_pCryptoSourceInterface);
    m_pCryptoSourceInterface = NULL;
  }
  /*
  if (m_pInitSessionConfData)
  {
    MM_Free(m_pInitSessionConfData);
    m_pInitSessionConfData = NULL;
  }
  */
  if (mICryptoHandle != NULL)
  {
    //status_t drmErr = mICryptoHandle->deinitSession();
    if(IS_DRM_SUCCESS(drmErr))
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d Session sucessfully deint'ed", mTrackType);
    }
  }
}

void
ECALInterface::TrackCryptoObj::SetCryptoHandle(sp<ICrypto> cryptoObj)
{
  mICryptoHandle = cryptoObj;
}

status_t
ECALInterface::TrackCryptoObj::CreatePlugin(const uint8_t uuid[16],
                                            int32 sessionID)
{
  return((mICryptoHandle != NULL) ?
      mICryptoHandle->createPlugin(uuid,
        (void *)&sessionID,(sizeof(sessionID))): DRM_ERROR_UNKNOWN);
}


status_t
ECALInterface::TrackCryptoObj::InitializeSession()
{
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : InitSession", mTrackType);
  status_t drmErr = DRM_ERROR_UNKNOWN;

  if (ENCRYPTED == GetTrackEncType())
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d is Encrypted", mTrackType);
    if(m_pCryptoSourceInterface)
    {

      QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *cpInfo =
                 (QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *)
                  m_pCryptoSourceInterface->GetStoredContentProtectionInfo();

      QOMX_PARAM_STREAMING_PSSHINFO *psshInfo =
                  (QOMX_PARAM_STREAMING_PSSHINFO *)
                  m_pCryptoSourceInterface->GetStoredPSSHInfo();

      drmErr = initsession(cpInfo, psshInfo);
    }
  }
  return drmErr;
}
status_t
ECALInterface::TrackCryptoObj::initsession
(
  QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *cpInfo,
  QOMX_PARAM_STREAMING_PSSHINFO *psshInfo
)
{
  status_t drmErr = DRM_NO_ERROR;
  int initSessionConfDataSize = 0;
  initSessionConfDataSize = sizeof(InitSessionConfData);

  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "initSessionConfDataSize %d",
               initSessionConfDataSize);

  if (cpInfo)
  {
    initSessionConfDataSize += (int)cpInfo->nContentProtectionInfoSize;
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "cpInfo Present: nContentProtectionInfoSize %d "
               "initSessionConfDataSize %d",
               (int)cpInfo->nContentProtectionInfoSize,  initSessionConfDataSize);
  }
  if (psshInfo)
  {
    initSessionConfDataSize += (int)psshInfo->nPsshDataBufSize;
    mLastUniqueID = psshInfo->nUniqueID;
    QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "psshInfo Present: nPsshDataBufSize %d "
               "initSessionConfDataSize %d, uniqueId '%d'",
               (int)psshInfo->nPsshDataBufSize,  initSessionConfDataSize,
               mLastUniqueID);
  }

  InitSessionConfData  *m_pInitSessionConfData = NULL;
  m_pInitSessionConfData = (InitSessionConfData *)
                               MM_Malloc(initSessionConfDataSize);

  memset(m_pInitSessionConfData, 0x0, initSessionConfDataSize);

  if (m_pInitSessionConfData)
  {
    m_pInitSessionConfData->StreamType = mStreamType;
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "StreamType %d",
               m_pInitSessionConfData->StreamType);

    if (cpInfo)
    {
      m_pInitSessionConfData->ContentProtectionSize =
                   (uint32)(cpInfo->nContentProtectionInfoSize - 1);
      if (m_pInitSessionConfData->ContentProtectionSize > 0)
      {
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
               "Port[%d]  ContentProtectionSize %d",(int)cpInfo->nPortIndex,
                m_pInitSessionConfData->ContentProtectionSize);
        std_memmove(m_pInitSessionConfData->InitConfData,
                    cpInfo->cContentProtectionData,
                    m_pInitSessionConfData->ContentProtectionSize);
#if DRM_DEBUG_CONTENT_PRINT
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
             "----Content Protection Elemen:Port[%d]:Size[%d]----",
              (int)cpInfo->nPortIndex, (int)m_pInitSessionConfData->ContentProtectionSize);
        printBuf((const char*)m_pInitSessionConfData->InitConfData,
                  (int)m_pInitSessionConfData->ContentProtectionSize);
#endif
#if DRM_DEBUG_FILE_DUMP
        if(mTrackType == eTrackAudio)
        {
          (void)writeToFile((const char*)m_pInitSessionConfData->InitConfData,
                            m_pInitSessionConfData->ContentProtectionSize,
                            "/data/audioCP.hex");
        }
        else if(mTrackType == eTrackVideo)
        {
          (void)writeToFile((const char*)m_pInitSessionConfData->InitConfData,
                            m_pInitSessionConfData->ContentProtectionSize,
                            "/data/videoCP.hex");
        }
#endif

      }
    }
    if (IS_DRM_SUCCESS(drmErr))
    {
      if (psshInfo)
      {
        m_pInitSessionConfData->psshDataSize =
             (uint32)(psshInfo->nPsshDataBufSize - 1);
        QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Port[%d] InitDataSize %d",(int)psshInfo->nPortIndex,
                  m_pInitSessionConfData->psshDataSize);
        if (m_pInitSessionConfData->psshDataSize > 0)
        {
          std_memmove(m_pInitSessionConfData->InitConfData +
                      m_pInitSessionConfData->ContentProtectionSize,
                      psshInfo->cPSSHData,
                      m_pInitSessionConfData->psshDataSize);
#if DRM_DEBUG_CONTENT_PRINT
          QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
             "----PSSH:Port[%d]:Size[%d]----",
              (int)psshInfo->nPortIndex, (int)m_pInitSessionConfData->psshDataSize);
          printBuf((const char*)(m_pInitSessionConfData->InitConfData +
                   m_pInitSessionConfData->ContentProtectionSize),
                   (int)m_pInitSessionConfData->psshDataSize);
          QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
             "----Default_KID:Port[%d]:Size[16]----",
              (int)psshInfo->nPortIndex);
          printBuf((const char*)psshInfo->cDefaultKeyID,
                   MAX_KID_SIZE);
#endif
#if DRM_DEBUG_FILE_DUMP
          if(mTrackType == eTrackAudio)
          {
            (void)writeToFile((const char*)(m_pInitSessionConfData->InitConfData +
                              m_pInitSessionConfData->ContentProtectionSize),
                             m_pInitSessionConfData->InitDataSize,
                               "/data/audioPSSH.hex");
          }
          else if (mTrackType == eTrackVideo)
          {
            (void)writeToFile((const char*)(m_pInitSessionConfData->InitConfData +
                              m_pInitSessionConfData->ContentProtectionSize),
                              m_pInitSessionConfData->InitDataSize,
                              "/data/videoPSSH.hex");
          }
#endif
        }
      }
    }
    if (IS_DRM_SUCCESS(drmErr))
    {
      /*drmErr = mICryptoHandle->initSession(m_pInitSessionConfData,
                                                       initSessionConfDataSize);
      */
      if (IS_DRM_SUCCESS(drmErr))
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                                     "Success: initSession");
      }
      else
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                                     "Error: initSession");
      }
    }
  }
  return drmErr;
}



ECALInterface::DASHEncryptionStatus
ECALInterface::TrackCryptoObj::QueryContentProtectionInfo
(
  uint32 portIndex,
  int &trackDrmType
)
{
  DASHEncryptionStatus ret = ENCRYPTED;
  QOMX_DRM_TYPE drmType = QOMX_NO_DRM;
  ContentStreamType streamType = INVALID_STREAM;
  if(m_pCryptoSourceInterface)
  {
    ContentProtectionStatus retCP = CP_ERROR_UNKNOWN;
    //if (mTrackEncType == NOT_ENCRYPTED )
    //{
      retCP =  m_pCryptoSourceInterface->GetContentProtectionInfo(
                                                   portIndex,
                                                   (int &)drmType,
                                                   (int &)streamType);
      if (retCP == CP_ERROR_UNKNOWN || retCP == CP_DRM_NOT_SUPPORT)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d CP_ERROR_UNKNOWN/CP_DRM_NOT_SUPPORT : UNKNOWN_ERROR",
                 mTrackType);
        ret = UNKNOWN_ERROR;
      }
      else if (retCP == CP_NOT_PRESENT)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d CP_NOT_PRESENT : NOT_ENCRYPTED", mTrackType);
        ret = NOT_ENCRYPTED;
        SetTrackEncType(NOT_ENCRYPTED);
      }
      else
      {
         QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d CP_PRESENT: ENCRYPTED", mTrackType);
         mDRMType =  drmType;
         mStreamType = MP4_STREAM;
         trackDrmType = (int &)drmType;
         QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d CP_PRESENT: ENCRYPTED drmtype %d stremtype %d",
                 mTrackType, mDRMType, mStreamType);
         ret = ENCRYPTED;
         SetTrackEncType(ENCRYPTED);
      }
    //}
  }
  return ret;
}



ECALInterface::DASHEncryptionStatus
ECALInterface::TrackCryptoObj::QueryPsshInfo
(
  uint32 portIndex,
  int &trackDrmType,
  int uniqueId
)
{
  DASHEncryptionStatus ret = UNKNOWN_ERROR;
  if(m_pCryptoSourceInterface)
  {
    PsshStatus retPSSH =
         m_pCryptoSourceInterface->GetPSSHInfo(portIndex, uniqueId);
    if (INIT_ERROR_UNKNOWN == retPSSH)
    {
      ret = UNKNOWN_ERROR;
      if (NOT_ENCRYPTED == GetTrackEncType())
      {
        SetTrackEncType(NOT_ENCRYPTED);
        ret = NOT_ENCRYPTED;
      }
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d INIT_ERROR_UNKNOWN: UNKNOWN_ERROR", mTrackType);

    }
    else if (INIT_PRESENT == retPSSH)
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d INIT_PRESENT : ENCRYPTED", mTrackType);
      ret = ENCRYPTED;
      SetTrackEncType(ENCRYPTED);
    }
    else if (INIT_NOT_PRESENT == retPSSH)
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d INIT_NOT_PRESENT : NOT_ENCRYPTED", mTrackType);
      ret = NOT_ENCRYPTED;
      SetTrackEncType(NOT_ENCRYPTED);
    }
  }
  return ret;
}



ECALInterface::DASHEncryptionStatus
ECALInterface::TrackCryptoObj::QueryTrackEncInfo
(
  uint32 portIndex,
  int &trackDrmType
)
{
  DASHEncryptionStatus ret = UNKNOWN_ERROR;
  if(m_pCryptoSourceInterface)
  {
    if (mTrackEncType == ENCRYPTED)
    {
      ret = QueryPsshInfo(portIndex, trackDrmType);
      if (ret == UNKNOWN_ERROR)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : UNKNOWN_ERROR", mTrackType);
        ret = UNKNOWN_ERROR;
      }
    }
    else
    {
      DASHEncryptionStatus retCP = QueryContentProtectionInfo(portIndex,
                                                              trackDrmType);

      DASHEncryptionStatus retPSSH = QueryPsshInfo(portIndex, trackDrmType);
      if (UNKNOWN_ERROR == retPSSH || UNKNOWN_ERROR == retCP )
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : UNKNOWN_ERROR", mTrackType);
        ret = UNKNOWN_ERROR;
      }
      else if (ENCRYPTED == retCP || ENCRYPTED == retPSSH)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : ENCRYPTED", mTrackType);
        ret = ENCRYPTED;
        SetTrackEncType(ENCRYPTED);
      }
      else if (NOT_ENCRYPTED == retCP && NOT_ENCRYPTED == retPSSH)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : NOT_ENCRYPTED", mTrackType);
        ret = NOT_ENCRYPTED;
        SetTrackEncType(NOT_ENCRYPTED);
      }
    }
  }
   QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : QueryTrackEncInfo returns %d",mTrackType, ret);
  return ret;
}

void
ECALInterface::TrackCryptoObj::ParseExtraData
(
  MMI_BufferCmdType *pCmdBuf,
  QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
  QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo
)
{
  if(m_pCryptoSourceInterface)
  {
    m_pCryptoSourceInterface->ParseExtraData(pCmdBuf,
                                             psshInfo,
                                             extraSampleInfo);
  }
}


status_t
ECALInterface::TrackCryptoObj::DoDecyrpt
(
  uint32 portIndex,
  int encBufferSize,
  int subsampleCount,
  int sampleIVSize,
  const char* sampleIV,
  const char* sampleKID,
  QOMX_ENCRYPTED_SUBSAMPLE_INFO *sEncSubsampleInfo,
  const char *bufferEncData,
  int Fd
)
{
  static int frameCount = 0;
  status_t drmErr = DRM_NO_ERROR;
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
             "Track %d : Decrypting now", mTrackType);

  bool secureFlag = true;
  CryptoPlugin::Mode mode = CryptoPlugin::kMode_AES_CTR;
  char iv[MAX_IV_SIZE];
  memset (iv, 0x0, MAX_IV_SIZE);
#if 1
  for (int i = 0; i < sampleIVSize; i++)
  {
     memcpy(&iv[sampleIVSize - 1 - i], &sampleIV[i], 1);
  }
#else if
  memcpy (iv, sampleIV, sampleIVSize);
#endif
  uint8_t *keyId = (uint8_t *)sampleKID;
  size_t numEncryptedUnits = (size_t)subsampleCount;
  AString *errorDetailMsg = NULL;
  CryptoPlugin::SubSample *subSamples = NULL;
  uint32_t encSize = 0;
  uint32_t clearSize = 0;

#if DRM_DEBUG_FILE_DUMP
  if(mTrackType == eTrackAudio)
  {
    (void)writeToFile((const char*)iv, MAX_IV_SIZE, "/data/audioIV.hex");
    (void)writeToFile((const char*)keyId,MAX_KID_SIZE, "/data/audioKID.hex");
  }
  else
#endif
  if(mTrackType == eTrackVideo && frameCount <= 15)
  {
    (void)writeToFile((const char*)iv, MAX_IV_SIZE, "/data/videoIV.hex");
    //(void)writeToFile((const char*)keyId,MAX_KID_SIZE,"/data/videoKID.hex");
  }
  frameCount++;


  if (numEncryptedUnits > 0 ) //video
  {
  /*
  // hack
    if (numEncryptedUnits == 1)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
             "For Codec Config frame, secure flag is false");
      secureFlag = true;
    }
  */
    subSamples = new CryptoPlugin::SubSample[(int)numEncryptedUnits];
    if (subSamples)
    {
      for (int incUnit=0; incUnit< (int)numEncryptedUnits; incUnit++)
      {
        subSamples[incUnit].mNumBytesOfClearData     =
                  sEncSubsampleInfo[incUnit].nSizeOfClearData;
        subSamples[incUnit].mNumBytesOfEncryptedData =
                  sEncSubsampleInfo[incUnit].nSizeOfEncryptedData;
        encSize += subSamples[incUnit].mNumBytesOfEncryptedData;
        clearSize += sEncSubsampleInfo[incUnit].nSizeOfClearData;
      }
    }
  }
  else //audio
  {
    numEncryptedUnits = 1;
    secureFlag = false;
    subSamples =  new CryptoPlugin::SubSample[(int)numEncryptedUnits];
    if (subSamples)
    {
      subSamples[0].mNumBytesOfClearData = 0;
      subSamples[0].mNumBytesOfEncryptedData = encBufferSize;
      encSize = subSamples[0].mNumBytesOfEncryptedData;
      clearSize = subSamples[0].mNumBytesOfClearData;
    }
  }
  if (subSamples && encBufferSize > 0)
  {
    QTV_MSG_PRIO5(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "Calling crypto decrypt with flag '%d'numEncryptedUnits '%d'"
           "enc length %d clear size %d with Fd 0X%x",
           secureFlag, numEncryptedUnits, encSize, clearSize, Fd);
    int retVal =  (mICryptoHandle != NULL) ?
           mICryptoHandle->decrypt(secureFlag,
                                   keyId,
                                   //(uint8_t *)sampleIV,
                                   (uint8_t *)iv,
                                   mode,
                                   bufferEncData,
                                   subSamples,
                                   numEncryptedUnits,
                                   (void *)Fd,
                                   errorDetailMsg):
                                   DRM_ERROR_CANNOT_HANDLE;
    if (retVal > 0)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "Success :: decrypt");
      drmErr = DRM_NO_ERROR;
    }
    else
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
          "Error[%d] :: decrypt", retVal);
      // Need to map retVal to DrmError
      drmErr = DRM_ERROR_UNKNOWN;
    }
    delete[] subSamples;
    subSamples = NULL;
  }
  else
  {
     QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                   "Error: -Allocation fail for subSamples");
  }
  return drmErr;
}



status_t
ECALInterface::TrackCryptoObj::HasDrmMetaInfoChanged
(
  uint32 portIndex,
  int uniqueId
)
{
  status_t drmErr = DRM_NO_ERROR;
  QTV_MSG_PRIO3(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : mLastUniqueID %d current uniqueId %d",
                   mTrackType, mLastUniqueID, uniqueId);
  if (mLastUniqueID != uniqueId)
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : unique has changed", mTrackType);
    mLastUniqueID = uniqueId;
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Track %d : DeInitializing the Current session", mTrackType);
    //drmErr = mICryptoHandle->deinitSession();
    if (IS_DRM_SUCCESS(drmErr))
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "Track %d : deinitSession Success.Initializing the "
                "Current session", mTrackType);
      int trackDrmType = 0;
      DASHEncryptionStatus retCP =
              QueryContentProtectionInfo(portIndex, trackDrmType);

      DASHEncryptionStatus retPSSH = QueryPsshInfo(portIndex,
                                                   trackDrmType,
                                                   uniqueId);

      DASHEncryptionStatus ret = UNKNOWN_ERROR;
      if (UNKNOWN_ERROR == retPSSH || UNKNOWN_ERROR == retCP )
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : UNKNOWN_ERROR", mTrackType);
        ret = UNKNOWN_ERROR;
      }
      else if (ENCRYPTED == retCP || ENCRYPTED == retPSSH)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : ENCRYPTED", mTrackType);
        ret = ENCRYPTED;
        SetTrackEncType(ENCRYPTED);
      }
      else if (NOT_ENCRYPTED == retCP && NOT_ENCRYPTED == retPSSH)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : NOT_ENCRYPTED", mTrackType);
        ret = NOT_ENCRYPTED;
        SetTrackEncType(NOT_ENCRYPTED);
      }

      if (UNKNOWN_ERROR == ret)
      {
        QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                   "Track %d : UNKNOWN_ERROR", mTrackType);
        drmErr = DRM_ERROR_UNKNOWN;
      }
      else if(ENCRYPTED == ret)
      {
        QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *pCpInfo =
                 (QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *)
                 m_pCryptoSourceInterface->GetStoredContentProtectionInfo();

        QOMX_PARAM_STREAMING_PSSHINFO *pPsshInfo =
                  (QOMX_PARAM_STREAMING_PSSHINFO *)
                  m_pCryptoSourceInterface->GetStoredPSSHInfo();


        drmErr = initsession(pCpInfo, pPsshInfo);
      }
    }
  }
  return drmErr;
}

#if 1 //DRM_DEBUG_FILE_DUMP
void
ECALInterface::TrackCryptoObj::writeToFile
(
  const char* buffer,
  int bufLength,
  const char* path
)
{
  FILE *Fp = fopen(path, "ab+");
  if (Fp)
  {
    int size = fwrite(buffer, 1, bufLength, Fp);
    if (size <= 0)
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL, "Error: fwrite");
    }
  }
  else
  {
   QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL, "Error: fopen");
  }
  fclose(Fp);
  return;
}

#endif

#if DRM_DEBUG_CONTENT_PRINT
void
ECALInterface::TrackCryptoObj::printBuf
(
  const char *buffer,
  int bufLength
)
{
   for (int i = 0; i< bufLength; i++)
   {
     QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
          "[%d]->[%x]", i, buffer[i]);
   }
}

#endif


