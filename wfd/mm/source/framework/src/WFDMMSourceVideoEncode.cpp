/*==============================================================================
*       WFDMMSourceVideoEncode.cpp
*
*  DESCRIPTION:
*       Provides interface for using OMX APIs to interact with encoder component
*
*
*  Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
12/27/2013                    InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/

#include "WFDMMSourceVideoEncode.h"
#include "wfd_cfg_parser.h"
#include "common_log.h"
#include "MMMemory.h"
#include "WFDMMLogs.h"
#include "OMX_Component.h"
#include "OMX_IndexExt.h"
#include "OMX_VideoExt.h"
#include "OMX_QCOMExtns.h"
#include "HardwareAPI.h"
#include "MMTimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include "QComOMXMetadata.h"

#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/DisplayInfo.h>

#include "WFDUtils.h"
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WFDMMSRCVENC"
#endif

/*------------------------------------------------------------------------------
 State definitions
--------------------------------------------------------------------------------
*/
#define DEINIT  0
#define INIT    1
#define OPENED  2
#define CLOSING 4
#define CLOSED  3


/*------------------------------------------------------------------------------
 Macros to Update states
--------------------------------------------------------------------------------
*/
#define ISSTATEDEINIT  (state(0, true) == DEINIT)
#define ISSTATEINIT    (state(0, true) == INIT)
#define ISSTATEOPENED  (state(0, true) == OPENED)
#define ISSTATECLOSED  (state(0, true) == CLOSED)
#define ISSTATECLOSING (state(0, true) == CLOSING)

#define SETSTATECLOSING (state(CLOSING, false))
#define SETSTATEINIT    (state(INIT   , false))
#define SETSTATEDEINIT  (state(DEINIT , false))
#define SETSTATEOPENED  (state(OPENED , false))
#define SETSTATECLOSED  (state(CLOSED , false))



#define NUMBER_OF_BFRAME 0
#define WFD_VENC_NUM_ENC_IP_BUF 5
#define WFD_VENC_NUM_ENC_OP_BUF 2
#define WFD_H264_PROFILE_CONSTRAINED_HIGH 2

#define WFD_VENC_Log2(number, power)  { OMX_U32 temp = number; power = 0; while( (0 == (temp & 0x1)) &&  power < 16) { temp >>=0x1; power++; } }
#define WFD_VENC_FractionToQ16(q,num,den) { OMX_U32 power; WFD_VENC_Log2(den,power); q = num << (16 - power); }

static char NonSecureEncoderComp[] = "OMX.qcom.video.encoder.avc";
static char SecureEncoderComp   [] = "OMX.qcom.video.encoder.avc.secure";

#ifndef OMX_SPEC_VERSION
#define OMX_SPEC_VERSION 0x00000101
#endif
#define OMX_INIT_STRUCT(_s_, _name_)            \
    memset((_s_), 0x0, sizeof(_name_));          \
    (_s_)->nSize = sizeof(_name_);               \
    (_s_)->nVersion.nVersion = OMX_SPEC_VERSION

#define WFD_VENC_CRITICAL_SECT_ENTER(critSect) if(critSect)                 \
                                      MM_CriticalSection_Enter(critSect);
    /*      END CRITICAL_SECT_ENTER    */

#define WFD_VENC_CRITICAL_SECT_LEAVE(critSect) if(critSect)                 \
                                      MM_CriticalSection_Leave(critSect);
    /*      END CRITICAL_SECT_LEAVE    */

#define WFD_VENC_FREEIF(x) if(x){ MM_Free(x);x=NULL;}
#define WFD_VENC_DELETEIF(x) if(x){ MM_Delete(x);x=NULL;}

//This macro provides a single point exit from function on
//encountering any error
#define WFD_OMX_ERROR_CHECK(result,error) ({ \
    if(result!= OMX_ErrorNone) \
    {\
        WFDMMLOGE(error);\
        WFDMMLOGE1("due to %x", result);\
        goto EXIT;\
    }\
})

