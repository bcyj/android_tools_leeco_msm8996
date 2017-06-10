/******************************************************************************
  @file    qcril_log.c
  @brief   qcril qmi - logging utilities

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2008-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#define QCRIL_IS_LOG_TO_FILE_ENABLED         "persist.radio.ril_log_enabled"
#define QCRIL_IS_LOG_TO_FILE_INTERVAL        "persist.radio.ril_log_interval"
#define QCRIL_IS_LOG_TO_FILE_DEF_INTERVAL    (5)
#define QCRIL_LOG_FILE                       "/data/misc/radio/ril_log"

#ifdef QMI_RIL_UTF
#define QCRIL_IPC_RILD_SOCKET_PATH_PREFIX    "./rild_sync_"
#else
#define QCRIL_IPC_RILD_SOCKET_PATH_PREFIX    "/dev/socket/qmux_radio/rild_sync_"
#endif
#define QCRIL_IPC_MAX_SOCKET_PATH_LENGTH     48
#define QCRIL_IPC_BIND_RETRY_MAX_ATTEMPTS    5
#ifdef QMI_RIL_UTF
#include <netinet/in.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#else
#include <utils/Log.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <cutils/properties.h>
#include <pthread.h>
#include "qcril_log.h"
#include "qcril_am.h"
#include <sys/un.h>

// Required for glibc compile
#include <limits.h>
#include <signal.h>

#ifdef LOG_TAG
#undef LOG_TAG // It might be defined differently in diag header files
#endif
#define LOG_TAG "RILQ"

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/



/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

FILE *rild_fp;
char log_buf[ QCRIL_MAX_LOG_MSG_SIZE ];
char log_fmt[ QCRIL_MAX_LOG_MSG_SIZE ];
char thread_name[ QMI_RIL_THREAD_NAME_MAX_SIZE ];
#ifdef QMI_RIL_UTF
char log_buf_raw[ QCRIL_MAX_LOG_MSG_SIZE ];
#endif
pthread_mutex_t log_lock_mutex;
pthread_mutex_t log_timer_mutex;
boolean diag_init_complete = FALSE;

/* Flag that controls whether QCRIL debug messages logged on ADB or not */
boolean qcril_log_adb_on = FALSE;

/* Flag that controls whether QCRIL message payload logged on QXDM log or not */
boolean qcril_log_ril_msg_payload_log_on = FALSE;

static pid_t qcril_qmi_instance_log_id;

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
  { RIL_REQUEST_ISIM_AUTHENTICATION,                  "RIL_REQUEST_ISIM_AUTHENTICATION" },
  { RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU,"RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU" },
  { RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS,        "RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS" },
  { RIL_REQUEST_VOICE_RADIO_TECH,                     "RIL_REQUEST_VOICE_RADIO_TECH" },
  { RIL_REQUEST_GET_CELL_INFO_LIST,                   "RIL_REQUEST_GET_CELL_INFO_LIST" },
  { RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE,        "RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE" },
  /* 111 */
  { RIL_REQUEST_SET_INITIAL_ATTACH_APN,               "RIL_REQUEST_SET_INITIAL_ATTACH_APN" },
  { RIL_REQUEST_IMS_REGISTRATION_STATE,               "RIL_REQUEST_IMS_REGISTRATION_STATE" },
  { RIL_REQUEST_IMS_SEND_SMS,                         "RIL_REQUEST_IMS_SEND_SMS" },
  { RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC,              "RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC" },
  { RIL_REQUEST_SIM_OPEN_CHANNEL,                     "RIL_REQUEST_SIM_OPEN_CHANNEL" },
  { RIL_REQUEST_SIM_CLOSE_CHANNEL,                    "RIL_REQUEST_SIM_CLOSE_CHANNEL" },
  { RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL,            "RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL" },
  { RIL_REQUEST_NV_READ_ITEM,                         "RIL_REQUEST_NV_READ_ITEM" },
  { RIL_REQUEST_NV_WRITE_ITEM,                        "RIL_REQUEST_NV_WRITE_ITEM" },
  { RIL_REQUEST_NV_WRITE_CDMA_PRL,                    "RIL_REQUEST_NV_WRITE_CDMA_PRL" },
  /* 121 */
  { RIL_REQUEST_NV_RESET_CONFIG,                      "RIL_REQUEST_NV_RESET_CONFIG" },
  { RIL_REQUEST_SET_UICC_SUBSCRIPTION,                "RIL_REQUEST_SET_UICC_SUBSCRIPTION" },
  { RIL_REQUEST_ALLOW_DATA,                           "RIL_REQUEST_ALLOW_DATA" },
  { RIL_REQUEST_GET_HARDWARE_CONFIG,                  "RIL_REQUEST_GET_HARDWARE_CONFIG" },
#ifdef RIL_REQUEST_SIM_AUTHENTICATION
  { RIL_REQUEST_SIM_AUTHENTICATION,                   "RIL_REQUEST_SIM_AUTHENTICATION" },
#endif /* RIL_REQUEST_SIM_AUTHENTICATION */
  { RIL_REQUEST_GET_DC_RT_INFO,                       "RIL_REQUEST_GET_DC_RT_INFO" },
  { RIL_REQUEST_SET_DC_RT_INFO_RATE,                  "RIL_REQUEST_SET_DC_RT_INFO_RATE" },
  { RIL_REQUEST_SET_DATA_PROFILE,                     "RIL_REQUEST_SET_DATA_PROFILE" },
  { RIL_REQUEST_SHUTDOWN,                             "RIL_REQUEST_SHUTDOWN" },
  { RIL_REQUEST_GET_DATA_CALL_PROFILE,                "RIL_REQUEST_GET_DATA_CALL_PROFILE" },
  /* 131 */

#ifdef RIL_REQUEST_SIM_GET_ATR
  { RIL_REQUEST_SIM_GET_ATR,                          "RIL_REQUEST_SIM_GET_ATR" },
#endif

  /* 10002 */
  { RIL_REQUEST_MODIFY_CALL_INITIATE,                 "RIL_REQUEST_MODIFY_CALL_INITIATE" },
  { RIL_REQUEST_MODIFY_CALL_CONFIRM,                  "RIL_REQUEST_MODIFY_CALL_CONFIRM" },
#if (RIL_QCOM_VERSION >= 2)
  { RIL_REQUEST_SETUP_QOS,                            "RIL_REQUEST_SETUP_QOS" },
  { RIL_REQUEST_RELEASE_QOS,                          "RIL_REQUEST_RELEASE_QOS" },
  { RIL_REQUEST_GET_QOS_STATUS,                       "RIL_REQUEST_GET_QOS_STATUS" },
  { RIL_REQUEST_MODIFY_QOS,                           "RIL_REQUEST_MODIFY_QOS" },
  { RIL_REQUEST_SUSPEND_QOS,                          "RIL_REQUEST_SUSPEND_QOS" },
  { RIL_REQUEST_RESUME_QOS,                           "RIL_REQUEST_RESUME_QOS" },
#endif
  /* 10110 */
  { RIL_REQUEST_SET_DATA_SUBSCRIPTION,                "RIL_REQUEST_SET_DATA_SUBSCRIPTION" },
  { RIL_REQUEST_GET_UICC_SUBSCRIPTION,                "RIL_REQUEST_GET_UICC_SUBSCRIPTION" },
  { RIL_REQUEST_GET_DATA_SUBSCRIPTION,                "RIL_REQUEST_GET_DATA_SUBSCRIPTION" },
  { RIL_REQUEST_SET_SUBSCRIPTION_MODE,                "RIL_REQUEST_SET_SUBSCRIPTION_MODE" },
  { RIL_REQUEST_UNKOWN,                               "RIL_REQUEST_UNKOWN" },
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
  { RIL_UNSOL_CELL_INFO_LIST,                         "RIL_UNSOL_CELL_INFO_LIST"},
  { RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,     "RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED" },
  { RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED,       "RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED" },
  { RIL_UNSOL_SRVCC_STATE_NOTIFY,                     "RIL_UNSOL_SRVCC_STATE_NOTIFY" },
  { RIL_UNSOL_HARDWARE_CONFIG_CHANGED,                "RIL_UNSOL_HARDWARE_CONFIG_CHANGED" },
  /* 1041 */
  { RIL_UNSOL_DC_RT_INFO_CHANGED,                     "RIL_UNSOL_DC_RT_INFO_CHANGED" },
  { RIL_UNSOL_ON_SS,                                  "RIL_UNSOL_ON_SS" },
  { RIL_UNSOL_STK_CC_ALPHA_NOTIFY,                    "RIL_UNSOL_STK_CC_ALPHA_NOTIFY" },
#if (RIL_QCOM_VERSION >= 2)
  { RIL_UNSOL_QOS_STATE_CHANGED_IND,                  "RIL_UNSOL_QOS_STATE_CHANGED_IND" },
#endif
  { RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED,   "RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED" },
  { RIL_UNSOL_MODIFY_CALL,                            "RIL_UNSOL_MODIFY_CALL" },
  { RIL_UNSOL_UNKOWN,                                 "RIL_UNSOL_UNKOWN"}
 };

typedef struct
{
 qcril_instance_id_e_type rild_instance_id;
 size_t rild_addrlen;
 struct sockaddr_un rild_addr;
}other_rild_addr_info_type;

static struct inter_rild_info_type
{
  int info_valid;
  int my_sockid;
  other_rild_addr_info_type *other_rilds_addr_info;
  int other_rilds_addr_info_len;
  pthread_t recv_thread_id;
  pthread_attr_t recv_thread_attr;
  pthread_mutex_t send_lock_mutex;
}inter_rild_info;

typedef struct ipc_send_recv_data_info
{
  qcril_instance_id_e_type rild_instance_id;
  ipc_message_id_type message_id;
  char payload[QCRIL_MAX_IPC_PAYLOAD_SIZE];
  int payload_length;
}ipc_send_recv_data_info_type;

typedef struct
{
    int is_valid;
    pthread_t thread_id;
    char thread_name[QMI_RIL_THREAD_NAME_MAX_SIZE];
} qmi_ril_thread_name_info_type;

#ifndef QMI_RIL_UTF
static qmi_ril_thread_name_info_type qmi_ril_thread_name_info[QMI_RIL_THREAD_INFO_MAX_SIZE];
#else
qmi_ril_thread_name_info_type qmi_ril_thread_name_info[QMI_RIL_THREAD_INFO_MAX_SIZE];
#endif

static int qmi_ril_log_enabled;
static int qmi_ril_log_timer_interval;
static uint32 qmi_ril_log_timer_id;

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES


===========================================================================*/

