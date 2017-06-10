/******************************************************************************
  @file:  loc_api_test.c
  @brief: loc_api_test application framework

  DESCRIPTION

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
12/12/08   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_test.c#21 $
======================================================================*/

#define LOG_NDDEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <stringl.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>

#define LOG_TAG "loc_api_app"
#ifndef USE_GLIB
#include <utils/Log.h>
#endif /* USE_GLIB */

#ifdef USE_GLIB
#include <glib.h>
#endif /* USE_GLIB */


#include "loc_api_rpc_glue.h"
#include "loc_apicb_appinit.h"
#include "loc_api_test.h"
#include "loc_api_cb_hub.h"
#include "loc_api_cb_ni.h"
#include "loc_api_data.h"
#include "loc_api_cmd.h"
#include "loc_api_ini.h"
#include "loc_api_gui.h"
#include "platform_lib_includes.h"

#if defined(FEATURE_GPS_DCM_ENABLED) && !defined(FEATURE_GPS_DCM_ALWAYS_ON)
#  include "loc_api_cb_ds.h"
#endif /* FEATURE_GPS_DCM_ENABLED */

#if defined(FEATURE_GPS_DCM_ALWAYS_ON)
#  include "loc_api_cb_ds_on.h"
#endif /* FEATURE_GPS_DCM_ALWAYS_ON */

#define USE_COLOR 0

/* File handles */
FILE *loc_log_file_ptr = NULL;
FILE *loc_sub_log_file_ptr = NULL;   /* if open, log will be routed to this file */
FILE *loc_param_file_ptr = NULL;
FILE *loc_filelist_file_ptr = NULL;  /* script file list FP */

#define GPS_MAX_PARAM_LINE_LENGTH 80
unsigned long  gps_umts_pdp_profile_num;
char gps_apn_name[GPS_MAX_PARAM_LINE_LENGTH + 1];

pthread_mutex_t loc_unit_test_ticks_lock = PTHREAD_MUTEX_INITIALIZER;

/*===========================================================================

FUNCTION loc_test_get_time

DESCRIPTION
   Logs a callback event header.
   The pointer time_string should point to a buffer of at least 13 bytes:

   XX:XX:XX.000\0

RETURN VALUE
   The time string

===========================================================================*/
char *loc_test_get_time(char *time_string)
{
   struct timeval now;     /* sec and usec     */
   struct tm now_tm;       /* broken-down time */
   char hms_string[80];    /* HH:MM:SS         */

   gettimeofday(&now, NULL);
   localtime_r(&now.tv_sec, &now_tm);

   strftime(hms_string, sizeof hms_string, "%H:%M:%S", &now_tm);
   sprintf(time_string, "%s.%03d", hms_string, (int) (now.tv_usec / 1000));

   return time_string;
}

/*===========================================================================

FUNCTION chop_newline

DESCRIPTION
   Chops off end-line characters

RETURN VALUE
   none

===========================================================================*/
void chop_newline(char *line)
{
   static char *el_chars = "\r\n";
   int len = strlen(line);
   while (len && strchr(el_chars, line[len-1])) { len--; }
   line[len] = '\0';
}

/*===========================================================================
FUNCTION trim_space

DESCRIPTION
   Removes leading and trailing spaces of the string

DEPENDENCIES
   N/A

RETURN VALUE
   None

SIDE EFFECTS
   N/A
===========================================================================*/
void trim_space(char *org_string)
{
   char *scan_ptr, *write_ptr;
   char *first_nonspace = NULL, *last_nonspace = NULL;

   scan_ptr = write_ptr = org_string;

   while (*scan_ptr)
   {
      if ( !isspace(*scan_ptr) && first_nonspace == NULL)
      {
         first_nonspace = scan_ptr;
      }

      if (first_nonspace != NULL)
      {
         *(write_ptr++) = *scan_ptr;
         if ( !isspace(*scan_ptr))
         {
            last_nonspace = write_ptr;
         }
      }

      scan_ptr++;
   }

   if (last_nonspace) { *last_nonspace = '\0'; }
}

