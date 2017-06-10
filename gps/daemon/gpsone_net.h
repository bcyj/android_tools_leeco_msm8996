/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "gpsone_ctrl_msg.h"

int gpsone_net_init(void);
int gpsone_net_release(void);
int gpsone_net_if_request(unsigned is_supl, unsigned long ipv4_addr, unsigned char ipv6_addr[16]);
int gpsone_net_if_release(unsigned is_supl, unsigned long ipv4_addr, unsigned char ipv6_addr[16]);
int gpsone_net_dsi_init(void);
int gpsone_net_dsi_release(void);
int gpsone_net_unforce_dormancy(void);
int gpsone_net_disable_dormancy_timer(void);
int gpsone_net_force_dormancy(void);
int gpsone_net_check_dormancy_status(void);
