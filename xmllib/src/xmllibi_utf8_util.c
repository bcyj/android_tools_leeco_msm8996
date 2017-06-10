/*===========================================================================

                  X M L    U T I L    L I B R A R Y
                      U T F - 8    E N C O D I N G
                   
DESCRIPTION
 This file implements some utility functions to decode UTF-8 encoded byte
 sequences.
 
EXTERNALIZED FUNCTIONS

Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_utf8_util.c#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Created module.

===========================================================================*/
/*===========================================================================
                      INCLUDE FILES FOR MODULE
===========================================================================*/
#include "comdef.h"

#ifdef FEATURE_XMLLIB
#include "xmllib_common.h"
#include "xmllibi_utf8_util.h"
#include <string.h>

/*===========================================================================
                             MACRO DEFINITIONS
===========================================================================*/

/*---------------------------------------------------------------------------
  Number of bits per byte
---------------------------------------------------------------------------*/
#define UTF8_NUM_BITS_PER_BYTE       8

/*---------------------------------------------------------------------------
  The maximum number of bytes in a UTF-8 sequence.
---------------------------------------------------------------------------*/
#define UTF8_SEQ_MAX_BYTES           6

/*---------------------------------------------------------------------------
  To find out the number of indicated bytes in the UTF-8 sequence
---------------------------------------------------------------------------*/
#define UTF8_SEQ_BYTE_COUNT_MASK     0x80  /* Used to count the 1 bits   */

/*---------------------------------------------------------------------------
  For doing operations on leading byte of the UTF-8 sequence
---------------------------------------------------------------------------*/
#define UTF8_LEAD_BYTE_DECOD_MASK    0xFF  /* To extract the UCS value   */

/*---------------------------------------------------------------------------
  For doing operations on trailing bytes of the UTF-8 sequence
---------------------------------------------------------------------------*/
#define UTF8_TRAIL_BYTE_DECOD_MASK   0x7F  /* To extract the UCS value   */
#define UTF8_TRAIL_BYTE_BIT_SHIFT    6     /* Bits to shift              */

/*---------------------------------------------------------------------------
  The continuation byte. All trailing bytes of a valid UTF-8 sequence are of
  the form 10xx xxxx
---------------------------------------------------------------------------*/
#define UTF8_CONTINUATION_BYTE        0x80

/*---------------------------------------------------------------------------
  The continuation byte, 10xx xxxx , when right shifted by 6 bits
---------------------------------------------------------------------------*/
#define UTF8_CONT_BYTE_R_SHIFTED      0x02

/*---------------------------------------------------------------------------
  Invalid UTF-8 bytes that should not be present in a UTF-8 sequence
---------------------------------------------------------------------------*/
#define UTF8_INVALID_BYTE_1          0xFE  /* Invalid byte */
#define UTF8_INVALID_BYTE_2          0xFF  /* Invalid byte */

/*===========================================================================
                         local definitions
===========================================================================*/
static struct 
{ 
  uint32 low; 
  uint32 high; 
} range[6] = 
{ 
  {0,         0x7f},
  {0x80,      0x7ff},
  {0x800,     0xffff},
  {0x10000,   0x1fffff},
  {0x200000,  0x3ffffff},
  {0x4000000, 0x7fffffff}
};

/*===========================================================================
                     XML DECL FUNCTIONS
===========================================================================*/

/*===========================================================================
FUNCTION XMLLIBI_UTF8_IS_SEQ_VALID

DESCRIPTION
  This function determines whether the Unicode Character Set (UCS) code point
  is valid, and whether any overlong UTF-8 sequence was used.
  
  Overlong UTF-8 sequences:
  UTF-8 sequences must not be longer than necessary. For example, the UCS 
  character U+000B must be present only as 0x0B. It cannot be in any of the 
  following overlong forms: 

  0xC0 0x8B
  0xE0 0x80 0x8B
  0xF0 0x80 0x80 0x8B
  0xF8 0x80 0x80 0x80 0x8B
  0xFC 0x80 0x80 0x80 0x80 0x8B

DEPENDENCIES
  None

RETURN VALUE
  TRUE   if the UCS code point is a valid ISO 16046 character, and
         no overlong UTF-8 sequence was used
  FALSE  if not

SIDE EFFECTS
  None
===========================================================================*/
boolean xmllibi_utf8_is_seq_valid
(
  uint32  ucsVal,
  uint32  utf8_bytes
)
{
  /*-------------------------------------------------------------------------
    The following code points are not valid ISO 10646 characters and
    should be rejected.
    -------------------------------------------------------------------------*/
  if( ucsVal == 0xD800 ||
      ucsVal == 0xDB7F ||
      ucsVal == 0xDB80 ||
      ucsVal == 0xDBFF ||
      ucsVal == 0xDC00 ||
      ucsVal == 0xDF80 ||
      ucsVal == 0xDFFF )
  {
    return FALSE;
  }

  /*-------------------------------------------------------------------------
    Make sure no overlong sequence was used - a simple check is to look at
    the number of UTF-8 bytes used to encode the UCS code point.
    -------------------------------------------------------------------------*/
  return ( utf8_bytes >= 1 && 
           utf8_bytes <= 6 && 
           ucsVal >= range[utf8_bytes -1].low && 
           ucsVal <= range[utf8_bytes -1].high );
}
  

