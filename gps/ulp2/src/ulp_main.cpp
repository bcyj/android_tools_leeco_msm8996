/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       ULP Main Source File

GENERAL DESCRIPTION
  This file contains the entry point for the ULP module.

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
#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <errno.h>
#include <LBSProxy.h>
// Internal include files
#include <ulp_engine.h>
#include "ulp_quipc.h"

#include "loc_cfg.h"
#include "msg_q.h"

#include "ulp_data.h"
#include "ulp_internal.h"

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_libulp"

static bool          ulp_initialized = false;
int ulp_init(UlpEngineCallbacks* pEngineCallbacks,
              UlpNetworkLocationCallbacks* pNetworkLocationCallbacks,
              UlpPhoneContextCallbacks* pPhoneContextCallbacks);
//ULP/Hybrid provider Function definitions
static int ulp_send_network_position(UlpNetworkPositionReport *position_report);
static int ulp_phone_context_settings_update(UlpPhoneContextSettings *settings);
static int ulp_engine_update_criteria(UlpLocationCriteria criteria);
static const void* loc_get_extension(const char* name);
static int  ulp_engine_start();
static int  ulp_engine_stop();
static bool ulp_inject_raw_command(char* command, int length);
static int ulp_phone_system_update(UlpSystemEvent event);

static const UlpEngineInterface sLocEngUlpEngInterface =
{
   sizeof(UlpEngineInterface),
   ulp_engine_update_criteria,
   ulp_engine_start,
   ulp_engine_stop,
   ulp_phone_system_update
};

static const InjectRawCmdInterface sLocEngInjectRawCmdInterface =
{
   sizeof(InjectRawCmdInterface),
   ulp_inject_raw_command
};

//ULP/Hybrid provider interfaces
static const UlpNetworkInterface sUlpNetworkInterface =
{
   sizeof(UlpNetworkInterface),
   ulp_send_network_position
};
static const UlpPhoneContextInterface sLocEngUlpPhoneContextInterface =
{
    sizeof(UlpPhoneContextInterface),
    ulp_phone_context_settings_update
};

// Defines the ulpInterface in ulp.h
static const ulpInterface ulpInf =
{
   sizeof(ulpInterface),
   ulp_init
};

using namespace lbs_core;

/*====================================================================================
 * FUNCTION    ulp_get_interface
 *
 * Description
 *   This function is called to retrieve the ulp interface
 *
 * Parameters:
 *   NULL
 *
 * Return value:
 *   pointer to ulpInterface: on success
 *   NULL: on failure
 =====================================================================================*/
extern "C" const ulpInterface* ulp_get_interface ()
{
   return &ulpInf;
}

/*=============================================================================================
 * Function: ulp_init
 *
 * Description
 *   This function initializes ULP module.
 *
 * Parameters:
 *   arg: client passed argument
 *
 * Return value:
 *   0: on success
 *   non-zero: on failure
 =============================================================================================*/
int ulp_init(UlpEngineCallbacks* pEngineCallbacks,
              UlpNetworkLocationCallbacks* pNetworkLocationCallbacks,
              UlpPhoneContextCallbacks* pPhoneContextCallbacks)
{
   int retVal = -1;

   ENTRY_LOG_CALLFLOW();

   if ((pEngineCallbacks == NULL) ||
        (pNetworkLocationCallbacks == NULL) ||
        (pPhoneContextCallbacks == NULL)) {
       LOC_LOGE("ulp_init failed. cb = NULL\n");

       EXIT_LOG(%d, retVal);
       return retVal;
   }

   ulp_data.ulp_loc_cb = pEngineCallbacks->location_cb;
   ulp_data.ulp_create_thread_cb = pEngineCallbacks->create_thread_cb;
   ulp_data.ulp_phone_context_req_cb = pPhoneContextCallbacks->ulp_request_phone_context_cb;
   ulp_data.ulp_network_callback = pNetworkLocationCallbacks->ulp_network_location_request_cb;

    do
    {
       if (ulp_initialized)
       {
          retVal = 0;
          break;
       }
       ulp_initialized = true;

       // Initialize all data variables first in ULP module
       retVal = ulp_data_init ();
       if (retVal != 0)
       {
          LOC_LOGE ("%s: ulp_data_init failed \n", __func__);
          break;
       }

       // Initialize ULP message module
       retVal = ulp_msg_init ();
       if (retVal != 0)
       {
          LOC_LOGE ("%s: ulp_msg_init failed \n", __func__);
          break;
       }

       // Initialize ULP monitor module
       retVal = ulp_monitor_init ();
       if (retVal != 0)
       {
          LOC_LOGE ("%s: ulp_monitor_init failed \n", __func__);
          break;
       }

 #ifndef DEBUG_X86
       // Initialize ULP QUIPC module
       retVal = ulp_quipc_init ();
       if (retVal != 0)
       {
          LOC_LOGW ("%s: ulp_quipc_init failed \n", __func__);
       }
 #endif // DEBUG_X86

       LBSProxy::ulpRequestLoc(ulp_data.loc_proxy);

       retVal = 0;
    } while (0);

   EXIT_LOG(%d, retVal);
   return retVal;

}

