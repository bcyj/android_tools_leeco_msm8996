/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP Message processing module

GENERAL DESCRIPTION
  This file contains functions to process various ULP messages.

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
#include <new>
#include <android_runtime/AndroidRuntime.h>

// Internal include files
#include <ulp_engine.h>
#include <LocUlpProxy.h>
#include "loc_cfg.h"
#include "msg_q.h"

#include "ulp_quipc.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

static int ulp_msg_process_criteria_update         (const UlpLocationCriteria* locationCriteriaPtr);
static int ulp_msg_process_raw_command             (const char* rawCmd, int rawCmdLength);
static int ulp_msg_process_start_req               (void);
static int ulp_msg_process_stop_req                (void);
static int ulp_msg_process_phone_setting_update    (const UlpPhoneContextSettings* phoneSettingPtr);
static int ulp_msg_process_network_position_report (const UlpNetworkPositionReport* networkPositionPtr);
static int ulp_msg_process_gnss_position_report    (enum loc_sess_status status,
                                                    LocPosTechMask     tech_mask,
                                                    const UlpLocation* quipc_location_ptr,
                                                    const GpsLocationExtended* locationExtendedPtr,
                                                    const void* locationExt);
static int ulp_msg_process_zpp_position_report     (enum loc_sess_status status,
                                                    LocPosTechMask     tech_mask,
                                                    const UlpLocation* zpp_location_ptr);
static int ulp_msg_process_geofence_position_report(const UlpLocation* locationPtr);
static int ulp_msg_process_gnss_sv_report          (const GpsSvStatus* svStatus,
                                                    const GpsLocationExtended* locationExtendedPtr,
                                                    const void* svExt);
static int ulp_msg_process_quipc_position_report   (int   error_code,
                                                    const UlpLocation* quipc_location_ptr);
static int ulp_msg_process_monitor_request         (void);
static int ulp_msg_process_coarse_position_request (void);
static int ulp_msg_process_system_update           (const UlpSystemEvent systemEvent);

static int ulp_msg_process_gnss_stop               (void);
static int ulp_msg_process_gnss_set_pos_mode       (uint32_t min_interval,
                                                    GpsPositionRecurrence recurrence,
                                                    LocPositionMode pos_mode);
static void ulp_msg_process_status_report          (GpsStatusValue status);
/*=====================================================================================
 * Function ulp_msg_main thread processing routine
 *
 * Description
 *  This is the processing routine of ulp_msg thread. It waits on messages posted
 *  to libulp module and processes on those messages.
 *
 * Parameters:
 *   context.  //unused
 *
 * Return value:
 *   NULL: on exit
 =============================================================================================*/
