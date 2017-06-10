/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef _GPSONE_UDP_MODEM_PROC_H_
#define _GPSONE_UDP_MODEM_PROC_H_

#include <linux/types.h>

#include "gpsone_ctrl_msg.h"
#include "gpsone_conn_bridge.h"

int gpsone_udp_modem_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
int gpsone_udp_modem_unblock_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
int gpsone_udp_modem_connect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj,
                             unsigned short sin_port);

int gpsone_udp_modem_disconnect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
int udp_pipe_connect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
int udp_pipe_disconnect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);

#endif /* _GPSONE_UDP_MODEM_PROC_H_ */
