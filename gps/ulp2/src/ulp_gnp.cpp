/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP GNP module

GENERAL DESCRIPTION
  This file contains functions to start/stop GNP providers.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
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
#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <errno.h>
#include <new>

// Internal include files
#include <ulp_engine.h>

#include "loc_cfg.h"
#include "msg_q.h"

#include "ulp_quipc.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

/*===========================================================================
FUNCTION    ulp_gnp_engine_running

DESCRIPTION
   This function returns true if GNSS engine is running.

DEPENDENCIES
   None

RETURN VALUE
   true: engine is running
   false: engine is not running

SIDE EFFECTS
   N/A
===========================================================================*/
bool ulp_gnp_engine_running ()
{
   bool gnp_running = true;

   ENTRY_LOG_CALLFLOW();

   if (ulp_data.gnp_provider_info.state == GNP_STATE_IDLE)
   {
      gnp_running = false;
   }
   else
   {
      gnp_running = true;
   }

   EXIT_LOG(%d, gnp_running);
   return gnp_running;
}

/*===========================================================================
FUNCTION    ulp_gnp_start_engine

DESCRIPTION
   This function is called to start network provider.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_gnp_start_engine ()
{
   int                   ret_val = -1;
   bool               gnp_need_configuration = false;
   UlpRecurrenceCriteria gnp_recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
   uint32_t              gnp_fix_interval = 0;

   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, gnp state = %d, ulp_gnp_engine_running () = %d, "
             "first fix pending = %d, new request active = %d\n",
             __func__,
             ulp_data.gnp_provider_info.state,
             ulp_gnp_engine_running(),
             ulp_data.gnp_provider_info.first_fix_pending,
             ulp_data.gnss_provider_info.new_request_active);

   do
   {
      // Wait for first fix to complete
      if (ulp_data.gnp_provider_info.first_fix_pending == true)
      {
         break;
      }

      // Figure out recurrence type and fix interval
      if (ulp_data.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC)
      {
         gnp_recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
         gnp_fix_interval    = ulp_data.fix_interval;
         if (gnp_fix_interval < ULP_LOW_ACCURACY_MIN_TRACKING_INTERVAL_MSEC)
         {
            gnp_fix_interval = ULP_LOW_ACCURACY_MIN_TRACKING_INTERVAL_MSEC;
         }
      }
      else
      {
         gnp_recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
         gnp_fix_interval    = 0;
      }

      if (ulp_gnp_engine_running () == false)
      {
         gnp_need_configuration = true;
      }
      // GNP is running, but it has different recurrenc type
      else if ((ulp_data.gnp_provider_info.recurrence_type != gnp_recurrence_type) ||
               ((ulp_data.gnp_provider_info.recurrence_type == ULP_LOC_RECURRENCE_PERIODIC) &&
                (ulp_data.gnp_provider_info.fix_interval != gnp_fix_interval)))
      {
         gnp_need_configuration = true;
      }
      // In case a new GNP fix request has arrived
      else if (ulp_data.gnp_provider_info.new_request_active == true)
      {
         gnp_need_configuration = true;
      }

      if (gnp_need_configuration == true)
      {
         // Send start msg
         UlpNetworkRequestPos networkPosRequest;
         networkPosRequest.request_type = ULP_NETWORK_POS_START_PERIODIC_REQUEST;
         networkPosRequest.interval_ms = gnp_fix_interval;

         if(ulp_data.ulp_network_callback != NULL)
         {
             ulp_data.ulp_network_callback(&networkPosRequest);
         }
         else
             LOC_LOGE("Ulp Network call back not initialized");

         ulp_gnp_set_state (GNP_STATE_ACTIVE);
         ulp_data.gnp_provider_info.recurrence_type    = gnp_recurrence_type;
         ulp_data.gnp_provider_info.fix_interval       = gnp_fix_interval;
         ulp_data.gnp_provider_info.first_fix_pending  = true;
         ret_val = 0;
      }
   } while (0);

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_gnp_stop_engine

DESCRIPTION
   This function is called to stop network provider.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_gnp_stop_engine ()
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   LOC_LOGD ("%s, gnp state = %d, ulp_gnp_engine_running () = %d\n",
             __func__,
             ulp_data.gnp_provider_info.state,
             ulp_gnp_engine_running());

   do
   {
      if (ulp_gnp_engine_running () == true)
      {
         // send stop msg
         UlpNetworkRequestPos networkPosRequest;
         networkPosRequest.request_type = ULP_NETWORK_POS_STOP_REQUEST;
         networkPosRequest.interval_ms  = 0;

         if(ulp_data.ulp_network_callback != NULL)
         {
             ulp_data.ulp_network_callback(&networkPosRequest);
         }
         else
             LOC_LOGE("Ulp Network call back not initialized");

         // Set network provider state to INACTIVE
         // For future phases, need to get state info from GNP provider
         ulp_gnp_set_state (GNP_STATE_IDLE);
         ulp_data.gnp_provider_info.first_fix_pending      = false;
         ret_val = 0;
      }
      else
      {
         ret_val = 0;
      }
   } while (0);
   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_gnp_set_state

DESCRIPTION
   This function is called to update the current GNP engine state.
   In the meantime, it will save the old state.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_gnp_set_state (gnp_state_e_type new_state)
{
   LOC_LOGD ("%s, current gnp state = %d, new gnp state = %d\n",
             __func__,
             ulp_data.gnp_provider_info.state,
             new_state);

   if (ulp_data.gnp_provider_info.state != new_state)
   {
      if (ulp_data.gnp_provider_info.state == GNP_STATE_IDLE)
      {
         ulp_data.gnp_provider_info.last_started_time_ms = ulp_util_get_time_ms ();
      }
      else if (new_state == GNP_STATE_IDLE)
      {
         ulp_data.gnp_provider_info.last_stopped_time_ms = ulp_util_get_time_ms ();
      }

      ulp_data.gnp_provider_info.last_state = ulp_data.gnp_provider_info.state;
      ulp_data.gnp_provider_info.state = new_state;
   }

   return 0;
}

