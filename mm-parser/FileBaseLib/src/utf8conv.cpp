/* =======================================================================
                               utf8conv.cpp
DESCRIPTION
  Implementation of functions to convert between Unicode and
  UTF8.  For more information on Unicode and UTF8 please see
    http://www.unicode.org/unicode/faq/utf_bom.html

                             UTF-8 Bit Distribution 

UTF-16                                  1st Byte 2nd Byte 3rd Byte 4th Byte 
-------- -------- -------- --------     -------- -------- -------- --------
00000000 0xxxxxxx                       0xxxxxxx       
00000yyy yyxxxxxx                       110yyyyy 10xxxxxx     
zzzzyyyy yyxxxxxx                       1110zzzz 10yyyyyy 10xxxxxx   
110110ww wwzzzzyy 110111yy yyxxxxxx     11110uuu 10uuzzzz 10yyyyyy 10xxxxxx 

NOTE: 
 uuuuu = wwww+1 (to account for addition of 0x10000 as in Section 3.7, Surrogates)

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright (c) 2011-2014 QUALCOMM Technologies Inc, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/utf8conv.cpp#6 $
$DateTime: 2012/03/07 21:17:30 $
$Change: 2256197 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"

#include "utf8conv.h"

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define BYTE_1_REP          0x80    /* if <, will be represented in 1 byte */
#define BYTE_2_REP          0x800   /* if <, will be represented in 2 bytes */

/* If the unicode value falls on or between these values, it will be
   represented as 4 bytes 
*/
#define SURROGATE_MIN       0xd800
#define SURROGATE_MAX       0xdfff

/* Convention for naming of following defines

   SIGMASK_3_1     - Signature mask for 1st byte of 3 byte transformation
   CLEARMASK_2_1   - Clearout mask for 1st byte of 2 byte transformation
   ROR_3_2         - Rotate right value for 2nd byte of 3 byte transformation

*/

#define SIGMASK_2_1                0xc0
#define SIGMASK_3_1                0xe0

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
  UnicodeToUTF8

DESCRIPTION
  Convert Unicode string to UTF8 byte sequence
  
  szSrc - Unicode string to be converted
  nSrcLen - Length of szSrc
  strDest - char buffer for UTF8 text
  nDestLen - size (in characters) of buffer

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  On success, the number of bytes in the destination buffer
  0 on failure due to insufficient buffer size
  
SIDE EFFECTS
  Detail any side effects.
  
========================================================================== */
/*lint -e10 -e49 -e40 -e63 */
int32 UnicodeToUTF8(const wchar_t *pucTemp, int32 nSrcLen,
                    int8 *strDest, int32 nDestLen)
{
  int32 i=0;
  int32 i_cur_output=0; 
  int8 ch_tmp_byte;
  int16* szSrc = (int16*)pucTemp;

  for ( i=0; i<nSrcLen; i++ )
  {
    if ( BYTE_1_REP > szSrc[i] ) /* 1 byte utf8 representation */
    {
      if ( i_cur_output+1<nDestLen )
        strDest[i_cur_output++]=(int8)szSrc[i];
      else
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
    }
    else if ( BYTE_2_REP > szSrc[i] ) /* 2 byte utf8 representation */
    {
      if ( i_cur_output+2<nDestLen )
      {
        strDest[i_cur_output++]=(int8)(szSrc[i] >> 6 | 0xc0);
        strDest[i_cur_output++]=(int8)((szSrc[i] & 0x3f) | 0x80);
      }
      else
      {
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
      }
    }
    else if ( SURROGATE_MAX > szSrc[i] && SURROGATE_MIN < szSrc[i] )
    {        /* 4 byte surrogate pair representation */
      if ( i_cur_output+4<nDestLen )
      {
        ch_tmp_byte = (int8)(((szSrc[i] & 0x3c0) >> 6) + 1);
        strDest[i_cur_output++]=(int8)(ch_tmp_byte >> 2 | 0xf0); 
        strDest[i_cur_output++]=(int8)(((ch_tmp_byte & 0x03) | 0x80) | (szSrc[i] & 0x3e) >> 2);
      }
      else
      {
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
      }
    }
    else /* 3 byte utf8 representation */
    {
      if ( i_cur_output+3<nDestLen )
      {
        strDest[i_cur_output++]=(int8)(szSrc[i] >> 12 | 0xe0);
        strDest[i_cur_output++]=(int8)((szSrc[i] >> 6  & 0x3f) | 0x80);
        strDest[i_cur_output++]=(int8)((szSrc[i] & 0x3f) | 0x80);
      }
      else
      {
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
      }
    } /* ToDo:  Handle surrogate pairs */
  }

  strDest[i_cur_output] = '\0'; /* Terminate string */

  return i_cur_output; /* This value is in bytes */
}
/*lint +e10 +e49 +e40 +e63 */

/* ======================================================================
FUNCTION 
  UTF8ToUnicode

DESCRIPTION
  Convert UTF8 byte sequence to Unicode string

  szSrc - UTF8 byte sequence to be converted
  nSrcLen - Length of szSrc
  strDest - unicode char buffer for
  nDestLen - size (in characters) of buffer

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  On success, the number of characters in the destination buffer
  0 on failure due to insufficient buffer size
  
SIDE EFFECTS
  Detail any side effects.
  
========================================================================== */
/*lint -e10 -e49 -e40 -e63 -e601 -e57 -e60 -e26 -e515 -e64 */
int32 UTF8ToUnicode(const int8 *szSrc, int32 nSrcLen, wchar_t *strDest, int32 nDestLen)
{
  int32 i=0;
  int32 i_cur_output=0;

  uint8 *pszSrc = (uint8 *)szSrc;  /* cast to avoid signed/unsigned promomtion problems */
  while ( i<nSrcLen )
  {
    if ( SIGMASK_3_1 <= pszSrc[i] ) /* 1st byte of 3 byte representation */
    {
      if ( i+2<nSrcLen && i_cur_output+1<nDestLen )
      {
        strDest[i_cur_output++] =(wchar_t)( (wchar_t)pszSrc[i] << 12 | 
                                            ((wchar_t)pszSrc[i+1] & 0x3f) << 6 | 
                                            ((wchar_t)pszSrc[i+2] & 0x3f));
        i+=3;
      }
      else
      {
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
      }
    }
    else if ( SIGMASK_2_1 <= pszSrc[i] ) /* 1st byte of 2 byte representation */
    {
      if ( i+1<nSrcLen && i_cur_output+1<nDestLen )
      {
        strDest[i_cur_output++] = (wchar_t)(((wchar_t)pszSrc[i] & ~0xc0) << 6 | 
                                            ((wchar_t)pszSrc[i+1] & ~0x80));
        i+=2;
      }
      else
      {
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
      }
    }
    else /* Single byte representation */
    {
      if ( i<nSrcLen && i_cur_output+1<nDestLen )
      {
        strDest[i_cur_output++] = (wchar_t)pszSrc[i];
        ++i;
      }
      else
      {
        return 0; /* ERROR_INSUFFICIENT_BUFFER */
      }
    }
  }

  strDest[i_cur_output] = 0; /* Terminate string */
  return i_cur_output;
}
/*lint +e10 +e49 +e40 +e63 +e601 +e57 +e60 +e26 +e515 +e64 */

