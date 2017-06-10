#ifndef UTF8CONV_H
#define UTF8CONV_H
/* =======================================================================
                               utf8conf.h
DESCRIPTION
  Declarations for functions to convert between Unicode and
  UTF8.  For more information on Unicode and UTF8 please see
  http://www.unicode.org/unicode/faq/utf_bom.html

              UTF-8 Bit Distribution 

UTF-16                  1st Byte 2nd Byte 3rd Byte 4th Byte 
-------- -------- -------- --------     -------- -------- -------- --------
00000000 0xxxxxxx           0xxxxxxx       
00000yyy yyxxxxxx           110yyyyy 10xxxxxx     
zzzzyyyy yyxxxxxx           1110zzzz 10yyyyyy 10xxxxxx   
110110ww wwzzzzyy 110111yy yyxxxxxx     11110uuu 10uuzzzz 10yyyyyy 10xxxxxx 

NOTE: 
 uuuuu = wwww+1 (to account for addition of 0x10000 as in Section 3.7, Surrogates)
  
Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/utf8conv.h#5 $
$DateTime: 2011/03/28 16:40:17 $
$Change: 1675367 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#ifdef __CC_ARM
#include <stddef.h>
#endif
#include <stdio.h>

#include "parserdatadef.h"
#include "parserinternaldefs.h"

#include "oscl_file_io.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations 
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
MACRO MYOBJ

ARGS 
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Function Declarations
** ======================================================================= */

#ifdef __cplusplus

extern "C" {

#endif

/* ======================================================================
FUNCTION 
  UnicodeToUTF8

DESCRIPTION
  Convert UTF8 byte sequence to Unicode string 

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  On success, the number of characters in the destination buffer
  0 on failure due to insufficient buffer size

SIDE EFFECTS
  Detail any side effects.
  
========================================================================== */
int32 UTF8ToUnicode(const int8 *input, int32 inLength, wchar_t *output, int32 outLength);

/* ======================================================================
FUNCTION 
  UnicodeToUTF8

DESCRIPTION
  Convert Unicode string to UTF8 byte sequence 

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  On success, the number of bytes in the destination buffer
  0 on failure due to insufficient buffer size

SIDE EFFECTS
  Detail any side effects.
  
========================================================================== */
int32 UnicodeToUTF8(const wchar_t *input, int32 inLength, int8 *output, int32 outLength);

#ifdef __cplusplus

}

#endif

/* PV (MP4 FileFormat library?) specific #defines */
//#define WideCharToMultiByte(a,b,c,d,e,f,g,h) UnicodeToUTF8(c,d,e,f)
//#define MultiByteToWideChar(a,b,c,d,e,f) UTF8ToUnicode(c,d,e,f)

#endif /* UTF8CONV_H */