static void* qcril_multiple_rild_ipc_recv_func(void *arg);
static void qcril_multiple_rild_ipc_signal_handler_sigusr1(int arg);
static int qcril_ipc_evaluate_rilds_socket_paths(char *rild_socket_name);
static int qcril_log_lookup_ril_event_index
(
  int event,
  qcril_qmi_event_log_type *data,
  int min_index,
  int max_index
);


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

  qcril_qmi_instance_log_id = getpid();

  ret_val = 0;
  remove(QCRIL_LOG_FILE);
  property_get( QCRIL_IS_LOG_TO_FILE_ENABLED, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
    #ifndef QMI_RIL_UTF
      RLOGE( "Fail to convert ril_log_enabled setting %s", args );
    #endif
    }
    else if (ret_val)
    {
        rild_fp = fopen(QCRIL_LOG_FILE, "w");
        if ( !rild_fp )
        {
      #ifndef QMI_RIL_UTF
          RLOGE( "Fail to create %s for QCRIL logging\n",QCRIL_LOG_FILE );
      #endif
        }
        else
        {
             setvbuf ( rild_fp , NULL , _IOFBF , 1024 );
      #ifndef QMI_RIL_UTF
            RLOGE( "%s for QCRIL logging created successfully\n",QCRIL_LOG_FILE );
      #endif
            qmi_ril_log_enabled = 1;
        }
    }
  }

  if(!ret_val)
  {
      rild_fp = NULL;
    #ifndef QMI_RIL_UTF
      RLOGE( "log to %s for QCRIL logging is not enabled\n",QCRIL_LOG_FILE );
    #endif
  }

  /* Initialize Diag for QCRIL logging */
  ret_val = Diag_LSM_Init(NULL);
  if ( !ret_val )
  {
#ifndef QMI_RIL_UTF
    RLOGE( "Fail to initialize Diag for QCRIL logging\n" );
#endif
  }
  else
  {
    diag_init_complete = TRUE;
  }

  QCRIL_LOG_DEBUG ( "qcril_log_init() 1" );

  property_get( QCRIL_LOG_ADB_ON, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QCRIL_LOG_ERROR( "Fail to convert adb_log_on setting %s", args );
    }
    else if ( ret_val > 1 )
    {
      QCRIL_LOG_ERROR( "Invalid saved adb_log_on setting %ld, use default", ret_val );
    }
    else
    {
      qcril_log_adb_on = ( boolean ) ret_val;
    }
  }

  #ifdef FEATURE_QCRIL_ADB_LOG_ON
  qcril_log_adb_enabled = TRUE;
  #endif /* FEATURE_QCRIL_ADB_LOG_ON */

  QCRIL_LOG_DEBUG ( "qcril_log_init() 2" );

  QCRIL_LOG_DEBUG( "adb_log_on = %d", (int) qcril_log_adb_on );

  /* Save ADB Log Enabled setting to system property */
  QCRIL_SNPRINTF( args, sizeof( args ), "%d", (int) qcril_log_adb_on );
  if ( property_set( QCRIL_LOG_ADB_ON, args ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "Fail to save %s to system property", QCRIL_LOG_ADB_ON );
  }


  /* Fetching RIL payload dumping property */
  property_get( QCRIL_LOG_RIL_PAYLOAD_LOG_ON, args, "" );
  len = strlen( args );
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QCRIL_LOG_ERROR( "Fail to convert ril_msg_payload_on setting %s\n", args );
    }
    else if ( ret_val > 1 )
    {
      QCRIL_LOG_ERROR( "Invalid saved ril_msg_payload_on setting %ld, use default\n", ret_val );
    }
    else
    {
      qcril_log_ril_msg_payload_log_on = ( boolean ) ret_val;
    }
  }

  QCRIL_LOG_DEBUG ( "qcril_log_init() 3" );

  QCRIL_LOG_DEBUG( "ril_msg_payload_on = %d", (int) qcril_log_ril_msg_payload_log_on );

  /* Save RIL Payload Log Enabled setting to system property */
  QCRIL_SNPRINTF( args, sizeof( args ), "%d", qcril_log_ril_msg_payload_log_on );
  if ( property_set( QCRIL_LOG_RIL_PAYLOAD_LOG_ON, args ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "Fail to save %s to system property\n", QCRIL_LOG_RIL_PAYLOAD_LOG_ON );
  }

  pthread_mutexattr_t mtx_atr;
  pthread_mutexattr_init(&mtx_atr);
  pthread_mutex_init(&log_lock_mutex, &mtx_atr);
  pthread_mutex_init(&log_timer_mutex, &mtx_atr);

} /* qcril_log_init */


void qcril_log_cancel_log_timer()
{
    pthread_mutex_lock(&log_timer_mutex);
    if(qmi_ril_log_enabled && qmi_ril_log_timer_id)
    {
        qcril_cancel_timed_callback((void *)(uintptr_t)qmi_ril_log_timer_id);
        qmi_ril_log_timer_id = QMI_RIL_ZERO;
    }
    pthread_mutex_unlock(&log_timer_mutex);
}

void qcril_log_timer_expiry_cb(void * params)
{
    struct timeval tv;
    struct timespec ts;
    struct tm* tm = NULL;
    char time_string[40];
    long milliseconds;

    if(qmi_ril_is_feature_supported(QMI_RIL_FEATURE_POSIX_CLOCKS))
    {
        clock_gettime(CLOCK_MONOTONIC,
                      &ts);
        tv.tv_sec = ts.tv_sec;
        tv.tv_usec = ts.tv_nsec/1000;
    }
    else
    {
        gettimeofday(&tv,
                     NULL);
    }
    tm = localtime (&tv.tv_sec);

    if(NULL != tm)
    {
        strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", tm);
        milliseconds = tv.tv_usec / 1000;

        pthread_mutex_lock(&log_timer_mutex);
        qmi_ril_log_timer_id = QMI_RIL_ZERO;

        pthread_mutex_lock(&log_lock_mutex);
        if (rild_fp)
        {
            fflush(rild_fp);
            fprintf (rild_fp, "Timestamp : %s.%03ld\n", time_string, milliseconds);
        }
        pthread_mutex_unlock(&log_lock_mutex);

        qcril_log_timer_setup();
        pthread_mutex_unlock(&log_timer_mutex);
    }
}

void qcril_log_timer_setup( void )
{
    struct timeval log_timeout;
    struct timeval *log_timeout_ptr;

    log_timeout_ptr = NULL;

    if( qmi_ril_log_timer_interval )
    {
        log_timeout.tv_sec = qmi_ril_log_timer_interval;
        log_timeout.tv_usec = QMI_RIL_ZERO;
        log_timeout_ptr = &log_timeout;
    }
    qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                                qcril_log_timer_expiry_cb,
                                log_timeout_ptr, &qmi_ril_log_timer_id);
}

/*=========================================================================
  FUNCTION:  qcril_log_timer_init

===========================================================================*/
/*!
    @brief
    Initialization for log timer.

    @return
    None
*/
/*=========================================================================*/
void qcril_log_timer_init( void )
{
    char args[ PROPERTY_VALUE_MAX ];
    int len;
    char *end_ptr;
    unsigned long ret_val;

    pthread_mutex_lock(&log_timer_mutex);
    if( qmi_ril_log_enabled )
    {
        ret_val = QCRIL_IS_LOG_TO_FILE_DEF_INTERVAL;
        property_get( QCRIL_IS_LOG_TO_FILE_INTERVAL, args, "" );
        len = strlen( args );
        if ( len > 0 )
        {
          ret_val = strtoul( args, &end_ptr, 0 );
          if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
          {
            ret_val = QCRIL_IS_LOG_TO_FILE_DEF_INTERVAL;
          #ifndef QMI_RIL_UTF
            RLOGE( "Fail to convert ril_log_interval setting %s", args );
          #endif
          }
        }
        qmi_ril_log_timer_interval = ret_val;
        #ifndef QMI_RIL_UTF
        RLOGE( "using %d for qmi_ril_log_timer_interval", qmi_ril_log_timer_interval);
        #endif
        qcril_log_timer_setup();
    }
    pthread_mutex_unlock(&log_timer_mutex);

} /* qcril_log_timer_init */


/*=========================================================================
  FUNCTION:  qcril_log_cleanup

===========================================================================*/
/*!
    @brief
    Cleanup for logging.

    @return
    None
*/
/*=========================================================================*/
void qcril_log_cleanup
(
  void
)
{
    pthread_mutex_lock(&log_lock_mutex);
    if (rild_fp)
    {
        fclose(rild_fp);
        rild_fp = NULL;
    }
    pthread_mutex_unlock(&log_lock_mutex);

} /* qcril_log_cleanup */


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


  va_start( ap, fmt );

  if ( NULL != buf_ptr && buf_size > 0 )
  {
      vsnprintf( buf_ptr, buf_size, fmt, ap );
  }

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
  #if !defined(FEATURE_UNIT_TEST) && !defined(QMI_RIL_UTF)
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

#ifndef QMI_RIL_UTF
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
  switch ( lvl )
  {
      case MSG_LEGACY_ERROR:
      case MSG_LEGACY_FATAL:
#ifndef QMI_RIL_UTF
        RLOGE( "(%d/%d): %s",
               (int)qmi_ril_get_process_instance_id(),
               (int)qcril_qmi_instance_log_id,
               msg_ptr );
#endif
        break;

      case MSG_LEGACY_HIGH:                         // fall through
        RLOGW_IF( qcril_log_adb_on, "(%d/%d): %s",
                  (int)qmi_ril_get_process_instance_id(),
                  (int)qcril_qmi_instance_log_id,
                  msg_ptr );
        break;

      case MSG_LEGACY_ESSENTIAL:
        RLOGI( "(%d/%d):%s",
               (int)qmi_ril_get_process_instance_id(),
               (int)qcril_qmi_instance_log_id,
                msg_ptr );
        break;

      case MSG_LEGACY_MED:
        RLOGI_IF( qcril_log_adb_on, "(%d/%d): %s",
                  (int)qmi_ril_get_process_instance_id(),
                  (int)qcril_qmi_instance_log_id,
                  msg_ptr );
        break;

      default:
        RLOGV_IF( qcril_log_adb_on, "(%d/%d): %s",
                  (int)qmi_ril_get_process_instance_id(),
                  (int)qcril_qmi_instance_log_id,
                  msg_ptr );
        break;
  }

} /* qcril_log_msg_to_adb */

