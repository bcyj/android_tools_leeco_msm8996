/*!
  @file
  qcril_log.c

  @brief
  REQUIRED brief description of this C module.

  @detail
  OPTIONAL detailed description of this C module.
  - DELETE this section if unused.

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/28/10   js      fixed GSTK QMI raw command callback
05/13/10   at      Featurization for FEATURE_QCRIL_UIM_QMI
03/01/10   fc      Re-architecture to support split modem.


===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0
#include <utils/Log.h>
#include "common_log.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <cutils/properties.h>
#include <pthread.h>
#include "qcril_log.h"

#define LOG_TAG "RILC"
#define QCRIL_RPC_TAG "QCRIL_RPC"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/



/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* Flag that controls whether QCRIL debug messages logged on ADB or not */
static boolean qcril_log_adb_on = FALSE;

/* QCRIL request names */
static const qcril_qmi_event_log_type qcril_request_name[]  =
{
  /* 1 */
  { RIL_REQUEST_GET_SIM_STATUS,                       "RIL_REQUEST_GET_SIM_STATUS" },
  { RIL_REQUEST_ENTER_SIM_PIN,                        "RIL_REQUEST_ENTER_SIM_PIN" },
  { RIL_REQUEST_ENTER_SIM_PUK,                        "RIL_REQUEST_ENTER_SIM_PUK" },
  { RIL_REQUEST_ENTER_SIM_PIN2,                       "RIL_REQUEST_ENTER_SIM_PIN2" },
  { RIL_REQUEST_ENTER_SIM_PUK2,                       "RIL_REQUEST_ENTER_SIM_PUK2" },
  { RIL_REQUEST_CHANGE_SIM_PIN,                       "RIL_REQUEST_CHANGE_SIM_PIN" },
  { RIL_REQUEST_CHANGE_SIM_PIN2,                      "RIL_REQUEST_CHANGE_SIM_PIN2" },
  { RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE,         "RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE" },
  { RIL_REQUEST_GET_CURRENT_CALLS,                    "RIL_REQUEST_GET_CURRENT_CALLS" },
  { RIL_REQUEST_DIAL,                                 "RIL_REQUEST_DIAL" },
  /* 11 */
  { RIL_REQUEST_GET_IMSI,                             "RIL_REQUEST_GET_IMSI" },
  { RIL_REQUEST_HANGUP,                               "RIL_REQUEST_HANGUP" },
  { RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND,         "RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND" },
  { RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND,  "RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND" },
  { RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, "RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE" },
  { RIL_REQUEST_CONFERENCE,                           "RIL_REQUEST_CONFERENCE" },
  { RIL_REQUEST_UDUB,                                 "RIL_REQUEST_UDUB" },
  { RIL_REQUEST_LAST_CALL_FAIL_CAUSE,                 "RIL_REQUEST_LAST_CALL_FAIL_CAUSE" },
  { RIL_REQUEST_SIGNAL_STRENGTH,                      "RIL_REQUEST_SIGNAL_STRENGTH" },
  { RIL_REQUEST_REGISTRATION_STATE,                   "RIL_REQUEST_REGISTRATION_STATE" },
  /* 21 */
  { RIL_REQUEST_DATA_REGISTRATION_STATE,              "RIL_REQUEST_DATA_REGISTRATION_STATE" },
  { RIL_REQUEST_OPERATOR,                             "RIL_REQUEST_OPERATOR" },
  { RIL_REQUEST_RADIO_POWER,                          "RIL_REQUEST_RADIO_POWER" },
  { RIL_REQUEST_DTMF,                                 "RIL_REQUEST_DTMF" },
  { RIL_REQUEST_SEND_SMS,                             "RIL_REQUEST_SEND_SMS" },
  { RIL_REQUEST_SEND_SMS_EXPECT_MORE,                 "RIL_REQUEST_SEND_SMS_EXPECT_MORE" },
  { RIL_REQUEST_SETUP_DATA_CALL,                      "RIL_REQUEST_SETUP_DATA_CALL" },
  { RIL_REQUEST_SIM_IO,                               "RIL_REQUEST_SIM_IO" },
  { RIL_REQUEST_SEND_USSD,                            "RIL_REQUEST_SEND_USSD" },
  { RIL_REQUEST_CANCEL_USSD,                          "RIL_REQUEST_CANCEL_USSD" },
  /* 31 */
  { RIL_REQUEST_GET_CLIR,                             "RIL_REQUEST_GET_CLIR" },
  { RIL_REQUEST_SET_CLIR,                             "RIL_REQUEST_SET_CLIR" },
  { RIL_REQUEST_QUERY_CALL_FORWARD_STATUS,            "RIL_REQUEST_QUERY_CALL_FORWARD_STATUS" },
  { RIL_REQUEST_SET_CALL_FORWARD,                     "RIL_REQUEST_SET_CALL_FORWARD" },
  { RIL_REQUEST_QUERY_CALL_WAITING,                   "RIL_REQUEST_QUERY_CALL_WAITING" },
  { RIL_REQUEST_SET_CALL_WAITING,                     "RIL_REQUEST_SET_CALL_WAITING" },
  { RIL_REQUEST_SMS_ACKNOWLEDGE,                      "RIL_REQUEST_SMS_ACKNOWLEDGE" },
  { RIL_REQUEST_GET_IMEI,                             "RIL_REQUEST_GET_IMEI" },
  { RIL_REQUEST_GET_IMEISV,                           "RIL_REQUEST_GET_IMEISV" },
  { RIL_REQUEST_ANSWER,                               "RIL_REQUEST_ANSWER" },
  /* 41 */
  { RIL_REQUEST_DEACTIVATE_DATA_CALL,                 "RIL_REQUEST_DEACTIVATE_DATA_CALL" },
  { RIL_REQUEST_QUERY_FACILITY_LOCK,                  "RIL_REQUEST_QUERY_FACILITY_LOCK" },
  { RIL_REQUEST_SET_FACILITY_LOCK,                    "RIL_REQUEST_SET_FACILITY_LOCK" },
  { RIL_REQUEST_CHANGE_BARRING_PASSWORD,              "RIL_REQUEST_CHANGE_BARRING_PASSWORD" },
  { RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE,         "RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE" },
  { RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC,      "RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC" },
  { RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL,         "RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL" },
  { RIL_REQUEST_QUERY_AVAILABLE_NETWORKS,             "RIL_REQUEST_QUERY_AVAILABLE_NETWORKS" },
  { RIL_REQUEST_DTMF_START,                           "RIL_REQUEST_DTMF_START" },
  { RIL_REQUEST_DTMF_STOP,                            "RIL_REQUEST_DTMF_STOP" },
  /* 51 */
  { RIL_REQUEST_BASEBAND_VERSION, "RIL_REQUEST_BASEBAND_VERSION" },
  { RIL_REQUEST_SEPARATE_CONNECTION, "RIL_REQUEST_SEPARATE_CONNECTION" },
  { RIL_REQUEST_SET_MUTE, "RIL_REQUEST_SET_MUTE" },
  { RIL_REQUEST_GET_MUTE, "RIL_REQUEST_GET_MUTE" },
  { RIL_REQUEST_QUERY_CLIP, "RIL_REQUEST_QUERY_CLIP" },
  { RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, "RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE" },
  { RIL_REQUEST_DATA_CALL_LIST, "RIL_REQUEST_DATA_CALL_LIST" },
  { RIL_REQUEST_RESET_RADIO, "RIL_REQUEST_RESET_RADIO" },
  { RIL_REQUEST_OEM_HOOK_RAW, "RIL_REQUEST_OEM_HOOK_RAW" },
  { RIL_REQUEST_OEM_HOOK_STRINGS, "RIL_REQUEST_OEM_HOOK_STRINGS" },
  /* 61 */
  { RIL_REQUEST_SCREEN_STATE,                         "RIL_REQUEST_SCREEN_STATE" },
  { RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION,            "RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION" },
  { RIL_REQUEST_WRITE_SMS_TO_SIM,                     "RIL_REQUEST_WRITE_SMS_TO_SIM" },
  { RIL_REQUEST_DELETE_SMS_ON_SIM,                    "RIL_REQUEST_DELETE_SMS_ON_SIM" },
  { RIL_REQUEST_SET_BAND_MODE,                        "RIL_REQUEST_SET_BAND_MODE" },
  { RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE,            "RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE" },
  { RIL_REQUEST_STK_GET_PROFILE,                      "RIL_REQUEST_STK_GET_PROFILE" },
  { RIL_REQUEST_STK_SET_PROFILE,                      "RIL_REQUEST_STK_SET_PROFILE" },
  {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND,             "RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND" },
  { RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE,           "RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE" },
  /* 71 */
  { RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, "RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM" },
  { RIL_REQUEST_EXPLICIT_CALL_TRANSFER,               "RIL_REQUEST_EXPLICIT_CALL_TRANSFER" },
  { RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE,           "RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE" },
  { RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE,           "RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE" },
  { RIL_REQUEST_GET_NEIGHBORING_CELL_IDS,             "RIL_REQUEST_GET_NEIGHBORING_CELL_IDS" },
  { RIL_REQUEST_SET_LOCATION_UPDATES,                 "RIL_REQUEST_SET_LOCATION_UPDATES" },
  { RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE,         "RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE" },
  { RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE,          "RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE" },
  { RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE,        "RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE" },
  { RIL_REQUEST_SET_TTY_MODE,                         "RIL_REQUEST_SET_TTY_MODE" },
  /* 81 */
  { RIL_REQUEST_QUERY_TTY_MODE,                       "RIL_REQUEST_QUERY_TTY_MODE" },
  { RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, "RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE" },
  { RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, "RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE" },
  { RIL_REQUEST_CDMA_FLASH,                           "RIL_REQUEST_CDMA_FLASH" },
  { RIL_REQUEST_CDMA_BURST_DTMF,                      "RIL_REQUEST_CDMA_BURST_DTMF" },
  { RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY,         "RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY" },
  { RIL_REQUEST_CDMA_SEND_SMS,                        "RIL_REQUEST_CDMA_SEND_SMS" },
  { RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE,                 "RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE" },
  { RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG,         "RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG" },
  { RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG,         "RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG" },
  /* 91 */
  { RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION,         "RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION" },
  { RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG,        "RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG" },
  { RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG,        "RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG" },
  { RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,        "RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION" },
  { RIL_REQUEST_CDMA_SUBSCRIPTION,                    "RIL_REQUEST_CDMA_SUBSCRIPTION" },
  { RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM,               "RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM" },
  { RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM,              "RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM" },
  { RIL_REQUEST_DEVICE_IDENTITY,                      "RIL_REQUEST_DEVICE_IDENTITY" },
  { RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE,         "RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE" },
  { RIL_REQUEST_GET_SMSC_ADDRESS,                     "RIL_REQUEST_GET_SMSC_ADDRESS" },
  /* 101 */
  { RIL_REQUEST_SET_SMSC_ADDRESS,                     "RIL_REQUEST_SET_SMSC_ADDRESS" },
  { RIL_REQUEST_REPORT_SMS_MEMORY_STATUS,             "RIL_REQUEST_REPORT_SMS_MEMORY_STATUS" },
  { RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING,        "RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING" },
  { RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE,         "RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE" },
  { RIL_REQUEST_ISIM_AUTHENTICATION,                   "RIL_REQUEST_ISIM_AUTHENTICATION" },
  { RIL_REQUEST_VOICE_RADIO_TECH,                     "RIL_REQUEST_VOICE_RADIO_TECH" },
  { RIL_REQUEST_IMS_REGISTRATION_STATE,               "RIL_REQUEST_IMS_REGISTRATION_STATE" },
  { RIL_REQUEST_IMS_SEND_SMS,                         "RIL_REQUEST_IMS_SEND_SMS" },
  /* 108 */
  { RIL_REQUEST_GET_DATA_CALL_PROFILE,                "RIL_REQUEST_GET_DATA_CALL_PROFILE" },
  { RIL_REQUEST_SET_UICC_SUBSCRIPTION,                "RIL_REQUEST_SET_UICC_SUBSCRIPTION" },
  { RIL_REQUEST_SET_DATA_SUBSCRIPTION,                "RIL_REQUEST_SET_DATA_SUBSCRIPTION" },
  { RIL_REQUEST_GET_UICC_SUBSCRIPTION,                "RIL_REQUEST_GET_UICC_SUBSCRIPTION" },
  { RIL_REQUEST_GET_DATA_SUBSCRIPTION,                "RIL_REQUEST_GET_DATA_SUBSCRIPTION" },
  { RIL_REQUEST_SET_SUBSCRIPTION_MODE,                "RIL_REQUEST_SET_SUBSCRIPTION_MODE" },
  /* 114 */
  { RIL_REQUEST_SET_TRANSMIT_POWER,                   "RIL_REQUEST_SET_TRANSMIT_POWER" },
#if (RIL_QCOM_VERSION >= 2)
  { RIL_REQUEST_SETUP_QOS,                            "RIL_REQUEST_SETUP_QOS" },
  { RIL_REQUEST_RELEASE_QOS,                          "RIL_REQUEST_RELEASE_QOS" },
  { RIL_REQUEST_GET_QOS_STATUS,                       "RIL_REQUEST_GET_QOS_STATUS" },
  { RIL_REQUEST_MODIFY_QOS,                           "RIL_REQUEST_MODIFY_QOS" },
  { RIL_REQUEST_SUSPEND_QOS,                          "RIL_REQUEST_SUSPEND_QOS" },
  { RIL_REQUEST_RESUME_QOS,                           "RIL_REQUEST_RESUME_QOS"},
#endif
  { RIL_REQUEST_UNKOWN,                           "RIL_REQUEST_UNKOWN"},  
  /* 121 */
};

