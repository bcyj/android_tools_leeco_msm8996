/******************************************************************************
  @file    qmi_service.c
  @brief   The QMI common service layer.

  DESCRIPTION
  QMI common service routines.  All services will be build on top of these
  routines for initializing, sending messages and receiving responses/
  indications.  File also contains some common utility routines for decoding
  message headers and TLV's.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_service_init() needs to be called before sending or receiving of any
  service specific messages

  ---------------------------------------------------------------------------
  Copyright (c) 2007-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

/*
qmi_service.c:2353: warning: implicit declaration of function 'qmi_qmux_if_setup_ctl_txn'

*/

#include <sys/types.h>
#include <unistd.h>
#include "qmi.h"
#include "qmi_i.h"
#include "qmi_platform.h"
#include "qmi_qmux_if.h"
#include "qmi_qmux.h"
#include "qmi_util.h"
#include "qmi_proxy.h"

static qmi_sys_event_rx_hdlr  qmi_qmux_if_sys_event_hdlr = NULL;
static void                   *qmi_qmux_if_sys_event_user_data = NULL;

static QMI_PLATFORM_MUTEX_DATA_TYPE   qmi_qmux_if_tx_mutex;

#ifdef QMI_MSGLIB_MULTI_PD
#define QMI_QMUX_IF_BUF_SIZE   QMI_MAX_MSG_SIZE
static unsigned char qmi_qmux_if_rx_buf[QMI_QMUX_IF_BUF_SIZE];
#endif


#define QMI_QMUX_IF_DEFAULT_SYNC_TIMEOUT QMI_SYNC_MSG_DEFAULT_TIMEOUT


typedef struct
{
  qmi_txn_hdr_type                hdr;
  qmi_qmux_if_msg_hdr_type        qmux_hdr;
  qmi_qmux_if_cmd_rsp_type        cmd_data;
  QMI_PLATFORM_SIGNAL_DATA_TYPE   signal_data;
} qmi_qmux_if_txn_type;

static qmi_txn_hdr_type *qmi_qmux_if_txn_list;
static QMI_PLATFORM_MUTEX_DATA_TYPE qmi_qmux_if_txn_list_mutex;


/* Define a port for use for internal communications.  Initialize this based
** on whether or not platform defines a port (qmi_platform_config.h).  If
** platform defines, then initizlize to INVALID (will be set in
** qmi_qmux_if_pwr_up_init()), otherwise initialize to RMNET_0
*/
#ifdef QMI_PLATFORM_INTERNAL_USE_PORT_ID
static qmi_connection_id_type qmi_qmux_if_internal_use_conn_id = QMI_CONN_ID_INVALID;
#else
static qmi_connection_id_type qmi_qmux_if_internal_use_conn_id = QMI_CONN_ID_RMNET_0;
#endif

/* Data structures dealing with having multiple qmux_if clients */
QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX (qmi_qmux_if_client_list_mutex);

typedef struct qmi_qmux_if_client_data_type
{
  struct qmi_qmux_if_client_data_type   *next;
  qmi_qmux_clnt_id_t                    qmux_client_id;
  qmi_qmux_if_rx_msg_hdlr_type          rx_msg_hdlr;
  qmi_sys_event_rx_hdlr                 sys_event_rx_hdlr;
  void                                  *sys_event_user_data;
  unsigned char                         *rx_msg_buf;
  qmi_qmux_if_clnt_mode_t               qmux_client_mode;
}qmi_qmux_if_client_data_type;

/* List of qmux_if clients */
static qmi_qmux_if_client_data_type *qmi_qmux_if_client_data_list = NULL;

/* Initialization state */
static int qmi_qmux_if_is_initialized = FALSE;

/* QMUX txn ID's */
static unsigned long                  qmi_qmux_if_qmux_txn_id       = 0;

static boolean                        event_reports_enabled = FALSE;

/* Service versions cache */
static boolean is_version_list_cached[QMI_MAX_CONN_IDS];
static qmi_service_version_list_type version_list_cache[QMI_MAX_CONN_IDS];

/* Global mutex to protect competing qmi_qmux_if operations */
QMI_PLATFORM_MUTEX_CREATE_AND_INIT_STATIC_MUTEX(qmi_qmux_if_mutex);

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

static int
qmi_qmux_if_send_to_qmux
(
  qmi_qmux_if_hndl_t         qmux_if_client_handle,
  qmi_qmux_if_msg_id_type    msg_id,
  unsigned long              qmux_txn_id,
  qmi_connection_id_type     qmi_conn_id,
  qmi_service_id_type        qmi_service_id,
  qmi_client_id_type         qmi_client_id,
  unsigned char              *msg,
  int                        msg_len,
  qmi_qmux_clnt_id_t         *qmux_client_id_ptr
)
{
  qmi_qmux_if_msg_hdr_type hdr;
  int rc;
  qmi_qmux_if_client_data_type  *client, *prev;
  qmi_qmux_clnt_id_t            qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;

  /*-----------------------------------------------------------------------*/

  /* Validate qmux_client_id */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               qmi_qmux_if_client_data_list,
               (client == qmux_if_client_handle));

  if (client)
  {
    qmux_client_id = client->qmux_client_id;
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

  if (!client)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_send_to_qmux: Invalid qmux client ID=0x%x\n",
                   qmux_if_client_handle);
    return QMI_INTERNAL_ERR;
  }

  if (NULL != qmux_client_id_ptr)
  {
    *qmux_client_id_ptr = qmux_client_id;
  }

  /* Set up message for sending */
  memset(&hdr, 0, sizeof(qmi_qmux_if_msg_hdr_type));

  hdr.msg_id = msg_id;
  hdr.qmux_client_id = qmux_client_id;
  hdr.qmux_txn_id = qmux_txn_id;
  hdr.qmi_conn_id = qmi_conn_id;
  hdr.qmi_service_id = qmi_service_id;
  hdr.qmi_client_id = qmi_client_id;
  hdr.control_flags = 0; // Unused for TX to QMUX, only valid for RX

  /* Decrement msg pointer and increment msg_len */
  msg -= QMI_QMUX_IF_HDR_SIZE;
  msg_len += (int)QMI_QMUX_IF_HDR_SIZE;

  /* Copy header into message buffer */
  memcpy ((void *)msg, (void *)&hdr, QMI_QMUX_IF_HDR_SIZE);

  /* Lock TX mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_tx_mutex);

  #ifndef QMI_MSGLIB_MULTI_PD
    rc = qmi_qmux_tx_msg (qmux_client_id,msg,msg_len);
  #else
    rc = QMI_QMUX_IF_PLATFORM_TX_MSG( qmux_client_id,
                                      msg,
                                      msg_len );
  #endif

  /* UnLock TX mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_tx_mutex);

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
static int
qmi_qmux_if_cmp_txn
(
  qmi_txn_hdr_type  *txn_data,
  void              *cmp_data
)
{
  unsigned long txn_id = (unsigned long) cmp_data;
  qmi_qmux_if_txn_type  *txn = (qmi_qmux_if_txn_type *) txn_data;
  int rc = 0;

  if ((txn != NULL) && (txn_id != QMI_INVALID_TXN_ID))
  {
    if (txn->qmux_hdr.qmux_txn_id == txn_id)
    {
      rc = 1;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qmux_if_complete_sync_txns
===========================================================================*/
/*!
@brief
  Cleans up any pending sync messages at the IF layer and unblocks the
  thread waiting for the sync txn completion

@return
  None

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_if_complete_sync_txns
(
  qmi_connection_id_type  conn_id
)
{
  qmi_txn_hdr_type  *txn = NULL, *prev_txn = NULL, *head_txn = NULL;

  QMI_PLATFORM_MUTEX_LOCK(&qmi_qmux_if_txn_list_mutex);

  head_txn = qmi_qmux_if_txn_list;
  QMI_SLL_FIND(txn,prev_txn,head_txn,(((qmi_qmux_if_txn_type *)txn)->qmux_hdr.qmi_conn_id == conn_id));

  while (txn)
  {
    QMI_ERR_MSG_4("qmi_qmux_if_complete_sync_txns: completing txn conn_id=%d, qmux_client_id=0x%x, msg_id=0x%02x, txn=0x%x",
                  conn_id,
                  ((qmi_qmux_if_txn_type *)txn)->qmux_hdr.qmux_client_id,
                  ((qmi_qmux_if_txn_type *)txn)->qmux_hdr.msg_id,
                  ((qmi_qmux_if_txn_type *)txn)->qmux_hdr.qmux_txn_id);

    ((qmi_qmux_if_txn_type *)txn)->qmux_hdr.sys_err_code = QMI_TIMEOUT_ERR;

    QMI_DEBUG_MSG_0("qmi_qmux_if_complete_sync_txns: Sending signal ...... to read cmd data \n");

    /* Send signal to calling thread to unblock */
    QMI_PLATFORM_SEND_SIGNAL (((qmi_qmux_if_txn_type *)txn)->qmux_hdr.qmi_conn_id,
                              &(((qmi_qmux_if_txn_type *)txn)->signal_data));

    head_txn = txn->next;
    QMI_SLL_FIND(txn,prev_txn,head_txn,(((qmi_qmux_if_txn_type *)txn)->qmux_hdr.qmi_conn_id == conn_id));
  }

  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_txn_list_mutex);
}

