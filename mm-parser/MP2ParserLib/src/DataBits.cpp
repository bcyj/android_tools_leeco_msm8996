/* =======================================================================
                              DataBits.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/src/DataBits.cpp#30 $
========================================================================== */

#include "MMDebugMsg.h"
#include "SectionHeaderParser.h"

/*! ======================================================================
@brief  getBytesValue

@detail    Returns the value associated with nBytes located in Data

@param[in] N/A

@return    Value associated with nBytes else returns 0
@note      None.
========================================================================== */
uint32 getBytesValue(int nBytes,uint8* Data)
{
  uint32 val = 0;
  int itr = 0;
  while(Data && (nBytes > 0) && (nBytes <= (int)4) )
  {
    val = val <<8;
    val += Data[itr++];
    nBytes--;
  }
  return val;
}
/*! ======================================================================
@brief  make33BitValue

@detail    Returns the 33 bit value.Various bits fragments are passed in which are
        used to construct the value
@param[in] N/A

@return    Value associated with nBytes else returns 0
@note      None.
========================================================================== */
uint64 make33BitValue(uint8 ms3bits,uint16 middle15bits,uint16 ls15bits)
{
  uint64 result = 0;

  //first copy over most significant 3 bits as we construct the number by <<

  for(int i = 0; i <= 2; i++)
  {
    result <<= 1;
    if(ms3bits & 0x80)
    {
      result = result | (uint64)0x01;
    }
    else
    {
      result = result | (uint64)0x00;
    }
    ms3bits = (uint8)(ms3bits<<1);
  }
  //Now copy remaining 30 bits
  for(int i = 0; i <= 14; i++)
  {
    result <<= 1;
    if(middle15bits & (uint16)0x4000)
    {
      result = result | (uint64)0x01;
    }
    else
    {
      result = result | (uint64)0x00;
    }
    middle15bits =(uint16)(middle15bits << 1);
  }
  for(int i = 0; i <= 14; i++)
  {
    result <<= 1;
    if(ls15bits & (uint16)0x4000)
    {
      result = result | (uint64)0x01;
    }
    else
    {
      result = result | (uint64)0x00;
    }
    ls15bits = (uint16)(ls15bits << 1);
  }
  return result;
}
/*! ======================================================================
@brief  make15BitValue

@detail    Returns a 15 value.Various bits fragments are passed in which are
        used to construct the value.
@param[in] N/A

@return    Value associated with nBytes else returns 0
@note      None.
========================================================================== */
uint16 make15BitValue(uint8 topbits,uint16 lowerbits)
{
  uint16 result = 0;
  //first copy over most significant 2 bits as we construct the number by <<
  for(int i = 0; i <= 1; i++)
  {
    result = (uint16)(result << 1);
    if(topbits & 0x80)
    {
      result = result | (uint16)0x01;
    }
    else
    {
      result = result | (uint16)0x00;
    }
    topbits = (uint8)(topbits << 1);
  }
  //Now copy remaining 13 bits
  for(int i = 0; i <= 12; i++)
  {
    result = (uint16)(result << 1);
    if(lowerbits & 0x8000)
    {
      result = result | (uint16)0x01;
    }
    else
    {
      result = result | (uint16)0x00;
    }
    lowerbits = (uint16) (lowerbits << 1);
  }
  return result;
}

/*! ======================================================================
@brief  make9BitValue

@detail    Returns a 9 bit value.Various bits fragments are passed in which are
        used to construct the value.
@param[in] N/A

@return    Value associated with nBytes else returns 0
@note      None.
========================================================================== */
uint16 make9BitValue(uint8 topbits,uint8 lowerbits)
{
  uint16 result = 0;
  //first copy over most significant 2 bits as we construct the number by <<
  for(int i = 0; i <= 1; i++)
  {
    result = (uint16) (result << 1);
    if(topbits & 0x80)
    {
      result = result | (uint16)0x01;
    }
    else
    {
      result = result | (uint16)0x00;
    }
    topbits = (uint8) (topbits << 1);
  }
  //Now copy remaining 7 bits
  for(int i = 0; i <= 6; i++)
  {
    result = (uint16)(result << 1);
    if(lowerbits & 0x80)
    {
      result = result | (uint16)0x01;
    }
    else
    {
      result = result | (uint16)0x00;
    }
    lowerbits = (uint8)(lowerbits << 1);
  }
  return result;
}

/*! ======================================================================
@brief  make22BitValue

@detail    Returns the 22 bit value.Various bits fragments are passed in which are
        used to construct the value
@param[in] N/A

@return    Value associated with nBytes else returns 0
@note      None.
========================================================================== */
uint32 make22BitValue(uint16 part1,uint8 part2)
{
  uint32 result = 0;
  //first copy over most significant 15 bits as we construct the number by <<
  for(int i = 0; i <= 14; i++)
  {
    result <<= 1;
    if(part1 & 0x8000)
    {
      result = result | (uint32)0x01;
    }
    else
    {
      result = result | (uint32)0x00;
    }
    part1 = (uint16)(part1<< 1);
  }
  //Now copy remaining 7 bits
  for(int i = 0; i <= 6; i++)
  {
    result <<= 1;
    if(part2 & 0x80)
    {
      result = result | (uint32)0x01;
    }
    else
    {
      result = result | (uint32)0x00;
    }
    part2 =(uint8)(part2 << 1);
  }
  return result;
}
/*! ======================================================================
@brief  make42BitValue

@detail    Returns the 42 bit value.Various bits fragments are passed in which are
        used to construct the value
@param[in] N/A

@return    Value associated with nBytes else returns 0
@note      None.
========================================================================== */
uint64 make42BitValue(uint64 part1, uint16 part2)
{
  uint64 result = 0;
  //first copy over most significant 16 bits as we construct the number by <<
  for(int i = 0; i <= 32; i++)
  {
    result <<= 1;
    if(part1 & 0x8000000000000000ULL)
    {
      result = result | (uint64)0x01;
    }
    else
    {
      result = result | (uint64)0x00;
    }
    part1 <<= 1;
  }
  //Now copy remaining bits
  for(int i = 0; i <= 8; i++)
  {
    result <<= 1;
    if(part2 & 0x8000)
    {
      result = result | (uint64)0x01;
    }
    else
    {
      result = result | (uint64)0x00;
    }
    part2 =(uint16)(part2 << 1);
  }
  return result;
}

/*===========================================================================
FUNCTION:
  getByteFromBitStream

DESCRIPTION:
  This function gets given number of bits from the given bit of source in the
  destination byte.

INPUT/OUTPUT PARAMETERS:
  uint8   *pByte      destination byte
  uint8   *pSrc       source stream of bits
  int      nFromBit   from which bit of source
  int      nBits      number of bits to copy in byte (it should not be more than 8)

  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
===========================================================================*/
void getByteFromBitStream(uint8 *pByte, uint8 *pSrc, int nFromBit, int nBits)
{
  int a, b, i;
  uint8 temp;
  *pByte = 0;       /* reset all the bits */
  if(nBits <=8 )
  {
    for( i=0; i < nBits; i++)
    {
      a = nFromBit/8;
      b = nFromBit%8;
      *pByte = (uint8)(*pByte << 1);     /* make space for next bit */
      temp = (uint8)(pSrc[a] << b);
      *pByte = (uint8)(*pByte | (uint8)(temp >> 7));      /* OR after masking all other bits */
      nFromBit++;
    }
  }
}

