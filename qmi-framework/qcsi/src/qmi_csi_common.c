/******************************************************************************
  @file    qmi_csi_common.c
  @brief   The QMI Common Service Interface (CSI)

  DESCRIPTION
  QMI common server routines.  All server will be build on top of these
  routines for initializing, sending responses and indications.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include <string.h>
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_csi_target.h"
#include "qmi_csi_common.h"
#include "qmi_fw_debug.h"
#include "common_v01.h"

#define MAX_XPORTS 10
#define QCSI_MIN(a, b) ((a) > (b) ? (b) : (a))

extern qmi_csi_xport_ops_type qcsi_ipc_router_ops;
extern void qmi_csi_debug_init(void);
#define DEFAULT_MAX_TX_BUFS (30)

/* Global lists of services, clients and outstanding txns */
static qmi_csi_lock_type service_list_lock;
static LIST(qmi_csi_service_type, service_list);
static qmi_csi_lock_type client_list_lock;
static LIST(qmi_csi_client_type, client_list);
static qmi_csi_lock_type txn_list_lock;
static LIST(qmi_csi_txn_type, txn_list);

/* globally unique service, client and txn descriptors - monotonically
 * increasing numbers. Use list locks to protect these
 */
static uint32_t global_service_desc;
static uint32_t global_client_desc;
static uint32_t global_txn_desc;

static uint32_t num_xports;
struct qmi_csi_xport_tbl_s
{
  qmi_csi_xport_ops_type *ops;
  void *xport_data;
};

static struct qmi_csi_xport_tbl_s xport_tbl[MAX_XPORTS];
static uint32_t inited;

/* Translate QCSI errors to common error types */
static uint16_t qmi_csi_err_translate(uint32_t csi_err)
{
  qmi_error_type_v01 rc = QMI_ERR_INTERNAL_V01;
  switch(csi_err)
  {
    case QMI_CSI_NO_ERR:
      rc = QMI_ERR_NONE_V01;
      break;
    case QMI_CSI_CONN_REFUSED:
      rc = QMI_ERR_CLIENT_IDS_EXHAUSTED_V01;
      break;
    case QMI_CSI_ENCODE_ERR:
    case QMI_CSI_DECODE_ERR:
      rc = QMI_ERR_ENCODING_V01;
      break;
    case QMI_CSI_NO_MEM:
      rc = QMI_ERR_NO_MEMORY_V01;
      break;
    default:
      rc = QMI_ERR_INTERNAL_V01;
      break;
  }
  return (uint32_t)rc;
}

/* add service to the global list */
static uint32_t add_service(qmi_csi_service_type *svc)
{
  uint32_t service_handle;

  LOCK(&service_list_lock);
  service_handle = svc->handle = ++global_service_desc;
  LIST_ADD(service_list, svc, link);
  UNLOCK(&service_list_lock);

  return service_handle;
}

/* remove a service from the global list */
static void remove_service(qmi_csi_service_type *svc)
{
  LOCK(&service_list_lock);
  LIST_REMOVE(service_list, svc, link);
  UNLOCK(&service_list_lock);
}

/* service list lock needs to be held */
static qmi_csi_service_type *find_service(uint32_t handle)
{
  qmi_csi_service_type *svc = LIST_HEAD(service_list);

  while(svc)
  {
    if(svc->handle == handle)
      return svc;
    svc = svc->link.next;
  }
  return NULL;
}

/* neither svc nor clnt can be NULL, client_list_lock held */
static void add_client(qmi_csi_service_type *svc, qmi_csi_client_type *clnt)
{
  /* set unique client handle */
  clnt->handle = ++global_client_desc;
  LIST_ADD(svc->client_list, clnt, local);
  LIST_ADD(client_list, clnt, global);
}

/* neither svc nor clnt can be NULL, client_list_lock held */
static void remove_client(qmi_csi_service_type *svc, qmi_csi_client_type *clnt)
{
  LIST_REMOVE(svc->client_list, clnt, local);
  LIST_REMOVE(client_list, clnt, global);
}

/* client list lock needs to be held */
static qmi_csi_client_type *find_client(uint32_t handle)
{
  qmi_csi_client_type *clnt = LIST_HEAD(client_list);

  while(clnt)
  {
    if(clnt->handle == handle)
      return clnt;
    clnt = clnt->global.next;
  }
  return NULL;
}

/* txn_list_lock needs to be held */
static uint32_t add_txn(qmi_csi_client_type *clnt, qmi_csi_txn_type *txn)
{
  txn->handle = ++global_txn_desc;
  LIST_ADD(clnt->txn_list, txn, local);
  LIST_ADD(txn_list, txn, global);
  return txn->handle;
}

/* txn_list_lock needs to be held */
static void remove_txn(qmi_csi_client_type *clnt, qmi_csi_txn_type *txn)
{
  LIST_REMOVE(clnt->txn_list, txn, local);
  LIST_REMOVE(txn_list, txn, global);
}

/* txn list lock needs to be held */
static qmi_csi_txn_type *find_txn(uint32_t handle)
{
  qmi_csi_txn_type *txn = LIST_HEAD(txn_list);
  while(txn)
  {
    if(txn->handle == handle)
      return txn;
    txn = txn->global.next;
  }
  return NULL;
}

