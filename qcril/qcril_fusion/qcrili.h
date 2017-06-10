/*!
  @file
  qcrili.h

  @brief
  REQUIRED brief description of this C header file.

  @detail
  OPTIONAL detailed description of this C header file.
  - DELETE this section if unused.

*/

/*===========================================================================

  Copyright (c) 2009-2010, 2013, 2014 Qualcomm Technologies, Inc. All Rights
  Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcrili.h#34 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/28/10   js      fixed GSTK QMI raw command callback
05/13/10   at      Added respective event ids for FEATURE_QCRIL_UIM_QMI
03/01/10   fc      Re-architecture to support split modem.
03/05/10   sm      Added support for CM_PH_EVENT_PRL_INIT.
03/04/10   sk      Added DSAC changes with Featurization
02/09/10   pg      Added support to indicate call re-establishment to UI.
11/19/09   js      Added Support for illegal card state
10/02/09   sb      Added support for getting and setting the SMSC address.
09/15/09   sb      Added support for Extended Burst Type International DBM.
08/10/09   sb      Added support for RUIM SMS.
08/10/09   xz      Added qcril_gstk_request_stk_service_is_running
08/07/09   sb      Added support for reporting memory status.
07/24/09   sb      Added qcril_response_error2, so that a response can be sent
                   on the payload in the case of a failure.
07/10/09   tml     Added intermediate pin2 verificaiton support for sim io
06/23/09   tml     Fixed 6F00 invalid card notification issue
06/15/09   nrn     Adding support for NAM programming
06/15/09   nd      Added support to check the CDMA emergency flash number in ECC property file.
06/06/09   nrn     Adding support for Authentication and Registration Reject
06/03/09   nd      Added support for Otasp/Otapa.
06/01/09   sk      Changes related to new PBM, CM events which support 
                   property file updation with emergency numbers
05/29/09   fc      Renamed functions.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Added support for CDMA phase II.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/28/09   fc      Added support to perform card powerup/powerdown for
                   LPM to ONLINE or vice versa transition. 
04/05/09   fc      Cleanup log macros and mutex macros.
03/17/09   fc      Cleanup unreferenced header and enum.
03/05/09   pg      Added definitions to make data code backward compatible.
02/09/09   fc      Changed to force RTRE config to NV subscription in case of 
                   NV RTRE config set to CM_RTRE_CONFIG_RUIM_OR_DROP_BACK, but
                   runtime control of RTRE config is not enabled. 
02/09/09   fc      Changed the power up sequence to start with Radio State 
                   Unavailable, to command CM to LPM and subscription 
                   unavailable, and to wait for the very first CM_PH_EVENT_INFO 
                   before reading RTRE configuration and transition to Radio
                   State Off.
01/30/09   pg      Get band/mode capability from CM_PH_EVENT_INFO.
01/27/09   xz      Added support of new event QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY
01/26/08   fc      Logged assertion info.
12/16/08   fc      Added function prototypes.
12/08/08   pg      For multi-mode, Radio State is now changed from 
                   RADIO_STATE_NV_READY to RADIO_STATE_SIM_READY when the
                   modem acquires GW system.
                   Added support to pass CDMA voice privacy mode to RILD.
11/14/08   sm      Added temp CDMA data support.
10/28/08   pg      Added support for RIL_REQUEST_SET_LOCATION_UPDATES.
10/10/08   pg      Change token_id used internally from NULL to 0xFFFFFFFF.
10/06/08   pg      Added support for RIL_REQUEST_CDMA_SUBSCRIPTION.
08/29/08   asn     Added data event and and command
08/07/08   pg      Changed CM event that request DTMF wait on.
08/05/08   pg      Added GET IMEI,IMEISV support
06/11/08   jod     Added GET IMSI support
05/23/08   tml     Added perso, pin enable/disable and fdn enable/disable 
                   support
05/22/08   tml     Fixed compilation issue with LTK
05/20/08   tml     Added internal command support
05/19/08   tml     Separate UIM into MMGSDI and GSTK
05/08/08   fc      Added CM events.
05/05/08   da      Initial framework.


===========================================================================*/

#ifndef QCRILI_H
#define QCRILI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "ril.h"
#include "IxErrno.h"
#include "comdef.h"
#include "cm.h"
#include "nv.h"
#include "wms.h"
#include "pbmlib.h"
#include "qcril_uim_srvc.h"
#include "qcril_log.h"
#include "ril_cdma_sms.h"


/*===========================================================================

                   EXTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#ifdef _MSC_VER
#define QCRIL_SNPRINTF _snprintf
#else
#define QCRIL_SNPRINTF snprintf
#endif 

#if (RIL_VERSION > 6)
//Google added this struct with version 7 whereas we introduced it in v6
#define RIL_SimRefreshResponse_v6 RIL_SimRefreshResponse_v7

#endif

#if (RIL_VERSION >= 6)

// ICS support
#undef RIL_CDMA_CallWaiting
#define RIL_CDMA_CallWaiting    RIL_CDMA_CallWaiting_v6
#undef RIL_SignalStrength
#define RIL_SignalStrength      RIL_SignalStrength_v6

#define FEATURE_ICS

// These messages have been renamed on ICS to VOICE_REGISTRATION_STATE and
// VOICE_NETWORK_STATE_CHANGED. But RIL can just use the old names
#define RIL_REQUEST_REGISTRATION_STATE              RIL_REQUEST_VOICE_REGISTRATION_STATE
#define RIL_UNSOL_RESPONSE_NETWORK_STATE_CHANGED    RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED

#endif

#ifndef RIL_QCOM_VERSION

#ifndef RADIO_TECH_GSM
#define RADIO_TECH_GSM     16
#endif

// Added by Qualcomm to identify illegal SIM apps. If RIL_QCOM_VERSION is not defined
// we need this for compilation to succeed. We default it to RIL_APPSTATE_READY
#define RIL_APPSTATE_ILLEGAL    RIL_APPSTATE_READY


// SMS over IMS support using IMS stack in the modem is added by QCOM
// So if RIL_QCOM_VERSION is not defined, then these messages will
// not be defined in ril.h. Add these in here for compilation
// and to avoid conditionally compiled code in QMI RIL
#define RIL_REQUEST_IMS_REGISTRATION_STATE 10009
#define RIL_REQUEST_IMS_SEND_SMS 10010
#define RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED 11009

typedef enum {
    RADIO_TECH_3GPP = 1, /* 3GPP Technologies - GSM, WCDMA */
    RADIO_TECH_3GPP2 = 2 /* 3GPP2 Technologies - CDMA */
} RIL_RadioTechnologyFamily;

typedef struct {
    RIL_RadioTechnologyFamily tech;
    unsigned char             retry;       /* 0 == not retry, nonzero == retry */
    int                       messageRef;  /* Valid field if retry is set to nonzero.
                                              Contains messageRef from RIL_SMS_Response
                                              corresponding to failed MO SMS.
                                            */

    union {
        /* Valid field if tech is RADIO_TECH_3GPP2. See RIL_REQUEST_CDMA_SEND_SMS */
        RIL_CDMA_SMS_Message* cdmaMessage;

        /* Valid field if tech is RADIO_TECH_3GPP. See RIL_REQUEST_SEND_SMS */
        char**                gsmMessage;
    } message;
} RIL_IMS_SMS_Message;

#define RIL_UNSOL_ON_SS 11038
#define RIL_UNSOL_STK_CC_ALPHA_NOTIFY 11039

// OMH is only supported by ril.h when RIL_QCOM_VERSION is defined.

/* Data Call Profile: Simple IP User Profile Parameters*/
typedef struct {
  int  profileId;
  int  priority;       /* priority. [0..255], 0 - highest */
} RIL_DataCallProfileInfo;

#define RIL_REQUEST_GET_DATA_CALL_PROFILE 10111

#define RIL_UNSOL_RESPONSE_TETHERED_MODE_STATE_CHANGED 11037

#endif // #ifndef RIL_QCOM_VERSION

#ifndef RIL_REQUEST_SET_TRANSMIT_POWER
#define RIL_REQUEST_SET_TRANSMIT_POWER  10117
#endif

#define RIL_REQUEST_UNKOWN  998
#define RIL_UNSOL_UNKOWN 1101

#define QCRIL_HOOK_HEADER_SIZE           16
#define QCRIL_HOOK_OEM_NAME              "QUALCOMM"

/* Emergency number ECC list System Property */
#define QCRIL_ECC_LIST                   "ril.ecclist"

#define QCRIL_ARR_SIZE( a ) ( sizeof( ( a ) ) / sizeof( ( a[ 0 ] ) ) )

#define QCRIL_ASSERT( xx_exp ) QCRIL_LOG_ASSERT( xx_exp )

#define QCRIL_MUTEX_LOCK( mutex, log_str )                                                        \
  {                                                                                               \
    QCRIL_LOG_DEBUG("LOCK mutex  %s by thread %u", log_str, (unsigned int) gettid());             \
    pthread_mutex_lock( mutex );                                                                  \
    QCRIL_LOG_DEBUG("LOCK mutex  %s by thread %u success!", log_str, (unsigned int) gettid());    \
  }