/*=============================================================================

         FUNCTION:          dumpVencStats

         DESCRIPTION:
*//**       @brief          used for dumping statistics realted to encoder component
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

void inline WFDMMSourceVideoEncode::dumpVencStats() const
{
    WFDMMLOGM4("ETB = %lld EBD = %lld FTB = %lld FBD = %lld",
    mWfdVencStats.nETB,mWfdVencStats.nEBD,
    mWfdVencStats.nFTB,mWfdVencStats.nFBD);
}

/*=============================================================================

         FUNCTION:          IS_DUMP_ENABLE

         DESCRIPTION:
*//**       @brief          helper function to enable h264 dumping
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

inline OMX_BOOL IS_DUMP_ENABLE() {
    int ret = 0;
    char szTemp[PROPERTY_VALUE_MAX];
    // For legacy purposes leaving it as v4l2
    ret = property_get("persist.debug.wfd.dumpv4l2",szTemp,NULL);
    if (ret <= 0 )
    {
        WFDMMLOGE2("Failed to read prop %d %s value", ret, szTemp);
        return OMX_FALSE;
    }
    if(strcmp(szTemp,"1")==0)
    {
        WFDMMLOGD("IS_DUMP_ENABLE OMX_TRUE");
        return OMX_TRUE;
    }
    else
    {
        WFDMMLOGD("IS_DUMP_ENABLE OMX_FALSE");
        return OMX_FALSE;
    }
}

/*=============================================================================

         FUNCTION:          WFDMMSourceVideoEncode

         DESCRIPTION:
*//**       @brief          CTOR for WFDMMSourceVideoEncode
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

WFDMMSourceVideoEncode::WFDMMSourceVideoEncode()
{
    initData();
    WFDMMLOGH("WFDMMSourceVideoEncode ctor");
}

/*=============================================================================

         FUNCTION:          ~WFDMMSourceVideoEncode

         DESCRIPTION:
*//**       @brief          DTOR for WFDMMSourceVideoEncode
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

WFDMMSourceVideoEncode::~WFDMMSourceVideoEncode()
{
    WFDMMLOGH("WFDMMSourceVideoEncode dtor");
    destroy();
    WFDMMLOGH("Done with ~WFDMMSourceVideoEncode");
}

/*=============================================================================

         FUNCTION:          initData

         DESCRIPTION:
*//**       @brief          initializes data members of WFDMMSourceVideoEncode
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::initData()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    m_hEncoder = NULL;
    m_eState = OMX_StateLoaded;
    m_nModuleId = 0;
    m_pAppData = NULL;
    m_eCodec = OMX_VIDEO_CodingAVC;
    m_eProfType = AVCProfileBaseline;
    m_bCABAC = OMX_FALSE;
    m_pInputBuffers = NULL;
    m_nNumInputBuffers = WFD_VENC_NUM_ENC_IP_BUF;
    m_pOutputBuffers = NULL;
    m_nNumOutputBuffers = WFD_VENC_NUM_ENC_OP_BUF;
    m_nInputBufferSize = 0;
    m_nOutputBufferSize = 0;
    m_nBitrateFactor = 0;
    m_nInputPortIndex = 0;
    m_nOutputPortIndex = 0;
    m_bInputPortFound = OMX_FALSE;
    m_bOutputPortFound = OMX_FALSE;
    m_pCommandQ = NULL;
    m_pInpBufQ = NULL;
    m_pOutBufQ = NULL;
    m_pFnEventCallback = NULL;
    m_pFnFrameDeliver = NULL;
    m_hCritSect = NULL;
    m_hInputQLock = NULL;
    m_hOutputQLock = NULL;
#ifdef ENABLE_H264_DUMP
    H264_dump_file = NULL;
    m_bEnableH264Dump = OMX_FALSE;
    if(IS_DUMP_ENABLE())
    {
        H264_dump_file = fopen("/data/media/v4l2_dump_user.264","wb");
        if(!H264_dump_file)
        {
            WFDMMLOGE("WFDMMSourceVideoEncode::file open failed");
            m_bEnableH264Dump = OMX_FALSE;
        }
        else
        {
            m_bEnableH264Dump = OMX_TRUE;
        }
    }
#endif
    m_bEnableProfiling = WFDMMSourceStatistics::isProfilingEnabled();
    mWFDMMSrcStats     = NULL;
    memset(&m_sConfig, 0, sizeof(m_sConfig));
    m_hVideoCapture = NULL;
    m_bHDCPEnabled  = OMX_FALSE;
    SETSTATEDEINIT;
    return result;
}

/*=============================================================================

         FUNCTION:            configure

         DESCRIPTION:
*//**       @brief           Configures encoder copmponent
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      pConfig         The input configuration
                                                    for WFD session

    *       @param[in]      pFnCallback     The callback for reporting
                                            events to controller module

    *       @param[in]      pFnFrameDelivery   The callback for delivering
                                                 encoded frames to Mux

    *       @param[in]      pAppData   The appdata for use in callbacks

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::configure(
              VideoEncStaticConfigType* pConfig,
              eventHandlerType pFnCallback,
              FrameDeliveryFnType pFnFrameDelivery,
              OMX_BOOL bHDCPEnabled,
              OMX_U32 nModuleId,
              void* pAppData)
{

    //////////////////////////////////////////
    // Validate parameters
    //////////////////////////////////////////
    int nPeriodicIDRSupport = -1;
    int nRefreshInterval = 4; // Default value
    int nNrwSrchRange = 0; //Disable narrow search range by default
    int32 nMacroBlocks = 0;
    int32 nRetVal = -1;
    int32 nPerfLevelTurboMode = 0;
    OMX_INDEXTYPE index = OMX_IndexMax;
    OMX_STRING string = NULL;
    //Get primary display resolution
    android::DisplayInfo sPrimaryPanelRes;
    memset(&sPrimaryPanelRes,0,sizeof(sPrimaryPanelRes));
    android::sp<android::IBinder> primaryDisplay
         (android::SurfaceComposerClient::getBuiltInDisplay(
          android::ISurfaceComposer::eDisplayIdMain));

    if(!(pConfig && pFnCallback && pAppData))
    {
        WFDMMLOGE("Invalid parameters to configure encoder!!");
        return OMX_ErrorBadParameter;
    }

    //////////////////////////////////////////
    // Assign the parameters passed in
    //////////////////////////////////////////

    memcpy(&m_sConfig, pConfig, sizeof(m_sConfig));

    m_pFnEventCallback = pFnCallback;
    m_pFnFrameDeliver  = pFnFrameDelivery;
    m_nModuleId        = nModuleId;
    m_pAppData         = pAppData;
    m_bHDCPEnabled     = bHDCPEnabled;

    OMX_ERRORTYPE result = OMX_ErrorNone;

    //////////////////////////////////////////
    // Create resources
    //////////////////////////////////////////

    result = createResources();
    WFD_OMX_ERROR_CHECK(result,"Unable to create resources");

    WFDMMLOGE("Configuring encoder...");

    //////////////////////////////////////////
    // Configure encoder
    //////////////////////////////////////////


    //////////////////////////////////////////
    // Find ports of encoder component
    //////////////////////////////////////////

    OMX_PORT_PARAM_TYPE sPortParam;
    OMX_PARAM_PORTDEFINITIONTYPE     sPortDef;
    OMX_INIT_STRUCT(&sPortParam, OMX_PORT_PARAM_TYPE);
    OMX_INIT_STRUCT(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    result = OMX_GetParameter(m_hEncoder,
                            OMX_IndexParamVideoInit,
                            (OMX_PTR)&sPortParam);
    if (sPortParam.nPorts == 0)
    {
        WFDMMLOGE("WFDMMSourceVideoEncode: No ports found in component!");
        result = OMX_ErrorUndefined;
    }
    WFD_OMX_ERROR_CHECK(result,"Can't get OMX_IndexParamVideoInit");

    WFDMMLOGH1("Found %ld Video Ports", sPortParam.nPorts);

    for (uint32 i = 0; i < sPortParam.nPorts; i++)
    {
        sPortDef.nPortIndex = i;
        result = OMX_GetParameter(m_hEncoder,
                                OMX_IndexParamPortDefinition,
                                (OMX_PTR) &sPortDef);
        WFD_OMX_ERROR_CHECK(result,"Can't get OMX_IndexParamPortDefinition");
        if (sPortDef.eDir == OMX_DirInput)
        {
            WFDMMLOGH("Input port found");

            if (OMX_FALSE == m_bInputPortFound)
            {
                m_bInputPortFound = OMX_TRUE;
                m_nInputPortIndex = i;
            }
        }

        if (sPortDef.eDir == OMX_DirOutput)
        {
            WFDMMLOGH("Output port found");
            if (OMX_FALSE == m_bOutputPortFound)
            {
                m_bOutputPortFound = OMX_TRUE;
                m_nOutputPortIndex = i;
            }
        }
    }

    //////////////////////////////////////////
    // input buffer requirements
    //////////////////////////////////////////

    OMX_PARAM_PORTDEFINITIONTYPE inPortDef; // OMX_IndexParamPortDefinition
    OMX_INIT_STRUCT(&inPortDef,OMX_PARAM_PORTDEFINITIONTYPE);
    inPortDef.nPortIndex = m_nInputPortIndex; // input
    result = OMX_GetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
    WFD_OMX_ERROR_CHECK(result,"Cannot get OMX_IndexParamPortDefinition");
    inPortDef.format.video.nFrameWidth  = m_sConfig.nFrameWidth;
    inPortDef.format.video.nFrameHeight = m_sConfig.nFrameHeight;
    inPortDef.format.video.eColorFormat =
        (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;
    WFD_VENC_FractionToQ16(inPortDef.format.video.xFramerate,
        (int)(m_sConfig.nFramerate * 2),2);
    result = OMX_SetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
    WFD_OMX_ERROR_CHECK(result,"Cannot set OMX_IndexParamPortDefinition");
    result = OMX_GetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
    WFD_OMX_ERROR_CHECK(result,"Cannot get OMX_IndexParamPortDefinition");
    if(m_nNumInputBuffers >= (OMX_S32)inPortDef.nBufferCountMin)
    {
       inPortDef.nBufferCountActual = inPortDef.nBufferCountMin =
                                                m_nNumInputBuffers;
    }
    result = OMX_SetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
    WFD_OMX_ERROR_CHECK(result,"Cannot set OMX_IndexParamPortDefinition");
    result = OMX_GetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &inPortDef);
    WFD_OMX_ERROR_CHECK(result,"Cannot get OMX_IndexParamPortDefinition");
    if (m_nNumInputBuffers!= (OMX_S32)inPortDef.nBufferCountActual)
    {
        WFDMMLOGH2("Mismatch in input buffer count , required %ld set %ld",
        m_nNumInputBuffers,inPortDef.nBufferCountActual);
    }
    m_nInputBufferSize= (OMX_S32)(inPortDef.nBufferSize);

    //////////////////////////////////////////
    // output buffer requirements
    //////////////////////////////////////////

    OMX_PARAM_PORTDEFINITIONTYPE outPortDef; // OMX_IndexParamPortDefinition
    OMX_INIT_STRUCT(&outPortDef,OMX_PARAM_PORTDEFINITIONTYPE);
    outPortDef.nPortIndex = m_nOutputPortIndex; // output
    result = OMX_GetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
    WFD_OMX_ERROR_CHECK(result,"Can't get OMX_IndexParamPortDefinition");
    outPortDef.format.video.nFrameWidth  = m_sConfig.nOutputFrameWidth;
    outPortDef.format.video.nFrameHeight = m_sConfig.nOutputFrameHeight;
    WFD_VENC_FractionToQ16(outPortDef.format.video.xFramerate,
        (int)(m_sConfig.nFramerate * 2),2);
    result = OMX_SetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_IndexParamPortDefinition");
    result = OMX_GetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
    WFD_OMX_ERROR_CHECK(result,"Can't get OMX_IndexParamPortDefinition");
    if(m_nNumOutputBuffers >= (OMX_S32)outPortDef.nBufferCountMin)
    {
      outPortDef.nBufferCountActual = m_nNumOutputBuffers;
    }
    else
    {
        WFDMMLOGE2("OUT: Buffer min cnt mismatch ...%ld != %ld\n",
                outPortDef.nBufferCountActual, m_nNumOutputBuffers);
        m_nNumOutputBuffers = (OMX_S32)outPortDef.nBufferCountMin;
        m_sConfig.nOutBufferCount = m_nNumOutputBuffers;
    }
    result = OMX_SetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_IndexParamPortDefinition");
    result = OMX_GetParameter(m_hEncoder,
                              OMX_IndexParamPortDefinition,
                              (OMX_PTR) &outPortDef);
    WFD_OMX_ERROR_CHECK(result,"Can't get OMX_IndexParamPortDefinition");
    if (m_nNumOutputBuffers!= (OMX_S32) outPortDef.nBufferCountActual)
    {
       WFDMMLOGE2("OUT: Buffer reqs dont match...%ld != %ld\n",
        (OMX_S32)outPortDef.nBufferCountActual, m_nNumOutputBuffers);
       m_nNumOutputBuffers = (OMX_S32) outPortDef.nBufferCountMin;
       m_sConfig.nOutBufferCount = m_nNumOutputBuffers;
    }
    m_nOutputBufferSize= (OMX_S32) outPortDef.nBufferSize;

#if 1// TODO: Not compiling AS IS
    //////////////////////////////////////////
    //Peak Bitrate Setting
    //////////////////////////////////////////

    OMX_QCOM_VIDEO_PARAM_PEAK_BITRATE peakBitrate;
    OMX_INIT_STRUCT(&peakBitrate,OMX_QCOM_VIDEO_PARAM_PEAK_BITRATE);
    peakBitrate.nPeakBitrate = m_sConfig.nBitrate * 2;
    WFDMMLOGE1("Setting peak bitrate to %ld",peakBitrate.nPeakBitrate);
    result = OMX_SetParameter(m_hEncoder,
                          (OMX_INDEXTYPE)OMX_QcomIndexParamPeakBitrate,
                          (OMX_PTR) & peakBitrate);
#endif

    /*-----------------------------------------------------------------
    Bitrate & QP settings
    ---------------------------------------------------------------
    */

    OMX_VIDEO_PARAM_BITRATETYPE bitrate; // OMX_IndexParamVideoBitrate
    OMX_INIT_STRUCT(&bitrate,OMX_VIDEO_PARAM_BITRATETYPE);
    bitrate.nPortIndex =  m_nOutputPortIndex; // output
    bitrate.nTargetBitrate = m_sConfig.nBitrate;

    OMX_QCOM_VIDEO_PARAM_QPRANGETYPE qprange; // OMX_QcomIndexParamVideoQPRange
    OMX_INIT_STRUCT(&qprange,OMX_QCOM_VIDEO_PARAM_QPRANGETYPE);
    qprange.nPortIndex = m_nOutputPortIndex; // output
    qprange.minQP = 22;

    /*-----------------------------------------------------------------
    Bitrate factor is used to determine the RC mode and the QP values
    ---------------------------------------------------------------
    */

    if(m_sConfig.nBitrate)
    {
        m_nBitrateFactor = (m_sConfig.nFrameHeight * m_sConfig.nFrameWidth *
                            m_sConfig.nFramerate )/m_sConfig.nBitrate;
    }


    /*-----------------------------------------------------------------
    RC Mappings are as follows:

    OMX_Video_ControlRateVariableSkipFrames    VBR_VFR
    OMX_Video_ControlRateVariable                   VBR_CFR
    OMX_Video_ControlRateConstantSkipFrames   CBR_VFR
    OMX_Video_ControlRateConstant                  CBR_CFR

    ---------------------------------------------------------------
    */

    if (m_nBitrateFactor < 13)
    {
        bitrate.eControlRate = OMX_Video_ControlRateConstant;
        qprange.maxQP = 44;
    }
    else
    {
        bitrate.eControlRate = OMX_Video_ControlRateVariable;
        qprange.maxQP = 32;
    }

    WFDMMLOGH2("Setting bitrate to %ld with RC mode = %d",
        bitrate.nTargetBitrate, bitrate.eControlRate);
    result = OMX_SetParameter(m_hEncoder,
                              OMX_IndexParamVideoBitrate,
                             (OMX_PTR) &bitrate);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_IndexParamVideoBitrate");

    WFDMMLOGH2("Setting minQP = %ld, maxQP = %ld",
                qprange.minQP, qprange.maxQP);
    result = OMX_SetParameter(m_hEncoder,
                              (OMX_INDEXTYPE)OMX_QcomIndexParamVideoQPRange,
                              (OMX_PTR) &qprange);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_QcomIndexParamVideoQPRange");

    //////////////////////////////////////////
    // Frame rate
    //////////////////////////////////////////
    OMX_CONFIG_FRAMERATETYPE framerate; // OMX_IndexConfigVideoFramerate
    OMX_INIT_STRUCT(&framerate,OMX_CONFIG_FRAMERATETYPE);
    framerate.nPortIndex = m_nInputPortIndex; // input
    result = OMX_GetConfig(m_hEncoder,
                          OMX_IndexConfigVideoFramerate,
                          (OMX_PTR) &framerate);
    WFD_OMX_ERROR_CHECK(result,"Can't get OMX_IndexConfigVideoFramerate");
    WFD_VENC_FractionToQ16(framerate.xEncodeFramerate,
                        (int) (m_sConfig.nFramerate * 2),2);
    result = OMX_SetConfig(m_hEncoder,
                           OMX_IndexConfigVideoFramerate,
                           (OMX_PTR) &framerate);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_IndexConfigVideoFramerate");

    //////////////////////////////////////////
    //AU Delimiter
    //////////////////////////////////////////

    OMX_QCOM_VIDEO_CONFIG_H264_AUD auDelim;
    OMX_INIT_STRUCT(&auDelim,OMX_QCOM_VIDEO_CONFIG_H264_AUD);
    auDelim.bEnable = OMX_TRUE;
    result = OMX_SetParameter(m_hEncoder,
                              (OMX_INDEXTYPE)OMX_QcomIndexParamH264AUDelimiter,
                              (OMX_PTR) &auDelim);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_QCOM_VIDEO_CONFIG_H264_AUD");

    //////////////////////////////////////////
    //Performance Level
    //////////////////////////////////////////