static qmi_csi_client_type *find_client_by_addr
(
 qmi_csi_xport_type *xport,
 void *addr
 )
{
  qmi_csi_client_type *clnt;

  if(!xport || !xport->service)
    return NULL;

  clnt = LIST_HEAD(xport->service->client_list);
  while(clnt)
  {
    if(clnt->xport.xport == xport &&
        !memcmp(clnt->xport.addr, addr, xport->addr_len))
      return clnt;
    clnt = clnt->local.next;
  }
  return NULL;
}

/* client list should be locked */
static void clean_txns(qmi_csi_client_type *clnt)
{
  qmi_csi_txn_type *txn;
  LOCK(&txn_list_lock);
  txn = LIST_HEAD(clnt->txn_list);
  while(txn)
  {
    qmi_csi_txn_type *to_free = txn;
    txn = txn->local.next;
    /* remove from global txn list */
    LIST_REMOVE(txn_list, to_free, global);
    FREE(to_free);
  }
  UNLOCK(&txn_list_lock);
}

/* client list lock needs to be held */
static qmi_csi_client_type *create_client
(
 qmi_csi_xport_type *xport,
 void *addr
)
{
  qmi_csi_service_type *svc = xport->service;
  qmi_csi_client_type  *clnt;


  /* client not found, create new connection */
  clnt = CALLOC(1, sizeof(qmi_csi_client_type));
  if(!clnt)
    return NULL;
  LINK_INIT(clnt->local);
  LINK_INIT(clnt->global);
  LIST_INIT(clnt->txn_list);

  add_client(svc, clnt);

  /* initialize client struct fields */
  clnt->service = svc;
  clnt->xport.xport = xport;
  clnt->next_ind_txn_id = 1;
  memcpy(clnt->xport.addr, addr, xport->addr_len);

  return clnt;
}

/* encode a send message to client */
static qmi_csi_error internal_send
(
 qmi_csi_service_type *svc,
 qmi_csi_client_type *clnt,
 qmi_idl_type_of_message_type msg_type,
 tnx_id_type txn_id,
 uint16_t msg_id,
 void *c_struct,
 unsigned int c_struct_len,
 int encode
)
{
  qmi_csi_xport_type *xport;
  uint32_t max_msg_len = 0, out_len;
  qmi_csi_error rc;
  int32_t encdec_rc;
  unsigned char *msg;
  uint8_t cntl_flag;
  uint32_t send_flags = 0;


  if(encode)
  {
    uint32_t idl_c_struct_len;
    encdec_rc = qmi_idl_get_message_c_struct_len(svc->service_obj, msg_type, msg_id,
      &idl_c_struct_len);
    if(encdec_rc != QMI_IDL_LIB_NO_ERR)
      return QMI_CSI_ENCODE_ERR;

    if(c_struct_len != idl_c_struct_len)
      return QMI_CSI_ENCODE_ERR;

    encdec_rc = qmi_idl_get_max_message_len(svc->service_obj, msg_type, msg_id,
      &max_msg_len);
    if(encdec_rc != QMI_IDL_LIB_NO_ERR)
      return QMI_CSI_ENCODE_ERR;
  }
  else
  {
    max_msg_len = c_struct_len;
  }

  if(c_struct && c_struct_len)
  {
    msg = MALLOC(max_msg_len + QMI_HEADER_SIZE);
    if(!msg)
      return QMI_CSI_NO_MEM;

    if(encode)
    {
       if( qmi_idl_message_encode(
              svc->service_obj,
              msg_type,
              msg_id,
              c_struct,
              c_struct_len,
              msg + QMI_HEADER_SIZE,
              max_msg_len,
              &out_len) != QMI_IDL_LIB_NO_ERR)
      {
         FREE(msg);
         return QMI_CSI_ENCODE_ERR;
      }
    }
    else
    {
       memcpy(msg + QMI_HEADER_SIZE, c_struct, c_struct_len);
       out_len = c_struct_len;
    }
  }
  else
  {
    /* Empty message */
    out_len = 0;
    msg = MALLOC(QMI_HEADER_SIZE);
    if(!msg)
      return QMI_CSI_NO_MEM;
  }

  switch(msg_type)
  {
    case QMI_IDL_INDICATION:
      cntl_flag = QMI_INDICATION_CONTROL_FLAG;
      send_flags |= QMI_CSI_SEND_FLAG_RATE_LIMITED;
      break;
    case QMI_IDL_RESPONSE:
      cntl_flag = QMI_RESPONSE_CONTROL_FLAG;
      break;
    default:
      cntl_flag = QMI_REQUEST_CONTROL_FLAG;
      break;
  }

  /* fill in header */
  encode_header(msg, cntl_flag, txn_id, msg_id, (uint16_t)out_len);

  QMI_CSI_OS_LOG_TX(msg);

  out_len += QMI_HEADER_SIZE;

  if(clnt)
  {
    xport = clnt->xport.xport;
    if(xport->ops->send_legacy)
    {
      rc = xport->ops->send_legacy(xport->handle, clnt->xport.addr, msg, out_len,
          &clnt->xport.client_data);
    }
    else
    {
      rc = xport->ops->send(xport->handle, clnt->xport.addr, msg, out_len,
                            send_flags, &clnt->xport.client_data);
    }
  }
  else
  {
    /* broadcast too all clients. ignore errors */
    clnt = LIST_HEAD(svc->client_list);
    while(clnt)
    {
      xport = clnt->xport.xport;
      if(xport->ops->send_legacy)
      {
        xport->ops->send_legacy(xport->handle, clnt->xport.addr, msg, out_len,
          &clnt->xport.client_data);
      }
      else
      {
        xport->ops->send(xport->handle, clnt->xport.addr, msg, out_len,
                         send_flags, &clnt->xport.client_data);
      }
      clnt = clnt->local.next;
    }
    rc = QMI_CSI_NO_ERR;
  }

  FREE(msg);

  return rc;
}

