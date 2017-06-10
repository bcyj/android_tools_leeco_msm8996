/*==============================================================================
*       WFDMMSinkAudioDecode.cpp
*
*  DESCRIPTION:
*       Abstracts OMX calls to audio decoder and provides more sensible APIs
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "WFDMMLogs.h"
#include "WFDMMSinkAudioDecode.h"
#include "WFDMMSourceMutex.h"
#include "WFDMMSourceSignalQueue.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "qmmList.h"
#include "MMTimer.h"
#include "OMX_QCOMExtns.h"
#include "QOMX_AudioExtensions.h"
#include "QOMX_AudioIndexExtensions.h"

#ifdef USE_OMX_AAC_CODEC
using namespace android;

#define MIN_AUDIO_BUFFERS 10

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifndef OMX_SPEC_VERSION
#define OMX_SPEC_VERSION 0x00000101
#endif
#define OMX_INIT_STRUCT(_s_, _name_)            \
    memset((_s_), 0x0, sizeof(_name_));          \
    (_s_)->nSize = sizeof(_name_);               \
    (_s_)->nVersion.nVersion = OMX_SPEC_VERSION


#define CRITICAL_SECT_ENTER if(mhCritSect)                                    \
                                  MM_CriticalSection_Enter(mhCritSect);       \
/*      END CRITICAL_SECT_ENTER    */

#define CRITICAL_SECT_LEAVE if(mhCritSect)                                    \
                                  MM_CriticalSection_Leave(mhCritSect);       \
/*      END CRITICAL_SECT_LEAVE    */
#define NOTIFYERROR  mpFnHandler((void*)mClientData,                           \
                                 mnModuleId,WFDMMSINK_ERROR,                   \
                                 OMX_ErrorUndefined,                           \
                                 0 );

struct CmdType
{
    OMX_EVENTTYPE   eEvent;
    OMX_COMMANDTYPE eCmd;
    OMX_U32         sCmdData;
    OMX_ERRORTYPE   eResult;
};


/*------------------------------------------------------------------------------
 State definitions
--------------------------------------------------------------------------------
*/
#define DEINIT  0
#define INIT    1
#define OPENED  2
#define CLOSING 4
#define CLOSED  3

#define ISSTATEDEINIT  (state(0, true) == DEINIT)
#define ISSTATEINIT    (state(0, true) == INIT)
#define ISSTATEOPENED  (state(0, true) == OPENED)
#define ISSTATECLOSED  (state(0, true) == CLOSED)
#define ISSTATECLOSING (state(0, true) == CLOSING)

#define SETSTATECLOSING (state(CLOSING, false))
#define SETSTATEINIT    (state(INIT   , false))
#define SETSTATEDEINIT  (state(DEINIT , false))
#define SETSTATEOPENED  (state(OPENED , false))

#ifdef WFD_DUMP_AUDIO_DATA

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

#endif

/*==============================================================================

         FUNCTION:         WFDMMSinkAudioDecode

         DESCRIPTION:
*//**       @brief         WFDMMSinkAudioDecode contructor
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSinkAudioDecode::WFDMMSinkAudioDecode(int moduleId,
                                     WFDMMSinkEBDType       pFnEBD,
                                     WFDMMSinkFBDType       pFnFBD,
                                     WFDMMSinkHandlerFnType pFnHandler,
                                     int clientData)
{

    InitData();
    /*--------------------------------------------------------------------------

    ----------------------------------------------------------------------------
    */
    mpFnHandler = pFnHandler;
    mpFnEBD     = pFnEBD;
    mpFnFBD     = pFnFBD;
    mnModuleId  = moduleId;
    mClientData = clientData;

    mhCritSect = NULL;
    int nRet = MM_CriticalSection_Create(&mhCritSect);
    if(nRet != 0)
    {
        mhCritSect = NULL;
        WFDMMLOGE("Error in Critical Section Create");
    }

    SETSTATEDEINIT;

}

/*==============================================================================

         FUNCTION:         ~WFDMMSinkAudioDecode

         DESCRIPTION:       Destructor
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
WFDMMSinkAudioDecode::~WFDMMSinkAudioDecode()
{
    /*--------------------------------------------------------------------------
     Call destroy resources to make sure destructor is not called in the
     middle of play
    ----------------------------------------------------------------------------
    */
    destroyResources();

    WFDMMLOGH("WFDMMSinkAudioDecode Destructor");

    if (mhCritSect)
    {
        MM_CriticalSection_Release(mhCritSect);
    }
}

