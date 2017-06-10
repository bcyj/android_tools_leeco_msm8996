/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_ctrl_msg.h"
#include "gpsone_conn_bridge.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_glue_msg.h"
#include "gpsone_glue_data_service.h"
#include "gpsone_conn_client.h"
#include "gpsone_bit_forward.h"
#include "gpsone_daemon_manager_handler.h"

/*===========================================================================
FUNCTION    gpsone_conn_bridge_connect

DESCRIPTION
   This function will connect to a specified server

   p_conn_bridge_obj : pinter to a connection bridge object
   ctrl_msg_connect, sin_port: ip v4/v6 address union and port of the server
   rx_pipename       : pipe name for receiving
   tx_pipename       : pipe name for sending

DEPENDENCIES
   None

RETURN VALUE
   0 if success or anything else for a failure cause

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_conn_bridge_connect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj,
    struct ctrl_msg_connect    cmsg_connect,
    const char * rx_pipename, const char * tx_pipename)
{
    int result = 0;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    p_conn_bridge_obj->socket_inet = gpsone_client_connect(cmsg_connect);
    GPSONE_DMN_DBG("%s:%d]connect done, socket_inet = %d\n", __func__, __LINE__, p_conn_bridge_obj->socket_inet);

    if (p_conn_bridge_obj->socket_inet >= 0) {
        p_conn_bridge_obj->socket_inet_valid = 1;
        GPSONE_DMN_DBG("%s:%d] to create pipe %s\n", __func__, __LINE__, rx_pipename);
        p_conn_bridge_obj->rx_pipe = gpsone_glue_pipeget(rx_pipename, O_RDWR);
        p_conn_bridge_obj->fwd_rx_pipe = gpsone_glue_pipeget(rx_pipename, O_RDWR | O_NONBLOCK);

        GPSONE_DMN_DBG("%s:%d] to create pipe %s\n", __func__, __LINE__, tx_pipename);
        p_conn_bridge_obj->tx_pipe = gpsone_glue_pipeget(tx_pipename, O_RDWR | O_NONBLOCK);
        p_conn_bridge_obj->fwd_tx_pipe = gpsone_glue_pipeget(tx_pipename, O_RDWR);

        p_conn_bridge_obj->rx_bufsz = MAX_BUFFER;
        p_conn_bridge_obj->rx_cnt = 0;
        p_conn_bridge_obj->tx_bufsz = MAX_BUFFER;
        p_conn_bridge_obj->tx_cnt = 0;

    } else {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        result = -1;
    }

    GPSONE_DMN_DBG("%s:%d] result = %d\n", __func__, __LINE__, result);
    return result;
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_disconnect

DESCRIPTION
   This function will disconnect from server

   p_conn_bridge_obj : pinter to a connection bridge object
   rx_pipename       : pipe name for receiving
   tx_pipename       : pipe name for sending

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_conn_bridge_disconnect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj,
    const char * rx_pipename, const char * tx_pipename)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    gpsone_glue_piperemove(tx_pipename, p_conn_bridge_obj->tx_pipe);
    gpsone_glue_piperemove(tx_pipename, p_conn_bridge_obj->fwd_tx_pipe);
    gpsone_glue_piperemove(rx_pipename, p_conn_bridge_obj->rx_pipe);
    gpsone_glue_piperemove(rx_pipename, p_conn_bridge_obj->fwd_rx_pipe);
    gpsone_client_disconnect(p_conn_bridge_obj->socket_inet);
    p_conn_bridge_obj->socket_inet_valid = 0;
    return 0;
}

/*===========================================================================
FUNCTION    conn_bridge_tx

DESCRIPTION
   This function will send out data

   tx_pipe     - pipe that has the data to send out
   socket_inet - socket that is connected to server
   buf         - buffer that is used to temperarily store data
   size        - size of the buffer

DEPENDENCIES
   None

RETURN VALUE
   number of bytes sent out; 0 or -1 if failed.

SIDE EFFECTS
   N/A

===========================================================================*/
static int conn_bridge_tx(struct gpsone_conn_bridge_obj * p_conn_bridge_obj, int tx_pipe, int socket_inet, void * buf, int size)
{
    int result = 0;
    int len;
    len = gpsone_glue_piperead(tx_pipe, buf, size);
    GPSONE_DMN_DBG("%s:%d] socket_inet = %d, len = %d\n", __func__, __LINE__, socket_inet, len);

    if (len > 0 && p_conn_bridge_obj->unblock_flag != 1) {
        result = gpsone_write_glue(socket_inet, buf, len);
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] tx bridge broken?\n", __func__, __LINE__);
        GPSONE_DMN_PR_ERR("tx failed: %s\n", strerror(errno));
    }
    return result;
}