#define QCRIL_MUTEX_UNLOCK( mutex, log_str )                                                      \
  {                                                                                               \
    pthread_mutex_unlock( mutex );                                                                \
    QCRIL_LOG_DEBUG("UNLOCK mutex %s by thread %u", log_str, (unsigned int) gettid());            \
  }

/* Most Significant 16 bits are the Instance ID + Modem ID and Least Significant 16 bits are the user data */
#define QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, user_data )                               \
  ( ( ( instance_id & 0xFF ) << 24 ) | ( ( modem_id & 0xFF ) << 16 ) | ( user_data & 0xFFFF ) )   

#define QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data )                                     \
  ( ( user_data & 0xFF000000 ) >> 24 )                                                            

#define QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( user_data )                                        \
  ( ( user_data & 0xFF0000 ) >> 16 )                                                   

#define QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data )                                         \
  ( user_data & 0xFFFF )                                                                          

#define QCRIL_NOTUSED( var ) ((var) = (var))

#ifdef FEATURE_QCRIL_SUBS_CTRL
/* Indicates RF capabilities */
#define QCRIL_MODE_1XEVDO_CONFIG_MASK   0x0001 /*!< RF supports 1X and/or EVDO */
#define QCRIL_MODE_GW_CONFIG_MASK       0x0002 /*!< RF supports GSM and/or WCDMA */

/* Indicates subscription availability */
#define QCRIL_SUBSCRIPTION_NV_MASK      0x0001 /*!< Use NV subscription */
#define QCRIL_SUBSCRIPTION_RUIM_MASK    0x0002 /*!< Use RUIM subscription */
#define QCRIL_SUBSCRIPTION_NV_RUIM_MASK 0x0004 /*!< Use NV or RUIM subscription depending RTRE control */
#define QCRIL_SUBSCRIPTION_SIM_MASK     0x0008 /*!< Use SIM subscription */
#endif /* FEATURE_QCRIL_SUBS_CTRL */

/* Internal Token ID */
#define QCRIL_TOKEN_ID_INTERNAL         0xFFFF
#define QCRIL_TOKEN_ID_INTERNAL1        0xFFFF1

/* Internal Request ID */
#define QCRIL_REQ_ID_INTERNAL           0xFFFF

#ifndef RIL_REQUEST_SET_SUBSCRIPTION_MODE
#define RIL_REQUEST_SET_SUBSCRIPTION_MODE  10113
#endif

#ifndef RIL_REQUEST_SET_UICC_SUBSCRIPTION
#define RIL_REQUEST_SET_UICC_SUBSCRIPTION  10109
#define RIL_REQUEST_SET_DATA_SUBSCRIPTION  10110
#define RIL_REQUEST_GET_UICC_SUBSCRIPTION  10111
#define RIL_REQUEST_GET_DATA_SUBSCRIPTION  10112
#define RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED 11041

typedef enum {
  RIL_UICC_SUBSCRIPTION_DEACTIVATE = 0,
  RIL_UICC_SUBSCRIPTION_ACTIVATE = 1
} RIL_UiccSubActStatus;

typedef enum {
  RIL_SUBSCRIPTION_1 = 0,
  RIL_SUBSCRIPTION_2 = 1
} RIL_SubscriptionType;

typedef struct {
  int   slot;                       /* 0, 1, ... etc. */
  int   app_index;                  /* array subscriptor from applications[RIL_CARD_MAX_APPS] in
                                       RIL_REQUEST_GET_SIM_STATUS */
  RIL_SubscriptionType sub_type;    /* Indicates subscription 0 or subscription 1 */
  RIL_UiccSubActStatus  act_status;
} RIL_SelectUiccSub;

#define RIL_E_SUBSCRIPTION_NOT_SUPPORTED 26

#endif /* RIL_REQUEST_SET_UICC_SUBSCRIPTION */

/* QCRIL Instance ID */
typedef enum
{
  QCRIL_DEFAULT_INSTANCE_ID = 0,
  #ifdef FEATURE_QCRIL_DSDS
  QCRIL_SECOND_INSTANCE_ID  = 1,
  #else
  QCRIL_SECOND_INSTANCE_ID  = 0, /* this is only for compile time compatibility with QMI RIL as UIM code always requires QCRIL_SECOND_INSTANCE_ID declared */
  #endif /* FEATURE_QCRIL_DSDS */
  QCRIL_MAX_INSTANCE_ID
} qcril_instance_id_e_type;

// TSTS feature is not present in RPC RIL. This is defined to avoid
// compilation  issues with UIM which is common for RPC/QMI RIL.
#define QCRIL_THIRD_INSTANCE_ID   2

/* Primary QCRIL Instance */
#define QCRIL_PRIMARY_INSTANCE( instance_id ) ( instance_id == QCRIL_DEFAULT_INSTANCE_ID )

/* Last QCRIL Instance */
#define QCRIL_LAST_INSTANCE( instance_id ) ( instance_id == ( QCRIL_MAX_INSTANCE_ID - 1 ) )

/* DSDS QCRIL Instance Pair */
#define QCRIL_DSDS_INSTANCE_PAIR( instance_id ) ( ( instance_id + 1 ) % 2 )

#ifdef FEATURE_QCRIL_FUSION
#define QCRIL_MAX_MODEM_ID 2
#else
#define QCRIL_MAX_MODEM_ID 1
#endif /* FEATURE_QCRIL_FUSION */

/* QCRIL Modem ID */
typedef enum
{
  QCRIL_DEFAULT_MODEM_ID = 0,   /* MSM */
  QCRIL_SECOND_MODEM_ID  = 1,   /* MDM */
} qcril_modem_id_e_type;

/* QCRIL Event Data Type */
typedef enum
{
  QCRIL_DATA_NOT_ON_STACK = 0,
  QCRIL_DATA_ON_STACK     = 1
} qcril_data_src_e_type;

/* Modem states */
typedef enum
{
  QCRIL_MODEM_STATE_UNAVAILABLE, /*!< Modem is resetting or booting */
  QCRIL_MODEM_STATE_OFF,         /*!< Modem is in low power mode */
  QCRIL_MODEM_STATE_ON           /*!< Modem is in online mode */
} qcril_modem_state_e_type;

/* SIM or RUIM states */
typedef enum
{
  QCRIL_SIM_STATE_ABSENT                  = 0,
  QCRIL_SIM_STATE_NOT_READY               = 1,
  QCRIL_SIM_STATE_READY                   = 2,
  QCRIL_SIM_STATE_PIN                     = 3,
  QCRIL_SIM_STATE_PUK                     = 4,
  QCRIL_SIM_STATE_NETWORK_PERSONALIZATION = 5,
  QCRIL_SIM_STATE_CARD_ERROR              = 6,
  QCRIL_SIM_STATE_ILLEGAL                 = 7
} qcril_sim_state_e_type;

#ifndef FEATURE_ICS
#define QCRIL_RADIO_TECH_IS_3GPP2( voice_tech )                                                 \
       ( voice_tech == QCRIL_RADIO_TECH_3GPP2 )
#else
#define QCRIL_RADIO_TECH_IS_3GPP2( voice_tech )                                                 \
  ( ( voice_tech == QCRIL_RADIO_TECH_IS95A ) || ( voice_tech == QCRIL_RADIO_TECH_IS95B ) ||( voice_tech == QCRIL_RADIO_TECH_1xRTT )  ||  \
    ( voice_tech == QCRIL_RADIO_TECH_EVDO_0 ) || ( voice_tech == QCRIL_RADIO_TECH_EVDO_A )  || ( voice_tech == QCRIL_RADIO_TECH_EVDO_B ) || \
    ( voice_tech == QCRIL_RADIO_TECH_EHRPD ) )
#endif


#ifndef FEATURE_ICS
#define QCRIL_RADIO_TECH_IS_3GPP( voice_tech )                                                 \
       ( voice_tech == QCRIL_RADIO_TECH_3GPP )
#else
#define QCRIL_RADIO_TECH_IS_3GPP( voice_tech )                                                 \
   ( ( voice_tech == QCRIL_RADIO_TECH_GPRS ) || ( voice_tech == QCRIL_RADIO_TECH_EDGE ) ||( voice_tech == QCRIL_RADIO_TECH_UMTS )  ||  \
     ( voice_tech == QCRIL_RADIO_TECH_HSDPA ) || ( voice_tech == QCRIL_RADIO_TECH_HSUPA )  || ( voice_tech == QCRIL_RADIO_TECH_HSPA ) || \
     ( voice_tech == QCRIL_RADIO_TECH_LTE ) )
#endif


/* Radio technology */
typedef enum
{
  QCRIL_RADIO_TECH_NONE   = 0, /*!< Indicates that modem is not on any system yet */
  QCRIL_RADIO_TECH_3GPP   = 1, /*!< Indicates that modem is on 1XEVDO system */
  QCRIL_RADIO_TECH_3GPP2  = 2, /*!< Indicates that modem is on GWL system */
  QCRIL_RADIO_TECH_GLOBAL = 127  /*!< Indicates that modem is on global mode but no service */
} qcril_radio_tech_family_e_type;