/*==============================================================================

         FUNCTION:          InitData

         DESCRIPTION:       Initializes class variables
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
void WFDMMSinkAudioDecode::InitData()
{
    memset(&mCfg, 0, sizeof(mCfg));
    mpFnHandler = NULL;
    mpFnFBD = NULL;
    mpFnEBD = NULL;
    mAudioOutputQ = NULL;
    mAudioInputQ = NULL;
    mSignalQ = NULL;
    mnOutputPortIndex = 0;
    mnInputPortIndex = 0;
    mnNumInputBuffers = 0;
    mnNumOutputBuffers = 0;
    mhAdec = NULL;
    meCompressionFmt = OMX_AUDIO_CodingAAC;
    mnModuleId = 0;
    mClientData = 0;
    mhCritSect = NULL;
    meState = DEINIT;
#ifdef WFD_DUMP_AUDIO_DATA
    mpPCMFile = NULL;
#endif
    mbInputPortFound = false;
    mbOutputPortFound = false;
    mbAudioCodecHdrSet = false;
    return;
}

/*==============================================================================

         FUNCTION:         Configure

         DESCRIPTION:
*//**       @brief         Configures the renderer
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pCfg        - Configuration Parameters

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::Configure(audioConfigType *pCfg)
{
    CHECK_ARGS_RET_OMX_ERR_4(pCfg,
                             pCfg->nSampleRate, pCfg->nChannels, mpFnFBD);

    memcpy(&mCfg, pCfg, sizeof(audioConfigType));

    SETSTATEINIT;
    if(createResources() != OMX_ErrorNone)
    {
        WFDMMLOGE("AudioDec Failed to createResources");
        NOTIFYERROR;
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          AllocateAudioBuffers

         DESCRIPTION:
*//**       @brief         Allocates Audio Buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other Errors
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::AllocateAudioBuffers()
{
    OMX_ERRORTYPE eErr = OMX_ErrorNone;

    eErr = allocateAudioInputBuffers();

    if (eErr != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to allocate Input Buffers");
        return eErr;
    }

    eErr = allocateAudioOutputBuffers();

    if (eErr != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to allocate Output Buffers");
        return eErr;
    }

    return eErr;
}

/*==============================================================================

         FUNCTION:          allocateAudioInputBuffers

         DESCRIPTION:
*//**       @brief          allocates Audio Input Buffers OMX component allocs
                            the buffers.
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
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::allocateAudioInputBuffers()
{
    OMX_ERRORTYPE eErr = OMX_ErrorNone;

    /*--------------------------------------------------------------------------
      We will use allocate Buffer model to let the decoder decide the buffers it
      needs
    ----------------------------------------------------------------------------
    */

    /*--------------------------------------------------------------------------
     Create Buffers one by one and maintain locally in a queue. Will be useful
     when freeing the buffers
    ----------------------------------------------------------------------------
    */
    OMX_BUFFERHEADERTYPE *pBuffer;

    for (unsigned int i = 0; i < mnNumInputBuffers; i++)
    {
        eErr = OMX_AllocateBuffer(mhAdec,
                                 &pBuffer,
                                  mnInputPortIndex,
                                  NULL,
                                  mnInputBufferSize);

        if(eErr != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to allocateBuffer");
            return eErr;
        }

        mAudioInputQ->Push(&pBuffer, sizeof(&pBuffer));
    }

    return eErr;
}

