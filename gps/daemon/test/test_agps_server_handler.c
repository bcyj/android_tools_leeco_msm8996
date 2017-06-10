/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test_agps_server_handler.h"

#include <unistd.h>
#include <sys/time.h>

static int TS_ECHO_handler(const char * args, char * result_str,
        char * p_ack, int ack_size)
{
    int result = 0;
    strncpy(p_ack, args, ack_size-1);
    return result;
}

int command_dispatcher(TS_ID command_id, char * args, char * result_str,
	char * p_ack, int ack_size)
{
    int result = 0;

    switch(command_id)
    {
        case TS_ECHO:
        default:
            if (args) {
                result = TS_ECHO_handler(args, result_str, p_ack, ack_size);
            } else {
                result = TS_ECHO_handler("NULL", result_str, p_ack, ack_size);
            }
            break;
    }
    return result;
}