/*===========================================================================
  FUNCTION  qmi_qmux_if_handle_sys_ind_msg
===========================================================================*/
/*!
@brief
  QMI QMUX system indication handler

@return
  None

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_if_handle_sys_ind_msg
(
  qmi_qmux_if_msg_hdr_type  *msg_hdr,
  unsigned char             *msg,
  int                       msg_len
)
{
  int qmi_err_code;
  qmi_sys_event_info_type   ind_data;
  qmi_sys_event_type        ind_event = QMI_SYS_EVENT_INVALID;
  qmi_qmux_if_cmd_rsp_type  cmd_data;
  qmi_qmux_if_client_data_type  tmp_client_data;
  qmi_qmux_if_client_data_type  *client, *prev;


  memset (&tmp_client_data, 0, sizeof (qmi_qmux_if_client_data_type));

  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               qmi_qmux_if_client_data_list,
               (client->qmux_client_id == msg_hdr->qmux_client_id));

  /* Save client data in local variable */
  if (client != NULL)
  {
    tmp_client_data = *client;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

  /* Make sure we have a non-NULL system event indication handler */
  if (!(client))
  {
    QMI_ERR_MSG_2 ("qmi_qmux_if_handle_sys_ind_msg: got indication msg_id=%d, "
                   "for an invalid qmux_client_id=0x%x\n",
                   msg_hdr->msg_id, msg_hdr->qmux_client_id);
    return;
  }

  /* Make sure we have at least a valid message header */
  if (msg_len < (int) sizeof (qmi_qmux_if_cmd_rsp_type))
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_handle_sys_ind_msg: Message too small, size is %d\n",msg_len);
    return;
  }

  memcpy (&cmd_data, msg, sizeof (qmi_qmux_if_cmd_rsp_type));
  memset (&ind_data, 0, sizeof(qmi_sys_event_info_type));

  if (msg_hdr->msg_id == QMI_QMUX_IF_PWR_STATE_IND_MSG_ID)
  {
    ind_event = QMI_SYS_EVENT_PWR_REPORT;
    ind_data.qmi_sys_event_pwr_report.curr_pwr_state_hndl = cmd_data.qmi_qmux_if_pwr_state_ind.curr_pwr_state_hndl;
    ind_data.qmi_sys_event_pwr_report.prev_pwr_state_hndl = cmd_data.qmi_qmux_if_pwr_state_ind.prev_pwr_state_hndl;
  }
  else if (msg_hdr->msg_id == QMI_QMUX_IF_SYNC_IND_MSG_ID)
  {
    ind_event = QMI_SYS_EVENT_SYNC_IND;
    ind_data.qmi_sync_ind.conn_id = (int) msg_hdr->qmi_conn_id;
  }
  else if (msg_hdr->msg_id == QMI_QMUX_IF_NEW_SRVC_AVAIL_MSG_ID)
  {
    /* New service indication will be sent to the clients */
    ind_event = QMI_SYS_EVENT_MODEM_NEW_SRVC_IND;
  }
  else if (msg_hdr->msg_id == QMI_QMUX_IF_MODEM_OUT_OF_SERVICE_MSG_ID)
  {
    ind_event = QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND;
    ind_data.qmi_modem_service_ind.conn_id = cmd_data.qmi_qmux_if_sub_sys_restart_ind.conn_id;
    ind_data.qmi_modem_service_ind.dev_id =
                      QMI_PLATFORM_CONN_ID_TO_DEV_NAME (cmd_data.qmi_qmux_if_sub_sys_restart_ind.conn_id);

    QMI_DEBUG_MSG_4 ("[%d] %x: MODEM_OUT_OF_SERVICE indication on conn_id=%d, dev_id=%s\n",
                     getpid(),
                     tmp_client_data.qmux_client_id,
                     ind_data.qmi_modem_service_ind.conn_id,
                     ind_data.qmi_modem_service_ind.dev_id);

    /* Reset the whole service cache, legacy targets do not support per-port SSR. */
    memset(&is_version_list_cached, 0, sizeof(is_version_list_cached));
    memset(&version_list_cache, 0, sizeof(version_list_cache));

    /* Clean up any pending sync qmi_qmux_if transactions on this connection */
    qmi_qmux_if_complete_sync_txns(ind_data.qmi_modem_service_ind.conn_id);
  }
  else if (msg_hdr->msg_id == QMI_QMUX_IF_MODEM_IN_SERVICE_MSG_ID)
  {
    ind_event = QMI_SYS_EVENT_MODEM_IN_SERVICE_IND;
    ind_data.qmi_modem_service_ind.conn_id = cmd_data.qmi_qmux_if_sub_sys_restart_ind.conn_id;
    ind_data.qmi_modem_service_ind.dev_id =
                      QMI_PLATFORM_CONN_ID_TO_DEV_NAME (cmd_data.qmi_qmux_if_sub_sys_restart_ind.conn_id);

    QMI_DEBUG_MSG_4 ("[%d] %x: MODEM_IN_SERVICE indication on conn_id=%d, dev_id=%s\n",
                     getpid(),
                     tmp_client_data.qmux_client_id,
                     ind_data.qmi_modem_service_ind.conn_id,
                     ind_data.qmi_modem_service_ind.dev_id);

    /* Update the QMUX client ID */
    if (TRUE == cmd_data.qmi_qmux_if_sub_sys_restart_ind.is_valid)
    {
      QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
      QMI_SLL_FIND(client,
                   prev,
                   qmi_qmux_if_client_data_list,
                   (client->qmux_client_id == msg_hdr->qmux_client_id));

      if (client != NULL)
      {
        QMI_DEBUG_MSG_2 ("qmi_qmux_if_handle_sys_ind_msg: updating qmux_client_id old=0x%x, new=0x%x",
                         msg_hdr->qmux_client_id,
                         cmd_data.qmi_qmux_if_sub_sys_restart_ind.qmux_client_id);

        client->qmux_client_id = cmd_data.qmi_qmux_if_sub_sys_restart_ind.qmux_client_id;
      }
      else
      {
        QMI_ERR_MSG_1 ("qmi_qmux_if_handle_sys_ind_msg: failed to find qmux_client_id=0x%x",
                       msg_hdr->qmux_client_id);
      }
      QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

      /* Add the client back to qmuxd */
      if (client != NULL)
      {
        memset(&cmd_data, 0, sizeof(qmi_qmux_if_cmd_rsp_type));
        cmd_data.qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id = client->qmux_client_id;
        cmd_data.qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_mode = client->qmux_client_mode;

        QMI_DEBUG_MSG_1("qmi_qmux_if_handle_sys_ind_msg: adding new qmux_client_id=0x%x to qmuxd",
                         client->qmux_client_id);

        if (qmi_qmux_if_send_if_msg_to_qmux(client,
                                            QMI_QMUX_IF_ADD_QMUX_CLIENT_MSG_ID,
                                            qmi_qmux_if_internal_use_conn_id,
                                            &cmd_data,
                                            &qmi_err_code,
                                            QMI_SYNC_MSG_DEFAULT_TIMEOUT) < 0)
        {
          QMI_ERR_MSG_2 ("qmi_qmux_if_handle_sys_ind_msg: Could not add qmux_client_id=0x%x to qmuxd [%d]",
                         client->qmux_client_id,
                         qmi_err_code);
        }
      }
    }
    else if(msg_hdr->msg_id == QMI_QMUX_IF_PORT_WRITE_FAIL_IND_MSG_ID)
    {
      ind_event = QMI_SYS_EVENT_PORT_WRITE_FAIL_IND;
      ind_data.qmi_sys_port_write_failed_ind.conn_id = cmd_data.qmi_qmux_if_port_write_failed_ind.conn_id;
      ind_data.qmi_sys_port_write_failed_ind.write_err_code = cmd_data.qmi_qmux_if_port_write_failed_ind.write_err_code;

      QMI_DEBUG_MSG_2("qmi_qmux_if_handle_sys_ind_msg: sending QMI_SYS_EVENT_PORT_WRITE_FAIL_IND on conn_id [%d] "
                      "to client_id [%d]",
                      ind_data.qmi_sys_port_write_failed_ind.conn_id,
                      client->qmux_client_id);
    }
    else
    {
      QMI_DEBUG_MSG_1 ("qmi_qmux_if_handle_sys_ind_msg: not updating qmux_client_id=0x%x",
                       msg_hdr->qmux_client_id);
    }
  }
  else
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_handle_sys_ind_msg: unknown msg_id %d", msg_hdr->msg_id);
    return;
  }

  /* Make system event callback */
  if (tmp_client_data.sys_event_rx_hdlr)
  {
    QMI_DEBUG_MSG_2 ("qmi_qmux_if_handle_sys_ind_msg: Sending system event indication %d for qmux_client_id=0x%x\n",
                     ind_event,
                     msg_hdr->qmux_client_id);

    tmp_client_data.sys_event_rx_hdlr (ind_event,
                                       &ind_data,
                                       tmp_client_data.sys_event_user_data);
  }
  else
  {
    QMI_ERR_MSG_2 ("qmi_qmux_if_handle_sys_ind_msg: got indication msg_id=%d, "
                   "but no handler registered for qmux_client_id=0x%x\n",
                   msg_hdr->msg_id, msg_hdr->qmux_client_id);
  }
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_unsol_release_service_client
===========================================================================*/
/*!
@brief
  This function is used to send an unsolicited service client release
  request to QMUXD

@return
  None

@note

  - Side Effects
    -

*/
/*=========================================================================*/
static
void qmi_qmux_if_unsol_release_service_client
(
  qmi_connection_id_type  qmi_conn_id,
  qmi_qmux_clnt_id_t      qmux_client_id,
  qmi_client_id_type      qmi_client_id,
  qmi_service_id_type     qmi_service_id
)
{
  qmi_qmux_if_cmd_rsp_type  cmd_data;
  qmi_qmux_if_msg_hdr_type  tx_hdr;
  unsigned char  tx_buf [QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];
  unsigned char  *msg = tx_buf;
  int rc = QMI_NO_ERR;


  QMI_DEBUG_MSG_4 ("qmi_qmux_if_unsol_release_service_client %x: releasing client=%d for service=%d on conn_id=%d\n",
                   qmux_client_id,
                   qmi_client_id,
                   qmi_service_id,
                   qmi_conn_id);

  /* Update the cmd_data to send */
  memset(&cmd_data, 0, sizeof(qmi_qmux_if_cmd_rsp_type));

  cmd_data.qmi_qmux_if_release_client_req.delete_client_id  = qmi_client_id;
  cmd_data.qmi_qmux_if_release_client_req.delete_service_id = qmi_service_id;

  /* Update the msg hdr */
  memset(&tx_hdr, 0, sizeof(qmi_qmux_if_msg_hdr_type));

  tx_hdr.msg_id         = QMI_QMUX_IF_RELEASE_QMI_CLIENT_ID_MSG_ID;
  tx_hdr.qmux_client_id = qmux_client_id;
  tx_hdr.qmux_txn_id    = QMI_INVALID_TXN_ID;
  tx_hdr.qmi_conn_id    = qmi_conn_id;
  tx_hdr.qmi_service_id = (qmi_service_id_type)0;  /* Not used */
  tx_hdr.qmi_client_id  = 0;                       /* Not used */
  tx_hdr.control_flags  = 0; // Unused for TX to QMUX, only valid for RX

  /* Copy header into message buffer */
  memcpy ((void *)msg, (void *)&tx_hdr, QMI_QMUX_IF_HDR_SIZE);

  msg += QMI_QMUX_IF_HDR_SIZE;

  /* Copy the cmd data */
  memcpy ((void *)msg, (void *)&cmd_data, sizeof(cmd_data));

  /* Lock TX mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_tx_mutex);

  #ifndef QMI_MSGLIB_MULTI_PD
    rc = qmi_qmux_tx_msg (qmux_client_id, tx_buf, sizeof(tx_buf));
  #else
    rc = QMI_QMUX_IF_PLATFORM_TX_MSG( qmux_client_id,
                                      tx_buf,
                                      sizeof(tx_buf) );
  #endif

  /* UnLock TX mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_tx_mutex);

  if (QMI_NO_ERR != rc)
  {
    QMI_ERR_MSG_0 ("qmi_qmux_if_unsol_release_service_client: failed to TX msg to QMUXD\n");
  }
}

/*===========================================================================
  FUNCTION  qmi_qmux_if_rx_hdlr
===========================================================================*/
/*!
@brief
  QMI QMUX service RX callback handler.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
void
qmi_qmux_if_rx_msg
(
  unsigned char *msg,
  int            msg_len
)
{
  qmi_qmux_if_msg_hdr_type msg_hdr;
  qmi_qmux_if_txn_type     *txn;
  int                      rc = QMI_NO_ERR;
  qmi_qmux_if_client_data_type  tmp_client_data;
  qmi_qmux_if_client_data_type  *client, *prev;

  /* validate input parameter */
  if (NULL == msg)
  {
    QMI_DEBUG_MSG_0 ("qmi_qmux_if_rx_msg: input msg is NULL");
    return;
  }

  /* Make sure we have at least a valid message header */
  if (msg_len < (int) sizeof (qmi_qmux_if_msg_hdr_type))
  {
    QMI_DEBUG_MSG_1 ("qmi_qmux_if_rx_msg: Message too small, size is %d\n",msg_len);
    return;
  }

  /* We copy the message header into a structure because of possible  alignment
  ** issues.  If platform layer is not doing word alignment, then this could cause
  ** problems.  From this point on, packet is treated as byte array so no issues
  */
  memcpy(&msg_hdr,msg,QMI_QMUX_IF_HDR_SIZE);

  /* Advance msg pointer and decrement msg_len */
  msg += QMI_QMUX_IF_HDR_SIZE;
  msg_len -= (int)QMI_QMUX_IF_HDR_SIZE;

  if (msg_hdr.msg_id == QMI_QMUX_IF_QMI_MSG_ID)
  {
    /* Find client that message belongs to, and call appropriate callback */
    memset (&tmp_client_data, 0, sizeof (qmi_qmux_if_client_data_type));

    QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
    QMI_SLL_FIND(client,
                 prev,
                 qmi_qmux_if_client_data_list,
                 (client->qmux_client_id == msg_hdr.qmux_client_id));
    /* Save client data in local variable */
    if (client != NULL)
    {
      tmp_client_data = *client;
    }
    QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);


    if (client && (tmp_client_data.rx_msg_hdlr))
    {
      tmp_client_data.rx_msg_hdlr ( msg_hdr.qmi_conn_id,
                                    msg_hdr.qmi_service_id,
                                    msg_hdr.qmi_client_id,
                                    msg_hdr.control_flags,
                                    msg,
                                    msg_len);
    }
    else
    {
      QMI_ERR_MSG_1 ("qmi_qmux_if_rx_msg:  No RX msg handler found for qmux_client_id = %d\n",
                     msg_hdr.qmux_client_id);
    }
  }
  /* Handle any CTL or QMUX IF broadcast indications that may occur */
  else if (msg_hdr.msg_id >= QMI_QMUX_IF_FIRST_SYS_IND_MSG_ID &&
           msg_hdr.msg_id <= QMI_QMUX_IF_LAST_SYS_IND_MSG_ID)
  {
    qmi_qmux_if_handle_sys_ind_msg (&msg_hdr, msg, msg_len);
  }
  else
  {
    /* Find associated TXN */
    txn = (qmi_qmux_if_txn_type *)
        qmi_util_find_and_addref_txn ((void *) msg_hdr.qmux_txn_id,
                                      qmi_qmux_if_cmp_txn,
                                      &qmi_qmux_if_txn_list,
                                      &qmi_qmux_if_txn_list_mutex);

    /* Make sure we found the associated transaction */
    if (txn == NULL)
    {
      QMI_ERR_MSG_1 ("qmi_qmux_if_rx_msg: Unable to find TXN ID = %d\n",
                     (int)msg_hdr.qmux_txn_id);

      /* If the response is for a service client alloc request (that timed out),
         release the corresponding service client */
      if (QMI_QMUX_IF_ALLOC_QMI_CLIENT_ID_MSG_ID == msg_hdr.msg_id)
      {
        qmi_qmux_if_cmd_rsp_type  *alloc_rsp = (qmi_qmux_if_cmd_rsp_type *)msg;

        QMI_ERR_MSG_2 ("qmi_qmux_if_rx_msg: Received client_id=%d for service=%d\n",
                       alloc_rsp->qmi_qmux_if_alloc_client_rsp.new_client_id,
                       alloc_rsp->qmi_qmux_if_alloc_client_rsp.service_id);

        qmi_qmux_if_unsol_release_service_client(msg_hdr.qmi_conn_id,
                                                 msg_hdr.qmux_client_id,
                                                 alloc_rsp->qmi_qmux_if_alloc_client_rsp.new_client_id,
                                                 alloc_rsp->qmi_qmux_if_alloc_client_rsp.service_id);
      }
      return;
    }

    /* Copy QMUX IF header into transaction */
    memcpy (&txn->qmux_hdr, &msg_hdr, sizeof (qmi_qmux_if_msg_hdr_type));

    /* Make sure we got cmd_data back */
    if (msg_len < (int) sizeof (qmi_qmux_if_cmd_rsp_type))
    {
      QMI_ERR_MSG_1 ("qmi_qmux_if_rx_msg: QMUX IF cmd_data too short size = %d\n", msg_len);
      txn->qmux_hdr.sys_err_code = QMI_INTERNAL_ERR;
    }
    else
    {
      /* copy response data into transaction */
      memcpy (&txn->cmd_data, msg, sizeof (qmi_qmux_if_cmd_rsp_type));
    }


    QMI_DEBUG_MSG_0("Sending signal ...... to read cmd data \n");
    /* Send signal to calling thread to unblock */
    QMI_PLATFORM_SEND_SIGNAL (txn->qmux_hdr.qmi_conn_id,
                              &txn->signal_data);

    /* Release but don't delete transaction.  Transactions will be deleted by calling
    ** context since all QMUX IF transactions are synchronous
    */
    qmi_util_release_txn ((qmi_txn_hdr_type *)txn,
                           FALSE,
                           &qmi_qmux_if_txn_list,
                           &qmi_qmux_if_txn_list_mutex);
  }
}


