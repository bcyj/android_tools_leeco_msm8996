/* =======================================================================
                              zrex_string.cpp
DESCRIPTION
  Substitutions for Windows system calls and runtime
  library routines.  For use with the ZREX versions of the core libraries.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright 2011-2012 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

========================================================================== */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/src/zrex_string.cpp#11 $
$DateTime: 2012/06/26 21:55:21 $
$Change: 2538712 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#include <stdio.h>
#include <ctype.h>
#include "qcplayer_oscl_utils.h"
#include "zrex_string.h"

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

///////////////////////////////////////////////////////////////////////////
//
// Standard String and widestring calls
//
///////////////////////////////////////////////////////////////////////////

/* ======================================================================
FUNCTION:
  zrex_strstri

DESCRIPTION:
  Case insensitive string search
  For case sensitive string search use the ANSI C <string.h> lib's strstr()

INPUT/OUPUT PARAMETERS:
  string
  strCharSet

RETURN VALUE:
  Pointer to first occurrence of strCharSet in string

SIDE EFFECTS:
  None.
========================================================================== */
char *zrex_strstri( const char *string, const char *strCharSet )
{
  int i = ZUtils::Find(string,strCharSet);
  return(i==ZUtils::npos) ? NULL : (char *)&string[i];
}

/* ======================================================================
FUNCTION:
  zrex_strupr

DESCRIPTION:
  Equivalent implementation

INPUT/OUPUT PARAMETERS:
  string

RETURN VALUE:
  Return pointer to string

SIDE EFFECTS:
  None.
========================================================================== */
char *zrex_strupr( char *string )
{
  char * upper = string;
  while ( *upper )
  {
    *upper = (char)toupper(*upper);
    upper++;
  }
  return string;
}

/* ======================================================================
FUNCTION:
  zrex_strlwr

DESCRIPTION:
  Equivalent implementation

INPUT/OUPUT PARAMETERS:
  string

RETURN VALUE:
   Returns a pointer to string

SIDE EFFECTS:
  None.
========================================================================== */
char *zrex_strlwr( char *string )
{
  char * lower = string;
  while ( *lower )
  {
    *lower = (char)tolower(*lower);
    lower++;
  }
  return string;
}

/* ======================================================================
FUNCTION:
  zrex_strcmp

DESCRIPTION:
  String comparison for Wide Char strings

INPUT/OUPUT PARAMETERS:
  Input: 2 Null terminated strings

RETURN VALUE:
  <0 if str2 > str1
  >0 if str1 > str2
  =0 if both strings are same

SIDE EFFECTS:
  None.
========================================================================== */
int zrex_strcmp(const wchar_t *str1, const wchar_t *str2)
{
  while ((*str1 == *str2) && (*str1 != '\0') && (*str2 != '\0'))
  {
    str1++;
    str2++;
  }
  return (*str1 - *str2);
}

/* ======================================================================
FUNCTION:
  zrex_strncmp

DESCRIPTION:
  String comparison for Wide Char strings

INPUT/OUPUT PARAMETERS:
  Input: 2 Null terminated strings
         n Number of characters to compare

RETURN VALUE:
  <0 if str2 > str1
  >0 if str1 > str2
  =0 if both strings are same

SIDE EFFECTS:
  None.
========================================================================== */
int zrex_strncmp(const wchar_t *str1, const wchar_t *str2, unsigned int n)
{
  while ((*str1 == *str2) && (*str1 != '\0') && (*str2 != '\0') && n > 0)
  {
    str1++;
    str2++;
    n--;
  }
  if (n == 0)
  {
    return 0;
  }
  return (*str1 - *str2);
}

