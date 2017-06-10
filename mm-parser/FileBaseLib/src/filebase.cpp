/* =======================================================================
                              FileBase.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.  Include any initialization and synchronizing
  requirements.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright (c) 2008-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/filebase.cpp#106 $
$DateTime: 2014/05/04 22:38:25 $
$Change: 5822253 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */
#include "oscl_file_io.h"
#include "utf8conv.h"
#include "isucceedfail.h"
#include "filesourcestring.h"
#include "filebase.h"

#ifdef FEATURE_FILESOURCE_3GP_PARSER
#include "mpeg4file.h"
#include "mp4fragmentfile.h"
#endif

#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
#include "asffile.h"
#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */

#ifdef FEATURE_FILESOURCE_AVI
#include "avifile.h"
#endif
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  #include "DataSourcePort.h"
#endif
#ifdef FEATURE_FILESOURCE_DRM_DCF
   #include "IxStream.h"
#endif
#ifdef FEATURE_FILESOURCE_AMR
#include "amrfile.h"
#endif
#ifdef FEATURE_FILESOURCE_AMRWB
#include "amrwbfile.h"
#endif

#ifdef FEATURE_FILESOURCE_EVRC_WB
#include "evrcwbfile.h"
#endif

#ifdef FEATURE_FILESOURCE_EVRC_B
#include "evrcbfile.h"
#endif

#ifdef FEATURE_FILESOURCE_DTS
#include "dtsfile.h"
#endif

#ifdef FEATURE_FILESOURCE_MP3
#include "mp3file.h"
#endif

#ifdef FEATURE_FILESOURCE_AAC
#include "aacfile.h"
#endif

#ifdef FEATURE_FILESOURCE_QCP
#include "qcpfile.h"
#endif

#ifdef FEATURE_FILESOURCE_WAVADPCM
#include "wavfile.h"
#endif

#ifdef FEATURE_FILESOURCE_MPEG2_PARSER
#include "MP2Stream.h"
#endif
#ifdef FEATURE_FILESOURCE_AC3
#include "ac3file.h"
#endif /* FEATURE_FILESOURCE_AVI */

#ifdef FEATURE_FILESOURCE_OGG_PARSER
#include "OGGStream.h"
#endif

#ifdef FEATURE_FILESOURCE_FLAC_PARSER
#include "flacfile.h"
#endif

#ifdef FEATURE_FILESOURCE_RAW_PARSER
  #include "rawfile.h"
#endif

#ifdef FEATURE_FILESOURCE_RAW_H265_PARSER
  #include "rawh265file.h"
#endif
#ifdef FEATURE_FILESOURCE_MKV_PARSER
  #include "mkavfile.h"
#endif

#ifdef FEATURE_FILESOURCE_FLV_PARSER
#include "flvfile.h"
#endif

#ifdef FEATURE_FILESOURCE_REAL
  #include "realfile.h"
#endif

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/* ======================================================================
FUNCTION
  GetBitsFromBuffer

DESCRIPTION
  Generic function to extract specific number of bits from input buffer

INPUT PARAMETERS:
->uNeededBits: Number of bits need to extract
->uOffset: Bit Offset value
->pucInputBuffer: Input Data Buffer
->uMaxSize: Buffer Max Size

DEPENDENCIES
  None.

RETURN VALUE
  Value of bits extracted

SIDE EFFECTS
  None.

========================================================================== */
uint32 GetBitsFromBuffer(uint32 uNeededBits, uint32 uOffset,
                         uint8 * pucInputBuffer, uint32 const uMaxSize )
{

  uint32 uBitField =0, uMask;

  // Make sure we are not going to have to read out of bound
  if (( uMaxSize <= uOffset )||( uMaxSize < uOffset + uNeededBits)
          ||(32 <= uNeededBits))
  {
    return 0xFFFFFFFF; // bitfield failure when running out of data
  }

  // adjust to next uint8 boundary
  pucInputBuffer += (uOffset/8);
  uOffset %= 8;

  uMask = 0xFF >> uOffset;

  uint32 uLeftOffset = 8- uOffset;

  // extract the low bits
  uint8 uData = *(pucInputBuffer);
  uBitField = static_cast < uint32 > (uData & uMask);

  // If all the required bits are already available,
  // position it correctly and return
  if ( uNeededBits <= uLeftOffset)
  {
    uMask = (1 << uNeededBits) - 1;
    return (( uBitField >> (uLeftOffset - uNeededBits )) & uMask );
  }

  uNeededBits -= uLeftOffset;

  // fill with full bytes as needed
  while ( uNeededBits >= 8 )
  {
    uBitField <<= 8;
    uBitField += static_cast < uint32 > (*(++pucInputBuffer));
    uNeededBits -= 8;
  }

  // add remaining bits
  if ( uNeededBits > 0 )
  {
    uMask = (1 << uNeededBits) - 1;
    uBitField <<= uNeededBits;
    uData = *(++pucInputBuffer);
    uBitField += static_cast < uint32 >
                             ( uData >> ( 8 - uNeededBits )) & uMask;
  }

  return uBitField;
}
/*===========================================================================

FUNCTION  copyByteSwapData

DESCRIPTION:
  copies the data from one buffer to another.
  If byteswap is used, we may need to reverse the data.

INPUT/OUTPUT PARAMETERS:
  pucDstBuf    - OUTPUT        - buffer for data to be copied.
  uSize        - INPUT         - size of buffer and max data to be copied.
  pucSrcBuf    - INPUT         - buffer which need to be copied.
  byteSwap     - INPUT         - copy the data in reverse order if true.
  uAmount      - INPUT         - amount of data to be copied

RETURN VALUE:
  None

SIDE EFFECTS:
  None
===========================================================================*/
void copyByteSwapData(uint8 *pucDstBuf,
                      uint32 uSize,
                      uint8 *pucSrcBuf,
                      boolean byteSwap,
                      uint32 uAmount)
{
  uint32 uIndex;
  uint32 uDataToBeCopied = FILESOURCE_MIN(uSize, uAmount);
  if(pucDstBuf && pucSrcBuf && uSize)
  {
    if (byteSwap)
    {
      for (uIndex = 0; uIndex < uDataToBeCopied; ++uIndex)
      {
        pucDstBuf [uIndex] = pucSrcBuf[uDataToBeCopied - uIndex - 1];
      }
    }
    else
    {
      memcpy (pucDstBuf, pucSrcBuf, uDataToBeCopied);
    }
  }
}

/* ============================================================================
  @brief  This function is used to convert PCM sample data as per requirement

  @details This function is used to convert data from 8bit to 16bit or 24bit to
           16 bit or 24bit to 32bit format.

  @param[in]     pInBuf               Input data buffer.
  @param[in]     pOpBuf               Output data buffer.
  @param[in]     ulInSize             Input buffer size.
  @param[in]     pulOpSize            O/p buffer size.
  @param[in]     ucBitWidth           Bits per Sample field.
  @param[in]     bUpgrade             Flag to indicate whether upgrade
                                      or downgrade of data required.

  @return     None.
  @note       None.
============================================================================ */
void ConvertPCMSampleData(uint8* pInBuf, uint8* pOpBuf, uint32 ulInSize,
                          uint32* pulOpSize, uint8 ucBitWidth, bool bUpgrade)
{
  uint16* pWBuf = (uint16*)pOpBuf;
  uint32 ulIndex      = 0;
  uint32 ulSampleData = 0;
  uint32 ulOPBufSize  = 0;

  if(24 == ucBitWidth)
  {
    //! If user requests for downgrade, then convert from 24bit to 16bit
    if (false == bUpgrade)
    {
      for( ; (ulIndex + 3) < ulInSize; ulIndex +=3)
      {
        ulSampleData = 0;
        ulSampleData = pInBuf[ulIndex] | pInBuf[ulIndex + 1] << 8 |
                       pInBuf[ulIndex + 2] << 16;
        ulSampleData = (ulSampleData << 8) >> 8; //for sign extension
        //! Convert from 24bit to 16bit data
        ulSampleData = (ulSampleData >> 8);
        *pWBuf++ = (uint16)ulSampleData;
        ulOPBufSize += 2;
      }
    } //! if (false == bUpgrade)
    //! If user requests for upgrade, then convert from 24bit to 32bit
    else
    {
      for( ; (ulIndex + 3) < ulInSize; ulIndex +=3)
      {
        ulSampleData = 0;
        memcpy(&ulSampleData, (pInBuf + ulIndex), 3);
        ulSampleData = (((uint32)ulSampleData) << 8);
        memcpy((pOpBuf + ulOPBufSize), &ulSampleData, 4);
        ulOPBufSize += 4;
      }
    } //! else of if (false == bUpgrade)
  } //! if(24 == ucBitWidth)
  else if (8 == ucBitWidth)
  {
    /* Upgrade 8bit signed PCM to 16bit PCM data format.
       Following calculation will convert 8bit data into 16bit signed
       PCM format. */
    while(ulIndex < ulInSize)
    {
      *pWBuf++ = (uint16)((pInBuf[ulIndex++] - 0x80) * 0x100);
      ulOPBufSize += 2;
    }
  } //! else if (8 == ucBitWidth)

  *pulOpSize = ulOPBufSize;
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
    "ConvertPCMSampleData: %d bit sample data %lu converted to %lu",
               ucBitWidth,ulInSize, ulOPBufSize);
}

/* ======================================================================
FUNCTION
  ParseLOASFrameHeader

DESCRIPTION
  Function to validate whether input bitstream is LOAS compliant or not

INPUT PARAMETERS:
->pucDataBuf: Input Data Buffer
->ulFrameBufLen: Buffer Length

DEPENDENCIES
  None.

RETURN VALUE
  Error Type

SIDE EFFECTS
  None.

========================================================================== */
PARSER_ERRORTYPE ParseLOASFrameHeader(uint8*  pucDataBuf,
                                      uint32  ulFrameBufLen)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "ParseLOASFrameHeader");
  PARSER_ERRORTYPE result = PARSER_ErrorStreamCorrupt;
  uint8 ucData = 0;
  uint32 uOffset = 11;  // past sync bytpucDataBufes
  const uint32 uMaxSize = AAC_ADTS_HEADER_SIZE * 8;
  uint8* buffer     = pucDataBuf;
  uint32 ulFrameLen = 0;

  if(ulFrameBufLen < AAC_ADTS_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "Data is not sufficient to validate AAC");
      return PARSER_ErrorDataUnderRun;
  }

  /* Validate three frames. */
  for(uint32 nCount = 0; nCount < 3 ; nCount++)
  {
    //! Reduce buffer size
    ulFrameBufLen -= ulFrameLen;
    //! Check if buffer has sufficient data
    if(ulFrameBufLen < AAC_ADTS_HEADER_SIZE)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Data is not sufficient to validate AAC-LOAS");
      result = PARSER_ErrorDataUnderRun;
      break;
    }
    //! Reset return status as corrupt
    result  = PARSER_ErrorStreamCorrupt;
    //! Update buffer pointer to start of new frame sync marker
    buffer  = buffer + ulFrameLen;
    //! 11 bits are for sync marker byte, skip these bits
    uOffset = 11;
    //! Validate frame sync marker
    if(((buffer[0] << 3) | (buffer[1] >> 5)) != 0x2B7)
    {
      break;
    }

    //! Calculate frame length. It is available in 13bits after sync marker
    ulFrameLen  = GetBitsFromBuffer( 13, uOffset, buffer, uMaxSize);
    uOffset    += 13;
    //! Frame length does not include fixed sync header size (3bytes)
    ulFrameLen += 3;
    //! From second frame onwards, just valdiate frame sync marker
    if (nCount)
    {
      result = PARSER_ErrorNone;
      continue;
    }

    // Get useSameStreamMux
    ucData = (uint8) ( GetBitsFromBuffer (1, uOffset, buffer, uMaxSize ));
    ++uOffset;
    if ( ucData != 0 )
    {
      break;
    }
    // Get Audio mux version
    ucData = (uint8) ( GetBitsFromBuffer (1, uOffset, buffer, uMaxSize ));
    ++uOffset;
    if ( ucData != 0 )
    {
      break;
    }
    // Get allStreamsSameTimeFraming
    ucData = (uint8) ( GetBitsFromBuffer (1, uOffset, buffer, uMaxSize ));
    ++uOffset;
    if ( 0 == ucData )
    {
      break;
    }
    // Get numSubFrames
    ucData = (uint8) GetBitsFromBuffer ( 6, uOffset, buffer, uMaxSize );
    uOffset += 6;
    if ( ucData != 0 )
    {
      break;
    }
    // Get numProgram
    ucData = (uint8) GetBitsFromBuffer ( 4, uOffset, buffer, uMaxSize );
    uOffset += 4;
    if ( ucData != 0 )
    {
      break;
    }
    // Get numLayer
    ucData = (uint8)GetBitsFromBuffer ( 3, uOffset, buffer, uMaxSize );
    uOffset += 3;
    if ( ucData != 0 )
    {
      break;
    }
    ucData = (uint8)GetBitsFromBuffer ( 5, uOffset, buffer, uMaxSize);

    // Get Sampling frequency index
    uOffset += 5;
    ucData = (uint8)GetBitsFromBuffer(4, uOffset, buffer, uMaxSize);
    uOffset += 4;
    if (( ucData < 3 ) || ( ucData > 11 ))
    {
      break;
    }

    /* Comes to end of the loop means all the parameters successfully
       validated */
    result = PARSER_ErrorNone;
  }

  return result;
}