typedef enum
{
  QCRIL_RADIO_TECH_UNKNOWN = 0,
  QCRIL_RADIO_TECH_GPRS = 1,
  QCRIL_RADIO_TECH_EDGE = 2,
  QCRIL_RADIO_TECH_UMTS = 3,
  QCRIL_RADIO_TECH_IS95A = 4,
  QCRIL_RADIO_TECH_IS95B = 5,
  QCRIL_RADIO_TECH_1xRTT =  6,
  QCRIL_RADIO_TECH_EVDO_0 = 7,
  QCRIL_RADIO_TECH_EVDO_A = 8,
  QCRIL_RADIO_TECH_HSDPA = 9,
  QCRIL_RADIO_TECH_HSUPA = 10,
  QCRIL_RADIO_TECH_HSPA = 11,
  QCRIL_RADIO_TECH_EVDO_B = 12,
  QCRIL_RADIO_TECH_EHRPD = 13,
  QCRIL_RADIO_TECH_LTE = 14,
  QCRIL_RADIO_TECH_GENERIC = 127
} qcril_radio_tech_e_type;

/* Card status */
typedef enum
{
  QCRIL_CARD_STATUS_UNKNOWN          = 0, /*!< Internal use only */
  QCRIL_CARD_STATUS_NOT_APPLICABLE   = 1, /*!< Indicates that NV is the only subscription source */
  QCRIL_CARD_STATUS_DOWN             = 2, /*!< Indicates that card had been power down */
  QCRIL_CARD_STATUS_UP               = 3, /*!< Indicates that card has been power up */
  QCRIL_CARD_STATUS_NOT_ACCESSIBLE   = 4, /*!< Indicates that card is not accessible */
  QCRIL_CARD_STATUS_REFRESH          = 5, /*!< Indicates that card refresh */
  QCRIL_CARD_STATUS_POWERUP_FAILED   = 6, /*!< Indicates that card powerup failed */
  QCRIL_CARD_STATUS_POWERDOWN_FAILED = 7, /*!< Indicates that card powerdown failure */
  QCRIL_CARD_STATUS_ILLEGAL          = 8, /*!< Indicates that card is illegal, for internal use */
  QCRIL_CARD_STATUS_ABSENT           = 9  /*!< Indicates that card is removed or absent */
} qcril_card_status_e_type;

/* Provision status */
typedef enum
{ 
  QCRIL_PROVISION_STATUS_FAILURE     = 0, /*!< Provision success */
  QCRIL_PROVISION_STATUS_SUCCESS     = 1, /*!< Provision failure */
  QCRIL_PROVISION_STATUS_IN_PROGRESS = 2 /*!< Provision in progress */
} qcril_provision_status_e_type;

/* Unsolicited notification status */
typedef enum
{
  QCRIL_UNSOL_NOTIFICATION_STATUS_NO_CHANGE, /* Indicates that no change in unsolicited notification status */
  QCRIL_UNSOL_NOTIFICATION_STATUS_ON,        /* Indicates that enabled unsolicited notification */
  QCRIL_UNSOL_NOTIFICATION_STATUS_OFF        /* Indicates that disabled unsolicited notification */
} qcril_unsol_notification_status_e_type;

/* Serving system info changed notification status */
typedef enum
{
  QCRIL_SSIC_NOTIFICATION_STATUS_NO_CHANGE,   /* Indicates that no need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_INFO,        /* SS_INFO, may need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_RSSI,        /* RSSI, may need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_SRV_CHANGED, /* SRV_CHANGED, may need to send unsolicited network state changed notification */
  QCRIL_SSIC_NOTIFICATION_STATUS_REG_REJECT   /* REG-REJECT, may need to send unsolicited network state changed notification */
} qcril_ssic_notification_status_e_type;

typedef enum {
  QCRIL_SUBS_MODE_1X = 0,
  QCRIL_SUBS_MODE_GW  = 1
} qcril_subs_mode_pref;

/*! @brief Card status for a Slot 
*/
typedef struct
{
  int slot;                              /* Slot where the card resides */
  qcril_card_status_e_type status;       /* Status of the card */
} qcril_card_info_type;

/*! @brief Slot IDs List 
*/
typedef struct
{
  uint8 num_of_slots;
  uint8 slot_id[ QMI_UIM_MAX_CARD_COUNT ];
} qcril_slot_ids_list_type;

/*! @brief Provision status  
*/
typedef struct
{
  qcril_provision_status_e_type status;  /* Status of the provision */
  qmi_uim_session_type     session_type; /* UIM session type on which this subscription is currently provisioned */
  RIL_Errno err_code;
} qcril_provision_info_type;

#ifdef FEATURE_QCRIL_DSDS
/*! @brief Sub IDs List 
*/
typedef struct
{
  uint8 num_of_subs;
  sys_modem_as_id_e_type sub_id[ QMI_UIM_MAX_CARD_COUNT ];
} qcril_sub_ids_list_type;

typedef struct
{
  uint8 num_of_subs;
  qcril_instance_id_e_type  instance_id[ QMI_UIM_MAX_CARD_COUNT ];
} qcril_instance_ids_list_type;
#endif /* FEATURE_QCRIL_DSDS */

typedef struct {
RIL_SelectUiccSub    uicc_subs_info;
qcril_subs_mode_pref subs_mode_pref;
}qcril_uicc_subs_info_type;

/*! @brief Modem IDs List  
*/
typedef struct
{
  uint8 num_of_modems;
  qcril_modem_id_e_type modem_id[ QCRIL_MAX_MODEM_ID ];
} qcril_modem_ids_list_type;

/* RIL info */
typedef struct
{
  boolean modem_state_changed;                                         /*!< Indicates if modem state changed */ 
  boolean pri_gw_sim_state_changed;                                    /*!< Indicates if primary GW SIM state changed */
  boolean pri_cdma_sim_state_changed;                                  /*!< Indicates if primary CDMA SIM state changed */
  boolean sec_gw_sim_state_changed;                                    /*!< Indicates if secondary GW SIM state changed */
  boolean sec_cdma_sim_state_changed;                                  /*!< Indicates if secondary CDMA SIM state changed */
// TSTS feature is not present in RPC RIL. Thes are defined to avoid
// compilation  issues with UIM which is common for RPC/QMI RIL.
  boolean ter_gw_sim_state_changed;                                    /*!< Indicates if tertiary GW SIM state changed */
  boolean ter_cdma_sim_state_changed;                                  /*!< Indicates if tertiary CDMA SIM state changed */
  boolean voice_radio_tech_changed;                                    /*!< Indicates if voice radio tech changed */
  qcril_modem_state_e_type next_modem_state;                           /*!< Next Modem state if modem_state_changed is set */
  qcril_sim_state_e_type next_pri_gw_sim_state;                        /*!< Next primary GW SIM state if gw_sim_state_changed is set */
  qcril_sim_state_e_type next_pri_cdma_sim_state;                      /*!< Next primary CDMA SIM state if cdma_sim_state_changed is set */
  qcril_sim_state_e_type next_sec_gw_sim_state;                        /*!< Next secondary GW SIM state if gw_sim_state_changed is set */
  qcril_sim_state_e_type next_sec_cdma_sim_state;                      /*!< Next secondary CDMA SIM state if cdma_sim_state_changed is set */
  qcril_sim_state_e_type next_ter_gw_sim_state;                        /*!< Next tertiary GW SIM state if ter_gw_sim_state_changed is set */
  qcril_sim_state_e_type next_ter_cdma_sim_state;                      /*!< Next tertiary CDMA SIM state if ter_cdma_sim_state_changed is set */
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type new_voice_radio_tech;                        /*!< New voice radio technology if voice_radio_tech_changed is set */
  #else
  qcril_radio_tech_e_type        new_voice_radio_tech;                        /*!< New voice radio technology if voice_radio_tech_changed is set */
  #endif
  qcril_ssic_notification_status_e_type ssic_notification_status;      /*!< Serving System Changed notification status */

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  boolean subscription_config_changed;                                 /*!< Indicates if subscription config changed */
  uint16 new_subscription_config_mask;                                 /*!< New subscription configuration mask if subscription_config_changed = TRUE */
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

} qcril_request_return_type;

