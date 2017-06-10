/* Copyright (c) 2010, Q
   Incorporated.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/socket.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_ctrl_msg.h"
#include "gpsone_conn_bridge.h"
#include "gpsone_conn_bridge_proc.h"
#include "gpsone_udp_modem.h"
#include "gpsone_udp_modem_proc.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_glue_msg.h"
#include "gpsone_glue_data_service.h"
#include "gpsone_conn_client.h"
#include "gpsone_bit_forward.h"
#include "gpsone_daemon_manager_handler.h"

int write_nbytes(int fd, void *buf, int nbytes);
FILE *ppm_enabled;

/*===========================================================================
FUNCTION    gpsone_udp_modem_connect

DESCRIPTION
   This function will set up a listening fd for udp modem to be a server on
   and sets up the named pipes for communication with daemon manager.

   p_conn_bridge_obj : pointer to the connection bridge object
   sin_addr, sin_port : ip address and port of the server

DEPENDENCIES
   None

RETURN VALUE
   0: success, non-zero for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_udp_modem_connect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj,
                             unsigned short sin_port)
{
    struct sockaddr_in si_me;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    p_conn_bridge_obj->socket_inet = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (p_conn_bridge_obj->socket_inet < 0) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        return -1;
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(sin_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(p_conn_bridge_obj->socket_inet, (const struct sockaddr *)&si_me, sizeof(si_me)) < 0) {
      GPSONE_DMN_DBG("bind failed!\n");
      return -1;
    }

    GPSONE_DMN_DBG("%s:%d\n", __func__, __LINE__);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_udp_modem_disconnect

DESCRIPTION
  UDP modem disconnect

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_udp_modem_disconnect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    close(p_conn_bridge_obj->socket_inet);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_udp_modem_task

DESCRIPTION
   This function gets run over and over as long as the return value is 0.
   it is the main task for udp_modem thread.
   block until a client connects and call server task

   p_conn_bridge_obj : pointer to connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   0: always

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_udp_modem_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
    int result = 0;
    gpsone_bit_event_payload_type event_payload;

    GPSONE_DMN_DBG("%s:%d] waiting for a udp packet. blocking..\n", __func__, __LINE__);
    result = recv(p_conn_bridge_obj->socket_inet,
                  p_conn_bridge_obj->rx_buf,
                   p_conn_bridge_obj->rx_bufsz,
                   0);
    if (result < 0) {
      GPSONE_DMN_PR_ERR("%s:%d] recvfrom failed! %s\n", __func__, __LINE__, strerror(errno));
      return -1;
    } else {
      GPSONE_DMN_DBG("%s:%d] received this data %s from udp client! size is %d\n", __func__, __LINE__, p_conn_bridge_obj->rx_buf, result);
      gpsone_glue_pipewrite(p_conn_bridge_obj->rx_pipe,
                            p_conn_bridge_obj->rx_buf,
                            result);
      GPSONE_DMN_DBG("%s:%d] done writing to the pipe, about to notify modem data ready\n", __func__, __LINE__);
      event_payload.result = GPSONE_BIT_STATUS_SUCCESS;
      event_payload.event = GPSONE_BIT_EVENT_DATA_READY;
      event_payload.arg.bytes_available = result;
      gpsone_bit_forward_notify(gpsone_daemon_manager_get_session_handle(p_conn_bridge_obj), 2, &event_payload);
      GPSONE_DMN_DBG("%s:%d] done notifyng modem data ready\n", __func__, __LINE__);
    }
    GPSONE_DMN_DBG("%s:%d] done\n", __func__, __LINE__);
    GPSONE_DMN_DBG("result of udp_modem_proc was : %d\n", result);
    return 0;
}

/*===========================================================================
FUNCTION   gpsone_udp_modem_unblock_task

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
int gpsone_udp_modem_unblock_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
    int len;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    p_conn_bridge_obj->unblock_flag = 1;
    len = gpsone_glue_pipewrite(p_conn_bridge_obj->fwd_tx_pipe, "\n", 2);
    return len;
}

/*===========================================================================
FUNCTION   udp_pipe_connect

DESCRIPTION
   This function will connect the pipes used in UDP thread

   p_conn_bridge_obj - pointer to the connection bridge instance


DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int udp_pipe_connect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
  GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
  p_conn_bridge_obj->rx_bufsz = MAX_BUFFER;
  p_conn_bridge_obj->tx_bufsz = MAX_BUFFER;
  p_conn_bridge_obj->rx_pipe = gpsone_glue_pipeget(UDP_RX_PIPENAME, O_RDWR);
  p_conn_bridge_obj->fwd_rx_pipe = gpsone_glue_pipeget(UDP_RX_PIPENAME, O_RDWR | O_NONBLOCK );
  GPSONE_DMN_DBG("%s:%d] to create pipe %s with fd=%d\n", __func__, __LINE__, UDP_RX_PIPENAME, p_conn_bridge_obj->rx_pipe);
  p_conn_bridge_obj->tx_pipe = gpsone_glue_pipeget(UDP_TX_PIPENAME, O_RDWR | O_NONBLOCK);
  p_conn_bridge_obj->fwd_tx_pipe = gpsone_glue_pipeget(UDP_TX_PIPENAME, O_RDWR);
  GPSONE_DMN_DBG("%s:%d] to create pipe %s with fd=%d\n", __func__, __LINE__, UDP_TX_PIPENAME, p_conn_bridge_obj->tx_pipe);

  return 0;
}

/*===========================================================================
FUNCTION   udp_pipe_disconnect

DESCRIPTION
   This function will disconnect the pipes used in UDP thread

   p_conn_bridge_obj - pointer to the connection bridge instance


DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int udp_pipe_disconnect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj)
{
  GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
  gpsone_glue_piperemove(UDP_TX_PIPENAME, p_conn_bridge_obj->tx_pipe);
  gpsone_glue_piperemove(UDP_TX_PIPENAME, p_conn_bridge_obj->fwd_tx_pipe);
  gpsone_glue_piperemove(UDP_RX_PIPENAME, p_conn_bridge_obj->rx_pipe);
  gpsone_glue_piperemove(UDP_RX_PIPENAME, p_conn_bridge_obj->fwd_rx_pipe);
  return 0;
}
