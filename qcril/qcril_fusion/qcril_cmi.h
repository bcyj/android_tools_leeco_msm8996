/*!
  @file
  qcril_cmi.h

  @brief

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_cmi.h#16 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/01/10   fc      Re-architecture to support split modem.
02/25/10   sb      Drop MT SMS if mobile is in Emergency Callback state.
11/12/09   sb      Added sanity timer to info rec processing.
09/21/09   pg      Fixed data type mismatched issue in Rgistration State info
                   response.
06/15/09   nd      Added support for CDMA Time of Day.
06/16/09   fc      Removed ecc_mutex.
06/06/09   nrn     Adding support for Authentication and Registration Reject
06/02/09   fc      Remove the ECC structure.
05/29/09   fc      Added support for FTM mode.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/28/09   fc      Added support to perform card powerup/powerdown for
                   LPM to ONLINE or vice versa transition. 
04/05/09   fc      Cleanup log macros.
04/01/09   fc      Fixed the issue in cell selection failure caused by
                   redundant call to cm_ph_cmd_sys_sel_pref(). 
03/17/09   fc      Added ONS support for NITZ.
02/09/09   fc      Changed the power up sequence to start with Radio State 
                   Unavailable, to command CM to LPM and subscription 
                   unavailable, and to wait for the very first CM_PH_EVENT_INFO 
                   before reading RTRE configuration and transition to Radio
                   State Off.
01/30/09   pg      Get band/mode capability from CM_PH_EVENT_INFO.
01/26/08   fc      Logged assertion info.
01/14/06   fc      Changes to report "Limited Service" as "No Service" in
                   (GPRS) Registration State payload.
12/16/08   fc      Changes to support the release of AMSS CM object for ONCRPC.
                   and to consolidate internal data structure. 
10/28/08   pg      Changed QCRIL_REQUEST_OPERATOR API.
10/23/08   pg      Added call failed reasons. 
06/09/08   pg      Fixed some bugs.
05/28/08   pg      Added basic call services support.
05/07/08   fc      First cut implementation.

===========================================================================*/

#ifndef QCRIL_CMI_H
#define QCRIL_CMI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include "comdef.h"
#include "cm.h"
#include "nv.h"
#include "ril.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_cm.h"
#include "qcril_cm_ons.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* CLIR Persistent System Property */ 
#define QCRIL_CM_CLIR                         "persist.radio.clir" 

/* TUNE AWAY Persistent System Property */
#define QCRIL_TUNE_AWAY                       "persist.radio.tuneaway"

/* PAGING PRIORITY Persistent System Property */
#define QCRIL_PAGING_PRIORITY                       "persist.radio.paging_priority"

/* ECIO Delta System Property */
#define QCRIL_ECIO_DELTA                      "persist.radio.ecio_delta"

/* Airplane Mode SIM Not Powerdown System Property */
#define QCRIL_APM_SIM_NOT_PWDN                "persist.radio.apm_sim_not_pwdn"

/* Power Saving System Property */
#define QCRIL_ADD_POWER_SAVE                      "persist.radio.add_power_save"

/* Process USSD from other clients Property */
#define QCRIL_PROCESS_USSD_FROM_OTHER_CLIENTS     "persist.radio.process_ussd"

/* Bitmask represents the preferred network setting 
   Bit 1 - 1X
   Bit 2 - EVDO
   Bit 3 - GSM
   Bit 4 - WCDMA
   Bit 5 - LTE
*/
#define QCRIL_CM_PREF_NETWORK_MASK_1X      SYS_SYS_MODE_MASK_CDMA
#define QCRIL_CM_PREF_NETWORK_MASK_EVDO    SYS_SYS_MODE_MASK_HDR
#define QCRIL_CM_PREF_NETWORK_MASK_GSM     SYS_SYS_MODE_MASK_GSM
#define QCRIL_CM_PREF_NETWORK_MASK_WCDMA   SYS_SYS_MODE_MASK_WCDMA
#define QCRIL_CM_PREF_NETWORK_MASK_LTE     SYS_SYS_MODE_MASK_LTE

#define QCRIL_CM_PREF_NETWORK_MASK_1XEVDO        ( QCRIL_CM_PREF_NETWORK_MASK_1X | QCRIL_CM_PREF_NETWORK_MASK_EVDO )
#define QCRIL_CM_PREF_NETWORK_MASK_GW            ( QCRIL_CM_PREF_NETWORK_MASK_GSM | QCRIL_CM_PREF_NETWORK_MASK_WCDMA )
#define QCRIL_CM_PREF_NETWORK_MASK_GW_1XEVDO     ( QCRIL_CM_PREF_NETWORK_MASK_GW | QCRIL_CM_PREF_NETWORK_MASK_1XEVDO )
#define QCRIL_CM_PREF_NETWORK_MASK_LTE_1XEVDO    ( QCRIL_CM_PREF_NETWORK_MASK_LTE | QCRIL_CM_PREF_NETWORK_MASK_1XEVDO )
#define QCRIL_CM_PREF_NETWORK_MASK_LTE_GW        ( QCRIL_CM_PREF_NETWORK_MASK_LTE | QCRIL_CM_PREF_NETWORK_MASK_GW )
#define QCRIL_CM_PREF_NETWORK_MASK_LTE_GW_1XEVDO ( QCRIL_CM_PREF_NETWORK_MASK_LTE_GW | QCRIL_CM_PREF_NETWORK_MASK_1XEVDO )

