/******************************************************************************
  @file:  loc_api_cb_log.c
  @brief:  module for loc_api callback logging

  DESCRIPTION
     LOC_API callback logger

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

$Id: //source/qcom/qct/modem/gps/cgps/dev/locationmiddleware/locapi/app-linux/source/test/loc_api_test/loc_api_cb_log.c#10 $
======================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "loc_api_rpc_glue.h"
#include "loc_api_test.h"
#include "loc_api_data.h"
#include "loc_api_cb_log.h"
#include "loc_api_gui.h"

typedef struct
{
   char                 name[128];
   long                 val;
} loc_name_val_s_type;

#define NAME_VAL(x) {"" #x "", x }

/* Event names */
loc_name_val_s_type loc_test_event_name[] =
   {
      NAME_VAL( RPC_LOC_EVENT_PARSED_POSITION_REPORT ),
      NAME_VAL( RPC_LOC_EVENT_SATELLITE_REPORT ),
      NAME_VAL( RPC_LOC_EVENT_NMEA_1HZ_REPORT ),
      NAME_VAL( RPC_LOC_EVENT_NMEA_POSITION_REPORT ),
      NAME_VAL( RPC_LOC_EVENT_NI_NOTIFY_VERIFY_REQUEST ),
      NAME_VAL( RPC_LOC_EVENT_ASSISTANCE_DATA_REQUEST ),
      NAME_VAL( RPC_LOC_EVENT_LOCATION_SERVER_REQUEST ),
      NAME_VAL( RPC_LOC_EVENT_IOCTL_REPORT ),
      NAME_VAL( RPC_LOC_EVENT_STATUS_REPORT )
   };
int loc_event_num = sizeof loc_test_event_name / sizeof(loc_name_val_s_type);

/* IOCTL Status names */
loc_name_val_s_type loc_ioctl_status_name[] =
   {
      NAME_VAL( RPC_LOC_API_SUCCESS ),
      NAME_VAL( RPC_LOC_API_GENERAL_FAILURE ),
      NAME_VAL( RPC_LOC_API_UNSUPPORTED ),
      NAME_VAL( RPC_LOC_API_INVALID_HANDLE ),
      NAME_VAL( RPC_LOC_API_INVALID_PARAMETER ),
      NAME_VAL( RPC_LOC_API_ENGINE_BUSY ),
      NAME_VAL( RPC_LOC_API_PHONE_OFFLINE ),
      NAME_VAL( RPC_LOC_API_TIMEOUT )
   };
int loc_ioctl_status_num = sizeof loc_ioctl_status_name / sizeof(loc_name_val_s_type);

/* Fix session status names */
loc_name_val_s_type loc_sess_status_name[] =
   {
      NAME_VAL( RPC_LOC_SESS_STATUS_SUCCESS ),
      NAME_VAL( RPC_LOC_SESS_STATUS_IN_PROGESS ),
      NAME_VAL( RPC_LOC_SESS_STATUS_GENERAL_FAILURE ),
      NAME_VAL( RPC_LOC_SESS_STATUS_TIMEOUT ),
      NAME_VAL( RPC_LOC_SESS_STATUS_USER_END ),
      NAME_VAL( RPC_LOC_SESS_STATUS_BAD_PARAMETER ),
      NAME_VAL( RPC_LOC_SESS_STATUS_PHONE_OFFLINE ),
      NAME_VAL( RPC_LOC_SESS_STATUS_USER_END ),
      NAME_VAL( RPC_LOC_SESS_STATUS_ENGINE_LOCKED )
   };
int loc_sess_status_num = sizeof loc_sess_status_name / sizeof(loc_name_val_s_type);

/* Engine state names */
loc_name_val_s_type loc_engine_state_name[] =
   {
      NAME_VAL( RPC_LOC_ENGINE_STATE_ON ),
      NAME_VAL( RPC_LOC_ENGINE_STATE_OFF )
   };
int loc_engine_state_num = sizeof loc_engine_state_name / sizeof(loc_name_val_s_type);

