/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <linux/types.h>

#include "gpsone_ctrl_msg.h"

int gpsone_daemon_manager_register(void);
int gpsone_daemon_manager_deregister(void);

gpsone_bit_session_handle_type gpsone_daemon_manager_get_session_handle(void * conn_bridge_handle);
int gpsone_daemon_manager_get_session_surfix(void * conn_bridge_handle);
int gpsone_daemon_manager_add_conn_handle(void * conn_bridge_handle);
void * gpsone_daemon_manager_get_conn_handle(gpsone_bit_session_handle_type session_handle);

int gpsone_daemon_manager_if_up_handler(struct ctrl_msgbuf *pmsg, int len);
int gpsone_daemon_manager_force_dormancy_handler(struct ctrl_msgbuf *pmsg, int len);
int gpsone_daemon_manager_unforce_dormancy_handler(struct ctrl_msgbuf *pmsg, int len);
int gpsone_daemon_manager_dormancy_status_handler(struct ctrl_msgbuf *pmsg, int len);
int gpsone_daemon_manager_get_local_ip_addr_handler(struct ctrl_msgbuf *pmsg, int len);

int gpsone_daemon_manager_open_handler(struct ctrl_msgbuf *pmsg, int len);
int gpsone_daemon_manager_close_handler(struct ctrl_msgbuf *pmsg, int len);

int gpsone_daemon_manager_connect_handler(struct ctrl_msgbuf *pmsg, int len);
int gpsone_daemon_manager_disconnect_handler(struct ctrl_msgbuf *pmsg, int len);