#if 1 // TODO: Not compiling AS IS
    nRetVal = getCfgItem(PERF_LEVEL_TURBO_MODE_KEY,(int*)(&nPerfLevelTurboMode));
    if(nRetVal == 0 && nPerfLevelTurboMode)
    {
        WFDMMLOGE("Enabling Turbo Mode in encoder");
        OMX_QCOM_VIDEO_PARAM_PERF_LEVEL perfLevel;
        OMX_INIT_STRUCT(&perfLevel,OMX_QCOM_VIDEO_PARAM_PERF_LEVEL);
        perfLevel.nSize = sizeof(OMX_QCOM_VIDEO_PARAM_PERF_LEVEL);
        perfLevel.ePerfLevel = OMX_QCOM_PerfLevelTurbo;
        result = OMX_SetParameter(m_hEncoder,
                                  (OMX_INDEXTYPE)OMX_QcomIndexParamPerfLevel,
                                  (OMX_PTR) & perfLevel);
    }
    else
    {
        //Check for performance mode if Turbo mode is not enabled
        nRetVal = getCfgItem(PERF_LEVEL_PERF_MODE_KEY,(int*)(&nPerfLevelTurboMode));
        if(nRetVal == 0 && nPerfLevelTurboMode)
        {
            // TODO: NO SUPPORT FOR PERFORMANCE MODE
        }
    }
#endif

    //////////////////////////////////////////
    //Force SPS/PPS to be added to each IDR frame
    //////////////////////////////////////////

    android::PrependSPSPPSToIDRFramesParams prepSPSPPS;
    OMX_INIT_STRUCT(&prepSPSPPS,android::PrependSPSPPSToIDRFramesParams);
    prepSPSPPS.bEnable = OMX_TRUE;
    result = OMX_SetParameter(m_hEncoder,
                              (OMX_INDEXTYPE)OMX_QcomIndexParamSequenceHeaderWithIDR,
                              (OMX_PTR) & prepSPSPPS);
    WFD_OMX_ERROR_CHECK(result,"Can't set PrependSPSPPSToIDRFramesParams");
#if 1 // TODO: Not compiling AS IS

    //////////////////////////////////////////
    //VUI Timing Info
    //////////////////////////////////////////

    OMX_QCOM_VIDEO_PARAM_VUI_TIMING_INFO vuiTmInfo;
    OMX_INIT_STRUCT(&vuiTmInfo,OMX_QCOM_VIDEO_PARAM_VUI_TIMING_INFO);
    vuiTmInfo.bEnable = OMX_TRUE;
    result = OMX_SetParameter(m_hEncoder,
                          (OMX_INDEXTYPE)OMX_QcomIndexParamH264VUITimingInfo,
                          (OMX_PTR) & vuiTmInfo);
#endif

    //////////////////////////////////////////
    //Enable MetaDataInBuffer mode for input port
    //////////////////////////////////////////

/*-----------------------------------------------------------------
    Need for this:
    SurfaceMediaSource read will not be yielding rawYUV data. Instead it
    provides a handle to Gralloc buffers which need to be interpreted by the
    encoder accordingly, exonerating WFD from actually feeding any raw YUV
    data.
*/
    string = const_cast<OMX_STRING>(
            "OMX.google.android.index.storeMetaDataInBuffers");

    result= OMX_GetExtensionIndex(m_hEncoder, string, &index);
    WFD_OMX_ERROR_CHECK(result,
        "No index OMX.google.android.index.storeMetaDataInBuffers");

    android::StoreMetaDataInBuffersParams sMetaDataParam;
    OMX_INIT_STRUCT(&sMetaDataParam,android::StoreMetaDataInBuffersParams);
    sMetaDataParam.nPortIndex = m_nInputPortIndex;
    sMetaDataParam.bStoreMetaData = OMX_TRUE;
    result = OMX_SetParameter(m_hEncoder,index,
                          (OMX_PTR) & sMetaDataParam);
    WFD_OMX_ERROR_CHECK(result,
        "Failed to set StoreMetaDataInBuffersParams");

    /*-----------------------------------------------------------------
        Profile and Level setting
            ---------------------------------------------------------------
    */

    if (m_eCodec == OMX_VIDEO_CodingAVC)
    {
        OMX_VIDEO_PARAM_AVCTYPE sAVCType;
        OMX_INIT_STRUCT(&sAVCType,OMX_VIDEO_PARAM_AVCTYPE);

        sAVCType.nPortIndex = m_nOutputPortIndex; // output

        if(m_sConfig.nProfile == WFD_H264_PROFILE_CONSTRAINED_HIGH)
        {
            m_eProfType = AVCProfileHigh;
        }
        else
        {
            m_eProfType = AVCProfileBaseline;
        }

        switch(m_eProfType)
        {
            case AVCProfileBaseline:
               sAVCType.eProfile = OMX_VIDEO_AVCProfileBaseline;
               sAVCType.nBFrames = NUMBER_OF_BFRAME;
               break;
            case AVCProfileHigh:
               sAVCType.eProfile = OMX_VIDEO_AVCProfileHigh;
               sAVCType.nBFrames = NUMBER_OF_BFRAME;
               sAVCType.nPFrames = m_sConfig.nIntraPeriod/2;
               break;
            case AVCProfileMain:
               sAVCType.eProfile = OMX_VIDEO_AVCProfileMain;
               sAVCType.nBFrames = 1;
               sAVCType.nPFrames = m_sConfig.nIntraPeriod/2;
               break;
            default:
               sAVCType.eProfile = OMX_VIDEO_AVCProfileBaseline;
               sAVCType.nBFrames = NUMBER_OF_BFRAME;
               break;
        }
        switch (m_sConfig.nLevel)
        {
          case 1:
            sAVCType.eLevel = OMX_VIDEO_AVCLevel31;
            break;
          case 2:
            sAVCType.eLevel = OMX_VIDEO_AVCLevel32;
            break;
          case 4:
            sAVCType.eLevel = OMX_VIDEO_AVCLevel4;
            break;
          case 8:
            sAVCType.eLevel = OMX_VIDEO_AVCLevel41;
            break;
          case 16:
            sAVCType.eLevel = OMX_VIDEO_AVCLevel42;
            break;
          default:
            sAVCType.eLevel = OMX_VIDEO_AVCLevel31;
            break;
        }

        WFDMMLOGE3("avc.nBFrames = %ld, avc.eProfile = %x avc.elevel = %x",
                    sAVCType.nBFrames, sAVCType.eProfile, sAVCType.eLevel);
        sAVCType.bUseHadamard = OMX_FALSE;
        sAVCType.nRefFrames = 1;
        sAVCType.nRefIdx10ActiveMinus1 = 1;
        sAVCType.nRefIdx11ActiveMinus1 = 0;
        sAVCType.bEnableUEP = OMX_FALSE;
        sAVCType.bEnableFMO = OMX_FALSE;
        sAVCType.bEnableASO = OMX_FALSE;
        sAVCType.bEnableRS = OMX_FALSE;
        sAVCType.nAllowedPictureTypes = 2;
        sAVCType.bFrameMBsOnly = OMX_FALSE;
        sAVCType.bMBAFF = OMX_FALSE;
        sAVCType.bWeightedPPrediction = OMX_FALSE;
        sAVCType.nWeightedBipredicitonMode = 0;
        sAVCType.bconstIpred = OMX_FALSE;
        sAVCType.bDirect8x8Inference = OMX_FALSE;
        sAVCType.bDirectSpatialTemporal = OMX_FALSE;
        if(sAVCType.eProfile == OMX_VIDEO_AVCProfileBaseline)
        {
           sAVCType.bEntropyCodingCABAC = OMX_FALSE;
           sAVCType.nCabacInitIdc = 0;
        }
        else
        {
            if (m_bCABAC)
            {
               sAVCType.bEntropyCodingCABAC = OMX_TRUE;
               sAVCType.nCabacInitIdc = 0;
            }
            else
            {
               sAVCType.bEntropyCodingCABAC = OMX_FALSE;
               sAVCType.nCabacInitIdc = 0;
            }
        }

        if(m_sConfig.eResyncMarkerType == RESYNC_MARKER_NONE)
        {
            sAVCType.nSliceHeaderSpacing = m_sConfig.nResyncMarkerSpacing;
        }

        nRetVal = getCfgItem(PERIODIC_IDR_INTERVAL_VALID_KEY,
                            &nPeriodicIDRSupport);
        if((nRetVal == 0 && nPeriodicIDRSupport == 0))//No IDR route
        {

            /*-----------------------------------------------------------------
            RIR mode kicks in ONLY if IDR is explicitly disabled in config file
            Set PFrame interval to a huge value else if its not set explicitly
            all will be IDR frames, defeating the very purpose of RIR
            ---------------------------------------------------------------
            */
            sAVCType.nPFrames = 5000;
        }
        else
        {
            sAVCType.nPFrames = m_sConfig.nFramerate*nRefreshInterval-1;
        }

        WFDMMLOGE1("Value of sAVCType.nPFrames = %ld",sAVCType.nPFrames);
        result = OMX_SetParameter(m_hEncoder,
                                  OMX_IndexParamVideoAvc,
                                  (OMX_PTR) &sAVCType);
        WFD_OMX_ERROR_CHECK(result,"Cannot set OMX_IndexParamVideoAvc");

        //////////////////////////////////////////
        // Error resilience
        //////////////////////////////////////////

        if(m_sConfig.eResyncMarkerType != RESYNC_MARKER_NONE)
        {
            OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE errorCorrection;
            OMX_INIT_STRUCT(&errorCorrection,OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE);
            errorCorrection.nPortIndex = m_nOutputPortIndex; // output
            errorCorrection.bEnableRVLC = OMX_FALSE;
            errorCorrection.bEnableDataPartitioning = OMX_FALSE;
            result = OMX_SetParameter(m_hEncoder,
                                   (OMX_INDEXTYPE) OMX_IndexParamVideoErrorCorrection,
                                   (OMX_PTR) &errorCorrection);
            WFD_OMX_ERROR_CHECK(result,"Cannot set OMX_IndexParamVideoErrorCorrection");
        }
    }

    /*-----------------------------------------------------------------
    IntraRefresh settings
    ---------------------------------------------------------------
    */

    getCfgItem(PERIODIC_IDR_INTERVAL_KEY, &nRefreshInterval);
    if(nRefreshInterval <= 0)
    {
        nRefreshInterval = 5;//Default to 5 seconds
    }

    OMX_VIDEO_PARAM_INTRAREFRESHTYPE sIntraReferesh; // OMX_IndexParamVideoIntraRefresh

    OMX_INIT_STRUCT(&sIntraReferesh,OMX_VIDEO_PARAM_INTRAREFRESHTYPE);

