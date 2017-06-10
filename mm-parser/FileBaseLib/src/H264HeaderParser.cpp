/************************************************************************* */
/**
 * @file
 * @brief implementation for the parsing of the meta data for video formats
 * like H264 and MPEG4
 *
 * Copyright 2011-2012, 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/H264HeaderParser.cpp#7 $
$DateTime: 2013/09/12 23:01:33 $
$Change: 4425557 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Streamer library files */
#include "H264HeaderParser.h"
//#include "IPStreamSourceUtils.h"
#include "parserdatadef.h"
#include "MMDebugMsg.h"
#include "MMMalloc.h"
#include <string.h>

/* =======================================================================
**                         DATA DEFINATIONS
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

/* ==========================================================================
@brief      Function to get data equivalent to required bytes.

@details    This function reads data from pointer and converts to uint32 value.

@param[in]  nBytes          Number of bytes needed
@param[in]  pucDataBuf      Buffer pointer

@return     Value if successful, otherwise returns ZERO.
@note       None.
========================================================================== */
uint32 getByteValue(int nBytes, uint8* pucDataBuf)
{
  uint32 ulValue = 0;
  int itr = 0;
  while(pucDataBuf && (nBytes > 0) && (nBytes <= (int)4) )
  {
    ulValue = ulValue <<8;
    ulValue += pucDataBuf[itr++];
    nBytes--;
  }
  return ulValue;
}

/* ======================================================================
FUNCTION
  ReadBits

DESCRIPTION
  Generic function to extract specific number of bits from input buffer

INPUT PARAMETERS:
->pBitBuf:          Bit buffer structure pointer
->ulNumBitstoRead:  Number of bits required

DEPENDENCIES
  None.

RETURN VALUE
  Value of bits extracted

SIDE EFFECTS
  None.

========================================================================== */
uint32 ReadBits(sBitBuffer* pBitBuf, uint32 ulNumBitstoRead)
{
  uint32 ulOPData = 0;
  if(ulNumBitstoRead >= 32)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
      "ReadBits, requested more than 32 bits %u",ulNumBitstoRead);
    return 0;
  }
  else if (pBitBuf->pBsCur >= pBitBuf->pBsEnd)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
      "ReadBits, Buffer exceeds the upper limit, start %p, end %p",
      pBitBuf->pBsCur, pBitBuf->pBsEnd);
    return 0;
  }

  //! If number of bits required to read is more than available in temp
  if (ulNumBitstoRead > pBitBuf->ulNumRemainBits) {
    if (pBitBuf->pBsCur+3 < pBitBuf->pBsEnd) {
      ulOPData = ((uint32)pBitBuf->pBsCur[0]<<24)|
              ((uint32)pBitBuf->pBsCur[1]<<16)|
              ((uint32)pBitBuf->pBsCur[2]<< 8)|
              ((uint32)pBitBuf->pBsCur[3]    );

      pBitBuf->pBsCur += 4;

      pBitBuf->ullBitBufData |= (uint64)ulOPData<<(32-pBitBuf->ulNumRemainBits);
      pBitBuf->ulNumRemainBits += 32;
    }
    else {
      while (pBitBuf->pBsCur < pBitBuf->pBsEnd) {
        ulOPData = (uint32)pBitBuf->pBsCur[0];

        pBitBuf->pBsCur++;
        pBitBuf->ullBitBufData |= (uint64)ulOPData<<
                                  (24 + 32 - pBitBuf->ulNumRemainBits);
        pBitBuf->ulNumRemainBits += 8;
      }
    }
  }
  //! Get the required number of bits from the 64 bit value
  ulOPData = (uint32)(pBitBuf->ullBitBufData>> (64-ulNumBitstoRead));
  //! Skip the number of bits read from the 64bit value
  pBitBuf->ullBitBufData <<= ulNumBitstoRead;
  pBitBuf->ulNumRemainBits -= ulNumBitstoRead;

  return ulOPData;
}

/******************************************************************************
 ** This class is used to convert an H.264 NALU (network abstraction layer
 ** unit) into RBSP (raw byte sequence payload) and extract bits from it.
 *****************************************************************************/
class H264HeaderRbspParser
{
public:
  H264HeaderRbspParser (const uint8 *begin, const uint8 *end);

