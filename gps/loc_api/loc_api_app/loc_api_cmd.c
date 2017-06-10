   /******************************************************************************
  @file:  loc_api_cmd.c
  @brief:  module for loc_api_test commands

  DESCRIPTION

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
10/31/09   dx       Add EFS injection and WiFi test cases
03/17/09   dx       Android version
01/09/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cmd.c#30 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "stringl.h"
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef USE_GLIB
#include <glib.h>
#endif
#ifdef HAS_NCURSES
#include <term.h>
#endif /* HAS_NCURSES */

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_ini.h"
#include "loc_api_cmd.h"
#include "loc_api_data.h"
#include "loc_api_cb_hub.h"
#include "loc_api_cb_log.h"
#include "loc_api_cb_ni.h"
#include "loc_api_xtra_bin.h"
#include "loc_api_xtra_time.h"
#include "loc_api_gui.h"

#include "loc_apicb_appinit.h"
#include "platform_lib_includes.h"
/*=============================================================================
 *
 *                        GLOBAL DATA DECLARATION
 *
 *============================================================================*/

#define GPS_EFS_INJECT_PART_SIZE             1800   /* Reduced for librpc (<< 2048) */

/* Active command loop function */
loc_test_cmd_loop_f_type *loc_test_cmd_loop = NULL;

/* Active command processor for the application */
loc_test_cmd_proc_f_type *loc_test_cmdline_proc = NULL;

/* Active extra help processor */
loc_test_cmd_help_f_type *loc_test_cmdline_extra_help = NULL;

/* Command-specific processors */
static int cmd_help_handler(int argc, char* argv[]);
static int cmd_loc_open(int argc, char* argv[]);
static int cmd_loc_close(int argc, char* argv[]);
static int cmd_loc_start_fix(int argc, char* argv[]);
static int cmd_loc_stop_fix(int argc, char* argv[]);
static int cmd_loc_ioctl_get_version(int argc, char* argv[]);
static int cmd_loc_ioctl_set_fix_criteria(int argc, char* argv[]);
static int cmd_loc_ioctl_get_fix_criteria(int argc, char* argv[]);
static int cmd_loc_ioctl_set_engine_lock(int argc, char* argv[]);
static int cmd_loc_ioctl_get_engine_lock(int argc, char* argv[]);
static int cmd_loc_ioctl_set_mode(int argc, char* argv[]);
static int cmd_loc_ioctl_get_mode(int argc, char* argv[]);
static int cmd_loc_ioctl_notify_wiper_status(int argc, char* argv[]);
static int cmd_loc_ioctl_send_wiper_pos(int argc, char* argv[]);
static int cmd_loc_xtra_query_data_validity(int argc, char* argv[]);
static int cmd_loc_xtra_query_data_source(int argc, char* argv[]);
static int cmd_loc_xtra_inject_time(int argc, char* argv[]);
static int cmd_loc_xtra_inject_data(int argc, char* argv[]);
static int cmd_loc_xtra_inject_pos(int argc, char* argv[]);
static int cmd_loc_inject_efs_file(int argc, char* argv[]);
static int cmd_loc_del_efs_file(int argc, char* argv[]);
static int cmd_loc_set_server_addr(int argc, char* argv[]);
static int cmd_loc_get_server_addr(int argc, char* argv[]);
static int cmd_loc_del_assist_data(int argc, char* argv[]);
static int cmd_sleep_handler(int argc, char* argv[]);
static int cmd_clear_handler(int argc, char* argv[]);
static int cmd_exit_handler(int argc, char* argv[]);

loc_cmd_handler_s_type loc_cmd_table[] =
{
  {"help",                          cmd_help_handler,                   "List all commands"},
  {"loc_open",                      cmd_loc_open,                       "Open loc_api connection"},
  {"loc_close",                     cmd_loc_close,                      "Close loc_api connection"},
  {"loc_start_fix",                 cmd_loc_start_fix,                  "Start position fix"},
  {"loc_stop_fix",                  cmd_loc_stop_fix,                   "Stop position fix"},
  {"loc_ioctl_set_fix_criteria",    cmd_loc_ioctl_set_fix_criteria,     "Enact params (call before loc_start_fix)"},
  {"loc_ioctl_get_fix_criteria",    cmd_loc_ioctl_get_fix_criteria,     "Get active params"},
  /*{"gps_ioctl_set_nmea_sentence",   trigger_gps_ioctl_set_nmea_sentence,     "Set NMEA sentence type"},
  {"gps_ioctl_get_nmea_sentence",   trigger_gps_ioctl_get_nmea_sentence,     "Get NMEA sentence type"},*/
  {"loc_ioctl_set_engine_lock",     cmd_loc_ioctl_set_engine_lock,      "Set engine lock"},
  {"loc_ioctl_get_engine_lock",     cmd_loc_ioctl_get_engine_lock,      "Get engine lock"},
  {"loc_ioctl_set [MODE] [1|2]",    cmd_loc_ioctl_set_mode,             "Set on/off mode"},
  {"loc_ioctl_get [MODE]",          cmd_loc_ioctl_get_mode,             "Get on/off mode"},
  {"loc_ioctl_notify_wiper_status", cmd_loc_ioctl_notify_wiper_status,  "Notify status of Wiper (wifi position)"},
  {"loc_ioctl_send_wiper_pos",      cmd_loc_ioctl_send_wiper_pos,       "Send Wifi position"},
  {"loc_xtra_query_data_validity",  cmd_loc_xtra_query_data_validity,   "Query XTRA validity"},
  {"loc_xtra_query_data_source",    cmd_loc_xtra_query_data_source,     "Query XTRA data source"},
  {"loc_xtra_inject_file",          cmd_loc_xtra_inject_data,           "Inject XTRA file"},
  {"loc_xtra_inject_time",          cmd_loc_xtra_inject_time,           "Inject XTRA time"},
  {"loc_xtra_inject_pos",           cmd_loc_xtra_inject_pos,            "Inject coarse position"},
  {"loc_inject_efs_file",           cmd_loc_inject_efs_file,            "Inject a file to EFS"},
  {"loc_del_efs_file",              cmd_loc_del_efs_file,               "Delete a file from EFS"},
  {"loc_del_assist_data",           cmd_loc_del_assist_data,            "Delete assisting data"},
  {"loc_set_server_addr",           cmd_loc_set_server_addr,            "Set PDE server addresses"},
  {"loc_get_server_addr",           cmd_loc_get_server_addr,            "Get PDE server addresses"},
  {"loc_ioctl_get_version",         cmd_loc_ioctl_get_version,          "Get LOC_API version"},
  {"sleep",                         cmd_sleep_handler,                  "Sleep N seconds"},
  {"clear",                         cmd_clear_handler,                  "Clear screen/(unit test)error code"},
  {"exit",                          cmd_exit_handler,                   "Exit"},
  {NULL, (loc_test_cmd_handler_f_type*) NULL, NULL} /* end mark */
};

#define CHECK_API_RESULT_MSG(ret_code,comment) \
   check_api_ret_code(ret_code, comment)

#define CHECK_API_RESULT(ret_code) \
   check_api_ret_code(ret_code, NULL)

#define CHECK_CB_RESULT(ret_code) \
   check_cb_wait_ret_code(ret_code)

#define CHECK_IOCTL_STATUS(payload_data) \
   check_ioctl_status(payload_data)

/*=============================================================================
 *
 *                      FUNCTION DECLARATION (PRIVATE)
 *
 *============================================================================*/

/*===========================================================================

FUNCTION cmdline_tokenizer

DESCRIPTION
   This function tokenizes a command line into the command name and its
   arguments. Results are stored in argc and argv.

DEPENDENCIES
   N/A

RETURN VALUE
   Number of arguments (command itself included)

SIDE EFFECTS
   Input string cmdline is changed with NULL added to the end of each token.

===========================================================================*/
static void cmdline_tokenizer(char *cmdline, int *argc, char **argv)
{
   char *separators = " \t";
   char *pArg, *lasts;
   *argc = 0;

   pArg = strtok_r(cmdline, separators, &lasts);
   while (pArg != NULL)
   {
      if (*argc < LOC_MAX_ARG_NUM)  /* Only store first max_args arguments */
      {
          argv[*argc] = pArg;
      }
      pArg = strtok_r((char*) NULL, separators, &lasts);
      (*argc)++;
   }
}

/*===========================================================================

FUNCTION find_cmd_handler

DESCRIPTION
   Looks for the command in the command table

DEPENDENCIES
   N/A

RETURN VALUE
   The command handle, or NULL if command not found

SIDE EFFECTS
   N/A

===========================================================================*/
static loc_test_cmd_handler_f_type* find_cmd_handler(const char *cmd_name)
{
   int i, len;
   for (i = 0; loc_cmd_table[i].cmd_name; i++)
   {
      if (strncmp(loc_cmd_table[i].cmd_name, cmd_name, len = strlen(cmd_name)) == 0 &&
         (loc_cmd_table[i].cmd_name[len] == '\0' || loc_cmd_table[i].cmd_name[len] == ' '))
      {
         return loc_cmd_table[i].cmd_handler;
      }
   }
   return NULL; /* Command not found */
}