static const qcril_qmi_event_log_type qcril_unsol_response_name[] =
{
  /* 1000 */
  { RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED,           "RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED" },
  /* 1001 */
  { RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED,            "RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED" },
  { RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED,         "RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED" },
  { RIL_UNSOL_RESPONSE_NEW_SMS,                       "RIL_UNSOL_RESPONSE_NEW_SMS" },
  { RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT,         "RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT" },
  { RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM,                "RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM" },
  { RIL_UNSOL_ON_USSD,                                "RIL_UNSOL_ON_USSD" },
  { RIL_UNSOL_ON_USSD_REQUEST,                        "RIL_UNSOL_ON_USSD_REQUEST" },
  { RIL_UNSOL_NITZ_TIME_RECEIVED,                     "RIL_UNSOL_NITZ_TIME_RECEIVED" },
  { RIL_UNSOL_SIGNAL_STRENGTH,                        "RIL_UNSOL_SIGNAL_STRENGTH" },
  { RIL_UNSOL_DATA_CALL_LIST_CHANGED,                 "RIL_UNSOL_DATA_CALL_LIST_CHANGED" },
  /*1011 */
  { RIL_UNSOL_SUPP_SVC_NOTIFICATION,                  "RIL_UNSOL_SUPP_SVC_NOTIFICATION" },
  { RIL_UNSOL_STK_SESSION_END,                        "RIL_UNSOL_STK_SESSION_END" },
  { RIL_UNSOL_STK_PROACTIVE_COMMAND,                  "RIL_UNSOL_STK_PROACTIVE_COMMAND" },
  { RIL_UNSOL_STK_EVENT_NOTIFY,                       "RIL_UNSOL_STK_EVENT_NOTIFY" },
  { RIL_UNSOL_STK_CALL_SETUP,                         "RIL_UNSOL_STK_CALL_SETUP" },
  { RIL_UNSOL_SIM_SMS_STORAGE_FULL,                   "RIL_UNSOL_SIM_SMS_STORAGE_FULL" },
  { RIL_UNSOL_SIM_REFRESH,                            "RIL_UNSOL_SIM_REFRESH" },
  { RIL_UNSOL_CALL_RING,                              "RIL_UNSOL_CALL_RING" },
  { RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED,            "RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED" },
  { RIL_UNSOL_RESPONSE_CDMA_NEW_SMS,                  "RIL_UNSOL_RESPONSE_CDMA_NEW_SMS" },
  /* 1021 */
  { RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS,             "RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS" },
  { RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL,             "RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL" },
  { RIL_UNSOL_RESTRICTED_STATE_CHANGED,               "RIL_UNSOL_RESTRICTED_STATE_CHANGED" },
  { RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE,          "RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE" },
  { RIL_UNSOL_CDMA_CALL_WAITING,                      "RIL_UNSOL_CDMA_CALL_WAITING" },
  { RIL_UNSOL_CDMA_OTA_PROVISION_STATUS,                "RIL_UNSOL_CDMA_OTA_PROVISION_STATUS" },
  { RIL_UNSOL_CDMA_INFO_REC,                          "RIL_UNSOL_CDMA_INFO_REC" },
  { RIL_UNSOL_OEM_HOOK_RAW,                           "RIL_UNSOL_OEM_HOOK_RAW" },
  { RIL_UNSOL_RINGBACK_TONE,                          "RIL_UNSOL_RINGBACK_TONE" },
  { RIL_UNSOL_RESEND_INCALL_MUTE,                     "RIL_UNSOL_RESEND_INCALL_MUTE" },
  /* 1031 */
  { RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED,       "RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED" },
  { RIL_UNSOL_CDMA_PRL_CHANGED,                       "RIL_UNSOL_CDMA_PRL_CHANGED" },
  { RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE,           "RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE" },
  { RIL_UNSOL_RIL_CONNECTED,                          "RIL_UNSOL_RIL_CONNECTED" },
  { RIL_UNSOL_VOICE_RADIO_TECH_CHANGED,               "RIL_UNSOL_VOICE_RADIO_TECH_CHANGED" },
  { RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,     "RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED" },
  { RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED,     "RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED" },
  { RIL_UNSOL_ON_SS,                                  "RIL_UNSOL_ON_SS" },
  { RIL_UNSOL_STK_CC_ALPHA_NOTIFY,                    "RIL_UNSOL_STK_CC_ALPHA_NOTIFY" },
  { RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED,       "RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED" },
#if (RIL_QCOM_VERSION >= 2)
  { RIL_UNSOL_QOS_STATE_CHANGED_IND,                  "RIL_UNSOL_QOS_STATE_CHANGED_IND" },
#endif
  { RIL_UNSOL_UNKOWN,                                 "RIL_UNSOL_UNKOWN"}
 };

