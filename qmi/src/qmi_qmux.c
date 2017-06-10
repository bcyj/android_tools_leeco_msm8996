/******************************************************************************
  @file    qmi_qmux.c
  @brief   The QMI QMUX layer

  DESCRIPTION
  QMUX layer for the QMI user space driver

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/******************************************************************************
******************************************************************************/

#include "qmi_platform_qmux_io.h"
#include "qmi_i.h"
#include "qmi_platform.h"
#include "qmi_service.h"
#include "qmi_qmux.h"
#include "qmi_qmux_if.h"
#include "qmi_util.h"

/*****************************************************************************
** Type declarations
*****************************************************************************/

#define QMI_CTL_SRVC_CLIENT_ID 0x00

#define MAX_QMI_CLIENT_IDS      50
#define QMI_RESULT_CODE_TYPE_ID 0x02
typedef enum
{
  QMI_CTL_SERVICE_REQUEST_MSG,
  QMI_CTL_SERVICE_RESPONSE_MSG,
  QMI_CTL_SERVICE_INDICATION_MSG,
  QMI_CTL_SERVICE_ERROR_MSG
} qmi_ctl_service_msg_type;

/* TODO:  Clean this up */
typedef unsigned char qmi_qmux_rx_buf_type[QMI_MAX_MSG_SIZE];

qmi_qmux_rx_buf_type *qmi_qmux_rx_buffers [QMI_MAX_CONNECTIONS];

/* Data kept for each client of each connection/service pair */
typedef struct qmi_qmux_client_srvc_type
{
  struct qmi_qmux_client_srvc_type *next;
  qmi_qmux_clnt_id_t               qmux_client_id;
  unsigned char qmi_client_ids [MAX_QMI_CLIENT_IDS];
} qmi_qmux_client_srvc_type;

/* Definitions for status flags */
#define QMI_QMUX_CONN_STATUS_IS_ACTIVE    0x00000001U
#define QMI_QMUX_CONN_STATUS_IN_RESET     0x00000002U
#define QMI_QMUX_CONN_STATUS_IS_DISABLED  0x00000004U

/* Data kept for each connection ID */
typedef struct
{
  QMI_PLATFORM_MUTEX_DATA_TYPE   list_mutex;
  QMI_PLATFORM_MUTEX_DATA_TYPE   tx_mutex;
  unsigned long                  status_flags;

  /* Each list in this array will be a list of qmi_qmux_client_srvc_type data type elements */
  qmi_qmux_client_srvc_type      *qmi_qmux_client_srvc_data [QMI_MAX_SERVICES];
} qmi_qmux_conn_info_type;

static int qmi_qcci_internal_public_service_id_to_bookkeeping_service_id(int public_service_id);
static int qmi_qcci_internal_bookkeeping_service_id_to_public_service_id(int bookkeeping_service_id);
static void qmi_ctl_delete_all_qmux_client_txns (qmi_qmux_clnt_id_t  qmux_client_id);

/*****************************************************************************
** A list of QMUX client ID's
*****************************************************************************/
typedef struct qmi_qmux_client_list_type
{
  struct qmi_qmux_client_list_type  *next;
  qmi_qmux_clnt_id_t                qmux_client_id;
  qmi_qmux_if_clnt_mode_t           qmux_client_mode;
} qmi_qmux_client_list_type;

static qmi_qmux_client_list_type *qmux_client_list = NULL;

static QMI_PLATFORM_MUTEX_DATA_TYPE qmux_client_list_mutex;

/*****************************************************************************
** Data declarations
*****************************************************************************/
/* A mutex for general purpose use */
static QMI_PLATFORM_MUTEX_DATA_TYPE qmux_general_purpose_mutex;

/* A mutex to stop from sending messages to clients concurrently.  Want to
** make sure we don't intersperse messages going to same qmux client.  This
** is a crude way to do it, but should be fine
*/
static QMI_PLATFORM_MUTEX_DATA_TYPE qmux_send_to_client_mutex;

static qmi_qmux_conn_info_type qmi_qmux_conn_info[QMI_MAX_CONN_IDS];


static unsigned char srvc_list[QMI_MAX_SERVICES+1] = { 0 };
static int           srvc_list_index = 1;
static int           num_services = 0;

/*****************************************************************************
** Forward declarations for CTL functions defined at bottom of file
*****************************************************************************/

#ifdef QMI_DEBUG

#define PRINT_QMI_MSG(msg,len) qmi_platform_log_raw_qmi_msg (msg,len)

#else

#define PRINT_QMI_MSG(msg,len)

#endif

/* Forward prototype declarations for some CTL functions */
static void
qmi_ctl_rx_msg
(
  qmi_connection_id_type  conn_id,
  unsigned char           control_flags,
  unsigned char           *msg,
  int                     msg_len
);

static int
qmi_ctl_handle_request
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned int              alloc_txn
);

void qmi_ctl_powerup_init(void);

static void
qmi_ctl_send_sync_msg
(
  qmi_connection_id_type conn_id
);


static void
qmi_ctl_delete_all_txns
(
  qmi_connection_id_type conn_id
);

static int
qmi_qmux_handle_raw_qmi_ctl_req
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  unsigned char             *msg,
  int                       msg_len
);

static int
qmi_qmux_handle_raw_qmi_ctl_resp
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  unsigned char             control_flags,
  unsigned char             *msg,
  int                       msg_len
);

/*===========================================================================
  FUNCTION count_clients
===========================================================================*/
/*!
@brief
  Function used for debugging... generally #ifdef'd out


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
#if 0
static void
count_clients (void)
{
  qmi_qmux_conn_info_type   *conn_info;
  qmi_qmux_client_srvc_type *client_srvc_info;


  qmi_connection_id_type   conn_id;
  qmi_service_id_type      service_id;
  int qmux_id_count = 0;
  int qmi_id_count = 0;
  int i;

  QMI_DEBUG_MSG_0 ("\n\nQMUX client info\n");
  for (conn_id = QMI_CONN_ID_FIRST; conn_id < QMI_MAX_CONNECTIONS; conn_id++)
  {

    for (service_id = QMI_FIRST_SERVICE; service_id < QMI_MAX_SERVICES; service_id++)
    {

      QMI_DEBUG_MSG_2 ("Connection %d, service %d:\n",conn_id, service_id);
      /* Set connection info pointer */
      conn_info = &qmi_qmux_conn_info[(int) conn_id];

      /* Get pointer to head list item for particular service */
      client_srvc_info = conn_info->qmi_qmux_client_srvc_data[service_id];

      /* Search through the list looking for a qmux_client_id that matches
      ** the one to be removed
      */
      if (client_srvc_info == NULL)
      {
        QMI_DEBUG_MSG_0 ("  None\n");
      }
      else
      {
      while (client_srvc_info != NULL)
      {
          QMI_DEBUG_MSG_1 ("   QMUX ID=%d CLNTS= ",client_srvc_info->qmux_client_id);
          for (i = 0; i < MAX_QMI_CLIENT_IDS; i++)
          {
            if (client_srvc_info->qmi_client_ids [i] == QMI_QMUX_INVALID_QMI_CLIENT_ID)
            {
              QMI_DEBUG_MSG_0 ("X ");
            }
            else
            {
              qmi_id_count++;
              QMI_DEBUG_MSG_1 ("%d ",client_srvc_info->qmi_client_ids [i]);
            }
          }
          qmux_id_count++;
          client_srvc_info = client_srvc_info->next;
        }
      }
    }
  }
  QMI_DEBUG_MSG_2  ("Total number of QMUX clients = %d, QMI clients = %d\n\n",qmux_id_count,qmi_id_count);
}
#endif



/*****************************************************************************
** Function definitions
*****************************************************************************/

static
void qmi_qmux_if_send_to_client (
  qmi_qmux_if_msg_id_type    msg_id,
  qmi_qmux_clnt_id_t         qmux_client_id,
  unsigned long              qmux_txn_id,
  qmi_connection_id_type     qmi_conn_id,
  qmi_service_id_type        qmi_service_id,
  qmi_client_id_type         qmi_client_id,
  unsigned char              control_flags,
  int                        sys_err_code,
  int                        qmi_err_code,
  unsigned char              *msg,
  int                        msg_len
)
{
  qmi_qmux_if_msg_hdr_type hdr;

  /* If QMUX client ID is invalid, just return.  We will
  ** hit this condition for "internally" (by qmux) generated
  ** messages
  */
  if (qmux_client_id == QMI_QMUX_INVALID_QMUX_CLIENT_ID)
  {
    return;
  }

  memset(&hdr, 0, sizeof(qmi_qmux_if_msg_hdr_type));

  /* Set hdr values */
  hdr.msg_id = msg_id;
  hdr.qmux_client_id = qmux_client_id;
  hdr.qmux_txn_id = qmux_txn_id;
  hdr.qmi_conn_id = qmi_conn_id;
  hdr.qmi_service_id = qmi_service_id;
  hdr.qmi_client_id = qmi_client_id;
  hdr.control_flags = control_flags;
  hdr.sys_err_code = sys_err_code;
  hdr.qmi_err_code = qmi_err_code;

  /* Decrement msg pointer and increment msg_len */
  msg -= QMI_QMUX_IF_HDR_SIZE;
  msg_len += (int)QMI_QMUX_IF_HDR_SIZE;

  /* Copy header into message buffer */
  memcpy ((void *)msg, (void *)&hdr, QMI_QMUX_IF_HDR_SIZE);

  QMI_PLATFORM_MUTEX_LOCK (&qmux_send_to_client_mutex);

#ifndef QMI_MSGLIB_MULTI_PD
  qmi_qmux_if_rx_msg (msg,msg_len);
#else
  QMI_QMUX_IF_PLATFORM_RX_MSG (qmux_client_id,msg,msg_len);
#endif

  QMI_PLATFORM_MUTEX_UNLOCK (&qmux_send_to_client_mutex);

}

/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_delete_all_srvc_clients (
  qmi_connection_id_type      conn_id,
  qmi_service_id_type         service_id,
  qmi_qmux_client_srvc_type   *client_srvc_info
)
{
  qmi_qmux_if_msg_hdr_type  msg_hdr;
  int i;
  unsigned char *client_id_ptr;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  /* Initialize message header fields for message to be passed to QMUX
  */
  msg_hdr.msg_id = QMI_QMUX_IF_RELEASE_QMI_CLIENT_ID_MSG_ID;
  msg_hdr.qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;
  msg_hdr.qmi_conn_id = conn_id;
  cmd_data.qmi_qmux_if_release_client_req.delete_service_id = service_id;

  /* Initialize client ID pointer */
  client_id_ptr = client_srvc_info->qmi_client_ids;


  /* Now cycle through all of the client ID's for this service and release
  ** all of them.
  */
  for (i = 0; i < MAX_QMI_CLIENT_IDS; i++)
  {
    if (*client_id_ptr != QMI_QMUX_INVALID_QMI_CLIENT_ID)
    {
      cmd_data.qmi_qmux_if_release_client_req.delete_client_id = *client_id_ptr;
      QMI_DEBUG_MSG_1 ("qmi_qmux_delete_all_srvc_clients: deleting qmi client %d\n", *client_id_ptr);

      /* Don't allocate CTL transactions for the client releases */
      if (qmi_ctl_handle_request (&msg_hdr, &cmd_data, FALSE) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_qmux_delete_all_srvc_clients: qmi_ctl_handle_request failed!!\n");
      }

      *client_id_ptr = QMI_QMUX_INVALID_QMI_CLIENT_ID;
    }
    client_id_ptr++;
  }
}


/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
void qmi_qmux_delete_qmux_client (
  qmi_qmux_clnt_id_t  qmux_client_id
)
{
  qmi_qmux_conn_info_type   *conn_info;
  qmi_qmux_client_srvc_type *client_srvc_info;
  qmi_qmux_client_srvc_type *prev;
  unsigned short             service_id;
  unsigned short             conn_id;

  qmi_qmux_client_list_type   *client_id,*prev_client_id;

  /* Lock the qmux client list mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmux_client_list_mutex);

  /* Find and remove the client */
  QMI_SLL_FIND_AND_REMOVE (client_id,
                           prev_client_id,
                           qmux_client_list,
                           (qmux_client_id == client_id->qmux_client_id));

  /* Unlock list mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmux_client_list_mutex);

  /* Send out error message if client wasn't found */
  if (client_id == NULL)
  {
    QMI_ERR_MSG_1 ("QMUX qmux_client_id=%x not found in qmux client list, unable to remove\n",qmux_client_id);
  }
  else
  {
    free (client_id);
    QMI_DEBUG_MSG_1 ("QMUX start qmux_client_id=%x being deleted from QMUX client ID list\n",qmux_client_id);
  }

  /* Delete all pending transactions for this QMUX client */
  qmi_ctl_delete_all_qmux_client_txns (qmux_client_id);

  /* Cycle through all connections and services and release all service clients
  ** of the QMUX client
  */
  for (conn_id = (unsigned short)QMI_CONN_ID_FIRST; conn_id < (unsigned short)QMI_MAX_CONN_IDS; conn_id++)
  {
    /* Set connection info pointer */
    conn_info = &qmi_qmux_conn_info[ conn_id ];

    /* Lock the list mutex */
    QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);

    for (service_id = (unsigned short)QMI_FIRST_SERVICE; service_id < (unsigned short)QMI_MAX_SERVICES; service_id++)
    {
      /* Get pointer to head list item for particular service */

      QMI_SLL_FIND_AND_REMOVE (client_srvc_info,
                               prev,
                               conn_info->qmi_qmux_client_srvc_data[ service_id ],
                               (client_srvc_info->qmux_client_id == qmux_client_id));

      if (client_srvc_info != NULL)
      {
        /* Release all clients associated with qmux_client_id */
        qmi_qmux_delete_all_srvc_clients ((qmi_connection_id_type)conn_id,
                                          qmi_qcci_internal_bookkeeping_service_id_to_public_service_id ( (qmi_service_id_type)service_id ) ,
                                          client_srvc_info);
        free (client_srvc_info);
        client_srvc_info = NULL;
      }
    } /* service ID for loop */

    /* Unlock list mutex */
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
  } /* connection ID for loop */

  QMI_DEBUG_MSG_1 ("QMUX finish qmux_client_id=%x being deleted from QMUX client ID list\n",qmux_client_id);
}

