/*************************************************************************
Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/
#ifndef __FLACDEC_WRAPPER_H__
#define __FLACDEC_WRAPPER_H__

// Definitions
#define DEC_SUCCESS              0x0000
#define DEC_NOMEMORY_FAILURE     0xFA02
#define DEC_FAILURE              0x0001
#define DEC_BADPARAM_FAILURE     0xFA03
#define DEC_NEED_MORE            0xFA04

#include "FLACDec_typedefs.h"
#include "FLACDec_API.h"

typedef struct {
    uint32 ui32MaxLen;
    uint32 ui32WriteOffset; //From where to start writing parser data into
    uint8 internalBuffer[MAXINPBUFFER*2]; // has to be two - such that there is sufficient data always
}stInternalBuffer;

typedef struct {
    uint64*  m_pFlacDecoder;
    void*    pFlacDecMetaDataStrmInfo;
    uint32  ui32MaxBlockSize;
    uint32 ui32NumChannels;
    uint32 m_bits_per_sample;
    int8  m_bEndOfFrame;
    int32   m_bIsStreamInfoPresent;
    int8*  m_p8MD5Sum;
    stInternalBuffer fIBuffer;
    uint32  bytesInInternalBuffer;
}CFlacDecState;

typedef struct {
    uint64*  m_pFlacDecoder;
    void*    pFlacDecMetaDataStrmInfo;
    uint32  ui32MaxBlockSize;
    uint32 m_bits_per_sample;
    int8  m_bEndOfFrame;
    int32   m_bIsStreamInfoPresent;
    int8*  m_p8MD5Sum;
    //stInternalBuffer fIBuffer;
}CFlacDecStateWithoutfIBuffer;

// External Function defintions
FLACDecStatus CFlacDecoderLib_Process ( void* pFlacDecState, uint8* pInBitStream,
                                        uint32 nActualDataLen, void* pOutSamples,
                                        uint32* uFlacOutputBufSize, uint32* usedBitstream,
                                        uint32* ui32BlockSize );

void CFlacDecoderLib_End( void* pFlacDec ) ; //Destroys mem of the decoder
//FLACDecStatus CFlacDecoderLib_Init(void* dec); //Initializes the decoder
                                                            // For flush as well call init again
int CFlacDecoderLib_SetParam (void* pFlacDecState, int nParamIdx, int nParamVal );

// Internal Function definitions
void CFlacDecoderLib_Meminit(CFlacDecState* pFlacDecState, int* nRes, int bitWidth ) ; //Alloates mem for the decoder

void FlacDec_GetDataInit(stInternalBuffer* fIBuffer);
#endif /* __FLACDEC_WRAPPER_H__ */
