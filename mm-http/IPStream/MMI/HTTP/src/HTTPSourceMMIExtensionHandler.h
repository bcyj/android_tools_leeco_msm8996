#ifndef HTTPSOURCEMMIEXTENSIONHANDLER_H
#define HTTPSOURCEMMIEXTENSIONHANDLER_H
/************************************************************************* */
/**
 * HTTPSourceMMIExtensionHandler.h
 * @brief Defines the HTTPSource MMI Extensions Handler
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/MMI/HTTP/dev/DASH/src/HTTPSourceMMIExtensionHandler.h#15 $
$DateTime: 2013/01/10 04:20:24 $
$Change: 3214068 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "httpInternalDefs.h"
#include "QOMX_StreamingExtensions.h"
#include "HTTPSourceMMIExtensionEventHandler.h"
#include "QOMX_VideoExtensions.h"
#include "QOMX_AudioExtensions.h"
#include "QOMX_IVCommonExtensions.h"
#include "QOMX_SourceExtensions.h"
#include "filesourcetypes.h"
#include "mmiDeviceApi.h"
#include "IPStreamProtocolHeaders.h"
#include "QOMX_StreamingExtensionsPrivate.h"

namespace video {

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

//forward  class decleration
class HTTPSourceMMI;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
//Qualcomm vendor extension indices for streaming source component.
typedef enum QOMX_HTTP_STREAMING_INDEXTYPE
{
  QOMX_HTTP_IndexParam_StreamingStartUnused = OMX_IndexVendorStartUnused,
  QOMX_HTTP_IndexStreamingConfigWaterMark,            /** <refernce: OMX_QUALCOMM_INDEX_CONFIG_WATERMARK */
  QOMX_HTTP_IndexStreamingConfigWaterMarkStatus,       /** <reference: OMX_QUALCOMM_INDEX_CONFIG_WATERMARKSTATUS*/
  QOMX_HTTP_IndexParamStreamingNetworkInterface,  /**<reference: QOMX_PARAM_STREAMING_NETWORKINTERFACE */
  QOMX_HTTP_IndexParamStreamingNetworkProfile,  /**<reference: OMX_PARAM_U32TYPE */
  QOMX_HTTP_IndexParamStreamingProxyServer,  /**<reference: OMX_PARAM_CONTENTURITYPE */
  QOMX_HTTP_IndexConfigStreamingProtocolHeader, /**<reference: QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE */
  QOMX_HTTP_IndexParamStreamingSyntaxHeader,    /**<reference: QOMX_VIDEO_SYNTAXHDRTYPE */
  QOMX_HTTP_IndexConfigStreamingProtocolEvent,  /**<reference: QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE */
  QOMX_HTTP_IndexParamVideoDivx, /**<reference: QOMX_VIDEO_PARAM_DIVXTYPE */
  QOMX_HTTP_IndexParamVideoVp6, /**<reference: QOMX_VIDEO_PARAM_VPTYPE */
  QOMX_HTTP_IndexParamVideoSpark, /**<reference: QOMX_VIDEO_PARAM_SPARKTYPE */
  QOMX_HTTP_IndexParamVideoVc1, /**<reference: QOMX_VIDEO_PARAM_VC1TYPE */
  QOMX_HTTP_IndexParamAudioEvrcb, /**<reference: OMX_AUDIO_PARAM_EVRCTYPE */
  QOMX_HTTP_IndexParamAudioEvrcWb, /**<reference: OMX_AUDIO_PARAM_EVRCTYPE */
#ifdef FEATURE_HTTP_WM
  QOMX_HTTP_IndexParamAudioWmaPro, /**<reference: QOMX_AUDIO_PARAM_WMATYPE */
#endif
  QOMX_HTTP_IndexConfigMediaInfo,  /**<reference: OMX_QCOM_INDEX_CONFIG_MEDIAINFO */
  QOMX_HTTP_IndexParamSeekAccess,  /** <reference: OMX_QCOM_INDEX_PARAM_SEEK_ACCESS */
#ifdef FEATURE_HTTP_AMR
  QOMX_HTTP_IndexParamAmrWbPlus,    /** <reference: OMX_QCOM_INDEX_PARAM_AMRWBPLUS */