/*===========================================================================

FUNCTION loc_vwrite_log

DESCRIPTION
   If loc_sub_log_file_ptr is open, writes to it; otherwise,
   writes the log file LOC_MAIN_LOG_FILE (using va_list as argument).

RETURN VALUE
   None

===========================================================================*/
void loc_vwrite_log(char *str, va_list arglist)
{
   if (loc_sub_log_file_ptr != NULL)
   {
      /* Write to sub log file */
      vfprintf(loc_sub_log_file_ptr, str, arglist);
      fflush(loc_sub_log_file_ptr);
   }
   else {
      /* Write to main log file if sub file is closed */
      if (loc_log_file_ptr != NULL)
      {
         vfprintf(loc_log_file_ptr, str, arglist);
         fflush(loc_log_file_ptr);
      }
   }

   if (param.use_logcat)
   {
      char buf[2048];
      if (-1 != vsnprintf(buf, sizeof buf, str, arglist))
      {
         ALOGD("loc_api_app_msg %s",buf);
      }
   }
}

/*===========================================================================

FUNCTION loc_write_log

DESCRIPTION
  Writes the log file.

RETURN VALUE
  None

===========================================================================*/
#ifndef NO_LOC_API_APP_LOG
void loc_write_log(char *str, ...)
{
   va_list arglist;
   va_start(arglist, str);
   loc_vwrite_log(str, arglist);
   va_end(arglist);
}

/*===========================================================================

FUNCTION gps_write_log

DESCRIPTION
  Writes to the log file. This function emulates gps_write_log
  in gps_api_test.

DEPENDENCIES

RETURN VALUE
  None

SIDE EFFECTS

===========================================================================*/
void gps_write_log(char *str, ...)
{
  va_list arglist;
  va_start(arglist, str);
  loc_vwrite_log(str, arglist);
  loc_write_log("\n");
  va_end(arglist);
}

/*===========================================================================

FUNCTION loc_vwrite_error

DESCRIPTION
  Writes an error message to stderr and the log.

RETURN VALUE
  None

===========================================================================*/
void loc_vwrite_error(char *str, va_list arglist)
{
   /* Screen print */
#if USE_COLOR
   if (isatty(fileno(stdout))) { printf("\033[1;31m"); }  /* red */
#endif /* USE_COLOR */
   vprintf(str, arglist);
#if USE_COLOR
   if (isatty(fileno(stdout))) { printf("\033[0m"); }     /* reset color */
#endif /* USE_COLOR */

   /* Log file */
   loc_write_log("ERR ");
   loc_vwrite_log(str, arglist);
}

/*===========================================================================

FUNCTION loc_write_error

DESCRIPTION
  Writes an error message to stderr and the log.

RETURN VALUE
  None

===========================================================================*/
void loc_write_error(char *str, ...)
{
   va_list arglist;
   va_start(arglist, str);
   loc_vwrite_error(str, arglist);
   va_end(arglist);
}

/*===========================================================================

FUNCTION loc_write_msg

DESCRIPTION
  Writes an error message to stderr and the log.

RETURN VALUE
  None

===========================================================================*/
void loc_write_msg(char *str, ...)
{
   va_list arglist;
   va_start(arglist, str);
   vfprintf(stdout, str, arglist);
   loc_vwrite_log(str, arglist);
   va_end(arglist);
}
#endif
/*===========================================================================

FUNCTION loc_abort

DESCRIPTION
  Prints the farewell message and aborts the program

RETURN VALUE
  None

===========================================================================*/
int loc_abort(char *str, ...)
{
   va_list arglist;
   va_start(arglist, str);
   vfprintf(stderr, str, arglist);
   va_end(arglist);

   abort();
}