/* ======================================================================
FUNCTION
  ParseADTSFrameHeader

DESCRIPTION
  Function to validate whether input bitstream is ADTS compliant or not

INPUT PARAMETERS:
->pucDataBuf: Input Data Buffer
->ulBufLen: Buffer Length

DEPENDENCIES
  None.

RETURN VALUE
  Error Type

SIDE EFFECTS
  None.

========================================================================== */
PARSER_ERRORTYPE ParseADTSFrameHeader (uint8*  pucDataBuf, uint32 ulBufLen)
{
  PARSER_ERRORTYPE reterror = PARSER_ErrorStreamCorrupt;
  bool  bFixedADTSHdrSet = false;
  uint8  adts_fix_hdr[AAC_ADTS_FIX_HDR_SIZE];
  uint64 ullFrameLen = 0;

  for(uint32 nCount = 0; nCount < 2 ; nCount++)
  {
    if(ulBufLen < AAC_ADTS_HEADER_SIZE)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Data is not sufficient to validate AAC");
      reterror = PARSER_ErrorDataUnderRun;
      break;
    }
    pucDataBuf += (uint32)ullFrameLen;
    uint8* frame = pucDataBuf;
    uint16 uData = uint16((frame[1] << 8) + frame[0]);

    //Update status with Error. This will get updated to success only once.
    reterror = PARSER_ErrorStreamCorrupt;

    // Verify sync word and layer field.
    if (ADTS_HEADER_MASK_RESULT != (uData & ADTS_HEADER_MASK))
    {
      break;
    }

    // Extract frame length from the frame header
    ullFrameLen
                   = (static_cast<uint64> (frame [3] & 0x03) << 11)
                   | (static_cast<uint64> (frame [4]) << 3)
                   | (static_cast<uint64> (frame [5] & 0xE0) >> 5);

    // Verify we have a valid frame length
    if (0 == ullFrameLen)
    {
      break;
    }
    // parser framework handles max frame length of 32 bits
    ullFrameLen = (ullFrameLen & 0xFFFF);

    //Update the buffer lenght value
    if(ulBufLen > ullFrameLen)
      ulBufLen -= (uint32)ullFrameLen;
    else
      ulBufLen  = 0;

    // Extract sampling frequency
    const uint8 samplingFrequencyIndex = ((frame [2] >> 2) & 0x0F);

    //make sure sampling freq. index is within the range
    //layer should be always be 00
    if(samplingFrequencyIndex && samplingFrequencyIndex <= 12 &&
      (frame [1] & 0x06) == 0x00)
    {
      //if fixed header is not set, store the fixed header
      if(!bFixedADTSHdrSet)
      {
        //store 24 bits as it is
        memcpy(adts_fix_hdr, frame, AAC_ADTS_FIX_HDR_SIZE - 1);
        //only 4 bits are part of fixed header
        uint8 byte = frame[3];
        byte &= 0xF0;
        adts_fix_hdr[3] = byte;
        bFixedADTSHdrSet = true;
        reterror = PARSER_ErrorNone;
      }
      else
      {
        //compare this header against fixed header to detect false sync..
        if(memcmp(frame, adts_fix_hdr, AAC_ADTS_FIX_HDR_SIZE-1) == 0 &&
           adts_fix_hdr[3] == (frame[3] & 0xF0))
        {
          reterror = PARSER_ErrorNone;
        }
      }
    }//if(samplingFrequencyIndex <= 12 && ..)
    else
    {
      break;
    }
  }
  return reterror;
}

/* ============================================================================
  @brief  This function is used to convert lower case letters into upper case.

  @details This function is used to convert lower case letters into upper case.

  @param[in/out] pucStr                 String which needs to be converted.
  @param[in]     ulStrLen               Length of string.

  @return     None.
  @note       None.
============================================================================ */
void ConvertToUpperCase(uint8* pucStr, size_t ulStrLen)
{
  if(pucStr)
  {
    for(int nIndex = 0; nIndex < (int)ulStrLen; nIndex++)
    {
      if ((pucStr[nIndex] >= 'a') && (pucStr[nIndex] <= 'z'))
      {
        pucStr[nIndex] = uint8(pucStr[nIndex] & ~32);
      }
    }
  }
}

/* ============================================================================
  @brief  Function to parse MP3 header

  @details    This function is used to parse MPEG Audio Frame Header.

  @param[in/out] pucFramedata               Frame Data Pointer.
  @param[in]     ulFrameBufSize             Frame Buffer size.

  @return     TRUE to indicate Success.
              Else, FALSE will be returned.
  @note       None.
============================================================================ */
PARSER_ERRORTYPE ParseMP3FrameHeader (uint8* pucBuffer,
                                      uint32 ulFrameBufSize)
{
  PARSER_ERRORTYPE error = PARSER_ErrorNone;

  uint32 ulFrameLen = 0;
  uint32 nBufIndex  = 0;
  int    nCount     = 0;

  for( ; nCount < 2 && PARSER_ErrorNone == error; nCount++)
  {
    uint8* pucFramedata = pucBuffer;
    if((ulFrameLen + nBufIndex) >= ulFrameBufSize)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Data is not sufficient to validate MP3");
      error = PARSER_ErrorDataUnderRun;
      break;
    }
    else
    {
      nBufIndex += ulFrameLen;
    }
    while(nBufIndex < ulFrameBufSize)
    {
      if ((0xFF == pucFramedata[nBufIndex]) &&
          (0xE0 == (pucFramedata[nBufIndex+1] & 0xE0)) )
      {
        break;
      }

      //! Sync marker should be sequential from second frame onwards
      if ((nCount) || (nBufIndex > 2048) )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "MP3 Sync marker is not found within 2K data range");
        error = PARSER_ErrorInHeaderParsing;
        break;
      }
      else if (nBufIndex > ulFrameBufSize)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                    "Data is not sufficient to validate MP3");
        error = PARSER_ErrorDataUnderRun;
        break;
      }
      nBufIndex++;
      continue;
    }
    //! Update pointer to start of sync marker
    pucFramedata = pucBuffer + (nBufIndex);

    // extract the MP3 header information from input data
    uint8 version = (mp3_ver_enum_type)
      ((pucFramedata[MP3HDR_VERSION_OFS] & MP3HDR_VERSION_M) >> MP3HDR_VERSION_SHIFT);

    // Check if MPEG version of input data is supported
    if (!(MP3_VER_25 == version ||
          MP3_VER_2 == version ||
          MP3_VER_1 == version))
    {
      error = PARSER_ErrorInHeaderParsing;
    }
    uint8 layer = (mp3_layer_enum_type)
      ((pucFramedata[MP3HDR_LAYER_OFS] & MP3HDR_LAYER_M) >> MP3HDR_LAYER_SHIFT);

    // Check if MPEG layer of input data  is supported
    if(!((MP3_LAYER_3 == layer) ||
         (MP3_LAYER_2 == layer) ||
         (MP3_LAYER_1 == layer) ))
    {
      error = PARSER_ErrorInHeaderParsing;
    }

    byte bitrate_index = (uint8)
      ((pucFramedata[MP3HDR_BITRATE_OFS] &
      MP3HDR_BITRATE_M) >> MP3HDR_BITRATE_SHIFT);

    /* If bitrate index value is ZERO, then bitrate calculated is ZERO */
    if (MP3_MAX_BITRATE_INDEX <= bitrate_index || 0 == bitrate_index)
    {
      error = PARSER_ErrorInHeaderParsing;
    }

    uint8 sample_index = (uint8)
      ((pucFramedata[MP3HDR_SAMPLERATE_OFS] &
      MP3HDR_SAMPLERATE_M) >> MP3HDR_SAMPLERATE_SHIFT);

    if (MP3_MAX_SAMPLERATE_INDEX <= sample_index)
    {
      error = PARSER_ErrorInHeaderParsing;
    }
    if(PARSER_ErrorNone == error)
    {
      // Get the actual bitrate from bitrate index
      uint32 bitrate = 1000 *
        MP3_BITRATE[(version == MP3_VER_1 ? 0 : 1)]
                   [layer][bitrate_index];

      // Get the actual samplerate from samplerate index
      uint32 samplerate = MP3_SAMPLING_RATE[version][sample_index];
      boolean is_padding = (boolean)((pucFramedata[MP3HDR_PADDING_OFS] &
                            MP3HDR_PADDING_M) >> MP3HDR_PADDING_SHIFT);
      //calculate frameLength if bitrate and samplerate fields are valid
      if (samplerate && bitrate)
      {
        if( (layer == MP3_LAYER_2)|| (layer == MP3_LAYER_3))
        {
          // Using MP3 Coefficient & MP3 Slot size to calculate proper frame bytes
          // based on different MP3 layer types.
          // frame_bytes = ((MP3_Coeff * Bit-Rate)/Sample-Rate )* SlotSize
          ulFrameLen =  (MP3_COEFFICIENTS[(version == MP3_VER_1 ? 0 : 1)][layer] * bitrate
                        / samplerate)* MP3_SLOT_SIZES[layer];
          if(is_padding)
          {
            ulFrameLen++;
          }
        }
        else if(layer == MP3_LAYER_1)
        {
          ulFrameLen = (12 * bitrate / samplerate + is_padding) * 4;
        }
      }
    }//! if(PARSER_ErrorNone == error)
  }//! for( ; nCount < 2 && PARSER_ErrorNone == error; nCount++)
  return error;
}