/* encode and send error response to client */
static qmi_csi_error encode_and_send_resp
(
 qmi_csi_xport_type *xport,
 void *addr,
 uint16_t msg_id,
 uint16_t txn_id,
 uint16_t result,
 uint16_t error
)
{
  uint32_t resp_msg_len;
  qmi_csi_error rc;
  unsigned char *msg;

  resp_msg_len = qmi_idl_get_std_resp_tlv_len();

  msg = MALLOC(resp_msg_len + QMI_HEADER_SIZE);
  if(!msg)
    return QMI_CSI_NO_MEM;

  if( qmi_idl_encode_resp_tlv(
        result,
        error,
        msg + QMI_HEADER_SIZE,
        resp_msg_len
        ) != QMI_IDL_LIB_NO_ERR)
  {
    FREE(msg);
    return QMI_CSI_ENCODE_ERR;
  }

  /* fill in header */
  encode_header(msg, QMI_RESPONSE_CONTROL_FLAG, txn_id, msg_id,
      (uint16_t)resp_msg_len);
  QMI_CSI_OS_LOG_TX(msg);
  resp_msg_len += QMI_HEADER_SIZE;

  if(xport->ops->send_legacy)
  {
    rc = xport->ops->send_legacy(xport->handle, addr, msg, resp_msg_len, NULL);
  }
  else
  {
    /* Do not rate limit responses */
    rc = xport->ops->send(xport->handle, addr, msg, resp_msg_len, 0, NULL);
  }

  FREE(msg);

  return rc;
}

/*=============================================================================
                                PUBLIC FUNCTIONS
=============================================================================*/

/*=============================================================================
                               Up-calls from xport
=============================================================================*/
/*=============================================================================
  FUNCTION  qmi_csi_xport_connect
=============================================================================*/
qmi_csi_error qmi_csi_xport_connect
(
 qmi_csi_xport_type *xport,
 void *addr
)
{
  qmi_csi_client_type  *clnt;
  qmi_csi_service_type *svc;
  qmi_csi_connect connect_cb = NULL;
  uint32_t client_handle = 0xffffffff;
  void *service_cookie = NULL;
  void *connection_handle = NULL;
  qmi_csi_error rc = QMI_CSI_INTERNAL_ERR;
  qmi_csi_cb_error cb_rc;

  if(!xport || !xport->service || !addr)
    return QMI_CSI_INTERNAL_ERR;

  LOCK(&client_list_lock);
  /* figure out if client address exists in client list */
  clnt = find_client_by_addr(xport, addr);

  if(clnt)
  {
    /* client exits, do nothing */
    UNLOCK(&client_list_lock);
    return QMI_CSI_NO_ERR;
  }

  clnt = create_client(xport, addr);

  if(!clnt)
  {
    UNLOCK(&client_list_lock);
    return QMI_CSI_NO_MEM;
  }

  /* cache params to the callback */
  svc = clnt->service;
  connect_cb = svc->service_connect;
  service_cookie = svc->service_cookie;
  client_handle = clnt->handle;

  UNLOCK(&client_list_lock);

  /* invoke service_connect without lock held */
  if(connect_cb)
  {
    cb_rc = connect_cb((qmi_client_handle)(uintptr_t)client_handle,
        service_cookie, &connection_handle);

    /* re-lock client list and re-lookup client with handle */
    LOCK(&client_list_lock);
    clnt = find_client(client_handle);
    if(cb_rc != QMI_CSI_CB_NO_ERR)
    {
      if(clnt)
      {
        remove_client(clnt->service, clnt);
        FREE(clnt);
      }
      if(cb_rc == QMI_CSI_CB_NO_MEM)
      {
        rc = QMI_CSI_NO_MEM;
      }
      else
      {
        rc = QMI_CSI_CONN_REFUSED;
      }
    }
    else
    {
      /* check to see if clnt still exits */
      if(clnt)
      {
        clnt->connection_handle = connection_handle;
        rc = QMI_CSI_NO_ERR;
      }
    }
    UNLOCK(&client_list_lock);
  }

  return rc;
}

