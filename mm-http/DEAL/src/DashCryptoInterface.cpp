
/************************************************************************* */
/**
 * DashCryptoInterface.h
 * @brief implements drm specific functionalities for  DASH
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 ************************************************************************* */

/* =======================================================================
**       Include files for DashCryptoInterface.cpp
** ======================================================================= */
#include "DashCryptoInterface.h"



using namespace android;
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



DashCryptoInterface::DashCryptoInterface
(
 OMX_HANDLETYPE mmiHandle
)
:mLastUniqueId(0),
m_MMISourceHandle(mmiHandle),
mIndexDrmExtraSampleInfo(OMX_IndexComponentStartUnused),
mIndexDrmParamPsshInfo(OMX_IndexComponentStartUnused),
m_pContentProtectionInfo(NULL),
m_pPsshInfo(NULL)
{
  /*
  GetIndexForExtensions((OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO,
                        &mIndexDrmParamPsshInfo);
  GetIndexForExtensions((OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_EXTRASAMPLE_INFO,
                        &mIndexDrmExtraSampleInfo);
  */
  GetIndexForExtensions();
}


DashCryptoInterface::~DashCryptoInterface()
{
  if(m_pPsshInfo != NULL)
  {
     MM_Free(m_pPsshInfo);
     m_pPsshInfo = NULL;
  }
  if(m_pContentProtectionInfo != NULL)
  {
     MM_Free(m_pContentProtectionInfo);
     m_pContentProtectionInfo = NULL;
  }
}


PsshStatus
DashCryptoInterface::GetPSSHInfo
(
  uint32 portIndex,
  int uniqueId
)
{
  PsshStatus ret = INIT_PRESENT;
  OMX_HANDLETYPE handle = GetMMIHandle();
  QOMX_PARAM_STREAMING_PSSHINFO psshInfo;
  QOMX_STRUCT_INIT(psshInfo,QOMX_PARAM_STREAMING_PSSHINFO);
  psshInfo.nUniqueID = uniqueId;
  psshInfo.nPortIndex = portIndex;
  OMX_INDEXTYPE psshIndex;
  int err = OMXPrefetch((OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO,&psshInfo, &psshIndex);

  //setPSSHDataSizeOnPort(nPortIndex, psshInfo.nPsshDataBufSize);

  if (err == -1)
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                 "port %d INIT_ERROR_UNKNOW", (int)portIndex);
    ret = INIT_ERROR_UNKNOWN;
  }
  else if(err == 0 && psshInfo.nPsshDataBufSize == 0)
  {
    QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "port %d INIT_ERROR_UNKNOW", (int)portIndex);
    ret = INIT_NOT_PRESENT;

  }
  if (ret == INIT_PRESENT)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "port %d INIT_PRESENT: PSSHDataSize %lu", (int)portIndex,
              psshInfo.nPsshDataBufSize);

    if (m_pPsshInfo)
    {
      MM_Free(m_pPsshInfo);
      m_pPsshInfo = NULL;
    }

    int psshInfoAllocSize = 4 * sizeof (OMX_U32) +
             sizeof(OMX_VERSIONTYPE) +
             sizeof(OMX_U8) * sizeof(psshInfo.cDefaultKeyID) +
             sizeof(OMX_U8) * psshInfo.nPsshDataBufSize;
    m_pPsshInfo = (QOMX_PARAM_STREAMING_PSSHINFO *)
                  MM_Malloc(sizeof(OMX_U8) * psshInfoAllocSize);
    if (m_pPsshInfo)
    {
      memset(m_pPsshInfo, 0, psshInfoAllocSize);
      m_pPsshInfo->nSize = psshInfoAllocSize;
      m_pPsshInfo->nPortIndex = psshInfo.nPortIndex;
      m_pPsshInfo->nUniqueID =  psshInfo.nUniqueID;
      m_pPsshInfo->nPsshDataBufSize = psshInfo.nPsshDataBufSize;
      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "Querying using nUniqueID [%ld] nPsshDataBufSize [%lu]",
                   m_pPsshInfo->nUniqueID,
                   m_pPsshInfo->nPsshDataBufSize);

      MMI_OmxParamCmdType cmd;
      cmd.nParamIndex = psshIndex;
      cmd.pParamStruct = m_pPsshInfo;
      uint32 mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);
      if(mmiret != MMI_S_COMPLETE)
      {
          QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                  "INIT_ERROR_UNKNOWN:Get PSSH Data failed %x for port %d",
                  (unsigned int)mmiret, (int)portIndex);
          MM_Free(m_pPsshInfo);
          ret = INIT_ERROR_UNKNOWN;
      }
      //SetLastUniqueID(psshInfoPtr->nPortIndex, psshInfoPtr->nUniqueID);
      //PrintPSSHInfo(psshInfoPtr);
    }
    else
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "INIT_ERROR_UNKNOWN: memory allocation failed,");
     ret = INIT_ERROR_UNKNOWN;
    }
  }
  return ret;
}


