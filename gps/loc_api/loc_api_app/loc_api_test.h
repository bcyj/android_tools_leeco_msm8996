/******************************************************************************
  @file:  loc_api_test.h
  @brief: loc_api_test main header

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2007 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

$Id$
******************************************************************************/

#ifndef LOC_API_TEST_H
#define LOC_API_TEST_H

#include <stdarg.h>

/*=============================================================================
 *
 *                        CONFIGURATION AND CONSTANTS
 *
 *============================================================================*/
#define LOC_API_TEST_VER     "1.0"

#ifndef LOC_PARAMETER_FILE
#define LOC_PARAMETER_FILE       "loc_parameter.ini"        /* primary */
#endif

#define LOC_PARAMETER_FILE_ALT   "/etc/loc_parameter.ini"   /* alternative */

#define LOC_MAIN_LOG_FILE    "loc_api.log"

/*=============================================================================
 *
 *                        GLOBAL DATA DECLARATION
 *
 *============================================================================*/

/* File handle for log file */
extern FILE *loc_log_file_ptr;
extern FILE *loc_param_file_ptr;

/*=====================================================================
     Exported functions
======================================================================*/
extern void chop_newline(char *line);
extern void trim_space(char *org_string);
extern char *loc_test_get_time(char *time_string); /* fills the current time into string buffer */
#ifndef NO_LOC_API_APP_LOG
extern void gps_write_log(char *str,...);    /* stub function that calls loc_write_log (for gps_dcm.c) */
extern void loc_write_log(char *str,...);    /* write log file */
extern void loc_write_error(char *str, ...); /* write error msg to log and stderr */
extern void loc_write_msg(char *str, ...);   /* write normal msg to log and stderr */
#else
#include <log_util.h>
#define gps_write_log LOC_LOGD
#define loc_write_log LOC_LOGD
#define loc_write_error LOC_LOGE
#define loc_write_msg LOC_LOGD
#endif
extern int loc_abort(char *str, ...);        /* write message and abort */
extern int loc_exit(int code);
extern void loc_api_build(void);

extern void loc_test_init(void);
extern void on_signal_interrupt(int nParam);
extern int loc_test_general_main(int argc, char *argv[]);

/*=====================================================================
     External declarations
======================================================================*/
extern void oncrpc_task_start(void);
extern void oncrpc_init(void);

#endif /* LOC_API_TEST_H */