/* AMDD event IDs */
typedef enum
{
  /* 0 - 0xFFFF reserved for RIL requests defined in ril.h */
  QCRIL_EVT_BASE                               = 0x10000,

  /* AMSS(CM) to QCRIL(CM) events */
  QCRIL_EVT_CM_BASE                            = 0x10000,
  QCRIL_EVT_CM_COMMAND_CALLBACK,

  QCRIL_EVT_CM_PH_BASE                         = 0x11000,
  QCRIL_EVT_CM_PH_OPRT_MODE                    = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_OPRT_MODE,
  QCRIL_EVT_CM_PH_INFO                         = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_INFO, 
  QCRIL_EVT_CM_PH_SYS_SEL_PREF                 = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_SYS_SEL_PREF,
  #ifdef FEATURE_QCRIL_PRL_INIT
  QCRIL_EVT_CM_PH_PRL_INIT                     = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_PRL_INIT,
  #endif /* FEATURE_QCRIL_PRL_INIT */
  QCRIL_EVT_CM_PH_SUBSCRIPTION_AVAILABLE       = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_SUBSCRIPTION_AVAILABLE,
  #ifdef FEATURE_QCRIL_SUBS_CTRL
  QCRIL_EVT_CM_PH_SUBSCRIPTION_NOT_AVAILABLE   = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_SUBSCRIPTION_NOT_AVAILABLE,
  #endif /* FEATURE_QCRIL_SUBS_CTRL */
  #ifdef FEATURE_QCRIL_DSDS
  QCRIL_EVT_CM_PH_DUAL_STANDBY_PREF            = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_DUAL_STANDBY_PREF,
  QCRIL_EVT_CM_PH_SUBSCRIPTION_PREF_INFO       = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_SUBSCRIPTION_PREF_INFO,
  #endif /* FEATURE_QCRIL_DSDS */
  QCRIL_EVT_CM_PH_AVAILABLE_NETWORKS_CONF      = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_AVAILABLE_NETWORKS_CONF,
  QCRIL_EVT_CM_PH_TERMINATE_GET_NETWORKS       = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_TERMINATE_GET_NETWORKS,
  QCRIL_EVT_CM_PH_NVRUIM_CONFIG_CHANGED        = QCRIL_EVT_CM_PH_BASE + CM_PH_EVENT_NVRUIM_CONFIG_CHANGED,

  QCRIL_EVT_CM_SS_BASE                         = 0x12000,
  QCRIL_EVT_CM_SS_SRV_CHANGED                  = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_SRV_CHANGED,
  QCRIL_EVT_CM_SS_RSSI                         = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_RSSI,
  QCRIL_EVT_CM_SS_INFO                         = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_INFO,
  QCRIL_EVT_CM_SS_REG_REJECT                   = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_REG_REJECT,
  QCRIL_EVT_CM_SS_HDR_RSSI                     = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_HDR_RSSI,
  QCRIL_EVT_CM_SS_EMERG_NUM_LIST               = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_EMERG_NUM_LIST,
  #ifdef FEATURE_QCRIL_DSAC  
  QCRIL_EVT_CM_SS_CELL_ACCESS_IND              = QCRIL_EVT_CM_SS_BASE + CM_SS_EVENT_CELL_ACCESS_IND,
  #endif /* FEATURE_QCRIL_DSAC */

  QCRIL_EVT_CM_CALL_BASE                       = 0x13000,
  QCRIL_EVT_CM_CALL_ORIG                       = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_ORIG,
  QCRIL_EVT_CM_CALL_ANSWER                     = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_ANSWER,
  QCRIL_EVT_CM_CALL_END                        = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_END,
  QCRIL_EVT_CM_CALL_SUPS                       = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_SUPS,
  QCRIL_EVT_CM_CALL_INCOM                      = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_INCOM,
  QCRIL_EVT_CM_CALL_CONNECT                    = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CONNECT,
  QCRIL_EVT_CM_CALL_PRIVACY                    = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_PRIVACY,
  QCRIL_EVT_CM_CALL_PRIVACY_PREF               = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_PRIVACY_PREF,
  QCRIL_EVT_CM_CALL_CALLER_ID                  = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALLER_ID,
  QCRIL_EVT_CM_CALL_SIGNAL                     = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_SIGNAL,
  QCRIL_EVT_CM_CALL_DISPLAY                    = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_DISPLAY,
  QCRIL_EVT_CM_CALL_CALLED_PARTY               = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALLED_PARTY,
  QCRIL_EVT_CM_CALL_CONNECTED_NUM              = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CONNECTED_NUM,
  QCRIL_EVT_CM_CALL_EXT_DISP                   = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_EXT_DISP,
  QCRIL_EVT_CM_CALL_EXT_BRST_INTL              = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_EXT_BRST_INTL,
  QCRIL_EVT_CM_CALL_NSS_CLIR_REC               = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_NSS_CLIR_REC,
  QCRIL_EVT_CM_CALL_NSS_REL_REC                = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_NSS_REL_REC,
  QCRIL_EVT_CM_CALL_NSS_AUD_CTRL               = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_NSS_AUD_CTRL,
  QCRIL_EVT_CM_CALL_MNG_CALLS_CONF             = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_MNG_CALLS_CONF,
  QCRIL_EVT_CM_CALL_BARRED                     = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_BARRED,
  QCRIL_EVT_CM_CALL_ON_HOLD                    = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_ON_HOLD,
  QCRIL_EVT_CM_CALL_IS_WAITING                 = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_IS_WAITING,
  QCRIL_EVT_CM_CALL_RETRIEVED                  = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_RETRIEVED,
  QCRIL_EVT_CM_CALL_ORIG_FWD_STATUS            = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_ORIG_FWD_STATUS,
  QCRIL_EVT_CM_CALL_FORWARDED                  = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_FORWARDED,
  QCRIL_EVT_CM_CALL_BEING_FORWARDED            = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_BEING_FORWARDED,
  QCRIL_EVT_CM_CALL_INCOM_FWD_CALL             = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_INCOM_FWD_CALL,
  QCRIL_EVT_CM_CALL_RESTRICTED                 = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_RESTRICTED,
  QCRIL_EVT_CM_CALL_CUG_INFO_RECEIVED          = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CUG_INFO_RECEIVED,
  QCRIL_EVT_CM_CALL_SETUP_IND                  = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_SETUP_IND,
  QCRIL_EVT_CM_CALL_USER_DATA_IND              = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_USER_DATA_IND,
  QCRIL_EVT_CM_CALL_PROGRESS_INFO_IND          = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_PROGRESS_INFO_IND,
  QCRIL_EVT_CM_CALL_DEFLECTION                 = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CALL_DEFLECTION,
  QCRIL_EVT_CM_CALL_TRANSFERRED_CALL           = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_TRANSFERRED_CALL,
  QCRIL_EVT_CM_CALL_CNAP_INFO_RECEIVED         = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_CNAP_INFO_RECEIVED,
  QCRIL_EVT_CM_CALL_OTASP_STATUS               = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_OTASP_STATUS,
  QCRIL_EVT_CM_CALL_REDIRECTING_NUMBER         = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_REDIRECTING_NUMBER,
  QCRIL_EVT_CM_CALL_LINE_CTRL                  = QCRIL_EVT_CM_CALL_BASE + CM_CALL_EVENT_LINE_CTRL,

  QCRIL_EVT_CM_INBAND_BASE                     = 0x14000,
  QCRIL_EVT_CM_INBAND_REV_BURST_DTMF           = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_REV_BURST_DTMF,
  QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_REV_START_CONT_DTMF,
  QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF_CONF = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_REV_START_CONT_DTMF_CONF,
  QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF  = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_REV_STOP_CONT_DTMF,
  QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF_CONF  = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_REV_STOP_CONT_DTMF_CONF,
  QCRIL_EVT_CM_INBAND_FWD_BURST_DTMF           = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_FWD_BURST_DTMF,
  QCRIL_EVT_CM_INBAND_FWD_START_CONT_DTMF      = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_FWD_START_CONT_DTMF,
  QCRIL_EVT_CM_INBAND_FWD_STOP_CONT_DTMF       = QCRIL_EVT_CM_INBAND_BASE + CM_INBAND_EVENT_FWD_STOP_CONT_DTMF,
  QCRIL_EVT_CM_SUPS_BASE                       = 0x15000,
  QCRIL_EVT_CM_SUPS_REGISTER                   = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_REGISTER,
  QCRIL_EVT_CM_SUPS_REGISTER_CONF              = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_REGISTER_CONF,
  QCRIL_EVT_CM_SUPS_ERASE                      = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_ERASE,
  QCRIL_EVT_CM_SUPS_ERASE_CONF                 = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_ERASE_CONF,
  QCRIL_EVT_CM_SUPS_ACTIVATE                   = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_ACTIVATE,
  QCRIL_EVT_CM_SUPS_ACTIVATE_CONF              = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_ACTIVATE_CONF,
  QCRIL_EVT_CM_SUPS_DEACTIVATE                 = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_DEACTIVATE,
  QCRIL_EVT_CM_SUPS_DEACTIVATE_CONF            = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_DEACTIVATE_CONF,
  QCRIL_EVT_CM_SUPS_INTERROGATE                = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_INTERROGATE,
  QCRIL_EVT_CM_SUPS_INTERROGATE_CONF           = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_INTERROGATE_CONF,
  QCRIL_EVT_CM_SUPS_REG_PASSWORD               = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_REG_PASSWORD,
  QCRIL_EVT_CM_SUPS_REG_PASSWORD_CONF          = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_REG_PASSWORD_CONF,
  QCRIL_EVT_CM_SUPS_PROCESS_USS_CONF           = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_PROCESS_USS_CONF,
  QCRIL_EVT_CM_SUPS_EVENT_FWD_CHECK_IND        = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_FWD_CHECK_IND,  
  QCRIL_EVT_CM_SUPS_USS_NOTIFY_IND             = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_USS_NOTIFY_IND,
  QCRIL_EVT_CM_SUPS_USS_IND                    = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_USS_IND,
  QCRIL_EVT_CM_SUPS_RELEASE_USS_IND            = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_RELEASE_USS_IND,
  QCRIL_EVT_CM_SUPS_GET_PASSWORD_IND           = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_GET_PASSWORD_IND,
  QCRIL_EVT_CM_SUPS_PROCESS_USS           = QCRIL_EVT_CM_SUPS_BASE + CM_SUPS_EVENT_PROCESS_USS,

  QCRIL_EVT_CM_STATS_BASE                      = 0x16000,
  #ifdef FEATURE_QCRIL_NCELL
  QCRIL_EVT_CM_STATS_MODEM_INFO                = QCRIL_EVT_CM_STATS_BASE + CM_STATS_EVENT_MODEM_INFO,
  #endif /* FEATURE_QCRIL_NCELL */

  /* Other QCRIL sub-modules to QCRIL(CM) events */
  QCRIL_EVT_CM_INTERNAL_BASE                   = 0x17000,
  QCRIL_EVT_CM_CARD_STATUS_UPDATED,
  #ifdef FEATURE_QCRIL_SUBS_CTRL
  QCRIL_EVT_CM_ENABLE_SUBSCRIPTION,
  QCRIL_EVT_CM_DISABLE_SUBSCRIPTION,
  #endif /* FEATURE_QCRIL_SUBS_CTRL */
  QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS,
  QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS,
  QCRIL_EVT_CM_UPDATE_FDN_STATUS,
  QCRIL_EVT_CM_MAX,

  /* AMSS(SMS) to QCRIL(SMS) events */
  QCRIL_EVT_SMS_BASE                         = 0x20000,
  QCRIL_EVT_SMS_COMMAND_CALLBACK,
  QCRIL_EVT_SMS_CFG_BASE                     = 0x21000,
  #ifdef FEATURE_QCRIL_DSDS
  QCRIL_EVT_SMS_CFG_MS_MEMORY_FULL           = QCRIL_EVT_SMS_CFG_BASE + WMS_CFG_EVENT_MS_MEMORY_FULL,
  QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET     = QCRIL_EVT_SMS_CFG_BASE + WMS_CFG_EVENT_MS_MEMORY_STATUS_SET,
  #else
  QCRIL_EVT_SMS_CFG_MEMORY_FULL              = QCRIL_EVT_SMS_CFG_BASE + WMS_CFG_EVENT_MEMORY_FULL,
  QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET        = QCRIL_EVT_SMS_CFG_BASE + WMS_CFG_EVENT_MEMORY_STATUS_SET,
  #endif /* FEATURE_QCRIL_DSDS */
  QCRIL_EVT_SMS_CFG_MESSAGE_LIST             = QCRIL_EVT_SMS_CFG_BASE + WMS_CFG_EVENT_MESSAGE_LIST,
  QCRIL_EVT_SMS_MSG_BASE                     = 0x22000,
  QCRIL_EVT_SMS_SEND                         = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_SEND,
  QCRIL_EVT_SMS_WRITE                        = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_WRITE,
  QCRIL_EVT_SMS_DELETE                       = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_DELETE,
  QCRIL_EVT_SMS_RECEIVED_MESSAGE             = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_RECEIVED_MESSAGE,
  QCRIL_EVT_SMS_SUBMIT_RPT                   = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_SUBMIT_REPORT,
  QCRIL_EVT_SMS_STATUS_RPT                   = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_STATUS_REPORT,
  QCRIL_EVT_SMS_READ_TEMPLATE                = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_READ_TEMPLATE,
  QCRIL_EVT_SMS_WRITE_TEMPLATE               = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_WRITE_TEMPLATE,
  #ifdef FEATURE_QCRIL_IMS
  QCRIL_EVT_SMS_TRANSPORT_REG                = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_TRANSPORT_REG,
  #endif /* FEATURE_QCRIL_IMS */
  #ifdef FEATURE_QCRIL_WMS_ETWS
  QCRIL_EVT_SMS_ETWS_NOTIFICATION            = QCRIL_EVT_SMS_MSG_BASE + WMS_MSG_EVENT_ETWS_NOTIFICATION,
  #endif /* FEATURE_QCRIL_WMS_ETWS */
  QCRIL_EVT_SMS_BC_MM_BASE                   = 0x23000,
  QCRIL_EVT_SMS_BC_MM_PREF                   = QCRIL_EVT_SMS_BC_MM_BASE + WMS_BC_MM_EVENT_PREF,
  QCRIL_EVT_SMS_BC_MM_TABLE                  = QCRIL_EVT_SMS_BC_MM_BASE + WMS_BC_MM_EVENT_TABLE,
  QCRIL_EVT_SMS_BC_MM_ADD_SRVS               = QCRIL_EVT_SMS_BC_MM_BASE + WMS_BC_MM_EVENT_ADD_SRVS,

  /* Other QCRIL components to QCRIL(SMS) events */
  QCRIL_EVT_SMS_INTERNAL_BASE                = 0x24000,
  QCRIL_EVT_SMS_SUBSCRIPTION_PREF_INFO,
  QCRIL_EVT_SMS_MAX,

  /* AMSS(UIM) to QCRIL(UIM) events */
  QCRIL_EVT_MMGSDI_BASE                        = 0x30000,

  #ifndef FEATURE_QCRIL_UIM_QMI
  QCRIL_EVT_MMGSDI_COMMAND_CALLBACK,
  QCRIL_EVT_MMGSDI_EVENT_CALLBACK,
  QCRIL_EVT_MMGSDI_GSDI_COMMAND_CALLBACK,
  QCRIL_EVT_MMGSDI_PERSO_EVENT_CALLBACK,
  QCRIL_EVT_MMGSDI_INTERNAL_VERIFY_PIN_COMMAND_CALLBACK,
  #else
  QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK,
  QCRIL_EVT_UIM_QMI_INDICATION,
  #endif
  QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK,

  /* Other QCRIL components to QCRIL(UIM) events */
  QCRIL_EVT_MMGSDI_INTERNAL_BASE              = 0x31000,
  #ifndef FEATURE_QCRIL_UIM_QMI
  QCRIL_EVT_INTERNAL_MMGSDI_VERIFY_PIN_COMMAND_CALLBACK,
  #else
  QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK, 
  #endif
  QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP,
  QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN,
  QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS,
  QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS,
  QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS,
  QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS,
  QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE,
  QCRIL_EVT_INTERNAL_MMGSDI_READ_UST_VALUE,
  QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS, 
  QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS, 
  QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START,
  QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE,
  QCRIL_EVT_MMGSDI_MAX,

  /* AMSS(GSTK) to QCRIL(GSTK) events */
  QCRIL_EVT_GSTK_BASE                          = 0x40000,

  #ifdef FEATURE_QCRIL_QMI_CAT
  QCRIL_EVT_GSTK_QMI_CAT_INDICATION,
  QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK,
  QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK,
  #else
  QCRIL_EVT_GSTK_COMMAND_CALLBACK,
  QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK,
  QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK,
  QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK,
  QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK,
  #endif /* FEATURE_QCRIL_QMI_CAT */

  /* Other QCRIL components to QCRIL(GSTK) events */
  QCRIL_EVT_GSTK_INTERNAL_BASE                 = 0x41000,
  #ifdef FEATURE_QCRIL_QMI_CAT
  QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY,
  #else
  QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY,
  #endif /* FEATURE_QCRIL_QMI_CAT */
  QCRIL_EVT_GSTK_MAX,                        

  /* AMSS(Data) to QCRIL(Data) events */
  QCRIL_EVT_DATA_BASE                          = 0x50000,
  QCRIL_EVT_DATA_COMMAND_CALLBACK,
  QCRIL_EVT_DATA_EVENT_CALLBACK,
  QCRIL_EVT_DATA_WDS_EVENT_CALLBACK,
  QCRIL_EVT_DATA_MAX,

  /* AMSS(PBM) to QCRIL(PBM) events */
  QCRIL_EVT_PBM_BASE                           = 0x60000,
  QCRIL_EVT_PBM_REFRESH_START                  = QCRIL_EVT_PBM_BASE + PBM_EVENT_PB_REFRESH_START,
  QCRIL_EVT_PBM_REFRESH_DONE                   = QCRIL_EVT_PBM_BASE + PBM_EVENT_PB_REFRESH_DONE,
  QCRIL_EVT_PBM_PB_READY                       = QCRIL_EVT_PBM_BASE + PBM_EVENT_PB_READY,
  QCRIL_EVT_PBM_SIM_INIT_DONE                  = QCRIL_EVT_PBM_BASE + PBM_EVENT_SIM_INIT_DONE,
  #if defined(FEATURE_QCRIL_FUSION) || defined(FEATURE_QCRIL_DSDS)
  QCRIL_EVT_PBM_SESSION_INIT_DONE              = QCRIL_EVT_PBM_BASE + PBM_EVENT_SESSION_INIT_DONE,
  #endif /* FEATURE_QCRIL_FUSION || FEATURE_QCRIL_DSDS */

  /* Other QCRIL components to QCRIL(PBM) events */
  QCRIL_EVT_PBM_INTERNAL_BASE                  = 0x61000,
  QCRIL_EVT_PBM_CARD_INSERTED,
  QCRIL_EVT_PBM_CARD_INIT_COMPLETED,
  QCRIL_EVT_PBM_CARD_ERROR,
  QCRIL_EVT_PBM_UPDATE_OTA_ECC_LIST,
  QCRIL_EVT_PBM_MAX,

  QCRIL_EVT_OTHER_BASE                         = 0x70000,
  QCRIL_EVT_OTHER_MAX,

  /* OEM Hook events */
  QCRIL_EVT_HOOK_BASE                          = 0x80000,
  QCRIL_EVT_HOOK_NV_READ                       = QCRIL_EVT_HOOK_BASE + 1,
  QCRIL_EVT_HOOK_NV_WRITE                      = QCRIL_EVT_HOOK_BASE + 2,
  QCRIL_EVT_HOOK_DATA_GO_DORMANT               = QCRIL_EVT_HOOK_BASE + 3,
  QCRIL_EVT_HOOK_ME_DEPERSONALIZATION          = QCRIL_EVT_HOOK_BASE + 4,
  QCRIL_EVT_HOOK_SET_TUNE_AWAY                 = QCRIL_EVT_HOOK_BASE + 5,
  QCRIL_EVT_HOOK_GET_TUNE_AWAY                 = QCRIL_EVT_HOOK_BASE + 6,
  QCRIL_EVT_HOOK_SET_PAGING_PRIORITY           = QCRIL_EVT_HOOK_BASE + 7,
  QCRIL_EVT_HOOK_GET_PAGING_PRIORITY           = QCRIL_EVT_HOOK_BASE + 8,
  QCRIL_EVT_HOOK_UNSOL_EXTENDED_DBM_INTL       = QCRIL_EVT_HOOK_BASE + 1000,
  QCRIL_EVT_HOOK_UNSOL_CDMA_BURST_DTMF         = QCRIL_EVT_HOOK_BASE + 1001,
  QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_START    = QCRIL_EVT_HOOK_BASE + 1002,
  QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_STOP     = QCRIL_EVT_HOOK_BASE + 1003,
  QCRIL_EVT_HOOK_UNSOL_CALL_EVT_PROGRESS_INFO_IND = QCRIL_EVT_HOOK_BASE + 1004,
  QCRIL_EVT_HOOK_MAX,

  QCRIL_EVT_OEM_BASE                           = 0x90000,
  QCRIL_EVT_OEM_MAX                            = 0x9ffff,

  QCRIL_EVT_NONE                               = 0xfffff /* Internal use only */
} qcril_evt_e_type;