static void
qmi_qmux_if_txn_delete
(
  void *del_txn
)
{
  qmi_qmux_if_txn_type *txn = (qmi_qmux_if_txn_type *) del_txn;
  QMI_PLATFORM_DESTROY_SIGNAL_DATA (&txn->signal_data);
}

int
qmi_qmux_if_send_if_msg_to_qmux
(
  qmi_qmux_if_hndl_t        qmux_if_client_handle,
  qmi_qmux_if_msg_id_type   msg_id,
  qmi_connection_id_type    conn_id,
  qmi_qmux_if_cmd_rsp_type  *cmd_data,
  int                       *qmi_err_code,
  unsigned int              timeout
)
{
  qmi_qmux_if_txn_type      *txn;
  int                       rc;
  unsigned char             tx_buf [QMI_QMUX_IF_MSG_HDR_SIZE + sizeof (qmi_qmux_if_cmd_rsp_type)];

  /* Initialize QMI error code */
  if (qmi_err_code)
  {
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
  }

  /* Make sure there is cmd_data */
  if (cmd_data == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_qmux_if_send_if_msg_to_qmux: NULL cmd_data invalid\n");
    return QMI_INTERNAL_ERR;
  }

  /* Allocate a transaction structure */

  txn = (qmi_qmux_if_txn_type *)
        qmi_util_alloc_and_addref_txn (sizeof (qmi_qmux_if_txn_type),
                                       qmi_qmux_if_txn_delete,
                                       &qmi_qmux_if_txn_list,
                                       &qmi_qmux_if_txn_list_mutex);


  /* Make sure we found the associated transaction */
  if (txn == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_qmux_if_send_if_msg_to_qmux: Unable to alloc TXN\n");
    return QMI_INTERNAL_ERR;
  }

  /* Allocate a transaction ID.... (use txn_list mutex.. no point
  ** in separate mutex just for this)
  */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_txn_list_mutex);
  if (++qmi_qmux_if_qmux_txn_id == QMI_INVALID_TXN_ID)
  {
    ++qmi_qmux_if_qmux_txn_id;
  }
  txn->qmux_hdr.qmux_txn_id = qmi_qmux_if_qmux_txn_id;
  txn->qmux_hdr.qmi_conn_id = conn_id;
  txn->qmux_hdr.msg_id = msg_id;

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_txn_list_mutex);

  QMI_PLATFORM_INIT_SIGNAL_FOR_WAIT( conn_id,
                                     &txn->signal_data );

  /* Set up tx_msg buffer by copying command data to it */
  memcpy ((unsigned char *)(tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
          cmd_data,
          sizeof (qmi_qmux_if_cmd_rsp_type));

  rc = qmi_qmux_if_send_to_qmux (qmux_if_client_handle,
                                 msg_id,
                                 txn->qmux_hdr.qmux_txn_id,
                                 conn_id,
                                 (qmi_service_id_type)0,   /* Not used */
                                 0,   /* Not used */
                                 (unsigned char *)(tx_buf + QMI_QMUX_IF_MSG_HDR_SIZE),
                                 sizeof (qmi_qmux_if_cmd_rsp_type),
                                 &txn->qmux_hdr.qmux_client_id);

  if (rc == QMI_NO_ERR)
  {
    /* Blocks waiting for signal from QMUX */
    /* If we get a timeout, indicate so accordingly */
    if ( QMI_PLATFORM_WAIT_FOR_SIGNAL( conn_id,
                                       &txn->signal_data,
                                       (int)timeout * 1000 ) == QMI_TIMEOUT_ERR )
    {
      QMI_DEBUG_MSG_0("Timeout error.............\n");
      QMI_DEBUG_MSG_4("conn_id=%d, qmux_client_id=0x%x, msg=0x%02x, txid=0x%x",
                      conn_id, txn->qmux_hdr.qmux_client_id, msg_id, txn->qmux_hdr.qmux_txn_id);
      rc = QMI_TIMEOUT_ERR;
      if (qmi_err_code)
      {
         QMI_DEBUG_MSG_1("qmi error code.........:%d\n",*qmi_err_code);
        *qmi_err_code = txn->qmux_hdr.qmi_err_code;
      }
    }
    else
    {
      QMI_DEBUG_MSG_4("conn_id=%d, qmux_client_id=0x%x, msg=0x%02x, txid=0x%x",
                      conn_id, txn->qmux_hdr.qmux_client_id, msg_id, txn->qmux_hdr.qmux_txn_id);

      if (qmi_err_code)
      {
        *qmi_err_code = txn->qmux_hdr.qmi_err_code;
        QMI_DEBUG_MSG_1("qmi error code.........:%d\n",*qmi_err_code);
      }
      rc = txn->qmux_hdr.sys_err_code;
      QMI_DEBUG_MSG_1("qmi sys error code.........:%d\n",rc);
      memcpy (cmd_data, &txn->cmd_data, sizeof (qmi_qmux_if_cmd_rsp_type));
    }
  }
  else
  {
    QMI_PLATFORM_MUTEX_UNLOCK (&txn->signal_data.cond_mutex);
  }

  /* Release and delete the transaction */
  qmi_util_release_txn ((qmi_txn_hdr_type *)txn,
                         TRUE,
                         &qmi_qmux_if_txn_list,
                         &qmi_qmux_if_txn_list_mutex);

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_alloc_service_client
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to add a service client

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_alloc_service_client
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      *client_id,
  int                     *qmi_err_code
)
{
  int rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  /* Set the cmd_data */
  cmd_data.qmi_qmux_if_alloc_client_req.service_id = service_id;

  /* Using extended timeout here as some transports can open yet hold message for a bit */
  rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                        QMI_QMUX_IF_ALLOC_QMI_CLIENT_ID_MSG_ID,
                                        conn_id,
                                        &cmd_data,
                                        qmi_err_code,
                                        QMI_SYNC_MSG_EXTENDED_TIMEOUT);

  if (rc == QMI_NO_ERR)
  {
    *client_id = cmd_data.qmi_qmux_if_alloc_client_rsp.new_client_id;
  }

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_release_service_client
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to remove a service client

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_release_service_client
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  int                     *qmi_err_code
)
{
  int rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  cmd_data.qmi_qmux_if_release_client_req.delete_client_id = client_id;
  cmd_data.qmi_qmux_if_release_client_req.delete_service_id = service_id;

  rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                        QMI_QMUX_IF_RELEASE_QMI_CLIENT_ID_MSG_ID,
                                        conn_id,
                                        &cmd_data,
                                        qmi_err_code,
                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT);

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_set_data_format
===========================================================================*/
/*!
@brief
  Set QMI data port format

@return
  TRUE if connection is active, FALSE otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_set_data_format
(
  qmi_qmux_if_hndl_t                    qmux_if_client_handle,
  qmi_connection_id_type                conn_id,
  qmi_data_format_qos_hdr_state_type    qos_hdr_state,
  qmi_link_layer_protocol_type          *link_protocol,
  int                                   *qmi_err_code
)
{
  int rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  cmd_data.qmi_qmux_if_set_data_format_req.link_protocol = *link_protocol;
  cmd_data.qmi_qmux_if_set_data_format_req.qos_hdr_state = qos_hdr_state;

  rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                        QMI_QMUX_IF_SET_DATA_FORMAT_MSG_ID,
                                        conn_id,
                                        &cmd_data,
                                        qmi_err_code,
                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT);

  /* Set return link protocol */
  *link_protocol = cmd_data.qmi_qmux_if_set_data_format_rsp.link_protocol;

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qmux_if_reg_pwr_save_mode
===========================================================================*/
/*!
@brief
  Set QMI data port format

@return
  TRUE if connection is active, FALSE otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_reg_pwr_save_mode
(
  qmi_qmux_if_hndl_t                    qmux_if_client_handle,
  qmi_pwr_report_type                   report_state,
  int                                   *qmi_err_code
)
{
  int rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  cmd_data.qmi_qmux_if_reg_pwr_save_mode_req.report_state = report_state;

  rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                        QMI_QMUX_IF_REG_PWR_SAVE_MODE_MSG_ID,
                                        qmi_qmux_if_internal_use_conn_id,
                                        &cmd_data,
                                        qmi_err_code,
                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_qmi_config_pwr_save_settings
===========================================================================*/
/*!
@brief
  Configures the power state indication filter for each connection.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_config_pwr_save_settings
(
  qmi_qmux_if_hndl_t   qmux_if_client_handle,
  int                  pwr_state_hndl,
  qmi_service_id_type  service_id,
  int                  num_indication_ids,
  unsigned short       indication_ids[],
  int                  *qmi_err_code
)
{
  int rc, i, tmp_rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;
  qmi_connection_id_type    conn_id;

  rc = QMI_NO_ERR;
  if (num_indication_ids > QMI_MAX_PWR_INDICATIONS)
  {
    QMI_ERR_MSG_2 ("Too many indication ID's configured: num=%d, max=%d\n",
                   num_indication_ids,
                   (int)QMI_MAX_PWR_INDICATIONS);

    return QMI_INTERNAL_ERR;
  }

  /* Make best attempt to change state for service on all ports */
  for (conn_id = QMI_CONN_ID_FIRST; conn_id <=QMI_CONN_ID_MAX_NON_BCAST; conn_id++)
  {
    if( FALSE == qmi_qmux_if_is_conn_active( qmux_if_client_handle, conn_id ))
    {
      continue;
    }

    cmd_data.qmi_qmux_if_config_pwr_save_settings_req.pwr_state_hndl = pwr_state_hndl;
    cmd_data.qmi_qmux_if_config_pwr_save_settings_req.service_id = service_id;
    cmd_data.qmi_qmux_if_config_pwr_save_settings_req.num_indication_ids = num_indication_ids;

    for (i=0; i < num_indication_ids; i++)
    {
      cmd_data.qmi_qmux_if_config_pwr_save_settings_req.indication_ids[i] = indication_ids[i];
    }

    tmp_rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                              QMI_QMUX_IF_CONFIG_PWR_SAVE_SETTINGS_MSG_ID,
                                              conn_id,
                                              &cmd_data,
                                              qmi_err_code,
                                              QMI_SYNC_MSG_DEFAULT_TIMEOUT);
    /* We may get QMI_INTERNAL_ERR
    ** from qmux if the port isn't opened.... we should just ignore this and keep going
    */
    if ((tmp_rc == QMI_INTERNAL_ERR) || (tmp_rc == QMI_NO_ERR))
    {
      continue;
    }
    else  /* More serious error.  Stop processing */
    {
      rc = tmp_rc;
      break;
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_qmi_set_pwr_state
===========================================================================*/
/*!
@brief
  Sets power state for each connection.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    - Talks to modem processor
    - Modem will not send filtered indications until later power state change.

*/
/*=========================================================================*/
int
qmi_qmux_if_set_pwr_state
(
  qmi_qmux_if_hndl_t   qmux_if_client_handle,
  unsigned long        pwr_state,
  int                  *qmi_err_code
)
{
  int rc, tmp_rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;
  qmi_connection_id_type    conn_id;

  rc = QMI_NO_ERR;

  /* Make best attempt to change state on all ports */
  for (conn_id = QMI_CONN_ID_FIRST; conn_id <=QMI_CONN_ID_MAX_NON_BCAST; conn_id++)
  {
    if( FALSE == qmi_qmux_if_is_conn_active( qmux_if_client_handle, conn_id ))
    {
      continue;
    }

    cmd_data.qmi_qmux_if_set_pwr_save_mode_req.new_pwr_state = pwr_state;
    tmp_rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                              QMI_QMUX_IF_SET_PWR_STATE_MSG_ID,
                                              conn_id,
                                              &cmd_data,
                                              qmi_err_code,
                                              QMI_SYNC_MSG_DEFAULT_TIMEOUT);

    /* We may get QMI_INTERNAL_ERR
    ** from qmux if the port isn't opened.... we should just ignore this and keep going
    */
    if ((tmp_rc == QMI_INTERNAL_ERR) || (tmp_rc == QMI_NO_ERR))
    {
      continue;
    }
    else  /* More serious error.  Stop processing */
    {
      rc = tmp_rc;
      break;
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_qmi_get_pwr_state
===========================================================================*/
/*!
@brief
  Gets power state for specified connection.

@return
  0 if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_get_pwr_state
(
  qmi_qmux_if_hndl_t       qmux_if_client_handle,
  qmi_connection_id_type   conn_id,
  unsigned long            *pwr_state,
  int                      *qmi_err_code
)
{
  int rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                        QMI_QMUX_IF_GET_PWR_STATE_MSG_ID,
                                        conn_id,
                                        &cmd_data,
                                        qmi_err_code,
                                        QMI_SYNC_MSG_DEFAULT_TIMEOUT);

  *pwr_state = cmd_data.qmi_qmux_if_get_pwr_save_mode_rsp.curr_pwr_state;

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_is_conn_active
===========================================================================*/
/*!
@brief
  Calls the QMUX layer to query the active state of a connection

@return
  TRUE if connection is active, FALSE otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_is_conn_active
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id
)
{
  (void)qmux_if_client_handle;
  (void) conn_id;

#ifndef QMI_MSGLIB_MULTI_PD
  return qmi_qmux_is_connection_active (conn_id);
#else
  return TRUE;
#endif

}


/*===========================================================================
  FUNCTION  qmi_qmux_if_send_msg
===========================================================================*/
/*!
@brief
  Sends a QMI message for the connection/service/client specified

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_send_qmi_msg
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  qmi_client_id_type      client_id,
  unsigned char           *msg_buf,
  int                     msg_buf_size
)
{
  int rc;
  rc = qmi_qmux_if_send_to_qmux (qmux_if_client_handle,
                                 QMI_QMUX_IF_QMI_MSG_ID,
                                 QMI_INVALID_TXN_ID,
                                 conn_id,
                                 service_id,
                                 client_id,
                                 msg_buf,
                                 msg_buf_size,
                                 NULL);
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_qmux_if_pwr_up_init_ex
===========================================================================*/
/*!
@brief
  Starts up a new "QMUX" client.  There can be multiple QMUX clients per PD.
  If the mode parameter is set to QMI_QMUX_IF_CLNT_MODE_RAW then all the CTL
  responses and indications will be received raw (QMUX SDU) via the
  rx_msg_hdlr.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

  Upon successful return, qmi_qmux_handle parameter will contain a handle
  that will need to be retained and passed into the qmi_qmux_if_pwr_down_release
  function.
@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_pwr_up_init_ex
(
  qmi_qmux_if_rx_msg_hdlr_type  rx_msg_hdlr,
  qmi_sys_event_rx_hdlr         sys_event_rx_hdlr,
  void                          *sys_event_user_data,
  qmi_qmux_if_hndl_t            *qmi_qmux_handle,
  qmi_qmux_if_clnt_mode_t       mode
)
{
  int rc, qmi_err_code;
  qmi_qmux_if_cmd_rsp_type      cmd_data;
  qmi_qmux_if_client_data_type  *qmi_qmux_if_client_data;
  qmi_qmux_clnt_id_t            qmi_qmux_if_qmux_client_id;
  unsigned char                 *qmi_qmux_if_rx_buf;


  if (!qmi_qmux_handle)
  {
    QMI_ERR_MSG_0 ("Invalid input handle\n");
    return QMI_INTERNAL_ERR;
  }

  *qmi_qmux_handle = QMI_QMUX_IF_INVALID_HNDL;

  QMI_PLATFORM_MUTEX_LOCK(&qmi_qmux_if_mutex);
  /* Initialize internal use port ID if specified by platform */
#ifdef QMI_PLATFORM_INTERNAL_USE_PORT_ID
  qmi_qmux_if_internal_use_conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID (QMI_PLATFORM_INTERNAL_USE_PORT_ID);

  if (qmi_qmux_if_internal_use_conn_id == QMI_CONN_ID_INVALID)
  {
    QMI_ERR_MSG_1 ("Unable to initialize internal use conn_id, dev_name=%s\n",QMI_PLATFORM_INTERNAL_USE_PORT_ID);
    QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
    return QMI_INTERNAL_ERR;
  }
#endif

  /* Highjack client list mutex to do initialization synchronization */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);

  if (qmi_qmux_if_is_initialized == FALSE)
  {
    QMI_PLATFORM_MUTEX_INIT (&qmi_qmux_if_tx_mutex);
    QMI_PLATFORM_MUTEX_INIT (&qmi_qmux_if_txn_list_mutex);

    /* Initialize the cache */
    memset(&is_version_list_cached, 0, sizeof(is_version_list_cached));
    memset(&version_list_cache, 0, sizeof(version_list_cache));

#ifndef QMI_MSGLIB_MULTI_PD
    /* Call qmux powerup init function */
    rc = qmi_qmux_pwr_up_init ();
     /* Initialize the qmux client ID... just make it 1 */
    qmi_qmux_if_qmux_client_id = 1;

#endif

    qmi_qmux_if_is_initialized = TRUE;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

  /* Do initialization with platform layer if multi-PD */
#ifdef QMI_MSGLIB_MULTI_PD
  qmi_qmux_if_rx_buf = malloc (QMI_QMUX_IF_BUF_SIZE);
  if (!qmi_qmux_if_rx_buf)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_pwr_up_init_ex: Unable to allocate dynamic memory for RX buf, sz = %d\n",QMI_QMUX_IF_BUF_SIZE);
    QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
    return QMI_INTERNAL_ERR;
  }

  rc = QMI_QMUX_IF_PLATFORM_CLIENT_INIT (&qmi_qmux_if_qmux_client_id,
                                         qmi_qmux_if_rx_buf,
                                         (int)QMI_QMUX_IF_BUF_SIZE);
#endif

  /* Make sure initialization went OK... */
  if (rc != QMI_NO_ERR)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_pwr_up_init_ex:  Initialization failed, rc = %d\n",rc);
    free (qmi_qmux_if_rx_buf);
    QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
    return rc;
  }

  qmi_qmux_if_client_data = (qmi_qmux_if_client_data_type *) malloc (sizeof (qmi_qmux_if_client_data_type));

  /* Make sure memory allocation succeeds */
  if (!qmi_qmux_if_client_data)
  {
    QMI_DEBUG_MSG_0 ("qmi_qmux_if_pwr_up_init_ex:  Malloc failed, returning error\n");
    QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
    return QMI_INTERNAL_ERR;
  }

  /* Save client data */
  qmi_qmux_if_client_data->qmux_client_id = qmi_qmux_if_qmux_client_id;
  qmi_qmux_if_client_data->rx_msg_hdlr = rx_msg_hdlr;
  qmi_qmux_if_client_data->sys_event_rx_hdlr = sys_event_rx_hdlr;
  qmi_qmux_if_client_data->sys_event_user_data = sys_event_user_data;
  qmi_qmux_if_client_data->rx_msg_buf = qmi_qmux_if_rx_buf;
  qmi_qmux_if_client_data->qmux_client_mode = mode;

  /* Lock list mutex, add new client data, and then unlock */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_ADD (qmi_qmux_if_client_data,qmi_qmux_if_client_data_list);
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

  /* Tell QMUX about new qmux_client... always do this last to avoid possible race conditions */
  cmd_data.qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id = qmi_qmux_if_qmux_client_id;
  cmd_data.qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_mode = mode;

  rc = qmi_qmux_if_send_if_msg_to_qmux (qmi_qmux_if_client_data,
                                        QMI_QMUX_IF_ADD_QMUX_CLIENT_MSG_ID,
                                        qmi_qmux_if_internal_use_conn_id,
                                        &cmd_data,
                                        &qmi_err_code,
                                        QMI_SYNC_MSG_EXTENDED_TIMEOUT);

  if (rc < 0)
  {
    QMI_ERR_MSG_3 ("Addition of QMUX client %d returns err %d, qmi_err_code %d\n",
                                       qmi_qmux_if_qmux_client_id,rc,qmi_err_code);
  }
  else
  {
    *qmi_qmux_handle = qmi_qmux_if_client_data;
    rc = QMI_NO_ERR;
    QMI_DEBUG_MSG_1 ("qmi_qmux_if_pwr_up_init_ex:  Successfully created and added QMUX client %d\n",
                     qmi_qmux_if_qmux_client_id);
  }

  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_pwr_up_init
===========================================================================*/
/*!
@brief
  Starts up a new "QMUX" client.  There can be multiple QMUX clients per PD.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

  Upon successful return, qmi_qmux_handle parameter will contain a handle
  that will need to be retained and passed into the qmi_qmux_if_pwr_down_release
  function.

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_pwr_up_init
(
  qmi_qmux_if_rx_msg_hdlr_type  rx_msg_hdlr,
  qmi_sys_event_rx_hdlr         sys_event_rx_hdlr,
  void                          *sys_event_user_data,
  qmi_qmux_if_hndl_t            *qmi_qmux_handle
)
{
  return qmi_qmux_if_pwr_up_init_ex(rx_msg_hdlr,
                                    sys_event_rx_hdlr,
                                    sys_event_user_data,
                                    qmi_qmux_handle,
                                    QMI_QMUX_IF_CLNT_MODE_NORMAL);
}



/*===========================================================================
  FUNCTION  qmi_qmux_if_pwr_down_release
===========================================================================*/
/*!
@brief
  Called on client shutdown. Handle returned by qmi_qmux_handle parameter
  of qmi_qmux_if_pwr_up_init function call.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_pwr_down_release
(
  qmi_qmux_if_hndl_t  qmux_if_client_handle
)
{
  int rc, qmi_err_code;
  qmi_qmux_if_cmd_rsp_type      cmd_data;
  qmi_qmux_if_client_data_type  *item = NULL;
  qmi_qmux_if_client_data_type  *prev = NULL;
  qmi_qmux_clnt_id_t            qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;


  if (!qmux_if_client_handle)
  {
    QMI_ERR_MSG_0 ("qmi_qmux_if_pwr_down_release: Invalid client handle\n");
    return QMI_INTERNAL_ERR;
  }

  QMI_PLATFORM_MUTEX_LOCK(&qmi_qmux_if_mutex);
  /* Remove client data from qmi_qmux_if list */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND (item,
                prev,
                qmi_qmux_if_client_data_list,
                (item == qmux_if_client_handle));
  if (item)
  {
    qmux_client_id = item->qmux_client_id;
  }
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);


  /* Make sure that the item to be removed was found */
  if (!item)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_pwr_down_release: qmux_if_client_handle 0x%x not found in list\n",
                   qmux_if_client_handle);
    QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
    return QMI_INTERNAL_ERR;
  }

  /* Send message to qmux to delete the QMUX client ID */
  cmd_data.qmi_qmux_add_delete_qmux_client_req_rsp.qmux_client_id = qmux_client_id;

  if ((rc = qmi_qmux_if_send_if_msg_to_qmux (qmux_if_client_handle,
                                             QMI_QMUX_IF_DELETE_QMUX_CLIENT_MSG_ID,
                                             qmi_qmux_if_internal_use_conn_id,
                                             &cmd_data,
                                             &qmi_err_code,
                                             QMI_SYNC_MSG_DEFAULT_TIMEOUT)) != QMI_NO_ERR)
  {
    QMI_ERR_MSG_2 ("qmi_qmux_if_pwr_down_release: Unable to remove qmux client ID [%d] %x from qmux\n",
                   getpid(),
                   qmux_client_id);
  }

  QMI_DEBUG_MSG_2 ("qmi_qmux_if_pwr_down_release: Released QMUX client handle [%d] %x\n",
                   getpid(),
                   qmux_client_id);

  /* Remove client data from qmi_qmux_if list */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND_AND_REMOVE (item,
                           prev,
                           qmi_qmux_if_client_data_list,
                           (item == qmux_if_client_handle));
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