void ulp_msg_main(void * context)
{
   int msg_length;
   ulp_msg *msg = NULL;

   ENTRY_LOG_CALLFLOW();

   while (1)
   {
      msq_q_err_type result = msg_q_rcv(ulp_data.loc_proxy->mQ,
                                        (void **) &msg);
      if (eMSG_Q_SUCCESS != result)
      {
         LOC_LOGE("%s:%d] fail receiving msg: \n", __func__, __LINE__);
         return;
      }
      LOC_LOGD("%s received msg of type: 0x%x\n", __func__, msg->msgid);

      switch(msg->msgid)
      {
      // Message is sent by GPS HAL layer to add/remove unique request criteria
      case ULP_MSG_UPDATE_CRITERIA:
      {
         ulp_msg_update_criteria* criteriaMsg = (ulp_msg_update_criteria*) msg;
         ulp_msg_process_criteria_update (&(criteriaMsg->locationCriteria));
         break;
      }

      // Message is sent by GPS HAL layer to request ULP to provide the debug info.
      case ULP_MSG_INJECT_RAW_COMMAND:
      {
        ulp_msg_inject_raw_command* rawCmdMsg = (ulp_msg_inject_raw_command*) msg;
        ulp_msg_process_raw_command(rawCmdMsg->rawCommand, rawCmdMsg->rawCommandLength);
        break;
      }

      // Message is sent by GPS HAL layer to request ULP to start producing position fixes
      case ULP_MSG_START_FIX:
      {
         ulp_msg_process_start_req ();
         break;
      }

      // Message is sent by FLP to request ULP to stop producing position fixes
      case ULP_MSG_STOP_FLP_FIX:
      {
         ulp_msg_process_stop_req ();
         break;
      }

      // Message is sent by GPS HAL to request ULP to stop producing position fixes
      case ULP_MSG_STOP_GNSS_FIX:
      {
         ulp_msg_process_gnss_stop ();
         break;
      }


      // Message is sent by GPS HAL layer to report phone context setting
      // include initial phone context setting and subsequent changes
      case ULP_MSG_INJECT_PHONE_CONTEXT_SETTINGS:
      {
         ulp_msg_inject_phone_context_settings* phoneSettingMsg = (ulp_msg_inject_phone_context_settings*) msg;
         ulp_msg_process_phone_setting_update (&phoneSettingMsg->phoneSetting);
         break;
      }

      // Message is sent by network provider to report the position in UlpNetworkPositionReport format
      case ULP_MSG_INJECT_NETWORK_POSITION:
      {
         ulp_msg_inject_network_position* networkPositionMsg = (ulp_msg_inject_network_position*) msg;
         ulp_msg_process_network_position_report (&networkPositionMsg->networkPosition);
         break;
      }

      // Message is sent by GNSS provider in order to report the position in GpsPosition format
      case ULP_MSG_REPORT_POSITION:
      {
         ulp_msg_report_position* positionMsg = (ulp_msg_report_position*) msg;

         if ( positionMsg->location.position_source == ULP_LOCATION_IS_FROM_GNSS)
         {
            ulp_msg_process_gnss_position_report ( positionMsg->status,
                                                   positionMsg->technology_mask,
                                                   &positionMsg->location,
                                                   &positionMsg->locationExtended,
                                                   positionMsg->locationExt);
         }
         else if(positionMsg->location.position_source == ULP_LOCATION_IS_FROM_ZPP)
         {
            ulp_msg_process_zpp_position_report ( positionMsg->status,
                                                  positionMsg->technology_mask,
                                                  &positionMsg->location);
         }
         else if(positionMsg->location.position_source == ULP_LOCATION_IS_FROM_GEOFENCE)
         {
            LOC_LOGD("%s:%d]: ULP_MSG_REPORT_POSITION ULP_LOCATION_IS_FROM_GEOFENCE",
                     __func__, __LINE__);
            ulp_msg_process_geofence_position_report ( &positionMsg->location);
         }
         else if(positionMsg->location.position_source == ULP_LOCATION_IS_FROM_HW_FLP)
         {
             LOC_LOGD("%s:%d]: ULP_MSG_REPORT_POSITION ULP_LOCATION_IS_FROM_HW_FLP",
                     __func__, __LINE__);
             //TODO
         }
         break;
      }

      // Message is sent by HW FLP
      case ULP_MSG_REPORT_POSITIONS:
      {
          ulp_msg_report_positions* positionMsg = (ulp_msg_report_positions*) msg;
          LOC_LOGD("%s:%d]: ULP_MSG_REPORT_POSITION ULP_LOCATION_IS_FROM_HW_FLP",
                  __func__, __LINE__);
          //TODO
          break;
      }

      // Message is sent by GNSS provider in order to report the SV info
      case ULP_MSG_REPORT_SV:
      {
         ulp_msg_report_sv* svMsg = (ulp_msg_report_sv*) msg;
         ulp_msg_process_gnss_sv_report ( &svMsg->svStatus, &svMsg->locationExtended, svMsg->svExt);
         break;
      }

      // Message is sent by QUIPC in order to report the position in GpsPosition format
      // together with QUIPC status
      case ULP_MSG_REPORT_QUIPC_POSITION:
      {
         ulp_msg_report_quipc_position* positionMsg = (ulp_msg_report_quipc_position*) msg;
         ulp_msg_process_quipc_position_report (positionMsg->quipc_error_code, &positionMsg->location);
         break;
      }

      // Message is sent by QUIPC in order to request coarse position info
      case ULP_MSG_REQUEST_COARSE_POSITION:
      {
         ulp_msg_process_coarse_position_request ();
         break;
      }

      // Message is sent by ULP monitor thread to request ULP main thread
      // to re-evaulate the status of each subsystem
      case ULP_MSG_MONITOR:
      {
         ulp_msg_process_monitor_request ();
         break;
      }

      // Message is sent by GPS Location provider to start a session
      case ULP_MSG_SET_POSITION_MODE:
      {
         ulp_msg_position_mode *setPosModeMsg = (ulp_msg_position_mode*)msg;
         ulp_msg_process_gnss_set_pos_mode(setPosModeMsg->pMode.min_interval,
                                           setPosModeMsg->pMode.recurrence,
                                           setPosModeMsg->pMode.mode);
         break;
      }

      //Status report is sent by GPS engine to inform the following events:
      //Session begin, session end, engine on, engine off
      case ULP_MSG_REPORT_STATUS:
      {
          ulp_msg_report_status *statusMsg = (ulp_msg_report_status*)msg;
          ulp_msg_process_status_report(statusMsg->engStatus);
          break;
      }

      // Message is sent to ULP module when system events occur
      case ULP_MSG_SYSTEM_UPDATE:
      {
         ulp_msg_system_update* systemUpdateMsg = (ulp_msg_system_update* ) msg;
         ulp_msg_process_system_update (systemUpdateMsg->systemEvent);
         break;
      }

      case ULP_MSG_REPORT_BATCHING_SESSION:
      {
          //TODO
          ulp_msg_report_batching_session * batchingSessionMsg =
              (ulp_msg_report_batching_session*)msg;
          LOC_LOGV("%s:%d]",__func__, __LINE__);
          LOC_LOGV("Max Power Allocation(mw): %f\n sources to use: %d\n flags: %d\n period(ns) %lld active: %d\n",
              batchingSessionMsg->options.max_power_allocation_mW,
              batchingSessionMsg->options.sources_to_use,
              batchingSessionMsg->options.flags,
              batchingSessionMsg->options.period_ns,
              batchingSessionMsg->active);
          break;
      }

      default:
         LOC_LOGE ("%s, received unknown message of type %d, discard \n", __func__, msg->msgid);
         break;
      }

      delete msg;
   };

   EXIT_LOG(%s, VOID_RET);
   return;
}

