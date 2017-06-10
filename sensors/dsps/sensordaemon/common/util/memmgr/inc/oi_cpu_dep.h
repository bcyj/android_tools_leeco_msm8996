#ifndef _OI_CPU_DEP_H
#define _OI_CPU_DEP_H
/**
 * @file
 * This file contains definitions for characteristics of the target CPU and
 * compiler, including primitive data types and endianness.
 *
 * This file defines the byte order and primitive data types for various
 * CPU families. The preprocessor symbol 'CPU' must be defined to be an
 * appropriate value or this header will generate a compile-time error.
 *
 * @note The documentation for this header file uses the x86 family of processors
 * as an illustrative example for CPU/compiler-dependent data type definitions.
 * Go to the source code of this header file to see the details of primitive type
 * definitions for each platform.
 *
 * Additional information is available in the @ref data_types_docpage section.
 */

/**********************************************************************************
  $AccuRev-Revision: 344/2 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

/** @name Definitions indicating family of target OI_CPU_TYPE
 *  @{
 */

#define OI_CPU_X86         1 /**< x86 processor family */
#define OI_CPU_SH3         2 /**< Hitachi SH-3 processor family */
#define OI_CPU_H8          3 /**< Hitachi H8 processor family */
#define OI_CPU_MIPS        4 /**< MIPS processor family */
#define OI_CPU_SPARC       5 /**< SPARC processor family */
#define OI_CPU_M68000      6 /**< Motorola M68000 processor family */
#define OI_CPU_PPC         7 /**< PowerPC (PPC) processor family */
#define OI_CPU_SH4_7750    8 /**< Hitachi SH7750 series in SH-4 processor family */
#define OI_CPU_SH2         9 /**< Hitachi SH-2 processor family */
#define OI_CPU_ARM7_LEND  10 /**< ARM7, little-endian */
#define OI_CPU_ARM7_BEND  11 /**< ARM7, big-endian */
#define OI_CPU_GDM1202    12 /**< GCT GDM1202 */
#define OI_CPU_ARC_LEND   13 /**< ARC processor family, little-endian */
#define OI_CPU_ARC_BEND   14 /**< ARC processor family, big-endian */
#define OI_CPU_M30833F    15 /**< Mitsubishi M308 processor family */
#define OI_CPU_CR16C      16 /**< National Semiconductor 16-bit processor family */
#define OI_CPU_M64111     17 /**< Renesas M64111 processor (M32R family) */
#define OI_CPU_ARMV5_LEND 18 /**< ARM5, little-endian */
#define OI_CPU_ARM9_LEND  19 /**< ARM9, little-endian */
#define OI_CPU_ARM11_LEND 20 /**< ARM11, little-endian */

  /* sean
#ifndef OI_CPU_TYPE
    #error "OI_CPU_TYPE type not defined"
#endif
*/
/**@}*/


/** @name Definitions indicating byte-wise endianness of target CPU
 *  @{
 */

#define OI_BIG_ENDIAN_BYTE_ORDER    0  /**< Multiple-byte values are stored in memory beginning with the most significant byte at the lowest address.  */
#define OI_LITTLE_ENDIAN_BYTE_ORDER 1  /**< Multiple-byte values are stored in memory beginning with the least significant byte at the lowest address. */

/**@}*/


/** @name  CPU/compiler-independent primitive data type definitions
 *  @{
 */

typedef int             OI_BOOL;   /**< Boolean values use native integer data type for target CPU. */
typedef int             OI_INT;    /**< Integer values use native integer data type for target CPU. */
typedef unsigned int    OI_UINT;   /**< Unsigned integer values use native unsigned integer data type for target CPU. */
typedef unsigned char   OI_BYTE;   /**< Raw bytes type uses native character data type for target CPU. */
typedef intptr_t        OI_INTPTR; /**< type for pointer */

typedef int8_t          OI_INT8;   /**< 8-bit signed integer values */
typedef int16_t         OI_INT16;  /**< 16-bit signed integer values */
typedef int32_t         OI_INT32;  /**< 32-bit signed integer values */
typedef uint8_t         OI_UINT8;  /**< 8-bit unsigned integer values */
typedef uint16_t        OI_UINT16; /**< 16-bit unsigned integer values */
typedef uint32_t        OI_UINT32; /**< 32-bit unsigned integer values */



/**@}*/

#define OI_CPU_TYPE OI_CPU_X86

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_X86

#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER  /**< x86 platform byte ordering is little-endian */

/** @name CPU/compiler-dependent primitive data type definitions for x86 processor family
 *  @{
 */
typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_SH3
/* The Hitachi SH C compiler defines _LIT or _BIG, depending on the endianness
    specified to the compiler on the command line. */
#if defined(_LIT)
    #define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER  /**< If _LIT is defined, SH-3 platform byte ordering is little-endian. */
#elif defined(_BIG)
    #define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER     /**< If _BIG is defined, SH-3 platform byte ordering is big-endian. */
#else
    #error SH compiler endianness undefined
#endif

/** @name CPU/compiler-dependent primitive data type definitions for SH-3 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif
/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_SH2

#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER /**< SH-2 platform byte ordering is big-endian. */

/** @name  CPU/compiler-dependent primitive data type definitions for SH-2 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif
/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_H8
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER
#error basic types not defined
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_MIPS
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER
/** @name  CPU/compiler-dependent primitive data type definitions for MIPS processor family
 *  @{
 */
typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_SPARC
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER
#error basic types not defined
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_M68000
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER  /**< M68000 platform byte ordering is big-endian. */

/** @name  CPU/compiler-dependent primitive data type definitions for M68000 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_PPC
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER


/** @name  CPU/compiler-dependent primitive data type definitions for PPC 8XX processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_SH4_7750
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER  /**< SH7750 platform byte ordering is big-endian. */

/** @name   CPU/compiler-dependent primitive data type definitions for SH7750 processor series of the SH-4 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM7_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM7 processor family
 *  @{
 */

typedef void * OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM7_BEND
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER
/** @name   big-endian CPU/compiler-dependent primitive data type definitions for the ARM7 processor family
 *  @{
 */
typedef void * OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_GDM1202
#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARC_LEND

#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name CPU/compiler-dependent primitive data type definitions for ARC processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARC_BEND

#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER

/** @name CPU/compiler-dependent primitive data type definitions for ARC processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_M30833F

#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name CPU/compiler-dependent primitive data type definitions for Mitsubishi M308 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_CR16C

#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name CPU/compiler-dependent primitive data type definitions for National Semicnductor processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_M64111

#define OI_CPU_BYTE_ORDER OI_BIG_ENDIAN_BYTE_ORDER

/** @name CPU/compiler-dependent primitive data type definitions for Renesas M32R processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/
#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARMV5_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM7 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM9_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM9 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

#if OI_CPU_TYPE==OI_CPU_ARM11_LEND
#define OI_CPU_BYTE_ORDER OI_LITTLE_ENDIAN_BYTE_ORDER

/** @name   little-endian CPU/compiler-dependent primitive data type definitions for the ARM11 processor family
 *  @{
 */

typedef OI_UINT32 OI_ELEMENT_UNION; /**< Type for first element of a union to support all data types up to pointer width. */

/**@}*/

#endif

/*********************************************************************************/

/* sean
#ifndef OI_CPU_BYTE_ORDER
    #error "Byte order (endian-ness) not defined"
#endif
*/

/**@}*/

#ifdef __cplusplus
}
#endif

/*********************************************************************************/
#endif /* _OI_CPU_DEP_H */