/*===========================================================================

FUNCTION loc_exit

DESCRIPTION
  Writes the application log and aborts the program

DEPENDENCIES

RETURN VALUE
  None

SIDE EFFECTS

===========================================================================*/
int loc_exit(int code)
{
   exit(code);
}

/*===========================================================================

FUNCTION    LOC_API_INIT

DESCRIPTION
   Initialize the application

DEPENDENCIES

RETURN VALUE

SIDE EFFECTS

===========================================================================*/
void loc_test_init(void)
{
   if (!sys.rpc_started)
   {
#ifdef FEATURE_LOC_API_RPCGEN
      if (!loc_api_glue_init())
      {
         loc_exit(-1);
      }
#else
      /* Initialize ONCRPC client for LOC_API */
      oncrpc_init();         /* init oncrpc */
      loc_apicb_app_init();  /* initialize remote client */
      oncrpc_task_start();
#endif /* FEATURE_LOC_API_RPCGEN */

      /* Simply ping the loc_api RPC server to check status */
      if (!loc_api_null())
      {
         char *err_oncrpc = "Cannot connect to LOC_API RPC server.\n";
         loc_write_error("%s\n", err_oncrpc);
         loc_exit(-1);
      }
      else
      {
         char *succ_oncrpc = "LOC_API RPC server connected.";
         loc_write_msg("%s\n", succ_oncrpc);
      }

      sys.rpc_started = 1;
   }

#if defined(FEATURE_GPS_DCM_ENABLED) || defined(FEATURE_GPS_DCM_ALWAYS_ON)
   if (!sys.dcm_started && !sys.disable_dcm)
   {
      /* Start Data call manager thread if it has not been started */
      if (gps_dcm_thread_start() == 0)
      {
         loc_write_msg("Data Call Manager started.\n");
         sys.dcm_started = 1;
         sleep(2); /* wait for DCM to initialize before return */
      }
      else {
         loc_write_error("Data Call Manager failed to start.\n");
      }
   }
#endif /* FEATURE_GPS_DCM_ENABLED */
}

/*===========================================================================

FUNCTION    on_signal_interrupt

DESCRIPTION
  Handler of SIGINT. Exists program upon SIGINT.

DEPENDENCIES

RETURN VALUE
  none

SIDE EFFECTS
 Exits program

===========================================================================*/
void on_signal_interrupt(int nParam)
{
   printf("<break> exit.\n");
   exit(-1);
}

/*===========================================================================

FUNCTION    close_client_resources

DESCRIPTION
   Closes all Loc API client resources at the end of each test

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
static void close_client_resources(void)
{
   if (sys.fix_started)
   {
      loc_stop_fix(sys.loc_handle);
      sys.fix_started = 0;
   }

   loc_gui_close();

   if (sys.loc_handle != -1)
   {
      loc_close(sys.loc_handle);
      sys.loc_handle = -1;
   }

   loc_apicb_app_deinit();
}

/*===========================================================================

FUNCTION    close_all_resources

DESCRIPTION
   Closes all files before exit.

DEPENDENCIES
   N/A

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
static void close_all_resources(void)
{
   fprintf(stderr, "Exiting...\n");

   close_client_resources();

   if (loc_log_file_ptr != NULL)
   {
      fclose(loc_log_file_ptr);
      loc_log_file_ptr = NULL;
   }
}

/*===========================================================================

FUNCTION    loc_api_welcome

DESCRIPTION
   Print welcome information.

DEPENDENCIES
   n/a

RETURN VALUE
   none

SIDE EFFECTS
   n/a

===========================================================================*/
void loc_api_welcome(void)
{
   printf("\n");
   printf("LOC API Testing Program V%s\n", LOC_API_TEST_VER);
   printf("Type 'help' for help; 'exit' to quit application.\n\n");
   printf("Get a position fix:\n  loc_open\n  loc_ioctl_set_fix_criteria\n  loc_start_fix ...\n");
   printf("\n");
}

