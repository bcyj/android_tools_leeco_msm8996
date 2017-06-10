/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_ulp_log"

#include "loc_log.h"
#include "ulp_log.h"
#include "ulp_engine.h"

static loc_name_val_s_type ulp_msg_name[] =
{
    NAME_VAL( ULP_MSG_UPDATE_CRITERIA ),
    NAME_VAL( ULP_MSG_START_FIX ),
    NAME_VAL( ULP_MSG_STOP_FLP_FIX ),
    NAME_VAL( ULP_MSG_STOP_GNSS_FIX ),
    NAME_VAL( ULP_MSG_INJECT_PHONE_CONTEXT_SETTINGS ),
    NAME_VAL( ULP_MSG_INJECT_NETWORK_POSITION ),
    NAME_VAL( ULP_MSG_REPORT_QUIPC_POSITION ),
    NAME_VAL( ULP_MSG_REQUEST_COARSE_POSITION ),
    NAME_VAL( ULP_MSG_MONITOR ),
    NAME_VAL( ULP_MSG_INJECT_RAW_COMMAND ),
    NAME_VAL( ULP_MSG_SET_POSITION_MODE ),
    NAME_VAL( ULP_MSG_REPORT_POSITION),
    NAME_VAL( ULP_MSG_REPORT_SV)
};
static int ulp_msg_num = sizeof(ulp_msg_name) / sizeof(loc_name_val_s_type);

/* Find Android GPS status name */
const char* ulp_get_ulp_msg_name(int id)
{
   return loc_get_name_from_val(ulp_msg_name, ulp_msg_num, (long)id);
}