/* QCRIL errno names */
static const char *qcril_errno_name[] =
{
  /* 0 */
  "Success",
  "Radio Not Available",
  "Generic Failure",
  "Password Incorrect",
  "SIM Pin2",
  "SIM Puk2",
  "Request Not Supported",
  "Cancelled",
  "OP Not Allowed During Voice Call",
  "OP Not Allowed Before Reg To NW",
  /* 10 */
  "SMS Send Fail Retry",
  "SIM Absent",
  "Subscription Not Available",
  "Mode Not Supported",
  "FDN Check Failure",
  "Illegal SIM Or ME",
  "Setup Data Call Failure",
  "Dial Modified To USSD",
  "Dial Modified To SS",
  "Dial Modified To DIAL",
  /* 20 */
  "USSD Modified To DIAL",
  "USSD Modified To SS",
  "USSD Modified To USSD",
  "SS Modified To DIAL",
  "SS Modified To USSD",
  "SS Modified To SS",
  "Subscription Not Supported"
};


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES


===========================================================================*/


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_log_init

===========================================================================*/
/*!
    @brief
    Initialization for logging.

    @return
    None
*/
/*=========================================================================*/
void qcril_log_init
(
  void
)
{
  char args[ PROPERTY_VALUE_MAX ];     
  int len;
  char *end_ptr;
  unsigned long ret_val;

  /*-----------------------------------------------------------------------*/

  /* Initialize Diag for QCRIL logging */
  ret_val = Diag_LSM_Init(NULL);
  if ( !ret_val )
  {
    LOGE( "Fail to initialize Diag for QCRIL logging\n" );
  }

  QCRIL_LOG_DEBUG ( "%s\n", "qcril_log_init()" );

  #ifdef FEATURE_QCRIL_ADB_LOG_ON
  qcril_log_adb_enabled = TRUE;
  #endif /* FEATURE_QCRIL_ADB_LOG_ON */

  property_get( QCRIL_LOG_ADB_ON, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 ); 
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) ) 
    {
      QCRIL_LOG_ERROR( "Fail to convert adb_log_on setting %s\n", args );
    }
    else if ( ret_val > 1 ) 
    {
      QCRIL_LOG_ERROR( "Invalid saved adb_log_on setting %ld, use default\n", ret_val );
    }
    else
    { 
      qcril_log_adb_on = ( boolean ) ret_val;
    }
  }
  
  QCRIL_LOG_DEBUG( "adb_log_on = %d\n", qcril_log_adb_on );

  /* Save ADB Log Enabled setting to system property */
  QCRIL_SNPRINTF( args, sizeof( args ), "%d", qcril_log_adb_on );
  if ( property_set( QCRIL_LOG_ADB_ON, args ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "Fail to save %s to system property\n", QCRIL_LOG_ADB_ON );
  }                                                                                

} /* qcril_log_init */


