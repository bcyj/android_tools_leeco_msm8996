/* =======================================================================
                             WFDMMSourceMux.cpp
DESCRIPTION

This module is for WFD source implementation
Interacts with Audio and Video source files.

Copyright (c) 2011 - 2015 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/src/WFDMMSourceMux.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Change:$
========================================================================== */

/*========================================================================
  Include Files
 ==========================================================================*/
#include "WFDMMSourceMux.h"
#include "OMX_Component.h"
#include "string.h"
#include "RTPEncoder.h"
#include "RTCPReceiver.h"
#include "QOMX_SourceExtensions.h"
#include "QOMX_FileFormatExtensions.h"
#include "QOMX_AudioExtensions.h"
#include "QOMX_AudioIndexExtensions.h"
#include "MMMemory.h"
#include "wfd_netutils.h"

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define CHK(result) if (result != OMX_ErrorNone) { MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"*************** error *************"); exit(0); }
#ifndef OMX_SPEC_VERSION
#define OMX_SPEC_VERSION 0x00000101
#endif
#define OMX_INIT_STRUCT(_s_, _name_)            \
   memset((_s_), 0x0, sizeof(_name_));          \
   (_s_)->nSize = sizeof(_name_);               \
   (_s_)->nVersion.nVersion = OMX_SPEC_VERSION

  struct CmdType
  {
    OMX_EVENTTYPE   eEvent;
    OMX_COMMANDTYPE eCmd;
    OMX_U32         sCmdData;
    OMX_ERRORTYPE   eResult;
  };

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Muxer::Muxer()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Should not be here this is a private constructor");
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Muxer::Muxer(EmptyDoneCallbackType pEmptyDoneFn,
     OMX_PTR pAppData,eventHandlerType pEventHandler,OMX_U32 nModuleId):

    m_pRTCPReceiver(NULL),
    m_nRTCPSock(0),
    m_nRTPSock(0),
    m_nRTPPort(0),
    m_nRTCPPort(0),
    m_pEventFn(NULL),
    m_pEmptyDoneFn(pEmptyDoneFn),
    m_pAppData(pAppData),
    m_bAudioUseBuffer(OMX_FALSE),
    m_bVideoUseBuffer(OMX_FALSE),
    m_bAudioPortEnabled(OMX_TRUE),
    m_bVideoPortEnabled(OMX_TRUE),
    m_pSignalQueue(new SignalQueue(32, sizeof(CmdType))),
    m_pAudioBuffers(NULL),
    m_pVideoBuffers(NULL),
    m_pAudioBufferHdrs(NULL),
    m_pVideoBufferHdrs(NULL),
    m_hMuxer(NULL),
    m_eState(OMX_StateLoaded),
    m_nAudioBuffers(0),
    m_nVideoBuffers(0),
    m_nAudioBufferSize(0),
    m_nVideoBufferSize(0),
    m_pSourcePort(NULL)
   {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Creating Muxer...");
    static OMX_CALLBACKTYPE callbacks = {EventCallback,
      EmptyDoneCallback,NULL};

    if (m_pEmptyDoneFn == NULL)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Empty buffer fn is NULL");
    }
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    ret = OMX_GetHandle(&m_hMuxer,
          (OMX_STRING)"OMX.qcom.file.muxer",
          this,
          &callbacks);
    if(ret != OMX_ErrorNone)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Error getting component handle = %x", ret);
    }
    if ( m_hMuxer == NULL)
    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Muxer is NULL");
    }
    else
    {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "GotMuxHandle");

    memset(&omxPortAudio, 0, sizeof(omxPortAudio));
    memset(&omxPortVideo, 0, sizeof(omxPortVideo));

       /**--------------------------------------------------------------------
          Get Mux default Settings.
       -----------------------------------------------------------------------
       */
       OMX_INIT_STRUCT(&omxPortVideo, OMX_PARAM_PORTDEFINITIONTYPE);
       omxPortVideo.nPortIndex = (OMX_U32) PORT_INDEX_VIDEO; // input
       ret = OMX_GetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortVideo);

       if(ret != OMX_ErrorNone)
       {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                       "::Muxer Failed to get Video Port Def");
       }

       OMX_INIT_STRUCT(&omxPortAudio, OMX_PARAM_PORTDEFINITIONTYPE);
       omxPortAudio.nPortIndex = (OMX_U32) PORT_INDEX_AUDIO; // input
       ret = OMX_GetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortAudio);

       if(ret != OMX_ErrorNone)
       {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                       "::Muxer Failed to get Audio Port Def");
       }

       m_nAudioBuffers = omxPortAudio.nBufferCountMin;
       m_nVideoBuffers = omxPortVideo.nBufferCountMin;
       bool bErr = getIPSockPair(true, &m_nRTPSock, &m_nRTCPSock,
                                         &m_nRTPPort, &m_nRTCPPort, false);
       if(bErr == true)
       {
           MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                        "Muxer: RTP Port pair found %d %d", m_nRTPPort, m_nRTCPPort);
           MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                        "Muxer: RTP Socket pair found %d %d", m_nRTPSock, m_nRTCPSock);
       }
       MUX_RTPinfo rtpInfo = {0,0,0,0,0,0,0,0,0};
       rtpInfo.portno1 = m_nRTPPort;
       rtpInfo.rtcpportno1 = m_nRTCPPort;
       m_pEventHandler = pEventHandler;
       m_nModuleId     = nModuleId;
       if(m_pEventHandler)
       {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "Muxer EVENT HANDLER");
          m_pEventHandler(m_pAppData, m_nModuleId,
                          WFDMMSRC_RTP_RTCP_PORT_UPDATE,
                          OMX_ErrorNone,(void*) (&rtpInfo));
       }
       b_IsStopped = OMX_FALSE;
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  Muxer::~Muxer()
  {
    OMX_FreeHandle(m_hMuxer);

    if(m_pRTCPReceiver)
    {
        MM_Delete(m_pRTCPReceiver);
    }

    if(m_nRTPSock > 0)
    {
        socketClose(m_nRTPSock);
    }

    if(m_nRTCPSock > 0)
    {
        socketClose(m_nRTCPSock);
    }

    if (m_pSignalQueue)
    {
      delete m_pSignalQueue;
    }
    if(m_pSourcePort)
    {
      delete m_pSourcePort;
      m_pSourcePort = NULL;
    }

  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::Configure(MuxerConfigType* pConfig,
                                 eventHandlerType pEventHandler,
                                 OMX_U32 nModuleId)
  {
     OMX_ERRORTYPE result = OMX_ErrorNone;
     OMX_VIDEO_CODINGTYPE eVideoCoding = OMX_VIDEO_CodingUnused;
     OMX_AUDIO_CODINGTYPE eAudioCoding = OMX_AUDIO_CodingUnused;

     if(!m_hMuxer)
     {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Invalid Mux handle");
        return OMX_ErrorComponentNotFound;
     }
     if (pConfig == NULL || !pEventHandler)
     {
        result = OMX_ErrorBadParameter;
     }
     else
     {
        memcpy(&m_sConfig, pConfig, sizeof(m_sConfig));
        m_pEventHandler = pEventHandler;
        m_nModuleId     = nModuleId;
        m_nAudioBuffers = pConfig->nAudioBufferCount;
        m_nVideoBuffers = pConfig->nVideoBufferCount;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Entered configure");

        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Num Audio buffers = %ld", m_nAudioBuffers);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Num Video buffers = %ld", m_nVideoBuffers);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Mux format = %d", pConfig->eFormat);
     }

     //////////////////////////////////////////
     // Index param Container Info (set the format type)
     //////////////////////////////////////////
     if (result == OMX_ErrorNone)
     {
        QOMX_CONTAINER_INFOTYPE fmt;
        OMX_INDEXTYPE pIndexType;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "About to set OMX struct");
        OMX_INIT_STRUCT(&fmt, QOMX_CONTAINER_INFOTYPE);
        if( OMX_GetExtensionIndex(m_hMuxer,
                                  (char *)"QOMX.Qualcomm.index.param.containerinfo",
                                  &pIndexType) == OMX_ErrorNone)
        {
          fmt.eFmtType = pConfig->eFormat;
          result = OMX_SetParameter(m_hMuxer,
             (OMX_INDEXTYPE)pIndexType,
             (OMX_PTR) &fmt);
          if (result == OMX_ErrorNone)
          {
             m_eFormat = pConfig->eFormat;
          }
       }
     }

     //////////////////////////////////////////
     // Index param encrypt type
     //////////////////////////////////////////
     if( (result == OMX_ErrorNone) && (pConfig->sEncryptParam.nStreamEncrypted == OMX_TRUE))
     {
        QOMX_ENCRYPTIONTYPE encryptType;
        OMX_INDEXTYPE pIndexType;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "About to set OMX struct");
        OMX_INIT_STRUCT(&encryptType, QOMX_ENCRYPTIONTYPE);
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "sateesh: Muxer::Configure calling OMX_GetExtensionIndex");
        if( OMX_GetExtensionIndex(m_hMuxer,
                                  (char *)"QOMX.Qualcomm.index.config.EncryptType",
                                  &pIndexType) == OMX_ErrorNone)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Muxer::Configure setting the encrypt extension");
          encryptType.nStreamEncrypted = pConfig->sEncryptParam.nStreamEncrypted;
          encryptType.nEncryptVersion = pConfig->sEncryptParam.nEncryptVersion;
          encryptType.nType = pConfig->sEncryptParam.nType;
          result = OMX_SetParameter(m_hMuxer,
             (OMX_INDEXTYPE)pIndexType,
             (OMX_PTR) &encryptType);
        }
     }

     if (result == OMX_ErrorNone)
     {
        if(pConfig->sOutputobj.outputType == MUXER_OUTPUT_RTP)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MUX:: RTP setting");
          QOMX_DATAINTERFACETYPE *pSource;
          OMX_INDEXTYPE pIndexType;

          if(pConfig->sOutputobj.outputInfo.rtpInfo.bPortTypeUDP)
          {
              bool bPair = false;

              if(pConfig->sOutputobj.outputInfo.rtpInfo.rtcpportno1)
              {
                  bPair = true;
              }
              if(m_nRTCPSock > 0 && bPair)
              {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Create RTCP");
               m_pRTCPReceiver = MM_New_Args(CRTCPReceiver,(m_nRTCPSock,RTPEventCallback, this));
              }

          }

          RTPNetworkInfoType sNwInfo = {0,0,0,0};

          sNwInfo.bRtpPortTypeUdp = pConfig->sOutputobj.outputInfo.rtpInfo.bPortTypeUDP;
          sNwInfo.DestIp          = (uint32)pConfig->sOutputobj.outputInfo.rtpInfo.ipaddr1;
          sNwInfo.Port0           = (uint32)pConfig->sOutputobj.outputInfo.rtpInfo.portno1;
          sNwInfo.nSocket         = m_nRTPSock;

          //m_pSourcePort = new CRTPEncoder(pConfig->sOutputobj.outputInfo.rtpInfo.portno1,
          //                              pConfig->sOutputobj.outputInfo.rtpInfo.ipaddr1,
          //                        ((pConfig->sAudioParams.bitrate + pConfig->svideoParams.bitrate)),
          //                        pConfig->sOutputobj.outputInfo.rtpInfo.bPortTypeUDP);

          m_pSourcePort = MM_New_Args(CRTPEncoder,
                                      (&sNwInfo,(uint32)(pConfig->sAudioParams.bitrate +
                                       pConfig->svideoParams.bitrate)));
          if (m_pSourcePort)
          {
            pSource = (QOMX_DATAINTERFACETYPE *)malloc(sizeof(QOMX_DATAINTERFACETYPE) + sizeof(m_pSourcePort));
            if (pSource)
            {
              pSource->nInterfaceSize = sizeof(m_pSourcePort);
              pSource->nSize = sizeof(QOMX_DATAINTERFACETYPE) + sizeof(m_pSourcePort);
              memcpy(&pSource->pInterface[0],&m_pSourcePort, sizeof(m_pSourcePort));
              pSource->nVersion.nVersion = 0x01020101;
              if(OMX_GetExtensionIndex(m_hMuxer,
                                       (char *)"OMX.QCOM.index.param.contentinterface.istreamport",
                                       &pIndexType) == OMX_ErrorNone)
              {
                OMX_SetParameter(m_hMuxer, (OMX_INDEXTYPE)pIndexType, pSource);
              }
              free(pSource);
            }
            else
            {
              result = OMX_ErrorInsufficientResources;
            }
          }
          else
          {
            result = OMX_ErrorInsufficientResources;
          }
        }
     }
     if(result == OMX_ErrorNone)
     {
        MUX_RTPinfo rtpInfo = {0,0,0,0,0,0,0,0,0};
        rtpInfo.portno1 = m_nRTPPort;
        rtpInfo.rtcpportno1 = m_nRTCPPort;
        if(m_pEventHandler)
        {
           m_pEventHandler(m_pAppData, m_nModuleId,
                           WFDMMSRC_RTP_SESSION_START,
                           OMX_ErrorNone,(void*)(&rtpInfo));
        }
     }
     //////////////////////////////////////////
     // Index param Audio Init
     //////////////////////////////////////////
     if (result == OMX_ErrorNone)
     {
        OMX_PORT_PARAM_TYPE portParam;
        result = OMX_GetParameter( m_hMuxer,
           OMX_IndexParamAudioInit,
           (OMX_PTR)&portParam);

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "About to set AUDIO codec settings");
        if (result == OMX_ErrorNone)
        {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Audio CODEC Settings");
           switch(pConfig->sAudioParams.format)
           {
              //////////////////////////////////////////
              // Audio init parameters
              //////////////////////////////////////////
           case MUX_AUDIO_AMR :
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Audio CODEC AMR Settings");

                 OMX_AUDIO_PARAM_AMRTYPE AMRAudioParam;

                 OMX_INIT_STRUCT(&AMRAudioParam, OMX_AUDIO_PARAM_AMRTYPE);
                 AMRAudioParam.nPortIndex = portParam.nStartPortNumber;
                 AMRAudioParam.eAMRBandMode = OMX_AUDIO_AMRBandModeNB7;
                 AMRAudioParam.eAMRDTXMode = OMX_AUDIO_AMRDTXModeOff;
                 AMRAudioParam.eAMRFrameFormat = OMX_AUDIO_AMRFrameFormatConformance;

                 AMRAudioParam.nBitRate = pConfig->sAudioParams.bitrate;
                 AMRAudioParam.nChannels = pConfig->sAudioParams.num_channels;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamAudioAmr,
                    (OMX_PTR) &AMRAudioParam);
                 eAudioCoding = OMX_AUDIO_CodingAMR;
                 break;
              }

           case MUX_AUDIO_MPEG4_AAC :
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Audio CODEC AAC Settings");

                 OMX_AUDIO_PARAM_AACPROFILETYPE AACAudioParam;
                 OMX_INIT_STRUCT(&AACAudioParam, OMX_AUDIO_PARAM_AACPROFILETYPE);
                 AACAudioParam.nPortIndex = portParam.nStartPortNumber;
                 AACAudioParam.nBitRate = pConfig->sAudioParams.bitrate;
                 AACAudioParam.nChannels = pConfig->sAudioParams.num_channels;
                 AACAudioParam.eChannelMode = pConfig->sAudioParams.aac_params.eChannelMode;
                 AACAudioParam.eAACStreamFormat=pConfig->sAudioParams.aac_params.eAACStreamFormat;
                 AACAudioParam.eAACProfile= pConfig->sAudioParams.aac_params.eAACProfile;
                 AACAudioParam.nSampleRate = pConfig->sAudioParams.sampling_frequency;
                 AACAudioParam.nFrameLength = 1024;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamAudioAac,
                    (OMX_PTR) &AACAudioParam);
                 eAudioCoding = OMX_AUDIO_CodingAAC;
                 break;

              }

           case MUX_AUDIO_AC3:
              {
                 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Audio CODEC AC3 Settings");

                 QOMX_AUDIO_PARAM_AC3TYPE sAC3AudioParam;
                 OMX_INIT_STRUCT(&sAC3AudioParam, QOMX_AUDIO_PARAM_AC3TYPE);

                 sAC3AudioParam.nPortIndex = portParam.nStartPortNumber;

                 sAC3AudioParam.eChannelConfig = pConfig->sAudioParams.num_channels == 6?
                     OMX_AUDIO_AC3_CHANNEL_CONFIG_3_2_1: pConfig->sAudioParams.num_channels == 8?
                     OMX_AUDIO_AC3_CHANNEL_CONFIG_3_2_1: OMX_AUDIO_AC3_CHANNEL_CONFIG_2_0;

                 sAC3AudioParam.bLfeOn = pConfig->sAudioParams.num_channels > 2?OMX_TRUE: OMX_FALSE;
                 sAC3AudioParam.eFormat = pConfig->sAudioParams.num_channels == 8 ? omx_audio_eac3:omx_audio_ac3;
                 sAC3AudioParam.nChannels = pConfig->sAudioParams.num_channels;
                 sAC3AudioParam.nSamplingRate = pConfig->sAudioParams.sampling_frequency;

                 result = OMX_SetParameter(m_hMuxer,
                    (OMX_INDEXTYPE) QOMX_IndexParamAudioAc3,
                    (OMX_PTR) &sAC3AudioParam);
                 if(result != OMX_ErrorNone)
                 {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Failed to set AC3 Params");
                 }
                 eAudioCoding = OMX_AUDIO_CodingAC3;
                 break;
              }
           case MUX_AUDIO_PCM:
              {
                 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Audio CODEC  PCM Settings");

                 OMX_AUDIO_PARAM_PCMMODETYPE PCMAudioParam;
                 OMX_INIT_STRUCT(&PCMAudioParam, OMX_AUDIO_PARAM_PCMMODETYPE);
                 PCMAudioParam.nPortIndex = portParam.nStartPortNumber;
                 PCMAudioParam.nBitPerSample = pConfig->sAudioParams.bits_per_sample;
                 PCMAudioParam.nChannels = pConfig->sAudioParams.num_channels;
                 PCMAudioParam.nSamplingRate = pConfig->sAudioParams.sampling_frequency;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamAudioPcm,
                    (OMX_PTR) &PCMAudioParam);
                 eAudioCoding = OMX_AUDIO_CodingPCM;
                 MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Audio CODEC  PCM Settings %d", result);
                 break;

              }
           case MUX_AUDIO_EVRC:
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Audio CODEC EVRC Settings");

                 OMX_AUDIO_PARAM_EVRCTYPE EVRCAudioParam;
                 OMX_INIT_STRUCT(&EVRCAudioParam, OMX_AUDIO_PARAM_EVRCTYPE);
                 EVRCAudioParam.nPortIndex = portParam.nStartPortNumber;
                 EVRCAudioParam.eCDMARate = OMX_AUDIO_CDMARateFull;
                 EVRCAudioParam.nChannels = pConfig->sAudioParams.num_channels;
                 //EVRCAudioParam.nMaxBitRate = 0;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamAudioPcm,
                    (OMX_PTR) &EVRCAudioParam);
                 eAudioCoding = OMX_AUDIO_CodingEVRC;
                 break;
              }
           case MUX_AUDIO_QCELP13K_FULL:
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Audio CODEC QCELP Settings");

                 OMX_AUDIO_PARAM_QCELP13TYPE QCELPAudioParam;
                 OMX_INIT_STRUCT(&QCELPAudioParam, OMX_AUDIO_PARAM_QCELP13TYPE);
                 QCELPAudioParam.nPortIndex = portParam.nStartPortNumber;
                 QCELPAudioParam.eCDMARate = OMX_AUDIO_CDMARateFull;
                 //QCELPAudioParam.nMaxBitRate = pConfig->sAudioParams.max_rate;
                 //QCELPAudioParam.nMinBitRate = pwfx->min_rate;
                 QCELPAudioParam.nChannels = pConfig->sAudioParams.num_channels;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamAudioQcelp13,
                    (OMX_PTR) &QCELPAudioParam);
                 eAudioCoding = OMX_AUDIO_CodingQCELP13;
                 break;
              }
           default:
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Disable Audio CODEC ");

               OMX_SendCommand(m_hMuxer, OMX_CommandPortDisable, portParam.nStartPortNumber,0);
               m_bAudioPortEnabled = OMX_FALSE;
                 eAudioCoding = OMX_AUDIO_CodingUnused;
               break;
           }
         }

     //////////////////////////////////////////
     // Index param Video Init
     //////////////////////////////////////////
        result = OMX_GetParameter( m_hMuxer,
           OMX_IndexParamVideoInit,
           (OMX_PTR)&portParam);
        if (result == OMX_ErrorNone)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Video CODEC Settings");
           //////////////////////////////////////////
           // Video init parameters
           //////////////////////////////////////////
           switch(pConfig->svideoParams.format)
           {
           case MUX_VIDEO_MPEG4:
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Video CODEC Mpeg 4 Settings");

                 OMX_VIDEO_PARAM_MPEG4TYPE Mpeg4VideoParam;
                 OMX_INIT_STRUCT(&Mpeg4VideoParam, OMX_VIDEO_PARAM_MPEG4TYPE);

                 //Mpeg4VideoParam.bACPred = OMX_TRUE;
                 Mpeg4VideoParam.eLevel = pConfig->svideoParams.eMpeg4Level;
                 Mpeg4VideoParam.eProfile =  pConfig->svideoParams.eMpeg4Profile;
                 //Mpeg4VideoParam.nTimeIncRes = 1000;
                 Mpeg4VideoParam.nPortIndex = portParam.nStartPortNumber;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamVideoMpeg4,
                    (OMX_PTR) &Mpeg4VideoParam);

                 eVideoCoding = OMX_VIDEO_CodingMPEG4;
                 break;
              }
           case MUX_VIDEO_H264:
              {

                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Video CODEC H264 Settings");

                 OMX_VIDEO_PARAM_AVCTYPE AVCVideoParam;

                 OMX_INIT_STRUCT(&AVCVideoParam, OMX_VIDEO_PARAM_AVCTYPE);
                 AVCVideoParam.nPortIndex = portParam.nStartPortNumber;
                 AVCVideoParam.eProfile = pConfig->svideoParams.eH264Profile;
                 AVCVideoParam.eLevel =  pConfig->svideoParams.eH264Level;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamVideoAvc,
                    (OMX_PTR) &AVCVideoParam);
                 eVideoCoding = OMX_VIDEO_CodingAVC;
                 MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Video CODEC H264 Settings %d",result );
                 break;
              }
           case MUX_VIDEO_H263:
              {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Video CODEC H263 Settings");
                 OMX_VIDEO_PARAM_H263TYPE H263VideoParam;
                 OMX_INIT_STRUCT(&H263VideoParam, OMX_VIDEO_PARAM_H263TYPE);
                 H263VideoParam.nPortIndex = portParam.nStartPortNumber;
                 H263VideoParam.eProfile = pConfig->svideoParams.eH263Profile;
                 H263VideoParam.eLevel =  pConfig->svideoParams.eH263Level;

                 result = OMX_SetParameter(m_hMuxer,
                    OMX_IndexParamVideoH263,
                    (OMX_PTR) &H263VideoParam);
                 eVideoCoding = OMX_VIDEO_CodingH263;
                 break;
              }
           default:
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Video CODEC Disable Settings");
               OMX_SendCommand(m_hMuxer, OMX_CommandPortDisable, portParam.nStartPortNumber,0);
               m_bVideoPortEnabled =  OMX_FALSE;
               eVideoCoding = OMX_VIDEO_CodingUnused;
               break;
           }
        }
     }


     //////////////////////////////////////////
     // input buffer requirements
     //////////////////////////////////////////
     if (result == OMX_ErrorNone)
     {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "PortDefSettings");

        if(m_bAudioPortEnabled)
        {
            OMX_INIT_STRUCT(&omxPortAudio, OMX_PARAM_PORTDEFINITIONTYPE);
            omxPortAudio.nPortIndex = (OMX_U32) PORT_INDEX_AUDIO; // input
            result = OMX_GetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortAudio);
            if (result == OMX_ErrorNone)
            {
               if(omxPortAudio.eDomain == OMX_PortDomainAudio)
               {
                  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Num Audio buffers potdef = %ld", omxPortAudio.nBufferCountMin);
                   m_nAudioBuffers = MAX((unsigned)m_nAudioBuffers, omxPortAudio.nBufferCountMin);
                   omxPortAudio.nBufferCountActual = m_nAudioBuffers;
                   omxPortAudio.format.audio.eEncoding = eAudioCoding;
               }
            }
            else
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Audio PortDef FAILED");
            result = OMX_SetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortAudio);

        }

        //video
        if(m_bVideoPortEnabled)
        {

            OMX_INIT_STRUCT(&omxPortVideo, OMX_PARAM_PORTDEFINITIONTYPE);
            omxPortVideo.nPortIndex = (OMX_U32) PORT_INDEX_VIDEO; // input
            result = OMX_GetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortVideo);

            if(result != OMX_ErrorNone)
            {
                   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Audio PortDef FAILED");
            }
            if(omxPortVideo.eDomain == OMX_PortDomainVideo)
            {
               m_nVideoBuffers = MAX((unsigned)m_nVideoBuffers, omxPortVideo.nBufferCountMin) ;
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Num Video buffers potdef = %ld", omxPortVideo.nBufferCountMin);
               omxPortVideo.format.video.nFrameWidth = pConfig->svideoParams.width;
               omxPortVideo.format.video.nFrameHeight = pConfig->svideoParams.height;
               omxPortVideo.format.video.nBitrate = pConfig->svideoParams.bitrate;
               omxPortVideo.format.video.xFramerate = pConfig->svideoParams.frame_rate << 16;
               omxPortVideo.nBufferCountActual = m_nVideoBuffers;
               omxPortVideo.format.video.eCompressionFormat = eVideoCoding;
            }
            result = OMX_SetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortVideo);
        }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Num Audio buffers = %ld", m_nAudioBuffers);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Num Video buffers = %ld", m_nVideoBuffers);


   //     if ((m_nAudioBuffers != (OMX_S32) omxPortAudio.nBufferCountActual)||
   //        (m_nVideoBuffers != (OMX_S32) omxPortVideo.nBufferCountActual))
   //     {
    //       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Buffer reqs dont match...");
   //     }

        m_nAudioBufferSize = (OMX_S32) omxPortAudio.nBufferSize;
        m_nVideoBufferSize = (OMX_S32) omxPortVideo.nBufferSize;
     }

     return OMX_ErrorNone;
  }

  /////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////
   OMX_ERRORTYPE Muxer::EnableUseBufferModel(OMX_BOOL bInUseBuffer, OMX_U32 portIndex,
                                              OMX_BUFFERHEADERTYPE **pInBuffers,
                                             OMX_U32 nBuffers )
   {
   OMX_ERRORTYPE result = OMX_ErrorNone;
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Enable Use Buffer model %ld", portIndex);
   if(portIndex==(OMX_U32)PORT_INDEX_AUDIO )
    {
        m_bAudioUseBuffer = bInUseBuffer;
        m_pAudioBuffers = pInBuffers;
        if(pInBuffers[0]->nAllocLen != omxPortAudio.nBufferSize ||
          omxPortAudio.nBufferCountActual != nBuffers)
        {
            omxPortAudio.nBufferCountActual = nBuffers;
            omxPortAudio.nBufferSize = pInBuffers[0]->nAllocLen;
            m_nAudioBuffers = nBuffers;
            result = OMX_SetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortAudio);
            if(result != OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Enable Use Buffer model Failed to set Portdef for size");
            }
        }
    }
    else if(portIndex==(OMX_U32)PORT_INDEX_VIDEO )
    {
        m_bVideoUseBuffer = bInUseBuffer;
        m_pVideoBuffers = pInBuffers;
        if(pInBuffers[0]->nAllocLen != omxPortVideo.nBufferSize ||
          omxPortVideo.nBufferCountActual != nBuffers)
        {
            omxPortVideo.nBufferCountActual = nBuffers;
            omxPortVideo.nBufferSize = pInBuffers[0]->nAllocLen;
            m_nVideoBuffers = nBuffers;
            result = OMX_SetParameter(m_hMuxer,
               OMX_IndexParamPortDefinition,
               (OMX_PTR) &omxPortVideo);
            if(result != OMX_ErrorNone)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Enable Use Buffer model Failed to set Portdef for size");
            }
        }
    }
    else
    {
        result = OMX_ErrorBadPortIndex;
    }

    return result;

   }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  OMX_ERRORTYPE Muxer::GoToExecutingState()
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_eState == OMX_StateLoaded)
    {
      ///////////////////////////////////////
      // 1. send idle state command
      // 2. allocate audio buffers
      // 3. allocate Video buffers
      // 4. wait for idle state complete
      // 5. send executing state command and wait for complete
      ///////////////////////////////////////

      // send idle state comand
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "going to state OMX_StateIdle...");
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

      // send executing state command and wait for complete
      if (result == OMX_ErrorNone)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "going to state OMX_StateExecuting...");
        result = SetState(OMX_StateExecuting, OMX_TRUE);
      }

    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "invalid state");
      result = OMX_ErrorIncorrectStateTransition;
    }

    return result;
  }


  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::GoToLoadedState()
  {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    if(!m_hMuxer)
    {
      return OMX_ErrorInsufficientResources;
    }
    ///////////////////////////////////////
    // 1. send idle state command and wait for complete
    // 2. send loaded state command
    // 3. free audio buffers
    // 4. free video buffers
    // 5. wait for loaded state complete
    ///////////////////////////////////////

    // send idle state comand and wait for complete
    if (m_eState == OMX_StateExecuting ||
        m_eState == OMX_StatePause)
    {
      Flush();
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"going to state OMX_StateIdle...");
      result = SetState(OMX_StateIdle, OMX_TRUE);
    }


    // send loaded state command
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"going to state OMX_StateLoaded...");
    result = SetState(OMX_StateLoaded, OMX_FALSE);

    if (result == OMX_ErrorNone)
    {

      result = FreeBuffers();
      // wait for loaded state complete
      result = WaitState(OMX_StateLoaded);
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::DeliverInput(OMX_BUFFERHEADERTYPE* pBuffer)
  {
    if(!m_hMuxer)
    {
      return OMX_ErrorInsufficientResources;
    }

    if(m_eState != OMX_StateExecuting)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Deliver Input: Mux in invalid state");
      return OMX_ErrorInvalidState;
    }

    /*MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "H264  Data");
    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM, "%x %x %x %x", pBuffer->pBuffer[pBuffer->nOffset],
                       pBuffer->pBuffer[pBuffer->nOffset + 1],
                       pBuffer->pBuffer[pBuffer->nOffset + 2],
                       pBuffer->pBuffer[pBuffer->nOffset + 3]);
    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM, "%x %x %x %x", pBuffer->pBuffer[pBuffer->nOffset + 4],
                       pBuffer->pBuffer[pBuffer->nOffset + 5],
                       pBuffer->pBuffer[pBuffer->nOffset + 6],
                       pBuffer->pBuffer[pBuffer->nOffset + 7]);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "H264  Data No Offset");
    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_MEDIUM, "%x %x %x %x", pBuffer->pBuffer[0],
                       pBuffer->pBuffer[1],
                       pBuffer->pBuffer[2],
                       pBuffer->pBuffer[3]);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "%x %x %x %x", pBuffer->pBuffer[4],
                       pBuffer->pBuffer[5],
                       pBuffer->pBuffer[6],
                       pBuffer->pBuffer[7]); */

    /*

    TODO need to copy to a muxer buffer header... Dangerous use...*/
    if( pBuffer->nInputPortIndex ==  PORT_INDEX_AUDIO)
    {
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSource::MuxerAudioEmptyBufferDone %ld",pBuffer->nFilledLen );
    }
    else if( pBuffer->nInputPortIndex ==  PORT_INDEX_VIDEO)
    {
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "WFDMMSource::MuxerVideoEmptyBufferDone %ld",pBuffer->nFilledLen );
    }


