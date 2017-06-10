/************************************************************************* */
/**
 * HTTPSourceMMIExtensionHandler.cpp
 * @brief Implements the HTTPSource MMI Extensions
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIExtensionHandler.cpp#37 $
$DateTime: 2013/07/30 01:48:05 $
$Change: 4184523 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stddef.h>
#include "HTTPSourceMMIExtensionHandler.h"
#include <OMX_CoreExt.h>
#include <OMX_IndexExt.h>
#include "HTTPSourceMMI.h"
#include "QOMX_IVCommonExtensions.h"
#include "StreamSourceTimeUtils.h"
#include "HTTPSourceMMIHelper.h"
#include "HTTPCommon.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
namespace video {

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
//Extension string <-> index mapping
const HTTPSourceMMIExtensionHandler::HTTPSourceMMIExtnMap
HTTPSourceMMIExtensionHandler::m_HttpStreamingExtnMap[] = {
  { (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_WATERMARK,
    QOMX_HTTP_IndexStreamingConfigWaterMark },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_WATERMARKSTATUS,
    QOMX_HTTP_IndexStreamingConfigWaterMarkStatus },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_STREAMING_NETWORKINTERFACE,
    QOMX_HTTP_IndexParamStreamingNetworkInterface },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_STREAMING_NETWORKPROFILE,
    QOMX_HTTP_IndexParamStreamingNetworkProfile },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_STREAMING_PROXYSERVER,
    QOMX_HTTP_IndexParamStreamingProxyServer },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_STREAMING_PROTOCOLEVENT,
    QOMX_HTTP_IndexConfigStreamingProtocolEvent },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_STREAMING_PROTOCOLHEADER,
    QOMX_HTTP_IndexConfigStreamingProtocolHeader },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_SYNTAXHDR,
    QOMX_HTTP_IndexParamStreamingSyntaxHeader },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_DIVX,
    QOMX_HTTP_IndexParamVideoDivx },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_VP,
    QOMX_HTTP_IndexParamVideoVp6 },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_SPARK,
    QOMX_HTTP_IndexParamVideoSpark },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_VIDEO_VC1,
    QOMX_HTTP_IndexParamVideoVc1 },
#ifdef FEATURE_HTTP_EVRC
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_EVRCB,
    QOMX_HTTP_IndexParamAudioEvrcb },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_EVRCWB,
    QOMX_HTTP_IndexParamAudioEvrcWb },
#endif
#ifdef FEATURE_HTTP_WM
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_WMA,
    QOMX_HTTP_IndexParamAudioWmaPro },
#endif
  { (OMX_STRING)OMX_QCOM_INDEX_CONFIG_MEDIAINFO,
    QOMX_HTTP_IndexConfigMediaInfo },
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_SEEK_ACCESS,
    QOMX_HTTP_IndexParamSeekAccess },
#ifdef FEATURE_HTTP_AMR
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_AMRWBPLUS,
    QOMX_HTTP_IndexParamAmrWbPlus },
#endif // FEATURE_HTTP_AMR
  { (OMX_STRING)OMX_QCOM_INDEX_PARAM_FRAMESIZEQUERYSUPPORTED,
    QOMX_HTTP_IndexParamFrameSizeQuerySupported },
  { (OMX_STRING)OMX_QCOM_INDEX_CONFIG_STREAMING_DOWNLOADPROGRESSUNITSTYPE,
    QOMX_HTTP_IndexConfigStreamingDownloadProgressUnitsType },
  { (OMX_STRING)OMX_QOMX_INDEX_CONFIG_STREAMING_DOWNLOADPROGRESS,
    QOMX_HTTP_IndexConfigStreamingDownloadProgress },
  { (OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_STREAMING_PROTOCOLHEADERSEVENT,
    QOMX_HTTP_IndexConfigStreamingProtocolHeadersEvent},
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_COMPLETE_DASH_ADAPTATION_PROPERTIES,
     QOMX_HTTP_IndexParamCompleteDashAdaptationProperties},
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_SELECTED_DASH_ADAPTATION_PROPERTIES,
     QOMX_HTTP_IndexParamSelectedDashAdaptationProperties},
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS,
     QOMX_HTTP_IndexParamSMPTETimeTextDimensionExtraData},
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_SUBINFO,
     QOMX_HTTP_IndexParamSMPTETimeTextSubInfoExtraData },
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_DRM_INFO,
    QOMX_HTTP_IndexParamDrmInfo},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_CONTENTPROTECTION_INFO,
    QOMX_HTTP_IndexParamContentProtectionInfo},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_PSSH_INFO,
    QOMX_HTTP_IndexParamPsshInfo},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_EXTRASAMPLE_INFO,
    QOMX_HTTP_IndexParamExtraSampleInfo},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_DASH_MPD,
    QOMX_HTTP_IndexParamDashMPD},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_QOE,
    QOMX_HTTP_IndexParamQOE},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_QOE_PLAY,
    QOMX_HTTP_IndexParamQOEPlay},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_QOE_STOP,
    QOMX_HTTP_IndexParamQOEStop},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_QOE_SWITCH,
    QOMX_HTTP_IndexParamQOESwitch},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_QOE_PERIODIC,
    QOMX_HTTP_IndexParamQOEPeriodic},
{(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_CURRENT_REPRESENTATION,
    QOMX_HTTP_IndexParamCurrentRepresenation},
{(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_COOKIES,
    QOMX_HTTP_IndexParamCookie},
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_AC3,
    QOMX_HTTP_IndexParamAudioAc3},
  { (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_MP2,
    QOMX_HTTP_IndexParamAudioMp2},
{ (OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_BUFFERED_DURATION,
    QOMX_HTTP_IndexParamBufferedDuration},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_PARAM_MEDIA_TRACK_ENCODING,
    QOMX_HTTP_IndexParamMediaTrackEncoding},
{(OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_DASH_RESUME_DISCONTINUITY,
      QOMX_HTTP_IndexConfigDASHResumeDiscontinuity},
  {(OMX_STRING)OMX_QUALCOMM_INDEX_CONFIG_DASH_REPOSITION_RANGE,
      QOMX_HTTP_IndexConfigDASHRepositionRange}
};

//Supported HTTP method name <-> enum mapping for header configuration
const HTTPSourceMMIExtensionHandler::AffectedHTTPMethodPair
HTTPSourceMMIExtensionHandler::m_supportedHTTPMethodTable[] =
{
  {"GET",       HTTP_METHOD_GET},
  {"HEAD",      HTTP_METHOD_HEAD},
  {"POST",      HTTP_METHOD_POST},
  {"",           HTTP_METHOD_ALL}
};

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/**
 * Constructor
 */
HTTPSourceMMIExtensionHandler::HTTPSourceMMIExtensionHandler()
{
  m_pHTTPSourceMMI = NULL;
}

/**
 * Dtor
 */
HTTPSourceMMIExtensionHandler::~HTTPSourceMMIExtensionHandler()
{
}

/*
 * Initializes the Extensions handler, preceeded before any other call.
 *
 * @param[in] pRTPMMIControl - Reference to CRTPMMIControl
 *
 * @return MMI_S_COMPLETE on success, MMI_S_EFAIL on failure
 */
bool HTTPSourceMMIExtensionHandler::Initialize(HTTPSourceMMI *pHTTPSourceMMI)
{
  bool ret = true;
  m_pHTTPSourceMMI = pHTTPSourceMMI;

  for(int i= MMI_HTTP_PORT_START_INDEX;
        i <= MMI_HTTP_NUM_MAX_PORTS ; i++)
  {
    m_eDownloadProgressUnits[i] = HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_DATA;
  }

  ret = m_eventHandler.m_HTTPProtocolEventMgr.Initialize();
  ret = m_eventHandler.m_HTTPProtocolHeadersEventMgr.Initialize();

  for(int i= MMI_HTTP_PORT_START_INDEX;
       i <= MMI_HTTP_NUM_MAX_PORTS && ret; i++)
  {
    ret = m_eventHandler.m_HTTPBufferingEventMgr[i].Initialize();
    m_eventHandler.m_HTTPBufferingEventMgr[i].Reset();
  }
  if(ret)
  {
    m_eventHandler.m_HTTPProtocolEventMgr.Reset();
  }
  return ret;
}

void HTTPSourceMMIExtensionHandler::Reset()
{
  m_eventHandler.m_HTTPProtocolEventMgr.Reset();
  m_eventHandler.m_HTTPProtocolHeadersEventMgr.Reset();

  for(int i=0; i<=MMI_HTTP_NUM_MAX_PORTS ; i++)
  {
    m_eventHandler.m_HTTPBufferingEventMgr[i].Reset();
  }
}

/*
 * @breif  Translates a vendor specific OEM extension string to
 *       OMX IL vendor extension Index
 *
 * @Param[in]  Pointer to MMI_GetExtensionCmdType
 *
 * @Return  MMI_S_COMPLETE on success or MMI_S_ENOTIMPL
 *        otherwise
 */
uint32 HTTPSourceMMIExtensionHandler::ProcessMMIGetExtensionCommand(
                            MMI_GetExtensionCmdType *pMMIGetExtnCmd)
{
  uint32 rsltCode = MMI_S_ENOTIMPL;
  for (unsigned int index = 0;
       index < sizeof(m_HttpStreamingExtnMap)/sizeof(HTTPSourceMMIExtnMap);
       index++)
  {
    if (std_stricmp(pMMIGetExtnCmd->cParamName,
                    m_HttpStreamingExtnMap[index].pExtensionName) == 0)
    {
      *(pMMIGetExtnCmd->pIndex) = (OMX_INDEXTYPE)m_HttpStreamingExtnMap[index].nExtensionIndex;
      rsltCode = MMI_S_COMPLETE;
      break;
    }
  }
  return rsltCode;
}

/*
 * @breif Processes the Get request for vendor Specific OEM extensions
 *
 * @Param[in] Reference object to MMI_OmxParamCmdType
 *
 * @return  MMI_S_COMPLETE on success or MMI_S_ENOTIMPL otherwise
 */