/* Fix session state names */
loc_name_val_s_type loc_fix_session_state_name[] =
   {
      NAME_VAL( RPC_LOC_FIX_SESSION_STATE_BEGIN ),
      NAME_VAL( RPC_LOC_FIX_SESSION_STATE_END )
   };
int loc_fix_session_state_num = sizeof loc_fix_session_state_name / sizeof(loc_name_val_s_type);

/* Get names from value */
static char* loc_test_get_name_from_mask(loc_name_val_s_type table[], int table_size, long mask)
{
   int i;
   for (i = 0; i < table_size; i++)
   {
      if (table[i].val & (long) mask)
      {
         return table[i].name;
      }
   }
   return NULL;
}

/* Get names from value */
static char* loc_test_get_name_from_val(loc_name_val_s_type table[], int table_size, long value)
{
   int i;
   for (i = 0; i < table_size; i++)
   {
      if (table[i].val == (long) value)
      {
         return table[i].name;
      }
   }
   return NULL;
}

/* Finds the first event found in the mask */
char* loc_get_event_name(rpc_loc_event_mask_type loc_event_mask)
{
   return loc_test_get_name_from_mask(loc_test_event_name, loc_event_num,
         (long) loc_event_mask);
}

/* Finds IOCTL status name */
char* loc_get_ioctl_status_name(uint32 status)
{
   return loc_test_get_name_from_val(loc_ioctl_status_name, loc_ioctl_status_num,
         (long) status);
}

/* Finds session status name */
char* loc_get_sess_status_name(rpc_loc_session_status_e_type status)
{
   return loc_test_get_name_from_val(loc_sess_status_name, loc_sess_status_num,
         (long) status);
}

/* Find engine state name */
char* loc_get_engine_state_name(rpc_loc_engine_state_e_type state)
{
   return loc_test_get_name_from_val(loc_engine_state_name, loc_engine_state_num,
         (long) state);
}

/* Find engine state name */
char* loc_get_fix_session_state_name(rpc_loc_fix_session_state_e_type state)
{
   return loc_test_get_name_from_val(loc_fix_session_state_name, loc_fix_session_state_num,
         (long) state);
}

/* Logs parsed report */
static void log_parsed_report(const rpc_loc_parsed_position_s_type *parsed_report)
{
   rpc_loc_session_status_e_type status = parsed_report->session_status;
   loc_write_log("Session status: %s\n", loc_get_sess_status_name(status));
   loc_write_log("Longitude: %.3f\n", parsed_report->longitude);
   loc_write_log("Latitude:  %.3f\n", parsed_report->latitude);
}

/* Logs NMEA report */
static void log_nmea_report(const rpc_loc_nmea_report_s_type *nmea_report)
{
#ifndef FEATURE_RPC_CHAR_ARRAY
   loc_write_log("[%s]\n", nmea_report->nmea_sentences.nmea_sentences_val);
#else
   loc_write_log("[%s]\n", nmea_report->nmea_sentences);
#endif /* FEATURE_RPC_CHAR_ARRAY */
}

/* Logs status report */
static void log_status_report(const rpc_loc_status_event_s_type *status_event)
{
   rpc_loc_status_event_e_type event = status_event->event;
   switch (event) {
   case RPC_LOC_STATUS_EVENT_ENGINE_STATE:
      loc_write_log("Engine state: %s\n",
            loc_get_engine_state_name(
                  status_event->payload.rpc_loc_status_event_payload_u_type_u.engine_state));
      break;
   case RPC_LOC_STATUS_EVENT_FIX_SESSION_STATE:
      loc_write_log("Fix session state: %s\n",
            loc_get_fix_session_state_name(
                  status_event->payload.rpc_loc_status_event_payload_u_type_u.fix_session_state));
      break;
   default:
      break;
   }
}

#define CHECK_MASK(type, value, mask_var, mask) \
   ((mask_var & mask) ? (type) value : (type) (-1))