// TODO: Does not compile as it is
#if 1
    if(nPeriodicIDRSupport == 0 && m_sConfig.nFramerate > 0)//Avoid DIV by 0 error
    {
        /*-----------------------------------------------------------------
              The value from config file gives the number of seconds in
              which a full frame refresh is required. Calculate the number
              of MBs to be refreshed accordingly by using

              Total MBs in a frame/(No. of frames over which entire refresh
                                            takes place)

                Where No. of frames over which entire refresh takes place is
                given by (frame rate) * (value read from cfg file)
            ---------------------------------------------------------------
            */
        nMacroBlocks = ((m_sConfig.nFrameWidth * m_sConfig.nFrameHeight) >> 8)/
                        (m_sConfig.nFramerate * nRefreshInterval);
        sIntraReferesh.nPortIndex = m_nOutputPortIndex; //Output port
        sIntraReferesh.eRefreshMode = OMX_VIDEO_IntraRefreshRandom;
        sIntraReferesh.nCirMBs = nMacroBlocks;
        WFDMMLOGE1("Setting Random Intra Refresh mode with %ld MBs",
                 sIntraReferesh.nCirMBs);
        result = OMX_SetParameter(m_hEncoder,
                                  OMX_IndexParamVideoIntraRefresh,
                                  (OMX_PTR) &sIntraReferesh);
       WFD_OMX_ERROR_CHECK(result,"Can't set OMX_IndexParamVideoIntraRefresh");
    }
    else
#endif
    {

        /*------------------------------------------------------------
          Go with the approach of fixed interval IDR
        --------------------------------------------------------------
        */

        /*-----------------------------------------------------------------
          Use QOMX_VIDEO_INTRAPERIODTYPE to explicitly set B-frame value
        ------------------------------------------------------------------
        */

        QOMX_VIDEO_INTRAPERIODTYPE sQOMXIntraPeriod;
        OMX_INIT_STRUCT(&sQOMXIntraPeriod,QOMX_VIDEO_INTRAPERIODTYPE);

        sQOMXIntraPeriod.nPortIndex = m_nOutputPortIndex;
        sQOMXIntraPeriod.nPFrames   = m_sConfig.nFramerate*nRefreshInterval-1;
        sQOMXIntraPeriod.nIDRPeriod = 1;
        sQOMXIntraPeriod.nBFrames   = 0;

        WFDMMLOGE3("I-frame interval %ld with IDR period %ld & B frames %ld",
                    sQOMXIntraPeriod.nPFrames,sQOMXIntraPeriod.nIDRPeriod,
                    sQOMXIntraPeriod.nBFrames);

        result = OMX_SetConfig(m_hEncoder,
                              (OMX_INDEXTYPE)QOMX_IndexConfigVideoIntraperiod ,
                              (OMX_PTR) &sQOMXIntraPeriod);

        WFD_OMX_ERROR_CHECK(result,"Cannot set QOMX_IndexConfigVideoIntraperiod ");

        /*-----------------------------------------------------------------
          Use OMX_VIDEO_CONFIG_AVCINTRAPERIOD to set IDR interval
        ------------------------------------------------------------------
        */

        OMX_VIDEO_CONFIG_AVCINTRAPERIOD sIntraPeriod;
        OMX_INIT_STRUCT(&sIntraPeriod,OMX_VIDEO_CONFIG_AVCINTRAPERIOD);
        sIntraPeriod.nPortIndex = m_nOutputPortIndex;
        sIntraPeriod.nPFrames = m_sConfig.nFramerate * nRefreshInterval - 1;
        sIntraPeriod.nIDRPeriod = 1;
        WFDMMLOGE2("Setting I-frame interval %ld with IDR period %ld",
            sIntraPeriod.nPFrames,sIntraPeriod.nIDRPeriod);
        result = OMX_SetConfig(m_hEncoder,
                              (OMX_INDEXTYPE) OMX_IndexConfigVideoAVCIntraPeriod,
                              (OMX_PTR) &sIntraPeriod);
        WFD_OMX_ERROR_CHECK(result,"Cannot set OMX_IndexConfigVideoAVCIntraPeriod");
    }

    /*--------------------------------------------------------------------------
      WARNING: Do Not Use Set Param of any index that triggers bitrate
        calculation beyond this point or any set param that uses
        IndexParamPortDefinition! Encoder re configures profile and level
        accordingly, altering the desired profile and level
    ----------------------------------------------------------------------------
    */

    /*---------------------------------------------------------------------
      Use OMX_QcomIndexParamSetMVSearchrange to enable narrow search range
    -----------------------------------------------------------------------
    */

    nRetVal = getCfgItem(NARROW_SEARCH_RANGE_KEY,&nNrwSrchRange);

    /*---------------------------------------------------------------------
        Query Primary display resolution and set narrow search range
        accordingly
    -----------------------------------------------------------------------
    */

    android::SurfaceComposerClient::getDisplayInfo(primaryDisplay, &sPrimaryPanelRes);
    WFDMMLOGD2("Primary Display panel resolution %d X %d",
              sPrimaryPanelRes.w, sPrimaryPanelRes.h);

    if(nRetVal == 0 && nNrwSrchRange == 1 &&
        ((sPrimaryPanelRes.w * sPrimaryPanelRes.h) >
         (WFD_720P_WIDTH * WFD_720P_HEIGHT)))
    {

        /*---------------------------------------------------------------------
          Ideally the set param should have allowed a null ptr to be passed in
          since the paramData is not used at all in component but null checks
          in OMX code prohibit that. So go with a temporary ptr to anything.
        -----------------------------------------------------------------------
        */

        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexParamSetMVSearchrange,
                (OMX_PTR) &nRetVal);

        WFD_OMX_ERROR_CHECK(result,"Cannot set OMX_QcomIndexParamSetMVSearchrange");

        WFDMMLOGH("Enabled narrow search range");

    }

    WFDMMLOGE("Done with encoder configuration");

    //==========================================
    // Go to executing state (also allocate buffers)
    WFDMMLOGE("Go to Idle state...");
    result = GoToIdleState();
    WFD_OMX_ERROR_CHECK(result,"Failed to go to Idle state");

    SETSTATEINIT;
EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            createResources

         DESCRIPTION:
*//**       @brief           create threads & queues for WFDMMSourceVideoEncode
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::createResources()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    int nRet = 0;

    m_pCommandQ = MM_New_Args(SignalQueue,(100, sizeof(CmdType)));
    m_pInpBufQ = MM_New_Args(SignalQueue,(10, sizeof(OMX_BUFFERHEADERTYPE*)));
    m_pOutBufQ = MM_New_Args(SignalQueue,(10, sizeof(OMX_BUFFERHEADERTYPE*)));

    if(!m_pCommandQ || !m_pInpBufQ || !m_pOutBufQ)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,"Failed to create signal Q");
    }

    result = createCaptureSource();

    WFD_OMX_ERROR_CHECK(result,"Failed to create capture source");

    nRet |= MM_CriticalSection_Create(&m_hInputQLock);
    nRet |= MM_CriticalSection_Create(&m_hOutputQLock);
    nRet |= MM_CriticalSection_Create(&m_hCritSect);

    if(nRet)
    {
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,"Failed to create Critical Sections");
    }
    memset(&mWfdVencStats,0,sizeof(mWfdVencStats));

    WFDMMLOGE("Creating encoder...");
    //////////////////////////////////////////
    // Acquire instance of encoder component
    //////////////////////////////////////////

    static OMX_CALLBACKTYPE callbacks = {EventCallback,
                                         EmptyDoneCallback,
                                         FillDoneCallback};

    if(OMX_TRUE == m_bHDCPEnabled)
    {
        WFDMMLOGE("Instantiating secure encoder");
        result = OMX_GetHandle(&m_hEncoder,
                               SecureEncoderComp,
                               this,
                               &callbacks);
    }
    else
    {
        WFDMMLOGE("Instantiating non-secure encoder");
        result = OMX_GetHandle(&m_hEncoder,
                           NonSecureEncoderComp,
                           this,
                           &callbacks);
    }
    WFD_OMX_ERROR_CHECK(result,"Error in getting component handle");

    if(m_bEnableProfiling)
    {
        mWFDMMSrcStats = WFDMMSourceStatistics::getInstance();
    }

EXIT:
    return result;
}