/* ============================================================================
  @brief  Update AVC sample with start code.

  @details    This function is used to replace NAL Unit Size field with NAL
              Unit start code (0x00 00 00 01).

  @param[in]     ulNALULengthSizeMinusOne   NAL Unit Size field length.
  @param[in]     ulBufferSize               Buffer size.
  @param[in/out] pucDestBuf                 Destination buffer pointer.
  @param[in/out] pucSrcBuf                  Source buffer pointer.

  @return     TRUE to indicate Success.
              Else, FALSE will be returned.
  @note       None.
============================================================================ */
uint32 UpdateAVC1SampleWithStartCode(uint32 ulNALULengthSizeMinusOne,
                                     uint32 ulBufferSize,
                                     uint8* pucDestBuf,
                                     uint8* pucSrcBuf)
{
  uint32 ulBytesProcessed   = 0;
  uint32 ulDataBufFilledLen = 0;
  bool bSecondIter = false;
  while((pucSrcBuf) && (pucDestBuf) &&
        (ulBytesProcessed < ulBufferSize) &&
        (ulNALULengthSizeMinusOne <= ulBufferSize) )
  {
    //Write start code for every NALU length for H264
    const unsigned long ulStartCode = 0x01000000;
    (void) memmove (pucDestBuf + ulDataBufFilledLen, &ulStartCode,
                    sizeof(ulStartCode));
    ulDataBufFilledLen  += (uint32)sizeof(ulStartCode);
    uint32 ulNALUPayloadSize = 0;
    uint32 ulNALIndex    = 0;
    for (; ulNALIndex < ulNALULengthSizeMinusOne; ++ulNALIndex)
    {
      ulNALUPayloadSize = ulNALUPayloadSize << 8;
      ulNALUPayloadSize = ulNALUPayloadSize | pucSrcBuf[ulBytesProcessed +
                                                ulNALIndex];
      //! In second iteration, first NAL size will be extracted from
      //! 3 bytes only.
      if (!ulBytesProcessed && bSecondIter && ulNALIndex == 2)
      {
        ulNALIndex++;
        break;
      }
    }
    if((ulBufferSize - ulBytesProcessed) < ulNALUPayloadSize)
    {
      //! Sometimes first NAL is containing only 3 bytes as size as opposed to
      //! 4 bytes.
      if (!bSecondIter)
      {
        bSecondIter         = true;
        ulBytesProcessed    = 0;
        ulDataBufFilledLen  = 0;
        continue;
      }
      /* Return ZERO to indicate NAL Unit Size corruption. */
      else
      {
        //NALU size from a given access unit can't be > the access unit size
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                   "NALUnitLength %lu > Sample Size %lu - bytesPrcessed %lu",
                   ulNALUPayloadSize, ulBufferSize, ulBytesProcessed);
        return 0;
      }
    }
    else
    {
      // Skip NALU length and copy over NALU contents.
      (void) memmove (pucDestBuf + ulDataBufFilledLen,
                      &(pucSrcBuf [ulBytesProcessed + ulNALIndex]),
                      ulNALUPayloadSize);
      ulDataBufFilledLen += ulNALUPayloadSize;
      ulBytesProcessed   += (ulNALIndex + ulNALUPayloadSize);
    }
  }//while( (ulBytesProcessed < ulBufferSize) && (pucSrcBuf) && (pucDestBuf))
  return ulDataBufFilledLen;
}

/* ============================================================================
  @brief  This function is used to copy data from Big Endian format.

  @details This function is used to copy data from BE format to LE format.

  @param[in]     pvDest               Destination buffer.
  @param[in]     nDestSize            Destination buffer size.
  @param[in]     pvSrc                Source buffer.
  @param[in]     nSrcSize             source buffer size.
  @param[in]     pszFields            Word Length (4byte or 8 byte).

  @return     None.
  @note       None.
============================================================================ */
int CopyBE(void *pvDest,      int nDestSize,
           const void *pvSrc, int nSrcSize,
           const char *pszFields)
{
  uint8 *pSource = (uint8 *)pvSrc;
  uint8 *pDest   = (uint8 *)pvDest;
  int nIndex     = 0;
  int nVarSize   = 2;
  int nCount     = 0;
  if ((!pSource) || (!pDest) || (nDestSize > nSrcSize))
  {
    return 0;
  }
  if('L' == *pszFields)
  {
    nVarSize = DOUBLEWORD_LENGTH;
  }
  else if('Q' == *pszFields)
  {
    nVarSize= QUADWORD_LENGTH;
  }
  for (nIndex = 0; nIndex < nDestSize; ++nIndex)
  {
    if((nIndex) % nVarSize == 0)
    {
      nCount++;
    }
    pDest [nIndex] = pSource[nVarSize*nCount - nIndex - 1];
  }

  return nDestSize;
}

/* ======================================================================
FUNCTION
  SAMPLE_FUNC

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */

/* ======================================================================
FUNCTION
  *FileBase::openMediaFile

DESCRIPTION
  Static method to read in a media file from disk and return the FileBase interface

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FileBase *FileBase::openMediaFile(unsigned char *pBuf,
                                  uint32 bufSize,
                                  bool bPlayVideo,
                                  bool bPlayAudio,
                                  bool bPlayText
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || \
    defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                                  ,bool bPseudoStream
                                  ,uint32 wBufferOffset
#endif  /* FEATURE_FILESOURCE_PSEUDO_STREAM */
        /* FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD */
                                  ,FileSourceFileFormat format
                                    )
{
#ifdef FEATURE_FILESOURCE_RAW_PARSER
  if(format == FILE_SOURCE_RAW)
  {
    RawFile* raw_file = MM_New_Args( RawFile, (NULL,pBuf,bufSize) );
    return raw_file;
  }
#endif
#ifdef FEATURE_FILESOURCE_RAW_H265_PARSER
  if(IsRawH265File(NULL,pBuf,false))
  {
    CRawH265File *hRawH265File = NULL;
    hRawH265File = MM_New_Args( CRawH265File, (NULL, pBuf, bufSize, FILE_SOURCE_RAW_H265) );
    if (hRawH265File)
    {
      return hRawH265File;
    }
    return NULL;
  }
#endif /*__#ifdef FEATURE_FILESOURCE_RAW_H264_PARSER__*/

#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
  uint32 size = (uint32)sizeof(asfFileIdentifier)+1;
 // uint8 Buf[sizeof(asfFileIdentifier)+1];
 // size = readFile(filename, Buf, 0, size);

  if((FILE_SOURCE_ASF == format) || IsASFFile(pBuf, size))
  {
    ASFFile *asf = NULL;
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)

    asf = MM_New_Args( ASFFile, (NULL, pBuf, bufSize,NULL, bPlayVideo, bPlayAudio,
                                 bPseudoStream, wBufferOffset) );

#else   /* #if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */

   asf = MM_New_Args( ASFFile, (NULL, pBuf, bufSize,NULL, bPlayVideo, bPlayAudio) );

#endif /* #if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD) */
    if (asf)
    {
      if ( asf->FileSuccess())
      {
        return asf;
      }
      else
      {
        MM_Delete( asf);
      }
    }
    return NULL;
  }
#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */

#ifdef FEATURE_FILESOURCE_REAL
    if(IsRMFile(NULL,pBuf,false))
    {
      RealFile *rm = NULL;
      rm = MM_New_Args( RealFile, (NULL, pBuf, bufSize, bPlayVideo, bPlayAudio) );
      if (rm)
      {
        return rm;
      }
      return NULL;
    }
#endif //FEATURE_FILESOURCE_REAL

#ifdef FEATURE_FILESOURCE_AVI
    if(IsAVIFile(NULL,pBuf,false))
    {
      AVIFile *avi = NULL;
      #if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
        avi = MM_New_Args( AVIFile, (NULL, pBuf, bufSize, bPlayVideo, bPlayAudio, bPseudoStream, wBufferOffset));
      #else
        avi = MM_New_Args( AVIFile, (NULL, pBuf, bufSize, bPlayVideo, bPlayAudio) );
      #endif
      if (avi)
      {
        return avi;
      }
      return NULL;
    }
#endif //FEATURE_FILESOURCE_AVI
    {
#ifdef FEATURE_FILESOURCE_3GP_PARSER
      Mpeg4File *mp4 = NULL;
      if(IsMP4_3GPFile(NULL, pBuf, bufSize, false))
      {
#ifdef FEATURE_FILESOURCE_PSEUDO_STREAM

          mp4 = MM_New_Args(Mp4FragmentFile, (NULL, pBuf, bufSize, bPlayVideo, bPlayAudio, bPlayText,
                                              bPseudoStream, wBufferOffset));
#elif defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
          mp4 = MM_New_Args( Mp4FragmentFile, (NULL, pBuf, bufSize, bPlayVideo, bPlayAudio, bPlayText,
                         bPseudoStream, wBufferOffset) );
#else
          mp4 = MM_New_Args( Mp4FragmentFile, (NULL, pBuf, bufSize,bPlayVideo,bPlayAudio, bPlayText) );
#endif //FEATURE_FILESOURCE_PSEUDO_STREAM
      }
      if (mp4)
      {
        if ( mp4->FileSuccess())
        {
          mp4->parseFirstFragment();
          return mp4;
        }
        else
        {
          MM_Delete( mp4 );
        }
      }
#endif
      return NULL;
    }
}



/* ======================================================================
FUNCTION
  FileBase::openMediaFile

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FileBase* FileBase::openMediaFile(  FILESOURCE_STRING filename,
                                    bool bPlayVideo,
                                    bool bPlayAudio,
                                    bool bPlayText,
                                    bool blookforcodechdr,
                                    FileSourceFileFormat format)
{
#ifdef FEATURE_FILESOURCE_RAW_PARSER
  if(format == FILE_SOURCE_RAW)
  {
    RawFile* raw_file = MM_New_Args( RawFile, (filename, NULL, 0) );
    return raw_file;
  }
#endif
  void *p_audiofmtfile = NULL;
  uint8  FileFormatBuf[FILE_FORMAT_BUF_SIZE];
  uint32 FileFormatBufSize = FILE_FORMAT_BUF_SIZE;
  uint32 nBytesRead = readFile(filename, FileFormatBuf, 0, FileFormatBufSize);
  if(nBytesRead != FileFormatBufSize)
  {
    return NULL;
  }

#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
  #ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if( IsASFFile(filename) )
  #else
  if( IsASFFile(FileFormatBuf, FileFormatBufSize) )
  #endif
  {
    ASFFile *asf = NULL;
    asf = MM_New_Args( ASFFile, (filename, NULL,0,0,bPlayVideo,bPlayAudio) );
    if (asf)
    {
      return asf;
    }
    return NULL;
  }
  else
#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */

#ifdef FEATURE_FILESOURCE_RAW_H265_PARSER
#ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
    if(IsRawH265File(filename,FileFormatBuf,false))
#else
    if(IsRawH265File(filename,FileFormatBuf,true))
