#ifndef __WFD_MM_SINK_AUDIO_DECODE_H__
#define __WFD_MM_SINK_AUDIO_DECODE_H__

/*==============================================================================
*       WFDMMSinkAudioDecode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSinkAudioDecode.
*
*
*  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
05/15/2013         SK            InitialDraft
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
#include "OMX_Core.h"
#include "OMX_IVCommon.h"
#include "OMX_Audio.h"
#include "OMX_Component.h"
#include "MMCriticalSection.h"
#include "WFDMMSinkCommon.h"
#include "qmmList.h"
#include "wdsm_mm_interface.h"

//#define WFD_DUMP_AUDIO_DATA

typedef struct audioConfig
{
    WFD_audio_type             eAudioType;
    uint32                     nSampleRate;
    uint32                     nChannels;
    uint32                     nBitsPerSample;
}audioConfigType;
#ifdef USE_OMX_AAC_CODEC
class  SignalQueue;

class WFDMMSinkAudioDecode
{
public:

    WFDMMSinkAudioDecode(int moduleId,
                         WFDMMSinkEBDType       pFnEBD,
                         WFDMMSinkFBDType       pFnFBD,
                         WFDMMSinkHandlerFnType pFnHandler,
                         int clientData);
    ~WFDMMSinkAudioDecode();

    OMX_ERRORTYPE  Configure(audioConfigType *pCfg);

    OMX_ERRORTYPE  DeliverInput
    (
        OMX_BUFFERHEADERTYPE *pBuffer
    );

    OMX_ERRORTYPE  SetFreeBuffer
    (
        OMX_BUFFERHEADERTYPE *pBuffer
    );

    OMX_ERRORTYPE  Stop();

    OMX_ERRORTYPE  Start();

private:
    OMX_ERRORTYPE  AllocateAudioBuffers();
    OMX_ERRORTYPE  allocateAudioInputBuffers();

    OMX_ERRORTYPE  allocateAudioOutputBuffers();
    OMX_ERRORTYPE  createResources();
    OMX_ERRORTYPE  destroyResources();

    bool  destroyAudioResources();

    bool  freeAudioBuffers();

    OMX_ERRORTYPE  freeAudioInputBuffers();

    OMX_ERRORTYPE  freeAudioOutputBuffers();

    bool  destroyThreadsAndQueues();

    bool  createThreadsAndQueues();

    OMX_ERRORTYPE  deinitialize();

    void  PrerequisitesforStop();

    void  InitData();

    int  state
    (
       int state,
       bool get_true_set_false
    );

    static OMX_ERRORTYPE  EventCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_EVENTTYPE eEvent,
          OMX_IN OMX_U32 nData1,
          OMX_IN OMX_U32 nData2,
          OMX_IN OMX_PTR pEventData);


    static OMX_ERRORTYPE  EmptyDoneCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    static OMX_ERRORTYPE  FillDoneCallback(
          OMX_IN OMX_HANDLETYPE hComponent,
          OMX_IN OMX_PTR pAppData,
          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_ERRORTYPE  SetState
    (
        OMX_STATETYPE eState,
        OMX_BOOL bSynchronous
    );

    OMX_ERRORTYPE  WaitState
    (
        OMX_STATETYPE eState
    );

    bool createAudioResources();
    bool HandlePortReconfig();
    bool GenerateAACHeaderFromADTS
    (
        uint8* pAdts, uint32 nAdtsLen,
        uint8 *pAacHeader, uint32 *nAACHdrLen
    );

    audioConfigType        mCfg;
    WFDMMSinkHandlerFnType mpFnHandler;
    WFDMMSinkFBDType       mpFnFBD;
    WFDMMSinkEBDType       mpFnEBD;
    SignalQueue *mAudioOutputQ;
    SignalQueue *mAudioInputQ;
    SignalQueue *mSignalQ;
    uint32 mnOutputPortIndex;
    uint32 mnInputPortIndex;
    uint32 mnNumInputBuffers;
    uint32 mnNumOutputBuffers;
    uint32 mnInputBufferSize;
    uint32 mnOutputBufferSize;
    OMX_HANDLETYPE mhAdec;
    OMX_AUDIO_CODINGTYPE meCompressionFmt;
    uint32 mnModuleId;
    uint32 mClientData;
    MM_HANDLE mhCritSect;
    int32  meState;
#ifdef WFD_DUMP_AUDIO_DATA
    FILE *mpPCMFile;
#endif
    bool   mbInputPortFound;
    bool   mbOutputPortFound;
    bool   mbAudioCodecHdrSet;

};
#else
//! WFDMMSinkAudioDecode class
//! Abstract interface for complete frame audio decode
class WFDMMSinkAudioDecode
{
public:
    static WFDMMSinkAudioDecode* create(WFD_audio_type eFmt);
    WFDMMSinkAudioDecode(){;};
    virtual ~WFDMMSinkAudioDecode();
    virtual OMX_ERRORTYPE  Configure(audioConfigType *pCfg)=0;
    virtual OMX_ERRORTYPE  Decode(OMX_BUFFERHEADERTYPE *pBuffer)=0;
};
#endif //USE_OMX_AAC_CODEC
#endif /*__WFD_MM_SINK_AUDIO_DECODE_H__*/