//       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "WFDMMSource::MuxerEmptyBufferDone %d",pBuffer->nFilledLen );
    return OMX_EmptyThisBuffer(m_hMuxer, pBuffer);

  }

  OMX_ERRORTYPE Muxer::GetInNumBuffers(OMX_U32 nPortIndex, OMX_U32 *nBuffers)
  {
    if(nPortIndex != PORT_INDEX_VIDEO &&
       nPortIndex != PORT_INDEX_AUDIO)
    {
      return OMX_ErrorBadPortIndex;
    }

    *nBuffers =
         ((nPortIndex == PORT_INDEX_AUDIO )?
          m_nAudioBuffers : m_nVideoBuffers);
    return OMX_ErrorNone;

  }

  OMX_ERRORTYPE Muxer::SetNumBuffers(OMX_U32 nPortIndex, OMX_U32 nBuffers)
  {
      if(!m_hMuxer)
      {
          return OMX_ErrorInsufficientResources;
      }
      OMX_ERRORTYPE eRet = OMX_ErrorNone;

      if(nPortIndex == PORT_INDEX_AUDIO)
      {
          if(nBuffers < (unsigned int)m_nAudioBuffers)
          {
              return OMX_ErrorBadParameter;
          }
          if(nBuffers != (unsigned int)m_nAudioBuffers )
          {
              eRet = OMX_GetParameter(m_hMuxer, OMX_IndexParamPortDefinition,&omxPortAudio);

              if(eRet == OMX_ErrorNone)
              {
                  omxPortAudio.nBufferCountActual = nBuffers;
                  eRet = OMX_SetParameter(m_hMuxer,OMX_IndexParamPortDefinition,&omxPortAudio);
              }
          }
      }

      else if(nPortIndex == PORT_INDEX_VIDEO)
      {
          if(nBuffers < (unsigned int)m_nVideoBuffers)
          {
              return OMX_ErrorBadParameter;
          }
          if(nBuffers != (unsigned int)m_nVideoBuffers )
          {
              eRet = OMX_GetParameter(m_hMuxer, OMX_IndexParamPortDefinition,&omxPortVideo);

              if(eRet == OMX_ErrorNone)
              {
                  omxPortVideo.nBufferCountActual = nBuffers;
                  eRet = OMX_SetParameter(m_hMuxer,OMX_IndexParamPortDefinition,&omxPortVideo);
              }
          }
      }
      else
      {
          return OMX_ErrorBadPortIndex;
      }
      return eRet;

  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_BUFFERHEADERTYPE** Muxer::GetBuffers(OMX_U32 nPortIndex)
  {
    OMX_BUFFERHEADERTYPE** ppBuffers;

    if (m_eState == OMX_StateExecuting)
    {
       ppBuffers = (nPortIndex == (OMX_U32)PORT_INDEX_AUDIO )? m_pAudioBufferHdrs : m_pVideoBufferHdrs;
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "GetBuffers success!");
    }
    else
    {
      ppBuffers = NULL;
    }
    return ppBuffers;
  }
   /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::AllocateBuffers()
  {
     int i;
     OMX_ERRORTYPE result = OMX_ErrorNone;
    if(!m_hMuxer)
    {
      return OMX_ErrorInsufficientResources;
    }
     //////////////////////////////////////////
     //  USE BUFFER MODEL
     //////////////////////////////////////////
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Mux allocate buf");///
     if( m_bAudioPortEnabled)
     {
         m_pAudioBufferHdrs = new OMX_BUFFERHEADERTYPE*[m_nAudioBuffers];
         if (m_bAudioUseBuffer == OMX_TRUE)
         {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "level1 USE BUFFER INPUT: allocating audio buffer");

            for (i = 0; i < m_nAudioBuffers; i++)
            {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "USE BUFFER INPUT: allocating audio buffer");
               MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_HIGH, "USE BUFFER INPUT: %p %p %ld %p", m_hMuxer,&m_pAudioBufferHdrs[i],m_pAudioBuffers[i]->nAllocLen,m_pAudioBuffers[i]->pBuffer  );
               result = OMX_UseBuffer(m_hMuxer,
                  &m_pAudioBufferHdrs[i],
                  PORT_INDEX_AUDIO, // port index
                  (void *)this,
                  m_pAudioBuffers[i]->nAllocLen,
                  m_pAudioBuffers[i]->pBuffer);
               if (result != OMX_ErrorNone)
               {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "error using Audio buffer");
                  break;
               }
            }
         }
         else
         {
#if 0
           // This implementation needs to be corrected. I the case of allocate buffer,
           // muxer needs to malloc buffer. making it unsupported now.
            for (i = 0; i < m_nAudioBuffers; i++)
            {
               result = OMX_AllocateBuffer(m_hMuxer,
                  &m_pAudioBuffers[i],
                  PORT_INDEX_AUDIO, // port index
                  this, // pAppPrivate
                  m_nAudioBufferSize);
               if (result != OMX_ErrorNone)
               {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "error allocating audio buffer");
                  break;
               }
            }