/*==============================================================================

         FUNCTION:

         DESCRIPTION:
*//**       @brief
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::ReleaseResources()
{

    if (m_pCommandQ)
    {
        MM_Delete(m_pCommandQ);
        m_pCommandQ = NULL;
    }

    if (m_pInpBufQ)
    {
        MM_Delete(m_pInpBufQ);
        m_pInpBufQ = NULL;
    }

    if (m_pOutBufQ)
    {
        MM_Delete(m_pInpBufQ);
        m_pInpBufQ = NULL;
    }

    if (m_hVideoCapture)
    {
        MM_Delete(m_hVideoCapture);
        m_hVideoCapture = NULL;
    }

    if (m_hCritSect)
    {
        MM_CriticalSection_Release(m_hCritSect);
        m_hCritSect = NULL;
    }

    if (m_hInputQLock)
    {
        MM_CriticalSection_Release(m_hInputQLock);
        m_hInputQLock = NULL;
    }

    if (m_hOutputQLock)
    {
        MM_CriticalSection_Release(m_hOutputQLock);
        m_hOutputQLock = NULL;
    }

    if (m_hEncoder)
    {
        OMX_FreeHandle(m_hEncoder);
        m_hEncoder = NULL;
    }

#ifdef ENABLE_H264_DUMP
    if (H264_dump_file)
    {
        fclose(H264_dump_file);
        H264_dump_file = NULL;
    }
#endif

    return OMX_ErrorNone;

}


/*=============================================================================

         FUNCTION:            createCaptureSource

         DESCRIPTION:
*//**       @brief           creates and configures instance of videocapture
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::createCaptureSource()
{
    OMX_ERRORTYPE eResult = OMX_ErrorNone;
    /*--------------------------------------------------------------------------
     Create the instance of WFDMMSourceVideoCapture
    ----------------------------------------------------------------------------
    */

    m_hVideoCapture = MM_New(WFDMMSourceVideoCapture);

    if (!m_hVideoCapture)
    {
        eResult = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(eResult,"Failed to create VideoCapture");
    }

    /*--------------------------------------------------------------------------
      Configure the instance of Data Source
    ----------------------------------------------------------------------------
    */

    eResult = m_hVideoCapture->configure(&m_sConfig,
                                         &captureEventsCb,
                                         &captureDataCb,
                                         0,
                                         this);

    WFD_OMX_ERROR_CHECK(eResult,"Failed to configure VideoCapture");

EXIT:
    return eResult;
}


/*=============================================================================

         FUNCTION:            GoToIdleState

         DESCRIPTION:
*//**       @brief           moves encoder componetnt to executing state
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::GoToIdleState()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_eState == OMX_StateLoaded)
    {
        ///////////////////////////////////////
        // 1. send idle state command
        // 2. allocate input buffers
        // 3. allocate output buffers
        // 4. wait for idle state complete
        // 5. send executing state command and wait for complete
        ///////////////////////////////////////

        // send idle state comand
        WFDMMLOGH("Going from OMX_StateLoaded --> OMX_StateIdle");
        result = SetState(OMX_StateIdle, OMX_FALSE);

        if (result == OMX_ErrorNone)
        {
            result = AllocateBuffers();
        }

        // wait for idle state complete
        if (result == OMX_ErrorNone)
        {
            result = WaitState(OMX_StateIdle);
        }
    }
    else if (m_eState == OMX_StateExecuting)
    {
        // send idle state comand
        WFDMMLOGH("Going from OMX_StateExecuting --> OMX_StateIdle");
        result = SetState(OMX_StateIdle, OMX_FALSE);

        // wait for idle state complete
        if (result == OMX_ErrorNone)
        {
            result = WaitState(OMX_StateIdle);
        }
    }
    else
    {
        WFDMMLOGE("invalid state");
        result = OMX_ErrorIncorrectStateTransition;
    }

    return result;
}


/*=============================================================================

         FUNCTION:            GoToLaodedState

         DESCRIPTION:
*//**       @brief           moves encoder componetnt to loaded state
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::GoToLoadedState()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_eState == OMX_StateLoaded)
    {
        WFDMMLOGH("Alread in loaded");
        return OMX_ErrorNone;
    }
    else if (m_eState == OMX_StateExecuting)
    {
        GoToIdleState();

    }

    if (m_eState == OMX_StateIdle)
    {
        result = SetState(OMX_StateLoaded, OMX_FALSE);
        if(result != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to move to State loaded");
        }


        WFDMMLOGH("Video OMX Component created");

        if(DeAllocateBuffers())
        {
            WFDMMLOGE("Failed to free Video Buffers");
        }

        result = WaitState(OMX_StateLoaded);

        if (result != OMX_ErrorNone)
        {
            WFDMMLOGH("Failed to wait to move to loaded");
        }

    }

    return result;
}


/*=============================================================================

         FUNCTION:            GoToExecutingState

         DESCRIPTION:
*//**       @brief           moves encoder componetnt to executing state
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::GoToExecutingState()
{
   OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_eState == OMX_StateIdle)
    {
        WFDMMLOGH("going to state OMX_StateExecuting...");
        result = SetState(OMX_StateExecuting, OMX_TRUE);
    }
    else
    {
        WFDMMLOGE("invalid state");
        result = OMX_ErrorIncorrectStateTransition;
    }

    return result;
}

/*=============================================================================

         FUNCTION:            AllocateBuffers

         DESCRIPTION:
*//**       @brief           allocates i/p & o/p buffers for encoder component
*//**
@par     DEPENDENCIES:
                            Should be called ONLY after encoder has been configured
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::AllocateBuffers()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    OMX_BUFFERHEADERTYPE* pBufferHdr;
    buff_hdr_extra_info* tempExtra;
    int i = 0;
    // allocate input buffers
    m_pInputBuffers = MM_New_Array(OMX_BUFFERHEADERTYPE*,m_nNumInputBuffers);
    OMX_U32 inpBuffSize = sizeof(android::encoder_media_buffer_type);
    for (i = 0; i < m_nNumInputBuffers; i++)
    {
       WFDMMLOGH2("allocating input buffer no=%d, size=%ld",
            i, inpBuffSize);
       result = OMX_AllocateBuffer(m_hEncoder,
                              &pBufferHdr,
                              m_nInputPortIndex, // port index
                              this, // pAppPrivate
                              inpBuffSize);
         WFD_OMX_ERROR_CHECK(result,"error OMX_AllocateBuffer");
         WFDMMLOGE1("Pushing input buffer %p",pBufferHdr->pBuffer);
         m_pInputBuffers[i] = pBufferHdr;
         pBufferHdr->pOutputPortPrivate = NULL;
         tempExtra = static_cast<buff_hdr_extra_info*>
            (calloc(1,sizeof(buff_hdr_extra_info)));
         if(tempExtra)
         {
            tempExtra->bPushed = OMX_FALSE;
            pBufferHdr->pPlatformPrivate = static_cast<OMX_PTR>(tempExtra);
            m_pInpBufQ->Push(&pBufferHdr,sizeof(pBufferHdr));
         }
    }
   // allocate output buffers
    m_pOutputBuffers = MM_New_Array(OMX_BUFFERHEADERTYPE*,m_nNumOutputBuffers);
    for (i = 0; i < m_nNumOutputBuffers; i++)
    {
       WFDMMLOGH2("allocating output buffer no=%d, size=%ld",
            i, m_nOutputBufferSize);
       //Let encoder take care of buffer allocation
       result = OMX_AllocateBuffer(m_hEncoder,
                              &pBufferHdr,
                              m_nOutputPortIndex, // port index
                              this, // pAppPrivate
                              m_nOutputBufferSize);
         WFD_OMX_ERROR_CHECK(result,"error allocating input buffer");
         WFDMMLOGE1("Pushing output buffer %p",pBufferHdr->pBuffer);
         m_pOutputBuffers[i] = pBufferHdr;
         tempExtra = static_cast<buff_hdr_extra_info*>
            (calloc(1,sizeof(buff_hdr_extra_info)));
         if(tempExtra)
         {
            tempExtra->bPushed = OMX_FALSE;
            pBufferHdr->pPlatformPrivate = static_cast<OMX_PTR>(tempExtra);
            m_pOutBufQ->Push(&pBufferHdr,sizeof(pBufferHdr));
         }
    }
EXIT:
        return result;
}


/*=============================================================================

         FUNCTION:            DeAllocateBuffers

         DESCRIPTION:
*//**       @brief           deallocates i/p & o/p buffers for encoder component
*//**
@par     DEPENDENCIES:
                            Should be called ONLY after encoder has been configured
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::DeAllocateBuffers()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    buff_hdr_extra_info* tempExtra;

    int i = 0;

    for (i = 0; i < m_nNumInputBuffers; i++)
    {
        if(m_pInputBuffers[i])
        {
            WFDMMLOGH1("Freeing input buffer %d", i);
            tempExtra = static_cast<buff_hdr_extra_info*>
                        ((m_pInputBuffers[i])->pPlatformPrivate);
            if(tempExtra)
            {
                MM_Free(tempExtra);
            }

            result = OMX_FreeBuffer(m_hEncoder,
                                    m_nInputPortIndex, m_pInputBuffers[i]);

            if (result != OMX_ErrorNone)
            {
                WFDMMLOGE("Failed to Free Buffer");
            }
        }
    }


    for (i = 0; i < m_nNumOutputBuffers; i++)
    {
        if(m_pOutputBuffers[i])
        {
            WFDMMLOGH1("Freeing output buffer %d", i);
            tempExtra = static_cast<buff_hdr_extra_info*>
                        ((m_pOutputBuffers[i])->pPlatformPrivate);
            if(tempExtra)
            {
                MM_Free(tempExtra);
            }

            result = OMX_FreeBuffer(m_hEncoder,
                                    m_nOutputPortIndex, m_pOutputBuffers[i]);

            if (result != OMX_ErrorNone)
            {
                WFDMMLOGE("Failed to Free Buffer");
            }
        }
    }

    return OMX_ErrorNone;
}

/*=============================================================================

         FUNCTION:            destroy

         DESCRIPTION:
*//**       @brief           destroy encoder component
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::destroy()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (ISSTATEOPENED)
    {
        Stop();
    }

    GoToLoadedState();

    ReleaseResources();

    if (m_hEncoder)
    {
        OMX_FreeHandle(m_hEncoder);
        m_hEncoder = NULL;
    }

    SETSTATEDEINIT;

    return result;
}

/*=============================================================================

         FUNCTION:            RequestIntraVOP

         DESCRIPTION:
*//**       @brief           request for IDR during session
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::RequestIntraVOP()
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(!ISSTATEOPENED)
    {
        result = OMX_ErrorInvalidState;
        WFD_OMX_ERROR_CHECK(result,"Not honouring IDR request");
    }

    WFDMMLOGH("IDR request");
    OMX_CONFIG_INTRAREFRESHVOPTYPE vop;
    OMX_INIT_STRUCT(&vop,OMX_CONFIG_INTRAREFRESHVOPTYPE);
    vop.nPortIndex = m_nOutputPortIndex; // output
    vop.IntraRefreshVOP = OMX_TRUE;
    result = OMX_SetConfig(m_hEncoder,
                           OMX_IndexConfigVideoIntraVOPRefresh,
                           (OMX_PTR) &vop);
    WFD_OMX_ERROR_CHECK(result,"WFDMMSourceVideoEncode:: Can't set\
                                OMX_IndexConfigVideoIntraVOPRefresh");
    EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            ChangeBitrate

         DESCRIPTION:
*//**       @brief           request for bitrate change during session
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::ChangeBitrate(OMX_S32 nBitrate)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(!ISSTATEOPENED)
    {
        result = OMX_ErrorInvalidState;
        WFD_OMX_ERROR_CHECK(result,"Not honouring bitrate change request");
    }

    OMX_VIDEO_CONFIG_BITRATETYPE bitrate; // OMX_IndexParamVideoBitrate
    OMX_INIT_STRUCT(&bitrate,OMX_VIDEO_CONFIG_BITRATETYPE);
    bitrate.nPortIndex =  m_nOutputPortIndex; // output
    bitrate.nEncodeBitrate = nBitrate;
    WFDMMLOGE1("Changing bitrate to %ld",nBitrate);
    result = OMX_SetConfig(m_hEncoder,
                          OMX_IndexConfigVideoBitrate,
                         (OMX_PTR) &bitrate);
    WFD_OMX_ERROR_CHECK(result,"Can't set OMX_IndexParamVideoBitrate");
    EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            EventCallback

         DESCRIPTION:
*//**       @brief           callback handler for events from encoder component
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      hComponent         handle of encoder component

    *       @param[in]      pAppData   appData supplied on creating encoder

    *       @param[in]      eEvent   type of event

    *       @param[in]      nData1   extra data for callback

    *       @param[in]      nData2   extra data for callback

    *       @param[in]      pEventData   event data for callback


*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::EventCallback(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_EVENTTYPE eEvent,
      OMX_IN OMX_U32 nData1,
      OMX_IN OMX_U32 nData2,
      OMX_IN OMX_PTR pEventData)
{
    (void)hComponent;
    (void) pEventData;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(!pAppData)
    {
        WFDMMLOGE("Invalid argument in EventCallback");
        return OMX_ErrorBadParameter;
    }

    WFDMMSourceVideoEncode* pMe = reinterpret_cast<WFDMMSourceVideoEncode*>
                                    (pAppData);

    if (eEvent == OMX_EventCmdComplete)
    {
        if ((OMX_COMMANDTYPE) nData1 == OMX_CommandStateSet)
        {
            WFDMMLOGH("Event callback with state change");

            switch ((OMX_STATETYPE) nData2)
            {
                case OMX_StateLoaded:
                   WFDMMLOGH("state status OMX_StateLoaded");
                   break;
                case OMX_StateIdle:
                   WFDMMLOGH("state status OMX_StateIdle");
                   break;
                case OMX_StateExecuting:
                   WFDMMLOGH("state status OMX_StateExecuting");
                   break;
                case OMX_StatePause:
                   WFDMMLOGH("state status OMX_StatePause");
                   break;
                case OMX_StateInvalid:
                   WFDMMLOGH("state status OMX_StateInvalid");
                   break;
                case OMX_StateWaitForResources:
                   WFDMMLOGH("state status OMX_StateWaitForResources");
                   break;
                default:
                   WFDMMLOGH1("Event callback with state %ld",nData2);
            }

            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandStateSet;
            cmd.sCmdData = nData2;
            cmd.eResult = result;
            WFDMMLOGE2("Pushing cmd with %d and %d", cmd.eEvent,cmd.eCmd);
            result = pMe->m_pCommandQ->Push(&cmd, sizeof(cmd));
        }
        else if ((OMX_COMMANDTYPE) nData1 == OMX_CommandFlush)
        {
            WFDMMLOGH("Event callback with flush status");
            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandFlush;
            cmd.sCmdData = 0;
            cmd.eResult = result;
            WFDMMLOGE("Pushing OMX_EventCmdComplete OMX_CommandFlush");
            result = pMe->m_pCommandQ->Push(&cmd, sizeof(cmd));
        }
        else
        {
            WFDMMLOGH("error status");
        }
    }
    else if (eEvent == OMX_EventError)
    {
        WFDMMLOGH("async error");
        WFDMMLOGH2("nData1 %lx, nData2 %lx", nData1, nData2);
        CmdType cmd;
        cmd.eEvent = OMX_EventError;
        cmd.eCmd = OMX_CommandMax;
        cmd.sCmdData = 0;
        cmd.eResult = (OMX_ERRORTYPE) nData1;
        result = static_cast<OMX_ERRORTYPE>(nData1);
        if(result == OMX_ErrorHardware)
        {
            WFDMMLOGE("Received hardware error");
            //pMe->m_hVideoCapture->stopCapture();
            //pMe->state(CLOSING, false);
            pMe->m_pFnEventCallback(pMe->m_pAppData, pMe->m_nModuleId,
                   WFDMMSRC_ERROR, OMX_ErrorNone, 0);
        }
        else
        {
            WFDMMLOGE("Ignoring error from component");
        }
    }
    else if (eEvent == OMX_EventBufferFlag)
    {
        WFDMMLOGH("Got buffer flag event");
    }
    else
    {
        WFDMMLOGH("Unimplemented event");
    }
    return result;
}

/*=============================================================================

         FUNCTION:            DeliverInput

         DESCRIPTION:
*//**       @brief           provides buffer for encoding to encoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      pBufferHdr         buffer holding data to encode

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::DeliverInput(
      OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    WFDMMLOGE1("WFDMMSourceVideoEncode::DeliverInput with %p",pBufferHdr);
    WFD_VENC_CRITICAL_SECT_ENTER(m_hInputQLock);

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if(!pBufferHdr)
    {
        result = OMX_ErrorBadParameter;
        WFD_OMX_ERROR_CHECK(result,
            "WFDMMSourceVideoEncode::Null buffer header to DeliverInput");
    }

    if (pBufferHdr->nInputPortIndex != m_nInputPortIndex)
    {
        result = OMX_ErrorBadPortIndex;

        WFD_OMX_ERROR_CHECK(result,
            "WFDMMSourceVideoEncode:: Invalid port index for DeliverInput");
    }

    if (!ISSTATEOPENED || (pBufferHdr->nFilledLen == 0))
    {
        WFDMMLOGE("Invalid State or flushing put buffer back to Queue");
        //m_pInpBufQ->Push(&pBufferHdr, sizeof(pBufferHdr));
        QInputBuffer(&pBufferHdr);
        result = OMX_ErrorInvalidState;
        WFD_OMX_ERROR_CHECK(result,
                "WFDMMSourceVideoEncode:: Invalid State in DeliverInput");
    }

    WFDMMLOGM5(" EmptyBuffer nFlags = %lx, nAllocLen= %ld, nFilledLen\
                = %ld, nOffset = %ld nTimestamp = %lld",
                pBufferHdr->nFlags,pBufferHdr->nAllocLen,
                pBufferHdr->nFilledLen,
                pBufferHdr->nOffset,
                pBufferHdr->nTimeStamp);

    WFDMMLOGD1("Difference in DeliverInputTS = %lld",
                pBufferHdr->nTimeStamp - mWfdVencStats.nETBDiff);

    mWfdVencStats.nETBDiff = pBufferHdr->nTimeStamp;

    if(mWfdVencStats.nMaxETBTime < mWfdVencStats.nETBDiff)
    {
        mWfdVencStats.nMaxETBTime = mWfdVencStats.nETBDiff;
    }

    result = OMX_EmptyThisBuffer(m_hEncoder, pBufferHdr);

    if (result != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to Q to encoder, collect in Input Q");
        //m_pInpBufQ->Push(&pBufferHdr, sizeof(pBufferHdr));
        QInputBuffer(&pBufferHdr);
    }

    if(m_bEnableProfiling)
    {
        /*-----------------------------------------------------------------
        Record statistics when an actual buffer was delivered to encoder.
        -------------------------------------------------------------------
        */
        if(mWFDMMSrcStats)
        {
            mWFDMMSrcStats->recordEncIP(pBufferHdr);
        }
    }

    WFDMMLOGM1("Input Buffer Header %p delivered to ENCODER",pBufferHdr);
    mWfdVencStats.nETB ++;