/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_add_qmux_client (
  qmi_qmux_clnt_id_t       qmux_client_id,
  qmi_qmux_if_clnt_mode_t  mode
)
{
  qmi_qmux_client_list_type   *client,*prev = NULL;
  (void)prev;

  /* Lock the list mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmux_client_list_mutex);

  /* Find out if client is already on the qmux_client_id list */
  /*lint --e{550} */
  QMI_SLL_FIND(client,prev,qmux_client_list,(qmux_client_id == client->qmux_client_id));
  if (client != NULL)
  {
    QMI_ERR_MSG_1 ("QMUX qmux_client_id=%x already qmux client list, unable to add\n",qmux_client_id);
  }
  /* Otherwise, add new client */
  else
  {
    client = (qmi_qmux_client_list_type *) malloc (sizeof (qmi_qmux_client_list_type));
    if(!client)
    {
      QMI_ERR_MSG_1 ("QMUX qmux_client_id=%x trying to add new client, malloc returned null\n",qmux_client_id);
    }
    else
    {
      client->qmux_client_id   = qmux_client_id;
      client->qmux_client_mode = mode;
      QMI_SLL_ADD (client, qmux_client_list);
      QMI_DEBUG_MSG_1 ("QMUX qmux_client_id=%x added to QMUX client ID list\n",qmux_client_id);
    }
  }

  /* Unlock list mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmux_client_list_mutex);
}


/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note
  conn_info->list_mutex must be locked by caller
  - Side Effects
    -

*/
/*=========================================================================*/
static
int qmi_qmux_add_qmi_service_client (
  qmi_qmux_clnt_id_t        qmux_client_id,
  qmi_connection_id_type    conn_id,
  qmi_service_id_type       service_id,
  qmi_client_id_type        client_id
)
{
  qmi_qmux_conn_info_type   *conn_info;
  qmi_qmux_client_srvc_type *client_srvc_info;
  int                       rc = QMI_NO_ERR;
  int                       i;

  /* Validate connection ID to make sure it is valid */
  if (((int)conn_id < (int)QMI_CONN_ID_FIRST) || ((int)conn_id >= (int)QMI_MAX_CONN_IDS))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_add_qmi_service_client: bad conn_id = %d\n",conn_id);
    return QMI_INTERNAL_ERR;
  }

  /* Validate service ID */
  if (( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) < QMI_FIRST_SERVICE) ||
      ( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) >= QMI_MAX_SERVICES))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_add_qmi_service_client: bad service id = %d\n",service_id);
    return QMI_INTERNAL_ERR;
  }

  /* Set connection info pointer */
  conn_info = &qmi_qmux_conn_info[(int) conn_id];

  /* Look to see if we already have client_srvc_info for this
  ** connection/qmux_client/service
  */

  /* Get pointer to head list item for particular service */
  client_srvc_info = conn_info->qmi_qmux_client_srvc_data[ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) ];

  /* Search through the list looking for a qmux_client_id that matches
  ** the one to be added
  */
  while (client_srvc_info != NULL)
  {
    if (client_srvc_info->qmux_client_id == qmux_client_id)
    {
      break;
    }
    client_srvc_info = client_srvc_info->next;
  }

  if (client_srvc_info) /* We already have a qmux client for this connection/service */
  {
    /* Find a free service qmi_client id spot and add the client ID */
    for (i=0; i < MAX_QMI_CLIENT_IDS; i++)
    {
      if (client_srvc_info->qmi_client_ids[i] == QMI_QMUX_INVALID_QMI_CLIENT_ID)
      {
        break;
      }
    }

    /* If there is a free slot for the qmi client ID, save it. */
    if (i < MAX_QMI_CLIENT_IDS)
    {
      QMI_DEBUG_MSG_3 ("Adding client %d, service %d, conn_id=%d...\n",client_id,service_id,conn_id);
      client_srvc_info->qmi_client_ids[i] = client_id;
    }
    else
    {
      rc = QMI_INTERNAL_ERR;
    }
  }
  else /* No client found for service */
  {
    client_srvc_info = (qmi_qmux_client_srvc_type *) malloc (sizeof (qmi_qmux_client_srvc_type));
    if (!client_srvc_info)
    {
      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      /* Initialize the fields of the structure */
      client_srvc_info->qmux_client_id = qmux_client_id;

      /* Initialize all client ID's in client ID arrary */
      for (i=1; i<MAX_QMI_CLIENT_IDS; i++)
      {
        client_srvc_info->qmi_client_ids[i] = QMI_QMUX_INVALID_QMI_CLIENT_ID;
      }

      /* Set client ID as first element of client ID array */
      client_srvc_info->qmi_client_ids[0] = client_id;

      /* Add QMUX client to list */
      QMI_SLL_ADD (client_srvc_info,conn_info->qmi_qmux_client_srvc_data[ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id (service_id ) ]);
    }
  }

  return rc;
}




/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note
  conn_info->list_mutex must be locked by caller
  - Side Effects
    -

*/
/*=========================================================================*/
static
int qmi_qmux_remove_qmi_service_client (
  qmi_qmux_clnt_id_t       qmux_client_id,
  qmi_connection_id_type   conn_id,
  qmi_service_id_type      service_id,
  qmi_client_id_type       client_id
)
{
  qmi_qmux_conn_info_type   *conn_info;
  qmi_qmux_client_srvc_type *client_srvc_info;
  qmi_qmux_client_srvc_type *prev;
  int                       rc;
  int                       qmux_client_should_be_deleted;

  /* Validate connection ID to make sure it is valid */
  if (((int)conn_id < (int)QMI_CONN_ID_FIRST) || ((int)conn_id >= (int)QMI_MAX_CONN_IDS))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_remove_qmi_service_client: bad conn_id = %d\n",conn_id);
    return QMI_INTERNAL_ERR;
  }

  /* Validate service ID */
  if (( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) < QMI_FIRST_SERVICE) ||
      ( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id  ( service_id ) >= QMI_MAX_SERVICES))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_remove_qmi_service_client: bad service id = %d\n",service_id);
    return QMI_INTERNAL_ERR;
  }

  /* Set connection info pointer */
  conn_info = &qmi_qmux_conn_info[(int) conn_id];

  QMI_SLL_FIND(client_srvc_info,
               prev,
               conn_info->qmi_qmux_client_srvc_data[ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) ],
               (client_srvc_info->qmux_client_id == qmux_client_id));

   /* We have found the client service info for this connection/service, process deletion */
  if (client_srvc_info)
  {

    /* Use a pointer... faster than array accesses */
    qmi_client_id_type *client_id_ptr;
    int                 i;

    qmux_client_should_be_deleted = TRUE;
    client_id_ptr = client_srvc_info->qmi_client_ids;

    /* Assume the worst */
    rc = QMI_INTERNAL_ERR;

    /* Run through the qmux client array of qmi_client ID's.  If I
    ** find the one to be deleted, set it's value to INVALID.  Also
    ** run through all of them to make sure there are still some valid
    ** clients left.  If not, delete the qmux client
    */
    for (i=0; i < MAX_QMI_CLIENT_IDS; i++)
    {
      /* Found client ID, make invalid and set rc */
      if (*client_id_ptr == client_id)
      {
        rc = QMI_NO_ERR;
        *client_id_ptr = QMI_QMUX_INVALID_QMI_CLIENT_ID;
      }
      else if (*client_id_ptr != QMI_QMUX_INVALID_QMI_CLIENT_ID)
      {
        qmux_client_should_be_deleted = FALSE;
      }
      client_id_ptr++;
    }

    /* If qmux client structure needs to be deleted, remove from the
    ** list and free the memory
    */
    if (qmux_client_should_be_deleted)
    {
      /*lint -e{774} */
      QMI_SLL_REMOVE(client_srvc_info,prev,conn_info->qmi_qmux_client_srvc_data[ qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id ) ]);

      /* Free the memory */
      free (client_srvc_info);
    }
  }
  else
  {
    QMI_ERR_MSG_3 ("qmi_qmux_remove_qmi_service_client: can't find qmux_client %d, service %d, client%d\n",qmux_client_id,service_id,client_id);
    rc = QMI_INTERNAL_ERR;
  }

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_add_remove_qmux_if_client
===========================================================================*/
/*!
@brief
  Adds or removes the given client from the qmux_client_list

@return
  QMI_NO_ERR on success
  QMI_INTERNAL_ERR otherwise

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
static
int qmi_qmux_add_delete_qmux_if_client
(
  qmi_qmux_clnt_id_t       qmux_client_id,
  qmi_qmux_if_msg_hdr_type *msg_hdr,
  qmi_qmux_if_cmd_rsp_type *cmd_data
)
{
  unsigned char tx_buf [QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];

  /* Validate the input */
  if (qmux_client_id != cmd_data->qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_add_delete_qmux_if_client: qmux_client_id=0x%x != cmd_data->qmux_client_id=0x%x\n",
                   qmux_client_id,
                   cmd_data->qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id);
    return QMI_INTERNAL_ERR;
  }

  if (msg_hdr->msg_id == QMI_QMUX_IF_ADD_QMUX_CLIENT_MSG_ID)
  {
    qmi_qmux_add_qmux_client (cmd_data->qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id,
                              cmd_data->qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_mode);
  }
  else if (msg_hdr->msg_id == QMI_QMUX_IF_DELETE_QMUX_CLIENT_MSG_ID)
  {
    qmi_qmux_delete_qmux_client (cmd_data->qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id);
  }

  /* Send response back to client */
  qmi_qmux_if_send_to_client (msg_hdr->msg_id,
                              msg_hdr->qmux_client_id,
                              msg_hdr->qmux_txn_id,
                              msg_hdr->qmi_conn_id,
                              msg_hdr->qmi_service_id,
                              msg_hdr->qmi_client_id,
                              0,
                              QMI_NO_ERR,
                              QMI_SERVICE_ERR_NONE,
                              (unsigned char *)(tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
                              sizeof (qmi_qmux_if_cmd_rsp_type));

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_qmux_broadcast_sys_ind_to_all_clients
===========================================================================*/
/*!
@brief
  Utility function to send qmi_qmux_if_cmd_rsp_type and mssage ID to all
  qmux client ID's


@return
  None.

@note

  - Side Effects
    - Broad

*/
/*=========================================================================*/
static void
qmi_qmux_broadcast_sys_ind_to_all_clients
(
  qmi_connection_id_type    conn_id,
  qmi_qmux_if_msg_id_type   msg_id,
  qmi_qmux_if_cmd_rsp_type  *cmd_data
)
{
  qmi_qmux_client_list_type  *client = NULL;
  qmi_qmux_client_list_type  *next_client = NULL;
  unsigned char              tx_buf [QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];

  memset ((void *)tx_buf, 0x0, sizeof (tx_buf));

  /* Set up tx_msg buffer by copying command data to it */
  if (cmd_data)
  {
    memcpy ((unsigned char *)(tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
             cmd_data,
             sizeof (qmi_qmux_if_cmd_rsp_type));
  }

  /* Lock the list mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmux_client_list_mutex);

  /* Cycle through all of the QMUX clients and send the indication.
  **
  ** Note that due to the fact that in some implementations, it is possible
  ** that calling the qmi_qmux_if_send_to_client() function can result in the
  ** qmux_client_id client being deleted from the same thread context, we have
  ** made the qmux_client_list_mutex recursive, and we will not access the
  ** client data structure at all after the qmi_qmux_if_send_to_client() call.
  ** ONLY the current client can be deleted in this scenario and since the mutex
  ** is locked, no other clients can be deleted, so it is safe to continue to use
  ** the next pointer subsequent to the call, just not the current client pointer.
  */
  client = qmux_client_list;
  while ( client != NULL )
  {
    next_client = client->next; /* Save next pointer first */

    /* Send result to client */
    QMI_ERR_MSG_3 ("qmi_qmux:  Sending sys indication=%d to qmux_client_id=0x%x on conn_id=%d",
                   (int)msg_id,
                   (int)client->qmux_client_id,
                   (int)conn_id);

    qmi_qmux_if_send_to_client ( msg_id,
                                 client->qmux_client_id,
                                 QMI_INVALID_TXN_ID,
                                 QMI_CONN_ID_INVALID,
                                 (qmi_service_id_type)0, /* Not used */
                                 0, /* Not used */
                                 0, // control flags not used
                                 QMI_NO_ERR,
                                 QMI_SERVICE_ERR_NONE,
                                 (unsigned char *)(tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
                                 sizeof (qmi_qmux_if_cmd_rsp_type));

    /* In some implementations, client pointer may have been deleted by qmi_qmux_if_send_to_client(),
    ** so don't dereference below that call.  Set client pointer to previously saved next pointer
    ** value.
    */
    client = next_client;
  }
  /* Unlock the list mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmux_client_list_mutex);
}

/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_rx_client_broadcast (
  qmi_connection_id_type    conn_id,
  qmi_service_id_type       service_id,
  qmi_client_id_type        client_id,
  unsigned char             control_flags,
  unsigned char             *msg_ptr,
  int                       msg_len
)
{
  qmi_qmux_conn_info_type *conn_info;
  qmi_qmux_client_srvc_type *client_srvc_info;
  qmi_service_id_type  book_keep_srvc_id;

  book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id );

  if (book_keep_srvc_id >= QMI_MAX_SERVICES)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_rx_client_broadcast: Invalid book_keep_srvc_id=%d service_id=%d",
                   book_keep_srvc_id,
                   service_id);
    return;
  }

  conn_info = &qmi_qmux_conn_info[(int) conn_id];

  /* Lock the mutex associated with the connection service list */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);

  /* Get pointer to head list item for particular service */
  client_srvc_info = conn_info->qmi_qmux_client_srvc_data[ book_keep_srvc_id ];

  /* Search through the list looking for a qmux_client_id that matches
  ** the one to be added
  */
  while (client_srvc_info != NULL)
  {
    qmi_qmux_if_send_to_client (QMI_QMUX_IF_QMI_MSG_ID,
                                client_srvc_info->qmux_client_id,
                                QMI_INVALID_TXN_ID,
                                conn_id,
                                service_id,
                                client_id,
                                control_flags,
                                QMI_NO_ERR,
                                QMI_SERVICE_ERR_NONE,
                                msg_ptr,
                                msg_len);

    /* Go to next client */
    client_srvc_info = client_srvc_info->next;
  }

  /* Unlock the mutex associated with the connection service list */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
}



/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
qmi_qmux_client_srvc_type *qmi_qmux_find_srvc_client (
  qmi_qmux_conn_info_type   *conn_info,
  qmi_service_id_type       service_id,
  unsigned char             qmi_client_id
)
{
  int i;
  unsigned int clnt_found = FALSE;
  qmi_qmux_client_srvc_type *client_srvc_info = NULL;
  qmi_service_id_type  book_keep_srvc_id;

  book_keep_srvc_id = qmi_qcci_internal_public_service_id_to_bookkeeping_service_id ( service_id );

  if (book_keep_srvc_id >= QMI_MAX_SERVICES)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_find_srvc_client: Invalid book_keep_srvc_id=%d service_id=%d",
                   book_keep_srvc_id,
                   service_id);
    goto bail;
  }


  /* Get pointer to head list item for particular service */
  client_srvc_info = conn_info->qmi_qmux_client_srvc_data[ book_keep_srvc_id ];

  /* Search through the list looking for a qmux_client_id that matches
  ** the one to be added
  */
  while (client_srvc_info != NULL)
  {
    unsigned char *clnt_id_ptr = client_srvc_info->qmi_client_ids;
    for (i=0; i<MAX_QMI_CLIENT_IDS; i++)
    {
      if (*clnt_id_ptr++ == qmi_client_id)
      {
        clnt_found = TRUE;
        break;
      }
    }

    /* If we found the client ID, break out of loop, otherwise
    ** go to next on list
    */
    if (clnt_found)
    {
      break;
    }
    else
    {
      client_srvc_info = client_srvc_info->next;
    }
  }

bail:
  return client_srvc_info;
}