#endif
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
     index = qcril_log_lookup_ril_event_index( event_id, (qcril_qmi_event_log_type *)qcril_request_name, 0, sizeof(qcril_request_name)/sizeof(qcril_request_name[0]) - 1);
     return qcril_request_name[ index ].event_name;
  }
  /* Lookup the name of a RIL unsolicited response */
  else if ( event_id < (int) QCRIL_EVT_BASE )
  {
     index = qcril_log_lookup_ril_event_index( event_id, (qcril_qmi_event_log_type *)qcril_unsol_response_name, 0, sizeof(qcril_unsol_response_name)/sizeof(qcril_unsol_response_name[0]) - 1);
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

      case QCRIL_EVT_CM_UPDATE_FDN_STATUS:
        return "CM_UPDATE_FDN_STATUS";

      case QCRIL_EVT_CM_CARD_APP_STATUS_CHANGED:
        return "QCRIL_EVT_CM_CARD_APP_STATUS_CHANGED";

      case QCRIL_EVT_CM_CARD_STATUS_UPDATED:
        return "CM_CARD_STATUS_UPDATED";

      case QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS:
        return "CM_ACTIVATE_PROVISION_STATUS";

      case QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS:
        return "CM_DEACTIVATE_PROVISION_STATUS";


      case QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK:
        return "UIM_QMI_COMMAND_CALLBACK";

      case QCRIL_EVT_UIM_MCC_MNC_INFO:
        return "QCRIL_EVT_UIM_MCC_MNC_INFO";

      case QCRIL_EVT_UIM_QMI_INDICATION:
        return "UIM_QMI_INDICATION";

      case QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK:
        return "INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK";

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

      case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START:
        return "QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START";

      case QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE:
        return "QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE";

      case QCRIL_EVT_INTERNAL_UIM_SAP_RESP:
        return "QCRIL_EVT_INTERNAL_UIM_SAP_RESP";

      case QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC:
        return "QCRIL_EVT_INTERNAL_UIM_GET_MCC_MNC";

       case QCRIL_EVT_UIM_RMT_QMI_COMMAND_CALLBACK:
        return "QCRIL_EVT_UIM_RMT_QMI_COMMAND_CALLBACK";

      case QCRIL_EVT_UIM_RMT_QMI_INDICATION_CALLBACK:
        return "QCRIL_EVT_UIM_RMT_QMI_INDICATION_CALLBACK";

      case QCRIL_EVT_GSTK_QMI_CAT_INDICATION:
        return "QCRIL_EVT_GSTK_QMI_CAT_INDICATION";

      case QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY:
        return "QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY";

      case QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK:
        return "QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK";

      case QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK:
        return "QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK";

      case QCRIL_EVT_GSTK_QMI_NOTIFY_CARD_ERROR:
        return "QCRIL_EVT_GSTK_QMI_NOTIFY_CARD_ERROR";

      case QCRIL_EVT_DATA_COMMAND_CALLBACK:
        return "DATA_COMMAND_CALLBACK";

      case QCRIL_EVT_DATA_EVENT_CALLBACK:
        return "DATA_EVENT_CALLBACK";

      case QCRIL_EVT_DATA_WDS_EVENT_CALLBACK:
        return "QCRIL_EVT_DATA_WDS_EVENT_CALLBACK";

      case QCRIL_EVT_DATA_DSD_EVENT_CALLBACK:
        return "QCRIL_EVT_DATA_DSD_EVENT_CALLBACK";

      case QCRIL_EVT_HOOK_NV_READ:
        return "RIL_REQUEST_OEM_HOOK_RAW(NV_READ)";

      case QCRIL_EVT_HOOK_CSG_PERFORM_NW_SCAN:
        return "RIL_REQUEST_OEM_HOOK_RAW(CSG_PERFORM_NW_SCAN)";

      case QCRIL_EVT_HOOK_CSG_SET_SYS_SEL_PREF:
        return "RIL_REQUEST_OEM_HOOK_RAW(CSG_SET_SYS_SEL_PREF)";

      case QCRIL_EVT_HOOK_CSG_GET_SYS_INFO:
        return "RIL_REQUEST_OEM_HOOK_RAW(CSG_GET_SYS_INFO)";

      case QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE:
        return "RIL_REQUEST_OEM_HOOK_RAW(QCRIL_EVT_HOOK_ENABLE_ENGINEER_MODE)";

      case QCRIL_EVT_HOOK_UNSOL_ENGINEER_MODE:
        return "QCRIL_EVT_HOOK_UNSOL_ENGINEER_MODE";

      case QCRIL_EVT_HOOK_NV_WRITE:
        return "RIL_REQUEST_OEM_HOOK_RAW(NV_WRITE)";

      case QCRIL_EVT_HOOK_ME_DEPERSONALIZATION:
        return "RIL_REQUEST_OEM_HOOK_RAW(ME_DEPERSONALIZATION)";

      case QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION:
        return "QCRIL_EVT_HOOK_SET_DATA_SUBSCRIPTION";

      case QCRIL_EVT_HOOK_SET_TUNE_AWAY:
        return "RIL_REQUEST_OEM_HOOK_RAW(SET_TUNE_AWAY)";

      case QCRIL_EVT_HOOK_GET_TUNE_AWAY:
        return "RIL_REQUEST_OEM_HOOK_RAW(GET_TUNE_AWAY)";

      case QCRIL_EVT_HOOK_SET_PAGING_PRIORITY:
        return "RIL_REQUEST_OEM_HOOK_RAW(SET_PAGING_PRIORITY)";

      case QCRIL_EVT_HOOK_GET_PAGING_PRIORITY:
        return "RIL_REQUEST_OEM_HOOK_RAW(GET_PAGING_PRIORITY)";

      case QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB:
        return "QCRIL_EVT_HOOK_SET_DEFAULT_VOICE_SUB";

      case QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD:
        return "QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD";

      case QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY:
        return "QCRIL_EVT_HOOK_GET_MODEM_CAPABILITY";

      case QCRIL_EVT_HOOK_UPDATE_SUB_BINDING:
        return "QCRIL_EVT_HOOK_UPDATE_SUB_BINDING";

      case QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY:
        return "QCRIL_EVT_HOOK_SET_LTE_TUNE_AWAY";

      case QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST:
        return "QCRIL_EVT_HOOK_SET_BUILTIN_PLMN_LIST";

      case QCRIL_EVT_HOOK_REJECT_INCOMING_CALL_CAUSE_21:
        return "QCRIL_EVT_HOOK_REJECT_INCOMING_CALL_CAUSE_21";

      case QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_START:
        return "RIL_UNSOL_OEM_HOOK_RAW(CDMA_CONT_FWD_DTMF_START)";

      case QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_STOP:
        return "RIL_UNSOL_OEM_HOOK_RAW(CDMA_CONT_FWD_DTMF_STOP)";

      case QCRIL_EVT_HOOK_UNSOL_CDMA_BURST_DTMF:
        return "RIL_UNSOL_OEM_HOOK_RAW(CDMA_CONT_FWD_DTMF_BURST)";

      case QCRIL_EVT_HOOK_UNSOL_CALL_EVT_PROGRESS_INFO_IND:
        return "RIL_UNSOL_OEM_HOOK_RAW(CALL_EVT_PROGRESS_INFO_IND)";

      case QCRIL_EVT_HOOK_UNSOL_NSS_RELEASE:
        return "QCRIL_EVT_HOOK_UNSOL_NSS_RELEASE";

      case QCRIL_EVT_HOOK_NEIGHBOR_CELL_INFO_RCVD:
        return "QCRIL_EVT_HOOK_NEIGHBOR_CELL_INFO_RCVD";

      case QCRIL_EVT_HOOK_UNSOL_EUTRA_STATUS:
        return "QCRIL_EVT_HOOK_UNSOL_EUTRA_STATUS";

      case QCRIL_EVT_HOOK_UNSOL_VOICE_SYSTEM_ID:
        return "QCRIL_EVT_HOOK_UNSOL_VOICE_SYSTEM_ID";

      case QCRIL_EVT_HOOK_UNSOL_MODEM_CAPABILITY:
        return "QCRIL_EVT_HOOK_UNSOL_MODEM_CAPABILITY";

      case QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN:
        return "QCRIL_EVT_HOOK_PERFORM_INCREMENTAL_NW_SCAN";

      case QCRIL_EVT_HOOK_UNSOL_INCREMENTAL_NW_SCAN_IND:
        return "QCRIL_EVT_HOOK_UNSOL_INCREMENTAL_NW_SCAN_IND";

      case QCRIL_EVT_QMI_REQUEST_NW_SCAN:
        return "REQUEST_NW_SCAN";

      case QCRIL_EVT_QMI_REQUEST_NW_SELECT:
        return "REQUEST_NW_SELECT";

      case QCRIL_EVT_QMI_REQUEST_POWER_RADIO:
        return "REQUEST_POWER_RADIO";

      case QCRIL_EVT_QMI_REQUEST_SHUTDOWN:
        return "REQUEST_SHUTDOWN";

      case QCRIL_EVT_QMI_RIL_COMMON_IND_SUBSCRIBE_CONSIDER_ACTION:
        return "QCRIL_EVT_QMI_RIL_COMMON_IND_SUBSCRIBE_CONSIDER_ACTION";

      case QCRIL_EVT_QMI_REQUEST_NEIGHBOR_CELL_INFO:
        return "QCRIL_EVT_QMI_REQUEST_NEIGHBOR_CELL_INFO";

      case QCRIL_EVT_QMI_NAS_CLEANUP_NW_SEL:
        return "QCRIL_EVT_QMI_NAS_CLEANUP_NW_SEL";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_REQ:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_REQ";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_SUSPEND_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_SUSPEND_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_SUSPEND_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_SUSPEND_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_PRE_RESUME_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_DATA_RESUME_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_UIM_RESUME_CON";

      case QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_CON:
        return "QCRIL_EVT_QMI_RIL_MODEM_RESTART_RIL_CORE_FINAL_RESUME_CON";

      case QCRIL_EVT_HOOK_EMBMS_ENABLE:
        return "QCRIL_EVT_HOOK_EMBMS_ENABLE";

      case QCRIL_EVT_HOOK_EMBMS_DISABLE:
          return "QCRIL_EVT_HOOK_EMBMS_DISABLE";

      case QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_ACTIVATE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_DEACTIVATE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_RSSI_IND:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_RSSI_IND";

      case QCRIL_EVT_HOOK_EMBMS_GET_AVAILABLE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_GET_AVAILABLE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_GET_ACTIVE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_ENABLE_RSSI:
          return "QCRIL_EVT_HOOK_EMBMS_ENABLE_RSSI";

      case QCRIL_EVT_HOOK_EMBMS_DISABLE_RSSI:
          return "QCRIL_EVT_HOOK_EMBMS_DISABLE_RSSI";

      case QCRIL_EVT_HOOK_EMBMS_GET_COVERAGE_STATE:
          return "QCRIL_EVT_HOOK_EMBMS_GET_COVERAGE_STATE";

      case QCRIL_EVT_HOOK_EMBMS_GET_RSSI:
          return "QCRIL_EVT_HOOK_EMBMS_GET_RSSI";

      case QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_SVC_STATE:
          return "QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_SVC_STATE";

      case QCRIL_EVT_HOOK_EMBMS_CONTENT_DESC_UPDATE:
          return "QCRIL_EVT_HOOK_EMBMS_CONTENT_DESC_UPDATE";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_CONTENT_DESC_CONTROL:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_CONTENT_DESC_CONTROL";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_SVC_STATE:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_SVC_STATE";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_ACTIVE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_ACTIVE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_COVERAGE:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_COVERAGE";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_OSS_WARNING:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_OSS_WARNING";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_AVAILABLE_TMGI:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_AVAILABLE_TMGI";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_CELL_INFO_CHANGED:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_CELL_INFO_CHANGED";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_SAI_LIST:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_SAI_LIST";

      case QCRIL_EVT_HOOK_EMBMS_GET_SIB_PLMN:
          return "QCRIL_EVT_HOOK_EMBMS_GET_SIB_PLMN";

      case QCRIL_EVT_HOOK_EMBMS_UNSOL_EMBMS_STATUS:
          return "QCRIL_EVT_HOOK_EMBMS_UNSOL_EMBMS_STATUS";

      case QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_STATUS:
          return "QCRIL_EVT_HOOK_EMBMS_GET_EMBMS_STATUS";

      case QCRIL_EVT_HOOK_UNSOL_GENERIC:
          return "QCRIL_EVT_HOOK_UNSOL_GENERIC";

      case QCRIL_EVT_HOOK_REQ_GENERIC:
          return "QCRIL_EVT_HOOK_REQ_GENERIC";

      case QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ:
          return "QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_REQ";

      case QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_CON:
          return "QCRIL_EVT_QMI_RIL_EMBMS_ENABLE_DATA_CON";

      case QCRIL_EVT_QMI_RIL_EMBMS_DISABLE_DATA_IND:
          return "QCRIL_EVT_QMI_RIL_EMBMS_DISABLE_DATA_IND";

      case QCRIL_EVT_QMI_RIL_SEND_UNSOL_RADIO_STATE_CHANGED:
          return "QCRIL_EVT_QMI_RIL_SEND_UNSOL_RADIO_STATE_CHANGED";

      case QCRIL_EVT_QMI_RIL_CHECK_PRL_VER_CHANGE:
          return "QCRIL_EVT_QMI_RIL_CHECK_PRL_VER_CHANGE";

      case QCRIL_EVT_QMI_RIL_PRL_VER_FETCH_ATTEMPT:
          return "QCRIL_EVT_QMI_RIL_PRL_VER_FETCH_ATTEMPT";

      case QCRIL_EVT_QMI_REQUEST_EMBMS_ENABLE:
          return "QCRIL_EVT_QMI_REQUEST_EMBMS_ENABLE";

      case QCRIL_EVT_QMI_REQUEST_EMBMS_DISABLE:
          return "QCRIL_EVT_QMI_REQUEST_EMBMS_DISABLE";

      case QCRIL_EVT_QMI_REQUEST_MODIFY_INITIATE:
          return "QCRIL_EVT_QMI_REQUEST_MODIFY_INITIATE";

      case QCRIL_EVT_QMI_REQUEST_MODIFY_CONFIRM:
          return "QCRIL_EVT_QMI_REQUEST_MODIFY_CONFIRM";

      case QCRIL_EVT_QMI_REQUEST_POWER_WAIT_FOR_CARD_STATUS:
          return "QCRIL_EVT_QMI_REQUEST_POWER_WAIT_FOR_CARD_STATUS";

      case QCRIL_EVT_QMI_REQUEST_3GPP2_SUB:
          return "QCRIL_EVT_QMI_REQUEST_3GPP2_SUB";

      case QCRIL_EVT_QMI_REQUEST_SET_SUBS_MODE:
          return "QCRIL_EVT_QMI_REQUEST_SET_SUBS_MODE";

      case QCRIL_EVT_QMI_NAS_SIG_STRENGTH_UPDATE:
          return "QCRIL_EVT_QMI_NAS_SIG_STRENGTH_UPDATE";

      case QCRIL_EVT_QMI_NAS_DSDS_SUBS_FOLLOWUP:
          return "QCRIL_EVT_QMI_NAS_DSDS_SUBS_FOLLOWUP";

      case QCRIL_EVT_QMI_NAS_DSDS_SUBS_DEACTIVATE_FOLLOWUP:
          return "QCRIL_EVT_QMI_NAS_DSDS_SUBS_DEACTIVATE_FOLLOWUP";

      case QCRIL_EVT_QMI_NAS_CARD_STATUS_UPDATE:
          return "QCRIL_EVT_QMI_NAS_CARD_STATUS_UPDATE";

      case QCRIL_EVT_QMI_VOICE_BURST_START_CONT_DTMF:
          return "QCRIL_EVT_QMI_VOICE_BURST_START_CONT_DTMF";

      case QCRIL_EVT_QMI_VOICE_BURST_STOP_CONT_DTMF:
          return "QCRIL_EVT_QMI_VOICE_BURST_STOP_CONT_DTMF";

      case QCRIL_EVT_QMI_VOICE_PENDING_MNG_CALLS_REQ_FINISHED:
          return "QCRIL_EVT_QMI_VOICE_PENDING_MNG_CALLS_REQ_FINISHED";

      case QCRIL_EVT_QMI_VOICE_GET_WAITING_CALL:
          return "QCRIL_EVT_QMI_VOICE_GET_WAITING_CALL";

      case QCRIL_EVT_QMI_VOICE_EMERGENCY_CALL_PENDING:
          return "QCRIL_EVT_QMI_VOICE_EMERGENCY_CALL_PENDING";

      case QCRIL_EVT_HOOK_VT_DIAL_CALL:
          return "QCRIL_EVT_HOOK_VT_DIAL_CALL";

      case QCRIL_EVT_HOOK_VT_END_CALL:
          return "QCRIL_EVT_HOOK_VT_END_CALL";

      case QCRIL_EVT_HOOK_VT_ANSWER_CALL:
          return "QCRIL_EVT_HOOK_VT_ANSWER_CALL";

      case QCRIL_EVT_HOOK_VT_GET_CALL_INFO:
          return "QCRIL_EVT_HOOK_VT_GET_CALL_INFO";

      case QCRIL_EVT_HOOK_VT_UNSOL_CALL_STATUS_IND:
          return "QCRIL_EVT_HOOK_VT_UNSOL_CALL_STATUS_IND";

      case QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ:
          return "QCRIL_EVT_HOOK_IMS_ENABLER_STATE_REQ";

      case QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ:
          return "QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_REQ";

      case QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ:
          return "QCRIL_EVT_HOOK_IMS_SEND_UNPUBLISH_REQ";

      case QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ:
          return "QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_REQ";

      case QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ:
          return "QCRIL_EVT_HOOK_IMS_SEND_UNSUBSCRIBE_REQ";

      case QCRIL_EVT_HOOK_IMS_NOTIFY_XML_IND_V01:
          return "QCRIL_EVT_HOOK_IMS_NOTIFY_XML_IND_V01";

      case QCRIL_EVT_HOOK_IMS_NOTIFY_IND_V01:
          return "QCRIL_EVT_HOOK_IMS_NOTIFY_IND_V01";

      case QCRIL_EVT_HOOK_IMS_ENABLER_STATUS_IND:
          return "QCRIL_EVT_HOOK_IMS_ENABLER_STATUS_IND";

      case QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ:
          return "QCRIL_EVT_HOOK_IMS_SEND_PUBLISH_XML_REQ";

      case QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ:
          return "QCRIL_EVT_HOOK_IMS_SEND_SUBSCRIBE_XML_REQ";

      case QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01:
          return "QCRIL_EVT_HOOK_IMS_SET_NOTIFY_FMT_REQ_V01";

      case QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01:
          return "QCRIL_EVT_HOOK_IMS_GET_NOTIFY_FMT_REQ_V01";

      case QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01:
          return "QCRIL_EVT_HOOK_IMS_SET_EVENT_REPORT_REQ_V01";

      case QCRIL_EVT_HOOK_IMS_GET_EVENT_REPORT_REQ_V01:
          return "QCRIL_EVT_HOOK_IMS_GET_EVENT_REPORT_REQ_V01";

      case QCRIL_EVT_HOOK_IMS_PUBLISH_TRIGGER_IND_V01:
          return "QCRIL_EVT_HOOK_IMS_PUBLISH_TRIGGER_IND_V01";

      case QCRIL_EVT_QMI_RIL_POST_VOICE_RTE_CHANGE_IND:
          return "QCRIL_EVT_QMI_RIL_POST_VOICE_RTE_CHANGE_IND";

      case QCRIL_EVT_QMI_VOICE_HANDLE_COMM_CALLBACKS:
          return "QCRIL_EVT_QMI_VOICE_HANDLE_COMM_CALLBACKS";

      case QCRIL_EVT_QMI_VOICE_HANDLE_INDICATIONS:
          return "QCRIL_EVT_QMI_VOICE_HANDLE_INDICATIONS";

      case QCRIL_EVT_QMI_IMSS_HANDLE_COMM_CALLBACKS:
          return "QCRIL_EVT_QMI_IMSS_HANDLE_COMM_CALLBACKS";

      case QCRIL_EVT_QMI_NAS_PASSOVER_NW_SEL_IND:
          return "QCRIL_EVT_QMI_NAS_PASSOVER_NW_SEL_IND";

      case QCRIL_EVT_QMI_NAS_HANDLE_INDICATIONS:
          return "QCRIL_EVT_QMI_NAS_HANDLE_INDICATIONS";

      case QCRIL_EVT_QMI_NAS_HANDLE_ASYNC_CB:
          return "QCRIL_EVT_QMI_NAS_HANDLE_ASYNC_CB";

      case QCRIL_EVT_QMI_DMS_HANDLE_INDICATIONS:
          return "QCRIL_EVT_QMI_DMS_HANDLE_INDICATIONS";

      case QCRIL_EVT_QMI_PBM_HANDLE_INDICATIONS:
          return "QCRIL_EVT_QMI_PBM_HANDLE_INDICATIONS";

      case QCRIL_EVT_QMI_SMS_HANDLE_INDICATIONS:
          return "QCRIL_EVT_QMI_SMS_HANDLE_INDICATIONS";

      case QCRIL_EVT_HOOK_DATA_GO_DORMANT:
          return "QCRIL_EVT_HOOK_DATA_GO_DORMANT";

      case QCRIL_EVT_PBM_CARD_INSERTED:
          return "QCRIL_EVT_PBM_CARD_INSERTED";

      case QCRIL_EVT_PBM_CARD_INIT_COMPLETED:
          return "QCRIL_EVT_PBM_CARD_INIT_COMPLETED";

      case QCRIL_EVT_PBM_CARD_ERROR:
          return "QCRIL_EVT_PBM_CARD_ERROR";

      case QCRIL_EVT_PBM_UPDATE_OTA_ECC_LIST:
          return "QCRIL_EVT_PBM_UPDATE_OTA_ECC_LIST";

      case QCRIL_EVT_SMS_RAW_READ:
          return "QCRIL_EVT_SMS_RAW_READ";

      case QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY:
          return "QCRIL_EVT_QMI_RIL_ASSESS_EMRGENCY_NUMBER_LIST_DESIGNATED_COUNTRY";

      case QCRIL_EVT_SMS_PERFORM_INITIAL_CONFIGURATION:
          return "QCRIL_EVT_SMS_PERFORM_INITIAL_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_POST_OPRT_ONLINE_ACTION:
          return "QCRIL_EVT_QMI_RIL_POST_OPRT_ONLINE_ACTION";

      case QCRIL_EVT_QMI_RIL_ENFORCE_DEFERRED_MODE_PREF_SET:
          return "QCRIL_EVT_QMI_RIL_ENFORCE_DEFERRED_MODE_PREF_SET";

      case QCRIL_EVT_HOOK_SET_RFM_SCENARIO_REQ:
          return "QCRIL_EVT_HOOK_SET_RFM_SCENARIO_REQ";

      case QCRIL_EVT_HOOK_GET_RFM_SCENARIO_REQ:
          return "QCRIL_EVT_HOOK_GET_RFM_SCENARIO_REQ";

      case QCRIL_EVT_HOOK_GET_PROVISIONED_TABLE_REVISION_REQ:
          return "QCRIL_EVT_HOOK_GET_PROVISIONED_TABLE_REVISION_REQ";

      case QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC:
          return "QCRIL_EVT_HOOK_SET_CDMA_SUB_SRC_WITH_SPC";

      case QCRIL_EVT_HOOK_CDMA_AVOID_CUR_NWK:
          return "QCRIL_EVT_HOOK_CDMA_AVOID_CUR_NWK";

      case QCRIL_EVT_HOOK_CDMA_CLEAR_AVOIDANCE_LIST:
          return "QCRIL_EVT_HOOK_CDMA_CLEAR_AVOIDANCE_LIST";

      case QCRIL_EVT_HOOK_CDMA_GET_AVOIDANCE_LIST:
          return "QCRIL_EVT_HOOK_CDMA_GET_AVOIDANCE_LIST";

      case QCRIL_EVT_HOOK_GET_SAR_REV_KEY:
          return "QCRIL_EVT_HOOK_GET_SAR_REV_KEY";

      case QCRIL_EVT_HOOK_SET_TRANSMIT_POWER:
          return "QCRIL_EVT_HOOK_SET_TRANSMIT_POWER";

      case QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE:
          return "QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE";

      case QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE:
          return "QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE";

      case QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS:
          return "QCRIL_EVT_HOOK_GET_AVAILABLE_CONFIGS";

      case QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER:
          return "QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_ACQ_ORDER";

      case QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER:
          return "QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_ACQ_ORDER";

      case QCRIL_EVT_HOOK_UNSOL_UICC_VOLTAGE_STATUS:
          return "QCRIL_EVT_HOOK_UNSOL_UICC_VOLTAGE_STATUS";

      case QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ:
          return "QCRIL_EVT_HOOK_UICC_VOLTAGE_STATUS_REQ";

      case QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ:
          return "QCRIL_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ";

      case QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ:
          return "QCRIL_EVT_HOOK_PERSONALIZATION_STATUS_REQ";

      case QCRIL_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED:
          return "QCRIL_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED";

      case QCRIL_EVT_HOOK_CLEANUP_LOADED_CONFIGS:
          return "QCRIL_EVT_HOOK_CLEANUP_LOADED_CONFIGS";

      case QCRIL_EVT_HOOK_SEL_CONFIG:
          return "QCRIL_EVT_HOOK_SEL_CONFIG";

      case QCRIL_EVT_HOOK_GET_META_INFO:
          return "QCRIL_EVT_HOOK_GET_META_INFO";

      case QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS:
          return "QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS";

      case QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE:
          return "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE";

      case QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID:
          return "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID";

      case QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE:
          return "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE";

      case QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID:
          return "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID";

      case QCRIL_EVT_HOOK_ACTIVATE_CONFIGS:
          return "QCRIL_EVT_HOOK_ACTIVATE_CONFIGS";

      case QCRIL_EVT_HOOK_VALIDATE_CONFIG:
          return "QCRIL_EVT_HOOK_VALIDATE_CONFIG";

      case QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION:
          return "QCRIL_EVT_QMI_RIL_PDC_LOAD_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION:
          return "QCRIL_EVT_QMI_RIL_PDC_SELECT_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_PDC_ACTIVATE_CONFIGURATION:
          return "QCRIL_EVT_QMI_RIL_PDC_ACTIVATE_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_PDC_DELETE_CONFIGURATION:
          return "QCRIL_EVT_QMI_RIL_PDC_DELETE_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION:
          return "QCRIL_EVT_QMI_RIL_PDC_LIST_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION:
          return "QCRIL_EVT_QMI_RIL_PDC_DEACTIVATE_CONFIGURATION";

      case QCRIL_EVT_QMI_RIL_PDC_PARSE_DIFF_RESULT:
          return "QCRIL_EVT_QMI_RIL_PDC_PARSE_DIFF_RESULT";

      case QCRIL_EVT_QMI_REQUEST_SET_SYS_SEL_PREF:
          return "QCRIL_EVT_QMI_REQUEST_SET_SYS_SEL_PREF";

      case QCRIL_EVT_HOOK_UNSOL_SIM_REFRESH:
          return "QCRIL_EVT_HOOK_UNSOL_SIM_REFRESH";

      case QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS:
          return "QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS";

      case QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER:
          return "QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER";

      case QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF:
          return "QCRIL_EVT_HOOK_SET_PREFERRED_NETWORK_BAND_PREF";

      case QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF:
          return "QCRIL_EVT_HOOK_GET_PREFERRED_NETWORK_BAND_PREF";

      case QCRIL_EVT_IMS_SOCKET_IMS_REGISTRATION_STATE:
          return "QCRIL_EVT_IMS_SOCKET_IMS_REGISTRATION_STATE";

      case QCRIL_EVT_IMS_SOCKET_REQ_DIAL:
          return "QCRIL_EVT_IMS_SOCKET_REQ_DIAL";

      case QCRIL_EVT_IMS_SOCKET_REQ_ANSWER:
          return "QCRIL_EVT_IMS_SOCKET_REQ_ANSWER";

      case QCRIL_EVT_IMS_SOCKET_REQ_HANGUP:
          return "QCRIL_EVT_IMS_SOCKET_REQ_HANGUP";

      case QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE:
          return "QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE";

      case QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS";

      case QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND:
          return "QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND";

      case QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND:
          return "QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND";

      case QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";

      case QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE:
          return "QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE";

      case QCRIL_EVT_IMS_SOCKET_REQ_EXIT_ECBM:
          return "QCRIL_EVT_IMS_SOCKET_REQ_EXIT_ECBM";

      case QCRIL_EVT_IMS_SOCKET_REQ_DTMF:
          return "QCRIL_EVT_IMS_SOCKET_REQ_DTMF";

      case QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START:
          return "QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START";

      case QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP:
          return "QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP";

      case QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE:
          return "QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE";

      case QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM:
          return "QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM";

      case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP:
          return "QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP";

      case QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR:
          return "QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR";

      case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS";

      case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING:
          return "QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING";

      case QCRIL_EVT_IMS_SOCKET_REQ_IMS_REG_STATE_CHANGE:
          return "QCRIL_EVT_IMS_SOCKET_REQ_IMS_REG_STATE_CHANGE";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION";

      case QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT:
          return "QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT";

      case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_SERVICE_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_QUERY_SERVICE_STATUS";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_SERVICE_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_SERVICE_STATUS";

      case QCRIL_EVT_IMS_SOCKET_REQ_STOP_CONT_DTMF:
          return "QCRIL_EVT_IMS_SOCKET_REQ_STOP_CONT_DTMF";

      case QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_STOP_CONT_DTMF";

      case QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION:
          return "QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION";

      case QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY:
          return "QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VT_CALL_QUALITY";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_VT_CALL_QUALITY:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_VT_CALL_QUALITY";

      case QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR:
          return "QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR";

      case QCRIL_EVT_IMS_SOCKET_SEND_UNSOL_CURRENT_CALLS:
          return "QCRIL_EVT_IMS_SOCKET_SEND_UNSOL_CURRENT_CALLS";

      case QCRIL_EVT_IMS_SOCKET_REQ_HOLD:
          return "QCRIL_EVT_IMS_SOCKET_REQ_HOLD";

      case QCRIL_EVT_IMS_SOCKET_REQ_RESUME:
          return "QCRIL_EVT_IMS_SOCKET_REQ_RESUME";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_TTY_MODE:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_TTY_MODE";

      case QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_STATISTICS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_STATISTICS";

      case QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_ERROR_STATISTICS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_GET_RTP_ERROR_STATISTICS";

      case QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_GET_WIFI_CALLING_STATUS";

      case QCRIL_EVT_IMS_SOCKET_REQ_SET_WIFI_CALLING_STATUS:
          return "QCRIL_EVT_IMS_SOCKET_REQ_SET_WIFI_CALLING_STATUS";

      default:
        return "<Unknown event> ?";
    } /* end switch */
  }

} /* qcril_log_lookup_event_name */


