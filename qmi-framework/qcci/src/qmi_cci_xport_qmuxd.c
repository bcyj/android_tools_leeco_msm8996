/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "qmi_fw_debug.h"
#include "qmi_qmux_if.h"
#include "qmi_client_instance_defs.h"

/* Maximum service id supported by QMUX transport */
#define QMI_QMUX_MAX_SERVICE_ID 255


struct xport_handle
{
  qmi_cci_client_type *clnt;
  qmi_connection_id_type conn_id;
  qmi_service_id_type service_id;
  qmi_client_id_type client_id;
  uint32_t max_rx_len;
  uint32_t client_id_valid;
  LINK(struct xport_handle, link);
};

/* Track all the active QMUX connections in a list because the Rx callback does
 * not have a user-data parameter. Argh!
 */
static qmi_cci_lock_type client_list_lock;
static LIST(struct xport_handle, client_list);
static int qmux_client_instances;
static qmi_cci_lock_type instances_count_lock = PTHREAD_MUTEX_INITIALIZER;

static qmi_qmux_if_hndl_t  qmux_if_handle = QMI_QMUX_IF_INVALID_HNDL;

static struct xport_handle *find_handle
(
 qmi_connection_id_type conn_id,
 qmi_service_id_type service_id,
 qmi_client_id_type client_id
 )
{
  struct xport_handle *xp = LIST_HEAD(client_list);
  while(xp)
  {
    if(xp->conn_id == conn_id && xp->service_id == service_id &&
       xp->client_id == client_id) {
      if(xp->client_id_valid)
        break;
      else
        QMI_FW_LOGE("QMUXD: WARNING - STALE CLIENT: service_id %d, client_id %d, conn_id %d\n",
                     service_id, client_id, conn_id);
    }
    xp = xp->link.next;
  }
  return xp;
}

/*  Callback for subsystem restart */
static void sys_event_cb
(
  qmi_sys_event_type              event_id,
  const qmi_sys_event_info_type   *event_info,
  void                            *user_data
)
{
  struct xport_handle *xp = LIST_HEAD(client_list);

  LOCK(&client_list_lock);

  if(event_id == QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND)
  {
    while(xp)
    {
      if(event_info->qmi_sync_ind.conn_id == xp->conn_id)
      {
        QMI_CCI_LOGD("%s: QMUXD: MODEM_OUT_OF_SERVICE_IND on conn_id %d "
                     "for service_id %d\n", __func__, xp->conn_id, xp->service_id);
        xp->client_id_valid = 0;
        qmi_cci_xport_event_remove_server(xp->clnt,xp->clnt->server_addr);
      }
      xp = xp->link.next;
    }
  }
  else if(event_id == QMI_SYS_EVENT_MODEM_IN_SERVICE_IND)
  {
    while(xp)
    {
      if(event_info->qmi_sync_ind.conn_id == xp->conn_id)
      {
        QMI_CCI_LOGD("%s: QMUXD: MODEM_IN_SERVICE_IND on conn_id %d "
                     "for service_id %d\n", __func__, xp->conn_id, xp->service_id);
        qmi_cci_xport_event_new_server(xp->clnt,xp->clnt->server_addr);
      }
      xp = xp->link.next;
    }
  }
  else if(event_id == QMI_SYS_EVENT_MODEM_NEW_SRVC_IND)
  {
    while(xp)
    {
      if(event_info->qmi_sync_ind.conn_id == xp->conn_id && !xp->client_id_valid)
      {
        QMI_CCI_LOGD("%s: QMUXD: MODEM_NEW_SERVICE Indication", __func__);
        qmi_cci_xport_event_new_server(xp->clnt, xp->clnt->server_addr);
      }
      else if(!xp->client_id_valid)
      {
        QMI_CCI_LOGD("%s: QMUXD: Warning MODEM_NEW_SERVICE Indication", __func__);
        qmi_cci_xport_event_new_server(xp->clnt, xp->clnt->server_addr);
      }
      xp = xp->link.next;
    }
  }
  else if(event_id == QMI_SYS_EVENT_PORT_WRITE_FAIL_IND)
  {
    while(xp)
    {
      if(event_info->qmi_sys_port_write_failed_ind.conn_id == xp->conn_id)
      {
        QMI_CCI_LOGD("%s: QMUXD: PORT_WRITE_FAILURE_IND on conn_id %d "
                     "for service_id %d\n", __func__, xp->conn_id, xp->service_id);
        xp->client_id_valid = 0;
        qmi_cci_xport_event_remove_server(xp->clnt,xp->clnt->server_addr);
      }
      xp = xp->link.next;
    }
  }

  UNLOCK(&client_list_lock);
}

