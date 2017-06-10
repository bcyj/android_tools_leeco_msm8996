/*!
  @file
  qcril_gstk.c

  @brief
  Handles RIL requests and notifications for SIM Toolkit (GSTK).

*/

/*===========================================================================

  Copyright (c) 2009-2011 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_gstk.c#8 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/10/11   yt      Remove bytes indicating BIP support from terminal profile
06/09/10   xz      Disable BIP settings in TP
05/07/10   xz      Fix the issue of hanlding proactive cmd longer than 128 bytes
03/01/10   fc      Re-architecture to support split modem.
10/14/09   xz      Deplay first proactive cmd instead of TP DL
09/14/09   xz      Reponse SUCCESS after process STK_SERVICES_IS_RUNNING
08/06/09   xz      Added qcril_gstk_request_stk_service_is_running()
04/05/09   fc      Clenaup log macros.
02/09/09   xz      Replace qcril_process_event() with qcril_event_queue()
02/05/09   xz      Fix compile issue when FEATURE_CDMA_NON_RUIM is not enabled
01/27/09   xz      Fix issue of sending proactive cmd to RIL too early
01/26/08   fc      Logged assertion info.
01/09/09   tml     Featurize for 1x non ruim build
07/11/08   adg     Initial full version
05/21/08   tml     Bring Logging macro to corresponding modules and Fixed
                   LTK compilation issue
05/19/08   tml     Initial version

===========================================================================*/

/*===========================================================================
                                INCLUDE FILES
===========================================================================*/

#ifndef FEATURE_QCRIL_QMI_CAT

#include <string.h>
#include "ril.h"
#include "IxErrno.h"
#include "qcril_gstki.h"

/*===========================================================================
                        INTERNAL DEFINITIONS AND TYPES
===========================================================================*/

/* Macro to give the number of elements of an array */
#define QCRIL_GSTK_ARRAY_SIZE(a)            (sizeof(a) / sizeof((a)[0]))

/* Hexadecimal digits, for binary <==> hexadecimal conversions */
#define QCRIL_GSTK_HEXDIGITS                "0123456789ABCDEF"

/* Event mask for class B commands - RUN_AT_COMMAND */
#define QCRIL_GSTK_EVT_MASK_CLASSB          (GSTK_REG_RUN_AT_CMDS_EVT)

/* Event mask for class C commands - LAUNCH_BROWSER */
#define QCRIL_GSTK_EVT_MASK_CLASSC          (GSTK_REG_LAUNCH_BROWSER_EVT)    | \
                                             GSTK_REG_SETUP_EVT_BROWSER_TERM_EVT

/* Event mask for class 0 commands which we process fully */
#define QCRIL_GSTK_EVT_MASK_CLASS0_FULL     (GSTK_REG_DISPLAY_TEXT_EVT       | \
                                             GSTK_REG_GET_INKEY_EVT          | \
                                             GSTK_REG_GET_INPUT_EVT          | \
                                             GSTK_REG_PLAY_TONE_EVT          | \
                                             GSTK_REG_SELECT_ITEM_EVT        | \
                                             GSTK_REG_SETUP_MENU_EVT         | \
                                             GSTK_REG_SETUP_IDLE_TEXT_EVT    | \
                                             GSTK_REG_PROVIDE_LANG_INFO_EVT  | \
                                             GSTK_REG_LANG_NOTIFY_EVT        | \
                                             GSTK_REG_SETUP_EVT_USER_ACT_EVT | \
                                             GSTK_REG_SETUP_EVT_IDLE_MENU_EVT| \
                                             GSTK_REG_SETUP_EVT_LANG_SEL_EVT | \
                                             GSTK_REG_EVT_DOWNLOAD_RSP_EVT   | \
                                             GSTK_REG_MENU_SEL_RSP_EVT)

/* Event mask for class D commands for which we only process the UI aspect */
#define QCRIL_GSTK_EVT_MASK_CLASS0_DISPLAY  (GSTK_REG_SEND_SMS_EVT           | \
                                             GSTK_REG_SEND_SS_EVT            | \
                                             GSTK_REG_SEND_USSD_EVT          | \
                                             GSTK_REG_SETUP_CALL_EVT         | \
                                             GSTK_REG_SEND_DTMF_EVT)

/* Event mask for class E commands - BIP */
#define QCRIL_GSTK_EVT_MASK_CLASSE    (GSTK_REG_DATA_EVT               | \
                                       GSTK_REG_SETUP_EVT_DATA_AVAILABLE_EVT | \
                                             GSTK_REG_SETUP_EVT_CH_STATUS_EVT)

/* If any Terminal Profile item is specific to a particular app type */
/* (SIM/USIM/CSIM), the corresponding app type must be specified:    */
/*    QCRIL_STK_APP_NONE  ==> both SAT and USAT                      */
/*    QCRIL_STK_APP_SIM   ==> SAT only                               */
/*    QCRIL_STK_APP_USIM  ==> USAT only                              */
#define QCRIL_STK_APP_NONE                  ((uint8) GSTK_APP_NONE)
#define QCRIL_STK_APP_SIM                   ((uint8) GSTK_APP_SIM)
#define QCRIL_STK_APP_USIM                  ((uint8) GSTK_APP_USIM)

/* Screen dimensions, in characters, for Terminal Profile */
#define QCRIL_STK_SCRN_WIDTH_CHARS          25
#define QCRIL_STK_SCRN_HEIGHT_LINES         10

/* Number of simultaneous BIP channels supported, for Terminal Profile */
#define QCRIL_STK_NUM_BIP_DATA_CH           0

/* Mandatory TLV sizes to help validate proactive command data */
#define QCRIL_GSTK_COMMAND_PROLOGUE_SIZE    2
#define QCRIL_GSTK_COMMAND_DETAILS_SIZE     5
#define QCRIL_GSTK_DEVICE_IDENTITIES_SIZE   4
#define QCRIL_GSTK_COMMAND_MIN_SIZE         (QCRIL_GSTK_COMMAND_PROLOGUE_SIZE + \
                                             QCRIL_GSTK_COMMAND_DETAILS_SIZE  + \
                                             QCRIL_GSTK_DEVICE_IDENTITIES_SIZE)

/* Index within GSTK raw command of the start of the command sent to Android */
/* 0 implies BER-TLV, 2 starts at command tag                                */
#define QCRIL_GSTK_COMMAND_START_INDEX      0

/* Index of Proactive Command Type within command */
#define QCRIL_GSTK_COMMAND_TYPE_INDEX       (5 - QCRIL_GSTK_COMMAND_START_INDEX)

/* Value to be used as the user data argument for GSTK callbacks */
#define QCRIL_GSTK_CB_USER_DATA             ((gstk_client_ref_data_type) &qcril_gstk_info)

/* QCRIL GSTK flag masks */
#define QCRIL_GSTK_RIL_IS_READY_BMSK         0x00000001 
#define QCRIL_GSTK_CLI_INIT_SUCC_BMSK        0x00000002
#define QCRIL_GSTK_HAS_SENT_TP_BMSK          0x00000004

/* Macros to deal with moving command and response data around between: */
/*   Android (which expects data in ASCII hex format), and              */
/*   GSTK (which expects data in binary format)                         */
#define QCRIL_GSTK_COMMAND_DATA_SIZE(n)     (2 * (n))
#define QCRIL_GSTK_COMMAND_BUFFER_SIZE(n)                \
          (sizeof(qcril_gstk_raw_proactive_command_type) \
          + QCRIL_GSTK_COMMAND_DATA_SIZE(n) + 1 )

#define QCRIL_GSTK_COMMAND_COPY(pd, ps, n)               \
          qcril_gstk_bin_to_hexstring((pd), (ps), (n))

#define QCRIL_GSTK_RESPONSE_DATA_SIZE(n)    ((n) / 2)

#define QCRIL_GSTK_RESPONSE_COPY(pd, ps, n)              \
          qcril_gstk_hexstring_to_bin((pd), (ps), (n))

#define QCRIL_GSTK_ENVCMD_DATA_SIZE(n)      ((n) / 2)

#define QCRIL_GSTK_ENVCMD_COPY(pd, ps, n)                \
          qcril_gstk_hexstring_to_bin((pd), (ps), (n))

#define QCRIL_GSTK_ENVRSP_DATA_SIZE(n)      (2 * (n))

#define QCRIL_GSTK_ENVRSP_BUFFER_SIZE(n)                 \
          (sizeof(qcril_gstk_raw_envelope_response_type) \
          + QCRIL_GSTK_ENVRSP_DATA_SIZE(n) + 1 )

#define QCRIL_GSTK_ENVRSP_COPY(pd, ps, n)                \
          qcril_gstk_bin_to_hexstring((pd), (ps), (n))

/*===========================================================================
                               LOCAL VARIABLES
===========================================================================*/

#ifndef FEATURE_CDMA_NON_RUIM

/* Structure to hold GSTK client initialisation result */
typedef struct
{
  gstk_status_enum_type     status;         /*!< GSTK call completion status  */
  gstk_client_id_type       client_id;      /*!< Our GSTK client ID           */
  gstk_client_ref_data_type user_data;      /*!< GSTK call user data argument */
}
qcril_gstk_client_init_data_type;

