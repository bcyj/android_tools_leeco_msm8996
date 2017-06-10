/* =======================================================================
                              zrex_string.h
DESCRIPTION
  Substitutions for Windows system calls and runtime
  library routines.  For use with the ZREX versions
  of the core libraries.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/zrex_string.h#9 $
$DateTime: 2012/06/26 22:33:13 $
$Change: 2538781 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#ifndef _ZREX_STRING_H_
#define _ZREX_STRING_H_


//#include "parserdatadef.h"
//#include "parserinternaldefs.h"
#include "AEEStdDef.h"

#ifdef __cplusplus
extern "C" {
#endif



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
**                        Class Declarations
** ======================================================================= */
/*
///////////////////////////////////////////////////////////////////////////
//
// Standard String and widestring calls
//
///////////////////////////////////////////////////////////////////////////
*/

char *zrex_strstri( const char *string, const char *strCharSet );
char *zrex_strupr(char * string);
char *zrex_strlwr(char * string);
wchar_t *zrex_wcscpy( wchar_t *strDestination, uint32 ndstsize,const wchar_t *strSource );

size_t zrex_wcslen( const wchar_t *string );
int zrex_strcmp(const wchar_t *str1, const wchar_t *str2);
int zrex_strncmp(const wchar_t *str1, const wchar_t *str2, unsigned int n);

/*
//////////////////////////////////////////////////////////////////////////////////
//
// Windows WideChar support calls.
//
//////////////////////////////////////////////////////////////////////////////////
*/

/*
** Redefine the multi-byte calls in the core libs.
** This is only limited support.
*/
int CharToWideChar(const char * src, int nSrc, wchar_t * dest, int nDest,
                   boolean isUnicode );

#define MultiByteToWideChar(a,b,c,d,e,f) CharToWideChar(c,d,e,f, FALSE)

int WideCharToChar(const wchar_t * src, int nSrc, char * dest, int nDest );

#define WideCharToMultiByte(a,b,c,d,e,f,g,h) WideCharToChar(c,d,e,f)

#ifdef __cplusplus
}
#endif

#endif //_ZREX_STRING_H_

