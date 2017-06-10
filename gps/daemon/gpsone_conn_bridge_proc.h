/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <linux/types.h>

#include "gpsone_ctrl_msg.h"
#include "gpsone_conn_bridge.h"

int gpsone_conn_bridge_connect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj,
    struct ctrl_msg_connect    cmsg_connect,
    const char * rx_pipename, const char * tx_pipename);
int gpsone_conn_bridge_disconnect(struct gpsone_conn_bridge_obj * p_conn_bridge_obj,
    const char * rx_pipename, const char * tx_pipename);

int gpsone_conn_bridge_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
int gpsone_conn_bridge_unblock_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
unsigned int gpsone_conn_bridge_ip_addr_task(struct gpsone_conn_bridge_obj * p_conn_bridge_obj);