/*===========================================================================
  FUNCTION
===========================================================================*/
/*!
@brief


@return

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_rx_client_msg (
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  unsigned char           control_flags,
  unsigned char           *msg_ptr,
  int                     msg_len
)
{
  qmi_qmux_conn_info_type    *conn_info;
  qmi_qmux_client_srvc_type  *client_srvc_info;

  /* Set connection info pointer */
  conn_info = &qmi_qmux_conn_info[(int) conn_id];

  /* Lock the mutex associated with the connection service list */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);

  if ((int)service_id == QMI_CTL_SERVICE)
  {
    qmi_ctl_rx_msg (conn_id, control_flags, msg_ptr, msg_len);
  }
  else
  {
    client_srvc_info = qmi_qmux_find_srvc_client (conn_info, service_id, client_id);

    /* Sent the client message */
    if (client_srvc_info)
    {
      qmi_qmux_if_send_to_client (QMI_QMUX_IF_QMI_MSG_ID,
                                   client_srvc_info->qmux_client_id,
                                   QMI_INVALID_TXN_ID,
                                   conn_id,
                                   service_id,
                                   client_id,
                                   control_flags,
                                   QMI_NO_ERR,
                                   QMI_SERVICE_ERR_NONE,
                                   msg_ptr,
                                   msg_len);
    }
    else
    {
      QMI_ERR_MSG_3 ("qmuxd: Unable to find qmux_client_id info for conn_id=%d, service_id=%d, client_id=%x",
                                                        conn_info,service_id,client_id);
    }
  }

  /* Unlock the mutex associated with the connection service list */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
}





/*===========================================================================
  FUNCTION  qmi_qmux_rx_msg
===========================================================================*/
/*!
@brief
  Routine for handling QMI messages received from SMD QMI control port.
  This function will be called by a thread that is spawned for each
  QMI connection to handle receive messages.

@return
  NULL... but this function should never return

@note

  Connection is assumed to be opened and valid data

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_rx_msg (
  qmi_connection_id_type  conn_id,
  unsigned char           *rx_buf_ptr,
  int                     rx_buf_len
)
{
  /* Pointer to QMUX connection info */
  qmi_qmux_conn_info_type *conn_info;
  int                     i_f_byte;
  unsigned short          length;
  qmi_client_id_type      client_id;
  unsigned char           control_flags;
  qmi_service_id_type     service_id;
  unsigned char           temp_char;
  int                     rem_bytes;
  int                     msg_len = 0;
  unsigned char           *msg_ptr = rx_buf_ptr;
  unsigned char           *msg_start_ptr = NULL;

  /* Validate connection ID to make sure it is valid */
  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_1 ("qmi_qmux: RX on bad conn_id = %d\n",conn_id);
    return;
  }

  conn_info = &qmi_qmux_conn_info[(int) conn_id];

  /* Make sure connection is up and running */
  if ( !(conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE) )
  {
    QMI_ERR_MSG_2 ("qmi_qmux: Got %d bytes of RX data for conn_id=%d, while connection not active... ignoring\n",
                    rx_buf_len,conn_id);
    return;
  }

  QMI_DEBUG_MSG_2 ("qmi_qmux: TX/RX - RX %d bytes on conn_id=%d\n",rx_buf_len,(int)conn_id);

  /* If there are multiple QMI messages, extract one at a time and deliver to the upper layer */
  for (rem_bytes = rx_buf_len; rem_bytes > 0; rem_bytes -= (msg_len + QMI_QMUX_HDR_SIZE))
  {/*lint --e{774} */

    /* Control Channel Mesage
    ------------------------------------------------------------------------------
    type:       I/F type | Length| Control Flags| Service Type| Client ID| QMUX SDU|
    ------------------------------------------------------------------------------
                         | QMUX Header                                   |
    ------------------------------------------------------------------------------
    size(byte): 1        |   2   |      1       |      1      |    1     |
    ------------------------------------------------------------------------------
     */

    /* Store pointer to the start of the message */
    msg_start_ptr = msg_ptr;

    /* Read the I/F byte, make sure it is a 1 */
    READ_8_BIT_VAL(msg_ptr,i_f_byte);

    if (i_f_byte != 1)
    {
      QMI_ERR_MSG_1 ("qmi_qmux: Received invalid I/F byte = %d\n",i_f_byte);
      return;
    }

    /* Read the message length */
    READ_16_BIT_VAL(msg_ptr, length);

    /* verify length is at least (QMI_QMUX_HDR_SIZE -1) long,
     per spec, Length of the QMUX message; includes the QMUX header
     itself, but not the preamble I/F type */
    if(length < ( QMI_QMUX_HDR_SIZE -1))
    {
      QMI_ERR_MSG_1(" qmi_qmux: Received invalid length(%d)", length);
      return;
    }

    /* Verify that the length of the packet is same as length field
    ** minus 1 (I/F field isn't part of length)
    */
    if (rem_bytes < (int) length + 1)
    {
      /* Houston, we have a problem... */
      QMI_ERR_MSG_2 ("qmi_qmux: packet rem_bytes < length (%d) + 1\n", rem_bytes, (int)length);
      return;
    }

    PRINT_QMI_MSG (msg_start_ptr, length+1);

    /* Read the control flags */
    READ_8_BIT_VAL(msg_ptr,control_flags);

    /* Read the service_type and client_id */
    READ_8_BIT_VAL(msg_ptr,temp_char);
    service_id = (qmi_service_id_type) temp_char;
    READ_8_BIT_VAL(msg_ptr,temp_char);
    client_id = (qmi_client_id_type) temp_char;

    /* Message is for QMI Proxy */
    if ( QMI_CONN_ID_IS_PROXY( conn_id ) )
    {
      QMI_DEBUG_MSG_3("QMUX header I/F(%d), length(%d) control flag(%d)",
                       i_f_byte,
                       length,
                       control_flags);
      QMI_DEBUG_MSG_2("service type(%d) client id(%d)\n",
                       (int)service_id,
                       (int)client_id);
    }

    /* Validate service ID */
    /*lint --e{774} */
    if ((( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id  ( service_id ) < (int)QMI_FIRST_SERVICE) ||
         ( qmi_qcci_internal_public_service_id_to_bookkeeping_service_id  ( service_id ) >= (int)QMI_MAX_SERVICES)) &&
        ((int)service_id != QMI_CTL_SERVICE))
    {
      QMI_ERR_MSG_1 ("qmi_qmux_rx_msg: bad service id = %d\n",service_id);
      return;
    }

    /* Calculate length of the actual message
    */
    msg_len = length - QMI_QMUX_HDR_SIZE + 1;

    /* Now, determine if the message is a broadcast message meant for all clients.  This
    ** is determined by the client ID == 0xFF
    */
    /*lint -e{774} */
    if (client_id == QMI_QMUX_INVALID_QMI_CLIENT_ID)
    {
      qmi_qmux_rx_client_broadcast (conn_id,
                                    service_id,
                                    client_id,
                                    control_flags,
                                    msg_ptr,
                                    msg_len);

    }
    else
    {
      qmi_qmux_rx_client_msg (conn_id,
                              service_id,
                              client_id,
                              control_flags,
                              msg_ptr,
                              msg_len);
    }

    /* Go to the beginning of the next QMI message (if any) */
    msg_ptr += msg_len;
  }
} /* qmi_qmux_rx_msg */


/*===========================================================================
  FUNCTION  qmi_qmux_tx_msg
===========================================================================*/
/*!
@brief
  Function to send a QMUX PDU via the SMD control port.  This function
  will add on the appropriate QMUX header to the PDU.  It is assumed
  that space has been pre-allocated in the PDU for the header.

@return
  0 if function is successful, negative value if not.

@note

  - Connection is assumed to be opened with valid data before this
  function starts to execute

  - Side Effects
    -

*/
/*=========================================================================*/
static
int qmi_qmux_tx_to_modem
(
  qmi_connection_id_type   conn_id,
  qmi_service_id_type      service_id,
  qmi_client_id_type       client_id,
  unsigned char            *msg_ptr,
  int                      msg_len
)
{
  qmi_qmux_conn_info_type   *conn_info;
  unsigned char             *tmp_msg_ptr;
  int                       rc = 0;

  /* Verifiy the conn_id parameter since it will be used to index
  ** into an array
  */
  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    return QMI_INTERNAL_ERR;
  }

  msg_ptr -= QMI_QMUX_HDR_SIZE;
  msg_len += QMI_QMUX_HDR_SIZE;

  /* Save a temporary copy of the msg_ptr.  This will be used and changed
  ** as we add QMUX header values to the message
  */
  tmp_msg_ptr = msg_ptr;


  /* Set the QMUX message header fields.  Note that the message comes into
  ** this function with the pointer set to the area where the QMUX header
  ** should start.
  */

  /* I/F type is 1 for a QMUX message */
  WRITE_8_BIT_VAL (tmp_msg_ptr, 1);

  /* Length is length of message to send which includes the QMUX header, but since
  ** QMI_QMUX_HDR_SIZE includes the I/F byte and the length field in a QMUX
  ** message doesn't include this, we need to subtract 1
  */
  WRITE_16_BIT_VAL (tmp_msg_ptr, (msg_len - 1));

  /* Control flags byte should be set to 0 for control point */
  WRITE_8_BIT_VAL (tmp_msg_ptr, 0);

  /* Now put in service type and client ID */
  WRITE_8_BIT_VAL (tmp_msg_ptr, service_id);
  WRITE_8_BIT_VAL (tmp_msg_ptr, client_id);

  /* Message is for QMI Proxy */
  if ( QMI_CONN_ID_IS_PROXY( conn_id ) )
  {
    QMI_DEBUG_MSG_3("QMUX header I/F(%d), length(%d) control flag(%d)",
                     1,
                     (msg_len-1),
                     0);
    QMI_DEBUG_MSG_2("service type(%d) client id(%d)\n",
                     (int)service_id,
                     (int)client_id);
  }

  /* Get pointer to connection info */
  conn_info = &qmi_qmux_conn_info[conn_id];

  /* Lock the connection TX mutex prior to sending packet to avoid any inter-mingling
  ** of packets from different threads
  */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->tx_mutex);

  /* Make sure connection is active and not in reset */
  if  ((conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE) &&
       (!(conn_info->status_flags & QMI_QMUX_CONN_STATUS_IN_RESET)))
  {
    QMI_DEBUG_MSG_2 ("qmi_qmux: TX/RX - TX %d bytes on conn_id=%d\n", (int) msg_len,(int)conn_id);
    PRINT_QMI_MSG (msg_ptr, (int)msg_len);
    rc = QMI_QMUX_IO_PLATFORM_SEND_QMI_MSG (conn_id, msg_ptr, (int) msg_len);

    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_2 ("qmi_qmux: TX failed in platform code, conn_id=%d, rc=%d\n", (int)conn_id, rc);
    }
  }
  else
  {
    QMI_ERR_MSG_2 ("qmi_qmux: TX failed, connection inactive or in reset, "
                   "conn_id=%d, status_flags=%x\n",
                   (int)conn_id,
                   (int)conn_info->status_flags);
    rc = QMI_PORT_NOT_OPEN_ERR;
  }

  /* Unlock the connection TX mutex after sending packet */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->tx_mutex);

  return rc;
}


int qmi_qmux_tx_msg  (
  qmi_qmux_clnt_id_t  qmux_client_id,
  unsigned char       *msg,
  int                 msg_len
)
{
  qmi_qmux_if_msg_hdr_type  msg_hdr;
  int rc = QMI_NO_ERR;
  qmi_qmux_conn_info_type *conn_info;

  if (qmux_client_id < 0)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_tx_msg: Bad input qmux_client_id=%d\n", qmux_client_id);
    return QMI_INTERNAL_ERR;
  }
  /* Make sure we have at least a valid message header */
  else if (msg_len < (int)QMI_QMUX_IF_HDR_SIZE)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_tx_msg: Bad input message size = %d\n",msg_len);
    return QMI_INTERNAL_ERR;
  }

  /* We copy the message header into a structure because of possible  alignment
  ** issues.  If platform layer is not doing word alignment, then this could cause
  ** problems.  From this point on, packet is treated as byte array so no issues
  */
  memcpy(&msg_hdr,msg,QMI_QMUX_IF_HDR_SIZE);

  /* Advance msg pointer and decrement msg_len */
  msg += QMI_QMUX_IF_HDR_SIZE;
  msg_len -= (int)QMI_QMUX_IF_HDR_SIZE;

  /* Verify connection ID */
  if (!QMI_CONN_ID_IS_VALID(msg_hdr.qmi_conn_id))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_tx_msg: bad conn_id=%d\n",msg_hdr.qmi_conn_id);
    return QMI_INTERNAL_ERR;
  }

  /* port open retry if necessary */
  conn_info = &qmi_qmux_conn_info[msg_hdr.qmi_conn_id];
  if (!(conn_info->status_flags & QMI_QMUX_CONN_STATUS_IN_RESET) &&
      !(conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE))
  {
    if (QMI_NO_ERR == qmi_qmux_open_connection( msg_hdr.qmi_conn_id,
                                                QMI_QMUX_OPEN_MODE_NORMAL))
    {
      QMI_DEBUG_MSG_1 ("qmi_qmux_tx_msg: successfully opened inactive connd_id=%d\n",
                       msg_hdr.qmi_conn_id);
    }
    else
    {
      QMI_ERR_MSG_1 ("qmi_qmux_tx_msg: failed to open inactive connd_id=%d\n",
                     msg_hdr.qmi_conn_id);
    }
  }

  /* If the message is a QMI message, send it on to the modem */
  if (msg_hdr.msg_id == QMI_QMUX_IF_QMI_MSG_ID)
  {
    qmi_qmux_client_srvc_type *client_srvc_info = qmi_qmux_find_srvc_client(conn_info,
                                                                            msg_hdr.qmi_service_id,
                                                                            msg_hdr.qmi_client_id);

    /* If the QMI service client ID is not allocated to this particular QMUX client then bail */
    if (!client_srvc_info || client_srvc_info->qmux_client_id != qmux_client_id)
    {
      QMI_ERR_MSG_4 ("qmi_qmux_tx_msg: service_id=%d, qmi_client_id=%d is not allocated to "
                     "qmux_client_id=%d on conn_id=%d\n",
                     msg_hdr.qmi_service_id,
                     msg_hdr.qmi_client_id,
                     qmux_client_id,
                     msg_hdr.qmi_conn_id);

      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      rc = qmi_qmux_tx_to_modem( msg_hdr.qmi_conn_id,
                                 msg_hdr.qmi_service_id,
                                 msg_hdr.qmi_client_id,
                                 msg,
                                 msg_len );
    }
  }
  else if (QMI_QMUX_IF_SEND_RAW_QMI_CTL_MSG_ID == msg_hdr.msg_id)
  {
    rc = qmi_qmux_handle_raw_qmi_ctl_req(&msg_hdr,
                                         msg,
                                         msg_len);
  }
  else
  {
    /* Make sure message ID is valid */
    if (msg_hdr.msg_id >= QMI_QMUX_IF_MAX_NUM_MSGS)
    {
      QMI_ERR_MSG_1 ("qmi_qmux_tx_msg: Unknown message ID = %d\n",msg_hdr.msg_id);
      rc = QMI_INTERNAL_ERR;
    }
    /* Make sure we got cmd_data back */
    else if (msg_len < (int) sizeof (qmi_qmux_if_cmd_rsp_type))
    {
      QMI_ERR_MSG_1 ("qmi_qmux_tx_msg: cmd_data too short size = %d\n", msg_len);
      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      qmi_qmux_if_cmd_rsp_type  cmd_data;
      memcpy (&cmd_data, msg, sizeof (qmi_qmux_if_cmd_rsp_type));

      /* Handle adding/deleting of QMUX clients */
      if ((msg_hdr.msg_id == QMI_QMUX_IF_ADD_QMUX_CLIENT_MSG_ID) ||
          (msg_hdr.msg_id == QMI_QMUX_IF_DELETE_QMUX_CLIENT_MSG_ID))
      {
        rc = qmi_qmux_add_delete_qmux_if_client (qmux_client_id,&msg_hdr,&cmd_data);
      }

      /* Everything else is a CTL message */
      else
      {
        rc = qmi_ctl_handle_request( &msg_hdr, &cmd_data, TRUE );
      }
    }
  }