#endif
           return OMX_ErrorNotImplemented;
         }

     }

     if(m_bVideoPortEnabled)
     {
         m_pVideoBufferHdrs = new OMX_BUFFERHEADERTYPE*[m_nVideoBuffers];
         if (m_bVideoUseBuffer == OMX_TRUE)
         {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Level 1USE BUFFER INPUT: allocating video buffer");
            for (i = 0; i < m_nVideoBuffers; i++)
            {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "USE BUFFER INPUT: allocating Video buffer");
                MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_HIGH, "USE BUFFER INPUT: %p %p %ld %p", m_hMuxer,&m_pVideoBufferHdrs[i],m_pVideoBuffers[i]->nAllocLen,m_pVideoBuffers[i]->pBuffer  );

               result = OMX_UseBuffer(m_hMuxer,
                  &m_pVideoBufferHdrs[i],
                  PORT_INDEX_VIDEO, // port index
                  (void*)this,
                  m_pVideoBuffers[i]->nAllocLen,
                  m_pVideoBuffers[i]->pBuffer);
               if (result != OMX_ErrorNone)
               {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "error using Video buffer");
                  break;
               }
            }
         }
         else
         {
#if 0
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "ALLOCATE BUFFER INPUT: allocating video buffer");


            for (i = 0; i < m_nVideoBuffers; i++)
            {
               result = OMX_AllocateBuffer(m_hMuxer,
                  &m_pVideoBuffers[i],
                  PORT_INDEX_VIDEO, // port index
                  this, // pAppPrivate
                  m_nVideoBufferSize);
               if (result != OMX_ErrorNone)
               {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "error allocating video buffer");
                  break;
               }
            }
