/*===========================================================================
                          V i d e o   W r a p p e r
               f o r   S i g n a l  Q u e u e  O b j e c t

*//** @file MMSignal.c
  This file defines provides functions to send and receive signals typically
  inter-process objects used for event notification.

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Common/OSAbstraction/POSIX/main/latest/src/MMSignalPosix.c#1 $

when       who         what, where, why
--------   ---         -------------------------------------------------------
07/15/09   rvontedd    Created file.

============================================================================*/

/*===========================================================================
 Include Files
============================================================================*/
#include "comdef.h"
#include "MMSignal.h"
#include "MMMalloc.h"
#include "MMCriticalSection.h"
#include "MMThread.h"
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "MMSignal"
#include "utils/Log.h"

/* =======================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MM_MAX_NUMBER_SIGNALS 32
#define WAIT_TIMEOUT        0xFF

#define USER_SIG_01   SIGRTMIN
#define USER_SIG_02   SIGRTMIN+1

#define UNUSED(x) ((void)x)

/* -----------------------------------------------------------------------
** Forward declarations
** ----------------------------------------------------------------------- */
typedef struct __MM_SignalQ MM_SignalQ;
typedef struct __MM_Signal MM_Signal;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

struct __MM_Signal {
  /*
   * Attributes of the signal.
   */
  int m_nSignalAttributes;
  /*
   * This signal has been assigned.
   */
  boolean m_bSignalInUse;
  /*
   * Indicates if the signal has been set.
   */
  boolean m_bSignalSet;
  /*
   * User data pointer associated with the event.
   */
  void *m_pvUserArg;
  /*
   * Signal handler associated with the event.
   */
  void  (*m_pfnSignalHandler)(void* );
  /*
   * Refernce to the parent signalQ with which the signal is associated.
   */
  MM_SignalQ *m_pParentSignalQ;

};

struct __MM_SignalQ {
  /*
   * This signal has been assigned.
   */
  boolean m_bSignalQInUse;

  /*
   * List of events that have been created in this signal queue
   */
  MM_Signal m_signals[MM_MAX_NUMBER_SIGNALS];
  /*
   * Lock to synchronize the addition and removal of events  m_signals
   */
  //MM_HANDLE m_signalsLock;

  /*
   *   Pthread handle
   */
  pthread_t p_tcb;

  MM_HANDLE timer_handle;
  pthread_mutex_t m_lock;
  pthread_cond_t m_cond;

};

static void MM_Signal_Internal_Timer_Handler(void* arg)
{
    MM_SignalQ* pSignalQ =(MM_SignalQ*)arg;
    pthread_kill(pSignalQ->p_tcb, USER_SIG_02);
}

static void MM_Signal_Catcher(int signo) {
    UNUSED(signo);
    return;
}

/*
 * Creates a signal queue object
 *
 * @param[out] return a reference to the Signal Queue handle on success
 *
 * @return zero when successfull else failure
 */
int MM_SignalQ_Create
(
  MM_HANDLE *pHandle
)
{
    MM_SignalQ *pSignalQ = NULL;
    int status = 1; // Failure

    if ( pHandle )
    {
        // Create a signal queue handle
        pSignalQ = MM_Malloc( sizeof(MM_SignalQ) );

        if ( pSignalQ )
        {
            int itr = 0;
            memset(pSignalQ, 0, sizeof(MM_SignalQ));
            pSignalQ->m_bSignalQInUse = TRUE;
            pthread_cond_init(&(pSignalQ->m_cond), 0);
            pthread_mutex_init(&(pSignalQ->m_lock), 0);

            for ( itr = 0; itr < MM_MAX_NUMBER_SIGNALS; itr++ )
            {
                pSignalQ->m_signals[itr].m_pParentSignalQ = pSignalQ;
            }
            *pHandle = pSignalQ;

            status = 0;

            /*
            if(MM_CriticalSection_Create(&pSignalQ->m_signalsLock) == 0)
            {
                int itr;
                for ( itr = 0; itr < MM_MAX_NUMBER_SIGNALS; itr++ )
                {
                    pSignalQ->m_signals[itr].m_pParentSignalQ = pSignalQ;
                }
                *pHandle = pSignalQ;

                status = 0;
            }
            else
            {
                MM_Free(pSignalQ);
            }
            */
        }
    }
    return status;
}

/*
 * Releases the resources associated with the signal queue
 *
 * @param[in] pSignalQ - the signal queue handle
 *
 * @return zero value on success else failure
 */
