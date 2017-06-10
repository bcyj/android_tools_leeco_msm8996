/*
 * Copyright (c) 2009 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *
 * The client-server is target independent. The testsuite server can be compiled
 * without camera using the command below for non-arm target,
 *
 * gcc -Wall -DTESTSUITE_SERVER_ONLY testsuite_server.c testsuite_utilities.c -o testsuite_server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testsuite.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/time.h>

#define TS_PRINT_HHMMSS()                                      \
{                                                              \
  struct timeval tv;                                           \
  struct timezone tz;                                          \
  int hh, mm, ss;                                              \
  gettimeofday(&tv, &tz);                                      \
  hh = tv.tv_sec/3600%24;                                      \
  mm = (tv.tv_sec%3600)/60;                                    \
  ss = tv.tv_sec%60;                                           \
  printf("%02d:%02d:%02d.%06ld\n", hh, mm, ss, tv.tv_usec);    \
}

int command_dispatcher(TS_CMD_ID command_id, char * args, char * result_str);

/*
 * result:  0 - it is a control test command, not camera test command
 *          1 - it is camera test command, and test success
 *         -1 - it is camera test command, and test failed
 */
int testsuite_server_command_dispatcher(TS_CMD_ID command_id, char * args, char * result_str)
{
    int result = 0;
    int test_result;
    long ms, sec, us_diff;
    static int test_count = 0;
    int test_result_expected;
    static struct timeval tv1;
    struct timeval tv2;
    struct timezone tz;

    switch(command_id)
    {
        case TS_MSLEEP:
            ms = 1000L * atol(args);
            usleep(ms);
            break;

        case TS_SLEEP:
           sec = atol(args);
           sleep(sec);
           break;

        case TS_COUNT:
            test_count++;
            printf("%5d] ---\n", test_count);
            break;

        case TS_COUNT_RESET:
            test_count = 0;
            break;

        case TS_TIME:
            TS_PRINT_HHMMSS();
            break;

        case TS_TIME_MARKER:
            TS_PRINT_HHMMSS();
            gettimeofday(&tv1, &tz);
            break;

        case TS_TIME_SINCE_MARKER:
            gettimeofday(&tv2, &tz);
            us_diff = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
            printf("%15ld\n", us_diff);
            TS_PRINT_HHMMSS();
            break;

        case TS_SERVER_EXIT:
            break;

        default:
#ifndef TESTSUITE_SERVER_ONLY
            test_result = command_dispatcher(command_id, args, result_str);
#else
            test_result = -1;
#endif
            if (result_str != NULL) {
                test_result_expected = atoi(result_str);
            }
            else {
                // check default
                test_result_expected = 0;
            }

            if (test_result != test_result_expected) {
                result = -1;
            }
            else {
                result = 1;
            }
            break;
    }

    return result;
}

int testsuite_server(int client_socket)
{
    TS_CMD_ID command_id = TS_CMD_ID_INVALID;
    int test_count = 0;
    int failure_count = 0;
    char * p_command_args;
    char * p_command_result;
    char * text = NULL;
    int result = 0;

    printf("%s starts\n", __FUNCTION__);

    do {
        result = testsuite_receive(client_socket, & text);
        if (result < 0) {
            if (command_id != TS_CLIENT_EXIT && command_id != TS_SERVER_EXIT) {
                printf("%s: fail on the connection.\n", __func__);
                command_id = TS_CLIENT_EXIT;
            } else {
                printf("%s: client connection closed.\n", __func__);
            }
            break;
        }

        if (!text) {
          printf("%s: text not allocated.\n", __func__);
          break;
        }

        command_id = parse_command( text, & p_command_args, & p_command_result);

        if ((command_id < 0) || (p_command_args == NULL)) {
            printf("Invalid command received\n");
            continue;
        }

        printf("received command: %s, args:%s, expected result:%s\n",
                text, p_command_args, p_command_result);

        result = testsuite_server_command_dispatcher(command_id, p_command_args, p_command_result);
        failure_count += result == -1 ? 1 : 0;
        test_count += result != 0? 1 : 0;

        printf("                  executing result = %d\n", result);
        free (text);

        if (result == -1) {
            result = testsuite_send(client_socket, "FAILED");
        } else {
            result = testsuite_send(client_socket, "OK");
        }
        if (result < 0) {
            printf("%s: fail on the connection.\n", __func__);
            command_id = TS_CLIENT_EXIT;
            break;
        }
    } while(1);
    /* do not exit until client close the socket, to avoid connection waiting for timeout
     * } while(command_id != TS_SERVER_EXIT && command_id != TS_CLIENT_EXIT);
     */

    printf("%s: %d test cases executed, failures %d\n\n", __FUNCTION__, test_count, failure_count);
    return command_id;
}