//============================================================================
// FUNCTION: qcril_ipc_init
//
// DESCRIPTION:
// create and bind sockets to respective ports, create receiver thread
// used for piping logs from 2nd ril instance to 1st instance so they can
// both be sent to diag
// Also used to send RADIO_POWER state change messages to sync RADIO_POWER
// state changes between multiple RILs.
//
// RETURN: None
//============================================================================
//
void qcril_ipc_init()
{
    int sockfd=0,rc=0,len=0,number_of_tries=0,bind_result=FALSE;
    struct sockaddr_un local;
    char server[QCRIL_IPC_MAX_SOCKET_PATH_LENGTH];

    signal(SIGUSR1,qcril_multiple_rild_ipc_signal_handler_sigusr1);
    memset(&inter_rild_info,0,sizeof(inter_rild_info));
    rc = qcril_ipc_evaluate_rilds_socket_paths(server);
    if(rc)
    {
        unlink (server);

        //mutex initialization
        pthread_mutex_init(&inter_rild_info.send_lock_mutex, NULL);

        //server initialization
        do
        {
          if (number_of_tries != 0)
          {
            sleep(1);
          }
          QCRIL_LOG_DEBUG("IPC socket init try # %d", number_of_tries);
          if ((sockfd = socket(AF_UNIX,SOCK_DGRAM,0)) >= 0)
          {
            local.sun_family = AF_UNIX;
            strlcpy(local.sun_path, server, sizeof(local.sun_path));
            len = strlen(server) + sizeof(local.sun_family);

            if (bind(sockfd,(struct sockaddr *)&local, len) >= 0)
            {
                inter_rild_info.my_sockid = sockfd;
                pthread_attr_init(&inter_rild_info.recv_thread_attr);
                pthread_attr_setdetachstate(&inter_rild_info.recv_thread_attr, PTHREAD_CREATE_JOINABLE);
                if(!pthread_create(&inter_rild_info.recv_thread_id,&inter_rild_info.recv_thread_attr,
                                  qcril_multiple_rild_ipc_recv_func,(void*) &inter_rild_info))
                {
                    qmi_ril_set_thread_name(inter_rild_info.recv_thread_id, QMI_RIL_IPC_RECEIVER_THREAD_NAME);
                    inter_rild_info.info_valid = TRUE;
                }
                else
                {
                    QCRIL_LOG_ERROR("unable to spawn dedicated thread for rild IPC");
                    close(sockfd);
                }
                bind_result = TRUE;
            }
            else
            {
                QCRIL_LOG_ERROR("unable to bind socket for rild IPC");
                close(sockfd);
            }
          }
          else
          {
            QCRIL_LOG_ERROR("unable to open socket for rild IPC");
          }
          number_of_tries++;
        } while((bind_result == FALSE) && (number_of_tries < QCRIL_IPC_BIND_RETRY_MAX_ATTEMPTS));
    }
    else if(inter_rild_info.other_rilds_addr_info)
    {
        qcril_free(inter_rild_info.other_rilds_addr_info);
        inter_rild_info.other_rilds_addr_info_len = QMI_RIL_ZERO;
        inter_rild_info.other_rilds_addr_info = NULL;
    }
}

