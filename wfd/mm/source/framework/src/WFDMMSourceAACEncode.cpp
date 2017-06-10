/*==============================================================================
*       WFDMMSourceAACEncode.cpp
*
*  DESCRIPTION:
*       AAC encoder wrapper class
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
04/02/2014         SK            InitialDraft
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
#include "WFDMMSourceAACEncode.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "aacenc_lib.h"

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#define UNUSED(x) ((void)x)

//Structure to hold the data which are used as arguments to
//AACEncode call.
struct aacSampleInfo
{
    aacSampleInfo() {
        memset(&sInBuff,  0, sizeof(sInBuff));
        memset(&sOutBuff, 0, sizeof(sOutBuff));
        memset(&sInArg,   0, sizeof(sInArg));
        memset(&sOutArg,  0, sizeof(sOutArg));
    }
    AACENC_BufDesc sInBuff;
    AACENC_BufDesc sOutBuff;
    AACENC_InArgs  sInArg;
    AACENC_OutArgs sOutArg;
    int nInElemSize[1];
    int nOutBufSize[1];
    int nInBufSize[1];
    int nOutElemSize[1];
    int nInSamples;
};

/*==============================================================================

         FUNCTION:         WFDMMSourceAACEncode

         DESCRIPTION:
*//**       @brief         WFDMMSourceAACEncode contructor
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
WFDMMSourceAACEncode::WFDMMSourceAACEncode()
{

    WFDMMLOGH("WFDMMSourceAACEncode:: Constructor");

    InitData();

}

/*==============================================================================

         FUNCTION:          ~WFDMMSourceAACEncode

         DESCRIPTION:
*//**       @brief          AAC decoder destructor
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
WFDMMSourceAACEncode::~WFDMMSourceAACEncode()
{
    /*--------------------------------------------------------------------------
     Call destroy resources to make sure destructor is not called in the
     middle of play
    ----------------------------------------------------------------------------
    */
    destroyResources();

    WFDMMLOGH("WFDMMSourceAACEncode Destructor");

}

/*==============================================================================

         FUNCTION:          InitData

         DESCRIPTION:
*//**       @brief          Initializes members of constructor
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
void WFDMMSourceAACEncode::InitData()
{
    memset(&mCfg, 0, sizeof(mCfg));

    m_hAACEncoder = 0;

    m_pAACInfo = 0;

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
OMX_ERRORTYPE WFDMMSourceAACEncode::Configure(audioConfigType *pCfg)
{
    if (!pCfg || pCfg->eAudioType != OMX_AUDIO_CodingAAC)
    {
        WFDMMLOGE1("AudioEncode Configure: Invalid Args 0x%x",
                   (long)pCfg);
        return OMX_ErrorBadParameter;
    }

    if(pCfg)
    {
        memcpy(&mCfg, pCfg, sizeof(audioConfigType));
    }


    if(createResources() != OMX_ErrorNone)
    {
        WFDMMLOGE("AudioEnc Failed to createResources");
        return OMX_ErrorInsufficientResources;
    }
    WFDMMLOGH("AudioEncode Resources Created");
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         createResources

         DESCRIPTION:
*//**       @brief         creates resources for WFDMMSourceAACEncode
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
OMX_ERRORTYPE WFDMMSourceAACEncode::createResources()
{
    if (m_hAACEncoder)
    {
        WFDMMLOGE("Encoder already created. return fail");
        return OMX_ErrorIncorrectStateTransition;
    }


    return ConfigureAACEncoder();
}


/*==============================================================================

         FUNCTION:         ConfigureAACEncoder

         DESCRIPTION:
*//**       @brief         This is the WFDMMSource Function used for
                           configuring the AAC Encoder
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

*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceAACEncode::ConfigureAACEncoder()
{
    AACENC_ERROR eError = AACENC_OK;

    m_pAACInfo = (void*)MM_New(aacSampleInfo);

    if(m_pAACInfo == NULL)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                 "AudioSource::AAC Info alloc failed");
        return OMX_ErrorInsufficientResources;
    }

    eError = aacEncOpen((HANDLE_AACENCODER*)(&m_hAACEncoder), 0x01 /*AAC*/,
                         mCfg.nChannels);

    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "AudioSource::AAC encoder open failed");
        return OMX_ErrorUndefined;
    }
    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                         AACENC_AOT, AOT_MP2_AAC_LC);

    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "AudioSource::AAC encoder setting AOT failed");
        return OMX_ErrorBadParameter;
    }

    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                                  AACENC_BITRATE,
                                  mCfg.nBitrate);
    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "AudioSource::AAC encoder open failed");
        return OMX_ErrorBadParameter;
    }

    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                                  AACENC_SAMPLERATE,
                                  mCfg.nSampleRate);
    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "AudioSource::AAC encoder set param failed sample rate");
        return OMX_ErrorBadParameter;
    }

    #define GET_CHANNEL_MODE(ch) (ch == 1 ? MODE_1 :     \
                                  ch == 2 ? MODE_2 :     \
                                  ch == 3 ? MODE_1_2 :   \
                                  ch == 4 ? MODE_1_2_1:  \
                                  ch == 5 ? MODE_1_2_2:  \
                                  ch == 6 ? MODE_1_2_2_1:\
                                  ch == 8 ? MODE_7_1_REAR_SURROUND:\
                                            MODE_2)

    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                                  AACENC_CHANNELMODE,
                                  GET_CHANNEL_MODE(mCfg.nChannels));

    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                 "AudioSource::AAC set param failed Channel Mode");
    }


    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                                  AACENC_CHANNELORDER,
                                  1/*wave order*/);
    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                 "AudioSource::AAC set param failed Channel Order");
    }

    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                                  AACENC_TRANSMUX,
                                  2/*ADTS*/);
    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                 "AudioSource::AAC set param failed Granule transmux");
        return OMX_ErrorBadParameter;
    }

    eError = aacEncoder_SetParam((HANDLE_AACENCODER)m_hAACEncoder,
                                  AACENC_GRANULE_LENGTH,
                                  AAC_FRAME_SIZE);
    if(eError != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                 "AudioSource::AAC set param failed Granule");
    }


    /*--------------------------------------------------------------------------
     Preconfigure the Encode call structures
    ----------------------------------------------------------------------------
    */

    aacSampleInfo *pAACInfo = (struct aacSampleInfo*)m_pAACInfo;

    static AACENC_BufferIdentifier eInBuf  = IN_AUDIO_DATA;
    static AACENC_BufferIdentifier eOutBuf = OUT_BITSTREAM_DATA;

    pAACInfo->nInElemSize[0]            = sizeof(OMX_U16);
    pAACInfo->nOutBufSize[0]            = AAC_FRAME_SIZE
                                          * 2 * mCfg.nChannels;
    pAACInfo->nInBufSize[0]             = AAC_FRAME_SIZE
                                          * 2 * mCfg.nChannels;

    pAACInfo->nOutElemSize[0]           = sizeof(OMX_U8);
    pAACInfo->nInSamples                = AAC_FRAME_SIZE
                                          * mCfg.nChannels;

    //Initialize Input Buffer Params
    pAACInfo->sInBuff.numBufs           = 1;
    pAACInfo->sInBuff.bufs              = NULL;
    pAACInfo->sInBuff.bufferIdentifiers = (int*)&eInBuf;
    pAACInfo->sInBuff.bufSizes          = pAACInfo->nInBufSize;
    pAACInfo->sInBuff.bufElSizes        = pAACInfo->nInElemSize;

    //Initialize Output Buffer Params
    pAACInfo->sOutBuff.numBufs           = 1;
    pAACInfo->sOutBuff.bufs              = NULL;
    pAACInfo->sOutBuff.bufferIdentifiers = (int*)&eOutBuf;
    pAACInfo->sOutBuff.bufSizes          = pAACInfo->nOutBufSize;
    pAACInfo->sOutBuff.bufElSizes        = pAACInfo->nOutElemSize;

    //Initialize InArgs with numsamples
    pAACInfo->sInArg.numInSamples        = pAACInfo->nInSamples;
    pAACInfo->sInArg.numAncBytes         = 0;

    return OMX_ErrorNone;
}



