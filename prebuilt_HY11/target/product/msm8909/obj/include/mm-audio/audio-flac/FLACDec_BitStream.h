/*************************************************************************
* Company                       : Tata Elxsi Ltd
* File                          : TEL_FLACDec_BitStream.h
* Author                        : TEL Audio Team
* Version                       : 1.0
* Date                          : 12-December-2009
* Modified Date                 : 14-October-2009
* Operating Environment         : Windows XP
* Compiler with Version         : Visual studio on PC
* Description                   : This file contains delcarations of
*                               :  BitStream Parser routines
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
#ifndef _TEL_FLACDEC_BITSTREAM_H_
#define _TEL_FLACDEC_BITSTREAM_H_

#define SEARCH_PATTERN 0x4f676753
#define MEM_POOL_SIZE (512 * 1024)
extern uint8  *pui8MemPoolPtr;

extern uint32 ui32Offset;
extern uint32 ui32InitMemSize;

#ifndef ENABLE_FREADS
typedef struct _InputBufferParams
{
    uint8* buffer;
    uint32 size;
    uint32 numBytesRemaining;
    uint32 error;
}stInBufferParams;
#endif

typedef struct _OggPackBuffer
{
   uint32           ui32Consumed;
   /* BitStream Error Handling */
   uint32           ui32PageConsumed;/*Total page size consumed*/
   uint32           ui32NoOfBytesRead;
   uint32           ui32BufSize;
   uint8            *pui8bufPtr;
} stOggPackBuffer;

typedef struct _FlacBlock
{
   int32              i32EofFlag;
   int64              i64GranulePos;
   } stFlacBlock;

/* Bitstream Parser Structure */
typedef struct _tagDTSDecBitStrm{
    uint8*      pui8IpBuff;                 /* I/P Buffer Pointer */
    uint32      ui32IpBuffSize;             /* Input Buffer Size */
    uint32      ui32BitsLeft;               /* Bits left inside Cache */
    uint32      ui32Cache;                  /* 4 Bytes input Cache */
    uint32      ui32BuffOffset;             /* Offset for i/p Buffer */
#ifdef ENABLE_FREADS
    PFNGetData  pFpGetData;                 /* Function pointer to read next word
                                               of data to cache, there are different
                                               implementations to read data in case
                                               of big endian streams & cd format streams */
#endif

    uint32      ui32EOS;                    /* End of Stream */
    uint32      ui32Length;
    /* Ogg Container Parameters */
    uint32      ui32PageSize;
    uint32      ui32PageHeaderSize;
    uint32      ui32Consumed;
    uint32      ui32Page_b_o_s;
    uint32      ui32Page_e_o_s;
    int32       is_Ogg;
    uint8       ui8crc;
    uint32      ui32crc;
    int32       ui32crcalign;
    int32       ogg_flag;
    int32       ui32align;
    /* ogg */
    stFlacBlock FlacBlock;
    uint32      ui32PageNo;
    uint8       ui8Version;
    uint8       EndOfStream; /*for eof in case if last page get corrupted */
    uint8       i32chained;
    uint32      PacketsDecoded; /*total packets decoded */
    uint8       FlagPacketEnd; /* indicate packet decode end */
    uint8       CRCFailure;     /* indicate CRC failure */
    uint8       FlagPageLost;   /* page no. mismatch */
    stOggPackBuffer  OggPackBuffer;
    uint32      ui32NumPackets;
    uint32      ui32PacketCount;
    uint32      ui32Segment;
    int32       aui32PacketSize[256];
    uint32      ui32UserData;
    uint32      ui32BuffOffsetAcc;

#ifndef ENABLE_FREADS
    stInBufferParams InBuffParams;
#endif

    uint8 InputBuffer[MAXINPBUFFER];
}stFLACDecBitStrm;

/* Function Pointer for FLACDecGetBits() */

extern uint32 (*FLACDecGetBits) (stFLACDecBitStrm* pstBit,  /* Bitstream context */
                        uint32 ui32BitsReq );
extern uint32 (*FLACDecGetBit) (stFLACDecBitStrm* pstBit,   /* Bitstream context */
                        uint32 ui32BitsReq );
extern int32 (*FLACDecGetSgndBits) (stFLACDecBitStrm* pstBit,   /* Bitstream context */
                          uint32 ui32BitsReq );     /* BitsRequired */

extern void32 (*FLACDecBSSkipBytes)( stFLACDecBitStrm *pstBit, uint32 ui32SkipBytes);