/*===========================================================================
FUNCTION loc_api_build

DESCRIPTION
   Prints build time and configuration
===========================================================================*/
void loc_api_build(void)
{
   printf("LOC API Testing Program V%s\n", LOC_API_TEST_VER);
   printf("Built on %s %s\n", __DATE__, __TIME__);

   /*****************************************
    * Print features
    ****************************************/
   printf("DCM: ");
#ifdef FEATURE_GPS_DCM_ENABLED
   printf("yes\n");
#else
   printf("no\n");
#endif /* FEATURE_GPS_DCM_ENABLED */

   printf("CINT: ");
#ifdef FEATURE_SCRIPT
   printf("yes\n");
#else
   printf("no\n");
#endif /* FEATURE_SCRIPT */

   printf("LIBEDIT: ");
#ifdef HAS_LIBEDIT
   printf("yes\n");
#else
   printf("no\n");
#endif /* HAS_LIBEDIT */

   printf("READLINE: ");
#if defined(HAS_READLINE) && !defined(LIBEDIT)
   printf("yes\n");
#else
   printf("no\n");
#endif /* HAS_READLINE && !HAS_LIBEDIT */

   printf("LCD SKYPLOT: ");
#if defined(FEATURE_SKYPLOT)
   printf("yes\n");
#else
   printf("no\n");
#endif
}

/*===========================================================================

FUNCTION    loc_api_main_cmdline_help

DESCRIPTION
   Display help for command line

===========================================================================*/
static void loc_api_main_cmdline_help(void)
{
   printf("LOC API Testing Program V%s\n\n", LOC_API_TEST_VER);
   printf("  %-10s%s\n", "-s script", "Use script ");
   printf("  %-10s%s\n", "-t", "Script timeout (in seconds)");
   printf("  %-10s%s\n", "-l", "Script list file (one script name per line)");
   printf("  %-10s%s\n", "-L", "Specify log path (default " LOC_MAIN_LOG_FILE ")");
   printf("  %-10s%s\n", "-d", "Disable DCM operations");
   printf("\n");
}

/*===========================================================================

FUNCTION    loc_api_main_cmdline

DESCRIPTION
   Process the main command line, and sets:
   sys.unit_test_mode
   sys.unit_test_filelist
   sys.unit_test_timeout

RETURN VALUE
   0            successful
   err code     contains error

===========================================================================*/
int loc_api_main_cmdline(int argc, char *argv[])
{
   int argi = 1;                /* argument index */
   char *cur_arg = argv[argi];  /* load first argument */
   int show_help = 0;           /* show help msg */

   while (argi < argc)
   {
      /* Input script option: -s */
      if (strncmp(cur_arg, "-s", 2) == 0)
      {
         /* Set cur_arg to filename */
         if (strlen(cur_arg) > 2)
         {
            /* no space between -s and file name */
            cur_arg += 2;
         }
         else {
            /* take file name from next argument */
            cur_arg = argv[++argi];
            if (argi >= argc) /* no file name specified */
            {
               fprintf(stderr, "No script filename for -s\n");
               return -1;
            }
         }
         strlcpy(sys.unit_test_script, cur_arg, sizeof sys.unit_test_script);
         sys.unit_test_mode = 1;
      }

      /* Filelist option: -l */
      if (strncmp(cur_arg, "-l", 2) == 0)
      {
         /* Set cur_arg to filename */
         if (strlen(cur_arg) > 2)
         {
            /* no space between -l and file name */
            cur_arg += 2;
         }
         else {
            /* take file name from next argument */
            cur_arg = argv[++argi];
            if (argi >= argc) /* no file name specified */
            {
               fprintf(stderr, "No filelist filename for -l\n");
               return -1;
            }
         }
         strlcpy(sys.unit_test_filelist, cur_arg, sizeof sys.unit_test_filelist);
         sys.unit_test_mode = 1;
      }

      /* Unit test time out: -t */
      if (strncmp(cur_arg, "-t", 2) == 0)
      {
         /* Set cur_arg to filename */
         if (strlen(cur_arg) > 2)
         {
            /* no space between -t and time */
            cur_arg += 2;
         }
         else {
            /* take the time from next argument */
            cur_arg = argv[++argi];
            if (argi >= argc) /* no time specified */
            {
               fprintf(stderr, "No timeout argument for -t\n");
               return -1;
            }
         }
         int timeout = atoi(cur_arg);
         if (timeout)
         {
            sys.unit_test_timeout = timeout;
         }
      }

      /* Log file path: -L */
      if (strncmp(cur_arg, "-L", 2) == 0)
      {
         /* Set cur_arg to filename */
         if (strlen(cur_arg) > 2)
         {
            /* no space between -L and filename */
            cur_arg += 2;
         }
         else {
            /* take the filename from next argument */
            cur_arg = argv[++argi];
            if (argi >= argc) /* no time specified */
            {
               fprintf(stderr, "No log file argument for -L\n");
               return -1;
            }
         }
         strlcpy(sys.log_path, cur_arg, sizeof sys.log_path);
      }

      /* Disable DCM: -d */
      if (strncmp(cur_arg, "-d", 2) == 0)
      {
         sys.disable_dcm = 1;
      }

      /* Help: -h or -? */
      if (strncmp(cur_arg, "-h", 2) == 0 || strncmp(cur_arg, "-?", 2) == 0)
      {
         show_help = 1;
      }

      /* load next argument */
      cur_arg = argv[++argi];
   }

   if (show_help)
   {
      loc_api_main_cmdline_help();
      loc_exit(0);
   }

   return 0;
}