#ifdef FEATURE_QCRIL_DSDS
#define QCRIL_CM_SS_MOBILITY_MGMT_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_MOBILITY_MGMT_MASK : CM_SS_MOBILITY_MGMT_MASK )
#else
#define QCRIL_CM_SS_MOBILITY_MGMT_MASK( gw_hybrid )       CM_SS_MOBILITY_MGMT_MASK 
#endif /* FEATURE_QCRIL_DSDS */

#ifdef FEATURE_QCRIL_DSDS
#define QCRIL_CM_SS_TRUE_SRV_STATUS_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_TRUE_SRV_STATUS_MASK : CM_SS_TRUE_SRV_STATUS_MASK )
#else
#define QCRIL_CM_SS_TRUE_SRV_STATUS_MASK( gw_hybrid )     CM_SS_TRUE_SRV_STATUS_MASK 
#endif /* FEATURE_QCRIL_DSDS */

#define QCRIL_CM_SS_SRV_STATUS_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_SRV_STATUS_MASK : CM_SS_SRV_STATUS_MASK )

#define QCRIL_CM_SS_SYS_MODE_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_SYS_MODE_MASK : CM_SS_SYS_MODE_MASK )

#define QCRIL_CM_SS_SYS_ID_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_SYS_ID_MASK : CM_SS_SYS_ID_MASK )

#define QCRIL_CM_SS_SRV_DOMAIN_MASK( gw_hybrid )   \
   ( gw_hybrid  ? CM_SS_GW_SRV_DOMAIN_MASK : CM_SS_SRV_DOMAIN_MASK )

#define QCRIL_CM_SS_SRV_CAPABILITY_MASK( gw_hybrid )   \
   ( gw_hybrid  ? CM_SS_GW_SRV_CAPABILITY_MASK : CM_SS_SRV_CAPABILITY_MASK )

#define QCRIL_CM_SS_ROAM_STATUS_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_ROAM_STATUS_MASK : CM_SS_ROAM_STATUS_MASK )

#ifdef FEATURE_QCRIL_DSDS
#define QCRIL_CM_SS_SRV_IND_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_SRV_IND_MASK : CM_SS_SRV_IND_MASK )
#else
#define QCRIL_CM_SS_SRV_IND_MASK( gw_hybrid )     CM_SS_SRV_IND_MASK 
#endif /* FEATURE_QCRIL_DSDS */

#ifdef FEATURE_QCRIL_DSDS
#define QCRIL_CM_SS_GW_CELL_INFO_MASK( gw_hybrid )   \
   ( gw_hybrid ? CM_SS_GW_CELL_INFO_MASK : CM_SS_CELL_INFO_MASK )
#else
#define QCRIL_CM_SS_GW_CELL_INFO_MASK( gw_hybrid )     CM_SS_CELL_INFO_MASK 
#endif /* FEATURE_QCRIL_DSDS */


/* Determine whether the mode capability is GSM only */
#define QCRIL_CM_MODE_CAPABILITY_SUPPORT_GSM( mode_capability )                                      \
  ( mode_capability & SYS_SYS_MODE_MASK_GSM )  

/* Determine whether the mode capability is WCDMA only */
#define QCRIL_CM_MODE_CAPABILITY_SUPPORT_WCDMA( mode_capability )                                    \
  ( mode_capability & SYS_SYS_MODE_MASK_WCDMA )

/* Determine whether the mode capability is LTE only */
#define QCRIL_CM_MODE_CAPABILITY_SUPPORT_LTE( mode_capability )                                    \
  ( mode_capability & SYS_SYS_MODE_MASK_LTE )

/* Determine whether the mode capability is CDMA only */
#define QCRIL_CM_MODE_CAPABILITY_SUPPORT_CDMA( mode_capability )                                     \
  ( mode_capability & SYS_SYS_MODE_MASK_CDMA )

/* Determine whether the mode capability is HDR only */
#define QCRIL_CM_MODE_CAPABILITY_SUPPORT_HDR( mode_capability )                                      \
  ( mode_capability & SYS_SYS_MODE_MASK_HDR )    

/* Determine whether the service domain supports PS */
#define QCRIL_CM_SRV_DOMAIN_SUPPORT_PS( srv_domain )                                                 \
  ( ( srv_domain == SYS_SRV_DOMAIN_PS_ONLY ) || ( srv_domain == SYS_SRV_DOMAIN_CS_PS ) )

