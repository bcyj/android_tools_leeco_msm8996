/* 
 * Copyright (c) 2009 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *
 * The client-server is target independent. The testsuite client can be compiled
 * using the command below for non-arm target,
 *
 * gcc -Wall testsuite_client.c testsuite_utilities.c -o mm-qcamera-testsuite-client
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testsuite.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

extern char *optarg;

int testsuite_client_connect(const char * const server_ip_str, const char * const server_port_str)
{
    struct sockaddr_in addr_inet;
    int len_inet;
    int result;
    int port = atoi(server_port_str);
    int server_socket_inet;

    memset(&addr_inet, 0, sizeof(addr_inet));
    addr_inet.sin_family = AF_INET;
    addr_inet.sin_port= htons(port);
    addr_inet.sin_addr.s_addr = inet_addr(server_ip_str);
    len_inet = sizeof(addr_inet);

    if (addr_inet.sin_addr.s_addr == INADDR_NONE)
        printf("%s invalid address.\n", __func__);

    server_socket_inet = socket (AF_INET, SOCK_STREAM, 0);
    if (server_socket_inet < 0)
        printf("%s socket failed\n", __func__);

    printf("server_socket_inet = %d, len_inet = %d\n", server_socket_inet, len_inet);
    result = connect(server_socket_inet, (const struct sockaddr *) &addr_inet, len_inet);
    if (result < 0) {
        printf("%s result = %d.\n", __func__, result);
        perror("connect failed");
        server_socket_inet = -1;
    }
    return server_socket_inet;
}

int testsuite_client_disconnect(int server_socket_inet)
{
    if (close(server_socket_inet) <0) {
        printf("close server_socket_inet=%d failed\n", server_socket_inet);
    } else {
        printf("close server_socket_inet=%d successed\n", server_socket_inet);
    }
    return 0;
}

int main(int argc, char *argv[])
{
  int command_id = 0;
  char test_command_filename[TS_CMD_FILENAME_MAX + 1];
  char test_command_buf[TS_CMD_STR_MAX + 1];
  char temp_buf[TS_CMD_STR_MAX + 1];
  char * receive_result_ptr;
  char * server_ip = "127.0.0.1";
  char * server_port = "9009";
  char * p_args, * p_result;
  int result;
  int loop_cnt = 0;
  int opt;
  int server_socket_inet;

  test_command_filename[0] = '\0';

  while ((opt = getopt(argc, argv, "a:p:f:h")) != -1) {
    switch(opt) {
      case 'a':
        server_ip = optarg;
        break;

      case 'p':
        server_port = optarg;
        break;

      case 'f':
        strlcpy(test_command_filename, optarg, TS_CMD_FILENAME_MAX);
        break;

      case 'h':
      default:
        printf("Available script command:\n");
        print_help();
        printf("\n");
        printf("usage: %s [-a <ip address>] [-p <port>] [-f <file name>]\n", argv[0]);
        printf("-a:  server ip address. By default it is 127.0.0.1\n");
        printf("-p:  server port number. By default it is 9009\n");
        printf("-f:  script file name. By default it goes to interactive mode\n");
        printf("-h:  print this help\n\n");
        printf("run adb port forwarding for android target:\n");
        printf("    adb forward tcp:9009 tcp:9009\n");
        return 0;
    }
  }

  printf("server_ip: %s\n", server_ip);
  printf("server_port: %s\n", server_port);
  printf("script file %s\n", test_command_filename);

  printf("%s connecting to server...\n", __func__);
  server_socket_inet = testsuite_client_connect(server_ip, server_port);
  if (server_socket_inet < 0) {
      printf("connection cannot be established\n");
      return -1;
  }

  // wait for command, send command string to main task
  do {
      test_command_buf[0] = '\0';

      if ( test_command_filename[0] != '\0') {
          if (read_a_line(test_command_filename, test_command_buf) == -1)
          {
              // end of the file
              test_command_filename[0] = '\0';
          }
      }
      else {
          // scan from command line then
          printf("Please type a command...: ");
          fgets(test_command_buf, TS_CMD_STR_MAX + 1, stdin);
      }

      if (test_command_buf[0] == ';' || test_command_buf[0] == '#' ) {
          continue;
      }

      strlcpy(temp_buf, test_command_buf, TS_CMD_STR_MAX);
      command_id = parse_command(temp_buf, &p_args, &p_result);
      if (command_id < 0) {
          continue;
      }

      switch(command_id)
      {
          case TS_LOOP_START:
              if (p_args == NULL) {
                  command_id = TS_CMD_ID_INVALID;
                  break;
              }
              loop_cnt = atoi(p_args);
              break;

          case TS_LOOP_END:
              loop_cnt--;
              printf("LOOP %d DONE\n", loop_cnt);
              if (loop_cnt) {
                  // back to loop start, rewind back to the file begining
                  read_a_line(NULL, NULL);
                  do {
                      if (read_a_line(test_command_filename, test_command_buf) == -1)
                      {
                          printf("TS_LOOP_START is not found\n");
                          break;
                      }
                      command_id = parse_command(test_command_buf, NULL, NULL);
                  } while (command_id != TS_LOOP_START); 
              }
              break;

          case TS_CMD_FILENAME:
              if (p_args == NULL) {
                  command_id = TS_CMD_ID_INVALID;
                  break;
              }
              strlcpy(test_command_filename, p_args, TS_CMD_FILENAME_MAX);
              printf("Use command file %s\n", test_command_filename);
              break;

          default:
              printf("send command: %s\n", test_command_buf);
              result = testsuite_send(server_socket_inet, test_command_buf);
              if (result < 0) {
                  printf("server probably exited\n");
                  command_id = TS_SERVER_EXIT;
              } else {
                  result = testsuite_receive( server_socket_inet, & receive_result_ptr);
                  if (result < 0) {
                      printf("server probably exited\n");
                      command_id = TS_SERVER_EXIT;
                  }
                  else {
                      printf("    received: %s\n", receive_result_ptr);
                      free(receive_result_ptr);
                  }
              }
              break;
      }
  } while(command_id != TS_SERVER_EXIT && command_id != TS_CLIENT_EXIT);

  testsuite_client_disconnect(server_socket_inet);

  printf( "testsuite client exit\n");
  return 0;
}

