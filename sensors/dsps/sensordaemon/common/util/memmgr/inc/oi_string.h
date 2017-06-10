#ifndef OI_STRING_H
#define OI_STRING_H
/**
 * @file    
 * This file contains BLUEmagic 3.0 supplied portable string.h functions.
 *
 */

/**********************************************************************************
  $AccuRev-Revision: 1170/1 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#include "oi_cpu_dep.h"
#include "oi_stddefs.h"

#if defined(USE_NATIVE_MEMCPY) || defined(USE_NATIVE_MALLOC)
#include <string.h>
#endif

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif


/* 
 * If we are using Native malloc(), we must also use
 * native ANSI string.h functions for memory manipulation.
 */
#ifdef USE_NATIVE_MALLOC
#ifndef USE_NATIVE_MEMCPY
#define USE_NATIVE_MEMCPY
#endif
#endif

#ifdef USE_NATIVE_MEMCPY

#define OI_MemCopy(to, from, size)    memcpy((to), (from), (size))
#define OI_MemSet(block, val, size)   memset((block), (val), (size))
#define OI_MemZero(block, size)       memset((block), 0, (size))
#define OI_MemCmp(s1, s2, n)          memcmp((s1), (s2), (n))
#define OI_StrLen(str)                strlen((str))
#define OI_Strcmp(s1, s2)             strcmp((s1), (s2))
#define OI_Strncmp(s1, s2, n)         strncmp((s1), (s2), (n))

#else

/*
 * OI_MemCopy
 *
 * Copy an arbitrary number of bytes from one memory address to another.
 * The underlying implementation is the ANSI memmove() or equivalant, so
 * overlapping memory copies will work correctly.
 */
void OI_MemCopy(void *To,
                void const *From,
                OI_UINT32 Size);


/*
 * OI_MemSet
 *
 * Sets all bytes in a block of memory to the same value
 */
void OI_MemSet(void *Block,
               OI_UINT8 Val,
               OI_UINT32 Size);


/*
 * OI_MemZero
 *
 * Sets all bytes in a block of memory to zero
 */
void OI_MemZero(void *Block,
                OI_UINT32 Size);


/*
 * OI_MemCmp
 * 
 * Compare two blocks of memory
 * 
 * Returns:
 *        0, if s1 == s2
 *      < 0, if s1 < s2
 *      > 0, if s1 > s2
 */
OI_INT OI_MemCmp(void const *s1, void const *s2, OI_UINT32 n);


/*
 * OI_StrLen
 *
 * Calculates the number of OI_CHARs in pStr (not including
 * the Null terminator) and returns the value.
 */
OI_UINT OI_StrLen(OI_CHAR const *pStr);

/*
 * OI_Strcmp
 * 
 * Compares two Null terminated strings
 * 
 * Returns:
 *        0, if s1 == s2
 *      < 0, if s1 < s2
 *      > 0, if s2 > s2
 */
OI_INT OI_Strcmp(OI_CHAR const *s1,
                 OI_CHAR const *s2);

/*
 * OI_Strcmp
 * 
 * Compares two Null terminated strings
 * 
 * Returns:
 *        0, if s1 == s2
 *      < 0, if s1 < s2
 *      > 0, if s2 > s2
 */
OI_INT OI_Strcmp(OI_CHAR const *s1,
                 OI_CHAR const *s2);

/*
 * OI_Strncmp
 * 
 * Compares the first "len" OI_CHARs of strings s1 and s2.
 * 
 * Returns:
 *        0, if s1 == s2
 *      < 0, if s1 < s2
 *      > 0, if s2 > s2
 */
/*
OI_INT OI_Strncmp(OI_CHAR const *s1,
                  OI_CHAR const *s2,
                  OI_UINT32      len);
*/

#endif /* USE_NATIVE_MEMCPY */

/*
 * OI_Strlcpy
 *
 * Safely copies the pStr string to pDest. Copies at most (len - 1) characters from pStr,
 * NUL-terminating the result. The caller can compare the return value against the len parameter to
 * determine if the string was truncated.
 *
 * @param pDest   A pointer to the destination string buffer which should be long enough to hold 
 *                (len + 1) characters.
 *
 * @param pStr    A pointer to the string to be copied to pDest.
 *
 * @param len     A value that is less than the size of the destination buffer referenced by pDest.
 *
 * @return   The length of the resultant string in pDest or the length that string would have had if
 *           there was room. In the latter case the return value with be >= len.
 */
/*
OI_UINT OI_Strlcpy(OI_CHAR *pDest,
                   OI_CHAR const *pStr,
                   OI_UINT len);

*/
/*
 * OI_Strlcat
 *
 * Safely concatenates the pStr string to the end of pDest. Appends at most (len - OI_Strlen(pDest) - 1)
 * characters from pStr, NUL-terminating the result. The caller can compare the return value against
 * the len parameter to determine if the string was truncated.
 *
 * @param pDest   A pointer to the destination string buffer which should be long enough to hold 
 *                (len + 1) characters.
 *
 * @param pStr    A pointer to the string to be appended to pDest.
 *
 * @param len     A value that is less than the size of the destination buffer referenced by pDest.
 *
 * @return   The length of the resultant string in pDest or the length that string would have had if
 *           there was room. In the latter case the return value with be >= len.
 */
/*
OI_UINT OI_Strlcat(OI_CHAR *pDest,
                   OI_CHAR const *pStr,
                   OI_UINT len);

*/
/*
 * OI_StrcmpInsensitive
 * 
 * Compares two Null terminated strings, treating
 * the Upper and Lower case of 'A' through 'Z' as
 * equivilent.
 * 
 * Returns:
 *        0, if s1 == s2
 *      < 0, if s1 < s2
 *      > 0, if s2 > s2
 */
/*
OI_INT OI_StrcmpInsensitive(OI_CHAR const *s1,
                            OI_CHAR const *s2);
*/
/*
 * OI_StrncmpInsensitive
 * 
 * Compares the first "len" OI_CHARs of strings s1 and s2,
 * treating the Upper and Lower case of 'A' through 'Z' as
 * equivilent.
 * 
 * 
 * Returns:
 *        0, if s1 == s2
 *      < 0, if s1 < s2
 *      > 0, if s2 > s2
 */
/*
OI_INT OI_StrncmpInsensitive(OI_CHAR const *s1,
                             OI_CHAR const *s2,
                             OI_UINT        len);
*/


#ifdef __cplusplus
}
#endif

/** @} */

/*****************************************************************************/
#endif /* OI_STRING_H */