#endif
    {
      CRawH265File* hRawH265File = MM_New_Args( CRawH265File,
                                                ( filename,
                                                  NULL,
                                                  0,
                                                  FILE_SOURCE_RAW_H265));
      return hRawH265File;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_REAL
    if(IsRMFile(filename,FileFormatBuf,false))
    {
      RealFile *rm = NULL;
      rm = MM_New_Args( RealFile, (filename, FileFormatBuf, 0, bPlayVideo, bPlayAudio) );
      if (rm)
      {
        return rm;
      }
      return NULL;
    }
#endif //FEATURE_FILESOURCE_REAL


#ifdef FEATURE_FILESOURCE_AVI
  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(IsAVIFile(filename,FileFormatBuf,false))
  #else
  if(IsAVIFile(filename,FileFormatBuf,true))
  #endif
  {
    AVIFile* aviFile = MM_New_Args( AVIFile, (filename, NULL, 0, bPlayVideo, bPlayAudio) );
    return aviFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_AMR

  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(IsAMRFile(filename,FileFormatBuf,false) )
  #else
  if(IsAMRFile(filename,FileFormatBuf,true))
  #endif
  {
    AMRFile* amrFile = MM_New_Args( AMRFile, (filename, NULL, 0) );
    return amrFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_AMRWB

  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(IsAMRWBFile(filename,FileFormatBuf,false) )
  #else
  if(IsAMRWBFile(filename,FileFormatBuf,true))
  #endif
  {
    AMRWBFile* amrwbFile = MM_New_Args( AMRWBFile, (filename, NULL, 0) );
    return amrwbFile;
  }
  else
#endif


#ifdef FEATURE_FILESOURCE_EVRC_WB

  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(IsEVRCWBFile(filename,FileFormatBuf,false) )
  #else
  if(IsEVRCWBFile(filename,FileFormatBuf,true))
  #endif
  {
    EVRCWBFile* evrcwbFile = MM_New_Args( EVRCWBFile, (filename,NULL, 0) );
    return evrcwbFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_DTS

  if(IsDTSFile(FileFormatBuf, FileFormatBufSize) )
  {
    cDTSFile* dtsFile = MM_New_Args( cDTSFile, (filename, NULL, 0) );
    return dtsFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_EVRC_B

  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(IsEVRCBFile(filename,FileFormatBuf,false) )
  #else
  if(IsEVRCBFile(filename,FileFormatBuf,true))
  #endif
  {
    EVRCBFile* evrcbFile = MM_New_Args( EVRCBFile, (filename, NULL, 0) );
    return evrcbFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_QCP

  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(IsQCPFile(filename,FileFormatBuf,false) )
  #else
  if(IsQCPFile(filename,FileFormatBuf,true) )
  #endif
  {
    QCPFile* qcpFile = MM_New_Args( QCPFile, (filename, NULL, 0) );
    return qcpFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_WAVADPCM

  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(IsWAVADPCMFile(filename,FileFormatBuf,false) )
  #else
  if(IsWAVADPCMFile(filename,FileFormatBuf,true)  )
  #endif
  {
    WAVFile* wavadpcmFile = MM_New_Args( WAVFile, (filename, NULL, 0) );
    return wavadpcmFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_MPEG2_PARSER
  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
    if(IsMPEG2File(filename,FileFormatBuf,FileFormatBufSize, false))
  #else
    if(IsMPEG2File(NULL,FileFormatBuf, FileFormatBufSize, true))
  #endif
    {
      MP2Stream* mpeg2File = MM_New_Args( MP2Stream, (filename,
            blookforcodechdr,NULL, 0, bPlayVideo, bPlayAudio, format));
      return mpeg2File;
    }
    else
#endif

#ifdef FEATURE_FILESOURCE_OGG_PARSER
  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
    if(IsOggFile(filename,FileFormatBuf,false))
  #else
    if(IsOggFile(NULL,FileFormatBuf,true))
  #endif
    {
      OGGStream* oggFile = MM_New_Args( OGGStream, (filename, NULL, 0, bPlayVideo, bPlayAudio) );
      return oggFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  #ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
    if(IsFlacFile(filename,FileFormatBuf,false) )
  #else
    if(IsFlacFile(NULL,FileFormatBuf,true))
  #endif
    {
      flacfile* flcfile = MM_New_Args( flacfile,(filename,NULL,0) );
      //flacfile* flacfile = new flacfile(filename,NULL,0);
      return flcfile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_3GP_PARSER
    // In MP4/3gp case, first four bytes are atom Size and next four bytes are atom Type.
    if(IsMP4_3GPFile(filename,FileFormatBuf, FileFormatBufSize,false))
    {
      Mpeg4File *mp4 = NULL;
      mp4 = MM_New_Args( Mp4FragmentFile, (filename, NULL,0,bPlayVideo,bPlayAudio,bPlayText) );
      if (mp4)
      {
        mp4->parseFirstFragment();
        if ( mp4->FileSuccess() )
        {
          return mp4;
        }
        else
        {
          MM_Delete( mp4 );
        }
      }
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_MKV_PARSER
    if(IsMKVFile(filename,FileFormatBuf,false))
    {
      MKAVFile* mkfile = NULL;
      mkfile = MM_New_Args(MKAVFile,(filename,0,0,bPlayVideo,bPlayAudio));
      return mkfile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_FLV_PARSER
      if(IsFLVFile(NULL,FileFormatBuf,false))
      {
        FLVFile* pFLVfile = NULL;
        pFLVfile = MM_New_Args(FLVFile,(filename,0,0,bPlayVideo,bPlayAudio));
        return pFLVfile;
      }
      else
#endif
#ifdef FEATURE_FILESOURCE_AC3
  {
    AC3File* ac3File = MM_New_Args( AC3File, (filename, NULL, 0) );
    if(true == ac3File->isAC3File())
      return ac3File;
    else
      MM_Delete(ac3File);
  }
#endif /* FEATURE_FILESOURCE_AC3 */

      /* There are lot of corrupted AAC/MP3 content with same frame sync
         marker. In order to make implementation easier, no checks are added
         for AAC and MP3 file formats..
      */
#ifdef FEATURE_FILESOURCE_AAC
#ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
      if(IsAACFile(filename,&p_audiofmtfile,false) && p_audiofmtfile != NULL)
#else
      if(IsAACFile(filename,&p_audiofmtfile,true))
#endif
      {
        return (FileBase*)p_audiofmtfile;
      }
#endif
#ifdef FEATURE_FILESOURCE_MP3

#ifndef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
    (void)IsMP3File(filename,&p_audiofmtfile,false);
    if(p_audiofmtfile != NULL)
#else
    if(IsMP3File(filename,&p_audiofmtfile,true) )
#endif
    {
      return (FileBase*)p_audiofmtfile;
    }
#endif
  return NULL;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION
  FileBase::openMediaFile

DESCRIPTION
  Playback support from StreamPort

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
FileBase* FileBase::openMediaFile(  video::iStreamPort* pPort,
                                    bool bPlayVideo,
                                    bool bPlayAudio,
                                    bool bPlayText,
                                    bool blookforcodechdr,
                                    FileSourceFileFormat format)
{
#if defined(FEATURE_FILESOURCE_AAC) || defined(FEATURE_FILESOURCE_MP3)
  void *p_audiofmtfile = NULL;
#endif
  if(!pPort)
  {
    return NULL;
  }
  uint8  FileFormatBuf[FILE_FORMAT_BUF_SIZE];
  uint32 FileFormatBufSize = FILE_FORMAT_BUF_SIZE;
  uint32 nBytesRead  = 0;

  if(format == FILE_SOURCE_UNKNOWN)
  {
    nBytesRead = readFile(pPort, FileFormatBuf, 0, FileFormatBufSize);
    //! Read at least 64 bytes worth of data
    if(nBytesRead < DTS_FILE_SIGNATURE_BYTES)
    {
      return NULL;
    }
  }
  else
  {
    return initFormatParser(pPort,bPlayVideo,bPlayAudio,bPlayText,blookforcodechdr,format);
  }

#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
  if( IsASFFile(FileFormatBuf, FileFormatBufSize) )
  {
    ASFFile *asf = NULL;
    asf = MM_New_Args( ASFFile, (pPort, NULL,0,0,bPlayVideo,bPlayAudio) );
    if (asf)
    {
      return asf;
    }
    return NULL;
  }
  else
#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */

#ifdef FEATURE_FILESOURCE_RAW_H265_PARSER
    if(IsRawH265File(pPort,FileFormatBuf,false))
    {
      CRawH265File* hRawH265File = MM_New_Args( CRawH265File,
                                              (pPort,FILE_SOURCE_RAW_H265));
      return hRawH265File;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_AVI
  if(IsAVIFile(pPort,FileFormatBuf,false))
  {
    AVIFile* aviFile = MM_New_Args( AVIFile, (pPort, bPlayVideo, bPlayAudio) );
    return aviFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_AMR
  if(IsAMRFile(NULL,FileFormatBuf,false) )
  {
    AMRFile* amrFile = MM_New_Args( AMRFile, (pPort) );
    return amrFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_AMRWB
  if(IsAMRWBFile(NULL,FileFormatBuf,false) )
  {
    AMRWBFile* amrwbFile = MM_New_Args( AMRWBFile, (pPort) );
    return amrwbFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_EVRC_WB
  if(IsEVRCWBFile(NULL,FileFormatBuf,false) )
  {
    EVRCWBFile* evrcwbFile = MM_New_Args( EVRCWBFile, (pPort) );
    return evrcwbFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_EVRC_B
  if(IsEVRCBFile(NULL,FileFormatBuf,false) )
  {
    EVRCBFile* evrcbFile = MM_New_Args( EVRCBFile, (pPort) );
    return evrcbFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_DTS

  if(IsDTSFile(FileFormatBuf, FileFormatBufSize) )
  {
    cDTSFile* dtsFile = MM_New_Args( cDTSFile, (pPort) );
    return dtsFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_QCP
  if(IsQCPFile(NULL,FileFormatBuf,false) )
  {
    QCPFile* qcpFile = MM_New_Args( QCPFile, (pPort) );
    return qcpFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_WAVADPCM
  if(IsWAVADPCMFile(NULL,FileFormatBuf,false) )
  {
    WAVFile* wavadpcmFile = MM_New_Args( WAVFile, (pPort) );
    return wavadpcmFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_MPEG2_PARSER
  if(IsMPEG2File(NULL,FileFormatBuf,FileFormatBufSize, false))
  {
    MP2Stream* mpeg2File = MM_New_Args( MP2Stream, (pPort,
          blookforcodechdr,bPlayVideo, bPlayAudio, format));
    return mpeg2File;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_OGG_PARSER
  if(IsOggFile(NULL,FileFormatBuf,false))
  {
    OGGStream* oggFile = MM_New_Args( OGGStream, (pPort, bPlayVideo, bPlayAudio) );
    return oggFile;
  }
  else
#endif
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  if(IsFlacFile(NULL,FileFormatBuf,false) )
  {
    flacfile* flcfile = MM_New_Args( flacfile,(pPort) );
    return flcfile;
  }
  else
#endif
#ifdef FEATURE_FILESOURCE_3GP_PARSER
  if(IsMP4_3GPFile(NULL,FileFormatBuf, FileFormatBufSize,false))
  {
    Mpeg4File *mp4 = NULL;
    mp4 = MM_New_Args( Mp4FragmentFile, (pPort, bPlayVideo, bPlayAudio,
                                         bPlayText, format) );
    if (mp4)
    {
      mp4->parseFirstFragment();
      if ( mp4->FileSuccess() )
      {
        return mp4;
      }
      else
      {
        MM_Delete( mp4 );
        return NULL;
      }
    }
  }
  else
#endif
#ifdef FEATURE_FILESOURCE_MKV_PARSER
  if(IsMKVFile(NULL,FileFormatBuf,false))
  {
    MKAVFile* mkfile = NULL;
    mkfile = MM_New_Args(MKAVFile,(pPort,bPlayVideo,bPlayAudio));
    return mkfile;
  }
  else
#endif
#ifdef FEATURE_FILESOURCE_FLV_PARSER
    if(IsFLVFile(NULL,FileFormatBuf,false))
    {
      FLVFile* pFLVfile = NULL;
      pFLVfile = MM_New_Args(FLVFile,(pPort,bPlayVideo,bPlayAudio));
      return pFLVfile;
    }
    else
#endif

#ifdef FEATURE_FILESOURCE_AC3
  {
    AC3File* ac3File = MM_New_Args( AC3File, (pPort) );
    if(true == ac3File->isAC3File())
      return ac3File;
    else
      MM_Delete(ac3File);
  }
#endif

#ifdef FEATURE_FILESOURCE_AAC
  if( IsAACFile(pPort,&p_audiofmtfile))
  {
    return (FileBase*)p_audiofmtfile;
  }
#endif
#ifdef FEATURE_FILESOURCE_MP3
  if(IsMP3File(pPort,&p_audiofmtfile))
  {
    return (FileBase*)p_audiofmtfile;
  }
#endif
  return NULL;
}
#endif

#ifdef FEATURE_FILESOURCE_DRM_DCF
/* ======================================================================
FUNCTION
  FileBase::openMediaFile

DESCRIPTION
  Creates media instance for supporting DCF playback

INPUT PARAMETERS:
->inputStream:IxStream*
->urnType:It should be URN_INPUTSTREAM
->bPlayVideo:Indicates if this is video instance
->bPlayAudio:Indicates if this is audio instance
->bPlayText:Indicates if this is text instance

DEPENDENCIES
None

RETURN VALUE
FileBase* (media instance such as mpeg4file/asfifle pointer)

SIDE EFFECTS
None
========================================================================== */
FileBase* FileBase::openMediaFile(  IxStream* inputStream,
                                    bool bPlayVideo,
                                    bool bPlayAudio,
                                    bool bPlayText)
{
  uint8  FileFormatBuf[FILE_FORMAT_BUF_SIZE];
  uint32 FileFormatBufSize = FILE_FORMAT_BUF_SIZE;
  uint32 nBytesRead = readFile(inputStream, FileFormatBuf, 0, FileFormatBufSize);
  if(nBytesRead != FileFormatBufSize)
  {
    return NULL;
  }
#ifdef FEATURE_FILESOURCE_MP3
  void* mp3filehnd = NULL;
#endif

#ifdef FEATURE_FILESOURCE_AAC
  void* aacfilehnd = NULL;
#endif

#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
  if( IsASFFile(FileFormatBuf, FileFormatBufSize) )
  {
    ASFFile *asf = NULL;
    asf = MM_New_Args( ASFFile, (inputStream, bPlayVideo, bPlayAudio) );
    if(asf)
    {
      return asf;
    }
    return NULL;
  }
  else
#endif /* FEATURE_FILESOURCE_WINDOWS_MEDIA */

#ifdef FEATURE_FILESOURCE_AMR
  if(IsAMRFile(NULL,FileFormatBuf,false))
  {
     AMRFile* amrFile = MM_New_Args( AMRFile, (inputStream) );
     return amrFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_AMRWB
  if(IsAMRWBFile(NULL,FileFormatBuf,false))
  {
    AMRWBFile* amrwbFile = MM_New_Args( AMRWBFile, (inputStream) );
    return amrwbFile;
  }
  else
#endif
#ifdef FEATURE_FILESOURCE_QCP
  if(IsQCPFile(NULL,FileFormatBuf,false,inputStream))
  {
    QCPFile* qcpFile = MM_New_Args( QCPFile, (inputStream) );
    return qcpFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_WAVADPCM
  if(IsWAVADPCMFile(NULL,FileFormatBuf,false,inputStream))
  {
    WAVFile* wavadpcmFile = MM_New_Args( WAVFile, (inputStream) );
    return wavadpcmFile;
  }
  else
#endif

#ifdef FEATURE_FILESOURCE_3GP_PARSER
  if(IsMP4_3GPFile(NULL,FileFormatBuf + FOURCC_SIGNATURE_BYTES,false))
  {
    Mpeg4File *mp4 = NULL;
    mp4 = MM_New_Args( Mp4FragmentFile, (inputStream, bPlayVideo, bPlayAudio, bPlayText));
    if(mp4)
    {
      if ( mp4->FileSuccess() )
      {
        mp4->parseFirstFragment();
        return mp4;
      }
      else
      {
        MM_Delete( mp4 );
        return NULL;
      }
    }
    return NULL;
  }
  else
#endif
#ifdef FEATURE_FILESOURCE_AAC
    if(IsAACFile(NULL,&aacfilehnd,false,inputStream) )
    {
      return (FileBase*)aacfilehnd;
    }
#endif
#ifdef FEATURE_FILESOURCE_MP3
  if(IsMP3File(NULL,&mp3filehnd,false,inputStream) )
  {
    return (FileBase*)mp3filehnd;
  }
#endif
  return NULL;

}

/* ======================================================================
FUNCTION
  FileBase::readFile

DESCRIPTION
  opens and reads requested bytes from IxStream

INPUT/OUTPUT
  inputStream  = inputStream to be be read from.
  buffer    = buffer in which to read
  pos       = from where to start reading in file
  size      = size of the buffer.

DEPENDENCIES
 None

RETURN VALUE
  bytes read
  0 in case of error

SIDE EFFECTS
  None

========================================================================== */
uint32 FileBase::readFile( IxStream* inputStream,
                            uint8 *buffer,
                            uint64 pos,
                            uint32 size )
{
  uint32 nRead = 0;
  IxStream* pStream = NULL;
  bool   isEndofStream = false;
  uint32 reqSizeToRead = size;

  if(inputStream == NULL)
    return nRead;

  /*
  * This function is used only to read first few bytes to decide whether it's an ASF/3GP file.
  * if we don't do pStream->Seek(0,IX_STRM_SEEK_START) at the begining before attempting to read,
  * we will hit an error when attempting to play WM clip without exiting QTV.
  *
  * For all other subsequent reads, readFile( OSCL_FILE *fp, ..) is used.
  */

  IxErrnoType errorCode = E_FAILURE;
  pStream = (IxStream*)inputStream;
  if(pStream)
  {
    errorCode = pStream->Seek((uint32)pos,IX_STRM_SEEK_START);
    if(errorCode == E_SUCCESS)
    {
      errorCode = E_FAILURE;
      errorCode = pStream->Read((byte*)buffer,(uint32)reqSizeToRead,&nRead,&isEndofStream);
      if(errorCode == E_SUCCESS)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "FileBase::readFile successful nRead %d",nRead );
        errorCode = pStream->Seek(0,IX_STRM_SEEK_START);
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Moved back IxStream to BEGINING errorCode= %d",errorCode );
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL, "FileBase::readFile failed error code %d",errorCode );
      }
    }
    else
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL, "FileBase::readFile pStream->Seek failed errorCode %d",errorCode );
    }
  }
  return nRead;
}
#endif


/* ======================================================================
FUNCTION
  FileBase::readFile

DESCRIPTION
  opens the file, reads it and closes the file.

INPUT/OUTPUT
  filename  = name of the file to be read.
  buffer    = buffer in which to read
  pos       = from where to start reading in file
  size      = size of the buffer.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  bytes read
  0 in case of error

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 FileBase::readFile( FILESOURCE_STRING filename,
                            uint8 *buffer,
                            uint64 pos,
                            uint32 size )
{
  uint32 nRead = 0;
  OSCL_FILE *fp = NULL;
  fp = OSCL_FileOpen (filename, (OSCL_TCHAR *) _T("rb"));
  if( fp )
  {
    nRead = readFile(fp, buffer, pos, size);
    (void)OSCL_FileClose(fp);
  }
  return nRead;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION
  FileBase::readFile

DESCRIPTION
  Reads specifid number of bytes from given stream port

INPUT/OUTPUT
  pPort     = Stream port to read from
  buffer    = buffer in which to read
  pos       = from where to start reading in file
  size      = size of the buffer.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  bytes read
  0 in case of error

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 FileBase::readFile( video::iStreamPort* pPort, uint8 *buffer,
                          uint64 pos, uint32 size,
                          bool *pbDataUnderRun )
{
  ssize_t nRead = 0;
  int64 nOutOffset = 0;
  if( pPort )
  {
    if (pPort->Seek(pos,video::iStreamPort::DS_SEEK_SET,&nOutOffset) ==
        video::iStreamPort::DS_SUCCESS)
    {
      video::iStreamPort::DataSourceReturnCode eStatus;
      eStatus = pPort->Read(buffer, (size_t)size, &nRead);
      if(pbDataUnderRun)
      {
        *pbDataUnderRun = false;
      }
      if ((pbDataUnderRun) && (video::iStreamPort::DS_WAIT == eStatus))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "readFile returned Under-run");
        *pbDataUnderRun = true;
      }

    }
    else
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "readFile seek failed");
  }
  return (uint32)nRead;
}
#endif
/*===========================================================================

FUNCTION
  FileBase::readFile

DESCRIPTION
  reads from file referenced by file pointer

INPUT/OUTPUT
  fp        = file pointer
  buffer    = buffer in which to read
  pos       = from where to start reading in file
  size      = size of the buffer.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  bytes read
  0 in case of error

SIDE EFFECTS
  Detail any side effects.

===========================================================================*/
uint32 FileBase::readFile( OSCL_FILE *fp,
                            uint8 *buffer,
                            uint64 pos,
                            uint32 size,
                            bool *pbDataUnderRun,
                            bool  bMediaAbort)
{
  if (pbDataUnderRun)
  {
    *pbDataUnderRun = false;
  }
  if((!fp) || (bMediaAbort))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "NULL File Pointer/User Aborted");
    return 0;
  }
  if ((fp->pStreamPort) && fp->videoHandle == FILE_HANDLE_INVALD)
  {
    return readFile(fp->pStreamPort, buffer, pos, size, pbDataUnderRun);
  }

  return (uint32)OSCL_FileSeekRead (buffer, 1, size, fp,  pos, SEEK_SET);
}

/*===========================================================================

FUNCTION
  FileBase::seekFile

DESCRIPTION
  sets the file pointer at given offset

INPUT/OUTPUT
  fp        = file pointer
  offset    = what is the offset
  origin    = location from where offset is applied

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 (-1) for error
 0 for success

SIDE EFFECTS
  Detail any side effects.

===========================================================================*/
int32 FileBase::seekFile( OSCL_FILE *fp, uint64 /*offset*/, uint32 /*origin*/)
{
  if(!fp)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "NULL File Pointer");
    return (-1);
  }
  //return OSCL_FileSeek (fp,  offset, origin);
  return 0;
}

/* ======================================================================
FUNCTION
  FileBase::IsASFFile

DESCRIPTION
  Checks if the file is WM by checking the ASF header object's GUID.

INPUT/OUTPUT
  pBuf = pointer ot the buffer containing file bytes from start
  size = buffer size, ahould be at least GUID size (16 bytes)

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  TRUE if file is ASF else FALSE

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool FileBase::IsASFFile(uint8 * pBuf, uint32 ulBufSoze)
{
  if( pBuf && (ulBufSoze >= (uint32)sizeof(asfFileIdentifier)) )
  {
    if( !memcmp(pBuf, asfFileIdentifier, sizeof(asfFileIdentifier)) )
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsASFFile

DESCRIPTION
  Checks if the file is WM by checking the file extensions
  This avoids extra FILE OPEN just to determine the type.
  File extensions that will make return TRUE from here
  .asf,.wma,.wmv

INPUT/OUTPUT
  filename identifying the content to be played.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  TRUE if file is ASF else FALSE

SIDE EFFECTS
  Detail any side effects.

 ========================================================================== */
bool FileBase::IsASFFile(FILESOURCE_STRING filename, uint8* pucDataBuf, bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if( (bUseExtension) &&
      (ZUtils::FindR( (const char*)filename.get_cstr(),".wma") != ZUtils::npos) ||
      (ZUtils::FindR( (const char*)filename.get_cstr(),".asf") != ZUtils::npos)||
      (ZUtils::FindR( (const char*)filename.get_cstr(),".wmv") != ZUtils::npos)
    )
  {
    return true;
  }
  else
#endif /*FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE  */
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if( ( NULL != pucDataBuf) &&
        ( 0 == memcmp(pucDataBuf, asfFileIdentifier, sizeof(asfFileIdentifier))))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsMPEG1

DESCRIPTION
  Returns True if the file is mpeg1 otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a mpeg1 file.
  pBuf    : mpeg1 signature worth of bytes have already been read into pBuf.
            Compare if it's matches with mpeg1 signature.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Returns True is the file is mpeg2 otherwise, returns FALSE;

SIDE EFFECTS
  None

 ========================================================================== */
bool FileBase::IsMPEG1VideoFile(FILESOURCE_STRING filename, uint8* pBuf)
{
  if((NULL != pBuf) &&
     (0 == memcmp(pBuf, MPEG2_SIG_BYTES, MPEG2_PS_SIGNATURE_BYTES)))
  {
    if((pBuf[4] & 0xf0) == 0x20)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                     " MPEG1 Video not supported for PS");
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsMPEG2

DESCRIPTION
  Returns True if the file is mpeg2 otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a mpeg2 file.
  pBuf    : mpeg2 signature worth of bytes have already been read into pBuf.
            Compare if it's matches with mpeg2 signature.
  bUseExtension: When true, QTV will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 a mpeg2 file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is mpeg2 otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsMPEG2File(FILESOURCE_STRING filename,uint8* pBuf,
                           uint32 ulBufSize, bool bUseExtension,
                           bool* pbIsProgStream)
{
  bool bRet = true;
  /* If pointer is not NULL, then reset */
  if(pbIsProgStream)
  {
    *pbIsProgStream = false;
  }
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     ((ZUtils::FindR( (const char*)filename.get_cstr(),".vob") != ZUtils::npos)||
      (ZUtils::FindR( (const char*)filename.get_cstr(),".mpg") != ZUtils::npos)))
  {
    if(pbIsProgStream)
    {
      *pbIsProgStream = true;
    }
    return true;
  }
  else if (( bUseExtension) &&
      ((ZUtils::FindR( (const char*)filename.get_cstr(),".ts") != ZUtils::npos)||
      (ZUtils::FindR( (const char*)filename.get_cstr(),".m2ts") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  if( NULL != pBuf)
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(0 == memcmp(pBuf, MPEG2_SIG_BYTES, MPEG2_PS_SIGNATURE_BYTES))
    {
      if((pBuf[4] & 0xf0) == 0x20)
      {
        bRet = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                     " MPEG1 Video is not supported");
      }
      if(pbIsProgStream)
      {
        *pbIsProgStream = true;
      }
      return bRet;
    } //! if(0 == memcmp(pBuf, MPEG2_SIG_BYTES, MPEG2_PS_SIGNATURE_BYTES))
    else
    {
      uint8  ucBufIndex   = 0xFF;
      int    nIndex       = MPEG2_TS_PKT_BYTES;
      /* If the first bytes satisfies MPEG2_TS_SIG_BYTES, we will read 4 more
         TS packets and confirm sync word occurrence. */
      if(0 == memcmp(pBuf, MPEG2_TS_SIG_BYTES, MPEG2_TS_SIGNATURE_BYTES))
      {
        ucBufIndex = 0;
      }
      //! In case of M2TS clips, first 4 bytes will contain other data
      //! 5th byte will contain TS signature byte 0x47.
      else if(0 == memcmp(pBuf+4, MPEG2_TS_SIG_BYTES, MPEG2_TS_SIGNATURE_BYTES))
      {
        //! This field used to check for successive TS Packets
        ucBufIndex = 4;
      }
      else
      {
        bRet = false;
      }
      if (true == bRet)
      {
        uint8 ucCount = 1;
        //! Check the data read
        while(nIndex < (int)ulBufSize)
        {
          //! If at least 3 TS Packets are validated, break the loop
          if (ucCount > 3)
          {
            break;
          }
          //! If 0x47 is not found
          if(memcmp(pBuf + nIndex + ucBufIndex, MPEG2_TS_SIG_BYTES,
                    MPEG2_TS_SIGNATURE_BYTES))
          {
            bRet = false;
            break;
          }
          nIndex += MPEG2_TS_PKT_BYTES;
          ucCount++;
        }//! while(nIndex < ulBufSize)
      }//! if (true == bRet)
    } //! else (Transport Stream check)
  }//! if( NULL != pBuf)
  return bRet;
}

/* ======================================================================
FUNCTION
  FileBase::IsOggFile

DESCRIPTION
  Returns True if the file is ogg container,otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a ogg file.
  pBuf    : ogg signature worth of bytes have already been read into pBuf.
            Compare if it's matches with ogg signature.
  bUseExtension: When true, we will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 an ogg file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is ogg file otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsOggFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     ((ZUtils::FindR( (const char*)filename.get_cstr(),".ogg") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(( NULL != pBuf ) &&
       ( 0 == memcmp(pBuf, OGG_SIG_BYTES, FOURCC_SIGNATURE_BYTES )))
    {
      return true;
    }
  }
  return false;
}
/* ======================================================================
FUNCTION
  FileBase::IsMKVFile

DESCRIPTION
  Returns True if the file is matroska file

INPUT/OUTPUT
  filename: File Name to determine whether it's a ogg file.
  pBuf    : ogg signature worth of bytes have already been read into pBuf.
            Compare if it's matches with ogg signature.
  bUseExtension: When true, we will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 an ogg file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is ogg file otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsMKVFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension ) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".mkv") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".webm") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(( NULL != pBuf ) &&
       ( 0 == memcmp(pBuf, MKV_SIG_BYTES, FOURCC_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsFLVFile

DESCRIPTION
  Returns True if the file is FLV file

INPUT/OUTPUT
  filename: File Name to determine whether it's a FLV file.
  pBuf    : FLV signature worth of bytes have already been read into pBuf.
            Compare if it's matches with FLV signature.
  bUseExtension: When true, we will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 an FLV file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is FLV file otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsFLVFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension ) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".flv") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(( NULL != pBuf ) &&
       ( 0 == memcmp(pBuf, FLV_SIG_BYTES, sizeof(FLV_SIG_BYTES))))
    {
      return true;
    }
  }
  return false;
}
/* ======================================================================
FUNCTION
  FileBase::IsFlacFile

DESCRIPTION
  Returns True if the file is flac container,otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a flac file.
  pBuf    : flac signature worth of bytes have already been read into pBuf.
            Compare if it's matches with flac signature.
  bUseExtension: When true, we will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 an ogg file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is ogg file otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsFlacFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension ) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".flac") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(( NULL != pBuf)&&
       ( 0 == memcmp(pBuf, FLAC_SIG_BYTES, FOURCC_SIGNATURE_BYTES)) )
    {
      return true;
    }
  }
  return false;
}
/* ======================================================================
FUNCTION
  FileBase::IsMP4_3GPFile

DESCRIPTION
  Returns True if file is 3gp/mp4 file.

INPUT/OUTPUT
  filename: File Name to determine whether it's a 3gp/mp4 file.
  pBuf    : 3gp/mp4 signature worth of bytes have already been read into pBuf from given file.
  bUseExtension: When true, we will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 3gp/mp4 file.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is 3gp/mp4 file otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsMP4_3GPFile(FILESOURCE_STRING filename,uint8* pBuf,
                             uint32 ulBufSize, bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension ) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".3gp") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".mp4") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".skm") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".3g2") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".k3g") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".amc") != ZUtils::npos)||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".mp4a") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif//#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    //! First atom should be "FTYP"
    if(( NULL != pBuf) &&
      ( 0 == memcmp(pBuf+4, MP4_3GP_ATOM_FTYP, FOURCC_SIGNATURE_BYTES)))
    {
      //! Size is stored in reverse order
      uint32 ulAtomSize = pBuf[3] | pBuf[2] <<8 | pBuf[1]<<16 | pBuf[0]<<24;
      // Skip 8 bytes ( FTYP-Size + FTYP-FOURCC)
      uint32 ulDataSkipped = FOURCC_SIGNATURE_BYTES*2;
      //! Check whether complete atom is read or not.
      //! Check for brand within "FTYP" atom range only.
      ulBufSize = FILESOURCE_MIN(ulAtomSize, ulBufSize);
      if (!memcmp(pBuf+ulDataSkipped, MP4_3GP_BRAND_DASH,
                  FOURCC_SIGNATURE_BYTES))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "'DASH' branch is found");
        return false;
      }

      //Skip Major and Minor Versions (each uint32)
      ulDataSkipped += FOURCC_SIGNATURE_BYTES *2;
      //Check compatibility brand also to double check
      while (ulBufSize > ulDataSkipped)
      {
        if (!memcmp(pBuf+ulDataSkipped, MP4_3GP_BRAND_DASH,
                    FOURCC_SIGNATURE_BYTES))
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "'DASH' branch is found");
          return false;
        }
        ulDataSkipped += FOURCC_SIGNATURE_BYTES;
      }//while-loop, scan through all compatible brands.
    }//if ( FTYP) atom found
    //! First 4bytes contain atom size and next 4bytes contain atom signature
    //! Atom signature should be one of these following atoms
    if(( NULL != pBuf) &&
       ((!memcmp(pBuf+4, MP4_3GP_ATOM_FTYP, FOURCC_SIGNATURE_BYTES))||
        (!memcmp(pBuf+4, MP4_3GP_ATOM_MOOV, FOURCC_SIGNATURE_BYTES))||
        (!memcmp(pBuf+4, MP4_3GP_ATOM_MDAT, FOURCC_SIGNATURE_BYTES))))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
      FileBase::Is3GPP2File

DESCRIPTION
      Brand identifier for this specification will be 3g2a(Rev-0),
      3g2b(Rev-A)& 3g2c(Rev-B).These identifier shall occur in compatibility
      brands list,and may also be present in major brand list. So this function
      will scan through both major & compatible brand.

INPUT/OUTPUT
  filename:      File Name to determine whether it's a 3gp/mp4 file.
  pBuf    :      3g2 signature worth of bytes have already been read into
                 pBuf from given file.
  ulBufSize:     Passed buffer size, which contain 'FTYP' atom data.
  bUseExtension: When true, we will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 3gp/mp4 file.

DEPENDENCIES
      List any dependencies for this function, global variables, state,
      resource availability, etc.

RETURN VALUE
      Returns True is the file is 3g2 file otherwise, returns FALSE;

SIDE EFFECTS
      None
========================================================================== */
bool FileBase::Is3GPP2File(FILESOURCE_STRING filename,
                           uint8* pBuf,
                           uint32 ulBufSize,
                           bool bUseExtension)
{
  bool bRetStatus = false;
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
    (( ZUtils::FindR( (const char*)filename.get_cstr(),".3g2") != ZUtils::npos)))
  {
    bRetStatus = true;
  }
  else
#endif//#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(( NULL != pBuf) &&
      ( 0 == memcmp(pBuf+4, MP4_3GP_ATOM_FTYP, FOURCC_SIGNATURE_BYTES)))
    {
      // Skip 8 bytes ( FTYP-Size + FTYP-FOURCC)
      uint32 ulDataSkipped = FOURCC_SIGNATURE_BYTES * 2;
      if (!memcmp(pBuf+ulDataSkipped, MP4_3GP_BRAND_DASH,
                  FOURCC_SIGNATURE_BYTES))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "'DASH' branch is found");
        return false;
      }
      //Check Major-Brand first offset = 8
      if (( 0 == memcmp(pBuf+ulDataSkipped, BRAND_3G2_REL_0, FOURCC_SIGNATURE_BYTES)) ||
          ( 0 == memcmp(pBuf+ulDataSkipped, BRAND_3G2_REL_A, FOURCC_SIGNATURE_BYTES)) ||
          ( 0 == memcmp(pBuf+ulDataSkipped, BRAND_3G2_REL_B, FOURCC_SIGNATURE_BYTES)) ||
          ( 0 == memcmp(pBuf+ulDataSkipped, MP4_3GP_BRAND_HEVC, FOURCC_SIGNATURE_BYTES)))
      {
        bRetStatus = true;
      }
      //Skip MinorVersion(uin32)
      ulDataSkipped += FOURCC_SIGNATURE_BYTES;
      //Check compatibility brand also to double check
      //Check maximum 6 brands, typically more than 4 brands are not available
      while (ulDataSkipped < 64)
      {
        if (( 0 == memcmp(pBuf+ulDataSkipped, BRAND_3G2_REL_0, FOURCC_SIGNATURE_BYTES)) ||
            ( 0 == memcmp(pBuf+ulDataSkipped, BRAND_3G2_REL_A, FOURCC_SIGNATURE_BYTES)) ||
            ( 0 == memcmp(pBuf+ulDataSkipped, BRAND_3G2_REL_B, FOURCC_SIGNATURE_BYTES)) ||
            ( 0 == memcmp(pBuf+ulDataSkipped, MP4_3GP_BRAND_HEVC, FOURCC_SIGNATURE_BYTES)))
        {
          bRetStatus = true;
        }
        if (!memcmp(pBuf+ulDataSkipped, MP4_3GP_BRAND_DASH,
                    FOURCC_SIGNATURE_BYTES))
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "'DASH' branch is found");
          bRetStatus = false;
          break;
        }
        ulDataSkipped += FOURCC_SIGNATURE_BYTES;
      }//while-loop, scan through all compatible brands.
    }//if ( FTYP) atom found
  }//else media-type
  return bRetStatus;
}

/* ======================================================================
FUNCTION
  FileBase::IsRMFile

DESCRIPTION
  Returns True if the file is AVI otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a divx/avi file.
  pBuf    : DIVX_AVI_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with DIVX_AVI_FileIdentifier.
  bUseExtension: When true, QTV will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 a avi file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AVI otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsRMFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".rm") != ZUtils::npos) ||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".ra") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if(( NULL != pBuf) &&
       ( 0 == memcmp(pBuf,".RMF", FOURCC_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}
/* ======================================================================
FUNCTION
  FileBase::IsAVIFile

DESCRIPTION
  Returns True if the file is AVI otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a divx/avi file.
  pBuf    : DIVX_AVI_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with DIVX_AVI_FileIdentifier.
  bUseExtension: When true, QTV will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 a avi file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AVI otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsAVIFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".divx") != ZUtils::npos) ||
      ( ZUtils::FindR( (const char*)filename.get_cstr(),".avi") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf ) &&
        ( 0 == memcmp(pBuf, "RIFF", FOURCC_SIGNATURE_BYTES)) &&
        ( 0 == memcmp(pBuf+8, "AVI ", FOURCC_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsAMRFile

DESCRIPTION
  Returns True if the file is AMR otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a amr file.
  pBuf    : AMR_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with AMR file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a amr file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AMR otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsAMRFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".amr") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf) &&
        ( 0 == memcmp(pBuf, "#!AMR\n", AMR_FILE_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsAMRWBFile

DESCRIPTION
  Returns True if the file is AMR-WB otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a amrwb file.
  pBuf    : AMRWB_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with AMRWB file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a amrwb file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AMRWB otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsAMRWBFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".awb") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf) &&
        ( 0 == memcmp(pBuf, "#!AMR-WB\n", AMR_WB_FILE_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}


/* ======================================================================
FUNCTION
  FileBase::IsEVRCWBFile

DESCRIPTION
  Returns True if the file is EVRC-WB otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a amrwb file.
  pBuf    : EVRCWB_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with EVRCWB file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a evrcwb file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is EVRCWB otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsEVRCWBFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     ((ZUtils::FindR( (const char*)filename.get_cstr(),".evw") != ZUtils::npos)))
  {
    return true;
  }
  else /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf) &&
        (0 == memcmp(pBuf, "#!EVCWB\n", EVRC_WB_FILE_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsEVRCBFile

DESCRIPTION
  Returns True if the file is EVRC-B otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a evrc-b file.
  pBuf    : EVRC_B_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with EVRC-B file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a evrcwb file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is EVRC-B otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsEVRCBFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".evb") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf)&&
        ( 0 == memcmp(pBuf, "#!EVRC-B\n", EVRC_B_FILE_SIGNATURE_BYTES)))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsMP3File

DESCRIPTION
  Returns True if the file is MP3 otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a amr file.
  pBuf    : MP3_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with MP3 file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a mp3 file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is MP3 otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */


bool FileBase::IsMP3File(FILESOURCE_STRING filename,
                         void **p_mp3file,
                         bool bUseExtension,
                         IxStream* ixstream)
{
#ifdef FEATURE_FILESOURCE_MP3
  MP3File* mp3file = NULL;
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(
     (bUseExtension) &&
     (
       (ZUtils::FindR( (const char*)filename.get_cstr(),".mp3") != ZUtils::npos)
     )
    )
   {
     mp3file = MM_New_Args( MP3File, (filename, NULL, 0) );
   }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(ixstream);
    if( (filename)&& filename.get_size() )
    {
      mp3file = MM_New_Args( MP3File, (filename, NULL, 0) );
    }
#ifdef FEATURE_FILESOURCE_DRM_DCF
    else if(ixstream)
    {
      mp3file = MM_New_Args( MP3File, (ixstream) );
    }
#endif
    if( NULL != mp3file)
    {
      if(mp3file->CheckMP3Format())
      {
        *p_mp3file = mp3file;
        return true;
      }
      else
      {
        MM_Delete(mp3file);
        return false;
      }
    }

  }
#endif
  return false;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
bool FileBase::IsMP3File(video::iStreamPort* pPort,void ** mp3file)
{
  bool bRet = false;
#ifdef FEATURE_FILESOURCE_MP3
  MP3File* ptmpmp3file = NULL;
  if( pPort && mp3file)
  {
    ptmpmp3file = MM_New_Args( MP3File, (pPort) );
  }
  if( NULL != ptmpmp3file)
  {
    if(ptmpmp3file->CheckMP3Format())
    {
      *mp3file = ptmpmp3file;
      bRet = true;
    }
    else
    {
      MM_Delete(ptmpmp3file);
    }
  }
#endif
  return bRet;
}
bool FileBase::IsAACFile(video::iStreamPort* pPort,void ** aacfile)
{
  bool bRet = false;
#ifdef FEATURE_FILESOURCE_AAC
  AACFile* ptmpaacfile = NULL;
  if( pPort && aacfile)
  {
    ptmpaacfile = MM_New_Args( AACFile, (pPort) );
  }
  if( NULL != ptmpaacfile)
  {
    if(ptmpaacfile->CheckAacFormat())
    {
      *aacfile = ptmpaacfile;
      bRet = true;
    }
    else
    {
      MM_Delete(ptmpaacfile);
    }
  }
#endif
  return bRet;
}
#endif

/* ======================================================================
FUNCTION
  FileBase::IsAACFile

DESCRIPTION
  Returns True if the file is AAC otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a amr file.
  pBuf    : AAC_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with AAC file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a aac file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AAC otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsAACFile(FILESOURCE_STRING filename,
                         void **p_aacfile,
                         bool bUseExtension,
                         IxStream* pixstream)
{
#ifdef FEATURE_FILESOURCE_AAC
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE

  if(
     (bUseExtension) &&
     (
       (ZUtils::FindR( (const char*)filename.get_cstr(),".aac") != ZUtils::npos)
     )
    )
   {
     *p_aacfile = NULL;
     AACFile* aacFile = MM_New_Args( AACFile, (filename, NULL, 0) );
     *p_aacfile = aacFile;
     return true;
   }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(pixstream);
    *p_aacfile = NULL;
    AACFile* aacFile = NULL;
#ifdef FEATURE_FILESOURCE_DRM_DCF
    if(pixstream)
    {
      aacFile = MM_New_Args( AACFile, ( pixstream) );
    }
    else
#endif
    {
      aacFile = MM_New_Args( AACFile, (filename, NULL, 0) );
    }
    if( NULL != aacFile)
    {
      if(aacFile->CheckAacFormat())
      {
        *p_aacfile = aacFile;
        return true;
      }
      MM_Delete(aacFile);
      return false;
    }
  }
#endif
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsQCPFile

DESCRIPTION
  Returns True if the file is QCP otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a qcp file.
  pBuf    : QCP_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with QCP file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a qcp file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is QCP otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsQCPFile(FILESOURCE_STRING filename,uint8* pBuf,bool bUseExtension,IxStream* ixstream)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".qcp") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(ixstream);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf)&&
        (( 0 == memcmp(pBuf, "RIFF", FOURCC_SIGNATURE_BYTES ))&&
         ( 0 == memcmp(pBuf+8, "QLCM", FOURCC_SIGNATURE_BYTES ))))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsWAVADPCMFile

DESCRIPTION
  Returns True if the file is WAVADPCM otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a wavadpcm file.
  pBuf    : WAVADPCM_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with WAVADPCM file identifier.
  bUseExtension: When true, the function will inspect the file extension rather
                 than opening and reading first few bytes to determine whether
                 it's a wavadpcm file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is WAVADPCM otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsWAVADPCMFile(FILESOURCE_STRING filename,
                              uint8* pBuf,
                              bool bUseExtension,
                              IxStream* ixstream)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if(( bUseExtension) &&
     (( ZUtils::FindR( (const char*)filename.get_cstr(),".wav") != ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(ixstream);
    UNUSED_PARAM(filename);
    if (( NULL != pBuf)&&
        (( 0 == memcmp(pBuf, "RIFF", FOURCC_SIGNATURE_BYTES))&&
         ( 0 == memcmp(pBuf+8, "WAVE", FOURCC_SIGNATURE_BYTES))))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsDTSFile

DESCRIPTION
  Returns True if the file is DTS otherwise, returns FALSE;

INPUT/OUTPUT
  pBuf    : DTS_FRAME_SYNC_MARKER bytes have already been read into pBuf.
            Compare if it's matches with DTS_FileIdentifier.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is DTS otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsDTSFile(uint8* pBuf, uint32 ulBufSize)
{
  if(pBuf)
  {
    uint32 ulIndex = 0;
    uint8  ucSyncMarker[FOURCC_SIGNATURE_BYTES];
    uint8  ucNumFrames = 0;
    memset(ucSyncMarker, 0, FOURCC_SIGNATURE_BYTES);

    /* DTS bit-stream in AV containers contains header.
       First check whether input file is complaint to any standard container
       or not. DTS codec can be part of MKV, MP2, MP4, AVI and ASF containers.
    */
    if ((IsMKVFile(NULL, pBuf, false)) ||
        (IsMP4_3GPFile(NULL, pBuf, ulBufSize, false)) ||
        (Is3GPP2File(NULL, pBuf, ulBufSize, false)) ||
        (IsMPEG2File(NULL, pBuf, ulBufSize, false)) ||
        (IsASFFile(NULL, pBuf, false)) ||
        (IsAVIFile(NULL, pBuf, false)) ||
        (IsMPEG1VideoFile(NULL,pBuf))
       )
    {
      return false;
    }
    if(IsWAVADPCMFile(NULL, pBuf, false))
    {
      ulIndex = 44;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
          " RIFF chunk found, data chunk starts at %lu", ulIndex);
    }
    while((ulIndex < ulBufSize) &&
          (ulBufSize - ulIndex) > DTS_14BIT_SYNC_MARKER_SIZE)
    {
      if ((!memcmp(pBuf + ulIndex, DTS_SYNCWORD_CORE_LE, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_LBR_LE, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_SUBSTREAM_LE, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_CORE, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_LBR, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_SUBSTREAM, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_PCM, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_SYNCWORD_PCM_LE, FOURCC_SIGNATURE_BYTES)) ||
          (!memcmp(pBuf + ulIndex, DTS_HD_FRAME_SYNC_MARKER, DTSHD_FRAME_SYNC_SIZE)))
      {
        ucNumFrames++;
        if( (!memcmp(pBuf + ulIndex, DTS_HD_FRAME_SYNC_MARKER,
                     DTSHD_FRAME_SYNC_SIZE)) ||
            ((ucNumFrames > 1) &&
             (!memcmp(pBuf + ulIndex, ucSyncMarker, FOURCC_SIGNATURE_BYTES))))
        {
          return true;
        }
        else
        {
          memcpy(ucSyncMarker, pBuf + ulIndex, FOURCC_SIGNATURE_BYTES);
        }
      }
      ulIndex += FOURCC_SIGNATURE_BYTES;
    }
#ifdef PLATFORM_LTK
    //! In Windows return true, if at least one DTS frame is found
    if (ucNumFrames)
    {
      return true;
    }
#endif
  }
  return false;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION
  FileBase::IsAVIFile

DESCRIPTION
  Returns True if the file is AVI otherwise, returns FALSE;

INPUT/OUTPUT
  pPort: Stream port to determine whether it's a divx/avi file.
  pBuf    : DIVX_AVI_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with DIVX_AVI_FileIdentifier.
  bUseExtension: N/A.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AVI otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsAVIFile(video::iStreamPort* pPort,uint8* pBuf,bool bUseExtension)
{
  UNUSED_PARAM(bUseExtension);
  UNUSED_PARAM(pPort);
  if(pBuf)
  {
    if ( (!memcmp(pBuf, "RIFF", FOURCC_SIGNATURE_BYTES)) &&
         (!memcmp(pBuf+8, "AVI ", FOURCC_SIGNATURE_BYTES)) )
    {
       return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsRawH265File

DESCRIPTION
  Returns True if the file is QRAW-H265 otherwise, returns FALSE;

INPUT/OUTPUT
  pPort: Stream port to determine whether it's a QRAW-H265 file.
  pBuf    : QRAW_SIGNATURE_BYTES bytes have already been read into pRawBuf.
            Compare if it's matches with QRAW-H265 file signature.
  bUseExtension: N/A.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is QRAW-H265 otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsRawH265File(video::iStreamPort* pPort,
                             uint8* pRawH265Buf,
                             bool bUseExtension)
{
  UNUSED_PARAM(bUseExtension);
  UNUSED_PARAM(pPort);
  if(pRawH265Buf)
  {
    if ( (!memcmp(pRawH265Buf, "QRAW", FOURCC_SIGNATURE_BYTES)) &&
         (!memcmp(pRawH265Buf+8, "H265", FOURCC_SIGNATURE_BYTES)) )
    {
       return true;
    }
  }
  return false;
}
#endif

/* ======================================================================
FUNCTION
  FileBase::IsRawH265File

DESCRIPTION
  Returns True if the file is QRAW-H265 otherwise, returns FALSE;

INPUT/OUTPUT
  filename: File Name to determine whether it's a QRAW-H265 file.
  pBuf    : QRAW_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
            Compare if it's matches with QRAW-H265 file signature.
  bUseExtension: When true, It will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 a QRAW-H265 file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is QRAW-H265 otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsRawH265File(FILESOURCE_STRING filename,
                             uint8* pRawH265Buf,
                             bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if((bUseExtension) &&
    ((ZUtils::FindR((const char*)filename.get_cstr(),".h265")!= ZUtils::npos)))
  {
    return true;
  }
  else
#endif /*_FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE_*/
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (( NULL != pRawH265Buf) &&
        (( 0 == memcmp(pRawH265Buf, "QRAW", FOURCC_SIGNATURE_BYTES)) &&
         ( 0 == memcmp(pRawH265Buf+8, "H265", FOURCC_SIGNATURE_BYTES))))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsAC3File

DESCRIPTION
  Returns True if the file is AC3/EAC3 otherwise, returns FALSE;

INPUT/OUTPUT
  filename:      File Name to determine whether it's a AC3/EAC3 file.
  pBuf    :      AC3_FILE_SIGNATURE_BYTES bytes have already been read into pBuf.
                 Compare if it's matches with AC3/EAC3 file signature.
  bUseExtension: When true, It will inspect the file extension rather than
                 opening and reading first few bytes to determine whether it's
                 a AC3/EAC3 file.


DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is the file is AC3/EAC3 otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
bool FileBase::IsAC3File(FILESOURCE_STRING filename,
                         uint8* pAC3Buf,
                         bool bUseExtension)
{
#ifdef FEATURE_FILESOURCE_USE_FILE_EXTENSION_FOR_MEDIA_TYPE
  if((bUseExtension) &&
     ((ZUtils::FindR((const char*)filename.get_cstr(),".ac3")!= ZUtils::npos))||
     ((ZUtils::FindR((const char*)filename.get_cstr(),".eac3")!= ZUtils::npos)))
  {
    return true;
  }
  else
#endif
  {
    UNUSED_PARAM(bUseExtension);
    UNUSED_PARAM(filename);
    if (AUDIO_FMT_IS_AC3_SYNC(pAC3Buf))
    {
      return true;
    }
  }
  return false;
}

/* ======================================================================
FUNCTION
  FileBase::IsID3TagPresent

DESCRIPTION
  Returns True if the file start with ID3 tag otherwise, returns FALSE;

INPUT/OUTPUT
  pucDataBuf[in]    : Data buffer to compare ID3 tag
  pulID3TagLen[in/out]: Return ID3 tag size if found any.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
 Returns True is ID3 tag found otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */

bool FileBase::IsID3TagPresent( uint8* pucDataBuf, uint32* pulID3TagLen )
{
  //Check if file has ID3Tag present before AAC/MP3 file header.
  bool bRetStatus = false;
  uint8* pProcessBuff = pucDataBuf;
  if(( NULL != pProcessBuff ) && ( NULL != pulID3TagLen ) &&
     ( 0 == memcmp("ID3",pProcessBuff,strlen("ID3"))))
  {
    uint32 ulID3TagSize = 0;
    uint32 ulIndex;
    // Find ID3 tag size: skip 6 byte = ID3(3byte)+ Ver(2byte)+ Flag (1byte)
    pProcessBuff +=6;
    for (ulIndex =0; ulIndex < 4; ulIndex++)
    {
      ulID3TagSize = (ulID3TagSize << 7) | (*(pProcessBuff)++ & 0x7F);
    }
    *pulID3TagLen = ulID3TagSize;
    bRetStatus = true;
  }
  return bRetStatus;
}

/* ======================================================================
FUNCTION
  FileBase::IsAACFormat

DESCRIPTION
  Returns True if AAC format detacted otherwise, returns FALSE;

INPUT/OUTPUT
  pucDataBuf[in]    : Data buffer to validate AAC frame header
  pulFrameLen       : Pointer to store frameLen value

  DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Returns True is AAC format found otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
PARSER_ERRORTYPE FileBase::IsAACFormat (uint8 *pucFrameBuff,
                                        uint32 ulFrameBufLen)
{
  PARSER_ERRORTYPE RetStatus = PARSER_ErrorDefault;

  if(NULL == pucFrameBuff)
  {
    return RetStatus;
  }
  uint16 usFrameSync = (uint16)((pucFrameBuff[1] << 8) + pucFrameBuff[0]);

  if(( NULL != pucFrameBuff) &&
    ( 0 == memcmp("ADIF", pucFrameBuff,strlen("ADIF"))))
  {
    RetStatus = PARSER_ErrorNone;
  }
  else if((NULL != pucFrameBuff &&
           0 == memcmp(ucLATAMMask, pucFrameBuff,sizeof(ucLATAMMask))))
  {
    RetStatus = PARSER_ErrorNone;
  }
  else if((LOAS_HEADER_MASK_RESULT == (usFrameSync & LOAS_HEADER_MASK)))
  {
    RetStatus = ParseLOASFrameHeader(pucFrameBuff, ulFrameBufLen);
  }
  else if( ADTS_HEADER_MASK_RESULT == (usFrameSync & ADTS_HEADER_MASK))
  {
    RetStatus = ParseADTSFrameHeader(pucFrameBuff, ulFrameBufLen);
  }
  return (RetStatus);
}

/* ======================================================================
FUNCTION
  FileBase::IsMP3Format

DESCRIPTION
  Returns True if MP3 Syncword found otherwise, returns FALSE;

INPUT/OUTPUT
  pucDataBuf[in]    : Data buffer to validate MP3 sync word

  DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Returns True is MP3 format found otherwise, returns FALSE;

SIDE EFFECTS
  None
========================================================================== */
PARSER_ERRORTYPE FileBase::IsMP3Format (uint8* pucFrameBuff,
                                        uint32  ulFrameBufLen )
{
  PARSER_ERRORTYPE RetStatus = PARSER_ErrorInHeaderParsing;
  if (NULL != pucFrameBuff)
  {
    if ((IsMKVFile(NULL, pucFrameBuff, false)) ||
        (IsMP4_3GPFile(NULL, pucFrameBuff, ulFrameBufLen, false)) ||
        (Is3GPP2File(NULL, pucFrameBuff, ulFrameBufLen, false)) ||
        (IsMPEG2File(NULL, pucFrameBuff, ulFrameBufLen, false)) ||
        (IsASFFile(NULL, pucFrameBuff, false)) ||
        (IsAVIFile(NULL, pucFrameBuff, false)) ||
        (IsMPEG1VideoFile(NULL,pucFrameBuff))
        )
    {
      return RetStatus;
    }
    RetStatus = ParseMP3FrameHeader(pucFrameBuff, ulFrameBufLen);
  }
  return RetStatus;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
/* ======================================================================
FUNCTION
  FileBase::initFormatParser

DESCRIPTION
  Static method to read in a media file from disk and return the FileBase interface

INPUT/OUTPUT
  pPort      : iStreamPort interface
  bPlayVideo : Indicates if this is video instance
  bPlayAudio : Indicates if this is audio instance
  bPlayText  : Indicates if this is text instance
  blookforcodechdr : Indicates if we need to find codec header during parsing
  format : Media File Format

  DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Returns FileBase interface or NULL;

SIDE EFFECTS
  None
========================================================================== */
FileBase* FileBase::initFormatParser ( video::iStreamPort* pPort,
                                       bool bPlayVideo,
                                       bool bPlayAudio,
                                       bool bPlayText,
                                       bool blookforcodechdr,
                                       FileSourceFileFormat format )
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_DEBUG, "initFormatParser");
  if(pPort)
  {
#ifdef FEATURE_FILESOURCE_RAW_PARSER
    if(FILE_SOURCE_RAW == format)
    {
      RawFile* raw_file = MM_New_Args( RawFile, (pPort) );
      return raw_file;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
    if(FILE_SOURCE_ASF == format)
    {
      ASFFile* asf = MM_New_Args( ASFFile, (pPort, NULL,0,0,bPlayVideo,bPlayAudio) );
      return asf;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_RAW_H265_PARSER
    if(FILE_SOURCE_RAW_H265 == format)
    {
      CRawH265File* hRawH265File = MM_New_Args( CRawH265File,
                                              (pPort,FILE_SOURCE_RAW_H265));
      return hRawH265File;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_AVI
    if(FILE_SOURCE_AVI == format)
    {
      AVIFile* aviFile = MM_New_Args( AVIFile, (pPort, bPlayVideo, bPlayAudio) );
      return aviFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_AMR
    if(FILE_SOURCE_AMR_NB == format)
    {
      AMRFile* amrFile = MM_New_Args( AMRFile, (pPort) );
      return amrFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_AMRWB
    if(FILE_SOURCE_AMR_WB == format)
    {
      AMRWBFile* amrwbFile = MM_New_Args( AMRWBFile, (pPort) );
      return amrwbFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_EVRC_WB
    if(FILE_SOURCE_EVRC_WB == format)
    {
      EVRCWBFile* evrcwbFile = MM_New_Args( EVRCWBFile, (pPort) );
      return evrcwbFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_EVRC_B
    if(FILE_SOURCE_EVRCB == format)
    {
      EVRCBFile* evrcbFile = MM_New_Args( EVRCBFile, (pPort) );
      return evrcbFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_DTS
    if(FILE_SOURCE_DTS == format)
    {
      cDTSFile* dtsFile = MM_New_Args( cDTSFile, (pPort) );
      return dtsFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_QCP
    if(FILE_SOURCE_QCP == format)
    {
      QCPFile* qcpFile = MM_New_Args( QCPFile, (pPort) );
      return qcpFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_WAVADPCM
    if(FILE_SOURCE_WAV == format)
    {
      WAVFile* wavadpcmFile = MM_New_Args( WAVFile, (pPort) );
      return wavadpcmFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_MPEG2_PARSER
    if((FILE_SOURCE_DASH_MP2TS == format )||
        (FILE_SOURCE_WFD_MP2TS == format ) ||
        (FILE_SOURCE_MP2TS == format) ||
        (FILE_SOURCE_MP2PS == format))
    {
      MP2Stream* mpeg2File = MM_New_Args( MP2Stream, (pPort,
            blookforcodechdr,bPlayVideo, bPlayAudio,format));
      return mpeg2File;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_OGG_PARSER
    if(FILE_SOURCE_OGG == format)
    {
      OGGStream* oggFile = MM_New_Args( OGGStream, (pPort, bPlayVideo, bPlayAudio) );
      return oggFile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
    if(FILE_SOURCE_FLAC == format)
    {
      flacfile* flcfile = MM_New_Args( flacfile,(pPort) );
      return flcfile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_3GP_PARSER
    if((FILE_SOURCE_MPEG4 == format) || (FILE_SOURCE_MP4_DASH == format) ||
       (FILE_SOURCE_3G2 == format))
    {
      Mpeg4File *mp4 = NULL;
      mp4 = MM_New_Args( Mp4FragmentFile, (pPort, bPlayVideo, bPlayAudio,
                                           bPlayText, format) );
      if (mp4)
      {
        mp4->parseFirstFragment();
        if ( mp4->FileSuccess() )
        {
          return mp4;
        }
        else
        {
          MM_Delete( mp4 );
          return NULL;
        }
      }
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_MKV_PARSER
    if(FILE_SOURCE_MKV == format)
    {
      MKAVFile* mkfile = NULL;
      mkfile = MM_New_Args(MKAVFile,(pPort,bPlayVideo,bPlayAudio));
      return mkfile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_FLV_PARSER
      if(FILE_SOURCE_FLV == format)
      {
        FLVFile* pFLVfile = NULL;
        pFLVfile = MM_New_Args(FLVFile,(pPort,bPlayVideo,bPlayAudio));
        return pFLVfile;
      }
      else
#endif
#ifdef FEATURE_FILESOURCE_AC3
    if(FILE_SOURCE_AC3 == format)
    {
      AC3File* ac3File = MM_New_Args( AC3File, (pPort) );
      return ac3File;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_AAC
    if(FILE_SOURCE_AAC == format)
    {
      AACFile* ptmpaacfile = NULL;
      ptmpaacfile = MM_New_Args( AACFile, (pPort) );
      return ptmpaacfile;
    }
    else
#endif
#ifdef FEATURE_FILESOURCE_MP3
    if(FILE_SOURCE_MP3 == format)
    {
      MP3File* ptmpmp3file = NULL;
      ptmpmp3file = MM_New_Args( MP3File, (pPort) );
      return ptmpmp3file;
    }
#endif
    return NULL;
  }
  return NULL;
}
#endif