int MM_SignalQ_Release
(
  MM_HANDLE handle
)
{
    MM_SignalQ *pSignalQ = (MM_SignalQ *)handle;
    int status = 1; // Failure

    if (pSignalQ)
    {
        int itr = 0;
        pthread_mutex_lock(&(pSignalQ->m_lock));
        pSignalQ->m_bSignalQInUse = FALSE;

        // check if all the signals have been closed, if yes go ahead and delete
        // signal queue else wait for the last signal to be closed to delete the
        // signal queue. This is done by calling MM_SignalQ_Release in
        // MM_Signal_Release
        for ( itr = 0 ; itr < MM_MAX_NUMBER_SIGNALS; itr++)
        {
            if( pSignalQ->m_signals[itr].m_bSignalInUse )
            {
                break;
            }
        }

        pthread_mutex_unlock(&(pSignalQ->m_lock));

        // all signals have been released, delete the signalQ
        if ( itr == MM_MAX_NUMBER_SIGNALS )
        {
            pthread_cond_destroy(&(pSignalQ->m_cond));
            pthread_mutex_destroy(&(pSignalQ->m_lock));
            MM_Free( handle );
        }

        status = 0;
    }

    return status;
}

/*
 * This function blocks until a signal is set on the queue. If a signal handler
 * has been registered when the signal is created then the handler is called,
 * else function returns with the user data pointer passed in when creating
 * the signal
 *
 * @param[in] pSignalQ - reference to the signal queue handle
 * @param[out] ppvUserArg - user data pointer of the signal that is set on
 *                          success
 *
 * @return zero when successfull else failure
 */
int MM_SignalQ_Wait
(
  MM_HANDLE handle,
  void  **ppvUserArg
)
{
  MM_SignalQ *pSignalQ = (MM_SignalQ *)handle;
  int signalIndex = MM_MAX_NUMBER_SIGNALS;
  int nResult = 1; //failure

  if (pSignalQ)
  {
    pthread_mutex_lock(&(pSignalQ->m_lock));
    // check if any of the signals are set, if yes go ahead process the signal
    for ( signalIndex = 0 ; signalIndex < MM_MAX_NUMBER_SIGNALS;
          signalIndex++)
    {
      if( pSignalQ->m_signals[signalIndex].m_bSignalSet )
      {
        break;
      }
    }


    // none of the signals have been set, wait
    while (signalIndex == MM_MAX_NUMBER_SIGNALS)
    {
      //int ret_sig;
      //pSignalQ->p_tcb = pthread_self();
      pthread_cond_wait(&(pSignalQ->m_cond), &(pSignalQ->m_lock));

      for ( signalIndex = 0 ; signalIndex < MM_MAX_NUMBER_SIGNALS;
            signalIndex++)
      {
        if( pSignalQ->m_signals[signalIndex].m_bSignalSet )
        {
          break;
        }
      }
    }

    if ( signalIndex < MM_MAX_NUMBER_SIGNALS )
    {
      // Process the signal
      if ( pSignalQ->m_signals[signalIndex].m_nSignalAttributes ==
           MM_SIGNAL_ATTRIBUTE_AUTOMATIC )
      {
        pSignalQ->m_signals[signalIndex].m_bSignalSet = FALSE;
      }
      if ( pSignalQ->m_signals[signalIndex].m_pfnSignalHandler )
      {
        pSignalQ->m_signals[signalIndex].m_pfnSignalHandler(
                       pSignalQ->m_signals[signalIndex].m_pvUserArg);
      }
      else if ( pSignalQ->m_signals[signalIndex].m_pvUserArg && ppvUserArg)
      {
        *ppvUserArg = pSignalQ->m_signals[signalIndex].m_pvUserArg;
      }
      nResult = 0;
    }
    pthread_mutex_unlock(&(pSignalQ->m_lock));
  }

  return nResult;
}

/*
 * This function blocks until a signal is set on the queue. If a signal handler
 * has been registered when the signal is created then the handler is called,
 * else function returns with the user data pointer passed in when creating
 * the signal
 *
 * @param[in] pSignalQ - reference to the signal queue handle
 * @param[out] ppvUserArg - user data pointer of the signal that is set on
 *                          success
 *
 * @return zero when successfull else failure
 */