#ifndef QMI_MSGLIB_MULTI_PD
  rc = QMI_NO_ERR;
#else
  rc = QMI_QMUX_IF_PLATFORM_CLIENT_RELEASE (qmux_client_id);
#endif

  if (rc != QMI_NO_ERR)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_pwr_down_release: PLATFORM client release fails, rc = %d\n",rc);
  }

  /* Free client data */
  free (item->rx_msg_buf);
  free (item);

  /* If there is no longer any client data in list, then this was the
  ** last qmi_release() call... do library cleanup
  */
  if (qmi_qmux_if_client_data_list == NULL)
  {
    QMI_DEBUG_MSG_0 ("qmi_qmux_if_pwr_down_release: Last client releases, performing de-init\n");
    QMI_PLATFORM_MUTEX_DESTROY (&qmi_qmux_if_tx_mutex);
    QMI_PLATFORM_MUTEX_DESTROY (&qmi_qmux_if_txn_list_mutex);
    qmi_qmux_if_is_initialized = FALSE;
  }
  else
  {
    QMI_DEBUG_MSG_0 ("qmi_qmux_if_pwr_down_release: More clients in list, no de-init performed\n");
  }

  QMI_PLATFORM_MUTEX_UNLOCK(&qmi_qmux_if_mutex);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_open_connection