/*=========================================================================
  FUNCTION:  qcril_format_log_msg

===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void qcril_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( buf_ptr != NULL );
  QCRIL_ASSERT( buf_size > 0 );

  /*-----------------------------------------------------------------------*/

  va_start( ap, fmt );

  vsnprintf( buf_ptr, buf_size, fmt, ap );

  va_end( ap );

} /* qcril_format_log_msg */


/*=========================================================================
  FUNCTION:  qcril_log_call_flow_packet

===========================================================================*/
/*!
    @brief
    Log the call flow packet.

    @return
    None
*/
/*=========================================================================*/
void qcril_log_call_flow_packet
(
  qcril_call_flow_subsystem_e_type src_subsystem,
  qcril_call_flow_subsystem_e_type dest_subsystem,
  qcril_call_flow_arrow_e_type arrow,
  char *label
)
{
  #ifndef FEATURE_UNIT_TEST
  qcril_call_flow_log_packet_type *log_buf;                                                                
  uint16 label_len, log_packet_size = 0;                                                                   
                                                                                                           
  /* Calculate the size of the log packet */                                                              
  label_len = strlen( label );                                                                             
  log_packet_size = sizeof( qcril_call_flow_log_packet_type ) + label_len;                                

  /* Allocate log buffer */                                                                               
  log_buf = ( qcril_call_flow_log_packet_type * ) log_alloc( LOG_QCRIL_CALL_FLOW_C, log_packet_size ); 
                                                                                                           
  if ( log_buf != NULL )
  {                                                                         
    /* Fill in the log buffer */                                                                            
    log_buf->src_subsystem = (uint8) src_subsystem;                                                          
    log_buf->dest_subsystem = (uint8) dest_subsystem;                                                       
    log_buf->arrow = (uint8) arrow;                                                                        
    log_buf->label[ 0 ] = '\0';                                                                             
    if ( label_len > 0 )                                                                                    
    {                                                                                                       
      memcpy( (void *) log_buf->label, label, label_len + 1 );
    }                                                                                                      

    /* Commit log buffer */                                                                                  
    log_commit( log_buf ); 
  }
  #endif /* !FEATURE_UNIT_TEST */
                                                                                                            
} /* qcril_log_call_flow_packet */


/*=========================================================================
  FUNCTION:  qcril_log_msg_to_adb_main

===========================================================================*/
/*!
    @brief
    Log debug message to ADB main buffer.

    @return
    None
*/
/*=========================================================================*/
void qcril_log_msg_to_adb_main
(
  char *msg_ptr
)
{
  LOG( LOG_DEBUG, QCRIL_RPC_TAG, "%s", msg_ptr );
}

/*=========================================================================
  FUNCTION:  qcril_log_msg_to_adb

===========================================================================*/
/*!
    @brief
    Log debug message to ADB.

    @return
    None
*/
/*=========================================================================*/
void qcril_log_msg_to_adb
(
  int  lvl,
  char *msg_ptr
)
{
  if ( qcril_log_adb_on )
  {
    switch ( lvl )
    {
      case MSG_LEGACY_ERROR:
      case MSG_LEGACY_FATAL:
        LOGE( "%s", msg_ptr ); 
        break;

      case MSG_LEGACY_HIGH:
        LOGD( "%s", msg_ptr );    
        break;  

      default:
        break;
    }
  }

} /* qcril_log_msg_to_adb */


/*===========================================================================

  FUNCTION: QCRIL_LOG_GET_TOKEN_ID

===========================================================================*/
/*!
    @brief
    Return the value of the Token ID.

    @return
    The value of Token ID
*/
/*=========================================================================*/
int qcril_log_get_token_id
(
  RIL_Token t
)
{
  int token_id = 0;

  /*-----------------------------------------------------------------------*/

  if ( t == NULL )
  {
    token_id = 0xFFFE;
  }
  else if ( t == ( void * ) QCRIL_TOKEN_ID_INTERNAL )
  {
    token_id = 0xFFFF;
  }
  else if ( t == ( void * ) QCRIL_TOKEN_ID_INTERNAL1 )
  {
    token_id = 0xFFFF1;
  }
  else
  {
    token_id =  *( (int *) t );
  }

  return token_id;

} /* qcril_log_get_token_id */