  virtual ~H264HeaderRbspParser ();

  uint32 next ();
  void advance ();
  uint32 u (uint32 n);
  uint32 ue ();
  int32 se ();

private:
  const uint8 *begin, *end;
  int32 pos;
  uint32 bit;
  uint32 cursor;
  bool advanceNeeded;

};


H264HeaderRbspParser::H264HeaderRbspParser (const uint8 *_begin, const uint8 *_end)
: begin (_begin), end(_end), pos (- 1), bit (0),
cursor (0xFFFFFF), advanceNeeded (true)
{
}


// Destructor
/*lint -e{1540}  Pointer member neither freed nor zeroed by destructor
 * No problem
 */
H264HeaderRbspParser::~H264HeaderRbspParser () {}

// Return next RBSP byte as a word
uint32 H264HeaderRbspParser::next ()
{
  if (advanceNeeded) advance ();
  //return static_cast<uint32> (*pos);
  return static_cast<uint32> (begin[pos]);
}

// Advance RBSP decoder to next byte
void H264HeaderRbspParser::advance ()
{
  ++pos;

  //if (pos >= stop)
  if (begin + pos == end)
  {
    return;
  }
  cursor <<= 8;
  //cursor |= static_cast<uint32> (*pos);
  cursor |= static_cast<uint32> (begin[pos]);
  if ((cursor & 0xFFFFFF) == 0x000003)
  {
    advance ();
  }
  advanceNeeded = false;
}

// Decode unsigned integer
uint32 H264HeaderRbspParser::u (uint32 n)
{
  uint32 i, s, x = 0;
  for (i = 0; i < n; i += s)
  {
    s = static_cast<uint32>FILESOURCE_MIN(static_cast<int>(8 - bit),
                                   static_cast<int>(n - i));
    x <<= s;

    x |= ((next () >> ((8 - static_cast<uint32>(bit)) - s)) &
          ((1 << s) - 1));

    bit = (bit + s) % 8;
    if (!bit)
    {
      advanceNeeded = true;
    }
  }
  return x;
}

// Decode unsigned integer Exp-Golomb-coded syntax element
uint32 H264HeaderRbspParser::ue ()
{
  int leadingZeroBits = -1;
  for (uint32 b = 0; !b; ++leadingZeroBits)
  {
    b = u (1);
  }
  return((1 << leadingZeroBits) - 1) + u (static_cast<uint32>(leadingZeroBits));
}

// Decode signed integer Exp-Golomb-coded syntax element
int32 H264HeaderRbspParser::se ()
{
  const uint32 x = ue ();
  if (!x) return 0;
  else if (x & 1) return static_cast<int32> ((x >> 1) + 1);
  else return - static_cast<int32> (x >> 1);
}

H264HeaderParser::H264HeaderParser()
{
  m_streamInfo.m_index = -1;
  for (int i=0; i<MAX_SETS; i++)
  {
    memset(&m_streamInfo.m_seq[i], 0x00, sizeof(m_streamInfo.m_seq[i]));
    memset(&m_streamInfo.m_pic[i], 0x00, sizeof(m_streamInfo.m_pic[i]));
    m_streamInfo.m_seq[i].seqSetID = -1;
    m_streamInfo.m_seq[i].picSetID = -1;
    m_streamInfo.m_pic[i].picSetID = -1;
    m_streamInfo.m_pic[i].seqSetID = -1;
  }
}

H264HeaderParser::~H264HeaderParser()
{
  for (int i=0; i<MAX_SETS; i++)
  {
    if (m_streamInfo.m_seq[i].nalu)
    {
      MM_Free(m_streamInfo.m_seq[i].nalu);
      m_streamInfo.m_seq[i].nalu = NULL;
    }
    if (m_streamInfo.m_pic[i].nalu)
    {
      MM_Free(m_streamInfo.m_pic[i].nalu);
      m_streamInfo.m_pic[i].nalu = NULL;
    }
  }
}

/*
 * @brief Parse the H264 parameter sets and populates the H264StreamInfo
 *
 * param[in] encodedBytes, the parameter set
 * param[in] totalBytes, the length of the encoded bytes
 */
