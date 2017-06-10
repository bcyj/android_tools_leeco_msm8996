/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#ifdef DEBUG_X86
#include "comdef.h"
#endif
#include "gpsone_ctrl_msg.h"

int gpsone_client_connect(struct ctrl_msg_connect cmsg_connect);
int gpsone_client_disconnect(int server_socket_inet);

unsigned int sock_name(int server_sock);
unsigned int peer_name(int newclient_soc);
