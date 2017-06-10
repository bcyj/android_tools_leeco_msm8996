/* Copyright (c) 2010, 2014, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __GPSONE_UDP_MODEM_H__
#define __GPSONE_UDP_MODEM_H__

#include "gpsone_ctrl_msg.h"
#include "gpsone_thread_helper.h"

#ifdef _ANDROID_
#define UDP_RX_PIPENAME "/data/misc/location/gpsone_d/gpsone_udp_pipe_rx"
#define UDP_TX_PIPENAME "/data/misc/location/gpsone_d/gpsone_udp_pipe_tx"
#else
#define UDP_RX_PIPENAME "/tmp/gpsone_udp_pipe_rx"
#define UDP_TX_PIPENAME "/tmp/gpsone_udp_pipe_tx"
#endif


void * gpsone_udp_modem_create(struct ctrl_msgbuf *pmsg, int len);
int gpsone_udp_modem_destroy(void * connection_bridge_handle);

#endif /* __GPSONE_UDP_MODEM_H__ */