bail:
  if (rc != QMI_NO_ERR)
  {
    /* respond with error this error only for CTL_SVC messages errors for all other
       services are handled by the QMI_QMUX_IF_PORT_WRITE_FAIL_IND_MSG_ID sys ind  */
    if( QMI_CTL_SERVICE == msg_hdr.qmi_service_id )
    {
      QMI_DEBUG_MSG_0("qmi_qmux_tx_msg failed....replying back to client\n");
      unsigned char tx_buf [QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];
      memset (tx_buf, 0x0, (QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)));
      qmi_qmux_if_send_to_client (msg_hdr.msg_id,
                                  msg_hdr.qmux_client_id,
                                  msg_hdr.qmux_txn_id,
                                  msg_hdr.qmi_conn_id,
                                  msg_hdr.qmi_service_id,
                                  msg_hdr.qmi_client_id,
                                  0, // control flags not used
                                  rc,
                                  QMI_SERVICE_ERR_NONE,
                                  (unsigned char *)(tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
                                  sizeof (qmi_qmux_if_cmd_rsp_type));
    }
  }

  return rc;
}



static
int qmi_qmux_process_rx_ctl_sync_msg
(
  qmi_connection_id_type     conn_id,
  qmi_qmux_if_cmd_rsp_type   *cmd_data
)
{
  qmi_qmux_conn_info_type *conn_info;

  int send_modem_in_service_msg = FALSE;
  int rc = QMI_NO_ERR;

  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_1 ("qmi_qmux: CTL indication received invalid conn_id=%d\n",(int)conn_id);
    return QMI_INTERNAL_ERR;
  }

  conn_info = &qmi_qmux_conn_info[conn_id];


  QMI_PLATFORM_MUTEX_LOCK (&qmux_general_purpose_mutex);

  if (conn_info->status_flags & QMI_QMUX_CONN_STATUS_IN_RESET)
  {
      conn_info->status_flags &= ~QMI_QMUX_CONN_STATUS_IN_RESET;
      send_modem_in_service_msg = TRUE;
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmux_general_purpose_mutex);

  if (send_modem_in_service_msg == TRUE)
  {
    QMI_ERR_MSG_1 ("qmi_qmux: Modem is OUT of RESET for conn_id=%d!\n",conn_id);

    /* Generate SYNC request back to Modem per QMI CTL spec Appendix B */
    qmi_ctl_send_sync_msg( conn_id );

    cmd_data->qmi_qmux_if_sub_sys_restart_ind.conn_id = conn_id;
    qmi_qmux_broadcast_sys_ind_to_all_clients (conn_id, QMI_QMUX_IF_MODEM_IN_SERVICE_MSG_ID, cmd_data);

    /* We set return code to error in case of reset because we don't want SYNC indications
    ** sent to clients in this case, and returning an error code will stop calling function
    ** from sending SYNC indications
    */
    rc = QMI_INTERNAL_ERR;
  }
  else
  {
    cmd_data->qmi_qmux_if_sync_ind.conn_id = conn_id;
  }

  return rc;
}


/*===========================================================================
  FUNCTION qmi_qmux_forget_all_qmi_service_clients
===========================================================================*/
/*!
@brief
  This function will remove all mappings between QMUX client ID and the
  QMI client ID/QMI service ID pairs.  This should only be called in case
  of modem reset.


@return

@note

  - All QMUX client ID <-> QMI client/service ID mappings will be lost,
  and no QMI messages will work until clients are re-initialized.

*/
/*=========================================================================*/
static void
qmi_qmux_forget_all_qmi_service_clients
(
  qmi_connection_id_type conn_id
)
{
  qmi_qmux_conn_info_type   *conn_info;
  qmi_qmux_client_srvc_type *client_srvc_info;
  qmi_service_id_type      service_id;
  int i;

  QMI_DEBUG_MSG_1 ("Called qmi_qmux_forget_all_qmi_service_clients on conn_id=%d\n",conn_id);

  conn_info = &qmi_qmux_conn_info[(int) conn_id];

  for (service_id = QMI_FIRST_SERVICE; service_id < QMI_MAX_SERVICES; service_id++)
  {

    QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);
    client_srvc_info = conn_info->qmi_qmux_client_srvc_data[service_id];
    while (client_srvc_info != NULL)
    {
      qmi_qmux_client_srvc_type *tmp = client_srvc_info->next;
      client_srvc_info->next = NULL;
      free (client_srvc_info);
      client_srvc_info = tmp;
    }
    conn_info->qmi_qmux_client_srvc_data[service_id] = NULL;
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
  }
  QMI_DEBUG_MSG_0 ("Finished qmi_qmux_forget_all_qmi_service_clients!\n");
}



static
void qmi_qmux_event_cb
(
  qmi_connection_id_type                conn_id,
  qmi_qmux_io_platform_event_type       event,
  qmi_qmux_io_platform_event_info_type  *event_info
)
{
  qmi_qmux_conn_info_type *conn_info;

  (void) event_info;

  QMI_DEBUG_MSG_2 ("qmi_qmux: I/O Platform EVENT callback: event=%d, conn_id=%d\n",(int)event,(int)conn_id);

  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_1 ("qmi_qmux: I/O Platform EVENT callback: invalid conn_id=%d\n",(int)conn_id);
    return;
  }

  conn_info = &qmi_qmux_conn_info[conn_id];


  switch (event)
  {
    case QMI_QMUX_IO_PORT_READ_ERR_MODEM_RESET_EVT:
    {
      int send_oos_ind = FALSE;

      QMI_PLATFORM_MUTEX_LOCK (&qmux_general_purpose_mutex);
      if (!(conn_info->status_flags & QMI_QMUX_CONN_STATUS_IN_RESET))
      {
        conn_info->status_flags |= QMI_QMUX_CONN_STATUS_IN_RESET;
        send_oos_ind = TRUE;
        QMI_ERR_MSG_1 ("qmi_qmux:  Modem is RESET on conn_id=%d!\n",conn_id);
        qmi_qmux_forget_all_qmi_service_clients(conn_id);
        qmi_ctl_delete_all_txns (conn_id);
      }
      QMI_PLATFORM_MUTEX_UNLOCK (&qmux_general_purpose_mutex);

      if (send_oos_ind == TRUE)
      {
        qmi_qmux_if_cmd_rsp_type   cmd_data;
        memset((void*)&cmd_data, 0x0, sizeof(cmd_data));
        cmd_data.qmi_qmux_if_sub_sys_restart_ind.conn_id = conn_id;
        qmi_qmux_broadcast_sys_ind_to_all_clients (conn_id, QMI_QMUX_IF_MODEM_OUT_OF_SERVICE_MSG_ID, &cmd_data);
        QMI_QMUX_IF_PLATFORM_REINIT_CONN(conn_id);
      }
    }
    break;

    case QMI_QMUX_IO_PORT_WRITE_FAILED_EVT:
    {
      qmi_qmux_if_cmd_rsp_type   cmd_data;
      memset((void*)&cmd_data, 0x0, sizeof(cmd_data));
      cmd_data.qmi_qmux_if_port_write_failed_ind.conn_id = event_info->qmi_qmux_io_platform_write_failed_err.conn_id;
      cmd_data.qmi_qmux_if_port_write_failed_ind.write_err_code = event_info->qmi_qmux_io_platform_write_failed_err.write_err_code;
      qmi_qmux_broadcast_sys_ind_to_all_clients (conn_id, QMI_QMUX_IF_PORT_WRITE_FAIL_IND_MSG_ID, &cmd_data);
    }
    case QMI_QMUX_IO_PORT_READ_ERR_UNKNOWN_EVT:
    case QMI_QMUX_IO_PORT_READ_ERR_CLEARED_EVT:
    break;

    default:
      QMI_ERR_MSG_1 ("qmi_qmux: I/O Platform EVENT callback: unknown event=%d",(int)event);
      break;
  }

}

/*===========================================================================
  FUNCTION  qmux_open_connection
===========================================================================*/
/*!
@brief
  Function to open a QMI QMUX control port connection.  Function takes
  two parameters:  The connection ID of the connection to open, and a callback
  pointer to a function that will be called when messages are received
  (responses and indications)

@return
  0 if function is successful, negative value if not.

@note

  -

  - Side Effects
    -

*/
/*=========================================================================*/
int qmi_qmux_open_connection (
  qmi_connection_id_type   conn_id,
  qmi_qmux_open_mode_type  mode
)
{
  qmi_qmux_conn_info_type *conn_info;
  int                     rc;
  unsigned char *rx_buf;
  int            rx_buf_len;


  /* Verifiy the conn_id parameter since it will be used to index
  ** into an array
  */
  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_0 ("qmi_qmux_open_connection: bad connection ID\n");
    return QMI_INTERNAL_ERR;
  }

  /* Get pointer to connection info */
  conn_info = &qmi_qmux_conn_info[conn_id];

  /* Lock connection semaphore */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);

  /* Verify that connection isn't already in use
  */
  if (conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_DISABLED)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_open_connection: connection is disabled for conn_id=%d\n",conn_id);
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
    return QMI_INTERNAL_ERR;
  }

  /* Verify that connection isn't already in use
  */
  if (conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_open_connection: connection already opened for conn_id=%d\n",conn_id);
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
    return QMI_NO_ERR;
  }

  /* Unlock the mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);

  if (NULL != qmi_qmux_rx_buffers[conn_id])
  {
    QMI_ERR_MSG_1 ("qmi_qmux_open_connection: freeing pre-existing RX buffer for conn_id=%d\n",conn_id);
    free(qmi_qmux_rx_buffers[conn_id]);
    qmi_qmux_rx_buffers[conn_id] = NULL;
  }

  /* Set up the offset into rx_buf */
  qmi_qmux_rx_buffers[conn_id] = malloc(sizeof(qmi_qmux_rx_buf_type));

  if (NULL == qmi_qmux_rx_buffers[conn_id])
  {
    QMI_ERR_MSG_1 ("qmi_qmux_open_connection: failed to allocate RX buffer for conn_id=%d\n",conn_id);
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
    return QMI_INTERNAL_ERR;
  }

  rx_buf = *qmi_qmux_rx_buffers[conn_id];
  rx_buf += (QMI_QMUX_IF_MSG_HDR_SIZE - QMI_QMUX_HDR_SIZE);
  rx_buf_len = QMI_MAX_MSG_SIZE - (QMI_QMUX_IF_MSG_HDR_SIZE - QMI_QMUX_HDR_SIZE);

  /* If we don't sucessfully open connection, then re-init connection info fields */
  rc = QMI_QMUX_IO_PLATFORM_OPEN_CONN (conn_id,rx_buf,rx_buf_len,mode);

  /* Lock connection semaphore */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);

  if (rc < 0)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_open_connection: QMI_QMUX_IO_PLATFORM_OPEN_CONN failed conn_id=%d rc=%d \n",
                   (int) conn_id,
                   (int) rc);
    conn_info->status_flags &= ~QMI_QMUX_CONN_STATUS_IS_ACTIVE;
    free(qmi_qmux_rx_buffers[conn_id]);
    qmi_qmux_rx_buffers[conn_id] = NULL;
  }
  else
  {
    conn_info->status_flags |= QMI_QMUX_CONN_STATUS_IS_ACTIVE;
  }

  /* We are done, unlock mutex and return success code */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);

  /* If we successfully opened connection, send sync message */
  if ((conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE) && !QMI_CONN_ID_IS_PROXY( conn_id ))
  {
    qmi_ctl_send_sync_msg (conn_id);
  }

  return rc;
} /* qmux_open_connection */


/*===========================================================================
  FUNCTION  qmi_qmux_close_connection
===========================================================================*/
/*!
@brief
  Function to close a QMI QMUX control port connection.  Function takes
  two parameters:  The connection ID of the connection to close

@return
  0 if function is successful, negative value if not.

@note

  -

  - Side Effects
    -
*/

/*=========================================================================*/
int qmi_qmux_close_connection
(
  qmi_connection_id_type  conn_id
)
{
  qmi_qmux_conn_info_type *conn_info;
  int                     rc;

  /* Verifiy the conn_id parameter since it will be used to index
  ** into an array
  */
  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_0 ("qmi_qmux_close_connection: bad connection ID\n");
    return QMI_INTERNAL_ERR;
  }

  /* Get pointer to connection info */
  conn_info = &qmi_qmux_conn_info[conn_id];

  /* Lock connection semaphore */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->list_mutex);

  /* Verify that connection isn't already in use
  */
  if (conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_DISABLED)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_close_connection: connection is disabled for conn_id=%d\n",conn_id);
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
    return QMI_INTERNAL_ERR;
  }

  /* Verify that connection isn't already in use
  */
  if (!(conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_close_connection: connection already inactive for conn_id=%d\n",conn_id);
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);
    return QMI_NO_ERR;
  }

  /* If we don't sucessfully open connection, then re-init connection info fields */
  if ((rc = QMI_QMUX_IO_PLATFORM_CLOSE_CONN (conn_id)) < 0)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_close_connection: QMI_QMUX_IO_PLATFORM_CLOSE_CONN failed conn_id=%d rc=%d \n",
                   (int) conn_id,
                   (int) rc);
  }

  conn_info->status_flags &= ~QMI_QMUX_CONN_STATUS_IS_ACTIVE;
  free(qmi_qmux_rx_buffers[conn_id]);
  qmi_qmux_rx_buffers[conn_id] = NULL;

  /* We are done, unlock mutex and return success code */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->list_mutex);

  return rc;

} /* qmi_qmux_close_connection */


/*===========================================================================
  FUNCTION  qmi_qmux_disable_port
===========================================================================*/
/*!
@brief
  Function to disable/enable a port from being used.  If called with TRUE
  argument, port will be disabled and not available for any use.  If called
  with FALSE argument, port will be re-enabled and available for normal
  operations.  NOTE:  Ports will be enabled by default.  This function must
  only be used if you wish to disable a port or (possibly) re-enable later.
  Also, re-enabling a disabled port will not open the port....
  qmux_open_connection would still need to be called.  The intent of this
  function is to be called at startup to permanently disable ports that are
  not valid in a particular configuration

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    May permanently disable a port from being used.
*/
/*=========================================================================*/
int
qmi_qmux_disable_port
(
  qmi_connection_id_type  conn_id,
  char                   *conn_id_str,
  int                     disable
)
{

  qmi_qmux_conn_info_type *conn_info;
  int                     rc;

  /* Verify connection ID */
  if (((int) conn_id <  (int)QMI_CONN_ID_FIRST) ||
      ((int) conn_id >= (int)QMI_MAX_CONN_IDS))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_disable_port: bad conn_id = %d\n",conn_id);
    return QMI_INTERNAL_ERR;
  }

  conn_info = &qmi_qmux_conn_info[conn_id];

  if (conn_info->status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE)
  {
    QMI_ERR_MSG_3 ("qmi_qmux_disable_port: Cannot disable/enable=%d QMI port "
                   "if already active, conn_id = %d, flags = %x\n",
                   disable,conn_id,conn_info->status_flags);
    return QMI_INTERNAL_ERR;
  }

  /* Set to disabled */
  if (disable)
  {
    conn_info->status_flags |= QMI_QMUX_CONN_STATUS_IS_DISABLED;
    QMI_DEBUG_MSG_2 ("qmi_qmux_disable_port:  Sucessfully disabled port_id=%d, %s\n",conn_id, conn_id_str);
  }
  else
  {
    conn_info->status_flags &= ~QMI_QMUX_CONN_STATUS_IS_DISABLED;
    QMI_DEBUG_MSG_2 ("qmi_qmux_disable_port:  Sucessfully enabled port_id=%d, %s\n",conn_id, conn_id_str);
  }

  return QMI_NO_ERR;

}






