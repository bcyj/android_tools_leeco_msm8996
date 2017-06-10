/******************************************************************************
  @file    qmi.c
  @brief   The QMI management layer.  This includes system-wide intialization
  and configuration funtions.

  DESCRIPTION
  QMI management.  Routines for client, system-wide initialization
  and de-initialization

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_init() and qmi_connection_init() needs to be called before starting
  any of the specific service clients.

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2011 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include "qmi.h"
#include "qmi_i.h"
#include "qmi_service.h"

/* Minimum value for init client handle.  This must be a positive integer value */
#define QMI_MIN_INIT_CLIENT_HANDLE_VALUE  0x000000FF


static int qmi_next_client_id = QMI_MIN_INIT_CLIENT_HANDLE_VALUE;

QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX (qmi_mutex);
QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX (qmi_client_list_mutex);

static int qmi_initialized = FALSE;


/* Client registration structure.  One is created for each new client
** and they are stored in a linked list
*/
typedef struct qmi_sys_event_client_data_type
{
  struct qmi_sys_event_client_data_type   *next;

  qmi_sys_event_rx_hdlr                   sys_event_rx_hdlr;
  void                                    *sys_event_user_data;
  int                                     init_client_handle;
} qmi_sys_event_client_data_type;

static qmi_sys_event_client_data_type *qmi_sys_event_client_list = NULL;

/*===========================================================================
  FUNCTION  qmi_get_next_init_client_handle
===========================================================================*/
/*!
@brief
  This function generates an init_client_handle that is a positive value
  and not currently used by another client.  Assumption is that the qmi_mutex
  is locked by calling function.

@return

@note
  - qmi_mutex must be locked by calling function prior to calling this
*/
/*=========================================================================*/
static int
qmi_get_next_init_client_handle
(
  void
)
{
  int new_handle, unique_client_id;
  qmi_sys_event_client_data_type *item = NULL;
  qmi_sys_event_client_data_type *prev = NULL;

  unique_client_id = -1;

  while (unique_client_id < 0)
  {
    /* Search qmi_sys_event_client_list and see if there are any matches to qmi_next_client_id */
    QMI_SLL_FIND (item,
                  prev,
                  qmi_sys_event_client_list,
                  (item->init_client_handle == qmi_next_client_id));


    /* If item is NULL, no items is list have handle that matches qmi_next_client_id...
    ** so we have a unique client ID.
    */
    if (!item)
    {
      unique_client_id = qmi_next_client_id;
    }
    else
    {
      item = prev = NULL;  /* Not really necessary, but good practice */
    }

    /* Increment the next qmi_next_client_id value.  This value must be positive
    ** value so ensure it
    */
    if  (++qmi_next_client_id < 0)
    {
      qmi_next_client_id = QMI_MIN_INIT_CLIENT_HANDLE_VALUE;
    }
  }

  return unique_client_id;
}




/*===========================================================================
  FUNCTION  qmi_sys_event_hdlr
===========================================================================*/
/*!
@brief
  Function that will receive the system event callbacks and broadcast
  to all registered clients

@return

@note
  - Side Effects
*/
/*=========================================================================*/
static void
qmi_sys_event_hdlr
(
        qmi_sys_event_type        event_id,
  const qmi_sys_event_info_type   *event_info,
        void                      *user_data
)
{
  qmi_sys_event_client_data_type        *sys_event_client_data = NULL;

  (void) user_data;

  QMI_PLATFORM_MUTEX_LOCK (&qmi_client_list_mutex);
  sys_event_client_data = qmi_sys_event_client_list;


  /* Traverse list of clients and report event to each of them.
 */
  while (sys_event_client_data)
  {
    if (sys_event_client_data->sys_event_rx_hdlr != NULL)
    {
      sys_event_client_data->sys_event_rx_hdlr (event_id,
                                                event_info,
                                                sys_event_client_data->sys_event_user_data);
    }

    sys_event_client_data = sys_event_client_data->next;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_client_list_mutex);
}


