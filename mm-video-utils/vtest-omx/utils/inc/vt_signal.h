/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VT_SIGNAL_H
#define _VT_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/

/**
* @brief Constructor
*/
int vt_signal_create(void **handle);

/**
* @brief Destructor
*/
int vt_signal_destroy(void *handle);

/**
* @brief Set a signal
*/
int vt_signal_set(void *handle);

/**
* @brief Wait for signal to be set
*
* @param timeout Milliseconds before timeout. Specify 0 for infinite.
*
* @return 0 on success, 1 on error, 2 on timeout
*/
int vt_signal_wait(void *handle, int timeout);

/**
* @brief Broadcast a signal
*/
int vt_signal_broadcast(void *handle);

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VT_SIGNAL_H
