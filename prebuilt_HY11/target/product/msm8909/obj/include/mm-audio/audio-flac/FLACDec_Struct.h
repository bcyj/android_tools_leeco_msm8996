/******************************************************************************
* Company                       : Tata Elxsi Ltd
* File                          : TEL_FLACDec_Struct.h
* Author                        : TEL Audio Team
* Version                       : 1.0
* Date                          : 12-December-2009
* Modified Date                 : 14-October-2009
* Operating Environment         : Windows XP
* Compiler with Version         : Visual studio on PC
* Description                   : This file contains the Basic FLAC
*                               : structure and declarations of decoder
*                               : internal functions.
* Copyright                     : <2009> Tata Elxsi Ltd.
*                               : All rights reserved.
*                               : This document/code contains information that
*                               : is proprietary to Tata Elxsi Ltd. No part of
*                               : this document/code may be reproduced or used
*                               : in whole or part in any form or by any means
*                               : - graphic, electronic or mechanical without
*                               : the written permission of Tata Elxsi Ltd.
*******************************************************************************/

#ifndef _TEL_FLACDEC_STRUCT_H_
#define _TEL_FLACDEC_STRUCT_H_

#define MAXINSTSIZE     (2048+ MAXINPBUFFER + 65536*8*4)/* Instance size */
#define SCRATCHBUFOFFSET 65536
 /*65536-for sampling rate >48000 ,%%%if<= 48000 */

#define  FLAC_STRM_SYNC           0x664C6143
#define  OGG_STRM_SYNC            0x4f676753 /** OggS - sync word **/

#define QLP_COEFF_PRECISION_LEN   4
#define QLP_SHIFT_LEN             5
#define ENTROPY_TYPE_LEN          2
#define RICE_ORDER_LEN            4
#define RICE_PARAM_LEN            4
#define RICE2_PARAM_LEN           5

#define RICE_ESCAPE_PARAMETER    15
#define RICE2_ESCAPE_PARAMETER   31
#define RICE_RAW_LEN              5
#define STRING_LENGTH           100

/* maximum LPC order permitted by the format. */
#define MAX_LPC_ORDER           (32)

/* maximum Number of channels supported by  FLAC Decoder */
#define MAX_CHANNELS             (8)

/* maximum order of the fixed predictors permitted by the format. */
#define MAX_FIXED_ORDER          (4)