/* Determine whether the service capability supports PS */
#define QCRIL_CM_SRV_CAPABILITY_SUPPORT_PS( srv_capability )                                         \
  ( ( srv_capability == SYS_SRV_DOMAIN_PS_ONLY ) || ( srv_capability == SYS_SRV_DOMAIN_CS_PS ) )

/* Determine whether the service domain supports CS */
#define QCRIL_CM_SRV_DOMAIN_SUPPORT_CS( srv_domain )                                                 \
  ( ( srv_domain == SYS_SRV_DOMAIN_CS_ONLY ) || ( srv_domain == SYS_SRV_DOMAIN_CS_PS ) )

/* Determine whether the service capability supports CS */
#define QCRIL_CM_SRV_CAPABILITY_SUPPORT_CS( srv_capability )                                         \
  ( ( srv_capability == SYS_SRV_DOMAIN_CS_ONLY ) || ( srv_capability == SYS_SRV_DOMAIN_CS_PS ) )

/* Determine whether the PLMN preference is wildcard */
#define QCRIL_CM_PLMN_PREFERENCE_IS_WILDCARD( plmn1, plmn2, plmn3 )                                  \
  ( ( plmn1 == 0xFF) && ( plmn2 == 0xFF) && ( plmn3 == 0xFF ) )

#ifdef FEATURE_QCRIL_DSDS
/* Determine whether the service domain supports CS */
#define QCRIL_CM_DUAL_STANDBY_PREF_NAME( standby_pref )                                                 \
   ( (standby_pref == SYS_MODEM_DS_PREF_DUAL_STANDBY) ? "dual standby - tune away enabled" : "dual standby - tune away disabled" )

#define QCRIL_CM_DUAL_STANDBY_PREF( tune_away )                                                         \
   ( (tune_away == TRUE) ? SYS_MODEM_DS_PREF_DUAL_STANDBY : SYS_MODEM_DS_PREF_DUAL_STANDBY_NO_TUNEAWAY )
#endif

/* Maximum number of networks */
#define QCRIL_CM_MAX_NETWORKS                               2

/* Maximum length of LAC (hexdecimal format) ASCII representation */
#define QCRIL_CM_LAC_ASCII_MAX_LEN                          7

/* Maximum length of CID (hexdecimal format) ASCII representation */
#define QCRIL_CM_CID_ASCII_MAX_LEN                          9                

/* Maximum length of BASE ID (hexdecimal format) ASCII representation */
#define QCRIL_CM_BASE_ID_ASCII_MAX_LEN                      5

/* Maximum length of BS Latitude (hexdecimal format) ASCII representation */
#define QCRIL_CM_BASE_LATITUDE_ASCII_MAX_LEN                9

/* Maximum length of BS Longitude (hexdecimal format) ASCII representation */
#define QCRIL_CM_BASE_LONGITUDE_ASCII_MAX_LEN               9

/* Maximum length of SID ASCII representation */
#define QCRIL_CM_SID_ASCII_MAX_LEN                          6

/* Maximum length of NID ASCII representation */
#define QCRIL_CM_NID_ASCII_MAX_LEN                          6

/* Maximum length of Roaming Status ASCII representation */
#define QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN                  4

/* Maximum length of PRL version */
#define QCRIL_CM_PRL_VERSION_ASCII_MAX_LEN                  6

/* Maximum length of PSC */
#define QCRIL_CM_PSC_HEX_MAX_LEN                  4

/* Maximum size of RIL responses */
#define QCRIL_CM_RESPONSE_GPRS_REGISTRATION_STATE_MAX_SIZE  4
#define QCRIL_CM_RESPONSE_NITZ_TIME_RXED_MAX_SIZE          28
#define QCRIL_CM_RESPONSE_OPERATOR_MAX_SIZE                 3
#define QCRIL_CM_RESPONSE_REGISTRATION_STATE_MAX_SIZE      15
#define QCRIL_CM_RESPONSE_DATA_REGISTRATION_STATE_MAX_SIZE 6
#define QCRIL_CM_RESPONSE_AVAILABLE_BAND_MODE_MAX_SIZE     19

/* Signal strength unknown */
#define QCRIL_CM_LTE_SIGNAL_STRENGTH_UNKNOWN           -32767
#define QCRIL_CM_GW_SIGNAL_STRENGTH_UNKNOWN                99
#define QCRIL_CM_SIGNAL_STRENGTH_UNKNOWN                   -1

/* Signal strength delta values */
#define QCRIL_CM_RSSI_DELTA_DEFAULT          5
#define QCRIL_CM_ECIO_DELTA_DEFAULT          2
#define QCRIL_CM_IO_DELTA_DEFAULT            5
#define QCRIL_CM_ECIO_DELTA_MAX            255

/* Signal level conversion result codes */
#define QCRIL_CM_RSSI_TOOLO_CODE   0
#define QCRIL_CM_RSSI_TOOHI_CODE   31