// Building the LTK will cause a conflict with zrex_wcscpy() defined in LIBCMT
#ifndef PLATFORM_LTK
/* ======================================================================
FUNCTION:
  zrex_wcscpy

DESCRIPTION:
  Equivalent implementation

INPUT/OUPUT PARAMETERS:
  strDestination
  strSource

RETURN VALUE:
  Returns pointer to strDestination

SIDE EFFECTS:
  None.
========================================================================== */
/*lint -e129 -e10 */
wchar_t *zrex_wcscpy( wchar_t *strDestination, uint32 ndstsize, const wchar_t *strSource )
{
  for (uint32 i=0; i<ndstsize;i++)
  {
    strDestination[i] = strSource[i];
    if (strSource[i]==0)
    {
      break;
    }
  }
  return strDestination;
}
/*lint +e129 +e10 */
#endif /* PLATFORM_LTK */
/* ======================================================================
FUNCTION:
  zrex_wcslen

DESCRIPTION:
  Equivalent implementation

INPUT/OUPUT PARAMETERS:
  string

RETURN VALUE:
  Return length of string

SIDE EFFECTS:
  None.
========================================================================== */
/*lint -e10 -e49 -e40 */
size_t zrex_wcslen( const wchar_t *str )
{
  for (int i=0; ;i++)
  {
    if (str[i] == 0)
    {
      return i;
    }
  }
}
/*lint +e10 +e49 +e40 */
///////////////////////////////////////////////////////////////////////////
//
// Windows multi-byte support
//
///////////////////////////////////////////////////////////////////////////
/* ======================================================================
FUNCTION:
  CharToWideChar

DESCRIPTION:
  limited substitute for MultiByteToWideChar

INPUT/OUPUT PARAMETERS:
  src
  nSrc
  dest
  nDest

RETURN VALUE:
  String length

SIDE EFFECTS:
  None.
========================================================================== */

/*lint -e40 -e601 -e10 -e530 */
int CharToWideChar(
                  const char * src, // address of string to map
                  int nSrc,      // number of bytes in string
                  wchar_t * dest,  // address of wide-character buffer
                  int nDest,        // size of buffer
                  boolean isUnicode = FALSE
                  )
{
  char *pDest = (char*)dest;
  if (nDest==0)
  {
    //return size of required output buffer
    return (int)strlen(src) + 1;
  }
  else
  {
    //see how many chars to convert
    int n = (nSrc < 0) ? (int)strlen(src)+1 : nSrc;
    int count = 0;

    //reset output memory
    memset(dest, 0, nDest);
    if ((n * sizeof(wchar_t))/(1+isUnicode) > (unsigned int)nDest)
    {
      return 0;
    }
    for (int i = 0; i < n; i++)
    {
      if (i<nDest)
      {
        //convert to unsigned first to avoid sign-extension.
        if(FALSE == isUnicode)
          dest[i]=(wchar_t)((unsigned char)src[i]);
        else //convert uint16 character into wchar(uint32) for LA platform
        {
          pDest[count * 4] = src[i];
          pDest[count * 4 + 1] = src[i + 1];
          i++;
        }
        count++;
      }
    }

    dest[count] = (wchar_t) 0;
    dest[nDest - 1] = (wchar_t) 0;

    return count;
  }
}
/*lint +e40 +e601 +e10 +e530 */

/* ======================================================================
FUNCTION:
  WideCharToChar

DESCRIPTION:
  limited substitute for WideCharToMultiByte

INPUT/OUPUT PARAMETERS:
  src
  nSrc
  dest
  nDest

RETURN VALUE:
  Return string length

SIDE EFFECTS:
  None.
========================================================================== */
/*lint -e49 -e40 -e10 -e63 */
int WideCharToChar(
                  const wchar_t * src, // address of string to map
                  int nSrc,      // number of bytes in string
                  char * dest,  // address of wide-character buffer
                  int nDest        // size of buffer
                  )
{
  uint16 *WideChar16 = (uint16*)src;
  if (nDest==0)
  {
    //return size of required output buffer
    return (int)zrex_wcslen(src)+1;
  }
  else
  {
    //see how many chars to convert
    int n = (nSrc < 0) ? (int)zrex_wcslen(src)+1 : nSrc;
    int count = 0;
    for (int i = 0; (i < n) && (i < nDest); i++)
    {
      //truncate the output string, depending on wchar size.
      if(2 == sizeof(wchar_t))
      {
        dest[i]=(char)src[i];
      }
      else if(4 == sizeof(wchar_t))
      {
        dest[i]=(char)WideChar16[i];
      }
      count++;
    }

    dest[nDest - 1] = '\0';
    dest[count] = '\0';

    return count;
  }
}
/*lint +e49 +e40 +e10 +e63 */