uint32 HTTPSourceMMIExtensionHandler::ProcessMMIGetStdExtnParam(
                                 MMI_OmxParamCmdType* pOmxParam)
{
  HTTPSourceMMI::HttpSourceMmiPortInfo* pPortInfo = NULL;
  HTTPController* pHTTPController = NULL;
  uint32 ret = MMI_S_EBADPARAM;
  uint32 size = 0;

  if ((NULL == m_pHTTPSourceMMI) || (NULL == pOmxParam))
  {
    return ret;
  }

  pHTTPController = m_pHTTPSourceMMI->m_pHTTPController;

  OMX_INDEXTYPE nParamIndex = pOmxParam->nParamIndex;
  if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingNetworkInterface)
  {
    //Validate pOmxParam->pParamStruct
    QOMX_PARAM_STREAMING_NETWORKINTERFACE* pNetIface =
    (QOMX_PARAM_STREAMING_NETWORKINTERFACE *)pOmxParam->pParamStruct;
    if (pNetIface && pNetIface->nSize == sizeof(QOMX_PARAM_STREAMING_NETWORKINTERFACE))
    {
      if (pHTTPController)
      {
        int32 networkIfaceId = -1;
        if (pHTTPController->GetNetworkInterface(networkIfaceId))
        {
          QOMX_STREAMING_NETWORKINTERFACETYPE eNetIface =
          (QOMX_STREAMING_NETWORKINTERFACETYPE)networkIfaceId;
          if (eNetIface <= QOMX_STREAMING_NETWORKINTERFACE_CMMB_IFACE)
          {
            ret = MMI_S_COMPLETE;
            pNetIface->eNetworkInterface = eNetIface;
          }
          else
          {
            ret = MMI_S_EBADPARAM;
          }
        }
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pNetIface );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingNetworkProfile)
  {
    //Validate pOmxParam->pParamStruct
    OMX_PARAM_U32TYPE* pValue = (OMX_PARAM_U32TYPE *)pOmxParam->pParamStruct;
    if (pValue && pValue->nSize == sizeof(OMX_PARAM_U32TYPE))
    {
      uint32 networkProfileId;
      if (pHTTPController && pHTTPController->GetNetworkProfile(networkProfileId))
      {
        ret = MMI_S_COMPLETE;
        pValue->nU32 = (OMX_U32)networkProfileId;
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pValue );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingProxyServer)
  {
    //Validate pOmxParam->pParamStruct
    OMX_PARAM_CONTENTURITYPE* pURI =
    (OMX_PARAM_CONTENTURITYPE *)pOmxParam->pParamStruct;
    if (pURI)
    {
      int uriSize = (int)(pURI->nSize - offsetof(OMX_PARAM_CONTENTURITYPE, contentURI[0]));
      if (uriSize >= 0)
      {
        size_t proxyURILenReq = 0;
        ret = MMI_S_EBADPARAM;
        if (pHTTPController)
        {
          // Handle  bad input param return value(0) from the HTTP Source
          int32 nReturn = pHTTPController->GetProxyServer((char*)pURI->contentURI,uriSize,proxyURILenReq);

          if(nReturn > 0)
          {
            ret = MMI_S_COMPLETE;
          }
          else if(nReturn < 0)
          {
            ret = MMI_S_EBADPARAM;
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "HTTPMMIExtensionHandler Proxy server not present");
          }
          else
          {
            pURI->nSize = (OMX_U32)proxyURILenReq;
            QTV_MSG_PRIO2( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "HTTPMMIExtensionHandler Insufficient uriSize %d (required %u)", uriSize, (uint32)pURI->nSize);
            ret = MMI_S_COMPLETE;
          }
        }
      }
      else
      {
        ret = MMI_S_EBADPARAM;
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "HTTPMMIExtensionHandler Insufficient uriSize %d ", uriSize);
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pURI );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigStreamingProtocolHeader)
  {
    //Validate pOmxParam->pParamStruct
    QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE* pHeader =
      (QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE *)pOmxParam->pParamStruct;
    if (pHeader)
    {
      //Get the protocol header
      ret = GetProtocolHeader(*pHeader);
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pHeader );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingSyntaxHeader)
  {
    //Validate pOmxParam->pParamStruct
    QOMX_VIDEO_SYNTAXHDRTYPE* pHeader =
        (QOMX_VIDEO_SYNTAXHDRTYPE *)pOmxParam->pParamStruct;
    if (pHeader)
    {
      //Get the video syntax header
      ret = GetSyntaxHeader(*pHeader);
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p",(void *)pHeader );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigStreamingProtocolEvent)
  {
    //Validate pOmxParam->pParamStruct
    QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE* pEvent =
    (QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE *)pOmxParam->pParamStruct;
    if (pEvent)
    {
      //Get the protocol error
      ret = GetHTTPProtocolEvent(*pEvent);
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pEvent );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamContentProtectionInfo)
  {
    //Validate pOmxParam->pParamStruct
    ret = MMI_S_EBADPARAM;
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "Query for  QOMX_HTTP_IndexParamContentProtectionInfo");

    QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO* pContentProtectionInfo =
    (QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO *)pOmxParam->pParamStruct;
    if (pContentProtectionInfo == NULL)
    {

      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pContentProtectionInfo );
    }
    else
    {
      if (pContentProtectionInfo->nSize < sizeof(QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO))
      {
        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  " Invalid nSize %u, minSize %d",
                  (uint32)pContentProtectionInfo->nSize, sizeof(QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO));
      }
      else
      {
        ret = MMI_S_EFAIL;
        HTTPMediaType mediaType;
        m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->MapPortIDToMediaType(
                                  pContentProtectionInfo->nPortIndex,mediaType);
        HTTPDrmType drmType = (HTTPDrmType)pContentProtectionInfo->eDrmType;
        HTTPContentStreamType streamType =
                    (HTTPContentStreamType)pContentProtectionInfo->nStreamType;
        HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
        if (pContentProtectionInfo->nContentProtectionInfoSize == 0)
        {
          QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "Query for QOMX_HTTP_IndexParamContentProtectionInfo : size");
           eStatus =
                    m_pHTTPSourceMMI->m_pHTTPDataInterface->GetContentProtectElem(
                          pContentProtectionInfo->nPortIndex,
                          mediaType,
                          drmType,
                          streamType,
                          (uint32&)pContentProtectionInfo->nContentProtectionInfoSize,
                          NULL);
        if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
        {
             if (pContentProtectionInfo->nContentProtectionInfoSize > 0)
          {
               pContentProtectionInfo->eDrmType = (QOMX_DRM_TYPE)drmType;
               pContentProtectionInfo->nStreamType = (ContentStreamType)streamType;
               pContentProtectionInfo->nContentProtectionInfoSize += 1;
             }
             else
             {
               QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                              "Retreived contentProtection Size is zero");
             }
             ret = MMI_S_COMPLETE;
           }
         }
        else if (pContentProtectionInfo->nSize >=
                (offsetof(QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO, cContentProtectionData[0])) +
                      pContentProtectionInfo->nContentProtectionInfoSize)
            {
                QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                "Query for QOMX_HTTP_IndexParamContentProtectionInfo : data");
                eStatus = m_pHTTPSourceMMI->m_pHTTPDataInterface->GetContentProtectElem(
                    pContentProtectionInfo->nPortIndex,
                    mediaType,
                          drmType,
                    streamType,
                          (uint32&)pContentProtectionInfo->nContentProtectionInfoSize,
                          pContentProtectionInfo->cContentProtectionData);
          if (HTTPCommon::HTTPDL_SUCCESS == eStatus)
          {
          pContentProtectionInfo->eDrmType = (QOMX_DRM_TYPE)drmType;
             pContentProtectionInfo->nStreamType = (ContentStreamType)streamType;
             ret = MMI_S_COMPLETE;
        }
        }
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamPsshInfo)
  {
    //Validate pOmxParam->pParamStruct
    ret = MMI_S_EBADPARAM;
    QOMX_PARAM_STREAMING_PSSHINFO* pPSSHInfo =
    (QOMX_PARAM_STREAMING_PSSHINFO *)pOmxParam->pParamStruct;
    if (pPSSHInfo == NULL)
    {

      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pPSSHInfo );
    }
    else if (pPSSHInfo->nPortIndex == OMX_ALL ||
            (pPSSHInfo->nPortIndex >= MMI_HTTP_PORT_START_INDEX  &&
              pPSSHInfo->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX ))
    {
      if (pPSSHInfo->nSize < sizeof(QOMX_PARAM_STREAMING_PSSHINFO))
      {
        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  " Invalid nSize %u, minSize %d",
                 (uint32) pPSSHInfo->nSize, sizeof(QOMX_PARAM_STREAMING_DRMINFO));
      }
      else
      {
        if (pPSSHInfo->nPortIndex == OMX_ALL)
        {
          for(OMX_U32 portIndex = MMI_HTTP_AUDIO_PORT_INDEX;
                portIndex <= MMI_HTTP_VIDEO_PORT_INDEX;portIndex++)
          {
            ret = GetPsshInfo(portIndex , pPSSHInfo);
          }
        }
        else
        {
          ret = GetPsshInfo(pPSSHInfo->nPortIndex, pPSSHInfo );
        }
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Invalid portIndex: %u",(uint32)pPSSHInfo->nPortIndex);
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigStreamingProtocolHeadersEvent)
  {
    //Validate pOmxParam->pParamStruct
    QOMX_STREAMING_PROTOCOLHEADERSTYPE* pEvent =
      (QOMX_STREAMING_PROTOCOLHEADERSTYPE *)pOmxParam->pParamStruct;
    if (pEvent)
    {
      //Get the protocol headers event
      ret = GetHTTPProtocolHeadersEvent(*pEvent);
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p"
                      , (void *)pEvent );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexStreamingConfigWaterMark)
  {
    QOMX_BUFFERINGWATERMARKTYPE *pBufferingWaterMark =
                    (QOMX_BUFFERINGWATERMARKTYPE *)pOmxParam->pParamStruct;

    if(pBufferingWaterMark &&
          pBufferingWaterMark->nSize == sizeof(QOMX_BUFFERINGWATERMARKTYPE))
    {
      if(pBufferingWaterMark->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
          pBufferingWaterMark->nPortIndex <= MMI_HTTP_VIDEO_PORT_INDEX )
      {

        if(pBufferingWaterMark->eWaterMark == QOMX_WATERMARK_NORMAL)
        {
          if(pHTTPController)
          {
            pBufferingWaterMark->nLevel = pHTTPController->GetInitialPreroll();
            ret = MMI_S_COMPLETE;
          }
        }
        else if(pBufferingWaterMark->eWaterMark == QOMX_WATERMARK_UNDERRUN)
        {
          //return zero LWM
          pBufferingWaterMark->nLevel = 0;
          ret = MMI_S_COMPLETE;
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                "HTTPMMIExtensionHandler Invalid Buffering WaterMark Type %d",
                        pBufferingWaterMark->eWaterMark );
          ret = MMI_S_EBADPARAM;
        }

        //handle Enabling buffering notification for port(s)
        if(ret == MMI_S_COMPLETE)
        {
            bool enable= m_eventHandler.m_HTTPBufferingEventMgr[pBufferingWaterMark->nPortIndex].
                                          GetBufferingNotification(pBufferingWaterMark->eWaterMark);
          pBufferingWaterMark->bEnable = (enable == true)?OMX_TRUE:OMX_FALSE;
          pBufferingWaterMark->nLevel *= 1000;
          pBufferingWaterMark->eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;
        }
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexStreamingConfigWaterMarkStatus)
  {
    QOMX_BUFFERINGSTATUSTYPE *pBufferStatus =
            (QOMX_BUFFERINGSTATUSTYPE*)pOmxParam->pParamStruct;

    if(pBufferStatus->nSize == sizeof(QOMX_BUFFERINGSTATUSTYPE) &&
      ((pBufferStatus->nPortIndex == OMX_ALL) ||
        ((pBufferStatus->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX) &&
       (pBufferStatus->nPortIndex <=  MMI_HTTP_VIDEO_PORT_INDEX))))
    {
      if (pBufferStatus->nPortIndex == OMX_ALL)
      {
        uint32 currentBufferLevel = MAX_UINT32_VAL;
        ret = GetCurrentWatermarkStatus(MMI_HTTP_AUDIO_PORT_INDEX, pBufferStatus);
        if(ret)
        {
          currentBufferLevel = STD_MIN(currentBufferLevel, pBufferStatus->nCurrentLevel);
          ret = GetCurrentWatermarkStatus(MMI_HTTP_VIDEO_PORT_INDEX, pBufferStatus);
          if (ret)
          {
            pBufferStatus->nCurrentLevel = STD_MIN(currentBufferLevel, pBufferStatus->nCurrentLevel);
          }
        }
      }
      else
    {
      ret = GetCurrentWatermarkStatus(pBufferStatus->nPortIndex, pBufferStatus);
    }
  }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)OMX_IndexConfigCallbackRequest)
  {
    //Validate pOmxParam->pParamStruct and the request could be for a
    //specific port or all ports (OMX_ALL)
    OMX_CONFIG_CALLBACKREQUESTTYPE* pCallback =
    (OMX_CONFIG_CALLBACKREQUESTTYPE *)pOmxParam->pParamStruct;
    if (pCallback && pCallback->nSize == sizeof(OMX_CONFIG_CALLBACKREQUESTTYPE))
    {
      if ((int)pCallback->nIndex == (int)QOMX_HTTP_IndexConfigStreamingProtocolEvent)
      {
        //Make sure the request is for all ports (OMX_ALL) and set the event
        //notification flag
        if (pCallback->nPortIndex == OMX_ALL)
        {
          //enable Protocol error Event notification flag
          pCallback->bEnable =
          m_eventHandler.m_HTTPProtocolEventMgr.GetNotify()? OMX_TRUE : OMX_FALSE;
          ret = MMI_S_COMPLETE;
        }
        else
        {
          ret = MMI_S_EBADPARAM;
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
          "HTTPMMIExtensionHandler Invalid port index %u",
          (uint32)pCallback->nPortIndex );
        }
      }
      else
      {
        ret = MMI_S_ENOTIMPL;
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "HTTPMMIExtensionHandler Unsupported callback extension index %d",
        pCallback->nIndex );

      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pCallback );
    }
  }
#ifdef FEATURE_HTTP_WM
  else if(nParamIndex == OMX_IndexParamAudioWma || nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamAudioWmaPro)
  {
    size = (nParamIndex == OMX_IndexParamAudioWma) ? sizeof(OMX_AUDIO_PARAM_WMATYPE) : sizeof(QOMX_AUDIO_PARAM_WMATYPE);

    QOMX_AUDIO_PARAM_WMATYPE* pProFormat = NULL;
    pProFormat = (QOMX_AUDIO_PARAM_WMATYPE*)pOmxParam->pParamStruct;
    if (!pProFormat || !m_pHTTPSourceMMI->IsValidPort(pProFormat->nPortIndex, OMX_PortDomainAudio))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "OMX_IndexParamAudioWma: pWmaFormat/port index is invalid" );
    }
    else
    {
      pPortInfo = &m_pHTTPSourceMMI->m_portAudio[pProFormat->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      QOMX_STRUCT_INIT(*pProFormat, QOMX_AUDIO_PARAM_WMATYPE);
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pProFormat, pPortInfo->m_pPortSpecificData, size);
      }
      ret = MMI_S_COMPLETE;
    }
  }
#endif /*FEATURE_HTTP_WM*/

  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigMediaInfo)
  {
    QOMX_MEDIAINFOTYPE *pMediaInfo =
    (QOMX_MEDIAINFOTYPE*)pOmxParam->pParamStruct;

    if (pMediaInfo == NULL)
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pMediaInfo );
    }
    else
    {
      ret = HandleMediaInfoQuery(pMediaInfo);
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamSeekAccess)
  {
    QOMX_PARAM_SEEKACCESSTYPE *pseekAccesstype =
    (QOMX_PARAM_SEEKACCESSTYPE*)pOmxParam->pParamStruct;
    if (pseekAccesstype == NULL ||
        (pseekAccesstype->nSize != sizeof(QOMX_PARAM_SEEKACCESSTYPE)))
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pseekAccesstype );
    }
    else
    {
      if (pseekAccesstype->nPortIndex == OMX_ALL ||
        ((pseekAccesstype->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX) &&
          (pseekAccesstype->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX)))
      {
        if (m_pHTTPSourceMMI->IsOpenComplete())
        {
          pseekAccesstype->bSeekAllowed = OMX_TRUE;
          ret = MMI_S_COMPLETE;
        }
        else
        {
          QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "Seek Access Query:open didnt complete" );
          ret = MMI_S_EFAIL;
        }
      }
      else
      {
        ret = MMI_S_EBADPARAM;
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "HTTPMMIExtensionHandler Invalid port index %u",
                        (uint32)pseekAccesstype->nPortIndex );
      }
    }
  }