/* RSSI range conversion */
#define QCRIL_CM_RSSI_MIN        51   /* per 3GPP 27.007  (negative value) */
#define QCRIL_CM_RSSI_MAX        113  /* per 3GPP 27.007  (negative value) */
#define QCRIL_CM_RSSI_NO_SIGNAL  125  
#define QCRIL_CM_RSSI_OFFSET     182.26
#define QCRIL_CM_RSSI_SLOPE      (-100.0/62.0)

/* Local Ringback tone */
#define QCRIL_CM_REMOTE_ALERT    3

/* User Resumed inband signal from NW to stop local ringback tone */
#define QCRIL_CM_USER_RESUMED    1

/* Maximum size of the alpha buffer (from ril.h) */
#define QCRIL_CM_MAX_ALPHA_BUF_SIZE 64

/* Cause for No CLI values as per 3GPP 24.008 */
#define QCRIL_CM_NO_CLI_CAUSE_UNAVAILABLE                     0
#define QCRIL_CM_NO_CLI_CAUSE_REJECT_BY_USER                  1
#define QCRIL_CM_NO_CLI_CAUSE_INTERACTION_WITH_OTHER_SERVICE  2
#define QCRIL_CM_NO_CLI_CAUSE_PAYPHONE                        3

/* Number presentation values (from ril.h) */
#define QCRIL_CM_NUM_PRESENTATION_RESTRICTED    1
#define QCRIL_CM_NUM_PRESENTATION_UNKNOWN       2
#define QCRIL_CM_NUM_PRESENTATION_PAYPHONE      3

/* OEM HOOK DTMF forward burst payload length (72 bytes)
onlength   : 4 bytes 
off_len    : 4 bytes
dtmf_digits: 64 bytes */ 
#define QCRIL_CM_OEM_DTMF_FWD_BURST_PAYLOAD_LENGTH 72

/* Maximum number of wcdma monitored set info to store */
#define QCRIL_CM_MAX_WCDMA_MONITORED 6

#define REG_STATE_SEARCHING_EMERGENCY "12"
#define REG_STATE_DENIED_EMERGENCY "13"
#define REG_STATE_UNKNOWN_EMERGENCY "14"

/*! @brief FTM Mode
*/
typedef enum
{
  QCRIL_CM_FTM_MODE_OFF     = 0,
  QCRIL_CM_FTM_MODE_ON      = 1,
  QCRIL_CM_FTM_MODE_UNKNOWN = 2
} qcril_cm_ftm_mode_e_type;

/*! @brief Radio Power Mode
*/
typedef enum
{
  QCRIL_CM_RADIO_POWER_MODE_LPM    = 0,
  QCRIL_CM_RADIO_POWER_MODE_ONLINE = 1,
  QCRIL_CM_RADIO_POWER_MODE_OFF    = 2
} qcril_cm_radio_power_mode_e_type;

/*! @brief Network Type Preference
*/
typedef enum
{
  QCRIL_CM_NETWORK_PREF_GW_PREFERRED_WCDMA = 0,
  QCRIL_CM_NETWORK_PREF_GSM_ONLY           = 1,
  QCRIL_CM_NETWORK_PREF_WCDMA_ONLY         = 2,
  QCRIL_CM_NETWORK_PREF_GW_AUTO            = 3,
  QCRIL_CM_NETWORK_PREF_CDMA_HDR_AUTO      = 4,
  QCRIL_CM_NETWORK_PREF_CDMA_ONLY          = 5,
  QCRIL_CM_NETWORK_PREF_HDR_ONLY           = 6,
  QCRIL_CM_NETWORK_PREF_DIGITAL            = 7
} qcril_cm_network_pref_e_type;

/*! @brief Network Selection Mode Preference
*/
typedef enum
{
  QCRIL_CM_NETWORK_SEL_MODE_PREF_AUTOMATIC = 0,
  QCRIL_CM_NETWORK_SEL_MODE_PREF_MANUAL    = 1
} qcril_cm_network_sel_mode_pref_e_type;

/*! @brief GW Band Mode
*/
typedef enum
{
  QCRIL_CM_BAND_MODE_UNSPECIFIED = 0,
  QCRIL_CM_BAND_MODE_EURO        = 1,
  QCRIL_CM_BAND_MODE_US          = 2,
  QCRIL_CM_BAND_MODE_JPN         = 3,
  QCRIL_CM_BAND_MODE_AUS         = 4,
  QCRIL_CM_BAND_MODE_AUS2        = 5,
  QCRIL_CM_BAND_MODE_CELL        = 6,
  QCRIL_CM_BAND_MODE_PCS         = 7,
  QCRIL_CM_BAND_MODE_BC3         = 8,
  QCRIL_CM_BAND_MODE_BC4         = 9,
  QCRIL_CM_BAND_MODE_BC5         = 10,
  QCRIL_CM_BAND_MODE_BC6         = 11,
  QCRIL_CM_BAND_MODE_BC7         = 12,
  QCRIL_CM_BAND_MODE_BC8         = 13,
  QCRIL_CM_BAND_MODE_BC9         = 14,
  QCRIL_CM_BAND_MODE_BC10        = 15,
  QCRIL_CM_BAND_MODE_BC11        = 16,
  QCRIL_CM_BAND_MODE_BC15        = 17,
  QCRIL_CM_BAND_MODE_BC16        = 18
} qcril_cm_band_mode_e_type;

