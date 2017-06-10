/******************************************************************************
  @file    qcril_event.c
  @brief   qcril qmi - event queue

  DESCRIPTION
    This module queues QCRIL events generated as a result of RPC callbacks from
    AMSS (ARM9 processor) and processes them in a separate thread, thus allowing
    the RPC threads in the kernel to return before QCRIL processing is done.

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/



/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#ifdef QMI_RIL_UTF
#include <ril_utf_wake_lock_redef.h>
#else
#include <hardware_legacy/power.h>
#endif
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_log.h"
#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
#include "qcril_uim_remote_client_msg_meta.h"
#endif
/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define QCRIL_WAKE_LOCK_NAME "qcril"

typedef struct qcril_event_buf
{
  struct qcril_event_buf *next;
  struct qcril_event_buf *prev;
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_evt_e_type event_id;
  void *data;
  size_t datalen;
  RIL_Token t;
  boolean data_must_be_freed;
} qcril_event_type;

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/*! static data for this module */
typedef struct
{
  pthread_t tid;
  pthread_mutex_t list_mutex;
  pthread_mutex_t startup_mutex;
  qcril_event_type list;
  fd_set readFds;
  int started;
  int fdWakeupRead;
  int fdWakeupWrite;

  pthread_mutex_t       activity_lock_mutex;
  pthread_mutexattr_t   activity_lock_mutex_atr;

} qcril_event_static_data_type;

static qcril_event_static_data_type qcril_event;

static pthread_cond_t qcril_event_startupCond = PTHREAD_COND_INITIALIZER;

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/



/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_event_init_list

===========================================================================*/
/*!
    @brief
    Initializes the list

    @return
    Void
*/
/*=========================================================================*/
static void qcril_event_init_list
(
  qcril_event_type *list
)
{
  memset(list, 0, sizeof(qcril_event_type));
  list->next = list;
  list->prev = list;
  list->event_id = QCRIL_EVT_NONE;
} /* qcril_event_init_list() */

/*===========================================================================

  FUNCTION:  qcril_event_add_to_list

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
static void qcril_event_add_to_list
(
  qcril_event_type *ev,
  qcril_event_type *list
)
{
  ev->next = list;
  ev->prev = list->prev;
  ev->prev->next = ev;
  list->prev = ev;
} /* qcril_event_add_to_list() */

/*===========================================================================

  FUNCTION:  qcril_event_remove_from_list

===========================================================================*/
/*!
    @brief
    Removes an entry from the list.

    @return
*/
/*=========================================================================*/
static void qcril_event_remove_from_list
(
  qcril_event_type *ev
)
{
  ev->next->prev = ev->prev;
  ev->prev->next = ev->next;
  ev->next = NULL;
  ev->prev = NULL;
} /* qcril_event_remove_from_list() */