ContentProtectionStatus
DashCryptoInterface::GetContentProtectionInfo
(
  uint32 portIndex,
  int &drmType,
  int &steamType
)
{
  //Get Content Protection Info
  ContentProtectionStatus ret = CP_PRESENT;
  QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO cpInfo;
  int cpInfoAllocSize = 0;
  memset(&cpInfo, 0, sizeof(cpInfo));
  cpInfo.nPortIndex = portIndex;
  cpInfo.eDrmType = QOMX_NO_DRM;
  cpInfo.nStreamType = INVALID_STREAM;
  cpInfo.nSize = sizeof(cpInfo);
  OMX_INDEXTYPE cpIndex = OMX_IndexComponentStartUnused;
  int err =
    OMXPrefetch((OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_CONTENTPROTECTION_INFO,
                     &cpInfo, &cpIndex);
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "DRM type [%d] size [%lu]",
              cpInfo.eDrmType, cpInfo.nContentProtectionInfoSize );

  if(err == -1)
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
                 "CP_ERROR_UNKNOWN");
    ret = CP_ERROR_UNKNOWN;
  }
  else if (err == 0 && cpInfo.nContentProtectionInfoSize == 0 )
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                 "CP_NOT_PRESENT");
    ret = CP_NOT_PRESENT;
  }
  else if(cpInfo.nContentProtectionInfoSize == 0 &&
          (cpInfo.eDrmType != QOMX_MARLIN_DRM ||
           cpInfo.eDrmType != QOMX_PLAYREADY_DRM))
  {
    QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                 "CP_DRM_NOT_SUPPORT");
    ret = CP_DRM_NOT_SUPPORT;
  }

  if (ret == CP_PRESENT)
  {
    drmType =  (int)cpInfo.eDrmType;
    if(m_pContentProtectionInfo != NULL)
    {
      MM_Free(m_pContentProtectionInfo);
      m_pContentProtectionInfo = NULL;
    }

    OMX_HANDLETYPE handle = GetMMIHandle();
    cpInfoAllocSize = sizeof (QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO)*
                                             cpInfo.nContentProtectionInfoSize;
    m_pContentProtectionInfo =
        (QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO*)MM_Malloc(cpInfoAllocSize);

    if (m_pContentProtectionInfo)
    {
      m_pContentProtectionInfo->nSize = cpInfoAllocSize;
      m_pContentProtectionInfo->nPortIndex = cpInfo.nPortIndex;
      m_pContentProtectionInfo->eDrmType = cpInfo.eDrmType;
      m_pContentProtectionInfo->nContentProtectionInfoSize =
                                           cpInfo.nContentProtectionInfoSize;

      MMI_OmxParamCmdType cmd;
      cmd.nParamIndex = cpIndex;
      cmd.pParamStruct = m_pContentProtectionInfo;
      uint32 mmiret =
                  HTTPMMIDeviceCommand(handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);
      if(mmiret != MMI_S_COMPLETE)
      {
         QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
             "CP_ERROR_UNKNOW: GetContentProtectionInfo failed %x for port %d",
             (unsigned int)mmiret, (int)portIndex);
        ret = CP_ERROR_UNKNOWN;
      }
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
           "CP_PRESENT: Element is '%s'",
           m_pContentProtectionInfo->cContentProtectionData);
    }
    else
    {
      QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
        "CP_ERROR_UNKNOW: memory allocation failed, ");
      ret = CP_ERROR_UNKNOWN;
    }
  }
  return ret;
}

void *DashCryptoInterface::GetStoredPSSHInfo()
{
  return (void*)m_pPsshInfo;
}

void *DashCryptoInterface::GetStoredContentProtectionInfo()
{
  return (void*)m_pContentProtectionInfo;
}

void
DashCryptoInterface::ParseExtraData
(
  MMI_BufferCmdType *pMMICmdBuf,
  QOMX_PARAM_STREAMING_PSSHINFO *psshInfo,
  QOMX_EXTRA_SAMPLE_INFO *extraSampleInfo
)
{
  //MMI_BufferCmdType *pMMICmdBuf = (MMI_BufferCmdType *)pCmdBuf;

  if(pMMICmdBuf)
  {
    OMX_BUFFERHEADERTYPE* pBufHdr = (OMX_BUFFERHEADERTYPE*)pMMICmdBuf->pBufferHdr;

    OMX_OTHER_EXTRADATATYPE *pExtraData = NULL;
    uint32 ulAddr = (uint32)( pBufHdr->pBuffer) +  pBufHdr->nFilledLen;
    // Aligned address to DWORD boundary
    ulAddr = (ulAddr + 0x3) & (~0x3);
    pExtraData = (OMX_OTHER_EXTRADATATYPE *)ulAddr;

    // Traverse the list of extra data sections
    while(pExtraData->eType != OMX_ExtraDataNone)
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Processing pExtraData->eType(0x%x)",
              (OMX_EXTRADATATYPE)pExtraData->eType);
      if(pExtraData->eType == (OMX_EXTRADATATYPE)mIndexDrmParamPsshInfo)
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING,
                      QTVDIAG_PRIO_HIGH, "Found PSSH");
        psshInfo = (QOMX_PARAM_STREAMING_PSSHINFO *)&pExtraData->data;
        //PrintPSSHInfo(psshInfo);
      }
      if(pExtraData->eType == (OMX_EXTRADATATYPE)mIndexDrmExtraSampleInfo)
      {
        QTV_MSG_PRIO(MSG_SSID_APPS_QTV_STREAMING,
                      QTVDIAG_PRIO_HIGH, "Found ExtraSample");
        extraSampleInfo = ( QOMX_EXTRA_SAMPLE_INFO *)&pExtraData->data;
        //PrintExtraSampleInfo(extraSampleInfo);
      }
      ulAddr = ulAddr + pExtraData->nSize;
      ulAddr = (ulAddr + 0x3) & (~0x3);
      pExtraData = (OMX_OTHER_EXTRADATATYPE*)ulAddr;
    }
  }
  return;
}

