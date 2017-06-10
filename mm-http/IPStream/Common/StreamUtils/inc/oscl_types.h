#ifndef OSCL_TYPES_H
#define OSCL_TYPES_H
/************************************************************************* */
/**
 *
 * @brief implementation of oscl_types.h
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */

/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/oscl_types.h#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $
========================================================================== */
/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */

#include "AEEStdDef.h"

/* ==========================================================================
                        DATA DECLARATIONS
========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#ifndef __cplusplus
#ifndef bool
// The bool type is mapped to an integer.
typedef int bool;
#endif
#else // __cplusplus
// The NULL_TERM_CHAR is used to terminate c-style strings.
const char NULL_TERM_CHAR = '\0';
#endif

#define BYTE_ORDER_LITTLE_ENDIAN
#if( !defined(BYTE_ORDER_LITTLE_ENDIAN) && !defined(BYTE_ORDER_BIG_ENDIAN) )
# error "Must specify valid endianness"
#endif // !LITTLE_ENDIAN && !BIG_ENDIAN

#if !defined(BYTE_ORDER_LITTLE_ENDIAN) && !defined(BYTE_ORDER_BIG_ENDIAN)
#error "must define either BYTE_ORDER_LITTLE_ENDIAN or BYTE_ORDER_BIG_ENDIAN"
#endif

// mbchar is multi-byte char with null termination.
#ifndef mbchar
typedef char mbchar;
#endif

// MemoryFragment goal is to allow this data structure to be
// passed directly to I/O routines that take scatter/gather arrays.
struct MemoryFragment {
  uint32 len;
  void *ptr;
};

// oscl_status_t defines return codes for oscl functions
// OSCL Return Code Definitions
typedef enum
{
  SUCCESS,
  FAIL,
  ETIMEOUT,
  ECONNREF,
  EINVALIDADDRESS,
  ENOPORT,
  EBADSOCK,
  ECANTBIND,
  ECANTLISTEN,
  EINDEXOUTOFBOUND,
  EMEMORY
} oscl_status_t;

#endif
