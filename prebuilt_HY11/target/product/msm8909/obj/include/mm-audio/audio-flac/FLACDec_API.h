/*************************************************************************
* Company                       : Tata Elxsi Ltd
* File                          : TEL_FLACDec_API.h
* Author                        : TEL Audio Team
* Version                       : 1.0
* Date                          : 12-December-2009
* Modified Date                 : 15-October-2009
* Operating Environment         : Windows XP
* Compiler with Version         : Visual studio on PC
* Description                   : This file contains decoder interface
*                                 routine exposed to Application
* Copyright                     : <2009> Tata Elxsi Ltd.
*                               : All rights reserved.
*                               : This document/code contains information
*                                 that is proprietary to Tata Elxsi Ltd.
*                                 No part of this document/code may be
*                                 reproduced or used in whole or part in
*                                 any form or by any means - graphic,
*                                 electronic or mechanical without the
*                                 written permission of Tata Elxsi Ltd.
*************************************************************************/
#ifndef _TEL_FLACDEC_API_H_
#define _TEL_FLACDEC_API_H_


#include "FLACDec_typedefs.h"

//typedef struct {
//	uint32 ui32MaxLen;
//	uint32 ui32WriteOffset; //From where to start writing parser data into
//	uint8 internalBuffer[MAXINPBUFFER*2]; // has to be two - such that there is sufficient data always
//}stInternalBuffer;


/*VNOTE: From 100 to avoid clashes with the ICAPI */
enum FlacParamIdx
{
	eIcapiNumChannels,
	eIcapiSamplingRate,
	eIcapiBitsPerSample,

      eFlacBlockSize = 100,
	  eFlacIsStreamInfoPresent,
      eFlacSampleSize,
      eFlacStrmInfoMinBlkSize,
      eFlacStrmInfoMaxBlkSize,
      eFlacStrmInfoMinFrmSize,
      eFlacStrmInfoMaxFrmSize,
      eFlacStrmInfoMd5Sum,
	  eFlacEndOfFrame
};


typedef enum
{
    FLACDecSearchMetaData,
    FLACDecParseMetaData,
    FLACDecSearchFrameSync,
    FLACDecDecodeFrame,
    FLACDecEndofStream
}eFLACDecState;

typedef enum
{
    FLACDecNoError,
    FLACDec_EOS,
    FLACDecError
}FLACDecReadStatus;

typedef enum eFLACDec_Result {
    FLACDEC_SUCCESS=0,                  /* Success */
    FLACDEC_FAIL,                       /* Failure */
    FLACDEC_EOF,                         /* End of Stream */
    FLACDEC_METADATA_NOT_FOUND,
    FLACDEC_ERROR_CODE_NEEDS_MORE_DATA
}FLACDecStatus;

typedef enum eFLACDec_Error {
        FLACDEC_NO_ERROR=0,           /* Success */
        FLACDEC_ALLOC_ERROR,          /* Memory allocation is failed */
        FLACDEC_INSUFFICIENT_BUFFER,  /* Buffer is not sufficient */
        FLACDEC_INVALID_SAMPLE_RATE,  /* Invalid sampling rate */
        FLACDEC_INVALID_BLOCK_SIZE,   /* invalid block size*/
        FLACDEC_INVALID_SAMPLE_SIZE,  /* Invalid sampling size */
        FLACDEC_INVALID_CHANNEL_COUNT,/* Invalid channel count */
        FLACDEC_INSUFFICIENT_DATA,    /* Insufficient data */
        FLACDEC_PARTIAL_FRAME,        /* Partial Frame */
        FLACDEC_FEATURE_NOT_SUPPORTED,/* Feature not supported */
        FLACDEC_CORRUPT_DATA,         /* Data is corrupted */
        FLACDEC_INVALID_PROPERTY,     /* Invalid Property of the decoder*/
        FLACDEC_ILLEGAL_CALLSEQ,      /* Incorrect Sequence of API calls
                                          by the application */
        FLACDEC_INVALID_HANDLE,
        FLACDEC_INVALID_FRAMEHEADER,
        FLACDEC_LOST_SYNC,
        FLACDEC_UNPARSEABLE_STREAM,
        FLACDEC_BITSTREAM_INIT_ERROR,
        FLACDEC_INVALIDARG,
        FLACDEC_ENDOFSTRM,
        FLACDEC_CRCFAILURE,
        FLACDEC_INVALID_STREAM,
        FLACDEC_INVALID_METADATA,    /* metadata parsed is invalid*/
        FLACDEC_INVALID_QLP_COEFF_PRECISION_LEN, /*invalid LPC precision*/
        FLACDEC_INVALID_ENTROPY_TYPE  /*invalid rice coding type*/
}eFLACDecErrCode;

#ifdef ENABLE_FREADS
typedef FLACDecReadStatus (*PFNGetData) (uint8* pui8InputBuffer,
                                         uint32 ui32bytestofill,
                                         uint32* pui32bytesread,
                                         uint32 ui32UserData);
