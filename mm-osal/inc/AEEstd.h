#ifndef AEESTD_H
#define AEESTD_H
/*====================================================================
        Copyright © 2012 Qualcomm Technologies, Inc.
               All Rights Reserved.
            Qualcomm Technologies Confidential and Proprietary
======================================================================

DESCRIPTION:  Standard library; general-purpose utility functions.

====================================================================*/
#include "AEEVaList.h"
#include "AEEStdDef.h"
#include <stdlib.h>
#include <string.h>

#define STD_CONSTRAIN( val, min, max ) (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val))
#define STD_BETWEEN( val, minGE, maxLT ) \
                   ( (unsigned)((unsigned)(val) - (unsigned)(minGE)) < \
                   (unsigned)((unsigned)(maxLT) - (unsigned)(minGE)) )
#define STD_ARRAY_SIZE(a)             ((int)((sizeof((a))/sizeof((a)[0]))))
#define STD_ARRAY_MEMBER(p,a) (((p) >= (a)) && ((p) < ((a) + STD_ARRAY_SIZE(a))))

#define STD_SIZEOF(x)                 ((int)sizeof(x))
#define STD_OFFSETOF(type,member)     (((char*)(&((type*)1)->member))-((char*)1))

#define STD_RECOVER_REC(type,member,p) ((void)((p)-&(((type*)1)->member)),\
                                        (type*)(void*)(((char*)(void*)(p))-STD_OFFSETOF(type,member)))
#define STD_MIN(a,b)   ((a)<(b)?(a):(b))
#define STD_MAX(a,b)   ((a)>(b)?(a):(b))
//lint -emacro(545,STD_ZEROAT)
#define STD_ZEROAT(p)  std_memset((p), 0, sizeof(*p))

//
// Error codes
//
#define STD_NODIGITS   1
#define STD_NEGATIVE   2
#define STD_OVERFLOW   3
#define STD_BADPARAM   4


#ifdef __cplusplus
#include <cstdio>

