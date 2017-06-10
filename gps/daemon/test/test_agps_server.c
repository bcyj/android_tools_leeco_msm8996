/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// #include "gpsone_data_client.h"
#include "gpsone_glue_data_service.h"
#include "test_agps_server_handler.h"
#include "test_socket_server_helper.h"

#include "test_dbg.h"

/*
 * result:  0 - it is a local control test command, not remote command
 *          1 - success for remote command
 *         -1 - failed for remote command
 */
int test_data_server_command_dispatcher(TS_ID command_id,
	char * p_command_args, char * p_command_result,
	char * p_ack, int ack_size)
{
    int result = 0;
    int test_result;
    int test_result_expected;

    switch(command_id)
    {
        case TS_CLIENT_EXIT:
	    strncpy(p_ack, "CLIENT EXIT", ack_size - 1);
            break;

        default:
            result = test_script_common_handler(command_id, p_command_args, p_command_result);
            if (!result) {
                /* command has been handled */
		strncpy(p_ack, "OK", ack_size - 1);
                break;
            }

            test_result = command_dispatcher(command_id, p_command_args, p_command_result,
		p_ack, ack_size);
            if (p_command_result!= NULL) {
                test_result_expected = atoi(p_command_result);
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

int test_data_server(int client_socket)
{
    TS_ID command_id;
    int test_count = 0;
    int failure_count = 0;
    char * p_command_args;
    char * p_command_result;
    char text[TS_STR_MAX];
    char text_ack[TS_STR_MAX];
    int result = 0;

    TEST_DBG("%s: starts\n", __FUNCTION__);

    do {
        command_id = TS_ID_INVALID;
        result = gpsone_read_glue(client_socket, text, TS_STR_MAX);
        if (result <= 0) {
            TEST_DBG("%s:%d] disconnected\n", __func__, __LINE__);
            break;
        }

        TEST_DBG("received: %s...\n", text);

        command_id = parse_command(text, & p_command_args, & p_command_result);

        if (command_id > TS_ID_MAX) {
            TEST_DBG("         Invalid command received: %d\n", command_id);
            continue;
        }

	text_ack[0] = '\0';
        result = test_data_server_command_dispatcher(command_id, p_command_args, p_command_result,
		text_ack, TS_STR_MAX);
        failure_count += result < 0 ? 1 : 0;
        test_count ++;

        result = gpsone_write_glue(client_socket, text_ack, strlen(text_ack) + 1);
        if (result <= 0) {
            TEST_DBG("%s:%d] fail\n", __func__, __LINE__);
            break;
        }

        TEST_DBG("result = %d, replied \"%s\"\n", result, text_ack);

   } while(command_id != TS_CLIENT_EXIT && command_id != TS_SERVER_EXIT);

    TEST_DBG("%s: %d test cases executed, failures %d\n", __FUNCTION__, test_count, failure_count);
    return command_id;
}

/* 
 * test_data_server_main
 *
 */
int main(int argc, char * argv[])
{
    int result;
    const char * server_ip = "127.0.0.1";
    const char * server_port = "9009";
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
          TEST_DBG("usage: %s [-a <ip address>] [-p <port>]\n", argv[0]);
          TEST_DBG("-a:  server ip address. By default it is 127.0.0.1\n");
          TEST_DBG("-p:  server port number. By default it is 9009\n");
          TEST_DBG("-h:  print this help\n\n");
          TEST_DBG("run adb port forwarding for android target:\n");
          TEST_DBG("    adb forward tcp:9009 tcp:9009\n");
          return 0;
      }
    }

    result = gpsone_socket_server_manager(server_ip, server_port, test_data_server);

    return result;
}

