#ifndef MMSIGNAL_H
#define MMSIGNAL_H
/*===========================================================================
                          M M    W r a p p e r
                   f o r   S i g n a l  S e r v i c e s

*//** @file MMSignal.h
  This file defines provides functions to send and receive signals typically
  inter-process objects used for event notification.

Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/platform/OSAbstraction/main/latest/inc/MMSignal.h#3 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/28/09   rmandal   Converted comments to doxygen style.
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
typedef void* MM_HANDLE;
#define _MM_HANDLE
#endif


#define MM_SIGNAL_ATTRIBUTE_AUTOMATIC  0x01
#define MM_SIGNAL_ATTRIBUTE_MANUAL     0x02

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/**
 * Creates a signal queue object
 *
 * @param[out] pHandle return a reference to the Signal Queue handle on success
 *
 * @return zero when successfull else failure
 */
int MM_SignalQ_Create
(
  MM_HANDLE *pHandle
);

/**
 * Releases the resources associated with the signal queue
 *
 * @param[in] handle - the signal queue handle
 *
 * @return zero value on success else failure
 */
int MM_SignalQ_Release
(
  MM_HANDLE handle
);

/**
 * This function blocks until a signal is set on the queue. If a signal handler
 * has been registered when the signal is created then the handler is called,
 * else function returns with the user data pointer passed in when creating
 * the signal
 *
 * @param[in] handle - reference to the signal queue handle
 * @param[out] ppvUserArg - user data pointer of the signal that is set on
 *                          success
 *
 * @return zero when successfull else failure
 */
int MM_SignalQ_Wait
(
  MM_HANDLE handle,
  void       **ppvUserArg
);

/**
 * This function blocks until a signal is set on the queue. If a signal handler
 * has been registered when the signal is created then the handler is called,
 * else function returns with the user data pointer passed in when creating
 * the signal
 *
 * @param[in] handle - reference to the signal queue handle
 * @param[out] ppvUserArg - user data pointer of the signal that is set on
 *                          success
 * @param[in] pSigs - signals to be waited for
 * @param[in] numSignals - num of signals
 *
 * @return zero when successfull else failure
 */
int MM_SignalQ_WaitEx
(
  MM_HANDLE handle,
  void  **ppvUserArg,
  MM_HANDLE *pSignals,
  int numSignals
);


/**
 * This function blocks until a signal is set on the queue or the specified
 * time has elapsed.
 *
 * If a signal handler has been registered when the signal is created then
 * the handler is called, else function returns with the user data pointer
 * passed in when creating the signal.
 *
 * @param[in] handle - the signal queue handle
 * @param[in] nTimeOut - The time in msec to wait for the signal to be set
 * @param[out] ppvUserArg - user data pointer passed in when the signal is
 *                          created
 * @param[out] pbTimedOut - 1 indicates a time out no signal set
 *
 * @return zero on sucess else failure
 */
int MM_SignalQ_TimedWait
(
  MM_HANDLE   handle,
  int            nTimeOut,
  void         **ppvUserArg,
  int           *pbTimedOut
);

/**
 * This function blocks until a signal is set on the queue or the specified
 * time has elapsed.
 *
 * If a signal handler has been registered when the signal is created then
 * the handler is called, else function returns with the user data pointer
 * passed in when creating the signal.
 *
 * @param[in] handle - the signal queue handle
 * @param[in] nTimeOut - The time in msec to wait for the signal to be set
 * @param[out] ppvUserArg - user data pointer passed in when the signal is
 *                          created
 * @param[out] pbTimedOut - 1 indicates a time out no signal set
 * @param[in] pSigs - signals to be waited for
 * @param[in] numSignals - num of signals

 *
 * @return zero on sucess else failure
 */
int MM_SignalQ_TimedWaitEx
(
  MM_HANDLE   handle,
  int         nTimeOut,
  void        **ppvUserArg,
  int         *pbTimedOut,
  MM_HANDLE *pSigs,
  int numSignals
);

/*
 * Creates a the signal handle
 *
 * When the associated signal is set the registered call back will be called.
 * Note that the signal implementation does not have a context of its own, so
 * Pop() needs to be called to give context of execution for the handler to be
 * called.
 *
 * If the optional signal handler is registered then the handler will be called
 * with the optional user argument when the signal is set else user argument is
 * returned in Pop() call.
 *
 * @param[in] signalQHandle - reference to the signal queue handle
 * @param[in] pfnSignalHandler - optional signal handler to be called when the
 *                               signal is set
 * @param[in] pvUserArg - optional user data pointer to be passed in the call
 *                        back or returned when the signal is set.
 * @param[in] nFlags - indicates additional flags that can be specified for
                       signal
 * @param[out] pSignalHandle - returns a reference to the signal handle
 *
 * @return zero value is success else failure
 */
int MM_Signal_CreateEx
(
  MM_HANDLE   signalQHandle,
  void          *pvUserArg,
  void         (*pfnSignalHandler)(void* ),
  int            nFlags,
  MM_HANDLE  *pSignalHandle
);

/**
 * Creates a the signal handle
 *
 * When the associated signal is set the registered call back will be called.
 * Note that the signal implementation does not have a context of its own, so
 * Wait() or TimerWait() needs to be called to give context of execution for
 * the handler to be called.
 *
 * If the optional signal handler is registered then the handler will be called
 * with the optional user argument when the signal is set else user argument is
 * returned in Wait()/TimerWait() call.
 *
 * @param[in] signalQHandle - reference to the signal queue handle
 * @param[in] pfnSignalHandler - optional signal handler to be called when the
 *                               signal is set
 * @param[in] pvUserArg - optional user data pointer to be passed in the call
 *                        back or returned when the signal is set.
 * @param[out] pSignalHandle - returns a reference to the signal handle
 *
 * @return zero value is success else failure
 */
int MM_Signal_Create
(
  MM_HANDLE  signalQHandle,
  void       *pvUserArg,
  void       (*pfnSignalHandler)(void* ),
  MM_HANDLE  *pSignalHandle
);

/**
 * Releases the signal resources
 *
 * @param[in] handle - reference to the signal handle
 *
 * @return zero value on success else failure
 */
int MM_Signal_Release
(
  MM_HANDLE handle
);

/**
 * Sets the signal in the associated signal queue
 *
 * @param[in] handle - a reference to the signal handle
 *
 * @return zero when successfull else failure
 */
int MM_Signal_Set
(
  MM_HANDLE handle
);

/**
 * Resets the signal state, used for manual signals. For automatic signals
 * the signal is set to reset on exiting a wait to wait with timeout.
 *
 * @param[in] handle - a reference to the signal handle
 *
 * @return zero when successfull else failure
 */
int MM_Signal_Reset
(
  MM_HANDLE handle
);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif // MMSIGNAL_H
