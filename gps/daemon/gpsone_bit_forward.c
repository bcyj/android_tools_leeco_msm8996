/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012, 2013 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_bit_forward.h"
#include "gpsone_ctrl_msg.h"
#include "gpsone_glue_msg.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_conn_bridge_proc.h"
#include "gpsone_daemon_manager_handler.h"
#include "gpsone_udp_modem_proc.h"

#include <errno.h>
#include <arpa/inet.h> /* for inet_ntoa */
#include <netdb.h>     /* for gethostbyname */

static int daemon_manager_client_msgqid = -1;

static gpsone_bit_transport_handle_type transport_handle;
static gpsone_bit_session_handle_type global_session_count = 1;

extern const char * global_gpsone_ctrl_q_path;

/*===========================================================================
FUNCTION    gpsone_bit_forward_register

DESCRIPTION
   This function will register daemon to modem through RPC BIT API

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_bit_forward_register(void)
{
    gpsone_bit_status_e_type status;
    gpsone_bit_register_params_type reg_param = {"gpsone_daemon", GPSONE_BIT_TYPE_IP};

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    daemon_manager_client_msgqid = gpsone_glue_msgget(global_gpsone_ctrl_q_path, O_RDWR);

    GPSONE_DMN_DBG("%s:%d] transport_handle = %d\n", __func__, __LINE__, transport_handle);
    status = gpsone_bit_register(&reg_param, &transport_handle,
        gpsone_bit_forward_open,
        gpsone_bit_forward_close,
        gpsone_bit_forward_connect,
        gpsone_bit_forward_disconnect,
        gpsone_bit_forward_send,
        gpsone_bit_forward_recv,
        gpsone_bit_forward_ioctl);
    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, status = %d\n", __func__, __LINE__, transport_handle, status);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_deregister

DESCRIPTION
   This function will deregister daemon from modem through RPC BIT API

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_bit_forward_deregister(void)
{
    gpsone_bit_status_e_type status;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    gpsone_glue_piperemove(global_gpsone_ctrl_q_path, daemon_manager_client_msgqid);
    daemon_manager_client_msgqid = -1;

    status = gpsone_bit_deregister(transport_handle);
    GPSONE_DMN_DBG("%s:%d] status = %d\n", __func__, __LINE__, status);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_notify

DESCRIPTION
   This function sends notification to modem through RPC BIT API

   session_handle - unique session handle for each connection
   transaction_id - unique transaction id for each transaction
   event_payload  - event_payload to modem

DEPENDENCIES
   None

RETURN VALUE
   transaction_id

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_bit_forward_notify(gpsone_bit_session_handle_type session_handle, uint32_t transaction_id, gpsone_bit_event_payload_type * event_payload)
{
    gpsone_bit_status_e_type status = GPSONE_BIT_STATUS_SUCCESS;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    switch (event_payload->event) {
    case GPSONE_BIT_EVENT_OPEN_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY OPEN\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_CLOSE_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY CLOSE\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_DISCONNECT_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY DISCONNECT\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_CONNECT_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY CONNECT\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_SEND_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY SEND\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_DATA_READY:
        GPSONE_DMN_DBG("%s:%d] NOTIFY DATA\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_IOCTL_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY IOCTL\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_NETWORK_STATUS:
        GPSONE_DMN_DBG("%s:%d] NOTIFY NETWORK\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_NONE:
        GPSONE_DMN_DBG("%s:%d] NOTIFY NONE\n", __func__, __LINE__);
        break;

    default:
        GPSONE_DMN_DBG("%s:%d] NOTIFY default\n", __func__, __LINE__);
        break;
    }

    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, session_handle = %d, transaction_id = %d\n", __func__, __LINE__, transport_handle, session_handle, transaction_id);
    status = gpsone_bit_notify(transport_handle, session_handle, transaction_id, event_payload);
    GPSONE_DMN_DBG("%s:%d] status = %d\n", __func__, __LINE__, status);
    return transaction_id;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_open

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will send the open request to daemon manager

    transport_handle - unique handle for gpsone_daemon
    open_param       - open parameters

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_WAIT

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_open
(
  gpsone_bit_transport_handle_type    transport_handle,
  const gpsone_bit_open_params_type   *open_param
)
{
    struct ctrl_msgbuf cmsgbuf;

    cmsgbuf.session_handle = 0;
    cmsgbuf.transaction_id = 0;
    cmsgbuf.ctrl_type = GPSONE_BIT_OPEN;
    GPSONE_DMN_DBG("%s:%d] daemon_manager_client_msgqid = %d\n", __func__, __LINE__, daemon_manager_client_msgqid);
    gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));
    return  GPSONE_BIT_STATUS_WAIT;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_close

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will send the close request to daemon manager

    transport_handle - unique handle for gpsone_daemon
    close_param      - close parameters

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_WAIT

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_close
(
  gpsone_bit_transport_handle_type    transport_handle,
  const gpsone_bit_close_params_type  *close_param
)
{
    struct ctrl_msgbuf cmsgbuf;

    cmsgbuf.session_handle = 0;
    cmsgbuf.transaction_id = 0;
    cmsgbuf.ctrl_type = GPSONE_BIT_CLOSE;
GPSONE_DMN_DBG("%s:%d] daemon_manager_client_msgqid = %d\n", __func__, __LINE__, daemon_manager_client_msgqid);
    gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));
    return GPSONE_BIT_STATUS_WAIT;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_connect

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will send the connect request to daemon manager

   transport_handle - unique handle for gpsone_daemon
   transaction_id   - unique transaction id for the transaction
   connect_param    - connect parameters

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_WAIT

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_connect
(
  gpsone_bit_transport_handle_type    transport_handle,
  uint32                              transaction_id,
  const gpsone_bit_connect_params_type    *connect_param
)
{
    struct ctrl_msgbuf cmsgbuf;
    struct hostent *p_hostent;
    struct in6_addr v6_addr;
    struct in_addr v4_addr;

    memset(&cmsgbuf, 0, sizeof cmsgbuf);
    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, transaction_id = %d\n", __func__, __LINE__, transport_handle, (int) transaction_id);
    GPSONE_DMN_DBG("%s:%d] global_session_count = %d\n", __func__, __LINE__, (int) global_session_count);
    GPSONE_DMN_DBG("%s:%d] connecting to addr type: %d port: %d\n", __func__, __LINE__, (int) connect_param->adr_type, (int)connect_param->ip_port);

    cmsgbuf.session_handle = global_session_count ++;
    cmsgbuf.transaction_id = transaction_id;

    cmsgbuf.cmsg.cmsg_connect.is_supl = connect_param->protocol_type != GPSONE_BIT_PROTOCOL_TYPE_ANY? 1 : 0;
    cmsgbuf.cmsg.cmsg_connect.is_udp  = connect_param->link_type == GPSONE_BIT_LINK_UDP? 1 : 0;
    cmsgbuf.cmsg.cmsg_connect.ip_port = connect_param->ip_port;
    if ((int)connect_param->adr_type == GPSONE_BIT_HOST_NAME) {
        GPSONE_DMN_DBG("%s:%d] host name %s\n", __func__, __LINE__, connect_param->host_name);
        p_hostent = gethostbyname2(connect_param->host_name, AF_INET);
        if (p_hostent == NULL) {
            GPSONE_DMN_PR_ERR("%s:%d] gethostbyname2 for ip v4 returned null, trying ipv6!\n", __func__, __LINE__);
            p_hostent = gethostbyname2(connect_param->host_name, AF_INET6);
            if (p_hostent == NULL) {
                GPSONE_DMN_PR_ERR("%s:%d] gethostbyname2 for ip v6 returned null could not resolve hostname\n", __func__, __LINE__);
                return GPSONE_BIT_STATUS_FAIL;
            }
            cmsgbuf.cmsg.cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V6;
            GPSONE_DMN_DBG("%s:%d] gethostbyname returned 0x%x from %s\n", __func__, __LINE__, p_hostent->h_addr, connect_param->host_name);
            memcpy((void *)&cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v6_addr,
                    p_hostent->h_addr,
                    p_hostent->h_length);
            //TODO: do we convert the ipv6 to host byte order too??
        } else {
            cmsgbuf.cmsg.cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V4;
            GPSONE_DMN_DBG("%s:%d] gethostbyname returned 0x%x from %s\n", __func__, __LINE__, p_hostent->h_addr, connect_param->host_name);
            memcpy((void *)&cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr,
                    p_hostent->h_addr,
                    p_hostent->h_length);
#ifndef DEBUG_X86
            cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr = ntohl(cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr);
#endif
        }
    }
    else {
        uint32 ipv4_addr = connect_param->ip_addr.addr.v4_addr;
        GPSONE_DMN_DBG("%s:%d] v4_addr: %d.%d.%d.%d", __func__, __LINE__,
                 (unsigned char)(ipv4_addr>>24),
                 (unsigned char)(ipv4_addr>>16),
                 (unsigned char)(ipv4_addr>>8),
                 (unsigned char)(ipv4_addr));
        memcpy((void *)&cmsgbuf.cmsg.cmsg_connect.ip_addr,
               (void *)&connect_param->ip_addr,
               sizeof(gpsone_bit_ip_addr_type));
    }

    cmsgbuf.ctrl_type = GPSONE_BIT_CONNECT;

    gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));

    return GPSONE_BIT_STATUS_WAIT;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_disconnect

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will send the disconnect request to daemon manager

   transport_handle - unique handle for gpsone_daemon
   session_handle   - unique handle for the connection
   transaction_id   - unique transaction id for the transaction
   disconnect_param - disconnect parameters

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_WAIT

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_disconnect
(
  gpsone_bit_transport_handle_type  transport_handle,
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  const gpsone_bit_disconnect_params_type *disconnect_param
)
{
    struct ctrl_msgbuf cmsgbuf;

    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, session_handle = %d, transaction_id = %d\n", __func__, __LINE__, transport_handle, session_handle, (int) transaction_id);

    cmsgbuf.session_handle = session_handle;
    cmsgbuf.transaction_id = transaction_id;
    cmsgbuf.ctrl_type = GPSONE_BIT_DISCONNECT;

GPSONE_DMN_DBG("%s:%d] daemon_manager_client_msgqid = %d\n", __func__, __LINE__, daemon_manager_client_msgqid);
    gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));

    return GPSONE_BIT_STATUS_WAIT;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_send

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will send data to connection bridge

   transport_handle - unique handle for gpsone_daemon
   session_handle   - unique handle for the connection
   transaction_id   - unique transaction id for the transaction
   send_buf         - buffer with data to be sent
   length           - send_buf size

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_FAIL or GPSONE_BIT_STATUS_SUCCESS

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_send
(
  gpsone_bit_transport_handle_type  transport_handle,
  gpsone_bit_session_handle_type    session_handle,
  uint32                            transaction_id,
  uint8                             *send_buf,
  uint32                            length
)
{
    int result = -1;
    int fwd_tx_pipe;
    gpsone_bit_status_e_type rtn;
    void * conn_bridge_handle;

    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, session_handle = %d, transaction_id = %d\n", __func__, __LINE__, transport_handle, session_handle, (int) transaction_id);

    conn_bridge_handle = gpsone_daemon_manager_get_conn_handle(session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle 0x%x not found\n", __func__, __LINE__, session_handle);
        return GPSONE_BIT_STATUS_FAIL;
    }

    fwd_tx_pipe = ((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->fwd_tx_pipe;

    if (fwd_tx_pipe) {
        result = gpsone_glue_pipewrite(fwd_tx_pipe, send_buf, length);
      if (result < 0)
      {
        GPSONE_DMN_PR_ERR("%s:%d] pipewrite failed! reason: %s\n", __func__, __LINE__, strerror(errno));
      }
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] connection bridge not ready!\n", __func__, __LINE__);
    }

    GPSONE_DMN_DBG("%s:%d] fd = %d length = %d result = %d\n", __func__, __LINE__, fwd_tx_pipe, (int) length, result);
    if (result <= 0) {
        rtn = GPSONE_BIT_STATUS_FAIL;
        GPSONE_DMN_PR_ERR("%s:%d] tx failed: fd = %d length = %ld result = %d\n", __func__, __LINE__, fwd_tx_pipe, (long) length, result);
    } else {
        rtn = GPSONE_BIT_STATUS_SUCCESS;
        GPSONE_DMN_DBG("%s:%d] tx successed: fd = %d length = %ld result = %d\n", __func__, __LINE__, fwd_tx_pipe, (long) length, result);
    }

    return rtn;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_recv

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will receive data from connection bridge

   transport_handle - unique handle for gpsone_daemon
   session_handle   - unique handle for the connection
   recv_buf         - buffer to receive buffer
   max_buf_size     - recv_buf size
   bytes_returned   - number of bytes received upon success
   bytes_leftover   - number of bytes left in the pipe

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_FAIL or GPSONE_BIT_STATUS_SUCCESS

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_recv
(
  gpsone_bit_transport_handle_type   transport_handle,
  gpsone_bit_session_handle_type     session_handle,
  uint8                              *recv_buf,
  uint32                             max_buf_size,
  uint32                             *bytes_returned,
  uint32                             *bytes_leftover
)
{
    int result = -1;
    int fwd_rx_pipe;
    gpsone_bit_status_e_type rtn;
    void * conn_bridge_handle;

    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, session_handle = %d\n", __func__, __LINE__, transport_handle, session_handle);

    conn_bridge_handle = gpsone_daemon_manager_get_conn_handle(session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle 0x%x not found\n", __func__, __LINE__, session_handle);
        return GPSONE_BIT_STATUS_FAIL;
    }

    fwd_rx_pipe = ((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->fwd_rx_pipe;

    if (fwd_rx_pipe) {
        result = gpsone_glue_piperead(fwd_rx_pipe, recv_buf, max_buf_size);
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] connection bridge not ready!\n", __func__, __LINE__);
    }

    GPSONE_DMN_DBG("%s:%d] fd = %d buf_size = %d result = %d\n", __func__, __LINE__, fwd_rx_pipe, (int) max_buf_size, result);
    if (result >= 0) {
        rtn = GPSONE_BIT_STATUS_SUCCESS;
        *bytes_returned = result;
        if (ioctl(fwd_rx_pipe, FIONREAD, bytes_leftover) == -1)
            *bytes_leftover = 0;
    } else {
        if (errno == EAGAIN) {
            GPSONE_DMN_PR_ERR("%s:%d] error calling pipread EAGAIN\n", __func__, __LINE__);

            rtn = GPSONE_BIT_STATUS_NO_MORE_DATA; /* modem shall try again */
            *bytes_returned = 0;
            *bytes_leftover = 0;
        } else {
            GPSONE_DMN_PR_ERR("%s:%d] error calling pipread: %s\n", __func__, __LINE__, strerror(errno));

            rtn = GPSONE_BIT_STATUS_FAIL;
            *bytes_returned = result;
            *bytes_leftover = 0;
        }
    }

    return rtn;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_ioctl