/* Logs valid fields in the GNSS SV constellation report */
static void log_satellite_report(const rpc_loc_gnss_info_s_type *gnss)
{
   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_POS_DOP)
   {
      loc_write_log("position dop: %.3f\n", (float) gnss->position_dop);
   }
   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_HOR_DOP)
   {
      loc_write_log("horizontal dop: %.3f\n", (float) gnss->horizontal_dop);
   }
   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_VERT_DOP)
   {
      loc_write_log("vertical dop: %.3f\n", (float) gnss->vertical_dop);
   }
   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_ALTITUDE_ASSUMED)
   {
      loc_write_log("altitude assumed: %d\n", (int) gnss->altitude_assumed);
   }
   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_SV_COUNT)
   {
      loc_write_log("sv count: %d\n", (int) gnss->sv_count);
   }
   if (gnss->valid_mask & RPC_LOC_GNSS_INFO_VALID_SV_LIST)
   {
      loc_write_log("sv list: ");

      if (gnss->sv_count)
      {
         loc_write_log("\n\tsys\tprn\thlth\tproc\teph\talm\telev\tazi\tsnr\n");
      }
      else {
         loc_write_log("empty\n");
      }

      int i;
      for (i = 0; i < gnss->sv_count; i++)
      {
         const rpc_loc_sv_info_s_type *sv = &gnss->sv_list.sv_list_val[i];
         rpc_loc_sv_info_valid_mask_type mask = sv->valid_mask;
         loc_write_log("  %d: ", i);
         loc_write_log("\t%d\t%d\t%d\t%d\t%d\t%d\t%.3f\t%.3f\t%.3f\n",
               CHECK_MASK(int,   sv->system,         mask, RPC_LOC_SV_INFO_VALID_SYSTEM),
               CHECK_MASK(int,   sv->prn,            mask, RPC_LOC_SV_INFO_VALID_PRN),
               CHECK_MASK(int,   sv->health_status,  mask, RPC_LOC_SV_INFO_VALID_HEALTH_STATUS),
               CHECK_MASK(int,   sv->process_status, mask, RPC_LOC_SV_INFO_VALID_PROCESS_STATUS),
               CHECK_MASK(int,   sv->has_eph,        mask, RPC_LOC_SV_INFO_VALID_HAS_EPH),
               CHECK_MASK(int,   sv->has_alm,        mask, RPC_LOC_SV_INFO_VALID_HAS_ALM),
               CHECK_MASK(float, sv->elevation,      mask, RPC_LOC_SV_INFO_VALID_ELEVATION),
               CHECK_MASK(float, sv->azimuth,        mask, RPC_LOC_SV_INFO_VALID_AZIMUTH),
               CHECK_MASK(float, sv->snr,            mask, RPC_LOC_SV_INFO_VALID_SNR)
         );
      }
   }
}

/* Logs a callback event header */
int loc_test_callback_log_header(
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   char time_string[1024]; /* full time string */
   char *event_name = loc_get_event_name(loc_event);
   if (event_name == NULL)
   {
      event_name = "UNKNOWN";
   }

   /* Event header */
   loc_write_log("\n[%s] Event 0x%lx: %s\n",
         loc_test_get_time(time_string),
         (long) loc_event, event_name);

   return 0;
}

/* Logs a callback event */
int loc_test_callback_log(
      rpc_loc_event_mask_type               loc_event,              /* event mask           */
      const rpc_loc_event_payload_u_type*   loc_event_payload       /* payload              */
)
{
   switch (loc_event)
   {
   case RPC_LOC_EVENT_SATELLITE_REPORT:
      log_satellite_report(&loc_event_payload->
            rpc_loc_event_payload_u_type_u.gnss_report);
      if (sys.sky_plot) { loc_update_sky_plot(&loc_event_payload->
            rpc_loc_event_payload_u_type_u.gnss_report); }
      break;
   case RPC_LOC_EVENT_STATUS_REPORT:
      log_status_report(&loc_event_payload->
            rpc_loc_event_payload_u_type_u.status_report);
      break;
   case RPC_LOC_EVENT_PARSED_POSITION_REPORT:
      log_parsed_report(&loc_event_payload->
            rpc_loc_event_payload_u_type_u.parsed_location_report);
      break;
   case RPC_LOC_EVENT_NMEA_POSITION_REPORT:
      log_nmea_report(&loc_event_payload->
            rpc_loc_event_payload_u_type_u.nmea_report);
      break;
   default:
      break;
   }

   return 0;
}
