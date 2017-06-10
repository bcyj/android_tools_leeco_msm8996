/* =======================================================================
                              wfd_util_mutex.h
DESCRIPTION
  This header defines the wfd source item class.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/wfd_util_mutex.h
 
========================================================================== */

#ifndef _VENC_MUTEX_H
#define _VENC_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/

/**
 * @brief Constructor
 */
int venc_mutex_create(void** handle);

/**
 * @brief Destructor
 */
int venc_mutex_destroy(void* handle);

/**
 * @brief Locks the mutex
 */
int venc_mutex_lock(void* handle);

/**
 * @brief Unlocks the mutex
 */
int venc_mutex_unlock(void* handle);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VENC_MUTEX_H