EXIT:

    WFD_VENC_CRITICAL_SECT_LEAVE(m_hInputQLock);
    return result;
}

/*=============================================================================

         FUNCTION:            EmptyDoneCallback

         DESCRIPTION:
*//**       @brief           callback from encoder when it has emptied
                                   input buffer, where the buffer is Q'd back
                                   to the i/p Q to be reused by captire module
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      hComponent         handle of encoder component

    *       @param[in]      pAppData   appData supplied on creating encoder

    *       @param[in]      pBufferHdr   buffer initially supplied to encoder

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::EmptyDoneCallback(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    (void)hComponent;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    WFDMMSourceVideoEncode* pMe = (WFDMMSourceVideoEncode*)pAppData;
    if (!pBufferHdr || !pAppData)
    {
        result = OMX_ErrorBadParameter;
        WFD_OMX_ERROR_CHECK(result,
        "WFDMMSourceVideoEncode::EmptyDoneCallback:Invalid Args");
    }
    WFDMMLOGE5(" EmptyDoneCallback nFlags = %lx, nAllocLen= %ld,\
                 nFilledLen = %ld, nOffset = %ld nTimestamp = %lld",
        pBufferHdr->nFlags,pBufferHdr->nAllocLen, pBufferHdr->nFilledLen,
        pBufferHdr->nOffset,pBufferHdr->nTimeStamp);
    WFDMMLOGM1("Input Buffer Header %p received from ENCODER",pBufferHdr);
    pMe->QInputBuffer(&pBufferHdr);
    pMe->mWfdVencStats.nEBDDiff =
        pBufferHdr->nTimeStamp - pMe->mWfdVencStats.nEBDDiff;
    pMe->mWfdVencStats.nEBD ++;
EXIT:
    return result;
}


/*=============================================================================

         FUNCTION:            FillDoneCallback

         DESCRIPTION:
*//**       @brief           callback from encoder when encoding is done. This
                                   buffer is to be the input to the muxer(after
                                   encryption in case of secure session)
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      hComponent         handle of encoder component

    *       @param[in]      pAppData   appData supplied on creating encoder

    *       @param[in]      pBufferHdr   buffer fhaving encoded data

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::FillDoneCallback(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    (void)hComponent;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    WFDMMSourceVideoEncode* pMe = (WFDMMSourceVideoEncode*)pAppData;
    if (!pBufferHdr || !pAppData)
    {
        result = OMX_ErrorBadParameter;
        WFD_OMX_ERROR_CHECK(result,
        "FillDoneCallback:Invalid Args");
    }
    WFDMMLOGM1("Output Buffer Header: %p received from ENCODER",pBufferHdr);
    WFDMMLOGM5("FillDoneCallback:nFlags = %lx, nAllocLen= %ld,\
           nFilledLen = %ld, nOffset = %ld nTimestamp = %lld",
        pBufferHdr->nFlags,pBufferHdr->nAllocLen, pBufferHdr->nFilledLen,
        pBufferHdr->nOffset,pBufferHdr->nTimeStamp);
    WFDMMLOGE1("Difference in FillDoneTS = %lld",
        pBufferHdr->nTimeStamp - pMe->mWfdVencStats.nFBDDiff);
    pMe->mWfdVencStats.nFBDDiff = pBufferHdr->nTimeStamp;
#ifdef ENABLE_H264_DUMP
    if (OMX_FALSE == pMe->m_bHDCPEnabled&&
        pMe->m_bEnableH264Dump &&(pMe->H264_dump_file))
    {
       fwrite((void *)pBufferHdr->pBuffer, 1, pBufferHdr->nFilledLen,
             pMe->H264_dump_file);
    }
#endif
    pMe->mWfdVencStats.nFBD ++;
    pMe->dumpVencStats();

    if((pBufferHdr->nFlags & OMX_BUFFERFLAG_CODECCONFIG) &&
       !(pBufferHdr->nFlags & OMX_BUFFERFLAG_SYNCFRAME))
    {
        /*-----------------------------------------------------------------
        Don't send it to MUX, Q it back to encoder
        -------------------------------------------------------------------
        */
        WFDMMLOGE("Not delivering frame");
        pMe->SetFreeBuffer(pBufferHdr);
        return OMX_ErrorNone;
    }

    if(pMe->m_bEnableProfiling)
    {
        if(pMe->mWFDMMSrcStats)
        {
            pMe->mWFDMMSrcStats->recordEncOP(pBufferHdr);
        }
    }

    WFDMMLOGM1("Output Buffer Header %p sending to MUX", pBufferHdr);

    pMe->m_pFnFrameDeliver(pBufferHdr,pMe->m_pAppData);