/*===========================================================================

FUNCTION loc_validate_menu_key

DESCRIPTION
   Checks if the key is in the menu.

RETURN VALUE
   Returns the same key if so, or 0 if not.

===========================================================================*/
static char loc_validate_menu_key(loc_submenu_s_type *menu, char key)
{
   int i = 0;
   while (menu->items[i].key)
   {
      if (key == menu->items[i].key)
      {
         return key;
      }
      i++;
   }
   return '\0';
}

/*===========================================================================

FUNCTION loc_use_submenu

DESCRIPTION
   Displays and takes user selection from a menu. If argc=0 and there is a default
   key, the default key is returned. If argc=1, argv[1] is taken as the selection.
   If the selection is valid, then it's returned, otherwise, the menu is displayed
   and the user is prompted to make a selection.

   By the above logic, the user can typically type "cmd ?" or "cmd *" to display
   the menu.

RETURN VALUE
   The selected menu key
   or -1 if the user chooses "exit"

===========================================================================*/
static int loc_use_submenu(loc_submenu_s_type *menu, int argc, char *argv[])
{
   assert(menu);
   char linebuf[80];

   /* Default selection */
   if (argc <= 1 && menu->default_key)
   {
      return menu->default_key;
   }

   /* Take cmd choice if it is a single char */
   int cur_key = 0;
   if (argc >= 2 && strlen(argv[1]) >= 1)
   {
      cur_key = argv[1][0];
   }

   /* Display menu until a valid key is entered */
   while (!(cur_key = loc_validate_menu_key(menu, cur_key)))
   {
      if (menu->title)
      {
         printf("%s\n", menu->title);
      }

      int i = 0;
      while (menu->items[i].key)
      {
         printf("  %c: %s\n", menu->items[i].key, menu->items[i].text);
         i++;
      }
      printf("  0: exit\n"); /* 0 is always for exit */
      printf("Choice: ");
      fgets(linebuf, sizeof linebuf, stdin);
      cur_key = linebuf[0];

      if (cur_key == '0')
      {
         return -1; /* exit */
      }
   }

   return cur_key;
}

/*===========================================================================

FUNCTION loc_enter_number

DESCRIPTION
   If number if not given, then ask the user to enter.

RETURN VALUE
   The numeric value of user input

===========================================================================*/
static int loc_enter_number(char *prompt, char *input)
{
   int num = 0;
   if (input == NULL || input[0] == '\0')
   {
      printf("%s", prompt);
      scanf("%d", &num);
   }
   else {
      num = atoi(input);
   }
   return num;
}

/*===========================================================================

FUNCTION loc_enter_double

DESCRIPTION
   If number if not given, then ask the user to enter.

RETURN VALUE
   The numeric value of user input

===========================================================================*/
static double loc_enter_double(char *prompt, char *input)
{
   double num = 0.0;
   if (input == NULL || input[0] == '\0')
   {
      printf("%s", prompt);
      scanf("%lf", &num);
   }
   else {
      num = atof(input);
   }
   return num;
}

/*===========================================================================

FUNCTION loc_enter_string

DESCRIPTION
   If the string if not given, asks the user to enter.

RETURN VALUE
   The user input string

===========================================================================*/
static char* loc_enter_string(char *prompt, char *input)
{
   static char string_buf[256];
   char *output = input;

   if (input == NULL || input[0] == '\0')
   {
      printf("%s", prompt);
      fgets(string_buf, sizeof string_buf, stdin);
      chop_newline(string_buf);
      output = string_buf;
   }

   return output;
}

/*===========================================================================

FUNCTION loc_enter_ipv4_addr

DESCRIPTION
   If no input is given, then ask the user to enter a string, and parse
   it as an ipv4 address.

RETURN VALUE
   1 if success
   0 if failed

===========================================================================*/
static int loc_enter_ipv4_addr(char* prompt, char* ip_input, char* port_input, rpc_loc_server_addr_ipv4_type *addr)
{
   struct in_addr inet_addr;
   int port = 0;

   /* Enter IP */
   if (ip_input == NULL || ip_input[0] == '\0')
   {
      ip_input = loc_enter_string(prompt, NULL);
   }

   /* Parse address string */
   if (inet_aton(ip_input, &inet_addr) == 0)
   {
      loc_write_error("Not a valid IPv4 address: %s\n", ip_input);
      return 0; /* not valid */
   }
   else {
      addr->addr = (uint32_t) htonl(inet_addr.s_addr);
   }

   /* Enter port */
   if (port_input == NULL || port_input[0] == '\0')
   {
      port = loc_enter_number("Port: ", NULL);
   } else {
      port = atoi(port_input);
   }

   addr->port = (uint16) port;

   return 1; /* success */
}

/*===========================================================================

FUNCTION check_api_ret_code

DESCRIPTION
   Checks LOC API result code, and logs any error.

DEPENDENCIES
   N/A

RETURN VALUE
   ret_code

SIDE EFFECTS
   N/A

===========================================================================*/
static int check_api_ret_code(int ret_code, const char *comment)
{
   sys.api_code = ret_code;
   if (ret_code != 0)
   {
      loc_write_error("LOC API error %s %c %s\n",
            loc_get_ioctl_status_name(ret_code),
            (comment && *comment) ? ':' : ' ',
            comment ? comment : "");
   }

   return ret_code;
}

/*===========================================================================

FUNCTION check_cb_wait_ret_code

DESCRIPTION
   Checks LOC API callback return code, and logs any error.

DEPENDENCIES
   N/A

RETURN VALUE
   ret_code

SIDE EFFECTS
   N/A

===========================================================================*/
static int check_cb_wait_ret_code(int ret_code)
{
   sys.cbw_code = ret_code;

   /* Check callback status */
   switch (ret_code)
   {
   case ETIMEDOUT:
      loc_write_error("TIMED OUT\n");
      break;
   case EBUSY:
      loc_write_error("BUSY\n");
      break;
   case EINVAL:
      loc_write_error("INVALID\n");
      break;
   }

   return ret_code;
}

/*===========================================================================

FUNCTION check_ioctl_status

DESCRIPTION
   Checks LOC API IOCTL status, and logs any error.

DEPENDENCIES
   N/A

RETURN VALUE
   ret_code (0 if successful)

SIDE EFFECTS
   N/A

===========================================================================*/
static int check_ioctl_status(rpc_loc_event_payload_u_type *cb_payload)
{
   uint32 status = cb_payload->rpc_loc_event_payload_u_type_u.ioctl_report.status;
   sys.ioctl_code = status;

   if (status != RPC_LOC_API_SUCCESS)
   {
      loc_write_error("IOCTL status: %d %s\n",
            status, loc_get_ioctl_status_name(status));
   }

   return status == RPC_LOC_API_SUCCESS ? 0 : (int) status;
}

/*=============================================================================
 *
 *                             COMMAND HANDLERS
 *
 *============================================================================*/

/*===========================================================================

FUNCTION cmd_help_handler

DESCRIPTION
   This function shows current supported commands

DEPENDENCIES
   N/A

RETURN VALUE
   Always 0 (no error).

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_help_handler(int argc, char* argv[])
{
   int i;
   char *fmt = "%-30s\t\t%s\r\n";

   printf("\r\nCommand Name\t\t                Description\r\n");
   printf("----------------------------------------------------\r\n");

   for(i = 0; loc_cmd_table[i].cmd_name; i++)
   {
      printf(fmt, loc_cmd_table[i].cmd_name, loc_cmd_table[i].cmd_help_str);
   }

   /* extra help */
   if (loc_test_cmdline_extra_help != NULL)
   {
      printf("----------------------------------------------------\r\n");
      loc_test_cmdline_extra_help(fmt);
   }

   printf("----------------------------------------------------\r\n\r\n");

   return LOC_CMD_QUIET_RET_CODE;
}

/*===========================================================================

FUNCTION cmd_sleep_handler

DESCRIPTION
   This functions pauses the command line thread for N seconds. N is the
   first parameter argv[1] of the command. If no argument is given, 1 second
   is assumed.

===========================================================================*/
static int cmd_sleep_handler(int argc, char* argv[])
{
   int sleep_seconds = 1;
   if (argc >= 2)
   {
      sleep_seconds = atoi(argv[1]);
   }

   while (sleep_seconds > 0 && !sys.unit_test_quit)
   {
      sleep(1);
      sleep_seconds--;
   }

   return 0;
}