#ifdef FEATURE_HTTP_AMR
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamAmrWbPlus)
  {
    QOMX_AUDIO_PARAM_AMRWBPLUSTYPE* pAmrWBPlusFormat;
    pAmrWBPlusFormat = (QOMX_AUDIO_PARAM_AMRWBPLUSTYPE*)pOmxParam->pParamStruct;
    if (!pAmrWBPlusFormat || !m_pHTTPSourceMMI->IsValidPort(pAmrWBPlusFormat->nPortIndex, OMX_PortDomainAudio) ||
        (sizeof(QOMX_AUDIO_PARAM_AMRWBPLUSTYPE) != pAmrWBPlusFormat->nSize))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                    "QOMX_HTTP_IndexParamAmrWbPlus: pAmrWBPlusFormat/port index is invalid" );
    }
    else
    {
      pPortInfo = &m_pHTTPSourceMMI->m_portAudio[pAmrWBPlusFormat->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      QOMX_STRUCT_INIT(*pAmrWBPlusFormat, QOMX_AUDIO_PARAM_AMRWBPLUSTYPE);
      if (pPortInfo && pPortInfo->m_pPortSpecificData)
      {
        memcpy(pAmrWBPlusFormat, pPortInfo->m_pPortSpecificData, sizeof(QOMX_AUDIO_PARAM_AMRWBPLUSTYPE));
        ret = MMI_S_COMPLETE;
      }
    }
  }
#endif // FEATURE_HTTP_AMR

  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamFrameSizeQuerySupported)
  {
    QOMX_FRAMESIZETYPE *pFrameSizeType =
      (QOMX_FRAMESIZETYPE*) pOmxParam->pParamStruct;
    if (pFrameSizeType == NULL ||
        (pFrameSizeType->nSize != sizeof(QOMX_FRAMESIZETYPE)))
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pFrameSizeType );
    }
    else
    {
      //If frame size is queried for video port
      if (((pFrameSizeType->nPortIndex >= MMI_HTTP_VIDEO_PORT_INDEX) &&
            (pFrameSizeType->nPortIndex < MMI_HTTP_VIDEO_PORT_INDEX + MMI_HTTP_NUM_VIDEO_PORTS)))
      {
        if(m_pHTTPSourceMMI->IsOpenComplete())
        {
            //If frame size index is 0 return QCIF, for 1 QVGA, for 2 VGA
            // And return no more for frame size index > 2.
            ret = MMI_S_COMPLETE;
            if(pFrameSizeType->nFrameSizeIndex == 0)
            {
              pFrameSizeType->sFrameSize.nWidth = 176;
              pFrameSizeType->sFrameSize.nHeight = 144;
            }
            else if(pFrameSizeType->nFrameSizeIndex == 1)
            {
              pFrameSizeType->sFrameSize.nWidth = 320;
              pFrameSizeType->sFrameSize.nHeight = 240;
            }
            else if(pFrameSizeType->nFrameSizeIndex == 2)
            {
              pFrameSizeType->sFrameSize.nWidth = 640;
              pFrameSizeType->sFrameSize.nHeight = 480;
            }
            else if(pFrameSizeType->nFrameSizeIndex == 3)
            {
              pFrameSizeType->sFrameSize.nWidth = 800;
              pFrameSizeType->sFrameSize.nHeight = 480;
            }
            else if(pFrameSizeType->nFrameSizeIndex > 3)
            {
              ret = MMI_S_ENOMORE;
            }

            QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
              "QOMX_HTTP_IndexParamFrameSizeQuerySupported height %d width %d",
              (int)pFrameSizeType->sFrameSize.nHeight, (int)pFrameSizeType->sFrameSize.nWidth);
          }
        else
        {
          QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "HTTPMMIExtensionHandler FrameSize query before auto detect");
          ret = MMI_S_ENOMORE;
        }
      }
      else
      {
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "HTTPMMIExtensionHandler FrameSize query on a non video port %u",
                         (uint32)pFrameSizeType->nPortIndex);
        ret = MMI_S_EBADPARAM;
      }

      QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_FATAL,
        "QOMX_HTTP_IndexParamFrameSizeQuerySupported port %u, ret %u", (int)pFrameSizeType->nPortIndex, ret);
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigStreamingDownloadProgressUnitsType)
  {
    QOMX_CONFIG_STREAMING_DOWNLOADPROGRESSUNITS* pDownloadUnitsType =
        (QOMX_CONFIG_STREAMING_DOWNLOADPROGRESSUNITS *)pOmxParam->pParamStruct;

    if (pDownloadUnitsType && pDownloadUnitsType->nSize ==
          sizeof(QOMX_CONFIG_STREAMING_DOWNLOADPROGRESSUNITS))
    {
      if(pDownloadUnitsType->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
          pDownloadUnitsType->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX )

      {
        if(GetDownloadProgressUnitsType(pDownloadUnitsType->nPortIndex,pDownloadUnitsType->eUnitsType))
        {
          ret = MMI_S_COMPLETE;
        }
      }
      else
      {
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "HTTPSourceMMIExtensionHandler:Invalid PortIndex:%u",
                           (uint32)pDownloadUnitsType->nPortIndex);
          ret = MMI_S_EBADPARAM;
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigStreamingDownloadProgress)
  {
    QOMX_CONFIG_STREAMING_DOWNLOADPROGRESSTYPE* pDownloadProgress =
        (QOMX_CONFIG_STREAMING_DOWNLOADPROGRESSTYPE *)pOmxParam->pParamStruct;

    if (pDownloadProgress && pDownloadProgress->nSize ==
          sizeof(QOMX_CONFIG_STREAMING_DOWNLOADPROGRESSTYPE))
    {
      if((pDownloadProgress->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
          pDownloadProgress->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX )||
          (pDownloadProgress->nPortIndex == OMX_ALL))
      {
        if(GetDownloadProgress(pDownloadProgress->nPortIndex,
                                (uint32&)pDownloadProgress->nCurrentStartOffset,
                                (uint32&)pDownloadProgress->nDataDownloaded))
        {
          ret = MMI_S_COMPLETE;
        }
      }
      else
      {
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "HTTPSourceMMIExtensionHandler:Invalid PortIndex:%u",
                          (uint32) pDownloadProgress->nPortIndex);
          ret = MMI_S_EBADPARAM;
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamCompleteDashAdaptationProperties)
  {
    QOMX_DASHPROPERTYINFO* pDashPropertyInfo =
          (QOMX_DASHPROPERTYINFO *)pOmxParam->pParamStruct;

    if(pDashPropertyInfo && pDashPropertyInfo->nSize >= sizeof(QOMX_DASHPROPERTYINFO))
    {
      if((pDashPropertyInfo->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
          pDashPropertyInfo->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX )||
          (pDashPropertyInfo->nPortIndex == OMX_ALL))
      {
        uint32 AdpatationPropertiesLen = 0;

        if(m_pHTTPSourceMMI->m_pHTTPController->GetMediaProperties(NULL,AdpatationPropertiesLen))
        {
          if(AdpatationPropertiesLen > 0)
          {
            ret = MMI_S_COMPLETE;
            if(pDashPropertyInfo->nPropertiesSize >= AdpatationPropertiesLen)
            {
              m_pHTTPSourceMMI->m_pHTTPController->GetMediaProperties((char *)pDashPropertyInfo->cDashProperties,
                                                                        (uint32&)pDashPropertyInfo->nPropertiesSize);
            }
            else
            {
              pDashPropertyInfo->nPropertiesSize = AdpatationPropertiesLen;
            }
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                              "Retreived DashProperties length is zero");
            ret = MMI_S_COMPLETE;
          }
        }
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamSelectedDashAdaptationProperties)
  {
    ret = MMI_S_ENOTIMPL;
  }
    else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamDashMPD)
    {
      QOMX_DASHPROPERTYINFO* pDashPropertyInfo =
            (QOMX_DASHPROPERTYINFO *)pOmxParam->pParamStruct;

      if(pDashPropertyInfo && pDashPropertyInfo->nSize >= sizeof(QOMX_DASHPROPERTYINFO))
      {
       if((pDashPropertyInfo->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
               pDashPropertyInfo->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX )||
               (pDashPropertyInfo->nPortIndex == OMX_ALL))
       {
          uint32 nMPDSize = 0;
          if(m_pHTTPSourceMMI->m_pHTTPController->GetMPDText(NULL,nMPDSize))
          {
            if(nMPDSize > 0)
            {
              ret = MMI_S_COMPLETE;
              if(pDashPropertyInfo->nPropertiesSize >= nMPDSize)
              {
                m_pHTTPSourceMMI->m_pHTTPController->GetMPDText((char *)pDashPropertyInfo->cDashProperties,
                                                                    (uint32&)pDashPropertyInfo->nPropertiesSize);
              }
              else
              {
                pDashPropertyInfo->nPropertiesSize = nMPDSize;
              }
            }
            else
            {
              QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                                "Retreived MPDtext length is zero");
              ret = MMI_S_COMPLETE;
            }
         }
       }
     }
   }
  else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamQOEPlay)
  {
    uint32 nSize = 0;
    QOMX_QOE_DATA_PLAY* pQOEPlayInfo =
            (QOMX_QOE_DATA_PLAY *)pOmxParam->pParamStruct;

    if(pQOEPlayInfo)
    {
      nSize = pQOEPlayInfo->size;
      m_eventHandler.m_HTTPQOEEventMgr.FillQOEData(QOMX_HTTP_IndexParamQOEPlay,
                                                   (char *)pQOEPlayInfo,nSize);
      ret = MMI_S_COMPLETE;
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamQOESwitch)
  {
    uint32 nSize = 0;
    QOMX_QOE_DATA_SWITCH* pQOESwitchInfo =
            (QOMX_QOE_DATA_SWITCH *)pOmxParam->pParamStruct;
    if(pQOESwitchInfo)
    {
      nSize = pQOESwitchInfo->size;
      m_eventHandler.m_HTTPQOEEventMgr.FillQOEData(QOMX_HTTP_IndexParamQOESwitch,
                                                   (char *)pQOESwitchInfo,nSize);
      ret = MMI_S_COMPLETE;
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamQOEStop)
  {
    QOMX_QOE_DATA_STOP* pQOEStopInfo =
            (QOMX_QOE_DATA_STOP *)pOmxParam->pParamStruct;

    if(pQOEStopInfo && pQOEStopInfo->size >= sizeof(QOMX_QOE_DATA_STOP))
    {
      m_eventHandler.m_HTTPQOEEventMgr.FillQOEData(QOMX_HTTP_IndexParamQOEStop,
                                                   (char *)pQOEStopInfo,
                                                   (uint32&)pQOEStopInfo->size);
      ret = MMI_S_COMPLETE;
    }
    else if( pQOEStopInfo )
    {
      m_eventHandler.m_HTTPQOEEventMgr.FillQOEData(QOMX_HTTP_IndexParamQOEStop,
                                                   NULL,
                                                   (uint32&)pQOEStopInfo->size);
      ret = MMI_S_COMPLETE;
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamQOEPeriodic)
  {
    QOMX_QOE_DATA_PERIODIC* pQOEPeriodicInfo =
            (QOMX_QOE_DATA_PERIODIC *)pOmxParam->pParamStruct;
    UpdateQOEData(QOMX_HTTP_IndexParamQOEPeriodic);
    if(pQOEPeriodicInfo && pQOEPeriodicInfo->size >= sizeof(QOMX_QOE_DATA_PERIODIC))
    {
      m_eventHandler.m_HTTPQOEEventMgr.FillQOEData(QOMX_HTTP_IndexParamQOEPeriodic,
                                                   (char *)pQOEPeriodicInfo,
                                                   (uint32&)pQOEPeriodicInfo->size);
      ret = MMI_S_COMPLETE;
    }
    else if (pQOEPeriodicInfo)
    {
      m_eventHandler.m_HTTPQOEEventMgr.FillQOEData(QOMX_HTTP_IndexParamQOEPeriodic,
                                                   NULL,
                                                   (uint32&)pQOEPeriodicInfo->size);
      ret = MMI_S_COMPLETE;
    }
  }
else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamCurrentRepresenation)
  {
    //Validate pOmxParam->pParamStruct
    ret = MMI_S_EBADPARAM;
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "Query for  QOMX_HTTP_IndexParamCurrentRepresenation");

    QOMX_PARAM_REPRESENTATION *pCurrRepInfo =
    (QOMX_PARAM_REPRESENTATION *)pOmxParam->pParamStruct;
    if (pCurrRepInfo == NULL)
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pCurrRepInfo );
    }
    else
    {
      if (pCurrRepInfo->nSize < sizeof(QOMX_PARAM_REPRESENTATION))
      {
        ret = MMI_S_EFAIL;
        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid nSize %u, minSize %d",
                  (uint32)pCurrRepInfo->nSize, sizeof(QOMX_PARAM_REPRESENTATION));
      }
      else
      {
         ret = GetCurrentPlayingRepInfo(pCurrRepInfo);
      }
    }
  }
