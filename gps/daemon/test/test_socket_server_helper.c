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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <errno.h>

#include "test_socket_server_helper.h"
#include "test_script_parser.h"

#include "test_dbg.h"

/*===========================================================================
FUNCTION    gpsone_socket_server_setup

DESCRIPTION
  set up a listening server socket/fd
  NOTE: you have to use htons if you want to have a Java client connect

  server_ip_str : ip address on which the server will be listening
  server_port : port on which the server will be listening

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_socket_server_setup(const char * const server_ip_str,
                               unsigned short server_port)
{
  struct sockaddr_in addr_inet;
  int socket_inet;
  int result = 0;
  int option = 1;

  socket_inet = socket (AF_INET, SOCK_STREAM, 0);
  if (socket_inet < 0) {
      GPSONE_DMN_DBG("%s socket failed\n", __func__);
      goto fail;
  }

  result = setsockopt(socket_inet, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (result < 0)
  {
      GPSONE_DMN_DBG("%s result %d\n", __func__, result);
      perror("setsockopt failed");
      goto fail2;
  }

  memset(&addr_inet, 0, sizeof(addr_inet));
  addr_inet.sin_family = AF_INET;
  addr_inet.sin_addr.s_addr = inet_addr(server_ip_str);
  //addr_inet.sin_port = htons(server_port);   if try to connect in JAVA
  addr_inet.sin_port = server_port;
  result = bind (socket_inet, (struct sockaddr *) &addr_inet, sizeof(struct sockaddr_in));
  if (result != 0) {
      GPSONE_DMN_DBG("%s result %d.\n", __func__, result);
      perror("bind failed");
      goto fail2;
  }

  GPSONE_DMN_DBG("%s:%d] connect to %s:%d\n", __func__, __LINE__, inet_ntoa(addr_inet.sin_addr), server_port);

  result = listen (socket_inet, 5);
  if (result != 0) {
      GPSONE_DMN_DBG("%s result %d.\n", __func__, result);
      perror("listen failed");
      goto fail2;
  }

  return socket_inet;

fail2:
    close(socket_inet);
fail:
    return -1;

}

int gpsone_socket_server_manager(const char * const server_ip_str, const char * const server_port_str,
    int (*server_loop) (int client_socket))
{
    int count = 0;
    int server_socket_inet;
    struct sockaddr_in addr_client_inet;
    socklen_t addr_client_len;
    int command_id;

    server_socket_inet = gpsone_socket_server_setup(server_ip_str, (unsigned short) atoi(server_port_str));
    if (server_socket_inet < 0) {
        return server_socket_inet;
    }

    do {
        int client_socket_inet;
        TEST_DBG("listening on %s:%s...\n", server_ip_str, server_port_str);
        client_socket_inet = accept(server_socket_inet, (struct sockaddr *) &addr_client_inet, &addr_client_len);
        count ++;
        if (client_socket_inet >= 0) {
            TEST_DBG("connection %d accepted, client_socket_inet=%d\n", count, client_socket_inet);
            command_id = server_loop(client_socket_inet);
        }
        else {
            TEST_DBG("connection %d failed: %s, client_socket_inet=%d\n", count, strerror(errno), client_socket_inet);
        }

        if (close (client_socket_inet ) < 0) {
            TEST_DBG("connection %d close failed, client_socket_inet=%d\n", count, client_socket_inet);
        } else {
            TEST_DBG("connection %d close success, client_socket_inet=%d\n", count, client_socket_inet);
        }
    } while (command_id != TS_SERVER_EXIT);

    /* Remove the socket file. */
    if (close (server_socket_inet) < 0) {
        TEST_DBG("server close failed\n");
    } else {
        TEST_DBG("server close success\n");
    }
    return 0;
}
