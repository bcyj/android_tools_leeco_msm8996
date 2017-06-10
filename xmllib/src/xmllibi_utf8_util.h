#ifndef _XMLLIBI_UTF8_UTIL
#define _XMLLIBI_UTF8_UTIL
/*===========================================================================

                  X M L    U T I L    L I B R A R Y
                      U T F - 8    E N C O D I N G
                        
                   
DESCRIPTION
 This file implements some utility functions to decode UTF-8 encoded byte
 sequences.
 
Copyright (c) 2006 by Qualcomm Technologies, Inc.  All Rights Reserved.
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //depot/asic/sandbox/projects/cne/common/xmllib/src/xmllibi_utf8_util.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/06   ssingh  Created module.

===========================================================================*/
#include "xmllib_common.h"


/*===========================================================================
                   XML UTF-8 UTILITY FUNCTIONS
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
);


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
);


/* TODO: This function is for LTK based unit testing only. It will be removed 
 * once the code has been freezed. 
 */
int32 xmllibi_utf8_decod_test
(
  xmllib_metainfo_s_type  *metainfo,      /* XML meta information           */
  xmllib_error_e_type     *error_code     /* Error code                     */
);

#endif /* _XMLLIBI_UTF8_UTIL */