/*=============================================================================================
 * Function: ulp_msg_init
 *
 * Description
 *   This function initializes ULP message queue module. A dedicated thread will be created
 *   to listen and process all the messages posted in ULP modules.
 *
 * Parameters:
 *   arg: client passed argument
 *
 * Dependency:
 *    All input to ULP module must be put in the ULP message queue.
 *
 * Return value:
 *   0: on success
 *   non-zero: on failure
 =============================================================================================*/
int ulp_msg_init ()
{
   int ret_val = -1;
   pthread_t thread_id = NULL;

   ENTRY_LOG_CALLFLOW();
   do
   {
      thread_id = ulp_data.ulp_create_thread_cb("ulp_msg",
                                                ulp_msg_main,
                                                (void *)&ulp_data);
      if (NULL == thread_id)
      {
         LOC_LOGE ("%s: failed to create ulp msg therad, errno = %d\n", __func__, errno);
         break;
      }

      ret_val = 0;
   } while (0);

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_raw_command

DESCRIPTION
   This function is called when libulp module receives an inject raw command message.
   The message is to provide / stop the debug info along with location information.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_raw_command (const char* rawCmd, int rawCmdLength)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();
   LOC_LOGD("%s: , Cmd = %s, Len = %d\n", __func__, rawCmd, rawCmdLength);

#ifndef DEBUG_X86
   if (rawCmd != NULL)
   {
     ret_val = ulp_debug_process_raw_command (rawCmd, rawCmdLength);
   }
#endif

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_criteria_update

DESCRIPTION
   This function is called when libulp module receives a criteria update message.
   The message is either to add a unique criteria or remove an exisitng criteria.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_criteria_update (const UlpLocationCriteria* locationCriteriaPtr)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();
   if (locationCriteriaPtr != NULL)
   {
     LOC_LOGI("%s: valid 0x%x action:%d, minTime:%d, minDistance:%f, singleShot:%d, horizontalAccuracy:%d, powerRequirement:%d \n",
            __func__,
            locationCriteriaPtr->valid_mask,
            locationCriteriaPtr->action,
            locationCriteriaPtr->min_interval,
            locationCriteriaPtr->min_distance,
            locationCriteriaPtr->recurrence_type,
            locationCriteriaPtr->preferred_horizontal_accuracy,
            locationCriteriaPtr->preferred_power_consumption );

      if (locationCriteriaPtr->valid_mask & ULP_CRITERIA_HAS_ACTION)
      {
         if (locationCriteriaPtr->action == ULP_ADD_CRITERIA)
         {
            ret_val = ulp_data_add_criteria (locationCriteriaPtr);
         }
         else if (locationCriteriaPtr->action == ULP_REMOVE_CRITERIA)
         {
            ret_val = ulp_data_remove_criteria (locationCriteriaPtr);
         }
      }

      // Criteria has been successfully added or removed, invoke the brain logic
      if (ret_val == 0)
      {
         (void) ulp_brain_process_criteria_update ();
         if (ulp_data.run_provider_selection_logic == true)
         {
            ret_val = ulp_brain_select_providers ();
         }
      }
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_start_req

DESCRIPTION
   This function is called when libulp module receives a message to request
   universal location provider to start producing position reports.

   The module will choose providers based on available criteria and context
   settings.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_start_req (void)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();

   LOC_LOGI ("%s, at ulp state = %d\n", __func__, ulp_data.ulp_started);

   do
   {
      ulp_data.ulp_started = true;

      // We request phone context setting upon receiving first start request
      if (ulp_data.phoneSettingRequested == false)
      {
         UlpPhoneContextRequest phoneContextReq;

         phoneContextReq.context_type = (ULP_PHONE_CONTEXT_GPS_SETTING |
                                         ULP_PHONE_CONTEXT_NETWORK_POSITION_SETTING |
                                         ULP_PHONE_CONTEXT_WIFI_SETTING |
                                         ULP_PHONE_CONTEXT_BATTERY_CHARGING_STATE |
                                         ULP_PHONE_CONTEXT_AGPS_SETTING |
                                         ULP_PHONE_CONTEXT_ENH_LOCATION_SERVICES_SETTING |
                                         ULP_PHONE_CONTEXT_PIP_USER_SETTING);

         phoneContextReq.request_type = ULP_PHONE_CONTEXT_REQUEST_TYPE_ON_CHANGE;
         phoneContextReq.interval_ms = 0;

         if(ulp_data.ulp_phone_context_req_cb != NULL)
         {
             ulp_data.ulp_phone_context_req_cb(&phoneContextReq);
         }
         else
             LOC_LOGE("Ulp Phone Context call back not initialized");

         LOC_LOGD("%s, ulp_msg_request_phone_context- message send with context_type: 0x%x and req_type: 0x%x \n",
                  __func__,phoneContextReq.context_type, phoneContextReq.request_type);
         ulp_data.phoneSettingRequested = true;

         ret_val = 0;
         break;
      }

      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_stop_req

DESCRIPTION
   This function is called when libulp module receives a message to request
   universal location provider to stop producing position reports.

   The module will stop all providers that are currently producing fixes
   initiated by libulp modules.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_stop_req (void)
{
   int ret_val = 0;
   //With the new design ulp_stop is essentailly a no-op since the
   //criteria update processing will shut off all providers when
   //last criteria is removed from the queue
   ENTRY_LOG_CALLFLOW();
   LOC_LOGI ("%s, at ulp state = %d\n", __func__, ulp_data.ulp_started);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_phone_setting_update

DESCRIPTION
   This function is called when libulp module receives the message regarding
   current phone setting.

   libulp will need choose the appropriate provider based on new phone
   settings.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_phone_setting_update (const UlpPhoneContextSettings* phoneSettingPtr)
{
   int      ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   do
   {
      if (phoneSettingPtr == NULL)
      {
         break;
      }

      LOC_LOGI("%s, context type: 0x%x\n  gps enabled: %d\n network position available %d\n "
               "wifi setting enabled %d\n battery charging %d, agps enabled %d, enh_location_services_enabled %d"
               "is_pip_user_setting_enabled %d\n",
               __func__,
               phoneSettingPtr->context_type,
               phoneSettingPtr->is_gps_enabled,
               phoneSettingPtr->is_network_position_available,
               phoneSettingPtr->is_wifi_setting_enabled,
               phoneSettingPtr->is_battery_charging,
               phoneSettingPtr->is_agps_enabled,
               phoneSettingPtr->is_enh_location_services_enabled,
               phoneSettingPtr->is_pip_user_setting_enabled);

      // Controls GNSS
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_GPS_SETTING)
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_GPS_SETTING;
         ulp_data.phoneSetting.is_gps_enabled = phoneSettingPtr->is_gps_enabled;
      }

      // Controls Network positioning
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_NETWORK_POSITION_SETTING )
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_NETWORK_POSITION_SETTING ;
         ulp_data.phoneSetting.is_network_position_available = phoneSettingPtr->is_network_position_available;
      }

      // Controls QUIPC -WiFi Setting
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_WIFI_SETTING)
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_WIFI_SETTING;
         ulp_data.phoneSetting.is_wifi_setting_enabled = phoneSettingPtr->is_wifi_setting_enabled;
      }

      // Controls QUIPC -Enhanced Location Services Setting configured by user through XT App
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_ENH_LOCATION_SERVICES_SETTING)
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_ENH_LOCATION_SERVICES_SETTING;
         ulp_data.phoneSetting.is_enh_location_services_enabled = phoneSettingPtr->is_enh_location_services_enabled;
      }

      // AGPS setting
      // Setting change will only impact next single shot fix request
      // If there is on-going single shot session, it will not be impacted
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_AGPS_SETTING)
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_AGPS_SETTING;
         ulp_data.phoneSetting.is_agps_enabled = phoneSettingPtr->is_agps_enabled;
      }

      // Battery charging state
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_BATTERY_CHARGING_STATE)
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_BATTERY_CHARGING_STATE;
         ulp_data.phoneSetting.is_battery_charging = phoneSettingPtr->is_battery_charging;
      }

      // Controls QUIPC -PIP user setting through OS settings menu on UI
      if (phoneSettingPtr->context_type & ULP_PHONE_CONTEXT_PIP_USER_SETTING)
      {
         ulp_data.phoneSetting.context_type |= ULP_PHONE_CONTEXT_PIP_USER_SETTING;
         ulp_data.phoneSetting.is_pip_user_setting_enabled = phoneSettingPtr->is_pip_user_setting_enabled;
      }

      ret_val = ulp_brain_process_phone_setting_update ();

      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   } while (0);
   ret_val = 0;

   EXIT_LOG(%d, ret_val);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_network_position_report

