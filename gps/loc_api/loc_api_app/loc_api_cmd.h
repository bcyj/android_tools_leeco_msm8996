/******************************************************************************
  @file:  loc_api_cmd.h
  @brief:  header for loc_api_cmd.c

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
03/17/09   dx       Android version
01/09/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cmd.h#9 $
======================================================================*/

#ifndef LOC_API_CMD_H
#define LOC_API_CMD_H

#define LOC_MAX_CMDLINE_LENGTH    512   /* Max command line length */
#define LOC_MAX_CMD_NAME_LENGTH   36    /* Max command name length */
#define LOC_MAX_CMD_ARG_LENGTH    128   /* Max length for each command line argument */
#define LOC_MAX_ARG_NUM           6     /* Max number of arguments */
#define LOC_MAX_CMD_HELP_LENGTH   128   /* Max length command help line */

#define LOC_CB_WAIT_SECONDS       5     /* Max wait time for loc_api function calls */

/* Special return value for command handlers */
#define LOC_CMD_EXIT_RET_CODE     (-99)     /* Exit the command loop */
#define LOC_CMD_QUIET_RET_CODE    (-98)     /* Successful but do not display OK */
#define LOC_CMD_CLEARERR_RET_CODE (-97)     /* Unit test only: clear all previous errors */

#define MIN(a,b) ( (a) <= (b) ? (a) : (b) )

/* Command loop function type */
typedef int loc_test_cmd_loop_f_type(void);

/* Command line processor function type */
typedef int loc_test_cmd_proc_f_type(const char *cmdline);

/* Command line extra help type */
typedef void loc_test_cmd_help_f_type(const char *fmt); /* fmt contains 2 strings, cmd and help */

/****************************************************************************
 * Command handler type
 *
 * Example:
 *                    loc_api_cmd arg1 arg2 ...
 *                              |
 *        +--------------+-------------+---------------+
 *        |              |             |               |
 *   loc_api_cmd        arg1          arg2            ...
 *        |              |             |
 *      argv[0]        argv[1]       argv[2]       argv[argc-1]
 *
 * Return 0 if successful, error code > 0 if error.
 * Should never return negative values (reserved for "command not found" error)
 *
 ***************************************************************************/
typedef int loc_test_cmd_handler_f_type(int argc, char* argv[]);

/* Command handler type */
typedef struct
{
  char                             *cmd_name;         /* command name */
  loc_test_cmd_handler_f_type      *cmd_handler;      /* pointer to cmd handler */
  char                             *cmd_help_str;     /* help string */
} loc_cmd_handler_s_type;

/* Command submenu entry type */
typedef struct
{
   char      key;
   char     *text;
} loc_submenu_entry_s_type;

/* Command submenu type */
typedef struct
{
   char                            *title;              /* Optional, NULL if no title      */
   char                             default_key;        /* Optional, '\0' if no default    */
   loc_submenu_entry_s_type         items[];            /* End with a null entry {0, NULL} */
} loc_submenu_s_type;

/****************************************************************************
 *                        EXPORTED DATA & FUNCTIONS
 ***************************************************************************/
extern loc_test_cmd_loop_f_type *loc_test_cmd_loop;
extern loc_test_cmd_proc_f_type *loc_test_cmdline_proc;
extern loc_cmd_handler_s_type loc_cmd_table[];

extern void loc_test_reg_cmd_loop(loc_test_cmd_loop_f_type *cmd_loop);
extern void loc_test_reg_cmd_proc(loc_test_cmd_proc_f_type *cmd_proc);
extern void loc_test_reg_cmd_extra_help(loc_test_cmd_help_f_type *cmd_help);

extern int loc_test_default_cmd_loop(void);
extern int loc_test_default_cmdline_proc(const char* cmdline);

extern int loc_test_checked_ioctl
(
      rpc_loc_ioctl_e_type ioctl_type,                  /* ioctl command */
      rpc_loc_ioctl_data_u_type* ioctl_data,      /* ioctl data */
      rpc_loc_event_payload_u_type* callback_payload,   /* callback buffer, NULL if not used */
      const char *error_tag                             /* msg displayed with errors */
);

#endif /* LOC_API_CMD_H */
