/*************************************************************************
* Company                       : Tata Elxsi Ltd
* File                          : TEL_FLACDec_MetaData.h
* Author                        : TEL Audio Team
* Version                       : 1.0
* Date                          : 12-December-2009
* Modified Date                 : 15-October-2009
* Operating Environment         : Windows XP
* Compiler with Version         : Visual studio on PC
* Description                   : This file contains the declarations of
*                               : various functions required for parsing
*                               : metadata block data
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
#ifndef _TEL_FLACDEC_METADATA_H_
#define _TEL_FLACDEC_METADATA_H_

#define  STREAM_METADATA_IS_LAST_LEN         1
#define  STREAM_METADATA_TYPE_LEN            7
#define  STREAM_METADATA_LENGTH_LEN         24
#define  STREAM_METADATA_SEEKPOINT_LENGTH   18
#define  STREAM_METADATA_STREAMINFO_LENGTH  34


typedef enum
{
    MetaData_StreamInfo = 0,
    MetaData_Padding,
    MetaData_Application,
    MetaData_SeekTable,
    MetaData_VorbisComment,
    MetaData_CueSheet,
    MetaData_Picture,
    MetaData_Undefined
}eMetaDataType;

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
}stMetaDataStrmInfo;

typedef struct
{
    uint64 ui64SampleNum;
    uint64 ui64StrmOffset;
    uint32 ui32NumSamples;
}stSeekTablePoints;

typedef struct
{
   stSeekTablePoints SeekTablePoints[5];
}stMetaDataSeekTable;

typedef struct
{
    stMetaDataStrmInfo MetaDataStrmInfo;
    stMetaDataSeekTable MetaDataSeekTable;
}stMetaDataBlocks;


/***********************************************************************
* Function Name     : FLACDec_ParseStreamInfo()
* Description       : This function parses METADATA_BLOCK_STREAMINFO
* Parameters        : stMetaDataStrmInfo* pstMetaDataStrmInfo -> metadata
*                                                stream infor structure
*                   : stFLACDecBitStrm* pstFLACDecBitStrm -> Bit stream
*                                                           context
*                   : uint32 ui32MetaDataLen -> Metadata length
* Called functions  :FLACDecBSSkipBytes
*                   :FLACDecGetBits
* Global Data       :none
* Return Value      :FLACDecStatus FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        :none
***********************************************************************/
FLACDecStatus FLACDec_ParseStreamInfo(stMetaDataStrmInfo* pstMetaDataStrmInfo,
                                      stFLACDecBitStrm* pstFLACDecBitStrm,
                                      uint32 ui32MetaDataLen,
                                      eFLACDecErrCode* peError);


/***********************************************************************
* Function Name     : FLACDec_ParseSeekTable()
* Description       : This function parses the seek table
* Parameters        : stMetaDataStrmInfo* pstMetaDataStrmInfo -> metadata
*                                                stream infor structure
*                   : stFLACDecBitStrm* pstFLACDecBitStrm -> Bit stream
*                                                           context
*                   : uint32 ui32MetaDataLen -> Metadata length
* Called functions  : FLACDecBSSkipBytes
*                   : FLACDecGetBits
* Global Data       : none
* Return Value      : FLACDecStatus FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
***********************************************************************/
FLACDecStatus FLACDec_ParseSeekTable(stMetaDataSeekTable* pstMetaDataSeekTable,
                                     stFLACDecBitStrm* pstFLACDecBitStrm,
                                     uint32 ui32MetaDataLen);


/*************************************************************************
* Function Name     : FLACDec_SearchMetaData_Ogg()
* Description       : This function searches for MetaData FLAG/OGG
*                     sync word.
* Parameters        : FLACDecContext FLACDecHandle -> FLAC Decoder handle
*                     FLACDec_API *pstFLACAPI -> Config parameter from the
*                                                application
*                    eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDecGetBits
* Global Data       : none
* Return Value      : FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        : none
*************************************************************************/
FLACDecStatus FLACDec_SearchStreamSync(FLACDecContext FLACDecHandle,
                                         eFLACDecErrCode* peError);


/*************************************************************************
* Function Name     : FLACDec_ParseMetaData_Ogg()
* Description       : This function parses the OGG metadata
* Parameters        : FLACDecContext FLACDecHandle -> FLAC Decoder handle
*                     eFLACDecErrCode *peError -> Error code
* Called functions  : FLACDecGetBits
*                   : FLACDec_ParseStreamInfo
*                   : FLACDec_ParseSeekTable
*                   : FLACDec_Metadata
*                   : FLACDecBSSkipBytes
* Global Data       :none
* Return Value      :FLACDecStatus   FLACDEC_SUCCESS/FLACDEC_FAIL
* Exceptions        :none
*************************************************************************/
FLACDecStatus FLACDec_ParseMetaData(FLACDecContext FLACDecHandle,
                                        eFLACDecErrCode* peError);




#endif /*_TEL_FLACDEC_METADATA_H_ */

/******************************************************************************
 * End of file
 *****************************************************************************/


