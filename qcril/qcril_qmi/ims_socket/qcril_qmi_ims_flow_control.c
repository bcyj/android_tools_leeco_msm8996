/******************************************************************************
  @file    qcril_qmi_ims_flow_control.c
  @brief   IMS request flow control logic

  DESCRIPTION

----------------------------------------------------------------------------
  Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

---------------------------------------------------------------------------

******************************************************************************/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <cutils/properties.h>

#include "ril.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_client.h"
#include "qcril_qmi_ims_flow_control.h"
#include "qcril_qmi_ims_socket.h"
#include "qcril_qmi_ims_misc.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define IMS_FLOW_CONTROL_LIST_LOCK() do { \
        QCRIL_LOG_INFO("LOCK IMS_FLOW_CONTROL_LIST_LOCK"); \
        pthread_mutex_lock(&qcril_ifc_info.list_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_LIST_UNLOCK() do { \
        QCRIL_LOG_INFO("UNLOCK IMS_FLOW_CONTROL_LIST_LOCK"); \
        pthread_mutex_unlock(&qcril_ifc_info.list_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_PROCESS_LOCK() do { \
        QCRIL_LOG_INFO("LOCK IMS_FLOW_CONTROL_PROCESS_LOCK"); \
        pthread_mutex_lock(&qcril_ifc_info.ims_flow_control_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_PROCESS_UNLOCK() do { \
        QCRIL_LOG_INFO("UNLOCK IMS_FLOW_CONTROL_PROCESS_LOCK"); \
        pthread_mutex_unlock(&qcril_ifc_info.ims_flow_control_mutex); \
    }while(0)

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/
static qcril_ims_flow_control_type qcril_ifc_info;
static qcril_qmi_ims_family_ring_list_type *qcril_ims_flow_control_family_ring;
static qcril_qmi_ims_flow_control_fw_req_overview *qcril_ims_flow_control_req_overview = NULL;


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES


===========================================================================*/



/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES


===========================================================================*/
boolean qcril_qmi_is_flow_control_required(qcril_evt_e_type event_id);
void qcril_qmi_ims_flow_control_req_timeout_handler
(
  qcril_timed_callback_handler_params_type *param
);
void qcril_qmi_ims_flow_control_get_next_req_list_entry
(
  qcril_qmi_ims_flow_control_fw_request_list *req_list,
  qcril_qmi_ims_flow_control_fw_request_list **next_req_list
);



/*===========================================================================

                                FUNCTIONS

===========================================================================*/
//////////////////////////////////////////////////////////////////////////////
//                             Main routines                                //
//////////////////////////////////////////////////////////////////////////////
//IMS flow control init routine
/***************************************************************************************************
    @function
    qcril_ims_flow_control_pre_init
***************************************************************************************************/
void qcril_ims_flow_control_pre_init()
{
  pthread_mutexattr_t attr;
  pthread_attr_t      pthread_attr;
  int                 ret;
  int                 itr=0;
  char property_name[ 40 ];
  char args[ PROPERTY_VALUE_MAX ];
  int property_param_len;
  char *end_ptr;
  unsigned long ret_val = 0;
  RIL_Errno res = RIL_E_SUCCESS;
  int dtmf_interval = QCRIL_QMI_VOICE_DTMF_INTERVAL_VAL;

  QCRIL_LOG_FUNC_ENTRY();

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init( &qcril_ifc_info.ims_flow_control_mutex, &attr );
  pthread_attr_init (&pthread_attr);
#ifndef QMI_RIL_UTF
   pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
   ret = pthread_create(&qcril_ifc_info.ims_flow_control_thread_id,
           &pthread_attr, qcril_qmi_ims_flow_control_main, NULL);
#else
   ret = utf_pthread_create_handler(&qcril_ifc_info.ims_flow_control_thread_id,
           &pthread_attr, qcril_qmi_ims_flow_control_main, NULL);
#endif

  if (ret == 0)
  {
    qmi_ril_set_thread_name(qcril_ifc_info.ims_flow_control_thread_id, QMI_RIL_IFC_THREAD_NAME);
    QCRIL_LOG_INFO("RIL IMS flow control thread started");
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to start IMS flow control thread!");
    QCRIL_ASSERT(0); // this is a noop in release build
    return;
  }

  //Read DTMF interval from adb property
  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s",
      QCRIL_QMI_VOICE_DTMF_INTERVAL);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QCRIL_QMI_VOICE_DTMF_INTERVAL %s",
          args );
    }
    else
    {
      dtmf_interval = ret_val;
    }
  }
  QCRIL_LOG_DEBUG("QCRIL_QMI_VOICE_DTMF_INTERVAL=%d", dtmf_interval);

  //Initiliaze ims_flow_control family ring
  qcril_ims_flow_control_family_ring =
              qcril_malloc(sizeof(qcril_qmi_ims_family_ring_list_type));
  if(qcril_ims_flow_control_family_ring != NULL)
  {
    QCRIL_LOG_DEBUG("Init dtmf request group for flow control");
    qcril_ims_flow_control_family_ring->max_req =
                     (sizeof(ims_flow_control_family_ring_dtmf_req) /
                      sizeof(ims_flow_control_family_ring_dtmf_req[0]));
    qcril_ims_flow_control_family_ring->action_on_dup_req =
                                              QCRIL_QMI_IMS_WAIT_FOR_RESP;
    qcril_ims_flow_control_family_ring->timer = dtmf_interval;
    qcril_ims_flow_control_family_ring->ims_flow_control_family_ring =
                qcril_malloc(qcril_ims_flow_control_family_ring->max_req);
    if(qcril_ims_flow_control_family_ring->ims_flow_control_family_ring)
    {
      for(itr=0; itr<qcril_ims_flow_control_family_ring->max_req; itr ++)
      {
        qcril_ims_flow_control_family_ring->ims_flow_control_family_ring[itr] =
                                     ims_flow_control_family_ring_dtmf_req[itr];
      }
    }
    qcril_ims_flow_control_family_ring->next = NULL;
  }

  pthread_mutexattr_destroy(&attr);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_init_list

===========================================================================*/
/*!
    @brief
    Initializes the list

    @return
    Void
*/
/*=========================================================================*/
static void qcril_ims_flow_control_init_list
(
  qcril_ims_flow_control_list_type *list
)
{
  memset(list, 0, sizeof(qcril_ims_flow_control_list_type));
  list->next = list;
  list->prev = list;
  list->event_id = QCRIL_EVT_NONE;
} /* qcril_ims_flow_control_init_list() */

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_add_to_list

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
static void qcril_ims_flow_control_add_to_list
(
  qcril_ims_flow_control_list_type *ev,
  qcril_ims_flow_control_list_type *list
)
{
  ev->next = list;
  ev->prev = list->prev;
  ev->prev->next = ev;
  list->prev = ev;
} /* qcril_ims_flow_control_add_to_list() */

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_remove_from_list

===========================================================================*/
/*!
    @brief
    Removes an entry from the list.

    @return
*/
/*=========================================================================*/
static void qcril_ims_flow_control_remove_from_list
(
  qcril_ims_flow_control_list_type *ev
)
{
  ev->next->prev = ev->prev;
  ev->prev->next = ev->next;
  ev->next = NULL;
  ev->prev = NULL;
} /* qcril_ims_flow_control_remove_from_list() */

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_main

===========================================================================*/
/*!
    @brief
    Main loop of the thread to process events queued in ims
    flow control thread

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_main()
{
    qcril_ims_flow_control_list_type *ev;
    int    filedes[2];
    fd_set rfds;
    char   buff[16];
    int    go_on;
    int    ret;
    int    n;

    QCRIL_LOG_FUNC_ENTRY();

    /*-----------------------------------------------------------------------*/
    pthread_mutex_init(&qcril_ifc_info.ims_flow_control_mutex, NULL);
    qcril_ims_flow_control_init_list(&qcril_ifc_info.list);

    FD_ZERO(&qcril_ifc_info.readFds);

    /* Create a pipe so main thread can wake us up */
    ret = pipe(filedes);

    if (ret < 0)
    {
        QCRIL_LOG_ERROR("Error opening pipe (%d)", errno);
        qmi_ril_clear_thread_name(pthread_self());
        return NULL;
    }

    qcril_ifc_info.fdWakeupRead = filedes[0];
    qcril_ifc_info.fdWakeupWrite = filedes[1];

    fcntl(qcril_ifc_info.fdWakeupRead, F_SETFL, O_NONBLOCK);
    FD_SET(qcril_ifc_info.fdWakeupRead, &qcril_ifc_info.readFds);

    do
    {
        memcpy(&rfds, &qcril_ifc_info.readFds, sizeof(fd_set));

        QCRIL_LOG_DEBUG("Waiting...");

        /* Block waiting for a event to be put on the queue */
        n = select(qcril_ifc_info.fdWakeupRead + 1, &rfds, NULL, NULL, NULL);
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
            ret = read(qcril_ifc_info.fdWakeupRead, &buff, sizeof(buff));
            if (ret > 0)
            {
                QCRIL_LOG_DEBUG("%d items on queue", ret);
            }
        } while (ret > 0 || (ret < 0 && errno == EINTR));

        IMS_FLOW_CONTROL_LIST_LOCK();

        do
        {
            if ((NULL != (ev = qcril_ifc_info.list.next) && (ev != &qcril_ifc_info.list)))
            {
                qcril_ims_flow_control_remove_from_list(ev);

                QCRIL_LOG_DEBUG("De-queued ims flow control event %s(%d)",
                   qcril_log_lookup_event_name(ev->event_id), ev->event_id);

#ifdef QMI_RIL_UTF
                // shutdown thread upon request
                if (ev->event_id == -1)
                {
                  close(filedes[0]);
                  close(filedes[1]);
                  qmi_ril_clear_thread_name(pthread_self());
                  IMS_FLOW_CONTROL_LIST_UNLOCK();
                  pthread_mutex_unlock( &qcril_ifc_info.ims_flow_control_mutex );
                  pthread_exit(NULL);
                }
#endif
                IMS_FLOW_CONTROL_LIST_UNLOCK();
                switch(ev->req_type)
                {
                  case QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_REQ:
                    qcril_ims_flow_control_process_request( ev->event_id,
                                                            ev->data,
                                                            ev->datalen,
                                                            ev->t);
                    break;

                  case QCRIL_QMI_IMS_FLOW_CONTROL_REQ_COMPLETE:
                    qcril_ims_flow_control_request_complete( ev->event_id,
                                                             ev->t );
                    break;

                  case QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_NEXT_REQ:
                    qcril_ims_flow_control_handle_next_request( ev->event_id,
                                                                ev->t );
                    break;

                  case QCRIL_QMI_IMS_FLOW_CONTROL_CLEAR_LIST:
                    qcril_ims_flow_control_clear_list();
                    break;

                  default:
                    QCRIL_LOG_ERROR("Invalid req_type %d", ev->req_type);
                    break;
                }
                IMS_FLOW_CONTROL_LIST_LOCK();

                if(ev)
                {
                  if(ev->data && QCRIL_DATA_ON_STACK == ev->data_must_be_freed)
                  {
                    qcril_free(ev->data);
                  }
                  qcril_free(ev);
                }
            }
            go_on = ((NULL != (ev = qcril_ifc_info.list.next) && (ev != &qcril_ifc_info.list)));
        } while ( go_on );

        IMS_FLOW_CONTROL_LIST_UNLOCK();
    } while(1);

    qmi_ril_clear_thread_name(pthread_self());

    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_event_queue

===========================================================================*/
/*!
    @brief
    Queues an event to be dispatched in the the ims flow control thread.

    @return
    E_SUCCESS If the queue operation was successful, approprate failure code otherwise.

*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_event_queue
(
  qcril_qmi_ims_flow_control_req_type req_type,
  qcril_data_src_e_type data_src,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
    qcril_ims_flow_control_list_type *ev;
    RIL_Errno           result;
    int                 ret;

    /*-----------------------------------------------------------------------*/
    do
    {
        result = RIL_E_SUCCESS;
        ev = (qcril_ims_flow_control_list_type *) qcril_malloc( sizeof(qcril_ims_flow_control_list_type) );
        if (!ev) {
            result = RIL_E_GENERIC_FAILURE;
            QCRIL_LOG_ERROR("Failed to allocate memory, aborting!");
            QCRIL_ASSERT(0); // this is a noop in release build
            break;
        }

        ev->req_type = req_type;
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

        IMS_FLOW_CONTROL_LIST_LOCK();
        qcril_ims_flow_control_add_to_list( ev, &qcril_ifc_info.list );
        IMS_FLOW_CONTROL_LIST_UNLOCK();

        if (!pthread_equal(pthread_self(), qcril_ifc_info.ims_flow_control_thread_id))
        {
            /* Wake up the IMS flow control thread. */
            do
            {
                ret = write (qcril_ifc_info.fdWakeupWrite, " ", 1);
            } while (ret < 0 && errno == EINTR);
        }
        QCRIL_LOG_DEBUG("Queued IMS event %s(%d)", qcril_log_lookup_event_name(event_id), event_id);
    } while(0);

    if(RIL_E_SUCCESS != result)
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
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_process_request

===========================================================================*/
/*!
    @brief

    @return
    E_SUCCESS of the event was handled normally
    E_NOT_SUPPORTED if the event_id was invalid
    E_NOT_ALLOWED if the event is not supported in the current state

*/
/*=========================================================================*/
void qcril_ims_flow_control_process_request
(
  qcril_evt_e_type                    event_id,
  void                                *data,
  size_t                              datalen,
  RIL_Token                           t
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *orig_req_list = NULL;
  boolean flow_control_required = TRUE;

  QCRIL_LOG_FUNC_ENTRY();

  IMS_FLOW_CONTROL_PROCESS_LOCK();

  if(qcril_qmi_is_flow_control_required(event_id) == TRUE)
  {
    QCRIL_LOG_INFO("Flow control required for %s(%d)",
                qcril_log_lookup_event_name(event_id), event_id);

    req_node = qcril_qmi_ims_flow_control_fw_create_node(t, event_id, data, datalen);
    if(req_node)
    {
      qcril_qmi_ims_flow_control_fw_check_req_from_family_ring(req_node, &orig_req_list);

      if(orig_req_list == NULL)
      {
        QCRIL_LOG_INFO("Req not found in flow control list %s(%d)",
                  qcril_log_lookup_event_name(event_id), event_id);
        qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                       QCRIL_DEFAULT_MODEM_ID,
                       QCRIL_DATA_NOT_ON_STACK,
                       event_id, data, datalen, t);

        req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_IN_PROGRESS;
        //Create qcril_qmi_ims_flow_control_fw_request_list and add req_node to it
        //Add qcril_qmi_ims_flow_control_fw_request_list to overview_list
        qcril_qmi_ims_flow_control_add_req_node(req_node, &orig_req_list);
        qcril_qmi_ims_flow_control_add_list_overview(orig_req_list);
      }
      else
      {
        QCRIL_LOG_INFO("Req found in flow control list %s(%d)",
                  qcril_log_lookup_event_name(event_id), event_id);
        switch(orig_req_list->action_on_dup_req)
        {
          case QCRIL_QMI_IMS_SEND_SUCCESS_RESP:
            QCRIL_LOG_INFO("Sending success response for duplicate request");
            qcril_qmi_ims_socket_send(t, IMS__MSG_TYPE__RESPONSE,
                                      qcril_qmi_ims_map_event_to_request(event_id),
                                      IMS__ERROR__E_SUCCESS,
                                      NULL, 0);
            break;

          case QCRIL_QMI_IMS_SEND_FAILURE_RESP:
            QCRIL_LOG_INFO("Sending error response for duplicate request");
            qcril_qmi_ims_socket_send(t, IMS__MSG_TYPE__RESPONSE,
                                      qcril_qmi_ims_map_event_to_request(event_id),
                                      IMS__ERROR__E_GENERIC_FAILURE,
                                      NULL, 0);
            break;

          case QCRIL_QMI_IMS_WAIT_FOR_RESP:
            QCRIL_LOG_INFO("Queue req and wait for prev request response");
            qcril_qmi_ims_flow_control_add_req_node(req_node, &orig_req_list);
            req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_IN_QUEUE;
            break;

          default:
            QCRIL_LOG_ERROR("No action defined for this family ring");
            break;
        }
      }
    }
    else
    {
      QCRIL_LOG_ERROR("Failed to create node... Skip flow control...");
      flow_control_required = FALSE;
    }
  }
  else
  {
    QCRIL_LOG_INFO("Flow control not required for %s(%d)",
              qcril_log_lookup_event_name(event_id), event_id);
    flow_control_required = FALSE;
  }

  if(flow_control_required == FALSE)
  {
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_NOT_ON_STACK,
                   event_id, data, datalen, t);
  }
  IMS_FLOW_CONTROL_PROCESS_UNLOCK();

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_handle_next_request