#endif // FEATURE_HTTP_AMR
  QOMX_HTTP_IndexParamFrameSizeQuerySupported,  /**<reference: OMX_QCOM_INDEX_PARAM_FRAMESIZEQUERYSUPPORTED */
  QOMX_HTTP_IndexConfigStreamingDownloadProgressUnitsType,   /** <reference: OMX_QCOM_INDEX_CONFIG_STREAMING_DOWNLOADPROGRESSUNITSTYPE */
  QOMX_HTTP_IndexConfigStreamingDownloadProgress, /** <reference: OMX_QOMX_INDEX_CONFIG_STREAMING_DOWNLOADPROGRESS */
  QOMX_HTTP_IndexConfigStreamingProtocolHeadersEvent, /**<reference: QOMX_CONFIG_STREAMING_PROTOCOLHEADERSTYPE*/
  QOMX_HTTP_IndexParamCompleteDashAdaptationProperties,       /**<reference: OMX_QUALCOMM_INDEX_PARAM_COMMON_DASH_ADAPTATION_PROPERTIES */
  QOMX_HTTP_IndexParamSelectedDashAdaptationProperties,        /**<reference: OMX_QUALCOMM_INDEX_PARAM_SELECTED_DASH_ADAPTATION_PROPERTIES */
  QOMX_HTTP_IndexParamSMPTETimeTextDimensionExtraData,  /**<reference: OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_DIMENSIONS */
  QOMX_HTTP_IndexParamSMPTETimeTextSubInfoExtraData,  /**<reference: OMX_QUALCOMM_INDEX_PARAM_SMPTE_TIME_TEXT_SUBINFO */
  QOMX_HTTP_IndexParamDrmInfo,                  /**<reference: QOMX_PARAM_STREAMING_DRMINFO */
  QOMX_HTTP_IndexParamContentProtectionInfo,    /**<reference: QOMX_PARAM_STREAMING_CONTENTPROTECTIONINFO */
  QOMX_HTTP_IndexParamPsshInfo,                  /**<reference: QOMX_PARAM_STREAMING_PSSHINFO */
  QOMX_HTTP_IndexParamExtraSampleInfo,          /**<reference: OMX_EXTRA_SAMPLE_INFO */
  QOMX_HTTP_IndexParamDashMPD,                  /**<reference: OMX_QUALCOMM_INDEX_PARAM_DASH_MPD */
  QOMX_HTTP_IndexParamQOE,                      /**<reference: OMX_QUALCOMM_INDEX_PARAM_QOE */
  QOMX_HTTP_IndexParamQOEPlay,                  /**<reference: OMX_QUALCOMM_INDEX_PARAM_QOE_PLAY */
  QOMX_HTTP_IndexParamQOEStop,                  /**<reference: OMX_QUALCOMM_INDEX_PARAM_QOE_STOP */
  QOMX_HTTP_IndexParamQOESwitch,                /**<reference: OMX_QUALCOMM_INDEX_PARAM_QOE_SWITCH */
  QOMX_HTTP_IndexParamQOEPeriodic,              /**<reference: OMX_QUALCOMM_INDEX_PARAM_QOE_PERIODIC */
  QOMX_HTTP_IndexParamCurrentRepresenation,      /**<reference: QOMX_PARAM_REPRESENTATION */
  QOMX_HTTP_IndexConfigDASHResumeDiscontinuity, /**<reference: OMX_QUALCOMM_INDEX_CONFIG_DASH_RESUME_DISCONTINUITY */
  QOMX_HTTP_IndexConfigDASHRepositionRange,      /**<reference: OMX_QUALCOMM_INDEX_CONFIG_DASH_REPOSITION_RANGE */
  QOMX_HTTP_IndexParamAudioAc3,                 /**< reference: QOMX_AUDIO_PARAM_AC3PROFILETYPE */
  QOMX_HTTP_IndexParamAudioMp2,                 /**< reference: QOMX_AUDIO_PARAM_MP2TYPE */
  QOMX_HTTP_IndexParamCookie,
  QOMX_HTTP_IndexParamBufferedDuration,
  QOMX_HTTP_IndexParamMediaTrackEncoding
} QOMX_HTTP_STREAMING_INDEXTYPE;

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */


