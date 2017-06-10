#if !defined (USE_OMX_AAC_CODEC) && !defined (USE_AUDIO_TUNNEL_MODE)

/*==============================================================================
*       WFDMMSinkAACDecode.cpp
*
*  DESCRIPTION:
*       Abstracts OMX calls to video decoder and provides more sensible APIs
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
#include "WFDMMSinkAACDecode.h"
#include "MMMalloc.h"
#include "MMMemory.h"
#include "aacdecoder_lib.h"

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


/*==============================================================================

         FUNCTION:         WFDMMSinkAACDecode

         DESCRIPTION:
*//**       @brief         WFDMMSinkAACDecode contructor
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
WFDMMSinkAACDecode::WFDMMSinkAACDecode()
{

    WFDMMLOGH("WFDMMSinkAACDecode:: Constructor");

    InitData();
    /*--------------------------------------------------------------------------

    ----------------------------------------------------------------------------
    */

    mhCritSect = NULL;
    int nRet = MM_CriticalSection_Create(&mhCritSect);
    if(nRet != 0)
    {
        mhCritSect = NULL;
        WFDMMLOGE("Error in Critical Section Create");
    }

}

/*==============================================================================

         FUNCTION:          ~WFDMMSinkAACDecode

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
WFDMMSinkAACDecode::~WFDMMSinkAACDecode()
{
    /*--------------------------------------------------------------------------
     Call destroy resources to make sure destructor is not called in the
     middle of play
    ----------------------------------------------------------------------------
    */
    destroyResources();

    WFDMMLOGH("WFDMMSinkAACDecode Destructor");

    if (mhCritSect)
    {
        MM_CriticalSection_Release(mhCritSect);
    }
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
void WFDMMSinkAACDecode::InitData()
{
    memset(&mCfg, 0, sizeof(mCfg));

    mhAACDec = 0;

    mpAACStreamInfo = NULL;

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
OMX_ERRORTYPE WFDMMSinkAACDecode::Configure(audioConfigType *pCfg)
{
    if (!pCfg || pCfg->eAudioType != WFD_AUDIO_AAC)
    {
        WFDMMLOGE1("AudioDecode Configure: Invalid Args 0x%x",
                   pCfg);
    }

    if(pCfg)
        memcpy(&mCfg, pCfg, sizeof(audioConfigType));


    if(createResources() != OMX_ErrorNone)
    {
        WFDMMLOGE("AudioDec Failed to createResources");
        return OMX_ErrorInsufficientResources;
    }
    WFDMMLOGH("AudioDecode Resources Created");
    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         createResources

         DESCRIPTION:
*//**       @brief         creates resources for WFDMMSinkAACDecode
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
OMX_ERRORTYPE WFDMMSinkAACDecode::createResources()
{
    if (mhAACDec)
    {
        WFDMMLOGE("Decoder already created. return fail");
        return OMX_ErrorIncorrectStateTransition;
    }

    mbAudioCodecHdrSet = false;

    mhAACDec =  aacDecoder_Open(TT_MP4_ADTS, 1);

    if (!mhAACDec)
    {
        WFDMMLOGE("Failed to create AAC Decoder");
        return OMX_ErrorInsufficientResources;
    }

    mpAACStreamInfo = (void*)aacDecoder_GetStreamInfo
                                                ((HANDLE_AACDECODER)mhAACDec);

    if (!mpAACStreamInfo)
    {
        WFDMMLOGE("Cant get AAC StreamInfo. Something Wrong");
    }

    aacDecoder_SetParam((HANDLE_AACDECODER)mhAACDec,
                        AAC_DRC_REFERENCE_LEVEL,
                        64);

    aacDecoder_SetParam((HANDLE_AACDECODER)mhAACDec,
                        AAC_DRC_ATTENUATION_FACTOR,
                        127);

    WFDMMLOGH("AudioDecoder Initialized");

    return OMX_ErrorNone;
}


/*==============================================================================

         FUNCTION:          Decode

         DESCRIPTION:
*//**       @brief          Decodes an AAC frame
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
OMX_ERRORTYPE WFDMMSinkAACDecode::Decode(OMX_BUFFERHEADERTYPE *pBuffer)
{
    AAC_DECODER_ERROR nRet = AAC_DEC_UNKNOWN;
    bool bDecFillStatus = true;
    const uint nFlag = 0;

    if (!pBuffer || !pBuffer->pBuffer)
    {
        /*----------------------------------------------------------------------
          No valid pointers to work with return fail
        ------------------------------------------------------------------------
        */
        WFDMMLOGE("WFDMMSink AudioDecode-- Decode Fail -- Invalid Args");
        return OMX_ErrorBadParameter;
    }

    if (!mhAACDec)
    {
        WFDMMLOGE("Audio Decoder not available");
        return OMX_ErrorInvalidState;
    }

    /*--------------------------------------------------------------------------
     ADTS has 7 bytes header so we need at least 7 bytes to decode
    ----------------------------------------------------------------------------
    */
    if (pBuffer->nFilledLen <= 7)
    {
        WFDMMLOGE("Not enough Data to decode");
        return OMX_ErrorInsufficientResources;
    }

    UCHAR* pInBuffer[] = {pBuffer->pBuffer};
    UINT nSizes[]      = {(UINT)pBuffer->nFilledLen};
    UINT nOccupancy[]  = {(UINT)pBuffer->nFilledLen};


    /*--------------------------------------------------------------------------
      In WFD we receive one ADTS packet per buffer. If the data is more than
      that decoder can decode, then we may not be able to decode the frame.
      With this assumption, we can always provide
    ----------------------------------------------------------------------------
    */

    nRet =  aacDecoder_Fill((HANDLE_AACDECODER)mhAACDec,
                            pInBuffer, nSizes, nOccupancy);

    if (nRet != AAC_DEC_OK || nOccupancy[0] != 0)
    {
        WFDMMLOGE2("Problem in Filling AAC bytes Err = %d - occupancy = %d",
                   nRet, nOccupancy[0]);

        pBuffer->nFilledLen = 0;
        if(recoverAACDecoder() != OMX_ErrorNone)
        {
            WFDMMLOGE("Problem in recovering AAC Decoder at aacDecoder_Fill");
        }
        return OMX_ErrorInsufficientResources;
    }

    nRet =  aacDecoder_DecodeFrame((HANDLE_AACDECODER)mhAACDec,
                                  (INT_PCM*) pBuffer->pBuffer,
                                   pBuffer->nAllocLen,
                                   nFlag
                                  );
    if (nRet != AAC_DEC_OK)
    {
        WFDMMLOGE1("Problem in Decoding AAC bytes Err = %d",
                   nRet);
        pBuffer->nFilledLen = 0;
        if(recoverAACDecoder() != OMX_ErrorNone)
        {
            WFDMMLOGE("Problem in recovering AAC Decoder at DecodeFrame");
        }
        return OMX_ErrorInsufficientResources;
    }

    pBuffer->nFilledLen = ((CStreamInfo*)mpAACStreamInfo)->aacSamplesPerFrame
                           * 2 * mCfg.nChannels;
    WFDMMLOGM1("AudioDecoder DecodeFrame Success Size = %lu",
               pBuffer->nFilledLen);

    return OMX_ErrorNone;
}

/*==============================================================================

         FUNCTION:         recoverAACDecoder

         DESCRIPTION:
*//**       @brief         releases resources and creates again
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
OMX_ERRORTYPE WFDMMSinkAACDecode::recoverAACDecoder()
{
    WFDMMLOGE("WFDMMSinkAACDecode : recovering AAC decoder");
    destroyResources();
    WFDMMLOGE("WFDMMSinkAACDecode : creating AAC decoder");
    return createResources();
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
OMX_ERRORTYPE WFDMMSinkAACDecode::destroyResources()
{
    if (mhAACDec)
    {
        aacDecoder_Close((HANDLE_AACDECODER)mhAACDec);
        mhAACDec = 0;
    }
    return OMX_ErrorNone;
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
OMX_ERRORTYPE WFDMMSinkAACDecode::deinitialize()
{
    return OMX_ErrorNone;
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
bool WFDMMSinkAACDecode::GenerateAACHeaderFromADTS
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

#endif //!defined (USE_OMX_AAC_CODEC) && !defined (USE_AUDIO_TUNNEL_MODE)