else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamCookie)
  {
    //Validate pOmxParam->pParamStruct
    ret = MMI_S_EBADPARAM;
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "Query for  QOMX_HTTP_IndexParamCookie");

    QOMX_PARAM_COOKIES *pCookieInfo =
    (QOMX_PARAM_COOKIES *)pOmxParam->pParamStruct;
    if (pCookieInfo == NULL)
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pCookieInfo );
    }
    else
    {
      if (pCookieInfo->nSize < sizeof(QOMX_PARAM_COOKIES))
      {
        ret = MMI_S_EFAIL;
        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid nSize %u, minSize %d",
                  (uint32)pCookieInfo->nSize, sizeof(QOMX_PARAM_REPRESENTATION));
      }
      else
      {
         size_t cookieLen = 0;
        if (pHTTPController &&
            pHTTPController->GetCookies((const char*)pCookieInfo->cURLString,
                                        (const char*)pCookieInfo->cURLString+pCookieInfo->nURLLen,
                                         cookieLen))
        {
          pCookieInfo->nCookieLen = (OMX_U32)cookieLen;
          ret = MMI_S_COMPLETE;
        }
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamAudioAc3)
  {
    QOMX_AUDIO_PARAM_AC3PROFILETYPE *pAc3Fmt =
      (QOMX_AUDIO_PARAM_AC3PROFILETYPE*)pOmxParam->pParamStruct;
    if (!pAc3Fmt || !m_pHTTPSourceMMI->IsValidPort(pAc3Fmt->nPortIndex, OMX_PortDomainAudio) ||
      (sizeof(QOMX_AUDIO_PARAM_AC3PROFILETYPE) != pAc3Fmt->nSize))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "OMX_IndexParamAudioAc3: pAc3Fmt/port index is invalid" );
    }
    else if(m_pHTTPSourceMMI->IsSeekPending())
    {
      ret = MMI_S_EFAIL;
    }
    else
    {
      pPortInfo = &m_pHTTPSourceMMI->m_portAudio[ pAc3Fmt->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      ret = MMI_S_COMPLETE;
      QOMX_STRUCT_INIT(*pAc3Fmt, QOMX_AUDIO_PARAM_AC3PROFILETYPE);
      if (pPortInfo->m_pPortSpecificData)
      {
        memcpy(pAc3Fmt, pPortInfo->m_pPortSpecificData, sizeof(QOMX_AUDIO_PARAM_AC3PROFILETYPE));
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexParamAudioMp2)
  {
    QOMX_AUDIO_PARAM_MP2PROFILETYPE *pMp2Fmt =
      (QOMX_AUDIO_PARAM_MP2PROFILETYPE*)pOmxParam->pParamStruct;
    if (!pMp2Fmt || !m_pHTTPSourceMMI->IsValidPort(pMp2Fmt->nPortIndex, OMX_PortDomainAudio) ||
      (sizeof(QOMX_AUDIO_PARAM_MP2PROFILETYPE) != pMp2Fmt->nSize))
    {
      QTV_MSG_PRIO( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
        "OMX_IndexParamAudioMp2: pAc3Fmt/port index is invalid" );
    }
    else if(m_pHTTPSourceMMI->IsSeekPending())
    {
      ret = MMI_S_EFAIL;
    }
    else
    {
      pPortInfo = &m_pHTTPSourceMMI->m_portAudio[ pMp2Fmt->nPortIndex - MMI_HTTP_AUDIO_PORT_INDEX];
      ret = MMI_S_COMPLETE;
      QOMX_STRUCT_INIT(*pMp2Fmt, QOMX_AUDIO_PARAM_MP2PROFILETYPE);
      if (pPortInfo->m_pPortSpecificData)
      {
        memcpy(pMp2Fmt, pPortInfo->m_pPortSpecificData, sizeof(QOMX_AUDIO_PARAM_MP2PROFILETYPE));
      }
    }

  }
else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamBufferedDuration)
  {
    QOMX_BUFFERED_DURATION *pBufferStatus =
            (QOMX_BUFFERED_DURATION*)pOmxParam->pParamStruct;

    if(((pBufferStatus->nPortIndex == OMX_ALL) ||
        ((pBufferStatus->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX) &&
       (pBufferStatus->nPortIndex <=  MMI_HTTP_VIDEO_PORT_INDEX))))
    {
      uint64 currentBufferLevel = MAX_UINT32_VAL;
      uint64 dwldPos = MAX_UINT32_VAL;
      bool isSuccess = false;
      pBufferStatus->bufDuration = 0;

      if (pBufferStatus->nPortIndex == OMX_ALL)
      {
        isSuccess = m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper->GetDurationBuffered(MMI_HTTP_AUDIO_PORT_INDEX, currentBufferLevel, dwldPos);
        if(isSuccess)
        {
          pBufferStatus->bufDuration = dwldPos;
        }

        dwldPos = STD_MIN(dwldPos, pBufferStatus->bufDuration);
        isSuccess = m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper->GetDurationBuffered(MMI_HTTP_VIDEO_PORT_INDEX, currentBufferLevel, dwldPos);
        if (isSuccess)
        {
          pBufferStatus->bufDuration = (pBufferStatus->bufDuration > 0) ? STD_MIN(dwldPos, pBufferStatus->bufDuration) : dwldPos;
        }

        ret = MMI_S_COMPLETE;
      }
      else
      {
        isSuccess = m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper->GetDurationBuffered(pBufferStatus->nPortIndex, currentBufferLevel, pBufferStatus->bufDuration);
        if(isSuccess)
        {
          ret = MMI_S_COMPLETE;
        }
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamMediaTrackEncoding)
  {
    //Validate pOmxParam->pParamStruct
    ret = MMI_S_EBADPARAM;
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "Query for  QOMX_HTTP_IndexParamMediaTrackEncoding");

    QOMX_PARAM_MEDIA_TRACK_ENCODING *pTrackEncoding =
    (QOMX_PARAM_MEDIA_TRACK_ENCODING *)pOmxParam->pParamStruct;
    if (pTrackEncoding == NULL)
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pTrackEncoding );
    }
    else
    {
      if (pTrackEncoding->nSize < sizeof(QOMX_PARAM_MEDIA_TRACK_ENCODING))
      {
        ret = MMI_S_EFAIL;
        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid nSize %u, minSize %d",
                  (uint32)pTrackEncoding->nSize, sizeof(QOMX_PARAM_MEDIA_TRACK_ENCODING));
      }
      else
      {
         ret = GetTrackEncodingInfo(pTrackEncoding);
      }
    }
  }
  else if (nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigDASHResumeDiscontinuity)
  {
    QOMX_DASH_RESUME_DISCONTINUITY* pResumeDiscontinuity =
      (QOMX_DASH_RESUME_DISCONTINUITY*)pOmxParam->pParamStruct;
    if (pResumeDiscontinuity == NULL ||
        (pResumeDiscontinuity->nSize != sizeof(QOMX_DASH_RESUME_DISCONTINUITY)))
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pResumeDiscontinuity );
    }
    else if (pResumeDiscontinuity->nPortIndex == OMX_ALL ||
            (pResumeDiscontinuity->nPortIndex >= MMI_HTTP_PORT_START_INDEX  &&
             pResumeDiscontinuity->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX ))
    {
      if (m_pHTTPSourceMMI->m_pHTTPDataInterface &&
          m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
      {
        HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
        m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->MapPortIDToMediaType(pResumeDiscontinuity->nPortIndex, mediaType);
        HTTPCommon::HTTPAttrVal val;
        if (m_pHTTPSourceMMI->m_pHTTPDataInterface->GetConfig(mediaType, HTTPCommon::HTTP_ATTR_PLAYBACK_DISCONTINUITY, val))
        {
          pResumeDiscontinuity->bDiscontinuity = (OMX_BOOL)val.bool_attr_val;
          ret = MMI_S_COMPLETE;
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Failed to query resume discontinuity for mediaType %d", mediaType );
          ret = MMI_S_EFAIL;
        }
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid PortIndex: %u", (uint32) pResumeDiscontinuity->nPortIndex );
      ret = MMI_S_EBADPARAM;
    }
  }
  else if (nParamIndex == (OMX_INDEXTYPE) QOMX_HTTP_IndexConfigDASHRepositionRange)
  {
    QOMX_DASH_REPOSITION_RANGE* pReposRange =
      (QOMX_DASH_REPOSITION_RANGE*)pOmxParam->pParamStruct;
    if (pReposRange == NULL ||
        (pReposRange->nSize != sizeof(QOMX_DASH_REPOSITION_RANGE)))
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pReposRange );
    }
    else if (pReposRange->nPortIndex == OMX_ALL ||
            (pReposRange->nPortIndex >= MMI_HTTP_PORT_START_INDEX  &&
             pReposRange->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX ))
    {
      if (m_pHTTPSourceMMI->m_pHTTPDataInterface &&
          m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler)
      {
        HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
        m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->MapPortIDToMediaType(pReposRange->nPortIndex, mediaType);
        HTTPCommon::HTTPAttrVal val;
        if (m_pHTTPSourceMMI->m_pHTTPDataInterface->GetConfig(mediaType, HTTPCommon::HTTP_ATTR_REPOSITION_RANGE, val))
        {
          pReposRange->nMin = val.sReposRange.nMin;
          pReposRange->nMax = val.sReposRange.nMax;
          pReposRange->nMaxDepth = val.sReposRange.nMaxDepth;
          pReposRange->bDataEnd = (OMX_BOOL)val.sReposRange.bDataEnd;
          ret = MMI_S_COMPLETE;
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Failed to retrieve reposition range for mediaType %d", mediaType );
          ret = MMI_S_EFAIL;
        }
      }
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                     "Invalid PortIndex: %u", (uint32)pReposRange->nPortIndex );
      ret = MMI_S_EBADPARAM;
    }
  }
  return ret;
}

/*
 * @breif Processes the Set request for vendor Specific OEM extensions
 *
 * @Param[in] Reference object to MMI_OmxParamCmdType
 *
 * @return  MMI_S_COMPLETE on success or MMI_S_ENOTIMPL otherwise
 */