/*===========================================================================

  FUNCTION:  qcril_event_main

===========================================================================*/
/*!
    @brief
    Main loop of the thread to process events queued from AMSS RPC
    calls to QCRIL.

    @return
*/
/*=========================================================================*/
static void *qcril_event_main
(
  void *param
)
{
  int ret;
  int filedes[2];
  int n;
  fd_set rfds;
  qcril_event_type *ev;
  char buff[16];
  IxErrnoType err_no;
  int go_on;

  /*-----------------------------------------------------------------------*/

  param = param;
  pthread_mutex_init(&qcril_event.list_mutex, NULL);
  qcril_event_init_list(&qcril_event.list);

  FD_ZERO(&qcril_event.readFds); /* Needed to use select() system call */

  /* Signal main thread that we are created and running */
  QCRIL_MUTEX_LOCK( &qcril_event.startup_mutex, "[Event Thread] qcril_event.startup_mutex" );
  qcril_event.started = 1;

  /* Create a pipe so main thread can wake us up */
  ret = pipe(filedes);
  if (ret < 0)
  {
    QCRIL_LOG_ERROR("Error opening pipe (%d)", errno);
    QCRIL_MUTEX_UNLOCK( &qcril_event.startup_mutex, "[Event Thread] qcril_event.startup_mutex" );
    qmi_ril_clear_thread_name(pthread_self());
    return NULL;
  }

  qcril_event.fdWakeupRead = filedes[0];
  qcril_event.fdWakeupWrite = filedes[1];

  fcntl(qcril_event.fdWakeupRead, F_SETFL, O_NONBLOCK);
  FD_SET(qcril_event.fdWakeupRead, &qcril_event.readFds);

  /* Now that the socket is created notify the main thread it can continue,
     which may initiate callbacks from AMSS immediately */
  pthread_cond_broadcast(&qcril_event_startupCond);

  /* Now wait for the main thread to finish initializing before starting the
     main loop, to make sure we don't call onUnsolicitedResposne before
     rild has completed initialization. */
  while (qcril_event.started < 2)
  {
    QCRIL_LOG_VERBOSE("Event thread waiting for started == 2 (%d)", qcril_event.started );
    pthread_cond_wait(&qcril_event_startupCond, &qcril_event.startup_mutex);
  }

  QCRIL_MUTEX_UNLOCK( &qcril_event.startup_mutex, "[Event Thread] qcril_event.startup_mutex" );

  for (;;)
  {
    /* Make a local copy of read fd_set;  Don't ask why. */
    memcpy(&rfds, &qcril_event.readFds, sizeof(fd_set));
    QCRIL_LOG_DEBUG("Waiting...");

    /* Block waiting for a event to be put on the queue */
    n = select(qcril_event.fdWakeupRead + 1, &rfds, NULL, NULL, NULL);
    if (n < 0)
    {
      if (errno == EINTR) continue;
      QCRIL_LOG_ERROR("QCRIL event select error (%d)", errno);
      qmi_ril_clear_thread_name(pthread_self());
      return NULL;
    }

    /* Empty the socket */
    do
    {
      ret = read(qcril_event.fdWakeupRead, &buff, sizeof(buff));
      if (ret > 0)
      {
        QCRIL_LOG_DEBUG("%d items on queue", ret);
      }
    } while (ret > 0 || (ret < 0 && errno == EINTR));

    /* There is something on our list.  The list management is simple.
       The main thread adds entries at the end of the list.  This thread
       takes them off and processes them.  The pipe is written for each
       entry added so if more things are on the queue by the time we finish
       the select() will not block and we will do another round. */

    acquire_wake_lock( PARTIAL_WAKE_LOCK, QCRIL_WAKE_LOCK_NAME );

    pthread_mutex_lock( &qcril_event.activity_lock_mutex );
    QCRIL_MUTEX_LOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );

    do
    {
      if ( ( NULL != ( ev = qcril_event.list.next ) && ( ev != &qcril_event.list ) ) )
      {
        qcril_event_remove_from_list( ev );


        QCRIL_LOG_DEBUG( "RID %d MID %d De-queued event %s (%d)(obj 0x%"PRIxPTR")",
                         ev->instance_id, ev->modem_id,
                         qcril_log_lookup_event_name((int) ev->event_id),
                         ev->event_id, (intptr_t) ev );

#ifdef QMI_RIL_UTF
        // shutdown thread upon request
        if (ev->event_id == -1)
        {
          close(filedes[0]);
          close(filedes[1]);
          QCRIL_MUTEX_UNLOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );
          pthread_mutex_unlock( &qcril_event.activity_lock_mutex );
          pthread_exit(NULL);
        }
#endif

        QCRIL_MUTEX_UNLOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );
        err_no = qcril_process_event( ev->instance_id, ev->modem_id, ev->event_id, ev->data, ev->datalen, ev->t );
        QCRIL_MUTEX_LOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );

        boolean to_check_ev_data_free = TRUE;