static void rx_cb
(
 qmi_connection_id_type  conn_id,
 qmi_service_id_type     service_id,
 qmi_client_id_type      client_id,
 unsigned char           control_flags,
 unsigned char           *rx_msg,
 int                     rx_msg_len)
{
  struct xport_handle *xp;

  if (client_id == 0xff)
  {
    LOCK(&client_list_lock);
    xp = LIST_HEAD(client_list);
    while(xp)
    {
      if(xp->conn_id == conn_id && xp->service_id == service_id && xp->client_id_valid)
      {
        QMI_CCI_LOGD("QMUXD: %s delivering broadcast message <%d:%d:%d>\n",
                      __func__, conn_id, service_id, xp->client_id);
        qmi_cci_xport_recv(xp->clnt, &xp->client_id, rx_msg, rx_msg_len);
      }
      xp = xp->link.next;
    }
    UNLOCK(&client_list_lock);
    return;
  }

  LOCK(&client_list_lock);
  xp = find_handle(conn_id, service_id, client_id);
  UNLOCK(&client_list_lock);
  if(!xp)
  {
    QMI_FW_LOGE("QMUXD: %s Unable to find <%d:%d:%d> combination\n",
                 __func__, conn_id, service_id, client_id);
    return;
  }

  QMI_CCI_LOGD("QMUXD: xport_rx[%d <- %d]: %d bytes on conn_id: %d\n",
                xp->client_id, xp->service_id, rx_msg_len, xp->conn_id);
  /* address is actually not used, but pass in a non-NULL pointer in case */
  qmi_cci_xport_recv(xp->clnt, &client_id, rx_msg, rx_msg_len);
}

static int qmuxd_ref_count_up(void)
{
  int rc;
  LOCK(&instances_count_lock);

  /* This is the first instance, initialize qmi_qmux_if
   * Wait to increment the ref count till after the call
   * succeeds. */
  if(!qmux_client_instances)
  {
    rc = qmi_qmux_if_pwr_up_init(rx_cb, sys_event_cb, NULL, &qmux_if_handle);
    if(QMI_NO_ERR != rc)
    {
      UNLOCK(&instances_count_lock);
      QMI_FW_LOGE("QMUXD: WARNING qmi_qmux_if_pwr_up_init failed! rc=%d\n", rc);
      return -1;
    }
  }
  qmux_client_instances++;
  UNLOCK(&instances_count_lock);
  return 0;
}

static void qmuxd_ref_count_down(void)
{
  LOCK(&instances_count_lock);
  if(qmux_client_instances > 0)
  {
    qmux_client_instances--;
    /* This is the last instance, we do not need qmux anymore */
    if(qmux_client_instances == 0)
    {
      qmi_qmux_if_pwr_down_release(qmux_if_handle);
      qmux_if_handle = QMI_QMUX_IF_INVALID_HNDL;
    }
  }
  UNLOCK(&instances_count_lock);
}

static void *xport_open
(
 void *xport_data,
 qmi_cci_client_type *clnt,
 uint32_t service_id,
 uint32_t version,
 void *addr,
 uint32_t max_rx_len
 )
{
  int rc, error;

  if(service_id > QMI_QMUX_MAX_SERVICE_ID)
    return NULL;

  struct xport_handle *xp = calloc(sizeof(struct xport_handle), 1);
  if(!xp)
  {
    QMI_FW_LOGE("QMUXD: Failed to allocate xport_handle\n");
    return NULL;
  }

  xp->clnt = clnt;

  /* conn_id, i.e. QMI_CONN_ID_RMNET_0, is passed in when xport is registered */
  xp->conn_id = ((qmi_connection_id_type)xport_data - QMI_CLIENT_QMUX_BASE_INSTANCE);
  xp->service_id = service_id;
  xp->max_rx_len = max_rx_len;
  xp->client_id = 0xff;
  xp->client_id_valid = 0;

  QMI_CCI_LOGD("QMUXD: %s[%d] for service_id=%d max_rx_len=%d\n",
                __func__, xp->conn_id, xp->service_id, max_rx_len);
  if(qmuxd_ref_count_up())
  {
    free(xp);
    return NULL;
  }

  if (addr)
  {
    rc = qmi_qmux_if_alloc_service_client(qmux_if_handle, xp->conn_id, service_id,
                                          &xp->client_id, &error);
    if(rc != QMI_NO_ERR)
    {
      QMI_FW_LOGE("QMUXD: WARNING qmi_qmux_if_alloc_service_client failed!"
                  " service_id = %d, conn_id = %d, rc=%d error = %d\n",
                   xp->service_id, xp->conn_id, rc, error);
      qmuxd_ref_count_down();
      free(xp);
      return NULL;
    }
    xp->client_id_valid = 1;
  }

  LOCK(&client_list_lock);
  LIST_ADD(client_list, xp, link);
  UNLOCK(&client_list_lock);
  QMI_CCI_LOGD("QMUXD: %s[%d] successful for service_id=%d max_rx_len=%d\n",
                __func__, xp->conn_id, xp->service_id, max_rx_len);
  return xp;
}