/* Structure to hold a raw format proactive command */
/* - size will vary according to the command length */
typedef struct
{
  gstk_client_ref_data_type user_data;    /*!< GSTK call user data argument     */
  uint32                    command_type; /*!< Proactive command type           */
  size_t                    length;       /*!< Proactive command length         */
  uint8                     data[1];      /*!< Proactive command ASCII hex data */
}
qcril_gstk_raw_proactive_command_type;

/* Structure to hold the cards response to a send raw envelope command */
/* - size will vary according to the response data length              */
typedef struct
{
  gstk_general_envelope_rsp_enum_type result;
  gstk_client_ref_data_type           user_data;  /*!< GSTK call user data argument */
  size_t                              length;     /*!< Response data length         */
  uint8                               data[1];    /*!< Response ASCII hex data      */
}
qcril_gstk_raw_envelope_response_type;

/*! @brief Variables internal to module qcril_gstk.c */
typedef struct
{
  gstk_client_id_type                    client_id;      /*!< Our GSTK client ID */
  uint32                                 cmd_ref_id;     /*!< GSTK command reference for TR */

  int                                    call_timeout;   /*<! call timeout (ms) for SETUP_CALL*/

  uint32                                 reg_info_count; /*!< Number of event reg entries */
  gstk_toolkit_cmd_reg_info_type        *reg_info_ptr;    /*!< Event reg entries array */

  uint32                                 tp_count;        /*!< Number of TP entries */
  gstk_profile_dl_support_ext_type      *tp_ptr;          /*!< TP entries array */

  qcril_gstk_raw_proactive_command_type *cmd_ptr;         /*!< Pending proactive cmd */
  size_t                                 cmd_len;         /*!< Size of buffer pointed by cmd_ptr */

  uint32                                 flags;           /*!< Flags of events having occured */
}
qcril_gstk_info_type;

static qcril_gstk_info_type qcril_gstk_info;

/* Timer to resend proactive command  - 5 seconds */
static const struct timeval QCRIL_GSTK_TIMER_RESEND = { 5, 0 };

/* Structure to notify GSTK which events we are interested in */
/* and to what extent we process each proactive command       */
static gstk_toolkit_cmd_reg_info_type qcril_gstk_reg_info[] =
{
  /* Classes A (Multiple card) and B (RUN_AT_COMMAND) are not supported at all */

  /* Class C - LAUNCH_BROWSER */
  { { GSTK_REG_CATEGORY_C_CMD, QCRIL_GSTK_EVT_MASK_CLASSC }, GSTK_HANDLE_ALL_FUNC },

  /* Class 0 - Everything else except BIP - commands where we handle everything */
  { { GSTK_REG_CATEGORY_0_CMD, QCRIL_GSTK_EVT_MASK_CLASS0_FULL }, GSTK_HANDLE_ALL_FUNC },

  /* Class 0 - Everything else except BIP - commands where we handle display only */
  { { GSTK_REG_CATEGORY_0_CMD, QCRIL_GSTK_EVT_MASK_CLASS0_DISPLAY }, GSTK_HANDLE_DISPLAY_FUNC_ONLY },

  /* Events not associated with a command class */
 // { { GSTK_REG_CATEGORY_0_CMD, QCRIL_GSTK_EVT_MASK_NOCLASS }, GSTK_HANDLE_ALL_FUNC }
};