uint32 HTTPSourceMMIExtensionHandler::ProcessMMISetStdExtnParam(MMI_OmxParamCmdType* pOmxParam)
{
  uint32 ret = MMI_S_EBADPARAM;
  HTTPController* pHTTPController  = NULL;

  if ((NULL == m_pHTTPSourceMMI) || (NULL == pOmxParam))
  {
    return ret;
  }

  pHTTPController = m_pHTTPSourceMMI->m_pHTTPController;

  OMX_INDEXTYPE nParamIndex = pOmxParam->nParamIndex;
  if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingNetworkInterface)
  {
    //Validate pOmxParam->pParamStruct and make sure network iface is in range
    QOMX_PARAM_STREAMING_NETWORKINTERFACE* pNetIface =
    (QOMX_PARAM_STREAMING_NETWORKINTERFACE *)pOmxParam->pParamStruct;
    if (pNetIface && pNetIface->nSize == sizeof(QOMX_PARAM_STREAMING_NETWORKINTERFACE) &&
    pNetIface->eNetworkInterface <= QOMX_STREAMING_NETWORKINTERFACE_CMMB_IFACE)
    {
      //Set the network interface
      if (pHTTPController &&
          pHTTPController->SetNetworkInterface(pNetIface->eNetworkInterface))
      {
        ret = MMI_S_COMPLETE;
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p",(void *)pNetIface );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingNetworkProfile)
  {
    //Validate pOmxParam->pParamStruct
    OMX_PARAM_U32TYPE* pValue = (OMX_PARAM_U32TYPE *)pOmxParam->pParamStruct;
    if (pValue && pValue->nSize == sizeof(OMX_PARAM_U32TYPE))
    {
      //Set the netpolicy profile
      if (pHTTPController && pHTTPController->SetNetworkProfile(pValue->nU32))
      {
        ret = MMI_S_COMPLETE;
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pValue );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamStreamingProxyServer)
  {
    OMX_PARAM_CONTENTURITYPE* pURI =
    (OMX_PARAM_CONTENTURITYPE *)pOmxParam->pParamStruct;
    if (pURI)
    {
      size_t uriLen = std_strlen((const char*)pURI->contentURI);

      if ((uriLen > 0 &&
      pURI->nSize >= (uriLen + 1 + offsetof(OMX_PARAM_CONTENTURITYPE, contentURI[0]))))
      {
        if (pHTTPController &&
        pHTTPController->SetProxyServer((const char*)pURI->contentURI,uriLen))
        {
          ret = MMI_S_COMPLETE;
        }
      }
      else
      {
        ret = MMI_S_EBADPARAM;
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "HTTPSourceMMIExtensionhandler:Invalid URI length %d", uriLen );
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPSourceMMIExtensionhandler:Invalid OMX Paramter : 0x%p",(void *)pURI );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexConfigStreamingProtocolHeader)
  {
    //Validate pOmxParam->pParamStruct
    QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE* pHeader =
      (QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE *)pOmxParam->pParamStruct;
    if (pHeader)
    {
      //Set the protocol header
      ret = SetProtocolHeader(*pHeader);
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "CRTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pHeader );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexStreamingConfigWaterMark)
  {
    QOMX_BUFFERINGWATERMARKTYPE *pBufferingWaterMark =
                    (QOMX_BUFFERINGWATERMARKTYPE *)pOmxParam->pParamStruct;
    if(pBufferingWaterMark &&
          pBufferingWaterMark->nSize == sizeof(QOMX_BUFFERINGWATERMARKTYPE))
    {
      if(pBufferingWaterMark->eUnitsType == QOMX_WATERMARKUNITSTYPE_Time )
      {
        //DASH Cient shall use default values for initial/rebuff preroll
        //if the watermark level value is MAX_UINT32
        if(pBufferingWaterMark->nLevel != MAX_UINT32)
        {
          //convert level in to millisecs
          uint32 nLevel = pBufferingWaterMark->nLevel/1000;

          if(pBufferingWaterMark->eWaterMark == QOMX_WATERMARK_NORMAL)
          {
            if(pHTTPController)
            {
              pHTTPController->SetInitialPreroll(nLevel);
              pHTTPController->SetRebufferPreroll(nLevel);
              ret = MMI_S_COMPLETE;
            }
          }
          else if(pBufferingWaterMark->eWaterMark == QOMX_WATERMARK_UNDERRUN )
          {
            // Support only setting zero LMW
            if(nLevel == 0)
            {
              ret = MMI_S_COMPLETE;
            }
            else
            {
              QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                                  "HTTPMMIExtensionHandler: Setting non Zero"
                                  " Low watermark level isn't supported");
              ret = MMI_S_EBADPARAM;
            }
          }
          else
          {
            QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                            "HTTPMMIExtensionHandler Invalid Buffering WaterMark Type %d",
                            pBufferingWaterMark->eWaterMark );
            ret = MMI_S_EBADPARAM;
          }
        }
        else
        {
          if((QOMX_WATERMARK_UNDERRUN == pBufferingWaterMark->eWaterMark) ||
             (QOMX_WATERMARK_NORMAL == pBufferingWaterMark->eWaterMark))
          {
            ret = MMI_S_COMPLETE;
          }
        }
      }
      else
      {
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                        "HTTPMMIExtensionHandler Invalid/Unsupported Watermark units type:%d",
                        pBufferingWaterMark->eUnitsType);
        ret = MMI_S_EBADPARAM;
      }

      //handle Enabling buffering notification for port(s)
      if(ret == MMI_S_COMPLETE)
      {
        bool bEnable = (pBufferingWaterMark->bEnable == OMX_TRUE)? true : false;
        if(pBufferingWaterMark->nPortIndex == OMX_ALL)
        {
          for(int portIndex = MMI_HTTP_AUDIO_PORT_INDEX;
                portIndex <= MMI_HTTP_VIDEO_PORT_INDEX;portIndex++)
          {
            m_eventHandler.m_HTTPBufferingEventMgr[portIndex].SetBufferingNotifcation(
              pBufferingWaterMark->eWaterMark,bEnable);

            uint32 portIdxAndWatermarkType = 0;
            portIdxAndWatermarkType =
              ((portIndex & 0xffffffff) << 16) |
              (pBufferingWaterMark->eWaterMark & 0xffffffff);

            if (OMX_TRUE == bEnable)
            {
              m_pHTTPSourceMMI->m_pHTTPController->NotifyWaterMarkStatus(
                portIdxAndWatermarkType);
            }
          }
        }
        else if(pBufferingWaterMark->nPortIndex >= MMI_HTTP_PORT_START_INDEX &&
                pBufferingWaterMark->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX)
        {
          m_eventHandler.m_HTTPBufferingEventMgr[pBufferingWaterMark->nPortIndex].SetBufferingNotifcation(
              pBufferingWaterMark->eWaterMark,bEnable);

          uint32 portIdxAndWatermarkType = 0;
          portIdxAndWatermarkType =
            ((pBufferingWaterMark->nPortIndex & 0xffffffff) << 16) |
            (pBufferingWaterMark->eWaterMark & 0xffffffff);

          if (OMX_TRUE == bEnable)
          {
            m_pHTTPSourceMMI->m_pHTTPController->NotifyWaterMarkStatus(
              portIdxAndWatermarkType);
          }
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "HTTPMMIExtensionHandler: Invalid Port Index:%u", (uint32)pBufferingWaterMark->nPortIndex);
          ret = MMI_S_EBADPARAM;
        }
      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)OMX_IndexConfigCallbackRequest)
  {
    //Validate pOmxParam->pParamStruct and the request could be for a
    //specific port or all ports (OMX_ALL)
    OMX_CONFIG_CALLBACKREQUESTTYPE* pCallback =
    (OMX_CONFIG_CALLBACKREQUESTTYPE *)pOmxParam->pParamStruct;
    if (pCallback && pCallback->nSize == sizeof(OMX_CONFIG_CALLBACKREQUESTTYPE))
    {
      if ((int)pCallback->nIndex == (int)QOMX_HTTP_IndexConfigStreamingProtocolEvent)
      {
        //Make sure the request is for all ports (OMX_ALL) and set the event
        //notification flag
        if (pCallback->nPortIndex == OMX_ALL)
        {
          //enable Protocol error Event notification flag
          bool enable = (pCallback->bEnable == OMX_TRUE) ? true : false;
          m_eventHandler.m_HTTPProtocolEventMgr.SetNotify(enable);
          ret = MMI_S_COMPLETE;
        }
        else
        {
          ret = MMI_S_EBADPARAM;
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
          "HTTPMMIExtensionHandler Invalid port index %u",
          (uint32)pCallback->nPortIndex );
        }
      }

      else if ( (int)pCallback->nIndex == (int)QOMX_HTTP_IndexConfigStreamingProtocolHeadersEvent )
      {
          //Make sure the request is for all ports (OMX_ALL) and set the event
          //notification flag
          if (pCallback->nPortIndex == OMX_ALL)
          {
            m_eventHandler.m_HTTPProtocolHeadersEventMgr.SetNotify(
              (pCallback->bEnable == OMX_TRUE) ? true : false);
            ret = MMI_S_COMPLETE;

          }
          else
          {
            ret = MMI_S_EBADPARAM;
              QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "HTTPMMIExtensionHandler Invalid port index %u",
              (uint32)pCallback->nPortIndex );
          }
        }
      else
      {
        ret = MMI_S_ENOTIMPL;
        QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
        "HTTPMMIExtensionHandler Unsupported callback extension index %d",
        pCallback->nIndex );
      }
    }
    else
    {
      ret = MMI_S_EBADPARAM;
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pCallback );
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamSelectedDashAdaptationProperties)
  {
    QOMX_DASHPROPERTYINFO* pDashPropertyInfo =
          (QOMX_DASHPROPERTYINFO *)pOmxParam->pParamStruct;

    if(pDashPropertyInfo && pDashPropertyInfo->nSize >= sizeof(QOMX_DASHPROPERTYINFO))
    {
      if((pDashPropertyInfo->nPortIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
          pDashPropertyInfo->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX )||
          (pDashPropertyInfo->nPortIndex == OMX_ALL))
      {
        if(pDashPropertyInfo->nPropertiesSize > 0 && pDashPropertyInfo->cDashProperties)
        {
          if (m_pHTTPSourceMMI->IsSelectRepresentationsPending())
          {
            QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                         "Caching the select representations cmd to be executed later");
            m_pHTTPSourceMMI->CacheSelectedRepresentations(
               (const char*)pDashPropertyInfo->cDashProperties);
          }
          else
          {
            if(m_pHTTPSourceMMI->m_pHTTPController->SelectRepresentations((const char*)
                                                                      pDashPropertyInfo->cDashProperties))
            {
              ret = MMI_S_COMPLETE;
              QTV_MSG_PRIO(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
                 "Adaptationset change: SetSelectRepresentationsPending(true)");
              m_pHTTPSourceMMI->SetSelectRepresentationsPending(true);
            }
          }
        }

      }
      else
      {
        QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "SelectRepresentations length is zero");

      }
    }
  }
  else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamQOE)
  {
    QOMX_QOE_EVENT_REG* pQOEEventReg =
        (QOMX_QOE_EVENT_REG *)pOmxParam->pParamStruct;
    m_eventHandler.m_HTTPQOEEventMgr.SetQOENotifcation((const bool)pQOEEventReg->bNotify);
    ret = MMI_S_COMPLETE;
  }
else if(nParamIndex == (OMX_INDEXTYPE)QOMX_HTTP_IndexParamCookie)
  {
    //Validate pOmxParam->pParamStruct
    ret = MMI_S_EBADPARAM;
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
              "Query for  QOMX_HTTP_IndexParamCookie");

    QOMX_PARAM_COOKIES *pCookieInfo =
    (QOMX_PARAM_COOKIES *)pOmxParam->pParamStruct;
    if (pCookieInfo == NULL)
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pCookieInfo );
    }
    else
    {
      if (pHTTPController &&
         pHTTPController->SetCookie((const char*)pCookieInfo->cURLString,
                                     (const char*)(pCookieInfo->cURLString + pCookieInfo->nURLLen)))
      {
        ret = MMI_S_COMPLETE;
      }
    }
  }
  return ret;
}

/**
 *
 */
uint32 HTTPSourceMMIExtensionHandler::SetHTTPProtocolEvent
(
  void *protocolInfo
)
{
  uint32 rsltCode = MMI_S_EFAIL;

  if (m_eventHandler.m_HTTPProtocolEventMgr.SetHTTPProtocolEvent(
                                        m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper,
                                        protocolInfo,
                                        m_pHTTPSourceMMI->m_pClientData))
  {
    rsltCode = MMI_S_COMPLETE;
  }
  return rsltCode;
}

uint32 HTTPSourceMMIExtensionHandler::SetHTTPProtocolHeadersEvent
(
  void *protocolHeadersEventInfo
)
{
  uint32 rsltCode = MMI_S_EFAIL;

  if (m_eventHandler.m_HTTPProtocolHeadersEventMgr.SetHTTPProtocolHeadersEvent(
                                        m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper,
                                        protocolHeadersEventInfo,
                                        m_pHTTPSourceMMI->m_pClientData))
  {
    rsltCode = MMI_S_COMPLETE;
  }
  return rsltCode;
}


/*
 * Get the protocol event information.
 *
 * @param[in/out] protocolEvent - Reference to protocol event info
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure
 */
uint32 HTTPSourceMMIExtensionHandler::GetHTTPProtocolEvent
(
 QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE& protocolEvent
)
{
  uint32 result = MMI_S_EBADPARAM;

  HTTPSourceMMIExtensionEventHandler::HTTPProtocolEventManager::HTTPProtocolEvent eEvent = {0,0,0};

  if (protocolEvent.nSize < sizeof(QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE))
  {
    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "GetProtocolEvent: Invalid nSize %u, minSize %d",
                  (uint32)protocolEvent.nSize, sizeof(QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE));
    return result;
  }

  bool isQueryforSizes = (0 == protocolEvent.nContentUriSize) &&
                         (0 == protocolEvent.nEntityBodySize) &&
                         (0 == protocolEvent.nReasonPhraseSize);

  if (true == isQueryforSizes)
  {
    // Query for sizes
    if (m_eventHandler.m_HTTPProtocolEventMgr.PeekHTTPProtocolEvent(eEvent))
    {
      protocolEvent.nEntityBodySize = (OMX_U32)eEvent.entityBody.get_size();
      protocolEvent.nReasonPhraseSize = (OMX_U32)eEvent.reasonPhrase.get_size();
      //right now uri in event is not supported at stack level
      protocolEvent.nContentUriSize = 0;
      protocolEvent.nProtocolEvent = (OMX_U32)eEvent.nStatusCode;

      QTV_MSG_PRIO3(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
                    "GetHTTPProtocolEvent: Query result urisize %u, "
                    "entitybody size %u, reason size %u",
                    (uint32)protocolEvent.nContentUriSize,
                    (uint32)protocolEvent.nEntityBodySize,
                    (uint32)protocolEvent.nReasonPhraseSize);

      result = MMI_S_COMPLETE;
    }
    else
    {
      // Failure to peek event
      QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
                   "GetHTTPProtocolEvent: Queue is empty");
      result = MMI_S_ENOMORE;
    }
  }
  //Note that protocolEventText also includes terminating byte. protocolEvent structure looks like
  //<nSize = 4 bytes><nVersion = 4 bytes><nProtocolEvent = 4 bytes><nReasonPhraseSize = 4 bytes>
  //<nEntityBodySize = 4bytes><protocolEventText string = nReasonPhraseSize + nEntityBodySize + 1
  //bytes>. Validate nSize just to make sure
  else if (protocolEvent.nSize >= (offsetof(QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE, protocolEventText[0]))+
                                    protocolEvent.nReasonPhraseSize + protocolEvent.nEntityBodySize +
                                       protocolEvent.nContentUriSize + 1)
  {
    if (m_eventHandler.m_HTTPProtocolEventMgr.GetHTTPProtcolEvent(eEvent))
    {
      //Declare success if protocol error code is obtained because server message is optional and
      //also not all clients might be interested in server error message
      protocolEvent.nProtocolEvent = (OMX_U32)eEvent.nStatusCode;
      result = MMI_S_COMPLETE;

      //Fill the reason phrase and/or entity body if present and if the client wants both/either
      //of them, However make sure the protocolEventText field is big enough to hold the string,
      //else truncate data
      uint32 maxToCopy = 0;
      const char *pReasonPhrase = eEvent.reasonPhrase,
                 *pEntityBody = eEvent.entityBody;
      if (protocolEvent.nReasonPhraseSize > 0 && pReasonPhrase)
      {
        size_t reasonPhraseLen = std_strlen(pReasonPhrase);
        maxToCopy = QTV_MIN(protocolEvent.nReasonPhraseSize, (OMX_U32)reasonPhraseLen);
        (void)std_strlcpy((char*)(protocolEvent.protocolEventText),
                          pReasonPhrase,
                          maxToCopy + 1);
      }
      protocolEvent.nReasonPhraseSize = maxToCopy;

      maxToCopy = 0;
      if (protocolEvent.nEntityBodySize > 0 && pEntityBody)
      {
        size_t entityBodyLen = std_strlen(pEntityBody);
        maxToCopy = QTV_MIN(protocolEvent.nEntityBodySize, (OMX_U32)entityBodyLen);
        (void)std_strlcpy((char*)(protocolEvent.protocolEventText + protocolEvent.nReasonPhraseSize),
                          pEntityBody,
                          maxToCopy + 1);
      }
      protocolEvent.nEntityBodySize = maxToCopy;
      //right now uri in event is not supported at stack level
      protocolEvent.nContentUriSize = 0;
    }
    else
    {
      //Failure in getting error event
      result = MMI_S_EFAIL;
    }
  }
  else
  {
    QTV_MSG_PRIO3( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                   "HTTPSourceMMIExtensionHandler Invalid input parameter - "
                   "nSize %u, nReasonPhraseSize %u, nEntityBodySize %u",
                   (uint32)protocolEvent.nSize,
                   (uint32)protocolEvent.nReasonPhraseSize,(uint32) protocolEvent.nEntityBodySize );
  }

  return result;
}