/*===========================================================================

FUNCTION    loc_timeout_thread_proc

DESCRIPTION
   Exit program in sys.unit_test_timeout seconds

DEPENDENCIES

RETURN VALUE
   exit code -9    if timed out

===========================================================================*/
void* loc_timeout_thread_proc(void *arg /* NULL */)
{
   while (1)
   {
      sleep(1);

      pthread_mutex_lock(&loc_unit_test_ticks_lock);
      sys.unit_test_ticks++;
      pthread_mutex_unlock(&loc_unit_test_ticks_lock);

      if (sys.unit_test_mode && sys.unit_test_script[0] &&
            sys.unit_test_ticks >= sys.unit_test_timeout)
      {
         break; /* time out */
      }
   }
   loc_write_error("TIMEOUT: %ds\n", sys.unit_test_timeout);
   printf("\n"); /* separate exiting msgs */
   loc_exit(-9);
   return NULL;
}

/*===========================================================================

FUNCTION    loc_at_exit

DESCRIPTION
   Cleans up all resources before exit

===========================================================================*/
void loc_at_exit(void)
{
   close_all_resources();
}

/*===========================================================================

FUNCTION    cmd

DESCRIPTION
   (Unit test only) Executes a test command, e.g., loc_open

===========================================================================*/
void cmd(const char *str)
{
   int rc = loc_test_default_cmdline_proc(str);
   if (rc != 0)
   {
      sys.app_code = rc;
   }
}

/*===========================================================================

FUNCTION    failure_code

DESCRIPTION
   (Unit test only) Report the error and abort the program

===========================================================================*/
void failure_code(char *str, int code)
{
   loc_write_error("%s\n", str);
   loc_write_msg("\n");
   sys.app_code = code;
   sys.unit_test_quit = 1;
}

/*===========================================================================

FUNCTION    failure

DESCRIPTION
   (Unit test only) Report the error and abort the program

===========================================================================*/
void failure(char *str)
{
   failure_code(str, -1);
}

/*===========================================================================

FUNCTION    verify_code

DESCRIPTION
   (Unit test only) If condition is not TRUE, then report error and abort
   program.

===========================================================================*/
void verify_code(int condition, int code)
{
   if (!condition)
   {
      if (sys.unit_test_mode)
      {
         loc_write_error("Verification failed at %s:%d\n", sys.unit_test_script,
               sys.unit_test_line);
         sys.app_code = code;
         sys.unit_test_quit = 1;
      }
      else {
         loc_write_error("Verification failed\n");
      }
   }
}