/*=============================================================================
  FUNCTION  qmi_csi_xport_recv
=============================================================================*/
qmi_csi_error qmi_csi_xport_recv
(
 qmi_csi_xport_type *xport,
 void *addr,
 uint8_t *buf,
 uint32_t len
 )
{
  qmi_csi_service_type *svc;
  qmi_csi_client_type  *clnt;
  qmi_csi_txn_type     *txn;
  unsigned char *c_struct = NULL;
  uint32_t txn_handle, c_struct_len;
  uint8_t cntl_flag;
  uint16_t txn_id, msg_id, msg_len;
  int rc = QMI_ERR_INTERNAL_V01;
  void *connection_handle, *service_cookie;

  if(!xport || !xport->service || !addr || len < QMI_HEADER_SIZE)
    return QMI_CSI_INTERNAL_ERR;

  svc = xport->service;

  QMI_CSI_OS_LOG_RX(buf);

  decode_header(buf, &cntl_flag, &txn_id, &msg_id, &msg_len);

  /* got a client struct, handle only request */
  if(cntl_flag != QMI_REQUEST_CONTROL_FLAG)
  {
    rc = QMI_ERR_MALFORMED_MSG_V01;
    goto rx_cb_bail;
  }

  /* if received message is shorter than the size in the header then there
   * might have been a memory allocation error
   */
  if((len - QMI_HEADER_SIZE) != msg_len)
  {
    rc = QMI_ERR_NO_MEMORY_V01;
    goto rx_cb_bail;
  }

  LOCK(&client_list_lock);

  /* figure out if client address exists in client list */
  clnt = find_client_by_addr(xport, addr);
  if(!clnt)
  {
    UNLOCK(&client_list_lock);
    /* Auto connect on first packet */
    rc = qmi_csi_xport_connect(xport, addr);
    if(rc != QMI_CSI_NO_ERR)
    {
      rc = qmi_csi_err_translate(rc);
      goto rx_cb_bail;
    }

    /* Lock and find the client, if not found,
     * do not proceed. */
    LOCK(&client_list_lock);
    clnt = find_client_by_addr(xport, addr);
    if(!clnt)
    {
      rc = QMI_ERR_INTERNAL_V01;
      UNLOCK(&client_list_lock);
      goto rx_cb_bail;
    }
  }

  txn = CALLOC(1, sizeof(qmi_csi_txn_type));
  if(!txn)
  {
    rc = QMI_ERR_NO_MEMORY_V01;
    goto rx_cb_bail;
  }
  LINK_INIT(txn->local);
  LINK_INIT(txn->global);
  txn->client = clnt;
  txn->txn_id = txn_id;
  txn->msg_id = msg_id;
  /* add to txn list */
  LOCK(&txn_list_lock);
  txn_handle = add_txn(clnt, txn);
  UNLOCK(&txn_list_lock);

  /* cache handle values before unlocking and invoking callback */
  connection_handle = clnt->connection_handle;
  service_cookie = svc->service_cookie;
  UNLOCK(&client_list_lock);

  /* Handle pre-request */
  if(svc->service_process_pre_req)
  {
    rc = svc->service_process_pre_req(connection_handle,
                                     (qmi_req_handle)(uintptr_t)txn_handle,
                                     msg_id,
                                     msg_len ? buf + QMI_HEADER_SIZE: NULL,
                                     msg_len,
                                     service_cookie);

    if(rc == QMI_CSI_CB_REQ_HANDLED)
    {
      return QMI_CSI_NO_ERR;
    }
    if(rc != QMI_CSI_CB_NO_ERR)
    {
      rc = QMI_ERR_NOT_SUPPORTED_V01;
      goto rx_cb_free_txn_bail;
    }
  }

  /* decode message */
  rc = qmi_idl_get_message_c_struct_len(svc->service_obj, QMI_IDL_REQUEST,
      msg_id, &c_struct_len);

  /* If this message is unknown, see if the service is interested
   * in the raw message */
  if(rc == QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND && svc->service_process_raw_req)
  {
    rc = svc->service_process_raw_req(connection_handle,
                                     (qmi_req_handle)(uintptr_t)txn_handle,
                                     msg_id,
                                     msg_len ? buf + QMI_HEADER_SIZE: NULL,
                                     msg_len,
                                     service_cookie);
    if(rc != QMI_CSI_CB_NO_ERR)
    {
      rc = QMI_ERR_INTERNAL_V01;
      goto rx_cb_free_txn_bail;
    }
    return QMI_CSI_NO_ERR;
  }
  else if(rc != QMI_IDL_LIB_NO_ERR)
  {
    rc = QMI_ERR_ENCODING_V01;
    goto rx_cb_free_txn_bail;
  }

  /* Decode the message */
  if(c_struct_len)
  {
    c_struct = MALLOC(c_struct_len);

    if(!c_struct)
    {
      rc = QMI_ERR_NO_MEMORY_V01;
      goto rx_cb_free_txn_bail;
    }

    rc = qmi_idl_message_decode(
          svc->service_obj,
          QMI_IDL_REQUEST,
          msg_id,
          buf + QMI_HEADER_SIZE,
          len - QMI_HEADER_SIZE,
          c_struct,
         (uint16_t)c_struct_len);
    if(rc != QMI_IDL_LIB_NO_ERR)
    {
      if (rc < QMI_IDL_LIB_NO_ERR)
      {
        rc = QMI_ERR_ENCODING_V01;
      }
      FREE(c_struct);
      goto rx_cb_free_txn_bail;
    }
  }

  /* Finally handle the request */
  if(svc->service_process_req(
      connection_handle,
      (qmi_req_handle)(uintptr_t)txn_handle,
      msg_id,
      c_struct,
      c_struct_len,
      service_cookie
      ) != QMI_CSI_CB_NO_ERR)
  {
    if(c_struct)
      FREE(c_struct);
    goto rx_cb_free_txn_bail;
  }

  if(c_struct)
    FREE(c_struct);

  return QMI_CSI_NO_ERR;

rx_cb_free_txn_bail:
    LOCK(&txn_list_lock);
    txn = find_txn(txn_handle);
    if(txn)
    {
      remove_txn(txn->client, txn);
      FREE(txn);
    }
    UNLOCK(&txn_list_lock);
rx_cb_bail:
    encode_and_send_resp(xport, addr, msg_id, txn_id,
        QMI_RESULT_FAILURE_V01, (uint16_t)rc);
  return QMI_CSI_NO_ERR;
}