/**
 * Get the protocol headers event. Can be used in 2 modes. If
 * nSize is set to zero, then it is treated as a query to
 * determine the sizes of fields. If it is not zero, then it is
 * treated as a query to populate the fields based on the field
 * sizes passed to the function.
 *
 * @param protocolHeadersEvent
 *
 * @return uint32
 */
uint32 HTTPSourceMMIExtensionHandler::GetHTTPProtocolHeadersEvent
(
  QOMX_STREAMING_PROTOCOLHEADERSTYPE& protocolHeadersEvent
)
{
  uint32 result = MMI_S_EBADPARAM;

  static const int messageHeadersOffset =
    offsetof(QOMX_STREAMING_PROTOCOLHEADERSTYPE, messageHeaders[0]);

  HTTPSourceMMIExtensionEventHandler::HTTPProtocolHeadersEventManager::HTTPProtocolHeadersEvent eEvent = {0,0,0};


  bool isQueryforSizes = (0 == protocolHeadersEvent.nMessageClassSize) &&
                         (0 == protocolHeadersEvent.nMessageAtributesSize);

  uint32 maxBytesForMsgHeaders =
    (uint32)(protocolHeadersEvent.nSize - messageHeadersOffset);

  uint32 totalBytesForMsgHeaders = protocolHeadersEvent.nMessageClassSize +
                                protocolHeadersEvent.nMessageAtributesSize;

  bool isInvalidParam = false;

  if (protocolHeadersEvent.nSize < sizeof(QOMX_STREAMING_PROTOCOLHEADERSTYPE))
  {
    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetProtocolHeadersEvent: Invalid nSize %u, minSize %d",
       (uint32)protocolHeadersEvent.nSize, sizeof(QOMX_STREAMING_PROTOCOLHEADERSTYPE));

    isInvalidParam = true;
  }

  if (false == isInvalidParam)
  {
    if (true == isQueryforSizes)
    {
      // Query for sizes
      if (m_eventHandler.m_HTTPProtocolHeadersEventMgr.PeekHTTPProtocolHeadersEvent(eEvent))
      {
        const char *msgClass = eEvent.messageClass;
        const char *msgAttrs = eEvent.protocolHeaders;

        protocolHeadersEvent.nMessageClassSize =
          (OMX_U32)(msgClass ? std_strlen(msgClass) : 0);

        protocolHeadersEvent.nMessageAtributesSize =
          (OMX_U32)(msgAttrs ? std_strlen(msgAttrs) : 0);


        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_HIGH,
          "GetProtocolHeadersEvent: Query result msgClassSize %u, msgHdrsSize %u",
          (uint32)protocolHeadersEvent.nMessageClassSize, (uint32)protocolHeadersEvent.nMessageAtributesSize);

        result = MMI_S_COMPLETE;
      }
      else
      {
        // Failure to peek event
        QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
                     "GetProtocolHeadersEvent: Queue is empty");
        result = MMI_S_ENOMORE;
      }
    }
    else
    {
      // populate the structure.
      if (m_eventHandler.m_HTTPProtocolHeadersEventMgr.GetHTTPProtocolHeadersEvent(eEvent))
      {
        const char *msgClass = eEvent.messageClass;
        const char *msgHdrs = eEvent.protocolHeaders;

        size_t msgClassSize = (msgClass ? std_strlen(msgClass) : 0);
        size_t msgHdrsSize = (msgHdrs ? std_strlen(msgHdrs) : 0);

        protocolHeadersEvent.eMessageType =
            (eEvent.msgType
             ? QOMX_STREAMING_PROTOCOLMESSAGE_RESPONSE
             : QOMX_STREAMING_PROTOCOLMESSAGE_REQUEST);

        if (msgClass)
        {
          protocolHeadersEvent.nMessageClassSize =
            QTV_MIN(protocolHeadersEvent.nMessageClassSize,(OMX_U32)msgClassSize);

          memcpy(protocolHeadersEvent.messageHeaders,
                 msgClass,
                 protocolHeadersEvent.nMessageClassSize);
        }
        else
        {
          protocolHeadersEvent.nMessageClassSize = 0;
        }

        if (msgHdrs)
        {
          protocolHeadersEvent.nMessageAtributesSize =
            QTV_MIN(protocolHeadersEvent.nMessageAtributesSize, (OMX_U32)msgHdrsSize);

          memcpy((char *)protocolHeadersEvent.messageHeaders + protocolHeadersEvent.nMessageClassSize,
                 msgHdrs,
                 protocolHeadersEvent.nMessageAtributesSize);
        }
        else
        {
          protocolHeadersEvent.nMessageAtributesSize = 0;
        }

        QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
          "GetProtocolHeadersEvent: Populate msgClassSize %u, msgHdrsSize %u",
          (uint32)protocolHeadersEvent.nMessageClassSize, (uint32)protocolHeadersEvent.nMessageAtributesSize);

        result = MMI_S_COMPLETE;
      }
      else
      {
        //Failure in getting error event
        QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_MED,
                     "GetProtocolHeadersEvent: Queue is empty");

        result = MMI_S_ENOMORE;
      }
    }
  }

  return result;

}

/**
 * @brief
 *
 * @param portID
 * @param bDataAvailable
 *
 * @return uint32
 */
bool HTTPSourceMMIExtensionHandler::SetHTTPBufferingStatus
(
  const uint32 portID,
  bool bDataAvailable
)
{
  bool rsltCode = false;

  if((portID >= MMI_HTTP_AUDIO_PORT_INDEX &&
      portID <= MMI_HTTP_OTHER_PORT_INDEX) && m_pHTTPSourceMMI && !m_pHTTPSourceMMI->IsClosePending())
  {
    rsltCode = m_eventHandler.m_HTTPBufferingEventMgr[portID].ProcessBufferingStatus(
                          m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper,
                          portID,
                          bDataAvailable,
                          m_pHTTPSourceMMI->m_pClientData);
  }

  return rsltCode;
}

/**
 * @brief
 *  Notify watermark event to client if http component is
 *  already at the watermark level that was registered for.
 *
 * @param portIdxAndWatermarkType packed value of portIdx and
 *                                registerd watermark type.
 */
void HTTPSourceMMIExtensionHandler::NotifyWatermarkEvent(uint32 portIdxAndWatermarkType)
{
  // unpack portIdx and watermark type from arg.
  uint32 portID = (portIdxAndWatermarkType & 0xffff0000) >> 16;
  QOMX_WATERMARKTYPE watermarkType = (QOMX_WATERMARKTYPE)(portIdxAndWatermarkType & 0x0000ffff);

  if(portID >= MMI_HTTP_AUDIO_PORT_INDEX &&
      portID <= MMI_HTTP_OTHER_PORT_INDEX)
  {
    // Get the current watermart type.
    QOMX_BUFFERINGSTATUSTYPE bufferStatus;
    GetCurrentWatermarkStatus(portID, &bufferStatus);

    // Send notification if for the watermark being registered, we have are
    // already at that watermark level.
    bool sendNotification =
      (QOMX_WATERMARK_NORMAL == watermarkType &&
       QOMX_WATERMARK_NORMAL == bufferStatus.eCurrentWaterMark &&
       true == m_eventHandler.m_HTTPBufferingEventMgr[portID].GetBufferingNotification(QOMX_WATERMARK_NORMAL))
      ||
      (QOMX_WATERMARK_UNDERRUN == watermarkType &&
       QOMX_WATERMARK_UNDERRUN == bufferStatus.eCurrentWaterMark &&
       true == m_eventHandler.m_HTTPBufferingEventMgr[portID].GetBufferingNotification(QOMX_WATERMARK_UNDERRUN));

    if (sendNotification && !m_pHTTPSourceMMI->IsClosePending())
    {
      m_eventHandler.m_HTTPBufferingEventMgr[portID].NotifyBufferingEvent(
        m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper,
        portID,
        m_pHTTPSourceMMI->m_pClientData);

    }

    QTV_MSG_PRIO5( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM,
                   "HTTPBuffering Notification on registration: portID:%u,"
                   "WMType: %d, WMLevel: %u, WMUnits: %d. Notification Sent %d",
                   portID,
                   bufferStatus.eCurrentWaterMark,
                   (uint32)bufferStatus.nCurrentLevel,
                   bufferStatus.eUnitsType,
                   sendNotification );
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                  "NotifyWatermarkEvent Invalid portID %u", portID);
  }
}

/**
 * @brief
 *  Determine the current watermark status.
 *
 * @param portID
 * @param pBufferStatus
 */
uint32 HTTPSourceMMIExtensionHandler::GetCurrentWatermarkStatus(
  uint32 portID, QOMX_BUFFERINGSTATUSTYPE* pBufferStatus)
{
  HTTPSourceMMITrackHandler *trackHdlr = NULL;
  HTTPSourceMMI::HttpSourceMmiPortInfo* pPortInfo = NULL;
  QOMX_WATERMARKTYPE watermark = QOMX_WATERMARK_UNDERRUN;
  uint32 ret = MMI_S_EBADPARAM;
  HTTPMediaType majorType = HTTPCommon::HTTP_UNKNOWN_TYPE;

  if (m_pHTTPSourceMMI)
  {
    trackHdlr = m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler;
    if(portID == MMI_HTTP_AUDIO_PORT_INDEX)
    {
      majorType = HTTPCommon::HTTP_AUDIO_TYPE;
      pPortInfo = &m_pHTTPSourceMMI->m_portAudio[portID - MMI_HTTP_AUDIO_PORT_INDEX];
    }
    else if(portID == MMI_HTTP_VIDEO_PORT_INDEX)
    {
      majorType = HTTPCommon::HTTP_VIDEO_TYPE;
      pPortInfo = &m_pHTTPSourceMMI->m_portVideo[portID - MMI_HTTP_VIDEO_PORT_INDEX];
    }
    else if(portID == MMI_HTTP_OTHER_PORT_INDEX)
    {
      majorType = HTTPCommon::HTTP_TEXT_TYPE;
      pPortInfo = &m_pHTTPSourceMMI->m_portOther[portID - MMI_HTTP_OTHER_PORT_INDEX];
    }
  }

  if (pPortInfo && trackHdlr)
  {
    HTTPDownloadStatus status = trackHdlr->CanPlayTrack(pPortInfo->m_trackId,
                                                        majorType);
    if (status == HTTPCommon::HTTPDL_SUCCESS)
    {
      watermark = QOMX_WATERMARK_NORMAL;
    }

    if (status != HTTPCommon::HTTPDL_ERROR_ABORT)
    {
      ret = MMI_S_COMPLETE;
    }
  }

  pBufferStatus->eCurrentWaterMark = watermark;
  pBufferStatus->nCurrentLevel = 0;
  pBufferStatus->eUnitsType = QOMX_WATERMARKUNITSTYPE_Time;

  if (m_pHTTPSourceMMI && m_pHTTPSourceMMI->m_pHTTPController && trackHdlr)
  {
    uint64 duration = 0;
    uint64 curPos = 0;

    bool isOk = m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper->GetDurationBuffered(
        portID, duration, curPos);

    if (false == isOk)
    {
      duration = 0;
    }

    pBufferStatus->nCurrentLevel = (uint32)(duration * 1000);

    QTV_MSG_PRIO4(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_HIGH,
      "GetCurrentWatermarkStatus port %u: watermarkType %d, level = %u pos %llu",
      portID, (int)pBufferStatus->eCurrentWaterMark, (uint32)pBufferStatus->nCurrentLevel, curPos);
  }

  return ret;
}

/**
 * @brief
 *  Get boolean value whether QOE notification is required to be sent.
 *
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionHandler::GetQOENotification()
{
  return m_eventHandler.m_HTTPQOEEventMgr.GetQOENotification();
}

/**
 * @brief
 *  Update QOE data for eventID passed.
 *
 * @param eventID
 */
void HTTPSourceMMIExtensionHandler::UpdateQOEData(const uint32 eventID)
{
  m_eventHandler.m_HTTPQOEEventMgr.UpdateQOEData(m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper,
                                                 eventID);
}

/**
 * @brief
 *  Notify QOE Event for eventID passed.
 *
 * @param eventID
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionHandler::NotifyQOEEvent(const uint32 eventID)
{
  return m_eventHandler.m_HTTPQOEEventMgr.NotifyQOEEvent(m_pHTTPSourceMMI->m_pHTTPSourceMMIHelper,
                                                         OMX_ALL,
                                                         eventID,
                                                         m_pHTTPSourceMMI->m_pClientData);
}
/**
 * @brief Retrieve the clip related Media info
 *
 * @param pMediaInfo
 *
 * @return MMI_S_COMPLETE indicating success
 *        Other indicating error
 *
 */