/*===========================================================================
  FUNCTION  qmi_init
===========================================================================*/
/*!
@brief
  Function to initialize the QMI system.  This must be called by before
  any other QMI API's at startup time.  This function registers a system event
  callback and user data which will be call when/if a system event occurs.

@return
  Negative error code value if error.  Otherwise a positive value, opaque handle will
  be returned.  This handle should be passed to the qmi_release() function when
  client is exiting and no longer wants to use QMI.  Note that all individual
  service handles should be released prior to doing qmi_release().

@note
  - Side Effects
    Initializes QMI QMUX subsystem
*/
/*=========================================================================*/
int
qmi_init
(
  qmi_sys_event_rx_hdlr   sys_event_rx_hdlr,
  void                    *sys_event_user_data
)
{
  int rc;
  qmi_sys_event_client_data_type *sys_event_client_data = NULL;

  QMI_PLATFORM_MUTEX_LOCK (&qmi_mutex);
  if (!qmi_initialized)
  {
    QMI_DEBUG_MSG_0 ("qmi_init:  Not initialized, calling process init sequence\n");
    if ((rc = qmi_service_pwr_up_init(qmi_sys_event_hdlr, NULL)) < 0)
    {
      QMI_DEBUG_MSG_1 ("qmi_init:  qmi_service_pwr_up_init failed, rc = %d\n",rc);
      QMI_PLATFORM_MUTEX_UNLOCK (&qmi_mutex);
      return rc;
    }
    qmi_initialized = TRUE;
  }
  else
  {
    QMI_DEBUG_MSG_0 ("qmi_init:  Already initialized, not calling process init sequence\n");
  }

  sys_event_client_data = (qmi_sys_event_client_data_type *) malloc (sizeof (qmi_sys_event_client_data_type));

  /* Make sure memory allocation succeeds */
  if (!sys_event_client_data)
  {
    QMI_DEBUG_MSG_0 ("qmi_init:  Malloc failed, returning error\n");
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_mutex);
    return QMI_INTERNAL_ERR;
  }

  QMI_PLATFORM_MUTEX_LOCK (&qmi_client_list_mutex);

  /* Fill in client data and add to list */
  sys_event_client_data->sys_event_rx_hdlr = sys_event_rx_hdlr;
  sys_event_client_data->sys_event_user_data = sys_event_user_data;
  sys_event_client_data->init_client_handle = qmi_get_next_init_client_handle();

  QMI_SLL_ADD (sys_event_client_data,qmi_sys_event_client_list);

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_client_list_mutex);

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_mutex);

  QMI_DEBUG_MSG_1 ("qmi_init:  Created client handle %x\n", sys_event_client_data);

  return (int) sys_event_client_data->init_client_handle;
}


/*===========================================================================
  FUNCTION  qmi_release
===========================================================================*/
/*!
@brief
  Function to initialize release/cleanup QMI library prior to exit.  Handle
  passed in is the handle returned from the qmi_init() function call.  Note that
  client should release all individual service handles prior to calling this
  function.

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    Cleans up client ID's and data structures IF this is the last client
    in the PD to release.
*/
/*=========================================================================*/
int
qmi_release
(
  int init_client_handle
)
{
  intptr_t tmp = init_client_handle;
  qmi_sys_event_client_data_type *item = NULL;
  qmi_sys_event_client_data_type *prev = NULL;
  int rc = QMI_NO_ERR;

  /* Lock mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_mutex);

  QMI_PLATFORM_MUTEX_LOCK (&qmi_client_list_mutex);

  /* Find and remove the client data */
  QMI_SLL_FIND_AND_REMOVE (item,
                           prev,
                           qmi_sys_event_client_list,
                           (item->init_client_handle == init_client_handle));

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_client_list_mutex);

  /* Make sure that the item to be removed was found */
  if (!item)
  {
    QMI_ERR_MSG_1 ("qmi_release: handle %x not found in list\n",init_client_handle);
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_mutex);
    return QMI_INTERNAL_ERR;
  }

  QMI_DEBUG_MSG_1 ("qmi_release: Released client handle %x\n",(int)init_client_handle);
  /* If there is no longer any client data in list, then this was the
  ** last qmi_release() call... do library cleanup
  */

  if (qmi_sys_event_client_list == NULL)
  {
    QMI_DEBUG_MSG_0 ("qmi_release: Last client releases, performing library de-init\n");
    rc = qmi_service_pwr_down_release();
    qmi_initialized = FALSE;
  }
  else
  {
    QMI_DEBUG_MSG_0 ("qmi_release: More clients in list, no de-init performed\n");
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_mutex);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_connection_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the connection for a particular
  port.  Once the connection is brought up, the CTL service will also
  be initialized prior to this function returning.

