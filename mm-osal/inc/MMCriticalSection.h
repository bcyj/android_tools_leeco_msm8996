#ifndef MMCRITICALSECTION_H
#define MMCRITICALSECTION_H
/*===========================================================================
                          M M    W r a p p e r
               f o r   C r i t i c a l  S e c t i o n  O b j e c t

*//** @file MMCriticalSection.h
  This file defines a methods that can be used to synchronize access
  to data in multithreaded environment

Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMCriticalSection.h#1 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal     Converted comments to doxygen style.
07/02/08   gkapalli    Created file.

============================================================================*/

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

#ifndef _MM_HANDLE
typedef void *MM_HANDLE;
#define _MM_HANDLE
#endif

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Initializes the critical section
 *
 * @param[out] pHandle - a reference to the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Create
(
  MM_HANDLE  *pHandle
);

/**
 * Releases the resources associated with the critical section
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Release
(
  MM_HANDLE  handle
);

/**
 * Enter a critical section, blocks if another thread is inside the critical
 * section until the other thread leaves the critical section.
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Enter
(
  MM_HANDLE  handle
);

/**
 * Leave a critical section (releases the lock).
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value on success else failure
 */
int MM_CriticalSection_Leave
(
  MM_HANDLE  handle
);

/**
 * Tries to acquire the lock, return with failure immediately if not able to
 * acquire the lock unlike Enter which blocks until the lock is acquired.
 *
 * @param[in] handle - the critical section handle
 *
 * @return zero value is success else failure
 */
int MM_CriticalSection_TryEnter
(
  MM_HANDLE  handle
);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // MMCRITICALSECTION_H