/*===========================================================================

FUNCTION cmd_clear_screen

DESCRIPTION
   This function clears the screen.

DEPENDENCY
   #include <term.h>
   Dynamic library: -l termcap

===========================================================================*/
static int cmd_clear_screen(int argc, char* argv[])
{
   static char* clear_screen_string = NULL;
   static char* reset_cursor_string = NULL;

#ifdef HAS_NCURSES
   /* Init the term cmd string */
   if (!clear_screen_string || !reset_cursor_string)
   {
      int rc;
      setupterm(NULL, fileno(stdout), &rc);
      if (rc > 0)
      {
         clear_screen_string = tigetstr("clear");
         reset_cursor_string = tigetstr("cup");
      }
   }

   /* clear screen */
   if (clear_screen_string)
   {
      putp(clear_screen_string);
   }

   /* move cursor to (0,0) */
   if (reset_cursor_string)
   {
      putp( tparm( reset_cursor_string, 0 /* line */, 0 /* col */,
            0, 0, 0, 0, 0, 0, 0 ) );
   }

#else
   loc_write_error("Clear screen not supported.\n");
#endif /* HAS_NCURSES */

   return LOC_CMD_QUIET_RET_CODE; /* always returns successful */
}

/*===========================================================================

FUNCTION cmd_clear_handler

DESCRIPTION
   This function clears the screen in interactive mode, or clears error code
   in the unit test mode. After error code is cleared, a unit test can report
   PASSED despite previous errors.

===========================================================================*/
static int cmd_clear_handler(int argc, char* argv[])
{
   if (sys.unit_test_mode)
   {
      return LOC_CMD_CLEARERR_RET_CODE;
   }
   else {
      return cmd_clear_screen(argc, argv);
   }
}

/*===========================================================================

FUNCTION cmd_exit_handler

DESCRIPTION
   This function exits the application.

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   Exits application.

===========================================================================*/
static int cmd_exit_handler(int argc, char* argv[])
{
   return LOC_CMD_EXIT_RET_CODE; /* special code to exit program */
}

/*===========================================================================

FUNCTION cmd_loc_open

DESCRIPTION
   Opens a loc_api connection

DEPENDENCIES
   ONCRPC

RETURN VALUE
  Error code. Returns 0 if successful.

SIDE EFFECTS
   N/A
===========================================================================*/
static int cmd_loc_open(int argc, char* argv[])
{
   rpc_loc_event_mask_type event_mask = param.event_mask;

#if defined LOC_TEST_DEBUG && LOC_TEST_DEBUG == 1
   /* XXX remove when testing is done */
   event_mask = 0xffffffff;
#endif

   sys.loc_handle = loc_open(event_mask, loc_test_callback_handler, NULL, NULL);
   if (sys.loc_handle == RPC_LOC_CLIENT_HANDLE_INVALID)
   {
      loc_write_error("Location API Open failed.\n");
      return RPC_LOC_CLIENT_HANDLE_INVALID;
   }

   return 0;
}

/*===========================================================================

FUNCTION cmd_loc_close

DESCRIPTION
   Closes a loc_api connection

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

SIDE EFFECTS

===========================================================================*/
static int cmd_loc_close(int argc, char* argv[])
{
   int ret_code = 0;
   ret_code = loc_close(sys.loc_handle);
   CHECK_API_RESULT(ret_code);

   sys.loc_handle = -1;
   return ret_code;
}

/*===========================================================================

FUNCTION cmd_loc_start_fix

DESCRIPTION
   Start position fix

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

SIDE EFFECTS

===========================================================================*/
static int cmd_loc_start_fix(int argc, char* argv[])
{
   int ret_code = 0;

   /* Clears last position */
   memset(&last.pos, 0, sizeof last.pos);

   /* Start sky plot */
   if (param.sky_plot)
   {
      loc_enable_sky_plot(1);
   }

   ret_code = loc_start_fix(sys.loc_handle);
   CHECK_API_RESULT(ret_code);
   if (ret_code == 0)
   {
      sys.fix_started = 1;
      loc_write_msg("Check %s for position reports...\n", sys.log_path);
   }
   return ret_code;
}

/*===========================================================================

FUNCTION cmd_loc_stop_fix

DESCRIPTION
   Stop position fix

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

SIDE EFFECTS

===========================================================================*/
static int cmd_loc_stop_fix(int argc, char* argv[])
{
   int ret_code = 0;
   sys.fix_started = 0;

   ret_code = loc_stop_fix(sys.loc_handle);
   CHECK_API_RESULT(ret_code);

   return ret_code;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_get_version

DESCRIPTION
   Get LOC API version

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

SIDE EFFECTS

===========================================================================*/
static int cmd_loc_ioctl_get_version(int argc, char* argv[])
{
   /* Callback data */
   rpc_loc_event_payload_u_type callback_payload;

   /* Select callback to wait for */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &callback_payload);

   /* Make the API call */
   int ret_code = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_GET_API_VERSION,
         NULL
   );
   CHECK_API_RESULT_MSG(ret_code, "cmd_loc_ioctl_get_version");

   /* Wait for call back */
   ret_code = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(ret_code);

   /* Print the version */
   if (ret_code == 0 && 0 == CHECK_IOCTL_STATUS(&callback_payload))
   {
      int major, minor;
      major = callback_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.
         rpc_loc_ioctl_callback_data_u_type_u.api_version.major;
      minor = callback_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.
         rpc_loc_ioctl_callback_data_u_type_u.api_version.minor;
      printf("LOC API Version %d.%d\n", major, minor);
   }
   else
   {
      /* Use LOC API error code */
      ret_code = RPC_LOC_API_GENERAL_FAILURE;
   }

   return ret_code;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_set_fix_criteria

DESCRIPTION
   Sends params to modem. This function should be called before loc_start_fix if
   the parameters are not the defaults.

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

===========================================================================*/
static int cmd_loc_ioctl_set_fix_criteria(int argc, char* argv[])
{
   /* IOCTL data */
   rpc_loc_ioctl_data_u_type ioctl_data;
   rpc_loc_fix_criteria_s_type *fix_criteria = &ioctl_data.rpc_loc_ioctl_data_u_type_u.fix_criteria;

   /* Callback data */
   rpc_loc_event_payload_u_type callback_payload;

   /******************************************
    * Fill in data for IOCTL
    *****************************************/
   fix_criteria->valid_mask |= RPC_LOC_FIX_CRIT_VALID_PREFERRED_ACCURACY |
   RPC_LOC_FIX_CRIT_VALID_PREFERRED_RESPONSE_TIME;
   fix_criteria->preferred_accuracy       = (uint32) param.accuracy_threshold;
   fix_criteria->preferred_response_time  = (uint32) param.fix_timeout * 1000;

   /* Fix session type: tracking vs. single fix */
   fix_criteria->valid_mask |= RPC_LOC_FIX_CRIT_VALID_RECURRENCE_TYPE;
   if(param.fix_sess_type == 0)
   {
      fix_criteria->recurrence_type = RPC_LOC_PERIODIC_FIX;
      loc_write_msg(" - set periodic fix\n");
   }
   else
   {
      fix_criteria->recurrence_type = RPC_LOC_SINGLE_FIX;
      loc_write_msg(" - set single fix\n");
   }

   /* Fix operation mode: MSA / MSB / Standalone */
   fix_criteria->valid_mask |= RPC_LOC_FIX_CRIT_VALID_PREFERRED_OPERATION_MODE;
   switch(param.fix_operation_mode)
   {
   case 1:
      fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_DEFAULT;
      loc_write_msg(" - set default mode\n");
      break;
   case 2:
      fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_MSB;
      loc_write_msg(" - set MSB mode\n");
      break;
   case 3:
      fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_MSA;
      loc_write_msg(" - set MSA mode\n");
      break;
   case 4:
      fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_STANDALONE;
      loc_write_msg(" - set standalone mode\n");
      break;
   case 5:
      fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_SPEED_OPTIMAL;
      loc_write_msg(" - set speed optimal mode\n");
      break;
   case 6:
         fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_ACCURACY_OPTIMAL;
         loc_write_msg(" - set accuracy optimal mode\n");
         break;
   case 7:
         fix_criteria->preferred_operation_mode = RPC_LOC_OPER_MODE_DATA_OPTIMAL;
         loc_write_msg(" - set data optimal mode\n");
         break;
   default:
      fix_criteria->valid_mask &= ~ ((uint32) RPC_LOC_FIX_CRIT_VALID_PREFERRED_OPERATION_MODE);
      loc_write_error(" ? invalid operation mode: %d\n", param.fix_operation_mode);
   }

   fix_criteria->valid_mask |= RPC_LOC_FIX_CRIT_VALID_MIN_INTERVAL;
   fix_criteria->min_interval = param.time_between_fixes * 1000;
   loc_write_msg(" - set min interval %d secs\n", (int) param.time_between_fixes);

   /* Select callback to wait for */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &callback_payload);

   /******************************************
    * Make the API call
    *****************************************/
   int ret_code = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_SET_FIX_CRITERIA,
         &ioctl_data
   );
   CHECK_API_RESULT_MSG(ret_code, "cmd_loc_ioctl_set_fix_criteria");

   /* Wait for call back */
   ret_code = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(ret_code);

   if (ret_code == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&callback_payload);
   }

   return ret_code ? RPC_LOC_API_GENERAL_FAILURE: 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_notify_wiper_status