/*===========================================================================
FUNCTION    conn_bridge_rx

DESCRIPTION
   This function will receive data

   rx_pipe     - pipe that will hold data after receiving
   socket_inet - socket that is connected to server
   buf         - buffer that is used to temperarily store data
   size        - size of the buffer

DEPENDENCIES
   None

RETURN VALUE
   number of bytes received; 0 or -1 if failed.

SIDE EFFECTS
   N/A

===========================================================================*/
static int conn_bridge_rx(struct gpsone_conn_bridge_obj * p_conn_bridge_obj, int rx_pipe, int socket_inet, void * buf, int size)
{
    int len;
    len = gpsone_read_glue(socket_inet, buf, size);
    GPSONE_DMN_DBG("%s:%d] socket_inet = %d, len = %d\n", __func__, __LINE__, socket_inet, len);
    if (len > 0) {
        gpsone_glue_pipewrite(rx_pipe, buf, len);
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] rx socket %d read %d\n", __func__, __LINE__, socket_inet, len);
        GPSONE_DMN_PR_ERR("rx failed: %s\n", strerror(errno));
    }
    return len;
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_task

DESCRIPTION
   This is the task loop for connection bridge.

    p_conn_bridge_obj - pointer to the connection bridge instance.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_conn_bridge_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
    gpsone_bit_event_payload_type event_payload;
    int result;
    fd_set fds, xfds;
    int nfds = 0;

    struct timeval timeout;
    timeout.tv_usec = 0;
    timeout.tv_sec = 10;

    FD_ZERO(&fds);
    FD_ZERO(&xfds);
    if (p_conn_bridge_obj->socket_inet_valid && p_conn_bridge_obj->socket_inet > 0) {
        FD_SET(p_conn_bridge_obj->socket_inet, &fds);
        FD_SET(p_conn_bridge_obj->socket_inet, &xfds);
        nfds = p_conn_bridge_obj->socket_inet > nfds ?
               p_conn_bridge_obj->socket_inet : nfds;
    }
    if (p_conn_bridge_obj->tx_pipe > 0) {
        FD_SET(p_conn_bridge_obj->tx_pipe, &fds);
        FD_SET(p_conn_bridge_obj->tx_pipe, &xfds);
        nfds = p_conn_bridge_obj->tx_pipe > nfds ?
               p_conn_bridge_obj->tx_pipe : nfds;
    }

    GPSONE_DMN_DBG("%s:%d - %d] fd = %d, %d, waiting msg...\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj), p_conn_bridge_obj->socket_inet, p_conn_bridge_obj->tx_pipe);
    result = select(nfds + 1, &fds, NULL, &xfds, &timeout );
    GPSONE_DMN_DBG("%s:%d - %d] %lx unblock = %d\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj), (long) p_conn_bridge_obj, p_conn_bridge_obj->unblock_flag);
    if (result == 0) {
        /* timeout */
        GPSONE_DMN_DBG("%s:%d] select timeout?\n", __func__, __LINE__);
    }
    else if (result < 0) {
        GPSONE_DMN_DBG("%s:%d] select error?%s \n", __func__, __LINE__, strerror(errno));
    }
    else if (p_conn_bridge_obj->unblock_flag != 1) {
        GPSONE_DMN_DBG("%s:%d - %d] select returns %d\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj), result);
        if (p_conn_bridge_obj->socket_inet_valid && FD_ISSET(p_conn_bridge_obj->socket_inet, &fds)) {
            /* rx */
            result = conn_bridge_rx(p_conn_bridge_obj, p_conn_bridge_obj->rx_pipe, p_conn_bridge_obj->socket_inet,
                p_conn_bridge_obj->rx_buf, p_conn_bridge_obj->rx_bufsz);

            if (result > 0) {
                p_conn_bridge_obj->rx_cnt += result;
                GPSONE_DMN_MSG("%s:%d] rx received %d, total %ld\n", __func__, __LINE__, result, p_conn_bridge_obj->rx_cnt);

                event_payload.arg.bytes_available = result;
                event_payload.result = GPSONE_BIT_STATUS_SUCCESS;
                event_payload.event = GPSONE_BIT_EVENT_DATA_READY;
                gpsone_bit_forward_notify(gpsone_daemon_manager_get_session_handle(p_conn_bridge_obj), 0, &event_payload);
            } else if (result == 0) {
                GPSONE_DMN_PR_ERR("%s:%d] rx no data\n", __func__, __LINE__);

                p_conn_bridge_obj->socket_inet_valid = 0;
            } else {
                GPSONE_DMN_PR_ERR("%s:%d] rx failed, result = %d\n", __func__, __LINE__, result);

                event_payload.arg.bytes_available = -1;
                event_payload.result = GPSONE_BIT_STATUS_FAIL;
                event_payload.event = GPSONE_BIT_EVENT_DATA_READY;
                gpsone_bit_forward_notify(gpsone_daemon_manager_get_session_handle(p_conn_bridge_obj), 0, &event_payload);
            }
        }

        if (FD_ISSET(p_conn_bridge_obj->tx_pipe, &fds)) {
            /* tx */
            result = conn_bridge_tx(p_conn_bridge_obj, p_conn_bridge_obj->tx_pipe, p_conn_bridge_obj->socket_inet,
                p_conn_bridge_obj->tx_buf, p_conn_bridge_obj->tx_bufsz);
            if (result < 0) {
                GPSONE_DMN_PR_ERR("%s:%d] tx failed, result = %d\n", __func__, __LINE__, result);
            } else if (result == 0) {
                GPSONE_DMN_PR_ERR("%s:%d] tx failed\n", __func__, __LINE__);
            } else {
                p_conn_bridge_obj->socket_inet_valid = 1;
                p_conn_bridge_obj->tx_cnt += result;
                GPSONE_DMN_MSG("%s:%d] tx sent %d, total %ld\n", __func__, __LINE__, result, p_conn_bridge_obj->tx_cnt);
            }
        }
    } else {
        result = 0;
    }

    GPSONE_DMN_DBG("%s:%d - %d] done\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));

    if (result < 0 ) {
        return -1;
    } else {
        return 0;
    }
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_unblock_task

DESCRIPTION
   This function will unblock the connection bridge task by inject a char to
   the tx pipe.

    p_conn_bridge_obj - pointer to the connection bridge instance

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_conn_bridge_unblock_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
    int len;
    GPSONE_DMN_DBG("%s:%d - %d] %lx unblock = %d\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj), (long) p_conn_bridge_obj, p_conn_bridge_obj->unblock_flag);
    p_conn_bridge_obj->unblock_flag = 1;
    /* @todo this might be written to server; need to create another fd for ctrl */
    len = gpsone_glue_pipewrite(p_conn_bridge_obj->fwd_tx_pipe, "\n", 2);
    return len;
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_ip_addr_task

DESCRIPTION
   This function will return the server ip address associated with the connection

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
unsigned int gpsone_conn_bridge_ip_addr_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    return sock_name(p_conn_bridge_obj->socket_inet);
}