void
DashCryptoInterface::GetIndexForExtensions
(
/*
  OMX_STRING param,
  OMX_INDEXTYPE *omxIndex
*/
)
{
  /*
  OMX_HANDLETYPE handle = GetMMIHandle();
  MMI_GetExtensionCmdType ext;
  ext.cParamName = param;
  ext.pIndex = omxIndex;
  uint32 mmiret = MMI_S_EFAIL;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
  if(mmiret != MMI_S_COMPLETE)
  {
      QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_FATAL,
          "Error: Get Extension Index for '%s' is %lu",
           param, (long unsigned int)ext.pIndex);
      return;
  }
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
         "Success: Get Extension Index for '%s' is 0x%x", param, (uint32_t)omxIndex);
  return;
  */

  OMX_HANDLETYPE handle = GetMMIHandle();
  MMI_GetExtensionCmdType ext;

  mIndexDrmParamPsshInfo = OMX_IndexComponentStartUnused;
  ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO;
  ext.pIndex = &mIndexDrmParamPsshInfo;
  uint32 mmiret = MMI_S_EFAIL;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
  if(mmiret != MMI_S_COMPLETE)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
            "Error: Get Extension Index for '%s' is %lu", ext.cParamName,
            (long unsigned int)ext.pIndex);
  }
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Success: Get Extension Index for '%s' is  0x%x", ext.cParamName,
               mIndexDrmParamPsshInfo);

  mIndexDrmExtraSampleInfo = OMX_IndexComponentStartUnused;
  ext.cParamName = (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_EXTRASAMPLE_INFO;
  ext.pIndex = &mIndexDrmExtraSampleInfo;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
  if(mmiret != MMI_S_COMPLETE)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
             "Error: Get Extension Index for '%s' is %lu",
             ext.cParamName, (long unsigned int)ext.pIndex);
  }
  QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Success: Get Extension Index for '%s' is  0x%x",
              ext.cParamName, mIndexDrmExtraSampleInfo);
  return;
}



int
DashCryptoInterface::OMXPrefetch
(
  OMX_STRING param,
  void* omxStruct,
  OMX_INDEXTYPE *omxIndex
)
{
  int ret =  0;
  /*
  GetIndexForExtensions(param, omxIndex);
  OMX_HANDLETYPE handle = GetMMIHandle();
  uint32 mmiret = MMI_S_EFAIL;
  MMI_OmxParamCmdType cmd;
  cmd.nParamIndex = *omxIndex;
  cmd.pParamStruct = omxStruct;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);
  if(mmiret != MMI_S_COMPLETE)
  {
     QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
              "Error: Get Size for %s failed %x", param, (uint32_t)mmiret);
     return -1;
  }
  QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
            "Success: Get Size for %s: Success", param);
  return 0;
  */
  OMX_HANDLETYPE handle = GetMMIHandle();
  MMI_GetExtensionCmdType ext;
  ext.cParamName = param;
  ext.pIndex = omxIndex;
  uint32 mmiret = MMI_S_EFAIL;
  mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_EXTENSION_INDEX, &ext);
  if(mmiret != MMI_S_COMPLETE)
  {
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Error:Get Extension Index for '%s' failed %lu",
              param, (unsigned long)ext.pIndex);
    ret = -1;
  }
  else
  {
    omxIndex = ext.pIndex;
    QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
              "Success:Get Extension Index for '%s' is  %x", param, *omxIndex);

    MMI_OmxParamCmdType cmd;
    cmd.nParamIndex = *omxIndex;
    cmd.pParamStruct = omxStruct;
    mmiret = HTTPMMIDeviceCommand(handle, MMI_CMD_GET_STD_OMX_PARAM, &cmd);
    if(mmiret != MMI_S_COMPLETE)
    {
       QTV_MSG_PRIO2(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_ERROR,
                "Error :Get Size for '%s' failed %x", param, (uint32_t)mmiret);
       ret = -1;
    }
    else
    {
      QTV_MSG_PRIO1(MSG_SSID_APPS_QTV_STREAMING, QTVDIAG_PRIO_HIGH,
                "Success: Get Size for '%s'", param);
    }
  }
  return ret;
}