DESCRIPTION
   Sends params to modem. This function should be called before loc_start_fix if
   the parameters are not the defaults.

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

===========================================================================*/
static int cmd_loc_ioctl_notify_wiper_status(int argc, char* argv[])
{
   static loc_submenu_s_type menu =
         {
               "Wiper status notification menu",
               '\0', /* no default */
               {
                     {'1', "Available"},
                     {'2', "Unavailable"},
                     {'\0', NULL}
               }
         };

   /* IOCTL data */
   rpc_loc_ioctl_data_u_type         ioctl_data;
   rpc_loc_wiper_status_e_type       status;
   rpc_loc_ioctl_e_type              ioctl_cmd = 0;         /* ioctl command */

   /* Callback data */
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */

   int rc = 0;

   int menu_key = loc_use_submenu(&menu, argc, argv);       /* get item selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   status = menu_key == '1' ?
         RPC_LOC_WIPER_STATUS_AVAILABLE :
         RPC_LOC_WIPER_STATUS_UNAVAILABLE;

   /******************************************
    * Fill in data for IOCTL
    *****************************************/
   ioctl_data.rpc_loc_ioctl_data_u_type_u.wiper_status = status;
   loc_write_msg("Set WPS available = %d\n", status);

   /* Select callback to wait for */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);

   /******************************************
    * Make the API call
    *****************************************/
   int ret_code = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_NOTIFY_WIPER_STATUS,
         &ioctl_data
   );
   CHECK_API_RESULT_MSG(ret_code, "cmd_loc_ioctl_notify_wiper_status");

   /* Wait for call back */
   ret_code = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(ret_code);

   if (ret_code == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return ret_code ? RPC_LOC_API_GENERAL_FAILURE: 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_get_fix_criteria

DESCRIPTION
   Get location middleware fix criteria

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   Error code. Returns 0 if successful.

===========================================================================*/
static int cmd_loc_ioctl_get_fix_criteria(int argc, char* argv[])
{
   /* Callback data */
   rpc_loc_event_payload_u_type callback_payload;

   /* Select callback to wait for */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &callback_payload);

   /* Make the API call */
   int ret_code = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_GET_FIX_CRITERIA,
         NULL
   );
   CHECK_API_RESULT_MSG(ret_code, "cmd_loc_ioctl_get_fix_criteria");

   /* Wait for call back */
   ret_code = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(ret_code);

   /* Print the fix criteria */
   if (ret_code == 0 && 0 == CHECK_IOCTL_STATUS(&callback_payload))
   {
      /***************************************
       * Print active fix criteria
       **************************************/
      rpc_loc_fix_criteria_s_type *fix_crit =
         &callback_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.
         rpc_loc_ioctl_callback_data_u_type_u.fix_criteria;

      if(fix_crit->recurrence_type == RPC_LOC_PERIODIC_FIX)
      {
         loc_write_msg(" - periodic fix\n");
      }
      else
      {
         loc_write_msg(" - single fix\n");
      }

      /* Fix operation mode: MSA / MSB / Standalone */
      switch(fix_crit->preferred_operation_mode)
      {
      case RPC_LOC_OPER_MODE_MSA:
         loc_write_msg(" - MSA mode\n");
         break;
      case RPC_LOC_OPER_MODE_MSB:
         loc_write_msg(" - MSB mode\n");
         break;
      case RPC_LOC_OPER_MODE_STANDALONE:
         loc_write_msg(" - standalone mode\n");
         break;
      case RPC_LOC_OPER_MODE_SPEED_OPTIMAL:
         loc_write_msg(" - speed optimal mode\n");
         break;
      case RPC_LOC_OPER_MODE_ACCURACY_OPTIMAL:
         loc_write_msg(" - accuracy optimal mode\n");
         break;
      case RPC_LOC_OPER_MODE_DATA_OPTIMAL:
         loc_write_msg(" - data optimal mode\n");
         break;
      default:
         loc_write_msg(" ? unknown operation mode: %d\n",
               fix_crit->preferred_operation_mode);
      }

      /* Time between fixes */
      loc_write_msg(" - min interval %d ms\n", fix_crit->min_interval);

   }
   else
   {
      /* Use LOC API error code */
      ret_code = RPC_LOC_API_GENERAL_FAILURE;
   }

   return ret_code;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_set_engine_lock

DESCRIPTION
   Sets engine lock mode

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_ioctl_set_engine_lock(int argc, char* argv[])
{
   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "Set engine lock menu",
            '\0', /* no default */
            {
                  {'1', "None"},
                  {'2', "MI"},
                  {'3', "MT"},
                  {'4', "ALL"},
                  {'\0', NULL}
            }
      };

   int rc;
   rpc_loc_ioctl_data_u_type ioctl_data;                    /* ioctl data */
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */
   rpc_loc_lock_e_type choice = 0;                          /* mode choice */

   int menu_key = loc_use_submenu(&menu, argc, argv);   /* get item selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   switch (menu_key)
   {
   case '1': choice = RPC_LOC_LOCK_NONE; break;
   case '2': choice = RPC_LOC_LOCK_MI; break;
   case '3': choice = RPC_LOC_LOCK_MT; break;
   case '4': choice = RPC_LOC_LOCK_ALL; break;
   default: assert(0); // all choices should be processed
   }

   /* Makes the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   ioctl_data.rpc_loc_ioctl_data_u_type_u.engine_lock = choice;
   rc = loc_ioctl(sys.loc_handle, RPC_LOC_IOCTL_SET_ENGINE_LOCK, &ioctl_data);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_set_engine_lock");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);
   if (rc == 0)
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }
   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_set_engine_lock

DESCRIPTION
   Sets engine lock mode

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_ioctl_get_engine_lock(int argc, char* argv[])
{
   int rc;
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */
   rpc_loc_lock_e_type mode;                                /* mode setting */

   /* Makes the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle, RPC_LOC_IOCTL_GET_ENGINE_LOCK, NULL);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_get_engine_lock");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0)
   {
      /* Callback is ok */
      if (0 == CHECK_IOCTL_STATUS(&cb_payload))
      {
         mode = cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
            data.rpc_loc_ioctl_callback_data_u_type_u.engine_lock;
         loc_write_msg("Engine mode: ");
         switch (mode)
         {
         case RPC_LOC_LOCK_NONE: loc_write_msg("LOC_LOCK_NONE"); break;
         case RPC_LOC_LOCK_MI: loc_write_msg("LOC_LOCK_MI");break;
         case RPC_LOC_LOCK_MT: loc_write_msg("LOC_LOCK_MT");break;
         case RPC_LOC_LOCK_ALL: loc_write_msg("LOC_LOCK_ALL");break;
         default: loc_write_msg("Unknown %d", (int) mode);
         }
         loc_write_msg("\n");
      }
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_set_mode

DESCRIPTION
   Sets a binary mode: XTRA auto-download, SBAS.

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_ioctl_set_mode(int argc, char* argv[])
{
   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "Set mode menu",
            '\0', /* no default */
            {
                  {'A', "XTRA auto-download"},
                  {'S', "SBAS"},
                  {'L', "On-demand LPM"},
                  {'\0', NULL}
            }
      };

   static loc_submenu_s_type tf_menu =
         {
               NULL, /* no title */
               '\0', /* no default */
               {
                     {'1', "On"},
                     {'2', "Off"},
                     {'\0', NULL}
               }
         };

   int rc = 0;
   rpc_loc_ioctl_data_u_type ioctl_data;                    /* ioctl data */
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */
   boolean choice = 0;                                  /* binary choice */
   rpc_loc_ioctl_e_type ioctl_cmd = 0;                      /* ioctl command */
   int param;

   int menu_key = loc_use_submenu(&menu, argc, argv);   /* get item selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   int tf_key = loc_use_submenu(&tf_menu, argc - 1, argv + 1); /* get ON/OFF choice */
   if (tf_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   choice = tf_key == '1' ? 1 : 0; /* 0 for OFF, 1 for ON */

   /* Perform menu selection */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);

   switch (menu_key)
   {
   case 'A':
      ioctl_cmd = RPC_LOC_IOCTL_SET_PREDICTED_ORBITS_DATA_AUTO_DOWNLOAD;
      param = loc_enter_number("Check every N hours, enter N: ",
            argc >= 4 ? argv[3] : NULL);
      ioctl_data.rpc_loc_ioctl_data_u_type_u.predicted_orbits_auto_download.enable = choice;
      ioctl_data.rpc_loc_ioctl_data_u_type_u.predicted_orbits_auto_download.auto_check_every_hrs = param;
      rc = loc_ioctl(sys.loc_handle, ioctl_cmd, &ioctl_data);
      break;
   case 'S':
      ioctl_cmd = RPC_LOC_IOCTL_SET_SBAS_CONFIG;
      ioctl_data.rpc_loc_ioctl_data_u_type_u.sbas_mode = choice;
      rc = loc_ioctl(sys.loc_handle, ioctl_cmd, &ioctl_data);
      break;
   case 'L':
      ioctl_cmd = RPC_LOC_IOCTL_SET_ON_DEMAND_LPM;
      ioctl_data.rpc_loc_ioctl_data_u_type_u.on_demand_lpm = choice;
      rc = loc_ioctl(sys.loc_handle, ioctl_cmd, &ioctl_data);
      break;
   default:
      assert(0); /* all commands must be processed */
   }

   /* Makes the API call */
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_set_mode");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);
   if (rc == 0)
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_get_mode