===========================================================================*/
/*!
    @brief
    Retrieve next event queued in flow control, if any
    and process the same.

    @return

*/
/*=========================================================================*/
void qcril_ims_flow_control_handle_next_request
(
  qcril_evt_e_type  event_id,
  RIL_Token         token
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *next_req_list = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  IMS_FLOW_CONTROL_PROCESS_LOCK();
  req_list = qcril_qmi_ims_flow_control_get_req_list_entry(token, event_id);
  if(req_list != NULL)
  {
    QCRIL_LOG_INFO("Req found in flow control list for token %d", token);
    qcril_qmi_ims_flow_control_get_next_req_list_entry(req_list, &next_req_list);
  }
  else
  {
    QCRIL_LOG_INFO("Req not found in flow control list for token %d", token);
  }

  if(next_req_list)
  {
    if(next_req_list->req_node)
    {
      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                         QCRIL_DEFAULT_MODEM_ID,
                         QCRIL_DATA_NOT_ON_STACK,
                         next_req_list->req_node->req_id, next_req_list->req_node->payload,
                         next_req_list->req_node->payload_len, next_req_list->req_node->token);
      next_req_list->req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_IN_PROGRESS;
    }
    else
    {
      QCRIL_LOG_ERROR("req_node not found");
    }
  }
  else
  {
    QCRIL_LOG_INFO("End of the list");
  }

  IMS_FLOW_CONTROL_PROCESS_UNLOCK();

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_request_complete