/*=============================================================================
  FUNCTION  qmi_csi_xport_resume_client
=============================================================================*/
void qmi_csi_xport_resume_client
(
 qmi_csi_xport_type *xport,
 void *addr
 )
{
  qmi_csi_service_type *svc;
  qmi_csi_client_type  *clnt;
  qmi_csi_resume_ind resume_cb = NULL;
  qmi_client_handle client_handle;
  void *service_cookie, *connection_handle;

  if(!xport || !xport->service || !addr)
    return;

  svc = xport->service;

  LOCK(&client_list_lock);
  /* try to look for client, if found, call disconnect handler */
  clnt = find_client_by_addr(xport, addr);
  if(clnt)
  {
    resume_cb = svc->resume_ind_cb;
    client_handle = (qmi_client_handle)(uintptr_t)clnt->handle;
    connection_handle = clnt->connection_handle;
    service_cookie = svc->service_cookie;
  }
  UNLOCK(&client_list_lock);

  if(resume_cb)
  {
    resume_cb(client_handle, connection_handle, service_cookie);
  }
}


/*=============================================================================
  FUNCTION  qmi_csi_xport_disconnect
=============================================================================*/
qmi_csi_error qmi_csi_xport_disconnect
(
 qmi_csi_xport_type *xport,
 void *addr
 )
{
  qmi_csi_service_type *svc;
  qmi_csi_client_type  *clnt;

  if(!xport || !xport->service || !addr)
    return QMI_CSI_INTERNAL_ERR;

  svc = xport->service;

  LOCK(&client_list_lock);
  /* try to look for client, if found, call disconnect handler */
  clnt = find_client_by_addr(xport, addr);
  if(clnt)
  {
    /* remove client from active list */
    remove_client(svc, clnt);
    svc->service_disconnect(clnt->connection_handle, svc->service_cookie);
    clean_txns(clnt);
    FREE(clnt);
  }
  UNLOCK(&client_list_lock);
  return QMI_CSI_NO_ERR;
}

/*=============================================================================
  FUNCTION  qmi_csi_xport_closed
=============================================================================*/
void qmi_csi_xport_closed
(
 qmi_csi_xport_type *xport
 )
{
  qmi_csi_service_type *svc;
  qmi_csi_client_type  *clnt;
  unsigned int i, active = 0;

  if(!xport || !xport->service)
    return;

  /* go through client list and clean up clients associated with the xport */
  svc = xport->service;
  LOCK(&client_list_lock);
  clnt = LIST_HEAD(svc->client_list);
  while(clnt)
  {
    qmi_csi_client_type *to_free = clnt;
    clnt = clnt->local.next;
    if(to_free->xport.xport == xport)
    {
      /* call disconnect callback? */
      clean_txns(to_free);
      /* remove from global client list */
      remove_client(svc, to_free);
      FREE(to_free);
    }
  }
  UNLOCK(&client_list_lock);

  /* remove xport from xport list. When no xport left, free service */
  for(i = 0; i < num_xports; i++)
  {
    if(svc->xports[i] == xport)
    {
      svc->xports[i] = NULL;
      svc->num_xports--;
    }
    if(svc->xports[i])
      active++;
  }
  FREE(xport);

  if(!active)
  {
    /* no more xport left */
    remove_service(svc);
    FREE(svc->xports);
    FREE(svc);
  }
}

/*=============================================================================
  FUNCTION  qmi_csi_xport_start
=============================================================================*/
void qmi_csi_xport_start(qmi_csi_xport_ops_type *ops, void *xport_data)
{
  unsigned int i;

  if(ops->send_legacy == NULL && ops->send == NULL)
  {
    /* Do not allow an xport without either send */
    return;
  }

  if(ops->open_legacy == NULL && ops->open == NULL)
  {
    return;
  }

  for (i = 0; i < num_xports; i++)
  {
    if ((xport_tbl[i].ops == ops) && (xport_tbl[i].xport_data == xport_data))
      return;
  }

  if(num_xports < MAX_XPORTS)
  {
    xport_tbl[num_xports].ops = ops;
    xport_tbl[num_xports].xport_data = xport_data;
    num_xports++;
  }

}

/*===========================================================================
  FUNCTION  qmi_csi_xport_stop
===========================================================================*/
void qmi_csi_xport_stop(qmi_csi_xport_ops_type *ops, void *xport_data)
{
  int i;
  for (i = 0; i < MAX_XPORTS; i++)
  {
    if ((xport_tbl[i].ops == ops) && (xport_tbl[i].xport_data == xport_data))
    {
      xport_tbl[i].ops = NULL;
      xport_tbl[i].xport_data = NULL;
      num_xports--;
    }
  }
}

/*=============================================================================
                            QMI CSI API Implementation
=============================================================================*/