static int ulp_engine_start()
{
    ENTRY_LOG();
    //Pass the start messgage to ULP if present & activated
    ulp_msg *msg(new ulp_msg(&ulp_data, ULP_MSG_START_FIX));
    int ret_val = msg_q_snd(ulp_data.loc_proxy->mQ,
                            msg, ulp_msg_free);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

static int ulp_engine_stop()
{
    ENTRY_LOG();
    //Pass the start messgage to ULP if present & activated
    ulp_msg *msg(new ulp_msg(&ulp_data, ULP_MSG_STOP_FLP_FIX));
    int ret_val = msg_q_snd(ulp_data.loc_proxy->mQ,
                            msg, ulp_msg_free);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_update_criteria

DESCRIPTION
   This is used to inform the ULP module of new unique criteria that are passed
   in by the applications
DEPENDENCIES
   N/A

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_engine_update_criteria(UlpLocationCriteria criteria)
{
    ENTRY_LOG();
    LOC_LOGD("valid 0x%x action:%d, minTime:%ld, minDistance:%f, singleShot:%d, "
             "horizontalAccuracy:%d, powerRequirement:%d \n",
              criteria.valid_mask, criteria.action, criteria.min_interval,
              criteria.min_distance, criteria.recurrence_type,
              criteria.preferred_horizontal_accuracy,
              criteria.preferred_power_consumption );
    ulp_msg_update_criteria *msg(
      new ulp_msg_update_criteria(&ulp_data,criteria));
    int ret_val = msg_q_snd(ulp_data.loc_proxy->mQ,
                            msg, ulp_msg_free);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    ulp_get_extension

DESCRIPTION
   Get the gps extension to support all ULP operations.

DEPENDENCIES
   N/A

RETURN VALUE
   The GPS extension interface.

SIDE EFFECTS
   N/A

===========================================================================*/
const void* ulp_get_extension(const char* name)
{
    ENTRY_LOG();
    const void* ret_val = NULL;

   LOC_LOGD("%s:%d] For Interface = %s\n",__func__, __LINE__, name);
   if (strcmp(name, ULP_ENGINE_INTERFACE) == 0)
   {
      ret_val = &sLocEngUlpEngInterface;
   }
   else if (strcmp(name, ULP_RAW_CMD_INTERFACE) == 0)
   {
      ret_val = &sLocEngInjectRawCmdInterface;
   }
   else if(strcmp(name, ULP_PHONE_CONTEXT_INTERFACE) == 0)
   {
     ret_val = &sLocEngUlpPhoneContextInterface;
   }
   else if(strcmp(name, ULP_NETWORK_INTERFACE) == 0)
   {
     ret_val = &sUlpNetworkInterface;
   }

   else
   {
      LOC_LOGE ("get_extension: Invalid interface passed in\n");
   }
    EXIT_LOG(%p, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    ulp_phone_context_settings_update

DESCRIPTION
   This is used to inform the ULP module of phone settings changes carried out
   by the users
DEPENDENCIES
   N/A

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/

static int ulp_phone_context_settings_update(UlpPhoneContextSettings *settings)
{
    ENTRY_LOG();
    int ret_val = 0;
    ulp_msg_inject_phone_context_settings *msg
     (new ulp_msg_inject_phone_context_settings(&ulp_data, *settings));
    msg_q_snd(ulp_data.loc_proxy->mQ, msg,
              ulp_msg_free);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    ulp_send_network_position

DESCRIPTION
   Ulp send data

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int ulp_send_network_position(UlpNetworkPositionReport *position_report)
{
    ENTRY_LOG();
    int ret_val = 0;
    ulp_msg_inject_network_position *msg
    (new ulp_msg_inject_network_position(&ulp_data, *position_report));
    msg_q_snd(ulp_data.loc_proxy->mQ, msg, ulp_msg_free);
    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    ulp_inject_raw_command

DESCRIPTION
   This is used to send special test commands from the applications
   down into the ULP
DEPENDENCIES
   N/A

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static bool ulp_inject_raw_command(char* command, int length)
{
    ENTRY_LOG();
    ulp_msg_inject_raw_command *msg(
        new ulp_msg_inject_raw_command(&ulp_data,command, length));
    int ret_val = msg_q_snd(ulp_data.loc_proxy->mQ,
                            msg, ulp_msg_free);

    EXIT_LOG(%s, loc_logger_boolStr[ret_val!=0]);
    return ret_val;
}

/*===========================================================================
FUNCTION    ulp_phone_system_update

DESCRIPTION
   This is used to inform the ULP module of certain system events that are passed
   in by the framework
DEPENDENCIES
   N/A

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/

static int ulp_phone_system_update(UlpSystemEvent event)
{
    ENTRY_LOG();
    int ret_val = 0;
    ulp_msg_system_update *msg(new ulp_msg_system_update(&ulp_data, event));
    ret_val = msg_q_snd(ulp_data.loc_proxy->mQ, msg,ulp_msg_free);

    EXIT_LOG(%s, loc_logger_boolStr[ret_val!=0]);
    return ret_val;
}