DESCRIPTION
   This is a call back function for modem to call back to daemon. This
   function will send ioctl request to daemon manager

   transport_handle - unique handle for gpsone_daemon
   session_handle   - unique handle for the connection
   transaction_id   - unique id for each transaction
   ioctl_request    - ioctl request type
   ioctl_param      - ioctl request parameters

DEPENDENCIES
   None

RETURN VALUE
   GPSONE_BIT_STATUS_FAIL or GPSONE_BIT_STATUS_SUCCESS

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_status_e_type gpsone_bit_forward_ioctl
(
  gpsone_bit_transport_handle_type    transport_handle,
  gpsone_bit_session_handle_type      session_handle,
  uint32                              transaction_id,
  gpsone_bit_ioctl_e_type             ioctl_request,
  const gpsone_bit_ioctl_params_type  *ioctl_param
)
{
    int result;
    gpsone_bit_status_e_type rtn;
    struct ctrl_msgbuf cmsgbuf;

    GPSONE_DMN_DBG("%s:%d] transport_handle = %d, session_handle = %d, transaction_id = %d\n", __func__, __LINE__, transport_handle, session_handle, (int) transaction_id);

    cmsgbuf.session_handle = session_handle;
    cmsgbuf.transaction_id = transaction_id;
    cmsgbuf.cmsg.cmsg_ioctl.reserved = -1;
    switch(ioctl_request) {
    case GPSONE_BIT_IOCTL_FORCE_DORMANCY:
        cmsgbuf.ctrl_type = GPSONE_BIT_FORCE_DORMANCY;
        break;

    case GPSONE_BIT_IOCTL_UNFORCE_DORMANCY:
        cmsgbuf.ctrl_type = GPSONE_BIT_UNFORCE_DORMANCY;
        break;

    case GPSONE_BIT_IOCTL_GET_LOCAL_IP_ADDR:
        cmsgbuf.ctrl_type = GPSONE_BIT_GET_LOCAL_IP_ADDR;
        break;

    default:
        cmsgbuf.ctrl_type = GPSONE_INVALID;
        break;
    }

    if (cmsgbuf.ctrl_type != GPSONE_INVALID) {
        result = gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));
    } else {
        result = -1;
    }

    if (result < 0) {
        GPSONE_DMN_DBG("fail.");
        rtn = GPSONE_BIT_STATUS_FAIL;
    } else {
        GPSONE_DMN_DBG("received.");
        rtn = GPSONE_BIT_STATUS_WAIT;
    }

    return rtn;

}