EXIT:
    return result;
}

/*==============================================================================

         FUNCTION:

         DESCRIPTION:
*//**       @brief
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIDE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::Start()
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
    /*--------------------------------------------------------------------------
     Move the component to executing state
    ----------------------------------------------------------------------------
    */
    eError = GoToExecutingState();

    WFD_OMX_ERROR_CHECK(eError,"Error in GoToExecutingState");

    WFDMMLOGE("Moved to executing state");

    /*--------------------------------------------------------------------------
     Start the buffer flow
    ----------------------------------------------------------------------------
    */

    /*--------------------------------------------------------------------------
     Firstly send all the output buffers to omx encoder
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < m_nNumOutputBuffers; i++)
    {
        eError = m_pOutBufQ->Pop(&pBufferHdr,sizeof(pBufferHdr),(OMX_S32)100);
        WFD_OMX_ERROR_CHECK(eError,"Failed in getting output buffer");

        WFDMMLOGE("Calling OMX_FillThisBuffer");
        eError = OMX_FillThisBuffer(m_hEncoder,pBufferHdr);
        WFD_OMX_ERROR_CHECK(eError,"Error in OMX_FillThisBuffer");
        mWfdVencStats.nFTB++;
    }

    /*--------------------------------------------------------------------------
      Send input buffers to Capturer to start capturing
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < m_nNumInputBuffers; i++)
    {
        eError = m_pInpBufQ->Pop(&pBufferHdr,sizeof(pBufferHdr),(OMX_S32)100);
        WFD_OMX_ERROR_CHECK(eError,"Failed in getting input buffer");
        WFDMMLOGE("Calling SetFreeBuffer");
        eError = m_hVideoCapture->SetFreeBuffer(pBufferHdr);
        WFD_OMX_ERROR_CHECK(eError,"Error in SetFreeBuffer");
    }

    SETSTATEOPENED;

    m_hVideoCapture->startCapture();


EXIT:
    return eError;
}

/*==============================================================================

         FUNCTION:          Stop

         DESCRIPTION:
*//**       @brief          Stops encoder wrapper
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::Stop()
{
     WFDMMLOGH("Stopping encoder");


    if (!m_hVideoCapture || !m_pInpBufQ || !m_pOutBufQ || !m_hEncoder)
    {
        WFDMMLOGE("Not enough stuff to stop capture");
        SETSTATECLOSED;
        return OMX_ErrorNone;
    }


    /*--------------------------------------------------------------------------
     Stop capture module first and then move encoder module to Stop
    ----------------------------------------------------------------------------
    */
    m_hVideoCapture->stopCapture();


    /*--------------------------------------------------------------------------
     Stop the buffer flow.Wait for all buffers to be back
    ----------------------------------------------------------------------------
    */
    SETSTATECLOSING;

    GoToIdleState();

    /*--------------------------------------------------------------------------
      Wait for Capturer to return all buffers
    ----------------------------------------------------------------------------
    */
    while(m_pInpBufQ->GetSize() != m_nNumInputBuffers)
    {
        WFDMMLOGH2("Waiting for %ld input buffers to be returned returned = %ld",
                  m_nNumInputBuffers, m_pInpBufQ->GetSize());
        MM_Timer_Sleep(2);
    }

    /*--------------------------------------------------------------------------
     Wait for Encoder and Mux to return all buffers
    ----------------------------------------------------------------------------
    */

    while(m_pOutBufQ->GetSize() != m_nNumOutputBuffers)
    {
        WFDMMLOGH2("Waiting for %ld output buffers to be returned returned = %ld",
                   m_nNumOutputBuffers, m_pOutBufQ->GetSize());
        MM_Timer_Sleep(2);
    }

    SETSTATECLOSED;

    WFDMMLOGE("Encoder Stopped");

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          Pause

         DESCRIPTION:
*//**       @brief          Pauses entire flow
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::Pause()
{
    WFDMMLOGH("Pausing encoder");

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!m_hVideoCapture || !m_pInpBufQ || !m_pOutBufQ || !m_hEncoder)
    {
        SETSTATECLOSED;
        result = OMX_ErrorInsufficientResources;
        WFD_OMX_ERROR_CHECK(result,"Not enough stuff to pause capture");
    }

    /*--------------------------------------------------------------------------
     Pause capture module first and then move encoder module to Stop
    ----------------------------------------------------------------------------
    */
    m_hVideoCapture->pauseCapture();


    /*--------------------------------------------------------------------------
     Stop the buffer flow.Wait for all buffers to be back
    ----------------------------------------------------------------------------
    */
    SETSTATECLOSING;

    result = OMX_SendCommand(m_hEncoder,
                             OMX_CommandFlush,
                             -1,
                             NULL);

    WFD_OMX_ERROR_CHECK(result,"Failed in OMX_SendCommand");

    WFDMMLOGH("Waiting for flush to complete");

    CmdType cmd;
    cmd.eCmd   = OMX_CommandMax;
    cmd.eEvent = OMX_EventMax;
    for(int i= 0; i<2; i++)//Wait for flush on both ports
    {
        m_pCommandQ->Pop(&cmd, sizeof(cmd), 5000);

        WFDMMLOGE2("Popped command with %d & %d",cmd.eEvent,cmd.eCmd);
        result = cmd.eResult;

        if (cmd.eEvent == OMX_EventCmdComplete)
        {
            if (cmd.eCmd == OMX_CommandFlush)
            {
                WFDMMLOGH("Received flush on 1 port");
            }
            else
            {
                result = OMX_ErrorUndefined;
                WFD_OMX_ERROR_CHECK(result,"Expecting flush!");
            }
        }
        else
        {
             result = OMX_ErrorUndefined;
             WFD_OMX_ERROR_CHECK(result,"expecting cmd complete");
        }
    }

    WFDMMLOGH("Done with flush command processing");

    /*--------------------------------------------------------------------------
      Wait for Capturer to return all buffers
    ----------------------------------------------------------------------------
    */
    while(m_pInpBufQ->GetSize() != m_nNumInputBuffers)
    {
        WFDMMLOGH2("Waiting for %ld input buffers to be returned returned = %ld",
                  m_nNumInputBuffers, m_pInpBufQ->GetSize());
        MM_Timer_Sleep(2);
    }

    /*--------------------------------------------------------------------------
     Wait for Encoder and Mux to return all buffers
    ----------------------------------------------------------------------------
    */

    while(m_pOutBufQ->GetSize() != m_nNumOutputBuffers)
    {
        WFDMMLOGH2("Waiting for %ld output buffers to be returned returned = %ld",
                   m_nNumOutputBuffers, m_pOutBufQ->GetSize());
        MM_Timer_Sleep(2);
    }

    SETSTATECLOSED;

    WFDMMLOGE("Encoder Paused");

EXIT:
    return result;

}

/*==============================================================================

         FUNCTION:          Resume

         DESCRIPTION:
*//**       @brief          Resumes entire flow
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::Resume()
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
    buff_hdr_extra_info* tempExtra;

    /*--------------------------------------------------------------------------
     Firstly send all the output buffers to omx encoder
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < m_nNumOutputBuffers; i++)
    {
        eError = m_pOutBufQ->Pop(&pBufferHdr,sizeof(pBufferHdr),(OMX_S32)100);
        if(eError != OMX_ErrorNone)
        {
            WFD_OMX_ERROR_CHECK(eError,"Failed in getting output buffer");
        }
        if(!pBufferHdr)
        {
           eError = OMX_ErrorBadParameter;
           WFD_OMX_ERROR_CHECK(eError,"Null output buffer");
        }
        tempExtra = static_cast<buff_hdr_extra_info*>
                (pBufferHdr->pPlatformPrivate);
        tempExtra->bPushed = OMX_FALSE;
        WFDMMLOGE("Calling OMX_FillThisBuffer");
        eError = OMX_FillThisBuffer(m_hEncoder,pBufferHdr);
        WFD_OMX_ERROR_CHECK(eError,"Error in OMX_FillThisBuffer");
        mWfdVencStats.nFTB++;
    }

    /*--------------------------------------------------------------------------
      Send input buffers to Capturer to start capturing
    ----------------------------------------------------------------------------
    */
    for(int i = 0; i < m_nNumInputBuffers; i++)
    {
        eError = m_pInpBufQ->Pop(&pBufferHdr,sizeof(pBufferHdr),(OMX_S32)100);
        if(eError != OMX_ErrorNone)
        {
            WFD_OMX_ERROR_CHECK(eError,"Failed in getting input buffer");
        }
        if(!pBufferHdr)
        {
           eError = OMX_ErrorBadParameter;
           WFD_OMX_ERROR_CHECK(eError,"Null input buffer");
        }
        tempExtra = static_cast<buff_hdr_extra_info*>
                (pBufferHdr->pPlatformPrivate);
        tempExtra->bPushed = OMX_FALSE;
        WFDMMLOGE("Calling SetFreeBuffer");
        /*--------------------------------------------------------------------------
          Force the buffers to capture module
        ----------------------------------------------------------------------------
        */
        eError = m_hVideoCapture->SetFreeBuffer(pBufferHdr,OMX_TRUE);
        WFD_OMX_ERROR_CHECK(eError,"Error in SetFreeBuffer");
    }

    SETSTATEOPENED;
    /*--------------------------------------------------------------------------
      Start session with an I-Frame to appease aggressive decoders
    ----------------------------------------------------------------------------
    */
    RequestIntraVOP();
    m_hVideoCapture->resumeCapture();