/*==============================================================================

         FUNCTION:          Encode

         DESCRIPTION:
*//**       @brief          Encodes a frame
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
OMX_ERRORTYPE WFDMMSourceAACEncode::Encode(uint8* pInBuffer,
                                           uint32 nInBufSize,
                                           uint8* pOutBuffer,
                                           uint32 nOutBufSize,
                                           uint32 *pEncodedLen)
{
    UNUSED(nInBufSize);
    UNUSED(nOutBufSize);
    aacSampleInfo *pAACInfo = (aacSampleInfo*)m_pAACInfo;

    if(!pAACInfo || !pInBuffer || !pOutBuffer)
    {
        WFDMMLOGE("AAC encode invalid args");
        return OMX_ErrorBadParameter;
    }

    //Initialize Input Buffer
    pAACInfo->sInBuff.bufs              = (void**)(&pInBuffer);

    //Initialize Output Buffer
    pAACInfo->sOutBuff.bufs              = (void**)(&pOutBuffer);

    int nErr = aacEncEncode((HANDLE_AACENCODER)m_hAACEncoder,
                               &pAACInfo->sInBuff,
                               &pAACInfo->sOutBuff,
                               &pAACInfo->sInArg,
                               &pAACInfo->sOutArg);

    if(nErr != AACENC_OK)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "Audio_Handler:AAC encode failed");
        return OMX_ErrorDynamicResourcesUnavailable;
    }

    if(pEncodedLen)
    {
         *pEncodedLen= ((struct aacSampleInfo*)m_pAACInfo)->sOutArg.numOutBytes;
    }

    return OMX_ErrorNone;
}



/*==============================================================================

         FUNCTION:         destroyResources

         DESCRIPTION:
*//**       @brief         destroys resources for WFDMMSinkRenderer
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
OMX_ERRORTYPE WFDMMSourceAACEncode::destroyResources()
{
    if (m_hAACEncoder)
    {
        if(aacEncClose((HANDLE_AACENCODER*)(&m_hAACEncoder)) != AACENC_OK)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "AudioSource :: Failed to close AAC ENC");
        }
        m_hAACEncoder = NULL;
    }

    if(m_pAACInfo)
    {
        MM_Delete((aacSampleInfo*)m_pAACInfo);
        m_pAACInfo = NULL;
    }

    return OMX_ErrorNone;
}




