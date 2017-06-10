/* =======================================================================
                             WFDMMSourceMux.h
DESCRIPTION

Header file for WFDMMSourceMux.cpp file

Copyright (c) 2011 - 2012,2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/inc/WFDMMSourceMux.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$
========================================================================== */

#ifndef _WFD_MM_SOURCE_MUX_H
#define _WFD_MM_SOURCE_MUX_H

/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "WFDMMSourceMuxDefines.h"
#include "WFDMMSourceSignalQueue.h"
#include "MMDebugMsg.h"
#include "WFDMMSourceComDef.h"
#include "RTPEncoder.h"
#include "RTCPReceiver.h"

class CRTPEncoder;
extern  OMX_BOOL b_IsStopped;
namespace video
{
  class iStreamPort;
}

#define  PORT_INDEX_AUDIO  0
#define  PORT_INDEX_VIDEO  1

  class Muxer
  {
    public:

      /**
       * @brief Event cb type. Refer to OMX IL spec for param details.
       */
      typedef OMX_ERRORTYPE (*EventCallbackType)(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_EVENTTYPE eEvent,
          OMX_IN OMX_U32 nData1,
          OMX_IN OMX_U32 nData2,
          OMX_IN OMX_PTR pEventData);

      /**
       * @brief Empty buffer done cb type. Refer to OMX IL spec for param details.
       */
      typedef OMX_ERRORTYPE (*EmptyDoneCallbackType)(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);


    public:

      /**
       * @brief Constructor
       *
       * @param pEmptyDoneFn Empty buffer done callback. Refer to OMX IL spec
       * @param pAppData Client data passed to buffer and event callbacks
       * @param eCodec The codec
       */
      Muxer(EmptyDoneCallbackType pEmptyDoneFn,
          OMX_PTR pAppData,eventHandlerType pEventHandler,OMX_U32 nModuleId);

      /**
       * @brief Destructor
       */
      ~Muxer();

    private:

      /**
       * @brief Private default constructor. Use public constructor.
       */
      Muxer();


    public:

      ///////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////
      // Init time methods
      ///////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////

      /**
       * @brief Configure the muxer
       *
       * @param pConfig The muxer configuration
       */
      OMX_ERRORTYPE Configure(MuxerConfigType* pConfig,
                              eventHandlerType pEventHandler,
                              OMX_U32 nModuleId); // can call run time too.

      /**
       * @brief ReConfigure the muxer
       *
       * @param pConfig The muxer configuration
       */
      OMX_ERRORTYPE Update(MuxerConfigType* pConfig); // can call run time too.


      /**
       * @brief Enables OMX_UseBuffer allocation scheme. Default is OMX_AllocateBuffer.
       *
       * @param bUseBuffer Set to OMX_TRUE for OMX_UseBuffer model
       */
      //OMX_ERRORTYPE EnableUseBufferModel(OMX_BOOL bInUseBuffer);
      OMX_ERRORTYPE EnableUseBufferModel(OMX_BOOL bInUseBuffer, OMX_U32 portIndex,
                                           OMX_BUFFERHEADERTYPE **pInBuffers, OMX_U32 nBuffers );


      OMX_ERRORTYPE SetNumBuffers(OMX_U32 nPortIndex, OMX_U32 nBuffers);
      /**
       * @brief Synchronously transitions the muxer to OMX_StateExecuting.
       *
       * Only valid in OMX_StateLoaded;
       */
      OMX_ERRORTYPE GoToExecutingState();



      ///////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////
      // Run time methods
      ///////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////

      /**
       * @brief Synchronously transitions the muxer to OMX_StateLoaded.
       *
       * Only valid in OMX_StateExecuting;
       */
      OMX_ERRORTYPE GoToLoadedState();

      /**
       * @brief Deliver an input  buffer to the muxer.
       *
       * @param pBuffer The populated input buffer.
       */
      OMX_ERRORTYPE DeliverInput(OMX_BUFFERHEADERTYPE* pBuffer);

      /**
       * @brief Get array of audio and video buffer pointers header.
       *
       * Only valid in the executing state after all buffers have been allocated.
       *
       * @param bIsInput Set to OMX_TRUE for input OMX_FALSE for output.
       * @return NULL upon failure, array of buffer header pointers otherwise.
       */
      OMX_BUFFERHEADERTYPE** GetBuffers(OMX_U32 nPortIndex);


      /**
       * @brief Set the muxer state
       *
       * This method can be asynchronous or synchronous. If asynchonous,
       * WaitState can be called to wait for the corresponding state
       * transition to complete.
       *
       * @param eState The state to enter
       * @param bSynchronous If OMX_TRUE, synchronously wait for the state transition to complete
       */
      OMX_ERRORTYPE SetState(OMX_STATETYPE eState,
          OMX_BOOL bSynchronous);

      /**
       * @brief Wait for the corresponding state transition to complete
       *
       * @param eState The state to wait for
       */
      OMX_ERRORTYPE WaitState(OMX_STATETYPE eState);

      /**
       * @brief Allocate all input and output buffers
       */
      OMX_ERRORTYPE AllocateBuffers();

      /**
       * @brief Free all input and output buffers
       */
      OMX_ERRORTYPE FreeBuffers();

      /*
       * @BRIEF Get number of audio/video buffers
       * @param PortIndex of either audio or video
       */
      OMX_ERRORTYPE GetInNumBuffers(OMX_U32 nPortIndex, OMX_U32 *nBuffer);

      /**
       * @brief Flush the muxer
       */
      OMX_ERRORTYPE Flush();

    private:

      static OMX_ERRORTYPE EventCallback(OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_EVENTTYPE eEvent,
          OMX_IN OMX_U32 nData1,
          OMX_IN OMX_U32 nData2,
          OMX_IN OMX_PTR pEventData);

      static OMX_ERRORTYPE EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

      static void RTPEventCallback(RTPEventType eEvent, RTPStatusType eStatus,
                             void* nEvtData, void *pAppData);

      void SendEventCallback(RTPEventType nEvent,
                             RTPStatusType nStatus, void* nData);
      OMX_ERRORTYPE UpdateRTPPort(MUX_RTPinfo *pRTPInfo);

    private:
      CRTCPReceiver   *m_pRTCPReceiver;
      int             m_nRTCPSock;
      int             m_nRTPSock;
      int             m_nRTPPort;
      int             m_nRTCPPort;
      MuxerConfigType m_sConfig;
      EventCallbackType m_pEventFn;
      EmptyDoneCallbackType m_pEmptyDoneFn;
      OMX_PTR m_pAppData;
      OMX_BOOL m_bAudioUseBuffer;
      OMX_BOOL m_bVideoUseBuffer;
      OMX_BOOL m_bAudioPortEnabled;
      OMX_BOOL m_bVideoPortEnabled;
      SignalQueue* m_pSignalQueue;
      OMX_BUFFERHEADERTYPE** m_pAudioBuffers;
      OMX_BUFFERHEADERTYPE** m_pVideoBuffers;
      OMX_BUFFERHEADERTYPE** m_pAudioBufferHdrs;
      OMX_BUFFERHEADERTYPE** m_pVideoBufferHdrs;
      OMX_U32 nCurrentAudioIndex;
      OMX_U32 nCurrentVideoIndex;
      eventHandlerType m_pEventHandler;
      OMX_U32 m_nModuleId;
      OMX_HANDLETYPE m_hMuxer;
      OMX_STATETYPE m_eState;
      OMX_S32 m_nAudioBuffers;
      OMX_S32 m_nVideoBuffers;
      OMX_S32 m_nAudioBufferSize;
      OMX_S32 m_nVideoBufferSize;
      video::iStreamPort *m_pSourcePort;
      OMX_PARAM_PORTDEFINITIONTYPE     omxPortAudio;
      OMX_PARAM_PORTDEFINITIONTYPE     omxPortVideo;
      QOMX_CONTAINER_FORMATTYPE m_eFormat;
      CRTPEncoder *pRTPEnc;

     };
#endif // #ifndef  _WFD_MM_SOURCE_MUX_H
