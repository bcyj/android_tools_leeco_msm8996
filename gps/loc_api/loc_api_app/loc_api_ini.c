/******************************************************************************
  @file:  loc_api_ini.c
  @brief:

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
01/09/09   dx       Initial version

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_ini.c#6 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stringl.h>
#include <assert.h>
#include <ctype.h>
#ifdef USE_GLIB
#include <glib.h>
#endif /* USE_GLIB */
#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_ini.h"
#include "loc_api_data.h"
#include "platform_lib_includes.h"

/*=============================================================================
 *
 *                          GLOBAL DATA DECLARATION
 *
 *============================================================================*/

/* Parameter table */
loc_param_s_type loc_parameter_table[] =
{
  {"use_logcat",                  &param.use_logcat,              'n'},
  {"event_mask",                  &param.event_mask,              'n'},
  {"nmea_sentence_mask",          &param.nmea_sentence_mask,      'n'},
  {"engine_lock",                 &param.engine_lock,             'n'},
  {"operation_mode",              &param.fix_operation_mode,      'n'},
  {"fix_session_type",            &param.fix_sess_type,           'n'},
  {"time_between_fixes",          &param.time_between_fixes,      'n'},
  {"accuracy_threshold",          &param.accuracy_threshold,      'n'},
  {"fix_timeout",                 &param.fix_timeout,             'n'},
  {"enable_auto_download",        &param.enable_auto_download,    'n'},
  {"auto_download_period",        &param.auto_download_period,    'n'},
  {"xtra_server_addr",            &param.xtra_server_addr,        's'},
  {"sky_plot",                    &param.sky_plot,                'n'},
  {"data_request_reply",          &param.data_request_reply,      'n'},
  {"umts_profile_number",         &param.umts_pdp_profile_num,    'n'},
  {"apn_name",                    &param.apn_name,                's'}
};

int loc_param_num = sizeof(loc_parameter_table) / sizeof(loc_param_s_type);

/*===========================================================================
FUNCTION loc_reset_parameter

DESCRIPTION
   Resets the parameters

DEPENDENCIES
   N/A

RETURN VALUE
   None

SIDE EFFECTS
   N/A
===========================================================================*/

static void loc_reset_parameter(void)
{
   param.use_logcat = 1;
   param.event_mask = 0xFFFF;
   param.nmea_sentence_mask = 31;
   param.engine_lock = 0;
   param.fix_operation_mode = 4;
   param.fix_sess_type = 0;
   param.time_between_fixes = 60;
   param.accuracy_threshold = 255;
   param.fix_timeout = 60;
   param.sky_plot = 0;
   param.apn_name[0] = 0;
   param.xtra_server_addr[0] = 0;
   param.enable_auto_download = 1;
   param.auto_download_period = 1;
   param.data_request_reply = 0;
   param.umts_pdp_profile_num = 0;
   param.apn_name[0] = 0;
}

/*===========================================================================
FUNCTION loc_read_parameter

DESCRIPTION
   Reads the parameter file and sets global parameter data

DEPENDENCIES
   N/A

RETURN VALUE
   None

SIDE EFFECTS
   N/A
===========================================================================*/
void loc_read_parameter(void)
{
   char input_buf[LOC_MAX_PARAM_LINE];  /* declare a char array */
   char *lasts;
   char *param_name, *param_str_value;
   int  param_value;
   int i;

   loc_reset_parameter();

   if((loc_param_file_ptr = fopen(LOC_PARAMETER_FILE, "r")) != NULL)
   {
      loc_write_log("Using INI file: " LOC_PARAMETER_FILE "\n");
   }
   else
   {
      if ((loc_param_file_ptr = fopen(LOC_PARAMETER_FILE_ALT, "r")) != NULL)
      {
         loc_write_log("Using INI file: " LOC_PARAMETER_FILE_ALT "\n");
      }
      else
      {
         loc_write_log("Can not open parameter file");
         return; /* no parameter file */
      }
   }

   assert(loc_param_file_ptr); /* file must be opened here */
   while(fgets(input_buf, LOC_MAX_PARAM_LINE, loc_param_file_ptr) != NULL)
   {
      /* Separate variable and value */
      param_name = strtok_r(input_buf, "=", &lasts);
      if (param_name == NULL) continue;       /* skip lines that do not contain "=" */
      param_str_value = strtok_r(NULL, "=", &lasts);
      if (param_str_value == NULL) continue;  /* skip lines that do not contain two operands */

      /* Trim leading and trailing spaces */
      trim_space(param_name);
      trim_space(param_str_value);

      // printf("*(%s) = (%s)\n", param_name, param_str_value);

      /* Parse numerical value */
      if (param_str_value[0] == '0' && tolower(param_str_value[1]) == 'x')
      {
         /* hex */
         param_value = (int) strtol(&param_str_value[2], (char**) NULL, 16);
      }
      else {
         /* dec */
         param_value = atoi(param_str_value); /* dec */
      }

      for(i = 0; i < loc_param_num; i++)
      {
         if (strcmp(loc_parameter_table[i].param_name, param_name) == 0 &&
               loc_parameter_table[i].param_ptr)
         {
            switch (loc_parameter_table[i].param_type)
            {
            case 's':
               if (strcmp(param_str_value, "NULL") == 0)
               {
                  *((char*)loc_parameter_table[i].param_ptr) = '\0';
               }
               else {
                  strlcpy((char*) loc_parameter_table[i].param_ptr,
                        param_str_value,
                        LOC_MAX_PARAM_STRING + 1);
               }
               /* Log INI values */
               loc_write_log("PARAM %s = %s\n", param_name, loc_parameter_table[i].param_ptr);
               break;
            case 'n':
               *((int *)loc_parameter_table[i].param_ptr) = param_value;
               /* Log INI values */
               loc_write_log("PARAM %s = %d\n", param_name, param_value);
               break;
            default:
               assert(0); /* unknown param type */
            }
         }
      }
   }

   fclose(loc_param_file_ptr);
   loc_param_file_ptr = NULL;
}