/*==============================================================================

         FUNCTION:          allocateAudioOutputBuffers

         DESCRIPTION:
*//**       @brief          Allocates output buffers using Audio decoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::allocateAudioOutputBuffers()
{
    OMX_ERRORTYPE eErr = OMX_ErrorUndefined;

    OMX_BUFFERHEADERTYPE *pBuffer = NULL;
    for(unsigned int i = 0; i < mnNumOutputBuffers; i++)
    {
        eErr = OMX_AllocateBuffer(mhAdec,
                                 &pBuffer,
                                  mnOutputPortIndex,
                                  NULL,
                                  mnOutputBufferSize);

        if(eErr != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to allocateBuffer");
            return eErr;
        }

        mAudioOutputQ->Push(&pBuffer, sizeof(&pBuffer));
    }

    return eErr;
}


/*==============================================================================

         FUNCTION:         createResources

         DESCRIPTION:
*//**       @brief         creates resources for WFDMMSinkAudioDecode
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::createResources()
{
    if(!createThreadsAndQueues())
    {
        WFDMMLOGE("WFDAudioDec Failed to create Threads And Queues");
        return OMX_ErrorInsufficientResources;
    }

    if(!createAudioResources())
    {
        WFDMMLOGE("WFDAudioDec Failed to create AudioResources");
        return OMX_ErrorInsufficientResources;
    }

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         createAudioResources

         DESCRIPTION:
*//**       @brief         creates all audio dynamic resources
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
bool WFDMMSinkAudioDecode::createAudioResources()
{
    /*--------------------------------------------------------------------------
     Create OMX decoder. This is going to be a long function
    ----------------------------------------------------------------------------
    */

    static OMX_CALLBACKTYPE sCallbacks =
    {
        EventCallback,
        EmptyDoneCallback,
        FillDoneCallback
    };


    OMX_ERRORTYPE eRet = OMX_ErrorUndefined;

    switch (mCfg.eAudioType)
    {
    case WFD_AUDIO_AAC:
        meCompressionFmt = OMX_AUDIO_CodingAAC;
        eRet = OMX_GetHandle(&mhAdec,
              (OMX_STRING)"OMX.qcom.audio.decoder.multiaac",
              this,
              &sCallbacks);
        WFDMMLOGD("Opening Audio Comp OMX.qcom.audio.decoder.aac");
        break;
    case WFD_AUDIO_DOLBY_DIGITAL:
    default:
        WFDMMLOGE("Unsupported Audio Codec");
        return false;
    }

    if(eRet != OMX_ErrorNone || !mhAdec)
    {
        WFDMMLOGE1("Error getting component handle = %x", eRet);
        return false;
    }

    WFDMMLOGH("Audio OMX Component created");

    /*--------------------------------------------------------------------------
     At this point the component will be in State LOADED
    ----------------------------------------------------------------------------
    */


    OMX_PORT_PARAM_TYPE sPortParam;
    OMX_AUDIO_PARAM_PORTFORMATTYPE sPortFmt;
    OMX_PARAM_PORTDEFINITIONTYPE     sPortDef;

    memset(&sPortParam, 0, sizeof(sPortParam));
    memset(&sPortDef, 0, sizeof(sPortDef));
    memset(&sPortFmt, 0, sizeof(sPortFmt));
    OMX_INIT_STRUCT(&sPortParam, OMX_PORT_PARAM_TYPE);
    OMX_INIT_STRUCT(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    OMX_INIT_STRUCT(&sPortFmt, OMX_AUDIO_PARAM_PORTFORMATTYPE);

    /*--------------------------------------------------------------------------
      Find the available ports
    ----------------------------------------------------------------------------
    */
    eRet = OMX_GetParameter(mhAdec,
                            OMX_IndexParamAudioInit,
                            (OMX_PTR)&sPortParam);

    if (eRet != OMX_ErrorNone || sPortParam.nPorts == 0)
    {
        WFDMMLOGE("Error in getting Audio Ports");
        return false;
    }

    WFDMMLOGH1("Found Audio Ports %lu", sPortParam.nPorts);

    /*--------------------------------------------------------------------------
       Find Port Directions
    ----------------------------------------------------------------------------
    */
    for (uint32 i = 0; i < sPortParam.nPorts; i++)
    {
        sPortDef.nPortIndex = i;
        eRet = OMX_GetParameter(mhAdec,
                                OMX_IndexParamPortDefinition,
                                (OMX_PTR) &sPortDef);
        if (eRet != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to get PortDef for Direction");
            return false;
        }

        if (sPortDef.eDir == OMX_DirInput)
        {
            WFDMMLOGH("Input port found");

            if (!mbInputPortFound)
            {
                mbInputPortFound = true;
                mnInputPortIndex = i;
                mnNumInputBuffers = sPortDef.nBufferCountMin;
                mnNumInputBuffers = MAX(mnNumInputBuffers, MIN_AUDIO_BUFFERS);
                sPortDef.nBufferCountActual = mnNumInputBuffers;
                mnInputBufferSize = sPortDef.nBufferSize;
                sPortDef.format.audio.eEncoding = OMX_AUDIO_CodingAAC;

                eRet = OMX_SetParameter(mhAdec,
                                        OMX_IndexParamPortDefinition,
                                        (OMX_PTR) &sPortDef);

                if (eRet != OMX_ErrorNone)
                {
                    WFDMMLOGE("Faled to configure Input Port with codec");
                    return false;
                }

            }
        }

        if (sPortDef.eDir == OMX_DirOutput)
        {
            WFDMMLOGH("Output port found");
            if (!mbOutputPortFound)
            {
                mbOutputPortFound = true;
                mnOutputPortIndex = i;
                mnNumOutputBuffers = sPortDef.nBufferCountMin;
                mnNumOutputBuffers = MAX(mnNumOutputBuffers, MIN_AUDIO_BUFFERS);
                sPortDef.nBufferCountActual = mnNumOutputBuffers;
                sPortDef.format.audio.eEncoding = OMX_AUDIO_CodingPCM;
                mnOutputBufferSize = sPortDef.nBufferSize;

                eRet = OMX_SetParameter(mhAdec,
                                        OMX_IndexParamPortDefinition,
                                        (OMX_PTR) &sPortDef);

                if (eRet != OMX_ErrorNone)
                {
                    WFDMMLOGE("Faled to configure Output Port with color");
                    return false;
                }

            }
        }
    }


    WFDMMLOGH("Found Input/Output Ports");


    if (mCfg.eAudioType == WFD_AUDIO_AAC)
    {
        OMX_AUDIO_PARAM_AACPROFILETYPE sAACProfile;
        OMX_INIT_STRUCT(&sAACProfile, OMX_AUDIO_PARAM_AACPROFILETYPE);

        sAACProfile.eAACProfile = OMX_AUDIO_AACObjectLC;
        sAACProfile.eAACStreamFormat = OMX_AUDIO_AACStreamFormatMP2ADTS;
        sAACProfile.nBitRate = 512000;
        sAACProfile.nChannels = mCfg.nChannels;
        sAACProfile.nFrameLength = 4096;
        sAACProfile.nPortIndex = mnInputPortIndex;
        sAACProfile.nSampleRate = mCfg.nSampleRate;

        eRet = OMX_SetParameter(mhAdec, OMX_IndexParamAudioAac,
                                              &sAACProfile);

        if (eRet != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to configure Audio AAC settings");
            return false;
        }
/*
         QOMX_AUDIO_CONFIG_DUALMONOTYPE sDualMono;
         OMX_INIT_STRUCT(&sDualMono, QOMX_AUDIO_CONFIG_DUALMONOTYPE);

         sDualMono.eChannelConfig = OMX_AUDIO_DUAL_MONO_MODE_DEFAULT;

         eRet = OMX_SetConfig(mhAdec,
                             (OMX_INDEXTYPE)QOMX_IndexConfigAudioDualMono,
                              &sDualMono);

         if (eRet != OMX_ErrorNone)
         {
             WFDMMLOGH("AudioDecode Failed to set DualMono COnfig");
         }*/
    }

    OMX_AUDIO_PARAM_PCMMODETYPE sPCM;

    OMX_INIT_STRUCT(&sPCM, OMX_AUDIO_PARAM_PCMMODETYPE);

    //sPCM.eChannelMapping =
    sPCM.eEndian = OMX_EndianLittle;
    sPCM.eNumData = OMX_NumericalDataSigned;
    sPCM.ePCMMode = OMX_AUDIO_PCMModeLinear;
    sPCM.nBitPerSample = 16;
    sPCM.nChannels  = mCfg.nChannels;
    sPCM.nPortIndex = mnOutputPortIndex;
    sPCM.nSamplingRate = mCfg.nSampleRate;

    eRet = OMX_SetParameter(mhAdec, OMX_IndexParamAudioPcm,
                                          &sPCM);

    if (eRet != OMX_ErrorNone)
    {
        WFDMMLOGE("AudioDecode Failed to set PCM Output Params");
    }

    /*--------------------------------------------------------------------------
      Get ready to move the state to Idle.
    ----------------------------------------------------------------------------
    */
    eRet = SetState(OMX_StateIdle, OMX_FALSE);

    if(AllocateAudioBuffers() != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to allocate Audio Buffers");
        return false;
    }

    if(WaitState(OMX_StateIdle) != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to move to IDLE state");
    }

    /*--------------------------------------------------------------------------
      Buffer Allocations are Done. Now move the Component to executing
    ----------------------------------------------------------------------------
    */
    eRet = SetState(OMX_StateExecuting, OMX_TRUE);

    if (eRet != OMX_ErrorNone)
    {
        WFDMMLOGE("FAiled to move the component to executing");
        return false;
    }

    WFDMMLOGH("WFDMMSinkAudioDecode moves to Executing");


#ifdef WFD_DUMP_AUDIO_DATA
    mpPCMFile = fopen("/data/media/audiodump.wav","wb");
    if(!mpPCMFile)
    {
        WFDMMLOGE("AudioSource::audiodump fopen failed");
    }
    else
    {
        wav_header hdr;
        hdr.riff_id = ID_RIFF;
        hdr.riff_fmt = ID_WAVE;
        hdr.fmt_id = ID_FMT;
        hdr.fmt_sz = 16;
        hdr.audio_format = 1;
        hdr.num_channels = mCfg.nChannels;
        hdr.sample_rate = mCfg.nSampleRate;
        hdr.bits_per_sample = 16;
        hdr.byte_rate = (mCfg.nSampleRate * mCfg.nChannels * 16) / 8;
        hdr.block_align = ( mCfg.nSampleRate * mCfg.nChannels ) / 8;
        hdr.data_id = ID_DATA;
        hdr.data_sz = 2147483648LL;
        hdr.riff_sz = hdr.data_sz + 44 - 8;
        fwrite(&hdr,1, sizeof(hdr),mpPCMFile);
        WFDMMLOGD("AudioSource::Writing  wav header");
    }
#endif

    SETSTATEINIT;
    return true;
}


