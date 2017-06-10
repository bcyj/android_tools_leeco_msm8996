/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
int gpsone_socket_server_setup(const char * const server_ip_str,
                               unsigned short server_port);
int gpsone_socket_server_manager(const char * const server_ip_str, const char * const server_port_str,
    int (*server_loop) (int client_socket));