//============================================================================
// FUNCTION: qcril_ipc_release
//
// DESCRIPTION:
// release resources used to create inter-ril communication socket
//
// RETURN: None
//============================================================================
//
void qcril_ipc_release()
{
  if(inter_rild_info.info_valid)
  {
    inter_rild_info.info_valid = FALSE;
    pthread_mutex_destroy(&inter_rild_info.send_lock_mutex);
    pthread_kill(inter_rild_info.recv_thread_id,SIGUSR1);
    pthread_join(inter_rild_info.recv_thread_id,NULL);
    pthread_attr_destroy(&inter_rild_info.recv_thread_attr);
    close(inter_rild_info.my_sockid);

    if(inter_rild_info.other_rilds_addr_info)
    {
        qcril_free(inter_rild_info.other_rilds_addr_info);
        inter_rild_info.other_rilds_addr_info_len = QMI_RIL_ZERO;
        inter_rild_info.other_rilds_addr_info = NULL;
    }
  }
}

//============================================================================
// FUNCTION: qcril_ipc_evaluate_rilds_socket_paths
//
// DESCRIPTION:
// Evaluate socket paths for the current rild and the other rild's in mutiple rild scenario
//rild_socket_name will be updated to the current rild's socket path
//
// RETURN: FALSE If evaluation ended up with a error, TRUE otherwise
//============================================================================
//
int qcril_ipc_evaluate_rilds_socket_paths(char *rild_socket_name)
{
    int iter_i;
    int iter_other_rilds_addr_info;
    int num_of_rilds;
    int result = TRUE;
    struct sockaddr_un remote;

    num_of_rilds = qmi_ril_retrieve_number_of_rilds();

    iter_other_rilds_addr_info = 0;
    inter_rild_info.other_rilds_addr_info_len = num_of_rilds - 1;
    inter_rild_info.other_rilds_addr_info = qcril_malloc(sizeof(other_rild_addr_info_type) * (inter_rild_info.other_rilds_addr_info_len));
    if(inter_rild_info.other_rilds_addr_info)
    {
        for(iter_i = 0; iter_i < num_of_rilds; iter_i++)
        {
            if( iter_i == (int) qmi_ril_get_process_instance_id() )
            {
                snprintf( rild_socket_name, QCRIL_IPC_MAX_SOCKET_PATH_LENGTH, "%s%d", QCRIL_IPC_RILD_SOCKET_PATH_PREFIX,iter_i );
            }
            else
            {
                memset(&remote, 0, sizeof(remote));
                remote.sun_family = AF_UNIX;
                snprintf( remote.sun_path, sizeof(remote.sun_path), "%s%d", QCRIL_IPC_RILD_SOCKET_PATH_PREFIX,iter_i );
                inter_rild_info.other_rilds_addr_info[iter_other_rilds_addr_info].rild_addrlen =
                   strlen(remote.sun_path) + sizeof(remote.sun_family);
                inter_rild_info.other_rilds_addr_info[iter_other_rilds_addr_info].rild_addr = remote;
                inter_rild_info.other_rilds_addr_info[iter_other_rilds_addr_info].rild_instance_id = iter_i;
                iter_other_rilds_addr_info++;
            }
        }
    }
    else
    {
        result = FALSE;
        QCRIL_LOG_FATAL("Fail to allocate memory for inter_rild_info.other_rilds_addr_info");
    }
    return result;
} //qcril_ipc_evaluate_rilds_socket_paths