#define QCRIL_CM_BAND_PREF_ANY  CM_BAND_PREF_ANY

#define QCRIL_CM_BAND_PREF_EURO \
  ( CM_BAND_PREF_GSM_EGSM_900 |    \
    CM_BAND_PREF_GSM_PGSM_900 |    \
    CM_BAND_PREF_GSM_RGSM_900 |    \
    CM_BAND_PREF_GSM_DCS_1800 |    \
    CM_BAND_PREF_WCDMA_I_IMT_2000 )

#define QCRIL_CM_BAND_PREF_US \
  ( CM_BAND_PREF_GSM_850      |  \
    CM_BAND_PREF_GSM_PCS_1900 |  \
    CM_BAND_PREF_WCDMA_V_850  |  \
    CM_BAND_PREF_WCDMA_II_PCS_1900 )

#define QCRIL_CM_BAND_PREF_JPN \
  ( CM_BAND_PREF_WCDMA_VI_800 |   \
    CM_BAND_PREF_WCDMA_I_IMT_2000 )

#define QCRIL_CM_BAND_PREF_AUS \
  ( CM_BAND_PREF_GSM_EGSM_900 |   \
    CM_BAND_PREF_GSM_PGSM_900 |   \
    CM_BAND_PREF_GSM_RGSM_900 |   \
    CM_BAND_PREF_GSM_DCS_1800 |   \
    CM_BAND_PREF_WCDMA_V_850  |   \
    CM_BAND_PREF_WCDMA_I_IMT_2000 )

#define QCRIL_CM_BAND_PREF_AUS2 \
  ( CM_BAND_PREF_GSM_EGSM_900 |    \
    CM_BAND_PREF_GSM_PGSM_900 |    \
    CM_BAND_PREF_GSM_RGSM_900 |    \
    CM_BAND_PREF_GSM_DCS_1800 |    \
    CM_BAND_PREF_WCDMA_V_850 )

/*! @brief Network Status
*/
typedef enum
{
  QCRIL_CM_NETWORK_STATUS_UNKNOWN   = 0,
  QCRIL_CM_NETWORK_STATUS_AVAILABLE = 1,
  QCRIL_CM_NETWORK_STATUS_CURRENT   = 2,
  QCRIL_CM_NETWORK_STATUS_FORBIDDEN = 3,
  QCRIL_CM_NETWORK_STATUS_MAX       
} qcril_cm_network_status_e_type;

/*! @brief Roaming Preference
*/
typedef enum
{
  QCRIL_CM_ROAMING_PREF_HOME_NETWORK_ONLY       = 0,
  QCRIL_CM_ROAMING_PREF_AFFILIATED_NETWORK_ONLY = 1,
  QCRIL_CM_ROAMING_PREF_ANY_NETWORK             = 2
} qcril_cm_roaming_pref_e_type;

/* @brief Subscription Preference 
*/
typedef enum
{
  QCRIL_CM_SUBSCRIPTION_PREF_RUIM = 0,
  QCRIL_CM_SUBSCRIPTION_PREF_NV = 1
} qcril_cm_subscription_pref_e_type;

#ifdef FEATURE_QCRIL_SUBS_CTRL
/*! @brief Subscription State
*/
typedef enum
{
  QCRIL_CM_SUBSCRIPTION_DISABLED = 0,
  QCRIL_CM_SUBSCRIPTION_ENABLED  = 1
} qcril_cm_subscription_state_e_type;
#endif /* FEATURE_QCRIL_SUBS_CTRL */

/*! @brief Screen State
*/
typedef enum
{
  QCRIL_CM_SCREEN_STATE_OFF = 0,
  QCRIL_CM_SCREEN_STATE_ON  = 1
} qcril_cm_screen_state_e_type;

/*! @brief TTY Mode
*/
typedef enum
{
  QCRIL_CM_TTY_MODE_OFF = 0, /* TTY Off */
  QCRIL_CM_TTY_MODE_FULL  = 1,  /* TTY Full */
  QCRIL_CM_TTY_MODE_HCO = 2, /* TTY Hearing Carryover */
  QCRIL_CM_TTY_MODE_VCO = 3 /* TTY Voice Carryover */
} qcril_cm_tty_mode_e_type;

/*! @brief Voice Privacy Mode 
*/
typedef enum
{
  QCRIL_CM_VOICE_PRIVACY_MODE_STANDARD = 0, /* Standard voice privacy */
  QCRIL_CM_VOICE_PRIVACY_MODE_ENHANCED = 1  /* Enhanced voice privacy */
} qcril_cm_voice_privacy_mode_e_type;