DESCRIPTION
   Gets a binary mode: XTRA auto-download, SBAS.

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_ioctl_get_mode(int argc, char* argv[])
{
   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "Get mode menu",
            '\0', /* no default */
            {
                  {'S', "SBAS"},
                  {'L', "On-demand LPM"},
                  {'\0', NULL}
            }
      };

   int rc = 0;
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */
   rpc_loc_ioctl_e_type ioctl_cmd = 0;                      /* ioctl command */

   int menu_key = loc_use_submenu(&menu, argc, argv);   /* get item selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   /* Perform menu selection */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);

   switch (menu_key)
   {
   case 'S':
      ioctl_cmd = RPC_LOC_IOCTL_GET_SBAS_CONFIG;
      rc = loc_ioctl(sys.loc_handle, ioctl_cmd, NULL);
      break;
   case 'L':
      ioctl_cmd = RPC_LOC_IOCTL_GET_ON_DEMAND_LPM;
      rc = loc_ioctl(sys.loc_handle, ioctl_cmd, NULL);
      break;
   default:
      assert(0); /* all commands must be processed */
   }

   /* Makes the API call */
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_set_mode");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0) /* callback successful */
   {
      switch (menu_key)
      {
      case 'S':
         loc_write_msg("SBAS: %s\n",
               cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.sbas_mode ? "On" : "Off");
         break;
      case 'L':
         loc_write_msg("On-demand LPM: %s\n",
               cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.on_demand_lpm ? "On" : "Off");
         break;
      default:
         assert(0); /* all commands must be processed */
      }
   }
   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_xtra_query_data_validity