===========================================================================*/
/*!
    @brief
    Retrieve next event queued in flow control, if any
    and process the same.

    @return

*/
/*=========================================================================*/
void qcril_ims_flow_control_request_complete
(
  qcril_evt_e_type  event_id,
  RIL_Token         token
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *next_req_list = NULL;
  struct timeval timeout_value = {0, 0};

  QCRIL_LOG_FUNC_ENTRY();
  IMS_FLOW_CONTROL_PROCESS_LOCK();
  req_list = qcril_qmi_ims_flow_control_get_req_list_entry(token, event_id);

  if(req_list != NULL)
  {
    if(req_list->req_node != NULL)
    {
      if(req_list->timer > QMI_RIL_ZERO)
      {
        QCRIL_LOG_INFO("Start timer of %d ms to process next request",
                                                       req_list->timer);
        timeout_value.tv_sec = QMI_RIL_ZERO;
        timeout_value.tv_usec = (req_list->timer) * 1000;
        qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                              QCRIL_DEFAULT_MODEM_ID,
                                              qcril_qmi_ims_flow_control_req_timeout_handler,
                                              (void *)(req_list->req_node),
                                              &timeout_value,
                                              NULL);
      }
      else
      {
        IMS_FLOW_CONTROL_PROCESS_UNLOCK();
        qcril_ims_flow_control_handle_next_request(req_list->req_node->req_id,
                                                   req_list->req_node->token);
        IMS_FLOW_CONTROL_PROCESS_LOCK();
      }
    }
    else
    {
      QCRIL_LOG_ERROR("req_node is empty - Not an expected scenario");
    }
  }
  else
  {
     QCRIL_LOG_INFO("No req_list entry with this token %d", token);
  }
  IMS_FLOW_CONTROL_PROCESS_UNLOCK();
  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_req_timeout_handler

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_req_timeout_handler
(
  qcril_timed_callback_handler_params_type *param
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node;

  QCRIL_LOG_FUNC_ENTRY();

  req_node = (qcril_qmi_ims_flow_control_fw_request_holder *)param->custom_param;

  qcril_qmi_ims_flow_control_event_queue(QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_NEXT_REQ,
                                         QCRIL_DATA_NOT_ON_STACK,
                                         req_node->req_id, NULL, 0,
                                         req_node->token);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_clear_list

===========================================================================*/
/*!
    @brief
    Clear all the flow control entrie

    @return
*/
/*=========================================================================*/
void qcril_ims_flow_control_clear_list()
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;

  req_overview = qcril_ims_flow_control_req_overview;

  QCRIL_LOG_FUNC_ENTRY();

  IMS_FLOW_CONTROL_PROCESS_LOCK();

  while(req_overview)
  {
    req_list = req_overview->list_head;
    while(req_list)
    {
      req_node = req_list->req_node;
      //Send Cancelled response for events in queue
      if(req_node->req_state == QCRIL_QMI_IMS_REQ_STATE_IN_QUEUE)
      {
        qcril_qmi_ims_socket_send_flow_control( req_node->token,
                        IMS__MSG_TYPE__RESPONSE,
                        qcril_qmi_ims_map_event_to_request(req_node->req_id),
                        IMS__ERROR__E_CANCELLED, NULL, 0 );
      }
      qcril_free(req_node);

      req_overview->list_head = req_list->next;
      qcril_free(req_list);

      req_list = req_overview->list_head;
    }

    req_overview = req_overview->next;
    qcril_free(qcril_ims_flow_control_req_overview);

    qcril_ims_flow_control_req_overview = req_overview;
  }

  qcril_ims_flow_control_req_overview = NULL;

  IMS_FLOW_CONTROL_PROCESS_UNLOCK();

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_add_to_list

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
qcril_qmi_ims_flow_control_fw_request_holder *
qcril_qmi_ims_flow_control_fw_create_node
(
  RIL_Token token,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;

  req_node = qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_request_holder));
  if(req_node)
  {
    req_node->token = token;
    req_node->req_id = event_id;
    req_node->payload = data;
    req_node->payload_len = datalen;
    req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_NONE;
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to create req_node");
  }

  return req_node;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_fw_check_req_from_family_ring