int testsuite_server_setup(const char * const server_ip_str, const char * const server_port_str)
{
    int socket_inet = -1;
    struct sockaddr_in addr_inet;
    int len_inet;
    int port = atoi(server_port_str);
    int result;
    int option;

    memset(&addr_inet, 0, sizeof(addr_inet));
    addr_inet.sin_family = AF_INET;
    addr_inet.sin_port= htons(port);
    addr_inet.sin_addr.s_addr = inet_addr(server_ip_str);
    len_inet = sizeof(addr_inet);

    if (addr_inet.sin_addr.s_addr == INADDR_NONE) {
        printf("%s invalid address.\n", __func__);
        goto fail;
    }

    socket_inet = socket (AF_INET, SOCK_STREAM, 0);
    if (socket_inet < 0) {
        printf("%s socket failed\n", __func__);
        goto fail;
    }

    /* reuse in case it is in timeout */
    option = 1;
    result = setsockopt(socket_inet, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (result < 0)
    {
        printf("%s result %d\n", __func__, result);
        perror("setsockopt failed");
        goto fail2;
    }

    printf("socket_inet = %d, len_inet = %d\n", socket_inet, len_inet);
    result = bind (socket_inet, (const struct sockaddr *) &addr_inet, len_inet);
    if (result != 0) {
        printf("%s result %d.\n", __func__, result);
        perror("bind failed");
        goto fail2;
    }

    result = listen (socket_inet, 5);
    if (result != 0) {
        printf("%s result %d.\n", __func__, result);
        perror("listen failed");
        goto fail2;
    }
    return socket_inet;

fail2:
    close(socket_inet);
fail:
    return -1;
}

int testsuite_server_manager(const char * const server_ip_str, const char * const server_port_str)
{
    int count = 0;
    int server_socket_inet;
    struct sockaddr_in addr_client_inet;
    socklen_t addr_client_len;
    int command_id;

    server_socket_inet = testsuite_server_setup(server_ip_str, server_port_str);
    if (server_socket_inet < 0) {
        return server_socket_inet;
    }

    do {
        int client_socket_inet;
        printf("%s: listening on server_socket_inet=%d ...\n", __func__, server_socket_inet);
        client_socket_inet = accept(server_socket_inet, (struct sockaddr *) &addr_client_inet, &addr_client_len);
        count ++;
        printf("%s: accepted connection client_socket_inet=%d\n", __func__, client_socket_inet);
        command_id = testsuite_server (client_socket_inet);
        if (close (client_socket_inet ) < 0) {
            printf("%s: connection %d close client_socket_inet=%d failed\n", __func__, count, client_socket_inet);
        } else {
            printf("%s: connection %d close client_socket_inet=%d success\n", __func__, count, client_socket_inet);
        }
    } while (command_id != TS_SERVER_EXIT);

    /* Remove the socket file. */
    if (close (server_socket_inet) < 0) {
        printf("%s: close server_socket_inet=%d failed\n", __func__, server_socket_inet);
    } else {
        printf("%s: close server_socket_inet=%d success\n", __func__, server_socket_inet);
    }
    return 0;
}

/* 
 * testsuite_server_main
 *
 */
#ifdef TESTSUITE_SERVER_ONLY
int main(int argc, char * argv[])
{
    int result;
    char * server_ip = "127.0.0.1";
    char * server_port = "9009";
    int opt;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "a:p:h")) != -1) {
      switch(opt) {
        case 'a':
          server_ip = optarg;
          break;
  
        case 'p':
          server_port = optarg;
          break;

        case 'h':
        default:
          /*  help in main.c covers this already */
          /*
          printf("usage: %s [-a <ip address>] [-p <port>]\n", argv[0]);
          printf("-a:  server ip address. By default it is 127.0.0.1\n");
          printf("-p:  server port number. By default it is 9009\n");
          printf("-h:  print this help\n\n");
          printf("run adb port forwarding for android target:\n");
          printf("    adb forward tcp:9009 tcp:9009\n");
          */
          return 0;
      }
    }
    printf("server_ip: %s\n", server_ip);
    printf("server_port: %s\n", server_port);
    result = testsuite_server_manager(server_ip, server_port);

    return result;
}
#endif