class HTTPSourceMMIExtensionHandler
{
public:
  //c'tor and d'tor and intialization
  HTTPSourceMMIExtensionHandler();
  virtual ~HTTPSourceMMIExtensionHandler();
  bool Initialize(HTTPSourceMMI *pHTTPSourceMMI);
  void Reset();

  //processing extensions
  uint32 ProcessMMIGetExtensionCommand(MMI_GetExtensionCmdType *pMMIGetExtnCmd);
  uint32 ProcessMMIGetStdExtnParam(MMI_OmxParamCmdType* pOmxParam);
  uint32 ProcessMMISetStdExtnParam(MMI_OmxParamCmdType* pOmxParam);

  //extension related methods
  uint32 SetHTTPProtocolEvent(void *protocolEventInfo);
  uint32 SetHTTPProtocolHeadersEvent(void *protocolHeadersEventInfo);
  bool SetHTTPBufferingStatus(const uint32 portID,
                              bool bDataAvailable);

  void NotifyWatermarkEvent(uint32 portIdxAndWatermarkType);

  uint32 GetCurrentWatermarkStatus(uint32 portID,
                                 QOMX_BUFFERINGSTATUSTYPE* pBufferStatus);

  bool GetQOENotification();
  void UpdateQOEData(const uint32 eventID);
  bool NotifyQOEEvent(const uint32 eventID);
private:
  //Map to pair up the extension name to the extension string
  struct HTTPSourceMMIExtnMap
  {
    OMX_STRING pExtensionName;
    QOMX_HTTP_STREAMING_INDEXTYPE nExtensionIndex;
  };

  //Map to pair up HTTP method name to the enum value
  static const int m_nMaxHTTPMethodNameSize = 16;
  struct AffectedHTTPMethodPair
  {
    const char *methodName;
    AffectedMethod eMethod;
  };
  static const AffectedHTTPMethodPair m_supportedHTTPMethodTable[];

  //auxialury methods (internal methods)
  uint32 GetHTTPProtocolEvent(QOMX_CONFIG_STREAMING_PROTOCOLEVENTTYPE& protocolEvent);
   uint32 GetHTTPProtocolHeadersEvent(QOMX_STREAMING_PROTOCOLHEADERSTYPE& protocolHeadersEvent);
  uint32 HandleMediaInfoQuery(QOMX_MEDIAINFOTYPE *pMediaInfo);
  uint32 GetProtocolHeader(QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE& protocolHeader);
  uint32 SetProtocolHeader(const QOMX_CONFIG_STREAMING_PROTOCOLHEADERTYPE& protocolHeader);
  AffectedMethod GetAffectedHTTPMethod(const char* pMethodName);
  bool SetDownloadProgressUnitsType(OMX_U32 portIndex,
                                    QOMX_DOWNLOADPROGRESSUNITSTYPE eUnitsType);
  bool GetDownloadProgressUnitsType(OMX_U32 portIndex,
                                    QOMX_DOWNLOADPROGRESSUNITSTYPE &eUnitsType);
  bool GetDownloadProgress(OMX_U32 portIndex,
                           uint32& currStartOffset,
                           uint32& downloadOffset);
  uint32 GetSyntaxHeader(QOMX_VIDEO_SYNTAXHDRTYPE& syntaxHeader);

  uint32 GetPsshInfo(OMX_U32 portIndex ,  QOMX_PARAM_STREAMING_PSSHINFO* pPSSHInfo);

  uint32 GetCurrentPlayingRepInfo(QOMX_PARAM_REPRESENTATION *pCurrRepInfo);

  uint32 GetTrackEncodingInfo(QOMX_PARAM_MEDIA_TRACK_ENCODING *pTrackEncoding);

private:
  static const HTTPSourceMMIExtnMap m_HttpStreamingExtnMap[];

  HTTPSourceMMI* m_pHTTPSourceMMI;

  friend class HTTPSourceMMIExtensionEventHandler;
  HTTPSourceMMIExtensionEventHandler m_eventHandler;

  HTTPDownloadProgressUnitsType m_eDownloadProgressUnits[MMI_HTTP_NUM_MAX_PORTS+1];
};

}
#endif /* HTTPSOURCEMMIEXTENSIONHANDLER_H*/