/* Payload of RIL request or AMSS event */
typedef struct
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  int event_id;
  void *data;
  size_t datalen;
  RIL_Token t;
} qcril_request_params_type;

/* Payload of RIL Request Response */
typedef struct
{
  qcril_instance_id_e_type instance_id;
  RIL_Token t;
  int request_id;
  RIL_Errno ril_err_no;
  void *resp_pkt;
  size_t resp_len;
  const char *logstr;
} qcril_request_resp_params_type;

/* Payload of RIL Unsolicited Response */
typedef struct
{
  int response_id;
  void *resp_pkt;
  size_t resp_len;
  const char *logstr;
  uint8 instance_id;
} qcril_unsol_resp_params_type;

/* Timed Callback information */
struct qcril_timed_callback_info;
typedef struct qcril_timed_callback_info qcril_timed_callback_info;
struct qcril_timed_callback_info
{
  uint32 timer_id;
  RIL_TimedCallback callback;
  qcril_timed_callback_info *next;
};

/* Time services information */
typedef enum {
	QCRIL_TIME_BASE_GSM,
	QCRIL_TIME_BASE_CDMA,
	QCRIL_TIME_BASE_USER,
	QCRIL_TIME_BASE_LAST = QCRIL_TIME_BASE_USER
} qcril_time_base;

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