/*===========================================================================

  FUNCTION: QCRIL_LOG_LOOKUP_EVENT_NAME

===========================================================================*/
/*!
    @brief
    Lookup the name of a QCRIL event

    @return
    The string respresenting the name of a QCRIL request
*/
/*=========================================================================*/
const char *qcril_log_lookup_event_name
(
  int event_id
)
{
  /*-----------------------------------------------------------------------*/
  int index;

  /* Lookup the name of a RIL request */
  if ( event_id < RIL_UNSOL_RESPONSE_BASE )
  {
     index = qcril_log_lookup_ril_event_index( event_id, qcril_request_name, 0, sizeof(qcril_request_name)/sizeof(qcril_request_name[0]) - 1);
     return qcril_request_name[ index ].event_name;
  }
  /* Lookup the name of a RIL unsolicited response */
  else if ( event_id < (int) QCRIL_EVT_BASE )
  {
     index = qcril_log_lookup_ril_event_index( event_id, qcril_unsol_response_name, 0, sizeof(qcril_unsol_response_name)/sizeof(qcril_unsol_response_name[0]) - 1);
     return qcril_unsol_response_name[ index ].event_name;
  }
  /* Lookup the name of a QCRIL event */
  else
  {
    /* NOTE: All internal QCRIL events must return a string that prefix with "INTERNAL_" in order to support AMSS event profiling */
    switch( event_id )
    {
      case QCRIL_EVT_NONE:
        return "<none>";

      case QCRIL_EVT_CM_COMMAND_CALLBACK:
        return "CM_COMMAND_CALLBACK";

      #ifdef FEATURE_QCRIL_SUBS_CTRL
      case QCRIL_EVT_CM_ENABLE_SUBSCRIPTION:
        return "CM_ENABLE_SUBSCRIPTION";

      case QCRIL_EVT_CM_DISABLE_SUBSCRIPTION:
        return "CM_DISABLE_SUBSCRIPTION";
      #endif /* FEATURE_QCRIL_SUBS_CTRL */

      case QCRIL_EVT_CM_UPDATE_FDN_STATUS:
        return "CM_UPDATE_FDN_STATUS";

      case QCRIL_EVT_CM_CARD_STATUS_UPDATED:
        return "CM_CARD_STATUS_UPDATED";

      case QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS:
        return "CM_ACTIVATE_PROVISION_STATUS";

      case QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS:
        return "CM_DEACTIVATE_PROVISION_STATUS";

      case QCRIL_EVT_CM_PH_OPRT_MODE:
        return "CM_PH_EVENT_OPRT_MODE";

      case QCRIL_EVT_CM_PH_INFO:
        return "CM_PH_EVENT_INFO";

      case QCRIL_EVT_CM_PH_SYS_SEL_PREF:
        return "CM_PH_EVENT_SYS_SEL_PREF";

      #ifdef FEATURE_QCRIL_PRL_INIT
      case QCRIL_EVT_CM_PH_PRL_INIT:
        return "CM_PH_EVENT_PRL_INIT";
      #endif /* FEATURE_QCRIL_PRL_INIT */

      case QCRIL_EVT_CM_PH_SUBSCRIPTION_AVAILABLE:
        return "CM_PH_EVENT_SUBSCRIPTION_AVAILABLE";

      #ifdef FEATURE_QCRIL_SUBS_CTRL
      case QCRIL_EVT_CM_PH_SUBSCRIPTION_NOT_AVAILABLE:
        return "CM_PH_EVENT_SUBSCRIPTION_NOT_AVAILABLE";
      #endif /* FEATURE_QCRIL_SUBS_CTRL */

      #ifdef FEATURE_QCRIL_DSDS
      case QCRIL_EVT_CM_PH_DUAL_STANDBY_PREF:
        return "CM_PH_EVENT_DUAL_STANDBY_PREF";

      case QCRIL_EVT_CM_PH_SUBSCRIPTION_PREF_INFO:
        return "CM_PH_EVENT_SUBSCRIPTION_PREF_INFO";
      #endif /* FEATURE_QCRIL_DSDS */

      case QCRIL_EVT_CM_PH_AVAILABLE_NETWORKS_CONF:
        return "CM_PH_EVENT_AVAILABLE_NETWORKS_CONF";

      case QCRIL_EVT_CM_PH_TERMINATE_GET_NETWORKS:
        return "CM_PH_EVENT_TERMINATE_GET_NETWORKS";

      case QCRIL_EVT_CM_PH_NVRUIM_CONFIG_CHANGED:
        return "CM_PH_NVRUIM_CONFIG_CHANGED";

      case QCRIL_EVT_CM_SS_SRV_CHANGED:
        return "CM_SS_EVENT_SRV_CHANGED";

      case QCRIL_EVT_CM_SS_RSSI:
        return "CM_SS_EVENT_RSSI";

      case QCRIL_EVT_CM_SS_INFO:
        return "CM_SS_EVENT_INFO";

      case QCRIL_EVT_CM_SS_REG_REJECT:
        return "CM_SS_EVENT_REG_REJECT";

      case QCRIL_EVT_CM_SS_HDR_RSSI:
        return "CM_SS_EVENT_HDR_RSSI";

      case QCRIL_EVT_CM_SS_EMERG_NUM_LIST:
        return "CM_SS_EVENT_EMERG_NUM_LIST";

      #ifdef FEATURE_QCRIL_DSAC
      case QCRIL_EVT_CM_SS_CELL_ACCESS_IND:
        return "CM_SS_EVENT_CELL_ACCESS_IND";
      #endif /* FEATURE_QCRIL_DSAC */

      case QCRIL_EVT_CM_CALL_ORIG:
        return "CM_CALL_EVENT_ORIG";

      case QCRIL_EVT_CM_CALL_ANSWER:
        return "CM_CALL_EVENT_ANSWER";

      case QCRIL_EVT_CM_CALL_END:
        return "CM_CALL_EVENT_END";

      case QCRIL_EVT_CM_CALL_INCOM:
        return "CM_CALL_EVENT_INCOM";

      case QCRIL_EVT_CM_CALL_CONNECT:
        return "CM_CALL_EVENT_CONNECT";

      case QCRIL_EVT_CM_CALL_MNG_CALLS_CONF:
        return "CM_CALL_EVENT_MNG_CALLS_CONF";

      case QCRIL_EVT_CM_CALL_BARRED:
        return "CM_CALL_EVENT_CALL_BARRED";

      case QCRIL_EVT_CM_CALL_ON_HOLD:
        return "CM_CALL_EVENT_CALL_ON_HOLD";

      case QCRIL_EVT_CM_CALL_IS_WAITING:
        return "CM_CALL_EVENT_CALL_IS_WAITING";

      case QCRIL_EVT_CM_CALL_RETRIEVED:
        return "CM_CALL_EVENT_CALL_RETRIEVED";

      case QCRIL_EVT_CM_CALL_ORIG_FWD_STATUS:
        return "CM_CALL_EVENT_ORIG_FWD_STATUS";

      case QCRIL_EVT_CM_CALL_FORWARDED:
        return "CM_CALL_EVENT_CALL_FORWARDED";

      case QCRIL_EVT_CM_CALL_BEING_FORWARDED:
        return "CM_CALL_EVENT_CALL_BEING_FORWARDED";

      case QCRIL_EVT_CM_CALL_INCOM_FWD_CALL:
        return "CM_CALL_EVENT_INCOM_FWD_CALL";

      case QCRIL_EVT_CM_CALL_RESTRICTED:
        return "CM_CALL_EVENT_CALL_RESTRICTED";

      case QCRIL_EVT_CM_CALL_CUG_INFO_RECEIVED:
        return "CM_CALL_EVENT_CUG_INFO_RECEIVED";

      case QCRIL_EVT_CM_CALL_SETUP_IND:
        return "CM_CALL_EVENT_SETUP_IND";

      case QCRIL_EVT_CM_CALL_PROGRESS_INFO_IND:
        return "CM_CALL_EVENT_PROGRESS_INFO_IND";

      case QCRIL_EVT_CM_CALL_USER_DATA_IND:
        return "CM_CALL_EVENT_USER_DATA_IND";

      case QCRIL_EVT_CM_CALL_DEFLECTION:
        return "CM_CALL_EVENT_CALL_DEFLECTION";

      case QCRIL_EVT_CM_CALL_TRANSFERRED_CALL:
        return "CM_CALL_EVENT_TRANSFERRED_CALL";

      case QCRIL_EVT_CM_CALL_SUPS:
        return "CM_CALL_EVENT_SUPS";

      case QCRIL_EVT_CM_CALL_CNAP_INFO_RECEIVED:
        return "CM_CALL_EVENT_CNAP_INFO_RECEIVED";

      case QCRIL_EVT_CM_CALL_PRIVACY:
        return "CM_CALL_EVENT_PRIVACY";

      case QCRIL_EVT_CM_CALL_PRIVACY_PREF:
        return "CM_CALL_EVENT_PRIVACY_PREF";

      case QCRIL_EVT_CM_CALL_CALLER_ID:
        return "CM_CALL_EVENT_CALLER_ID";
  
      case QCRIL_EVT_CM_CALL_SIGNAL:
        return "CM_CALL_EVENT_SIGNAL";

      case QCRIL_EVT_CM_CALL_DISPLAY:
        return "CM_CALL_EVENT_DISPLAY";

      case QCRIL_EVT_CM_CALL_CALLED_PARTY:
        return "CM_CALL_EVENT_CALLED_PARTY";

      case QCRIL_EVT_CM_CALL_CONNECTED_NUM:
        return "CM_CALL_EVENT_CONNECTED_NUM";

      case QCRIL_EVT_CM_CALL_EXT_DISP:
        return "CM_CALL_EVENT_EXT_DISP";

      case QCRIL_EVT_CM_CALL_EXT_BRST_INTL:
        return "CM_CALL_EVENT_EXT_BRST_INTL";

      case QCRIL_EVT_CM_CALL_NSS_CLIR_REC:
        return "CM_CALL_EVENT_NSS_CLIR_REC";

      case QCRIL_EVT_CM_CALL_NSS_REL_REC:
        return "CM_CALL_EVENT_NSS_REL_REC";

      case QCRIL_EVT_CM_CALL_NSS_AUD_CTRL:
        return "CM_CALL_EVENT_NSS_AUD_CTRL";

      case QCRIL_EVT_CM_CALL_REDIRECTING_NUMBER:
        return "CM_CALL_EVENT_REDIRECTING_NUMBER";

      case QCRIL_EVT_CM_CALL_LINE_CTRL:
        return "CM_CALL_EVENT_LINE_CTRL";

      case QCRIL_EVT_CM_CALL_OTASP_STATUS:
        return "CM_CALL_EVENT_OTASP_STATUS";

      case QCRIL_EVT_CM_INBAND_REV_BURST_DTMF:
        return "CM_INBAND_EVENT_REV_BURST_DTMF";

      case QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF:
        return "CM_INBAND_EVENT_REV_START_CONT_DTMF";

      case QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF_CONF:
        return "CM_INBAND_EVENT_REV_START_CONT_DTMF_CONF";

      case QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF:
        return "QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF";

      case QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF_CONF:
        return "CM_INBAND_EVENT_REV_STOP_CONT_DTMF_CONF";

      case QCRIL_EVT_CM_INBAND_FWD_BURST_DTMF:
        return "CM_INBAND_EVENT_FWD_BURST_DTMF";

      case QCRIL_EVT_CM_INBAND_FWD_START_CONT_DTMF:
        return "CM_INBAND_EVENT_FWD_START_CONT_DTMF";

      case QCRIL_EVT_CM_INBAND_FWD_STOP_CONT_DTMF:
        return "CM_INBAND_EVENT_FWD_STOP_CONT_DTMF";

      case QCRIL_EVT_CM_SUPS_REGISTER:
        return "CM_SUPS_EVENT_REGISTER";

      case QCRIL_EVT_CM_SUPS_REGISTER_CONF:
        return "CM_SUPS_EVENT_REGISTER_CONF";

      case QCRIL_EVT_CM_SUPS_ERASE:
        return "CM_SUPS_EVENT_ERASE";

      case QCRIL_EVT_CM_SUPS_ERASE_CONF:
        return "CM_SUPS_EVENT_ERASE_CONF";

       case QCRIL_EVT_CM_SUPS_ACTIVATE:
        return "CM_SUPS_EVENT_ACTIVATE";

      case QCRIL_EVT_CM_SUPS_ACTIVATE_CONF:
        return "CM_SUPS_EVENT_ACTIVATE_CONF";

      case QCRIL_EVT_CM_SUPS_DEACTIVATE:
        return "CM_SUPS_EVENT_DEACTIVATE";

      case QCRIL_EVT_CM_SUPS_DEACTIVATE_CONF:
        return "CM_SUPS_EVENT_DEACTIVATE_CONF";

      case QCRIL_EVT_CM_SUPS_INTERROGATE:
        return "CM_SUPS_EVENT_INTERROGATE";

      case QCRIL_EVT_CM_SUPS_INTERROGATE_CONF:
        return "CM_SUPS_EVENT_INTERROGATE_CONF";

      case QCRIL_EVT_CM_SUPS_REG_PASSWORD:
        return "CM_SUPS_EVENT_REG_PASSWORD";

      case QCRIL_EVT_CM_SUPS_REG_PASSWORD_CONF:
        return "CM_SUPS_EVENT_REG_PASSWORD_CONF";

      case QCRIL_EVT_CM_SUPS_PROCESS_USS_CONF:
        return "CM_SUPS_EVENT_PROCESS_USS_CONF";

      case QCRIL_EVT_CM_SUPS_PROCESS_USS:
        return "CM_SUPS_EVENT_PROCESS_USS";

      case QCRIL_EVT_CM_SUPS_USS_NOTIFY_IND:
        return "CM_SUPS_EVENT_USS_NOTIFY_IND";

      case QCRIL_EVT_CM_SUPS_USS_IND:
        return "CM_SUPS_EVENT_USS_IND";

      case QCRIL_EVT_CM_SUPS_RELEASE_USS_IND:
        return "CM_SUPS_EVENT_RELEASE_USS_IND";

      case QCRIL_EVT_CM_SUPS_GET_PASSWORD_IND:
        return "CM_SUPS_EVENT_GET_PASSWORD_IND";

      #ifdef FEATURE_QCRIL_NCELL
      case QCRIL_EVT_CM_STATS_MODEM_INFO:
        return "CM_STATS_EVENT_MODEM_INFO";
      #endif /* FEATURE_QCRIL_NCELL */

      case QCRIL_EVT_SMS_COMMAND_CALLBACK:
        return "SMS_COMMAND_CALLBACK";

      case QCRIL_EVT_SMS_SUBSCRIPTION_PREF_INFO:
        return "SMS_SUBSCRIPTION_PREF_INFO";

      #ifdef FEATURE_QCRIL_DSDS
      case QCRIL_EVT_SMS_CFG_MS_MEMORY_FULL:
        return "WMS_CFG_EVENT_MS_MEMORY_FULL";
      #else
      case QCRIL_EVT_SMS_CFG_MEMORY_FULL:
        return "WMS_CFG_EVENT_MEMORY_FULL";
      #endif /* FEATURE_QCRIL_DSDS */

      case QCRIL_EVT_SMS_SEND:
        return "WMS_MSG_EVENT_SEND";

      case QCRIL_EVT_SMS_WRITE:
        return "WMS_MSG_EVENT_WRITE";

      case QCRIL_EVT_SMS_DELETE:
        return "WMS_MSG_EVENT_DELETE";

      case QCRIL_EVT_SMS_RECEIVED_MESSAGE:
        return "WMS_MSG_EVENT_RECEIVED_MESSAGE";

      case QCRIL_EVT_SMS_SUBMIT_RPT:
        return "WMS_MSG_EVENT_SUBMIT_REPORT";

      case QCRIL_EVT_SMS_STATUS_RPT:
        return "WMS_MSG_EVENT_STATUS_REPORT";

      case QCRIL_EVT_SMS_BC_MM_PREF:
        return "WMS_BC_MM_EVENT_PREF";

      case QCRIL_EVT_SMS_BC_MM_TABLE:
        return "WMS_BC_MM_EVENT_TABLE";

      case QCRIL_EVT_SMS_BC_MM_ADD_SRVS:
        return "WMS_BC_MM_EVENT_ADD_SRVS";

      #ifdef FEATURE_QCRIL_DSDS
      case QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET:
        return "WMS_CFG_EVENT_MS_MEMORY_STATUS_SET";
      #else
      case QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET:
        return "WMS_CFG_EVENT_MEMORY_STATUS_SET";
      #endif /* FEATURE_QCRIL_DSDS */

      case QCRIL_EVT_SMS_CFG_MESSAGE_LIST:
        return "WMS_CFG_EVENT_MESSAGE_LIST";

      case QCRIL_EVT_SMS_READ_TEMPLATE:
        return "WMS_MSG_EVENT_READ_TEMPLATE";

      case QCRIL_EVT_SMS_WRITE_TEMPLATE:
        return "WMS_MSG_EVENT_WRITE_TEMPLATE";

      #ifdef FEATURE_QCRIL_IMS
      case QCRIL_EVT_SMS_TRANSPORT_REG:
        return "WMS_MSG_EVENT_TRANSPORT_REG";
      #endif /* FEATURE_QCRIL_IMS */

      #ifdef FEATURE_QCRIL_WMS_ETWS
       case QCRIL_EVT_SMS_ETWS_NOTIFICATION:
        return "QCRIL_EVT_SMS_ETWS_NOTIFICATION";
      #endif /* FEATURE_QCRIL_WMS_ETWS */

      #ifndef FEATURE_QCRIL_UIM_QMI
      case QCRIL_EVT_MMGSDI_COMMAND_CALLBACK:
        return "MMGSDI_COMMAND_CALLBACK";

      case QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK:
        return "MMGSDI_IMSI_COMMAND_CALLBACK";

      case QCRIL_EVT_MMGSDI_EVENT_CALLBACK:
        return "MMGSDI_EVENT_CALLBACK";

      case QCRIL_EVT_MMGSDI_GSDI_COMMAND_CALLBACK:
        return "MMGSDI_GSDI_COMMAND_CALLBACK";

      case QCRIL_EVT_MMGSDI_PERSO_EVENT_CALLBACK:
        return "MMGSDI_PERSO_EVENT_CALLBACK";

      case QCRIL_EVT_MMGSDI_INTERNAL_VERIFY_PIN_COMMAND_CALLBACK:
        return "INTERNAL_MMGSDI_VERIFY_PIN_COMMAND_CALLBACK";
      #else
      case QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK:
        return "UIM_QMI_COMMAND_CALLBACK";

      case QCRIL_EVT_UIM_QMI_INDICATION:
        return "UIM_QMI_INDICATION";

      case QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK:
        return "INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK";
      #endif /* FEATURE_QCRIL_UIM_QMI */

      case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP:
        return "INTERNAL_MMGSDI_CARD_POWER_UP";

      case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN:
        return "INTERNAL_MMGSDI_CARD_POWER_DOWN";

      case QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS:
        return "INTERNAL_MMGSDI_GET_FDN_STATUS";

      case QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS:
        return "INTERNAL_MMGSDI_SET_FDN_STATUS";

      case QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS:
        return "INTERNAL_MMGSDI_GET_PIN1_STATUS";

      case QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS:
        return "INTERNAL_MMGSDI_SET_PIN1_STATUS";

      case QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE:
        return "INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE";

      case QCRIL_EVT_INTERNAL_MMGSDI_READ_UST_VALUE:
        return "INTERNAL_MMGSDI_READ_UST_VALUE";

      case QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS:
        return "QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS";

      case QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS:
        return "QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS";     
  
      #ifdef FEATURE_QCRIL_QMI_CAT
      case QCRIL_EVT_GSTK_QMI_CAT_INDICATION:
        return "QCRIL_EVT_GSTK_QMI_CAT_INDICATION";

      case QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY:
        return "QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY";

      case QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK:
        return "QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK";

      case QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK:
        return "QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK";
      #else
      case QCRIL_EVT_GSTK_COMMAND_CALLBACK:
        return "GSTK_COMMAND_CALLBACK";

      case QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK:
        return "GSTK_CLIENT_INIT_CALLBACK";

      case QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK:
        return "GSTK_CLIENT_REG_CALLBACK";

      case QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK:
        return "GSTK_RAW_COMMAND_CALLBACK";

      case QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK:
        return "GSTK_SEND_RAW_ENVELOPE_CALLBACK";

      case QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY:
        return "INTERNAL_GSTK_NOTIFY_RIL_IS_READY";
      #endif /* FEATURE_QCRIL_QMI_CAT */

      case QCRIL_EVT_DATA_COMMAND_CALLBACK:
        return "DATA_COMMAND_CALLBACK";

      case QCRIL_EVT_DATA_EVENT_CALLBACK:
        return "DATA_EVENT_CALLBACK";

      case QCRIL_EVT_DATA_WDS_EVENT_CALLBACK:
        return "DATA_WDS_EVENT_CALLBACK";

      case QCRIL_EVT_PBM_REFRESH_START:
        return "PBM_REFRESH_START";

      case QCRIL_EVT_PBM_REFRESH_DONE:
        return "PBM_REFRESH_DONE";

      case QCRIL_EVT_PBM_SIM_INIT_DONE:
        return "PBM_SIM_INIT_DONE";

      #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
      case QCRIL_EVT_PBM_SESSION_INIT_DONE:
        return "PBM_SESSION_INIT_DONE";
      #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */

      case QCRIL_EVT_PBM_PB_READY:
        return "PBM_PB_READY";

      case QCRIL_EVT_PBM_CARD_INSERTED:
        return "PBM_CARD_INSERTED";

      case QCRIL_EVT_PBM_CARD_INIT_COMPLETED:
        return "PBM_CARD_INIT_COMPLETED";

      case QCRIL_EVT_PBM_CARD_ERROR:
        return "PBM_CARD_ERROR";

      case QCRIL_EVT_PBM_UPDATE_OTA_ECC_LIST:
        return "PBM_UPDATE_OTA_ECC_LIST";

      case QCRIL_EVT_HOOK_NV_READ:
        return "RIL_REQUEST_OEM_HOOK_RAW(NV_READ)";

      case QCRIL_EVT_HOOK_NV_WRITE:
        return "RIL_REQUEST_OEM_HOOK_RAW(NV_WRITE)";

      case QCRIL_EVT_HOOK_ME_DEPERSONALIZATION:
        return "RIL_REQUEST_OEM_HOOK_RAW(ME_DEPERSONALIZATION)";

      case QCRIL_EVT_HOOK_SET_TUNE_AWAY:
        return "RIL_REQUEST_OEM_HOOK_RAW(SET_TUNE_AWAY)";

      case QCRIL_EVT_HOOK_GET_TUNE_AWAY:
        return "RIL_REQUEST_OEM_HOOK_RAW(GET_TUNE_AWAY)";

      case QCRIL_EVT_HOOK_SET_PAGING_PRIORITY:
        return "RIL_REQUEST_OEM_HOOK_RAW(SET_PAGING_PRIORITY)";

      case QCRIL_EVT_HOOK_GET_PAGING_PRIORITY:
        return "RIL_REQUEST_OEM_HOOK_RAW(GET_PAGING_PRIORITY)";

      case QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_START:
        return "RIL_UNSOL_OEM_HOOK_RAW(CDMA_CONT_FWD_DTMF_START)";

      case QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_STOP:
        return "RIL_UNSOL_OEM_HOOK_RAW(CDMA_CONT_FWD_DTMF_STOP)";

      case QCRIL_EVT_HOOK_UNSOL_CDMA_BURST_DTMF:
        return "RIL_UNSOL_OEM_HOOK_RAW(CDMA_CONT_FWD_DTMF_BURST)";

      case QCRIL_EVT_HOOK_UNSOL_CALL_EVT_PROGRESS_INFO_IND:   
        return "RIL_UNSOL_OEM_HOOK_RAW(CALL_EVT_PROGRESS_INFO_IND)";  

      default:
        return "<Unknown event> ?";
    } /* end switch */
  }

} /* qcril_log_lookup_event_name */