/*===========================================================================
  FUNCTION  qmi_qmux_is_connection_active
===========================================================================*/
/*!
@brief
  Routine to query whether or not a specific connection is active

@return
  TRUE if connection is ACTIVE, FALSE otherwise

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
int qmi_qmux_is_connection_active (
  qmi_connection_id_type  conn_id
)
{
  /* Verifiy the conn_id parameter since it will be used to index
  ** into an array
  */
  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    return FALSE;
  }

  return (qmi_qmux_conn_info[conn_id].status_flags & QMI_QMUX_CONN_STATUS_IS_ACTIVE) ? TRUE : FALSE;
} /* qmi_qmux_is_connection_active */


/*===========================================================================
  FUNCTION  qmi_qmux_pwr_up_init
===========================================================================*/
/*!
@brief
  Function to initialize the QMI QMUX layer.  Should only be called once.

@return
  0 if function is successful, negative value if not.

@note
  - Side Effects
    Initializes QMI QMUX subsystem
*/
/*=========================================================================*/
int
qmi_qmux_pwr_up_init (void)
{
  int i,j;

  /* Initialize the QMUX information */
  for (i=0; i < QMI_MAX_CONN_IDS; i++)
  {
    qmi_qmux_conn_info[i].status_flags = 0;
    for (j=0; j<(int)QMI_MAX_SERVICES; j++)
    {
      qmi_qmux_conn_info[i].qmi_qmux_client_srvc_data[j]= NULL;
    }
    QMI_PLATFORM_MUTEX_INIT_RECURSIVE (&qmi_qmux_conn_info[i].list_mutex);
    QMI_PLATFORM_MUTEX_INIT (&qmi_qmux_conn_info[i].tx_mutex);
  }

  QMI_PLATFORM_MUTEX_INIT_RECURSIVE (&qmux_client_list_mutex); /* This needs to be recursive */
  QMI_PLATFORM_MUTEX_INIT (&qmux_general_purpose_mutex);
  QMI_PLATFORM_MUTEX_INIT (&qmux_send_to_client_mutex);

  /* Initialize CTL service */
  qmi_ctl_powerup_init();

  /* Initialize the QMUX platform layer */
  return QMI_QMUX_IO_PLATFORM_PWR_UP_INIT(qmi_qmux_rx_msg,qmi_qmux_event_cb);

}

/****************************** CTL service *********************/
/******************************************************************************
  These are the QMI CTL functions and are kept logically separate from
  the above common QMI service functionality
******************************************************************************/

#define QMI_CTL_TXN_HDR_SIZE        (2) /* 1 byte flags + 1 byte txn id */
#define QMI_CTL_MSG_ID_FIELD_SIZE   (2)
#define QMI_CTL_MSG_LEN_FIELD_SIZE  (2)

#define QMI_CTL_MSG_HDR_SIZE (QMI_QMUX_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE + QMI_CTL_TXN_HDR_SIZE)

#define QMI_CTL_PDU_PTR(ptr) (unsigned char *)((unsigned char *)ptr + QMI_CTL_MSG_HDR_SIZE)
#define QMI_CTL_PDU_SIZE(initial_size) (initial_size - QMI_CTL_MSG_HDR_SIZE)


#define QMI_CTL_MAX_MSG_SIZE        128


/* QMI CTL message ID's */
#define QMI_CTL_GET_VER_INFO_MSG_ID       0x0021
#define QMI_CTL_GET_CLIENT_ID_MSG_ID      0x0022
#define QMI_CTL_RELEASE_CLIENT_ID_MSG_ID  0x0023
#define QMI_CTL_SET_DATA_FORMAT_MSG_ID    0x0026
#define QMI_CTL_SYNC_MSG_ID                       0x0027
#define QMI_CTL_REG_PWR_SAVE_MODE_MSG_ID          0x0028
#define QMI_CTL_CONFIG_PWR_SAVE_SETTINGS_MSG_ID   0x0029
#define QMI_CTL_SET_PWR_SAVE_MODE_MSG_ID          0x002A
#define QMI_CTL_GET_PWR_SAVE_MODE_MSG_ID          0x002B

#define QMI_CTL_SET_SVC_AVAIL_LIST_MSG_ID         0x002E
#define QMI_CTL_GET_SVC_AVAIL_LIST_MSG_ID         0x002F
#define QMI_CTL_SET_EVENT_REPORT_MSG_ID           0x0030
#define QMI_CTL_SVC_AVAIL_IND_MSG_ID              0x0031


/* QMI CTL TLV "type" ID's */
#define QMI_CTL_GET_CLIENT_ID_TYPE_ID_REQ_RSP     0x01
#define QMI_CTL_RELEASE_CLIENT_ID_TYPE_ID_REQ_RSP 0x01
#define QMI_CTL_GET_SERVICE_VERSION_LIST_TYPE_ID      0X01
#define QMI_CTL_GET_ADDENDUM_VERSION_LIST_TYPE_ID     0X10
#define QMI_CTL_SET_DATA_FORMAT_QOS_HDR_STATE_REQ   0x01
#define QMI_CTL_SET_DATA_FORMAT_LINK_PROT_REQ_RSP   0x10
#define QMI_CTL_REG_PWR_SAVE_MODE_REPORT_TYPE_ID_REQ        0x01
#define QMI_CTL_REG_PWR_SAVE_DESCRIPTOR_TYPE_ID_REQ         0x01
#define QMI_CTL_REG_PWR_SAVE_INDICATION_SET_TYPE_ID_REQ     0x11
#define QMI_CTL_SET_PWR_SAVE_MODE_TYPE_ID_REQ               0x01
#define QMI_CTL_GET_PWR_SAVE_MODE_TYPE_ID_RSP               0x01

#define QMI_CTL_PWR_SAVE_MODE_REPORT_TYPE_ID                0x10

#define QMI_CTL_SET_SVC_AVAIL_LIST_REQ 0x01
#define QMI_CTL_SET_EVENT_REPORT_REQ   0x10

/* QMI CTL indication message ID's */
#define QMI_CTL_PWR_SAVE_MODE_IND_MSG_ID  0x0028




#define QMI_CTL_REQUEST_CONTROL_FLAG 0x00
#define QMI_CTL_RESPONSE_CONTROL_FLAG 0x01
#define QMI_CTL_INDICATION_CONTROL_FLAG 0x02




typedef struct qmi_ctl_txn_type
{
  qmi_txn_hdr_type          hdr;
  qmi_qmux_if_msg_hdr_type  qmux_if_hdr;
  unsigned char             ctl_txn_id;
} qmi_ctl_txn_type;

typedef struct
{
  qmi_ctl_txn_type              *qmi_ctl_txn_list;
  unsigned char                 ctl_txn_id;
  QMI_PLATFORM_MUTEX_DATA_TYPE  mutex;
} qmi_ctl_conn_info_type;

static qmi_ctl_conn_info_type qmi_ctl_conn_info [QMI_MAX_CONN_IDS];


void qmi_ctl_powerup_init()
{
  int i;

  /* Initialize all CTL transaction ID's to 1 */
  for (i=0; i< QMI_MAX_CONN_IDS; i++)
  {
    qmi_ctl_conn_info[i].ctl_txn_id = 0;
    qmi_ctl_conn_info[i].qmi_ctl_txn_list = NULL;
    QMI_PLATFORM_MUTEX_INIT (&qmi_ctl_conn_info[i].mutex);
  }
}



static void
qmi_ctl_delete_all_txns
(
  qmi_connection_id_type conn_id
)
{
  qmi_ctl_conn_info_type  *conn_info;
  int i;
  qmi_ctl_txn_type *txn;
  QMI_DEBUG_MSG_1 ("Calling qmi_ctl_delete_all_txns for conn_id=%d",conn_id);
  conn_info = &qmi_ctl_conn_info[conn_id];

  QMI_PLATFORM_MUTEX_LOCK (&conn_info->mutex);
  txn = conn_info->qmi_ctl_txn_list;

  while (txn != NULL)
  {
    qmi_ctl_txn_type *tmp = (qmi_ctl_txn_type *) txn->hdr.next;

    /* Increment the ref count of the transaction to prevent the
     * possibility of the txn being deleted in another thread. The
     * txn would be deleted from memory only when ref count reaches
     * zero */
    qmi_util_addref_txn_no_lock((qmi_txn_hdr_type **) &txn);

    /* If the txn is marked ready_to_delete we do not increment the ref count
     * and would mark it as NULL because it is not valid anymore. We skip over
     * that particular txn and continue with the search because a client can
     * have multiple txns */
    if (txn == NULL)
    {
      txn = tmp;
      continue;
    }

    qmi_util_release_txn_no_lock((qmi_txn_hdr_type *) txn,
                                 TRUE,
                                 (qmi_txn_hdr_type **)&conn_info->qmi_ctl_txn_list);

    txn = tmp;
  }
  conn_info->qmi_ctl_txn_list = NULL;
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->mutex);

  QMI_DEBUG_MSG_0 ("Finishing qmi_ctl_delete_all_txns");
}

static void
qmi_ctl_delete_all_qmux_client_txns (qmi_qmux_clnt_id_t  qmux_client_id)
{
  qmi_ctl_conn_info_type  *conn_info;
  int i;
  qmi_ctl_txn_type *txn;
  QMI_DEBUG_MSG_1 ("Calling qmi_ctl_delete_all_qmux_client_txns for qmux_client_id=%x",
                   qmux_client_id);
  for (i=0; i< QMI_MAX_CONN_IDS; i++)
  {
    conn_info = &qmi_ctl_conn_info[i];

    QMI_PLATFORM_MUTEX_LOCK (&conn_info->mutex);
    txn = conn_info->qmi_ctl_txn_list;

    while (txn != NULL)
    {
      qmi_ctl_txn_type *next = (qmi_ctl_txn_type *) txn->hdr.next;

      if (txn->qmux_if_hdr.qmux_client_id == qmux_client_id)
      {
        /* Increment the ref count of the transaction to prevent the
         * possibility of the txn being deleted in another thread. The
         * txn would be deleted from memory only when ref count reaches
         * zero */
        qmi_util_addref_txn_no_lock((qmi_txn_hdr_type **) &txn);

        /* If the txn is marked ready_to_delete we do not increment the ref count
         * and would mark it as NULL because it is not valid anymore. We skip over
         * that particular txn and continue with the search because a client can
         * have multiple txns */
        if (txn == NULL)
        {
          txn = next;
          continue;
        }

        QMI_DEBUG_MSG_2 ("Releasing pending txn=%d, ref_cnt=%d\n",
                         txn->ctl_txn_id,
                         txn->hdr.ref_count);

        qmi_util_release_txn_no_lock((qmi_txn_hdr_type *) txn,
                                     TRUE,
                                     (qmi_txn_hdr_type **)&conn_info->qmi_ctl_txn_list);
      }

      txn = next;
    }
    QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->mutex);
  }
  QMI_DEBUG_MSG_1 ("Finishing qmi_ctl_delete_all_qmux_client_txns for qmux_client_id=%x",
                   qmux_client_id);
}

static
void qmi_qmux_write_ctl_srvc_msg_hdr (
  unsigned char **msg,
  int           *msg_len,
  unsigned long   msg_id,
  unsigned char   txn_id
)
{
  unsigned char *tmp_msg;
  int           tmp_msg_len;

  tmp_msg_len = *msg_len;

  /* Back up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  *msg -= (QMI_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE);
  *msg_len += (QMI_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE);

  tmp_msg = *msg;

  /* CTL service header consists of a 1 byte control flags
  ** field of message type (request/response/indication),
  ** followed by a 1 byte transaction
  ** ID
  */
  WRITE_8_BIT_VAL (tmp_msg,QMI_CTL_REQUEST_CONTROL_FLAG);
  WRITE_8_BIT_VAL (tmp_msg,txn_id);

  /* Write the message ID field (16 bits) */
  WRITE_16_BIT_VAL (tmp_msg,msg_id);
  /* Write the length field */
  WRITE_16_BIT_VAL (tmp_msg,tmp_msg_len);
}





static
int qmi_ctl_tx_msg (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  unsigned long             qmi_msg_id,
  unsigned char             *msg,
  int                       msg_len,
  unsigned int              alloc_txn
)
{
  qmi_ctl_conn_info_type  *conn_info;
  qmi_ctl_txn_type        *txn = NULL;
  int                     rc;

  /* Verify connection ID */
  if (((int) msg_hdr->qmi_conn_id <  (int)QMI_CONN_ID_FIRST) ||
      ((int) msg_hdr->qmi_conn_id >= (int)QMI_MAX_CONN_IDS))
  {
    QMI_ERR_MSG_1 ("qmi_ctl_tx_msg: bad conn_id=%d\n", msg_hdr->qmi_conn_id);
    return QMI_INTERNAL_ERR;
  }

  conn_info = &qmi_ctl_conn_info[msg_hdr->qmi_conn_id];

  /* Allocate a transaction structure, if desired by caller */
  if (alloc_txn)
  {
  txn = (qmi_ctl_txn_type *)
        qmi_util_alloc_and_addref_txn (sizeof (qmi_ctl_txn_type),
                                       NULL,
                                       (qmi_txn_hdr_type **) &conn_info->qmi_ctl_txn_list,
                                       &conn_info->mutex);

  /* Make sure we sucessfully allocated the txn */
  if (!txn)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Copy message header into txn */
  memcpy (&txn->qmux_if_hdr, msg_hdr, sizeof (qmi_qmux_if_msg_hdr_type));
  }



  /* Lock mutex and get CTL txn ID for connection */
  QMI_PLATFORM_MUTEX_LOCK (&conn_info->mutex);

  /* Get current txn_id and increment, skipping 0
  ** (according to CTL spec)
  */
  if (++conn_info->ctl_txn_id == QMI_INVALID_TXN_ID)
  {
    ++conn_info->ctl_txn_id;
  }

  if (txn)
  {
  txn->ctl_txn_id = conn_info->ctl_txn_id;
  }

  /* Add CTL message headers before sending to QMUX */
  qmi_qmux_write_ctl_srvc_msg_hdr (&msg,
                                   &msg_len,
                                   qmi_msg_id,
                                   conn_info->ctl_txn_id);

  /* Send message to QMUX for transmission */
  if ( ( rc = qmi_qmux_tx_to_modem( msg_hdr->qmi_conn_id,
                                    (qmi_service_id_type) QMI_CTL_SERVICE,
                                    QMI_CTL_SRVC_CLIENT_ID,
                                    msg,
                                    msg_len ) ) < 0 )
  {
    QMI_ERR_MSG_0( "qmi_ctl_tx_msg: qmi_qmux_tx_msg failed\n" );
  }

   /* Unlock mutex... note that we can't do this until message
  ** is sent to prevent modem from potentially receiving CTL txn id's out
  ** of order
  */
  QMI_PLATFORM_MUTEX_UNLOCK (&conn_info->mutex);


  /* Release transaction, but don't delete... receiver will delete it */
  if (txn)
  {
  qmi_util_release_txn ((qmi_txn_hdr_type *) txn,
                          (rc < 0) ? TRUE : FALSE,
                        (qmi_txn_hdr_type **)&conn_info->qmi_ctl_txn_list,
                        &conn_info->mutex);
  }

  return rc;
}