/*=============================================================================
  FUNCTION  qmi_csi_register_with_options
=============================================================================*/
qmi_csi_error
qmi_csi_register_with_options
(
 qmi_idl_service_object_type               service_obj,
 qmi_csi_connect                           service_connect,
 qmi_csi_disconnect                        service_disconnect,
 qmi_csi_process_req                        service_process_req,
 void                                      *service_cookie,
 qmi_csi_os_params                         *os_params,
 qmi_csi_options                           *options,
 qmi_csi_service_handle                    *service_provider
 )
{
  qmi_csi_service_type *svc;
  qmi_csi_xport_type *xport;
  uint32_t service_id,  i, max_msg_len;
  int active = 0;

  if(!service_obj || !service_connect || !service_disconnect || !service_process_req || !service_provider)
    return QMI_CSI_INTERNAL_ERR;

  *service_provider = NULL;

  svc = CALLOC(1, sizeof(qmi_csi_service_type));
  if(!svc)
  {
    /* failed to allocate memory */
    return QMI_CSI_INTERNAL_ERR;
  }

  LINK_INIT(svc->link);
  LIST_INIT(svc->client_list);
  svc->service_obj = service_obj;
  svc->service_connect = service_connect;
  svc->service_disconnect = service_disconnect;
  svc->service_process_req = service_process_req;
  svc->service_process_raw_req = NULL;
  svc->service_process_pre_req = NULL;
  svc->service_cookie = service_cookie;
  svc->resume_ind_cb = NULL;

  /* Set xport options to its default values */
  svc->xport_options.rate_limited_queue_size = DEFAULT_MAX_TX_BUFS;

  if(qmi_idl_get_service_id(service_obj, &service_id) != QMI_IDL_LIB_NO_ERR ||
     qmi_idl_get_idl_version(service_obj, &(svc->idl_version)) != QMI_IDL_LIB_NO_ERR ||
     qmi_idl_get_max_service_len(service_obj, &max_msg_len) != QMI_IDL_LIB_NO_ERR)
  {
    FREE(svc);
    return QMI_CSI_INTERNAL_ERR;
  }

  if(options)
  {
    if(options->options_set & QMI_CSI_OPTIONS_INSTANCE_ID_VALID)
    {
      svc->idl_version |= SET_INSTANCE(options->instance_id);
    }

#ifdef QMI_CSI_OPTIONS_MAX_OUTSTANDING_INDS_VALID
    if(options->options_set & QMI_CSI_OPTIONS_MAX_OUTSTANDING_INDS_VALID)
    {
      if(options->max_outstanding_inds > 0)
      {
        svc->xport_options.rate_limited_queue_size = options->max_outstanding_inds;
      }
    }
#endif
#ifdef QMI_CSI_OPTIONS_RAW_REQUEST_VALID
    if(options->options_set & QMI_CSI_OPTIONS_RAW_REQUEST_VALID)
    {
      svc->service_process_raw_req = options->raw_request_cb;
    }
#endif

#ifdef QMI_CSI_OPTIONS_PRE_REQUEST_VALID
    if(options->options_set & QMI_CSI_OPTIONS_PRE_REQUEST_VALID)
    {
      svc->service_process_pre_req = options->pre_request_cb;
    }
#endif

#ifdef QMI_CSI_OPTIONS_RESUME_VALID
    if(options->options_set & QMI_CSI_OPTIONS_RESUME_VALID)
    {
      svc->resume_ind_cb = options->resume_ind_cb;
    }
#endif
  }

  max_msg_len += QMI_HEADER_SIZE;

  /* handle OS-specific parameters such as storing away signal/event & TCB */
#ifdef QMI_CSI_OS_PARAMS_PROLOG
  QMI_CSI_OS_PARAMS_PROLOG(svc, os_params);
#endif

  svc->xports = CALLOC(num_xports, sizeof(qmi_csi_xport_type *));
  if(!svc->xports)
  {
    FREE(svc);
    return QMI_CSI_NO_MEM;
  }
  svc->num_xports = 0;

  /* open up all the known xports and register service */
  for(i = 0; i < num_xports; i++)
  {
    xport = CALLOC(1, sizeof(qmi_csi_xport_type));
    if(!xport)
    {
      continue;
    }
    xport->ops = xport_tbl[i].ops;
    xport->addr_len = QCSI_MIN(MAX_ADDR_LEN, xport->ops->addr_len());
    xport->service = svc;
    /* open xport */
    if(xport->ops->open_legacy)
    {
      xport->handle = xport->ops->open_legacy(xport_tbl[i].xport_data, xport,
        max_msg_len, os_params);
    }
    else
    {
      xport->handle = xport->ops->open(xport_tbl[i].xport_data, xport,
                                       max_msg_len, os_params, &svc->xport_options);
    }

    if(!xport->handle)
    {
      FREE(xport);
      continue;
    }
     /* At least one xport needs to be closed */
    svc->xports[svc->num_xports] = xport;
    svc->num_xports++;

    /* register service on xport */
    if(xport->ops->reg(xport->handle, service_id, svc->idl_version)
        != QMI_CSI_NO_ERR)
    {
      continue;
    }

    active = 1;
  }

  if(!active)
  {
    uint32_t num_xports = svc->num_xports;

    if(num_xports == 0)
    {
      FREE(svc->xports);
      FREE(svc);
      return QMI_CSI_INTERNAL_ERR;
    }
    /* num_xports is cached as close() could
     * free svc */
    for(i = 0; i < num_xports; i++)
    {
      if(svc->xports[i] && svc->xports[i]->handle)
      {
        svc->xports[i]->ops->close(svc->xports[i]->handle);
	/* svc->xports[i] freed in close.
	 * svc freed in last close */
      }
    }
    return QMI_CSI_INTERNAL_ERR;
  }

  /* handle OS-specific parameters such as returning fd_set */
#ifdef QMI_CSI_OS_PARAMS_EPILOG
  QMI_CSI_OS_PARAMS_EPILOG(svc, params);
#endif

  /* add to service list */
  *service_provider = (qmi_csi_service_handle)(uintptr_t)add_service(svc);

  return QMI_CSI_NO_ERR;
}

