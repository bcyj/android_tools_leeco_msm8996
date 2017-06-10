/************************************************************************* */
/**
 * DashCryptoInterface.h
 * @brief implements drm specific functionalities for  DASH
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */


#ifndef __DASH_CRYPTO_INTERFACE_H__
#define __DASH_CRYPTO_INTERFACE_H__

/* =======================================================================
**               Include files for DashCrypoInterface.cpp
** ======================================================================= */
#include "MMCriticalSection.h"
#include "mmiDeviceApi.h"
#include "HTTPSourceMMIEntry.h" // !Warning
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_VideoExtensions.h"
#include "QOMX_StreamingExtensionsPrivate.h"
#include "qtv_msg.h"
#include "MMDebugMsg.h"
#include "CryptoSourceInterface.h"
#include "MMMemory.h"


using namespace android;

/* =======================================================================
**                      Data Declarations
** ======================================================================= */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Definitions
** ======================================================================= */



class DashCryptoInterface:public CryptoSourceInterface
{
  private:
    int mLastUniqueId;
    OMX_HANDLETYPE m_MMISourceHandle;
    OMX_INDEXTYPE  mIndexDrmExtraSampleInfo;
    OMX_INDEXTYPE  mIndexDrmParamPsshInfo;
    QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO* m_pContentProtectionInfo;
    QOMX_PARAM_STREAMING_PSSHINFO* m_pPsshInfo;

    int OMXPrefetch(OMX_STRING param, void* omxStruct, OMX_INDEXTYPE *omxIndex);

    void GetIndexForExtensions(/*OMX_STRING param, OMX_INDEXTYPE *omxIndex*/);
  public:
    DashCryptoInterface(OMX_HANDLETYPE mmiHandle);

    virtual ~DashCryptoInterface();

    virtual PsshStatus GetPSSHInfo(uint32 portIndex, int uniqueId = 0);

    virtual ContentProtectionStatus GetContentProtectionInfo(uint32 portIndex,
                                                             int &drmType,
                                                             int &steamType);

    virtual void *GetStoredPSSHInfo();

    virtual void *GetStoredContentProtectionInfo();

    virtual void ParseExtraData(MMI_BufferCmdType *pMMICmdBuf,
                                QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
                                QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo);

    OMX_HANDLETYPE GetMMIHandle()
    {
      return m_MMISourceHandle;
    }
};


#endif

