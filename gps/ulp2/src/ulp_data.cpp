/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP Data module

GENERAL DESCRIPTION
  This file contains functions to manipulate data variables in libulp module.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Internal include files
#include <ulp_engine.h>
#include "loc_cfg.h"

#include "ulp_quipc.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"
#define IZAT_CONF_FILE "/etc/izat.conf"

ulp_data_s_type ulp_data;
//Initialize it here to 10 secs so that even if OEMs mistakenly
//omit the setting in izat.conf it will never default to 0
int threshold = ULP_ZPP_MIN_TRIGGER_INTERVAL;
static char conf_pip_mode[16];
static char conf_gtp_wifi_mode[16];
static char conf_gtp_cell_mode[16];
static char conf_gtp_cell_proc[16];
static loc_param_s_type izat_conf_param_table[] =
{
    {"PIP",                   &conf_pip_mode,      NULL, 's'},
    {"GTP_WIFI",              &conf_gtp_wifi_mode, NULL, 's'},
    {"GTP_CELL",              &conf_gtp_cell_mode, NULL, 's'},
    {"GTP_CELL_PROC",         &conf_gtp_cell_proc, NULL, 's'},
    {"ZPP_TRIGGER_THRESHOLD", &threshold ,         NULL, 'n'},
};

/*===========================================================================
FUNCTION  ulp_data_init

DESCRIPTION
   This function is called to initialize data variables in libulp module.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_data_init ()
{
   int ret_val = 0;
   int i;

   ENTRY_LOG_CALLFLOW();
   UTIL_READ_CONF(IZAT_CONF_FILE, izat_conf_param_table);
   if(strcmp(conf_pip_mode, "PREMIUM") == 0)
   {
      ulp_data.quipc_enabled = 1;
      LOC_LOGD("%s:%d]: PIP is set in premium mode. quipc_enabled = 1",
                __func__, __LINE__);
   }
   else
   {
      ulp_data.quipc_enabled = 0;
      LOC_LOGD("%s:%d]: PIP is only enabled in PREMIUM mode. quipc_enabled = 0",
                __func__, __LINE__);
   }

   LOC_LOGD("%s:%d]: GTP WiFi mode set in izat.conf: %s",
            __func__, __LINE__, conf_gtp_wifi_mode);

   if(strncmp(conf_gtp_wifi_mode, "DISABLED", sizeof (conf_gtp_wifi_mode)) == 0)
   {
      ulp_data.gtp_wifi_enabled = false;
      LOC_LOGE("%s:%d]: GTP WiFi is disabled.",
                __func__, __LINE__);
   }
   else
   {
      ulp_data.gtp_wifi_enabled = true;
      LOC_LOGE("%s:%d]: GTP WiFi is enabled.",
                __func__, __LINE__);
   }

   LOC_LOGD("%s:%d]: GTP cell set in izat.conf, proc: %s, mode: %s",
            __func__, __LINE__, conf_gtp_cell_proc, conf_gtp_cell_mode);
   if((strncmp(conf_gtp_cell_proc, "AP", sizeof (conf_gtp_cell_proc)) == 0) &&
      (strncmp(conf_gtp_cell_mode, "DISABLED", sizeof (conf_gtp_cell_mode)) != 0))
   {
      ulp_data.gtp_ap_cell_enabled = true;
      LOC_LOGE("%s:%d]: GTP AP cell is enabled",
                __func__, __LINE__);
   }
   else
   {
      ulp_data.gtp_ap_cell_enabled = false;
      LOC_LOGE("%s:%d]: GTP AP cell is disabled",
                __func__, __LINE__);
   }

   ulp_data.run_monitor_thread = false;

   // Initialize criteria array: none of the criteria has received
   for (i = 0; i < MAX_NUM_UNIQUE_CRITERIA; i++)
   {
      ulp_data.locationCriteriaArray[i].isUsed = false;
      ulp_data.locationCriteriaArray[i].refCnt = 0;
   }

   ulp_data.gps_provider_request_active = false;
   ulp_data.high_accuracy_request_active = false;

   // We have not received any context type
   ulp_data.phoneSettingRequested     = false;
   ulp_data.phoneSetting.context_type = 0;

   ulp_data.ulp_started = false;

   ulp_data.run_provider_selection_logic = false;

   memset (&ulp_data.gnss_provider_info, 0, sizeof (gnss_provider_info_s_type));
   ulp_data.gnss_provider_info.enabled              = false;
   ulp_data.gnss_provider_info.state                = GNSS_STATE_IDLE;
   ulp_data.gnss_provider_info.last_state           = GNSS_STATE_IDLE;
   ulp_data.gnss_provider_info.selected_to_run      = false;
   ulp_data.gnss_provider_info.onoff_cnt            = GNSS_TH_MAX;
   ulp_data.gnss_provider_info.high_speed           = false;
   ulp_data.gnss_provider_info.strong_sv_cnt        = 0;
   ulp_data.gnss_provider_info.first_fix_pending    = false;
   ulp_data.gnss_provider_info.tbf_update_pending   = false;
   ulp_data.gnss_provider_info.new_request_active   = false;
   ulp_data.gnss_provider_info.last_started_time_ms = 0;
   ulp_data.gnss_provider_info.last_stopped_time_ms = 0;
   ulp_data.gnss_provider_info.last_position_received.gpsLocation.flags = 0;
   ulp_data.gnss_provider_info.last_position_received_time_ms = 0;

   memset (&ulp_data.quipc_provider_info, 0, sizeof (quipc_provider_info_s_type));
   ulp_data.quipc_provider_info.enabled                      = false;
   ulp_data.quipc_provider_info.state                        = QUIPC_STATE_IDLE;
   ulp_data.quipc_provider_info.last_state                   = QUIPC_STATE_IDLE;
   ulp_data.quipc_provider_info.selected_to_run              = false;
   ulp_data.quipc_provider_info.onoff_cnt                    = QUIPC_TH_MAX;
   ulp_data.quipc_provider_info.new_request_active           = false;
   ulp_data.quipc_provider_info.last_started_time_ms         = 0;
   ulp_data.quipc_provider_info.last_stopped_time_ms         = 0;
   ulp_data.quipc_provider_info.last_position_received.gpsLocation.flags = 0;
   ulp_data.quipc_provider_info.last_position_received_time_ms = 0;
   ulp_data.quipc_provider_info.last_lci_transition_time_ms = 0;

   memset (&ulp_data.gnp_provider_info, 0, sizeof (gnp_provider_info_s_type));
   ulp_data.gnp_provider_info.enabled                = false;
   ulp_data.gnp_provider_info.state                  = GNP_STATE_IDLE;
   ulp_data.gnp_provider_info.last_state             = GNP_STATE_IDLE;
   ulp_data.gnp_provider_info.selected_to_run        = false;
   ulp_data.gnp_provider_info.onoff_cnt              = GNP_TH_MAX;
   ulp_data.gnp_provider_info.first_fix_pending      = false;
   ulp_data.gnp_provider_info.new_request_active     = false;
   ulp_data.gnp_provider_info.last_started_time_ms   = 0;
   ulp_data.gnp_provider_info.last_stopped_time_ms    = 0;
   ulp_data.gnp_provider_info.last_position_received.gpsLocation.flags = 0;
   ulp_data.gnp_provider_info.last_position_received_time_ms = 0;
   ulp_data.gnp_provider_info.coarse_pos_req_pending = false;

   memset (&ulp_data.zpp_provider_info , 0, sizeof (zpp_provider_info_s_type));
   ulp_data.zpp_provider_info.enabled                      = false;
   ulp_data.zpp_provider_info.state                        = ZPP_STATE_IDLE;
   ulp_data.zpp_provider_info.selected_to_run              = false;
   ulp_data.zpp_provider_info.onoff_cnt                    = ZPP_TH_MAX;
   ulp_data.zpp_provider_info.last_position_received.gpsLocation.flags = 0;
   ulp_data.zpp_provider_info.last_position_received_time_ms = 0;
   ulp_data.zpp_provider_info.periodic_session_active      = false;

   memset (&ulp_data.position_selection_info, 0, sizeof (ulp_position_selection_info_s_type));
   ulp_data.position_selection_info.first_fix_pending         = true;
   ulp_data.position_selection_info.last_report_position_type = ULP_POSITION_TYPE_UNKNOWN;

   ulp_data.ulp_debug_info_type = ULP_DEBUG_INFO_NONE;

   ulp_quipc_inf = NULL;
   ulp_data.gnss_interval_cache = ULP_MIN_INTERVAL_INVALID;

   ulp_data.loc_proxy = new LocUlpProxy();
   ulp_data.system_event = ULP_LOC_SCREEN_ON; //Since phone first booted will have screen-on event
   ulp_data.system_event_arrival_time = 0;
   pthread_cond_init(&ulp_data.zpp_provider_info.tCond, NULL);
   pthread_mutex_init(&ulp_data.zpp_provider_info.tLock, NULL);

   if (ULP_ZPP_MIN_TRIGGER_INTERVAL > threshold)
   {
     threshold = ULP_ZPP_MIN_TRIGGER_INTERVAL;
     LOC_LOGD("%s, Flooring zpp_trigger_threshold to 10s",__func__);
   }
   ulp_data.zpp_provider_info.zpp_trigger_threshold = (long )threshold;
   LOC_LOGI ("%s, quipc_enabled = %d, gps capabilities = 0x%x "
               "zpp trigger threshold = %ld\n",
             __func__, ulp_data.quipc_enabled,
             ulp_data.loc_proxy->mCapabilities,
             ulp_data.zpp_provider_info.zpp_trigger_threshold);

   EXIT_LOG(%d, ulp_data.quipc_enabled);
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION  ulp_data_deinit

DESCRIPTION
   This function is called to free up the resources used in libulp data
   module.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_data_deinit ()
{
   ENTRY_LOG_CALLFLOW();

   EXIT_LOG(%d, 0);

   return 0;
}

/*===========================================================================
FUNCTION  ulp_data_add_criteria

DESCRIPTION
   This function is called to add the new criteria to libulp module.
   If the same criteria already exists, the request will add the criteria as is.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_data_add_criteria (const UlpLocationCriteria* locationCriteriaPtr)
{
   int ret_val    = 0;
   int i;
   int free_index = -1;
   bool criteria_added = false;

   ENTRY_LOG_CALLFLOW();
   for (i = 0; i < MAX_NUM_UNIQUE_CRITERIA; i++)
   {
      if (ulp_data.locationCriteriaArray[i].isUsed == true)
      {
         //If the incoming Criteria update is for GPS provider then we want to simply update
         //the TBF attribute if an existing GPS provider criteria is present in the queue.
         //This is becuase with the new design, there will only be one request to ULP from
         //GPS provider, as all the requests from app to GPS provider will be consolidated into one.
         if ((ULP_PROVIDER_SOURCE_GNSS == locationCriteriaPtr->provider_source) &&
             (ULP_PROVIDER_SOURCE_GNSS == ulp_data.locationCriteriaArray[i].locationCriteria.provider_source))
         {
            LOC_LOGD ("%s, incoming Criteria is for GPS prov and doing update with TBF: %d mask: 0x%x\n", __func__,
                      locationCriteriaPtr->min_interval, locationCriteriaPtr->valid_mask);
            if (locationCriteriaPtr->valid_mask & ULP_CRITERIA_HAS_MIN_INTERVAL)
            {
               if (locationCriteriaPtr->min_interval !=
                ulp_data.locationCriteriaArray[i].locationCriteria.min_interval)
               {
                  //We have not received an identical criteria update from AFW ->SO process it
                  ulp_data.locationCriteriaArray[i].locationCriteria.min_interval = locationCriteriaPtr->min_interval;
                  ulp_data.locationCriteriaArray[i].isNew = true;
               }
               criteria_added = true;
               break;
            }
         }

         // Found identical criteria, increase ref cnt
         if (ulp_data_criteria_identical (locationCriteriaPtr,
                                          &ulp_data.locationCriteriaArray[i].locationCriteria ) == true)
         {
            LOC_LOGD ("%s, identical criteria already exists, increase ref cnt\n", __func__);
            ulp_data.locationCriteriaArray[i].isNew = true;
            ulp_data.locationCriteriaArray[i].refCnt ++;
            criteria_added = true;
            break;
         }
      }
      else if (ulp_data.locationCriteriaArray[i].isUsed == false)
      {
         if (free_index == -1)
         {
            free_index = i;
         }
      }
   }

   // Add the criteria regardless
   if (criteria_added == false)
   {
      if (free_index != -1)
      {
        LOC_LOGV("%s, criteria added at index[%d]: for: aciton %d\n  valid mask: %d\n provider source: %d\n accuracy %d\n recurrence type %d\n min interval %d\n power consumption %d\n intermediate pos %d ",
             __func__,
             free_index,
             locationCriteriaPtr->action,
             locationCriteriaPtr->valid_mask,
             locationCriteriaPtr->provider_source,
             locationCriteriaPtr->preferred_horizontal_accuracy,
             locationCriteriaPtr->recurrence_type,
             locationCriteriaPtr->min_interval,
             locationCriteriaPtr->preferred_power_consumption,
             locationCriteriaPtr->intermediate_pos_report_enabled);

         ulp_data.locationCriteriaArray[free_index].isUsed = true;
         ulp_data.locationCriteriaArray[free_index].isNew  = true;
         ulp_data.locationCriteriaArray[free_index].locationCriteria = *locationCriteriaPtr;
         ulp_data.locationCriteriaArray[free_index].refCnt = 1;
         criteria_added = true;
      }
      else
      {
        LOC_LOGE("%s, running out of space to add criteria for: aciton %d\n  valid mask: %d\n provider source: %d\n accuracy %d\n recurrence type %d\n min interval %d\n power consumption %d\n intermediate pos %d ",
             __func__,
             locationCriteriaPtr->action,
             locationCriteriaPtr->valid_mask,
             locationCriteriaPtr->provider_source,
             locationCriteriaPtr->preferred_horizontal_accuracy,
             locationCriteriaPtr->recurrence_type,
             locationCriteriaPtr->min_interval,
             locationCriteriaPtr->preferred_power_consumption,
             locationCriteriaPtr->intermediate_pos_report_enabled);
      }
   }

   if (criteria_added == true)
   {
     ret_val = 0;
   }
   else
   {
     ret_val = -1;
   }

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION  ulp_data_remove_criteria

DESCRIPTION
   This function is called to remove a unique criteria from libulp module.
   If the criteria is not found, the request will be ignored and error
   code (-1) will be returned.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_data_remove_criteria (const UlpLocationCriteria* locationCriteriaPtr)
{
   int  ret_val = -1;
   int  i;
   bool criteria_removed = false;

   ENTRY_LOG_CALLFLOW();
   for (i = 0; i < MAX_NUM_UNIQUE_CRITERIA; i++)
   {
      if (ulp_data.locationCriteriaArray[i].isUsed == true)
      {
         // Found identical criteria
         if (ulp_data_criteria_identical (locationCriteriaPtr,
                                          &ulp_data.locationCriteriaArray[i].locationCriteria ) == true)
         {
            LOC_LOGV("%s, criteria removed at index[%d]: aciton %d\n  valid mask: %d\n provider source: %d\n accuracy %d\n recurrence type %d\n min interval %d\n power consumption %d\n intermediate pos %d ",
                __func__,
                i,
                locationCriteriaPtr->action,
                locationCriteriaPtr->valid_mask,
                locationCriteriaPtr->provider_source,
                locationCriteriaPtr->preferred_horizontal_accuracy,
                locationCriteriaPtr->recurrence_type,
                locationCriteriaPtr->min_interval,
                locationCriteriaPtr->preferred_power_consumption,
                locationCriteriaPtr->intermediate_pos_report_enabled);

            if (ulp_data.locationCriteriaArray[i].refCnt >= 1)
            {
              ulp_data.locationCriteriaArray[i].refCnt --;
            }
            else
            {
              LOC_LOGE("%s, ulp criteria isUsed set to true with invalid ref cnt = %d",
                       __func__, ulp_data.locationCriteriaArray[i].refCnt);
            }

            if (ulp_data.locationCriteriaArray[i].refCnt == 0)
            {
              ulp_data.locationCriteriaArray[i].isUsed = false;
              ulp_data.locationCriteriaArray[i].isNew  = false;
              ulp_data.locationCriteriaArray[i].locationCriteria.valid_mask = 0;
            }

            criteria_removed = true;
            break;
         }
      }
   }

   if (criteria_removed == true)
   {
      ret_val = 0;
   }
   else
   {
      ret_val = 1;
   }

   // No identical criteria found
   if (ret_val != 0)
   {
      LOC_LOGE ("%s, matching criteria not found for removing \n", __func__);
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION  ulp_data_reset_criteria_array

DESCRIPTION
   This function is called to remove all requests received by libulp module.
   This is generally done when libulp module receives stop request.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_data_reset_criteria_array (void)
{
   int ret_val = 0;
   int i;

   ENTRY_LOG_CALLFLOW();
   for (i = 0; i < MAX_NUM_UNIQUE_CRITERIA; i++)
   {
      if (ulp_data.locationCriteriaArray[i].isUsed == true)
      {
         ulp_data.locationCriteriaArray[i].isUsed = false;
         ulp_data.locationCriteriaArray[i].refCnt = 0;
      }
   }
   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION  ulp_data_criteria_identical

DESCRIPTION
   This function checks whether the two criteria are identical.

   It currently only evaluates provider source, horizontal accuracy,
   power consumption, recurrence type and fix interval for recurring
   fix requests.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
bool ulp_data_criteria_identical (const UlpLocationCriteria* criteriaPtr1,
                                     const UlpLocationCriteria* criteriaPtr2)
{
   bool is_identical = false;

   do
   {
      // First, check that the provider source filed matches
      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_PROVIDER_SOURCE) !=
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_PROVIDER_SOURCE) )
      {
         break;
      }

      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_PROVIDER_SOURCE) &&
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_PROVIDER_SOURCE) )
      {
         if (criteriaPtr1->provider_source != criteriaPtr2->provider_source)
         {
            break;
         }
      }

      // check for horizontal accuracy
      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY) !=
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY) )
      {
         break;
      }

      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY) &&
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY) )
      {
         if (criteriaPtr1->preferred_horizontal_accuracy != criteriaPtr2->preferred_horizontal_accuracy)
         {
            break;
         }
      }

      // check for power consumption
      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_PREFERRED_POWER_CONSUMPTION) !=
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_PREFERRED_POWER_CONSUMPTION) )
      {
         break;
      }

      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_PREFERRED_POWER_CONSUMPTION) &&
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_PREFERRED_POWER_CONSUMPTION) )
      {
         if (criteriaPtr1->preferred_power_consumption != criteriaPtr2->preferred_power_consumption )
         {
            break;
         }
      }

      // check for recurrence_type
      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_RECURRENCE_TYPE) !=
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_RECURRENCE_TYPE) )
      {
         break;
      }

      if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_RECURRENCE_TYPE) &&
          (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_RECURRENCE_TYPE) )
      {
         if (criteriaPtr1->recurrence_type != criteriaPtr2->recurrence_type )
         {
            break;
         }

         if ((criteriaPtr1->recurrence_type == ULP_LOC_RECURRENCE_PERIODIC) &&
             (criteriaPtr2->recurrence_type == ULP_LOC_RECURRENCE_PERIODIC))
         {
            if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_MIN_INTERVAL) !=
                (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_MIN_INTERVAL))
            {
               break;
            }

            if ((criteriaPtr1->valid_mask & ULP_CRITERIA_HAS_MIN_INTERVAL) &&
                (criteriaPtr2->valid_mask & ULP_CRITERIA_HAS_MIN_INTERVAL))
            {
               if (criteriaPtr1->min_interval != criteriaPtr2->min_interval)
               {
                  break;
               }
            }
         }
      }

      is_identical = true;

   }while (0);

   return is_identical;
}