/*************************************************************************
* Function Name     : FLACDec_BSInit()
* Description       : Initializes the Bitstream decoding context
* Parameters        : stFLACDecBitStrm *pstBit -> Bitstream context
*                   : uint8 *pui8InputBuffer -> Compressed bitstream buffer
*                   : uint32 ui32InputBufferLength -> Bitstream buffer Length
*                   : PFNGetData FngetData -> Call back Function pointer
* Called functions  : None
* Global Data       : none
* Return Value      : 1 -> success
*                     0 -> failure
* Exceptions        : none
*************************************************************************/
uint32 FLACDec_BSInit(stFLACDecBitStrm *pstBit
#ifdef ENABLE_FREADS
                      ,uint8 *pui8InputBuffer,
                      uint32 ui32InputBufferLength,
                      PFNGetData FngetData
#endif
                      );


/*************************************************************************
* Function Name     : FLACDec_GetBits_FLAC()
* Description       : This function return the required number of bits
*                     from the FLAC input bit stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32BitsReq -> Bits Required
* Called functions  : FLACDec_CRC8
*                   : FLACDec_CRC16
* Global Data       : none
* Return Value      : Bit extracted from the stream
* Exceptions        : none
*************************************************************************/
uint32 FLACDec_GetBits_FLAC (stFLACDecBitStrm* pstBit,
                             uint32 ui32BitsReq );


/*************************************************************************
* Function Name     : FLACDec_GetBit_FLAC()
* Description       : This function return one bit
*                     from the FLAC input bit stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32BitsReq -> Bits Required
* Called functions  : FLACDec_CRC8
*                   : FLACDec_CRC16
* Global Data       : none
* Return Value      : Bit extracted from the stream
* Exceptions        : none
*************************************************************************/
uint32 FLACDec_GetBit_FLAC(stFLACDecBitStrm* pstBit, uint32 ui32BitsReq);


/*************************************************************************
* Function Name     : FLACDec_GetBits_Ogg()
* Description       : This function return the required number of bits
*                     from the OGG input bit stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32BitsReq -> Bits Required
* Called functions  : FLACDec_CRC8
*                   : FLACDec_CRC16
* Global Data       : none
* Return Value      : Bit extracted from the stream
* Exceptions        : none
*************************************************************************/
uint32 FLACDec_GetBits_Ogg (stFLACDecBitStrm* pstBit,
                            uint32 ui32BitsReq);


/*************************************************************************
* Function Name     : FLACDec_GetSgndBits_FLAC()
* Description       : This function return the required number of bits
*                     from the FLAC input bit stream with sign extension
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32BitsReq -> Bits Required
* Called functions  : FLACDec_CRC8
*                   : FLACDec_CRC16
* Global Data       : none
* Return Value      : Bit extracted from the stream
* Exceptions        : none
*************************************************************************/
int32 FLACDec_GetSgndBits_FLAC(stFLACDecBitStrm* pstBit,
                               uint32 ui32BitsReq);


/*************************************************************************
* Function Name     : FLACDec_GetSgndBits_Ogg()
* Description       : This function return the required number of bits
*                     from the OGG input bit stream with sign extension
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32BitsReq -> Bits Required
* Called functions  : FLACDec_CRC8
*                   : FLACDec_CRC16
* Global Data       : none
* Return Value      : Bit extracted from the stream
* Exceptions        : none
*************************************************************************/
int32 FLACDec_GetSgndBits_Ogg(stFLACDecBitStrm* pstBit,
                              uint32 ui32BitsReq);


/*************************************************************************
* Function Name     : FLACDec_BSSkipBytes_FLAC()
* Description       : This function skips the required number of bits
*                     from the FLAC input bit stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32SkipBytes -> Number of bits to be skipped
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_BSSkipBytes_FLAC(stFLACDecBitStrm *pstBit,
                                uint32 ui32SkipBytes);


/*************************************************************************
* Function Name     : FLACDec_BSSkipBytes_Ogg()
* Description       : This function skips the required number of bits
*                     from the OGG input bit stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32SkipBytes -> Number of bits to be skipped
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_BSSkipBytes_Ogg(stFLACDecBitStrm *pstBit,
                               uint32 ui32SkipBytes);


/*************************************************************************
* Function Name     : FLACDec_ByteAlign()
* Description       : This function byte aligns input bit stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_ByteAlign(stFLACDecBitStrm *pstBit);


/*************************************************************************
* Function Name     : FLACDec_ReadUTF8()
* Description       : This function reads 32 bit data from the UTF-8
*                     encoded stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32* pui32Value -> The pointer for storing the
*                                           output
* Called functions  :FLACDecGetBits
* Global Data       :none
* Return Value      :none
* Exceptions        :none
*************************************************************************/
void32 FLACDec_ReadUTF8(stFLACDecBitStrm *pstBit, uint32* pui32Value);


