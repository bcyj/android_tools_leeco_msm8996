/******************************************************************************
  @file:  loc_api_data.h
  @brief:

  DESCRIPTION
    This file contains LOC_API test application data that are directly
  accessible through the test command line. It will be parsed by CINT to
  enable run-time access. CINT is only 95% compatible with ANSI C. If any
  data in this file fails CINT scan during compilation, it can be moved to
  loc_api_app_main.h to solve the problem.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2007 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
03/17/09   dx       Android version
01/01/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_data.h#19 $
======================================================================*/

#ifndef LOC_API_DATA_H
#define LOC_API_DATA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exported data, types and functions to CINT */
#include "loc_api_rpc_glue.h"
#include "loc_api_ini.h"
#include "loc_api_cb_tests.h"

#define MAX_FILE_PATH_LEN                 256

#define LOC_TEST_DEBUG       1      /* 1 for debugging */

typedef struct loc_sys_data_s
{
   rpc_loc_client_handle_type   loc_handle;           /* handle of the test connection */
   int                          app_code;             /* application error code */
   int                          api_code;             /* last api call return code */
   int                          cbw_code;             /* last callback waiting return code */
   int                          ioctl_code;           /* last ioctl status */
   int                          rpc_started;          /* 1: rpc initialized */
   int                          dcm_started;          /* 1: data call manager is running */
   int                          fix_started;          /* 1: position fix started */
   int                          lcd_started;          /* 1: frame buffer initialized */
   int                          sky_plot;             /* 1: auto update sky plot */
   int                          ni_default_resp;      /* 1: accept, 2: deny, others no default */
   int                          disable_dcm;          /* 1: disable all DCM operations */
   int                          unit_test_mode;       /* 1: unit test mode */
   int                          unit_test_ticks;      /* running time */
   int                          unit_test_timeout;    /* timeout seconds */
   int                          unit_test_line;       /* current line number of unit-test script */
   int                          unit_test_quit;       /* set to 1 if unit test should quit */
   char                         unit_test_script[MAX_FILE_PATH_LEN]; /* input script for automatic a unit test */
   char                         unit_test_filelist[MAX_FILE_PATH_LEN]; /* file list for automatic a unit test */
   char                         log_path[MAX_FILE_PATH_LEN]; /* log file path */
} loc_sys_data_s_type;

typedef struct loc_param_data_s
{
   unsigned long  use_logcat;
   unsigned long  event_mask;
   unsigned long  nmea_sentence_mask;
   unsigned long  engine_lock;
   unsigned long  fix_operation_mode;
   unsigned long  fix_sess_type;
   unsigned long  time_between_fixes;
   unsigned long  accuracy_threshold;
   unsigned long  fix_timeout;
   unsigned long  enable_auto_download;
   unsigned long  auto_download_period;
   unsigned long  sky_plot;
   unsigned long  data_request_reply;
   char           xtra_server_addr[LOC_MAX_PARAM_STRING + 1]; /* IP address of NTP server */
   unsigned long  umts_pdp_profile_num;               /* 0 for default */
   char           apn_name[LOC_MAX_PARAM_STRING + 1]; /* APN name, NULL for default */
} loc_param_data_s_type;

typedef struct
{
   rpc_loc_parsed_position_s_type pos;
   rpc_loc_engine_state_e_type engine_state;
} loc_last_data_s_type;

typedef struct
{
   boolean xtra_inject_incomplete;
   boolean xtra_inject_out_of_order;
} loc_test_mode_s_type;

extern loc_sys_data_s_type sys;           /* system data */
extern loc_param_data_s_type param;       /* system params */
extern rpc_loc_event_payload_u_type lastcb; /* last callback payload */
extern loc_last_data_s_type last;         /* last misc data */
extern loc_cb_f_type *cbf;                /* selected callback test */
extern loc_test_mode_s_type tmode;        /* test modes */

extern void loc_api_data_init(void);
extern void loc_api_data_reset(void);

/********************************************************
 * SYSTEM EXPORTED FUNCTION
 *******************************************************/
extern void loc_api_build(void);
extern int loc_enable_sky_plot(int enable);
extern int loc_gui_test(void);

/********************************************************
 * EXPORTED FUNCTION FOR AUTOMATIC TESTING
 *******************************************************/

extern void verify(int condition);
extern void verify_code(int condition, int code);
extern void failure(char *str);
extern void failure_code(char *str, int code);
extern void ok(int condition);
extern void cmd(const char *cmdline);

/********************************************************
 * EXPORTED FUNCTION FOR DEVELOPMENT DEBUGGING
 *******************************************************/

#if defined LOC_TEST_DEBUG && LOC_TEST_DEBUG == 1
extern void test_ni(int cmd); /* cmd range 1 - 3 */
#endif /* LOC_TEST_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOC_API_DATA_H */