#endif


typedef struct
{
    int32 i32blklength;
    int32 i32MinBlkSize;
    int32 i32MaxBlkSize;
    int32 i32MinFrmSize;
    int32 i32MaxFrmSize;
    int32 i32SampleRate;
    int32 i32NumChannels;
    int32 i32BitsPerSample;
    int64 i64TotSamples;
    int8  i8MD5Sum[16];
}FLACDec_ParserInfo; //VNOTE: same as --> stMetaDataStrmInfo


typedef struct
{
    int32  InputBuffSize;
    uint8* pui8InpBuffer;
#ifdef ENABLE_FREADS
    PFNGetData FGetData;
    uint32 ui32UserData;
#endif
}FLACDec_API;

typedef struct
{
    int8   ai8ArtistBuffer[STRING_LENGTH];
    int8   ai8TitleBuffer[STRING_LENGTH];
    int8   ai8AlbumBuffer[STRING_LENGTH];
    int32  i32Era;
    int8   ai8GenreBuffer[STRING_LENGTH];
    int32  i32TrackNo;
}FLACDec_MetaData;

typedef struct
{
    uint32 ui32BlockSize;
    uint32 ui32SampleRate;
    uint32 ui32NumChannels;
    uint32 ui32SampleSize;
}FLACDec_Info;



/*************************************************************************
* Function Name     : FLACDec_GetInstanceSize()
* Description       : This function gets the instance size of FLAC decoder
* Parameters        : uint32 *ui32InstSize -> Pointer for updating instance
*                                             size
*                     eFLACDecErrCode *peError -> Error code
* Called functions  : none
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus  FLACDec_GetInstanceSize(uint32 *ui32InstSize,
                                       eFLACDecErrCode* peError);
/*************************************************************************
* Function Name     : FLACDec_GetSampleSize()
* Description       : This function gets sample size of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_GetSampleSize(void * pFLACDecHandle);
/*************************************************************************
* Function Name     : FLACDec_GetNumChannels()
* Description       : This function gets no of channels of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_GetNumChannels(void * pFLACDecHandle);

/*************************************************************************
* Function Name     : FLACDec_GetSampleRate()
* Description       : This function gets sample rate of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_GetSampleRate(void * pFLACDecHandle);

/*************************************************************************
* Function Name     : FLACDec_GetBlockSize()
* Description       : This function gets block size of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_GetBlockSize(void * pFLACDecHandle);

/*************************************************************************
* Function Name     : FLACDec_GetInfo()
* Description       : This function gets block size of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*                    : FLACDec_Info* pstFLACInfo -> Config parameters from
*                                                 the application
* Called functions  : none
* Global Data       : none
* Return Value      : void
* Exceptions        : none
*************************************************************************/
void FLACDec_GetInfo(void * pFLACDecHandle,
                      FLACDec_Info* pstFLACInfo);
/*************************************************************************
* Function Name     : FLACDec_GetMetaData()
* Description       : This function gets metadata of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*                    : FFLACDec_MetaData* pstFLACMetaData -> Config parameters from
*                                                 the application
* Called functions  : none
* Global Data       : none
* Return Value      : void
* Exceptions        : none
*************************************************************************/
void FLACDec_GetMetaData(void * pFLACDecHandle,
                      FLACDec_MetaData* pstFLACMetaData);

/*************************************************************************
* Function Name     : FLACDec_SetConfig()
* Description       : This function gets no of channels of decoding stream
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*                    : FLACDec_API *pstFLACAPI -> Config parameters from
*                                                 the application
*
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void FLACDec_SetConfig(void ** pFLACDecHandle,
                             FLACDec_API* pstFLACAPI);


/*************************************************************************
* Function Name     : FLACDec_GetOutputBuffSize()
* Description       : This function gets the output buffer size of FLAC decoder
* Parameters        : uint32 *ui32InstSize -> Pointer for updating instance
*                                             size
*                    : FLACDec_API *pstFLACAPI -> Config parameters from
*                                                 the application
*                     eFLACDecErrCode *peError -> Error code
* Called functions  : none
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus  FLACDec_GetOutputBuffSize(uint32 *ui32InstSize,
                                         void * pFLACDecHandle,
                                         eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_Create()
* Description       : This function creates the Decoder instance using
*                     the instance buffer provided by the application.
* Parameters        : void * *pFLACDecHandle -> Pointer to FLAC
*                                                       Decoder handle
*
*                     eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDec_BSInit
*                   : FLACDec_MD5Init
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_Create(void * pFLACDecHandle,
                             eFLACDecErrCode* peError);

/*************************************************************************
* Function Name     : FLACDec_Init()
* Description       : This function extracts stream informations and make
*                     initializations based on any stream informations.
* Parameters        : void * FLACDecHandle -> FLAC Decoder handle
*                    eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDec_SearchMetaData_Ogg
*                    : FLACDec_ParseMetaData_Ogg
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_Init(void * FLACDecHandle,
                           eFLACDecErrCode* peError);


/***********************************************************************
* Function Name     : FLACDec_SearchFrameSync()
* Description       : This function searches for the frame sync and
*                     returns the status.
* Parameters        : void * FLACDecHandle -> FLAC Decoder handle
*                     eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDecGetBits
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
***********************************************************************/
FLACDecStatus FLACDec_SearchFrameSync(void * FLACDecHandle,
                                      eFLACDecErrCode* peError);