/*===========================================================================

FUNCTION    verify

DESCRIPTION
   (Unit test only) If condition is not TRUE, then report error and abort
   program.

===========================================================================*/
void verify(int condition)
{
   verify_code(condition, -1);
}

/*===========================================================================

FUNCTION    ok

DESCRIPTION
   (Unit test only) If condition is TRUE, then reset application error code,
   so the test case will succeed.

===========================================================================*/
void ok(int condition)
{
   if (condition)
   {
      sys.app_code = 0;
   }
}

#if defined LOC_TEST_DEBUG && LOC_TEST_DEBUG == 1
/*===========================================================================

FUNCTION    test_ni

DESCRIPTION
   For NI debugging, simulate a network-initiated event.  The parameter
   cmd = 1, 2 or 3, specifying which NI event is to be issued.

===========================================================================*/
void test_ni(int cmd)
{
   rpc_loc_ni_event_e_type ni_event;
   switch (cmd)
   {
   case 1:
      loc_write_msg("Simulating LOC_NI_EVENT_VX_NOTIFY_VERIFY_REQ\n");
      ni_event = RPC_LOC_NI_EVENT_VX_NOTIFY_VERIFY_REQ;
      break;
   case 2:
      loc_write_msg("Simulating LOC_NI_EVENT_UMTS_CP_NOTIFY_VERIFY_REQ\n");
      ni_event = RPC_LOC_NI_EVENT_UMTS_CP_NOTIFY_VERIFY_REQ;
      break;
   case 3:
      loc_write_msg("Simulating LOC_NI_EVENT_SUPL_NOTIFY_VERIFY_REQ\n");
      ni_event = RPC_LOC_NI_EVENT_SUPL_NOTIFY_VERIFY_REQ;
      break;
   default:
      loc_write_msg("Unknown cmd: %d. Valid values are 1-3.\n", cmd);
      ni_event = 0;
   }

   /* Simulate NI callback */
   if (ni_event != 0)
   {
      rpc_loc_event_payload_u_type cb_data;
      cb_data.rpc_loc_event_payload_u_type_u.ni_request.event = ni_event;

      /* XXX Maybe populate other fields in cb_data here */

      loc_test_callback_handler(NULL, sys.loc_handle,
            RPC_LOC_EVENT_NI_NOTIFY_VERIFY_REQUEST,
            &cb_data);
   }
}
#endif /* LOC_TEST_DEBUG */

/*===========================================================================

FUNCTION    get_next_script

DESCRIPTION
   Reads the next file name in the script list file into sys.unit_test_script.
If there's no more script, sys.unit_test_script is set to NULL.

DEPENDENCIES
   loc_filelist_file_ptr is open

RETURN VALUE
   Returns 0 if EOF

===========================================================================*/
static int get_next_script(void)
{
   char line[1024];
   if (loc_filelist_file_ptr != NULL)
   {
      while (fgets(line, sizeof line, loc_filelist_file_ptr))
      {
         chop_newline(line);
         trim_space(line);
         if (line[0] != '\0') {
            strlcpy(sys.unit_test_script, line, sizeof sys.unit_test_script);
            return 1; /* has a new script */
         }
      }
   }

   sys.unit_test_script[0] = '\0';
   return 0; /* EOF */
}