int MM_SignalQ_WaitEx
(
  MM_HANDLE handle,
  void **ppvUserArg,
  MM_HANDLE *pSigs,
  int numSignals
)
{
  MM_SignalQ *pSignalQ = (MM_SignalQ *)handle;
  int signalIndex = 0;
  MM_Signal **pSignals = (MM_Signal **) pSigs;
  int nResult = 0; //success

  // sanity check for valid signals
  for ( signalIndex = 0 ; signalIndex < numSignals; signalIndex++)
  {
    if( pSignals[signalIndex] == NULL )
    {
      nResult = 1; //failure
      break;
    }
  }

  if (nResult == 0 && pSignalQ)
  {
    nResult = 1; //failure
    pthread_mutex_lock(&(pSignalQ->m_lock));
    // check if any of the signals are set, if yes go ahead process the signal
    for ( signalIndex = 0 ; signalIndex < numSignals;
          signalIndex++)
    {
      if( pSignals[signalIndex]->m_bSignalSet )
      {
        break;
      }
    }

    // none of the signals have been set, wait
    while ( signalIndex == numSignals)
    {
      //int ret_sig;
      //pSignalQ->p_tcb = pthread_self();
      pthread_cond_wait(&(pSignalQ->m_cond), &(pSignalQ->m_lock));
      for ( signalIndex = 0 ; signalIndex < numSignals;
            signalIndex++)
      {
        if( pSignals[signalIndex]->m_bSignalSet )
        {
          break;
        }
      }
    }

    if ( signalIndex < numSignals )
    {
      // Process the signal
      if ( pSignals[signalIndex]->m_nSignalAttributes ==
           MM_SIGNAL_ATTRIBUTE_AUTOMATIC )
      {
        pSignals[signalIndex]->m_bSignalSet = FALSE;
      }
      if ( pSignals[signalIndex]->m_pfnSignalHandler )
      {
        pSignals[signalIndex]->m_pfnSignalHandler(
                      pSignals[signalIndex]->m_pvUserArg);
      }
      else if ( pSignals[signalIndex]->m_pvUserArg && ppvUserArg)
      {
        *ppvUserArg = pSignals[signalIndex]->m_pvUserArg;
      }
      nResult = 0;
    }
    pthread_mutex_unlock(&(pSignalQ->m_lock));
  }

  return nResult;
}

/*
 * This function blocks until a signal is set on the queue or the specified
 * time has elapsed.
 *
 * If a signal handler has been registered when the signal is created then
 * the handler is called, else function returns with the user data pointer
 * passed in when creating the signal
 *
 * @param[in] handle - the signal queue handle
 * @param[in] nTimeOut - The time in msec to wait for the signal to be set
 * @param[out] ppvUserArg - user data pointer passed in when the signal is
 *                          created
 * @param[out] bTimedOut - 1 indicates a time out no signal set
 *
 * @return zero on sucess else failure
 */
int MM_SignalQ_TimedWait
(
  MM_HANDLE   handle,
  int         nTimeOut,
  void        **ppvUserArg,
  int         *pbTimedOut
)
{

  MM_SignalQ *pSignalQ = (MM_SignalQ *)handle;
  int signalIndex = MM_MAX_NUMBER_SIGNALS;
  int nResult = 1; //failure
  struct timespec ts;

  if (pSignalQ && pbTimedOut)
  {
    pthread_mutex_lock(&(pSignalQ->m_lock));
    *pbTimedOut = FALSE;
    // check if any of the signals are set, if yes go ahead process the signal
    for ( signalIndex = 0 ; signalIndex < MM_MAX_NUMBER_SIGNALS;
          signalIndex++)
    {
      if( pSignalQ->m_signals[signalIndex].m_bSignalSet )
      {
        break;
      }
    }

    // none of the signals have been set, wait
    while ( signalIndex == MM_MAX_NUMBER_SIGNALS && *pbTimedOut == FALSE)
    {
      //int ret_sig;
      //pSignalQ->p_tcb = pthread_self();
      clock_gettime(CLOCK_REALTIME, &ts);

      ts.tv_sec += (time_t)(nTimeOut/1000);
      ts.tv_nsec += (nTimeOut%1000) * 1000000;

      nResult = pthread_cond_timedwait(&(pSignalQ->m_cond),
                                       &(pSignalQ->m_lock),
                                       &ts);
      if (nResult == ETIMEDOUT)
      {
        *pbTimedOut = TRUE;
      }
      else
      {
        // check if any of the signals are set, if yes go ahead process
        // the signal
        for ( signalIndex = 0 ; signalIndex < MM_MAX_NUMBER_SIGNALS;
              signalIndex++)
        {
          if( pSignalQ->m_signals[signalIndex].m_bSignalSet )
          {
            break;
          }
        }
      }
    }

    if ( *pbTimedOut == TRUE )
    {
      // Timed out
      nResult = 0;
    }
    else if ( signalIndex < MM_MAX_NUMBER_SIGNALS )
    {
      // Process the signal
      if ( pSignalQ->m_signals[signalIndex].m_nSignalAttributes ==
           MM_SIGNAL_ATTRIBUTE_AUTOMATIC )
      {
        pSignalQ->m_signals[signalIndex].m_bSignalSet = FALSE;
      }
      if ( pSignalQ->m_signals[signalIndex].m_pfnSignalHandler )
      {
        pSignalQ->m_signals[signalIndex].m_pfnSignalHandler(
                       pSignalQ->m_signals[signalIndex].m_pvUserArg);
      }
      else if ( pSignalQ->m_signals[signalIndex].m_pvUserArg )
      {
        *ppvUserArg = pSignalQ->m_signals[signalIndex].m_pvUserArg;
      }
      nResult = 0;
    }
    pthread_mutex_unlock(&(pSignalQ->m_lock));
  }

  return nResult;
}