DESCRIPTION
   Queries XTRA data validity. It blocks for a certain amount of time
   until a callback is received.

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_loc_xtra_query_data_validity(int argc, char* argv[])
{
   int rc;
   rpc_loc_event_payload_u_type cb_payload;

   /* Makes the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_QUERY_PREDICTED_ORBITS_DATA_VALIDITY,
         NULL
   );
   CHECK_API_RESULT_MSG(rc, "cmd_loc_xtra_query_data_validity");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0)
   {
      loc_write_msg("Start UTC time: %ld sec, valid duration: %d hr\n",
            (long) cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.predicted_orbits_data_validity.start_time_utc,
            (int)  cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.predicted_orbits_data_validity.valid_duration_hrs
      );
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_xtra_query_data_validity

DESCRIPTION
   Queries XTRA data validity. It blocks for a certain amount of time
   until a callback is received.

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_loc_xtra_query_data_source(int argc, char* argv[])
{
   int rc;
   rpc_loc_event_payload_u_type cb_payload;

   /* Makes the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_QUERY_PREDICTED_ORBITS_DATA_SOURCE,
         NULL
   );
   CHECK_API_RESULT_MSG(rc, "cmd_loc_xtra_query_data_source");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0)
   {
      loc_write_msg("Max file size: %d, max part size: %d\n",
            (long) cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.predicted_orbits_data_source.max_file_size,
            (int)  cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.predicted_orbits_data_source.max_part_size);
      int i;
      char *serv_addr = NULL;
      for (i = 0; i < RPC_LOC_API_MAX_NUM_PREDICTED_ORBITS_SERVERS; i++)
         if ( NULL != (serv_addr=cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.
               data.rpc_loc_ioctl_callback_data_u_type_u.predicted_orbits_data_source.
               servers[i]))
         {
            loc_write_msg("Server %d: %s\n", i, serv_addr);
         }
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_xtra_inject_time

DESCRIPTION
   Inject XTRA time

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_loc_xtra_inject_time(int argc, char* argv[])
{
   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "XTRA inject time menu",
            '1',
            {
                  {'1', "SNTP server time"},
                  {'2', "Linux system time"},
                  {'3', "EC: 1970/1/1"},
                  {'4', "EC: 2005/4/1"},
                  {'5', "EC: 2030/4/1"},
                  {'\0', NULL}
            }
      };
   int menu_key = loc_use_submenu(&menu, argc, argv); /* get selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   int rc;
   rpc_loc_event_payload_u_type cb_payload;
   rpc_loc_ioctl_data_u_type data;
   struct tm tinfo;
   time_t unix_time;

   /* Clears tm struct */
   memset(&tinfo, 0, sizeof tinfo);

   switch (menu_key)
   {
   case '1':
      rc = loc_xtra_download_sntp_time(&data.rpc_loc_ioctl_data_u_type_u.assistance_data_time);
      if (rc) /* error */
      {
         loc_write_error("Download time error. Using system time.Please note if system time set on the phone "
                         "is incorrect this may result in incorrect time being injected into the engine\n");
         loc_xtra_get_sys_time(&data.rpc_loc_ioctl_data_u_type_u.assistance_data_time);
      }
      break;
   case '2': /* use Linux time */
      loc_write_msg("Use Linux system time.\n");
      loc_xtra_get_sys_time(&data.rpc_loc_ioctl_data_u_type_u.assistance_data_time);
      break;
   case '3': /* EC: 1970/1/1 */
      tinfo.tm_year = 1970 - 1900;
      tinfo.tm_mon = 1 - 1;
      tinfo.tm_mday = 1;
      tinfo.tm_isdst = -1;
      unix_time = mktime(&tinfo); /* seconds since 1/1/1970 UTC */
      data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.time_utc = (uint64) unix_time * 1000;
      data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.uncertainty = 10;
      break;
   case '4': /* EC: 2005/4/1 */
      tinfo.tm_year = 2005 - 1900;
      tinfo.tm_mon = 4 - 1;
      tinfo.tm_mday = 1;
      tinfo.tm_isdst = -1;
      unix_time = mktime(&tinfo); /* seconds since 1/1/1970 UTC */
      data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.time_utc = (uint64) unix_time * 1000;
      data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.uncertainty = 10;
      break;
   case '5': /* EC: 2030/4/1 */
      tinfo.tm_year = 2030 - 1900;
      tinfo.tm_mon = 4 - 1;
      tinfo.tm_mday = 1;
      tinfo.tm_isdst = -1;
      unix_time = mktime(&tinfo); /* seconds since 1/1/1970 UTC */
      data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.time_utc = (uint64) unix_time * 1000;
      data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.uncertainty = 10;
      break;
   default:
      assert(0); /* all choices must be handled */
   }

   loc_write_msg("UTC time (in msec): %lld, uncertainty: %ld\n",
         (long long) data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.time_utc,
         (long) data.rpc_loc_ioctl_data_u_type_u.assistance_data_time.uncertainty);

   /* Make the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle, RPC_LOC_IOCTL_INJECT_UTC_TIME, &data);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_xtra_inject_time");

   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_xtra_inject_data

DESCRIPTION
   Inject XTRA predicted orbits data

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_loc_xtra_inject_data(int argc, char* argv[])
{
   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "XTRA inject data menu",
            '1',
            {
                  {'1', "Sequential injection"},
                  {'2', "EC: Incomplete injection"},
                  {'3', "EC: Out of order injection"},
                  {'\0', NULL}
            }
      };
   int menu_key = loc_use_submenu(&menu, argc, argv); /* get selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   /* Translate menu selection to flags */
   tmode.xtra_inject_incomplete = (menu_key == '2');     /* exceptional case: incomplete   */
   tmode.xtra_inject_out_of_order = (menu_key == '3');   /* exceptional case: out of order */

   int rc;
   rpc_loc_event_payload_u_type cb_payload;              /* ioctl callback data */

   FILE*  xtra_fp  = NULL;                              /* xtra bin file */
   int    filesize = 0;
   char   *line;

   /* Check existing file */
   xtra_fp = fopen(GPS_XTRA_FILENAME, "r");
   if (xtra_fp != NULL)
   {
      int accept = 0;
      if (sys.unit_test_mode)
      {
         accept = 1;
      }
      else {
         line = loc_enter_string("Use existing file " GPS_XTRA_FILENAME " (y/n)? ", NULL);
         if (toupper(line[0]) == 'Y')
         {
            accept = 1;
         }
      }

      if (accept)
      {
         /* Get filesize and reset file position */
         fseek (xtra_fp, 0, SEEK_END);
         filesize = ftell(xtra_fp);
         fseek (xtra_fp, 0, SEEK_SET);

         loc_write_msg("Existing XTRA file accepted.\n");
      }
      else {
         /* Close existing file */
         fclose(xtra_fp);
         xtra_fp = NULL;
      }
   }

   /* Download XTRA data file if not using an existing one */
   if (xtra_fp == NULL)
   {
      if ( (xtra_fp=loc_xtra_download_bin(&filesize)) == NULL )
      {
         return RPC_LOC_API_GENERAL_FAILURE;  /* Cannot download file */
      }
   }

   /* Inject XTRA data to modem */
   loc_select_callback(RPC_LOC_IOCTL_INJECT_PREDICTED_ORBITS_DATA, &cb_payload);
   rc = loc_xtra_inject_bin(xtra_fp, filesize);

   /* Close the file */
   if (xtra_fp)
   {
      fclose(xtra_fp);
   }

   CHECK_API_RESULT_MSG(rc, "loc_xtra_inject_bin");

   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_xtra_inject_pos

DESCRIPTION
   Inject XTRA coarse position

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_loc_xtra_inject_pos(int argc, char* argv[])
{
   int rc = 0;

   /* IOCTL data */
   rpc_loc_ioctl_data_u_type ioctl_data;
   rpc_loc_assist_data_pos_s_type *assistance_data_position =
      &ioctl_data.rpc_loc_ioctl_data_u_type_u.assistance_data_position;

   /* Callback data */
   rpc_loc_event_payload_u_type cb_payload;

   /********************************
    * Enter latitude & longitude
    *******************************/
   assistance_data_position->valid_mask = RPC_LOC_ASSIST_POS_VALID_LATITUDE
      | RPC_LOC_ASSIST_POS_VALID_LONGITUDE;
   assistance_data_position->latitude =
      loc_enter_double("Latitude: ", argc >= 2 ? argv[1] : NULL);
   assistance_data_position->longitude =
      loc_enter_double("Longitude: ", argc >= 3 ? argv[2] : NULL);

   /* Hard code uncertainty */
   assistance_data_position->valid_mask |= RPC_LOC_ASSIST_POS_VALID_CONFIDENCE_HORIZONTAL
      | RPC_LOC_ASSIST_POS_VALID_HOR_UNC_CIRCULAR;
   assistance_data_position->hor_unc_circular = 1000;    /* meters */
   assistance_data_position->confidence_horizontal = 80; /* 80% confidence */

   /* Log */
   loc_write_msg("Inject coarse position Lat=%.3lf, Long=%.3lf\n",
         (double) assistance_data_position->latitude,
         (double) assistance_data_position->longitude);

   /* Make the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle, RPC_LOC_IOCTL_INJECT_POSITION, &ioctl_data);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_xtra_inject_pos");

   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_send_wiper_pos

DESCRIPTION
   Send WiFi position

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
static int cmd_loc_ioctl_send_wiper_pos(int argc, char* argv[])
{
   int rc = 0;

   /* IOCTL data */
   rpc_loc_ioctl_data_u_type ioctl_data;
   rpc_loc_wiper_position_report_s_type *wiper_pos =
      &ioctl_data.rpc_loc_ioctl_data_u_type_u.wiper_pos;

   /* Callback data */
   rpc_loc_event_payload_u_type cb_payload;

   /********************************
    * Enter latitude & longitude
    *******************************/
   wiper_pos->wiper_valid_info_flag = RPC_LOC_WIPER_LOG_POS_VALID;

   wiper_pos->wiper_fix_position.lat =
      loc_enter_number("Latitude (int32): ", argc >= 2 ? argv[1] : NULL);

   wiper_pos->wiper_fix_position.lon =
      loc_enter_number("Longitude (int32): ", argc >= 3 ? argv[2] : NULL);

   wiper_pos->wiper_fix_position.HEPE = (rpc_uint16)
         loc_enter_number("HEPE (meters, int16): ", argc >= 4 ? argv[3] : NULL);

   /* Set stock data */
   wiper_pos->wiper_fix_position.num_of_aps_used = 1; /* One access point */
   wiper_pos->wiper_fix_position.fix_error_code = 0;

   /**********************************
    * Set access point data
    *********************************/
   wiper_pos->wiper_valid_info_flag |= RPC_LOC_WIPER_LOG_AP_SET_VALID;
   wiper_pos->wiper_ap_set.num_of_aps = 1;
   /* Set fake data */
   char mac_addr[RPC_LOC_WIPER_MAC_ADDR_LENGTH] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
   memcpy(&wiper_pos->wiper_ap_set.ap_info[0].mac_addr,
         mac_addr, sizeof wiper_pos->wiper_ap_set.ap_info[0].mac_addr);
   wiper_pos->wiper_ap_set.ap_info[0].channel = 7;
   wiper_pos->wiper_ap_set.ap_info[0].rssi = 50; /* random, underspecified */
   wiper_pos->wiper_ap_set.ap_info[0].ap_qualifier = RPC_LOC_WIPER_AP_QUALIFIER_BEING_USED;

   /* Log */
   loc_write_msg("Send WiFi position Lat=%d, Long=%d, HEPE=%d\n",
         (int) wiper_pos->wiper_fix_position.lat,
         (int) wiper_pos->wiper_fix_position.lon,
         (int) wiper_pos->wiper_fix_position.HEPE);

   /* Make the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle, RPC_LOC_IOCTL_SEND_WIPER_POSITION_REPORT, &ioctl_data);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_send_wiper_pos");

   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);

   if (rc == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_inject_efs_file

DESCRIPTION
   Inject a file to the EFS

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

SIDE EFFECTS
   N/A

===========================================================================*/
#ifdef FEATURE_LOC_MW_EFS_ACCESS
static int loc_inject_efs_file(FILE* data_fp, int file_size, const char *path)
{
   if (file_size == 0)
   {
      return FALSE; /* no data to inject */
   }

   int part_size      = GPS_EFS_INJECT_PART_SIZE;
   int total_parts    = (file_size - 1) / part_size + 1;
   int remain_size    = file_size;
   int i, rc = RPC_LOC_API_SUCCESS;

   loc_write_msg("Begin to inject EFS file to modem (size %d, path: %s)...\n", file_size, path);

   rpc_loc_ioctl_data_u_type  data;                         /* ioctl data */
   rpc_loc_efs_data_s_type *efs_data = &data.rpc_loc_ioctl_data_u_type_u.efs_data;

   /* Allocate memory for data reading */
   unsigned char* data_buf = (unsigned char*) malloc(part_size);

   if (data_buf == NULL)
   {
      loc_write_error("EFS data: out of memory.\n");
      return RPC_LOC_API_GENERAL_FAILURE; /* out of memory */
   }

   /* Set up inject info */
#ifndef FEATURE_RPC_CHAR_ARRAY
   efs_data->filename = (char*) path;
#else
   strlcpy(efs_data->filename, path, sizeof efs_data->filename);
#endif /* FEATURE_RPC_CHAR_ARRAY */

   efs_data->total_size = file_size;
   efs_data->data_ptr.data_ptr_len = 0; /* to be filled below */
   efs_data->data_ptr.data_ptr_val = (char*) data_buf;
   efs_data->total_parts = total_parts;
   efs_data->reserved = 0;

   for (i = 0; i < efs_data->total_parts; i++)
   {
      int data_size = MIN(part_size, remain_size);
      int actual_data_size = fread((void*) data_buf, 1, data_size, data_fp);
      if (data_size != actual_data_size)
      {
         loc_write_error("EFS data file read error.\n");
         rc = RPC_LOC_API_GENERAL_FAILURE;
         break;
      }

      /* Inject a part */
      efs_data -> part_len = actual_data_size;
      efs_data -> part = i + 1;  /* Part 1 is the first part */
      efs_data -> data_ptr.data_ptr_len = efs_data -> part_len;

      loc_write_msg("Injecting part %d/%d (%d bytes remain)\n", efs_data -> part,
            total_parts, remain_size);

      efs_data->operation = (i == 0 ? RPC_LOC_FS_CREATE_WRITE_FILE : RPC_LOC_FS_APPEND_FILE);

      rc = loc_api_sync_ioctl(sys.loc_handle, RPC_LOC_IOCTL_ACCESS_EFS_DATA,
            &data, LOC_CB_WAIT_SECONDS*1000, NULL);

      if (rc != RPC_LOC_API_SUCCESS)
      {
         loc_write_error("EFS data injection failed at part %d, rc = %s\n", efs_data->part,
               loc_get_ioctl_status_name(rc));
         break;
      }

      /* Deduct remaining size */
      remain_size -= actual_data_size;
   }

   free(data_buf);

   return rc;
}
#endif /* FEATURE_LOC_MW_EFS_ACCESS */

static int cmd_loc_inject_efs_file(int argc, char* argv[])
{
#ifdef FEATURE_LOC_MW_EFS_ACCESS
   int rc = 0;

   /* File data */
   char *filename, *efs_path;
   FILE *data_fp = NULL;
   int filesize = 0;

   /********************************
    * Enter filename and EFS path
    *******************************/
   while (1) {
      filename = loc_enter_string("File to inject (empty to quit): ", argc >= 2 ? argv[1] : NULL);
      if (!strlen(filename)) return LOC_CMD_QUIET_RET_CODE;
      data_fp = fopen(filename, "rb");
      if (data_fp) {
         break;
      }
      else {
         loc_write_msg("Cannot open file: %s", filename);
         if (argc >= 2) break; // using command arg, do not loop
      }
   };
   efs_path = loc_enter_string("EFS path: ", argc >= 3 ? argv[2] : NULL);

   /* Get filesize and reset file position */
   fseek(data_fp, 0, SEEK_END);
   filesize = ftell(data_fp);
   fseek(data_fp, 0, SEEK_SET);

   /* Make the API calls */
   rc = loc_inject_efs_file(data_fp, filesize, efs_path);

   /* Close file */
   fclose(data_fp);

   return rc;
#else
   return RPC_LOC_API_UNSUPPORTED;
#endif /* FEATURE_LOC_MW_EFS_ACCESS */
}

static int cmd_loc_del_efs_file(int argc, char* argv[])
{
#ifdef FEATURE_LOC_MW_EFS_ACCESS
   /* File data */
   char *efs_path;

   /* IOCTL data */
   rpc_loc_ioctl_data_u_type data;
   rpc_loc_efs_data_s_type *efs_data = &data.rpc_loc_ioctl_data_u_type_u.efs_data;

   /* Callback data */
   rpc_loc_event_payload_u_type callback_payload;

   /********************************
    * Enter filename and EFS path
    *******************************/
   efs_path = loc_enter_string("EFS path: ", argc >= 2 ? argv[1] : NULL);
   if (strlen(efs_path) == 0)
   {
      return LOC_CMD_QUIET_RET_CODE;
   }

   /* Enter the data */
#ifndef FEATURE_RPC_CHAR_ARRAY
   efs_data->filename = (char*) efs_path;
#else
   strlcpy(efs_data->filename, efs_path, sizeof efs_data->filename);
#endif /* FEATURE_RPC_CHAR_ARRAY */

   efs_data->operation = RPC_LOC_FS_DELETE_FILE;
   efs_data->total_size = efs_data->part_len = 0;
   efs_data->data_ptr.data_ptr_val = NULL;
   efs_data->data_ptr.data_ptr_len = 0;
   efs_data->part = efs_data->total_parts = 0;
   efs_data->reserved = 0;

   /* Make the API call */
   int rc = loc_api_sync_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_ACCESS_EFS_DATA,
         &data, LOC_CB_WAIT_SECONDS*1000, NULL);

   return rc;
#else
   return RPC_LOC_API_UNSUPPORTED;
#endif /* FEATURE_LOC_MW_EFS_ACCESS */
}

/*===========================================================================

FUNCTION cmd_loc_del_assist_data

DESCRIPTION
   Delete all assisting data

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_del_assist_data(int argc, char* argv[])
{
   /* IOCTL data */
   rpc_loc_ioctl_data_u_type data;
   data.rpc_loc_ioctl_data_u_type_u.assist_data_delete.type = RPC_LOC_ASSIST_DATA_ALL;
   memset(data.rpc_loc_ioctl_data_u_type_u.assist_data_delete.reserved, 0,
         sizeof data.rpc_loc_ioctl_data_u_type_u.assist_data_delete.reserved);

   /* Callback data */
   rpc_loc_event_payload_u_type callback_payload;

   /* Select callback to wait for */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &callback_payload);

   /* Make the API call */
   int ret_code = loc_ioctl(sys.loc_handle,
         RPC_LOC_IOCTL_DELETE_ASSIST_DATA,
         &data
   );
   CHECK_API_RESULT_MSG(ret_code, "cmd_loc_del_assist_data");

   /* Wait for call back */
   ret_code = loc_wait_callback(LOC_CB_WAIT_SECONDS);

   CHECK_CB_RESULT(ret_code);

   if (ret_code == 0) /* callback ok */
   {
      CHECK_IOCTL_STATUS(&callback_payload);
   }

   return ret_code ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_ioctl_set_server_addr