/*************************************************************************
* Function Name     : FLACDec_ReadUTF8_uint64()
* Description       : This function reads 64 bit data from the UTF-8
*                     encoded stream
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint64 *pui64Value -> The pointer for storing the
*                                           output
* Called functions  :FLACDecGetBits
* Global Data       :none
* Return Value      :none
* Exceptions        :none
*************************************************************************/
void32 FLACDec_ReadUTF8_uint64(stFLACDecBitStrm *pstBit,
                               uint64 *pui64Value);


/*************************************************************************
* Function Name     : FLACDec_GetUnary()
* Description       : This function returns the number of 0s before the
*                     next 1 (leading zeores)
* Parameters        : stFLACDecBitStrm *pstBit -> Bitstream context
*                   : uint32* pui32UnCount -> Pointer for storing the output
* Called functions  : FLACDecGetBits
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_GetUnary(stFLACDecBitStrm *pstBit, uint32* pui32UnCount);


/*************************************************************************
* Function Name     : FLACDec_CRC8()
* Description       : This function calculates the CRC 8 for the input
*                     buffer
* Parameters        : stFLACDecBitStrm *pstFLACDecBitStrm ->
*                   :                                Bitstream context
*                   : const uint8 *data -> Input buffer
*                   : int32 i32Len -> Length of the input buffer
* Called functions  : none
* Global Data       : FLACDec_CRC8_table
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_CRC8(stFLACDecBitStrm* pstFLACDecBitStrm,
                    const uint8 *data,
                    int32 i32Len);


/*************************************************************************
* Function Name     : FLACDec_CRC16()
* Description       : This function calculates the CRC 16 for the input
*                     buffer
* Parameters        : stFLACDecBitStrm* pstFLACDecBitStrm ->
*                   :                                Bitstream context
*                   : const uint8 *data -> Input buffer
*                   : int32 i32Len -> Length of the input buffer
* Called functions  : none
* Global Data       : FLACDec_CRC16_table
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_CRC16(stFLACDecBitStrm* pstFLACDecBitStrm,
                     const uint8 *data,
                     int32 i32Len);


/*************************************************************************
* Function Name     : FLACDec_PageCheck()
* Description       : This function extrats the FALC bitstream from the
*                     OGG Container
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
*                   : uint32 ui32Size -> Page size
* Called functions  : none
* Global Data       : none
* Return Value      : Returns the number of FLAC bitstream data
* Exceptions        :none
*************************************************************************/
uint32 FLACDec_PageCheck (stFLACDecBitStrm* pstBit,
                          uint32 ui32Size);


/*************************************************************************
* Function Name     : FLACDec_ReLoad()
* Description       : This function updates the bitstream inpu buffer
* Parameters        : stFLACDecBitStrm* pstBit -> Bitstream context
* Called functions  : FLACDec_OggPackBufferClear
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_ReLoad (stFLACDecBitStrm* pstBit);


/***********************************************************************
* Function Name     : FLACDec_OggPageGranulepos()
* Description       : Extracts Granule posistion form the OGG Container
* Parameters        : uint8 *pui8OggPtr -> OGG pointer
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
***********************************************************************/
int64 FLACDec_OggPageGranulepos (uint8 *pui8OggPtr);


/*************************************************************************
* Function Name     : FLACDec_OggPackBufferClear()
* Description       : This function clears Ogg packet buffer
* Parameters        : stOggPackBuffer *pstOggPackBuffer -> OGG packet buff
* Called functions  : none
* Global Data       : none
* Return Value      : none
* Exceptions        : none
*************************************************************************/
void32 FLACDec_OggPackBufferClear (stOggPackBuffer *pstOggPackBuffer);

#ifndef ENABLE_FREADS
FLACDecReadStatus FLACDec_GetData(uint8* pui8InputBuffer,
                                  uint32 ui32Bytestofill,
                                  uint32* pui32Bytesread,
                                  stInBufferParams *pstInBuffParams);
#endif

#endif /*_TEL_FLACDEC_BITSTREAM_H_ */

/******************************************************************************
 * End of file
 *****************************************************************************/