#ifdef FEATURE_QCRIL_UIM_REMOTE_CLIENT
        if(ev->event_id > QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_BASE && ev->event_id < QCRIL_EVT_UIM_REMOTE_CLIENT_SOCKET_REQ_MAX)
        {
            //uim remote req must free it
            if(ev->data)
            {
                com_qualcomm_uimremoteclient_MessageId msg_id = (com_qualcomm_uimremoteclient_MessageId) qcril_uim_remote_client_map_event_to_request(ev->event_id);
                QCRIL_LOG_INFO("freeing request message for %s(id: %d)", qcril_uim_remote_client_get_msg_log_str(msg_id, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_REQUEST), msg_id);
                const pb_field_t* pb_fields = qcril_uim_remote_client_get_msg_pb_fields(msg_id, com_qualcomm_uimremoteclient_MessageType_UIM_REMOTE_MSG_REQUEST);
                if(pb_fields)
                {
                    qcril_qmi_npb_release(pb_fields, ev->data);
                }
                else
                {
                    QCRIL_LOG_ERROR("missing pb fields");
                }
                qcril_free(ev->data);
            }
            to_check_ev_data_free = false;
        }
#endif
        if ( to_check_ev_data_free && ev->data_must_be_freed && ev->data )
        {
          qcril_free( ev->data );
        }

        qcril_free( ev );

        if (err_no != E_SUCCESS)
        {
          QCRIL_LOG_ERROR("Error processing event! (%d)", err_no);
        }

      }

      go_on = ( ( NULL != ( ev = qcril_event.list.next ) && ( ev != &qcril_event.list ) ) );
    } while ( go_on );

    QCRIL_MUTEX_UNLOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );
    pthread_mutex_unlock( &qcril_event.activity_lock_mutex );

    release_wake_lock( QCRIL_WAKE_LOCK_NAME );

  }
  QCRIL_LOG_FUNC_RETURN();

  qmi_ril_clear_thread_name(pthread_self());
  return NULL;

} /* qcril_event_main() */


/*===========================================================================

  FUNCTION:  qcril_event_queue

===========================================================================*/
/*!
    @brief
    Queues an event to be dispatched in the the event thread.
    This queue allows RPC callbacks from AMSS to return before the event
    is processed.

    @return
    E_SUCCESS If the queue operation was successful, approprate failure code otherwise.

*/
/*=========================================================================*/
IxErrnoType qcril_event_queue
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_data_src_e_type data_src,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  int ret;
  qcril_event_type *ev;
  IxErrnoType result;

  /*-----------------------------------------------------------------------*/

  do
  {
    result = E_SUCCESS;
    ev = (qcril_event_type *) qcril_malloc( sizeof(qcril_event_type) );
    if (!ev) {
      result = E_NO_MEMORY;
      QCRIL_LOG_ERROR("Failed to allocate memory, aborting!");
      QCRIL_ASSERT(0); // this is a noop in release build
      break;
    }

    ev->instance_id = instance_id;
    ev->modem_id = modem_id;
    ev->event_id = event_id;
    ev->t = t;
    /* If the caller is passing a pointer to data on the stack,
       make a copy and free it when when we are done */
    ev->data_must_be_freed = data_src;

    if ( data == NULL )
    {
      QCRIL_LOG_DEBUG("Event [%s, %x] data was NULL", qcril_log_lookup_event_name( event_id ), event_id );
      ev->data = NULL;
      ev->datalen = 0;
    }
    else if ( data_src == QCRIL_DATA_ON_STACK )
    {
      /* Caller passed a pointer to stack data; make a copy */
      ev->data = qcril_malloc( datalen );
      if (!ev->data)
      {
        result = E_NO_MEMORY;
        QCRIL_LOG_ERROR("Failed to allocate memory, aborting!");
        QCRIL_ASSERT(0); // this is a noop in release build
        break;
      }
      memcpy(ev->data, data, datalen );
      ev->datalen = datalen;
    }
    else
    {
      /* Caller passed a pointer to heap data */
      ev->data = data;
      ev->datalen = datalen;
    }

    QCRIL_MUTEX_LOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );
    qcril_event_add_to_list( ev, &qcril_event.list );
    QCRIL_MUTEX_UNLOCK( &qcril_event.list_mutex, "[Event Thread] qcril_event.list_mutex" );

    if (!pthread_equal(pthread_self(), qcril_event.tid))
    {
      /* Wake up the event thread. */
      do
      {
        ret = write (qcril_event.fdWakeupWrite, " ", 1);
      } while (ret < 0 && errno == EINTR);
    }
    QCRIL_LOG_DEBUG( "RID %d MID %d Queued event %s (%d bytes)(obj 0x%"PRIxPTR")",
                      instance_id, modem_id,
                      qcril_log_lookup_event_name((int) event_id),
                      datalen, (intptr_t)ev );
  } while(0);

  if(E_SUCCESS != result)
  {
    if(ev)
    {
      if(ev->data && QCRIL_DATA_ON_STACK == ev->data_must_be_freed)
      {
        qcril_free(ev->data);
      }
      qcril_free(ev);
    }
  }

  return result;
} /* qcril_event_queue */