/*===========================================================================

  FUNCTION: QCRIL_LOG_LOOKUP_ERRNO_NAME

===========================================================================*/
/*!
    @brief
    Lookup the name of a QCRIL errno error

    @return
    The string respresenting the name of a QCRIL errno error
*/
/*=========================================================================*/
const char *qcril_log_lookup_errno_name
(
  int errno_id
)
{
  if ( errno_id < ( int ) QCRIL_ARR_SIZE( qcril_errno_name ) )
  {
    return qcril_errno_name[ errno_id ];
  }
  else
  {
    return "<RIL_ERRNO_UNKNOWN> ?";
  }
} /* qcril_log_lookup_ril_event_index */

/*===========================================================================

  FUNCTION: qcril_log_lookup_ril_event_index

===========================================================================*/
/*!
    @brief
    Lookup the index of  the given event in qcril_request_name[]

    @return
    Index of the event in data
*/
/*=========================================================================*/
int qcril_log_lookup_ril_event_index
(
  int event,
  qcril_qmi_event_log_type *data,
  int min_index,
  int max_index
)
{
   int min = min_index, max = max_index, index;

   while( min <= max )
   {
      index = (min+max)/2;

      if( data[index].event  == event )
      {
        return index;
      }
      else if( data[index].event  < event )
      {
	  min = index + 1;
      }
      else 
      {
        max = index - 1;
      }
   }

   // last element which is unkown event
   return max_index;
} /* qcril_log_lookup_errno_name */