uint32 HTTPSourceMMIExtensionHandler::HandleMediaInfoQuery
(
  QOMX_MEDIAINFOTYPE *pMediaInfo
)
{
  OMX_U32 rsltCode = MMI_S_EBADPARAM;
  if (pMediaInfo == NULL || m_pHTTPSourceMMI == NULL)
  {
    return rsltCode;
  }

  if(!m_pHTTPSourceMMI->IsOpenComplete()&&
       (pMediaInfo->eTag != QOMX_MediaInfoDuration))
  {
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                       "Open didn't complete, returning failure");
    return MMI_S_EFAIL;
  }

  switch (pMediaInfo->eTag)
  {
    case QOMX_MediaInfoTagTrackNum:
      {
        if ((pMediaInfo->nPortIndex >= MMI_HTTP_PORT_START_INDEX  &&
             pMediaInfo->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX ))
        {
          uint32 trackID;
          if (m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->GetTrackID(pMediaInfo->nPortIndex,
                                                                         trackID))
          {
            memcpy(&pMediaInfo->cData,&trackID,sizeof(OMX_U32));
            pMediaInfo->nDataSize = (OMX_U32)sizeof(OMX_U32);
            rsltCode = MMI_S_COMPLETE;
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                          "failed to retrieve TrackID");
            rsltCode = MMI_S_EFAIL;
          }
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Invalid portIndex: %u", (uint32)pMediaInfo->nPortIndex);
          rsltCode = MMI_S_EBADPARAM;
        }
      }
      break;

    case QOMX_MediaInfoDuration:
      {
        if (pMediaInfo->nPortIndex == OMX_ALL ||
            (pMediaInfo->nPortIndex >= MMI_HTTP_PORT_START_INDEX  &&
             pMediaInfo->nPortIndex <= MMI_HTTP_OTHER_PORT_INDEX ))
        {

          OMX_U32  duration = 0;
          bool bOk = false;

          bOk=m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->GetMediaDuration
              (pMediaInfo->nPortIndex, duration);

          if(bOk)
          {
            OMX_TICKS durationTicks;
            OMX_U64 mediaDurtion = 0;
            mediaDurtion = (OMX_U64)duration*1000;
#ifndef OMX_SKIP64BIT
            durationTicks = (unsigned long long)mediaDurtion;
#else
            durationTicks.nLowPart = (unsigned long)mediaDurtion;
            durationTicks.nHighPart = (unsigned long)mediaDurtion >>32;
#endif
            memcpy(&pMediaInfo->cData,&durationTicks,sizeof(OMX_TICKS));
            pMediaInfo->nDataSize = (OMX_U32) sizeof(OMX_U32);
            rsltCode = MMI_S_COMPLETE;
          }
          else
          {
            QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,"failed to retrieve Media Duration");
            rsltCode = MMI_S_EFAIL;
          }
        }
        else
        {
          QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                         "Invalid PortIndex: %u", (uint32)pMediaInfo->nPortIndex);
          rsltCode = MMI_S_EBADPARAM;
        }
      }
      break;

     case QOMX_MediaInfoTagMediaStreamType:
    {
      //Get the Stream Type
      uint32 reqLen = (uint32)sizeof(QOMX_MEDIASTREAMTYPE);

      if (pMediaInfo->cData && pMediaInfo->nDataSize >= reqLen)
      {
        rsltCode = MMI_S_EFAIL;
        union
        {
          OMX_U8               *in;
          QOMX_MEDIASTREAMTYPE *out;
        }u;
        u.in = pMediaInfo->cData;
        QOMX_MEDIASTREAMTYPE *pStreamType = u.out;
        //pStreamType = (QOMX_MEDIASTREAMTYPE*)pMediaInfo->cData;
        pStreamType->eURIType=QOMX_URITYPE_HTTP;
        if (m_pHTTPSourceMMI &&
              m_pHTTPSourceMMI->m_pHTTPController)
        {
          bool isLive = FALSE;
          m_pHTTPSourceMMI->m_pHTTPController->IsLiveStreamingSession(isLive);
          if(isLive)
          {
            pStreamType->eStreamType=QOMX_STREAMTYPE_LIVE;
          }
          else
          {
            pStreamType->eStreamType=QOMX_STREAMTYPE_VOD;
          }
          rsltCode = MMI_S_COMPLETE;
        }
      }
      pMediaInfo->nDataSize = reqLen;
    }
    break;

    default:
      {
        QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPMMIExtensionHandler: Invalid MediaInfo etag:%d",pMediaInfo->eTag);
        rsltCode = MMI_S_EBADPARAM;
      }
      break;
  }
  return rsltCode;
}

/*
 * Get the protocol header value given the name.
 *
 * @param[in/out] protocolHeader - Reference to protocol header to get
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure
 */
uint32 HTTPSourceMMIExtensionHandler::GetProtocolHeader
(
 QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE& protocolHeader
)
{
  uint32 result = MMI_S_EBADPARAM;

  if (protocolHeader.nSize < sizeof(QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE))
  {
    QTV_MSG_PRIO2(QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
      "GetProtocolHeader: Invalid nSize %u, minSize %d",
       (uint32)protocolHeader.nSize, sizeof(QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE));
    return result;
  }

  //Note that messageHeader also includes terminating byte. protocolHeader structure looks like
  //<nSize = 4 bytes><nVersion = 4 bytes><eMessageType = 4 bytes><eActionType = 4 bytes>
  //<nMessageClassSize = 4 bytes><nHeaderNameSize = 4bytes><nHeaderValueSize = 4bytes>
  //<messageHeader string = nMessageClassSize + nHeaderNameSize + nHeaderValueSize + 1 bytes>.
  //Validate the header name size and nSize before copying the message header over
  if (protocolHeader.nHeaderNameSize > 0 &&
      protocolHeader.nSize >= (offsetof(QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE, messageHeader[0])) +
                               protocolHeader.nMessageClassSize + protocolHeader.nHeaderNameSize +
                               protocolHeader.nHeaderValueSize + 1)
  {
    //For now ONLY request type is supported
    if (protocolHeader.eMessageType == QOMX_STREAMING_PROTOCOLMESSAGE_REQUEST)
    {
      char methodName[m_nMaxHTTPMethodNameSize + 1] = {0};
      (void)std_strlcpy(methodName, (const char*)protocolHeader.messageHeader,
                        protocolHeader.nMessageClassSize + 1);

      //Check if the HTTP method is supported and extract the header name
      AffectedMethod httpMethod = GetAffectedHTTPMethod(methodName);
      if (httpMethod != HTTP_METHOD_NONE && httpMethod != HTTP_METHOD_ALL)
      {
        result = MMI_S_EFAIL;
        char* pHdrName = (char*)QTV_Malloc(sizeof(char) * (protocolHeader.nHeaderNameSize + 1));
        if (pHdrName)
        {
          (void)std_strlcpy(pHdrName,
                            (const char*)(protocolHeader.messageHeader + protocolHeader.nMessageClassSize),
                            protocolHeader.nHeaderNameSize + 1);

          //Get the protocol header value for pHdrName
          HTTPController *pController = m_pHTTPSourceMMI->GetController();
          char *pHeaderValue = NULL;
          int headervalueSize = 0;
          // query the reqd buf size for header value
          if (pController &&
              pController->GetOemHttpHeaders(httpMethod, pHdrName,
                                             pHeaderValue, headervalueSize))
          {
            int reqdBufSize = headervalueSize;
            if (protocolHeader.nHeaderValueSize && reqdBufSize)
            {
              //Make sure the messageHeader field is big enough to hold the header value,
              //else truncate data
              pHeaderValue = (char *) protocolHeader.messageHeader +
                                      protocolHeader.nMessageClassSize +
                                      protocolHeader.nHeaderNameSize;
              headervalueSize = (int)protocolHeader.nHeaderValueSize + 1;

              // populate the header value.
              if (pController &&
                  pController->GetOemHttpHeaders(httpMethod, pHdrName,
                                                 pHeaderValue, headervalueSize))
              {
                result = MMI_S_COMPLETE;
              }
            }
            //reqdBufSize == 1, means header IS present but value length is zero
            else if (protocolHeader.nHeaderValueSize == 0 || reqdBufSize == 1)
            {
              result = MMI_S_COMPLETE;
            }
            protocolHeader.nHeaderValueSize = headervalueSize;
          }
          else // oem header name NOT available
          {
             result = MMI_S_EBADPARAM;
          }

          QTV_Free(pHdrName);
        }// if (pHdrName)
      }
      else // unsupported HTTP method
      {
        result = MMI_S_EBADPARAM;
      }
    }
  }
  else
  {
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "HTTPSourceMMIExtensionHandler Invalid input parameter - "
                   "nSize %u, nMessageClassSize %u, nHeaderNameSize %u, nHeaderValueSize %u",
                   (uint32)protocolHeader.nSize, (uint32)protocolHeader.nMessageClassSize,
                   (uint32)protocolHeader.nHeaderNameSize, (uint32)protocolHeader.nHeaderValueSize );
  }

  return result;
}

/*
 * Validate and set the protocol header.
 *
 * @param[in] protocolHeader - Reference to protocol header to be set
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure
 */
uint32 HTTPSourceMMIExtensionHandler::SetProtocolHeader
(
 const QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE& protocolHeader
)
{
  uint32 result = MMI_S_EBADPARAM;
  size_t msgHdrLen = std_strlen((const char*)protocolHeader.messageHeader);

  //Note that messageHeader also includes terminating byte. protocolHeader structure looks like
  //<nSize = 4 bytes><nVersion = 4 bytes><eMessageType = 4 bytes><eActionType = 4 bytes>
  //<nMessageClassSize = 4 bytes><nHeaderNameSize = 4bytes><nHeaderValueSize = 4bytes>
  //<messageHeader string = nMessageClassSize + nHeaderNameSize + nHeaderValueSize + 1 bytes>.
  //Validate the header name size and nSize before copying the message header over
  if (protocolHeader.nHeaderNameSize > 0 &&
      protocolHeader.nSize >= (offsetof(QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE, messageHeader[0])) +
                               protocolHeader.nMessageClassSize + protocolHeader.nHeaderNameSize +
                               protocolHeader.nHeaderValueSize + 1)
  {
    //For now ONLY request type is supported for adding/removing headers
    if (protocolHeader.eMessageType == QOMX_STREAMING_PROTOCOLMESSAGE_REQUEST &&
        protocolHeader.eActionType != QOMX_STREAMING_PROTOCOLHEADERACTION_NONE)
    {
      char methodName[m_nMaxHTTPMethodNameSize + 1] = {0};
      (void)std_strlcpy(methodName, (const char*)protocolHeader.messageHeader,
                        protocolHeader.nMessageClassSize + 1);

      //Check if the HTTP method is supported and extract the header name and value
      AffectedMethod httpMethod = GetAffectedHTTPMethod(methodName);
      if (httpMethod != HTTP_METHOD_NONE)
      {
        result = MMI_S_EFAIL;
        IPStreamProtocolHeaderCommand cmd = IPSTREAM_PROTOCOL_HEADER_NONE;
        if (protocolHeader.eActionType == QOMX_STREAMING_PROTOCOLHEADERACTION_ADD)
        {
          //Replace if already present
          cmd = IPSTREAM_PROTOCOL_HEADER_REPLACE;
        }
        else if (protocolHeader.eActionType == QOMX_STREAMING_PROTOCOLHEADERACTION_REMOVE)
        {
          cmd = IPSTREAM_PROTOCOL_HEADER_DELETE;
        }

        char* pHdrName = (char*)QTV_Malloc(sizeof(char) * (protocolHeader.nHeaderNameSize + 1));
        if (pHdrName)
        {
          (void)std_strlcpy(pHdrName,
                            (const char*)(protocolHeader.messageHeader + protocolHeader.nMessageClassSize),
                            protocolHeader.nHeaderNameSize + 1);
        }
        char* pHdrValue = NULL;
        if (cmd == IPSTREAM_PROTOCOL_HEADER_REPLACE)
        {
          pHdrValue = (char*)QTV_Malloc(sizeof(char) * (protocolHeader.nHeaderValueSize + 1));
          if (pHdrValue)
          {
            (void)std_strlcpy(pHdrValue,
                              (const char*)(protocolHeader.messageHeader + protocolHeader.nMessageClassSize +
                                            protocolHeader.nHeaderNameSize),
                              protocolHeader.nHeaderValueSize + 1);
          }
        }

        //Set the protocol header pHdrName<->pHdrValue pair (add or remove)
        HTTPController *pController = m_pHTTPSourceMMI->GetController();
        if (pController &&
            pController->SetOemHttpHeaders(cmd, httpMethod, pHdrName, pHdrValue))
        {
          result = MMI_S_COMPLETE;
        }

        if (pHdrName)
        {
          QTV_Free(pHdrName);
          pHdrName = NULL;
        }
        if (pHdrValue)
        {
          QTV_Free(pHdrValue);
          pHdrValue = NULL;
        }
      }// if (httpMethod != HTTP_METHOD_NONE)
    }
  }
  else
  {
    QTV_MSG_PRIO4( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_ERROR,
                   "HTTPSourceMMIExtensionHandler Invalid input parameter - "
                   "nSize %u, nMessageClassSize %u, nHeaderNameSize %u, nHeaderValueSize %u",
                   (uint32)protocolHeader.nSize, (uint32)protocolHeader.nMessageClassSize,
                   (uint32)protocolHeader.nHeaderNameSize, (uint32)protocolHeader.nHeaderValueSize );
  }

  return result;
}