/*
 * This function blocks until a signal is set on the queue or the specified
 * time has elapsed.
 *
 * If a signal handler has been registered when the signal is created then
 * the handler is called, else function returns with the user data pointer
 * passed in when creating the signal
 *
 * @param[in] handle - the signal queue handle
 * @param[in] nTimeOut - The time in msec to wait for the signal to be set
 * @param[out] ppvUserArg - user data pointer passed in when the signal is
 *                          created
 * @param[out] bTimedOut - 1 indicates a time out no signal set
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
)
{

  MM_SignalQ *pSignalQ = (MM_SignalQ *)handle;
  MM_Signal **pSignals = (MM_Signal **) pSigs;
  int signalIndex = 0;
  int nResult = 0; //success
  struct timespec ts;

  // sanity check for valid signals
  for ( signalIndex = 0 ; signalIndex < numSignals; signalIndex++)
  {
    if( pSignals[signalIndex] == NULL )
    {
      nResult = 1; //failure
      break;
    }
  }

  if (nResult == 0 && pSignalQ && pbTimedOut)
  {
    nResult = 1; //failure
    pthread_mutex_lock(&(pSignalQ->m_lock));
    *pbTimedOut = FALSE;

    // check if any of the signals are set, if yes go ahead process the signal
    for ( signalIndex = 0 ; signalIndex < numSignals;
          signalIndex++)
    {
      if( pSignals[signalIndex]->m_bSignalSet )
      {
        break;
      }
    }

    // none of the signals have been set, wait
    while ( signalIndex == numSignals && *pbTimedOut == FALSE)
    {
      //int ret_sig;
      //pSignalQ->p_tcb = pthread_self();
      clock_gettime(CLOCK_REALTIME, &ts);

      ts.tv_sec += (time_t)(nTimeOut/1000);
      ts.tv_nsec += (nTimeOut%1000) * 1000000;

      nResult = pthread_cond_timedwait(&(pSignalQ->m_cond),
                                       &(pSignalQ->m_lock),
                                       &ts);
      if (nResult == ETIMEDOUT)
      {
        *pbTimedOut = TRUE;
      }
      else
      {
        // check if any of the signals are set, if yes go ahead process
        // the signal
        for ( signalIndex = 0 ; signalIndex < numSignals;
              signalIndex++)
        {
          if( pSignals[signalIndex]->m_bSignalSet )
          {
            break;
          }
        }
      }
    }

    if ( *pbTimedOut == TRUE )
    {
      // Timed out
      nResult = 0;
    }
    else if ( signalIndex < numSignals )
    {
      // Process the signal
      if ( pSignals[signalIndex]->m_nSignalAttributes ==
           MM_SIGNAL_ATTRIBUTE_AUTOMATIC )
      {
        pSignals[signalIndex]->m_bSignalSet = FALSE;
      }

      if ( pSignals[signalIndex]->m_pfnSignalHandler )
      {
        pSignals[signalIndex]->m_pfnSignalHandler(
                      pSignals[signalIndex]->m_pvUserArg);
      }
      else if ( pSignals[signalIndex]->m_pvUserArg && ppvUserArg)
      {
        *ppvUserArg = pSignals[signalIndex]->m_pvUserArg;
      }

      nResult = 0;
    }

    pthread_mutex_unlock(&(pSignalQ->m_lock));
  }

  return nResult;
}

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
 * @param[out] pSignalHandle - returns a reference to the signal handle
 *
 * @return zero value is success else failure
 */