===========================================================================*/
/*!
@brief
  Opens up a new connection for the QMUX client

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_open_connection
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id
)
{
  (void)qmux_if_client_handle;
  (void)conn_id;

#ifndef QMI_MSGLIB_MULTI_PD

  return qmi_qmux_open_connection (conn_id);
#else
  return QMI_NO_ERR;
#endif
}


/*===========================================================================
  FUNCTION  qmi_qmux_if_qmi_proxy_send_to_qmux
===========================================================================*/
/*!
    @brief
    Send QMI Proxy message to QMUX.

    @return
    None
*/
/*=========================================================================*/
int qmi_qmux_if_qmi_proxy_send_to_qmux
(
  qmi_qmux_if_hndl_t       qmux_if_client_handle,
  qmi_qmux_if_msg_id_type  msg_id,
  unsigned long            qmux_txn_id,
  qmi_connection_id_type   qmi_conn_id,
  qmi_service_id_type      qmi_service_id,
  qmi_client_id_type       qmi_client_id,
  unsigned char            control_flags,
  int                      sys_err_code,
  int                      qmi_err_code,
  unsigned char            *msg,
  int                      msg_len
)
{
  qmi_qmux_if_msg_hdr_type hdr;
  int rc;
  qmi_qmux_if_client_data_type  *client = NULL, *prev = NULL;
  qmi_qmux_clnt_id_t            qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;

  /*-----------------------------------------------------------------------*/

  /* Validate qmux_client_id */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               qmi_qmux_if_client_data_list,
               (client == qmux_if_client_handle));

  if (client)
  {
    qmux_client_id = client->qmux_client_id;
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

  if (!client)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_send_to_qmux: Invalid qmux client ID=0x%x\n",
                   qmux_if_client_handle);
    return QMI_INTERNAL_ERR;
  }

  memset( &hdr, 0, sizeof(qmi_qmux_if_msg_hdr_type) );

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
  memcpy( (void *)msg, (void *)&hdr, QMI_QMUX_IF_HDR_SIZE );

  /* Lock TX mutex */
  QMI_PLATFORM_MUTEX_LOCK( &qmi_qmux_if_tx_mutex );

  #ifndef QMI_MSGLIB_MULTI_PD
  rc = qmi_qmux_tx_msg( qmux_client_id, msg, msg_len );
  #else
  rc = QMI_QMUX_IF_PLATFORM_TX_MSG( qmux_client_id,
                                    msg,
                                    msg_len );
  #endif

  /* UnLock TX mutex */
  QMI_PLATFORM_MUTEX_UNLOCK( &qmi_qmux_if_tx_mutex );

  return rc;

} /* qmi_qmux_if_qmi_proxy_send_to_qmux */