void H264HeaderParser::parseParameterSet(const unsigned char *encodedBytes,
                                         int totalBytes)
{
  int i = 0;
  // Determine NALU type.
  uint8 naluType = (encodedBytes [0] & 0x1F);

  // Process sequence and parameter set NALUs specially.
  if ((naluType == 7) || (naluType == 8))
  {
    // Parse parameter set ID and other stream information.
    H264ParamNalu newParam;
    //initialize newParam
    memset(&newParam,0,sizeof(newParam));
    H264HeaderRbspParser rbsp (&encodedBytes [1],
                     &encodedBytes [totalBytes]);
    uint32 id;
    //if nalu type SPS
    if (naluType == 7)
    {
      uint8 profile_idc;
      uint8 chroma_idc = 1;
      uint8 tmp;
      //profile_idc[u(8)]
      profile_idc = (uint8)rbsp.u(8);

      (void) rbsp.u (16);
      id = rbsp.ue ();
      newParam.seqSetID = (int)id;

      if(100 == profile_idc || 110 == profile_idc ||
         122 == profile_idc || 244 == profile_idc ||
          83 == profile_idc ||  86 == profile_idc )
      {
        //chroma_format_idc[ue(v)]
        chroma_idc = (uint8)rbsp.ue();
        if(3 == chroma_idc) rbsp.u(1);
        //skip bit_depth_luma_minus8/chroma_minus8
        (void) rbsp.ue();
        (void) rbsp.ue();
        //qpprime_y_zero_transform_bypass_flag[u(1)]
        tmp = (uint8)rbsp.u(1);
        //seq_scaling_matrix_present_flag[u(1)]
        tmp = (uint8)rbsp.u(1);
        if(1 == tmp)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "seq_scaling_matrix_present_flag present!!");
          uint32 ulSizeOfScalingList = 0;

          for(int idx = 0; idx < ((3 != chroma_idc) ? 8: 12); idx++)
          {
            bool seq_scaling_list_present_flag = rbsp.u(1)?true:false;
            if( seq_scaling_list_present_flag)
            {
              ulSizeOfScalingList = ( idx < 6)? 16 : 64;
              uint32 ulLastScale = 8, ulNextScale = 8, ulDeltaScale = 0;
              for(int idx1 = 0; idx1<(int)ulSizeOfScalingList; idx1++ )
              {
                if(ulNextScale !=0 )
                {
                  ulDeltaScale = rbsp.se();
                  ulNextScale = (ulLastScale + ulDeltaScale + 256)%256;
                }
                ulLastScale = (ulNextScale == 0)? ulLastScale : ulNextScale;
              }
            }//if(seq_scaling_list_present_flag)
          }//for(idx)
        }
      }
      //log2_max_frame_num_minus4(ue(v))
      newParam.log2MaxFrameNumMinus4 = rbsp.ue ();
      //pic_order_cnt_type(ue(v))
      newParam.picOrderCntType = rbsp.ue ();
      if (newParam.picOrderCntType == 0)
      {
        //log2_max_pic_order_cnt_lsb_minus4(ue(v))
        newParam.log2MaxPicOrderCntLsbMinus4 = rbsp.ue ();
      }
      else if (newParam.picOrderCntType == 1)
      {
        //delta_pic_order_always_zero_flag(u(1))
        newParam.deltaPicOrderAlwaysZeroFlag = (rbsp.u (1) == 1);
        //offset_for_non_ref_pic(se(v))
        (void) rbsp.se ();
        //offset_for_top_to_bottom_field(se(v))
        (void) rbsp.se ();
        //num_ref_frames_in_pic_order_cnt_cycle[ue(v)]
        const uint32 numRefFramesInPicOrderCntCycle = rbsp.ue ();
        for (uint32 i = 0; i < numRefFramesInPicOrderCntCycle; ++i)
        {
          //offset_for_ref_frame[se(v)]
          (void) rbsp.se ();
        }
      }
      //num_ref_frames[ue(v)]
      (void) rbsp.ue ();
      //gaps_in_frame_num_value_allowed_flag[u(1)]
      (void) rbsp.u (1);
      //pic_width_in_mbs_minus1[ue(v)]
      newParam.picWidthInMbsMinus1 = rbsp.ue ();
      //pic_height_in_map_units_minus1[ue(v)]
      newParam.picHeightInMapUnitsMinus1 = rbsp.ue ();
      //frame_mbs_only_flag[u(1)]
      newParam.frameMbsOnlyFlag = (rbsp.u (1) == 1);
    }
    else
    {
      id = rbsp.ue ();
      newParam.picSetID = (int)id;
      newParam.seqSetID = (int)rbsp.ue ();
      (void) rbsp.u (1);
      newParam.picOrderPresentFlag = (rbsp.u (1) == 1);
    }

    H264ParamNalu *naluSet
      = ((naluType == 7) ? m_streamInfo.m_seq : m_streamInfo.m_pic);

    // We currently don't support updating existing parameter sets.
    for (i=0; i<MAX_SETS; i++)
    {
      if (naluType == 7)
      {
        if ((uint32)naluSet[i].seqSetID == id)
        {
          break;
        }
      }
      else
      {
        if ((uint32)naluSet[i].picSetID == id)
        {
          break;
        }
      }
    }

    if (i != MAX_SETS)
    {
      const int tempSize = (int)naluSet[i].naluSize;
      if ((totalBytes != tempSize)
          || (0 != memcmp (&encodedBytes[0],
                               &naluSet[i].nalu[0],
                               totalBytes)))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                   "parseH264Frame::H.264 stream contains two"
                   " or more parameter set NALUs having the "
                   "same ID -- this requires either a separate "
                   "parameter set ES or multiple sample "
                   "description atoms, neither of which is "
                   "currently supported!");
      }
    }
    // Otherwise, add NALU to appropriate NALU set.
    else
    {
      if (m_streamInfo.m_index < 0)
      {
        m_streamInfo.m_index = 0;
      }

      if ((m_streamInfo.m_index >= 0) &&
          (m_streamInfo.m_index < MAX_SETS))
      {
        naluSet[m_streamInfo.m_index] = newParam;
        naluSet[m_streamInfo.m_index].nalu =
                         (uint8*)MM_Malloc(totalBytes+1);
        if(naluSet[m_streamInfo.m_index].nalu)
        {
          memcpy(naluSet[m_streamInfo.m_index].nalu,
                 encodedBytes, totalBytes);
        }
      }
      m_streamInfo.m_index++;
    }
  }
}

