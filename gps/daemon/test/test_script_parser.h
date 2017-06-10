/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __GPSONE_TEST_SCRIPT_PARSER_H__
#define __GPSONE_TEST_SCRIPT_PARSER_H__

#include <linux/types.h>

#define TS_FILENAME_MAX             32
#define TS_STR_MAX                  128

typedef enum
{
  /* For Test Client only */
  TS_FILENAME,        // This one is for command line only,
                      // cannot be used in command file.
  TS_LOOP_START,
  TS_LOOP_END,

  /* Common Control commands */
  TS_SERVER_EXIT,
  TS_CLIENT_EXIT,
  TS_MSLEEP,
  TS_SLEEP,
  TS_COUNT,
  TS_COUNT_RESET,
  TS_TIME,
  TS_TIME_MARKER,
  TS_TIME_SINCE_MARKER,
  TS_ECHO,

  /* For BIT Only */
  TS_BIT_REGISTER,
  TS_BIT_DEREGISTER,
  TS_BIT_WAIT_NOTIFY,
  TS_BIT_DATA_READY_WAIT_NOTIFY,
  TS_BIT_OPEN,
  TS_BIT_CLOSE,
  TS_BIT_CONNECT,
  TS_BIT_DISCONNECT,
  TS_BIT_SEND,
  TS_BIT_RECEIVE,
  TS_BIT_IOCTL,

  /* THIS IS THE END */
  TS_ID_MAX = TS_BIT_IOCTL,
  TS_ID_INVALID

} TS_ID;

typedef struct
{
    const char * string;
    TS_ID id;
} STRING_TO_ID;

TS_ID string_to_id( char * str);
void print_help(void);
int read_a_line(char * test_command_filename, char * test_command_buf);
TS_ID parse_command( char * command_str, char ** p_command_args, char ** p_command_result);
int test_script_common_handler(TS_ID command_id, char * args, char * result_str);

#endif /* __GPSONE_TEST_SCRIPT_PARSER_H__ */
