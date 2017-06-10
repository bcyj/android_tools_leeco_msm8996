#ifndef H264HEADERPARSER_H
#define H264HEADERPARSER_H
/************************************************************************* */
/**
 * @file
 * @brief defines API's and types for the parsing of the meta data for video 
 * formats like H264 and MPEG4
 * 
 * Copyright 2011, 2015 Qualcomm Technologies, Inc., All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/H264HeaderParser.h#7 $
$DateTime: 2011/06/30 00:28:03 $
$Change: 1816267 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "parserdatadef.h"
#include "AEEStdDef.h"
//#include "qtv_msg.h"

const uint32 MPEG4_VOL_HEADER_START_CODE  = 0x00000120;

/* This structure contains Bit-Buffer details. */
typedef struct {
  uint64 ullBitBufData;   //! 64 bit worth of data read from input buf
  uint32 ulNumRemainBits; //! Number of bits available in 64bit param
  uint8 *pBsCur;          //! Buffer current position value
  uint8 *pBsEnd;          //! Buffer end position
} sBitBuffer;

/* =======================================================================
**                        Class & Function Definations
** ======================================================================= */

 /* ======================================================================
  FUNCTION:
  GetDimensionsFromVOLHeader

  DESCRIPTION:
  Returns the height and width of video if VOL header is found.

  INPUT/OUTPUT PARAMETERS:
  pucBuf      -->   Input Buffer.
  ulBufSize   -->   Input Buffer Size.
  pslWidth    -->   Pointer for width.
  pslHeight    -->   Pointer for height.

  RETURN VALUE:
  false if VOL header is not found, else true.

  SIDE EFFECTS:
  None.
  ======================================================================*/
  bool GetDimensionsFromVOLHeader(uint8* pucBuf, uint32 ulBufSize,
                                  int32* pslWidth, int32* pslHeight);

class H264HeaderParser
{
  /* -----------------------------------------------------------------------
  ** Type Definations
  ** ----------------------------------------------------------------------- */

  // This type is used when parsing an H.264 bitstream to collect H.264 NAL
  // units that need to go in the meta data.
  struct H264ParamNalu {
      int picSetID;
      int seqSetID;
      uint32 picOrderCntType;
      bool frameMbsOnlyFlag;
      bool picOrderPresentFlag;
      uint32 picWidthInMbsMinus1;
      uint32 picHeightInMapUnitsMinus1;
      uint32 log2MaxFrameNumMinus4;
      uint32 log2MaxPicOrderCntLsbMinus4;
      bool deltaPicOrderAlwaysZeroFlag;
      uint32 naluSize;
      uint8* nalu;
  };
  
  static const int MAX_SETS = 5;
  // This structure contains persistent information about an H.264 stream as it
  // is parsed.
  struct H264StreamInfo {
      int m_index;
      H264ParamNalu m_pic[MAX_SETS];
      H264ParamNalu m_seq[MAX_SETS];
  };

public:
  /* constructor */
  H264HeaderParser();

  /* destructor */
  ~H264HeaderParser();

  /* get the video dimensions */
  void GetVideoDimensions( uint16 &height, uint16 &width );

  /* get the video dimensions */
  void GetVideoDimensions( uint32 &height, uint32 &width );

  /* parses the picture and sequnce parameter sets */
  void parseParameterSet(const unsigned char *encodedBytes,
                         int totalBytes);

private:
  H264StreamInfo m_streamInfo;

};
#endif // H264HEADERPARSER_H