#endif
            return OMX_ErrorNotImplemented;
         }
     }

     return OMX_ErrorNone;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::FreeBuffers()
  {
     int i;
     OMX_ERRORTYPE result = OMX_ErrorNone;
    if(!m_hMuxer)
    {
      return OMX_ErrorInsufficientResources;
    }
     if (m_pAudioBuffers)
     {
        for (i = 0; i < m_nAudioBuffers; i++)
        {
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"freeing audio buffer %d", i);

           result = OMX_FreeBuffer(m_hMuxer,
              PORT_INDEX_AUDIO, // port index
              m_pAudioBufferHdrs[i]);
           if (result != OMX_ErrorNone)
           {
              MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"error freeing audio buffer = %d", result);
              return result;
           }

        }

        delete [] m_pAudioBufferHdrs;
        m_pAudioBufferHdrs = NULL;
     }
     if(m_pVideoBuffers)
     {
        for (i = 0; i < m_nVideoBuffers; i++)
        {
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"freeing video buffer %d", i);

           result = OMX_FreeBuffer(m_hMuxer,
              PORT_INDEX_VIDEO,
              m_pVideoBufferHdrs[i]);
           if (result != OMX_ErrorNone)
           {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"error freeing video buffer");
              return result;
           }
        }
        delete [] m_pVideoBufferHdrs;
        m_pVideoBufferHdrs = NULL;
     }
     return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::SetState(OMX_STATETYPE eState,
      OMX_BOOL bSynchronous)
  {
     OMX_ERRORTYPE result = OMX_ErrorNone;
    if(!m_hMuxer)
    {
      return OMX_ErrorInsufficientResources;
    }
    result = OMX_SendCommand(m_hMuxer,
        OMX_CommandStateSet,
        (OMX_U32) eState,
        NULL);

    if (result == OMX_ErrorNone)
    {
      if (bSynchronous == OMX_TRUE)
      {
        result = WaitState(eState);
        if (result != OMX_ErrorNone)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "failed to wait state");
        }
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "failed to set state");
    }
    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::WaitState(OMX_STATETYPE eState)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    CmdType cmd;

    (void) m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0); // infinite wait
    result = cmd.eResult;

    if (cmd.eEvent == OMX_EventCmdComplete)
    {
      if (cmd.eCmd == OMX_CommandStateSet)
      {
        if ((OMX_STATETYPE) cmd.sCmdData == eState)
        {
          m_eState = (OMX_STATETYPE) cmd.sCmdData;
        }
        else
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "wrong state (%d)", (int) cmd.sCmdData);
          result = OMX_ErrorUndefined;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "expecting state change");
        result = OMX_ErrorUndefined;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "expecting cmd complete");
      if(m_pEventHandler)
      {
          m_pEventHandler(m_pAppData, m_nModuleId, WFDMMSRC_ERROR, OMX_ErrorUndefined, 0);
      }
      result = OMX_ErrorUndefined;
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::Flush()
  {
    if(!m_hMuxer)
    {
      return OMX_ErrorInsufficientResources;
    }
   OMX_ERRORTYPE result = OMX_ErrorNone;
    result = OMX_SendCommand(m_hMuxer,
        OMX_CommandFlush,
        OMX_ALL,
        NULL);
    if (result == OMX_ErrorNone)
    {
      CmdType cmd;
      if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "error popping signal");
      }
      result = cmd.eResult;
      if (cmd.eEvent != OMX_EventCmdComplete || cmd.eCmd != OMX_CommandFlush)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "expecting flush");
        result = OMX_ErrorUndefined;
      }

      if (result != OMX_ErrorNone)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "failed to wait for flush");
        result = OMX_ErrorUndefined;
      }
      else
      {
        if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "error popping signal");
        }
        else
        {
          result = cmd.eResult;
          if (cmd.eEvent != OMX_EventCmdComplete || cmd.eCmd != OMX_CommandFlush)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "expecting flush");
            result = OMX_ErrorUndefined;
          }
        }

        if (result != OMX_ErrorNone)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "failed to wait for flush");
          result = OMX_ErrorUndefined;
        }
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "failed to set state");
    }
    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::EventCallback(OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_EVENTTYPE eEvent,
      OMX_IN OMX_U32 nData1,
      OMX_IN OMX_U32 nData2,
      OMX_IN OMX_PTR pEventData)
  {
    (void)hComponent;
    (void)pEventData;
    OMX_ERRORTYPE result = OMX_ErrorNone;

   if (eEvent == OMX_EventCmdComplete)
    {
      if ((OMX_COMMANDTYPE) nData1 == OMX_CommandStateSet)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Event callback with state change");

        switch ((OMX_STATETYPE) nData2)
        {
          case OMX_StateLoaded:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status OMX_StateLoaded");
            break;
          case OMX_StateIdle:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status OMX_StateIdle");
            break;
          case OMX_StateExecuting:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status OMX_StateExecuting");
            break;
          case OMX_StatePause:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status OMX_StatePause");
            break;
          case OMX_StateInvalid:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status OMX_StateInvalid");
            break;
          case OMX_StateWaitForResources:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status OMX_StateWaitForResources");
            break;
      default:
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "state status Invalid");
        break;
        }

        CmdType cmd;
        cmd.eEvent = OMX_EventCmdComplete;
        cmd.eCmd = OMX_CommandStateSet;
        cmd.sCmdData = nData2;
        cmd.eResult = result;

         result = ((Muxer*) pAppData)->m_pSignalQueue->Push(&cmd, sizeof(cmd));
      }
      else if ((OMX_COMMANDTYPE) nData1 == OMX_CommandFlush)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Event callback with flush status");
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "flush status");

        CmdType cmd;
        cmd.eEvent = OMX_EventCmdComplete;
        cmd.eCmd = OMX_CommandFlush;
        cmd.sCmdData = 0;
        cmd.eResult = result;
        result = ((Muxer*) pAppData)->m_pSignalQueue->Push(&cmd, sizeof(cmd));
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "error status");
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Unimplemented command");
      }
    }
    else if (eEvent == OMX_EventError)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "async error");
      CmdType cmd;
      cmd.eEvent = OMX_EventError;
      cmd.eCmd = OMX_CommandMax;
      cmd.sCmdData = 0;
      cmd.eResult = (OMX_ERRORTYPE) nData1;
      ((Muxer*) pAppData)->m_pEventHandler(((Muxer*) pAppData)->m_pAppData,
           ((Muxer*) pAppData)->m_nModuleId, WFDMMSRC_ERROR, OMX_ErrorUndefined, 0);
      return result;
    }
    else if (eEvent == OMX_EventBufferFlag)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "got buffer flag event");
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Unimplemented event");
    }

    return result;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  OMX_ERRORTYPE Muxer::EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
  {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Muxer::EmptyDoneCallback");
    if (((Muxer*) pAppData)->m_pEmptyDoneFn)
    {
      ((Muxer*) pAppData)->m_pEmptyDoneFn(hComponent,
        ((Muxer*) pAppData)->m_pAppData, // forward the client from constructor
        pBuffer);
    }

    return result;
  }


  //This function must be called only when source components
  // have paused generating data or in the same thred context
  // as the deliverInput function
  OMX_ERRORTYPE Muxer::Update(MuxerConfigType *pConfig)
  {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                            "WFDMMSourceMux:: UpdateTransportType");
      if(!m_hMuxer)
      {
        return OMX_ErrorInsufficientResources;
      }
      //RIght now only Transport type update is supported.
      if(m_sConfig.sOutputobj.outputInfo.rtpInfo.bPortTypeUDP !=
         pConfig->sOutputobj.outputInfo.rtpInfo.bPortTypeUDP)
      {

          if(m_eState == OMX_StateExecuting)
          {
              m_eState = OMX_StateIdle;
              Flush();
          }
          if(UpdateRTPPort(&pConfig->sOutputobj.outputInfo.rtpInfo) !=
             OMX_ErrorNone)
          {
              return OMX_ErrorResourcesLost;
          }
          memcpy(&m_sConfig, pConfig, sizeof(m_sConfig));

          m_eState = OMX_StateExecuting;
      }

      return OMX_ErrorNone;

  }

  OMX_ERRORTYPE Muxer::UpdateRTPPort(MUX_RTPinfo *pRTPInfo)
  {
      if(!m_hMuxer)
      {
          return OMX_ErrorInsufficientResources;
      }
      QOMX_DATAINTERFACETYPE *pSource;
      OMX_INDEXTYPE pIndexType;
      //First set a NULL streamport to Mux so that an invalid access
      //to stream port is not made during the switch.
      pSource = (QOMX_DATAINTERFACETYPE *)malloc(sizeof(QOMX_DATAINTERFACETYPE)
                                                 + sizeof(int));

      if (!pSource)
      {
          return OMX_ErrorInsufficientResources;
      }

      int nZero = 0;
      pSource->nInterfaceSize = 4;
      pSource->nSize = sizeof(QOMX_DATAINTERFACETYPE) + sizeof(int);
      memcpy(&pSource->pInterface[0],&nZero, sizeof(nZero));
      pSource->nVersion.nVersion = 0x01020101;
      if(OMX_GetExtensionIndex(m_hMuxer,
                               (char *)"OMX.QCOM.index.param.contentinterface.istreamport",
                               &pIndexType) == OMX_ErrorNone)
      {
        OMX_SetConfig(m_hMuxer, (OMX_INDEXTYPE)pIndexType, pSource);
      }


      if(m_pSourcePort)
      {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Deleting RTP encoder");
          MM_Delete(m_pSourcePort);
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Deleted RTP encoder");
      }

      if(m_nRTPSock > 0)
      {
          socketClose(m_nRTPSock);
          m_nRTPSock = 0;
      }

      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MUX:: RTP Encoder Creating");

      m_pSourcePort = MM_New_Args (CRTPEncoder, ((uint32)pRTPInfo->portno1,
                                                 (uint32)pRTPInfo->ipaddr1,
                                                 (uint32)m_sConfig.sAudioParams.bitrate +
                                                 (uint32)m_sConfig.svideoParams.bitrate,
                                                 (uint8)pRTPInfo->bPortTypeUDP
                                                )
                                  );
      if (m_pSourcePort)
      {
          pSource->nInterfaceSize = 4;
          pSource->nSize = sizeof(QOMX_DATAINTERFACETYPE) + sizeof(int);
          memcpy(&pSource->pInterface[0],&m_pSourcePort, sizeof(m_pSourcePort));
          pSource->nVersion.nVersion = 0x01020101;
          if(OMX_GetExtensionIndex(m_hMuxer,
                                   (char *)"OMX.QCOM.index.param.contentinterface.istreamport",
                                   &pIndexType) == OMX_ErrorNone)
          {
            OMX_SetConfig(m_hMuxer, (OMX_INDEXTYPE)pIndexType, pSource);
          }

          free(pSource);
      }
      else
      {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Create New RTP Failed");
           free(pSource);
           return OMX_ErrorInsufficientResources;
      }

    return OMX_ErrorNone;
  }

  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
   void Muxer::RTPEventCallback(RTPEventType eEvent, RTPStatusType eStatus,
                                void* nEvtData, void *pAppData)
   {
       (void)eEvent;
       (void)eStatus;
       if(!pAppData)
       {
           return;
       }
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPCallback");
      ((Muxer*)pAppData)->SendEventCallback(eEvent,
                                            eStatus,nEvtData);

   }
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
   void Muxer::SendEventCallback(RTPEventType eEvent,
                                 RTPStatusType eStatus, void* nData)
   {
       (void) eStatus;
       switch(eEvent)
       {
       case RTP_SERVER_PORT_EVENT:
           break;

       case RTP_RTCP_RR_EVENT:
           {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Received RR at muxer");
              m_pEventHandler(m_pAppData, m_nModuleId, WFDMMSRC_RTCP_RR_MESSAGE,
                                      OMX_ErrorNone,nData);
           }
           break;
       default:
           break;
       }

       return;
   }