/*=============================================================================
  FUNCTION  qmi_csi_register
=============================================================================*/
qmi_csi_error
qmi_csi_register
(
 qmi_idl_service_object_type               service_obj,
 qmi_csi_connect                           service_connect,
 qmi_csi_disconnect                        service_disconnect,
 qmi_csi_process_req                       service_process_req,
 void                                      *service_cookie,
 qmi_csi_os_params                         *os_params,
 qmi_csi_service_handle                    *service_provider
 )
{
  return qmi_csi_register_with_options(
      service_obj,
      service_connect,
      service_disconnect,
      service_process_req,
      service_cookie,
      os_params,
      NULL,
      service_provider);
}

/*=============================================================================
  FUNCTION  qmi_csi_handle_event
=============================================================================*/
qmi_csi_error
qmi_csi_handle_event
(
 qmi_csi_service_handle                    service_provider,
 qmi_csi_os_params                         *os_params
 )
{
  qmi_csi_service_type *svc;
  qmi_csi_xport_type **xports;
  unsigned int i;

  LOCK(&service_list_lock);
  svc = find_service((uint32_t)(uintptr_t)service_provider);

  if(!svc)
  {
    UNLOCK(&service_list_lock);
    return QMI_CSI_INVALID_HANDLE;
  }

  /* save pointer to xport table and unlock, service must not unregister
   * at the same time since we don't hold the lock
   */
  xports = svc->xports;
  UNLOCK(&service_list_lock);

  /* walk through the xport list and call handle_event on each, if avail */
  for(i = 0; i < num_xports; i++)
  {
    if(xports[i] && xports[i]->ops->handle_event)
      xports[i]->ops->handle_event(xports[i]->handle, os_params);
  }


  return QMI_CSI_NO_ERR;
}

/*=============================================================================
  FUNCTION  qmi_csi_send_resp_internal
=============================================================================*/
qmi_csi_error
qmi_csi_send_resp_internal
(
 qmi_req_handle     req_handle,
 unsigned int       msg_id,
 void               *c_struct,
 unsigned int       c_struct_len,
 int                encode
)
{
  qmi_csi_txn_type *txn;
  qmi_csi_client_type *clnt;
  qmi_csi_error rc;

  if(c_struct_len <= 0)
    return QMI_CSI_INVALID_ARGS;

   /* as long as we hold the txn_list_lock, the client, service, and xport ptrs
   * will be valid
   */
  LOCK(&txn_list_lock);
  txn = find_txn((uint32_t)(uintptr_t)req_handle);

  if(!txn || txn->msg_id != msg_id || !txn->client || !txn->client->service ||
      !txn->client->xport.xport)
  {
    rc = QMI_CSI_INVALID_HANDLE;
    goto send_resp_bail;
  }

  clnt = txn->client;

  /* remove the transaction from the active list so it doesn't get freed
   * while we are holding onto the pointer
   */
  remove_txn(clnt, txn);

  /* encode and send */
  rc = internal_send(clnt->service, clnt, QMI_IDL_RESPONSE, txn->txn_id,
      (uint16_t)msg_id, c_struct, c_struct_len, encode);

  FREE(txn);

send_resp_bail:
  UNLOCK(&txn_list_lock);
  return rc;
}

/*=============================================================================
  FUNCTION  qmi_csi_send_resp
=============================================================================*/
qmi_csi_error
qmi_csi_send_resp
(
 qmi_req_handle     req_handle,
 unsigned int       msg_id,
 void               *c_struct,
 unsigned int       c_struct_len
)
{
  return qmi_csi_send_resp_internal(req_handle, msg_id, c_struct, c_struct_len, 1);
}

/*=============================================================================
  FUNCTION  qmi_csi_send_resp_raw
=============================================================================*/
qmi_csi_error
qmi_csi_send_resp_raw
(
 qmi_req_handle     req_handle,
 unsigned int       msg_id,
 void               *buf,
 unsigned int       buf_len
)
{
  return qmi_csi_send_resp_internal(req_handle, msg_id, buf, buf_len, 0);
}

/*=============================================================================
  FUNCTION  qmi_csi_send_ind_internal
=============================================================================*/
qmi_csi_error
qmi_csi_send_ind_internal
(
 qmi_client_handle  client_handle,
 unsigned int       msg_id,
 void               *c_struct,
 unsigned int       c_struct_len,
 int                encode
)
{
  qmi_csi_client_type *clnt;
  qmi_csi_error rc;

  LOCK(&client_list_lock);
  clnt = find_client((uint32_t)(uintptr_t)client_handle);

  if(!clnt || !clnt->service || !clnt->xport.xport)
  {
    rc = QMI_CSI_INVALID_HANDLE;
    goto send_ind_bail;
  }

  /* encode and send */
  rc = internal_send(clnt->service, clnt, QMI_IDL_INDICATION, clnt->next_ind_txn_id,
      (uint16_t)msg_id, c_struct, c_struct_len, encode);
  if (rc == QMI_CSI_NO_ERR)
  {
      clnt->next_ind_txn_id++;
      if (clnt->next_ind_txn_id == 0)
      {
          clnt->next_ind_txn_id++;
      }
  }


send_ind_bail:
  UNLOCK(&client_list_lock);
  return rc;
}

/*=============================================================================
  FUNCTION  qmi_csi_send_ind
=============================================================================*/
qmi_csi_error
qmi_csi_send_ind
(
 qmi_client_handle  client_handle,
 unsigned int             msg_id,
 void                    *c_struct,
 unsigned int             c_struct_len
)
{
  return qmi_csi_send_ind_internal(client_handle, msg_id, c_struct, c_struct_len, 1);
}

