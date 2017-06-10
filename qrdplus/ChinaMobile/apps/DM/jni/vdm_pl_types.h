#ifndef _VDM_PL_TYPES_H_
#define _VDM_PL_TYPES_H_

typedef void* VDM_Handle_t;

/* Define standard vDM types potentially requiring definitions dependent
 * on the toolset being used to compile the source.
 * The following definitions should work on most modern 32bit compilers. */
#ifndef IS32_DEFINED
#define IS32_DEFINED
typedef int IS32; /*!< 32bit integer */
#endif

#ifndef IU32_DEFINED
#define IU32_DEFINED
typedef unsigned int IU32; /*!< 32bit integer (unsigned) */
#endif
#ifndef IS16_DEFINED
#define IS16_DEFINED
typedef short IS16; /*!< 16bit integer */
#endif
#ifndef IU16_DEFINED
#define IU16_DEFINED
typedef unsigned short IU16; /*!< 16bit integer (unsigned) */
#endif
#ifndef IS8_DEFINED
#define IS8_DEFINED
typedef signed char IS8; /*!< 8bit integer */
#endif
#ifndef IU8_DEFINED
#define IU8_DEFINED
typedef unsigned char IU8; /*!< 8bit integer (unsigned) */
#endif
#ifndef IHPE_DEFINED
#define IHPE_DEFINED
typedef IU32 IHPE; /*! Integer type of equivalent size to a native pointer. */
#endif

/* Boolean definitions. Note that IBOOL must be at least as wide as the larger
 * of 'int' (the compiler's natural implicit boolean type), Ix32, and IHPE, to
 * allow bit-wise operations on any vDM type to be stored in an IBOOL
 * without loss of information. */
#ifndef IBOOL_DEFINED
#define IBOOL_DEFINED
typedef IS32 IBOOL; /*!< Boolean type */
#endif

/* Data types derived from standard vDM types. */
#ifndef UTF8Str_DEFINED
#define UTF8Str_DEFINED
typedef IU8 *UTF8Str; /*! UTF8 encoded Unicode string, NUL terminated. */
#endif
#ifndef UTF8CStr_DEFINED
#define UTF8CStr_DEFINED
typedef const IU8 *UTF8CStr; /*! UTF8 encoded Unicode string constant, NUL terminated. */
#endif

#ifndef IBITFLAGS_DEFINED
#define IBITFLAGS_DEFINED
typedef IU32 IBITFLAGS;
#endif

/* TRUE/FALSE may sometimes be defined already, so only add ours if not. */
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

/*! Replacement for NULL when <stdio.h> not wanted. */
#ifndef NULL
#define NULL    ((void *)0)
#endif

/*! Macro to remove warnings relating to unused function arguments. */
#ifndef UNUSED
#define UNUSED(x)   { (void)x; }
#endif

/* Macros for max/min - beware of the standard problem, namely that exactly one
 * of the arguments is evaluated twice. */
#ifndef min
#define min(a,b)    (((a)>(b))? (b):(a))
#endif
#ifndef max
#define max(a,b)    (((a)<(b))? (b):(a))
#endif

/* Other useful macros */
#define TABLE_SIZE(table)   (sizeof(table)/sizeof(table[0]))
#define EQUAL_BOOLEANS(b1, b2)  ((!(b1)) == (!(b2)))

#endif
