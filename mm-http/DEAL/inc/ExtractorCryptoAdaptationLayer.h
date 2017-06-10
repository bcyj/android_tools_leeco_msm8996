
/************************************************************************* */
/**
 * DashExtractorCryptoAdaptionLayer.h
 * @brief Provides implementation for general DRM functionlities
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */



#ifndef __EXTRACTOR_CRYPTO_ADAPTATION_H__
#define __EXTRACTOR_CRYPTO_ADAPTATION_H__

/* =======================================================================
**      Include files for ExtractorCryptoAdaptionLayer.h
** ======================================================================= */
#include "MMCriticalSection.h"
#include "mmiDeviceApi.h"
#include "HTTPSourceMMIEntry.h" // !Warning
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_VideoExtensions.h"
#include "QOMX_StreamingExtensionsPrivate.h"
#include "qtv_msg.h"
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AMessage.h>
#include <drm/DrmManagerClient.h>
#include <IServiceManager.h>
#include <IMediaPlayerService.h>
#include <CryptoSourceInterface.h>

#include "MMDebugMsg.h"
#include "DashMediaSource.h"
#include "Crypto.h"


using namespace android;

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

#define  DRM_DEBUG_FILE_DUMP 0
#define  DRM_DEBUG_CONTENT_PRINT 1


/* -----------------------------------------------------------------------
** Constant / Macro Definitions
** ----------------------------------------------------------------------- */

const uint8_t kUUIDPlayready[16] = {
0x11,0xEF,0x8B,0xA9,0x79,0xD6,0x4A,0xCE,
0xA3,0xC8,0x27,0xDC,0xD5,0x1D,0x21,0x11
};

const uint8_t kUUIDMarlin[16] = {
0x69,0xf9,0x08,0xaf,0x48,0x16,0x46,0xea,
0x91,0x0c,0xcd,0x5d,0xcc,0xcb,0x0a,0x3a};

static int32 sessionID = 1;

#define IS_DRM_SUCCESS(x) ((x)==DRM_NO_ERROR)


/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */


class  ECALInterface : public RefBase{
public:
  enum DASHSourceTrackType
  {
    eTrackAudio = 0,
    eTrackVideo    ,
    eTrackText     ,
    eTrackAll      ,
    eTrackNone     ,
  };

  typedef struct
  {
    ContentStreamType    StreamType;
    uint32               ContentProtectionSize;
    uint32               psshDataSize;
    char                 InitConfData[1];
    //char              ContentProtection[1];
  }InitSessionConfData;

  typedef enum DASHEncryptionStatus
  {
    NOT_ENCRYPTED = 0,
    ENCRYPTED,
    UNKNOWN_ERROR
  }DASHEncryptionStatus;

  class TrackCryptoObj
  {
    public:
      //C'tor
      TrackCryptoObj(){;}
      //C'tor
      TrackCryptoObj(uint32 portIndex,
                    DASHSourceTrackType trackId,
                    OMX_HANDLETYPE mmiHandle);
      //d'tor
      virtual ~TrackCryptoObj();

      DASHEncryptionStatus QueryTrackEncInfo(uint32 portIndex,
                                            int &trackDrmType);

      DASHEncryptionStatus QueryContentProtectionInfo(uint32 portIndex,
                                         int &trackDrmType);

      DASHEncryptionStatus QueryPsshInfo(uint32 portIndex,
                                         int &trackDrmType,
                                         int uniqueId = 0);

      PsshStatus GetPSSHInfo(uint32 portIndex, int uniqueId = 0);

      ContentProtectionStatus GetContentProtectionInfo(uint32 portIndex,
                                                       int &drmType,
                                                       int &streamType);

      void SetSourceMMIHandle(OMX_HANDLETYPE mmiSourceHandle);

      void SetCryptoHandle(sp<ICrypto> cryptoObj);

      status_t CreatePlugin(const uint8_t uuid[16], int32 sessionID);

      status_t InitializeSession();

      void ParseExtraData(MMI_BufferCmdType *pCmdBuf,
                          QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
                          QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo);

      status_t DoDecyrpt (uint32 portIndex,
                          int encBufferSize,
                          int subsampleCount,
                          int sampleIVSize,
                          const char* sampleIV,
                          const char* sampleKID,
                          QOMX_ENCRYPTED_SUBSAMPLE_INFO *sEncSubsampleInfo,
                          const char *bufferEncData,
                          int Fd);

      status_t HasDrmMetaInfoChanged(uint32 portIndex,int uniqueId);

      QOMX_DRM_TYPE GetDRMTtype()
      {
        return mDRMType;
      }

    private:
     sp<ICrypto>                                mICryptoHandle;
     uint32                                     mPortIndex;
     DASHSourceTrackType                        mTrackType;
     uint32_t                                   mTrackPSSHDataSize;
     int                                        mLastUniqueID;
     ContentStreamType                          mStreamType;
     DASHEncryptionStatus                       mTrackEncType;
     QOMX_DRM_TYPE                              mDRMType;
     //InitSessionConfData                        *m_pInitSessionConfData;
     CryptoSourceInterface                      *m_pCryptoSourceInterface;

     DASHEncryptionStatus  GetTrackEncType()
     {
       return mTrackEncType;
     }

     void  SetTrackEncType(DASHEncryptionStatus trackEncType)
     {
       mTrackEncType = trackEncType;
     }

     status_t initsession
     (
        QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *cpInfo,
        QOMX_PARAM_STREAMING_PSSHINFO *psshInfo
     );

     Mutex mDrmLock;



#if 1//def DRM_DEBUG_FILE_DUMP
     void writeToFile(const char *buffer, int bufLength,const char*path);
#endif
#ifdef DRM_DEBUG_CONTENT_PRINT
     void printBuf(const char *buffer, int bufLength);
#endif

  };
protected:
  //d'tor
  virtual ~ECALInterface();

private:
  TrackCryptoObj *mVideoTrackCryptoObj;

  TrackCryptoObj *mAudioTrackCryptoObj;

  sp<IMediaPlayerService> mMediaPlayerServiceHandle;

  OMX_HANDLETYPE m_MMISourceHandle;

  void SetServiceHandle();

  status_t CreateCryptoHandle(TrackCryptoObj *trackCryptoObj);

public:
  //c'tor
  ECALInterface();
  status_t CreateCryptoSession(uint32 portIndex);

  DASHEncryptionStatus QueryDrmInfo(uint32 portIndex, int &trackDrmType);

  //DASHEncryptionStatus QueryPsshInfo(uint32 portIndex, int &trackDrmType);

  DASHEncryptionStatus QueryTrackEncInfo(uint32 nPortIndex, int &trackDrmType);

  void SetSourceMMIHandle(OMX_HANDLETYPE mmiHandle);


  void ParseExtraData(uint32 portIndex, MMI_BufferCmdType *pCmdBuf,
                      QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
                      QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo);

  status_t DoDecyrpt (uint32 portIndex,
                      int encBufferSize,
                      int subsampleCount,
                      int sampleIVSize,
                      const char* sampleIV,
                      const char* sampleKID,
                      QOMX_ENCRYPTED_SUBSAMPLE_INFO *sEncSubsampleInfo,
                      const char *bufferEncData,
                      int Fd);

  status_t HasDrmMetaInfoChanged(uint32 portIndex, uint32 uniqueId);


};

#endif