@return
  0 if function is successful and connection is brought up,
   negative value if not.

@note
  - Side Effects
    Opens connection
*/
/*=========================================================================*/
int qmi_dev_connection_init
(
  const char              *dev_id,
  int                     *qmi_err_code
)
{
  qmi_connection_id_type conn_id = QMI_CONN_ID_INVALID;
  int mux_id = -1;
  int ep_type = -1;
  int epid = -1;

  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_id, &ep_type, &epid, &mux_id)) == QMI_CONN_ID_INVALID)
  {
    QMI_ERR_MSG_1 ("qmi_dev_connection_init: failed to find device[%s]\n", dev_id);
    return QMI_INTERNAL_ERR;
  }

  /* Tell services layer to bring up connection */
  return qmi_service_connection_init (conn_id, qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_set_port_data_format
===========================================================================*/
/*!
@brief
  This function is called to set the data format of a particular port
  to the user specified values.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_set_port_data_format
(
  const char                            *dev_id,
  qmi_data_format_qos_hdr_state_type    qos_hdr_state,
  qmi_link_layer_protocol_type          *link_protocol,
  int                                   *qmi_err_code
)
{
  qmi_connection_id_type conn_id;
  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }

  return qmi_service_set_data_format (conn_id,
                                      qos_hdr_state,
                                      link_protocol,
                                      qmi_err_code);
}


/*===========================================================================
  FUNCTION  qmi_reg_pwr_save_mode
===========================================================================*/
/*!
@brief
  This function is used to register/de-register for power state change
  events.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_reg_pwr_save_mode
(
  qmi_pwr_report_type   report_state,
  int                   *qmi_err_code
)
{
  return qmi_service_reg_pwr_save_mode (report_state, qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_config_pwr_save_settings
===========================================================================*/
/*!
@brief
  Configures the power state indication filter for each connection.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_config_pwr_save_settings
(
  int                  pwr_state_hndl,
  qmi_service_id_type  service_id,
  int                  num_indication_ids,
  unsigned short       indication_ids[],
  int                  *qmi_err_code
)
{
  return qmi_service_config_pwr_save_settings (pwr_state_hndl,
                                               service_id,
                                               num_indication_ids,
                                               indication_ids,
                                               qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_set_pwr_state
===========================================================================*/
/*!
@brief
  Sets power state for each connection.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
    - Modem will not send filtered indications until later power state change.
*/
/*=========================================================================*/
int
qmi_set_pwr_state
(
  unsigned long        pwr_state,
  int                  *qmi_err_code
)
{
  return qmi_service_set_pwr_state (pwr_state, qmi_err_code);
}

/*===========================================================================
  FUNCTION  qmi_get_pwr_state
===========================================================================*/
/*!
@brief
  Gets power state for specified connection.


@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_get_pwr_state
(
  const char       *dev_id,
  unsigned long    *pwr_state,
  int              *qmi_err_code
)
{
  return qmi_service_get_pwr_state (dev_id, pwr_state, qmi_err_code);
}