/*! @brief CM Command Type
*/
typedef enum
{
  QCRIL_CM_COMMAND_PH     = 0, /* CM phone services command */
  QCRIL_CM_COMMAND_SS     = 1, /* CM serving system services command */
  QCRIL_CM_COMMAND_CALL   = 2, /* CM call services command */
  QCRIL_CM_COMMAND_INBAND = 3, /* CM inband services command */
  QCRIL_CM_COMMAND_SUPS   = 4  /* CM supplementary services command */
} qcril_cm_command_e_type;

/*! @brief CDMA Dtmf On Length value
*/
typedef enum
{
  QCRIL_CM_DTMF_ON_95 = 0x0,
  /* 95 ms recommended pulse width */

  QCRIL_CM_DTMF_ON_150 = 0x1,
  /* 150 ms recommended pulse width */

  QCRIL_CM_DTMF_ON_200 = 0x2,
  /* 200 ms recommended pulse width */

  QCRIL_CM_DTMF_ON_250 = 0x3,
  /* 250 ms recommended pulse width */

  QCRIL_CM_DTMF_ON_300 = 0x4,
  /* 300 ms recommended pulse width */

  QCRIL_CM_DTMF_ON_350 = 0x5,
  /* 350 ms recommended pulse width */

  QCRIL_CM_DTMF_ON_SMS = 0x6
  /* SMS Tx special pulse width */
} qcril_cm_dtmf_onlength_e_type;

/*! @brief CDMA Dtmf Off Length value
*/
typedef enum
{
  QCRIL_CM_DTMF_OFF_60 = 0x0,
  /* 60 ms recommended minimum interdigit interval */

  QCRIL_CM_DTMF_OFF_100 = 0x1,
  /* 100 ms recommended minimum interdigit interval */

  QCRIL_CM_DTMF_OFF_150 = 0x2,
  /* 150 ms recommended minimum interdigit interval */

  QCRIL_CM_DTMF_OFF_200 = 0x3
  /* 200 ms recommended minimum interdigit interval */
} qcril_cm_dtmf_offlength_e_type;

/*! @brief CM phone command callback parameters */
typedef struct
{
  cm_ph_cmd_e_type     cmd;
  cm_ph_cmd_err_e_type cmd_err;
} qcril_cm_ph_command_callback_params_type;

/*! @brief CM Serving System command callback parameters */
typedef struct
{
  cm_ss_cmd_e_type     cmd;
  cm_ss_cmd_err_e_type cmd_err;
} qcril_cm_ss_command_callback_params_type;

/*! @brief CM Call command callback parameters */
typedef struct
{
  cm_call_cmd_e_type     cmd;
  cm_call_cmd_err_e_type cmd_err;
  #ifdef FEATURE_QCRIL_CALL_ORIG_EXTEN3
  cm_call_cmd_err_cause_e_type err_cause;
  #endif
} qcril_cm_call_command_callback_params_type;

/*! @brief CM Inband command callback parameters */
typedef struct
{
  cm_inband_cmd_e_type     cmd;
  cm_inband_cmd_err_e_type cmd_err;
} qcril_cm_inband_command_callback_params_type;

/*! @brief CM Supplementary Services command callback parameters */
typedef struct
{
  cm_sups_cmd_e_type     cmd;
  cm_sups_cmd_err_e_type cmd_err;
  #ifdef FEATURE_QCRIL_SUPS_CC_EXTEN
  cm_sups_cmd_err_cause_e_type err_cause;
  #endif
} qcril_cm_sups_command_callback_params_type;

/*! @brief CM command callback event parameters 
*/
typedef struct
{
  qcril_cm_command_e_type command;
  union
  {
    qcril_cm_ph_command_callback_params_type ph;
    qcril_cm_ss_command_callback_params_type ss;
    qcril_cm_call_command_callback_params_type call;
    qcril_cm_inband_command_callback_params_type inband;
    qcril_cm_sups_command_callback_params_type sups;
  } command_info;
} qcril_cm_command_callback_params_type;

/*! @brief Response to RIL command 
*/

typedef struct
{
  int available_band_mode[ QCRIL_CM_RESPONSE_AVAILABLE_BAND_MODE_MAX_SIZE ];
} qcril_cm_available_band_mode_type;

typedef struct
{
  char *available_network[ SYS_PLMN_LIST_MAX_LENGTH ][ 4 ];
  char long_eons[ QCRIL_CM_ONS_MAX_LENGTH ];
  char short_eons[ QCRIL_CM_ONS_MAX_LENGTH ];
  char mcc_mnc_ascii[ SYS_PLMN_LIST_MAX_LENGTH ][ QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN ]; 
} qcril_cm_available_network_type;

typedef struct
{
  char *operator[ QCRIL_CM_RESPONSE_OPERATOR_MAX_SIZE ];
  char long_eons[ QCRIL_CM_ONS_MAX_LENGTH ];
  char short_eons[ QCRIL_CM_ONS_MAX_LENGTH ];
  char mcc_mnc_ascii[ QCRIL_CM_ONS_MCC_MNC_ASCII_MAX_LEN ]; 
} qcril_cm_operator_type;