/*=============================================================================
  FUNCTION  qmi_csi_send_ind_raw
=============================================================================*/
qmi_csi_error
qmi_csi_send_ind_raw
(
 qmi_client_handle  client_handle,
 unsigned int       msg_id,
 void               *buf,
 unsigned int       buf_len
)
{
  return qmi_csi_send_ind_internal(client_handle, msg_id, buf, buf_len, 0);
}

/*=============================================================================
  FUNCTION  qmi_csi_send_broadcast_ind_internal
=============================================================================*/
qmi_csi_error
qmi_csi_send_broadcast_ind_internal
(
 qmi_csi_service_handle   service_provider,
 unsigned int             msg_id,
 void                     *c_struct,
 unsigned int             c_struct_len,
 int                      encode
)
{
  qmi_csi_service_type *svc;
  qmi_csi_error rc;

  /* lock client list first so if we find the service, the client list is
   * not going to be changed
   */
  LOCK(&client_list_lock);
  LOCK(&service_list_lock);
  svc = find_service((uint32_t)(uintptr_t)service_provider);

  if(!svc)
  {
    rc = QMI_CSI_INVALID_HANDLE;
    goto broadcast_ind_bail;
  }

  /* encode and send */
  rc = internal_send(svc, NULL, QMI_IDL_INDICATION, 0, (uint16_t)msg_id,
      c_struct, c_struct_len, encode);

broadcast_ind_bail:
  UNLOCK(&service_list_lock);
  UNLOCK(&client_list_lock);
  return rc;
}

/*=============================================================================
  FUNCTION  qmi_csi_send_broadcast_ind
=============================================================================*/
qmi_csi_error
qmi_csi_send_broadcast_ind
(
 qmi_csi_service_handle   service_provider,
 unsigned int             msg_id,
 void                     *c_struct,
 unsigned int             c_struct_len
)
{
  return qmi_csi_send_broadcast_ind_internal(service_provider, msg_id,
      c_struct, c_struct_len, 1);
}

/*=============================================================================
  FUNCTION  qmi_csi_send_broadcast_ind_raw
=============================================================================*/
qmi_csi_error
qmi_csi_send_broadcast_ind_raw
(
 qmi_csi_service_handle   service_provider,
 unsigned int             msg_id,
 void                     *buf,
 unsigned int             buf_len
)
{
  return qmi_csi_send_broadcast_ind_internal(service_provider, msg_id,
      buf, buf_len, 0);
}

/*=============================================================================
  FUNCTION  qmi_csi_unregister
=============================================================================*/
qmi_csi_error
qmi_csi_unregister
(
 qmi_csi_service_handle     service_provider
)
{
  qmi_csi_service_type *svc;
  qmi_csi_xport_type **xports;
  unsigned int i;

  /* Tricky problem because we may get a callback at the same time as the xport
   * is being closed. Instead, a close callback is used to synchronize freeing
   * of memory. We just need to close the transports here.
   */
  LOCK(&service_list_lock);
  svc = find_service((uint32_t)(uintptr_t)service_provider);

  if(!svc)
  {
    UNLOCK(&service_list_lock);
    return QMI_CSI_INVALID_HANDLE;
  }

  /* save pointer to xport table and unlock */
  xports = svc->xports;
  UNLOCK(&service_list_lock);

  for(i = 0; i < num_xports; i++)
  {
    if(xports[i])
      xports[i]->ops->close(xports[i]->handle);
  }

  return QMI_CSI_NO_ERR;
}

/*===========================================================================
FUNCTION: qmi_csi_init

DESCRIPTION:
   Initialize the QCSI library.  This function is called when the QCSI
   shared library is loaded, before application's main() is started.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   None

SIDE EFFECTS:
===========================================================================*/
#ifdef __GNUC__
void __attribute__ ((constructor)) qmi_csi_init(void)
{
#ifdef QMI_CSI_OS_INIT
  QMI_CSI_OS_INIT();
#endif
  /* initialize locks */
  LOCK_INIT(&service_list_lock);
  LOCK_INIT(&client_list_lock);
  LOCK_INIT(&txn_list_lock);
  inited = 1;

  smem_log_init();
  qmi_csi_xport_start(&qcsi_ipc_router_ops, NULL);
  qmi_csi_debug_init();
}
#endif

/*===========================================================================
FUNCTION: qmi_csi_exit

DESCRIPTION:
   Cleans up the QCSI library.  This function is called after exit() or
   after application's main() completes.

DEPENDENCIES:
   None

ARGUMENTS:
   None

RETURN VALUE:
   None

SIDE EFFECTS:
===========================================================================*/
#ifdef __GNUC__
void __attribute__ ((destructor)) qmi_csi_exit(void)
{
  qmi_csi_xport_stop(&qcsi_ipc_router_ops, NULL);
  smem_log_exit();
}
#endif
/*=============================================================================
  FUNCTION  qmi_csi_get_txn_id
=============================================================================*/
qmi_csi_error
qmi_csi_get_txn_id
(
 qmi_req_handle     req_handle,
 unsigned int       *txn_id
)
{
    qmi_csi_txn_type *txn;

    if(!txn_id)
        return QMI_CSI_INTERNAL_ERR;

    LOCK(&txn_list_lock);
    txn = find_txn((uint32_t)(uintptr_t)req_handle);
    if(!txn )
    {
        UNLOCK(&txn_list_lock);
        return QMI_CSI_INVALID_HANDLE;
    }

    *txn_id = txn->txn_id;
    UNLOCK(&txn_list_lock);
    return  QMI_CSI_NO_ERR;
}
