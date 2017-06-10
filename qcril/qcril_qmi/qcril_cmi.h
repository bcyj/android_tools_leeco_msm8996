/******************************************************************************
  @file    qcril_cmi.h
  @brief   qcril qmi - compatibility layer for CM

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_CMI_H
#define QCRIL_CMI_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <pthread.h>
#include "comdef.h"
#include "ril.h"
#include "qcrili.h"
#include "qcril_log.h"


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/* CLIR Persistent System Property */
#define QCRIL_CM_CLIR                         "persist.radio.clir"

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

/* Maximum size of RIL responses */
#define QCRIL_CM_RESPONSE_GPRS_REGISTRATION_STATE_MAX_SIZE  4
#define QCRIL_CM_RESPONSE_NITZ_TIME_RXED_MAX_SIZE          28
#define QCRIL_CM_RESPONSE_OPERATOR_MAX_SIZE                 3
#define QCRIL_CM_RESPONSE_REGISTRATION_STATE_MAX_SIZE      14
#define QCRIL_CM_RESPONSE_AVAILABLE_BAND_MODE_MAX_SIZE     19

/* Signal strength unknown */
#define QCRIL_CM_GW_SIGNAL_STRENGTH_UNKNOWN 99
#define QCRIL_CM_SIGNAL_STRENGTH_UNKNOWN    -1

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

/*! @brief TTY Mode
*/
typedef enum
{
  QCRIL_CM_TTY_MODE_OFF = 0,    // TTY Off
  QCRIL_CM_TTY_MODE_FULL  = 1,  // TTY Full
  QCRIL_CM_TTY_MODE_HCO = 2,    // TTY Hearing Carryover
  QCRIL_CM_TTY_MODE_VCO = 3,    // TTY Voice Carryover
  QCRIL_CM_TTY_MODE_MIN = QCRIL_CM_TTY_MODE_OFF,
  QCRIL_CM_TTY_MODE_MAX = QCRIL_CM_TTY_MODE_VCO
} qcril_cm_tty_mode_e_type;


#endif /* QCRIL_CMI_H */