typedef struct
{
  RIL_SignalStrength signal_strength;
} qcril_cm_signal_strength_type;

typedef struct
{
  char *registration_state[ QCRIL_CM_RESPONSE_REGISTRATION_STATE_MAX_SIZE ];
  char lac[ QCRIL_CM_LAC_ASCII_MAX_LEN ];
  char cid[ QCRIL_CM_CID_ASCII_MAX_LEN ];
  char base_id[ QCRIL_CM_BASE_ID_ASCII_MAX_LEN ];
  char base_latitude[ QCRIL_CM_BASE_LATITUDE_ASCII_MAX_LEN ];
  char base_longitude[ QCRIL_CM_BASE_LONGITUDE_ASCII_MAX_LEN ];
  char sid[ QCRIL_CM_SID_ASCII_MAX_LEN ];
  char nid[ QCRIL_CM_NID_ASCII_MAX_LEN ];
  char roam_status[ QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN ];
  char def_roam_ind[ QCRIL_CM_ROAM_STATUS_ASCII_MAX_LEN ];
  char psc[ QCRIL_CM_PSC_HEX_MAX_LEN ];
} qcril_cm_registration_state_type;

#ifdef FEATURE_QCRIL_NCELL
/*! @brief Structure used to compose neighbor cell info response */
typedef struct
{
  RIL_NeighboringCell *neighbor_cell_list[ SYS_ENG_MODE_MAX_NMR_ITEMS ];
  RIL_NeighboringCell neighbor_cell[ SYS_ENG_MODE_MAX_NMR_ITEMS ];
  char cid[ SYS_ENG_MODE_MAX_NMR_ITEMS ][ QCRIL_CM_CID_ASCII_MAX_LEN ];
} qcril_cm_neigh_cells_type;

typedef struct
{
  uint8 num_of_items;
  sys_wcdma_cell_info_s_type strongest[QCRIL_CM_MAX_WCDMA_MONITORED];
} qcril_cm_wcdma_monitored_set_info;
#endif /* FEATURE_QCRIL_NCELL */

typedef struct
{
  char network_time[ QCRIL_CM_RESPONSE_NITZ_TIME_RXED_MAX_SIZE ];
} qcril_cm_network_time_type;

typedef struct
{
  RIL_Call *info_ptr[ CM_CALL_ID_MAX ];
  RIL_Call info[ CM_CALL_ID_MAX ];
  #ifdef FEATURE_QCRIL_UUS
  RIL_UUS_Info uus_info[ CM_CALL_ID_MAX ];
  #endif /* FEATURE_QCRIL_UUS */
  uint32 num_of_calls;
} qcril_cm_current_calls_type;

/*! @brief Structure used to save the information records */
typedef struct
{
  cm_call_id_type call_id;
  RIL_CDMA_InformationRecords recs;
} qcril_cm_cdma_info_rec_type;

/*! @brief Structure used to store MT UUS Data.
*/
typedef struct
{
  cm_call_id_type call_id;
  cm_call_event_user_data_s_type user_data;
} qcril_cm_call_event_user_data_s_type;

/*! @brief Structure used to cache QCRIL CM data
*/

typedef struct 
{
  boolean client_id_is_valid;                                    /*! Indicates the validity of the client ID */
  cm_client_id_type client_id;                                   /*! Client ID registered with CM */
  boolean client_is_primary;                                     /*! Indicates whether this client info is associated with primary QCRIL instance */
} qcril_cm_client_info_type;

typedef struct
{
  #ifdef FEATURE_QCRIL_SUBS_CTRL
  qcril_cm_subscription_state_e_type pri_gw_subscription_state;   /*! GW subscription state */
  qcril_cm_subscription_state_e_type pri_cdma_subscription_state; /*! CDMA subscription state */
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  boolean capability_reported;                                    /*! Indicates whether the phone capability is reported */
  sys_oprt_mode_e_type oprt_mode;                                 /*! Indicate current operating mode */
  sys_sys_mode_mask_e_type mode_capability;                       /*! Mode capability */
  uint64 band_capability;                                         /*! Band capability */
  cm_rtre_config_e_type rtre_config;                              /*! RTRE config */
  cm_rtre_control_e_type rtre_control;                            /*! RTRE control */
  cm_mode_pref_e_type mode_pref;                                  /*! Mode preference */
  cm_gw_acq_order_pref_e_type gw_acq_order_pref;                  /*! GW acquisition order preference */             
  cm_band_pref_e_type band_pref;                                  /*! Band preference */
  cm_network_sel_mode_pref_e_type network_sel_mode_pref;          /*! Network selection mode preference */
  sys_plmn_id_s_type plmn_pref;                                   /*! PLMN preference for manual network selection mode */
  cm_roam_pref_e_type roam_pref;                                  /*! Roam preference */
  boolean prl_pref_only;                                          /*! Indicate PRL's prl_pref only setting */
  word prl_id;                                                    /*! Indicate PRL's prl id */

  #ifdef FEATURE_QCRIL_DSDS
  sys_modem_dual_standby_pref_e_type standby_pref;               /*! Standby preference of the phone. */
  sys_modem_as_id_e_type             active_subs;                /*! The active subscription in Single Standby mode. 
                                                                     Only valid when standby_pref is SYS_MODEM_DS_PREF_SINGLE_STANDBY. */
  sys_modem_as_id_e_type             default_voice_subs;         /*! Default voice subscription. */
  sys_modem_as_id_e_type             default_data_subs;          /*! Default data subscription. */
  sys_modem_as_id_e_type             priority_subs;              /*! Priority subscription. */
  boolean                            tune_away;                 /* tune away settings */
  RIL_SubscriptionType               paging_priority;                     /* paging priority settings */
  #endif /* FEATURE_QCRIL_DSDS */

} qcril_cm_ph_info_type;