/*
 * Check if given HTTP method is supported and return the method enum.
 *
 * @param[in] pMethodName - Reference to method name
 *
 * @return httpMethod
 */
AffectedMethod HTTPSourceMMIExtensionHandler::GetAffectedHTTPMethod
(
 const char* pMethodName
)
{
  AffectedMethod httpMethod = HTTP_METHOD_NONE;

  if (pMethodName)
  {
    for (unsigned int i = 0; i < ARR_SIZE(m_supportedHTTPMethodTable); i++)
    {
      if (std_strnicmp(pMethodName,
                       m_supportedHTTPMethodTable[i].methodName,
                       std_strlen(m_supportedHTTPMethodTable[i].methodName)) == 0)
      {
        httpMethod = m_supportedHTTPMethodTable[i].eMethod;
        break;
      }
    }
  }

  return httpMethod;
}

/**
 * @brief This Methods sets the download Progress units type
 * for a given port
 *
 * @param portIndex
 * @param eUnitsType
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionHandler::SetDownloadProgressUnitsType
(
  OMX_U32 portIndex,
  QOMX_DOWNLOADPROGRESSUNITSTYPE eUnitsType
)
{
  bool retVal = false;

  if(portIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
     portIndex <= MMI_HTTP_OTHER_PORT_INDEX)
  {
    if(eUnitsType == QOMX_DOWNLOADPROGRESSUNITSTYPE_DATA)
    {
       m_eDownloadProgressUnits[portIndex] = HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_DATA;
       retVal = true;
    }
    else if(eUnitsType == QOMX_DOWNLOADPROGRESSUNITSTYPE_TIME)
    {
      m_eDownloadProgressUnits[portIndex] = HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_TIME;
      retVal = true;
    }
    else
    {
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                      "HTTPSourceMMIExtensionHandler:Invalid DownloadProgress UnitsType:%d",
                      eUnitsType);
    }
  }

  return retVal;
}


/**
 * @brief This Methods returns the download Progress units type
 * for a given port
 *
 * @param portIndex
 * @param eUnitsType
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionHandler::GetDownloadProgressUnitsType
(
  OMX_U32 portIndex,
  QOMX_DOWNLOADPROGRESSUNITSTYPE& eUnitsType
)
{
  bool retVal = false;

  if(portIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
     portIndex <= MMI_HTTP_OTHER_PORT_INDEX)
  {
    if(m_eDownloadProgressUnits[portIndex] == HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_DATA)
    {
      eUnitsType = QOMX_DOWNLOADPROGRESSUNITSTYPE_DATA;
      retVal = true;
    }
    else if(m_eDownloadProgressUnits[portIndex] == HTTPCommon::HTTP_DOWNLOADPROGRESS_UNITS_TIME)
    {
      eUnitsType = QOMX_DOWNLOADPROGRESSUNITSTYPE_TIME;
      retVal = true;
    }
  }

  return retVal;
}

/**
 * This methods returns downloadprogress for track, if
 *        request if for all ports , then minimum
 *        downloadprogress of all the tracks
 *
 * @param portIndex
 * @param currStartOffset
 * @param downloadOffset
 *
 * @return bool
 */
bool HTTPSourceMMIExtensionHandler::GetDownloadProgress
(
  OMX_U32 portIndex,
  uint32& currStartOffset,
  uint32& downloadOffset
)
{
  bool nReturn = false;
  bool bEOS = false;
  currStartOffset = 0;
  downloadOffset = 0;

  if(portIndex >= MMI_HTTP_AUDIO_PORT_INDEX &&
     portIndex <= MMI_HTTP_OTHER_PORT_INDEX)
  {
    HTTPMediaType mediaType;
    m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->MapPortIDToMediaType(portIndex,mediaType);

    if(mediaType != HTTPCommon::HTTP_UNKNOWN_TYPE )
    {
      nReturn = m_pHTTPSourceMMI->m_pHTTPController->GetDownloadProgress(mediaType,
                                                                        currStartOffset,
                                                                        downloadOffset,
                                                                         m_eDownloadProgressUnits[portIndex],
                                                                         bEOS);
    }
  }
  else if(portIndex == OMX_ALL)
  {
    uint32 nAudioCurrentStartOffset = 0;
    uint32 nAudioDownloadOffset= 0;
    uint32 nVideoCurrentStartOffset= 0;
    uint32 nVideoDownloadOffset= 0;

    nReturn = m_pHTTPSourceMMI->m_pHTTPController->GetDownloadProgress(HTTPCommon::HTTP_AUDIO_TYPE,
                                                                        nAudioCurrentStartOffset,
                                                                        nAudioDownloadOffset,
                                                                       m_eDownloadProgressUnits[MMI_HTTP_AUDIO_PORT_INDEX],
                                                                       bEOS);
    if(nReturn)
    {
      nReturn = m_pHTTPSourceMMI->m_pHTTPController->GetDownloadProgress(HTTPCommon::HTTP_VIDEO_TYPE,
                                                                        nVideoCurrentStartOffset,
                                                                        nVideoDownloadOffset,
                                                                         m_eDownloadProgressUnits[MMI_HTTP_VIDEO_PORT_INDEX],
                                                                         bEOS);
    }

    if(nReturn)
    {
      currStartOffset = STD_MIN(nAudioCurrentStartOffset,nVideoCurrentStartOffset);
      downloadOffset = STD_MIN(nAudioDownloadOffset,nVideoDownloadOffset);
    }
  }

  if(!nReturn)
  {
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM,
                  "HTTPSourceMMIExtensionHandler:GetDownloadProgress failed - "
                  "could be that download hasn't started, reporting zero values");
    nReturn = true;
  }

  return nReturn;
}

/*
 * Get the syntax header for given port index.
 *
 * @param[in/out] syntaxHeader - Reference to video syntax header to get
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure
 */
uint32 HTTPSourceMMIExtensionHandler::GetSyntaxHeader
(
 QOMX_VIDEO_SYNTAXHDRTYPE& syntaxHeader
)
{
  uint32 result = MMI_S_EBADPARAM;
  HTTPMediaType mediaType = HTTPCommon::HTTP_UNKNOWN_TYPE;
  HTTPSourceMMITrackHandler* pTrackHandler = NULL;

  if ((NULL == m_pHTTPSourceMMI) ||
      (NULL == m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler) ||
      !m_pHTTPSourceMMI->IsValidPort(syntaxHeader.nPortIndex))
  {
    return result;
  }

  pTrackHandler = m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler;

  pTrackHandler->MapPortIDToMediaType(syntaxHeader.nPortIndex, mediaType);

  if (mediaType != HTTPCommon::HTTP_UNKNOWN_TYPE)
  {
    result = pTrackHandler->GetFormatBlock(mediaType,
                                           syntaxHeader.data,
                                           (uint32 &)syntaxHeader.nBytes);
  }
  else
  {
    QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "HTTPSourceMMIExtensionHandler Media track unknown");
  }

  return result;
}

/*
 * Get the PSSH info for given port index.
 *
 * @param portIndex
 * @param pPSSHInfo
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure

 */

uint32 HTTPSourceMMIExtensionHandler::GetPsshInfo
(
  OMX_U32 portIndex ,
  QOMX_PARAM_STREAMING_PSSHINFO* pPSSHInfo
)
{
  uint32 ret = MMI_S_COMPLETE;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  if (pPSSHInfo == NULL)
  {
    ret  = MMI_S_EBADPARAM;
    QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
    "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pPSSHInfo );
  }
  else
  {
   if (0 == pPSSHInfo->nPsshDataBufSize)
   {
     QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for PsshInfo : size");
     int uniqueId = (int)pPSSHInfo->nUniqueID;
     eStatus= m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->GetPsshInfo(
               (uint32)portIndex,
               uniqueId,
               pPSSHInfo->cDefaultKeyID,
               (uint32&)pPSSHInfo->nPsshDataBufSize,
               pPSSHInfo->cPSSHData,
               true);
     pPSSHInfo->nUniqueID = (OMX_S32)uniqueId;
     if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
     {
      ret = MMI_S_EFAIL;
     }
   }
   else  if (pPSSHInfo->nSize >= (offsetof(QOMX_PARAM_STREAMING_PSSHINFO, cPSSHData[0])) +
                                 pPSSHInfo->nPsshDataBufSize)
   {
     QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for PsshInfo : data");
     int uniqueId = (int)pPSSHInfo->nUniqueID;
     eStatus= m_pHTTPSourceMMI->m_pHTTPSourceMMITrackHandler->GetPsshInfo(
                            (uint32)portIndex,
                            uniqueId,
                            pPSSHInfo->cDefaultKeyID,
                            (uint32 &)pPSSHInfo->nPsshDataBufSize,
                            pPSSHInfo->cPSSHData);
     pPSSHInfo->nUniqueID = (OMX_S32)uniqueId;
     if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
     {
      ret = MMI_S_EFAIL;
     }
   }
 }
 return ret;
}

/*
 * Get the Current Playing representation Info.
 *
 * @param pCurrRepInfo
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure

 */

uint32 HTTPSourceMMIExtensionHandler::GetCurrentPlayingRepInfo(QOMX_PARAM_REPRESENTATION *pCurrRepInfo)
{
  uint32 ret = MMI_S_COMPLETE;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  if (pCurrRepInfo == NULL)
  {
    ret  = MMI_S_EBADPARAM;
    QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
    "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pCurrRepInfo );
  }
  else
  {
    if (0 == pCurrRepInfo->nRepSize)
    {
      eStatus = m_pHTTPSourceMMI->m_pHTTPDataInterface->GetCurrentPlayingRepInfo(
             (uint32 &)pCurrRepInfo->nRepSize);
      QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for current Playing Representation  : size %u",
                   (uint32)pCurrRepInfo->nRepSize);

      if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
      {
        ret = MMI_S_EFAIL;
      }
    }
    else  if (pCurrRepInfo->nSize >= (offsetof(QOMX_PARAM_REPRESENTATION, cRepString[0])) +
                                 pCurrRepInfo->nRepSize)
    {
      if (pCurrRepInfo->nRepSize > 0)
      {
        eStatus = m_pHTTPSourceMMI->m_pHTTPDataInterface->GetCurrentPlayingRepInfo(
                  (uint32 &)pCurrRepInfo->nRepSize,
                  pCurrRepInfo->cRepString);
        QTV_MSG_SPRINTF_PRIO_2( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for current Playing Representation : \
                                size %u, value %s", (uint32) pCurrRepInfo->nRepSize, pCurrRepInfo->cRepString);

      }
      if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
      {
        ret = MMI_S_EFAIL;
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for media Mime : Invalid");
      ret = MMI_S_EFAIL;
    }
  }
  return ret;
}

/*
 * Get the initial track encoding info from MPD.
 *
 * @param pCurrRepInfo
 *
 * @return MMI_S_COMPLETE on success,
 *         MM_S_EBADPARAM if bad input paramater,
 *         MMI_S_EFAIL on generic failure
 */

uint32 HTTPSourceMMIExtensionHandler::GetTrackEncodingInfo(QOMX_PARAM_MEDIA_TRACK_ENCODING *pTrackEncoding)
{
  uint32 ret = MMI_S_COMPLETE;
  HTTPDownloadStatus eStatus = HTTPCommon::HTTPDL_SUCCESS;
  if (pTrackEncoding == NULL)
  {
    ret  = MMI_S_EBADPARAM;
    QTV_MSG_PRIO1( QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
    "HTTPMMIExtensionHandler Invalid OMX Parameter 0x%p", (void *)pTrackEncoding );
  }
  else
  {
    if (pTrackEncoding->nSize >= sizeof(QOMX_PARAM_MEDIA_TRACK_ENCODING))
    {
      pTrackEncoding->audioEncoding = OMX_AUDIO_CodingUnused;
      pTrackEncoding->videoEncoding = OMX_VIDEO_CodingUnused;
      pTrackEncoding->otherEncoding = QOMX_OTHER_CodingUnused;

      if (pTrackEncoding->nSize > 0)
      {
        FileSourceMnMediaType audio = FILE_SOURCE_MN_TYPE_UNKNOWN;
        FileSourceMnMediaType video = FILE_SOURCE_MN_TYPE_UNKNOWN;
        FileSourceMnMediaType other = FILE_SOURCE_MN_TYPE_UNKNOWN;

        eStatus = m_pHTTPSourceMMI->m_pHTTPDataInterface->GetTrackEncodingInfo(audio, video, other);
        QTV_MSG_PRIO3( QTVDIAG_HTTP_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for TrackEncodingInfo : \
                                audio %d, video %d, text %d",  audio, video, other);

        pTrackEncoding->audioEncoding = m_pHTTPSourceMMI->GetOmxAudioMinorType(audio);
        pTrackEncoding->videoEncoding = m_pHTTPSourceMMI->GetOmxVideoMinorType(video);
        pTrackEncoding->otherEncoding = m_pHTTPSourceMMI->GetOmxOtherMinorType(other);
      }

      if (eStatus != HTTPCommon::HTTPDL_SUCCESS)
      {
        ret = MMI_S_EFAIL;
      }
    }
    else
    {
      QTV_MSG_PRIO( QTVDIAG_STREAMING, QTVDIAG_PRIO_MEDIUM, "Query for media Mime : Invalid");
      ret = MMI_S_EFAIL;
    }
  }
  return ret;
}

} /*namespace video*/