/*===========================================================================
FUNCTION XMLLIBI_UTF8_PEEKBYTES

DESCRIPTION
  This function is called to peek at the next UTF-8 byte sequence. It then
  decodes this UTF-8 byte sequence into the corresponding UCS code point 
  value. It also indicates the number of bytes in the UTF-8 sequence that
  were used to encode the UCS code point.
  
DEPENDENCIES
  None

RETURN VALUE
  TRUE   if the byte sequence was successfully decoded
  FALSE  if not

SIDE EFFECTS
  None
===========================================================================*/
int32 xmllibi_utf8_peekbytes
(
  xmllib_metainfo_s_type  *metainfo,      /* XML meta information           */
  uint32                  offset,         /* Offset in the text             */
  uint32                  *ucs,           /* UCS value of the UTF-8 seq     */
  uint32                  *utf8_bytes,    /* UTF-8 bytes found in the seq   */
  xmllib_error_e_type     *error_code     /* Error code                     */
)
{
  int32   ret;
  uint8   bytevalue;
  uint32  remainingbytes;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != ucs );
  XMLLIBI_DEBUG_ASSERT( NULL != utf8_bytes );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  /*-------------------------------------------------------------------------
    Peek at the first byte in the stream.  
    -------------------------------------------------------------------------*/
  ret = metainfo->peekbytes( metainfo,
                             offset,
                             sizeof(bytevalue),
                             &bytevalue );

  if( XMLLIB_ERROR == ret)
  {
  /*-------------------------------------------------------------------------
    Peekbytes() failed, we didn't peek at any byte
    -------------------------------------------------------------------------*/
    *utf8_bytes = 0;
    *ucs = 0;
    return XMLLIB_ERROR;
  }

  if( bytevalue == UTF8_INVALID_BYTE_1 || 
      bytevalue == UTF8_INVALID_BYTE_2 ||
      bytevalue == UTF8_CONTINUATION_BYTE )
  {
  /*-------------------------------------------------------------------------
    We peeked at an invalid byte
    -------------------------------------------------------------------------*/
    *utf8_bytes = 1;
    *ucs = 0;
    *error_code = XMLLIB_ERROR_INVALID_ENCODING;
    return XMLLIB_ERROR;
  }
  /*-------------------------------------------------------------------------
    Count the leading '1's in the MSB to determine how many bytes are 
    present in the UTF-8 sequence.
    -------------------------------------------------------------------------*/
  *utf8_bytes = 1;
  *ucs = bytevalue;
    
  /*-------------------------------------------------------------------------
    1 byte ascii subset? It will be of the form 0xxx xxxx
    No further processing required.
    -------------------------------------------------------------------------*/
  if( !(bytevalue & UTF8_SEQ_BYTE_COUNT_MASK) )
  {
    return XMLLIB_SUCCESS;
  }

  while( (bytevalue & (UTF8_SEQ_BYTE_COUNT_MASK >> *utf8_bytes)) &&
          *utf8_bytes <= UTF8_SEQ_MAX_BYTES )
  {
    ++(*utf8_bytes);
  }

  /*-------------------------------------------------------------------------
    Lead byte shouldn't be of the form 10xx xxxx  : the first if condition.
    Nor should it indicate more bytes than are valid : the second condition.
    -------------------------------------------------------------------------*/
  if(*utf8_bytes == 1 || *utf8_bytes > UTF8_SEQ_MAX_BYTES)
  {
    *error_code = XMLLIB_ERROR_INVALID_ENCODING;
    return XMLLIB_ERROR;
  }

  /*-------------------------------------------------------------------------
    Store the value from the MSB of the UTF-8 byte sequence into the UCS.
    We need to do some masking and bit shifting, and also process
    the remaining bytes in the sequence.
    -------------------------------------------------------------------------*/
  *ucs = *ucs & (UTF8_LEAD_BYTE_DECOD_MASK >> (*utf8_bytes + 1) );
  
  remainingbytes = *utf8_bytes - 1;

  while(remainingbytes-- > 0)
  {
    /*-------------------------------------------------------------------------
      Make way to store the trailing bytes.  
      -------------------------------------------------------------------------*/
    *ucs = *ucs << UTF8_TRAIL_BYTE_BIT_SHIFT;

    /*-------------------------------------------------------------------------
      Peek at the next byte in the stream.  
      -------------------------------------------------------------------------*/
    ret = metainfo->peekbytes( metainfo,
                               ++offset,
                               sizeof(bytevalue),
                               &bytevalue );


    /*-------------------------------------------------------------------------
      It is an encoding error if:
      a. the byte is missing (peekbytes() will return an error), or
      b. the byte is not of the form 10xx xxxx
      Test for (b) also ensures that the invalid bytes UTF8_INVALID_BYTE_1 and
      UTF8_INVALID_BYTE_2 are not present.
      -------------------------------------------------------------------------*/
    if( XMLLIB_ERROR == ret ||
        (bytevalue >> UTF8_TRAIL_BYTE_BIT_SHIFT) != UTF8_CONT_BYTE_R_SHIFTED)
    {
      /*----------------------------------------------------------------------
        Expected number of bytes were not found. So indicate the actual number
        of UTF-8 bytes that were peeked at.
      ----------------------------------------------------------------------*/
      *utf8_bytes = (XMLLIB_ERROR == ret)?
                    *utf8_bytes - remainingbytes - 1:
                    *utf8_bytes - remainingbytes;

      *ucs = 0;
      *error_code = XMLLIB_ERROR_INVALID_ENCODING;
      ret = XMLLIB_ERROR;
      break;
    }

    /*-------------------------------------------------------------------------
      OR the value from this UTF-8 byte into the UCS value. 
      -------------------------------------------------------------------------*/
    *ucs = *ucs | (UTF8_TRAIL_BYTE_DECOD_MASK & bytevalue);
  }

  /*-------------------------------------------------------------------------
    Make sure the UCS value was correctly encoded into using a 
    valid UTF-8 byte sequence
    -------------------------------------------------------------------------*/
  if( XMLLIB_SUCCESS == ret &&
      !xmllibi_utf8_is_seq_valid(*ucs,*utf8_bytes))
  {
    ret = XMLLIB_ERROR;
    *error_code = XMLLIB_ERROR_INVALID_ENCODING;
  }

  return ret;
}



