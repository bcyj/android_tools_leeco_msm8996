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

#include "test_script_parser.h"
#include "test_bit_cases.h"
#include "gpsone_daemon_dbg.h"

#include <unistd.h>

#include "test_dbg.h"

#include "../gpsone_conn_client.h"
extern char *optarg;
extern const char * global_gpsone_loc_api_q_path;
extern const char * global_gpsone_ctrl_q_path;

static char test_command_filename[TS_FILENAME_MAX + 1];

void test_bit_main_args(int argc, char *argv[])
{
  int opt;

  test_command_filename[0] = '\0';

  while ((opt = getopt(argc, argv, "f:hq:c:")) != -1) {
    switch(opt) {
      case 'f':
        strncpy(test_command_filename, optarg, TS_FILENAME_MAX);
        break;

#ifdef DEBUG_X86
      case 'q':
        global_gpsone_loc_api_q_path = optarg;
        break;

      case 'c':
        global_gpsone_ctrl_q_path = optarg;
        break;
#endif

      case 'h':
      default:
        TEST_DBG("Available script command:\n");
        print_help();
        TEST_DBG("\n");
        TEST_DBG("usage: %s [-f <file name>] [-q <loc_api_q_path]\n", argv[0]);
        TEST_DBG("-q:  loc_api_q_path\n");
        TEST_DBG("-c:  ctrl_q_path\n");
        TEST_DBG("-f:  script file name. By default it goes to interactive mode\n");
        TEST_DBG("-h:  print this help\n\n");
        TEST_DBG("run adb port forwarding for android target:\n");
        TEST_DBG("    adb forward tcp:9009 tcp:9009\n");
        return;
    }
  }

  TEST_DBG("script file %s\n", test_command_filename);
}

#ifdef DEBUG_X86
int test_bit_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
  TS_ID command_id = TS_ID_INVALID;
  char test_command_buf[TS_STR_MAX + 1];
  char temp_buf[TS_STR_MAX + 1];
  char * p_args, * p_result;
  int result;
  int loop_cnt = 0;
/*
int socket_inet;
struct ctrl_msg_connect cmsg_connect;
cmsg_connect.is_supl = 0;
cmsg_connect.is_udp = 0;
cmsg_connect.ip_port = 8009;
cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V4;
cmsg_connect.ip_addr.addr.v4_addr = 1<<24 | 0<<16 | 0<< 8 | 127;
socket_inet = gpsone_client_connect(cmsg_connect);
gpsone_client_disconnect(socket_inet);
TEST_DBG("DONE\n");
return 0;
*/
#ifndef DEBUG_X86
  test_bit_main_args( argc, argv);
#endif
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
          TEST_DBG("Please type a command...: ");
          fgets(test_command_buf, TS_STR_MAX + 1, stdin);
      }

      if (test_command_buf[0] == ';' || test_command_buf[0] == '#' ) {
          continue;
      }

      strncpy(temp_buf, test_command_buf, TS_STR_MAX);
      command_id = parse_command(temp_buf, &p_args, &p_result);
      TEST_DBG("command: %s...\n", test_command_buf);
      if (command_id > TS_ID_MAX) {
          TEST_DBG("command: TS_ID_INVALID\n");
          continue;
      }

      switch(command_id)
      {
          case TS_LOOP_START:
              if (p_args == NULL) {
                  command_id = TS_ID_INVALID;
                  break;
              }
              loop_cnt = atoi(p_args);
              break;

          case TS_LOOP_END:
              loop_cnt--;
              TEST_DBG("LOOP %d DONE\n", loop_cnt);
              if (loop_cnt) {
                  // back to loop start, rewind back to the file beginning
                  read_a_line(NULL, NULL);
                  do {
                      if (read_a_line(test_command_filename, test_command_buf) == -1)
                      {
                          TEST_DBG("TS_LOOP_START is not found\n");
                          break;
                      }
                      command_id = parse_command(test_command_buf, NULL, NULL);
                  } while (command_id != TS_LOOP_START);
              }
              break;

          case TS_FILENAME:
              if (p_args == NULL) {
                  command_id = TS_ID_INVALID;
                  break;
              }
              strncpy(test_command_filename, p_args, TS_FILENAME_MAX);
              TEST_DBG("Use command file %s\n", test_command_filename);
              break;

          default:
              result = test_script_common_handler(command_id, p_args, p_result);
              if (!result) {
                  /* command has been handled */
                  break;
              }

              result = test_bit_cases(command_id, p_args, p_result);
              if (result < 0) {
//                  TEST_DBG("         server probably exited\n");
                  command_id = TS_CLIENT_EXIT;
              } else {
//                  TEST_DBG("         send done.\n");
              }
              break;
      }
  } while(command_id != TS_CLIENT_EXIT);

  TEST_DBG( "test bit client exit\n");
  return 0;
}