/*===========================================================================

FUNCTION    LOC_TEST_GENERAL_MAIN

DESCRIPTION
   This function is the main body of main(). It is called by a stub main() function
in either loc_api_test_main.c or loc_api_test_main_cint.c. The choice of the stub
depends on the Makefile. For a build that includes the C interpreter (CINT),
loc_api_test_main_cint.c will be included. For a build without CINT,
loc_api_test_main.c will be included.

DEPENDENCIES

RETURN VALUE
   Application exit code.

SIDE EFFECTS

===========================================================================*/
int loc_test_general_main(int argc, char *argv[])
{
   pthread_t time_thread;
   int has_more_scripts = 0;
   int total_app_code = 0, app_code = 0;
   int unit_test_timeout = 0;

   /* These functions must be registered before entering loc_test_general_main() */
   assert(loc_test_cmd_loop != NULL);
   assert(loc_test_cmdline_proc != NULL);

   /* Register exit clean-up function */
   atexit(loc_at_exit);

   /* Global data initialization */
   loc_api_data_init();

   /* Init signal */
   signal(SIGINT, on_signal_interrupt);

   /* Process command line to check if a filelist is specified */
   loc_api_main_cmdline(argc, argv);
   unit_test_timeout = sys.unit_test_timeout; /* save */

   /* Initialize log */
   if((loc_log_file_ptr = fopen(sys.log_path, "w")) == NULL)
   {
      printf("Can not write to log file: %s. Please check directory permission.\n",
            sys.log_path);
      loc_exit(-1);
   }

   /* Check if unit test script is specified */
   has_more_scripts = (sys.unit_test_script[0] != 0);

   /* If specified, open script list file */
   if (sys.unit_test_filelist[0] != '\0')
   {
      if ((loc_filelist_file_ptr = fopen(sys.unit_test_filelist, "r")) == NULL)
      {
         loc_write_msg("Can not open script list file: %s\n", sys.unit_test_filelist);
         loc_exit(-1);
      }
      has_more_scripts = get_next_script();
   }

   /* Start NI thread */
   // loc_ni_thread_start();

   /* Prepare timing thread to kill process when TIME OUT */
   // pthread_create(&time_thread, NULL, loc_timeout_thread_proc, (void*) NULL);

   /* Loop over unit test scripts */
   while (!sys.unit_test_mode || has_more_scripts) {

      /* Global data reset */
      loc_api_data_reset();
      sys.unit_test_timeout = unit_test_timeout; /* load */

      /* Read parameter file */
      loc_read_parameter();

      /* Welcome screen */
      loc_api_welcome();

      /* Start unit test mode */
      if (sys.unit_test_mode && sys.unit_test_script[0])
      {
         loc_write_msg("Unit test mode using %s...\n", sys.unit_test_script);
         loc_write_msg("Maximum time: %ds\n", sys.unit_test_timeout);

         /* Map the file to STDIN */
         if (NULL == freopen(sys.unit_test_script, "r", stdin))
         {
            fprintf(stderr, "Failed to open script file: %s\n", sys.unit_test_script);
            loc_exit(-1); /* exit */
         }
      }

      /* MAIN LOOP */
      app_code = loc_test_cmd_loop();

      /* Reset timer */
      pthread_mutex_lock(&loc_unit_test_ticks_lock);
      sys.unit_test_ticks = 0;
      pthread_mutex_unlock(&loc_unit_test_ticks_lock);

      /* Unit test result */
      if (sys.unit_test_mode)
      {
         loc_write_msg("%s\n", app_code ? "FAILED" : "PASSED");
      }

      /* Closes loc_api client handle, LCD, etc. */
      close_client_resources();

      /* Fails the app if any test fails */
      if (app_code != 0) { total_app_code = app_code; }

      /* Check next script, if a filelist is used */
      has_more_scripts = (loc_filelist_file_ptr != NULL && get_next_script());

      /* Interactive mode; do not loop */
      if (!sys.unit_test_mode)
      {
         break;
      }
   } /* while */

   /* Unit test result */
   if (loc_filelist_file_ptr != NULL)
   {
      loc_write_msg("Result of filelist %s:\n", sys.unit_test_filelist);
      loc_write_msg("%s\n", app_code ? "FAILED" : "PASSED");
      fclose(loc_filelist_file_ptr);
      loc_filelist_file_ptr = NULL;
   }

   return total_app_code;
}