errno_enum_type qcril_cm_init( void );
void qcril_cm_release( void );
boolean qcril_cm_screen_is_off( qcril_instance_id_e_type instance_id );
void qcril_cm_get_modem_capability( void );
void qcril_cm_process_network_info( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                    qcril_ssic_notification_status_e_type ssic_notification_status );
errno_enum_type qcril_sms_init( void );
void qcril_sms_release( void );

#ifdef FEATURE_QCRIL_UIM_QMI
void qcril_uim_init( void );
void qcril_uim_release( void );
#else
void qcril_mmgsdi_init( void );
void qcril_mmgsdi_release( void );
#endif /* FEATURE_QCRIL_UIM_QMI */

#ifdef FEATURE_QCRIL_QMI_CAT
void qcril_gstk_qmi_init( void );
#else
void qcril_gstk_init( void );
#endif /* FEATURE_QCRIL_QMI_CAT */

void qcril_data_init();
void qcril_other_init( void );
void qcril_other_mute( qcril_instance_id_e_type instance_id, boolean mic_mute, boolean ear_mute );
void qcril_pbm_init( void );
void qcril_pbm_release( void );
void qcril_event_init( void );
void qcril_event_start( void );
IxErrnoType qcril_event_queue( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, qcril_data_src_e_type data_src,
                        qcril_evt_e_type event_id, void *data, size_t datalen, RIL_Token t );

errno_enum_type qcril_process_event( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id,
                                     qcril_evt_e_type event_id, void *data, size_t datalen, RIL_Token t );
int qcril_setup_timed_callback( qcril_instance_id_e_type instance_id, qcril_modem_id_e_type modem_id, RIL_TimedCallback callback,
                                   const struct timeval *relativeTime, uint32 *timer_id );
int qcril_cancel_timed_callback( void *param );
int qcril_timed_callback_active( uint32 timer_id );
void qcril_default_request_resp_params( qcril_instance_id_e_type instance_id, RIL_Token t, int request_id, RIL_Errno err_no,  
                                       qcril_request_resp_params_type *param_ptr );
void qcril_send_request_response( qcril_request_resp_params_type *param_ptr );
void qcril_default_unsol_resp_params( qcril_instance_id_e_type instance_id, int response_id, qcril_unsol_resp_params_type *param );
void qcril_send_unsol_response( qcril_unsol_resp_params_type *param_ptr );
void qcril_hook_unsol_response( qcril_instance_id_e_type instance_id, uint32 unsol_event, char *data, uint32 data_len );
void *qcril_malloc( size_t size );
void qcril_free( void *mem_ptr );
void qcril_release( void );
int ril_to_uim_is_dsds_enabled( void );
int ril_to_uim_is_tsts_enabled(void);
#define QCRIL_EXTERN( xxx_request ) \
    void qcril_##xxx_request ( const qcril_request_params_type *const params_ptr, \
                             qcril_request_return_type *const ret_ptr)

int qcril_get_time(qcril_time_base base, struct timeval *tv);

void qcril_common_update_current_imsi( char * imsi_str, int is_gwl );

/* Data Services */
QCRIL_EXTERN (data_request_setup_data_call);
QCRIL_EXTERN (data_request_deactivate_data_call);
QCRIL_EXTERN (data_request_last_data_call_fail_cause);
QCRIL_EXTERN (data_request_data_call_list);
QCRIL_EXTERN (data_request_omh_profile_info);
QCRIL_EXTERN (data_command_hdlr);
QCRIL_EXTERN (data_event_hdlr);
QCRIL_EXTERN (data_request_set_data_profile);
#ifdef FEATURE_QCRIL_USE_NETCTRL
QCRIL_EXTERN (data_wds_event_hdlr);
#endif /*FEATURE_QCRIL_USE_NETCTRL*/
QCRIL_EXTERN (data_process_qcrilhook_go_dormant);

/* PBM events internal, external handling*/
QCRIL_EXTERN (pbm_event_handler);
QCRIL_EXTERN (pbm_event_card_state_changed);
QCRIL_EXTERN (pbm_update_ota_ecc_list);

/* CM Services */
QCRIL_EXTERN (cm_phonesvc_request_set_preferred_network_type);
QCRIL_EXTERN (cm_phonesvc_request_get_preferred_network_type);
QCRIL_EXTERN (cm_phonesvc_request_set_network_selection_manual);
QCRIL_EXTERN (cm_phonesvc_request_set_network_selection_automatic);
QCRIL_EXTERN (cm_phonesvc_request_query_network_selection_mode);
QCRIL_EXTERN (cm_phonesvc_request_set_band_mode);
QCRIL_EXTERN (cm_phonesvc_request_query_available_band_mode);
QCRIL_EXTERN (cm_phonesvc_request_query_available_networks);
QCRIL_EXTERN (cm_phonesvc_request_radio_power);
QCRIL_EXTERN (cm_phonesvc_request_reset_radio);
QCRIL_EXTERN (cm_phonesvc_request_cdma_set_subscription_source);  
QCRIL_EXTERN (cm_phonesvc_request_cdma_set_roaming_preference);
QCRIL_EXTERN (cm_phonesvc_request_cdma_query_roaming_preference);
QCRIL_EXTERN (cm_phonesvc_request_exit_emergency_callback_mode);
QCRIL_EXTERN (cm_phonesvc_request_cdma_get_subscription_source);
QCRIL_EXTERN (cm_phonesvc_request_cdma_subscription);

