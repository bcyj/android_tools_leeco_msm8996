/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef DEBUG_X86
#include "comdef.h"
#endif
#include "gpsone_ctrl_msg.h"

int gpsone_write_glue(int socket_inet, const void * text, int size);
int gpsone_read_glue(int socket_inet, void * text, int size);

int gpsone_connect_glue(struct ctrl_msg_connect cmsg_connect);
int gpsone_disconnect_glue(int server_socket_inet);

int gpsone_set_ip_route(struct in_addr sin_addr);
unsigned int get_sock_name(int server_sock);
unsigned int get_peer_name(int newclient_soc);