int MM_Signal_Create
(
  MM_HANDLE   signalQHandle,
  void          *pvUserArg,
  void         (*pfnSignalHandler)(void* ),
  MM_HANDLE  *pSignalHandle
)
{
  return MM_Signal_CreateEx( signalQHandle, pvUserArg, pfnSignalHandler,
                                MM_SIGNAL_ATTRIBUTE_AUTOMATIC,
                                pSignalHandle );
}

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
 * @param[in] nSignalAttributes - indicates additional flags that can be specified for
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
  int            nSignalAttributes,
  MM_HANDLE  *pSignalHandle
)
{
  int nResult = 1; // failure
  MM_SignalQ *pSignalQ = (MM_SignalQ *)signalQHandle;

  if ( pSignalQ && pSignalHandle )
  {

    int itr = 0;
    pthread_mutex_lock(&(pSignalQ->m_lock));
    if ( pSignalQ->m_bSignalQInUse )
    {
      for ( itr = 0; itr < MM_MAX_NUMBER_SIGNALS; itr++ )
      {
        if ( pSignalQ->m_signals[itr].m_bSignalInUse == FALSE &&
             pSignalQ->m_signals[itr].m_bSignalSet == FALSE )
        {
          pSignalQ->m_signals[itr].m_nSignalAttributes = nSignalAttributes;
          pSignalQ->m_signals[itr].m_bSignalInUse = TRUE;
          pSignalQ->m_signals[itr].m_pvUserArg = pvUserArg;
          pSignalQ->m_signals[itr].m_pfnSignalHandler = pfnSignalHandler;
          *pSignalHandle = (MM_HANDLE)&pSignalQ->m_signals[itr];
          nResult = 0;
          break;
        }
      }
    }
    pthread_mutex_unlock(&(pSignalQ->m_lock));
  }

  return nResult;
}

/*
 * There are no further refernces to the object mark for deletion
 *
 * @param[in] pSignal - reference to the signal handle
 *
 * @return zero value on success else failure
 */
int MM_Signal_Release
(
  MM_HANDLE handle
)
{
  int nResult = 1; // failure
  MM_Signal *pSignal = (MM_Signal *)handle;

  if ( pSignal )
  {
    MM_SignalQ *pSignalQ = pSignal->m_pParentSignalQ;
    if (pSignalQ)
    {
      pthread_mutex_lock(&(pSignalQ->m_lock));
      if ( pSignal->m_bSignalInUse )
      {
        pSignal->m_bSignalInUse = FALSE;
        // Check if the the parent signal queue has been deleted in the earlier
        // call if the signal queue has been deleted and this is the last signal
        // to be released in the signal queue then delete the signal queue
        if ( pSignalQ->m_bSignalQInUse == FALSE )
        {
          MM_SignalQ_Release( pSignalQ );
        }
      }
      nResult = 0;
      pthread_mutex_unlock(&(pSignalQ->m_lock));
    }
  }
  return nResult;
}

/*
 * Sets the signal
 *
 * @param[in] pSignal - a reference to the signal handle
 *
 * @return zero when successfull else NULL on failure
 */
int MM_Signal_Set
(
  MM_HANDLE handle
)
{
  int nResult = 1; // failure
  MM_Signal *pSignal = (MM_Signal *)handle;

  if (pSignal)
  {
    MM_SignalQ *pSignalQ = pSignal->m_pParentSignalQ;
    if (pSignalQ)
    {
      pthread_mutex_lock(&(pSignalQ->m_lock));
      if ( pSignal->m_bSignalInUse && pSignalQ->m_bSignalQInUse )
      {
         pSignal->m_bSignalSet = TRUE;
         pthread_cond_signal(&(pSignalQ->m_cond));
         nResult = 0;
      }
      pthread_mutex_unlock(&(pSignalQ->m_lock));
    }
  }
  return nResult;
}

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
)
{
  int nResult = 1; // failure
  MM_Signal *pSignal = (MM_Signal *)handle;

  if ( pSignal )
  {
    MM_SignalQ *pSignalQ = pSignal->m_pParentSignalQ;
    if (pSignalQ)
    {
      pthread_mutex_lock(&(pSignalQ->m_lock));
      pSignal->m_bSignalSet = FALSE;
      pthread_mutex_unlock(&(pSignalQ->m_lock));
      nResult = 0;
    }
  }

  return nResult;
}