static
int qmi_ctl_handle_set_event_report_req
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int rc = QMI_NO_ERR;
  unsigned char set_event_report = 1;

  (void) msg_hdr;
  (void) cmd_data;

  /* Prepare TLV for QMI_CTL_SET_EVENT_REPORT_MSG_ID */
  if (qmi_util_write_std_tlv(msg,
                             msg_size,
                             QMI_CTL_SET_EVENT_REPORT_REQ,
                             1,
                             (void *) &set_event_report) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_handle_request: write tlv get client failed\n");
    rc = QMI_INTERNAL_ERR;
  }

  return rc;
}

static
int qmi_ctl_handle_reg_srvc_avail_req
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int i, rc = QMI_NO_ERR;
  int service_found = 0;

  (void) msg_hdr;

  QMI_DEBUG_MSG_1("qmi_ctl_handle_reg_srvc_avail_req: Requesting for new service id:%02x\n", cmd_data->qmi_qmux_if_reg_srvc_req.service_id);

  for (i=1; i<QMI_MAX_SERVICES+1; i++)
  {
    if (srvc_list[i] == (unsigned char) cmd_data->qmi_qmux_if_reg_srvc_req.service_id)
    {
      /* Service already present in the request list*/
      QMI_DEBUG_MSG_0("Service requested is already in the list");
      service_found = 1;
      break;
    }
  }

  if (service_found == 0 && srvc_list_index < QMI_MAX_SERVICES+1)
  {
    /* Increment number of service ref count */
    num_services++;
    srvc_list[0] = (unsigned char)num_services;

    /* Add service to the list of requested services */
    srvc_list[srvc_list_index] = (unsigned char) cmd_data->qmi_qmux_if_reg_srvc_req.service_id;

    QMI_DEBUG_MSG_0("New service request, added to list");

    srvc_list_index++;
  }

  /* Prepare the TLV to be sent to the modem */
  if (qmi_util_write_std_tlv(msg,
                            msg_size,
                            QMI_CTL_SET_SVC_AVAIL_LIST_REQ,
                            (unsigned long)(num_services+1),
                            (void *) srvc_list) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_handle_request: write tlv get client failed\n");
    rc = QMI_INTERNAL_ERR;
  }

  return rc;
}




static
int qmi_ctl_handle_get_client_id_req (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int rc = QMI_NO_ERR;

  (void) msg_hdr;

  /* Construct TLV for client ID request */
  if (qmi_util_write_std_tlv (msg,
                             msg_size,
                             QMI_CTL_GET_CLIENT_ID_TYPE_ID_REQ_RSP,
                             1,
                             (void *)&cmd_data->qmi_qmux_if_alloc_client_req.service_id) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_handle_request: write tlv get client failed\n");
    rc = QMI_INTERNAL_ERR;
  }
  return rc;
}


static
int qmi_ctl_handle_release_client_id_req (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int rc = QMI_NO_ERR;
  unsigned char tlv_data[2];

  (void) msg_hdr;

  /* Construct TLV for client ID release request TLV contains service ID followed
  ** by client ID
  */
  tlv_data[0] = (unsigned char) cmd_data->qmi_qmux_if_release_client_req.delete_service_id;
  tlv_data[1] = (unsigned char) cmd_data->qmi_qmux_if_release_client_req.delete_client_id;
  if (qmi_util_write_std_tlv (msg,
                             msg_size,
                             QMI_CTL_RELEASE_CLIENT_ID_TYPE_ID_REQ_RSP,
                             2,
                             (void *)tlv_data) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_handle_request: write tlv release client failed\n");
    rc = QMI_INTERNAL_ERR;
  }
  return rc;
}

static
int qmi_ctl_handle_set_data_format_req (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int rc = QMI_NO_ERR;

  (void) msg_hdr;

  /* Set the QoS header state TLV (mandatory) */
  if (qmi_util_write_std_tlv (msg,
                             msg_size,
                             QMI_CTL_SET_DATA_FORMAT_QOS_HDR_STATE_REQ,
                             1,
                             (void *)&cmd_data->qmi_qmux_if_set_data_format_req.qos_hdr_state) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_handle_set_data_format_req: write QoS hdr state TLV failed\n");
    rc = QMI_INTERNAL_ERR;
  }

  /* Take care of optional link protocol TLV */
  if (cmd_data->qmi_qmux_if_set_data_format_req.link_protocol != QMI_DATA_FORMAT_LINK_PROTOCOL_UNSPECIFIED)
  {
    if (qmi_util_write_std_tlv (msg,
                               msg_size,
                               QMI_CTL_SET_DATA_FORMAT_LINK_PROT_REQ_RSP,
                               2,
                               (void *)&cmd_data->qmi_qmux_if_set_data_format_req.link_protocol) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_ctl_handle_set_data_format_req: write link protocol TLV failed\n");
      rc = QMI_INTERNAL_ERR;
    }
  }
  return rc;
}


static
int qmi_ctl_reg_pwr_save_mode_req (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int rc = QMI_NO_ERR;

  (void) msg_hdr;

  /* Construct TLV for client ID request */
  if (qmi_util_write_std_tlv (msg,
                             msg_size,
                             QMI_CTL_REG_PWR_SAVE_MODE_REPORT_TYPE_ID_REQ,
                             4,
                             (void *)&cmd_data->qmi_qmux_if_reg_pwr_save_mode_req.report_state) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_reg_pwr_save_mode_req: write tlv failed\n");
    rc = QMI_INTERNAL_ERR;
  }
  return rc;
}


static
int qmi_ctl_config_pwr_save_settings_req (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  /* Buffer needs to be big enough to hold the list of power indications ID's
  ** and indication ID's are 2 bytes each
  */
  unsigned char tmp_buf [5];
  unsigned char *tmp_buf_ptr = tmp_buf;
  int rc = QMI_NO_ERR;

  (void) msg_hdr;

  /* First set up mandatory power save descriptor TLV */
  WRITE_32_BIT_VAL (tmp_buf_ptr,cmd_data->qmi_qmux_if_config_pwr_save_settings_req.pwr_state_hndl);
  WRITE_8_BIT_VAL (tmp_buf_ptr,(unsigned char)cmd_data->qmi_qmux_if_config_pwr_save_settings_req.service_id);

  /* Construct TLV for client ID request */
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_CTL_REG_PWR_SAVE_DESCRIPTOR_TYPE_ID_REQ,
                              5,
                             (void *)tmp_buf) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_reg_pwr_save_mode_req: write tlv failed\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set up list of indicator ID's */
  if ((cmd_data->qmi_qmux_if_config_pwr_save_settings_req.num_indication_ids > 0) &&
      (cmd_data->qmi_qmux_if_config_pwr_save_settings_req.num_indication_ids <= QMI_MAX_PWR_INDICATIONS))
  {
    /* Construct TLV for client ID request */
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_CTL_REG_PWR_SAVE_INDICATION_SET_TYPE_ID_REQ,
                                (unsigned long)(2 * cmd_data->qmi_qmux_if_config_pwr_save_settings_req.num_indication_ids),
                                (void *)cmd_data->qmi_qmux_if_config_pwr_save_settings_req.indication_ids) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_ctl_reg_pwr_save_mode_req: write tlv failed\n");
      return QMI_INTERNAL_ERR;
    }
  }
  return rc;
}

static
int qmi_ctl_set_pwr_save_mode_req (
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned char             **msg,
  int                       *msg_size
)
{
  int rc = QMI_NO_ERR;

  (void) msg_hdr;

  /* Construct TLV for client ID request */
  if (qmi_util_write_std_tlv (msg,
                             msg_size,
                             QMI_CTL_SET_PWR_SAVE_MODE_TYPE_ID_REQ,
                             4,
                             (void *)&cmd_data->qmi_qmux_if_set_pwr_save_mode_req.new_pwr_state) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_reg_pwr_save_mode_req: write tlv failed\n");
    rc = QMI_INTERNAL_ERR;
  }
  return rc;
}






static
int qmi_ctl_handle_request (
  qmi_qmux_if_msg_hdr_type *msg_hdr,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  unsigned int              alloc_txn
)
{
  unsigned long qmi_msg_id;
  unsigned char msg[QMI_CTL_MAX_MSG_SIZE];
  unsigned char *tmp_msg_ptr;
  int           msg_free_bytes, rc;


  /* Set message pointer to point to beginning of message
  ** plus the offset needed by CTL service and QMUX
  ** headers
  */
  tmp_msg_ptr = QMI_CTL_PDU_PTR(msg);
  msg_free_bytes = QMI_CTL_PDU_SIZE(QMI_CTL_MAX_MSG_SIZE);


  switch (msg_hdr->msg_id)
  {
    case QMI_QMUX_IF_ALLOC_QMI_CLIENT_ID_MSG_ID:
      qmi_msg_id = QMI_CTL_GET_CLIENT_ID_MSG_ID;
      rc = qmi_ctl_handle_get_client_id_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_GET_VERSION_INFO:
      qmi_msg_id = QMI_CTL_GET_VER_INFO_MSG_ID;
      rc = QMI_NO_ERR;
      break;

    case QMI_QMUX_IF_REG_SRVC_AVAIL_MSG_ID:
      qmi_msg_id = QMI_CTL_SET_SVC_AVAIL_LIST_MSG_ID;
      rc = qmi_ctl_handle_reg_srvc_avail_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_SET_EVENT_REPORT_MSG_ID:
      qmi_msg_id = QMI_CTL_SET_EVENT_REPORT_MSG_ID;
      rc = qmi_ctl_handle_set_event_report_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_RELEASE_QMI_CLIENT_ID_MSG_ID:
      qmi_msg_id = QMI_CTL_RELEASE_CLIENT_ID_MSG_ID;
      rc = qmi_ctl_handle_release_client_id_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

     case QMI_QMUX_IF_SET_DATA_FORMAT_MSG_ID:
      qmi_msg_id = QMI_CTL_SET_DATA_FORMAT_MSG_ID;
      rc = qmi_ctl_handle_set_data_format_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_REG_PWR_SAVE_MODE_MSG_ID:
      qmi_msg_id = QMI_CTL_REG_PWR_SAVE_MODE_MSG_ID;
      rc = qmi_ctl_reg_pwr_save_mode_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_CONFIG_PWR_SAVE_SETTINGS_MSG_ID:
      qmi_msg_id = QMI_CTL_CONFIG_PWR_SAVE_SETTINGS_MSG_ID;
      rc = qmi_ctl_config_pwr_save_settings_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_SET_PWR_STATE_MSG_ID:
      qmi_msg_id = QMI_CTL_SET_PWR_SAVE_MODE_MSG_ID;
      rc = qmi_ctl_set_pwr_save_mode_req (msg_hdr, cmd_data, &tmp_msg_ptr, &msg_free_bytes);
      break;

    case QMI_QMUX_IF_GET_PWR_STATE_MSG_ID:
      qmi_msg_id = QMI_CTL_GET_PWR_SAVE_MODE_MSG_ID;
      rc = QMI_NO_ERR;
      break;

    default:
      qmi_msg_id = 0; /* This is to make lint happy */
      QMI_ERR_MSG_1 ("qmi_ctl_handle_request: unknown message ID = %d\n",msg_hdr->msg_id);
      rc = QMI_INTERNAL_ERR;
      break;
  }

  /* TODO: If we have failure, send message back to client */
  if (rc == QMI_NO_ERR)
  {
    QMI_DEBUG_MSG_1("Sending control message with message id:%02x\n",qmi_msg_id);
    if  ((rc = qmi_ctl_tx_msg (msg_hdr,
                               qmi_msg_id,
                               QMI_CTL_PDU_PTR(msg),
                               QMI_CTL_PDU_SIZE(QMI_CTL_MAX_MSG_SIZE) - msg_free_bytes,
                               alloc_txn)) < 0)

    {
      QMI_ERR_MSG_0 ("qmi_ctl_handle_request: qmi_ctl_tx_msg call failed\n");
    }
  }

  return rc;
}


static
int qmi_qmux_read_ctl_srvc_msg_hdr (
  unsigned char             **msg,
  int                       *msg_len,
  unsigned long             *rx_msg_id,
  unsigned long             *rx_length,
  unsigned long             *rx_txn_id,
  qmi_ctl_service_msg_type  *rx_msg_type
)
{
  unsigned char *tmp_msg;
  unsigned char tmp_msg_type;

    /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  /* Make sure buffer size is correct */
  if (*msg_len < (QMI_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE))
  {
    return QMI_INTERNAL_ERR;
  }

    /* Move up the msg_buf pointer by the size of a QMI_SRVC_STD_TXN_HDR_SIZE */
  tmp_msg = *msg;
  *msg += (QMI_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE);
  *msg_len -= (QMI_CTL_TXN_HDR_SIZE + QMI_SRVC_STD_MSG_HDR_SIZE);

  { /*lint --e{774} */
  /* Read the message type, and translate into the
  ** generic message type enum
  */
  READ_8_BIT_VAL (tmp_msg,tmp_msg_type);

  if (tmp_msg_type == QMI_CTL_REQUEST_CONTROL_FLAG)
  {
    *rx_msg_type = QMI_CTL_SERVICE_REQUEST_MSG;
  }
  else if (tmp_msg_type == QMI_CTL_RESPONSE_CONTROL_FLAG)
  {
    *rx_msg_type = QMI_CTL_SERVICE_RESPONSE_MSG;
  }
  else if (tmp_msg_type == QMI_CTL_INDICATION_CONTROL_FLAG)
  {
    *rx_msg_type = QMI_CTL_SERVICE_INDICATION_MSG;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }
  }

  /* Read 8 bit transaction ID */
  READ_8_BIT_VAL (tmp_msg,*rx_txn_id);

  /* Get the message ID field (16 bits) */
  READ_16_BIT_VAL (tmp_msg,*rx_msg_id);

  /* Get the length field (16 bits) */
  READ_16_BIT_VAL (tmp_msg,*rx_length);

  return QMI_NO_ERR;
}