DESCRIPTION
   Sets a PDE server address

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_set_server_addr(int argc, char* argv[])
{
   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "Set server address menu",
            '\0', /* no default */
            {
                  {'P', "CDMA PDE"},
                  {'M', "CDMA MPC"},
                  {'C', "CDMA Customer PDE"},
                  {'S', "UMTS SLP"},
                  {'\0', NULL}
            }
      };

   static loc_submenu_s_type addr_type_menu =
         {
               NULL, /* no title */
               '\0', /* no default */
               {
                     {'1', "IPv4"},
                     {'2', "URL"},
                     {'\0', NULL}
               }
         };

   int rc;
   rpc_loc_ioctl_data_u_type ioctl_data;                    /* ioctl data */
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */
   rpc_loc_ioctl_e_type ioctl_cmd = 0;                      /* ioctl command */
   char *url = NULL;

   int menu_key = loc_use_submenu(&menu, argc, argv);   /* get item selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   int addr_type_key = loc_use_submenu(&addr_type_menu, argc - 1, argv + 1);   /* get item selection */
   if (addr_type_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   /* Gets the address */

   switch (addr_type_key)
   {
   case '1':
      ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_type = RPC_LOC_SERVER_ADDR_IPV4;
      loc_enter_ipv4_addr("Enter IP: ",
            argc > 3 ? argv[3] : NULL, /* IP */
            argc > 4 ? argv[4] : NULL, /* port */
            &ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.ipv4);
      break;
   case '2':
      ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_type = RPC_LOC_SERVER_ADDR_URL;
      url = loc_enter_string("Enter URL: ",
            argc > 3 ? argv[3] : NULL);

#ifndef FEATURE_RPC_CHAR_ARRAY
      ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr.addr_val=url;
      ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr.addr_len =
         ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.length = strlen(url)+1;

      loc_write_log("Setting server address [%s] len=%d\n",
            ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr.addr_val,
            ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.length);
#else
      strlcpy(ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr, url,
            sizeof ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr);

      loc_write_log("Setting server address [%s] len=%d\n",
            ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr,
            ioctl_data.rpc_loc_ioctl_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.length);
#endif /* FEATURE_RPC_CHAR_ARRAY */

      break;
   default:
      loc_write_error("Unknown server type key: %c\n", (char) addr_type_key);
      break;
   }

   switch (menu_key)
   {
   case 'P':
      ioctl_cmd = RPC_LOC_IOCTL_SET_CDMA_PDE_SERVER_ADDR;
      break;
   case 'M':
      ioctl_cmd = RPC_LOC_IOCTL_SET_CDMA_MPC_SERVER_ADDR;
      break;
   case 'C':
      ioctl_cmd = RPC_LOC_IOCTL_SET_CUSTOM_PDE_SERVER_ADDR;
      break;
   case 'S':
      ioctl_cmd = RPC_LOC_IOCTL_SET_UMTS_SLP_SERVER_ADDR;
      break;
   default:
      assert(0); /* all commands must be processed */
   }

   /* Makes the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle, ioctl_cmd, &ioctl_data);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_set_server_addr");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);
   if (rc == 0)
   {
      CHECK_IOCTL_STATUS(&cb_payload);
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*===========================================================================

FUNCTION cmd_loc_get_server_addr

DESCRIPTION
   Gets a PDE server address

DEPENDENCIES
   cmd_loc_open

RETURN VALUE
   error code

===========================================================================*/
static int cmd_loc_get_server_addr(int argc, char* argv[])
{

   /* Submenu */
   static loc_submenu_s_type menu =
      {
            "Get server address menu",
            '\0', /* no default */
            {
                  {'P', "CDMA PDE"},
                  {'M', "CDMA MPC"},
                  {'C', "CDMA Customer PDE"},
                  {'S', "UMTS SLP"},
                  {'\0', NULL}
            }
      };

   int rc;
   rpc_loc_ioctl_data_u_type ioctl_data;                    /* ioctl data */
   rpc_loc_event_payload_u_type cb_payload;                 /* ioctl callback data */
   rpc_loc_ioctl_e_type ioctl_cmd = 0;                      /* ioctl command */
   struct in_addr inet_addr;

   int menu_key = loc_use_submenu(&menu, argc, argv);   /* get item selection */
   if (menu_key == -1) { return LOC_CMD_QUIET_RET_CODE; }

   switch (menu_key)
   {
   case 'P':
      ioctl_cmd = RPC_LOC_IOCTL_GET_CDMA_PDE_SERVER_ADDR;
      break;
   case 'M':
      ioctl_cmd = RPC_LOC_IOCTL_GET_CDMA_MPC_SERVER_ADDR;
      break;
   case 'C':
      ioctl_cmd = RPC_LOC_IOCTL_GET_CUSTOM_PDE_SERVER_ADDR;
      break;
   case 'S':
      ioctl_cmd = RPC_LOC_IOCTL_GET_UMTS_SLP_SERVER_ADDR;
      break;
   default:
      assert(0); /* all commands must be processed */
   }

   /* Makes the API call */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, &cb_payload);
   rc = loc_ioctl(sys.loc_handle, ioctl_cmd, &ioctl_data);
   CHECK_API_RESULT_MSG(rc, "cmd_loc_ioctl_set_server_addr");

   /* Blocks until callback */
   rc = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(rc);
   if (rc == 0)
   {
      CHECK_IOCTL_STATUS(&cb_payload);

      /* Prints the address */
      switch (cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.rpc_loc_ioctl_callback_data_u_type_u.server_addr.addr_type)
      {
      case RPC_LOC_SERVER_ADDR_IPV4:
         inet_addr.s_addr = (uint32) ntohl(cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.rpc_loc_ioctl_callback_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.ipv4.addr);
         loc_write_msg("IP: %s\n", inet_ntoa(inet_addr));
         loc_write_msg("Port: %d\n", (int) cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.rpc_loc_ioctl_callback_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.ipv4.port);
         break;
      case RPC_LOC_SERVER_ADDR_URL:
#ifndef FEATURE_RPC_CHAR_ARRAY
         loc_write_msg("URL: %s\n", cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.rpc_loc_ioctl_callback_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr.addr_val);
#else
         loc_write_msg("URL: %s\n", cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.rpc_loc_ioctl_callback_data_u_type_u.server_addr.addr_info.rpc_loc_server_addr_u_type_u.url.addr);
#endif /* FEATURE_RPC_CHAR_ARRAY */
         break;
      default:
         loc_write_error("Unknown address type: %d\n", cb_payload.rpc_loc_event_payload_u_type_u.ioctl_report.data.rpc_loc_ioctl_callback_data_u_type_u.server_addr.addr_type);
         break;
      }
   }

   return rc ? RPC_LOC_API_GENERAL_FAILURE : 0;
}