/*
 * @brief Gets the video dimensions, from the accumulated stream info
 *
 * @param[in] height of the clip
 * @param[in] width of the clip
 */
void H264HeaderParser::GetVideoDimensions( uint16 &height, uint16 &width )
{
  // Store a copy of the first sequence parameter set.
  H264ParamNalu &seq = m_streamInfo.m_seq[0];
  width = 0;
  height = 0;

  if ((m_streamInfo.m_index >= 0) && (m_streamInfo.m_index < MAX_SETS))
  {
    width = (uint16)(16 * (seq.picWidthInMbsMinus1 + 1));
    height = (uint16)(16 * (seq.picHeightInMapUnitsMinus1 + 1));
  }
}

/*
 * @brief Gets the video dimensions, from the accumulated stream info
 *
 * @param[in] height of the clip
 * @param[in] width of the clip
 */
void H264HeaderParser::GetVideoDimensions( uint32 &height, uint32 &width )
{
  // Store a copy of the first sequence parameter set.
  H264ParamNalu &seq = m_streamInfo.m_seq[0];
  width = 0;
  height = 0;

  if ((m_streamInfo.m_index >= 0) && (m_streamInfo.m_index < MAX_SETS))
  {
    width = 16  * (seq.picWidthInMbsMinus1 + 1);
    height = 16 * (seq.picHeightInMapUnitsMinus1 + 1);
  }
}

