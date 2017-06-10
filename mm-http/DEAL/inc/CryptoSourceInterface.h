
/************************************************************************* */
/**
 * CryptoSourceInterface.h
 * @brief provides interface DRM functionlities
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */


#ifndef __CRYPTO_SOURCE_INTERFACE_H__
#define __CRYPTO_SOURCE_INTERFACE_H__

/* =======================================================================
**               Include files for CryptoSourceInterface.h
** ======================================================================= */
#include "MMCriticalSection.h"
#include "mmiDeviceApi.h"
#include "HTTPSourceMMIEntry.h" // !Warning
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_VideoExtensions.h"
#include "QOMX_StreamingExtensionsPrivate.h"
#include "qtv_msg.h"
#include "MMDebugMsg.h"


using namespace android;

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

typedef enum ContentProtectionStatus
{
  CP_PRESENT = 0,
  CP_NOT_PRESENT,
  CP_DRM_NOT_SUPPORT,
  CP_ERROR_UNKNOWN
}ContentProtectionStatus;

typedef enum PsshStatus
{
  INIT_PRESENT = 0,
  INIT_NOT_PRESENT,
  INIT_ERROR_UNKNOWN
}PsshStatus;

typedef enum DASHEncryptionStatus
{
  NOT_ENCRYPTED = 0,
  ENCRYPTED,
  UNKNOWN_ERROR
}DASHEncryptionStatus;

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */

class CryptoSourceInterface
{
  public:
   virtual PsshStatus GetPSSHInfo(uint32 portIndex,
                                  int uniqueId = 0) = 0;

   virtual ContentProtectionStatus GetContentProtectionInfo(uint32 portIndex,
                                 int &drmType,
                                 int &steamType) = 0;

   virtual void *GetStoredPSSHInfo() = 0;

   virtual void *GetStoredContentProtectionInfo() = 0;

   virtual void ParseExtraData(MMI_BufferCmdType *pCmdBuf,
                               QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
                               QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo) = 0;

   virtual ~CryptoSourceInterface(){};
};


#endif