/***********************************************************************
* Function Name     : FLACDec_DecodeFrame()
* Description       : This function decoder a compressed FLAC frame
* Parameters        : void * FLACDecHandle -> FLAC Decoder handle
*                   : uint32* ui32BlockSize
*                   : eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDec_DecodeFrameHeader
*                   : FLACDec_DecodeSubFrame
*                   : FLACDecGetBits
*                   : FLACDec_ByteAlign
*                   : FLACDec_MD5accumulate
* Global Data       :none
* Return Value      :FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        :none
***********************************************************************/
FLACDecStatus FLACDec_DecodeFrame(void * FLACDecHandle,
                                  uint8* inpBuffer,
                                  uint32 inBufSize,
                                  uint32 *inBufSizeUsed,
                                  void* piOut,
                                  uint32 *outBufSize,
                                  uint32* ui32BlockSize,
                                  eFLACDecErrCode* peError,
                                  int32 bIsStreamInfoPresent);


/*************************************************************************
* Function Name     : FLACDec_Close()
* Description       : This function closes the FLAC Decoder instance
* Parameters        : void * *pFLACDecHandle -> FLAC Decoder handle
* Called functions  : FLACDec_MD5Final
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_Close(void ** pFLACDecHandle);

/*************************************************************************
* Function Name     : FLACDec_GetMetaDataStreamInfoSize()
* Description       : This function gets the size of FLAC decoder streamInfo
*                     metadata
* Parameters        : uint32 *ui32Size -> Pointer for updating size
*                     eFLACDecErrCode *peError -> Error code
* Called functions  : none
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus  FLACDec_GetMetaDataStreamInfoSize(uint32 *ui32Size,
                                       eFLACDecErrCode* peError);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoNumChannels()
* Description       : This function sets no of channels of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoNumChannels(void* pFLACDecStrmInfo, int32 i32NumChannels);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoMinBlkSize()
* Description       : This function sets minBlockSize of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoMinBlkSize(void* pFLACDecStrmInfo, int32 i32MinBlkSize);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoMaxBlkSize()
* Description       : This function sets maxBlockSize of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoMaxBlkSize(void* pFLACDecStrmInfo, int32 i32MaxBlkSize);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoSampleRate()
* Description       : This function sets Sample Rate of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoSampleRate(void* pFLACDecStrmInfo, int32 i32SampleRate);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoMinFrmSize()
* Description       : This function sets min frame size of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoMinFrmSize(void* pFLACDecStrmInfo, int32 i32MinFrmSize);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoMaxFrmSize()
* Description       : This function sets max frame size of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoMaxFrmSize(void* pFLACDecStrmInfo, int32 i32MaxFrmSize);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoSampleSize()
* Description       : This function sets sample size of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoSampleSize(void* pFLACDecStrmInfo, int32 i32BitsPerSample);

/*************************************************************************
* Function Name     : FLACDec_SetMetaDataStreamInfoMd5Sum()
* Description       : This function sets Md5 Sum of stream info in
*                     metadata
* Parameters        : pFLACDecStrmInfo *pFLACDecStrmInfo -> Pointer to FLAC
*                                                       Decoder stream info
*
* Called functions  : none
* Global Data       : none
* Return Value      : int32
* Exceptions        : none
*************************************************************************/
int32 FLACDec_SetMetaDataStreamInfoMd5Sum(void* pFLACDecStrmInfo, int8* pi8MD5Sum);

/*************************************************************************
* Function Name     : FLACDec_Init()
* Description       : This function extracts stream informations and make
*                     initializations based on any stream informations.
* Parameters        : FLACDecContext FLACDecHandle -> FLAC Decoder handle
*                    eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDec_SearchMetaData_Ogg
*                   : FLACDec_ParseMetaData_Ogg
*                   : FLACDec_BSInit
*                   : FLACDec_MD5Init
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/

FLACDecStatus FLACDec_InitBitStream(void* pstFLACDec,
#ifndef ENABLE_FREADS
                           uint8* inBuffer,
                           uint32 inBufSize,
#endif
                           eFLACDecErrCode* peError);

int get_bitstream_bytes_used(FLACDecContext FLACDecHandle);


#ifdef __cplusplus
#endif

#endif /*_TEL_FLACDEC_API_H_ */

/**************************************************************************
 * End of file
 *************************************************************************/

