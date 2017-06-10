/* =======================================================================
                            wfd_util_signal.h
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
   $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/wfd-util/inc/wfd_util_signal.h
 
========================================================================== */

#ifndef _VENC_SIGNAL_H
#define _VENC_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/

/**
* @brief Constructor
*/
int venc_signal_create(void** handle);

/**
* @brief Destructor
*/
int venc_signal_destroy(void* handle);

/**
* @brief Set a signal
*/
int venc_signal_set(void* handle);

/**
* @brief Wait for signal to be set
*
* @param timeout Milliseconds before timeout. Specify 0 for infinite.
*
* @return 0 on success, 1 on error, 2 on timeout
*/
int venc_signal_wait(void* handle, int timeout);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VENC_SIGNAL_H