static
int qmi_qmux_get_ctl_srvc_client_ids (
  unsigned char         **msg,
  int                   *msg_len,
  qmi_client_id_type    *client_id,
  qmi_service_id_type   *service_id
)
{
  unsigned char *value_ptr;
  unsigned long  type = 0, length;
  unsigned char temp;
  int rc;

  if ((rc = qmi_util_read_std_tlv (msg,
                                   msg_len,
                                   &type,
                                   &length,
                                   &value_ptr)) > 0)
  {
    if ((type != QMI_CTL_GET_CLIENT_ID_TYPE_ID_REQ_RSP) &&
             (type != QMI_CTL_RELEASE_CLIENT_ID_TYPE_ID_REQ_RSP))
    {
      rc = QMI_INTERNAL_ERR;
    }
    else
    {
      /* Read service ID and client ID */
      READ_8_BIT_VAL (value_ptr,temp);
      *service_id = (qmi_service_id_type) temp;

      READ_8_BIT_VAL (value_ptr,temp);
      *client_id = (qmi_client_id_type) temp;
      rc = QMI_NO_ERR;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_service_cmp_txn
===========================================================================*/
/*!
@brief
  Routine that should be called once by each service to set the calback
  called to recieve indications, and service-specific write/read transaction
  header routines if they exist

@return
  QMI_NO_ERR if success, negative value if not

@note

  - Dependencies
    - None

  - Side Effects
    -

*/
/*=========================================================================*/
static
int qmi_ctl_cmp_txn (
  qmi_txn_hdr_type  *txn_data,
  void              *cmp_data
)
{
  unsigned char txn_id = (unsigned char) ((unsigned long) cmp_data);
  qmi_ctl_txn_type  *txn = (qmi_ctl_txn_type *) txn_data;
  int rc = 0;

  if ((txn != NULL) && (txn_id != QMI_INVALID_TXN_ID))
  {
    if (txn->ctl_txn_id == txn_id)
    {
      rc = 1;
    }
  }
  return rc;
}

static
int qmi_ctl_handle_get_client_id_rsp (
  unsigned char             *msg,
  int                       msg_len,
  qmi_ctl_txn_type          *txn,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  int                       *qmi_err_code
)
{
  int rc = QMI_NO_ERR;
  qmi_client_id_type  client_id = 0;
  qmi_service_id_type service_id = QMI_MAX_SERVICES;

  while (msg_len > 0)
  {
    /*TLV type*/
    switch (*msg)
    {
      case QMI_RESULT_CODE_TYPE_ID:
        rc = qmi_util_get_std_result_code(&msg, &msg_len, qmi_err_code);
        break;

      default:
        rc = qmi_qmux_get_ctl_srvc_client_ids(&msg,
                                              &msg_len,
                                              &client_id,
                                              &service_id);
        cmd_data->qmi_qmux_if_alloc_client_rsp.new_client_id = client_id;
        cmd_data->qmi_qmux_if_alloc_client_rsp.service_id = service_id;
        break;
    }

    if (rc != QMI_NO_ERR)
    {
      break;
    }
  }

  /* Add client in QMUX */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_qmux_add_qmi_service_client(txn->qmux_if_hdr.qmux_client_id,
                                         txn->qmux_if_hdr.qmi_conn_id,
                                         service_id,
                                         client_id);
  }
  return rc;
}

static
int qmi_ctl_handle_get_version_info_rsp (
    unsigned char             *msg,
    int                       msg_len,
    qmi_ctl_txn_type          *txn,
    qmi_qmux_if_cmd_rsp_type  *cmd_data,
    int                       *qmi_err_code
    )
{
  unsigned char   *value_ptr;
  unsigned long   type, length;
  unsigned char   temp;
  unsigned char   num_instances;
  unsigned char   qmi_svc_type;
  unsigned short  major_ver;
  unsigned short  minor_ver;
  int rc,index;

  (void) txn;

  rc = QMI_NO_ERR;
  while (msg_len > 0)
  {
    if (*msg == QMI_RESULT_CODE_TYPE_ID)
    {
      rc = qmi_util_get_std_result_code(&msg, &msg_len, qmi_err_code);
    }
    else
    {
      if ((rc = qmi_util_read_std_tlv(&msg,
                                      &msg_len,
                                      &type,
                                      &length,
                                      &value_ptr)) < 0)
      {
        return rc;
      }

      QMI_DEBUG_MSG_0("Parsing the version list \n");
      switch (type)
      {
        case QMI_CTL_GET_SERVICE_VERSION_LIST_TYPE_ID:
          /* Read number of instances */
          READ_8_BIT_VAL (value_ptr,num_instances);

          if (num_instances > QMI_MAX_SERVICE_VERSIONS )
          {
            QMI_DEBUG_MSG_1("Num of instances greater than Max service version:%d\n",QMI_MAX_SERVICE_VERSIONS);
            return QMI_INTERNAL_ERR;
          }

          cmd_data->qmi_qmux_if_get_version_info_rsp.qmi_service_version_len = num_instances;
          QMI_DEBUG_MSG_1("Number of instances is :%d\n",num_instances);
          for (index = 0;index < num_instances; index++ )
          {
            /* Read the service type */
            READ_8_BIT_VAL(value_ptr,qmi_svc_type);
            /* Read the major number */
            READ_16_BIT_VAL(value_ptr,major_ver);
            /* Read the minor number */
            READ_16_BIT_VAL(value_ptr,minor_ver);

            /* Copy it in the structure */
            cmd_data->qmi_qmux_if_get_version_info_rsp.qmi_service_version[index].qmi_svc_type =  qmi_svc_type;
            cmd_data->qmi_qmux_if_get_version_info_rsp.qmi_service_version[index].major_ver = major_ver;
            cmd_data->qmi_qmux_if_get_version_info_rsp.qmi_service_version[index].minor_ver = minor_ver;
            QMI_DEBUG_MSG_1("Service ID ........%d\n",qmi_svc_type);
            QMI_DEBUG_MSG_1("Major Number.......%d\n",major_ver);
            QMI_DEBUG_MSG_1("Minor Number.......%d\n",minor_ver);

          }
          QMI_DEBUG_MSG_0("Parsing successful ........\n");
          rc = QMI_NO_ERR;
          break;

        default:
          rc = QMI_INTERNAL_ERR;
          break;
      } /* switch type */
    }
  } /* while */

  return rc;
}

static
int qmi_ctl_handle_release_client_id_rsp (
  unsigned char             *msg,
  int                       msg_len,
  qmi_ctl_txn_type          *txn,
  int                       *qmi_err_code
)
{
  int rc = QMI_NO_ERR;
  qmi_client_id_type  client_id;
  qmi_service_id_type service_id;

  while (msg_len > 0)
  {
    switch (*msg)/*TLV type*/
    {
      case QMI_RESULT_CODE_TYPE_ID:
        rc = qmi_util_get_std_result_code(&msg, &msg_len, qmi_err_code);
        break;

      default:
        rc = qmi_qmux_get_ctl_srvc_client_ids(&msg,
                                              &msg_len,
                                              &client_id,
                                              &service_id);
        break;
    }

    if (rc != QMI_NO_ERR)
    {
      break;
    }
  }

  /* Add client in QMUX.  Only do so if the QMUX client ID is valid.  QMUX
  ** client ID will be invalid for "internally" (qmux) generated removals which
  ** happens when a QMUX client dies without releasing it's QMI client ID's
  */
  if (rc == QMI_NO_ERR &&
      (txn->qmux_if_hdr.qmux_client_id != QMI_QMUX_INVALID_QMUX_CLIENT_ID))
  {
    rc = qmi_qmux_remove_qmi_service_client (txn->qmux_if_hdr.qmux_client_id,
                                             txn->qmux_if_hdr.qmi_conn_id,
                                             service_id,
                                             client_id);
  }
  return rc;
}


static
int qmi_ctl_handle_set_data_format_rsp (
  unsigned char             *msg,
  int                       msg_len,
  qmi_ctl_txn_type          *txn,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  int                       *qmi_err_code
)
{
  unsigned char *value_ptr;
  unsigned long  type, length, temp;
  int rc;

  (void) txn;

  /* Initialize response data */
  rc = QMI_NO_ERR;
  cmd_data->qmi_qmux_if_set_data_format_rsp.link_protocol =
                               QMI_DATA_FORMAT_LINK_PROTOCOL_UNSPECIFIED;

  while (msg_len > 0)
  {
    switch (*msg)/*TLV type*/
    {
      case QMI_RESULT_CODE_TYPE_ID:
        rc = qmi_util_get_std_result_code (&msg, &msg_len, qmi_err_code);
        break;

      default:
        while (msg_len > 0)
        {
          if (qmi_util_read_std_tlv (&msg,
                                     &msg_len,
                                     &type,
                                     &length,
                                     &value_ptr) > 0)
          {
            if (type != QMI_CTL_SET_DATA_FORMAT_LINK_PROT_REQ_RSP)
            {
              QMI_ERR_MSG_1 ("qmi_ctl_handle_set_data_format_rsp, skipping unknown type = %d\n",(int)type);
            }
            else
            {
              /* Read service ID and client ID */
              READ_16_BIT_VAL (value_ptr,temp);
              cmd_data->qmi_qmux_if_set_data_format_rsp.link_protocol =
                                         (qmi_link_layer_protocol_type) temp;
            }
          }
          else
          {
            rc = QMI_INTERNAL_ERR;
            break;
          }
        } /* while */
        break;
    } /* switch */

    if (rc != QMI_NO_ERR)
    {
      break;
    }
  }
  /* Process all TLV's */

  return rc;
}



static
int qmi_ctl_get_pwr_save_mode_rsp (
  unsigned char             *msg,
  int                       msg_len,
  qmi_ctl_txn_type          *txn,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  int                       *qmi_err_code
)
{
  unsigned char *value_ptr;
  unsigned long  type, length;
  int rc = QMI_NO_ERR;

  (void) txn;

  while (msg_len > 0)
  {
    switch (*msg)/*TLV type*/
    {
     case QMI_RESULT_CODE_TYPE_ID:
       rc = qmi_util_get_std_result_code(&msg, &msg_len, qmi_err_code);
       break;

     default:
       if (qmi_util_read_std_tlv(&msg,
                                 &msg_len,
                                 &type,
                                 &length,
                                 &value_ptr) > 0)
       {
         if (type == QMI_CTL_GET_PWR_SAVE_MODE_TYPE_ID_RSP)
         {
           unsigned long tmp;
           READ_32_BIT_VAL (value_ptr,tmp);
           cmd_data->qmi_qmux_if_get_pwr_save_mode_rsp.curr_pwr_state = tmp;
         }
         else
         {
           QMI_ERR_MSG_1 ("qmi_ctl_get_pwr_save_mode_rsp: Unknown TLV type = %x",(unsigned)type);
           rc = QMI_INTERNAL_ERR;
         }
       }
       break;
    } /* switch */

    if (rc != QMI_NO_ERR)
    {
      break;
    }
  } /* while */

  return rc;
}

static
int qmi_ctl_get_pwr_save_ind_data (
  unsigned char             *msg,
  int                       msg_len,
  qmi_qmux_if_cmd_rsp_type  *cmd_data
)
{
  unsigned char *value_ptr;
  unsigned long  type, length;
  int rc = QMI_INTERNAL_ERR;

  while (msg_len > 0)
  {
    if (qmi_util_read_std_tlv (&msg,
                               &msg_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      rc = QMI_INTERNAL_ERR;
      break;
    }
    else
    {
      switch (type)
      {
        case QMI_CTL_PWR_SAVE_MODE_REPORT_TYPE_ID:
        {
          unsigned long tmp;
          READ_32_BIT_VAL (value_ptr,tmp);
          cmd_data->qmi_qmux_if_pwr_state_ind.curr_pwr_state_hndl = (int)tmp;
          READ_32_BIT_VAL (value_ptr,tmp);
          cmd_data->qmi_qmux_if_pwr_state_ind.prev_pwr_state_hndl = (int)tmp;
          rc = QMI_NO_ERR;
        }
        break;

        default:
        {
          QMI_ERR_MSG_1 ("qmi_ctl_get_pwr_save_mode_rsp: Unknown TLV type = %x",(unsigned)type);
        }
        break;
      } /* switch */
    } /* else */
  } /* while */
  return rc;
}

static
void qmi_ctl_process_indication (
  qmi_connection_id_type    conn_id,
  unsigned long             rx_msg_id,
  unsigned char             *msg,
  int                       msg_len
)
{
  qmi_qmux_if_cmd_rsp_type  cmd_data;
  qmi_qmux_if_msg_id_type   ind_id = QMI_QMUX_IF_MAX_NUM_MSGS;
  int rc;

  QMI_DEBUG_MSG_2 ("qmi_qmux: CTL indication received ind_id=%d, conn_id=%d\n",(int)rx_msg_id,(int)conn_id);

  if( !QMI_CONN_ID_IS_VALID( conn_id ) )
  {
    QMI_ERR_MSG_1 ("qmi_qmux: CTL indication received invalid conn_id=%d\n",(int)conn_id);
    return;
  }

  memset((void*)&cmd_data, 0x0, sizeof(cmd_data));

  /* Fill in the command data for the indication */
  switch (rx_msg_id)
  {
    case QMI_CTL_PWR_SAVE_MODE_IND_MSG_ID:
      ind_id = QMI_QMUX_IF_PWR_STATE_IND_MSG_ID;
      rc = qmi_ctl_get_pwr_save_ind_data (msg,msg_len,&cmd_data);
      break;

    case QMI_CTL_SYNC_MSG_ID:
    {
      ind_id = QMI_QMUX_IF_SYNC_IND_MSG_ID;
      rc = qmi_qmux_process_rx_ctl_sync_msg (conn_id,&cmd_data);
    }
    break;

  case QMI_CTL_SVC_AVAIL_IND_MSG_ID:
      /* Nothing to process from modem so just set appropriate
      ** msg_id */
      ind_id = QMI_QMUX_IF_NEW_SRVC_AVAIL_MSG_ID;
      rc = QMI_NO_ERR;
      break;

    default:
      QMI_ERR_MSG_1 ("Unknown CTL indication received, ind_id=%x\n",(unsigned)rx_msg_id);
      rc = QMI_INTERNAL_ERR;
      break;
  }

  /* If we get a recognized, well formed indication, send it to all QMUX clients */
  if (rc == QMI_NO_ERR)
  {
    qmi_qmux_broadcast_sys_ind_to_all_clients (conn_id, ind_id, &cmd_data);
  }
}


/*===========================================================================
  FUNCTION  qmi_ctl_send_raw_indication
===========================================================================*/
/*!
@brief
  Utility function for sending raw QMI CTL indications to expecting
  clients

@return
  None.

@note

  - Side Effects
    - Broad

*/
/*=========================================================================*/
static void
qmi_ctl_send_raw_indication
(
  qmi_connection_id_type  conn_id,
  unsigned char           control_flags,
  unsigned long           msg_id,
  unsigned char           *raw_ind,
  int                     raw_ind_len
)
{
  qmi_qmux_client_list_type  *client = NULL;
  qmi_qmux_client_list_type  *next_client = NULL;

  if (!raw_ind || raw_ind_len <= 0)
  {
    QMI_ERR_MSG_0("qmi_ctl_send_raw_indication: bad param(s)\n");
    return;
  }

  /* Lock the list mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmux_client_list_mutex);

  /* Cycle through all of the QMUX clients and send the raw indication.
  **
  ** Note that due to the fact that in some implementations, it is possible
  ** that calling the qmi_qmux_if_send_to_client() function can result in the
  ** qmux_client_id client being deleted from the same thread context, we have
  ** made the qmux_client_list_mutex recursive, and we will not access the
  ** client data structure at all after the qmi_qmux_if_send_to_client() call.
  ** ONLY the current client can be deleted in this scenario and since the mutex
  ** is locked, no other clients can be deleted, so it is safe to continue to use
  ** the next pointer subsequent to the call, just not the current client pointer.
  */
  client = qmux_client_list;
  while ( client != NULL )
  {
    next_client = client->next;

    /* Tunnel the raw indication as a QMI message to the clients expecting
       a raw indication */
    if (QMI_QMUX_IF_CLNT_MODE_RAW == client->qmux_client_mode)
    {
      /* Send raw indication to client */
      QMI_DEBUG_MSG_2 ("qmi_qmux:  Sending raw indication %d to qmux client 0x%x",
                       (int)msg_id,
                       (int)client->qmux_client_id);

      qmi_qmux_if_send_to_client ( QMI_QMUX_IF_QMI_MSG_ID,
                                   client->qmux_client_id,
                                   QMI_INVALID_TXN_ID,
                                   conn_id,
                                   QMI_CTL_SERVICE,
                                   0, /* Not used */
                                   control_flags,
                                   QMI_NO_ERR,
                                   QMI_SERVICE_ERR_NONE,
                                   raw_ind,
                                   raw_ind_len );
    }

    /* In some implementations, client pointer may have been deleted by qmi_qmux_if_send_to_client(),
    ** so don't dereference below that call.  Set client pointer to previously saved next pointer
    ** value.
    */
    client = next_client;
  }

  /* Unlock the list mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmux_client_list_mutex);
}


static void qmi_ctl_rx_msg
(
  qmi_connection_id_type  conn_id,
  unsigned char           control_flags,
  unsigned char           *msg,
  int                      msg_len
)
{

  unsigned long             rx_msg_id;
  unsigned long             rx_length;
  unsigned long             rx_txn_id;
  qmi_ctl_service_msg_type  rx_msg_type;
  qmi_ctl_conn_info_type    *conn_info;
  qmi_ctl_txn_type          *txn;
  int                       rc = QMI_NO_ERR,qmi_err_code;
  qmi_qmux_if_cmd_rsp_type  cmd_data;
  unsigned char             tx_buf [QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];
  unsigned char             *raw_msg = msg;
  int                       raw_msg_len = msg_len;


  /* Get CTL service message header values */
  if (qmi_qmux_read_ctl_srvc_msg_hdr (&msg,
                                      &msg_len,
                                      &rx_msg_id,
                                      &rx_length,
                                      &rx_txn_id,
                                      &rx_msg_type) < 0)
  {
    QMI_ERR_MSG_0 ("qmi_ctl_rx_msg: qmi_qmux_read_ctl_srvc_msg_hdr failed\n");
    // TODO - Send error message to client
    return;
  }

  /* Sanity check length values */
  if ((int)rx_length != msg_len)
  {
    QMI_ERR_MSG_2 ("qmi_ctl_rx_msg.c RX: srvc_msg_len (%d) != rx_msg_len (%d)\n",
           (int)rx_length,(int)msg_len);
    // TODO - Send error message to client
    return;
  }

  /* Check to see if we have received an CTL indication */
  if (rx_msg_type == QMI_CTL_SERVICE_INDICATION_MSG)
  {
    /* Send the raw indication to any interested clients */
    qmi_ctl_send_raw_indication(conn_id,
                                control_flags,
                                rx_msg_id,
                                raw_msg,
                                raw_msg_len);

    qmi_ctl_process_indication (conn_id,
                                rx_msg_id,
                                msg,
                                msg_len);
    return;
  }

  /* Make sure message is a response... */
  if (rx_msg_type != QMI_CTL_SERVICE_RESPONSE_MSG)
  {
    QMI_ERR_MSG_1 ("qmi_ctl_rx_msg.c received non-response msg = %x\n", rx_msg_type);
    return;
  }

  /* Look up transaction */
  conn_info = &qmi_ctl_conn_info[conn_id];

  txn = (qmi_ctl_txn_type *)
          qmi_util_find_and_addref_txn ((void *) rx_txn_id,
                                        qmi_ctl_cmp_txn,
                                        (qmi_txn_hdr_type **)&conn_info->qmi_ctl_txn_list,
                                        &conn_info->mutex);

  if (!txn)
  {
    QMI_ERR_MSG_1 ("qmi_ctl_rx_msg.c Can't find txn info for txn_id = %d\n", (int) rx_txn_id);
    return;
  }

  /* Zero out cmd_data for good measure. */
  memset (&cmd_data,0,sizeof (qmi_qmux_if_cmd_rsp_type));
  QMI_DEBUG_MSG_0(" Striping of the standard result code ...\n");
  /* Read the standard result code from message */
  /* Process message */
  switch (rx_msg_id)
  {
     case QMI_CTL_GET_CLIENT_ID_MSG_ID:
       rc = qmi_ctl_handle_get_client_id_rsp (msg, msg_len, txn, &cmd_data, &qmi_err_code);
       break;

     case QMI_CTL_GET_VER_INFO_MSG_ID:
       printf("  Parsing the version information ...\n");
       QMI_DEBUG_MSG_0(" Parsing the version information ...\n");
       rc = qmi_ctl_handle_get_version_info_rsp(msg, msg_len, txn, &cmd_data,&qmi_err_code);
       break;

     case QMI_CTL_RELEASE_CLIENT_ID_MSG_ID:
       rc = qmi_ctl_handle_release_client_id_rsp (msg, msg_len, txn, &qmi_err_code);
     break;

     case QMI_CTL_SET_DATA_FORMAT_MSG_ID:
       rc = qmi_ctl_handle_set_data_format_rsp (msg, msg_len, txn, &cmd_data,&qmi_err_code);
     break;

     case QMI_CTL_GET_PWR_SAVE_MODE_MSG_ID:
       rc = qmi_ctl_get_pwr_save_mode_rsp (msg, msg_len, txn, &cmd_data,&qmi_err_code);
     break;

     /* No reply data other than result code for following messages */
     case QMI_CTL_REG_PWR_SAVE_MODE_MSG_ID:
     case QMI_CTL_CONFIG_PWR_SAVE_SETTINGS_MSG_ID:
     case QMI_CTL_SET_PWR_SAVE_MODE_MSG_ID:
     case QMI_CTL_SET_SVC_AVAIL_LIST_MSG_ID:
     case QMI_CTL_SET_EVENT_REPORT_MSG_ID:
     break;

     default:
       rc = QMI_INTERNAL_ERR;
       QMI_ERR_MSG_1 ("qmi_ctl_rx_msg.c Unhandled RX msg ID = %d\n", (int) rx_msg_id);
  }

  /* If this is a response to a raw control request previously sent */
  if (QMI_QMUX_IF_SEND_RAW_QMI_CTL_MSG_ID == txn->qmux_if_hdr.msg_id)
  {
    QMI_DEBUG_MSG_2("Received QMI CTL RAW response ... qmux_txn=0x%x clnt_txn=0x%x\n",
                    txn->ctl_txn_id,
                    txn->qmux_if_hdr.qmux_txn_id);

    qmi_qmux_handle_raw_qmi_ctl_resp(&txn->qmux_if_hdr,
                                     control_flags,
                                     raw_msg,
                                     raw_msg_len);
  }
  else
  {
    /* Set up tx_msg buffer by copying command data to it */
    memcpy( ( unsigned char * ) ( tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE ),
            &cmd_data,
            sizeof( qmi_qmux_if_cmd_rsp_type ) );

    /* Send result to client */
    qmi_qmux_if_send_to_client( txn->qmux_if_hdr.msg_id,
                                txn->qmux_if_hdr.qmux_client_id,
                                txn->qmux_if_hdr.qmux_txn_id,
                                conn_id,
                                txn->qmux_if_hdr.qmi_service_id,
                                txn->qmux_if_hdr.qmi_client_id,
                                0, // control flags not used
                                rc,
                                qmi_err_code,
                                ( unsigned char * ) ( tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
                                sizeof( qmi_qmux_if_cmd_rsp_type ) );
  }

  /* Delete the transaction */
  qmi_util_release_txn( ( qmi_txn_hdr_type * ) txn,
                        TRUE,
                        ( qmi_txn_hdr_type ** ) &conn_info->qmi_ctl_txn_list,
                        &conn_info->mutex );

} /* qmi_ctl_rx_msg  */


static
void qmi_ctl_send_sync_msg  (
  qmi_connection_id_type conn_id
)
{
  qmi_qmux_if_msg_hdr_type  msg_hdr;
  unsigned char msg[QMI_CTL_MAX_MSG_SIZE];

  /* Initialize fields of msg_hdr */
   /* Initialize message header fields for message to be passed to QMUX
  */
  msg_hdr.msg_id = QMI_QMUX_IF_MAX_NUM_MSGS; /* Set to invalid message ID */
  msg_hdr.qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;
  msg_hdr.qmi_conn_id = conn_id;

  /* Send message to modem.  Note that at this point, we don't bother allocating
  ** a transaction since most targets will not have support for this, we will
  ** never receive a reply which means a memory of the txn data.  So, right
  ** now we send it and forget it
  */
  if  (qmi_ctl_tx_msg (&msg_hdr,
                       QMI_CTL_SYNC_MSG_ID,
                       QMI_CTL_PDU_PTR(msg),
                       0,
                       FALSE) < 0)

  {
    QMI_ERR_MSG_0 ("qmi_ctl_send_qmi_sync_msg: qmi_ctl_tx_msg call failed\n");
  }

}


/*===========================================================================
  FUNCTION qmi_qmux_handle_raw_qmi_ctl_req
===========================================================================*/
/*!
@brief
  This function sends the raw QMI CTL request to the modem

@return
 QMI_NO_ERR on success or QMI_*_ERR on error

@note
  - Side Effects
    - None

*/
/*=========================================================================*/
static int
qmi_qmux_handle_raw_qmi_ctl_req
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  unsigned char             *msg,
  int                       msg_len
)
{
  int rc = QMI_INTERNAL_ERR;
  unsigned short qmi_msg_id = 0;
  unsigned short qmi_msg_len = 0;

  if (!msg_hdr || !msg || msg_len < (QMI_CTL_TXN_HDR_SIZE + QMI_CTL_MSG_ID_FIELD_SIZE + QMI_CTL_MSG_LEN_FIELD_SIZE))
  {
    QMI_ERR_MSG_0 ("qmi_qmux_handle_raw_qmi_ctl_req: bad param(s)\n");
    return rc;
  }

  /* Skip over the QMI control TXN header */
  msg += QMI_CTL_TXN_HDR_SIZE;

  /* Get the message ID field (16 bits) */
  READ_16_BIT_VAL (msg,qmi_msg_id);

  /* Get the message length field (16 bits) */
  READ_16_BIT_VAL (msg,qmi_msg_len);

  /* Ensure that the length of the QMI message is of expected value */
  if ((msg_len - (QMI_CTL_TXN_HDR_SIZE + QMI_CTL_MSG_ID_FIELD_SIZE + QMI_CTL_MSG_LEN_FIELD_SIZE)) != qmi_msg_len)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_handle_raw_qmi_ctl_req: length mismatch msg_len=%d, qmi_msg_len=%d\n",
                   (msg_len - (QMI_CTL_TXN_HDR_SIZE + QMI_CTL_MSG_ID_FIELD_SIZE + QMI_CTL_MSG_LEN_FIELD_SIZE)),
                   (int)qmi_msg_len);
    return rc;
  }

  QMI_DEBUG_MSG_3("Sending raw control message with msg_id=0x%2x, qmux_clnt=0x%x, qmux_txn=0x%x\n",
                  qmi_msg_id,
                  msg_hdr->qmux_client_id,
                  msg_hdr->qmux_txn_id);

  if  ((rc = qmi_ctl_tx_msg (msg_hdr,
                             (unsigned long) qmi_msg_id,
                             msg,
                             (int) qmi_msg_len,
                             TRUE)) < 0)

  {
    QMI_ERR_MSG_0 ("qmi_qmux_handle_raw_qmi_ctl_req: qmi_ctl_tx_msg call failed\n");
  }

  return rc;
}


/*===========================================================================
  FUNCTION qmi_qmux_handle_raw_qmi_ctl_resp
===========================================================================*/
/*!
@brief
  This function sends the raw QMI CTL response to the client that issued
  the request earlier

@return
 QMI_NO_ERR on success or QMI_*_ERR on error

@note
  - Side Effects
    - None

*/
/*=========================================================================*/
static int
qmi_qmux_handle_raw_qmi_ctl_resp
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  unsigned char             control_flags,
  unsigned char             *msg,
  int                       msg_len
)
{
  unsigned char *ctl_txn = NULL;
  unsigned char *tmp_msg = NULL;
  unsigned short qmi_msg_id = 0;

  if (!msg_hdr || !msg)
  {
    QMI_ERR_MSG_0 ("qmi_qmux_handle_raw_qmi_ctl_resp: bad param(s)\n");
    return QMI_INTERNAL_ERR;
  }

  /* Update the txn id in the response */
  ctl_txn = msg+1;
  WRITE_8_BIT_VAL(ctl_txn, msg_hdr->qmux_txn_id);

  /* Skip over the QMI control TXN header */
  tmp_msg = msg + QMI_CTL_TXN_HDR_SIZE;

  /* Get the message ID field (16 bits) */
  READ_16_BIT_VAL (tmp_msg,qmi_msg_id);

  QMI_DEBUG_MSG_3("Sending raw control response with msg_id=0x%2x, qmux_clnt=0x%x, qmux_txn=0x%x\n",
                  qmi_msg_id,
                  msg_hdr->qmux_client_id,
                  msg_hdr->qmux_txn_id);

  /* Tunnel the raw response as a QMI message to the client */
  qmi_qmux_if_send_to_client( QMI_QMUX_IF_QMI_MSG_ID,
                              msg_hdr->qmux_client_id,
                              msg_hdr->qmux_txn_id,
                              msg_hdr->qmi_conn_id,
                              msg_hdr->qmi_service_id,
                              msg_hdr->qmi_client_id,
                              control_flags,
                              QMI_NO_ERR,
                              QMI_SERVICE_ERR_NONE,
                              msg,
                              msg_len );

  return QMI_NO_ERR;
}