/*=============================================================================
 *
 *                     FUNCTION DECLARATION (PUBLIC)
 *
 *============================================================================*/

/*===========================================================================

FUNCTION    loc_test_checked_ioctl

DESCRIPTION
   Makes an ioctl call and performs result checking

DEPENDENCIES
   N/A

RETURN VALUE
   error code, 0 for success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_test_checked_ioctl
(
      rpc_loc_ioctl_e_type ioctl_type,                  /* ioctl command */
      rpc_loc_ioctl_data_u_type* ioctl_data,      /* ioctl data */
      rpc_loc_event_payload_u_type* callback_payload,   /* callback buffer, NULL if not used */
      const char *error_tag                         /* msg displayed with errors */
)
{
   rpc_loc_event_payload_u_type cb_buffer;
   rpc_loc_event_payload_u_type *cb_ptr = callback_payload ? callback_payload : &cb_buffer;

   /* Select callback to wait for */
   loc_select_callback(RPC_LOC_EVENT_IOCTL_REPORT, cb_ptr);

   /* Make the API call */
   int ret_code = loc_ioctl(sys.loc_handle,
         ioctl_type,
         ioctl_data
   );
   CHECK_API_RESULT_MSG(ret_code, error_tag);

   /* Wait for call back */
   ret_code = loc_wait_callback(LOC_CB_WAIT_SECONDS);
   CHECK_CB_RESULT(ret_code);

   /* Print the version */
   if (ret_code != 0 || 0 != CHECK_IOCTL_STATUS(cb_ptr))
   {
      /* Use LOC API error code */
      ret_code = RPC_LOC_API_GENERAL_FAILURE;
   }

   return ret_code;
}

/*===========================================================================

FUNCTION    loc_test_reg_cmd_proc

DESCRIPTION
   Sets the global command line processor

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_test_reg_cmd_loop(loc_test_cmd_loop_f_type *cmd_loop)
{
   loc_test_cmd_loop = cmd_loop;
}

/*===========================================================================

FUNCTION    loc_test_reg_cmd_proc

DESCRIPTION
   Sets the global command line processor

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_test_reg_cmd_proc(loc_test_cmd_proc_f_type *cmd_proc)
{
   loc_test_cmdline_proc = cmd_proc;
}

/*===========================================================================

FUNCTION    loc_test_reg_cmd_extra_help

DESCRIPTION
   Sets the global command line extra help processor

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_test_reg_cmd_extra_help(loc_test_cmd_help_f_type *cmd_help)
{
   loc_test_cmdline_extra_help = cmd_help;
}

/*===========================================================================

FUNCTION    loc_test_default_cmd_loop

DESCRIPTION
   Main command loop.

RETURN VALUE
    0              if no command has returned any error
    err code       the last error occurred

===========================================================================*/
int loc_test_default_cmd_loop()
{
   int ret_val;
   char linebuf[512];

   sys.app_code = 0;

   while(!sys.unit_test_quit)
   {
      printf("loc> ");
      fgets(linebuf, sizeof linebuf, stdin);
      sys.unit_test_line++;

      if (linebuf == NULL)
      {
         break; /* EOF */
      }

      if (linebuf != NULL)
      {
         /* Strip \n at the end */
         chop_newline(linebuf);

         /* Process the cmd line */
         ret_val = loc_test_default_cmdline_proc(linebuf);

         if (ret_val == LOC_CMD_EXIT_RET_CODE)
         {
            break; /* exit loop */
         }

         if (ret_val == LOC_CMD_CLEARERR_RET_CODE)
         {
            sys.app_code = 0;
            continue;
         }

         if (ret_val != 0 && ret_val != LOC_CMD_QUIET_RET_CODE)
         {
            fprintf(stderr, "Error (Code=%d)\n", ret_val);
            sys.app_code = ret_val;
         }
      }
   }

   return sys.app_code;
}

/*===========================================================================
FUNCTION    loc_test_default_cmdline_proc

DESCRIPTION
  Command line interpreter for the test application (default handler)

DEPENDENCIES
  N/A

RETURN VALUE
  Returns 0       if successful.
         -1       command not found
          n > 0   error code from the command
          n < 0   special return code

SIDE EFFECTS
  N/A
===========================================================================*/
int loc_test_default_cmdline_proc(const char* cmdline)
{
   int   argc;                          /* argument count */
   char *argv[LOC_MAX_ARG_NUM];     /* argument strings */
   char  linebuf[LOC_MAX_CMDLINE_LENGTH + 1];

   loc_test_cmd_handler_f_type* cmd_handler = NULL;

   strlcpy(linebuf, cmdline, LOC_MAX_CMDLINE_LENGTH + 1);

   /* Tokenize the cmdline */
   cmdline_tokenizer(linebuf, &argc, (char**) argv);

   if (argc > 0)
   {
      /* Skip comment lines */
      if (argv[0][0] == '#')
      {
         return 0;
      }

      /* Forward NI response */
      if (loc_ni_need_input)
      {
         return loc_ni_cmd_handler(argc, argv);
      }

      /* Find command handler */
      cmd_handler = find_cmd_handler(argv[0]);

      if (cmd_handler != NULL) {
         /* Log the command */
         loc_write_log("COMMAND: %s\n", cmdline);

         /* Init RPC if necessary */
         if (strncmp(argv[0], "loc", 3) == 0)
         {
            loc_test_init();
         }

         /* Process the command */
         int ret_code = cmd_handler(argc, (char**) argv);
         if (ret_code == 0)
         {
            loc_write_msg("OK\n");
         }
         return ret_code;
      }
      else {
         /* Developer commands */
         if (strcmp(argv[0], "build") == 0)
         {
            /* Display build info */
            loc_api_build();
            return 0;
         }
         /* Unknown command */
         return -1;
      }
   }

   /* Empty command line, no error. */
   return 0;
}