static qmi_client_error_type xport_send
(
 void *handle,
 void *addr,
 uint8_t *buf,
 uint32_t len
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  uint8_t *new_msg;
  int rc;

  QMI_CCI_LOGD("QMUXD: %s[%d -> %d]: %d bytes on conn_id=%d\n",
                __func__, xp->client_id, xp->service_id, len, xp->conn_id);
  if (!xp->client_id_valid)
  {
    QMI_FW_LOGE("QMUXD: %s on an invalid client id\n", __func__);
    return QMI_SERVICE_ERR;
  }

  /* create a new message with an empty header */
  new_msg = MALLOC(len + QMI_QMUX_IF_MSG_HDR_SIZE);
  if(!new_msg)
  {
    QMI_FW_LOGE("QMUXD: Unable to allocate buffer!\n");
    return QMI_INTERNAL_ERR;
  }

  memcpy(new_msg + QMI_QMUX_IF_MSG_HDR_SIZE, buf, len);

  rc = qmi_qmux_if_send_qmi_msg (qmux_if_handle,
                                 xp->conn_id,
                                 xp->service_id,
                                 xp->client_id,
      new_msg + QMI_QMUX_IF_MSG_HDR_SIZE, len);

  FREE(new_msg);
  return rc;
}

static void xport_close
(
 void *handle
 )
{
  struct xport_handle *xp = (struct xport_handle *)handle;
  int error;

  QMI_CCI_LOGD("QMUXD: %s[%d] service_id=%d, conn_id=%d\n",
                __func__, xp->conn_id, xp->service_id, xp->client_id);
  if (xp->client_id_valid)
  {
    xp->client_id_valid = 0;
    qmi_qmux_if_release_service_client(qmux_if_handle,
                                       xp->conn_id,
                                       xp->service_id,
                                       xp->client_id,
                                       &error);
    xp->client_id = 0;
  }


  qmuxd_ref_count_down();

  LOCK(&client_list_lock);
  LIST_REMOVE(client_list, xp, link);
  UNLOCK(&client_list_lock);
  QMI_CCI_LOGD("QMUXD: %s[%d] complete, service_id=%d, conn_id=%d\n",
                __func__, xp->conn_id, xp->service_id, xp->client_id);
  qmi_cci_xport_closed(xp->clnt);
  free(xp);
}


static uint32_t xport_lookup
(
 void *xport_data,
 uint8_t xport_num,
 uint32_t service_id,
 uint32_t version,
 uint32_t *num_entries,
 qmi_cci_service_info *service_info
)
{
  uint32_t rc;
  int qmi_err_code;
  unsigned short major_ver;
  unsigned short minor_ver;
  qmi_connection_id_type conn_id = ((qmi_connection_id_type)xport_data - QMI_CLIENT_QMUX_BASE_INSTANCE);

  if(service_id > QMI_QMUX_MAX_SERVICE_ID)
  {
    if (num_entries)
      *num_entries = 0;
    return 0;
  }

  if(qmuxd_ref_count_up())
  {
    if (num_entries)
      *num_entries = 0;
    return 0;
  }

  QMI_CCI_LOGD("QMUXD: %s for service_id=%d on conn_id=%d\n",
                __func__, service_id, conn_id);
  rc = qmi_qmux_if_get_version_list(qmux_if_handle,
                                    conn_id,
                                    service_id,
                                    &major_ver,
                                    &minor_ver,
                                    &qmi_err_code);
  if (rc == QMI_NO_ERR && qmi_err_code == QMI_SERVICE_ERR_NONE )
  {
    if (num_entries && service_info)
    {
      service_info[0].xport = xport_num;
      service_info[0].version = version;
      service_info[0].instance = (qmi_client_qmux_instance_type)xport_data;
      service_info[0].reserved = minor_ver;
      *num_entries = 1;
    }
    QMI_CCI_LOGD("QMUXD: Service_id=%d found over conn_id=%d\n",
                  service_id, conn_id);
    rc = 1;
  }
  else
  {
    if (num_entries)
      *num_entries = 0;
    QMI_CCI_LOGD("QMUXD: Service_id=%d not found over conn_id=%d, Register service notification\n",
                 service_id, conn_id);
    rc = qmi_qmux_if_reg_srvc_avail(qmux_if_handle,
                                    conn_id,
                                    service_id,
                                    &qmi_err_code);
    if (rc != QMI_NO_ERR)
      QMI_CCI_LOGD("QMUXD: Registration for service availability failed:%d\n", qmi_err_code);
    rc = 0;
  }
  qmuxd_ref_count_down();
  return rc;
}

static uint32_t xport_addr_len
(
 void
 )
{
  return 0;
}

qmi_cci_xport_ops_type qmuxd_ops =
{
  xport_open,
  xport_send,
  xport_close,
  xport_lookup,
  xport_addr_len
};