#ifdef FEATURE_QCRIL_DSDS
QCRIL_EXTERN (cm_phonesvc_request_set_uicc_subscription);
QCRIL_EXTERN (cm_phonesvc_request_set_data_subscription);
QCRIL_EXTERN (cm_phonesvc_request_get_uicc_subscription);
QCRIL_EXTERN (cm_phonesvc_request_get_data_subscription);
QCRIL_EXTERN (cm_phonesvc_request_set_subscription_mode);
QCRIL_EXTERN (cm_phonesvc_request_set_tune_away);
QCRIL_EXTERN (cm_phonesvc_request_get_tune_away);
QCRIL_EXTERN (cm_phonesvc_request_set_paging_priority);
QCRIL_EXTERN (cm_phonesvc_request_get_paging_priority);
#endif /* FEATURE_QCRIL_DSDS */
QCRIL_EXTERN (cm_srvsys_request_voice_radio_tech);
QCRIL_EXTERN (cm_srvsys_request_operator);
QCRIL_EXTERN (cm_srvsys_request_signal_strength);
QCRIL_EXTERN (cm_srvsys_request_registration_state);
QCRIL_EXTERN (cm_srvsys_request_data_registration_state);
QCRIL_EXTERN (cm_srvsys_request_get_neighboring_cell_ids);
QCRIL_EXTERN (cm_srvsys_request_screen_state);
QCRIL_EXTERN (cm_srvsys_request_set_location_updates);
QCRIL_EXTERN (cm_callsvc_request_dial);
QCRIL_EXTERN (cm_callsvc_request_answer);
QCRIL_EXTERN (cm_callsvc_request_hangup);
QCRIL_EXTERN (cm_callsvc_request_get_current_calls );
QCRIL_EXTERN (cm_callsvc_request_last_call_fail_cause);
QCRIL_EXTERN (cm_callsvc_request_set_tty_mode);
QCRIL_EXTERN (cm_callsvc_request_query_tty_mode);
QCRIL_EXTERN (cm_callsvc_request_cdma_set_preferred_voice_privacy_mode);
QCRIL_EXTERN (cm_callsvc_request_cdma_query_preferred_voice_privacy_mode);
QCRIL_EXTERN (cm_callsvc_request_cdma_flash); 
QCRIL_EXTERN (cm_inbandsvc_request_dtmf);
QCRIL_EXTERN (cm_inbandsvc_request_dtmf_start);
QCRIL_EXTERN (cm_inbandsvc_request_dtmf_stop);
QCRIL_EXTERN (cm_inbandsvc_request_cdma_burst_dtmf);
QCRIL_EXTERN (cm_inbandsvc_event_rev_burst_dtmf);
QCRIL_EXTERN (cm_inbandsvc_event_rev_start_cont_dtmf);
QCRIL_EXTERN (cm_inbandsvc_event_rev_start_cont_dtmf_conf);
QCRIL_EXTERN (cm_inbandsvc_event_rev_stop_cont_dtmf);
QCRIL_EXTERN (cm_inbandsvc_event_rev_stop_cont_dtmf_conf);
QCRIL_EXTERN (cm_inbandsvc_event_fwd_burst_dtmf);
QCRIL_EXTERN (cm_inbandsvc_event_fwd_start_cont_dtmf);
QCRIL_EXTERN (cm_inbandsvc_event_fwd_stop_cont_dtmf);
QCRIL_EXTERN (cm_supsvc_request_udub);
QCRIL_EXTERN (cm_supsvc_request_conference);
QCRIL_EXTERN (cm_supsvc_request_separate_connection);
QCRIL_EXTERN (cm_supsvc_request_explicit_call_transfer);
QCRIL_EXTERN (cm_supsvc_request_switch_waiting_or_holding_and_active);
QCRIL_EXTERN (cm_supsvc_request_hangup_waiting_or_background);
QCRIL_EXTERN (cm_supsvc_request_hangup_foreground_resume_background);
QCRIL_EXTERN (cm_supsvc_request_query_clip);
QCRIL_EXTERN (cm_supsvc_request_set_clir);
QCRIL_EXTERN (cm_supsvc_request_get_clir);
QCRIL_EXTERN (cm_supsvc_request_set_call_forward);
QCRIL_EXTERN (cm_supsvc_request_query_call_waiting);
QCRIL_EXTERN (cm_supsvc_request_set_call_waiting);
QCRIL_EXTERN (cm_supsvc_request_query_call_forward_status);
QCRIL_EXTERN (cm_supsvc_request_change_barring_password);
QCRIL_EXTERN (cm_supsvc_request_set_facility_lock);
QCRIL_EXTERN (cm_supsvc_request_query_facility_lock);
QCRIL_EXTERN (cm_supsvc_request_send_ussd);
QCRIL_EXTERN (cm_supsvc_request_cancel_ussd);
QCRIL_EXTERN (cm_supsvc_request_set_supp_svc_notification);
QCRIL_EXTERN (cm_event_command_callback);
QCRIL_EXTERN (cm_phonesvc_event_oprt_mode);
QCRIL_EXTERN (cm_phonesvc_event_info);
QCRIL_EXTERN (cm_phonesvc_event_sys_sel_pref);
#ifdef FEATURE_QCRIL_DSDS
QCRIL_EXTERN (cm_phonesvc_event_dual_standby_pref);
QCRIL_EXTERN (cm_phonesvc_event_subscription_pref_info);
#endif /* FEATURE_QCRIL_DSDS */
QCRIL_EXTERN (cm_phonesvc_event_prl_init);
QCRIL_EXTERN (cm_phonesvc_event_subscription_available);
#ifdef FEATURE_QCRIL_SUBS_CTRL
QCRIL_EXTERN (cm_phonesvc_event_subscription_not_available);
#endif /* FEATURE_QCRIL_SUBS_CTRL */
QCRIL_EXTERN (cm_phonesvc_event_available_networks_conf);
QCRIL_EXTERN (cm_phonesvc_event_terminate_get_networks);
QCRIL_EXTERN (cm_phonesvc_event_nvruim_config_changed);
QCRIL_EXTERN (cm_srvsys_event_reg_reject);
QCRIL_EXTERN (cm_srvsys_event_info);
QCRIL_EXTERN (cm_srvsys_event_rssi);
QCRIL_EXTERN (cm_srvsys_event_srv_changed);
QCRIL_EXTERN (cm_srvsys_event_emerg_num_list);
#ifdef FEATURE_QCRIL_DSAC
QCRIL_EXTERN (cm_srvsys_event_cell_access_ind);
#endif /* FEATURE_QCRIL_DSAC */
QCRIL_EXTERN (cm_callsvc_event_orig);
QCRIL_EXTERN (cm_callsvc_event_setup_ind);
QCRIL_EXTERN (cm_callsvc_event_user_data_ind);
QCRIL_EXTERN (cm_callsvc_event_progress_info_ind);
QCRIL_EXTERN (cm_callsvc_event_incom);
QCRIL_EXTERN (cm_callsvc_event_answer);
QCRIL_EXTERN (cm_callsvc_event_end);
QCRIL_EXTERN (cm_callsvc_event_connect);
QCRIL_EXTERN (cm_callsvc_event_sups);
QCRIL_EXTERN (cm_callsvc_event_privacy);
QCRIL_EXTERN (cm_callsvc_event_privacy_pref);
QCRIL_EXTERN (cm_callsvc_event_caller_id);
QCRIL_EXTERN (cm_callsvc_event_signal);
QCRIL_EXTERN (cm_callsvc_event_display);
QCRIL_EXTERN (cm_callsvc_event_called_party);
QCRIL_EXTERN (cm_callsvc_event_connected_num);
QCRIL_EXTERN (cm_callsvc_event_ext_display);
QCRIL_EXTERN (cm_callsvc_event_nss_clir);
QCRIL_EXTERN (cm_callsvc_event_nss_rel);
QCRIL_EXTERN (cm_callsvc_event_nss_aud_ctrl);
QCRIL_EXTERN (cm_callsvc_event_redirecting_number);
QCRIL_EXTERN (cm_callsvc_event_line_ctrl);
QCRIL_EXTERN (cm_callsvc_event_mng_calls_conf);
QCRIL_EXTERN (cm_callsvc_event_orig_fwd_status);
QCRIL_EXTERN (cm_callsvc_event_call_being_forwarded);
QCRIL_EXTERN (cm_callsvc_event_call_is_waiting);
QCRIL_EXTERN (cm_callsvc_event_call_barred);
QCRIL_EXTERN (cm_callsvc_event_call_restricted);
QCRIL_EXTERN (cm_callsvc_event_incom_fwd_call);
QCRIL_EXTERN (cm_callsvc_event_cug_info_received);
QCRIL_EXTERN (cm_callsvc_event_call_on_hold);
QCRIL_EXTERN (cm_callsvc_event_call_retrieved);
QCRIL_EXTERN (cm_callsvc_event_call_forwarded);
QCRIL_EXTERN (cm_callsvc_event_transferred_call);
QCRIL_EXTERN (cm_callsvc_event_call_deflection);
QCRIL_EXTERN (cm_callsvc_event_cnap_info_received);
QCRIL_EXTERN (cm_callsvc_event_otasp_status);
QCRIL_EXTERN (cm_supsvc_event_ack);
QCRIL_EXTERN (cm_supsvc_event_activate_conf);
QCRIL_EXTERN (cm_supsvc_event_deactivate_conf);
QCRIL_EXTERN (cm_supsvc_event_erase_conf);
QCRIL_EXTERN (cm_supsvc_event_interrogate_conf);
QCRIL_EXTERN (cm_supsvc_event_register_conf);
QCRIL_EXTERN (cm_supsvc_event_reg_password_conf);
QCRIL_EXTERN (cm_supsvc_event_process_uss);
QCRIL_EXTERN (cm_supsvc_event_process_uss_conf);
QCRIL_EXTERN (cm_supsvc_event_fwd_check_ind);
QCRIL_EXTERN (cm_supsvc_event_uss_notify_ind);
QCRIL_EXTERN (cm_supsvc_event_uss_ind);
QCRIL_EXTERN (cm_supsvc_event_release_uss_ind);
QCRIL_EXTERN (cm_supsvc_event_get_password_ind);
QCRIL_EXTERN (cm_callsvc_event_ext_brst_intl);
QCRIL_EXTERN (cm_stats_event_modem_info);
QCRIL_EXTERN (cm_event_card_status_updated);
#ifdef FEATURE_QCRIL_SUBS_CTRL
QCRIL_EXTERN (cm_event_enable_subscription);
QCRIL_EXTERN (cm_event_disable_subscription);
#endif /* FEATURE_QCRIL_SUBS_CTRL */
#ifdef FEATURE_QCRIL_DSDS
QCRIL_EXTERN (cm_event_activate_provision_status);
QCRIL_EXTERN (cm_event_deactivate_provision_status);
#endif /* FEATURE_QCRIL_DSDS */
QCRIL_EXTERN (cm_event_update_fdn_status);