// ---------------------------------------------------------------------------------------
// qmi_qcci_internal_public_service_id_to_bookkeeping_service_id
// ---------------------------------------------------------------------------------------
int qmi_qcci_internal_public_service_id_to_bookkeeping_service_id(int public_service_id)
{
   int res;

   if ( public_service_id < QMI_FIRST_VS_SERVICE )
   {
     res = public_service_id;
   }
   else
   {
     switch ( public_service_id )
     {
       case 0xE3: // RF SAR
         res = QMI_RF_SAR_SERVICE;
         break;

       default:
         res = QMI_MAX_SERVICES;
         break;
     }
   }

   return res;
} // qmi_qcci_internal_public_service_id_to_bookkeeping_service_id

// ---------------------------------------------------------------------------------------
// qmi_qcci_internal_bookkeeping_service_id_to_public_service_id
// ---------------------------------------------------------------------------------------
int qmi_qcci_internal_bookkeeping_service_id_to_public_service_id(int bookkeeping_service_id)
{
   int res;

   if ( bookkeeping_service_id < QMI_FIRST_VS_SERVICE )
   {
     res = bookkeeping_service_id;
   }
   else
   {
     switch ( bookkeeping_service_id )
     {
       case QMI_RF_SAR_SERVICE:
         res = 0xE3; // RF SAR
         break;

       default:
         res = QMI_MAX_SERVICES;
         break;
     }
   }

   return res;
} // qmi_qcci_internal_bookkeeping_service_id_to_public_service_id