/*==============================================================================

         FUNCTION:         destroyResources

         DESCRIPTION:
*//**       @brief         releases dynamic resources
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ErrorNone or other error
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::destroyResources()
{
    if(!destroyAudioResources())
    {
        return OMX_ErrorUndefined;
    }

    if(!destroyThreadsAndQueues())
    {
        return OMX_ErrorUndefined;
    }


    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         destroyAudioResources

         DESCRIPTION:
*//**       @brief         release Audio dynamic resources
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            true or false
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkAudioDecode::destroyAudioResources()
{
    if (!mhAdec)
    {
        return false;
    }
    /*--------------------------------------------------------------------------
     destroy OMX decoder.
    ----------------------------------------------------------------------------
    */

    OMX_ERRORTYPE eRet = OMX_ErrorUndefined;

    /*--------------------------------------------------------------------------
      Wait for all buffers to be back
    ----------------------------------------------------------------------------
    */

    uint32 nNumInBuffersInQ = 0;
    uint32 nNumOutBuffersInQ = 0;

    WFDMMLOGH("Waiting for all Buffers to be returned");

    if (ISSTATEOPENED)
    {
        Stop();
    }

    /*--------------------------------------------------------------------------
      At this point the assumption is that buffer exchanges have been stopped
    ----------------------------------------------------------------------------
    */

    /*--------------------------------------------------------------------------
      Move state to Idle
    ----------------------------------------------------------------------------
    */
    /*--------------------------------------------------------------------------
      Get ready to move the state to Idle.
    ----------------------------------------------------------------------------
    */
    eRet = SetState(OMX_StateIdle, OMX_TRUE);

    if(eRet != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to move to Idle");
    }

    eRet = SetState(OMX_StateLoaded, OMX_FALSE);
    if(eRet != OMX_ErrorNone)
    {
        WFDMMLOGE("Failed to move to State loaded");
    }


    if(!freeAudioBuffers())
    {
        WFDMMLOGE("Failed to free Audio Buffers");
    }

    eRet = WaitState(OMX_StateLoaded);

    if (eRet != OMX_ErrorNone)
    {
        WFDMMLOGH("Failed to wait to move to loaded");
    }

    OMX_FreeHandle(mhAdec);

    mhAdec = NULL;
#ifdef WFD_DUMP_AUDIO_DATA
    if (mpPCMFile)
    {
        fclose(mpPCMFile);
        mpPCMFile = NULL;
    }
#endif

    WFDMMLOGH("Cleaned up audio resources");

    return true;
}