DESCRIPTION
   This function is called when libulp module receives a message regarding
   network position report.

   libulp will translate the network position report to generic GpsPosition
   and sent it to GPS HAL layer. GPS HAL layer will then send the position
   to Android location framework.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_network_position_report (const UlpNetworkPositionReport* networkPositionPtr)
{
   int ret_val = -1;
   UlpLocation ulpLocation;

   ENTRY_LOG_CALLFLOW();
   do
   {
      if (networkPositionPtr == NULL)
      {
         break;
      }

      if (ulp_data.ulp_started == false)
      {
         // Disgard position report if ulp is not started
         break;
      }

      LOC_LOGD ("%s, network position valid flag = 0x%x, lat = %f, lon = %f, hepe = %f \n",
                __func__,
                networkPositionPtr->valid_flag,
                networkPositionPtr->position.latitude,
                networkPositionPtr->position.longitude,
                networkPositionPtr->position.HEPE);

      if ((networkPositionPtr->valid_flag & ULP_NETWORK_POSITION_REPORT_HAS_POSITION) == 0)
      {
         // No position info, break
         break;
      }

      memset (&ulpLocation, 0, sizeof (UlpLocation));
      ulpLocation.gpsLocation.size = sizeof (GpsLocation);
      ulpLocation.gpsLocation.flags |= GPS_LOCATION_HAS_LAT_LONG |
                           GPS_LOCATION_HAS_ACCURACY |
                           LOCATION_HAS_SOURCE_INFO;
      ulpLocation.gpsLocation.latitude = networkPositionPtr->position.latitude;
      ulpLocation.gpsLocation.longitude = networkPositionPtr->position.longitude;
      ulpLocation.gpsLocation.accuracy  = networkPositionPtr->position.HEPE;
      ulpLocation.position_source = ULP_LOCATION_IS_FROM_HYBRID;
      ulpLocation.rawDataSize     = 0;
      ulpLocation.rawData         = NULL;

      if (networkPositionPtr->valid_flag & ULP_NETWORK_POSITION_REPORT_HAS_FIX_TIME)
      {
         ulpLocation.gpsLocation.timestamp = networkPositionPtr->fix_time;
      }

      ret_val = ulp_brain_process_gnp_position_report (&ulpLocation);

      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_gnss_sv_report

DESCRIPTION
   This function is called when libulp module receives a message from
   GNSS provider regarding sv report.

   libulp will do necessary processing, including forwarding the position
   report to GPS HAL layer and decide whether to dynamically turn on/off
   GPS/QUIPC HAL based on the SV report.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
static int ulp_msg_process_gnss_sv_report (const GpsSvStatus* svStatusPtr,
                                           const GpsLocationExtended* locationExtendedPtr,
                                           const void* svExt)
{
   int ret_val = -1;
   GpsSvStatus svStatus;
   GpsLocationExtended locationExtended;
   void* svExtCopy;

   do
   {
      if (svStatusPtr == NULL)
      {
         break;
      }

      if (ulp_data.ulp_started == false)
      {
         // Disgard position report if ulp is not yet started
         break;
      }

      svStatus = *svStatusPtr;
      locationExtended = *locationExtendedPtr;
      svExtCopy = (void*) svExt;

      ulp_data.loc_proxy->getAdapter()->
          reportSv(svStatus, locationExtended, (void*) svExtCopy);

      ret_val = ulp_brain_process_gnss_sv_report (svStatusPtr);

      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   } while (0);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_gnss_position_report

DESCRIPTION
   This function is called when libulp module receives a message from
   GNSS provider regarding position report.

   libulp will do necessary processing, including deciding among all
   received position report, whether to forward the position report
   to GPS HAL layer. GPS HAL layer will then send the position to
   Android location framework.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_gnss_position_report (const enum loc_sess_status status,
                                          LocPosTechMask     tech_mask,
                                          const UlpLocation* locationPtr,
                                          const GpsLocationExtended* locationExtendedPtr,
                                          const void* locationExt)
{
   int ret_val = -1;

   do
   {
      if (locationPtr == NULL)
      {
         break;
      }

      if (ulp_data.ulp_started == false)
      {
         // Disgard position report if ulp is not yet started
         break;
      }

      ret_val = ulp_brain_process_gnss_position_report (status, tech_mask, locationPtr, locationExtendedPtr, locationExt);
      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   } while (0);

   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_zpp_position_report

DESCRIPTION
   This function is called when libulp module receives a message from
   GNSS provider regarding ZPP position report.

   libulp will do necessary processing, including deciding among all
   received position report, whether to forward the position report
   to GPS HAL layer. GPS HAL layer will then send the position to
   Android location framework.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_zpp_position_report (const enum loc_sess_status status,
                                         LocPosTechMask     tech_mask,
                                         const UlpLocation* locationPtr)
{
   int ret_val = -1;

   do
   {
      if (locationPtr == NULL)
      {
         break;
      }

      if (ulp_data.ulp_started == false)
      {
         // Disgard position report if ulp is not yet started
         break;
      }

      ret_val = ulp_brain_process_zpp_position_report (status,tech_mask,locationPtr);

      zpp_provider_info_s_type* zpp_provider_info_p = &ulp_data.zpp_provider_info;
      //reset flag for the next session
      ulp_data.zpp_request_active = false;
      ret_val = ulp_brain_select_providers ();
   } while (0);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_geofence_position_report

DESCRIPTION
   This function is called when libulp module receives a message from
   Geofence Adapter after a geofence breach event

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_geofence_position_report (const UlpLocation* locationPtr)
{
   int ret_val = -1;
   LOC_LOGD("%s:%d]: Enter", __func__, __LINE__);
   do
   {
      if (locationPtr == NULL)
      {
         break;
      }

      ret_val = ulp_brain_process_geofence_position_report (locationPtr);
   } while (0);
   LOC_LOGD("%s:%d]: Exit", __func__, __LINE__);
   return ret_val;
}


/*===========================================================================
FUNCTION    ulp_msg_process_quipc_position_report

DESCRIPTION
   This function is called when libulp module receives a message from
   QUIPC regarding QUIPC status and position report.

   libulp will do necessary processing, including deciding among all
   received position report, whether to forward the position report
   to GPS HAL layer. GPS HAL layer will then send the position to
   Android location framework.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_quipc_position_report (int                error_code,
                                           const UlpLocation* quipc_location_ptr)
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();

   do
   {
      if (quipc_location_ptr == NULL)
      {
         break;
      }

      if (ulp_data.ulp_started == false)
      {
         // Disgard position report if ulp is not yet started
         break;
      }

      ret_val = ulp_brain_process_quipc_position_report (error_code, quipc_location_ptr);
      if (ulp_data.run_provider_selection_logic == true)
      {
         ret_val = ulp_brain_select_providers ();
      }
   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_send_generic_position_report

DESCRIPTION
   This function is called when libulp module need to send a position report
   to GPS HAL layer.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_send_generic_position_report (enum loc_sess_status status,
                                          LocPosTechMask     tech_mask,
                                          const UlpLocation* locationPtr,
                                          const GpsLocationExtended* locationExtendedPtr,
                                          const void* locationExt)
{
   int ret_val = -1;
   UlpLocation ulpLocation;
   GpsLocationExtended gpsLocationExtended;

   ENTRY_LOG_CALLFLOW();
   do
   {
      if (locationPtr == NULL)
      {
         break;
      }

      ulpLocation = *locationPtr;
      ulpLocation.gpsLocation.size = sizeof (GpsLocation);
      if ( ULP_LOCATION_IS_FROM_HYBRID & ulpLocation.position_source ) {
          ulp_data.ulp_loc_cb(&ulpLocation);
      } else {
          //Send position report back to HAL
          gpsLocationExtended = *locationExtendedPtr;
          gpsLocationExtended.size = sizeof (GpsLocationExtended);

          ulp_data.loc_proxy->getAdapter()->
              reportPosition(ulpLocation, gpsLocationExtended,
                             (void*)locationExt, status, tech_mask);
      }

     ret_val = 0;
   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_monitor_request

DESCRIPTION
   This function is called when libulp monitor thread is sending a message
   to ULP main thread for it to re-evaluate its subsystems. This generally
   happens when the ULP main thread is not receiving status update from
   its sub-systems for some duration.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_monitor_request ()
{
    int ret_val = -1;

    ENTRY_LOG_CALLFLOW();

    ret_val = ulp_brain_process_monitor_request ();

    if (ulp_data.run_provider_selection_logic == true)
    {
      ret_val = ulp_brain_select_providers ();
    }

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_send_monitor_request

DESCRIPTION
   This function is called when ulp monitor thread need to send a message
   to ULP main thread to request ULP to re-evaluate its subsystems.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_send_monitor_request ()
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();
   do
   {
      //Pass the start messgage to ULP if present & activated
      ulp_msg *ulpMonitorMsg(new ulp_msg(&ulp_data, ULP_MSG_MONITOR));

      if (ulpMonitorMsg == NULL)
      {
         LOC_LOGE ("%s, failed to create message: ULP_MSG_MONITOR \n", __func__);
         break;
      }

      msg_q_snd(ulp_data.loc_proxy->mQ,
                ulpMonitorMsg,
                ulp_msg_free);

     ret_val = 0;

   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_forward_quipc_position_report

DESCRIPTION
   This function is called when libulp module need to forward a position report
   from ULP named pipe to message queue. This is done so that all ULP
   state transition is done in one thread.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_forward_quipc_position_report (int                    error_code,
                                           const UlpLocation*     quipc_location_ptr,
                                           int                    debug_info_length,
                                           char*                  debug_info_ptr)
{
   int ret_val = -1;
   UlpLocation quipc_location;
   char* debug_info = NULL;

   ENTRY_LOG_CALLFLOW();
   do
   {
      if (quipc_location_ptr == NULL)
      {
         break;
      }

      quipc_location = *quipc_location_ptr;

      LOC_LOGI ("%s, Debug Info = %s, Length = %d\n", __func__, debug_info_ptr, debug_info_length);

      // Make a copy of the debug info received. Needs to be freed by ulp_brain.
      if (debug_info_length != 0) {
        debug_info = new char[debug_info_length];
        if (debug_info != NULL)
        {
          strlcpy((char*)debug_info, debug_info_ptr, debug_info_length);
          quipc_location.rawData = (void*) debug_info;
          quipc_location.rawDataSize = debug_info_length;
        }
      }

      ulp_msg_report_quipc_position *ulpPositionMsg(
            new ulp_msg_report_quipc_position(&ulp_data,
                                              quipc_location,
                                              error_code));
      if (ulpPositionMsg == NULL)
      {
         LOC_LOGE ("%s, failed to create message: ulp_msg_report_quipc_position \n", __func__);

         // Free the allocated memory
         delete [] debug_info;
         debug_info = NULL;
         quipc_location.rawData = NULL;
         quipc_location.rawDataSize = 0;
         break;
      }

     msg_q_snd(ulp_data.loc_proxy->mQ,
               ulpPositionMsg,
               ulp_msg_free);

     ret_val = 0;

   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_forward_quipc_coarse_position_request

DESCRIPTION
   This function is called when libulp module need to forward coarse
   position request from ULP named pipe to message queue. This is done
   so that all ULP state transition is done in one thread.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_forward_quipc_coarse_position_request ()
{
   int ret_val = -1;

   ENTRY_LOG_CALLFLOW();
   do
   {
      // Forward the coarse position request in ULP named pipe to ULP message queue
      // so that all state transition is done in one thread
      ulp_msg *msg(new ulp_msg(&ulp_data, ULP_MSG_REQUEST_COARSE_POSITION));
      if (msg == NULL)
      {
         LOC_LOGE ("%s, failed to create message: ULP_MSG_REQUEST_COARSE_POSITION \n", __func__);
         break;
      }

      msg_q_snd(ulp_data.loc_proxy->mQ, msg, ulp_msg_free);

     ret_val = 0;

   } while (0);

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_coarse_position_request

DESCRIPTION
   This function is called by QUIPC to request a coarse position to seed
   indoor positioning engine.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A
===========================================================================*/
int ulp_msg_process_coarse_position_request ()
{
   int         ret_val = -1;
   UlpLocation cached_location;
   bool     send_cached_position = false;

   ENTRY_LOG_CALLFLOW();

   ret_val = ulp_brain_retrieve_cache_position (&cached_location);
   if (ret_val == 0)
   {
      if ((cached_location.gpsLocation.flags & GPS_LOCATION_HAS_ACCURACY) &&
          (cached_location.gpsLocation.accuracy <= ULP_QUIPC_COARSE_POSITION_UNCERTAINTY_THRESHOLD))
      {
         send_cached_position = true;
      }
   }

   if (send_cached_position == true)
   {
       LOC_LOGD ("%s, send out coarse position: lat = %f, lon = %f total unc = %f\n",
                 __func__,
                 cached_location.gpsLocation.latitude,
                 cached_location.gpsLocation.longitude,
                 cached_location.gpsLocation.accuracy);
      ulp_quipc_inf->send_coarse_position (&cached_location);
   }
   else
   {
      ulp_brain_process_coarse_pos_request ();
   }
   ret_val = 0;

   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_free

DESCRIPTION
   This function is called to free the msg allocated by ULP module.

DEPENDENCIES
   None

RETURN VALUE
   NULL

SIDE EFFECTS
   N/A

===========================================================================*/
void ulp_msg_free (void* msg)
{
   delete (ulp_msg*)msg;
}

/*===========================================================================
FUNCTION    ulp_msg_process_gnss_set_pos_mode

DESCRIPTION
   This function is called when libulp module receives a QoS update message from
   the GPS Provider. The message is either to add a unique criteria for a GPS Provider
   client.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_gnss_set_pos_mode (uint32_t min_interval,
                                       GpsPositionRecurrence recurrence,
                                       LocPositionMode pos_mode)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();
   UlpLocationCriteria native_criteria;
   native_criteria.valid_mask = (ULP_CRITERIA_HAS_ACTION | ULP_CRITERIA_HAS_PROVIDER_SOURCE | ULP_CRITERIA_HAS_RECURRENCE_TYPE |
                             ULP_CRITERIA_HAS_MIN_INTERVAL);
   native_criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
   native_criteria.min_distance = 0; //This is not used by ULP engine so leaving it 0 for now

   if (LOC_POSITION_MODE_MS_ASSISTED == pos_mode)
      native_criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
   else
      native_criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;

   //For a GPS client horizontal_accuracy & power_consumption are irrelevant
   native_criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_DONT_CARE;
   native_criteria.preferred_power_consumption = ULP_POWER_REQ_DONT_CARE;
   native_criteria.action = ULP_ADD_CRITERIA;
   native_criteria.min_interval = min_interval;
   ulp_data.gnss_interval_cache = min_interval; //cache a copy
   ulp_data.gnss_recurrence_type_cache = native_criteria.recurrence_type;

   ret_val = ulp_msg_process_criteria_update(&native_criteria);
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_gnss_stop

DESCRIPTION
   This function is called when libulp module receives a QoS update message from
   the GPS Provider. The message is either to add a unique criteria for a GPS Provider
   client.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_msg_process_gnss_stop (void)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();
   UlpLocationCriteria native_criteria;
   native_criteria.valid_mask = (ULP_CRITERIA_HAS_ACTION | ULP_CRITERIA_HAS_PROVIDER_SOURCE | ULP_CRITERIA_HAS_RECURRENCE_TYPE |
                             ULP_CRITERIA_HAS_MIN_INTERVAL);
   native_criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
   native_criteria.min_distance = 0; //This is not used by ULP engine so leaving it 0 for now
   //For a GPS client horizontal_accuracy & power_consumption are irrelevant
   native_criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_DONT_CARE;
   native_criteria.preferred_power_consumption = ULP_POWER_REQ_DONT_CARE;
   native_criteria.action = ULP_REMOVE_CRITERIA;
   native_criteria.min_interval = ulp_data.gnss_interval_cache;
   native_criteria.recurrence_type = ulp_data.gnss_recurrence_type_cache;

   ret_val = ulp_msg_process_criteria_update(&native_criteria);
   EXIT_LOG(%d, ret_val);
   return ret_val;
}

/*===========================================================================
FUNCTION    ulp_msg_process_status_report

DESCRIPTION
   This function is called to preserve the ordering of the msgs sent to
   Android. Passing this msg through ULP ensures that it follows the order
   like msgs of position report that are routed through ULP as well.
   Status report is sent by the GPS engine to inform these events:
   session begin, session end, engine on, engine off

DEPENDENCIES
   None
RETURN VALUE
   NULL
SIDE EFFECTS
   N/A
===========================================================================*/
void ulp_msg_process_status_report(const GpsStatusValue status)
{
   ulp_data.loc_proxy->getAdapter()->reportStatus(status);
   LOC_LOGD("%s:%d]: Status: %d\n", __func__, __LINE__, (int)status);
}

/*===========================================================================
FUNCTION    ulp_msg_process_system_update

DESCRIPTION
   This function is called when libulp module receives a system event update message.
   The message is to inform libulp of system event udaptes like Apps processor wakeup.

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
static int ulp_msg_process_system_update (const UlpSystemEvent systemEvent)
{
   int ret_val = -1;
   ENTRY_LOG_CALLFLOW();
   LocAdapterBase*       adapter = ulp_data.loc_proxy->getAdapter();

   if (NULL == adapter) {
      LOC_LOGW("Loc HAL handshake did not happen yet...");
      return 0;
   }

   LOC_LOGI("%s: systemEvent:%d \n",__func__,systemEvent);
   ret_val = ulp_brain_process_system_update (systemEvent);
   // System event has been successfully received invoke the brain logic
   if (ret_val == 0)
   {
      ulp_msg_process_start_req();
   }

   EXIT_LOG(%d, ret_val);
   return ret_val;
}