/* UIM/SIM Toolkit */
#ifdef FEATURE_QCRIL_QMI_CAT
QCRIL_EXTERN (gstk_qmi_process_qmi_indication);
QCRIL_EXTERN (gstk_qmi_process_notify_ril_is_ready);
QCRIL_EXTERN (gstk_qmi_request_stk_get_profile);
QCRIL_EXTERN (gstk_qmi_request_stk_set_profile);
QCRIL_EXTERN (gstk_qmi_request_stk_send_envelope_command);
QCRIL_EXTERN (gstk_qmi_request_stk_send_terminal_response);
QCRIL_EXTERN (gstk_qmi_request_stk_handle_call_setup_requested_from_sim);
QCRIL_EXTERN (gstk_qmi_request_stk_service_is_running);
QCRIL_EXTERN (gstk_qmi_process_raw_command_callback);
QCRIL_EXTERN (gstk_qmi_process_qmi_response);
#else
QCRIL_EXTERN (gstk_process_send_raw_envelope_callback);
QCRIL_EXTERN (gstk_process_notify_ril_is_ready);
QCRIL_EXTERN (gstk_process_command_callback);
QCRIL_EXTERN (gstk_process_client_init_callback);
QCRIL_EXTERN (gstk_process_client_reg_callback);
QCRIL_EXTERN (gstk_process_raw_command_callback);
QCRIL_EXTERN (gstk_process_event_callback);
QCRIL_EXTERN (gstk_request_stk_get_profile);
QCRIL_EXTERN (gstk_request_stk_set_profile);
QCRIL_EXTERN (gstk_request_stk_send_envelope_command);
QCRIL_EXTERN (gstk_request_stk_send_terminal_response);
QCRIL_EXTERN (gstk_request_stk_handle_call_setup_requested_from_sim);
QCRIL_EXTERN (gstk_request_stk_service_is_running);
#endif /* FEATURE_QCRIL_QMI_CAT */

#ifdef FEATURE_QCRIL_UIM_QMI
QCRIL_EXTERN (uim_process_qmi_callback);
QCRIL_EXTERN (uim_process_qmi_indication);
QCRIL_EXTERN (uim_process_internal_command);
QCRIL_EXTERN (uim_process_imsi_callback);
QCRIL_EXTERN (uim_request_get_sim_status);
QCRIL_EXTERN (uim_request_enter_pin);
QCRIL_EXTERN (uim_request_enter_puk);
QCRIL_EXTERN (uim_request_change_pin);
QCRIL_EXTERN (uim_request_enter_perso_key);
QCRIL_EXTERN (uim_request_get_imsi);
QCRIL_EXTERN (uim_request_sim_io);
#else
QCRIL_EXTERN (mmgsdi_process_command_callback);
QCRIL_EXTERN (mmgsdi_process_imsi_command_callback);
QCRIL_EXTERN (mmgsdi_process_internal_verify_pin_command_callback);
QCRIL_EXTERN (mmgsdi_process_event_callback);
QCRIL_EXTERN (mmgsdi_process_gsdi_command_callback);
QCRIL_EXTERN (mmgsdi_process_perso_event_callback);
QCRIL_EXTERN (mmgsdi_process_internal_command);
QCRIL_EXTERN (mmgsdi_request_get_fdn_status);
QCRIL_EXTERN (mmgsdi_request_set_fdn_status);
QCRIL_EXTERN (mmgsdi_request_get_pin_status);
QCRIL_EXTERN (mmgsdi_request_set_pin_status);
QCRIL_EXTERN (mmgsdi_request_get_sim_status);
QCRIL_EXTERN (mmgsdi_request_enter_pin);
QCRIL_EXTERN (mmgsdi_request_enter_puk);
QCRIL_EXTERN (mmgsdi_request_change_pin);
QCRIL_EXTERN (mmgsdi_request_enter_perso_key);
QCRIL_EXTERN (mmgsdi_request_get_imsi);
QCRIL_EXTERN (mmgsdi_request_sim_io);
QCRIL_EXTERN (mmgsdi_request_oem_hook_me_depersonalization);
QCRIL_EXTERN (mmgsdi_process_fdn_record_update_from_pbm);
QCRIL_EXTERN (mmgsdi_process_internal_read_ust_callback);
#endif /* FEATURE_QCRIL_UIM_QMI */

/* SMS (WMS) */
QCRIL_EXTERN (sms_request_send_sms);
QCRIL_EXTERN (sms_request_send_sms_expect_more);
QCRIL_EXTERN (sms_request_sms_acknowledge);
QCRIL_EXTERN (sms_request_write_sms_to_sim);
QCRIL_EXTERN (sms_request_delete_sms_on_sim);
QCRIL_EXTERN (sms_request_get_smsc_address);
QCRIL_EXTERN (sms_request_set_smsc_address);
QCRIL_EXTERN (sms_request_report_sms_memory_status);
QCRIL_EXTERN (sms_request_gsm_get_broadcast_sms_config);
QCRIL_EXTERN (sms_request_gsm_set_broadcast_sms_config);
QCRIL_EXTERN (sms_request_gsm_sms_broadcast_activation);
QCRIL_EXTERN (sms_request_cdma_send_sms);
QCRIL_EXTERN (sms_request_cdma_sms_acknowledge);
QCRIL_EXTERN (sms_request_cdma_write_sms_to_ruim);
QCRIL_EXTERN (sms_request_cdma_delete_sms_on_ruim);
QCRIL_EXTERN (sms_request_cdma_get_broadcast_sms_config);
QCRIL_EXTERN (sms_request_cdma_set_broadcast_sms_config);
QCRIL_EXTERN (sms_request_cdma_sms_broadcast_activation);
QCRIL_EXTERN (sms_request_ims_registration_state);
QCRIL_EXTERN (sms_request_ims_send_sms);
QCRIL_EXTERN (sms_command_event_callback);
QCRIL_EXTERN (sms_msg_event_send);
QCRIL_EXTERN (sms_msg_event_submit_report);
QCRIL_EXTERN (sms_msg_event_write);
QCRIL_EXTERN (sms_msg_event_delete);
QCRIL_EXTERN (sms_msg_event_read_template);
QCRIL_EXTERN (sms_msg_event_write_template);
QCRIL_EXTERN (sms_msg_event_received_message);
QCRIL_EXTERN (sms_msg_event_status_report);
QCRIL_EXTERN (sms_msg_event_transport_reg);
QCRIL_EXTERN (sms_msg_event_etws_notification);
QCRIL_EXTERN (sms_cfg_event_message_list);
QCRIL_EXTERN (sms_cfg_event_memory_full);
QCRIL_EXTERN (sms_cfg_event_mem_status_set);
QCRIL_EXTERN (sms_bc_mm_event_table);
QCRIL_EXTERN (sms_bc_mm_event_add_services);
QCRIL_EXTERN (sms_bc_mm_event_pref);
QCRIL_EXTERN (sms_subscription_pref_info);


/* Other RIL services */
QCRIL_EXTERN (other_request_get_imei);
QCRIL_EXTERN (other_request_get_imeisv);
QCRIL_EXTERN (other_request_set_mute);
QCRIL_EXTERN (other_request_get_mute);
QCRIL_EXTERN (other_request_baseband_version);
QCRIL_EXTERN (other_request_oem_hook_strings);
QCRIL_EXTERN (other_request_oem_hook_nv_read);
QCRIL_EXTERN (other_request_oem_hook_nv_write);
QCRIL_EXTERN (other_request_device_identity);
QCRIL_EXTERN (other_request_cdma_validate_and_write_akey);
QCRIL_EXTERN (other_request_cdma_subscription);

/* PBM */
QCRIL_EXTERN (pbm_event_handler);
QCRIL_EXTERN (pbm_event_card_state_changed);
QCRIL_EXTERN (pbm_event_update_ota_ecc_list);

/* SAR */
#ifdef FEATURE_QCRIL_SAR
QCRIL_EXTERN (sar_request_set_transmit_power);
#endif /* FEATURE_QCRIL_SAR */

#endif /* QCRILI_H */