/*===========================================================================
  FUNCTION  qmi_qmux_if_get_version_list
===========================================================================*/
/*!
@brief
  Query modem for QMI service version list, returning specified
  service information.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    -

*/
/*=========================================================================*/
int
qmi_qmux_if_get_version_list
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  qmi_service_id_type     service_id,
  unsigned short          *major_ver,  /*  Major version number */
  unsigned short          *minor_ver,  /*  Minor version number */
  int                     *qmi_err_code
)
{
  int rc,index,service_id_index = -1;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  /* Initialize QMI error code */
  if (qmi_err_code)
  {
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
  }

  if (conn_id >= QMI_MAX_CONN_IDS)
  {
    QMI_DEBUG_MSG_1("Invalid conn_id:%d\n",conn_id);
    return QMI_INTERNAL_ERR;
  }

  if (is_version_list_cached[conn_id])
  {
    for (index = 0; index < version_list_cache[conn_id].qmi_service_version_len; index++)
    {
      if (version_list_cache[conn_id].qmi_service_version[index].qmi_svc_type == service_id)
      {
        service_id_index = index;
        break;
      }
    }

    if (service_id_index >= 0)
    {
      *major_ver = version_list_cache[conn_id].qmi_service_version[service_id_index].major_ver;
      *minor_ver = version_list_cache[conn_id].qmi_service_version[service_id_index].minor_ver;
      QMI_DEBUG_MSG_4("Getting service version from cache: Conn_id %d, Service %d, Major version %d, Minor version %d..........\n",
          conn_id, service_id, *major_ver, *minor_ver);
      return QMI_NO_ERR;
    }
  }

  /* If service not found, send request to QMUX */
  memset(&cmd_data,0,sizeof (qmi_qmux_if_cmd_rsp_type));
  QMI_DEBUG_MSG_0(" Sending message control to modem for getting version information..........\n");

  /* Using extended timeout has some transports open yet hold message for a bit */
  rc = qmi_qmux_if_send_if_msg_to_qmux(qmux_if_client_handle,
                                       QMI_QMUX_IF_GET_VERSION_INFO,
                                       conn_id,
                                       &cmd_data,
                                       qmi_err_code,
                                       QMI_SYNC_MSG_EXTENDED_TIMEOUT);
  if (rc == QMI_NO_ERR ) {
    //memcpy(version_list,&(cmd_data.qmi_qmux_if_get_version_info_rsp),sizeof(qmi_service_version_info));
    QMI_DEBUG_MSG_1(" Number of services:%d\n",cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version_len);
    is_version_list_cached[conn_id] = TRUE;
    memcpy (&version_list_cache[conn_id], &cmd_data.qmi_qmux_if_get_version_info_rsp, sizeof(qmi_service_version_list_type));

    for (index = 0; index < cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version_len; index++ ) {
      if (cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version[index].qmi_svc_type == service_id)
      {
        service_id_index =  index;
        break;
      }
    }
  }

  if (service_id_index >= 0) {
    *major_ver = cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version[service_id_index].major_ver;
    *minor_ver = cmd_data.qmi_qmux_if_get_version_info_rsp.qmi_service_version[service_id_index].minor_ver;
  }
  else {
    rc = QMI_SERVICE_NOT_PRESENT_ON_MODEM;
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qmux_if_reg_service_avail
===========================================================================*/
/*!
@brief
  This function is used to register for service availability indication
  from modem.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    - None
*/
/*=========================================================================*/

int
qmi_qmux_if_reg_srvc_avail
(
  qmi_qmux_if_hndl_t     qmux_if_client_handle,
  qmi_connection_id_type conn_id,
  qmi_service_id_type    service_id,
  int                   *qmi_err_code
)
{
  int rc;
  qmi_qmux_if_cmd_rsp_type  cmd_data;

  memset(&cmd_data,0,sizeof (qmi_qmux_if_cmd_rsp_type));
  cmd_data.qmi_qmux_if_reg_srvc_req.service_id = service_id;

  rc = qmi_qmux_if_send_if_msg_to_qmux(qmux_if_client_handle,
                                       QMI_QMUX_IF_REG_SRVC_AVAIL_MSG_ID,
                                       conn_id,
                                       &cmd_data,
                                       qmi_err_code,
                                       QMI_SYNC_MSG_EXTENDED_TIMEOUT);

  if (FALSE == event_reports_enabled) {
    /* Send SET_EVENT_REPORT immediately after if not sent
    This must be done only once */
    event_reports_enabled = TRUE;
    rc = qmi_qmux_if_send_if_msg_to_qmux(qmux_if_client_handle,
                                         QMI_QMUX_IF_SET_EVENT_REPORT_MSG_ID,
                                         conn_id,
                                         &cmd_data,
                                         qmi_err_code,
                                         QMI_SYNC_MSG_EXTENDED_TIMEOUT);
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qmux_if_send_raw_qmi_cntl_msg
===========================================================================*/
/*!
@brief
  Send a raw QMI control message to the modem. A QMUX SDU is expected in
  the raw message.

@return
  QMI_NO_ERR if no error occurred, QMI_*_ERR error code otherwise

@note

  - Side Effects
    - None

*/
/*=========================================================================*/
int
qmi_qmux_if_send_raw_qmi_cntl_msg
(
  qmi_qmux_if_hndl_t      qmux_if_client_handle,
  qmi_connection_id_type  conn_id,
  unsigned char           *msg,
  int                     msg_len
)
{

  qmi_qmux_if_msg_hdr_type hdr;
  int rc = QMI_INTERNAL_ERR;
  qmi_qmux_if_client_data_type  *client = NULL, *prev = NULL;
  unsigned char *ctl_sdu = NULL;
  unsigned char qmux_txn_id = 0;
  unsigned char tmp8 = 0;
  qmi_qmux_clnt_id_t  qmux_client_id = QMI_QMUX_INVALID_QMUX_CLIENT_ID;

  /*-----------------------------------------------------------------------*/

  /* Validate input parameters */
  if (!msg)
  {
    QMI_ERR_MSG_0 ("qmi_qmux_if_send_raw_qmi_cntl_msg: bad parameter(s)\n");
    return QMI_INTERNAL_ERR;
  }

  /* Validate qmux_client_id */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_client_list_mutex);
  QMI_SLL_FIND(client,
               prev,
               qmi_qmux_if_client_data_list,
               (client == qmux_if_client_handle));

  if (client)
  {
    qmux_client_id = client->qmux_client_id;
  }

  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_client_list_mutex);

  if (!client)
  {
    QMI_ERR_MSG_1 ("qmi_qmux_if_send_raw_qmi_cntl_msg: Invalid qmux client ID=%d\n",
                   qmux_if_client_handle);
    return QMI_INTERNAL_ERR;
  }

  /* Find the txn_id in the given CTL SDU */
  ctl_sdu = msg;

  /* Read the control flags */
  READ_8_BIT_VAL (ctl_sdu,tmp8);

  /* Read the control txn_id */
  READ_8_BIT_VAL (ctl_sdu,qmux_txn_id);

  /* Set up message for sending */
  memset(&hdr, 0, sizeof(qmi_qmux_if_msg_hdr_type));

  hdr.msg_id = QMI_QMUX_IF_SEND_RAW_QMI_CTL_MSG_ID;
  hdr.qmux_client_id = qmux_client_id;
  hdr.qmux_txn_id = qmux_txn_id;
  hdr.qmi_conn_id = conn_id;
  hdr.qmi_service_id = 0; /* Control Service */
  hdr.qmi_client_id = 0;  /* Unused */
  hdr.control_flags = 0;  /* Unused for TX to QMUX, only valid for RX */

  /* Decrement msg pointer and increment msg_len */
  msg -= QMI_QMUX_IF_HDR_SIZE;
  msg_len += (int)QMI_QMUX_IF_HDR_SIZE;

  /* Copy header into message buffer */
  memcpy ((void *)msg, (void *)&hdr, QMI_QMUX_IF_HDR_SIZE);

  /* Lock TX mutex */
  QMI_PLATFORM_MUTEX_LOCK (&qmi_qmux_if_tx_mutex);

#ifndef QMI_MSGLIB_MULTI_PD
  rc = qmi_qmux_tx_msg (qmux_client_id,msg,msg_len);
#else
  rc = QMI_QMUX_IF_PLATFORM_TX_MSG( qmux_client_id,
                                    msg,
                                    msg_len );
#endif

  /* UnLock TX mutex */
  QMI_PLATFORM_MUTEX_UNLOCK (&qmi_qmux_if_tx_mutex);

  return rc;
}