// prepare for sending radio power sync state to other rild
void qcril_multiple_rild_ipc_radio_power_propagation_helper_func(int is_genuine_signal)
{
    int iter_i;
    int num_of_rilds;

    if(inter_rild_info.other_rilds_addr_info)
    {
        num_of_rilds = qmi_ril_retrieve_number_of_rilds();

        for(iter_i = 0; iter_i < num_of_rilds; iter_i++)
        {
            if( iter_i != (int) qmi_ril_get_process_instance_id() )
            {
                qcril_multiple_rild_ipc_send_func(IPC_MESSAGE_RADIO_POWER, &is_genuine_signal, sizeof(is_genuine_signal), iter_i);
            }
        }
    }
}

int qcril_multiple_rild_ipc_send_func(ipc_message_id_type message_id, void * payload, int payload_length, int dest_rild_instance_id) //generic send function
{
    ipc_send_recv_data_info_type send_data;
    int iter_i;
    int match = FALSE;

    if(inter_rild_info.info_valid)
    {
          pthread_mutex_lock(&inter_rild_info.send_lock_mutex);
          if(inter_rild_info.other_rilds_addr_info)
          {
              for(iter_i = 0 ; iter_i < inter_rild_info.other_rilds_addr_info_len && !match; iter_i++)
              {
                  if(dest_rild_instance_id == (int) inter_rild_info.other_rilds_addr_info[iter_i].rild_instance_id)
                  {
                      match = TRUE;
                  }
              }
          }

          if(match)
          {
              memset(&send_data, 0, sizeof(send_data));         //HEADER = rild_instance_id + message_id
              send_data.rild_instance_id = qmi_ril_get_process_instance_id();
              send_data.message_id = message_id;
              send_data.payload_length = sizeof(send_data.message_id) + sizeof(send_data.rild_instance_id);

              if(NULL != payload && QMI_RIL_ZERO != payload_length)
              {
                  memcpy(&send_data.payload, payload, payload_length);
                  send_data.payload_length += payload_length;
              }
              sendto(inter_rild_info.my_sockid, &send_data, send_data.payload_length, 0,
                    (struct sockaddr *)(&inter_rild_info.other_rilds_addr_info[iter_i - 1].rild_addr),
                    inter_rild_info.other_rilds_addr_info[iter_i -1].rild_addrlen);
          }

          pthread_mutex_unlock(&inter_rild_info.send_lock_mutex);
    }
    return 0;
}

void* qcril_multiple_rild_ipc_recv_func(void *arg) //generic recv function
{
    int sockfd = ((struct inter_rild_info_type*)arg)->my_sockid;
    struct sockaddr_storage source_addr;
    socklen_t source_addr_len = 0;
    int received_buffer_length = 0;
    ipc_message_id_type message_id;

    int radio_power_is_genuine_signal;
    ipc_send_recv_data_info_type recv_data;

    source_addr_len = sizeof(source_addr);
    while(1)
    {
        memset(&source_addr,0,sizeof(source_addr));
        memset(&recv_data, 0, sizeof(recv_data));
        if ((received_buffer_length = recvfrom(sockfd, &recv_data, sizeof(recv_data) , 0,(struct sockaddr *)&source_addr, &source_addr_len)) == -1)
        {
            close(sockfd);
            break;
        }

        switch(recv_data.message_id)
        {
            case IPC_MESSAGE_RADIO_POWER:
                radio_power_is_genuine_signal = *((int *) recv_data.payload);
                qcril_qmi_nas_handle_multiple_rild_radio_power_state_propagation(radio_power_is_genuine_signal);
                break;

            case IPC_MESSAGE_AM_CALL_STATE:
#ifndef QMI_RIL_UTF
                qcril_am_handle_event( QCRIL_AM_EVENT_INTER_RIL_CALL_STATE,
                                       (qcril_am_call_state_type*)recv_data.payload );
#endif
                break;

            default:
                break;
        }
    }

    qmi_ril_clear_thread_name(pthread_self());
    return NULL;
}

void qcril_multiple_rild_ipc_signal_handler_sigusr1(int arg)
{
    QCRIL_NOTUSED(arg);
    return;
}

//===========================================================================
// qmi_ril_set_thread_name
//===========================================================================
void qmi_ril_set_thread_name(pthread_t thread_id, const char *thread_name)
{
    int iter_i = 0;

    for(iter_i = 0; iter_i < QMI_RIL_THREAD_INFO_MAX_SIZE; iter_i++)
    {
        if(FALSE == qmi_ril_thread_name_info[iter_i].is_valid)
        {
            qmi_ril_thread_name_info[iter_i].is_valid = TRUE;
            qmi_ril_thread_name_info[iter_i].thread_id = thread_id;
            strlcpy(qmi_ril_thread_name_info[iter_i].thread_name, thread_name, QMI_RIL_THREAD_NAME_MAX_SIZE);
            break;
        }
    }

} //qmi_ril_set_thread_name

//===========================================================================
// qmi_ril_get_thread_name
//===========================================================================
int qmi_ril_get_thread_name(pthread_t thread_id, char *thread_name)
{
    int iter_i = 0,res = FALSE;

    for(iter_i = 0; iter_i < QMI_RIL_THREAD_INFO_MAX_SIZE; iter_i++)
    {
        if(TRUE == qmi_ril_thread_name_info[iter_i].is_valid && thread_id == qmi_ril_thread_name_info[iter_i].thread_id)
        {
            strlcpy(thread_name, qmi_ril_thread_name_info[iter_i].thread_name, QMI_RIL_THREAD_NAME_MAX_SIZE);
            res = TRUE;
            break;
        }
    }

    return res;
} //qmi_ril_get_thread_name

//===========================================================================
// qmi_ril_clear_thread_name
//===========================================================================
void qmi_ril_clear_thread_name(pthread_t thread_id)
{
    int iter_i = 0;

    for(iter_i = 0; iter_i < QMI_RIL_THREAD_INFO_MAX_SIZE; iter_i++)
    {
        if(TRUE == qmi_ril_thread_name_info[iter_i].is_valid && thread_id == qmi_ril_thread_name_info[iter_i].thread_id)
        {
            qmi_ril_thread_name_info[iter_i].is_valid = FALSE;
            break;
        }
    }

} //qmi_ril_clear_thread_name

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
} /* qcril_log_lookup_ril_event_index */

/*===========================================================================

  FUNCTION: qcril_log_print_ril_message_payload

===========================================================================*/
/*!
    @brief
    Dump one line of RIL payload to the log

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_ril_message_payload(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  char new_format[QCRIL_MAX_LOG_MSG_SIZE];
  snprintf(new_format, QCRIL_MAX_LOG_MSG_SIZE, "$$$$$$ %s", format);
  char buffer[QCRIL_MAX_LOG_MSG_SIZE];
  vsnprintf(buffer, QCRIL_MAX_LOG_MSG_SIZE, new_format, args);
  QCRIL_LOG_INFO("%s", buffer);
  va_end(args);
} /* qcril_log_print_ril_message_payload */

/*===========================================================================

  FUNCTION: qcril_log_print_string_array

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is char**

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_string_array(char** ptr, size_t length)
{
  size_t i;
  for (i = 0; i < length; ++i)
  {
    if (ptr[i] == NULL)
      break;
    else if (ptr[i])
      qcril_log_print_ril_message_payload("(char**)Payload[%d] %s", i, ptr[i]);
  }
} /* qcril_log_print_string_array */