typedef struct
{ 
  boolean pwr_oprt_in_progress;                                 /*! Indicates whether the power down or up operation is in progress */
  qcril_card_status_e_type status;                              /*! Card status */
} qcril_cm_card_info_type;

typedef struct
{
  #ifdef FEATURE_QCRIL_SUBS_CTRL
  qcril_cm_subscription_state_e_type pri_gw_subscription_state;   /*! GW subscription state */
  qcril_cm_subscription_state_e_type pri_cdma_subscription_state; /*! CDMA subscription state */
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  qcril_cm_card_info_type card_info[ QMI_UIM_MAX_CARD_COUNT ];            /*! Card status info */
  pthread_mutex_t card_info_mutex;                               /*! Mutex to control access/update to card status */

  boolean report_location_updates;                               /*! Indicates if QCRIL should report network state changes due to changes in
                                                                     LAC and/or CID */

  boolean fdn_enabled;                                           /*! Indicates whether or not FDN check is enabled on SIM */
  pthread_mutex_t fdn_status_mutex;                              /*! Mutex to control access/update to fdn status maintained in fdn_enabled*/

  cm_ph_state_e_type emer_cb_state;                              /*! Emergency callback mode Info */
  pthread_mutex_t emer_cb_state_mutex;                           /*! Mutex to control access/update to emer cb state */

  RIL_LastCallFailCause call_fail_cause;                         /*! Last call fail cause */
  pthread_mutex_t call_fail_cause_mutex;                         /*! Mutex to control access/update to call fail cause */

  qcril_cm_tty_mode_e_type tty_mode;                             /*! TTY mode setting */

  qcril_cm_cdma_info_rec_type cdma_info_recs;                    /*! CDMA information records*/
  uint32 cdma_info_recs_timer_id;                                /*! Sanity Timer ID allocated for CDMA Info Rec processing */
  pthread_mutex_t cdma_info_rec_mutex;                           /*! Mutex to control access/update to CDMA info recs*/

  qcril_cm_call_event_user_data_s_type uus_data;                 /*! MT UUS data */

  qcril_cm_client_info_type client_info[ QCRIL_MAX_MODEM_ID ];   /*! CM Client ID */

  qcril_cm_ph_info_type ph_info[ QCRIL_MAX_MODEM_ID ];           /*! Modem PH Info */
  pthread_mutex_t ph_mutex;                                      /*! Mutex to control access/update to PH info */

  qcril_cm_screen_state_e_type screen_state;                     /*! Screen state */
  uint8 clir;                                                    /*! CLIR setting */
  boolean ecio_delta;                                            /*! ECIO delta report setting */
  boolean apm_sim_not_pwdn;                                      /*! Airplane mode SIM not powerdown setting */

  #ifdef FEATURE_QCRIL_NCELL
  sys_eng_mode_nmr_info_s_type nmr_info;                         /*! Neighbor Cell Info for GSM/UMTS network */
  qcril_cm_wcdma_monitored_set_info monitored_set_info;          /*! Neighbor Cell Info for WCDMA network */ 
  #endif /* FEATURE_QCRIL_NCELL */

  qcril_cm_reg_reject_info_type reg_reject_info;                 /*! GW Registration/ Authentication Reject Information */
  
  #ifdef FEATURE_QCRIL_DSDS
  qcril_cm_subs_pref_info subs_pref_info;                        /*! Subscription info pending to be processed */
  #endif /* FEATURE_QCRIL_DSDS */

  qcril_cm_ss_info_type ss_info[ QCRIL_MAX_MODEM_ID ];           /*! Serving System information */
  pthread_mutex_t ss_mutex;                                      /*! Mutex to control access/update to SS info, NMR, Screen state, CLIR setting */

  boolean power_save_enabled;                                    /* Power Save property */
  boolean process_ussd_from_other_clients;                       /* Process ussd from other clients property */
} qcril_cm_struct_type;

#endif /* QCRIL_CMI_H */