/*===========================================================================

  FUNCTION:  qcril_event_init

===========================================================================*/
/*!
    @brief
    Creates a new thread for processing events received via RPC calls
    from AMSS.

    @return
    Void
*/
/*=========================================================================*/
void qcril_event_init( void )
{
  pthread_attr_t attr;
  int ret;
  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_event.startup_mutex, "[Event Thread] qcril_event.startup_mutex" );

  qcril_event.started = 0;

  #ifdef QMI_RIL_UTF
  pthread_attr_init (&attr);
  ret = utf_pthread_create_handler(&qcril_event.tid, &attr, qcril_event_main, NULL);
  pthread_attr_destroy( &attr );
  #else
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  ret = pthread_create(&qcril_event.tid, &attr, qcril_event_main, NULL);
  pthread_attr_destroy( &attr );
  #endif
  qmi_ril_set_thread_name(qcril_event.tid, QMI_RIL_EVENT_THREAD_NAME);

  if (ret < 0)
  {
    QCRIL_LOG_ERROR("Failed to start event AMSS callback thread!");
    QCRIL_MUTEX_UNLOCK( &qcril_event.startup_mutex, "[Event Thread] qcril_event.startup_mutex" );
    QCRIL_ASSERT(0); // this is a noop in release build
    return;
  }

  pthread_mutexattr_init( &qcril_event.activity_lock_mutex_atr );
  pthread_mutex_init( &qcril_event.activity_lock_mutex,
                      &qcril_event.activity_lock_mutex_atr );


  while (qcril_event.started == 0)
  {
    pthread_cond_wait(&qcril_event_startupCond, &qcril_event.startup_mutex);
  }

  QCRIL_LOG_INFO("event thread started");

  QCRIL_MUTEX_UNLOCK( &qcril_event.startup_mutex, "[Main Thread] qcril_event.startup_mutex" );

} /* qcril_event_init() */

/*===========================================================================

  FUNCTION:  qcril_event_start

===========================================================================*/
/*!
    @brief
    Starts the event loop. needs to wait until after RIL_Init is finished
    so that ril.cpp has some time to complete RIL_Register before we start
    calling onUnsolicitedResponse.

    @return
    Void
*/
/*=========================================================================*/
void qcril_event_start( void )
{
  QCRIL_MUTEX_LOCK( &qcril_event.startup_mutex, "[Main Thread] qcril_event.startup_mutex" );

  qcril_event.started = 2;

  pthread_cond_broadcast(&qcril_event_startupCond);

  QCRIL_MUTEX_UNLOCK( &qcril_event.startup_mutex, "[Main Thread] qcril_event.startup_mutex" );

} /* qcril_event_start() */
//===========================================================================
//qmi_ril_leave_critical_section
//===========================================================================
void qcril_event_suspend()
{
  pthread_mutex_lock( &qcril_event.activity_lock_mutex );
} // qcril_event_suspend
//===========================================================================
//qcril_event_resume
//===========================================================================
void qcril_event_resume()
{
  pthread_mutex_unlock( &qcril_event.activity_lock_mutex );
} // qcril_event_resume