/*==============================================================================

         FUNCTION:          freeAudioBuffers

         DESCRIPTION:
*//**       @brief          frees Audio Buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            true for success else false
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkAudioDecode::freeAudioBuffers()
{
    OMX_ERRORTYPE eErr = OMX_ErrorNone;

    (void)freeAudioInputBuffers();
    (void)freeAudioOutputBuffers();

    return true;
}

/*==============================================================================

         FUNCTION:          freeAudioInputBuffers

         DESCRIPTION:
*//**       @brief          Asks OMX module to free input buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone for success or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::freeAudioInputBuffers()
{

    OMX_BUFFERHEADERTYPE *pBuffer = NULL;
    OMX_ERRORTYPE  eErr = OMX_ErrorNone;

    if(!mAudioInputQ)
    {
        WFDMMLOGE("AudioDecode: Audio Ip Q is NULL. No Buffer expected");
        return OMX_ErrorNone;
    }

    uint32 qSize = mAudioInputQ->GetSize();

    if(mhAdec && qSize)
    {
        for(uint32 i = 0; i < qSize; i++)
        {
            mAudioInputQ->Pop(&pBuffer, sizeof(&pBuffer), 0);
            if(pBuffer)
            {
                eErr = OMX_FreeBuffer(mhAdec, mnInputPortIndex, pBuffer);

                if (eErr != OMX_ErrorNone)
                {
                    WFDMMLOGE("Failed to Free Buffer");
                }
                pBuffer = NULL;
            }
        }
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          freeAudioOutputBuffers

         DESCRIPTION:
*//**       @brief          Uses native window APIs to free output buffers
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::freeAudioOutputBuffers()
{

    OMX_BUFFERHEADERTYPE *pBuffer = NULL;
    OMX_ERRORTYPE  eErr = OMX_ErrorNone;

    if(!mAudioOutputQ)
    {
        WFDMMLOGE("AudioDecode: Audio Q is NULL. No Buffer expected");
        return OMX_ErrorNone;
    }
    uint32 qSize = mAudioOutputQ->GetSize();

    if(mhAdec && qSize)
    {
        for(uint32 i = 0; i < qSize; i++)
        {
            mAudioOutputQ->Pop(&pBuffer, sizeof(&pBuffer), 0);
            if(pBuffer)
            {
                eErr = OMX_FreeBuffer(mhAdec, mnOutputPortIndex, pBuffer);

                if (eErr != OMX_ErrorNone)
                {
                    WFDMMLOGE("Failed to Free Buffer");
                }
                pBuffer = NULL;
            }
        }
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          destroyThreadsAndQueues

         DESCRIPTION:
*//**       @brief          Frees the threads and Queues used in this module
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
bool WFDMMSinkAudioDecode::destroyThreadsAndQueues()
{
    if(mSignalQ)
    {
        MM_Delete(mSignalQ);
        mSignalQ = NULL;
    }

    if(mAudioOutputQ)
    {
        MM_Delete(mAudioOutputQ);
        mAudioOutputQ = NULL;
    }

    if (mAudioInputQ)
    {
        MM_Delete(mAudioOutputQ);
        mAudioOutputQ = NULL;
    }

    return true;

}

/*==============================================================================

         FUNCTION:         createThreadsAndQueues

         DESCRIPTION:
*//**       @brief         creates threads and queues needed for the module
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
bool WFDMMSinkAudioDecode::createThreadsAndQueues()
{
    mSignalQ = MM_New_Args(SignalQueue,(100, sizeof(CmdType)));
    mAudioOutputQ = MM_New_Args(SignalQueue, (100, sizeof(int*)));
    mAudioInputQ  = MM_New_Args(SignalQueue, (100, sizeof(int*)));

    if(!mSignalQ || !mAudioOutputQ || !mAudioInputQ)
    {
        WFDMMLOGE("Failed to create one of the Quueues");
        return false;
    }
    return true;
}

/*==============================================================================

         FUNCTION:         deinitialize

         DESCRIPTION:
*//**       @brief         deallocated all resources
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::deinitialize()
{

    SETSTATECLOSING;

    WFDMMLOGH("Wait for all buffers to be returned");


    /*--------------------------------------------------------------------------
      Close Threads and signal queues
    ----------------------------------------------------------------------------
    */
    if (mSignalQ)
    {
        MM_Delete(mSignalQ);
        mSignalQ = NULL;
    }

    if (mAudioInputQ)
    {
        MM_Delete(mAudioInputQ);
        mAudioInputQ = NULL;
    }

    if (mAudioOutputQ)
    {
        MM_Delete(mAudioOutputQ);
        mAudioOutputQ = NULL;
    }

    return OMX_ErrorNone;
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
int WFDMMSinkAudioDecode::state(int state, bool get_true_set_false)
{
    CRITICAL_SECT_ENTER;

    if(get_true_set_false == true)
    {
        /*----------------------------------------------------------------------
          Just return the current state
        ------------------------------------------------------------------------
        */

    }
    else
    {
        meState = state;
    }

    CRITICAL_SECT_LEAVE;

    return meState;
}

/*==============================================================================

         FUNCTION:          EventCallback

         DESCRIPTION:
*//**       @brief          Function for openmax component event callback
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or Other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::EventCallback(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_EVENTTYPE eEvent,
      OMX_IN OMX_U32 nData1,
      OMX_IN OMX_U32 nData2,
      OMX_IN OMX_PTR pEventData)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    WFDMMLOGH1("Event callback with %d event", eEvent);
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
                WFDMMLOGH("state status Invalid");
                break;
            }

            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandStateSet;
            cmd.sCmdData = nData2;
            cmd.eResult = result;

            result = ((WFDMMSinkAudioDecode*)pAppData)->
                                mSignalQ->Push(&cmd, sizeof(cmd));
        }
        else if ((OMX_COMMANDTYPE) nData1 == OMX_CommandFlush)
        {
            WFDMMLOGH("Event callback with flush status");
/*            CmdType cmd;
            cmd.eEvent = OMX_EventCmdComplete;
            cmd.eCmd = OMX_CommandFlush;
            cmd.sCmdData = 0;
            cmd.eResult = result;
            result = ((WFDMMSinkAudioDecode*)pAppData)->
                                                  mSignalQ->Push(&cmd,
                                                               sizeof(cmd));*/
        }
        else
        {
            WFDMMLOGE("error status");
            WFDMMLOGE("Unimplemented command");
        }
    }
    else if (eEvent == OMX_EventError)
    {
        WFDMMLOGH("async error");
        CmdType cmd;
        cmd.eEvent = OMX_EventError;
        cmd.eCmd = OMX_CommandMax;
        cmd.sCmdData = 0;
        cmd.eResult = (OMX_ERRORTYPE) nData1;
                  ((WFDMMSinkAudioDecode*) pAppData)->mpFnHandler(
           (void*)((WFDMMSinkAudioDecode*) pAppData)->mClientData,
                  ((WFDMMSinkAudioDecode*) pAppData)->mnModuleId,
                                                      WFDMMSINK_ERROR,
                                                      OMX_ErrorUndefined,
                                                      0);
        return result;
    }
    else if (eEvent == OMX_EventBufferFlag)
    {
        WFDMMLOGH("got buffer flag event");
    }
    else if (eEvent == OMX_EventPortSettingsChanged)
    {
        if (nData2 == OMX_IndexConfigCommonOutputCrop)
        {
            WFDMMLOGH("PortSettingsCHanged==> OMX_IndexConfigCommonOutputCrop");
            ((WFDMMSinkAudioDecode*) pAppData)->HandlePortReconfig();

        }
    }
    else
    {
        WFDMMLOGH("Unimplemented event");
    }

    return result;
}