/* TODO: This function is for LTK based unit testing only. It will be removed 
 * once the code has been freezed. 
 */
int32 xmllibi_utf8_decod_test
(
  xmllib_metainfo_s_type  *metainfo,      /* XML meta information          */
  xmllib_error_e_type     *error_code     /* Error code                    */
)
{
  uint32                  offset  = 0;   /* Offset in the text             */
  uint32                  ucs;           /* UCS value of the UTF-8 seq     */
  uint32                  utf8_bytes;    /* UTF-8 bytes found in the seq   */
  int32                   ret = XMLLIB_SUCCESS;

  xmllib_string_s_type *tempXmlString;
  /*-------------------------------------------------------------------------
    Verify arguments
    -------------------------------------------------------------------------*/
  XMLLIBI_DEBUG_ASSERT( NULL != metainfo );
  XMLLIBI_DEBUG_ASSERT( NULL != error_code );

  
  tempXmlString = (xmllib_string_s_type *) metainfo->xmltext;

  /* Look at all the bytes until the end is reached */
  while(tempXmlString->len > 0)
  {
    /* Peek at the next byte */
    ret = xmllibi_utf8_peekbytes ( metainfo,
                                  offset,
                                  &ucs,
                                  &utf8_bytes,
                                  error_code);

    /* Go to start of next sequence */
    metainfo->getbytes( metainfo,
                        utf8_bytes,
                        NULL );
  }

  return ret;
}


#endif  /* FEATURE_XMLLIB */