extern "C" {
#endif /* #ifdef __cplusplus */

//Version function
extern ssize_t           std_getversion(char *pcDst, ssize_t nDestSize);

//String functions
extern size_t            std_strlen(const char* s1);
extern ssize_t           std_strcmp(const char *s1, const char *s2);
extern ssize_t           std_strncmp(const char *s1, const char *s2, ssize_t n);
extern ssize_t           std_stricmp(const char *s1, const char *s2);
extern ssize_t           std_strnicmp(const char *s1, const char *s2, ssize_t n);
extern ssize_t           std_strlcpy(char *pcDst, const char *pszSrc, ssize_t nDestSize);
extern ssize_t           std_strlcat(char *pcDst, const char *pszSrc, ssize_t nDestSize);
extern char *        std_strstr(const char *pszString, const char *pszSearch);

//Character functions
extern char          std_tolower(char c);
extern char          std_toupper(char c);

// Mem functions
extern ssize_t       std_memcmp(const void* p1, const void* p2, ssize_t n);
extern void*         std_memset(void* s, int c, ssize_t n);
extern void *        std_memmove(void* s, const void* c, ssize_t n);
extern void *        std_memchr(const void* s, int c, ssize_t n);
extern void *        std_memstr(const char* cpHaystack, const char* cpszNeedle, ssize_t nHaystackLen);
extern void *        std_memrchr(const void* s, int c, ssize_t n);
extern void *        std_memrchrbegin(const void* p, int c, ssize_t nLen);
extern void *        std_memchrend(const void* cpcSrch, int c, ssize_t nLen);
extern void *        std_memchrsend(const void *cpSrch, const char* cpszChars, ssize_t nLen);

//Other String functions
extern char *        std_strchr(const char* s, int c);
extern char *        std_strchrs(const char* sSrch, const char *sChars);
extern char *        std_strrchr(const char* s, int c);
extern char *        std_strchrend(const char *cpszSrch, char c);
extern char *        std_strchrsend(const char* s, const char* cpszSrch);
extern char *        std_strends(const char* cpsz, const char* cpszSuffix);
extern char *        std_striends(const char* cpsz, const char* cpszSuffix);
extern char *        std_strbegins(const char* cpsz, const char* cpszPrefix);
extern char *        std_stribegins(const char* cpsz, const char* cpszPrefix);

//Wide char string functions
extern ssize_t        std_wstrlen(const AECHAR *s);
extern ssize_t        std_wstrlcpy(AECHAR *pcDst, const AECHAR *pszSrc, int nDestSize);
extern ssize_t        std_wstrlcat(AECHAR *pcDst, const AECHAR *pszSrc, int nDestSize);

//Path functions
extern ssize_t        std_makepath(const char *cpszDir,
                                  const char *cpszFile,
                                  char *pszDest, int nDestSize);
extern char *        std_splitpath(const char *cpszPath, const char *cpszDir);
extern char *        std_cleanpath(char *pszPath);
extern char *        std_basename(const char *pszPath);

//Inet functions, number functions
extern uint32        std_scanul(const char *pchBuf, int64 nRadix,
                                const char **ppchEnd, int *pnError);

// Rand functions
extern unsigned      std_rand_next(unsigned uRand);


// printf functions
extern int           std_vstrlprintf(char *pszDest, int nDestSize,
                                     const char *pszFmt, AEEVaList args);

#define std_strlprintf snprintf
//extern int           std_strlprintf(char *pszDest, int nDestSize,
//                                    const char *pszFmt, ...);

// endian swapping functions
extern int           std_CopyLE(void *pvDest,       int nDestSize,
                                const void *pvSrc,  int nSrcSize,
                                const char *pszFields);

extern int           std_CopyBE(void *pvDest,      int nDestSize,
                                const void *pvSrc, int nSrcSize,
                                const char *pszFields);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */


#define STD_SWAPS(us) \
   ( (uint16)( ( ( (us) & 0xff ) << 8 ) + (((us) & 0xff00) >> 8) ) )


static __inline unsigned short std_swaps(unsigned short us)
{
   return STD_SWAPS(us);
}

/* note, STD_SWAPL() requires that ul be an l-value, and destroyable.
   this macro is not intended for use outside AEEstd.h */
#define STD_SWAPL(ul) \
    (((ul) = (((ul) & 0x00ff00ff) << 8) | (((ul)>>8) & 0x00ff00ff)),(((ul) >> 16) | ((ul) << 16)))

static __inline unsigned long std_swapl(unsigned long ul)
{
   return STD_SWAPL(ul);
}

#ifdef AEE_BIGENDIAN
#  define STD_HTONL(u)     (u)
#  define STD_HTONS(u)     (u)
#  define STD_HTOLEL(u)    (STD_SWAPL(u))
#  define STD_HTOLES(u)    (STD_SWAPS(u))
#else
#  define STD_HTONL(u)     (STD_SWAPL(u))
#  define STD_HTONS(u)     (STD_SWAPS(u))
#  define STD_HTOLEL(u)    (u)
#  define STD_HTOLES(u)    (u)
#endif

static __inline unsigned short std_letohs(unsigned short us)
{
   return STD_HTOLES(us);
}

static __inline unsigned short std_htoles(unsigned short us)
{
   return STD_HTOLES(us);
}

static __inline unsigned long std_letohl(unsigned long ul)
{
   return STD_HTOLEL(ul);
}

static __inline unsigned long std_htolel(unsigned long ul)
{
   return STD_HTOLEL(ul);
}

static __inline unsigned short std_ntohs(unsigned short us)
{
   return STD_HTONS(us);
}

static __inline unsigned short std_htons(unsigned short us)
{
   return STD_HTONS(us);
}

static __inline unsigned long std_ntohl(unsigned long ul)
{
   return STD_HTONL(ul);
}

static __inline unsigned long std_htonl(unsigned long ul)
{
   return STD_HTONL(ul);
}


#undef STD_HTONL   // private macro; not exported as a supported API
#undef STD_HTONS   // private macro; not exported as a supported API
#undef STD_HTOLEL  // private macro; not exported as a supported API
#undef STD_HTOLES  // private macro; not exported as a supported API
#undef STD_SWAPS   // private macro; not exported as a supported API
#undef STD_SWAPL   // private macro; not exported as a supported API

/*
=======================================================================
MACROS DOCUMENTATION
=======================================================================

STD_CONTSTRAIN()

Description:
  STD_CONTSTRAIN() constrains a number to be between two other numbers.

Definition:
   STD_CONSTRAIN( val, min, max ) \
          (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val))

Parameters:
  val: number to constrain
  min: number to stay greater than or equal to
  max: number to stay less than or equal to

Evaluation Value:
   the constrained number

=======================================================================

STD_BETWEEN()

Description:
    STD_BETWEEN() tests whether a number is between two other numbers.

Definition:
    STD_BETWEEN( val, minGE, maxLT ) \
               ((unsigned)((unsigned)(val) - (unsigned)(minGE)) < \
                (unsigned)((unsigned)(maxLT) - (unsigned)(minGE)))

Parameters:
     val: value to test
     minGE: lower bound
     maxLT: upper bound

Evaluation Value:
     1 if val >= minGE and val < maxLT

=======================================================================

STD_ARRAY_SIZE()

Description:
   STD_ARRAY_SIZE() gives the number of elements in a statically allocated array.

Definition:
    STD_ARRAY_SIZE(a) (sizeof((a))/sizeof((a)[0]))

Parameters:
    a: array to test

Evaluation Value:
    number of elements in a

=======================================================================

STD_ARRAY_MEMBER()

Description:
   STD_ARRAY_MEMBER() tests whether an item is a member of a statically allocated array.

Definition:
   STD_ARRAY_MEMBER(p,a) (((p) >= (a)) && ((p) < ((a) + STD_ARRAY_SIZE(a))))

Parameters:
    p: item to test
    a: array to check

Evaluation Value:
    1 if p is in a

=======================================================================

STD_OFFSETOF()

Description:
  STD_OFFSETOF() gives the offset of member of a struct.

Definition:
   STD_OFFSETOF(type,member) (((char *)(&((type *)0)->member))-((char *)0))

Parameters:
    type: structured type
    member: name of member in the struct

Evaluation Value:
    offset of member (in bytes) in type

=======================================================================

STD_RECOVER_REC()

Description:
  STD_RECOVER_REC() provides a safe cast from a pointer to a member
    of a struct to a pointer to the containing struct

Definition:
  STD_RECOVER_REC(type,member,p) ((type*)(((char*)(p))-STD_OFFSETOF(type,member)))

Parameters:
    type: structured type
    member: name of member in the struct
    p: pointer to the member of the struct

Evaluation Value:
    a pointer of type type to the containing struct

=======================================================================

STD_MIN()

Description:
   STD_MIN() finds the smaller of two values.

Definition:
   STD_MIN(a,b)   ((a)<(b)?(a):(b))

Parameters:
    a, b: values to compare

Evaluation Value:
    smaller of a and b

=======================================================================

STD_MAX()

Description:
  STD_MAX() finds the larger of two values.

Definition:
   STD_MAX(a,b)   ((a)>(b)?(a):(b))

Parameters:
    a, b: values to compare

Evaluation Value:
    larger of a and b

=======================================================================

STD_ZEROAT()

Description:
   STD_ZEROAT() zero-initializes the contents of a typed chunk of memory.

Definition:
   STD_ZEROAT(p)  std_memset((p), 0, sizeof(*p))

Parameters:
    p: the chunk to initialize

Evaluation Value:
    p

=====================================================================
INTERFACES DOCUMENTATION
=======================================================================

std Interface

   This library provides a set of general-purpose utility functions.
   Functionality may overlap that of a subset of the C standard library, but
   this library differs in a few respects:

   - Functions are fully reentrant and avoid use of static variables.

   - The library can be supported consistently across all environments.
   Compiler-supplied libraries sometimes behave inconsistently and are
   unavailable in some environments.

   - Omits "unsafe" functions.  C standard library includes many functions
   that are best avoided entirely: strcpy, strcat, strtok, etc.


=======================================================================

std_getversion()

Description:

   The std_getversion() copies the stdlib version to pcDst.  This function
   takes the size of the destination buffer as an argument and guarantees
   to zero-terminate the result and not to overflow the nDestSize size.

   This function copies up to size-1 characters from the stdlib version
   string to pcDest and NUL-terminates the pcDest string.

Prototype:
   int  std_getversion(char *pcDst, int nDestSize)


Parameters:
   pcDst :     Destination string
   nDestSize:  Size of the destination buffer in bytes

Return Value:

   Returns the length of the version string (in characters).

=======================================================================

std_strlen()

Description:
   The std_strlen() computes the length of the given string.

Prototype:
   int std_strlen(const char *cpszStr)

Parameters:
   cpszStr : String whose length will be computed

Return Value:
   Length of the string in characters that precede the terminating NUL
   character.

=======================================================================

std_strcmp()

Description:
   The std_strcmp() compares two NUL-terminated character strings.
   Comparison is strictly by byte values with no character set
   interpretation.

Prototype:

   int std_strcmp(const char *s1, const char *s2);

Parameters:
   s1, s2: strings to compare

Return Value:
   0 if strings are the same
   < 0 if s1 is less than s2
   > 0 if s1 is greater than s2

=======================================================================

std_strncmp()

Description:
   The std_strncmp() compares at most n bytes of two NUL-terminated character strings.

Prototype:

   int std_strncmp(const char *s1, const char *s2, int n);

Parameters:
   s1, s2: strings to compare
   n: maximum number of bytes to compare.  if either s1 or s2 is
       shorter than n, the function terminates there

Return Value:
   0 if strings are the same
   < 0 if s1 is less than s2
   > 0 if s1 is greater than s2

=======================================================================

std_stricmp()

Description:
   The std_stricmp() compares two NUL-terminated character strings, case-folding any
   ASCII characters.

Prototype:

   int std_stricmp(const char *s1, const char *s2);

Parameters:
   s1, s2: strings to compare

Return Value:
   0 if strings are the same
   < 0 if s1 is less than s2
   > 0 if s1 is greater than s2

=======================================================================

std_strnicmp()

Description:
   The std_strnicmp() compares at most n bytes of 2 NUL-terminated character strings,
   case-folding any ASCII characters.

Prototype:

   int std_strnicmp(const char *s1, const char *s2, int n);

Parameters:
   s1, s2: strings to compare
   n: maximum number of bytes to compare.  if either s1 or s2 is
       shorter than n, the function terminates there

Return Value:
   0 if strings are the same
   < 0 if s1 is less than s2
   > 0 if s1 is greater than s2

=======================================================================

std_strlcpy()

Description:

   The std_strlcpy() copies pszSrc string to the pcDst.  It is a safer
   alternative to strcpy() or strncpy().  This function takes the size of the
   destination buffer as an argument and guarantees to NUL-terminate the
   result and not to overflow the nDestSize size.

   This function copies up to nDestSize-1 characters from the pszSrc string
   to pcDest and NUL-terminates the pcDest string.

Prototype:
   int std_strlcpy(char *pcDst, const char *pszSrc, int nDestSize)

Parameters:
   pcDst :     Destination string
   pcSrc :     Source string
   nDestSize:  Size of the destination buffer in bytes

Return Value:

   Returns the length of the string (in characters) it tried to create,
   which is same as length of pszSrc.

   Example:

   {
      char buf[64];
      if (std_strlcpy(buf, file_name, STD_ARRAY_SIZE(buf) >=
          STD_ARRAY_SIZE(buf)) {
         //Truncated -- Handle overflow....
      }
   }

Comment:

   Unlike strlcpy, std_strlcpy accepts an integer size and does nothing when a
   negative value is passed.  When passing valid sizes for objects on our
   supported platforms, this should not result in any observed difference.
   However, calling strlcpy() with UINT_MAX will result in the entire source
   string being copied, whereas std_strlcpy() will do nothing.  Passing INT_MAX
   to str_strlcpy() will achieve the same result (although both these cases are
   bad practice since they defeat bounds checking).


=======================================================================

std_strlcat()

Description:

   The std_strlcat() function concatenates a string to a string already
   residing in a buffer.  It is a safer alternative to strcat() or strncat().
   This function takes the size of the destination buffer as an argument and
   guarantees not to create an improperly terminated string and not to
   overflow the nDestSize size.

   This function appends pszSrc to pcDst, copying at most nDestSize minus
   the length of the string in pcDest minus 1 bytes, always NUL-terminating
   the result.

   For compatibility with "strlcat()", std_strlcat() does *not* zero-terminate
   the destination buffer in cases where the buffer lacks termination on entry
   to the function.  Do not rely on std_strlcat() to zero-terminate a buffer
   that is not already zero-terminated; instead ensure that the buffer is
   properly initialized using std_strlcpy() or some other means.

Prototype:

   int std_strlcat(char *pcDst, const char *pszSrc, int nDestSize)

Parameters:

   pcDst :     Destination string
   pcSrc :     Source string
   nDestSize:  Size of the destination buffer in bytes

Return Value:

   Returns the length of the string (in characters) it tried to create,
   which is same as length of pszSrc plus the length of pszDest.

   Example:

   {
      char buf[64];
      if (std_strlcat(buf, file_name, STD_ARRAY_SIZE(buf) >=
          STD_ARRAY_SIZE(buf)) {
         //Truncated -- Handle overflow....
      }
   }


=======================================================================

std_strstr()

Description:
   The std_strstr() finds the first occurrence of a substring in a string.

Prototype:

   char * std_strstr(const char *pszString, const char *pszSearch);

Parameters:
   pszString: string to search
   pszSearch: sub string to search for

Return Value:
   a pointer to the first character in the first occurrence of
       the substring if found, NULL otherwise

=======================================================================

std_tolower()

Description:
   The std_tolower() converts an uppercase letter to the corresponding
   lowercase letter.

Prototype:
   char std_tolower(char c);

Parameters:
   c: is a character.

Return Value:
   the corresponding lowercase letter if c is an ASCII character whose
   value is representable as an uppercase letter, else the same character
   c is returned.

=======================================================================

std_toupper()

Description:
   The std_toupper() converts an lowercase letter to the corresponding
   uppercase letter.

Prototype:
   char std_toupper(char c);

Parameters:
   c: is a character.

Return Value:
   the corresponding uppercase letter if c is an ASCII character whose
   value is representable as an lowercase letter, else the same character
   c is returned.

=======================================================================

std_memset()

Description:
   The std_memset() sets each byte in a block of memory to a value.

Prototype:

   void *std_memset(void *p, int c, int nLen);

Parameters:
   p: memory block to set
   c: value to set each byte to
   nLen: size of p in bytes

Return Value:
   p

=======================================================================

std_memmove()

Description:
   The std_memmove() copies a block of memory from one buffer to another.

Prototype:

   void *std_memmove(void *pTo, const void *cpFrom, int nLen);

Parameters:
   pTo: destination buffer
   cpFrom: source buffer
   nLen: number of bytes to copy

Return Value:
   pTo

=======================================================================

std_memcmp()

Description:
   The std_memcmp() compares two memory buffers, byte-wise.

Prototype:

   int std_memcmp(const void *a, const void *b, int length);

Parameters:
   a, b: buffers to compare
   length: number of bytes to compare

Return Value:
   0 if buffers are the same for nLength
   < 0 if a is less than b
   > 0 if a is greater than b

=======================================================================

std_memchr()

Description:
   The std_memchr() finds the first occurrence of a character in a memory
   buffer.

Prototype:

   void *std_memchr(const void* s, int c, int n);

Parameters:
   s: buffer to search
   c: value of byte to look for
   n: size of s in bytes

Return Value:
   a pointer to the occurrence of c, NULL if not found

=======================================================================

std_memstr()

Description:
   The std_memstr() finds the first occurrence of a substring in a memory
   buffer.

Prototype:

   void *std_memstr(const char* cpHaystack, const char* cpszNeedle,
                    int nHaystackLen);

Parameters:
   cpHaystack: buffer to search
   cpszNeedle: NUL-terminated string to search for
   nHaystackLen: size of cpHaystack in bytes

Return Value:
   a pointer to the first occurrence of cpszNeedle if found,
       NULL otherwise

Comments:
   None

Side Effects:
   None

See Also:
   None

=======================================================================

std_memrchr()

Description:

   The std_memrchr() finds the last occurrence of a character in a memory
   buffer.

Prototype:

   void *std_memrchr(const void* s, int c, int n);

Parameters:
   s: buffer to search
   c: value of byte to look for
   n: size of s in bytes

Return Value:
   a pointer to the last occurrence of c, NULL if not found

=======================================================================

std_memrchrbegin()

Description:
   The std_memrchrbegin() finds the last occurrence of a character in a
   memory buffer.

Prototype:

   void *std_memrchrbegin(const void* s, int c, int n);

Parameters:
   s: buffer to search
   c: value of byte to look for
   n: size of s in bytes

Return Value:
   a pointer to the last occurrence of c, or s if not found

=======================================================================

std_memchrend()

Description:
   The std_memchrend() finds the first occurrence of a character in a
   memory buffer.

Prototype:

   void *std_memchrend(const void* s, int c, int n);

Parameters:
   s: buffer to search
   c: value of byte to look for
   n: size of s in bytes

Return Value:
   a pointer to the occurrence of c, s + n if not found

=======================================================================
std_memchrsend()

Description:
   The std_memchrsend() finds the first occurrence of any character in a
   NUL-terminated list of characters in a memory buffer.

Prototype:

   void *std_memchrend(const void* s, const char* cpszChars, int n);

Parameters:
   s: buffer to search
   cpszChars: characters to look for
   n: size of s in bytes

Return Value:
   a pointer to the first occurrence of one of cpszChars, s + n if not found

=======================================================================

std_strchr()

Description:
   The std_strchr() finds the first occurrence of a character in a
   NUL-terminated string.

Prototype:

   char *std_strchr(const char* s, int c);

Parameters:
   s: string to search
   c: char to search for

Return Value:
   pointer to first occurrence, NULL if not found

=======================================================================

std_strchrs()

Description:
   The std_strchrs() searches s, a NUL-terminated string, for the first
   occurrence of any characters in cpszSrch, a NUL-terminated list of
   characters.

Prototype:

   char *std_strchrs(const char* s, const char *cpszSrch);

Parameters:
   s: string to search
   cpszSrch: a list of characters to search for

Return Value:
   first occurrence of any of cpszSrch, NULL if not found

=======================================================================

std_strrchr()

Description:
   The std_strrchr() finds the last occurrence of a character in a
   NUL-terminated string.

Prototype:

   char *std_strrchr(const char* s, int c);

Parameters:
   s: string to search
   c: char to search for

Return Value:
   pointer to last occurrence, NULL if not found

=======================================================================

std_strchrend()

Description:
   The std_strchrend() finds the first occurrence of a character in a
   NUL-terminated string.

Prototype:

   char *std_strchrend(const char* s, int c);

Parameters:
   s: string to search
   c: char to search for

Return Value:
   pointer to first occurrence, s + std_strlen(s) if not found

=======================================================================

std_strchrsend()

Description:
   The std_strchrsend() searches s, a NUL-terminated string, for the first
   occurrence of any characters in cpszSrch, a NUL-terminated list of
   characters.

Prototype:

   char *std_strchrsend(const char* s, const char* cpszSrch);

Parameters:
   s: string to search
   cpszSrch: a list of characters to search for

Return Value:
   first occurrence of any of cpszSrch or s+strlen(s) if not found

=======================================================================

std_strends()

Description:
   The std_strends() tests whether a string ends in a particular suffix.

Prototype:

   char *std_strends(const char* cpsz, const char* cpszSuffix);

Parameters:
   cpsz: string to test
   cpszSuffix: suffix to test for

Return Value:
   the first character of cpsz+std_strlen(cpsz)-std_strlen(cpszSuffix)
     if cpsz ends with cpszSuffix.  NULL otherwise.

=======================================================================

std_striends()

Description:
   The std_striends() tests whether a string ends in a particular suffix,
   case-folding ASCII characters.

Prototype:

   char *std_striends(const char* cpsz, const char* cpszSuffix);

Parameters:
   cpsz: string to test
   cpszSuffix: suffix to test for

Return Value:
   the first character of cpsz+std_strlen(cpsz)-std_strlen(cpszSuffix)
     if cpsz ends with cpszSuffix.  NULL otherwise.

=======================================================================

std_strbegins()

Description:
   The std_strbegins() tests whether a string begins with a particular
   prefix string.

Prototype:

   char *std_strbegins(const char* cpsz, const char* cpszPrefix);

Parameters:
   cpsz: string to test
   cpszPrefix: prefix to test for

Return Value:
   cpsz + std_strlen(cpszPrefix) if cpsz does begin with cpszPrefix,
     NULL otherwise

=======================================================================

std_stribegins()

Description:
   The std_stribegins() tests whether a string begins with a particular
   prefix string, case-folding ASCII characters.

Prototype:

   char *std_stribegins(const char* cpsz, const char* cpszPrefix);

Parameters:
   cpsz: string to test
   cpszPrefix: prefix to test for

Return Value:
   cpsz + std_strlen(cpszPrefix) if cpsz does begin with cpszPrefix,
     NULL otherwise


=======================================================================

std_wstrlcpy()

Description:

   The std_wstrlcpy() function copies a string.  It is equivalent to
   str_strlcpy() except that it operates on wide (16-bit) character strings.
   See std_strlcpy() for details.


Prototype:

   int std_wstrlcpy(AECHAR *pcDest, const AECHAR *pszSrc, int nDestSize);

Parameters:
   pcDst: destination string
   pszSrc: source string
   int nDestSize: size of pcDest __in AECHARs__

Return Value:
   Returns the length of the string (in AECHARs) it tried to create,
   which is same as length of pszSrc.

   Example:

   {
      AECHAR buf[64];
      if (std_wstrlcpy(buf, file_name, STD_ARRAY_SIZE(buf)) >=
          STD_ARRAY_SIZE(buf)) {
         //Truncated -- Handle overflow....
      }
   }

=======================================================================

std_wstrlcat()

Description:

   The std_wstrlcat() function concatenates two strings.  It is equivalent to
   std_strlcat() except that it operates on wide (16-bit) character strings.
   See std_strlcat() for more information.

Prototype:
   int std_wstrlcat(AECHAR *pcDst, const AECHAR *pszSrc, int nDestSize)

Parameters:
   pcDst[out]: Destination string
   pcSrc :     Source string
   nDestSize:  Size of the destination buffer in AECHARs

Return Value:
   Returns the length of the string (in AECHARs) it tried to create,
   which is same as length of pszSrc + the length of pszDest.

   Example:

   {
      char buf[64];
      if (std_wstrlcat(buf, file_name, STD_ARRAY_SIZE(buf)) >=
          STD_ARRAY_SIZE(buf)) {
         //Truncated -- Handle overflow....
      }
   }

=======================================================================

std_makepath()

Description:
   The std_makepath() constructs a path from a directory portion and a file
   portion, using forward slashes, adding necessary slashes and deleting extra
    slashes.  This function guarantees NUL-termination of pszDest

Prototype:

   int std_makepath(const char *cpszDir, const char *cpszFile,
                    char *pszDest, int nDestSize)

Parameters:
   cpszDir: directory part
   cpszFile: file part
   pszDest: output buffer
   nDestSize: size of output buffer in bytes

Return Value:
   the required length to construct the path, not including
   NUL-termination

Comments:
   The following list of examples shows the strings returned by
   std_makepath() for different paths.

Example:

   cpszDir  cpszFile  std_makepath()
   ""        ""           ""
   ""        "/"          ""
   "/"       ""           "/"
   "/"       "/"          "/"
   "/"       "f"          "/f"
   "/"       "/f"         "/f"
   "d"       "f"          "d/f"
   "d/"      "f"          "d/f"
   "d"       "/f"         "d/f"
   "d/"      "/f"         "d/f"



=======================================================================

std_splitpath()

Description:
   The std_splitpath() finds the filename part of a path given an inclusive
   directory, tests for cpszPath being in cpszDir. The forward slashes are
   used as directory delimiters.

Prototype:

   char *std_splitpath(const char *cpszPath, const char *cpszDir);

Parameters:
   cpszPath: path to test for inclusion
   cpszDir: directory that cpszPath might be in

Return Value:
   the part of cpszPath that actually falls beneath cpszDir, NULL if
   cpszPath is not under cpszDir

Comments:
   The std_splitpath() is similar to std_strbegins(), but it ignores trailing
   slashes on cpszDir, and it returns a pointer to the first character of
   the subpath.

   The return value of std_splitpath() will never begin with a '/'.

   The following list of examples shows the strings returned by
   std_splitpath() for different paths.

Example:
   cpszPath cpszDir  std_splitpath()
   ""        ""           ""
   ""        "/"          ""
   "/"       ""           ""
   "/"       "/"          ""
   "/d"      "d"          null
   "/d"      "/"          "d"
   "/d/"     "/d"         ""
   "/d/f"    "/"          "d/f"
   "/d/f"    "/d"         "f"
   "/d/f"    "/d/"        "f"



=======================================================================

std_cleanpath()

Description:
   The std_cleanpath() removes double slashes, ".", and ".." from
   slash-delimited paths,. It operates in-place.

Prototype:

   char *std_cleanpath(char *pszPath);

Parameters:
   pszPath[in/out]: path to "clean"

Return Value:
   pszPath

Comments:
   Passing an "fs:/" path to this function may produce undesirable
   results.  This function assumes '/' is the root.

Examples:
       pszPath  std_cleanpath()
         "",           "",
         "/",          "/",

         // here"s, mostly alone
         "./",         "/",
         "/.",         "/",
         "/./",        "/",

         // "up"s, mostly alone
         "..",         "",
         "/..",        "/",
         "../",        "/",
         "/../",       "/",

         // fun with x
         "x/.",        "x",
         "x/./",       "x/",
         "x/..",       "",
         "/x/..",      "/",
         "x/../",      "/",
         "/x/../",     "/",
         "/x/../..",   "/",
         "x/../..",    "",
         "x/../../",   "/",
         "x/./../",    "/",
         "x/././",     "x/",
         "x/.././",    "/",
         "x/../.",     "",
         "x/./..",     "",
         "../x",       "/x",
         "../../x",    "/x",
         "/../x",      "/x",
         "./../x",     "/x",

         // double slashes
         "//",         "/",
         "///",        "/",
         "////",       "/",
         "x//x",       "x/x",


Side Effects:
   None

See Also:
   None


=======================================================================

std_basename()

Description:
   The std_basename() returns the filename part of a string,
   assuming '/' delimited filenames.

Prototype:

   char *std_basename(const char *cpszPath);

Parameters:
   cpszPath: path of interest

Return Value:
   pointer into cpszPath that denotes part of the string immediately
   following the last '/'

Examples:
     cpszPath       std_basename()
         ""            ""
         "/"           ""
         "x"           "x"
         "/x"          "x"
         "y/x"         "x"
         "/y/x"        "x"

=======================================================================

std_rand_next()

Description:
  The std_rand_next() generates pseudo-random bytes.

Prototype:

   unsigned std_rand_next(unsigned uRand);

Parameters:
   uRand: a seed for the pseudo-random generator

Return Value:
   the next value in the generator from uRand

Comments:
   for best results, this function should be called with its last
   generated output.

   This is an example of code to generate 256 bytes of pseudo-random data.

   This is not crypto quality and should not be used for key generation
   and the like.

Example:
   {
       unsigned rand_buf[256/sizeof(unsigned)];
       int      i;
       unsigned uLast = std_rand_next(uCurrentTime);
       for (i = 0; i < STD_ARRAY_SIZE(rand_buf); i++) {
          rand_buf[i] = (uLast = std_rand_next(uLast));
       }
   }


=======================================================================

std_CopyLE()

Description:

   The std_CopyLE() function copies data while translating numeric values
   between host byte ordering and "little endian" byte ordering.

   pvDest and pvSrc are NOT required to be 16 or 32-bit word aligned.

   Behavior is undefined when the destination and source arrays overlap,
   except in the special case where pvDest and pvSrc are equal.  In that case,
   std_CopyLE() modifies the buffer in-place.

   When the target byte ordering (little endian) matches the host byte
   ordering, in-place translations reduce to a no-op, and copies are
   delegated directly to std_memmove().


Prototype:
   int std_CopyLE(void *pvDest, int nDestSize,
                  const void *pvSrc,  int nSrcSize,
                  const char *pszFields);

Parameters:
   pvDest:    Pointer to destination buffer.
   nDestSize: Size of the destination buffer.
   pvSrc:     Pointer to buffer containing source data.
   nSrcSize:  Size of source data.
   pszFields: Description of the fields that comprise the source data.

              Each field size is given by a positive decimal integer or one of
              the following characters: "S", "L", "Q", or "*".  The letters
              denote fields that should be converted to the desired byte
              ordering:

===pre>
                S : a 2 byte (16 bit) value.
                L : a 4 byte (32 bit) value.
                Q : a 8 byte (64 bit) value.
===/pre>

              An integer gives a number of bytes and "*" represents the
              remainder of the pvSrc[] buffer.  No reordering is performed on
              data in these fields.

              Comparisons are case-sensitive.  Behavior is undefined when
              other characters are supplied in pszFields.

              For example: "L12S*" would be appropriate to copy a structure
              containing a uint32 followed by a 12 byte character array,
              followed by a uint16, followed by an arbitrary amount of
              character data.

              If nSrcSize is greater than the structure size (total of all the
              sizes in pszFields[]) then pvSrc[] is treated as an array of
              structures, each of which is described by pszFields.

Return Value:

   The number of bytes actually copied or translated in-place.  This will be
   the smaller of nDestSize and nSrcSize, or zero if one of them are negative.


=======================================================================

std_CopyBE()

Description:

   The std_CopyBE() function has the same semantics as std_CopyLE() except it
   copies between host byte ordering and big-endian ("network") byte order.

   See std_CopyLE() for more details.


Prototype:
   void *std_CopyBE(void *pvDest, const void *pvSrc,
                           int cbDest, int nItems, const char *pszFields);

Parameters:
   pvDest:    Pointer to destination buffer.
   nDestSize: Size of the destination buffer.
   pvSrc:     Pointer to buffer containing source data.
   nSrcSize:  Size of source data.
   pszFields: Description of the fields that comprise the source data,
              as defined in std_CopyLE.

Return Value:

   The number of bytes actually copied or translated in-place.  This will be
   the smaller of nDestSize and nSrcSize, or zero if one of them are negative.

=======================================================================

std_swapl()

Description:
   The std_swapl() changes endianness of an unsigned long.

Prototype:

   unsigned long std_swapl(unsigned long ul)

Parameters:
   ul: input unsigned long

Return Value:
   ul, reversed in byte-ordering

=======================================================================

std_swaps()

Description:
   The std_swaps() changes endianness of an unsigned short.

Prototype:

   unsigned short std_swaps(unsigned short us)

Parameters:
   us: input unsigned short

Return Value:
   us, reversed in byte-ordering

=======================================================================

std_letohs()

Description:
   The std_letohs() changes a short from little-endian to host byte order.

Prototype:

   unsigned short std_letohs(unsigned short us)

Parameters:
   us: short to convert

Return Value:
   us converted from little-endian to host byte order.  If the
     host is little endian, just returns us

=======================================================================

std_htoles()

Description:
   The std_htoles() converts a short from host byte-order to little-endian.

Prototype:

   unsigned short std_htoles(unsigned short us)

Parameters:
   us: short to convert

Return Value:
   us converted from host byte order to little-endian.  If the
   host is little endian, just returns us

=======================================================================

std_letohl()

Description:
   The std_letohl() changes a long from little-endian to host byte order.

Prototype:

   unsigned long std_letohl(unsigned long ul)

Parameters:
   ul: long to convert

Return Value:
   ul converted from little-endian to host byte order.  If the
   host is little endian, just returns ul

=======================================================================

std_htolel()

Description:
   The std_htolel() converts a long from host byte-order to little-endian.

Prototype:

   unsigned long std_htolel(unsigned long ul)

Parameters:
   ul: long to convert

Return Value:
   ul converted from host byte order to little-endian.  If the
   host is little endian, just returns ul


foo
=======================================================================

std_ntohs()

Description:
   The std_ntohs() changes a short from big-endian to host byte order.

Prototype:

   unsigned short std_ntohs(unsigned short us)

Parameters:
   us: short to convert

Return Value:
   us converted from big-endian to host byte order.  If the
   host is big endian, just returns us

=======================================================================

std_htons()

Description:
   The std_htons() converts a short from host byte-order to big-endian.

Prototype:

   unsigned short std_htons(unsigned short us)

Parameters:
   us: short to convert

Return Value:
   us converted from host byte order to big-endian.  If the
   host is big endian, just returns us

=======================================================================

std_ntohl()

Description:
   The std_ntohl() changes a long from big-endian to host byte order.

Prototype:

   unsigned long std_ntohl(unsigned long ul)

Parameters:
   ul: long to convert

Return Value:
   ul converted from big-endian to host byte order.  If the
   host is big endian, just returns ul

=======================================================================

std_htonl()

Description:
   The std_htonl() converts a long from host byte-order to big-endian.

Prototype:

   unsigned long std_htonl(unsigned long ul)

Parameters:
   ul: long to convert

Return Value:
   ul converted from host byte order to big-endian.  If the
   host is big endian, just returns ul


=======================================================================

std_strlprintf()

Description:

   The functions std_strlprintf() and std_vstrlprintf() write formatted
   output to a string.  These functions guarantee NUL-termination of
   the output buffer when its size is greater than zero.

   A format string is copied to the output buffer, except for conversion
   specifiers contained within the format string.  Conversion specifiers
   begin with a "%" and specify some action that consumes an argument from
   the argument list.

   Conversion specifiers have the following form:
===pre>
       %[FLAGS] [WIDTH] [.PRECISION] [TYPE] CONV
===/pre>

   CONV is the only required field.  It is always a single character,
   and determines the action to be taken.  Supported values are:

===pre>
    CONV | Description
   ======|=======================================================
     c   | Output a single character.
         |
     s   | Output a NUL-terminated single-byte character string.
         |
    d, i | Ouptut a signed decimal integer.
         |
     u   | Output an unsigned decimal integer.
         |
     o   | Output an unsigned octal integer.
         |
     x   | Output an unsigned hexadecimal integer, using
         | lower case digits.
         |
     X   | Output an unsigned hexadecimal integer, using
         | upper case digits.
         |
     p   | Output a pointer value as eight hexadecimal digits,
         | using upper case digits.
===/pre>

   The next argument from the argument list supplies the value to be
   formatted and output.

   FLAGS, WIDTH, and PRECISION can modify the formatting of the value.

   FLAGS consists of one or more of the following characters:

===pre>
   Flag | Meaning
   =====|=================================================================
     +  | Prefix positive numbers with "+" (%d and %i only).
   -----|-----------------------------------------------------------------
     -  | When padding to meet WIDTH, pad on the right.
   -----|-----------------------------------------------------------------
     0  | Pad with '0' characters when padding on the left to meet WIDTH.
   -----|-----------------------------------------------------------------
   blank| Prefix positive numbers with " " (%d and %i only).
   space|
   -----|-----------------------------------------------------------------
     #  | With %x or %X: prefixes non-zero values with "0x"/"0X".
        | With %o, ensure the value begins with "0" (increasing PRECISION
        |    if necessary).
        | Ignored for all other CONV specifiers.
   -----|-----------------------------------------------------------------
===/pre>

   WIDTH is an unsigned decimal integer or the character "*".

   WIDTH gives the minimum number of characters to be written.  The
   formatted value will be padded with spaces until the minimum size is
   met; it never causes a value to be truncated The sign of the WIDTH
   integer selects between left and right padding.  Padding will be on
   the left unless the "-" flag is specified.

   When "*" is used, an 'int' argument is consumed from the argument
   list and used as the WIDTH.  A negative argument specifies padding on
   the right, and its absolute value gives the amount of padding.

   If the "0" flags is specified, any padding on the left will consist
   of "0" characters.  An exception to this rule is that the "0" flag is
   ignored when precision is specified for a numeric value.

   PRECISION is a non-negative decimal integer or "*" preceded by ".".

   When PRECISION accompanies any of the numeric conversions, it
   specifies the minimum number of digits to output.  Values are padded
   on the left with '0' to meet the specified size.  PRECISION defaults
   to 1 for numbers.

   When PRECISION accompanies other conversions, it specifies the
   maximum number of characters from the value to output.  The value
   will be truncated to ensure that at most PRECISION characters are
   output.

   TYPE provides information about the type of arguments.  This is used
   to determine the size of integer arguments. Values larger than 'int'
   can be properly obtained from the argument list.  Their behavior
   should be considered undefined for CONV operations other than integer
   formatting.

===pre>
    TYPE  | Meaning
   =======|=====================
     hh   | sizeof(char)
   -------|---------------------
      h   | sizeof(short)
   -------|---------------------
      l   | sizeof(long)
   -------|---------------------
    L, ll | sizeof(long long)
   -------|---------------------
      j   | sizeof(int64)
   -------|---------------------
      z   | sizeof(size_t)
   -------|---------------------
===/pre>

   For 64-bit integers, "ll" may be the most widely-supported type
   specifier in other printf implementation, but "j" has been introduced
   in ISO C99. This implementation supports both.

   Note that arguments to variadic functions are promoted to 'int' when
   smaller than 'int', so 'h' and 'hh' have no observable effect.
   Static analysis tools that understand standard format string syntax
   may use this information for other purposes.

Prototype:

   int std_strlprintf(char *pszDest, int nDestSize,
                      const char *pszFmt, ...);
Parameters:
   pszDest [out]: output buffer, where output will be placed
   nDestSize:     size of pszDest in bytes
   pszFmt:        format string

Return Value:

   The size required to hold the entire untruncated output, NOT
   including NUL-termination.

Comments:

   Notable omissions from std_strlprintf() are lack of support for
   floating point and lack of support for "%n".

Side Effects:
   None

See Also:
   None

=======================================================================

std_vstrlprintf()

Description:

  The std_vstrlprintf() is documented with std_strlprintf(), it's the
  vector form of std_strlprintf().  See std_strlprintf() for a
  more complete description.

Prototype:
   int std_vstrlprintf(char *pszDest, int nDestSize,
                       const char *pszFmt, AEEVaList args);

Parameters:
   pszDest [out]: output buffer, where output will be placed
   nDestSize:     size of pszDest in bytes
   pszFmt:        format string
   args:          arguments


=======================================================================

std_scanul()

Description:

    The std_scanul() converts an ASCII representation of a number to an unsigned
    long.  It expects strings that match the following pattern:
===pre>
         spaces [+|-] digits
===/pre>

    'Spaces' is zero or more ASCII space or tab characters.

    'Digits' is any number of digits valid in the radix.  Letters 'A' through
    'Z' are treated as digits with values 10 through 35.  'Digits' may begin
    with "0x" when a radix of 0 or 16 is specified.

    Upper and lower case letters can be used interchangeably.


Prototype:

    uint32 std_scanul( const char *pchBuf, int nRadix, const char **ppchEnd,
                       int *pnError)

Parameters:

    pchBuf [in] : the start of the string to scan.

    nRadix [in] : the numeric radix (or base) of the number.  Valid values are
                  2 through 36 or zero, which implies auto-detection.
                  Auto-detection examines the digits field.  If it begins with
                  "0x", radix 16 is selected.  Otherwise, if it begins with
                  "0" radix 8 is selected.  Otherwise, radix 10 is selected.

    ppchEnd [out] : if ppchEnd is not NULL, *ppchEnd points to the first
                    character that did not match the expected pattern shown
                    above, except on STD_BADPARAM and STD_OVERFLOW when it is
                    set to the start of the string.

    pnError [out] : If pnError is not NULL, *pnError holds the error code,
                    which is one of the following:
~
        0 : Numeric value is from 0 to MAX_UINT32.

        STD_NEGATIVE : The scanned value was negative and its absolute value was
                       from 1 to MAX_UINT32.  The result is the negated value
                       (cast to a uint32).

        STD_NODIGITS : No digits were found.  The result is zero.

        STD_OVERFLOW : The absolute value exceeded MAX_UINT32.  The result
                       is set to MAX_UINT32 and *ppchEnd is set to pchBuf.

        STD_BADPARAM : An improper value for nRadix was received.  The result
                       is set to zero, and *ppchEnd is set to pchBuf.
*

Return Value:

    The converted numeric result.

Comments:

   The std_scanul() is similar to ANSI C's strtoul() but differs in the following
   respects:

     1. It returns an error/success code.  strtoul() results are ambiguous
        unless the caller sets errno to zero before calling it.

     2. std_scanul() is free of references to current locale and errno.  Some
        strtoul() implementations use locale; some don't.

     3. It provides more complete reporting of range underflow.  strtoul()
        does not distinguish between "-1" and "0xFFFFFFFF", and underflow is
        poorly defined.

     4. std_scanul() reports a "no digits" error code to distinguish "0" from
        whitespace, "+", etc..

=======================================================================*/

#endif // AEESTD_H