/*==============================================================================

         FUNCTION:          EmptyDoneCallback

         DESCRIPTION:
*//**       @brief          OMX component calls this when data in a buffer
                            is consumed
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or Other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::EmptyDoneCallback(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!pBuffer || !pAppData)
    {
        WFDMMLOGE("EmptyDoneCallBack:Invalid Args");
        return OMX_ErrorBadParameter;
    }


    WFDMMLOGM1("WFDMMSinkAdec::EmptyDoneCallback nFilledLen = %lu",
               pBuffer->nFilledLen);

    if (((WFDMMSinkAudioDecode*)pAppData)->mpFnEBD)
    {
        ((WFDMMSinkAudioDecode*)pAppData)->mpFnEBD(
                     ((WFDMMSinkAudioDecode*)pAppData)->mClientData,
                     ((WFDMMSinkAudioDecode*)pAppData)->mnModuleId,
                      SINK_AUDIO_TRACK_ID,
                      pBuffer);
    }

    return result;
}


/*==============================================================================

         FUNCTION:          FillDoneCallback

         DESCRIPTION:
*//**       @brief          Callback indicatin OMX Module has completed
                            filling the buffer
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error code
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::FillDoneCallback(
      OMX_IN OMX_HANDLETYPE hComponent,
      OMX_IN OMX_PTR pAppData,
      OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{

    if (!pBuffer || !pAppData)
    {
        WFDMMLOGE("FillDoneCallBack:Invalid Args");
        return OMX_ErrorBadParameter;
    }

    OMX_ERRORTYPE result = OMX_ErrorNone;

    WFDMMLOGM2("WFDMMSinkAdec::FillDoneCallback Size = %lu, TS = %lu",
               pBuffer->nFilledLen, (uint32)pBuffer->nTimeStamp);

    if (((WFDMMSinkAudioDecode*)pAppData)->mpFnEBD)
    {
        ((WFDMMSinkAudioDecode*)pAppData)->mpFnFBD(
                     ((WFDMMSinkAudioDecode*)pAppData)->mClientData,
                     ((WFDMMSinkAudioDecode*)pAppData)->mnModuleId,
                      SINK_AUDIO_TRACK_ID,
                      pBuffer);
#ifdef WFD_DUMP_AUDIO_DATA
        if (((WFDMMSinkAudioDecode*)pAppData)->mpPCMFile
             && pBuffer->nFilledLen)
        {
            fwrite(pBuffer->pBuffer + pBuffer->nOffset,
                    1, pBuffer->nFilledLen,
                   ((WFDMMSinkAudioDecode*)pAppData)->mpPCMFile);
        }
#endif
    }

    return result;
}


/*==============================================================================

         FUNCTION:          SetState

         DESCRIPTION:
*//**       @brief          Asks OMX module to move to particular state
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::SetState
(
    OMX_STATETYPE eState,
    OMX_BOOL bSynchronous
)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    if(!mhAdec)
    {
        return OMX_ErrorInsufficientResources;
    }

    result = OMX_SendCommand(mhAdec,
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
                WFDMMLOGE("failed to wait state");
            }
        }
    }
    else
    {
        WFDMMLOGE("failed to set state");
    }
    return result;
}

/*==============================================================================

         FUNCTION:          WaitState

         DESCRIPTION:
*//**       @brief          Waits for state transition to complete
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
OMX_ERRORTYPE WFDMMSinkAudioDecode::WaitState
(
    OMX_STATETYPE eState
)
{
    OMX_ERRORTYPE result = OMX_ErrorUndefined;
    CmdType cmd;
    uint32  nNumIter = 50;

    while (result != OMX_ErrorNone && nNumIter--)
    {
        result =  mSignalQ->Pop(&cmd, sizeof(cmd), 100);

        if (result != OMX_ErrorTimeout && result != OMX_ErrorNone)
        {
            WFDMMLOGE("Error in popping from Signal Queue");
            return result;
        }
    }

    result = cmd.eResult;

    if (cmd.eEvent == OMX_EventCmdComplete)
    {
        if (cmd.eCmd == OMX_CommandStateSet)
        {
            if ((OMX_STATETYPE) cmd.sCmdData == eState)
            {
                //meState = (OMX_STATETYPE) cmd.sCmdData;
            }
            else
            {
                WFDMMLOGE1("wrong state (%d)", (int) cmd.sCmdData);
                result = OMX_ErrorUndefined;
            }
        }
        else
        {
            WFDMMLOGE("expecting state change");
            result = OMX_ErrorUndefined;
        }
    }
    else
    {
        WFDMMLOGH("expecting cmd complete");
        if(mpFnHandler)
        {
            mpFnHandler((void*)mClientData,
                        mnModuleId,
                        WFDMMSINK_ERROR,
                        OMX_ErrorUndefined,
                        0);
        }
        result = OMX_ErrorUndefined;
    }

    return result;
}

/*==============================================================================

         FUNCTION:          DeliverInput

         DESCRIPTION:
*//**       @brief          Provides an input buffer for processing
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
OMX_ERRORTYPE WFDMMSinkAudioDecode::DeliverInput
(
    OMX_BUFFERHEADERTYPE *pBuffer
)
{
    OMX_ERRORTYPE eErr = OMX_ErrorUndefined;

    if (!pBuffer)
    {
        WFDMMLOGE("Invalid Buffer Pointer in DeliverInput");
        return OMX_ErrorBadParameter;
    }

    if(!ISSTATEOPENED || !mhAdec)
    {
        WFDMMLOGE("Emptythisbuffer in invalid state");
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBuffer->nInputPortIndex != mnInputPortIndex)
    {
        WFDMMLOGE("Invalid PortIndex in DeliverInput");
        return OMX_ErrorBadPortIndex;
    }

    if(0)//mCfg.eAudioType == WFD_AUDIO_AAC)
    {
        if(!mbAudioCodecHdrSet)
        {
            uint8 sAACHeader[2];
            uint32 nSize = 2;

            WFDMMLOGH("Generate AAC Raw Header");
            bool bRet = GenerateAACHeaderFromADTS((uint8*)pBuffer->pBuffer,
                                      (uint32)pBuffer->nFilledLen,
                                      (uint8*)sAACHeader,
                                      &nSize);
            pBuffer->pBuffer[0] = sAACHeader[0];
            pBuffer->pBuffer[1] = sAACHeader[1];
            pBuffer->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
            pBuffer->nFilledLen = 2;
         /*   memmove(pBuffer->pBuffer + 2,
                    pBuffer->pBuffer + 7,
                    pBuffer->nFilledLen - 7);
            pBuffer->nFilledLen += sizeof(sAACHeader);
            pBuffer->nFilledLen -= 7;
*/
            mbAudioCodecHdrSet = true;
        }
        else
        {
            /*------------------------------------------------------------------
             Audio OMX decoder doesn't support using pBuffer->nOffset.
             It could have avoided the memcpy. Bad
            --------------------------------------------------------------------
            */
            WFDMMLOGD("Strip ADTS Header");
            memmove(pBuffer->pBuffer,
                    pBuffer->pBuffer + 7,
                    pBuffer->nFilledLen - 7);
            pBuffer->nFilledLen -= 7;
        }
    }

    WFDMMLOGH("Calling Audio Empty this Buffer");
    eErr = OMX_EmptyThisBuffer(mhAdec, pBuffer);

    if (eErr != OMX_ErrorNone)
    {
        WFDMMLOGE("FAiled to call EmptythisBuffer");
        return eErr;
    }
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          SetFreeBuffer

         DESCRIPTION:
