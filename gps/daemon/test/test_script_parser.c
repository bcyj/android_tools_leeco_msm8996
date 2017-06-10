/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "test_script_parser.h"
#include "gpsone_daemon_dbg.h"

#include <sys/time.h>

#include "test_dbg.h"

STRING_TO_ID string_to_id_table[] =
{
    /* For Test Client only */
    {"TS_FILENAME",               TS_FILENAME},
    {"TS_LOOP_START",             TS_LOOP_START},
    {"TS_LOOP_END",               TS_LOOP_END},

    /* Common Control commands */
    {"TS_SERVER_EXIT",            TS_SERVER_EXIT},
    {"TS_CLIENT_EXIT",            TS_CLIENT_EXIT},
    {"TS_MSLEEP",                 TS_MSLEEP},
    {"TS_SLEEP",                  TS_SLEEP},
    {"TS_COUNT",                  TS_COUNT},
    {"TS_COUNT_RESET",            TS_COUNT_RESET},
    {"TS_TIME",                   TS_TIME},
    {"TS_TIME_MARKER",            TS_TIME_MARKER},
    {"TS_TIME_SINCE_MARKER",      TS_TIME_SINCE_MARKER},
    {"TS_ECHO",                   TS_ECHO},

    /* For BIT Only */
    {"TS_BIT_REGISTER",           TS_BIT_REGISTER},
    {"TS_BIT_DEREGISTER",         TS_BIT_DEREGISTER},
    {"TS_BIT_WAIT_NOTIFY",        TS_BIT_WAIT_NOTIFY},
    {"TS_BIT_DATA_READY_WAIT_NOTIFY",       TS_BIT_DATA_READY_WAIT_NOTIFY},
    {"TS_BIT_OPEN",               TS_BIT_OPEN},
    {"TS_BIT_CLOSE",              TS_BIT_CLOSE},
    {"TS_BIT_CONNECT",            TS_BIT_CONNECT},
    {"TS_BIT_DISCONNECT",         TS_BIT_DISCONNECT},
    {"TS_BIT_SEND",               TS_BIT_SEND},
    {"TS_BIT_RECEIVE",            TS_BIT_RECEIVE},
    {"TS_BIT_IOCTL",              TS_BIT_IOCTL},
};

TS_ID string_to_id( char * str)
{
    int i;

    if (!str) return TS_ID_INVALID;

    for ( i = 0; i < (int) (sizeof(string_to_id_table)/sizeof(STRING_TO_ID)); i++ )
    {
        if (strcmp(str, string_to_id_table[i].string) == 0) {
            return string_to_id_table[i].id;
        }
    }

    return TS_ID_INVALID;
}

void print_help(void)
{
    int i;

    TEST_DBG("Here are all test commands:\n");
    TEST_DBG("--------------------------------\n");

    for ( i = 0; i < (int) (sizeof(string_to_id_table)/sizeof(STRING_TO_ID)); i++ )
    {
        TEST_DBG(" %2d: %s\n", string_to_id_table[i].id, string_to_id_table[i].string);
//        if (string_to_id_table[i].id == TS_ID_INVALID) break;
    }
    TEST_DBG("--------------------------------\n");
}

int read_a_line(char * test_command_filename, char * test_command_buf)
{
    int len;

    static FILE * fhandle = NULL;

    if (test_command_filename == NULL) {
        rewind(fhandle);
        return 0;
    }

    if (fhandle == NULL) {
        fhandle = fopen(test_command_filename, "r");
        if (fhandle == NULL) {
            perror("failed to open test script file");
            exit(-1);
        }
    }

    if ( fgets( test_command_buf, TS_STR_MAX + 1, fhandle) == NULL) {
        // end of file
        fclose(fhandle);
        fhandle = NULL;
        return -1;
    }

    len = strlen(test_command_buf);
    for (; len >= 0; len --)
    {
        if (test_command_buf[len-1] == '\n' || test_command_buf[len-1] == '\r') {
            test_command_buf[len-1] = '\0';
        }
        else {
            break;
        }
    }

    return 0;
}

TS_ID parse_command( char * command_str, char ** p_command_args, char ** p_command_result)
{
    TS_ID command_id;
    char * command_str2;
    char * command_args;
    char * command_result;
    const char delim_str[] = " ;\t\n\r";

    TEST_DBG("command_str is: {%s}\n", command_str);

    command_str2 = strtok(command_str, delim_str);
    command_result = strtok(NULL, delim_str);
    command_args   = strtok(NULL, "\n\r");

    command_id = string_to_id( command_str2);
    if ( p_command_result != NULL ) * p_command_result = command_result;
    if ( p_command_args != NULL ) * p_command_args = command_args;

/*
    TEST_DBG("command_str2 = {%s}, ", command_str2);
    TEST_DBG("command_id = %d, ", command_id);
    TEST_DBG("p_command_result = {%s}, ", command_result);
    TEST_DBG("p_command_args = {%s}\n", command_args);
*/
    return command_id;
}

int test_script_common_handler(TS_ID command_id, char * args, char * result_str)
{
    int result = 0;
    long ms, sec, us_diff;
    static int test_count = 0;
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
            TEST_DBG("[%d]\n", test_count);
            break;

        case TS_COUNT_RESET:
            test_count = 0;
            break;

        case TS_TIME:
            TS_PRINT_HHMMSS();
            TEST_DBG("\n");
            break;

        case TS_TIME_MARKER:
            TS_PRINT_HHMMSS();
            TEST_DBG("\n");
            gettimeofday(&tv1, &tz);
            break;

        case TS_TIME_SINCE_MARKER:
            gettimeofday(&tv2, &tz);
            TS_PRINT_HHMMSS();
            us_diff = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
            TEST_DBG(" %ldus\n", us_diff);
            break;

        default:
            /* do not consume request */
            result = -1;
            break;
    }

    return result;
}

