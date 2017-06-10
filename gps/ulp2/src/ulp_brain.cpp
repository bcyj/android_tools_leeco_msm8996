/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP brain logic module

GENERAL DESCRIPTION
  This file contains ULP brain logic to select position provider and its
  configuration.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <errno.h>
#include <sys/time.h>
#include <inttypes.h>

// Internal include files
#include <ulp_engine.h>

#include "ulp_quipc.h"

#include "loc_cfg.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

static int gnss_sv_report_cnt = 0;
static int gnss_pos_report_cnt = 0;

static int ulp_brain_turn_onoff_gnss_provider  ();
static int ulp_brain_turn_onoff_quipc_provider ();
static int ulp_brain_turn_onoff_gnp_provider   ();
static int ulp_brain_turn_onoff_zpp_provider   ();

/*===========================================================================
FUNCTION    ulp_brain_process_criteria_update

DESCRIPTION
   This function is invoked when a new position request is added or an
   existing request is removed.

   This function finds out the following pieces of info:
   (1) Any GPS provider request active or no.
       Info will be stored at: ulp_data.gps_provider_request_active
   (2) Any high accuracy fix request active or no.
       Info will be stored at: ulp_data.high_accuracy_request_active
   (3) Any low accuracy fix request active or no.
       Info will be stored at: ulp_data.low_accuracy_request_active
   (4) Fix recurrence and interval of the most frequent request.
       Info will be stored at: ulp_data.recurrence_type and ulp_data.fix_interval.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_criteria_update (void)
{
   int  ret_val = -1;
   int  i;

   // local flags for each criteria
   bool    high_accuracy_fix;
   uint32_t   fix_interval;

   ENTRY_LOG_CALLFLOW();
   LOC_LOGD ("%s: ulp_started = %d, phone context type = 0x%x\n",
         __func__,
         ulp_data.ulp_started,
         ulp_data.phoneSetting.context_type);

   // This function finds out the following pieces of info:
   // (1) Any GPS provider request active or no
   // (2) Any high accuracy fix request active or no
   // (3) Any low accuracy fix request active or no
   // (4) Fix recurrence and interval of the most frequent request
   ulp_data.gps_provider_request_active  = false;
   ulp_data.high_accuracy_request_active = false;
   ulp_data.low_accuracy_request_active  = false;
   ulp_data.recurrence_type              = ULP_LOC_RECURRENCE_SINGLE;
   ulp_data.fix_interval                 = 0;
   ulp_data.gnss_provider_info.new_request_active = false;
   ulp_data.gnp_provider_info.new_request_active  = false;

   // Go over each criteria
   for (i = 0; i < MAX_NUM_UNIQUE_CRITERIA; i++)
   {
      if (ulp_data.locationCriteriaArray[i].isUsed == true)
      {
         LOC_LOGI ("%s index at %d, isNew = %d, mask 0x%x, provider source = %d, accuracy = %d, recurrence = %d, tbf = %d\n",
               __func__,
               i,
               ulp_data.locationCriteriaArray[i].isNew,
               ulp_data.locationCriteriaArray[i].locationCriteria.valid_mask,
               ulp_data.locationCriteriaArray[i].locationCriteria.provider_source,
               ulp_data.locationCriteriaArray[i].locationCriteria.preferred_horizontal_accuracy,
               ulp_data.locationCriteriaArray[i].locationCriteria.recurrence_type,
               ulp_data.locationCriteriaArray[i].locationCriteria.min_interval);

         high_accuracy_fix = false;
         fix_interval      = 0;

         if ((ulp_data.locationCriteriaArray[i].locationCriteria.valid_mask & ULP_CRITERIA_HAS_PROVIDER_SOURCE) &&
             (ulp_data.locationCriteriaArray[i].locationCriteria.provider_source & ULP_PROVIDER_SOURCE_GNSS))
         {
            high_accuracy_fix = true;
            ulp_data.gps_provider_request_active = true;
         }
         else if ((ulp_data.locationCriteriaArray[i].locationCriteria.valid_mask & ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY ) &&
                  (ulp_data.locationCriteriaArray[i].locationCriteria.preferred_horizontal_accuracy == ULP_HORZ_ACCURACY_HIGH))
         {
            high_accuracy_fix = true;
            ulp_data.high_accuracy_request_active = true;
         }
         else
         {
            ulp_data.low_accuracy_request_active = true;
         }

         // If a new criteria comes in, we need to initiate a fix request ASAP
         if (ulp_data.locationCriteriaArray[i].isNew == true)
         {
            ulp_data.gnss_provider_info.new_request_active = true;
            ulp_data.gnp_provider_info.new_request_active  = true;
            ulp_data.quipc_provider_info.new_request_active = true;
            ulp_data.locationCriteriaArray[i].isNew = false;
         }

         // Figure out recurrence type and min interval
         if ((ulp_data.locationCriteriaArray[i].locationCriteria.valid_mask & ULP_CRITERIA_HAS_RECURRENCE_TYPE) &&
             (ulp_data.locationCriteriaArray[i].locationCriteria.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC))
         {
            ulp_data.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;

            // Set default minimal interval
            if (high_accuracy_fix == true)
            {
               fix_interval = ULP_HIGH_ACCURACY_MIN_TRACKING_INTERVAL_MSEC;
            }
            else
            {
               fix_interval = ULP_LOW_ACCURACY_MIN_TRACKING_INTERVAL_MSEC;
            }

            // If the request has larger interval than the default value, update the interval
            if (ulp_data.locationCriteriaArray[i].locationCriteria.valid_mask & ULP_CRITERIA_HAS_MIN_INTERVAL)
            {
               if (ulp_data.locationCriteriaArray[i].locationCriteria.min_interval > fix_interval)
               {
                  fix_interval = ulp_data.locationCriteriaArray[i].locationCriteria.min_interval;
               }
            }

            // Now update the main interval (upon first interval or smaller interval)
            if (ulp_data.fix_interval == 0 || ulp_data.fix_interval > fix_interval)
            {
               ulp_data.fix_interval = fix_interval;
            }
         }
      }
   }

   // Initialize some global variabls
   ulp_data.run_provider_selection_logic = true;
   if ((ulp_data.high_accuracy_request_active  == false) &&
       (ulp_data.low_accuracy_request_active == false))
   {
      // Reset the flags here so next time when high or low accuracy requet comes in,
      // it will attempt to report first position report ASAP
      ulp_data.position_selection_info.first_fix_pending = true;
      ulp_data.position_selection_info.last_report_position_type = ULP_POSITION_TYPE_UNKNOWN;
   }

   // Set run_monitor_thread based on request type
   pthread_mutex_lock (&ulp_data.monitor_mutex);
   if ((ulp_data.high_accuracy_request_active == true) &&
       (ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC))
   {
      if (ulp_data.run_monitor_thread == false)
      {
         ulp_data.run_monitor_thread = true;
         pthread_cond_signal (&ulp_data.monitor_cond);
      }
   }
   else
   {
      ulp_data.run_monitor_thread = false;
   }
   pthread_mutex_unlock (&ulp_data.monitor_mutex);

   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_phone_setting_update

DESCRIPTION
   This function is called when phone context setting has been changed.
   libulp brain will update provider state and other associated variables
   based on new phone setting.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_phone_setting_update ()
{
   int ret_val = -1;
   bool gps_enabled_copy,
           quipc_enabled_copy,
           gnp_enabled_copy;

   ENTRY_LOG_CALLFLOW();

   gps_enabled_copy = ulp_data.gnss_provider_info.enabled;
   quipc_enabled_copy = ulp_data.quipc_provider_info.enabled;
   gnp_enabled_copy = ulp_data.gnp_provider_info.enabled;

   if (ulp_data.phoneSetting.context_type & ULP_PHONE_CONTEXT_GPS_SETTING)
   {
      if (ulp_data.phoneSetting.is_gps_enabled == true)
      {
         ulp_data.gnss_provider_info.enabled = true;
         if (ulp_data.phoneSetting.is_enh_location_services_enabled == true) {
             LOC_LOGD("%s:%d]: EULA Enabled; Disabling ZPP", __func__, __LINE__);
             ulp_data.zpp_provider_info.enabled = false;
         }
         else {
             LOC_LOGD("%s:%d]: EULA Disabled; Enabling ZPP", __func__, __LINE__);
             ulp_data.zpp_provider_info.enabled = true;
         }
      }
      else
      {
         ulp_data.gnss_provider_info.enabled = false;
         ulp_data.zpp_provider_info.enabled = false;
      }
   }

   //QUIPS to be enabled only if all three WiFi ,Enhanced Location Services
   //and PIP User setting turned on
   if ((ulp_data.phoneSetting.context_type & ULP_PHONE_CONTEXT_WIFI_SETTING) &&
       (ulp_data.phoneSetting.context_type & ULP_PHONE_CONTEXT_ENH_LOCATION_SERVICES_SETTING) &&
       (ulp_data.phoneSetting.context_type & ULP_PHONE_CONTEXT_PIP_USER_SETTING))
   {
      if ((ulp_data.phoneSetting.is_wifi_setting_enabled == true) &&
          (ulp_data.phoneSetting.is_enh_location_services_enabled == true) &&
          (ulp_data.phoneSetting.is_pip_user_setting_enabled == true))
      {
         ulp_data.quipc_provider_info.enabled = true;
      }
      else
      {
         ulp_data.quipc_provider_info.enabled = false;
      }
   }

   if (ulp_data.phoneSetting.context_type & ULP_PHONE_CONTEXT_NETWORK_POSITION_SETTING )
   {
      if (ulp_data.phoneSetting.is_network_position_available == true)
      {
         ulp_data.gnp_provider_info.enabled = true;
      }
      else
      {
         ulp_data.gnp_provider_info.enabled = false;
      }
   }

   // QUIPC configuration item from gps.conf
   if (ulp_data.quipc_enabled == 0)
   {
      ulp_data.quipc_provider_info.enabled = false;
   }

   if ((gps_enabled_copy != ulp_data.gnss_provider_info.enabled) ||
       (quipc_enabled_copy != ulp_data.quipc_provider_info.enabled) ||
       (gnp_enabled_copy != ulp_data.gnp_provider_info.enabled) )
   {
      ulp_data.run_provider_selection_logic = true;
   }

   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return 0;
}

/*===========================================================================
FUNCTION    ulp_brain_select_providers

DESCRIPTION
   This function will evaulate all three providers based on recent position
   requests, phone context settings, provider state update.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_select_providers ()
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   if ((ulp_data.ulp_started == false) || (ulp_data.phoneSetting.context_type == 0))
   {
      LOC_LOGE ("%s, return when ulp_started = %d, phone context_type = 0x%x\n,",
                __func__,
                ulp_data.ulp_started,
                ulp_data.phoneSetting.context_type);
   }
   else
   {
      // Check when GNSS need to be started
      ulp_data.gnss_provider_info.selected_to_run = false;
      if ((ulp_data.gps_provider_request_active == true) ||
          (ulp_data.high_accuracy_request_active == true))
      {
         ulp_data.gnss_provider_info.selected_to_run = true;
      }

      // fallback to gnss provider if GNP provider is disabled and
      // we have low accuracy request
      if ((ulp_data.low_accuracy_request_active == true) &&
          (ulp_data.gnp_provider_info.enabled == false))
      {
         ulp_data.gnss_provider_info.selected_to_run = true;
      }

      // Check when QUIPC need to be started
      ulp_data.quipc_provider_info.selected_to_run = false;
      if (ulp_data.high_accuracy_request_active == true)
      {
         ulp_data.quipc_provider_info.selected_to_run = true;
      }

      // Check when GNP need to be started
      ulp_data.gnp_provider_info.selected_to_run = false;
      if ((ulp_data.high_accuracy_request_active == true) ||
          (ulp_data.low_accuracy_request_active == true))
      {
         ulp_data.gnp_provider_info.selected_to_run = true;
      }

      // Check if ZPP needs to be started.
      ulp_data.zpp_provider_info.selected_to_run = false;
      if (ulp_data.zpp_request_active == true)
      {
         ulp_data.zpp_provider_info.selected_to_run = true;
      }

      // Note: choose QUIPC provider prior to GNSS provider, as GNSS configuration depends
      // on QUIPC on-off state
      ulp_brain_turn_onoff_quipc_provider ();
      ulp_brain_turn_onoff_gnss_provider  ();

      // Initialize some global variabls
      if ((ulp_gnss_engine_running() == false) &&
          (ulp_quipc_engine_running() == false))
      {
         ulp_data.gnss_provider_info.onoff_cnt  = GNSS_TH_MAX;
         ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
         ulp_data.gnp_provider_info.onoff_cnt   = GNP_TH_MAX;
         ulp_data.zpp_provider_info.onoff_cnt   = ZPP_TH_MAX;
      }

      ulp_brain_turn_onoff_gnp_provider   ();

      // Turn onoff zpp provider last
      ulp_brain_turn_onoff_zpp_provider ();

      ulp_data.run_provider_selection_logic = false;
      ret_val = 0;
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_turn_onoff_zpp_provider

DESCRIPTION
   This function is called when to start/stop ZPP provider based
   on its recent state change.

   The following will trigger ZPP state change:
   (1) GPS enabled via UI
   (2) Addition or removal of fix requests that are criteria based or for
       the GPS provider
   (4) System event updates from Android system like charger_connected,
       locale changed etc

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_turn_onoff_zpp_provider ()
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, zpp enabled = %d, zpp selected to run = %d, onoff cnt = %d\n",
         __func__,
         ulp_data.zpp_provider_info.enabled,
         ulp_data.zpp_provider_info.selected_to_run,
         ulp_data.zpp_provider_info.onoff_cnt);

   if (ulp_data.zpp_provider_info.enabled == false)
   {
      ulp_zpp_stop_engine ();
   }
   else if ((ulp_data.zpp_provider_info.selected_to_run == true) &&
             (ulp_data.zpp_provider_info.onoff_cnt >= ZPP_TH_MAX))
   {
      ulp_zpp_start_engine ();
   }
   else if (ulp_data.zpp_provider_info.selected_to_run == false)
   {
      ulp_zpp_stop_engine ();
   }
   else if (ulp_data.zpp_provider_info.onoff_cnt <= ZPP_TH_MIN)
   {
      ulp_zpp_stop_engine ();
      ulp_data.zpp_provider_info.onoff_cnt = 0;
   }
   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_turn_onoff_gnp_provider

DESCRIPTION
   This function is called when to start/stop GNP provider based
   on its recent state change.

   The following will trigger GNP state change:
   (1) GNP enabled via UI
   (2) Addition or removal of fix requests that are criteria based
   (3) Recurrence type, fix interval changes of criteria based requests
   (4) GNSS provider state change, QUIPC provider state change,
       and GNP provider state change

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_turn_onoff_gnp_provider ()
{
   int       ret_val = -1;

   ENTRY_LOG_CALLFLOW();
   LOC_LOGD ("%s, gnp enabled = %d, gnp selected to run = %d, onoff cnt = %d, request active = %d,"
             "gnss running %d, gnss state %d quipc running %d\n",
         __func__,
         ulp_data.gnp_provider_info.enabled,
         ulp_data.gnp_provider_info.selected_to_run,
         ulp_data.gnp_provider_info.onoff_cnt,
         ulp_data.gnp_provider_info.new_request_active,
         ulp_gnss_engine_running(),
         ulp_data.gnss_provider_info.state,
         ulp_quipc_engine_running());

   // GNP currently will not report failure to ULP
   // Add safe guard logic to reset GNP state if we do not get the fix for 30 seconds
   if ((ulp_gnp_engine_running() == true) &&
       (ulp_data.gnp_provider_info.first_fix_pending == true))
   {
      if ((ulp_data.gnp_provider_info.last_started_time_ms + 30000) < ulp_util_get_time_ms())
      {
         ulp_gnp_stop_engine ();
      }
   }

   if (ulp_data.gnp_provider_info.enabled == false)
   {
      ulp_gnp_stop_engine ();
   }
   else if (ulp_data.gnp_provider_info.coarse_pos_req_pending == true)
   {
      ulp_gnp_start_engine ();
   }
   else if ((ulp_data.gnp_provider_info.selected_to_run == true) &&
            (ulp_data.gnp_provider_info.onoff_cnt >= GNP_TH_MAX) &&
            ((ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC) ||
            ((ulp_data.gnp_provider_info.new_request_active == true) &&
             ((ulp_gnss_engine_running() == false || ulp_data.gnss_provider_info.state == GNSS_STATE_FAILED) &&
              ulp_quipc_engine_running() == false))))
   {
      ulp_gnp_start_engine ();
   }
   else if (ulp_data.gnp_provider_info.selected_to_run == false)
   {
      ulp_gnp_stop_engine ();
   }
   else if (ulp_data.gnp_provider_info.onoff_cnt <= GNP_TH_MIN)
   {
      ulp_gnp_stop_engine ();
      ulp_data.gnp_provider_info.onoff_cnt = 0;
   }
   // If GNP engine can maintain its own state, then we
   // need to check whether tbf need to be changed
   // If so, call ulp_gnp_start_engine again
   else
   {
      if (ulp_gnp_engine_running() == true)
      {
         ulp_gnp_start_engine ();
      }
   }

   if (ulp_gnp_engine_running () == true)
   {
      ulp_data.gnp_provider_info.new_request_active = false;
   }

   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_turn_onoff_gnss_provider

DESCRIPTION
   This function is called when to start/stop GNSS provider based
   on its recent state change.

   The following will trigger QUIPC state change:
   (1) GPS enabled via UI
   (2) GPS Provider based or high accuracy fix request addition or
       removal
   (3) Recurrence type, fix interval and position mode changes of
       GPS provider based or high accuracy fix requests
   (4) GNSS provider state change, QUIPC provider state change,
       and GNP provider state change

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_turn_onoff_gnss_provider ()
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, gnss enabled = %d, gnss selected to run = %d, onoff cnt = %d, "
             "new request active = %d\n",
         __func__,
         ulp_data.gnss_provider_info.enabled,
         ulp_data.gnss_provider_info.selected_to_run,
         ulp_data.gnss_provider_info.onoff_cnt,
         ulp_data.gnss_provider_info.new_request_active);

   // GNSS engine start when
   // (1) gps provider request is active
   // (2) high accuracy criteria based request is active and
   //     GNSS onoff cnt has reached GNSS_TH_MAX
   if ((ulp_data.gnss_provider_info.enabled == false) ||
       (ulp_data.gnss_provider_info.selected_to_run == false))
   {
      ulp_gnss_stop_engine ();
   }
   else if (((ulp_data.gps_provider_request_active == true) ||
             (ulp_data.gnss_provider_info.onoff_cnt >= GNSS_TH_MAX)) &&
             ((ulp_data.gnss_provider_info.new_request_active == true) ||
              (ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC)))
   {
      ret_val = ulp_gnss_start_engine ();
      // GNSS has started, run the transition logic
      if (ret_val == 1)
      {
         ulp_brain_transition_all_providers ();
      }
   }
   else if (ulp_data.gnss_provider_info.new_request_active == true)
   {
      // If GNSS need to maintain its current on off state, and GNSS engine
      // is running, then we still call ulp_gnss_start_engine to reconfigure its
      // tbf etc.
      if (ulp_gnss_engine_running() == true)
      {
         ulp_gnss_start_engine ();
      }
   }
   else if (ulp_data.gnss_provider_info.onoff_cnt <= GNSS_TH_MIN)
   {
      // In case the current sesion fails and QUIPC can not be run,
      // there is no need to stop the GNSS session, as the underneath
      // laywer will initiate subsequent sessions
      //
      // Please note that the QUIPC engine need to be selected
      // prior to this function
      if ((ulp_quipc_engine_running() == true) &&
          (ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC))
      {
         ulp_gnss_stop_engine ();
         // Reset onoff_cnt to 0 for GNSS/QUIPC transition logic
         ulp_data.gnss_provider_info.onoff_cnt = 0;
      }
   }
   else
   {
      // If GNSS need to maintain its current on off state, and GNSS engine
      // is running, then we still call ulp_gnss_start_engine to reconfigure its
      // tbf etc.
      if (ulp_gnss_engine_running() == true)
      {
         ulp_gnss_start_engine ();
      }
   }

   if (ulp_gnss_engine_running () == true)
   {
      ulp_data.gnss_provider_info.new_request_active = false;
   }

   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_turn_onoff_quipc_provider

DESCRIPTION
   This function is called when to start/stop QUIPC provider based
   on its recent state change.

   The following will trigger QUIPC state change:
   (1) WIFI enabled via UI
   (2) GNSS provider state change (via received position report)
   (3) QUIPC provider state change (via QUIPC status and position report)
   (4) Client requests changes (whether high accuracy criteria based fix
       requests come in or go out

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_turn_onoff_quipc_provider ()
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, quipc enabled = %d, selected to run %d, onoff cnt = %d\n",
         __func__,
         ulp_data.quipc_provider_info.enabled,
         ulp_data.quipc_provider_info.selected_to_run,
         ulp_data.quipc_provider_info.onoff_cnt);

   // QUIPC engine start when:
   //  high accuracy criteria based request is active and
   //  GNSS onoff cnt has reached GNSS_TH_MAX
   if ((ulp_data.quipc_provider_info.enabled == false) ||
       (ulp_data.quipc_provider_info.selected_to_run == false))
   {
      ulp_quipc_stop_engine ();
   }
   else if ((ulp_data.quipc_provider_info.onoff_cnt >= QUIPC_TH_MAX) &&
            ((ulp_data.quipc_provider_info.new_request_active == true) ||
             (ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC)))
   {
      ret_val = ulp_quipc_start_engine ();
      // QUIPC has started, run the transition logic
      if (ret_val == 1)
      {
         ulp_brain_transition_all_providers ();
      }
   }
   // Transition logic turns quipc off
   else if (ulp_data.quipc_provider_info.onoff_cnt <= QUIPC_TH_MIN)
   {
      ulp_quipc_stop_engine ();
      // Reset onoff_cnt to 0, for GNSS/QUIPC transition logic
      ulp_data.quipc_provider_info.onoff_cnt = 0;
   }

   if (ulp_quipc_engine_running () == true)
   {
      ulp_data.quipc_provider_info.new_request_active = false;
   }

   ret_val = 0;
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_gnp_position_report

DESCRIPTION
   This function is called when GNP provider sends position report to ULP.

   libulp brain will decide whether to send the report to criteria based
   requests or as coarse position request to QUIPC

DEPENDENCIES
   None

RETURN VALUE
   0: All provider maintains its current state
   1: GNSS/QUIPC/GNP on/off state may need to be changed
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_gnp_position_report (const UlpLocation* location_ptr)
{
   int         ret_val = 0;
   int         ret_val_intermediate;
   uint64_t    current_time_msec;
   UlpLocation location_copy;
   bool     report_position = false;

   ENTRY_LOG_CALLFLOW();

   LOC_LOGV ("%s, gnp session status = %d, source: %d, flags = 0x%x, lat = %f, lon = %f, uncertainty = %f\n",
             __func__,
             LOC_SESS_SUCCESS,
             location_ptr->position_source,
             location_ptr->gpsLocation.flags,
             location_ptr->gpsLocation.latitude,
             location_ptr->gpsLocation.longitude,
             location_ptr->gpsLocation.accuracy);

   LOC_LOGI ("%s, first_fix_pending %d, request active = %d,"
          "gnss running %d, gnss state %d quipc running %d quipc state %d\n",
      __func__,
      ulp_data.position_selection_info.first_fix_pending,
      ulp_data.gnp_provider_info.new_request_active,
      ulp_gnss_engine_running(),
      ulp_data.gnss_provider_info.state,
      ulp_quipc_engine_running(),
      ulp_data.quipc_provider_info.state);

   // InNav position has valid lat/long
   if ((location_ptr->gpsLocation.flags & (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY)) ==
       (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY))
   {
      current_time_msec = ulp_util_get_time_ms();
      // Save the last received position
      ulp_data.gnp_provider_info.last_position_received = *location_ptr;
      ulp_data.gnp_provider_info.last_position_received_time_ms = current_time_msec;

      // GNP position report can only be sent to criteria based fix requests
      if ((ulp_data.low_accuracy_request_active == true) ||
          (ulp_data.high_accuracy_request_active == true))
      {
         if (ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC)
         {
            if ((ulp_data.gnss_provider_info.state != GNSS_STATE_HIGH) &&
                (ulp_data.gnss_provider_info.state != GNSS_STATE_MEDIUM) &&
                (ulp_data.gnss_provider_info.state != GNSS_STATE_LOW) &&
                (ulp_data.quipc_provider_info.state != QUIPC_STATE_HIGH) &&
                (ulp_data.quipc_provider_info.state != QUIPC_STATE_LOW))
            {
               report_position = true;
            }
         }
         else
         {
            if ((ulp_gnss_engine_running() == false ||
                 ulp_data.gnss_provider_info.state == GNSS_STATE_FAILED) &&
                ulp_quipc_engine_running()== false)
            {
               report_position = true;
            }
         }
      }
   }

   LOC_LOGI ("%s, report GNP position to hybrid fix requests = %d\n", __func__, report_position);

   if (report_position == true)
   {
      // Tag the position as hybrid
      location_copy = *location_ptr;
      location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
      location_copy.position_source = ULP_LOCATION_IS_FROM_HYBRID;
      GpsLocationExtended location_extended_dummy;

      ulp_msg_send_generic_position_report (LOC_SESS_SUCCESS,
                                            LOC_POS_TECH_MASK_WIFI | LOC_POS_TECH_MASK_CELLID,
                                            location_ptr,
                                            &location_extended_dummy,
                                            NULL);
      ulp_data.position_selection_info.first_fix_pending = false;
      ulp_data.position_selection_info.last_report_position_type = ULP_POSITION_TYPE_GNP;
      ulp_data.position_selection_info.last_position_report_time = current_time_msec;
   }

   // Send coarse position info back
   if (ulp_data.gnp_provider_info.coarse_pos_req_pending == true)
   {
#ifndef DEBUG_X86
      ulp_quipc_inf->send_coarse_position (location_ptr);
#endif
      ulp_data.gnp_provider_info.coarse_pos_req_pending = false;
      ret_val = 1;
   }

   // We have received first final fix, update the flag
   ulp_data.gnp_provider_info.first_fix_pending = false;

   ret_val_intermediate = ulp_brain_transition_all_providers ();
   if (ret_val_intermediate == 1)
   {
      ret_val = 1;
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}


/*===========================================================================
FUNCTION    ulp_brain_process_monitor_request

DESCRIPTION
   This function is called when GNSS engines and QUIPC engine fails to send
   back status. In this case, ulp brain will evaluate the engine and decide
   what to do.

DEPENDENCIES
   None

RETURN VALUE
   0: no error
   non-0: error

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_monitor_request ()
{
   uint64_t current_time_msec;
   uint64_t time_diff_msec;
   int      ret_val = 0;
   bool  run_transition_logic = false;

   do
   {
      LOC_LOGI ("%s, gnss provider state = %d, quipc provider state = %d\n",
                __func__,
                ulp_data.gnss_provider_info.state,
                ulp_data.quipc_provider_info.state);

      current_time_msec = ulp_util_get_time_ms ();

      if (ulp_gnss_engine_running () == true)
      {
         if ((ulp_data.gnss_provider_info.state == GNSS_STATE_HIGH) ||
             (ulp_data.gnss_provider_info.state == GNSS_STATE_MEDIUM) ||
             (ulp_data.gnss_provider_info.state == GNSS_STATE_LOW))
         {
            time_diff_msec = current_time_msec - ulp_data.gnss_provider_info.last_position_received_time_ms;

            // If the last received position is more than (1.5*interval) and
            // (interval+ULP_GNSS_NO_FINAL_POS_REPORT_THRESHOLD_MSEC)
            // old, then force to run transition logic
            if ((time_diff_msec * 2 > ulp_data.gnss_provider_info.fix_interval * 3) &&
                (time_diff_msec > (ulp_data.gnss_provider_info.fix_interval +
                                   ULP_GNSS_NO_FINAL_POS_REPORT_THRESHOLD_MSEC)))
            {
               run_transition_logic = true;
               // Update GNSS provider state to GNSS_STATE_INITIALIZING to trigger
               // GNSS and QUIPC transition. We would like to keep GNSS running
               // (by keeping GNSS state to INITIALIZING) and also get QUIPC to run
               ulp_gnss_set_state (GNSS_STATE_INITIALIZING);
            }
         }
         // GNSS transition to INITIALIZING state, run transition logic to see
         // whether we can start QUIPC or GNP provider
         else if (ulp_data.gnss_provider_info.state == GNSS_STATE_INITIALIZING)
         {
            run_transition_logic = true;
            // If GNSS has not been out of initializing state after the threshold time
            // and QUIPC has been stopped more than the threshold time, set GNSS to FAILURE
            // so, QUIPC provider will be run again
            if ((current_time_msec > (ulp_data.quipc_provider_info.last_stopped_time_ms +
                                      ULP_QUIPC_RESTART_TIME_THRESHOLD_MSEC)) &&
                (current_time_msec > (ulp_data.gnss_provider_info.last_started_time_ms +
                                      ULP_GNSS_INITIALIZING_THRESHOLD_MSEC)))
            {
               ulp_gnss_set_state (GNSS_STATE_FAILED);
            }
         }
      }

      if (ulp_quipc_engine_running () == true)
      {
         if ((ulp_data.quipc_provider_info.state == QUIPC_STATE_HIGH) ||
             (ulp_data.quipc_provider_info.state == QUIPC_STATE_LOW))
         {
            time_diff_msec = current_time_msec - ulp_data.quipc_provider_info.last_position_received_time_ms;

            // QUIPS is supposed to run at 1hz rate, so if we do not get status update
            // for more than 5 seconds, trigger transition logic
            if (time_diff_msec > ULP_QUIPC_NO_FINAL_POS_REPORT_THRESHOLD_MSEC)
            {
               run_transition_logic = true;
               ulp_quipc_set_state (QUIPC_STATE_FAILED);
            }
         }
         else if (ulp_data.quipc_provider_info.state == QUIPC_STATE_INITIALIZING)
         {
            // If QUIPC has not been out of initializing state after the threshold time
            // and GNSS has been stopped more than the threshold time, set QUIPS to FAILURE
            // so, GNSS provider will be run again
            if ((current_time_msec > (ulp_data.gnss_provider_info.last_stopped_time_ms +
                                      ULP_GNSS_RESTART_TIME_THRESHOLD_MSEC)) &&
                (current_time_msec > (ulp_data.quipc_provider_info.last_started_time_ms +
                                      ULP_QUIPC_INITIALIZING_THRESHOLD_MSEC)))
            {
               ulp_quipc_set_state (QUIPC_STATE_FAILED);
            }
            // Run provider transition logic to start GNP provider
            run_transition_logic = true;
         }
         else if (ulp_data.quipc_provider_info.state == QUIPC_STATE_LCI_TRANSITION)
         {
            if (current_time_msec > (ulp_data.quipc_provider_info.last_lci_transition_time_ms +
                                     ULP_QUIPC_LCI_TRANSITION_THRESHOLD_MSEC))
            {
               run_transition_logic = true;
               ulp_quipc_set_state (QUIPC_STATE_FAILED);
            }
         }
      }

      // None of the provider (GNSS or QUIPC) is running, run transition logic
      // to see whether network provider need to be run
      if ((ulp_gnss_engine_running () == false) &&
          (ulp_quipc_engine_running () == false))
      {
         run_transition_logic = true;

         // In case both GNSS and PIP are stopped, we initialize
         // the counter to max value if the engine has been stopped
         // long enough time (60 seconds of now)
         if (current_time_msec > (ulp_data.gnss_provider_info.last_stopped_time_ms +
                                  ULP_GNSS_RESTART_TIME_THRESHOLD_MSEC))
         {
            ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
         }

         if (current_time_msec > (ulp_data.quipc_provider_info.last_stopped_time_ms +
                                  ULP_QUIPC_RESTART_TIME_THRESHOLD_MSEC))
         {
            ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
         }
      }
   } while (0);

   if (run_transition_logic == true)
   {
      ret_val = ulp_brain_transition_all_providers ();

      // ulp_data.run_provider_selection_logic will be set to true
      // in function ulp_brain_transition_all_providers
      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   }

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_gnss_sv_report

DESCRIPTION
   This function is called when GNSS engines sends SV report to ULP.
   The SV report is coming at every 1 second.

   libulp brain will update GNSS/QUIPC provider state and other associated
   variables based on GNSS SV info.

DEPENDENCIES
   None.

RETURN VALUE
   0: All providers maintains its current state
   1: Providers on/off state have been changed
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_gnss_sv_report  (const GpsSvStatus* svStatusPtr)
{
   int              sv_index;
   const GpsSvInfo* svInfoPtr = NULL;

   gnss_sv_report_cnt++;

   ulp_data.gnss_provider_info.strong_sv_cnt = 0;

   svInfoPtr = &(svStatusPtr->sv_list[0]);
   for (sv_index = 0; sv_index < svStatusPtr->num_svs; sv_index++)
   {
      if ((((svInfoPtr->prn >= ULP_GPS_PRN_FIRST) && (svInfoPtr->prn <= ULP_GPS_PRN_LAST)) ||
          ((svInfoPtr->prn >= ULP_GLONASS_PRN_FIRST) && (svInfoPtr->prn <= ULP_GLONASS_PRN_LAST))) &&
          (svInfoPtr->snr >= (float) ULP_STRONG_SV_SNR_TH))
      {
         ulp_data.gnss_provider_info.strong_sv_cnt++;
      }
      svInfoPtr++;
   }

   LOC_LOGI ("%s, gnss sv report cnt = %d, gnss pos report cnt = %d,"
             "strong sv cnt = %d\n",
             __func__,
             gnss_sv_report_cnt,
             gnss_pos_report_cnt,
             ulp_data.gnss_provider_info.strong_sv_cnt);

   return 0;
}

/*===========================================================================
FUNCTION    ulp_brain_process_gnss_position_report

DESCRIPTION
   This function is called when GNSS engines sends status and new
   position report to ULP.

   libulp brain will update GNSS/QUIPC provider state and other associated
   variables based on GNSS info.

DEPENDENCIES
   Intermedate GNSS position reports are not being used for GNSS and QUIPC
   switching logic.

RETURN VALUE
   0: GNSS and QUIPC maintains its current state
   1: GNSS and QUIPC on/off state has changed
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_gnss_position_report (enum loc_sess_status status,
                                            LocPosTechMask       tech_mask,
                                            const UlpLocation* gps_location_ptr,
                                            const GpsLocationExtended* gps_location_extended_ptr,
                                            const void* gps_location_ext)
{
   int         ret_val = 0;
   bool     report_hybrid_position = false;
   UlpLocation location_copy;
   GpsLocationExtended location_extended_copy;

   ENTRY_LOG_CALLFLOW();

   LOC_LOGV ("ulp_brain_process_gnss_position_report, "
             "gnss session status = %d, tech mask = 0x%x, source: %d, flags = 0x%x, "
             "lat = %f, lon = %f, uncertainty = %f, strong sv count = %d, "
             "altitude = %f, altitude unc = %f, speed = %f, speed unc = %f\n",
             status,
             tech_mask,
             gps_location_ptr->position_source,
             gps_location_ptr->gpsLocation.flags,
             gps_location_ptr->gpsLocation.latitude,
             gps_location_ptr->gpsLocation.longitude,
             gps_location_ptr->gpsLocation.accuracy,
             ulp_data.gnss_provider_info.strong_sv_cnt,
             gps_location_ptr->gpsLocation.altitude,
             gps_location_extended_ptr->vert_unc,
             gps_location_ptr->gpsLocation.speed,
             gps_location_extended_ptr->speed_unc);

   do
   {
      if (ulp_data.gnss_provider_info.state == GNSS_STATE_IDLE)
      {
         LOC_LOGD ("%s, received position report at GNSS_STATE_IDLE, discard \n", __func__);
         break;
      }

      // If this is not an intermediate position, update GNSS state and high speed flag
      if (status == LOC_SESS_INTERMEDIATE)
      {
         break;
      }

      // We have received first final fix, update the flag,
      // We need to do this regardless whether fix is successful or failed
      ulp_data.gnss_provider_info.first_fix_pending = false;

      if (ulp_data.gnss_provider_info.tbf_update_pending == true )
      {
         ulp_data.run_provider_selection_logic = true;
         ulp_data.gnss_provider_info.tbf_update_pending = false;
      }

      if (status != LOC_SESS_SUCCESS)
      {
         ulp_gnss_set_state (GNSS_STATE_FAILED);
      }
      // Successful fix: sensor or satellite computed
      else if ((tech_mask & LOC_POS_TECH_MASK_SATELLITE) != 0 ||
               (tech_mask & LOC_POS_TECH_MASK_SENSORS) != 0)
      {
         // GNSS final position cnt
         gnss_pos_report_cnt++;

         // First, update GNSS providers state with new GNSS info
         if ((gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_LAT_LONG) &&
             (gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_ACCURACY))
         {
            // Record down last received position report and its timestamp
            ulp_data.gnss_provider_info.last_position_received = *gps_location_ptr;
            ulp_data.gnss_provider_info.last_position_received_time_ms = ulp_util_get_time_ms();

            if ((gps_location_ptr->gpsLocation.accuracy < ULP_GNSS_HIGH_STATE_ACCURACY_TH) &&
                (ulp_data.gnss_provider_info.strong_sv_cnt >= ULP_STRONG_SV_CNT_TH))
            {
               ulp_gnss_set_state (GNSS_STATE_HIGH);
            }
            else if (gps_location_ptr->gpsLocation.accuracy > ULP_GNSS_MEDIUM_STATE_ACCURACY_TH)

            {
               ulp_gnss_set_state (GNSS_STATE_LOW);
            }
            else
            {
               ulp_gnss_set_state (GNSS_STATE_MEDIUM);
            }
         }
         else
         {
            LOC_LOGE ("%s, received final gnss report without lat and long\n", __func__);
         }

         // Update high speed flag
         ulp_data.gnss_provider_info.high_speed = false;
         if ((gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_SPEED) &&
             (gps_location_ptr->gpsLocation.speed >= ULP_GNSS_HIGH_SPEED_TH))
         {
            ulp_data.gnss_provider_info.high_speed = true;
         }
      }
      else
      {
         // First, update GNSS providers state with new GNSS info
         if ((gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_LAT_LONG) &&
             (gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_ACCURACY))
         {
            // Record down last received position report and its timestamp
            ulp_data.gnss_provider_info.last_position_received = *gps_location_ptr;
            ulp_data.gnss_provider_info.last_position_received_time_ms = ulp_util_get_time_ms();
            ulp_gnss_set_state (GNSS_STATE_LOW);
         }
         else
         {
            LOC_LOGE ("%s, received final gnss report without lat and long\n", __func__);
         }

         ulp_data.gnss_provider_info.high_speed = false;
      }

      // Figure out whether we need to report this position
      // to criteria based fix request
      if ((ulp_data.high_accuracy_request_active == true) ||
          (ulp_data.low_accuracy_request_active == true))
      {
         // GNSS has valid position report in those three states
         if ((status == LOC_SESS_SUCCESS) &&
             (gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_LAT_LONG) &&
             (gps_location_ptr->gpsLocation.flags & GPS_LOCATION_HAS_ACCURACY))
         {
            report_hybrid_position = true;

            // GNSS is of high speed, report GNSS position
            // QUIPC will be turned off soon
            if (ulp_data.gnss_provider_info.high_speed == true)
            {
               report_hybrid_position = true;
            }
            else if (ulp_data.quipc_provider_info.state == QUIPC_STATE_HIGH)
            {
               report_hybrid_position = false;
            }
         }
      }

      // Run the provider transition based on new GNSS state
      ret_val = ulp_brain_transition_all_providers ();

   } while (0);


   LOC_LOGI ("%s, transition provider returned = %d\n", __func__, ret_val);
   LOC_LOGI ("%s, report GNSS position/status to hybrid fix requests = %d\n",
             __func__,
             report_hybrid_position);

   // To preserve current GPS provider behavior:
   // Tag the position as GNSS and send it out
   // Work for both intermediate and final position reports
   if(ulp_data.gps_provider_request_active == true )
   {
      LOC_LOGD ("%s, report GNSS position to GPS provider fix requests\n", __func__);
      location_extended_copy = *gps_location_extended_ptr;
      location_copy = *gps_location_ptr;
      location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
      location_copy.position_source = ULP_LOCATION_IS_FROM_GNSS;
      ulp_msg_send_generic_position_report (status, tech_mask, &location_copy, &location_extended_copy, gps_location_ext);
   }

   if (report_hybrid_position == true)
   {
      ulp_data.position_selection_info.first_fix_pending = false;
      ulp_data.position_selection_info.last_report_position_type = ULP_POSITION_TYPE_GNSS;

      location_copy = *gps_location_ptr;
      // TAG the position as HYBRID and send it out
      location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
      location_copy.position_source = (ULP_LOCATION_IS_FROM_HYBRID|ULP_LOCATION_IS_FROM_GNSS);
      GpsLocationExtended location_extended_dummy;

#ifndef DEBUG_X86
      // Check & get the debug info, if we need to report the debug info.
      ulp_debug_get_debug_info (&location_copy.rawData, &location_copy.rawDataSize);
#endif

      ulp_msg_send_generic_position_report (status, tech_mask, &location_copy, &location_extended_dummy, NULL);
   }

   // Last, send out position report to XT-WLAN module for crowd sourcing
   ulp_xtwifi_send_position (GPS_PROVIDER_NAME, status, tech_mask,
                             gps_location_ptr, gps_location_extended_ptr);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_quipc_position_report

DESCRIPTION
   This function is called when QUIPC engines sends status and new
   position report to ULP.

   libulp brain will update GNSS/QUIPC provider state and other associated
   variables based on QUIPC status and position info.

DEPENDENCIES
   None

RETURN VALUE
   0: GNSS and QUIPC maintains its current state
   1: GNSS and QUIPC on/off state has changed
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_brain_process_quipc_position_report (int quipc_error_code,
                                             const UlpLocation* quipc_location_ptr)
{
   UlpLocation     location_copy;
   bool            report_position = false;
   int             ret_val = 0;
   loc_sess_status status = LOC_SESS_FAILURE;

   do
   {
      ENTRY_LOG_CALLFLOW();

      LOC_LOGV ("ulp_brain_process_quipc_position_report, "
                "quipc session status = %d, source: %d, flags = 0x%x, lat = %f, lon = %f, uncertainty = %f\n",
                quipc_error_code,
                quipc_location_ptr->position_source,
                quipc_location_ptr->gpsLocation.flags,
                quipc_location_ptr->gpsLocation.latitude,
                quipc_location_ptr->gpsLocation.longitude,
                quipc_location_ptr->gpsLocation.accuracy);

      if (ulp_data.quipc_provider_info.state == QUIPC_STATE_IDLE)
      {
         LOC_LOGD ("%s, received position report at QUIPC_STATE_IDLE, discard \n", __func__);
         // Need to free the memory if any in the raw data
         if (quipc_location_ptr->rawData != NULL)
         {
           delete (char*)quipc_location_ptr->rawData;
         }
         break;
      }

      // Update QUIPC state based on the new status and QUIPC position report
      if (quipc_error_code == ULP_QUIPC_STATUS_NO_ERROR)
      {
         ulp_quipc_set_state (QUIPC_STATE_HIGH);
         status = LOC_SESS_SUCCESS;
      }
      else if (quipc_error_code == ULP_QUIPC_STATUS_POSITION_OUTSIDE_LCI_BOUNDARY ||
               quipc_error_code == ULP_QUIPC_STATUS_NO_WIFI_MEAS)
      {
         ulp_quipc_set_state (QUIPC_STATE_LOW);
         status = LOC_SESS_SUCCESS;
      }
      else if (quipc_error_code == ULP_QUIPC_STATUS_NO_LCI_WITHIN_VICINITY)
      {
         ulp_quipc_set_state (QUIPC_STATE_OUT_OF_COVERAGE);
      }
      else if (quipc_error_code == ULP_QUIPC_STATUS_LCI_TRANSITION)
      {
         ulp_quipc_set_state (QUIPC_STATE_LCI_TRANSITION);
      }
      else  // all other errors
      {
         ulp_quipc_set_state (QUIPC_STATE_FAILED);
      }

      // Then, we decide whether to reprot the QUIPC position to hybrid fix request
      // Handle the case that QUIPC does not report lat/lon and accuracy
      if ((quipc_location_ptr->gpsLocation.flags & (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY)) ==
          (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY))
      {
         // First, update QUIPC providers state with newly received position info
         ulp_data.quipc_provider_info.last_position_received = *quipc_location_ptr;
         ulp_data.quipc_provider_info.last_position_received_time_ms = ulp_util_get_time_ms();

         if ((ulp_data.high_accuracy_request_active == true) ||
             (ulp_data.low_accuracy_request_active == true))
         {
            // QUIPC fix that is within LCI, always report it
            if (ulp_data.quipc_provider_info.state == QUIPC_STATE_HIGH)
            {
               report_position = true;
            }
            // QUIPC fix that is outside of LCI boundary or dead-reckoning, but first fix, still report
            else if (ulp_data.position_selection_info.first_fix_pending == true)
            {
               report_position = true;
            }
         }
      }

      // Then, we run the provider transition based on new QUIPC state
      ret_val = ulp_brain_transition_all_providers ();
   } while (0);


   LOC_LOGI ("%s, report QUIPC position/status to hybrid fix requests = %d\n", __func__, report_position);
   if (report_position == true)
   {
      ulp_data.position_selection_info.first_fix_pending = false;
      ulp_data.position_selection_info.last_report_position_type = ULP_POSITION_TYPE_QUIPC;

      location_copy = *quipc_location_ptr;
      // TAG the position as hybrid and send it out
      location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
      location_copy.position_source = ULP_LOCATION_IS_FROM_HYBRID;
      GpsLocationExtended location_extended_dummy;

#ifndef DEBUG_X86
      // Check & get the debug info, if we need to report the debug info.
      ulp_debug_get_debug_info (&location_copy.rawData, &location_copy.rawDataSize);
#endif

      ulp_msg_send_generic_position_report (LOC_SESS_SUCCESS, LOC_POS_TECH_MASK_WIFI, &location_copy, &location_extended_dummy, NULL);
   }

   // Last, send out position report to XT-WLAN module for crowd sourcing
   ulp_xtwifi_send_position (PIP_PROVIDER_NAME, status, LOC_POS_TECH_MASK_WIFI,
                             quipc_location_ptr, NULL);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_zpp_position_report

DESCRIPTION
   This function is called when ZPP provider sends position report to ULP.

   libulp brain will decide whether to send the report to criteria based
   requests

DEPENDENCIES
   None

RETURN VALUE
   0: Success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_zpp_position_report (const enum loc_sess_status status,
                                           LocPosTechMask     tech_mask,
                                           const UlpLocation* location_ptr)
{
   int         ret_val = -1;
   uint64_t    current_time_msec;
   UlpLocation location_copy;
   bool report_position = false;

   ENTRY_LOG_CALLFLOW();

   memset(&location_copy, 0, sizeof (UlpLocation));

   LOC_LOGV ("%s, zpp session status = %d, source: %d, flags = 0x%x, lat = %f, "
             "lon = %f, uncertainty = %f, tech_mask = 0x%x\n",
             __func__,
             LOC_SESS_SUCCESS,
             location_ptr->position_source,
             location_ptr->gpsLocation.flags,
             location_ptr->gpsLocation.latitude,
             location_ptr->gpsLocation.longitude,
             location_ptr->gpsLocation.accuracy,
             tech_mask);

   LOC_LOGV ("%s, first_fix_pending %d, request active = %d,"
             "zpp running %d, zpp state %d \n",
            __func__,
            ulp_data.position_selection_info.first_fix_pending,
            ulp_data.zpp_request_active,
            ulp_zpp_engine_running(),
            ulp_data.zpp_provider_info.state);

   // if ZPP position has valid lat/long
   if ((location_ptr->gpsLocation.flags & (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY)) ==
       (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY))
   {
      current_time_msec = ulp_util_get_time_ms();
      // Save the last received position
      ulp_data.zpp_provider_info.last_position_received = *location_ptr;
      ulp_data.zpp_provider_info.last_position_received_time_ms = current_time_msec;

      if (ulp_data.zpp_request_active == true)
      {
         GpsLocationExtended location_extended_dummy;

         location_copy = *location_ptr;
         location_copy.rawDataSize     = 0;
         location_copy.rawData         = NULL;

         // ZPP position report can only be sent to FLP only if it has no clients
         if((ulp_data.high_accuracy_request_active == false)&&
            (ulp_data.low_accuracy_request_active == false))
         {
            report_position = true;
            // Tag the position as hybrid

            location_copy.position_source = ULP_LOCATION_IS_FROM_HYBRID;
            location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
            ulp_msg_send_generic_position_report (LOC_SESS_SUCCESS, tech_mask,
                                                  &location_copy, &location_extended_dummy, NULL);
         }

         // ZPP position report can only be sent to GLP if has no clients
         // and the ZPP position is from the GNSS engine
         if((ulp_data.gps_provider_request_active == false)&&
            (tech_mask & LOC_POS_TECH_MASK_SATELLITE ))
         {
             enum loc_sess_status status = (location_copy.gpsLocation.accuracy <= 50) ?
                                            LOC_SESS_SUCCESS : LOC_SESS_INTERMEDIATE;
            report_position = true;
            location_copy.position_source = ULP_LOCATION_IS_FROM_GNSS;
            location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
            ulp_msg_send_generic_position_report (status, tech_mask,
                                                  &location_copy, &location_extended_dummy, NULL);
         }
      }
   }

   LOC_LOGI ("%s, report ZPP position to providers,report_position = %d\n",
             __func__, report_position);

   if (report_position == true)
   {
      ulp_data.position_selection_info.last_report_position_type = ULP_POSITION_TYPE_ZPP;
      ulp_data.position_selection_info.last_position_report_time = current_time_msec;
      ret_val = 0;
   }

   // We have received first final fix, update the flag
   ulp_data.zpp_provider_info.first_fix_pending = false;

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_geofence_position_report

DESCRIPTION
   This function is called when geofence adapter sends position report to
   ULP after a geofence breach.

   libulp brain will decide whether to send the report to criteria based
   requests

DEPENDENCIES
   None

RETURN VALUE
   0: Success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_geofence_position_report (const UlpLocation* location_ptr)
{
   int         ret_val = -1;
   UlpLocation location_copy;
   bool report_position = false;
   LocPosTechMask tech_mask = LOC_POS_TECH_MASK_HYBRID;
   ENTRY_LOG_CALLFLOW();

   memset(&location_copy, 0, sizeof (UlpLocation));

   LOC_LOGV ("%s, flags = 0x%x, lat = %f, "
             "lon = %f, uncertainty = %f, tech_mask = 0x%x\n",
             __func__,
             location_ptr->gpsLocation.flags,
             location_ptr->gpsLocation.latitude,
             location_ptr->gpsLocation.longitude,
             location_ptr->gpsLocation.accuracy,
             tech_mask);

   // if geofence position has valid lat/long
   if ((location_ptr->gpsLocation.flags & (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY)) ==
       (GPS_LOCATION_HAS_LAT_LONG | GPS_LOCATION_HAS_ACCURACY))
   {
       GpsLocationExtended location_extended_dummy;

       location_copy = *location_ptr;
       location_copy.rawDataSize     = 0;
       location_copy.rawData         = NULL;

       // position report can only be sent to FLP only if it has no clients
       if((ulp_data.high_accuracy_request_active == false)&&
          (ulp_data.low_accuracy_request_active == false))
       {
           report_position = true;
           // Tag the position as hybrid

           location_copy.position_source = ULP_LOCATION_IS_FROM_HYBRID;
           location_copy.gpsLocation.flags |= LOCATION_HAS_SOURCE_INFO;
           ulp_msg_send_generic_position_report (LOC_SESS_SUCCESS, tech_mask,
                                                 &location_copy, &location_extended_dummy, NULL);
           ret_val = 0;
       }
   }


   LOC_LOGI ("%s, report geofence position to providers,report_position = %d\n",
             __func__, report_position);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_transition_gnss_quipc_running_concurrently

DESCRIPTION
   This function implements ULP transition logic when both QUIPC and GNSS
   are considerred running in the transition logic.

   libulp brain will decide to dynamically turn on/off quipc based on
   the newly recevied status from both GNSS and QUIPC.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_transition_gnss_quipc_running_concurrently ()
{
   int ret_val = 0;

   ENTRY_LOG_CALLFLOW();
   if (ulp_data.quipc_provider_info.state == QUIPC_STATE_INITIALIZING)
   {
      if (ulp_data.gnss_provider_info.state == GNSS_STATE_HIGH)
      {
         ulp_data.gnss_provider_info.onoff_cnt = 0;
         ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MIN;
      }
      else if (ulp_data.gnss_provider_info.state == GNSS_STATE_FAILED)
      {
         ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MIN;
         ulp_data.quipc_provider_info.onoff_cnt = 0;
      }
      else
      {
         ulp_data.gnss_provider_info.onoff_cnt = 0;
         ulp_data.quipc_provider_info.onoff_cnt = 0;
      }
  }
  else if (ulp_data.quipc_provider_info.state == QUIPC_STATE_HIGH)
  {
     ulp_data.quipc_provider_info.onoff_cnt = 0;

     if (ulp_data.gnss_provider_info.state == GNSS_STATE_HIGH)
     {
        ulp_data.gnss_provider_info.onoff_cnt = 0;
     }
     else if (ulp_data.gnss_provider_info.state == GNSS_STATE_MEDIUM)
     {
        if (ulp_data.gnss_provider_info.onoff_cnt > GNSS_TH_MIN)
        {
           ulp_data.gnss_provider_info.onoff_cnt--;
        }
     }
     else if (ulp_data.gnss_provider_info.state == GNSS_STATE_INITIALIZING ||
              ulp_data.gnss_provider_info.state == GNSS_STATE_LOW ||
              ulp_data.gnss_provider_info.state == GNSS_STATE_FAILED)
     {
        ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MIN;
     }
     else
     {
        LOC_LOGE ("%s, line %d, unknown GNSS state %d\n",
                  __func__, __LINE__, ulp_data.gnss_provider_info.state);
        ret_val = -1;
     }
  }
  else if (ulp_data.quipc_provider_info.state == QUIPC_STATE_LOW)
  {
     if (ulp_data.gnss_provider_info.state == GNSS_STATE_FAILED)
     {
        ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MIN;
        ulp_data.quipc_provider_info.onoff_cnt = 0;
     }
     else
     {
        ulp_data.gnss_provider_info.onoff_cnt = 0;
        ulp_data.quipc_provider_info.onoff_cnt = 0;
     }
  }
  else if ((ulp_data.quipc_provider_info.state == QUIPC_STATE_OUT_OF_COVERAGE) ||
           (ulp_data.quipc_provider_info.state == QUIPC_STATE_FAILED))
  {
     ulp_data.gnss_provider_info.onoff_cnt = 0;
     ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MIN;
  }
  else
  {
     LOC_LOGE ("%s, line %d, unknown QUIPC state %d\n",
               __func__, __LINE__, ulp_data.quipc_provider_info.state);
     ret_val = -1;
  }

  EXIT_LOG(%d, ret_val);

  return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_transition_quipc_running

DESCRIPTION
   This function implements ULP transition logic when QUIPC is considered
   running in the transition logic.

   libulp brain will decide to dynamically turn on/off quipc based on
   the newly recevied status from both GNSS and QUIPC.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_transition_quipc_running ()
{
   int      ret_val = 0;
   uint64_t current_time_msec;

   LOC_LOGI ("%s, quipc state = %d, gnss old state = %d, gnss on off cnt = %d\n",
          __func__,
          ulp_data.quipc_provider_info.state,
          ulp_data.gnss_provider_info.last_state,
          ulp_data.gnss_provider_info.onoff_cnt);

   ENTRY_LOG_CALLFLOW();
   switch (ulp_data.quipc_provider_info.state)
   {
   case QUIPC_STATE_INITIALIZING:
      current_time_msec = ulp_util_get_time_ms();
      if (current_time_msec >= (ulp_data.gnss_provider_info.last_stopped_time_ms +
                                ULP_GNSS_RESTART_TIME_THRESHOLD_MSEC))
      {
         ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
      }
      else if (ulp_data.gnss_provider_info.last_state != GNSS_STATE_FAILED)
      {
         ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
      }
      else
      {
         ulp_data.gnss_provider_info.onoff_cnt = 0;
      }
      break;
   case QUIPC_STATE_HIGH:
      ulp_data.gnss_provider_info.onoff_cnt = 0;
      ulp_data.quipc_provider_info.onoff_cnt = 0;
      break;
   case QUIPC_STATE_LOW:
      current_time_msec = ulp_util_get_time_ms();
      if (ulp_data.gnss_provider_info.last_state == GNSS_STATE_FAILED)
      {
         // GNSS is off for at least 60 seconds
         if (current_time_msec >= (ulp_data.gnss_provider_info.last_stopped_time_ms + 60 * 1000))
         {
            ulp_data.gnss_provider_info.onoff_cnt++;
         }
         else
         {
            ulp_data.gnss_provider_info.onoff_cnt = 0;
         }
      }
      else
      {
         ulp_data.gnss_provider_info.onoff_cnt++;
      }
      ulp_data.quipc_provider_info.onoff_cnt = 0;
      break;
   case QUIPC_STATE_FAILED:
      ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
      ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MIN;
      break;
   case QUIPC_STATE_OUT_OF_COVERAGE:
      ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
      ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MIN;
      break;
   default:
      LOC_LOGE ("%s, line %d, unknown QUIPC state %d\n", __func__, __LINE__, ulp_data.quipc_provider_info.state);
      ret_val = 1;
      break;
   }

   if (ulp_data.gnss_provider_info.onoff_cnt > GNSS_TH_MAX)
   {
      ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_transition_gnss_running

DESCRIPTION
   This function implements ULP transition logic when GNSS is considered
   running in the transition logic.

   libulp brain will decide to dynamically turn on/off quipc based on
   the newly recevied status from both GNSS and QUIPC.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_transition_gnss_running ()
{
   int      ret_val = 0;
   uint64_t current_time_msec;

   ENTRY_LOG_CALLFLOW();

   switch (ulp_data.gnss_provider_info.state)
   {
   case GNSS_STATE_INITIALIZING:
      ulp_data.gnss_provider_info.onoff_cnt = 0;

      current_time_msec = ulp_util_get_time_ms();
      if (current_time_msec >= (ulp_data.quipc_provider_info.last_stopped_time_ms +
                                ULP_QUIPC_RESTART_TIME_THRESHOLD_MSEC))
      {
         ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
      }
      else if (ulp_data.quipc_provider_info.last_state == QUIPC_STATE_FAILED ||
               ulp_data.quipc_provider_info.last_state == QUIPC_STATE_OUT_OF_COVERAGE )
      {
         ulp_data.quipc_provider_info.onoff_cnt = 0;
         // In this case, QUIPC will wait for GNSS state change
      }
      else
      {
         ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
      }
      break;
   case GNSS_STATE_HIGH:
      ulp_data.gnss_provider_info.onoff_cnt = 0;
      ulp_data.quipc_provider_info.onoff_cnt = 0;
      break;
   case GNSS_STATE_MEDIUM:
      ulp_data.gnss_provider_info.onoff_cnt = 0;
      if (ulp_data.quipc_provider_info.last_state == QUIPC_STATE_FAILED ||
          ulp_data.quipc_provider_info.last_state == QUIPC_STATE_OUT_OF_COVERAGE )
      {
         current_time_msec = ulp_util_get_time_ms();
         if (current_time_msec >= (ulp_data.quipc_provider_info.last_stopped_time_ms + 60 * 1000))
         {
            ulp_data.quipc_provider_info.onoff_cnt++;
         }
         else
         {
            ulp_data.quipc_provider_info.onoff_cnt = 0;
         }
      }
      else
      {
         ulp_data.quipc_provider_info.onoff_cnt = 0;
      }

      break;
   case GNSS_STATE_LOW:
      ulp_data.gnss_provider_info.onoff_cnt = 0;
      if (ulp_data.quipc_provider_info.last_state == QUIPC_STATE_FAILED ||
          ulp_data.quipc_provider_info.last_state == QUIPC_STATE_OUT_OF_COVERAGE )
      {
         current_time_msec = ulp_util_get_time_ms();
         if (current_time_msec >= (ulp_data.quipc_provider_info.last_stopped_time_ms + 30 * 1000))
         {
            ulp_data.quipc_provider_info.onoff_cnt++;
         }
         else
         {
            ulp_data.quipc_provider_info.onoff_cnt = 0;
         }
      }
      else
      {
         ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
      }
      break;
   case GNSS_STATE_FAILED:
      ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MIN;
      ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
      break;
   default:
      LOC_LOGE ("%s, line %d, unknown GNSS state %d\n", __func__, __LINE__, ulp_data.gnss_provider_info.state);
      ret_val = 1;
      break;
   }

   if (ulp_data.quipc_provider_info.onoff_cnt > QUIPC_TH_MAX)
   {
      ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MAX;
   }

   EXIT_LOG(%d, ret_val);
   return 1;
}


/*===========================================================================
FUNCTION    ulp_brain_transition_gnp

DESCRIPTION
   This function .

   libulp brain will update GNP on/off count based on the newly recevied
   status from both GNSS and QUIPC.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
static int ulp_brain_transition_gnp ()
{
   uint64_t   current_time_msec,
              last_location_received_msec;
   uint64_t   time_diff_msec;

   if (ulp_data.gnss_provider_info.state  == GNSS_STATE_HIGH ||
       ulp_data.gnss_provider_info.state  == GNSS_STATE_MEDIUM ||
       ulp_data.gnss_provider_info.state  == GNSS_STATE_LOW)
   {
      ulp_data.gnp_provider_info.onoff_cnt = GNP_TH_MIN;
   }
   else if (ulp_data.quipc_provider_info.state == QUIPC_STATE_HIGH ||
            ulp_data.quipc_provider_info.state == QUIPC_STATE_LOW)
   {
      ulp_data.gnp_provider_info.onoff_cnt = GNP_TH_MIN;
   }
   else
   {
      // Enable GNP provider if and only if last received position is
      // at least two tbf away and more than ULP_LOW_ACCURACY_MIN_TRACKING_INTERVAL_MSEC
      last_location_received_msec = ulp_data.gnss_provider_info.last_position_received_time_ms;
      // received more recent QUIPC position
      if (ulp_data.quipc_provider_info.last_position_received_time_ms > last_location_received_msec)
      {
         last_location_received_msec = ulp_data.quipc_provider_info.last_position_received_time_ms;
      }

      current_time_msec = ulp_util_get_time_ms();

      time_diff_msec = current_time_msec - last_location_received_msec;

      if ((time_diff_msec > ULP_LOW_ACCURACY_MIN_TRACKING_INTERVAL_MSEC) &&
          (time_diff_msec > 2 * ulp_data.fix_interval))
      {
         ulp_data.gnp_provider_info.onoff_cnt = GNP_TH_MAX;
      }
   }

   return 0;
}

/*===========================================================================
FUNCTION    ulp_brain_transition_all_providers

DESCRIPTION
   This function is called when GNSS engines and QUIPC sends status and new
   position report to ULP.

   libulp brain will decide to dynamically turn on/off GPS/QUIPC/GNP based on
   the newly recevied status from both GNSS and QUIPC.

DEPENDENCIES
   None

RETURN VALUE
   0: GNSS and QUIPC maintains its current state
   1: GNSS and QUIPC on/off state has changed
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_transition_all_providers ()
{
   int ret_val = 0;
   ENTRY_LOG_CALLFLOW();

   LOC_LOGI ("%s, before transition, \n"
             " GNSS provider state = %d,  GNSS selected to run = %d,  GNSS onoff_cnt = %d, GNSS high speed flag = %d \n"
             " QUIPC provider state = %d, QUIPC selected to run = %d, QUIPC onoff_cnt = %d, \n"
             " GNP provider state = %d,   GNP selected to run = %d,   GNP onoff_cnt = %d, \n",
             __func__,
             ulp_data.gnss_provider_info.state,
             ulp_data.gnss_provider_info.selected_to_run,
             ulp_data.gnss_provider_info.onoff_cnt,
             ulp_data.gnss_provider_info.high_speed,
             ulp_data.quipc_provider_info.state,
             ulp_data.quipc_provider_info.selected_to_run,
             ulp_data.quipc_provider_info.onoff_cnt,
             ulp_data.gnp_provider_info.state,
             ulp_data.gnp_provider_info.selected_to_run,
             ulp_data.gnp_provider_info.onoff_cnt);

   // If high speed flag is set, then do not run QUIPC
   if ((ulp_gnss_engine_running () == true) &&
       (ulp_data.gnss_provider_info.high_speed == true))
   {
      ulp_data.gnss_provider_info.onoff_cnt = GNSS_TH_MAX;
      ulp_data.quipc_provider_info.onoff_cnt = QUIPC_TH_MIN;
   }
   // If QUIPC are running
   else if (ulp_quipc_engine_running () == true)
   {
      if (ulp_gnss_engine_running () == true)
      {
         ulp_brain_transition_gnss_quipc_running_concurrently ();
      }
      else
      {
         ulp_brain_transition_quipc_running ();
      }
   }
   // If only GNSS is running
   else if (ulp_gnss_engine_running () == true)
   {
      ulp_brain_transition_gnss_running ();
   }
   else
   {
      LOC_LOGE ("%s, no QUIPC/GNSS transition logic run due to both engines are OFF \n", __func__);
   }

   // Determine the GNP on_off cnt based on latest GNSS and QUIPC states and other info
   ulp_brain_transition_gnp ();

   LOC_LOGI ("%s, after transition, \n"
             " GNSS provider state = %d,  GNSS selected to run = %d,  GNSS onoff_cnt = %d, GNSS high speed flag = %d \n"
             " QUIPC provider state = %d, QUIPC selected to run = %d, QUIPC onoff_cnt = %d, \n"
             " GNP provider state = %d,   GNP selected to run = %d,   GNP onoff_cnt = %d \n",
             __func__,
             ulp_data.gnss_provider_info.state,
             ulp_data.gnss_provider_info.selected_to_run,
             ulp_data.gnss_provider_info.onoff_cnt,
             ulp_data.gnss_provider_info.high_speed,
             ulp_data.quipc_provider_info.state,
             ulp_data.quipc_provider_info.selected_to_run,
             ulp_data.quipc_provider_info.onoff_cnt,
             ulp_data.gnp_provider_info.state,
             ulp_data.gnp_provider_info.selected_to_run,
             ulp_data.gnp_provider_info.onoff_cnt);

   ret_val = 0;
   // Determine whether the transition logic will cause engine to be
   // turned ON/OFF
   if (ulp_data.gnss_provider_info.selected_to_run == true)
   {
      if ((ulp_gnss_engine_running() == false) &&
          (ulp_data.gnss_provider_info.onoff_cnt >= GNSS_TH_MAX))
      {
         ret_val = 1;
      }
      else if ((ulp_gnss_engine_running () == true) &&
               (ulp_data.gnss_provider_info.onoff_cnt <= GNSS_TH_MIN))
      {
         ret_val = 1;
      }
   }

   if (ulp_data.quipc_provider_info.selected_to_run == true)
   {
      if ((ulp_quipc_engine_running() == false) &&
          (ulp_data.quipc_provider_info.onoff_cnt >= QUIPC_TH_MAX))
      {
         ret_val = 1;
      }
      else if ((ulp_quipc_engine_running () == true) &&
               (ulp_data.quipc_provider_info.onoff_cnt <= QUIPC_TH_MIN))
      {
         ret_val = 1;
      }
   }

   if (ulp_data.gnp_provider_info.selected_to_run == true)
   {
      if ((ulp_gnp_engine_running() == false) &&
          (ulp_data.gnp_provider_info.onoff_cnt >= GNP_TH_MAX))
      {
         ret_val = 1;
      }
      else if ((ulp_gnp_engine_running () == true) &&
               (ulp_data.gnp_provider_info.onoff_cnt <= GNP_TH_MIN))
      {
         ret_val = 1;
      }
   }

   if (ret_val == 1)
   {
      ulp_data.run_provider_selection_logic = true;
   }

   // Set run_monitor_thread based on request type
   pthread_mutex_lock (&ulp_data.monitor_mutex);
   if (ulp_data.run_monitor_thread == true)
   {
      pthread_cond_signal (&ulp_data.monitor_cond);
   }
   pthread_mutex_unlock (&ulp_data.monitor_mutex);


   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_retrieve_cache_position

DESCRIPTION
   This function is called to obtain the cache position from all different
   sources with smallest uncertainty. Older cache position are aged at 10m/s
   for uncertainty for the first 60 seconds, and 30m/s onwards.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   returned position must be saved as it is from local variables
===========================================================================*/
int ulp_brain_retrieve_cache_position (UlpLocation* cached_location_ptr)
{
   int          ret_val = -1;
   int          index = 0;
   float        unc_hor;
   UlpLocation* location_ptr[3];
   uint64_t     location_received_time_msec[3];
   uint64_t     current_time_msec;
   uint64_t     time_diff_sec;

   ENTRY_LOG_CALLFLOW();
   cached_location_ptr->gpsLocation.flags = 0;

   location_ptr[0] = &ulp_data.quipc_provider_info.last_position_received;
   location_ptr[1] = &ulp_data.gnss_provider_info.last_position_received;
   location_ptr[2] = &ulp_data.gnp_provider_info.last_position_received;

   location_received_time_msec[0] = ulp_data.quipc_provider_info.last_position_received_time_ms;
   location_received_time_msec[1] = ulp_data.gnss_provider_info.last_position_received_time_ms;
   location_received_time_msec[2] = ulp_data.gnp_provider_info.last_position_received_time_ms;

   current_time_msec = ulp_util_get_time_ms ();

   // Go over each last received position
   for (index = 0; index < 3; index++)
   {
     if ((location_ptr[index]->gpsLocation.flags & GPS_LOCATION_HAS_LAT_LONG) &&
         (location_ptr[index]->gpsLocation.flags & GPS_LOCATION_HAS_ACCURACY))
     {
        unc_hor = location_ptr[index]->gpsLocation.accuracy;
        time_diff_sec = current_time_msec - location_received_time_msec[index];
        time_diff_sec = (time_diff_sec + 500) / 1000; // Get the seconds portion

        unc_hor = location_ptr[index]->gpsLocation.accuracy;
        // If the time_diff is greater than 60 seconds, grow at 30m/s
        if (time_diff_sec > 60)
        {
           unc_hor += (time_diff_sec - 60) * ULP_POS_HOR_UNCERTAINTY_GROW_RATE_AFTER;
           time_diff_sec = 60;
        }
        // For the first 60 seconds or less, grow at 10m/s
        unc_hor += time_diff_sec * ULP_POS_HOR_UNCERTAINTY_GROW_RATE_FIRST_MIN;

        LOC_LOGD ("%s, index = %d, time_diff = %d, orig uncertainty = %f, total unc = %f\n",
                  __func__, index,  (uint32_t) time_diff_sec, location_ptr[index]->gpsLocation.accuracy, unc_hor);

        if ((cached_location_ptr->gpsLocation.flags == 0) ||
            (cached_location_ptr->gpsLocation.accuracy > unc_hor))
        {
           *cached_location_ptr = *location_ptr[index];
           cached_location_ptr->gpsLocation.accuracy = unc_hor;
           ret_val = 0;
        }
     }
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_process_coarse_pos_request

DESCRIPTION
   This function is called to obtain single shot coarse position

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_coarse_pos_request ()
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, gnp state = %d, ulp_gnp_engine_running () = %d\n",
         __func__,
         ulp_data.gnp_provider_info.state,
         ulp_gnp_engine_running());

   ulp_data.gnp_provider_info.coarse_pos_req_pending = true;

   ret_val = ulp_brain_select_providers ();

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_brain_stop_all_providers

DESCRIPTION
   This function is called to stop all providers. This is called when
   libulp receives stop msg.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_stop_all_providers (void)
{
   ENTRY_LOG_CALLFLOW();

   // In case engines are still running, stop them
   ulp_gnss_stop_engine ();
   ulp_gnp_stop_engine ();
   ulp_quipc_stop_engine ();

   EXIT_LOG(%d, 0);
   return 0;
}

/*=============================================================================================
 * Function description:
 *    This function will return monotonic time since some unspecified starting point.
 *
 * Parameters:
 *    none
 *
 * Return value:
 *    number of milli-seconds of monotonic time.
 =============================================================================================*/
uint64_t ulp_util_get_time_ms ()
{
   uint64_t current_time_msec;

#ifndef DEBUG_X86
   struct timespec tp;
   clock_gettime(CLOCK_MONOTONIC, &tp);

   current_time_msec = tp.tv_sec;
   current_time_msec *= 1000;  // convert to milli-seconds

   current_time_msec += ((tp.tv_nsec + 500000) / 1000000);
#else
   struct timeval      present_time;

   // present time: seconds, and microseconds
   gettimeofday(&present_time, NULL);

   // Calculate absolute expire time (to avoid data overflow)
   current_time_msec = present_time.tv_sec;
   current_time_msec *= 1000;  // convert to milli-seconds

   current_time_msec += (present_time.tv_usec + 500) / 1000; // covert the micro-seconds portion to milliseconds
#endif

   return current_time_msec;
}

/*===========================================================================
FUNCTION    ulp_brain_process_system_update

DESCRIPTION
   This function is invoked when a new system event is received

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_brain_process_system_update (const UlpSystemEvent systemEvent)
{
   int  ret_val = -1;
   uint64_t time_elapsed = 0, threshold = (long long) ulp_data.zpp_provider_info.zpp_trigger_threshold;
   uint64_t current_time_msec = 0;

   ENTRY_LOG_CALLFLOW();
   LOC_LOGD ("%s: ulp_started = %d, phone context type = 0x%x"
             "systemEvent = %d threshold = %ld\n",
             __func__,
             ulp_data.ulp_started,
             ulp_data.phoneSetting.context_type,
             systemEvent,
             ulp_data.zpp_provider_info.zpp_trigger_threshold);

   // This function finds out the following pieces of info:
   // (1) Any GPS provider request active or no
   // (2) Any high accuracy fix request active or no
   // (3) Any low accuracy fix request active or no
   // (4) Should ZPP lookup be carried out
   if ((ulp_data.gps_provider_request_active == true) &&
      ((ulp_data.high_accuracy_request_active == true)||
       (ulp_data.low_accuracy_request_active == true)))
   {
      //ZPP request only if GPS & FLP dont have any clients
      LOC_LOGD ("%s: ZPP request ignored since both GLP & FLP have clients. "
                "gps_provider_request_active: %d"
                "high_accuracy_request_active: %d"
                "low_accuracy_request_active: %d\n",
                __func__,
                ulp_data.gps_provider_request_active,
                ulp_data.high_accuracy_request_active,
                ulp_data.low_accuracy_request_active);
      return ret_val;
   }

   ulp_data.system_event = systemEvent;

   //Objective of the logic below is-if we have periodic ZPP lookup enabled
   //then ignore all events other than power disconnected. They are
   //SCREEN_ON, TIMEZONE_CHANGE, POWER_CONNECTED, PHONE_CONTEXT_UPDATE &
   //ZPP_PERIODIC_WAKEUP
   if ((ULP_LOC_POWER_DISCONNECTED != systemEvent)&&
       (ULP_LOC_ZPP_PERIODIC_WAKEUP != systemEvent))
   {
      bool suppress_nonpwr_events = false;
      zpp_provider_info_s_type* zpp_provider_info_p = &ulp_data.zpp_provider_info;
      pthread_mutex_lock(&zpp_provider_info_p->tLock);
      if(true == zpp_provider_info_p->periodic_session_active)
      {
         suppress_nonpwr_events = true;
      }
      pthread_mutex_unlock(&zpp_provider_info_p->tLock);
      LOC_LOGD ("%s: suppress_nonpwr_events = %d\n",__func__,suppress_nonpwr_events);
      if (suppress_nonpwr_events)
      {
         return ret_val;
      }
   }

   current_time_msec = ulp_util_get_time_ms();

   if (0 != ulp_data.system_event_arrival_time)
   {
      time_elapsed = current_time_msec - ulp_data.system_event_arrival_time;

      LOC_LOGD ("%s: system_event_arrival_time:%lld current_time_msec: %lld  "
                   "time elapsed :%lld threshold :%lld\n",
                   __func__,
                   (long long) ulp_data.system_event_arrival_time,
                   (long long) current_time_msec,
                   (long long) time_elapsed,
                   (long long) threshold);
   }

   //ZPP threshold check here for all events except pwr connected/disocnnected
   if ((ulp_data.system_event_arrival_time != 0)&&
       (systemEvent != ULP_LOC_POWER_CONNECTED)&&
       (systemEvent != ULP_LOC_POWER_DISCONNECTED))
   {
      //Not the first event or power events
      if (time_elapsed < threshold)
      {

         LOC_LOGD ("%s:suppress processing since throttling threashold set\n",
                    __func__);

         return ret_val;
      }
   }

   ulp_data.system_event_arrival_time = current_time_msec;

   if ((systemEvent == ULP_LOC_POWER_DISCONNECTED))
   {
      ulp_data.zpp_request_active           = false;
   }
   else
   {
      //for all other events we need to start ZPP
      ulp_data.zpp_request_active           = true;
   }
   ret_val = 0;

   // Initialize some global variabls
   ulp_data.run_provider_selection_logic = true;

   ulp_brain_select_providers ();

   EXIT_LOG(%d, ret_val);
   return ret_val;
}