*//**       @brief          Provides a free o/p buffer so that it can be
                            queue back to OMX Decoder
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::SetFreeBuffer
(
    OMX_BUFFERHEADERTYPE *pBuffer
)
{
    OMX_ERRORTYPE eErr = OMX_ErrorUndefined;

    if (!pBuffer)
    {
        WFDMMLOGE("Invalid Buffer Pointer in DeliverInput");
        return OMX_ErrorBadParameter;
    }

    if (pBuffer->nOutputPortIndex != mnOutputPortIndex)
    {
        WFDMMLOGE("Invalid PortIndex in DeliverInput");
        return OMX_ErrorBadPortIndex;
    }

    if(!ISSTATEOPENED || !mhAdec)
    {
        /*----------------------------------------------------------------------
          This module is the rightful owner of the buffer at death ot stop of
          the module. So hold the buffer in the collectorQ
        ------------------------------------------------------------------------
        */
        mAudioOutputQ->Push(&pBuffer, sizeof(&pBuffer));
        return OMX_ErrorNone;
    }
    else
    {
        eErr = OMX_FillThisBuffer(mhAdec, pBuffer);

        if (eErr != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to call EmptythisBuffer");
            return eErr;
        }

    }

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          Stop

         DESCRIPTION:
*//**       @brief          Stops buffer flow
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
OMX_ERRORTYPE WFDMMSinkAudioDecode::Stop()
{
    if(!ISSTATEOPENED)
    {
        /*----------------------------------------------------------------------
           If the state is not opened we dont need to stop buffer flow.

           WARNING. This call must be made from the same thread that has created
           the resources. Otherwise there may be race conditions. - SK
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("Audio State not opened Stop is Done!!!");
        return OMX_ErrorNone;
    }

    /*--------------------------------------------------------------------------
      Now this is tricky. But with some care we can do the trick.

      Things to do.
      1. Change the state to something like stopping.
      2. Stop the buffer flow into decoder.
      3. Allow any outflow of buffer from decoder.
      4. Use the collector Q to collect all buffers that has been allocated
         for output. Remember that AudioDecoder is the rightful owner of these
         buffers.
      5. Flush the input/output port. Owner of input buffers at stop is the
         mediasource and output is collector queue
      6. Wait for buffers to flow normally and finally get collected in the
         collector Q.

    ----------------------------------------------------------------------------
    */

    /* 1.)*/SETSTATECLOSING;

    /* 2.) This will be done by SetFreeBuffer*/

    /* 3.) This will be done by EmptyBufferDone and FillBufferDone callbacks*/

    /* 4 )*/
    /* 5 )*/

    if (mhAdec)
    {
        OMX_ERRORTYPE eErr = OMX_ErrorNone;
        eErr = OMX_SendCommand(mhAdec,
                                 OMX_CommandFlush,
                                 OMX_ALL,
                                 NULL);

        if (eErr != OMX_ErrorNone)
        {
            WFDMMLOGE("Failed to issue flush command");
            /*------------------------------------------------------------------
               Nothing much to do. This will lead to some problem
            --------------------------------------------------------------------
            */
        }
    }

    /* 6 )*/
    WFDMMLOGH("Waiting for output Buffers to be collected");
    while ((uint32)mAudioOutputQ->GetSize() < mnNumOutputBuffers)
    {
        MM_Timer_Sleep(2);
    }

    WFDMMLOGH("OutputBuffers Collected");


    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          Start

         DESCRIPTION:
*//**       @brief          Make the AudioDecoder ready for buffer exchange.
                            Gives o/p buffers to OMX component and i/p buffers
                            to media source
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            OMX_ErrorNone or other error
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSinkAudioDecode::Start()
{
    OMX_BUFFERHEADERTYPE *pBuffer;
    /*--------------------------------------------------------------------------
      1. In this we need to do two things. One send all output buffers to decoder
    ----------------------------------------------------------------------------
    */
    for(uint32 i = 0; i < mnNumOutputBuffers; i++)
    {
        pBuffer = NULL;
        if (mhAdec)
        {
            mAudioOutputQ->Pop(&pBuffer, sizeof(&pBuffer), 0);

            if(!pBuffer)
            {
                WFDMMLOGE("Failed to Pop Buf from Output Collector");
                return OMX_ErrorUndefined;
            }
            /*------------------------------------------------------------------
              Push the buffers one by one to decoders output port
            --------------------------------------------------------------------
            */
            if(OMX_FillThisBuffer(mhAdec,pBuffer) != OMX_ErrorNone)
            {
                WFDMMLOGE("Failed to push buffer for FTB");
                mAudioOutputQ->Push(&pBuffer, sizeof(&pBuffer));
                return OMX_ErrorUndefined;
            }
            mAudioOutputQ->Push(&pBuffer, sizeof(&pBuffer));
        }
    }



    /*--------------------------------------------------------------------------
     2. Send all Input buffers to client to fill.
    ----------------------------------------------------------------------------
    */

    for(uint32 i = 0; i < mnNumInputBuffers; i++)
    {
        pBuffer = NULL;
        if (mhAdec)
        {
            mAudioInputQ->Pop(&pBuffer, sizeof(&pBuffer), 0);

            if(!pBuffer)
            {
                WFDMMLOGE("Failed to Pop Buf from Input Queue");
                return OMX_ErrorUndefined;
            }

            WFDMMLOGH1("Popped InputBuffer %x", (unsigned int)pBuffer);
            /*------------------------------------------------------------------
              Push the buffers one by one to Client saying EBD
            --------------------------------------------------------------------
            */
            if (mpFnEBD)
            {
                mpFnEBD(mClientData,mnModuleId, SINK_AUDIO_TRACK_ID, pBuffer);
            }

            mAudioInputQ->Push(&pBuffer, sizeof(&pBuffer));
        }
    }

    SETSTATEOPENED;

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:          HandlePortReconfig

         DESCRIPTION:
*//**       @brief          When OMX module reports port settings changed, this
                            function handles it
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            true or false
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkAudioDecode::HandlePortReconfig()
{
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;

    OMX_INIT_STRUCT(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);

    sPortDef.nPortIndex = mnOutputPortIndex;

    OMX_ERRORTYPE eErr =
         OMX_GetParameter(mhAdec,
                          OMX_IndexParamPortDefinition,
                         &sPortDef);
    if (eErr != OMX_ErrorNone)
    {
        WFDMMLOGE("PortSetting Changed Fail in GetPortDef");
        return false;
    }


    WFDMMLOGH1("AFter Port Change Coding type %d",
                sPortDef.format.audio.eEncoding);

    return true;
}

/*==============================================================================

         FUNCTION:          GenerateAACHeaderFromADTS

         DESCRIPTION:
*//**       @brief          Generates 2 byte AAC ADIF Header by parsing ADTS
                            header
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            true or false
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
bool WFDMMSinkAudioDecode::GenerateAACHeaderFromADTS
(
    uint8* pAdts, uint32 nAdtsLen,
    uint8 *pAacHeader, uint32 *nAACHdrLen
)
{
    if(!pAdts || nAdtsLen < 7 || !pAacHeader || (*nAACHdrLen) < 2)
    {
        WFDMMLOGE("GenerateAACHeader Invalid Args");
        return false;
    }

    uint8 nAudioObjectType = ((pAdts [2] >> 6) & 0x03) + 1;
    uint8 nSampFreqIndex   = ((pAdts [2] >> 2) & 0x0F);
    uint8 nChannelConfig   = ((pAdts [2] << 2) & 0x04)
                           | ((pAdts [3] >> 6) & 0x03);


    pAacHeader [0]
            = (OMX_U8)((nAudioObjectType << 3)
               | ((nSampFreqIndex & 0x0E) >> 1));
    pAacHeader [1]
            = (OMX_U8)(((nSampFreqIndex & 0x01) << 7)
               | (nChannelConfig << 3));

    *nAACHdrLen = 2;

    return true;
}


#else
#include "WFDMMSinkAACDecode.h"

/*==============================================================================

         FUNCTION:          create

         DESCRIPTION:
*//**       @brief          Creates an instance of audio decoder of the
                            desired codec
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
WFDMMSinkAudioDecode* WFDMMSinkAudioDecode::create(WFD_audio_type eFmt)
{
    WFDMMSinkAudioDecode *pAudioDec = NULL;
#ifndef USE_AAC_TUNNEL_MODE
    if(eFmt == WFD_AUDIO_AAC)
    {
        WFDMMLOGE("Create AAC Decoder");
        pAudioDec = MM_New(WFDMMSinkAACDecode);
    }
#endif
    return pAudioDec;
}


/*==============================================================================

         FUNCTION:          ~WFDMMSinkAudioDecode

         DESCRIPTION:
*//**       @brief          destructor.
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
WFDMMSinkAudioDecode::~WFDMMSinkAudioDecode()
{
    WFDMMLOGH("Destructor WFDMMSinkAudioDecode");
}
#endif//USE_OMX_AAC_CODEC