/* ======================================================================
FUNCTION:
  SearchForVOLHeaderStart

DESCRIPTION:
  Returns the index from where VOL header started.

INPUT/OUTPUT PARAMETERS:
  pucBuf      -->   Input Buffer.
  ulBufSize   -->   Input Buffer Size.

RETURN VALUE:
 ZERO if VOL header is not found, else index of VOL header

SIDE EFFECTS:
  None.
======================================================================*/
uint32 SearchForVOLHeaderStart(uint8* pucBuf, uint32 ulBufSize)
{
  uint32 nBufIndex = 0;
  const uint32 MPEG4_VOL_START_CODE_MASK  = 0xFFFFFFF0;
  //! Search for VOL header sync marker offset
  for (int nIndex = 0; nIndex < (int)ulBufSize; nIndex++)
  {
    uint32 ulData = getByteValue(4, pucBuf + nIndex);
    //! VOL Heade start code can be between 0x20 to 0x2F.
    //! Mask it always to check against 0x20
    if (MPEG4_VOL_HEADER_START_CODE == (ulData & MPEG4_VOL_START_CODE_MASK))
    {
      nBufIndex = nIndex;
      break;
    }
  }
  return nBufIndex;
}

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
                                int32* pslWidth, int32* pslHeight)
{
  uint32 ulValue = 0;
  sBitBuffer BitBuf;
  memset(&BitBuf, 0, sizeof(sBitBuffer));
  uint32 nIndex = SearchForVOLHeaderStart(pucBuf, ulBufSize);
  if (!nIndex) //! Return false, if VOL header is not found
  {
    return false;
  }
  BitBuf.pBsCur = pucBuf + 4 + nIndex;
  BitBuf.pBsEnd = pucBuf + ulBufSize;

  ReadBits(&BitBuf, 1); // random accessible vol
  uint8 VOLTypeIndication = (uint8)ReadBits(&BitBuf, 8);
  if (ReadBits(&BitBuf, 1)) // object layer identifier flag
  {
    uint8 VOLVerID = (uint8)ReadBits(&BitBuf, 4);
    uint8 VOLPriority = (uint8)ReadBits(&BitBuf, 3);
  }
  uint8 ucAsPectRatio = (uint8)ReadBits(&BitBuf, 4);
  if (ucAsPectRatio == 0x0f /* extended PAR */)
  {
    ReadBits(&BitBuf, 8);  // par width
    ReadBits(&BitBuf, 8);  // par height
  }
  if (ReadBits(&BitBuf, 1))   // vol control param flag
  {
    ulValue = ReadBits(&BitBuf, 2);  // chroma format value
    ulValue = ReadBits(&BitBuf, 1);  // low delay value
    if (ReadBits(&BitBuf, 1))  // vbv param flag
    {
      ulValue = ReadBits(&BitBuf, 15); // first half bit rate
      ReadBits(&BitBuf, 1);  // marker bit
      ulValue = ReadBits(&BitBuf, 15); // latter half bit rate
      ReadBits(&BitBuf, 1);  // marker bit
      ulValue = ReadBits(&BitBuf, 15); // first half vbv buf size
      ReadBits(&BitBuf, 1);  // marker bit
      ulValue = ReadBits(&BitBuf, 3);  // latter half vbv buf size
      ulValue = ReadBits(&BitBuf, 11); // first half vbv occupancy
      ReadBits(&BitBuf, 1);  // marker bit
      ulValue = ReadBits(&BitBuf, 15); // latter half vbv occupancy
      ReadBits(&BitBuf, 1);  // marker bit
    }
  }
  ReadBits(&BitBuf, 2); // VOL shape
  ReadBits(&BitBuf, 1);  // marker bit
  uint16 usVOPTimeIncResolution = (uint16)ReadBits(&BitBuf, 16);
  ReadBits(&BitBuf, 1);  // marker bit

  if (ReadBits(&BitBuf, 1)) // Fixed VOP rate
  {
    // range [0..vop_time_increment_resolution)

    // vop_time_increment_resolution
    // 2 => 0..1, 1 bit
    // 3 => 0..2, 2 bits
    // 4 => 0..3, 2 bits
    // 5 => 0..4, 3 bits
    // ...
    if(usVOPTimeIncResolution)
      --usVOPTimeIncResolution;

    uint8 ucNumBits = 0;
    while (usVOPTimeIncResolution > 0)
    {
      ++ucNumBits;
      usVOPTimeIncResolution >>= 1;
    }
    ReadBits(&BitBuf, ucNumBits);  // fixed VOP Time Increment
  }

  ReadBits(&BitBuf, 1);  // marker bit
  uint16 usVOLWidth = (uint16)ReadBits(&BitBuf, 13);
  ReadBits(&BitBuf, 1);  // marker bit
  uint16 usVOLHeight = (uint16)ReadBits(&BitBuf, 13);
  ReadBits(&BitBuf, 1);  // marker bit

  *pslWidth  = usVOLWidth;
  *pslHeight = usVOLHeight;
  return true;
}
