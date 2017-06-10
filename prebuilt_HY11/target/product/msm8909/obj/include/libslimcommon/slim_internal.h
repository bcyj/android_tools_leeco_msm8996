#ifndef SLIM_INTERNAL_H
#define SLIM_INTERNAL_H
/*============================================================================
  @file slim_internal.h

  SLIM processor specific utils declarations.

               Copyright (c) 2013-2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/osal/inc/slim_internal.h#8 $ */

/* Define to include ped alignment in engineering build.
   This flag will be removed altogether once the QMI_SNS-API is available in modem.
*/
#undef FEATURE_GNSS_SLIM_SSC_PED_ALIGNMENT

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_internal_api.h"
#include "slim_processor.h"
#include "msg.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

#define SLIM_MSG_MED0(str)              SLIM_MSG0(MSG_LEGACY_MED, str)
#define SLIM_MSG_MED1(str, a)           SLIM_MSG1(MSG_LEGACY_MED, str, a)
#define SLIM_MSG_MED2(str, a, b)        SLIM_MSG2(MSG_LEGACY_MED, str, a, b)
#define SLIM_MSG_MED(str, a, b, c)      SLIM_MSG(MSG_LEGACY_MED, str, a, b, c)
#define SLIM_MSG_HIGH0(str)             SLIM_MSG0(MSG_LEGACY_HIGH, str)
#define SLIM_MSG_HIGH1(str, a)          SLIM_MSG1(MSG_LEGACY_HIGH, str, a)
#define SLIM_MSG_HIGH2(str, a, b)       SLIM_MSG2(MSG_LEGACY_HIGH, str, a, b)
#define SLIM_MSG_HIGH(str, a, b, c)     SLIM_MSG(MSG_LEGACY_HIGH, str, a, b, c)
#define SLIM_MSG_ERROR0(str)            SLIM_MSG0(MSG_LEGACY_ERROR, str)
#define SLIM_MSG_ERROR1(str, a)         SLIM_MSG1(MSG_LEGACY_ERROR, str, a)
#define SLIM_MSG_ERROR2(str, a, b)      SLIM_MSG2(MSG_LEGACY_ERROR, str, a, b)
#define SLIM_MSG_ERROR(str, a, b, c)    SLIM_MSG(MSG_LEGACY_ERROR, str, a, b, c)

#define SLIM_INIT_CRIT_SECTION(pz_Mutex)  slim_MutexInit(pz_Mutex)
#define SLIM_ENTER_CRIT_SECTION(pz_Mutex) slim_MutexLock(pz_Mutex)
#define SLIM_LEAVE_CRIT_SECTION(pz_Mutex) slim_MutexUnlock(pz_Mutex)

#define SLIM_UNUSED(var) ((void)var)

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/** Enum for processors supported by SLIM.
*/
typedef enum
{
  SLIM_PROCESSOR_MPSS  = 0, /**< Modem processor */
  SLIM_PROCESSOR_APSS,      /**< Application processor */

  SLIM_PROCESSOR_MAX = 2147483647 /**< To force a 32 bit signed enum. Do not change or use */
} slim_ProcessorEnumType;


/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/**
@brief Returns current processor type.

Function returns current processor type.

@return Current processor type.
*/
slim_ProcessorEnumType slim_CurrentProcessor
(
  void
);

/**
@brief Size bounded memory copy.

Function copies bytes from the source buffer to the destination buffer.

@param dst Destination buffer.
@param dst_size Size of the destination buffer in bytes.
@param src Source buffer.
@param src_size Number of bytes to copy from source buffer.
@return The number of bytes copied to the destination buffer.
*/
size_t slim_Memscpy
(
  void *dst,
  size_t dst_size,
  const void *src,
  size_t src_size
);

/**
@brief Creates a mutex which can be associated with a resource.

Function creates a mutex which can be associated with a resource. Mutexes are
used to allow only one thread to enter the critical section of code that is
accessing shared data.

@param pz_Mutex Pointer to the mutex structure
@return TRUE if initialized with no errors, otherwise FALSE.
*/
boolean slim_MutexInit
(
  slim_MutexStructType *pz_Mutex
);

/**
@brief Locks the resource associated with pz_Mutex if it is not already locked.

Function locks the resource associated with pz_Mutex if it is not already
locked.

@param pz_Mutex Pointer to the mutex structure
*/
void slim_MutexLock
(
  slim_MutexStructType *pz_Mutex
);

/**
@brief Unlocks the resource associated with pz_Mutex.

Function unlocks the resource associated with pz_Mutex.

@param pz_Mutex Pointer to the mutex structure
*/
void slim_MutexUnlock
(
  slim_MutexStructType *pz_Mutex
);

/**
@brief Returns the timetick count in milliseconds.

Function returns the current timetick count in milliseconds.

@return Current timetick count in milliseconds.
*/
uint64 slim_TimeTickGetMilliseconds
(
  void
);
#endif /* #ifndef SLIM_INTERNAL_H */