EXIT:
    return eError;

}

/*=============================================================================

         FUNCTION:            SetFreeBuffer

         DESCRIPTION:
*//**       @brief           returns back o/p buffer to encoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      pBufferHdr         buffer to be filled by encoder

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::SetFreeBuffer
(
    OMX_BUFFERHEADERTYPE* pBufferHdr
)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    buff_hdr_extra_info* tempExtra = NULL;

    if(!pBufferHdr)
    {
        result = OMX_ErrorBadParameter;
        WFD_OMX_ERROR_CHECK(result,"Null Buffer passed in for SetFreeBuffer");
    }

    if(pBufferHdr->nOutputPortIndex != m_nOutputPortIndex)
    {
        result = OMX_ErrorBadPortIndex;
        WFD_OMX_ERROR_CHECK(result,"Bad Port Number in SetFreeBuffer");
    }

    WFDMMLOGM1("Output Buffer Header %p received from MUX",pBufferHdr);

    tempExtra = static_cast<buff_hdr_extra_info*>(pBufferHdr->pPlatformPrivate);

    if(!ISSTATEOPENED)
    {
        result = OMX_ErrorInvalidState;
        /*----------------------------------------------------------------------
          COllect the buffer in Output Q to be accounted for while stop
        ------------------------------------------------------------------------
        */
        if(m_pOutBufQ)
        {
            if(tempExtra && OMX_FALSE == tempExtra->bPushed)
            {
                WFDMMLOGE1("Output Buffer Header %p pushing back to Q",
                            pBufferHdr);
                tempExtra->bPushed = OMX_TRUE;
                m_pOutBufQ->Push(&pBufferHdr, sizeof(pBufferHdr));
            }
            else
            {
                WFDMMLOGE("Not pushing duplicate Output Buffer Header");
            }
        }
        WFD_OMX_ERROR_CHECK(result,"SetFreeBuffer in invalid state");
    }

    if(m_hEncoder)
    {
        result = OMX_FillThisBuffer(m_hEncoder,pBufferHdr);

        if(result != OMX_ErrorNone)
        {
            /*------------------------------------------------------------------
             Collect the Buffer in Output Q to be accounted for
            --------------------------------------------------------------------
            */
            if(m_pOutBufQ)
            {
                WFDMMLOGE1("Output Buffer Header %p pushing back to Q", pBufferHdr);
                m_pOutBufQ->Push(&pBufferHdr, sizeof(pBufferHdr));
            }
        }
        WFD_OMX_ERROR_CHECK(result,"SetFreeBuffer Failed to call FTB");

        WFDMMLOGM1("Output Buffer Header %p sent to ENCODER",pBufferHdr);

        mWfdVencStats.nFTB++;
    }
EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            GetInputBuffHdrs

         DESCRIPTION:
*//**       @brief           get input buffers. Onus is on the caller to check
                                   for null buffers.
*//**
@par     DEPENDENCIES:
                                   Will return a non-null header IFF encoder
                                                            has been configured

*//*
         PARAMETERS:
*//**       @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_BUFFERHEADERTYPE**
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_BUFFERHEADERTYPE** WFDMMSourceVideoEncode::GetInputBuffHdrs()const
{
    return m_pInputBuffers;
}

/*=============================================================================

         FUNCTION:            GetOutputBuffHdrs

         DESCRIPTION:
*//**       @brief           get output buffer headers. Onus is on the caller
                                   to check for null buffers
*//**
@par     DEPENDENCIES:
                                   Will return a non-null header IFF encoder
                                                            has been configured
*//*
         PARAMETERS:
*//**       @param
                            None

*//*     RETURN VALUE:
*//**       @return
                            OMX_BUFFERHEADERTYPE**
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_BUFFERHEADERTYPE** WFDMMSourceVideoEncode::GetOutputBuffHdrs()const
{
    return m_pOutputBuffers;
}

/*=============================================================================

         FUNCTION:            QInputBuffer

         DESCRIPTION:
*//**       @brief           returns back i/p buffer to i/p queue
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      ppBufferHdr         ptr to buffer to be pushed back
                                                             to i/p Q

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/

OMX_ERRORTYPE WFDMMSourceVideoEncode::QInputBuffer
(
    OMX_BUFFERHEADERTYPE** ppBufferHdr
)
{
    WFDMMLOGE("WFDMMSourceVideoEncode::QInputBuffer");
    WFD_VENC_CRITICAL_SECT_ENTER(m_hInputQLock);
    OMX_ERRORTYPE result = OMX_ErrorNone;
    buff_hdr_extra_info* tempExtra;
    if(!ppBufferHdr)
    {
        result = OMX_ErrorBadParameter;
        WFD_OMX_ERROR_CHECK(result,"Null Buffer passed in for InputQ");
    }

    if (m_hVideoCapture)
    {
        result = m_hVideoCapture->SetFreeBuffer(*ppBufferHdr);

        if (result != OMX_ErrorNone)
        {
            WFDMMLOGE1("Input Buffer Header pushing back to Q %p", *ppBufferHdr);
            tempExtra = static_cast<buff_hdr_extra_info*>
                ((*ppBufferHdr)->pPlatformPrivate);
            if(OMX_FALSE == tempExtra->bPushed)
            {
                m_pInpBufQ->Push(ppBufferHdr,sizeof(*ppBufferHdr));
                tempExtra->bPushed = OMX_TRUE;
            }
            else
            {
                WFDMMLOGE("Not pushing duplicate buffer Header");
            }
        }
    }

EXIT:
    WFD_VENC_CRITICAL_SECT_LEAVE(m_hInputQLock);
    return result;
}

/*=============================================================================

         FUNCTION:            SetState

         DESCRIPTION:
*//**       @brief           sets a specific state of encoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      eState         state to set encoder in

    *       @param[in]      bSynchronous         flag indicating whether the
                                     state setting has to be synchronous or not

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::SetState
(
    OMX_STATETYPE eState,
    OMX_BOOL bSynchronous
)
{
    if(!m_hEncoder)
    {
        return OMX_ErrorInsufficientResources;
    }

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = OMX_SendCommand(m_hEncoder,
                             OMX_CommandStateSet,
                             (OMX_U32) eState,
                             NULL);

    WFD_OMX_ERROR_CHECK(result,"Failed in OMX_SendCommand");

    if (bSynchronous == OMX_TRUE)
    {
        result = WaitState(eState);
        WFD_OMX_ERROR_CHECK(result,"failed to wait state");
    }
EXIT:
    return result;
}

/*=============================================================================

         FUNCTION:            WaitState

         DESCRIPTION:
*//**       @brief           To wait for state setting or timeout
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param[in]      eState         state to set encoder in

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Error
@par     SIFE EFFECTS:
                            None
*//*=========================================================================*/
OMX_ERRORTYPE WFDMMSourceVideoEncode::WaitState
(
    OMX_STATETYPE eState
)
{
    OMX_ERRORTYPE result = OMX_ErrorUndefined;
    CmdType       cmd;

    WFDMMLOGE("Entered Waitstate");

    (void)m_pCommandQ->Pop(&cmd, sizeof(cmd), 5000);

    WFDMMLOGE2("Popped command with %d & %d",cmd.eEvent,cmd.eCmd);

    result = cmd.eResult;

    if (cmd.eEvent == OMX_EventCmdComplete)
    {
        if (cmd.eCmd == OMX_CommandStateSet)
        {
            if ((OMX_STATETYPE) cmd.sCmdData == eState)
            {
                WFDMMLOGH1("Encoder moved to state %d", (int) cmd.sCmdData);
                m_eState = (OMX_STATETYPE) cmd.sCmdData;
            }
            else
            {
                WFDMMLOGE1("wrong state (%d)", (int) cmd.sCmdData);
                result = OMX_ErrorUndefined;
                WFD_OMX_ERROR_CHECK(result,"Wrong state transition");
            }
        }
        else
        {
            result = OMX_ErrorUndefined;
            WFD_OMX_ERROR_CHECK(result,"expecting state change");
        }
    }
    else
    {
         result = OMX_ErrorUndefined;
         WFD_OMX_ERROR_CHECK(result,"expecting cmd complete");
    }
    result = OMX_ErrorNone;
EXIT:
    return result;
}

/*==============================================================================

         FUNCTION:          captureDataCb

         DESCRIPTION:
*//**       @brief          Data callback from capturer module
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSourceVideoEncode::captureDataCb
(
    OMX_PTR handle,
    OMX_BUFFERHEADERTYPE *pBufferHdr
)
{
    WFDMMSourceVideoEncode *pMe = (WFDMMSourceVideoEncode*)handle;

    if (!handle)
    {
        WFDMMLOGE("Invalid Handle in Data Cb");
        return;
    }

    pMe->DeliverInput(pBufferHdr);

}

/*==============================================================================

         FUNCTION:         captureEventsCb

         DESCRIPTION:
*//**       @brief         Event callback from capture module
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
void WFDMMSourceVideoEncode::captureEventsCb
(
    OMX_PTR pClientData,
    OMX_U32 nEvent,
    OMX_U32 nEvtData
)
{
    (void) pClientData;
    (void) nEvent;
    (void) nEvtData;
    ;
}

/*==============================================================================

         FUNCTION:          state

         DESCRIPTION:
*//**       @brief          sets or gets the state. This makes state transitions
                            thread safe
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            int state when get else a Dont Care
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
int WFDMMSourceVideoEncode::state(int state, bool get_true_set_false)
{
    WFD_VENC_CRITICAL_SECT_ENTER(m_hCritSect);

    if(get_true_set_false == true)
    {
        /*----------------------------------------------------------------------
          Just return the current state
        ------------------------------------------------------------------------
        */

    }
    else
    {
        m_nState = state;
        WFDMMLOGE1("WFDMMSourceVideoEncode Moved to state %d",state);
    }

    WFD_VENC_CRITICAL_SECT_LEAVE(m_hCritSect);

    return m_nState;
}