/*===========================================================================

  FUNCTION: qcril_log_print_int

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is int

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_int(int* ptr)
{
  qcril_log_print_ril_message_payload("(int)Payload = %d", *ptr);
} /* qcril_log_print_int */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_Dial

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_Dial

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_Dial(const char* header, const RIL_Dial* ptr)
{
  if (ptr->address)
    qcril_log_print_ril_message_payload("%saddress %s", header, ptr->address);
  qcril_log_print_ril_message_payload("%sclir = %d", header, ptr->clir);
  if (ptr->uusInfo)
  {
    qcril_log_print_ril_message_payload("%suusInfo = (RIL_UUS_Info*)malloc(sizeof(RIL_UUS_Info));", header);
    qcril_log_print_ril_message_payload("%suusInfo->uusType = %d", header, ptr->uusInfo->uusType);
    qcril_log_print_ril_message_payload("%suusInfo->uusDcs = %d", header, ptr->uusInfo->uusDcs);
    qcril_log_print_ril_message_payload("%suusInfo->uusLength = %d", header, ptr->uusInfo->uusLength);
    if (ptr->uusInfo->uusData)
      qcril_log_print_ril_message_payload("%suusInfo->uusData %s", header, ptr->uusInfo->uusData);
  }
} /* qcril_log_print_RIL_Dial */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_Call

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_Call

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_Call(const char* header, const RIL_Call* ptr)
{
  qcril_log_print_ril_message_payload("%sstate = %d", header, ptr->state);
  qcril_log_print_ril_message_payload("%sindex = %d", header, ptr->index);
  qcril_log_print_ril_message_payload("%stoa = %d", header, ptr->toa);
  qcril_log_print_ril_message_payload("%sisMpty = %u", header, ptr->isMpty);
  qcril_log_print_ril_message_payload("%sisMT = %u", header, ptr->isMT);
  qcril_log_print_ril_message_payload("%sals = %u", header, ptr->als);
  qcril_log_print_ril_message_payload("%sisVoice = %u", header, ptr->isVoice);
  qcril_log_print_ril_message_payload("%sisVoicePrivacy = %u", header, ptr->isVoicePrivacy);
  if (ptr->number)
    qcril_log_print_ril_message_payload("%snumber %s", header, ptr->number);
  qcril_log_print_ril_message_payload("%snumberPresentation = %d", header, ptr->numberPresentation);
  if (ptr->name)
    qcril_log_print_ril_message_payload("%sname %s", header, ptr->name);
  qcril_log_print_ril_message_payload("%snamePresentation = %d", header, ptr->namePresentation);
  if (ptr->uusInfo)
  {
    qcril_log_print_ril_message_payload("%suusInfo = (RIL_UUS_Info*)malloc(sizeof(RIL_UUS_Info));", header);
    qcril_log_print_ril_message_payload("%suusInfo->uusType = %d", header, ptr->uusInfo->uusType);
    qcril_log_print_ril_message_payload("%suusInfo->uusDcs = %d", header, ptr->uusInfo->uusDcs);
    qcril_log_print_ril_message_payload("%suusInfo->uusLength = %d", header, ptr->uusInfo->uusLength);
    if (ptr->uusInfo->uusData)
      qcril_log_print_ril_message_payload("%suusInfo->uusData %s", header, ptr->uusInfo->uusData);
  }
} /* qcril_log_print_RIL_Call */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_SignalStrength

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_SignalStrength

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_SignalStrength (char* header, const RIL_SignalStrength* ptr)
{
  qcril_log_print_ril_message_payload("%sGW_SignalStrength.signalStrength = %d",
                               header, ptr->GW_SignalStrength.signalStrength);
  qcril_log_print_ril_message_payload("%sGW_SignalStrength.bitErrorRate = %d",
                               header, ptr->GW_SignalStrength.bitErrorRate);
  qcril_log_print_ril_message_payload("%sCDMA_SignalStrength.dbm = %d",
                               header, ptr->CDMA_SignalStrength.dbm);
  qcril_log_print_ril_message_payload("%sCDMA_SignalStrength.ecio = %d",
                               header, ptr->CDMA_SignalStrength.ecio);
  qcril_log_print_ril_message_payload("%sEVDO_SignalStrength.dbm = %d",
                               header, ptr->EVDO_SignalStrength.dbm);
  qcril_log_print_ril_message_payload("%sEVDO_SignalStrength.ecio = %d",
                               header, ptr->EVDO_SignalStrength.ecio);
  qcril_log_print_ril_message_payload("%sEVDO_SignalStrength.signalNoiseRatio = %d",
                               header, ptr->EVDO_SignalStrength.signalNoiseRatio);
  qcril_log_print_ril_message_payload("%sLTE_SignalStrength.signalStrength = %d",
                               header, ptr->LTE_SignalStrength.signalStrength);
  qcril_log_print_ril_message_payload("%sLTE_SignalStrength.rsrp = %d",
                               header, ptr->LTE_SignalStrength.rsrp);
  qcril_log_print_ril_message_payload("%sLTE_SignalStrength.rsrq = %d",
                               header, ptr->LTE_SignalStrength.rsrq);
  qcril_log_print_ril_message_payload("%sLTE_SignalStrength.rssnr = %d",
                               header, ptr->LTE_SignalStrength.rssnr);
  qcril_log_print_ril_message_payload("%sLTE_SignalStrength.cqi = %d",
                               header, ptr->LTE_SignalStrength.cqi);
#ifndef QMI_RIL_UTF
  qcril_log_print_ril_message_payload("%sTD_SCDMA_SignalStrength.rscp = %d",
                               header, ptr->TD_SCDMA_SignalStrength.rscp);
#endif
} /* qcril_log_print_RIL_SignalStrength */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_CallForwardInfo

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_CallForwardInfo

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_CallForwardInfo(const char* header, const RIL_CallForwardInfo* ptr)
{
  qcril_log_print_ril_message_payload("%sstatus = %d", header, ptr->status);
  qcril_log_print_ril_message_payload("%sreason = %d", header, ptr->reason);
  qcril_log_print_ril_message_payload("%sserviceClass = %d", header, ptr->serviceClass);
  qcril_log_print_ril_message_payload("%stoa = %d", header, ptr->toa);

  if (ptr->number)
    qcril_log_print_ril_message_payload("%snumber %s", header, ptr->number);

  qcril_log_print_ril_message_payload("%stimeSeconds = %d", header, ptr->timeSeconds);
} /* qcril_log_print_RIL_CallForwardInfo */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_Call_array

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is an array of RIL_Call

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_Call_array(const RIL_Call** ptr, size_t datalen)
{
  size_t i;
  for (i = 0; i < datalen/4; ++i)
  {
    if (ptr[i])
    {
      char header [512];
      int n = snprintf(header, 512, "(RIL_Call**)Payload[%u]->", i);
      if (n > 0)
        qcril_log_print_RIL_Call(header, ptr[i]);
    }
  }
} /* qcril_log_print_RIL_Call_array */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_CDMA_SignalInfoRecord

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_CDMA_SignalInfoRecord

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_CDMA_SignalInfoRecord(char* header, const RIL_CDMA_SignalInfoRecord* ptr)
{
  qcril_log_print_ril_message_payload("%sisPresent = %u", header, ptr->isPresent);
  qcril_log_print_ril_message_payload("%ssignalType = %u", header, ptr->signalType);
  qcril_log_print_ril_message_payload("%salertPitch = %u", header, ptr->alertPitch);
  qcril_log_print_ril_message_payload("%ssignal = %u", header, ptr->signal);
} /* qcril_log_print_RIL_CDMA_SignalInfoRecord */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_CallForwardInfo_array

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is an array of RIL_CallForwardInfo

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_CallForwardInfo_array(const RIL_CallForwardInfo** ptr, size_t datalen)
{
  size_t i;
  for (i = 0; i < datalen/4; ++i)
  {
    if (ptr[i])
    {
      char header [512];
      int n = snprintf(header, 512, "(RIL_CallForwardInfo**)Payload[%u]->", i);
      if (n > 0)
        qcril_log_print_RIL_CallForwardInfo(header, ptr[i]);
    }
  }
} /* qcril_log_print_RIL_CallForwardInfo_array */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_IMS_SMS_Message

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_IMS_SMS_Message(char* header, const RIL_IMS_SMS_Message* ptr)
{
  qcril_log_print_ril_message_payload("%stech = %d", header, ptr->tech);
  qcril_log_print_ril_message_payload("%sretry = %u", header, ptr->retry);
  qcril_log_print_ril_message_payload("%smessageRef = %d", header, ptr->messageRef);
  // TODO: array out of bounds
  if (ptr->message.cdmaMessage)
  {
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->uTeleserviceID = %d",
                                 header, ptr->message.cdmaMessage->uTeleserviceID);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->bIsServicePresent = %u",
                                 header, ptr->message.cdmaMessage->bIsServicePresent);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->uServicecategory = %d",
                                 header, ptr->message.cdmaMessage->uServicecategory);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->sAddress.digit_mode = %d",
                                 header, ptr->message.cdmaMessage->sAddress.digit_mode);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->sAddress.number_mode = %d",
                                 header, ptr->message.cdmaMessage->sAddress.number_mode);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->sAddress.number_type = %d",
                                 header, ptr->message.cdmaMessage->sAddress.number_type);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->sAddress.number_plan = %d",
                                 header, ptr->message.cdmaMessage->sAddress.number_plan);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->sAddress.number_of_digits = %u",
                                 header, ptr->message.cdmaMessage->sAddress.number_of_digits);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->sAddress.digits~%s",
                                 header, ptr->message.cdmaMessage->sAddress.digits);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->subaddressType = %d",
                                 header, ptr->message.cdmaMessage->sSubAddress.subaddressType);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->odd = %u",
                                 header, ptr->message.cdmaMessage->sSubAddress.odd);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->number_of_digits = %u",
                                 header, ptr->message.cdmaMessage->sSubAddress.number_of_digits);
    qcril_log_print_ril_message_payload("%smessage.cdmaMessage->digits~%s",
                                 header, ptr->message.cdmaMessage->sSubAddress.digits);
  }
  if (ptr->message.gsmMessage)
  {
    size_t i;
    for (i = 0; i < 2; ++i)
    {
      if (ptr->message.gsmMessage[i])
        qcril_log_print_ril_message_payload("%smessage.gsmMessage[%u] %s",
                                     i, header, ptr->message.gsmMessage[i]);
    }
  }
} /* qcril_log_print_RIL_IMS_SMS_Message */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_CardStatus_v6

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_CardStatus_v6

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_CardStatus_v6(char* header, RIL_CardStatus_v6* ptr)
{
  qcril_log_print_ril_message_payload("%scard_state = %d", header, ptr->card_state);
  qcril_log_print_ril_message_payload("%suniversal_pin_state = %d", header, ptr->universal_pin_state);
  qcril_log_print_ril_message_payload("%sgsm_umts_subscription_app_index = %d",
                               header, ptr->gsm_umts_subscription_app_index);
  qcril_log_print_ril_message_payload("%scdma_subscription_app_index = %d",
                               header, ptr->cdma_subscription_app_index);
  qcril_log_print_ril_message_payload("%sims_subscription_app_index = %d",
                               header, ptr->ims_subscription_app_index);
  qcril_log_print_ril_message_payload("%snum_applications = %d", header, ptr->num_applications);
  size_t i;
  for (i = 0; i < ptr->num_applications; ++i)
  {
    qcril_log_print_ril_message_payload("%s->applications[%u].app_type = %d",
                                 header, i, ptr->applications[i].app_type);
    qcril_log_print_ril_message_payload("%s->applications[%u].app_state = %d",
                                 header, i, ptr->applications[i].app_state);
    qcril_log_print_ril_message_payload("%s->applications[%u].perso_substate = %d",
                                 header, i, ptr->applications[i].perso_substate);
    if (ptr->applications[i].aid_ptr)
      qcril_log_print_ril_message_payload("%s->applications[%u].aid_ptr %s",
                                   header, i, ptr->applications[i].aid_ptr);
    if (ptr->applications[i].app_label_ptr)
      qcril_log_print_ril_message_payload("%s->applications[%u].app_label_ptr %s",
                                   header, i, ptr->applications[i].app_label_ptr);
    qcril_log_print_ril_message_payload("%s->applications[%u].pin1_replaced = %d",
                                 header, i, ptr->applications[i].pin1_replaced);
    qcril_log_print_ril_message_payload("%s->applications[%u].pin1 = %d",
                                 header, i, ptr->applications[i].pin1);
    qcril_log_print_ril_message_payload("%s->applications[%u].pin2 = %d",
                                 header, i, ptr->applications[i].pin2);
  }
} /* qcril_log_print_RIL_CardStatus_v6 */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_SMS_Response

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_SMS_Response

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_SMS_Response(char* header, RIL_SMS_Response* ptr)
{
  qcril_log_print_ril_message_payload("%smessageRef = %d", header, ptr->messageRef);
  if (ptr->ackPDU)
    qcril_log_print_ril_message_payload("%sackPDU %s", header, ptr->ackPDU);
  qcril_log_print_ril_message_payload("%serrorCode = %d", header, ptr->errorCode);
} /* qcril_log_print_RIL_SMS_Response */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_SelectUiccSub

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_SelectUiccSub

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_SelectUiccSub(char* header, RIL_SelectUiccSub* ptr)
{
  qcril_log_print_ril_message_payload("%sslot = %d", header, ptr->slot);
  qcril_log_print_ril_message_payload("%sapp_index = %d", header, ptr->app_index);
  qcril_log_print_ril_message_payload("%ssub_type = %d", header, ptr->sub_type);
  qcril_log_print_ril_message_payload("%sact_status = %d", header, ptr->act_status);
} /* qcril_log_print_RIL_SelectUiccSub */

/*===========================================================================

  FUNCTION: qcril_log_print_RIL_StkCcUnsolSsResponse

===========================================================================*/
/*!
    @brief
    Dump RIL payload if the payload type is RIL_StkCcUnsolSsResponse

    @return
    void
*/
/*=========================================================================*/
void qcril_log_print_RIL_StkCcUnsolSsResponse(char* header, RIL_StkCcUnsolSsResponse** ptr_a)
{
  RIL_StkCcUnsolSsResponse* ptr = *ptr_a;
  qcril_log_print_ril_message_payload("%sserviceType = %d", header, ptr->serviceType);
  qcril_log_print_ril_message_payload("%srequestType = %d", header, ptr->requestType);
  qcril_log_print_ril_message_payload("%steleserviceType = %d", header, ptr->teleserviceType);
  qcril_log_print_ril_message_payload("%sserviceClass = %d", header, ptr->serviceClass);
  qcril_log_print_ril_message_payload("%sresult = %d", header, ptr->result);
  size_t i;
  for (i = 0; i < SS_INFO_MAX; ++i)
    qcril_log_print_ril_message_payload("%sssInfo[%u] = %d", header, i, ptr->ssInfo[i]);
  qcril_log_print_ril_message_payload("%scfData.numValidIndexes = %d",
                               header, ptr->cfData.numValidIndexes);
  for (i = 0; i < NUM_SERVICE_CLASSES; ++i)
  {
    qcril_log_print_ril_message_payload("%scfData.cfInfo[%u].status = %d",
                                 header, i, ptr->cfData.cfInfo[i].status);
    qcril_log_print_ril_message_payload("%scfData.cfInfo[%u].reason = %d",
                                 header, i, ptr->cfData.cfInfo[i].reason);
    qcril_log_print_ril_message_payload("%scfData.cfInfo[%u].serviceClass = %d",
                                 header, i, ptr->cfData.cfInfo[i].serviceClass);
    qcril_log_print_ril_message_payload("%scfData.cfInfo[%u].toa = %d",
                                 header, i, ptr->cfData.cfInfo[i].toa);
    //TODO: array out of bounds
    if (ptr->cfData.cfInfo[i].number)
      qcril_log_print_ril_message_payload("%scfData.cfInfo[%u].number %s",
                                   header, i, ptr->cfData.cfInfo[i].number);
    qcril_log_print_ril_message_payload("%scfData.cfInfo[%u].timeSeconds = %d",
                                 header, i, ptr->cfData.cfInfo[i].timeSeconds);
  }
} /* qcril_log_print_RIL_StkCcUnsolSsResponse */