/*md5 functions */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f,w,x,y,z,in,s) \
     (w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

typedef enum {
    CHANNEL_ASSIGNMENT_INDEPENDENT = 0,
    CHANNEL_ASSIGNMENT_LEFT_SIDE   = 1,
    CHANNEL_ASSIGNMENT_RIGHT_SIDE  = 2,
    CHANNEL_ASSIGNMENT_MID_SIDE    = 3
}eChannelAssgn;

typedef enum {
    FRAME_NUMBER,
    SAMPLE_NUBER
}eNumType;

typedef enum {
    /* Residual is coded by partitioning into contexts, each with it's own */
    /* 4-bit Rice parameter. */
    ENTROPY_PARTITIONED_RICE = 0,

    /* Residual is coded by partitioning into contexts, each with it's own
       5-bit Rice parameter. */
    ENTROPY_PARTITIONED_RICE2 = 1

} EntropyCodingMethodType;

typedef enum
{
    NO_PARTIALFRAME = 0,
    PARTIALFRAME_PROCESS
}eFLACDecPartialFrameState;

typedef struct
{
    stFLACDecBitStrm FLACDecBitStrm;
    stMetaDataBlocks MetaDataBlocks;
    bool8  bStrmInfoPresent;

    /* Frame Header Parameters */
    uint8  ui8HeaderWarmUp[2];
    uint8  ui8BlockingStrategy;
    uint32 ui32BlockSize;
    uint32 ui32SampleRate;
    uint32 ui32NumChannels;
    eChannelAssgn ChannelAssignment;
    uint32 ui32SampleSize;

    /* md5 parameters */
    uint32 ui32md5Bytes[2];
    uint32 ui32md5Values[16];
    uint32 ui32md5Params[4];
    uint8  ui8md5Buffer[64];
    uint64 ui64HeaderEndOffset;
    /* FASTPLAY  params */
    bool8  FastPlayParseHeaderDone;
    bool8  FastPlaySyncSearchDone;
    bool8  is_Ogg;
    /* for metadata info */
    int8   ai8ArtistBuffer[STRING_LENGTH];
    int8   ai8TitleBuffer[STRING_LENGTH];
    int8   ai8AlbumBuffer[STRING_LENGTH];
    int32  i32Era;
    int8   ai8GenreBuffer[STRING_LENGTH];
    int32  i32TrackNo;
    int64  i64TotSamples;
    int32  *i32outBuf;
    void*  piOutBuffer[MAXOUTCHANNELS];
    void* piOut;

    //   uint8* pui8InstBuffer;
    //flags for stream info found and sync code found
    uint8 ui8MetaDataFound;
    uint8 ui8SyncCodeFound;

    /* partial frame contextsave varibales */
    uint32 ui32PartialFrameSize;
    uint32 ui32ChannelCount;
    uint32 ui32ChannelNum;
    uint32 ui32PartialFrameProcessedSampls[MAXOUTCHANNELS];
    uint32 ui32PartialRemSampls[MAXOUTCHANNELS];
    uint32 ui32PartialFrameConstValue[MAXOUTCHANNELS];
    uint32 ui32PartialFrameBitspSmpl[MAXOUTCHANNELS];
    uint32 ui32PartialFrameWastedBits[MAXOUTCHANNELS];
    uint32 ui32PartialFrameType[MAXOUTCHANNELS];
    uint32 ui32Order[MAXOUTCHANNELS];
    int32  i32Quantization_level[MAXOUTCHANNELS];
    /* Partial Frame Bitstream Data */
    uint32 ui32BitsLeft;
    uint32 ui32Cache;
    uint32 ui32crc;
    uint32 ui32crcalign;
    uint32 ui32partialFrameBytesUsed;
    int32  *pi32pcmBuf;
    int32  *pi32OutputBufLR[MAXOUTCHANNELS];
    int32  ai32Qlp_coeff[MAXOUTCHANNELS][MAX_LPC_ORDER];
    eFLACDecPartialFrameState  PartialFrameDecState[MAXOUTCHANNELS];
}stFLACDec;


/*************************************************************************
* Function Name     : FLACDec_DecodeFrameHeader()
* Description       : This function Parses and Decodes the frame header
* Parameters        : stFLACDec *pstFLACDec -> Pointer to the FLAC decoder
*                                              instance
* Called functions  : FLACDec_CRC8
*                   : FLACDec_CRC16
*                   : FLACDecGetBits
*                   : FLACDec_ReadUTF8
* Global Data       : none
* Return Value      : FLACDecStatus FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_DecodeFrameHeader(stFLACDec* pstFLACDec, eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_DecodeSubFrame()
* Description       : This function decodes a sub-frame
* Parameters        : stFLACDec* pstFLACDec -> FALC Decoder instance
*                   : ui32Chlcount-> Channel count
*                   : eFLACDecErrCode* peError ->Error code
* Called functions  : FLACDec_GetUnary
*                   : FLACDec_SubframeConstant
*                   : FLACDec_SubframeVerbatim
*                   : FLACDec_SubframeFixed
*                   : FLACDec_SubframeLPC
* Global Data       : none
* Return Value      : FLACDecStatus FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_DecodeSubFrame(stFLACDec* pstFLACDec,
                                     uint32 ui32Numchls,
                                     eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_PartialFrameDataChecks()
* Description       : This function is to check for process partial frame
* Parameters        : stFLACDec* pstFLACDec -> FLAC DEcoder instance
* Called functions  : none
* Global Data       : none
* Return Value      : BlockSize
* Exceptions        : none
*************************************************************************/
uint32 FLACDec_PartialFrameDataChecks(stFLACDec* pstFLACDec);


/*************************************************************************
* Function Name     : FLACDec_SubframeLPC()
* Description       : This function decodes the Sub Frame Lpc
* Parameters        : stFLACDec* pstFLACDec -> FLAC DEcoder instance
*                   : uint32 ui32Order -> lpc Order
*                   : uint32 ui32Bps -> Bits per sample
*                   : eFLACDecErrCode* peError -> Error code
* Called functions  : FLACDecGetSgndBits
*                   : FLACDecGetBits
*                   : FLACDec_DecodeResidual
*                   : FLACDec_LPCFilter
* Global Data       : none
* Return Value      : TRUE if successful
* Exceptions        : none
*************************************************************************/
bool8 FLACDec_SubframeLPC(stFLACDec* pstFLACDec,
                          uint32 ui32Order,
                          uint32 ui32Bps,
                          eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_SubframeConstant()
* Description       : This function reads sub frame constant
* Parameters        : stFLACDec* pstFLACDec -> FLAC DEcoder instance
*                   : uint32 ui32Bps -> Bits per sample
*                   : eFLACDecErrCode* peError -> Error code
* Called functions  : none
* Global Data       : none
* Return Value      : TRUE if successful
* Exceptions        : none
*************************************************************************/
bool8 FLACDec_SubframeConstant(stFLACDec* pstFLACDec,
                               uint32 ui32Bps);


/*************************************************************************
* Function Name     : FLACDec_SubframeFixed()
* Description       : This function decodes the sub frame fixed
* Parameters        : stFLACDec* pstFLACDec -> FLAC DEcoder instance
*                   : uint32 ui32Order -> lpc Order
*                   : uint32 ui32Bps -> Bits per sample
*                   : eFLACDecErrCode* peError -> Error code
* Called functions  : FLACDec_DecodeResidual
*                   : FLACDec_FixedPrediction
*                   : FLACDecGetSgndBits
* Global Data       : none
* Return Value      : TRUE if successful
* Exceptions        : none
*************************************************************************/
bool8 FLACDec_SubframeFixed(stFLACDec* pstFLACDec,
                            uint32 ui32order,
                            uint32 ui32Bps,
                            eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_SubframeVerbatim()
* Description       : This function decodes sub frame verbatim
* Parameters        : stFLACDec* pstFLACDec -> FLAC DEcoder instance
*                   : uint32 ui32Bps -> Bits per sample
*                   : eFLACDecErrCode* peError -> Error code
* Called functions  : FLACDecGetSgndBits
* Global Data       : none
* Return Value      : TRUE if successful
* Exceptions        : none
*************************************************************************/
bool8 FLACDec_SubframeVerbatim(stFLACDec* pstFLACDec,
                               uint32 ui32Bps);


/***********************************************************************
* Function Name     : FLACDec_DecodeResidual()
* Description       : This function performs the residual decoding
* Parameters        : stFLACDec* pstFLACDec -> FLAC DEcoder instance
*                   : stFLACDecBitStrm* pstFLACDecBitStrm ->
*                   :                                   Bitstream context
*                   : uint32 ui32Predorder -> Pre order
*                   : eFLACDecErrCode* peError -> Error Code
* Called functions  : FLACDecGetBits
* Global Data       : none
* Return Value      : TRUE if successful
* Exceptions        : none
***********************************************************************/
bool8 FLACDec_DecodeResidual(stFLACDec* pstFLACDec,
                             stFLACDecBitStrm* pstFLACDecBitStrm,
                             uint32 ui32Predorder,
                             eFLACDecErrCode* peError);


/***********************************************************************
* Function Name     : FLACDec_ReadResidualSamples()
* Description       : This function reads residual samples
* Parameters        : stFLACDec* pstFLACDec -> FLAC Decoder instance
*                   : int32 *pi32ResBuffer -> Residual buffer
*                   : uint32 ui32SamplesRqd -> Samples required
*                   : uint32 u32Riceparam -> Rice Parameter
* Called functions  : FLACDec_GetUnary
*                   : FLACDecGetBits
* Global Data       : none
* Return Value      : TRUE if successful
* Exceptions        : none
***********************************************************************/
bool8 FLACDec_ReadResidualSamples(stFLACDec* pstFLACDec,
                                  int32* pi32ResBuffer,
                                  uint32 ui32Samplesrqd,
                                  uint32 ui32Riceparam);


/***********************************************************************
* Function Name     : FLACDec_LPCFilter()
* Description       : This function performs the PLC filtering
* Parameters        : int32* pi32OutBuffer -> Output buffer
*                   : uint32 ui32Order -> LPC Order
*                   : int32 i32Quantization_level -> Quantization level
*                   : int32* pi32Qlp_coeff  -> Quantized linear predictor
*                   :                          coefficients
*                   : int32 i32BlockSz -> Block size
*                   : eFLACDecErrCode* peError -> Error code
* Called functions  : none
* Global Data       : none
* Return Value      : 1 or 0
* Exceptions        : none
***********************************************************************/
bool8 FLACDec_LPCFilter(int32* pi32OutBuffer,
                        uint32 ui32Order,
                        int32 i32Quantization_level,
                        int32* pi32Qlp_coeff ,
                        int32 i32BlockSz,
                        eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_FixedPrediction()
* Description       : This function performs Linear fixed prediction
* Parameters        : int32 *pi32OutBuffer -> Prediction IO buffer
*                   : uint32 ui32Order -> Prediction order
*                   : int32 i32BlockSz -> Block size
*                   : eFLACDecErrCode* peError -> Error code
* Called functions  : none
* Global Data       : none
* Return Value      : 1 or 0
* Exceptions        : none
*************************************************************************/
bool8 FLACDec_FixedPrediction(int32* pi32OutBuffer,
                              uint32 ui32Order ,
                              int32 i32BlockSz,
                              eFLACDecErrCode* peError);


/***********************************************************************
* Function Name     : FLACDec_MD5Init()
* Description       : This function initializes the MD5 Signature.
* Parameters        : uint32 *pui32Md5Params -> MD5 signature
*                   : uint32 *pui32Md5Bytes -> MD5 bytes
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
***********************************************************************/
void32 FLACDec_MD5Init(uint32 *pui32Md5Params,
                       uint32 *pui32Md5Bytes);


/*************************************************************************
* Function Name     : FLACDec_MD5Transform()
* Description       : This function calculates MD5 value
* Parameters        : uint32 *pui32Buf ->
*                   : uint32 *pui32In -> Input buffer
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_MD5Transform(uint32 *pui32Buf,
                            uint32 *pui32In);


/*************************************************************************
* Function Name     : FLACDec_MD5Update()
* Description       : This function updates MD5 Signature of each frame
* Parameters        : uint8* pui8Md5Buffer -> MD5 Buffer
*                   : uint32 *pui32Md5Bytes -> MD5 Bytes
*                   : uint32 *pui32Md5Values -> MD5 values
*                   : uint32 *pui32Md5Params -> Parameters
*                   : uint32 ui32TotBytes -> Total Bytes
* Called functions  : FLACDec_MD5Transform
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_MD5Update(uint8* pui8Md5Buffer,
                         uint32 *pui32Md5Bytes,
                         uint32 *pui32Md5Values,
                         uint32 *pui32Md5Params,
                         uint32 ui32TotBytes);


/*************************************************************************
* Function Name     : FLACDec_MD5Final()
* Description       : This function compares the Final MD5 value with
*                   : encoded value.
* Parameters        : uint32 aui32Md5Bytes[] -> MD5 bytes
*                   : uint32 aui32Md5Values[] -> MD5 values
*                   : uint32 aui32Md5Params[] -> Parameters
*                   : int8 ai8MD5Sum[16] -> MDS Sum
* Called functions  : FLACDec_MD5Transform
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_MD5Final(uint32 *pui32Md5Bytes,
                        uint32 *pui32Md5Values,
                        uint32 *pui32Md5Params,
                        int8  *pi8MD5Sum);


/*************************************************************************
* Function Name     : FLACDec_MD5accumulate()
* Description       : Generates MD5 signature from decoded bytes
* Parameters        : stFLACDec* pstFLACDec -> FLAC Decoder instance
*                   : int32 *ai32OutBuffer[MAXOUTCHANNELS] -> Output buffers
* Called functions  : FLACDec_MD5Update
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_MD5accumulate(stFLACDec* pstFLACDec,
                             void *ai32OutBuffer[MAXOUTCHANNELS]);


/*************************************************************************
* Function Name     : FLACDec_Metadata()
* Description       : Obtains the meta data information from the bit stream
* Parameters        : stFLACDec *pstFLACDec -> FLAC Decoder instance
*                     stFLACDecBitStrm *pstFLACDecBitStrm -> Bit stream
*                                                           info structure
* Called functions  : FLACDecGetBits
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_Metadata(stFLACDec* pstFLACDec,
                               stFLACDecBitStrm* pstFLACDecBitStrm);

#endif /*_TEL_FLACDEC_STRUCT_H_ */

/******************************************************************************
 * End of file
 *****************************************************************************/