/* Our Terminal Profile                                    */
/* Entries commented out are supported internally to GSTK, */
/* so avoid turning them off or on here.                   */
static gstk_profile_dl_support_ext_type qcril_gstk_terminal_profile[] =
{
  /* First byte (download) */
  /* GSTK_PROFILE_DL_SUPPORT set by GSTK only */                                          /* 0x0101 */
  /* GSTK_SMS_PP_DL_SUPPORT set by GSTK only */                                           /* 0x0102 */
  /* GSTK_CELL_BROADCAST_DATA_DL_SUPPORT set by GSTK only */                              /* 0x0104 */
  { 0x01,                        GSTK_MENU_SEL_SUPPORT,             QCRIL_STK_APP_NONE }, /* 0x0108 */
  /* GSTK_TIMER_EXPIRATION_SUPPORT set by GSTK only */                                    /* 0x0120 */
  { 0x01,                        GSTK_USSD_DO_IN_CC_SUPPORT,        QCRIL_STK_APP_SIM  }, /* 0x0140 */

  /* Second byte (other) */
  { 0x00,                        GSTK_CC_SUPPORT,                   QCRIL_STK_APP_SIM  }, /* 0x0202 */
  { 0x00,                        GSTK_MO_SMS_CONTROL_SUPPORT,       QCRIL_STK_APP_NONE }, /* 0x0208 */
  { 0x01,                        GSTK_SUPPORT_FOR_ALPHA_IN_CC,      QCRIL_STK_APP_SIM  }, /* 0x0210 */
  { 0x01,                        GSTK_UCS2_ENTRY_SUPPORT,           QCRIL_STK_APP_NONE }, /* 0x0220 */
  { 0x01,                        GSTK_UCS2_DISPLAY_SUPPORT,         QCRIL_STK_APP_NONE }, /* 0x0240 */
  { 0x01,                        GSTK_DISPLAY_OF_EXTN_TEXT,         QCRIL_STK_APP_NONE }, /* 0x0280 */

  /* Third byte (proactive commands) */
  { 0x01,                        GSTK_DISPLAY_TEXT_SUPPORT,         QCRIL_STK_APP_NONE }, /* 0x0301 */
  { 0x01,                        GSTK_GET_INKEY_SUPPORT,            QCRIL_STK_APP_NONE }, /* 0x0302 */
  { 0x01,                        GSTK_GET_INPUT_SUPPORT,            QCRIL_STK_APP_NONE }, /* 0x0304 */
  /* GSTK_MORE_TIME_SUPPORT set by GSTK only */                                           /* 0x0308 */
  { 0x01,                        GSTK_PLAY_TONE_SUPPORT,            QCRIL_STK_APP_NONE }, /* 0x0310 */
  /* GSTK_POLL_INTERVAL_SUPPORT set by GSTK only */                                       /* 0x0320 */
  /* GSTK_POLLING_OFF_SUPPORT set by GSTK only */                                         /* 0x0340 */
  { 0x00,                        GSTK_REFRESH_SUPPORT,              QCRIL_STK_APP_NONE }, /* 0x0380 */

  /* Fourth byte (proactive commands) */
  { 0x01,                        GSTK_SELECT_ITEM_SUPPORT,          QCRIL_STK_APP_NONE }, /* 0x0401 */
  { 0x01,                        GSTK_SEND_SHORT_MSG_SUPPORT,       QCRIL_STK_APP_NONE }, /* 0x0402 */
  { 0x01,                        GSTK_SEND_SS_SUPPORT,              QCRIL_STK_APP_NONE }, /* 0x0404 */
  { 0x01,                        GSTK_SEND_USSD_SUPPORT,            QCRIL_STK_APP_NONE }, /* 0x0408 */
  { 0x01,                        GSTK_SET_UP_CALL_SUPPORT,          QCRIL_STK_APP_NONE }, /* 0x0410 */
  { 0x01,                        GSTK_SET_UP_MENU_SUPPORT,          QCRIL_STK_APP_NONE }, /* 0x0420 */
  /* GSTK_PROVIDE_LOCAL_INFO_LOCATION_INFO_SUPPORT set by GSTK only */                    /* 0x0440 */
  /* GSTK_PROVIDE_LOCAL_INFO_NMR_SUPPORT set by GSTK only */                              /* 0x0480 */

  /* Fifth byte (event driven information) */
  { 0x01,                        GSTK_SET_UP_EVT_LIST_SUPPORT,      QCRIL_STK_APP_NONE }, /* 0x0501 */
  /* GSTK_MT_CALL_EVT_SUPPORT set by GSTK only */                                         /* 0x0502 */
  /* GSTK_CALL_CONNECTED_EVT_SUPPORT set by GSTK only */                                  /* 0x0504 */
  /* GSTK_CALL_DISCONNECTED_EVT_SUPPORT set by GSTK only */                               /* 0x0508 */
  /* GSTK_LOCATION_STATUS_EVT_SUPPORT set by GSTK only */                                 /* 0x0510 */
  { 0x01,                        GSTK_USER_ACT_EVT_SUPPORT,         QCRIL_STK_APP_NONE }, /* 0x0520 */
  { 0x01,                        GSTK_IDLE_SCRN_AVAIL_EVT_SUPPORT,  QCRIL_STK_APP_NONE }, /* 0x0540 */
  { 0x00,                        GSTK_CARD_READER_ST_EVT_SUPPORT,   QCRIL_STK_APP_NONE }, /* 0x0580 */

  /* Sixth byte (event driven information extensions) */
  { 0x01,                        GSTK_LANG_SEL_EVT_SUPPORT,         QCRIL_STK_APP_NONE }, /* 0x0601 */
  { 0x01,                        GSTK_BROWSER_TERM_EVT_SUPPORT,     QCRIL_STK_APP_NONE }, /* 0x0602 */
  { 0x00,                        GSTK_DATA_AVAIL_EVT_SUPPORT,       QCRIL_STK_APP_NONE }, /* 0x0604 */
  { 0x00,                        GSTK_CH_STATUS_EVT_SUPPORT,        QCRIL_STK_APP_NONE }, /* 0x0608 */

  /* Seventh byte (multiple card proactive commands) */
  { 0x00,                        GSTK_POWER_ON_CARD_SUPPORT,        QCRIL_STK_APP_NONE }, /* 0x0701 */
  { 0x00,                        GSTK_POWER_OFF_CARD_SUPPORT,       QCRIL_STK_APP_NONE }, /* 0x0702 */
  { 0x00,                        GSTK_PERFORM_CARD_APDU_SUPPORT,    QCRIL_STK_APP_NONE }, /* 0x0704 */
  { 0x00,                        GSTK_GET_READER_ST_SUPPORT,        QCRIL_STK_APP_NONE }, /* 0x0708 */
  { 0x00,                       GSTK_GET_READER_ST_CARD_ID_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0710 */

  /* Eighth byte (proactive commands) */
  { 0x00,                        GSTK_TIMER_START_STOP_SUPPORT,     QCRIL_STK_APP_NONE }, /* 0x0801 */
  { 0x00,                        GSTK_TIMER_GET_CURR_VALUE_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0802 */
  { 0x00,                     GSTK_PROVIDE_LOCAL_INFO_TIME_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0804 */
  { 0x01,                       GSTK_SET_UP_IDLE_MODE_TEXT_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0810 */
  { 0x00,                        GSTK_RUN_AT_CMD_SUPPORT,           QCRIL_STK_APP_NONE }, /* 0x0820 */
  { 0x01,                  GSTK_SECOND_ALPHA_IN_SETUP_CALL_SUPPORT, QCRIL_STK_APP_SIM  }, /* 0x0840 */
  { 0x01,                     GSTK_SECOND_CAP_CONFIG_PARAM_SUPPORT, QCRIL_STK_APP_SIM  }, /* 0x0880 */

  /* Ninth byte (proactive commands) */
  { 0x01,                        GSTK_SEND_DTMF_SUPPORT,            QCRIL_STK_APP_NONE }, /* 0x0902 */
  { 0x01,                     GSTK_PROVIDE_LOCAL_INFO_LANG_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0908 */
  { 0x00,                 GSTK_PROVIDE_LOCAL_INFO_TIME_ADV_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0910 */
  { 0x01,                       GSTK_LANGUAGE_NOTIFICATION_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x0920 */
  { 0x01,                        GSTK_LAUNCH_BROWSER_SUPPORT,       QCRIL_STK_APP_NONE }, /* 0x0940 */

  /* Tenth byte (soft keys support) */
  { 0x01,                        GSTK_SK_SUPPORT_IN_SEL_ITEM,       QCRIL_STK_APP_NONE }, /* 0x0A01 */
  { 0x01,                        GSTK_SK_SUPPORT_IN_SET_UP_MENU,    QCRIL_STK_APP_NONE }, /* 0x0A02 */

  /* Eleventh byte (number of soft keys) */
  { 0x02,                        GSTK_MAX_SK_SUPPORT,               QCRIL_STK_APP_NONE }, /* 0x0BFF */

  /* Fourteenth  byte (screen height) */
  { QCRIL_STK_SCRN_HEIGHT_LINES, GSTK_NUM_CHAR_SUPPORT_DOWN_ME,     QCRIL_STK_APP_NONE }, /* 0x0E1F */
  { 0x00,                        GSTK_SCREEN_SIZE_SUPPORT,          QCRIL_STK_APP_NONE }, /* 0x0E80 */

  /* Fifteenth byte (screen width) */
  { QCRIL_STK_SCRN_WIDTH_CHARS,  GSTK_NUM_CHAR_SUPPORT_ACROSS_ME,   QCRIL_STK_APP_NONE }, /* 0x0F7F */
  { 0x00,                        GSTK_VARIABLE_FONT_SIZE_SUPPORT,   QCRIL_STK_APP_NONE }, /* 0x0F80 */

  /* Sixteenth byte (screen effects) */
  { 0x00,                        GSTK_DISPLAY_RESIZED_SUPPORT,      QCRIL_STK_APP_NONE }, /* 0x1001 */
  { 0x00,                        GSTK_TEXT_WRAPPING_SUPPORT,        QCRIL_STK_APP_NONE }, /* 0x1002 */
  { 0x00,                        GSTK_TEXT_SCROLLING_SUPPORT,       QCRIL_STK_APP_NONE }, /* 0x1004 */
  { 0x00,                        GSTK_WIDTH_REDUCE_IN_MENU_SUPPORT, QCRIL_STK_APP_NONE }, /* 0x10E0 */

  /* Seventeenth byte (bearer independent protocol supported transports) */
  { 0x00,                        GSTK_TCP_SUPPORT,                  QCRIL_STK_APP_NONE }, /* 0x1101 */
  { 0x00,                        GSTK_UDP_SUPPORT,                  QCRIL_STK_APP_NONE }  /* 0x1102 */
};


/*===========================================================================
                             INTERNAL FUNCTIONS
===========================================================================*/

/*===========================================================================
 *   FUNCTION:  qcril_gstk_resend_proactive_cmd
 *   ===========================================================================*/
/*!
 *   @brief
 *     If STK RIL service is not ready, QCRIL/GSTK will resend the proactive
 *     command by calling this function.
 *
 *   @return
 *     Nothing
 *                                                                    */
/*=========================================================================*/
static void qcril_gstk_resend_proactive_cmd
(
  void *param_ptr
)
{
  uint32 timer_id = (uint32) param_ptr;
  qcril_instance_id_e_type instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( timer_id );
  qcril_modem_id_e_type modem_id = QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( timer_id );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "Timer expired with ID %d\n", (uint32) timer_id );

  if (!(qcril_gstk_info.flags & QCRIL_GSTK_RIL_IS_READY_BMSK))
  {
    QCRIL_LOG_INFO("%s", "STK service still not ready!");
    qcril_setup_timed_callback( instance_id, modem_id, qcril_gstk_resend_proactive_cmd,
                                &QCRIL_GSTK_TIMER_RESEND, &timer_id );
  }
  else
  {
    if (qcril_gstk_info.cmd_ptr != NULL)
    {
      QCRIL_LOG_INFO("%s", "Resending proactive cmd to RIL.\n");

      /* Resend the pending proactive command */
      qcril_event_queue( instance_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                         QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK, qcril_gstk_info.cmd_ptr, qcril_gstk_info.cmd_len, 
                         (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
      qcril_gstk_info.cmd_ptr = NULL;
      qcril_gstk_info.cmd_len = 0;
    }
    else
    {
      QCRIL_LOG_ERROR("%s", "qcril_gstk_info.cmd_ptr is NULL!\n");
    }
  }
} /* qcril_gstk_resend_proactive_cmd */

/*===========================================================================
 *   FUNCTION:  qcril_gstk_save_proactive_cmd
 *   ===========================================================================*/
/*!
 *   @brief
 *     Save proactive command into global data structure
 *
 *   @return
 *     Nothing
 *                                                                    */
/*=========================================================================*/
static void qcril_gstk_save_proactive_cmd
(
  qcril_gstk_raw_proactive_command_type *cmd_ptr,
  size_t                                 cmd_len
)
{
  if (qcril_gstk_info.cmd_ptr != NULL)
  {
    QCRIL_LOG_INFO("%s", "Dismiss pending proactive cmd!\n");
    free(qcril_gstk_info.cmd_ptr);
  }
  qcril_gstk_info.cmd_ptr = cmd_ptr;
  qcril_gstk_info.cmd_len = cmd_len;
} /* qcril_gstk_save_proactive_cmd */


/*===========================================================================
  FUNCTION:  qcril_gstk_bin_to_hexstring
===========================================================================*/
/*!
  @brief
    Convert an array of binary bytes into a hex string.

  @param[out] dest_ptr  Pointer to destination area. Must not be NULL.
  @param[in]  src_ptr   Pointer to source area. Must not be NULL.
  @param[in]  length    Length (in bytes) of the data pointed to by src_ptr.
                        The length of the data output via dest_ptr will be
                        twice this.

  @return
    Nothing
 */
/*=========================================================================*/
static void qcril_gstk_bin_to_hexstring
(
 uint8       *dest_ptr,
 const uint8 *src_ptr,
 uint32       length
)
{
  QCRIL_LOG_INFO("qcril_gstk_bin_to_hexstring, length %d\n", (int) length);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( dest_ptr != NULL );
  QCRIL_ASSERT( src_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  while (length--)
  {
    *dest_ptr++ = QCRIL_GSTK_HEXDIGITS[(*src_ptr >> 4) & 0x0F];
    *dest_ptr++ = QCRIL_GSTK_HEXDIGITS[(*src_ptr >> 0) & 0x0F];
    ++src_ptr;
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_bin_to_hexstring */


/*===========================================================================
  FUNCTION:  qcril_gstk_hexdigit_to_bin
===========================================================================*/
/*!
  @brief
    Convert a hex digit to a binary value.

  @param[in] src Source hex digit.

  @return
    The converted binary value in the range 0 to 15
 */
/*=========================================================================*/
static uint8 qcril_gstk_hexdigit_to_bin
(
 uint8 src
)
{
  QCRIL_LOG_INFO("qcril_gstk_hexdigit_to_bin digit = 0x%02X\n", src);
  /*-----------------------------------------------------------------------*/

  switch (src)
  {
    case 0x00: case 0x01: case 0x02: case 0x03:
    case 0x04: case 0x05: case 0x06: case 0x07:
    case 0x08: case 0x09: case 0x0A: case 0x0B:
    case 0x0C: case 0x0D: case 0x0E: case 0x0F:
      return src;
      break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return src - '0';
      break;

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
      return (src - 'A') + 10;
      break;

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
      return (src - 'a') + 10;
      break;
  }

  /*-----------------------------------------------------------------------*/

  return 0;

} /* qcril_gstk_hexdigit_to_bin */


/*===========================================================================
  FUNCTION:  qcril_gstk_hexstring_to_bin
===========================================================================*/
/*!
  @brief
    Convert a hex string to an array of binary bytes.

  @param[out] dest_ptr  Pointer to destination area. Must not be NULL.
  @param[in]  src_ptr   Pointer to source area. Must not be NULL.
  @param[in]  length    Length (in bytes) of the data pointed to by src_ptr.
                        The length of the data output via dest_ptr will be
                        half this.

  @return
    Nothing
 */
/*=========================================================================*/
static void qcril_gstk_hexstring_to_bin
(
 uint8       *dest_ptr,
 const uint8 *src_ptr,
 uint32       length
)
{
  QCRIL_LOG_INFO("qcril_gstk_hexstring_to_bin, length %d\n", (int) length);
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( dest_ptr != NULL );
  QCRIL_ASSERT( src_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  for (length /= 2; length; --length)
  {
    uint8 temp;

    temp  = qcril_gstk_hexdigit_to_bin(*src_ptr++) << 4;
    temp |= qcril_gstk_hexdigit_to_bin(*src_ptr++) << 0;
    *dest_ptr++ = temp;
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_hexstring_to_bin */


/*===========================================================================
  FUNCTION:  qcril_gstk_client_init_callback
===========================================================================*/
/*!
  @brief
    Callback for gstk_client_init_ext during RIL GSTK initialisation

    This function merely copies the data supplied by GSTK and signals a
    QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK event, which will cause
    qcril_gstk_process_client_init_callback to be invoked to do the work

  @param[in]  status     Client initialisation outcome from GSTK
  @param[in]  client_id  If status is GSTK_SUCCESS then this is a valid
                         client ID to be used in subsequent calls into GSTK
  @param[in]  user_data  The client_ref argument supplied in the call to
                         gstk_client_init_ext

  @return
    Nothing

  @see
    qcril_gstk_init
    qcril_gstk_process_client_init_callback
*/
/*=========================================================================*/
static void qcril_gstk_client_init_callback
(
  gstk_status_enum_type     status,
  gstk_client_id_type       client_id,
  gstk_client_ref_data_type user_data
)
{
  qcril_gstk_client_init_data_type init_data;

  QCRIL_LOG_INFO( "qcril_gstk_client_init_callback: status %d, client ID 0x%08lX\n", status, client_id);
  /*-----------------------------------------------------------------------*/

  init_data.status    = status;
  init_data.client_id = client_id;
  init_data.user_data = user_data;

  /* Queue the event, which wil cause the processing function to be invoked */
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_ON_STACK,
                     QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK, &init_data, sizeof(init_data), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_client_init_callback */


/*===========================================================================
  FUNCTION:  qcril_gstk_client_reg_callback
===========================================================================*/
/*!
  @brief
    Callback for gstk_client_toolkit_cmd_reg during RIL GSTK initialisation

    This function merely copies the data supplied by GSTK and signals a
    QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK event, which will cause
    qcril_gstk_process_client_reg_callback to be invoked to do the work

  @param[in]  status     Client event registration outcome from GSTK
  @param[in]  user_data  The client_user_data argument supplied in the call
                         to gstk_client_toolkit_cmd_reg

  @return
    Nothing

  @see
    qcril_gstk_init
    qcril_gstk_process_client_reg_callback
*/
/*=========================================================================*/
static void qcril_gstk_client_reg_callback
(
  gstk_status_enum_type     status,
  gstk_client_ref_data_type user_data
)
{
  qcril_gstk_client_init_data_type init_data;

  QCRIL_LOG_INFO("qcril_gstk_client_reg_callback: status %d\n", status);
  /*-----------------------------------------------------------------------*/

  init_data.status    = status;
  init_data.user_data = user_data;

  /* Queue the event, which wil cause the processing function to be invoked */
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_ON_STACK,
                     QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK, &init_data, sizeof(init_data), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_client_reg_callback */


/*===========================================================================
  FUNCTION:  qcril_gstk_toolkit_evt_callback
===========================================================================*/
/*!
  @brief
    Callback to handle notification of events from GSTK

    The only useful events are raw proactive commands and end of session
    notifications, for which this function will copy the data supplied by
    GSTK and signal QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK to cause
    qcril_gstk_process_raw_command_callback to be invoked to do the work.

  @param[in]  req_rsp_ptr Event data supplied by GSTK

  @return
    Nothing

  @msc
    hscale = "1.5";
    qcril_main, qcril_GSTK, AMSS;
    qcril_GSTK <<= AMSS       [label = "qcril_gstk_toolkit_evt_callback()"];

    --- [label = ".... if callback data is raw command or session end"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\nQCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK)",
                               URL = "\ref qcril_event_queue"];
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_process_raw_command_callback()",
                               URL = "\ref qcril_gstk_process_raw_command_callback"];
  @endmsc

  @see
    qcril_gstk_process_raw_command_callback
*/
/*=========================================================================*/
void qcril_gstk_toolkit_evt_callback
(
  gstk_client_pro_cmd_reg_data_type *req_rsp_ptr
)
{
  errno_enum_type qcril_result = E_SUCCESS;

  QCRIL_LOG_INFO("%s", "qcril_gstk_toolkit_evt_callback\n");
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( req_rsp_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  switch (req_rsp_ptr->data_format_type)
  {
    /* This is a proactive command in raw binary format */
    case GSTK_RAW_FORMAT:
      {
        gstk_raw_cmd_from_card_type *src_ptr = 
          req_rsp_ptr->pro_cmd_reg_data.raw_req_rsp_ptr;

        QCRIL_LOG_INFO("%s", "Raw format proactive command\n");

        if ( (src_ptr == NULL)                                        ||
             (src_ptr->payload.data_buffer_ptr == NULL)               ||
             (src_ptr->payload.data_len < QCRIL_GSTK_COMMAND_MIN_SIZE) )
        {
          QCRIL_LOG_ERROR("%s", "Invalid raw proactive command details from GSTK\n");
        }
        else
        {
          uint8 *src_data_ptr = &src_ptr->payload.data_buffer_ptr[
                                  QCRIL_GSTK_COMMAND_START_INDEX];
          uint32 src_data_len = (uint32) src_ptr->payload.data_len - 
                                QCRIL_GSTK_COMMAND_START_INDEX;

          /* Allocate and populate the structure to pass into the event queue */
          /* - will be freed later in qcril_gstk_process_raw_command_callback */
          size_t dest_size = QCRIL_GSTK_COMMAND_BUFFER_SIZE(src_data_len);
          qcril_gstk_raw_proactive_command_type *dest_ptr = malloc(dest_size);
          if (dest_ptr == NULL)
          {
            QCRIL_LOG_ERROR("%s", "Failed to allocate memory for raw proactive command\n");
          }
          else
          {
            memset(dest_ptr, 0, dest_size);
            dest_ptr->user_data    = src_ptr->raw_hdr.user_data;
            if (src_data_ptr[1] == 0x81)
            {
              dest_ptr->command_type = src_data_ptr[QCRIL_GSTK_COMMAND_TYPE_INDEX + 1];
            }
            else
            {
              dest_ptr->command_type = src_data_ptr[QCRIL_GSTK_COMMAND_TYPE_INDEX];
            }

            dest_ptr->length       = (size_t) QCRIL_GSTK_COMMAND_DATA_SIZE( src_data_len );

            QCRIL_GSTK_COMMAND_COPY(dest_ptr->data, src_data_ptr, src_data_len);

            qcril_gstk_info.cmd_ref_id = src_ptr->raw_hdr.cmd_ref_id;

            if (qcril_gstk_info.flags & QCRIL_GSTK_RIL_IS_READY_BMSK)
            {
              /* Queue the event, which wil cause the processing 
               * function to be invoked */
              qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                                 QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK, dest_ptr, dest_size, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
            }
            else
            {
              QCRIL_LOG_INFO("%s", "STK services not ready!");
              qcril_gstk_save_proactive_cmd(dest_ptr, dest_size);
            }
          }
        }
      }
      break;

    /* Some other kind of notification from GSTK */
    case GSTK_GSTK_FORMAT:
      {
        gstk_cmd_from_card_type *src_ptr = 
          req_rsp_ptr->pro_cmd_reg_data.gstk_req_rsp_ptr;

        QCRIL_LOG_INFO("%s", "GSTK format event notification\n");

        if (src_ptr == NULL)
        {
          QCRIL_LOG_ERROR("%s", "Invalid GSTK event details from GSTK\n");
        }
        else
        {
          switch (src_ptr->hdr_cmd.command_id)
          {
            /* End of proactive command session - queue the event */
            case GSTK_END_PROACTIVE_CMD_REQ:
              {
                if (qcril_gstk_info.flags & QCRIL_GSTK_RIL_IS_READY_BMSK)              
                {
                  /* Allocate and populate the structure to pass 
                   *  into the event queue
                   * - will be freed later in 
                   *   qcril_gstk_process_raw_command_callback */
                  size_t dest_size = QCRIL_GSTK_COMMAND_BUFFER_SIZE(0);
                  qcril_gstk_raw_proactive_command_type *dest_ptr = malloc(
                                                                      dest_size);
                  if (dest_ptr == NULL)
                  {
                    QCRIL_LOG_ERROR("%s", "Failed to allocate memory for End Session notification\n");
                  }
                  else 
                  {
                    memset(dest_ptr, 0, dest_size);
                    dest_ptr->user_data    = src_ptr->hdr_cmd.user_data;
                    dest_ptr->command_type = GSTK_CMD_STK_END_OF_PROACTIVE_SES;
                    dest_ptr->length       = 0;

                    qcril_gstk_info.cmd_ref_id = src_ptr->hdr_cmd.cmd_detail_reference;
                    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                                       QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK, dest_ptr, dest_size, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
                  }
                }
                else
                {
                  /* clean up pending cmd */
                  qcril_gstk_save_proactive_cmd(NULL, 0);
                }
              }
              break;

            case GSTK_PROFILE_DL_IND_RSP:
              {
                if (qcril_gstk_info.cmd_ptr != NULL)
                {
                  /* clean up pending cmd */
                  qcril_gstk_save_proactive_cmd(NULL, 0);
                }
              }
              break;

            /* No idea what this is */
            default:
              QCRIL_LOG_ERROR( "Unrecognised notification from GSTK (0x%08lX)\n", (unsigned long) src_ptr->hdr_cmd.command_id);
              break;
          }
        }
      }
      break;

    /* Not anything we were expecting to see */
    default:
      {
        QCRIL_LOG_ERROR("Invalid event data format (%d)\n", 
                        req_rsp_ptr->data_format_type);
      }
      break;
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_toolkit_evt_callback */


/*===========================================================================
  FUNCTION:  qcril_gstk_send_raw_envelope_callback
===========================================================================*/
/*!
  @brief
    Callback to handle completion of gstk_send_raw_envelope_command().

    This function simply copies the data supplied by GSTK and signals
    QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK to cause
    qcril_gstk_process_send_raw_envelope_callback to be invoked to do the work.

  @param[in]  req_rsp_ptr Event data supplied by GSTK

  @return
    Nothing

  @see
    qcril_gstk_request_stk_send_envelope_command
    qcril_gstk_process_send_raw_envelope_callback
*/
/*=========================================================================*/
void qcril_gstk_send_raw_envelope_callback
(
  gstk_cmd_from_card_type *req_rsp_ptr
)
{
  size_t dest_size = 0;
  qcril_gstk_raw_envelope_response_type *dest_ptr = NULL;

  QCRIL_LOG_INFO("%s", "qcril_gstk_send_raw_envelope_callback\n");
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( req_rsp_ptr != NULL );
  QCRIL_ASSERT( req_rsp_ptr->hdr_cmd.command_id == GSTK_RAW_ENVELOPE_RSP );
  QCRIL_ASSERT( (req_rsp_ptr->cmd.raw_envelope_rsp.length == 0) ||
          (req_rsp_ptr->cmd.raw_envelope_rsp.rsp_data_ptr != NULL) );

  /*-----------------------------------------------------------------------*/

  dest_size = QCRIL_GSTK_ENVRSP_BUFFER_SIZE(
                req_rsp_ptr->cmd.raw_envelope_rsp.length);
  dest_ptr = malloc(dest_size);
  if (dest_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Failed to allocate memory for raw envelope response\n");
  }
  else
  {
    memset(dest_ptr, 0, dest_size);
    dest_ptr->result = req_rsp_ptr->cmd.raw_envelope_rsp.general_resp;
    dest_ptr->user_data = req_rsp_ptr->hdr_cmd.user_data;

    if (req_rsp_ptr->cmd.raw_envelope_rsp.length != 0)
    {
      dest_ptr->length = req_rsp_ptr->cmd.raw_envelope_rsp.length;
      QCRIL_GSTK_ENVRSP_COPY(dest_ptr->data,
                             req_rsp_ptr->cmd.raw_envelope_rsp.rsp_data_ptr,
                             req_rsp_ptr->cmd.raw_envelope_rsp.length);
    }

    /* Queue the event, which wil cause the 
     * processing function to be invoked */
    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                       QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK, dest_ptr, dest_size, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_send_raw_envelope_callback */


/*===========================================================================
                          QCRIL INTERFACE FUNCTIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  qcril_gstk_init
===========================================================================*/
/*!
  @brief
    Initialize the RIL GSTK subsystem

    - Obtain client ID from GSTK
    - Notify GSTK which events we are interested in
    - Download the Terminal Profile

  @return
    Nothing

  @msc
    hscale = "1.5";
    qcril_main, qcril_GSTK, AMSS;
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_init()"];

    --- [label = "Obtain a GSTK client ID"];
    qcril_GSTK =>  AMSS       [label = "gstk_client_init_ext()"];

    ...;
    qcril_GSTK <<= AMSS       [label = "qcril_gstk_client_init_callback()",
                               URL = "\ref qcril_gstk_client_init_callback"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue()",
                               URL = "\ref qcril_event_queue"];
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_process_client_init_callback()",
                               URL = "\ref qcril_gstk_process_client_init_callback"];

    --- [label = "Notify GSTK which events are of interest"];
    qcril_GSTK =>  AMSS       [label = "gstk_client_toolkit_cmd_reg()"];

    ...;
    qcril_GSTK <<= AMSS       [label = "qcril_gstk_client_reg_callback()",
                               URL = "\ref qcril_gstk_client_reg_callback"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue()",
                               URL = "\ref qcril_event_queue"];
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_process_client_reg_callback()",
                               URL = "\ref qcril_gstk_process_client_reg_callback"];

    --- [label = "Download our Terminal Profile"];
    qcril_GSTK =>  AMSS       [label = "gstk_update_profile_download_value_ext()"];
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_init( void )
{
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  gstk_status_enum_type gstk_result = GSTK_SUCCESS;

  QCRIL_LOG_DEBUG( "%s", "qcril_gstk_init");
  /*-----------------------------------------------------------------------*/

  memset(&qcril_gstk_info, 0, sizeof(qcril_gstk_info));

  qcril_gstk_info.call_timeout = (2 * 60 * 1000);

  qcril_gstk_info.reg_info_count = QCRIL_GSTK_ARRAY_SIZE(qcril_gstk_reg_info);
  qcril_gstk_info.reg_info_ptr   = &qcril_gstk_reg_info[0];
  qcril_gstk_info.tp_count       = QCRIL_GSTK_ARRAY_SIZE(
                                     qcril_gstk_terminal_profile);
  qcril_gstk_info.tp_ptr         = &qcril_gstk_terminal_profile[0];

  QCRIL_LOG_RPC2( modem_id, "gstk_client_init_ext()", "" );
  gstk_result = gstk_client_init_ext(GSTK_UI_TYPE,
                                     qcril_gstk_client_init_callback,
                                     QCRIL_GSTK_CB_USER_DATA);

  if (gstk_result != GSTK_SUCCESS)
  {
    QCRIL_LOG_ERROR("Call to gstk_client_init_ext failed, result %d\n", gstk_result);
    QCRIL_LOG_INFO("%s", "CLIENT_INIT_EXT failed\n");
    return;
  }

  QCRIL_LOG_INFO("%s", "CLIENT_INIT_EXT success\n");

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_init */


/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_get_profile
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_GET_PROFILE

    Intended to retrieve a Terminal Profile, this request is not
    used by Google at present.

  @param[in]  params_ptr  Pointer to a struct containing the request data
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing
*/
/*=========================================================================*/
void qcril_gstk_request_stk_get_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_request_stk_get_profile\n");

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( &resp );

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_request_stk_get_profile */


/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_set_profile
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_SET_PROFILE

    Intended to initiate a Terminal Profile download, this request is not
    used by Google at present.

  @param[in]  params_ptr  Pointer to a struct containing the request data
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing
*/
/*=========================================================================*/
void qcril_gstk_request_stk_set_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_request_stk_set_profile\n");

  /*-----------------------------------------------------------------------*/

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( &resp );

} /* qcril_gstk_request_stk_set_profile */


/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_send_envelope_command
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND.

  @param[in]  params_ptr  Pointer to a struct containing the request data
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @msc
    hscale = "1.5";
    qcril_main, qcril_GSTK, AMSS;
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_request_stk_send_envelope_command()"];
    qcril_GSTK =>  AMSS       [label = "gstk_send_raw_envelope_command()"];
    qcril_GSTK <<= AMSS       [label = "qcril_gstk_send_raw_envelope_callback()",
                               URL = "\ref qcril_gstk_send_raw_envelope_callback"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\nQCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK)",
                               URL = "\ref qcril_event_queue"];
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_process_send_raw_envelope_callback()",
                               URL = "\ref qcril_gstk_process_send_raw_envelope_callback"];
    qcril_GSTK =>  qcril_main [label = "qcril_response_success()",
                               URL = "\ref qcril_response_success"];
  @endmsc

  @see
    qcril_gstk_send_raw_envelope_callback
    qcril_gstk_process_send_raw_envelope_callback
*/
/*=========================================================================*/
void qcril_gstk_request_stk_send_envelope_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  gstk_generic_data_type env = { NULL, 0 };
  gstk_status_enum_type gstk_result = GSTK_SUCCESS;
  RIL_Errno ril_result = RIL_E_SUCCESS;
  int len;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen != 0 );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_request_stk_send_envelope_command\n");

  /* Allocate memory for the raw envelope */
  len = strlen((char *) params_ptr->data);
  QCRIL_LOG_INFO("Envelope data length is %d\n",len);

  /* env.data_len = QCRIL_GSTK_ENVCMD_DATA_SIZE(params_ptr->datalen); */
  env.data_len = QCRIL_GSTK_ENVCMD_DATA_SIZE(len);
  env.data_buffer_ptr = malloc(env.data_len);

  if (env.data_buffer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Unable to allocate %d bytes for Envelope\n", (int) env.data_len);
    ril_result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    /* Copy over the command and send to the card */
    QCRIL_GSTK_ENVCMD_COPY(env.data_buffer_ptr,
                           params_ptr->data,
                           len /* was params_ptr->datalen */);

    QCRIL_LOG_RPC2( modem_id, "gstk_send_raw_envelope_command()", "" );
    gstk_result = gstk_send_raw_envelope_command(
                    qcril_gstk_info.client_id,
                                                 (uint32) params_ptr->t,
                                                 env,
                                                 qcril_gstk_send_raw_envelope_callback);
    if (gstk_result != GSTK_SUCCESS)
    {
      QCRIL_LOG_ERROR("%s", "Unable to send Envelope to card\n");
      ril_result = RIL_E_GENERIC_FAILURE;
    }

    /* Remember to free the envelope memory we allocated earlier */
    free(env.data_buffer_ptr);
  }

  /* If successful the result will be notified by the command */
  /* completion callback, otherwise that must be done here    */
  if (ril_result != RIL_E_SUCCESS)
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_result, &resp );
    qcril_send_request_response( &resp );
  }


} /* qcril_gstk_request_stk_send_envelope_command */


/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_send_terminal_response
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE

    For proactive commands for which the Terminal Response originates from
    the Android Toolkit application, this function arranges for that
    response to be sent to the card.

  @param[in]  params_ptr  Pointer to a struct containing the request data
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
    qcril_gstk_process_raw_command_callback
*/
/*=========================================================================*/
void qcril_gstk_request_stk_send_terminal_response
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  gstk_generic_data_type rsp = { NULL, 0 };
  gstk_status_enum_type gstk_result = GSTK_SUCCESS;
  RIL_Errno ril_result = RIL_E_SUCCESS;
  int len;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen != 0 );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_request_stk_send_terminal_response\n");

  QCRIL_LOG_INFO("Data length supplied is %d\n", params_ptr->datalen);

  len = strlen((char *) params_ptr->data);
  QCRIL_LOG_INFO("String length of data buffer is %d\n", len);

  /*-----------------------------------------------------------------------*/

  /* Allocate memory for the raw response */
  /* rsp.data_len = QCRIL_GSTK_RESPONSE_DATA_SIZE(params_ptr->datalen); */
  rsp.data_len = QCRIL_GSTK_RESPONSE_DATA_SIZE(len);
  rsp.data_buffer_ptr = malloc(rsp.data_len);

  if (rsp.data_buffer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Unable to allocate %d bytes for Terminal Response\n",
                     (int) rsp.data_len);
    ril_result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    /* Copy over the response and send to the card */
    QCRIL_GSTK_RESPONSE_COPY(rsp.data_buffer_ptr,
                             params_ptr->data,
                             len /* Was params_ptr->datalen */);

    QCRIL_LOG_RPC2( modem_id, "gstk_send_raw_terminal_response()", "");
    gstk_result = gstk_send_raw_terminal_response(qcril_gstk_info.client_id,
                                                  QCRIL_GSTK_CB_USER_DATA,
                                                  qcril_gstk_info.cmd_ref_id,
                                                  rsp);
    if (gstk_result != GSTK_SUCCESS)
    {
      QCRIL_LOG_ERROR("%s", "Unable to send Terminal Response to card\n");
      ril_result = RIL_E_GENERIC_FAILURE;
    }

    /* Remember to free the response memory we allocated earlier */
    free(rsp.data_buffer_ptr);
  }

  /* Notify the caller of the result */
  if (ril_result == RIL_E_SUCCESS)
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_result, &resp );
  }

  qcril_send_request_response( &resp );


} /* qcril_gstk_request_stk_send_terminal_response */


/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_handle_call_setup_requested_from_sim
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM.

    This request arrives from the SIM Toolkit application after the user
    has accepted or rejected a call originating from a SETUP_CALL command,
    and the request data indicates which.

  @param[in]  params_ptr  Pointer to a struct containing the request data
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
    qcril_gstk_process_raw_command_callback
*/
/*=========================================================================*/
void qcril_gstk_request_stk_handle_call_setup_requested_from_sim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  int user_accepted = 0;
  gstk_status_enum_type gstk_result = GSTK_SUCCESS;
  RIL_Errno ril_result = RIL_E_SUCCESS;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen == sizeof(int) );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_request_stk_handle_call_setup_requested_from_sim\n");

  user_accepted = *(int *) params_ptr->data;
  QCRIL_LOG_INFO("User %s call\n", 
                  (user_accepted ? "ACCEPTED" : "REJECTED"));

  /* Pass into GSTK and ensure it was well received */
  QCRIL_LOG_RPC2( modem_id, "gstk_send_setup_call_user_cnf_alpha_rsp()", "");
  gstk_result = gstk_send_setup_call_user_cnf_alpha_rsp( qcril_gstk_info.client_id,
                                                         qcril_gstk_info.cmd_ref_id,
                                                         (user_accepted ? TRUE : FALSE));
  if (gstk_result != GSTK_SUCCESS)
  {
    QCRIL_LOG_ERROR( "Call to gstk_send_setup_call_user_cnf_alpha_rsp failed, result %d\n", gstk_result);
    ril_result = RIL_E_GENERIC_FAILURE;
  }

  /* Notify the caller of the result */
  if (ril_result == RIL_E_SUCCESS)
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_result, &resp );
  }

  qcril_send_request_response( &resp );

} /* qcril_gstk_request_stk_handle_call_setup_requested_from_sim */


/*=========================================================================
  FUNCTION:  qcril_gstk_process_command_callback
===========================================================================*/
/*!
  @brief
    Handle GSTK command callback.

  @return
    Nothing
*/
/*=========================================================================*/
void qcril_gstk_process_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO("%s", "qcril_gstk_process_command_callback\n");
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_process_command_callback */


/*=========================================================================
  FUNCTION:  qcril_gstk_process_client_init_callback
===========================================================================*/
/*!
  @brief
    Handle the QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK event that is passed to
    qcril_event_queue by the callback invoked by GSTK.

  @param[in]  params_ptr  Pointer to a struct containing the data from GSTK
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
    qcril_gstk_init
    qcril_gstk_client_init_callback
*/
/*=========================================================================*/
void qcril_gstk_process_client_init_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_modem_id_e_type                modem_id      = QCRIL_DEFAULT_MODEM_ID;
  qcril_gstk_client_init_data_type     *data_ptr     = NULL;
  const gstk_toolkit_cmd_reg_info_type *reg_list_ptr = NULL;
  gstk_status_enum_type                 gstk_result  = GSTK_SUCCESS;

  QCRIL_LOG_INFO("%s", "qcril_gstk_process_client_init_callback\n");
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen == sizeof(*data_ptr) );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Find the client init result data */
  data_ptr = (qcril_gstk_client_init_data_type *) params_ptr->data;
  if (data_ptr->status == GSTK_SUCCESS)
  {
    /* We now have a client ID, so remember it and move on */
    QCRIL_LOG_INFO("Client init successful, ID 0x%08lX\n",
                    data_ptr->client_id);

    QCRIL_LOG_INFO("qcril_gstk_info.reg_info_count %d\n", 
                    (int) qcril_gstk_info.reg_info_count);
    reg_list_ptr = &qcril_gstk_reg_info[0];

    qcril_gstk_info.client_id = data_ptr->client_id;

    QCRIL_LOG_RPC2( modem_id, "gstk_client_toolkit_cmd_reg()", "");
    gstk_result = gstk_client_toolkit_cmd_reg(qcril_gstk_info.client_id,
                                              QCRIL_GSTK_CB_USER_DATA,
                                              GSTK_RAW_FORMAT,
                                              qcril_gstk_info.reg_info_count,
                                              reg_list_ptr,
                                              qcril_gstk_client_reg_callback,
                                              qcril_gstk_toolkit_evt_callback);

    if (gstk_result != GSTK_SUCCESS)
    {
      QCRIL_LOG_ERROR("Call to gstk_client_toolkit_cmd_reg failed, result %d\n", gstk_result);
    }
    else
    {
      QCRIL_LOG_INFO("%s", "toolkit_cmd_reg sent to arm9 \n");
    }
  }
  else
  {
    /* Failed to obtain a client ID */
    QCRIL_LOG_ERROR("Client init failed, result %d\n", data_ptr->status);
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_process_client_init_callback */

/*=========================================================================
  FUNCTION:  qcril_gstk_send_terminal_profile
===========================================================================*/
/*!
  @brief
    This function will send terminal profile to GSTK only in following
    conditions:
      1) QCRIL_GSTK_CLI_INIT_SUCC_BMSK is set
      AND
      2) QCRIL_GSTK_HAS_SENT_TP_BMSK is not set

  @return
    Nothing

  @see
*/
/*=========================================================================*/
void qcril_gstk_send_terminal_profile()
{
  qcril_modem_id_e_type             modem_id = QCRIL_DEFAULT_MODEM_ID;
  gstk_status_enum_type             gstk_result = GSTK_SUCCESS;

  QCRIL_LOG_INFO("%s", "qcril_gstk_send_terminal_profile\n");
  if (!(qcril_gstk_info.flags & QCRIL_GSTK_HAS_SENT_TP_BMSK) &&
      (qcril_gstk_info.flags & QCRIL_GSTK_CLI_INIT_SUCC_BMSK)  )
  {

    QCRIL_LOG_RPC2( modem_id, "gstk_update_profile_download_value_ext()", "" );
    gstk_result = gstk_update_profile_download_value_ext(
                    qcril_gstk_info.client_id,
                    QCRIL_GSTK_CB_USER_DATA,
                    (uint8) qcril_gstk_info.tp_count,
                    qcril_gstk_info.tp_ptr);

    if (gstk_result != GSTK_SUCCESS)
    {
      QCRIL_LOG_ERROR( "Call to gstk_client_toolkit_cmd_reg failed, result %d\n", gstk_result);
    }
    else
    {
      qcril_gstk_info.flags |= QCRIL_GSTK_HAS_SENT_TP_BMSK;
    }
  }
} /* qcril_gstk_send_terminal_profile */


/*=========================================================================
  FUNCTION:  qcril_gstk_process_notify_ril_is_ready
===========================================================================*/
/*!
  @brief
    Handle the QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY event that is passed to
    qcril_event_queue by getVersion() in qcril.c file.

    If QCRIL_GSTK_RIL_IS_READY_BMSK is not set, the function will set the
    flag and try to send terminal profile.

  @param[in]  params_ptr  Should be NULL
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
*/
/*=========================================================================*/
void qcril_gstk_process_notify_ril_is_ready
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  uint32 timer_id;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_process_notify_ril_is_ready\n");
  qcril_gstk_info.flags |= QCRIL_GSTK_RIL_IS_READY_BMSK;
  qcril_setup_timed_callback( instance_id, modem_id, qcril_gstk_resend_proactive_cmd,
                              &QCRIL_GSTK_TIMER_RESEND, &timer_id );

} /* qcril_gstk_process_notify_ril_is_ready */


/*=========================================================================
  FUNCTION:  qcril_gstk_process_client_reg_callback
===========================================================================*/
/*!
  @brief
    Handle the QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK event that is passed to
    qcril_event_queue by the callback invoked by GSTK.

  @param[in]  params_ptr  Pointer to a struct containing the data from GSTK
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
    qcril_gstk_init
    qcril_gstk_client_reg_callback
*/
/*=========================================================================*/
void qcril_gstk_process_client_reg_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_client_init_data_type *data_ptr    = NULL;

  QCRIL_LOG_INFO("%s", "qcril_gstk_process_client_reg_callback\n");
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen == sizeof(*data_ptr) );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Find the client reg result data */
  data_ptr = (qcril_gstk_client_init_data_type *) params_ptr->data;
  if (data_ptr->status == GSTK_SUCCESS)
  {
    QCRIL_LOG_INFO("%s", "Client reg successful\n");

    qcril_gstk_info.flags |= QCRIL_GSTK_CLI_INIT_SUCC_BMSK;
    qcril_gstk_send_terminal_profile();
  }
  else
  {
    QCRIL_LOG_ERROR("Client reg failed, result %d\n", data_ptr->status);
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_process_client_reg_callback */


/*=========================================================================
  FUNCTION:  qcril_gstk_process_raw_command_callback
===========================================================================*/
/*!
  @brief
    Processes session end notifications and raw proactive commands which
    GSTK delivers to its event callback, qcril_gstk_toolkit_evt_callback.

    The raw command data is already converted to the form Android requires
    by qcril_gstk_toolkit_evt_callback, as that halves the number of
    malloc calls that would otherwise be required.

    All that remains to be done here is to send one of a number of
    notifications up to Android itself, depending on the call flow that
    is used for this command:

    @li RIL_UNSOL_STK_PROACTIVE_COMMAND - used for commands that are
        handled entirely within Android. The Terminal Response originates
        from Android.

    @li RIL_UNSOL_STK_EVENT_NOTIFY - used for commands that are handled
        by the modem, but which have a UI component that is handled
        within Android. The Terminal Response for these originates from
        the modem.

    @li RIL_UNSOL_STK_SESSION_END - indicates the end of the proactive
        command session.

  @return
    Nothing

  @msc
    hscale = "1.5";
    qcril_main, qcril_GSTK, AMSS;
    qcril_GSTK <<= AMSS       [label = "qcril_gstk_toolkit_evt_callback()"];

    --- [label = ".... if callback data is raw command or session end"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\nQCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK)",
                               URL = "\ref qcril_event_queue"];
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_process_raw_command_callback()",
                               URL = "\ref qcril_gstk_process_raw_command_callback"];

    --- [label = ".... if command is End of Session"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\RIL_UNSOL_STK_SESSION_END)",
                               URL = "\ref qcril_event_queue"];

    --- [label = ".... if command is SEND_SS, SEND_USSD, SEND_DTMF, SEND_SMS"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\nRIL_UNSOL_STK_EVENT_NOTIFY)",
                               URL = "\ref qcril_event_queue"];

    --- [label = ".... if command is SETUP_CALL"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\nRIL_UNSOL_STK_EVENT_NOTIFY)",
                               URL = "\ref qcril_event_queue"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\nRIL_UNSOL_STK_CALL_SETUP)",
                               URL = "\ref qcril_event_queue"];
    ...;
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_request_stk_handle_call_setup_requested_from_sim()",
                               URL = "\ref qcril_gstk_request_stk_handle_call_setup_requested_from_sim"];
    qcril_GSTK =>  AMSS       [label = "gstk_send_setup_call_user_cnf_alpha_rsp()"];
    qcril_GSTK =>  qcril_main [label = "qcril_result_success()",
                               URL = "\ref qcril_result_success"];

    --- [label = ".... for any other command"];
    qcril_GSTK =>  qcril_main [label = "qcril_event_queue(\RIL_UNSOL_STK_PROACTIVE_COMMAND)",
                               URL = "\ref qcril_event_queue"];
    ...;
    qcril_main =>  qcril_GSTK [label = "qcril_gstk_request_stk_send_terminal_response()",
                               URL = "\ref qcril_gstk_request_stk_send_terminal_response"];
    qcril_GSTK =>  AMSS       [label = "gstk_send_raw_terminal_response()"];
    qcril_GSTK =>  qcril_main [label = "qcril_unsol_response()",
                               URL = "\ref qcril_unsol_response"];
  @endmsc

  @see
    qcril_gstk_toolkit_evt_callback
*/
/*=========================================================================*/
void qcril_gstk_process_raw_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_gstk_raw_proactive_command_type *cmd_ptr = NULL;
  int     ril_unsol_type = 0;
  int     ril_additional_unsol_type = 0;
  void   *ril_additional_unsol_data_ptr = NULL;
  size_t  ril_additional_unsol_data_size = 0;
  qcril_unsol_resp_params_type unsol_resp;

  QCRIL_LOG_INFO("%s", "qcril_gstk_process_raw_command_callback\n");

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen >= sizeof(*cmd_ptr) );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  /* Find and validate the proactive command, and process according to type */
  cmd_ptr = (qcril_gstk_raw_proactive_command_type *) params_ptr->data;
  if ( (cmd_ptr->command_type != GSTK_CMD_STK_END_OF_PROACTIVE_SES) &&
       (cmd_ptr->length < QCRIL_GSTK_COMMAND_MIN_SIZE) )
  {
    QCRIL_LOG_ERROR("Proactive command size (%d) less than minimum (%d)\n",
                    cmd_ptr->length, QCRIL_GSTK_COMMAND_MIN_SIZE);
  }
  else
  {
    switch (cmd_ptr->command_type)
    {
      /* Commands which are processed fully within Android */
      case GSTK_CMD_STK_REFRESH:
      case GSTK_CMD_STK_SET_UP_EVENT_LIST:
      case GSTK_CMD_STK_LAUNCH_BROWSER:
      case GSTK_CMD_STK_PLAY_TONE:
      case GSTK_CMD_STK_DISPLAY_TEXT:
      case GSTK_CMD_STK_GET_INKEY:
      case GSTK_CMD_STK_GET_INPUT:
      case GSTK_CMD_STK_SELECT_ITEM:
      case GSTK_CMD_STK_SET_UP_MENU:
      case GSTK_CMD_STK_SET_UP_IDLE_MODE_TEXT:
      case GSTK_CMD_STK_PROVIDE_LOCAL_INFO:     /* Not sure about this one */
      case GSTK_CMD_STK_LANG_NOTIFICATION:      /* Not sure about this one */
        QCRIL_LOG_INFO("Command will be handled by Android (0x%02lX)\n",
                       cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
        break;

      /* Commands for which Android handles the UI indication, */
      /* and the modem does the real work in parallel          */
      case GSTK_CMD_STK_SEND_SS:
      case GSTK_CMD_STK_USSD:
      case GSTK_CMD_STK_SEND_SHORT_MESSAGE:
      case GSTK_CMD_STK_SEND_DTMF:
        QCRIL_LOG_INFO("Command will be handled by modem with Android UI (0x%02lX)\n",
                        cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
        break;

      /* Commands for which Android handles the UI confirmation, */
      /* and the modem does the real work if the user accepts    */
      case GSTK_CMD_STK_SET_UP_CALL:
        QCRIL_LOG_INFO( "Command will be handled by modem with Android confirmation (0x%02lX)\n", cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
        ril_additional_unsol_type = RIL_UNSOL_STK_CALL_SETUP;
        ril_additional_unsol_data_ptr = &qcril_gstk_info.call_timeout;
        ril_additional_unsol_data_size = sizeof(qcril_gstk_info.call_timeout);
        break;

      /* Pseudo command to indicate the end of the proactive command session */
      case GSTK_CMD_STK_END_OF_PROACTIVE_SES:
        QCRIL_LOG_INFO( "End of Proactive Command Session notification (0x%02lX)\n", cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_SESSION_END;
        break;

      /* Commands which are handled entirely by the modem */
      /* - we shouldn't see any of these                  */
      case GSTK_CMD_STK_MORE_TIME:
      case GSTK_CMD_STK_POLL_INTERVAL:
      case GSTK_CMD_STK_POLLING_OFF:
      case GSTK_CMD_STK_TIMER_MANAGEMENT:
        QCRIL_LOG_ERROR("Command should be handled by modem (0x%02lX)\n",
                        cmd_ptr->command_type);
        break;

      /* Valid command types that are not supported (yet) */
      case GSTK_CMD_STK_OPEN_CHANNEL:
      case GSTK_CMD_STK_CLOSE_CHANNEL:
      case GSTK_CMD_STK_RECEIVE_DATA:
      case GSTK_CMD_STK_SEND_DATA:
      case GSTK_CMD_STK_GET_CHANNEL_STATUS:
      case GSTK_CMD_STK_PERFORM_CARD_APDU:
      case GSTK_CMD_STK_POWER_ON_CARD:
      case GSTK_CMD_STK_POWER_OFF_CARD:
      case GSTK_CMD_STK_GET_READER_STATUS:
      case GSTK_CMD_STK_RUN_AT_COMMAND:
        QCRIL_LOG_ERROR("Command not yet supported (0x%02lX)\n",
                        cmd_ptr->command_type);
        break;

      /* Don't recognise the command type at all */
      default:
        QCRIL_LOG_ERROR("Command type not recognised (0x%02lX)\n",
                        cmd_ptr->command_type);
        break;
    }
  }

  /* Send one or more notifications if required */
  if (ril_unsol_type)
  {
    qcril_default_unsol_resp_params( instance_id, ril_unsol_type, &unsol_resp );
    if ( cmd_ptr->length > 0 )
    {
      unsol_resp.resp_pkt = ( void * ) cmd_ptr->data;
      unsol_resp.resp_len = cmd_ptr->length;
    }
    qcril_send_unsol_response( &unsol_resp );

    /* If the command requires an additional notification then send it now */
    if (ril_additional_unsol_type)
    {
      QCRIL_LOG_INFO("%s", "Additional RIL_ADDITONAL_UNSOL_TYPE will be sent\n");
      qcril_default_unsol_resp_params( instance_id, ril_additional_unsol_type, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) ril_additional_unsol_data_ptr;
      unsol_resp.resp_len = ril_additional_unsol_data_size;
      qcril_send_unsol_response( &unsol_resp );
    }
  }

  /* Free the proactive command */
  free(cmd_ptr);

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_process_raw_command_callback */


/*=========================================================================
  FUNCTION:  qcril_gstk_process_send_raw_envelope_callback
===========================================================================*/
/*!
  @brief
    Handle the QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK event that is
    passed to qcril_event_queue by the callback invoked by GSTK.

  @param[in]  params_ptr  Pointer to a struct containing the data from GSTK
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
    qcril_gstk_request_stk_send_envelope_command
    qcril_gstk_send_raw_envelope_callback
*/
/*=========================================================================*/
void qcril_gstk_process_send_raw_envelope_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_gstk_raw_envelope_response_type *rsp_ptr = NULL;
  RIL_Errno ril_result = RIL_E_SUCCESS;
  void *data_ptr = NULL;
  size_t data_length = 0;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( params_ptr->data != NULL );
  QCRIL_ASSERT( params_ptr->datalen >= sizeof(*rsp_ptr) );
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_process_send_raw_envelope_callback\n");

  /* Find the raw envelope result data */
  rsp_ptr = (qcril_gstk_raw_envelope_response_type *) params_ptr->data;

  /* Was the card happy with the envelope? */
  if (rsp_ptr->result != GSTK_ENVELOPE_CMD_SUCCESS)
  {
    ril_result = RIL_E_GENERIC_FAILURE;
  }

  /* If successful, and we have response data, then prepare to return it */
  if (ril_result == RIL_E_SUCCESS)
  {
    if (rsp_ptr->length != 0)
    {
      data_ptr = rsp_ptr->data;
      data_length = rsp_ptr->length;
    }
  }

  /* Notify command completion and include any response data from the card */
  if (ril_result == RIL_E_SUCCESS)
  {
    qcril_default_request_resp_params( instance_id, (RIL_Token) rsp_ptr->user_data, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = data_ptr;
    resp.resp_len = data_length;
  }
  else
  {
    qcril_default_request_resp_params( instance_id, (RIL_Token) rsp_ptr->user_data, params_ptr->event_id, ril_result, &resp );
  }

  qcril_send_request_response( &resp );

  /* Free the raw envelope result data */
  free(rsp_ptr);

} /* qcril_gstk_process_send_raw_envelope_callback */


#else /* FEATURE_CDMA_NON_RUIM */

/*===========================================================================
  FUNCTION:  qcril_gstk_not_supported
===========================================================================*/
/*!
  @brief
    The function will be called by other functions when FEATURE_CDMA_NON_RUIM
    is not defined.
    
    It only prints out error message and send NOT SUPPORTED error response
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
static void qcril_gstk_not_supported
(
  const char *caller_func_name,
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", caller_func_name);

  if (params_ptr == NULL)
  {
    QCRIL_LOG_INFO("%s", "Null params_ptr\n");
    return;
  }

  qcril_default_request_resp_param( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( &resp );

} /* qcril_gstk_not_supported */

/*===========================================================================
  FUNCTION:  qcril_gstk_init
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_init( void )
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_init(): Not Supported\n");
} /* qcril_gstk_init */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_command_callback
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_command_callback(): Not Supported\n");
} /* qcril_gstk_process_command_callback */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_event_callback
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_event_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_event_callback(): Not Supported\n");
} /* qcril_gstk_process_event_callback */

/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_set_profile
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_request_stk_set_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_not_supported(
    "qcril_gstk_request_stk_set_profile()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_request_stk_set_profile */

/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_get_profile
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_request_stk_get_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_not_supported(
    "qcril_gstk_request_stk_set_profile()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_request_stk_get_profile */

/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_send_envelope_command
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_request_stk_send_envelope_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_not_supported(
    "qcril_gstk_request_stk_send_envelope_command()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_request_stk_send_envelope_command */

/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_send_terminal_response
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_request_stk_send_terminal_response
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_not_supported(
    "qcril_gstk_request_stk_send_terminal_response()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_request_stk_send_terminal_response */

/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_handle_call_setup_requested_from_sim
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_request_stk_handle_call_setup_requested_from_sim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_not_supported(
    "qcril_gstk_request_stk_handle_call_setup_requested_from_sim()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_request_stk_handle_call_setup_requested_from_sim */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_notify_ril_is_ready
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_notify_ril_is_ready
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  /* do nothing */  
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_notify_ril_is_ready(): nothing to do.\n");
} /* qcril_gstk_process_notify_ril_is_ready */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_client_init_callback
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_client_init_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_client_init_callback(): Not Supported\n");
} /* qcril_gstk_process_client_init_callback */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_client_reg_callback
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_client_reg_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_client_reg_callback(): Not Supported\n");
} /* qcril_gstk_process_client_reg_callback */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_raw_command_callback
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_raw_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_raw_command_callback(): Not Supported\n");
} /* qcril_gstk_process_raw_command_callback */

/*===========================================================================
  FUNCTION:  qcril_gstk_process_send_raw_envelope_callback
===========================================================================*/
/*!
  @brief
    Bogus function when FEATURE_CDMA_NON_RUIM is not defined
    
  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_process_send_raw_envelope_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_process_send_raw_envelope_callback(): Not Supported\n");
} /* qcril_gstk_process_send_raw_envelope_callback */
#endif /* !FEATURE_CDMA_NON_RUIM */

/*===========================================================================
  FUNCTION:  qcril_gstk_request_stk_service_is_running
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING

  @param[in]  params_ptr  Suppose to be NULL
  @param[in]  ret_ptr     Suppose to be NULL

  @return
    Nothing
*/
/*=========================================================================*/
void qcril_gstk_request_stk_service_is_running
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_request_stk_service_is_running\n");

  /* Notify GSTK that RIL STK is ready */
  /* This is to solve the issue of SIM card sending proactive cmd too early to RIL */
  qcril_event_queue( instance_id, modem_id, QCRIL_DATA_ON_STACK,
                     QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY, NULL, 0, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  qcril_send_request_response( &resp );

} /* qcril_gstk_request_stk_service_is_running */

#endif /* !FEATURE_QCRIL_QMI_CAT */