/*===========================================================================

FUNCTION: qcril_log_print_ril_message

===========================================================================*/
/*!
  @brief
  print content of RIL message name and payload to the log

  @return
  void
  */
/*=========================================================================*/
void qcril_log_print_ril_message(int message_id, RIL__MsgType message_type, void* data,
                                  size_t datalen, RIL_Errno error)
{
  if (!qcril_log_ril_msg_payload_log_on)
  {
    return;
  }
  qcril_log_print_ril_message_payload("%s", "Begin of RIL Message");
  qcril_log_print_ril_message_payload("Message ID: %d, Message Type: %d, Data Length: %u, Error: %d",
                               message_id, message_type, (unsigned int)datalen, error);
  qcril_log_print_ril_message_payload("Message name: %s", qcril_log_lookup_event_name(message_id));

  int is_request_or_unsol = 0;
  if ((message_type == RIL__MSG_TYPE__REQUEST) || (message_type == RIL__MSG_TYPE__UNSOL_RESPONSE))
  {
    is_request_or_unsol = 1;
  }
  if (data)
  {
    switch (message_id)
    {
      /* Switch Cases are sorted by payload type*/
      /* data = NULL, response = NULL */
      case RIL_REQUEST_ANSWER:
      case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED:
      case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED:
      case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED:
      case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
      case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
      case RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
      case RIL_REQUEST_CONFERENCE:
      case RIL_UNSOL_UNKOWN:
        break; /* data = NULL, response = NULL */

      /* data = int*, response = NULL */
      case RIL_REQUEST_HANGUP:
      case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
      case RIL_REQUEST_RADIO_POWER:
      case RIL_REQUEST_SEPARATE_CONNECTION:
      case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:
      case RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE:
      case RIL_REQUEST_SCREEN_STATE:
      case RIL_REQUEST_SET_TTY_MODE:
      case RIL_REQUEST_SMS_ACKNOWLEDGE:
      case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED:
      case RIL_UNSOL_SRVCC_STATE_NOTIFY:
        if (is_request_or_unsol == 1)
        {
          int* ptr = (int*) data;
          qcril_log_print_int(ptr);
        }
        break; /* data = int*, response = NULL */

      /* data = NULL, response = int* */
      case RIL_REQUEST_VOICE_RADIO_TECH:
      case RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE:
      case RIL_REQUEST_LAST_CALL_FAIL_CAUSE:
      case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
      case RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE:
      case RIL_REQUEST_IMS_REGISTRATION_STATE:
      case RIL_REQUEST_QUERY_CLIP:
        if (is_request_or_unsol == 0)
        {
          int* ptr = (int*) data;
          qcril_log_print_int(ptr);
        }
        break; /* data = NULL, response = int* */

      /* data = char**, response = int*/
      case RIL_REQUEST_QUERY_FACILITY_LOCK:
        if (is_request_or_unsol == 1)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 4);
        }
        else
        {
          int* ptr = (int*) data;
          qcril_log_print_int(ptr);
        }
        break;
      case RIL_REQUEST_SET_FACILITY_LOCK:
        if (is_request_or_unsol == 1)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 5);
        }
        else
        {
          int* ptr = (int*) data;
          qcril_log_print_int(ptr);
        }
        break;
      case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        if (is_request_or_unsol == 0)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 15);
        }
        break;
      case RIL_REQUEST_DATA_REGISTRATION_STATE:
        if (is_request_or_unsol == 0)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 11);
        }
        break;
      case RIL_REQUEST_OPERATOR:
        if (is_request_or_unsol == 0)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 3);
        }
        break;
      case RIL_REQUEST_QUERY_AVAILABLE_NETWORKS:
        if (is_request_or_unsol == 0)
        {
          char** ptr = (char**) data;
          // TODO: find the number of available networks n. argument should be 4*n
          qcril_log_print_string_array(ptr, 4);
        }
        break; /* data = char**, response = int*/

      /* data = char**, response = NULL */
      case RIL_UNSOL_ON_USSD:
        if (is_request_or_unsol == 1)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 2);
        }
        break; /* data = char**, response = NULL */


      /* data = RIL_Dial*, response = NULL */
      case RIL_REQUEST_DIAL:
        if (is_request_or_unsol == 1)
        {
          const RIL_Dial* ptr = (const RIL_Dial*) data;
          qcril_log_print_RIL_Dial("(RIL_Dial*)Payload->", ptr);
        }
        break; /* data = RIL_Dial*, response = NULL */

      /* data = NULL, response = RIL_Call** */
      case RIL_REQUEST_GET_CURRENT_CALLS:
        if (is_request_or_unsol == 0)
        {
          const RIL_Call** ptr = (const RIL_Call**) data;
          qcril_log_print_RIL_Call_array(ptr, datalen);
        }
        break; /* data = NULL, response = RIL_Call** */

      /* data = NULL, response = RIL_SignalStrength* */
      case RIL_REQUEST_SIGNAL_STRENGTH:
        if (is_request_or_unsol == 0)
        {
          const RIL_SignalStrength* ptr = (const RIL_SignalStrength*)data;
          qcril_log_print_RIL_SignalStrength("(RIL_SignalStrength*)Payload->", ptr);
        }
        break; /* data = NULL, response = RIL_SignalStrength* */

      /* data = RIL_SignalStrength*, response = NULL */
      case RIL_UNSOL_SIGNAL_STRENGTH:
        if (is_request_or_unsol == 1)
        {
          const RIL_SignalStrength* ptr = (const RIL_SignalStrength*)data;
          qcril_log_print_RIL_SignalStrength("(RIL_SignalStrength*)Payload->", ptr);
        }
        break; /* data = RIL_SignalStrength*, response = NULL */

      /* data = NULL for GSM, data = RIL_CDMA_SignalInfoRecord* for CDMA, response = NULL */
      case RIL_UNSOL_CALL_RING:
        if (is_request_or_unsol == 1)
        {
          const RIL_CDMA_SignalInfoRecord* ptr = (const RIL_CDMA_SignalInfoRecord*) data;
          if (ptr)
            qcril_log_print_RIL_CDMA_SignalInfoRecord("(RIL_CDMA_SignalInfoRecord*)Payload->", ptr);
        }
        break; /* data = NULL for GSM, RIL_CDMA_SignalInfoRecord* for CDMA, response = NULL */

      /* data = RIL_CallForwardInfo*, response = RIL_CallForwardInfo** */
      case RIL_REQUEST_QUERY_CALL_FORWARD_STATUS:
        if (is_request_or_unsol == 1)
        {
          const RIL_CallForwardInfo* ptr = (RIL_CallForwardInfo*)data;
          qcril_log_print_RIL_CallForwardInfo("(RIL_CallForwardInfo*)Payload->", ptr);
        }
        else
        {
          const RIL_CallForwardInfo** ptr = (RIL_CallForwardInfo**)data;
          qcril_log_print_RIL_CallForwardInfo_array(ptr, datalen);
        }
        break; /* data = RIL_CallForwardInfo*, response = RIL_CallForwardInfo** */

      /* data = RIL_CallForwardInfo*, response = NULL */
      case RIL_REQUEST_SET_CALL_FORWARD:
        if (is_request_or_unsol == 1)
        {
          const RIL_CallForwardInfo* ptr = (RIL_CallForwardInfo*)data;
          qcril_log_print_RIL_CallForwardInfo("(RIL_CallForwardInfo*)Payload->", ptr);
        }
        break; /* data = RIL_CallForwardInfo*, response = NULL */

      /* data = byte[] or char*, response = NULL */
      case RIL_UNSOL_OEM_HOOK_RAW:
      case RIL_UNSOL_RESPONSE_NEW_SMS:
        if (is_request_or_unsol == 1)
        {
          const char* ptr = (const char*) data;
          qcril_log_print_ril_message_payload("Payload %s", ptr);
        }
        break; /* data = byte[] or char*, response = NULL */

      /* data = char**, response = RIL_SMS_Response* */
      case RIL_REQUEST_SEND_SMS:
        if (is_request_or_unsol == 1)
        {
          char** ptr = (char**) data;
          qcril_log_print_string_array(ptr, 2);
        }
        else
        {
          const RIL_SMS_Response* ptr = (RIL_SMS_Response*)data;
          qcril_log_print_RIL_IMS_SMS_Message("(RIL_IMS_SMS_Message*)Payload->", ptr);
        }
        break; /* data = char**, response = RIL_SMS_Response* */

      /* data = NULL, response = RIL_CardStatus_v6* */
      case RIL_REQUEST_GET_SIM_STATUS:
        if (is_request_or_unsol == 0)
        {
          const RIL_CardStatus_v6* ptr = (RIL_CardStatus_v6*)data;
          qcril_log_print_RIL_CardStatus_v6("(RIL_CardStatus_v6*)Payload->", ptr);
        }
        break; /* data = NULL, response = RIL_CardStatus_v6* */

      /* data = RIL_IMS_SMS_Message*, response = RIL_SMS_Response* */
      case RIL_REQUEST_IMS_SEND_SMS:
        if (is_request_or_unsol == 0)
        {
          const RIL_SMS_Response* ptr = (RIL_SMS_Response*)data;
          qcril_log_print_RIL_SMS_Response("(RIL_SMS_Response)Payload->", ptr);
        }
        else
        {
          const RIL_IMS_SMS_Message* ptr = (RIL_IMS_SMS_Message*)data;
          qcril_log_print_RIL_IMS_SMS_Message("(RIL_IMS_SMS_Message*)Payload->", ptr);
        }
        break; /* data = RIL_IMS_SMS_Message*, response = RIL_SMS_Response* */

      /* data = RIL_SelectUiccSub*, response = NULL */
      case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
        if (is_request_or_unsol == 1)
        {
          const RIL_SelectUiccSub* ptr = (RIL_SelectUiccSub*)data;
          qcril_log_print_RIL_SelectUiccSub("(RIL_SelectUiccSub*)Payload->", ptr);
        }
        break; /* data = RIL_SelectUiccSub*, response = NULL */

      /* data = RIL_StkCcUnsolSsResponse*, response = NULL */
      case RIL_UNSOL_ON_SS:
        if (is_request_or_unsol == 1)
        {
          const RIL_StkCcUnsolSsResponse* ptr = (RIL_StkCcUnsolSsResponse*)data;
          qcril_log_print_RIL_StkCcUnsolSsResponse("(RIL_StkCcUnsolSsResponse*)Payload->", &ptr);
        }
        break; /* data = RIL_StkCcUnsolSsResponse*, response = NULL */

      default:
        qcril_log_print_ril_message_payload("%s %s", "Unsupported payload printing for RIL Message:",
                                     qcril_log_lookup_event_name(message_id));
        break;
    }
  }
  qcril_log_print_ril_message_payload("%s", "End of RIL Message");
} /* qcril_log_print_ril_message */