===========================================================================*/
/*!
    @brief
    Check any req_list entry exists in overview list
    from event_id family ring

    @return
    qcril_qmi_ims_flow_control_fw_request_list
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_fw_check_req_from_family_ring
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **orig_req_list
)
{
  boolean req_found = FALSE;
  int itr = 0;
  qcril_qmi_ims_family_ring_list_type *req_grp_list = NULL;
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  req_overview = qcril_ims_flow_control_req_overview;

  if(req_overview)
  {
    req_grp_list = qcril_ims_flow_control_family_ring;

    while (req_grp_list)
    {
      for(itr=0; itr<req_grp_list->max_req; itr++)
      {
        if(req_node->req_id == req_grp_list->ims_flow_control_family_ring[itr])
        {
          req_found = TRUE;
          break;
        }
      }
      if(req_found) break;
      req_grp_list = req_grp_list->next;
    }

    QCRIL_LOG_INFO("Check for entry in overview list now");
    if(req_found)
    {
      while(req_overview)
      {
        req_list = req_overview->list_head;
        if(req_list && req_list->req_node)
        {
          for(itr=0; itr<req_grp_list->max_req; itr++)
          {
            if(req_list->req_node->req_id ==
                req_grp_list->ims_flow_control_family_ring[itr])
            {
              QCRIL_LOG_INFO("Entry found in overview list");
              *orig_req_list = req_list;
              break;
            }
          }
        }
        if(*orig_req_list) break;
        req_overview = req_overview->next;
      }
    }
    else
    {
      QCRIL_LOG_INFO("Req not found in family ring");
    }
  }
  else
  {
    QCRIL_LOG_INFO("Overview list is empty");
  }

  QCRIL_LOG_FUNC_RETURN();
}

//////////////////////////////////////////////////////////////////////////////
//                            Helper routines                               //
//////////////////////////////////////////////////////////////////////////////

/*===========================================================================

  FUNCTION:  qcril_qmi_is_flow_control_required

===========================================================================*/
/*!
    @brief
    Checks whether event_id requires flow control

    @return
*/
/*=========================================================================*/
boolean qcril_qmi_is_flow_control_required(qcril_evt_e_type event_id)
{
  boolean ret = FALSE;
  int itr = 0;
  qcril_qmi_ims_family_ring_list_type *req_grp_list = NULL;

  req_grp_list = qcril_ims_flow_control_family_ring;

  while (req_grp_list)
  {
    for(itr=0; itr<req_grp_list->max_req; itr++)
    {
      if(event_id == req_grp_list->ims_flow_control_family_ring[itr])
      {
        ret = TRUE;
        break;
      }
    }
    req_grp_list = req_grp_list->next;
  }

  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_get_req_params

===========================================================================*/
/*!
    @brief
    Read event associated family ring params

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_get_req_params
(
  qcril_evt_e_type event_id,
  qcril_qmi_ims_flow_control_fw_request_action *req_action,
  int *timer
)
{
  int itr = 0;
  boolean req_node_found = FALSE;
  qcril_qmi_ims_family_ring_list_type *req_grp_list = NULL;

  req_grp_list = qcril_ims_flow_control_family_ring;

  QCRIL_LOG_FUNC_ENTRY();

  while (req_grp_list)
  {
    for(itr=0; itr<req_grp_list->max_req; itr++)
    {
      if(event_id == req_grp_list->ims_flow_control_family_ring[itr])
      {
        *req_action = req_grp_list->action_on_dup_req;
        *timer = req_grp_list->timer;
        QCRIL_LOG_INFO("req_action: %d, timer: %d", *req_action, *timer);
        req_node_found = TRUE;
        break;
      }
    }
    if(req_node_found) break;
    req_grp_list = req_grp_list->next;
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_add_req_node

===========================================================================*/
/*!
    @brief
    Adds request node to family rings request list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_add_req_node
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **req_list
)
{
  qcril_qmi_ims_flow_control_fw_request_list *trav_req_list = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  trav_req_list = *req_list;

  if(trav_req_list)
  {
    while(trav_req_list->next)
    {
      trav_req_list = trav_req_list->next;
    }
    trav_req_list->next =
         qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_request_list));
    trav_req_list = trav_req_list->next;
  }
  else
  {
    //Create new req_list entry
    QCRIL_LOG_INFO("Create new req_list entry");
    trav_req_list =
         qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_request_list));
    *req_list = trav_req_list;
  }

  if(trav_req_list)
  {
    trav_req_list->req_node = req_node;
    qcril_qmi_ims_flow_control_get_req_params(req_node->req_id,
                                  &trav_req_list->action_on_dup_req,
                                  &trav_req_list->timer);
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to create req_list entry");
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_add_list_overview

===========================================================================*/
/*!
    @brief
    Add req_list to flow control overview list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_add_list_overview
(
qcril_qmi_ims_flow_control_fw_request_list *req_list
)
{
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  req_overview = qcril_ims_flow_control_req_overview;

  if(req_overview)
  {
    while(req_overview->next)
    {
      req_overview = req_overview->next;
    }
    req_overview->next =
             qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_req_overview));
    req_overview = req_overview->next;
  }
  else
  {
    QCRIL_LOG_INFO("Overview list is empty");
    req_overview =
             qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_req_overview));
    qcril_ims_flow_control_req_overview = req_overview;
  }

  if(req_overview)
  {
    req_overview->list_head = req_list;
    req_overview->next = NULL;
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to create req_overview");
  }
  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_get_next_req_list_entry

===========================================================================*/
/*!
    @brief
    Add req_list to flow control overview list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_get_next_req_list_entry
(
  qcril_qmi_ims_flow_control_fw_request_list *req_list,
  qcril_qmi_ims_flow_control_fw_request_list **next_req_list
)
{
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list_trav = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  req_overview = qcril_ims_flow_control_req_overview;

  do
  {
    while(req_overview && (req_list != req_overview->list_head))
    {
      req_overview = req_overview->next;
    }

    if(req_overview)
    {
      req_list_trav = req_overview->list_head;
      req_overview->list_head = req_overview->list_head->next;
      *next_req_list = req_overview->list_head;
      if(*next_req_list && (*next_req_list)->req_node)
      {
        QCRIL_LOG_INFO("req_id: %s(%d), payload 0x%x payload_len %d",
                     qcril_log_lookup_event_name((*next_req_list)->req_node->req_id),
                     (*next_req_list)->req_node->req_id,
                     (*next_req_list)->req_node->payload,
                     (*next_req_list)->req_node->payload_len);
      }

      qcril_free(req_list_trav->req_node);
      qcril_free(req_list_trav);

      if(req_overview->list_head == NULL)
      {
        qcril_ims_flow_control_req_overview = req_overview->next;
        qcril_free(req_overview);
      }
    }
    else
    {
      QCRIL_LOG_ERROR("Overview list not found");
      break;
    }
  } while(FALSE);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_get_req_list_entry

===========================================================================*/
/*!
    @brief
    Find req_list entry based on RIL request token

    @return
    qcril_qmi_ims_flow_control_fw_request_list
*/
/*=========================================================================*/
qcril_qmi_ims_flow_control_fw_request_list *
qcril_qmi_ims_flow_control_get_req_list_entry
(
  RIL_Token         token,
  qcril_evt_e_type  req_id
)
{
  boolean req_list_found = FALSE;
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;

  req_overview = qcril_ims_flow_control_req_overview;

  while(req_overview)
  {
    req_list = req_overview->list_head;
    while(req_list)
    {
       if(token == req_list->req_node->token &&
          req_id == req_list->req_node->req_id)
       {
           req_list_found = TRUE;
           break;
       }
       req_list = req_list->next;
    }
    if(req_list_found) break;
    req_overview = req_overview->next;
  }

  if(!req_list_found)
  {
    QCRIL_LOG_INFO("Not found req_list entry with token %d event %d",token,req_id);
  }

  return req_list;
}
