/*!
  @file
  qcril_gstk.c

  @brief
  Handles RIL requests and notifications for SIM Toolkit (GSTK).

*/

/*===========================================================================

  Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

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
12/17/14   hh      Support for retrieve cached proactive commands
09/12/14   at      Change the GSTK disable property name to persist
07/24/14   at      Resolve compiler warnings by using the right qmi_client.h
06/12/14   at      Support for auto device configuration
06/09/14   at      Disable QCRIL GSTK based on the property
06/04/14   at      Support for sending activate unsol proactive cmd
05/27/14   at      Check and set QMI CAT configuration if needed
04/28/14   at      Added function to release QMI CAT client handle
03/07/14   at      Clear proactive cmd cache on setup call user cnf API
03/19/14   vdc     Added response for STK envelope commands
01/15/14   at      Added QMI CAT initialization retry mechanism
12/23/13   at      Support for Fusion 4.5 device configuration
12/02/13   tkl     Avoid int overflow and dup evt tag when filtering evt list
12/11/13   at      Switch to new QCCI framework
10/14/13   yt      Skip filtering of setup event list based on feature flag
10/09/13   vdc     Added support for missing third slot instances
09/27/13   tkl     Fix filtering of non-UI events from set up event list
07/24/13   at      Set the global proactive command pointer to NULL after free
07/22/13   at      Switch to QCCI based QMI_CAT APIs
07/10/13   at      Support for clearing proactive cmd cache on card error
05/15/13   at      Skip timer setup in case of extra STK service running API
05/21/13   tkl     Support for Refresh Alpha
04/23/13   at      Race condition fix during processing of QMI indication
04/17/13   yt      Critical KW fixes
02/28/13   at      Support for SGLTE2 device configuration
02/07/13   at      Support for DSDA2 device configuration
01/28/13   yt      Support for third SIM card slot
01/14/13   yt      Critical KW fixes
12/19/12   at      Move to newer qcril_event_queue API
11/15/12   at      Support for DSDA device configuration
10/08/12   at      Support for RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS
09/20/12   at      Fix for setup/clearing of timed callback for proactive cmds
08/09/12   sc      Packet splitting in qcril_scws_data_available_callback
07/18/12   at      Check for sglte modem type for QMI port
07/13/12   at      Clear global proactive command cache after sending TR
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
11/28/11   at      Fixed slot_mask for set event report in gstk qmi init
10/17/11   yt      Fixed Klocwork warnings
06/23/11   tkl     Populated uim_ref_id in send TR params
06/07/11   at      Support for Dual STK in set event report
04/25/11   at      Added SCWS Support
03/09/11   tkl     Added support of icon conf in event confirmation
03/08/11   tkl     Added support for PLI-Time proactive cmd
02/10/11   at      Find modem type by reading ro.baseband property
02/04/11   at      Slot id support for the module
02/02/11   tkl     Fixed launch browser processing
11/16/10   tkl     Filter out non-UI events from set up event list APDU
09/20/10   tkl     Enabled support for setup evt browser termination and BIP
                   related STK cmd
09/02/10   at      Featurization when opening QMI ports in CAT service init
08/27/10   js      Set up menu fixes
08/20/10   js      Register for provide local info
08/02/10   js      Enabled RIL registration with secondary modem for QMI
07/16/10   js      Enabled support for network related STK cmds
07/07/10   js      Fixed bugs in raw command processing
07/02/10   js      Removed hacks not to send UNSOL response to UI for proactive
                   commands
06/28/10   js      fixed raw command callback
06/15/10   js      Initial version

===========================================================================*/

/*===========================================================================
                                INCLUDE FILES
===========================================================================*/
#if defined (FEATURE_QCRIL_QMI_CAT)

#include <string.h>
#include <cutils/properties.h>
#include "ril.h"
#include "IxErrno.h"
#ifndef FEATURE_CDMA_NON_RUIM
#include "qcril_qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_idl_lib_internal.h"
#include "card_application_toolkit_v02.h"
#endif /* FEATURE_CDMA_NON_RUIM */
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_scws.h"
#include "qmi_client_instance_defs.h"
#include "qmi_cci_target_ext.h"
#include "qmi_ril_platform_dep.h"


/*===========================================================================
                        INTERNAL DEFINITIONS AND TYPES
===========================================================================*/

#define QCRIL_GSTK_QMI_FREE_PTR(ptr)                                        \
  if (ptr != NULL)                                                          \
  {                                                                         \
    qcril_free(ptr);                                                        \
    ptr = NULL;                                                             \
  }                                                                         \

#define QCRIL_GSTK_QMI_MALLOC_AND_CPY_CMD(d_ptr,d_len,s_ptr,s_len)          \
  d_ptr = NULL;                                                             \
  /* Destination buffer needs to hold string data hence *2 */               \
  d_ptr = qcril_malloc(2 * s_len + 1);                                      \
  if(d_ptr != NULL){                                                        \
    memset(d_ptr,0x00,(2 * s_len + 1));                                     \
    qcril_gstk_qmi_bin_to_hexstring(d_ptr, s_ptr, s_len);                   \
    d_len = 2 * s_len + 1;                                                  \
  }                                                                         \

#define QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(length,min,max)               \
  if ((length < min) || (length > max))                                     \
  {                                                                         \
    QCRIL_LOG_ERROR("Length out ot range: 0x%x, discarding TLV", length);   \
    return FALSE;                                                           \
  }                                                                         \

/* Macro to give the number of elements of an array */
#define QCRIL_GSTK_QMI_ARRAY_SIZE(a)            (sizeof(a) / sizeof((a)[0]))

/* Hexadecimal digits, for binary <==> hexadecimal conversions */
#define QCRIL_GSTK_QMI_HEXDIGITS                "0123456789ABCDEF"

/* Mandatory TLV sizes to help validate proactive command data */
#define QCRIL_GSTK_QMI_COMMAND_PROLOGUE_SIZE    2
#define QCRIL_GSTK_QMI_COMMAND_DETAILS_SIZE     5
#define QCRIL_GSTK_QMI_DEVICE_IDENTITIES_SIZE   4
#define QCRIL_GSTK_QMI_COMMAND_MIN_SIZE         (QCRIL_GSTK_QMI_COMMAND_PROLOGUE_SIZE + \
                                                 QCRIL_GSTK_QMI_COMMAND_DETAILS_SIZE  + \
                                                 QCRIL_GSTK_QMI_DEVICE_IDENTITIES_SIZE)

/* Index within GSTK raw command of the start of the command sent to Android */
/* 0 implies BER-TLV, 2 starts at command tag                                */
#define QCRIL_GSTK_QMI_COMMAND_START_INDEX      0

/* Index of Proactive Command Type within command */
#define QCRIL_GSTK_QMI_COMMAND_TYPE_INDEX       (5 - QCRIL_GSTK_QMI_COMMAND_START_INDEX)

/* QCRIL GSTK flag masks */
#define QCRIL_GSTK_QMI_RIL_IS_READY_BMSK         0x00000001
#define QCRIL_GSTK_QMI_CLI_INIT_SUCC_BMSK        0x00000002
#define QCRIL_GSTK_QMI_HAS_SENT_TP_BMSK          0x00000004

/* Index for invalid slot */
#define QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE      0xFF

#define QCRIL_GSTK_MAX_CARD_COUNT                3

/* Macros to deal with moving command and response data around between: */
/*   Android (which expects data in ASCII hex format), and              */
/*   GSTK (which expects data in binary format)                         */
#define QCRIL_GSTK_QMI_COMMAND_DATA_SIZE(n)     (2 * (n))
#define QCRIL_GSTK_QMI_COMMAND_BUFFER_SIZE(n)                \
          (sizeof(qcril_gstk_qmi_raw_proactive_command_type) \
          + QCRIL_GSTK_QMI_COMMAND_DATA_SIZE(n) + 1 )

#define QCRIL_GSTK_QMI_RESPONSE_DATA_SIZE(n)    ((n) / 2)

#define QCRIL_GSTK_QMI_RESPONSE_COPY(pd, ps, n)              \
          qcril_gstk_qmi_hexstring_to_bin((pd), (ps), (n))

#define QCRIL_GSTK_QMI_ENVCMD_DATA_SIZE(n)      ((n) / 2)

#define QCRIL_GSTK_QMI_ENVCMD_COPY(pd, ps, n)                \
          qcril_gstk_qmi_hexstring_to_bin((pd), (ps), (n))

#define QCRIL_GSTK_QMI_ENVRSP_DATA_SIZE(n)      (2 * (n))

#define QCRIL_GSTK_QMI_ENVRSP_COPY(pd, ps, n)                \
          qcril_gstk_qmi_bin_to_hexstring((pd), (ps), (n))

/* COMMAND TYPE INFORMATION
** STK command values
** Refernece - Type of Command Section 8.6 and 9.4 3GPP 31.111 v 3.3.
**
** This is used in the command details to indicate what proactive command is
** being issued from SIM/USIM to the mobile.
*/
#define QCRIL_GSTK_QMI_CMD_STK_REFRESH                0x01
#define QCRIL_GSTK_QMI_CMD_STK_MORE_TIME              0x02
#define QCRIL_GSTK_QMI_CMD_STK_POLL_INTERVAL          0x03
#define QCRIL_GSTK_QMI_CMD_STK_POLLING_OFF            0x04
#define QCRIL_GSTK_QMI_CMD_STK_SET_UP_EVENT_LIST      0x05
#define QCRIL_GSTK_QMI_CMD_STK_SET_UP_CALL            0x10
#define QCRIL_GSTK_QMI_CMD_STK_SEND_SS                0x11
#define QCRIL_GSTK_QMI_CMD_STK_USSD                   0x12
#define QCRIL_GSTK_QMI_CMD_STK_SEND_SHORT_MESSAGE     0x13
#define QCRIL_GSTK_QMI_CMD_STK_SEND_DTMF              0x14
#define QCRIL_GSTK_QMI_CMD_STK_LAUNCH_BROWSER         0x15
#define QCRIL_GSTK_QMI_CMD_STK_PLAY_TONE              0x20
#define QCRIL_GSTK_QMI_CMD_STK_DISPLAY_TEXT           0x21
#define QCRIL_GSTK_QMI_CMD_STK_GET_INKEY              0x22
#define QCRIL_GSTK_QMI_CMD_STK_GET_INPUT              0x23
#define QCRIL_GSTK_QMI_CMD_STK_SELECT_ITEM            0x24
#define QCRIL_GSTK_QMI_CMD_STK_SET_UP_MENU            0x25
#define QCRIL_GSTK_QMI_CMD_STK_PROVIDE_LOCAL_INFO     0x26
#define QCRIL_GSTK_QMI_CMD_STK_TIMER_MANAGEMENT       0x27
#define QCRIL_GSTK_QMI_CMD_STK_SET_UP_IDLE_MODE_TEXT  0x28
#define QCRIL_GSTK_QMI_CMD_STK_LANG_NOTIFICATION      0x35
#define QCRIL_GSTK_QMI_CMD_STK_OPEN_CHANNEL           0x40
#define QCRIL_GSTK_QMI_CMD_STK_CLOSE_CHANNEL          0x41
#define QCRIL_GSTK_QMI_CMD_STK_RECEIVE_DATA           0x42
#define QCRIL_GSTK_QMI_CMD_STK_SEND_DATA              0x43
#define QCRIL_GSTK_QMI_CMD_STK_GET_CHANNEL_STATUS     0x44
#define QCRIL_GSTK_QMI_CMD_STK_ACTIVATE               0x70
#define QCRIL_GSTK_QMI_CMD_STK_END_OF_PROACTIVE_SES   0x81

/* not supported  */
#define QCRIL_GSTK_QMI_CMD_STK_PERFORM_CARD_APDU      0x30
#define QCRIL_GSTK_QMI_CMD_STK_POWER_ON_CARD          0x31
#define QCRIL_GSTK_QMI_CMD_STK_POWER_OFF_CARD         0x32
#define QCRIL_GSTK_QMI_CMD_STK_GET_READER_STATUS      0x33
#define QCRIL_GSTK_QMI_CMD_STK_RUN_AT_COMMAND         0x34

/* Tags */
#define QCRIL_GSTK_QMI_PROACTIVE_SIM_COMMAND_TAG      0xD0
#define QCRIL_GSTK_QMI_IDLE_SCRN_AVAILABLE_EVT        0x05
#define QCRIL_GSTK_QMI_LANGUAGE_SELECTION_EVT         0x07
#define QCRIL_GSTK_QMI_BROWSER_TERMINATION_EVT        0x08
#define QCRIL_GSTK_QMI_HCI_CONNECTIVITY_EVT           0x13
#define QCRIL_GSTK_QMI_PROACTIVE_CMD_LEN_OFFSET       0x01

/* Android system property for fetching the modem type */
#define QCRIL_GSTK_PROPERTY_BASEBAND               "ro.baseband"

/* Android system property values for various modem types */
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_SVLTE_1     "svlte1"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_SVLTE_2A    "svlte2a"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_CSFB        "csfb"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_SGLTE       "sglte"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_SGLTE2      "sglte2"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_MSM         "msm"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_MDMUSB      "mdm"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_APQ         "apq"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_DSDA        "dsda"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_DSDA_2      "dsda2"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_FUSION_4_5  "mdm2"
#define QCRIL_GSTK_PROP_BASEBAND_VALUE_AUTO        "auto"

/* Android property to disable for certain test scenarios */
#define QCRIL_GSTK_PROPERTY_DISABLED               "persist.qcril_gstk.disable"
#define QCRIL_GSTK_PROP_DISABLED_VALUE             "1"

/* Slot masks for set event report */
#define QCRIL_GSTK_QMI_SLOT_MASK_SLOT_1             0x01
#define QCRIL_GSTK_QMI_SLOT_MASK_SLOT_2             0x02
#define QCRIL_GSTK_QMI_SLOT_MASK_SLOT_3             0x04

/* Maximum packet size for scws data available callback */
#define QCRIL_GSTK_QMI_SCWS_DATA_PKT_SIZE           500

/* Synchronous message default timeout (in milli-seconds) */
#define QMI_CAT_DEFAULT_TIMEOUT                     5000
/* QMI Client init timeout (in seconds) */
#define QMI_CAT_INIT_TIMEOUT                        4

/* QMI init retry related defines */
#define QCRIL_GSTK_QMI_INIT_MAX_RETRIES             10
#define QCRIL_GSTK_QMI_INIT_RETRY_INTERVAL           1

/* Bitmask for Set Event Report */
#define QMI_CAT_SET_EVENT_REPORT_DISPLAY_TEXT                     (0x00000001)
#define QMI_CAT_SET_EVENT_REPORT_GET_INKEY                        (0x00000002)
#define QMI_CAT_SET_EVENT_REPORT_GET_INPUT                        (0x00000004)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_MENU                       (0x00000008)
#define QMI_CAT_SET_EVENT_REPORT_SELECT_ITEM                      (0x00000010)
#define QMI_CAT_SET_EVENT_REPORT_SEND_SMS                         (0x00000020)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_USER_ACTIVITY        (0x00000040)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_IDLE_SCREEN_NOTIFY   (0x00000080)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_LANGUAGE_SEL_NOTIFY  (0x00000100)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_IDLE_MODE_TEXT             (0x00000200)
#define QMI_CAT_SET_EVENT_REPORT_LANGUAGE_NOTIFICATION            (0x00000400)
#define QMI_CAT_SET_EVENT_REPORT_REFRESH_ALPHA                    (0x00000800)
#define QMI_CAT_SET_EVENT_REPORT_END_PROACTIVE_SESSION            (0x00001000)
#define QMI_CAT_SET_EVENT_REPORT_PLAY_TONE                        (0x00002000)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_CALL                       (0x00004000)
#define QMI_CAT_SET_EVENT_REPORT_SEND_DTMF                        (0x00008000)
#define QMI_CAT_SET_EVENT_REPORT_LAUNCH_BROWSER                   (0x00010000)
#define QMI_CAT_SET_EVENT_REPORT_SEND_SS                          (0x00020000)
#define QMI_CAT_SET_EVENT_REPORT_SEND_USSD                        (0x00040000)
#define QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_LANG          (0x00080000)
#define QMI_CAT_SET_EVENT_REPORT_BIP                              (0x00100000)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_BROWSER_TERM         (0x00200000)
#define QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_TIME          (0x00400000)
#define QMI_CAT_SET_EVENT_REPORT_SCWS                             (0x00800000)
#define QMI_CAT_SET_EVENT_REPORT_ACTIVATE                         (0x01000000)
#define QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_HCI_CONN             (0x02000000)

/*===========================================================================
                               LOCAL VARIABLES
===========================================================================*/

#ifndef FEATURE_CDMA_NON_RUIM

/* Structure to hold a raw format proactive command */
/* - size will vary according to the command length */
/* NOTE Please keep *data as the last element always */
typedef struct
{
  uint32                    uim_ref_id;   /*!< UIM reference id                 */
  uint32                    command_type; /*!< Proactive command type           */
  qmi_client_type           handle;       /*!< QMI Handle                       */
  uint32                    length;       /*!< Proactive command length         */
  uint8                     *data;        /*!< Proactive command ASCII hex data */
} qcril_gstk_qmi_raw_proactive_command_type;

/*! @brief Variables internal to module qcril_gstk.c */
typedef struct
{
  int                                       call_timeout;   /*<! call timeout (ms) for SETUP_CALL*/

  qcril_gstk_qmi_raw_proactive_command_type *cmd_ptr;         /*!< Pending proactive cmd */

  size_t                                    cmd_len;         /*!< Size of command data pointed by cmd_ptr */

  uint32                                    flags;           /*!< Flags of events having occured */
}qcril_gstk_qmi_command_info_type;

typedef struct
{
  qmi_client_type                           qmi_cat_svc_client_primary;
                                                          /*!< QMI handle for primary modem */
  qmi_client_type                           qmi_cat_svc_client_secondary;
                                                          /*!< QMI handle for secondary modem */
  qcril_gstk_qmi_command_info_type          gstk_command_info[QCRIL_GSTK_MAX_CARD_COUNT];
                                                          /*!< GSTK info, one per card */
  boolean                                   scws_err_sent;
                                                          /*!< Timer ID for proactive commands */
  uint32                                    timer_id;
}qcril_gstk_qmi_info_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_GSTK_QMI_INDICATION_PARAMS_TYPE

   DESCRIPTION:
     Structure used to copy indications received from the
     modem
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_instance_id_e_type         instance_id;
  qcril_modem_id_e_type            modem_id;
  qmi_client_type                  handle;
  unsigned long                    msg_id;
  void                           * msg_ptr;
} qcril_gstk_qmi_ind_params_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_GSTK_ORIGINAL_REQUEST_TYPE

   DESCRIPTION:
     Structure used to copy relevant details in original request that is needed
     by qcril_gstk_qmi_command_cb. Currently supports only the instance ids from
     request, needs to be extended if needed.
-------------------------------------------------------------------------------*/
typedef struct
{
  qcril_instance_id_e_type          instance_id;
  qcril_modem_id_e_type             modem_id;
  uint32                            bip_id;
  qcril_scws_slot_enum_type         slot_id;
} qcril_gstk_original_request_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_GSTK_QMI_RSP_PARAMS_TYPE

   DESCRIPTION:
     Structure used to copy response received from the modem
-------------------------------------------------------------------------------*/
typedef struct
{
  unsigned long                        msg_id;
  void                               * msg_ptr;
  qmi_client_error_type                transp_err;
  qcril_gstk_original_request_type   * orig_req_data;
} qcril_gstk_qmi_rsp_params_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_GSTK_QMI_UNSOL_INFO_TYPE

   DESCRIPTION:
     Structure used to store info that is part of the unsol indication sent
     to Android Telephony
-------------------------------------------------------------------------------*/
typedef struct
{
  int                              ril_unsol_type;
  qcril_unsol_resp_params_type     ril_unsol_resp;
} qcril_gstk_qmi_unsol_info_type;

/*---------------------------------------------------------------------------
   STRUCTURE:    QMI_CLIENT_STRUCT_TYPE

   DESCRIPTION:   Structure used for holding qmi_client_type information
   NOTE: This structure is statically defined in qmi_client.c; not exported.
---------------------------------------------------------------------------*/
struct qmi_client_struct {
  int                               service_user_handle;
  qmi_idl_service_object_type       p_service;
} qmi_client_struct_type;

/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_GSTK_CMD_ID_QMI_REQ_ID_MAP_TYPE

   DESCRIPTION:
     Structure used to map QCRIL_GSTK_QMI_CMD_STK_XXX to cat_cached_command_id_enum
-------------------------------------------------------------------------------*/
typedef struct
{
  uint32                            gstk_cmd_id;
  cat_cached_command_id_enum_v02    qmi_req_cmd_id;
}qcril_gstk_cmd_id_qmi_req_id_map_type;

static qcril_gstk_qmi_info_type qcril_gstk_qmi_info;

/* Timer to resend proactive command  - 5 seconds */
static const struct timeval QCRIL_GSTK_QMI_TIMER_RESEND = { 5, 0 };

static void qcril_gstk_qmi_indication_cb
(
  qmi_client_type                 user_handle,
  unsigned int                    msg_id,
  unsigned char                 * ind_buf_ptr,
  unsigned int                    ind_buf_len,
  void                          * ind_cb_data_ptr
);

void qcril_gstk_qmi_scws_data_available_callback
(
  uint32                      bip_id,
  qcril_scws_slot_enum_type   slot_id,
  uint16                      data_len,
  uint8                     * data_ptr,
  uint16                      remaining_data_len
);

void qcril_gstk_qmi_scws_channel_status_callback
(
  uint32                              bip_id,
  qcril_scws_slot_enum_type           slot_id,
  qcril_scws_socket_state_enum_type   socket_state
);

static void qcril_gstk_qmi_copy_and_save_proactive_cmd
(
  const qcril_gstk_qmi_unsol_info_type     * ril_unsol_resp_ptr,
  uint32                                     command_type,
  uint32                                     user_ref,
  cat_slot_enum_v02                          slot_type,
  qmi_client_type                            qmi_handle
);

static void qcril_gstk_qmi_get_recovery_proactive_cache
(
  qcril_instance_id_e_type  instance_id
);

/*===========================================================================
                             INTERNAL FUNCTIONS
===========================================================================*/

/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_deactivate_timer

===========================================================================*/
/*!
    @brief
    Deactivates any running timer.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_gstk_qmi_deactivate_timer
(
  void
)
{
  if (qcril_gstk_qmi_info.timer_id != 0)
  {
    QCRIL_LOG_INFO("%s", "De-activating QCRIL_GSTK_QMI_TIMER_RESEND");
    qcril_cancel_timed_callback((void *)(uintptr_t) qcril_gstk_qmi_info.timer_id);
    qcril_gstk_qmi_info.timer_id = 0;
  }
} /* qcril_gstk_qmi_deactivate_timer */


/*=========================================================================

  FUNCTION:  qcril_gstk_find_primary_modem_port

===========================================================================*/
/*!
    @brief
    Maps the read Android property value to a QMI port for the modem that
    has the physical connection to the card. If there is no match, a default
    port is returned.

    @return
    Mapped port string value defined by QMI service.
*/
/*=========================================================================*/
static qmi_client_qmux_instance_type qcril_gstk_find_primary_modem_port
(
  char   * prop_value_ptr
)
{
  qmi_client_qmux_instance_type qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;

  /* Sanity check */
  if (prop_value_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL prop_value_ptr, using default port");
    QCRIL_ASSERT(0);
    return qmi_modem_port;
  }

  QCRIL_LOG_INFO("Baseband property value read: %s\n", prop_value_ptr);

  /* Map the port based on the read property */
  if ((strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SVLTE_1)  == 0) ||
      (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SVLTE_2A) == 0) ||
      (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_CSFB)     == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_0;
  }
  else if ((strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_MDMUSB) == 0) ||
           (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SGLTE2) == 0) ||
           (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_AUTO)   == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
  }
  else if ((strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_MSM)   == 0) ||
           (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_APQ)   == 0) ||
           (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SGLTE) == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
  }
  else if (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_DSDA) == 0)
  {
    /* If it is a DSDA configuration, ports are set based on RILD instance */
    if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
    }
    else
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0;
    }
  }
  else if (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_DSDA_2) == 0)
  {
    /* If it is a DSDA2 configuration, ports are set based on RILD instance.
       Note that there is no support of RMNET2 over USB on mainline since that
       config is not supported. Need to revisit this config for future support */
    if (qmi_ril_get_process_instance_id() == QCRIL_DEFAULT_INSTANCE_ID)
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
    }
    else
    {
      qmi_modem_port = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_1;
    }
  }
  else if (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_FUSION_4_5)   == 0)
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
  }
  else
  {
    QCRIL_LOG_ERROR("%s", "Property value does not match, using default port");
  }

  QCRIL_LOG_INFO("QMI port found for modem: 0x%x\n", qmi_modem_port);

  return qmi_modem_port;
} /* qcril_gstk_find_primary_modem_port */


/*=========================================================================

  FUNCTION:  qcril_gstk_find_secondary_modem_port

===========================================================================*/
/*!
    @brief
    Checks if the passed property value is one of the fusion types.

    @return
    Mapped port string value defined by QMI service.
*/
/*=========================================================================*/
static qmi_client_qmux_instance_type qcril_gstk_find_secondary_modem_port
(
  char   * prop_value_ptr
)
{
  qmi_client_qmux_instance_type qmi_modem_port = QMI_CLIENT_QMUX_MAX_INSTANCE_IDS;

  /* Sanity check */
  if (prop_value_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL prop_value_ptr, returning NULL");
    QCRIL_ASSERT(0);
    return qmi_modem_port;
  }

  /* Check the read property if it is a fusion type */
  if ((strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SVLTE_1)  == 0) ||
      (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SVLTE_2A) == 0)  )
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
    QCRIL_LOG_INFO("Modem is a fusion type: %s\n", prop_value_ptr);
  }
  else if ((strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SGLTE)  == 0) ||
           (strcmp(prop_value_ptr, QCRIL_GSTK_PROP_BASEBAND_VALUE_SGLTE2) == 0))
  {
    qmi_modem_port = QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0;
    QCRIL_LOG_INFO("Modem is a fusion type: %s\n", prop_value_ptr);
  }

  return qmi_modem_port;
} /* qcril_gstk_find_secondary_modem_port */


/*=========================================================================

  FUNCTION:  qcril_gstk_get_num_slots

===========================================================================*/
/*!
    @brief
    Returns the number of slots supported by QCRIL.

    @return
    Number of slots.
*/
/*=========================================================================*/
static uint8 qcril_gstk_get_num_slots
(
  void
)
{
  if (ril_to_uim_is_tsts_enabled())
  {
    return 3;
  }
  else if (ril_to_uim_is_dsds_enabled())
  {
    return 2;
  }

  return 1;
} /* qcril_gstk_get_num_slots */


/*=========================================================================

  FUNCTION:  qcril_gstk_find_slot_mask

===========================================================================*/
/*!
    @brief
    Depending upon the type of QCRIL compiled for, determines the value of
    the slot mask to be used. For RPC-based QCRIL, uses the static mask,
    for QMI-based QCRIL, uses mask based on the instance_id found.

    @return
    Slot mask value as determined.
*/
/*=========================================================================*/
static cat_set_event_report_slot_mask_v02 qcril_gstk_find_slot_mask
(
  void
)
{
  cat_set_event_report_slot_mask_v02    slot_mask = 0;
  uint32_t                              slot_id   = 0;

#ifdef FEATURE_QCRIL_UIM_QMI_RPC_QCRIL
  /* RPC-based QCRIL has 2 threads, so init is only once,
     hence the static definitions of slot mask */
  #ifdef FEATURE_QCRIL_DSDS
    slot_mask = CAT_SET_EVENT_REPORT_SLOT_1_V02 | CAT_SET_EVENT_REPORT_SLOT_2_V02;
  #else
    slot_mask = CAT_SET_EVENT_REPORT_SLOT_1_V02;
  #endif
#else
  /* QMI-based QCRIL has 3 process, so init is called once for each process,
     causing 2 QMI calls for event report. Hence we need to determine the
     slot mask for each instance
     Note on the slot id: we now have new API where the slot is:
     0 - if it is regular configuration including DSDA (non-DSDS)
     1 - only if it is a DSDS configuration */
  slot_id = qmi_ril_get_sim_slot();

  /* Update slot_id for event report parameters */
  switch (slot_id)
  {
    case 0:
      slot_mask = CAT_SET_EVENT_REPORT_SLOT_1_V02;
      break;
    case 1:
      slot_mask = CAT_SET_EVENT_REPORT_SLOT_2_V02;
      break;
    case 2:
      slot_mask = CAT_SET_EVENT_REPORT_SLOT_3_V02;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid slot id: 0x%x \n", slot_id);
      break;
  }
#endif /* FEATURE_QCRIL_UIM_QMI_RPC_QCRIL */

  QCRIL_LOG_INFO("slot_mask found: 0x%X", slot_mask);

  return slot_mask;
} /* qcril_gstk_find_slot_mask */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_convert_instance_to_slot_index
 *   ======================================================================*/
/*!
 *   @brief
 *     Converts an instance id to the respective slot index.
 *
 *   @return
 *     Slot index
 *                                                                         */
/*=========================================================================*/
static uint8 qcril_gstk_qmi_convert_instance_to_slot_index
(
  qcril_instance_id_e_type      instance_id
)
{
  uint8 slot_index = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;

  /* Convert instance id to slot index */
  switch (instance_id)
  {
    case QCRIL_DEFAULT_INSTANCE_ID:
      slot_index = 0;
      break;
    case QCRIL_SECOND_INSTANCE_ID:
      if (ril_to_uim_is_dsds_enabled() || ril_to_uim_is_tsts_enabled())
      {
        slot_index = 1;
      }
      break;
    case QCRIL_THIRD_INSTANCE_ID:
      if (ril_to_uim_is_tsts_enabled())
      {
        slot_index = 2;
      }
      break;
    default:
      QCRIL_LOG_ERROR("Invalid instance id for conversion: 0x%x \n", instance_id);
      break;
  }

  QCRIL_LOG_DEBUG( "Slot index found: 0x%x\n", slot_index );
  return slot_index;
} /* qcril_gstk_qmi_convert_instance_to_slot_index */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_convert_slot_type_to_slot_index
 *   =======================================================================*/
/*!
 *   @brief
 *     Converts a slot enum type to the respective slot index.
 *
 *   @return
 *     Array index for the respective slot
 *                                                                         */
/*=========================================================================*/
static uint8 qcril_gstk_qmi_convert_slot_type_to_slot_index
(
  cat_slot_enum_v02      slot_type
)
{
  uint8 slot_index = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;

  /* Convert instance id to slot index */
  switch (slot_type)
  {
    case CAT_SLOT1_V02:
      slot_index = 0;
      break;
    case CAT_SLOT2_V02:
      slot_index = 1;
      break;
    case CAT_SLOT3_V02:
      slot_index = 2;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid slot type for conversion: 0x%x \n", slot_type);
      break;
  }

  QCRIL_LOG_DEBUG( "Slot index found: 0x%x\n", slot_index );
  return slot_index;
} /* qcril_gstk_qmi_convert_slot_type_to_slot_index */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_convert_slot_index_to_slot_type
 *   ======================================================================*/
/*!
 *   @brief
 *     Converts a slot index to the respective slot id.
 *
 *   @return
 *     Nothing
 *                                                                         */
/*=========================================================================*/
static cat_slot_enum_v02 qcril_gstk_qmi_convert_slot_index_to_slot_type
(
  uint8     slot_index
)
{
  cat_slot_enum_v02 slot_type = CAT_SLOT1_V02;

  /* Convert instance id to slot index */
  switch (slot_index)
  {
    case 0:
      slot_type = CAT_SLOT1_V02;
      break;
    case 1:
      slot_type = CAT_SLOT2_V02;
      break;
    case 2:
      slot_type = CAT_SLOT3_V02;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid slot index for conversion: 0x%x \n", slot_index);
      break;
  }

  QCRIL_LOG_DEBUG( "Slot id found: 0x%x\n", slot_type );
  return slot_type;
} /* qcril_gstk_qmi_convert_slot_index_to_slot_type */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_convert_scws_socket_state
 *   ======================================================================*/
/*!
 *   @brief
 *     Converts a socket state to the respective channel state.
 *
 *   @return
 *     Corresponding channel state
 *                                                                         */
/*=========================================================================*/
static cat_scws_channel_state_enum_v02 qcril_gstk_qmi_convert_scws_socket_state
(
  qcril_scws_socket_state_enum_type     socket_state
)
{
  cat_scws_channel_state_enum_v02 channel_state = CAT_SCWS_CHANNEL_CLOSED_STATE_V02;

  /* Convert instance id to slot index */
  switch (socket_state)
  {
    case QCRIL_SCWS_SOCKET_STATE_CLOSED:
      channel_state = CAT_SCWS_CHANNEL_CLOSED_STATE_V02;
      break;
    case QCRIL_SCWS_SOCKET_STATE_LISTEN:
      channel_state = CAT_SCWS_CHANNEL_LISTEN_STATE_V02;
      break;
    case QCRIL_SCWS_SOCKET_STATE_ESTABLISHED:
      channel_state = CAT_SCWS_CHANNEL_ESTABLISHED_STATE_V02;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid socket state for conversion: 0x%x \n", socket_state);
      break;
  }

  QCRIL_LOG_DEBUG( "channel_state converted: 0x%x\n", channel_state );
  return channel_state;
} /* qcril_gstk_qmi_convert_scws_socket_state */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_convert_scws_slot_id
 *   ======================================================================*/
/*!
 *   @brief
 *     Converts the slot id from scws enum type to cat enum type.
 *
 *   @return
 *     Converted cat enum type
 *                                                                         */
/*=========================================================================*/
static cat_slot_enum_v02 qcril_gstk_qmi_convert_scws_slot_id
(
  qcril_scws_slot_enum_type       scws_slot_id_type
)
{
  cat_slot_enum_v02 cat_slot_type = CAT_SLOT1_V02;

  /* Convert scws type to cat type */
  switch (scws_slot_id_type)
  {
    case QCRIL_SCWS_SLOT_1:
      cat_slot_type = CAT_SLOT1_V02;
      break;
    case QCRIL_SCWS_SLOT_2:
      cat_slot_type = CAT_SLOT2_V02;
      break;
    case QCRIL_SCWS_SLOT_3:
      cat_slot_type = CAT_SLOT3_V02;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid input slot_id: %d \n", scws_slot_id_type);
      break;
  }
  return cat_slot_type;
} /* qcril_gstk_qmi_convert_scws_slot_id */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_convert_cat_slot_id
 *   ======================================================================*/
/*!
 *   @brief
 *     Converts the slot id from cat enum type to scws enum type.
 *
 *   @return
 *     Result of the conversion
 *                                                                         */
/*=========================================================================*/
static boolean qcril_gstk_qmi_convert_cat_slot_id
(
  cat_slot_enum_v02               cat_slot_id_type,
  qcril_scws_slot_enum_type     * scws_slot_id_type
)
{
  boolean ret = TRUE;

  /* Sanity check */
  if (scws_slot_id_type == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL scws_slot_id_type");
    QCRIL_ASSERT(0);
    return FALSE;
  }

  /* Convert cat type to scws type */
  switch (cat_slot_id_type)
  {
    case CAT_SLOT1_V02:
      *scws_slot_id_type = QCRIL_SCWS_SLOT_1;
      break;
    case CAT_SLOT2_V02:
      *scws_slot_id_type = QCRIL_SCWS_SLOT_2;
      break;
    case CAT_SLOT3_V02:
      *scws_slot_id_type = QCRIL_SCWS_SLOT_3;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid input slot_id: %d \n", cat_slot_id_type);
      ret = FALSE;
      break;
  }
  return ret;
} /* qcril_gstk_qmi_convert_cat_slot_id */


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
static void qcril_gstk_qmi_bin_to_hexstring
(
 uint8       *dest_ptr,
 const uint8 *src_ptr,
 uint32       length
)
{
  QCRIL_LOG_INFO("qcril_gstk_bin_to_hexstring, length %d\n", (int) length);

  /* Sanity check */
  if ((dest_ptr == NULL) || (src_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  while (length--)
  {
    *dest_ptr++ = QCRIL_GSTK_QMI_HEXDIGITS[(*src_ptr >> 4) & 0x0F];
    *dest_ptr++ = QCRIL_GSTK_QMI_HEXDIGITS[(*src_ptr >> 0) & 0x0F];
    ++src_ptr;
  }
} /* qcril_gstk_qmi_bin_to_hexstring */


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
  return 0;
} /* qcril_gstk_hexdigit_to_bin */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_hexstring_to_bin
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
static void qcril_gstk_qmi_hexstring_to_bin
(
 uint8       *dest_ptr,
 const uint8 *src_ptr,
 uint32       length
)
{
  QCRIL_LOG_INFO("qcril_gstk_hexstring_to_bin, length %d\n", (int) length);

  /* Sanity check */
  if ((dest_ptr == NULL) || (src_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  for (length /= 2; length; --length)
  {
    uint8 temp;

    temp  = qcril_gstk_hexdigit_to_bin(*src_ptr++) << 4;
    temp |= qcril_gstk_hexdigit_to_bin(*src_ptr++) << 0;
    *dest_ptr++ = temp;
  }
} /* qcril_gstk_qmi_hexstring_to_bin */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_send_unsol_resp
 *   ======================================================================*/
/*!
 *   @brief
 *     Utility function to send the unsol response to Android Telephony based
 *     on the passed payload.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_send_unsol_resp
(
  qcril_instance_id_e_type         instance_id,
  int                              ril_unsol_type,
  void                           * resp_pkt_ptr,
  size_t                           resp_pkt_len
)
{
  qcril_unsol_resp_params_type     ril_unsol_resp;

  /* Updated header & send the unsol response. Note: it is OK to send
     a NULL packet with 0 length for certain UNSOL types */
  qcril_default_unsol_resp_params(instance_id, ril_unsol_type, &ril_unsol_resp);
  ril_unsol_resp.resp_pkt = resp_pkt_ptr;
  ril_unsol_resp.resp_len = resp_pkt_len;

  QCRIL_LOG_INFO("Sending unsol_resp, resp_len: 0x%x", ril_unsol_resp.resp_len);
  qcril_send_unsol_response(&ril_unsol_resp);
} /* qcril_gstk_qmi_send_unsol_resp */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_prepare_unsol_response
 *   ======================================================================*/
/*!
 *   @brief
 *     Utility function to update the unsol response that will be sent to
 *     Android Telephony with the incoming TVL information from the modem.
 *     Note that memory allocated here for thh UNSOL response will be freed
 *     at the end of indication processing function.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_prepare_unsol_response
(
  qcril_unsol_resp_params_type       * unsol_resp_ptr,
  const uint8_t                      * proactive_cmd_ptr,
  const uint32_t                       proactive_cmd_len,
  uint32                             * cmd_type_ptr
)
{
  /* Sanity check */
  if ((unsol_resp_ptr    == NULL) ||
      (cmd_type_ptr      == NULL) ||
      (proactive_cmd_ptr == NULL) ||
      (proactive_cmd_len < (QCRIL_GSTK_QMI_COMMAND_TYPE_INDEX + 1)))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process");
    return;
  }

  /* Allocate & copy response packet's data & length */
  QCRIL_GSTK_QMI_MALLOC_AND_CPY_CMD(unsol_resp_ptr->resp_pkt,
                                    unsol_resp_ptr->resp_len,
                                    proactive_cmd_ptr,
                                    proactive_cmd_len);

  /* Update the command type */
  if(proactive_cmd_ptr[1] == 0x81)
  {
    *cmd_type_ptr = proactive_cmd_ptr[QCRIL_GSTK_QMI_COMMAND_TYPE_INDEX+1];
  }
  else
  {
    *cmd_type_ptr = proactive_cmd_ptr[QCRIL_GSTK_QMI_COMMAND_TYPE_INDEX];
  }
} /* qcril_gstk_qmi_prepare_unsol_response */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_filter_setup_events

===========================================================================*/
/*!
    @brief
    Removed events not supported in Android UI from the set up event list
    command

    @return
    pointer to new apdu with un-supported events removed.
*/
/*=========================================================================*/
static uint8 * qcril_gstk_qmi_filter_setup_events
(
  uint8   *apdu_ptr,
  uint16  *apdu_len_ptr
)
{
  uint16    offset                =  0;
  uint16    new_offset            =  0;
  uint16    num_of_event_offset   =  0;
  uint8     len_of_length_field   =  0;
  uint8    *new_apdu_ptr          =  NULL;
  uint16    total_length          =  0;
  uint8     tag                   =  0;
  uint16    tag_len               =  0;
  uint8     i                     =  0;
  uint8     delete_cnt            =  0;
  boolean   evt_list_tag          =  FALSE;


  QCRIL_LOG_INFO( "Entering %s\n", __FUNCTION__);
  if(apdu_ptr == NULL || apdu_len_ptr == NULL || *apdu_len_ptr <= 2)
  {
    QCRIL_LOG_ERROR( "%s\n", "Invalid input APDU Param");
    return new_apdu_ptr;
  }

  /* Make check for proactive command APDU tag the first byte */
  if(apdu_ptr[offset] != QCRIL_GSTK_QMI_PROACTIVE_SIM_COMMAND_TAG)
  {
    QCRIL_LOG_ERROR( "apdu_ptr[%d] != QCRIL_GSTK_QMI_PROACTIVE_SIM_COMMAND_TAG\n", offset);
    return new_apdu_ptr;
  }

  offset++;  /* Points to length field */
  new_offset++;
  /*
  ** BER-TLV LENGTH
  ** This length field can either be 0x00-0x7F or
  ** 0x81 if the 2nd byte is used
  */
  if(apdu_ptr[offset] <= 0x7F)
  {
    len_of_length_field = 1;
  }
  else if(apdu_ptr[offset] == 0x81)
  {
    len_of_length_field = 2;
  }
  else
  {
    QCRIL_LOG_ERROR( "Incorrect length value apdu[%d]=0x%x", offset,apdu_ptr[offset]);
    return new_apdu_ptr;
  }
  /* offset
  ** 0        Proactive SIM Command Tag
  ** 1-2      total length
  ** +1       Command Details tag  0x81
  ** +1       Command details len = 3
  ** +1       Command number
  ** +1       Command Type
  ** +1       Command Qualifier
  */
  if(len_of_length_field == 1)
  {
    total_length = apdu_ptr[offset];
  }
  else
  {
    total_length = apdu_ptr[offset+1];
  }

  if(total_length != (*apdu_len_ptr - 1 - len_of_length_field))
  {
    QCRIL_LOG_ERROR( "Malformed APDU apdu length %d apdu[%d]=0x%x",
                     *apdu_len_ptr,offset,apdu_ptr[offset]);
    return new_apdu_ptr;
  }

  new_apdu_ptr = (uint8*)qcril_malloc(*apdu_len_ptr);
  if(new_apdu_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s \n", "qcril_malloc error new_apdu_ptr\n" );
    return new_apdu_ptr;
  }
  memset(new_apdu_ptr, 0x00,*apdu_len_ptr);

  /* copy Proactive SIM Command Tag & len_of_length_field*/
  memcpy((void*)new_apdu_ptr, (void*)apdu_ptr, (1+ len_of_length_field));
  offset += len_of_length_field;
  new_offset += len_of_length_field;

  while(offset < *apdu_len_ptr)
  {
    /* get the tag len and look for next tag*/
    tag = apdu_ptr[offset];
    new_apdu_ptr[new_offset++] = apdu_ptr[offset++];

    /* check if apdu tag contains a tag len*/
    if(offset >= *apdu_len_ptr)
    {
      QCRIL_LOG_ERROR( "Malformed APDU tag: apdu length %d, offset %d\n",
                   *apdu_len_ptr, offset);
      QCRIL_GSTK_QMI_FREE_PTR(new_apdu_ptr);
      return NULL;
    }

    tag_len = apdu_ptr[offset];
    new_apdu_ptr[new_offset++] = apdu_ptr[offset++];

    /* check for valid tag_len*/
    if(tag_len + offset > *apdu_len_ptr ||
       tag_len + offset > QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02)
    {
      QCRIL_LOG_ERROR( "Incorrect APDU: apdu length %d, offset %d, tag_len %d\n",
                       *apdu_len_ptr, offset, tag_len);
      QCRIL_GSTK_QMI_FREE_PTR(new_apdu_ptr);
      return NULL;
    }

    if(tag == 0x99 || tag == 0x19)
    {
      /* check for evt_list_tag in case tag is seen more than once */
      if (evt_list_tag)
      {
        QCRIL_LOG_ERROR( "duplicat evt_list_tag: evt_list_tag %d\n",
                         evt_list_tag);
        QCRIL_GSTK_QMI_FREE_PTR(new_apdu_ptr);
        return NULL;
      }

      evt_list_tag = TRUE;
      num_of_event_offset = new_offset - 1;

      for(i=0; i < tag_len ; i++)
      {
        /* filter out the non-UI events from set up event list APDU */
        switch(apdu_ptr[offset])
        {
          case QCRIL_GSTK_QMI_IDLE_SCRN_AVAILABLE_EVT:
          case QCRIL_GSTK_QMI_LANGUAGE_SELECTION_EVT:
          case QCRIL_GSTK_QMI_BROWSER_TERMINATION_EVT:
          case QCRIL_GSTK_QMI_HCI_CONNECTIVITY_EVT:
            QCRIL_LOG_INFO("Setup Event 0x%x added \n", apdu_ptr[offset]);
            new_apdu_ptr[new_offset++] = apdu_ptr[offset++];
            break;
          default:
            QCRIL_LOG_INFO("Setup event 0x%x stripped \n", apdu_ptr[offset]);
            offset++;
            delete_cnt++;
            break;
        }
      }
      if(delete_cnt <= new_apdu_ptr[num_of_event_offset])
      {
        /* set correct value of number of events*/
        new_apdu_ptr[num_of_event_offset] -= delete_cnt;
      }
      else
      {
        QCRIL_LOG_ERROR( "Incorrect APDU: delete_cnt (0x%x) > new_apdu_ptr[%d](0x%x)\n",
                          delete_cnt, num_of_event_offset,
                          new_apdu_ptr[num_of_event_offset]);
        QCRIL_GSTK_QMI_FREE_PTR(new_apdu_ptr);
        return NULL;
      }
    }
    else
    {
      /* copy tag field*/
      memcpy((void*)&new_apdu_ptr[new_offset], (void*)&apdu_ptr[offset], tag_len);
      offset += tag_len;
      new_offset += tag_len;
    }
  }

  /* set correct value for length of proactive command */
  if(len_of_length_field == 1)
  {
    new_apdu_ptr[QCRIL_GSTK_QMI_PROACTIVE_CMD_LEN_OFFSET] -= delete_cnt;
  }
  else if(len_of_length_field == 2)
  {
    new_apdu_ptr[QCRIL_GSTK_QMI_PROACTIVE_CMD_LEN_OFFSET+1] -= delete_cnt;
  }

  /* set correct value for overall apdu len*/
  *apdu_len_ptr -= delete_cnt;
  QCRIL_LOG_INFO("apdu_len_ptr 0x%x after filtering\n", *apdu_len_ptr);

  return new_apdu_ptr;

} /* qcril_gstk_qmi_filter_setup_events */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_process_event_report_ind
 *   ======================================================================*/
/*!
 *   @brief
 *     Processes the event report indication by performing the necessary data
 *     parsing, packaging the data and sends the UNSOL response to Android
 *     Telephony. It also sends event confirmation to the modem for certain
 *     cases where there is no Android UI support.
 *
 *   @return
 *     Boolean that specifies success or failure
 *                                                                         */
/*=========================================================================*/
static boolean qcril_gstk_qmi_process_event_report_ind
(
  const qcril_gstk_qmi_ind_params_type   * ind_data_ptr
)
{
  int                                 qmi_err_code      = 0;
  uint32                              user_ref          = 0;
  uint32                              command_type      = 0;
  boolean                             send_evt_conf     = FALSE;
  cat_slot_enum_v02                   slot_type         = CAT_SLOT1_V02;
  qcril_instance_id_e_type            instance_id       = QCRIL_MAX_INSTANCE_ID;
  cat_event_report_ind_msg_v02      * ind_msg_ptr       = NULL;
  qcril_gstk_qmi_unsol_info_type      ril_unsol_resp;
  qcril_gstk_qmi_unsol_info_type      additional_ril_unsol_resp;
  cat_event_confirmation_req_msg_v02  event_conf_req;
  cat_event_confirmation_resp_msg_v02 event_conf_resp;

  /* Sanity check */
  if (ind_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL ind_data_ptr cannot process event report IND");
    return FALSE;
  }

  ind_msg_ptr = (cat_event_report_ind_msg_v02*)ind_data_ptr->msg_ptr;
  if (ind_msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL ind_msg_ptr cannot process event report IND");
    return FALSE;
  }

  memset(&ril_unsol_resp, 0x00, sizeof(qcril_gstk_qmi_unsol_info_type));
  memset(&additional_ril_unsol_resp, 0x00, sizeof(qcril_gstk_qmi_unsol_info_type));

  /* If Slot TLV is present, read it. Otherwise default to slot 1 */
  if (ind_msg_ptr->slot_valid == TRUE)
  {
    slot_type = ind_msg_ptr->slot.slot;
  }

  /* Since we expect only one of the following TLVs, */
  if (ind_msg_ptr->display_text_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->display_text.pc_display_text_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Display Text TLV");
    user_ref                      = ind_msg_ptr->display_text.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->display_text.pc_display_text,
                                          ind_msg_ptr->display_text.pc_display_text_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->get_inkey_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->get_inkey.pc_get_inkey_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Get Inkey TLV");
    user_ref                      = ind_msg_ptr->get_inkey.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->get_inkey.pc_get_inkey,
                                          ind_msg_ptr->get_inkey.pc_get_inkey_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->get_input_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->get_input.pc_get_input_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Get Input TLV");
    user_ref                      = ind_msg_ptr->get_input.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->get_input.pc_get_input,
                                          ind_msg_ptr->get_input.pc_get_input_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->setup_menu_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->setup_menu.pc_setup_menu_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Setup Menu TLV");
    user_ref                      = ind_msg_ptr->setup_menu.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->setup_menu.pc_setup_menu,
                                          ind_msg_ptr->setup_menu.pc_setup_menu_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->select_item_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->select_item.pc_select_item_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Select Item TLV");
    user_ref                      = ind_msg_ptr->select_item.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->select_item.pc_select_item,
                                          ind_msg_ptr->select_item.pc_select_item_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->idle_mode_text_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->idle_mode_text.pc_setup_idle_mode_text_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Setup Idle Mode Text TLV");
    user_ref                      = ind_msg_ptr->idle_mode_text.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->idle_mode_text.pc_setup_idle_mode_text,
                                          ind_msg_ptr->idle_mode_text.pc_setup_idle_mode_text_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->lang_notification_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->lang_notification.pc_lang_notification_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Language Notification TLV");
    user_ref                      = ind_msg_ptr->lang_notification.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->lang_notification.pc_lang_notification,
                                          ind_msg_ptr->lang_notification.pc_lang_notification_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->play_tone_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->play_tone.pc_play_tone_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Play Tone TLV");
    user_ref                      = ind_msg_ptr->play_tone.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->play_tone.pc_play_tone,
                                          ind_msg_ptr->play_tone.pc_play_tone_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->setup_call_valid)
  {
    uint8  slot_index = qcril_gstk_qmi_convert_slot_type_to_slot_index(slot_type);
    if (slot_index < QCRIL_GSTK_MAX_CARD_COUNT)
    {
      QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->setup_call.pc_setup_call_len,
                                            QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                            QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

      QCRIL_LOG_INFO("%s", "Command will be handled by Android: Setup Call TLV");
      user_ref                      = ind_msg_ptr->setup_call.uim_ref_id;
      ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
      qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                            ind_msg_ptr->setup_call.pc_setup_call,
                                            ind_msg_ptr->setup_call.pc_setup_call_len,
                                            &command_type);
      /* Update additional info as well */
      additional_ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_CALL_SETUP;
      additional_ril_unsol_resp.ril_unsol_resp.resp_pkt =
        (void *)&qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout;
      additional_ril_unsol_resp.ril_unsol_resp.resp_len =
        sizeof(qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout);
    }
  }
  else if (ind_msg_ptr->send_sms_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->send_sms.pc_send_sms_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Send SMS TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->send_sms.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->send_sms.pc_send_sms,
                                          ind_msg_ptr->send_sms.pc_send_sms_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->send_dtmf_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->send_dtmf.pc_send_dtmf_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Send DTMF TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->send_dtmf.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->send_dtmf.pc_send_dtmf,
                                          ind_msg_ptr->send_dtmf.pc_send_dtmf_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->launch_browser_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->launch_browser.pc_launch_browser_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Launch Browser TLV");
    user_ref                      = ind_msg_ptr->launch_browser.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->launch_browser.pc_launch_browser,
                                          ind_msg_ptr->launch_browser.pc_launch_browser_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->send_ss_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->send_ss.pc_send_ss_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Send SS TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->send_ss.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->send_ss.pc_send_ss,
                                          ind_msg_ptr->send_ss.pc_send_ss_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->send_ussd_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->send_ussd.pc_send_ussd_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Send USSD TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->send_ussd.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->send_ussd.pc_send_ussd,
                                          ind_msg_ptr->send_ussd.pc_send_ussd_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->provide_local_info_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->provide_local_info.pc_provide_local_info_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Provide Local Information TLV");
    user_ref                      = ind_msg_ptr->provide_local_info.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->provide_local_info.pc_provide_local_info,
                                          ind_msg_ptr->provide_local_info.pc_provide_local_info_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->setup_event_list_raw_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->setup_event_list_raw.pc_setup_event_list_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);
    user_ref                      = ind_msg_ptr->setup_event_list_raw.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;

#ifndef FEATURE_QCRIL_TOOLKIT_SKIP_SETUP_EVT_LIST_FILTER
    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Setup Event List Raw TLV");

    /* We need to filter out the non-UI events from set up event list APDU
       other wise Android UI will send error TR. Android UI does not support
       User Activity and all non-UI event as of now */
    uint16   apdu_length  = ind_msg_ptr->setup_event_list_raw.pc_setup_event_list_len;
    uint8*   temp_buf_ptr = qcril_gstk_qmi_filter_setup_events(
                              ind_msg_ptr->setup_event_list_raw.pc_setup_event_list,
                              &apdu_length);

    if(temp_buf_ptr == NULL)
    {
      QCRIL_LOG_ERROR("%s", "Malformed APDU.");
      return FALSE;
    }

    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          temp_buf_ptr,
                                          apdu_length,
                                          &command_type);
    QCRIL_GSTK_QMI_FREE_PTR(temp_buf_ptr);
#else
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->setup_event_list_raw.pc_setup_event_list,
                                          ind_msg_ptr->setup_event_list_raw.pc_setup_event_list_len,
                                          &command_type);
#endif /* FEATURE_QCRIL_TOOLKIT_SKIP_SETUP_EVT_LIST_FILTER */
  }
  else if (ind_msg_ptr->open_channel_valid)
  {
    uint8 slot_index = qcril_gstk_qmi_convert_slot_type_to_slot_index(slot_type);
    if (slot_index < QCRIL_GSTK_MAX_CARD_COUNT)
    {
      QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->open_channel.pc_open_channel_len,
                                            QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                            QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

      QCRIL_LOG_INFO("%s", "Command will be handled by Android: Open Channel TLV");
      user_ref                      = ind_msg_ptr->open_channel.uim_ref_id;
      ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;

      qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                            ind_msg_ptr->open_channel.pc_open_channel,
                                            ind_msg_ptr->open_channel.pc_open_channel_len,
                                            &command_type);

      /* Update additional info as well. Note:
         Set additional_unsol_type to SETUP CALL until UI handles BIP type */
      additional_ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_CALL_SETUP;
      additional_ril_unsol_resp.ril_unsol_resp.resp_pkt =
        (void *)&qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout;
      additional_ril_unsol_resp.ril_unsol_resp.resp_len =
        sizeof(qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout);
    }
  }
  else if (ind_msg_ptr->close_channel_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->close_channel.pc_close_channel_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Close Channel TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->close_channel.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->close_channel.pc_close_channel,
                                          ind_msg_ptr->close_channel.pc_close_channel_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->send_data_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->send_data.pc_send_data_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Send Data TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->send_data.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->send_data.pc_send_data,
                                          ind_msg_ptr->send_data.pc_send_data_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->receive_data_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->receive_data.pc_receive_data_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Receive Data TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->receive_data.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->receive_data.pc_receive_data,
                                          ind_msg_ptr->receive_data.pc_receive_data_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->proactive_session_end_type_valid)
  {
    QCRIL_LOG_INFO("%s", "Command will be handled by Android: End Proactive Session TLV");
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_SESSION_END;
  }
  else if (ind_msg_ptr->refresh_alpha_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->refresh_alpha.pc_refresh_alpha_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Refresh Alpha TLV");
    send_evt_conf                 = TRUE;
    user_ref                      = ind_msg_ptr->refresh_alpha.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->refresh_alpha.pc_refresh_alpha,
                                          ind_msg_ptr->refresh_alpha.pc_refresh_alpha_len,
                                          &command_type);
  }
  else if (ind_msg_ptr->activate_valid)
  {
    QCRIL_GSTK_QMI_RETURN_IF_OUT_OF_RANGE(ind_msg_ptr->activate.pc_activate_len,
                                          QCRIL_GSTK_QMI_COMMAND_MIN_SIZE,
                                          QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02);

    QCRIL_LOG_INFO("%s", "Command will be handled by Android: Activate TLV");

    user_ref                      = ind_msg_ptr->activate.uim_ref_id;
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
    qcril_gstk_qmi_prepare_unsol_response(&ril_unsol_resp.ril_unsol_resp,
                                          ind_msg_ptr->activate.pc_activate,
                                          ind_msg_ptr->activate.pc_activate_len,
                                          &command_type);
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "Unhandled TLV");
    return FALSE;
  }

  /* Save the command to the global cache before sending UNSOL */
  qcril_gstk_qmi_copy_and_save_proactive_cmd(&ril_unsol_resp,
                                             command_type,
                                             user_ref,
                                             slot_type,
                                             ind_data_ptr->handle);

  /* Convert slot to instance id */
  switch (slot_type)
  {
    case CAT_SLOT1_V02:
      instance_id = QCRIL_DEFAULT_INSTANCE_ID;
      break;
    case CAT_SLOT2_V02:
      instance_id = QCRIL_SECOND_INSTANCE_ID;
      break;
    case CAT_SLOT3_V02:
      instance_id = QCRIL_THIRD_INSTANCE_ID;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid slot id for conversion: 0x%x \n", slot_type);
      break;
  }

  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR("Invalid instance_id: 0x%x\n",instance_id);
    QCRIL_GSTK_QMI_FREE_PTR(ril_unsol_resp.ril_unsol_resp.resp_pkt);
    return FALSE;
  }

  /* Send one or more notifications if required */
  if (ril_unsol_resp.ril_unsol_type)
  {
    qcril_gstk_qmi_send_unsol_resp(instance_id,
                                   ril_unsol_resp.ril_unsol_type,
                                   ril_unsol_resp.ril_unsol_resp.resp_pkt,
                                   ril_unsol_resp.ril_unsol_resp.resp_len);

    /* If the command requires an additional notification then send it now */
    if (additional_ril_unsol_resp.ril_unsol_type)
    {
      QCRIL_LOG_INFO("%s", "Additional RIL_ADDITONAL_UNSOL_TYPE will be sent\n");
      qcril_gstk_qmi_send_unsol_resp(instance_id,
                                     additional_ril_unsol_resp.ril_unsol_type,
                                     additional_ril_unsol_resp.ril_unsol_resp.resp_pkt,
                                     additional_ril_unsol_resp.ril_unsol_resp.resp_len);
    }
    /* After we send the unsol to Telephony, we de-activate the timer so that we dont
       re-send the same pro-active command in case of timer expiry */
    qcril_gstk_qmi_deactivate_timer();
  }

  /* Mock a user confirmation for commands that need a user confirmation and
     where Android is expected to not send it */
  if (send_evt_conf == TRUE)
  {
    memset(&event_conf_req, 0x00, sizeof(cat_event_confirmation_req_msg_v02));
    memset(&event_conf_resp, 0x00, sizeof(cat_event_confirmation_resp_msg_v02));

    /* Limitation in UI: UI has no knowledge if icon is being displayed
       successfully. Assume icon displayed and send icon conf = TRUE */
    event_conf_req.display_valid   = TRUE;
    event_conf_req.display.display = 0x01;
    event_conf_req.slot_valid      = TRUE;
    event_conf_req.slot.slot       = slot_type;

    QCRIL_LOG_QMI(ind_data_ptr->modem_id, "qmi_cat_service", "qmi_cat_event_confirmation" );
    QCRIL_LOG_INFO("%s", "Sending QCCI API QMI_CAT_EVENT_CONFIRMATION_REQ_V02");
    qmi_err_code = qmi_client_send_msg_sync_with_shm(ind_data_ptr->handle,
                                            QMI_CAT_EVENT_CONFIRMATION_REQ_V02,
                                            (void *) &event_conf_req,
                                            sizeof(cat_event_confirmation_req_msg_v02),
                                            (void *) &event_conf_resp,
                                            sizeof(cat_event_confirmation_resp_msg_v02),
                                            QMI_CAT_DEFAULT_TIMEOUT);
    if (qmi_err_code != 0)
    {
      QCRIL_LOG_ERROR("Error for EVENT_CONFIRMATION_REQ, client_err: 0x%x, error_code: 0x%x\n",
                       qmi_err_code, event_conf_resp.resp.error);
    }
  }

  /* Free buffer allocated for the UNSOL response, if any */
  if(ril_unsol_resp.ril_unsol_resp.resp_len > 0)
  {
    QCRIL_GSTK_QMI_FREE_PTR(ril_unsol_resp.ril_unsol_resp.resp_pkt);
  }
  return TRUE;
} /* qcril_gstk_qmi_process_event_report_ind */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_scws_open_channel
 *   ======================================================================*/
/*!
 *   @brief
 *     Sends open channel command to SCWS agent and sends the result back to
 *     the modem.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_scws_open_channel
(
  cat_scws_open_channel_ind_msg_v02    * open_channel_ind_ptr
)
{
  int                                   qmi_err_code = 0;
  cat_slot_enum_v02                     slot_type    = CAT_SLOT1_V02;
  qcril_scws_slot_enum_type             slot_id      = QCRIL_SCWS_SLOT_1;
  cat_scws_open_channel_req_msg_v02     open_channel_req;
  cat_scws_open_channel_resp_msg_v02    open_channel_resp;

  /* Sanity check */
  if (open_channel_ind_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL open_channel_ind_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  /* If Slot TLV is present, use it. Otherwise default to slot 1 */
  if (open_channel_ind_ptr->slot_valid)
  {
    slot_type = open_channel_ind_ptr->slot.slot;
  }

  /* Convert slot_id */
  if (!qcril_gstk_qmi_convert_cat_slot_id(slot_type,
                                          &slot_id))
  {
    QCRIL_LOG_ERROR("Invalid input for slot_id: %d \n", slot_type);
    return;
  }

  if (!open_channel_ind_ptr->open_channel_info_valid)
  {
    QCRIL_LOG_ERROR("%s", "Invalid open channel info TLV, cannot proceed");
    return;
  }

  /* Init open channel params */
  memset(&open_channel_req, 0x00, sizeof(cat_scws_open_channel_req_msg_v02));
  memset(&open_channel_resp, 0x00, sizeof(cat_scws_open_channel_resp_msg_v02));
  open_channel_req.channel_status.ch_id  = open_channel_ind_ptr->open_channel_info.ch_id;
  open_channel_req.channel_status.state  = CAT_SCWS_CHANNEL_LISTEN_STATE_V02;
  open_channel_req.slot.slot             = slot_type;
  open_channel_req.slot_valid            = TRUE;

  QCRIL_LOG_INFO( "%s, <to Agent> for channel_id: 0x%X, cat_slot_type: %d, port: %d \n",
                  __FUNCTION__, open_channel_ind_ptr->open_channel_info.ch_id,
                  slot_type, open_channel_ind_ptr->open_channel_info.port);

  if (!qcril_scws_open_channel((uint16)open_channel_ind_ptr->open_channel_info.port,
                               (uint32)open_channel_ind_ptr->open_channel_info.ch_id,
                               slot_id))
  {
    /* On error, update channel_state parameter to 'closed' */
    open_channel_req.channel_status.state = CAT_SCWS_CHANNEL_CLOSED_STATE_V02;
  }

  /* Send the result to the card */
  QCRIL_LOG_INFO( "%s, <to Card> for channel_id: 0x%X, channel_state: 0x%X \n",
                  __FUNCTION__, open_channel_req.channel_status.ch_id,
                  open_channel_req.channel_status.state);

  qmi_err_code = qmi_client_send_msg_sync_with_shm( qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                                           QMI_CAT_SCWS_OPEN_CHANNEL_REQ_V02,
                                           (void *) &open_channel_req,
                                           sizeof(cat_scws_open_channel_req_msg_v02),
                                           (void *) &open_channel_resp,
                                           sizeof(cat_scws_open_channel_resp_msg_v02),
                                           QMI_CAT_DEFAULT_TIMEOUT);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_ERROR("Error for OPEN_CHANNEL_REQ, client_err: 0x%x, error_code: 0x%x\n",
                     qmi_err_code, open_channel_resp.resp.error);
  }
} /* qcril_gstk_qmi_scws_open_channel */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_scws_close_channel
 *   ======================================================================*/
/*!
 *   @brief
 *     Sends close channel command to SCWS agent and sends the result back to
 *     the modem.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_scws_close_channel
(
  cat_scws_close_channel_ind_msg_v02    * close_channel_ind_ptr
)
{
  int                                   qmi_err_code = 0;
  cat_slot_enum_v02                     slot_type    = CAT_SLOT1_V02;
  qcril_scws_slot_enum_type             slot_id      = QCRIL_SCWS_SLOT_1;
  boolean                               close_server = FALSE;
  cat_scws_close_channel_req_msg_v02    close_channel_req;
  cat_scws_close_channel_resp_msg_v02   close_channel_resp;

  /* Sanity check */
  if (close_channel_ind_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL close_channel_ind_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  /* If Slot TLV is present, use it. Otherwise default to slot 1 */
  if (close_channel_ind_ptr->slot_valid)
  {
    slot_type = close_channel_ind_ptr->slot.slot;
  }

  /* Convert slot_id */
  if (!qcril_gstk_qmi_convert_cat_slot_id(slot_type,
                                          &slot_id))
  {
    QCRIL_LOG_ERROR("Invalid input for slot_id: %d \n", slot_type);
    return;
  }

  if (!close_channel_ind_ptr->close_channel_info_valid)
  {
    QCRIL_LOG_ERROR("%s", "Invalid close channel info TLV, cannot proceed");
    return;
  }

  if (close_channel_ind_ptr->close_channel_info.state == CAT_SCWS_CHANNEL_CLOSED_STATE_V02)
  {
    close_server = TRUE;
  }

  QCRIL_LOG_INFO( "%s, <to Agent> for channel_id: 0x%X, cat_slot_type: %d, close_server: %s \n",
                  __FUNCTION__, close_channel_ind_ptr->close_channel_info.ch_id,
                  slot_type, !close_server ? "FALSE" : "TRUE" );

  /* Init close channel params */
  memset(&close_channel_req, 0x00, sizeof(cat_scws_close_channel_req_msg_v02));
  memset(&close_channel_resp, 0x00, sizeof(cat_scws_close_channel_resp_msg_v02));
  close_channel_req.channel_status.ch_id = close_channel_ind_ptr->close_channel_info.ch_id;
  close_channel_req.channel_status.state = close_channel_ind_ptr->close_channel_info.state;
  close_channel_req.slot.slot            = slot_type;
  close_channel_req.slot_valid           = TRUE;

  if (!qcril_scws_close_channel((uint32)close_channel_ind_ptr->close_channel_info.ch_id,
                                slot_id,
                                close_server))
  {
    /* On error, update channel_state parameter to 'closed' */
    close_channel_req.channel_status.state = CAT_SCWS_CHANNEL_CLOSED_STATE_V02;
  }

  /* Send the result to the card */
  QCRIL_LOG_INFO( "%s, <to card> for channel_id: 0x%X, channel_state: 0x%X \n",
                  __FUNCTION__, close_channel_req.channel_status.ch_id,
                  close_channel_req.channel_status.state);

  qmi_err_code = qmi_client_send_msg_sync_with_shm( qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                                           QMI_CAT_SCWS_CLOSE_CHANNEL_REQ_V02,
                                           (void *) &close_channel_req,
                                           sizeof(cat_scws_close_channel_req_msg_v02),
                                           (void *) &close_channel_resp,
                                           sizeof(cat_scws_close_channel_resp_msg_v02),
                                           QMI_CAT_DEFAULT_TIMEOUT);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_ERROR("Error for CLOSE_CHANNEL_REQ, client_err: 0x%x, error_code: 0x%x\n",
                     qmi_err_code, close_channel_resp.resp.error);
  }
} /* qcril_gstk_qmi_scws_close_channel */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_scws_send_data
 *   ======================================================================*/
/*!
 *   @brief
 *     Sends the data received from the modem to the SCWS agent and the result
 *     of the operation abck to modem. It first concatenates all the packets
 *     sent from modem if applicable and then sends the buffer the agent.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_scws_send_data
(
  cat_scws_send_data_ind_msg_v02    * send_data_ind_ptr
)
{
  int                                   qmi_err_code = 0;
  cat_slot_enum_v02                     slot_type    = CAT_SLOT1_V02;
  qcril_scws_slot_enum_type             slot_id      = QCRIL_SCWS_SLOT_1;
  cat_scws_send_data_req_msg_v02        send_data_req;
  cat_scws_send_data_resp_msg_v02       send_data_resp;

  /* Sanity check */
  if (send_data_ind_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL send_data_ind_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  /* If Slot TLV is present, use it. Otherwise default to slot 1 */
  if (send_data_ind_ptr->slot_valid)
  {
    slot_type = send_data_ind_ptr->slot.slot;
  }

  /* Convert slot_id */
  if (!qcril_gstk_qmi_convert_cat_slot_id(slot_type,
                                          &slot_id))
  {
    QCRIL_LOG_ERROR("Invalid input for slot_id: %d \n", slot_type);
    return;
  }

  if (!send_data_ind_ptr->send_data_info_valid)
  {
    QCRIL_LOG_ERROR("%s", "Invalid send data info TLV, cannot proceed");
    return;
  }

  QCRIL_LOG_INFO( "%s, channel_id: 0x%X, cat_slot_type: %d, total_packets: %d, current_packet: %d, data_len: 0x%X \n",
                  __FUNCTION__, send_data_ind_ptr->send_data_info.ch_id, slot_type,
                  send_data_ind_ptr->send_data_info.total_packets, send_data_ind_ptr->send_data_info.current_packet,
                  send_data_ind_ptr->send_data_info.data_len );

  /* Init send data params */
  memset(&send_data_req, 0x00, sizeof(cat_scws_send_data_req_msg_v02));
  memset(&send_data_resp, 0x00, sizeof(cat_scws_send_data_resp_msg_v02));
  send_data_req.result.ch_id  = send_data_ind_ptr->send_data_info.ch_id;
  send_data_req.result.result = TRUE;
  send_data_req.slot_valid    = TRUE;
  send_data_req.slot.slot     = slot_type;

  if ((send_data_ind_ptr->send_data_info.total_packets == 0) ||
      (send_data_ind_ptr->send_data_info.current_packet == 0) ||
      (send_data_ind_ptr->send_data_info.data_len == 0) ||
      (send_data_ind_ptr->send_data_info.data_len > QMI_CAT_SCWS_DATA_MAX_LENGTH_V02) ||
      (send_data_ind_ptr->send_data_info.current_packet > send_data_ind_ptr->send_data_info.total_packets))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input for qmi_cat_scws_send_data! \n");
    return;
  }

  /* Ignore subsequent packets on error */
  if (qcril_gstk_qmi_info.scws_err_sent)
  {
    if (send_data_ind_ptr->send_data_info.current_packet ==
        send_data_ind_ptr->send_data_info.total_packets)
    {
      qcril_gstk_qmi_info.scws_err_sent = FALSE;
    }
    QCRIL_LOG_DEBUG( "Already acknowledged send_data with error, ignoring packet: %d\n",
                     send_data_ind_ptr->send_data_info.current_packet );
    return;
  }

  /* Send the packet to the Agent */
  QCRIL_LOG_INFO( "%s, <to Agent> for channel_id: 0x%X, data_len: 0x%X \n", __FUNCTION__,
                  send_data_ind_ptr->send_data_info.ch_id,
                  send_data_ind_ptr->send_data_info.data_len );

  if (!qcril_scws_send_data((uint32)send_data_ind_ptr->send_data_info.ch_id,
                            slot_id,
                            send_data_ind_ptr->send_data_info.data,
                            send_data_ind_ptr->send_data_info.data_len))
  {
    /* On error, update result parameter to 'failure' */
    send_data_req.result.result = FALSE;
  }

  /* For the last packet or an error condition, send response to modem */
  if ((send_data_ind_ptr->send_data_info.total_packets ==
        send_data_ind_ptr->send_data_info.current_packet) ||
      (send_data_req.result.result == FALSE))
  {
    QCRIL_LOG_INFO( "%s, <to Card> for channel_id: 0x%X, result: %s \n",
                    __FUNCTION__, send_data_req.result.ch_id,
                    (send_data_req.result.result == TRUE) ? "Success" : "Failure" );

    qmi_err_code = qmi_client_send_msg_sync_with_shm( qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                                             QMI_CAT_SCWS_SEND_DATA_REQ_V02,
                                             (void *) &send_data_req,
                                             sizeof(cat_scws_send_data_req_msg_v02),
                                             (void *) &send_data_resp,
                                             sizeof(cat_scws_send_data_resp_msg_v02),
                                             QMI_CAT_DEFAULT_TIMEOUT);
    if (qmi_err_code != 0)
    {
      QCRIL_LOG_ERROR("Error for SEND_DATA_REQ, client_err: 0x%x, error_code: 0x%x\n",
                       qmi_err_code, send_data_resp.resp.error);
    }

    /* If there was an error for a intermediate packet, we still get the remaining packets
       in that set, so keep track of it */
    if ((send_data_ind_ptr->send_data_info.total_packets !=
          send_data_ind_ptr->send_data_info.current_packet) &&
        (send_data_req.result.result == FALSE))
    {
      qcril_gstk_qmi_info.scws_err_sent = TRUE;
    }
  }
} /* qcril_gstk_qmi_scws_send_data */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_scws_deinit
 *   ======================================================================*/
/*!
 *   @brief
 *     De-initializes the SCWS agent.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_scws_deinit
(
  void
)
{
  qcril_scws_deinitalize();
} /* qcril_gstk_qmi_scws_deinit */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_resend_proactive_cmd
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
static void qcril_gstk_qmi_resend_proactive_cmd
(
  void *param_ptr
)
{
  uint32 timer_id = (uint32)(uintptr_t) param_ptr;
  qcril_instance_id_e_type instance_id =
    QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( timer_id );
  qcril_modem_id_e_type modem_id =
    QCRIL_EXTRACT_MODEM_ID_FROM_USER_DATA( timer_id );
  uint8  slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
  if(slot_index >= qcril_gstk_get_num_slots())
  {
    QCRIL_LOG_ERROR("Invalid slot index 0x%x for instance_id 0x%x",
                    slot_index, instance_id);
    return;
  }

  /*-----------------------------------------------------------------------*/

  /* Clear the timer id */
  qcril_gstk_qmi_info.timer_id = 0;

  QCRIL_LOG_DEBUG( "Timer expired with ID %d\n", (uint32) timer_id );

  if (!(qcril_gstk_qmi_info.gstk_command_info[slot_index].flags & QCRIL_GSTK_QMI_RIL_IS_READY_BMSK))
  {
    QCRIL_LOG_INFO("%s", "STK service still not ready!");

    /* Skip timer setup if the cache is empty, since we expect to receive from modem */
    if (qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr != NULL)
    {
      qcril_setup_timed_callback( instance_id, modem_id,
                                  qcril_gstk_qmi_resend_proactive_cmd,
                                  &QCRIL_GSTK_QMI_TIMER_RESEND,
                                  &timer_id );
      qcril_gstk_qmi_info.timer_id = timer_id;
    }
  }
  else
  {
    if (qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr != NULL)
    {
      size_t       total_cmd_len = sizeof(qcril_gstk_qmi_raw_proactive_command_type) +
                                     qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_len;
      IxErrnoType  result        = E_FAILURE;

      QCRIL_LOG_INFO("%s", "Resending proactive cmd to RIL.\n");

      /* Resend the pending proactive command */
      result = qcril_event_queue(instance_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_DATA_NOT_ON_STACK,
                                 QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK,
                                 qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr,
                                 total_cmd_len,
                                 (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
      if (result != E_SUCCESS)
      {
        QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);
        /* No need to free anything in this case */
      }
    }
    else
    {
      QCRIL_LOG_DEBUG("qcril_gstk_qmi_info.gstk_command_info[%d].cmd_ptr is NULL!\n", slot_index);
    }
  }
} /* qcril_gstk_qmi_resend_proactive_cmd */


/*=========================================================================
  FUNCTION:  qcril_gstk_qmi_process_raw_command_callback
===========================================================================*/
/*!
  @brief
    Processes session end notifications and raw proactive commands which
    QMI CAT delivers to its event callback, qcril_gstk_qmi_indication_cb.

    The raw command data is already converted to the form Android requires
    by qcril_gstk_qmi_indication_cb, as that halves the number of
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

  @see

*/
/*=========================================================================*/
void qcril_gstk_qmi_process_raw_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type  instance_id                    = QCRIL_DEFAULT_INSTANCE_ID;
  int                       ril_unsol_type                 = 0;
  int                       ril_additional_unsol_type      = 0;
  void                    * ril_additional_unsol_data_ptr  = NULL;
  size_t                    ril_additional_unsol_data_size = 0;
  uint8                     slot_index                     = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  qcril_gstk_qmi_raw_proactive_command_type   * cmd_ptr    = NULL;

  QCRIL_LOG_DEBUG("%s", "qcril_gstk_qmi_process_raw_command_callback\n");

  /*-----------------------------------------------------------------------*/

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;

  if ((params_ptr->data    == NULL) ||
      (params_ptr->datalen <  sizeof(qcril_gstk_qmi_raw_proactive_command_type)) ||
      (instance_id         >= QCRIL_MAX_INSTANCE_ID))
  {
    QCRIL_LOG_ERROR("Invalid input, NULL data pointer or params_ptr->datalen: 0x%x, instance_id: 0x%x",
                    params_ptr->datalen, instance_id);
    QCRIL_ASSERT(0);
    return;
  }

  /*-----------------------------------------------------------------------*/

  /* Find and validate the proactive command, and process according to type */
  cmd_ptr = (qcril_gstk_qmi_raw_proactive_command_type *) params_ptr->data;

  if ( (cmd_ptr->command_type != QCRIL_GSTK_QMI_CMD_STK_END_OF_PROACTIVE_SES) &&
       (cmd_ptr->length < QCRIL_GSTK_QMI_COMMAND_MIN_SIZE) )
  {
    QCRIL_LOG_ERROR("Proactive command size (%d) less than minimum (%d)\n",
                    cmd_ptr->length, QCRIL_GSTK_QMI_COMMAND_MIN_SIZE);
    return;
  }
  else
  {
    switch (cmd_ptr->command_type)
    {
      /* Commands which are processed fully within Android */
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_EVENT_LIST:
      case QCRIL_GSTK_QMI_CMD_STK_LAUNCH_BROWSER:
      case QCRIL_GSTK_QMI_CMD_STK_PLAY_TONE:
      case QCRIL_GSTK_QMI_CMD_STK_DISPLAY_TEXT:
      case QCRIL_GSTK_QMI_CMD_STK_GET_INKEY:
      case QCRIL_GSTK_QMI_CMD_STK_GET_INPUT:
      case QCRIL_GSTK_QMI_CMD_STK_SELECT_ITEM:
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_MENU:
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_IDLE_MODE_TEXT:
      case QCRIL_GSTK_QMI_CMD_STK_PROVIDE_LOCAL_INFO:
      case QCRIL_GSTK_QMI_CMD_STK_LANG_NOTIFICATION:
      case QCRIL_GSTK_QMI_CMD_STK_ACTIVATE:
        QCRIL_LOG_DEBUG("Command will be handled by Android (0x%02lX)\n",
                       cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_PROACTIVE_COMMAND;
        break;

      /* Commands for which Android handles the UI indication, */
      /* and the modem does the real work in parallel          */
      case QCRIL_GSTK_QMI_CMD_STK_SEND_SS:
      case QCRIL_GSTK_QMI_CMD_STK_USSD:
      case QCRIL_GSTK_QMI_CMD_STK_SEND_SHORT_MESSAGE:
      case QCRIL_GSTK_QMI_CMD_STK_SEND_DTMF:
      case QCRIL_GSTK_QMI_CMD_STK_CLOSE_CHANNEL:
      case QCRIL_GSTK_QMI_CMD_STK_RECEIVE_DATA:
      case QCRIL_GSTK_QMI_CMD_STK_SEND_DATA:
      case QCRIL_GSTK_QMI_CMD_STK_REFRESH:
        QCRIL_LOG_INFO(
          "Command will be handled by modem with Android UI (0x%02lX)\n",
          cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
        break;

      /* Commands for which Android handles the UI confirmation, */
      /* and the modem does the real work if the user accepts    */
      case QCRIL_GSTK_QMI_CMD_STK_OPEN_CHANNEL:
        QCRIL_LOG_INFO(
          "Command will be handled by modem with Android confirmation (0x%02lX)\n",
           cmd_ptr->command_type);

        slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
        if( slot_index < qcril_gstk_get_num_slots() )
        {
          ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
          /* additional_unsol_type is set to STK_CALL_SETUP till a new unsol type
             is avail for BIP*/
          ril_additional_unsol_type = RIL_UNSOL_STK_CALL_SETUP;
          ril_additional_unsol_data_ptr = &qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout;
          ril_additional_unsol_data_size = sizeof(qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout);
        }
        else
        {
          QCRIL_LOG_ERROR("invalid slot_index 0x%x", slot_index);
          QCRIL_ASSERT(0);
        }
        break;

      /* Commands for which Android handles the UI confirmation, */
      /* and the modem does the real work if the user accepts    */
      case QCRIL_GSTK_QMI_CMD_STK_SET_UP_CALL:
        QCRIL_LOG_INFO(
          "Command will be handled by modem with Android confirmation (0x%02lX)\n",
           cmd_ptr->command_type);

        slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
        if( slot_index < qcril_gstk_get_num_slots() )
        {
          ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
          ril_additional_unsol_type = RIL_UNSOL_STK_CALL_SETUP;
          ril_additional_unsol_data_ptr = &qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout;
          ril_additional_unsol_data_size = sizeof(qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout);
        }
        else
        {
          QCRIL_LOG_ERROR("invalid slot_index 0x%x", slot_index);
          QCRIL_ASSERT(0);
        }
        break;

      /* Pseudo command to indicate the end of the proactive command session */
      case QCRIL_GSTK_QMI_CMD_STK_END_OF_PROACTIVE_SES:
        QCRIL_LOG_INFO(
          "End of Proactive Command Session notification (0x%02lX)\n",
           cmd_ptr->command_type);
        ril_unsol_type = RIL_UNSOL_STK_SESSION_END;
        break;

      /* Commands which are handled entirely by the modem */
      /* - we shouldn't see any of these                  */
      case QCRIL_GSTK_QMI_CMD_STK_MORE_TIME:
      case QCRIL_GSTK_QMI_CMD_STK_POLL_INTERVAL:
      case QCRIL_GSTK_QMI_CMD_STK_POLLING_OFF:
      case QCRIL_GSTK_QMI_CMD_STK_TIMER_MANAGEMENT:
        QCRIL_LOG_ERROR("Command should be handled by modem (0x%02lX)\n",
                        cmd_ptr->command_type);
        break;

      /* Valid command types that are not supported (yet) */
      case QCRIL_GSTK_QMI_CMD_STK_GET_CHANNEL_STATUS:
      case QCRIL_GSTK_QMI_CMD_STK_PERFORM_CARD_APDU:
      case QCRIL_GSTK_QMI_CMD_STK_POWER_ON_CARD:
      case QCRIL_GSTK_QMI_CMD_STK_POWER_OFF_CARD:
      case QCRIL_GSTK_QMI_CMD_STK_GET_READER_STATUS:
      case QCRIL_GSTK_QMI_CMD_STK_RUN_AT_COMMAND:
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
    qcril_gstk_qmi_send_unsol_resp(instance_id,
                                   ril_unsol_type,
                                   (void *)&cmd_ptr->data,
                                   (size_t)cmd_ptr->length);

    /* If the command requires an additional notification then send it now */
    if (ril_additional_unsol_type)
    {
      QCRIL_LOG_INFO("%s", "Additional RIL_ADDITONAL_UNSOL_TYPE will be sent\n");
      qcril_gstk_qmi_send_unsol_resp(instance_id,
                                     ril_additional_unsol_type,
                                     ril_additional_unsol_data_ptr,
                                     ril_additional_unsol_data_size);
    }
  }

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_qmi_process_raw_command_callback */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_copy_and_save_proactive_cmd
 *   ===========================================================================*/
/*!
 *   @brief
 *     Copies & saves proactive command into global data structure
 *
 *   @return
 *     Nothing
 *                                                                    */
/*=========================================================================*/
static void qcril_gstk_qmi_copy_and_save_proactive_cmd
(
  const qcril_gstk_qmi_unsol_info_type     * ril_unsol_resp_ptr,
  uint32                                     command_type,
  uint32                                     user_ref,
  cat_slot_enum_v02                          slot_type,
  qmi_client_type                            qmi_handle
)
{
  uint8                                        slot_index = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  qcril_gstk_qmi_raw_proactive_command_type  * cmd_ptr = NULL;
  size_t                                       cmd_len = 0;

  /* Sanity checks */
  if (ril_unsol_resp_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "NULL ril_unsol_resp_ptr, cannot save");
    return;
  }

  if ((ril_unsol_resp_ptr->ril_unsol_resp.resp_pkt == NULL) ||
      (ril_unsol_resp_ptr->ril_unsol_resp.resp_len == 0))
  {
    QCRIL_LOG_ERROR( "%s", "Invalid ril_unsol_resp parameters, cannot save");
    return;
  }

  /* Convert slot id type to an index value */
  slot_index = qcril_gstk_qmi_convert_slot_type_to_slot_index(slot_type);
  if (slot_index >= QCRIL_GSTK_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot_index: 0x%x, cannot save", slot_index);
    QCRIL_ASSERT(0);
    return;
  }

  cmd_ptr = qcril_malloc(sizeof(qcril_gstk_qmi_raw_proactive_command_type)+
                         ril_unsol_resp_ptr->ril_unsol_resp.resp_len);
  if (cmd_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Error in malloc for cmd_ptr, cannot save");
    return;
  }

  /* Allocate the command buffer & populate it */
  memset(cmd_ptr,0x00, sizeof(qcril_gstk_qmi_raw_proactive_command_type)+
         ril_unsol_resp_ptr->ril_unsol_resp.resp_len);
  cmd_ptr->length       = ril_unsol_resp_ptr->ril_unsol_resp.resp_len;
  cmd_ptr->command_type = command_type;
  cmd_ptr->uim_ref_id   = user_ref;
  cmd_ptr->handle       = qmi_handle;
  /* align data ptr */
  cmd_ptr->data         = (uint8*)(&cmd_ptr->data);
  memcpy(cmd_ptr->data,
         ril_unsol_resp_ptr->ril_unsol_resp.resp_pkt,
         ril_unsol_resp_ptr->ril_unsol_resp.resp_len);

  /* Save the command for the respective slot */
  if (qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr != NULL)
  {
    QCRIL_LOG_INFO("%s", "Dismiss pending proactive cmd!\n");
    QCRIL_GSTK_QMI_FREE_PTR(qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr);
  }
  qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr = cmd_ptr;
  qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_len = cmd_len;
} /* qcril_gstk_qmi_copy_and_save_proactive_cmd */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_copy_indication

===========================================================================*/
/*!
    @brief
    Decodes & makes a copy of the indication received from QMI.

    @return
    Pointer to copy of the indication
*/
/*=========================================================================*/
static qcril_gstk_qmi_ind_params_type * qcril_gstk_qmi_copy_indication
(
  qmi_client_type                      user_handle_ptr,
  unsigned long                        msg_id,
  const unsigned char                * ind_data_ptr,
  int                                  ind_data_len,
  uint32_t                           * total_size_ptr
)
{
  void                           * decoded_payload_ptr  = NULL;
  uint32_t                         decoded_payload_len  = 0;
  qcril_gstk_qmi_ind_params_type * out_ptr              = NULL;
  qmi_client_error_type            qmi_err              = QMI_INTERNAL_ERR;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if ((user_handle_ptr == NULL) ||
      (ind_data_ptr    == NULL) ||
      (total_size_ptr  == NULL))
  {
    QCRIL_LOG_ERROR( "%s\n", "Invalid input, cannot proceed");
    QCRIL_ASSERT(0);
    return NULL;
  }

  /* First decode the message payload from QCCI */
  qmi_idl_get_message_c_struct_len(cat_get_service_object_v02(),
                                   QMI_IDL_INDICATION,
                                   msg_id,
                                   &decoded_payload_len);
  if (decoded_payload_len == 0)
  {
    QCRIL_LOG_ERROR("%s: Failed to find decoded_payload_len");
    return NULL;
  }

  /* Allocate decoded payload buffer */
  decoded_payload_ptr = qcril_malloc(decoded_payload_len);
  if (decoded_payload_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Failed to allocate payload ptr, payload len: 0x%x\n",
                    decoded_payload_len);
    return NULL;
  }

  /* Decode the Indication payload */
  qmi_err = qmi_client_message_decode(user_handle_ptr,
                                      QMI_IDL_INDICATION,
                                      msg_id,
                                      ind_data_ptr,
                                      ind_data_len,
                                      decoded_payload_ptr,
                                      decoded_payload_len);
  if (qmi_err != QMI_NO_ERR)
  {
    QCRIL_LOG_ERROR("Failed to decode Indication: 0x%x, qmi_err: 0x%x",
                    msg_id, qmi_err);
    QCRIL_GSTK_QMI_FREE_PTR(decoded_payload_ptr);
    return NULL;
  }

  /* Note: out_ptr and decoded_payload_ptr will be freed after
     processing the event in qcril_gstk_qmi_process_qmi_indication */
  out_ptr = qcril_malloc(sizeof(qcril_gstk_qmi_ind_params_type));
  if (out_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "out_ptr alloc failed");
    QCRIL_GSTK_QMI_FREE_PTR(decoded_payload_ptr);
    return NULL;
  }

  /* Initialize the payload data. Assign the message data pointer based
     on the Message ID of the incoming IND */
  *total_size_ptr = sizeof(qcril_gstk_qmi_ind_params_type);
  memset(out_ptr, 0x00, sizeof(qcril_gstk_qmi_ind_params_type));
  out_ptr->handle  = user_handle_ptr;
  out_ptr->msg_id  = msg_id;
  out_ptr->msg_ptr = decoded_payload_ptr;

  return out_ptr;
} /* qcril_gstk_qmi_copy_indication */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_copy_response

===========================================================================*/
/*!
    @brief
    Makes a copy of the response received from QMI.

    @return
    Pointer to copy of the response
*/
/*=========================================================================*/
static qcril_gstk_qmi_rsp_params_type * qcril_gstk_qmi_copy_response
(
  unsigned long                        msg_id,
  void                               * resp_data_ptr,
  void                               * resp_cb_data_ptr,
  qmi_client_error_type                transp_err,
  uint32_t                           * out_len_ptr
)
{
  qcril_gstk_qmi_rsp_params_type   * out_ptr  = NULL;

  /* Sanity check */
  if ((resp_data_ptr    == NULL) ||
      (resp_cb_data_ptr == NULL) ||
      (out_len_ptr      == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot copy");
    QCRIL_ASSERT(0);
    return NULL;
  }

  /* Update size & allocate buffer */
  *out_len_ptr = sizeof(qcril_gstk_qmi_rsp_params_type);
  out_ptr = (qcril_gstk_qmi_rsp_params_type*)qcril_malloc(*out_len_ptr);
  if (out_ptr == NULL)
  {
    return NULL;
  }

  memset(out_ptr, 0, *out_len_ptr);

  /* Copy the response parameters */
  out_ptr->msg_id        = msg_id;
  out_ptr->msg_ptr       = resp_data_ptr;
  out_ptr->transp_err    = transp_err;
  out_ptr->orig_req_data = (qcril_gstk_original_request_type*)resp_cb_data_ptr;

  return out_ptr;
} /* qcril_gstk_qmi_copy_response */


/*=========================================================================

  FUNCTION:  qcril_gstk_send_set_event_report

===========================================================================*/
/*!
    @brief
    Composes and sends the event report to the passed modem.

    @return
    QMI error code
*/
/*=========================================================================*/
static int qcril_gstk_send_set_event_report
(
  qmi_client_type              modem_type_ptr,
  boolean                      is_scws
)
{
  cat_set_event_report_req_msg_v02   event_report_req;
  cat_set_event_report_resp_msg_v02  event_resport_resp;

  if (modem_type_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s","NULL modem_type_ptr !");
    return QMI_INTERNAL_ERR;
  }

  memset(&event_report_req, 0x00, sizeof(cat_set_event_report_req_msg_v02));

  /* First update the slot mask */
  event_report_req.slot_mask_valid = TRUE;
  event_report_req.slot_mask = qcril_gstk_find_slot_mask();

  /* Updated the necessary masks */
  if (is_scws)
  {
    /* For SCWS updated only SCWS decoded event report mask */
    event_report_req.pc_dec_evt_report_req_mask_valid = TRUE;
    event_report_req.pc_dec_evt_report_req_mask = QMI_CAT_SET_EVENT_REPORT_SCWS;
  }
  else
  {
    event_report_req.pc_evt_report_req_mask_valid = TRUE;
    event_report_req.pc_evt_report_req_mask =
      QMI_CAT_SET_EVENT_REPORT_DISPLAY_TEXT                    |
      QMI_CAT_SET_EVENT_REPORT_GET_INKEY                       |
      QMI_CAT_SET_EVENT_REPORT_GET_INPUT                       |
      QMI_CAT_SET_EVENT_REPORT_SETUP_MENU                      |
      QMI_CAT_SET_EVENT_REPORT_SELECT_ITEM                     |
      QMI_CAT_SET_EVENT_REPORT_SEND_SMS                        |
      QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_USER_ACTIVITY       |
      QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_IDLE_SCREEN_NOTIFY  |
      QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_LANGUAGE_SEL_NOTIFY |
      QMI_CAT_SET_EVENT_REPORT_SETUP_IDLE_MODE_TEXT            |
      QMI_CAT_SET_EVENT_REPORT_LANGUAGE_NOTIFICATION           |
      QMI_CAT_SET_EVENT_REPORT_REFRESH_ALPHA                   |
      QMI_CAT_SET_EVENT_REPORT_END_PROACTIVE_SESSION           |
      QMI_CAT_SET_EVENT_REPORT_PLAY_TONE                       |
      QMI_CAT_SET_EVENT_REPORT_SETUP_CALL                      |
      QMI_CAT_SET_EVENT_REPORT_SEND_DTMF                       |
      QMI_CAT_SET_EVENT_REPORT_LAUNCH_BROWSER                  |
      QMI_CAT_SET_EVENT_REPORT_SEND_SS                         |
      QMI_CAT_SET_EVENT_REPORT_SEND_USSD                       |
      QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_LANG         |
      QMI_CAT_SET_EVENT_REPORT_PROVIDE_LOCAL_INFO_TIME         |
      QMI_CAT_SET_EVENT_REPORT_BIP                             |
      QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_BROWSER_TERM        |
      QMI_CAT_SET_EVENT_REPORT_ACTIVATE                        |
      QMI_CAT_SET_EVENT_REPORT_SETUP_EVENT_HCI_CONN;
  }

  return qmi_client_send_msg_sync_with_shm( modem_type_ptr,
                                   QMI_CAT_SET_EVENT_REPORT_REQ_V02,
                                   (void *) &event_report_req,
                                   sizeof(cat_set_event_report_req_msg_v02),
                                   (void *) &event_resport_resp,
                                   sizeof(cat_set_event_report_resp_msg_v02),
                                   QMI_CAT_DEFAULT_TIMEOUT);
} /* qcril_gstk_send_set_event_report */


/*=========================================================================

  FUNCTION:  qcril_gstk_perform_config_check

===========================================================================*/
/*!
    @brief
    This function first checks if QMI CAT in the modem has correct
    configuration for Android HLOS. If not, it sends the QMI command to put
    QMI CAT in the right configuration. It is required to reboot the device
    in order for this configuration to take effect.

    @return
    QMI error code
*/
/*=========================================================================*/
static int qcril_gstk_perform_config_check
(
  qmi_client_type              modem_type_ptr
)
{
  qmi_client_error_type                qmi_err_code      = QMI_INTERNAL_ERR;
  cat_config_mode_enum_v02             config_mode       = CAT_CONFIG_MODE_DISABLED_V02;
  cat_get_configuration_resp_msg_v02 * get_conf_resp_ptr = NULL;
  cat_set_configuration_req_msg_v02  * set_conf_req_ptr  = NULL;
  cat_set_configuration_resp_msg_v02   set_conf_resp;

  if (modem_type_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s","NULL modem_type_ptr !");
    return QMI_INTERNAL_ERR;
  }

  get_conf_resp_ptr = (cat_get_configuration_resp_msg_v02*)qcril_malloc(
                        sizeof(cat_get_configuration_resp_msg_v02));
  if(get_conf_resp_ptr == NULL)
  {
    return QMI_SERVICE_ERR;
  }

  memset(get_conf_resp_ptr, 0x00, sizeof(cat_get_configuration_resp_msg_v02));

  qmi_err_code = qmi_client_send_msg_sync( modem_type_ptr,
                                           QMI_CAT_GET_CONFIGURATION_REQ_V02,
                                           NULL,
                                           0,
                                           (void *) get_conf_resp_ptr,
                                           sizeof(cat_get_configuration_resp_msg_v02),
                                           QMI_CAT_DEFAULT_TIMEOUT);

  if ((qmi_err_code                             != QMI_NO_ERR)             ||
      (get_conf_resp_ptr->resp.result           != QMI_RESULT_SUCCESS_V01) ||
      (get_conf_resp_ptr->resp.error            != QMI_ERR_NONE_V01)       ||
      (get_conf_resp_ptr->cat_config_mode_valid == 0))
  {
    /* Nothing we can do about an error, just return */
    QCRIL_LOG_ERROR("Error for get config, qmi_err_code: 0x%x, result: 0x%x, error: 0x%x",
                    qmi_err_code,
                    get_conf_resp_ptr->resp.result,
                    get_conf_resp_ptr->resp.error);
    qcril_free(get_conf_resp_ptr);
    return qmi_err_code;
  }

  config_mode = get_conf_resp_ptr->cat_config_mode;
  qcril_free(get_conf_resp_ptr);

  QCRIL_LOG_INFO("cat_config_mode: 0x%x", config_mode);

  if ((config_mode == CAT_CONFIG_MODE_ANDROID_V02) ||
      (config_mode == CAT_CONFIG_MODE_CUSTOM_RAW_V02))
  {
    /* Nothing else to do */
    return qmi_err_code;
  }

  /* Set it to Android mode if we found some other value */
  set_conf_req_ptr = (cat_set_configuration_req_msg_v02*)qcril_malloc(
                       sizeof(cat_set_configuration_req_msg_v02));
  if(set_conf_req_ptr == NULL)
  {
    return QMI_SERVICE_ERR;
  }

  memset(set_conf_req_ptr, 0x00, sizeof(cat_set_configuration_req_msg_v02));
  memset(&set_conf_resp, 0x00, sizeof(cat_set_configuration_resp_msg_v02));

  set_conf_req_ptr->cat_config_mode = CAT_CONFIG_MODE_ANDROID_V02;

  qmi_err_code = qmi_client_send_msg_sync( modem_type_ptr,
                                           QMI_CAT_SET_CONFIGURATION_REQ_V02,
                                           (void *) set_conf_req_ptr,
                                           sizeof(cat_set_configuration_req_msg_v02),
                                           (void *) &set_conf_resp,
                                           sizeof(cat_set_configuration_resp_msg_v02),
                                           QMI_CAT_DEFAULT_TIMEOUT);
  if ((qmi_err_code              != QMI_NO_ERR)             ||
      (set_conf_resp.resp.result != QMI_RESULT_SUCCESS_V01) ||
      (set_conf_resp.resp.error  != QMI_ERR_NONE_V01))
  {
    /* Nothing we can do about an error, just return */
    QCRIL_LOG_ERROR("Error for set config, qmi_err_code: 0x%x, result: 0x%x, error: 0x%x",
                    qmi_err_code,
                    set_conf_resp.resp.result,
                    set_conf_resp.resp.error);
  }

  qcril_free(set_conf_req_ptr);

  return qmi_err_code;
} /* qcril_gstk_perform_config_check */


/*===========================================================================
                          QCRIL INTERFACE FUNCTIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_init
===========================================================================*/
/*!
  @brief
    Initialize the RIL GSTK subsystem

    - Obtain Handle from QMI

  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_qmi_init
(
  void
)
{
  qcril_modem_id_e_type            modem_id           = QCRIL_MAX_MODEM_ID - 1;
  int                              qmi_err_code       = QMI_NO_ERR;
  uint8                            slot_index         = 0;
  qmi_client_qmux_instance_type    qmi_pri_modem_port = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
  qmi_client_qmux_instance_type    qmi_sec_modem_port = QMI_CLIENT_QMUX_MAX_INSTANCE_IDS;
  char                             prop_value[PROPERTY_VALUE_MAX];
  char                             prop_value_disable[PROPERTY_VALUE_MAX];
  uint8                            num_slots          = 0;
  qmi_idl_service_object_type      client_service;
  int                              time_out           = QMI_CAT_INIT_TIMEOUT;
  qmi_client_os_params             os_params;
  uint8                            num_retries        = 0;

  QCRIL_LOG_DEBUG( "%s", "qcril_gstk_qmi_init");

  memset(&qcril_gstk_qmi_info, 0x00, sizeof(qcril_gstk_qmi_info));
  memset(&os_params, 0x00, sizeof(qmi_client_os_params));

  num_slots = qcril_gstk_get_num_slots();

  for (slot_index = 0; slot_index < num_slots; slot_index++)
  {
    qcril_gstk_qmi_info.gstk_command_info[slot_index].call_timeout = (2 * 60 * 1000);
  }

   /* Initialize QMI interface */
  QCRIL_LOG_QMI( modem_id, "qmi_cat_service", "init" );

  /* If QCRIL_GSTK needs to be disabled, just return at this point */
  memset(prop_value_disable, 0x00, sizeof(prop_value_disable));
  property_get(QCRIL_GSTK_PROPERTY_DISABLED, prop_value_disable, "");
  if (strcmp(prop_value_disable, QCRIL_GSTK_PROP_DISABLED_VALUE)  == 0)
  {
    QCRIL_LOG_INFO("%s", "QCRIL GSTK is being disabled");
    return;
  }

  /* Find out the modem type */
  memset(prop_value, 0x00, sizeof(prop_value));
  property_get(QCRIL_GSTK_PROPERTY_BASEBAND, prop_value, "");

  /* Map to a respective primary QMI port */
  qmi_pri_modem_port = qcril_gstk_find_primary_modem_port(prop_value);

  /* Initialize QMI for the primary & secondary modem ports */
  client_service = cat_get_service_object_v02();

  /* Call common client layer initialization function */
  do
  {
    if (num_retries > 0)
    {
      sleep(QCRIL_GSTK_QMI_INIT_RETRY_INTERVAL);
    }

    QCRIL_LOG_INFO("Trying primary qmi_client_init_instance() try # %d", num_retries);
    qmi_err_code = qmi_client_init_instance(client_service,
                                            qmi_pri_modem_port,
                                            qcril_gstk_qmi_indication_cb,
                                            NULL,
                                            &os_params,
                                            time_out,
                                            &qcril_gstk_qmi_info.qmi_cat_svc_client_primary);
    num_retries++;
  } while ((qcril_gstk_qmi_info.qmi_cat_svc_client_primary == NULL) &&
           (qmi_err_code != QMI_NO_ERR) &&
           (num_retries < QCRIL_GSTK_QMI_INIT_MAX_RETRIES));

  if(qcril_gstk_qmi_info.qmi_cat_svc_client_primary == NULL)
  {
    QCRIL_LOG_ERROR("%s","QMI CAT service primary port open failure !");
    QCRIL_ASSERT(0);
    return;
  }

  /* Also get QMI CAT config & set it appropriately if needed */
  (void)qcril_gstk_perform_config_check(
          qcril_gstk_qmi_info.qmi_cat_svc_client_primary);

  /* If modem is a fusion type, also open a secondary */
  qmi_sec_modem_port = qcril_gstk_find_secondary_modem_port(prop_value);
  if (qmi_sec_modem_port != QMI_CLIENT_QMUX_MAX_INSTANCE_IDS)
  {
    num_retries = 0;
    do
    {
      if (num_retries > 0)
      {
        sleep(QCRIL_GSTK_QMI_INIT_RETRY_INTERVAL);
      }

      QCRIL_LOG_INFO("Trying secondary qmi_client_init_instance() try # %d", num_retries);
      qmi_err_code = qmi_client_init_instance(client_service,
                                              qmi_sec_modem_port,
                                              qcril_gstk_qmi_indication_cb,
                                              NULL,
                                              &os_params,
                                              time_out,
                                              &qcril_gstk_qmi_info.qmi_cat_svc_client_secondary);
      num_retries++;
    } while ((qcril_gstk_qmi_info.qmi_cat_svc_client_secondary == NULL) &&
             (qmi_err_code != QMI_NO_ERR) &&
             (num_retries < QCRIL_GSTK_QMI_INIT_MAX_RETRIES));

    if(qcril_gstk_qmi_info.qmi_cat_svc_client_secondary == NULL)
    {
      QCRIL_LOG_ERROR("%s","QMI CAT service secondary port open failure !");
      QCRIL_ASSERT(0);
      return;
    }

    /* Also get QMI CAT config & set it appropriately if needed */
    (void)qcril_gstk_perform_config_check(
            qcril_gstk_qmi_info.qmi_cat_svc_client_secondary);
  }

  /* Set event report for individual modem ports */
  qmi_err_code = qcril_gstk_send_set_event_report(
                   qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                   FALSE);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_ERROR("Error for set event report on primary modem port, qmi_err_code: 0x%x\n",
                    qmi_err_code);
  }

  /* Set event report with secondary modem only if needed */
  if (qcril_gstk_qmi_info.qmi_cat_svc_client_secondary != NULL)
  {
    qmi_err_code = qcril_gstk_send_set_event_report(
                     qcril_gstk_qmi_info.qmi_cat_svc_client_secondary,
                     FALSE);
    if (qmi_err_code != 0)
    {
      QCRIL_LOG_ERROR("Error for set event report on secondary modem, qmi_err_code: 0x%x\n",
                      qmi_err_code);
    }
  }

  /* Intialize SCWS Agent */
  qcril_scws_initalize(qcril_gstk_qmi_scws_data_available_callback,
                       qcril_gstk_qmi_scws_channel_status_callback);

  qmi_err_code = qcril_gstk_send_set_event_report(
                   qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                   TRUE);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_ERROR("Error for set event report on primary modem port, qmi_err_code: 0x%x\n",
                    qmi_err_code);
  }

  QCRIL_LOG_INFO("%s", "QCRIL_GSTK_QMI_INIT success\n");
} /* qcril_gstk_qmi_init */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_srvc_release_client
===========================================================================*/
/*!
  @brief
    Releases a previously opened QMI CAT client service handle via the
    qcril_gstk_qmi_init() API.

  @return
    Nothing

  @msc
  @endmsc
*/
/*=========================================================================*/
void qcril_gstk_qmi_srvc_release_client
(
  void
)
{
  qmi_client_error_type rc = -1;

  QCRIL_LOG_DEBUG( "%s", "qcril_gstk_qmi_srvc_release_client");

  if(qcril_gstk_qmi_info.qmi_cat_svc_client_primary != NULL)
  {
    rc = qmi_client_release(qcril_gstk_qmi_info.qmi_cat_svc_client_primary);
    qcril_gstk_qmi_info.qmi_cat_svc_client_primary = NULL;
    if (rc < 0)
    {
      QCRIL_LOG_ERROR("QMI CAT service primary port release failure, rc: 0x%x",rc);
    }
  }

  if(qcril_gstk_qmi_info.qmi_cat_svc_client_secondary != NULL)
  {
    rc = qmi_client_release(qcril_gstk_qmi_info.qmi_cat_svc_client_secondary);
    qcril_gstk_qmi_info.qmi_cat_svc_client_secondary = NULL;
    if (rc < 0)
    {
      QCRIL_LOG_ERROR("QMI CAT service secondary port release failure, rc: 0x%x",rc);
    }
  }
} /* qcril_gstk_qmi_srvc_release_client */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_indication_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI indications.

    @return
    None
*/
/*=========================================================================*/
static void qcril_gstk_qmi_indication_cb
(
  qmi_client_type                 user_handle,
  unsigned int                    msg_id,
  unsigned char                 * ind_buf_ptr,
  unsigned int                    ind_buf_len,
  void                          * ind_cb_data_ptr
)
{
  uint32_t                              ind_params_len = 0;
  qcril_gstk_qmi_ind_params_type      * ind_params_ptr = NULL;
  qmi_client_error_type                 qmi_err        = QMI_NO_ERR;
  IxErrnoType                           result         = E_FAILURE;

  QCRIL_LOG_INFO("qcril_gstk_qmi_indication_cb, msg_id: 0x%x", msg_id);

  if ((ind_buf_ptr == NULL) || (ind_buf_len == 0))
  {
    QCRIL_LOG_ERROR("%s","NULL ind_buf_ptr !");
    return;
  }

  /* Process valid IND messages */
  switch (msg_id)
  {
    case QMI_CAT_EVENT_REPORT_IND_V02:
    case QMI_CAT_SCWS_OPEN_CHANNEL_IND_V02:
    case QMI_CAT_SCWS_CLOSE_CHANNEL_IND_V02:
    case QMI_CAT_SCWS_SEND_DATA_IND_V02:
      ind_params_ptr = qcril_gstk_qmi_copy_indication(user_handle,
                                                      msg_id,
                                                      ind_buf_ptr,
                                                      ind_buf_len,
                                                      &ind_params_len);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported QMI CAT indication: 0x%x", msg_id);
      break;
  }

  if ((ind_params_ptr == NULL) || (ind_params_len == 0))
  {
    QCRIL_LOG_ERROR("Error decoding the indication msg_id: 0x%x", msg_id);
    return;
  }

  /* Post internal event in case of successful decoded IND message.
     Note: msg_ptr will be freed in the indication handler -
     qcril_gstk_qmi_process_qmi_indication */
  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  result = qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                              QCRIL_DEFAULT_MODEM_ID,
                              QCRIL_DATA_NOT_ON_STACK,
                              QCRIL_EVT_GSTK_QMI_CAT_INDICATION,
                              (void *)ind_params_ptr,
                              ind_params_len,
                              NULL);
  if (result != E_SUCCESS)
  {
    QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);

    /* Free allocated memory in case event queueing fails */
    QCRIL_GSTK_QMI_FREE_PTR(ind_params_ptr->msg_ptr);
    QCRIL_GSTK_QMI_FREE_PTR(ind_params_ptr);
  }
} /* qcril_gstk_qmi_indication_cb */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_process_qmi_indication

===========================================================================*/
void qcril_gstk_qmi_process_qmi_indication
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_gstk_qmi_ind_params_type     * ind_data_ptr        = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  ind_data_ptr = (qcril_gstk_qmi_ind_params_type*) params_ptr->data;
  if (ind_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL ind_data_ptr");
    QCRIL_ASSERT(0);
    return;
  }

  if (ind_data_ptr->msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL msg_ptr");
    QCRIL_GSTK_QMI_FREE_PTR(ind_data_ptr);
    QCRIL_ASSERT(0);
    return;
  }

  if (params_ptr->modem_id >= QCRIL_MAX_MODEM_ID)
  {
    QCRIL_LOG_ERROR("Invalid value, params_ptr->modem_id: 0x%x",
                     params_ptr->modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  ind_data_ptr->instance_id = QCRIL_MAX_INSTANCE_ID;
  ind_data_ptr->modem_id    = params_ptr->modem_id;

  /* Process respective indications */
  switch(ind_data_ptr->msg_id)
  {
     case QMI_CAT_EVENT_REPORT_IND_V02:
       (void)qcril_gstk_qmi_process_event_report_ind(ind_data_ptr);
       break;
     case QMI_CAT_SCWS_OPEN_CHANNEL_IND_V02:
       qcril_gstk_qmi_scws_open_channel(
          (cat_scws_open_channel_ind_msg_v02*)ind_data_ptr->msg_ptr);
       break;
     case QMI_CAT_SCWS_CLOSE_CHANNEL_IND_V02:
       qcril_gstk_qmi_scws_close_channel(
          (cat_scws_close_channel_ind_msg_v02*)ind_data_ptr->msg_ptr);
       break;
     case QMI_CAT_SCWS_SEND_DATA_IND_V02:
       qcril_gstk_qmi_scws_send_data(
          (cat_scws_send_data_ind_msg_v02*)ind_data_ptr->msg_ptr);
       break;
     default:
       QCRIL_LOG_ERROR("Indication type not recognized (0x%X)\n",
                       ind_data_ptr->msg_id);
       break;
  }

  /* Free buffers that need to be freed:
      ind_param_ptr & its msg_ptr is allocated in qcril_gstk_qmi_indication_cb
      ril_unsol_resp.resp_pkt is allocated in qcril_gstk_qmi_process_event_report_ind */
  QCRIL_LOG_INFO("%s", "cleanup in qcril_uim_indication_cb");
  QCRIL_GSTK_QMI_FREE_PTR(ind_data_ptr->msg_ptr);
  QCRIL_GSTK_QMI_FREE_PTR(ind_data_ptr);
} /* qcril_gstk_qmi_process_qmi_indication */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_command_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI commands.

    @return
    None
*/
/*=========================================================================*/
static void qcril_gstk_qmi_command_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                         * resp_data_ptr,
  unsigned int                   resp_data_len,
  void                         * resp_cb_data_ptr,
  qmi_client_error_type          transp_err
)
{
  qcril_gstk_qmi_rsp_params_type    * rsp_params_ptr = NULL;
  uint32_t                            rsp_params_len = 0;
  IxErrnoType                         result         = E_FAILURE;

  QCRIL_LOG_INFO("qcril_gstk_qmi_command_cb, msg_id: 0x%x", msg_id);

  /* Sanity check */
  if (resp_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input: NULL resp_data_ptr");
    return;
  }

  if ((user_handle == NULL) || (resp_data_len == 0))
  {
    QCRIL_LOG_ERROR("Invalid input: user_handle: 0x%x, resp_data_len: 0x%x",
                    user_handle, resp_data_len);
    QCRIL_GSTK_QMI_FREE_PTR(resp_data_ptr);
    return;
  }

  /* Process valid RESP messages */
  switch (msg_id)
  {
    case QMI_CAT_SCWS_DATA_AVAILABLEA_RESP_V02:
      rsp_params_ptr = qcril_gstk_qmi_copy_response(msg_id,
                                                    resp_data_ptr,
                                                    resp_cb_data_ptr,
                                                    transp_err,
                                                    &rsp_params_len);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported QMI CAT response: 0x%x", msg_id);
      break;
  }

  if ((rsp_params_ptr == NULL) || (rsp_params_len == 0))
  {
    QCRIL_LOG_ERROR("Error copying the response msg_id: 0x%X", msg_id);
    QCRIL_GSTK_QMI_FREE_PTR(resp_data_ptr);
    return;
  }

  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  result = qcril_event_queue( rsp_params_ptr->orig_req_data->instance_id,
                              rsp_params_ptr->orig_req_data->modem_id,
                              QCRIL_DATA_NOT_ON_STACK,
                              QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK,
                              (void *)rsp_params_ptr,
                              rsp_params_len,
                              NULL);
  if (result != E_SUCCESS)
  {
    QCRIL_LOG_ERROR( " qcril_event_queue failed, result: 0x%x\n", result);
    /* Free allocated memory in case event queueing fails */
     if (rsp_params_ptr->orig_req_data != NULL)
    {
      QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr->orig_req_data);
    }
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr);
    QCRIL_GSTK_QMI_FREE_PTR(resp_data_ptr);
  }
} /* qcril_gstk_qmi_command_cb */


/*=========================================================================

  FUNCTION:  qcril_gstk_qmi_process_qmi_response

===========================================================================*/
void qcril_gstk_qmi_process_qmi_response
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_gstk_qmi_rsp_params_type    * rsp_params_ptr = NULL;

  /* Sanity check */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  rsp_params_ptr = (qcril_gstk_qmi_rsp_params_type*)params_ptr->data;
  if (rsp_params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL rsp_params_ptr, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  if (rsp_params_ptr->msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL msg_ptr, cannot process response");
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr);
    QCRIL_ASSERT(0);
    return;
  }

  if (rsp_params_ptr->orig_req_data == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL orig_req_data, cannot process response");
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr);
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr->msg_ptr);
    QCRIL_ASSERT(0);
    return;
  }

  if ((rsp_params_ptr->orig_req_data->instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (rsp_params_ptr->orig_req_data->modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, modem_id: 0x%x",
                    rsp_params_ptr->orig_req_data->instance_id,
                    rsp_params_ptr->orig_req_data->modem_id);
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr->orig_req_data);
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr->msg_ptr);
    QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr);
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_DEBUG( "%s: Response for msg_id: 0x%X, transp_err: 0x%X",
                    __FUNCTION__,
                   rsp_params_ptr->msg_id,
                   rsp_params_ptr->transp_err );

  /* Currently, this callback processes only scws_data_available command */
  if (rsp_params_ptr->msg_id == QMI_CAT_SCWS_DATA_AVAILABLEA_RESP_V02)
  {
    cat_scws_data_available_resp_msg_v02 * data_available_resp_ptr =
      (cat_scws_data_available_resp_msg_v02 *)rsp_params_ptr->msg_ptr;
    if ((rsp_params_ptr->transp_err           != QMI_NO_ERR) ||
        (data_available_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01) ||
        (data_available_resp_ptr->resp.error  != QMI_ERR_NONE_V01))
    {
      /* Notify the Agent only about the error */
      QCRIL_LOG_ERROR("%s", "Error for a previous data_available command, notifying agent");

      /* Notify the Agent about the error */
      qcril_scws_data_available_error(rsp_params_ptr->orig_req_data->bip_id,
                                      rsp_params_ptr->orig_req_data->slot_id);
    }
  }

  /* Free memory allocated previously */
  QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr->orig_req_data);
  QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr->msg_ptr);
  QCRIL_GSTK_QMI_FREE_PTR(rsp_params_ptr);
} /* qcril_gstk_qmi_process_qmi_response */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_get_profile
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
void qcril_gstk_qmi_request_stk_get_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_get_profile\n");

  /*-----------------------------------------------------------------------*/

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR("Invalid instance_id: 0x%x", instance_id);
    QCRIL_ASSERT(0);
    return;
  }

  /*-----------------------------------------------------------------------*/

  qcril_default_request_resp_params( instance_id, params_ptr->t,
                                     params_ptr->event_id, RIL_E_REQUEST_NOT_SUPPORTED,
                                     &resp );
  qcril_send_request_response( &resp );

  /*-----------------------------------------------------------------------*/

} /* qcril_gstk_qmi_request_stk_get_profile */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_set_profile
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
void qcril_gstk_qmi_request_stk_set_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_set_profile\n");

  /*-----------------------------------------------------------------------*/

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  if (instance_id >= QCRIL_MAX_INSTANCE_ID)
  {
    QCRIL_LOG_ERROR("Invalid instance_id: 0x%x", instance_id);
    QCRIL_ASSERT(0);
    return;
  }

  /*-----------------------------------------------------------------------*/

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id,
                                     RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( &resp );

} /* qcril_gstk_qmi_request_stk_set_profile */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_send_envelope_command
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
  @endmsc

  @see
*/
/*=========================================================================*/
void qcril_gstk_qmi_request_stk_send_envelope_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type            instance_id     = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type               modem_id        = QCRIL_DEFAULT_MODEM_ID;
  int                                 len             = 0;
  int                                 qmi_err_code    = 0;
  uint8                               slot_index      = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  cat_send_envelope_cmd_req_msg_v02 * env_cmd_req_ptr = NULL;
  cat_send_envelope_cmd_resp_msg_v02  env_cmd_resp;
  qcril_request_resp_params_type      resp;

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  if ((params_ptr->data == NULL) || (params_ptr->datalen == 0))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL data pointer or params_ptr->datalen: 0x%x");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);;
  modem_id = params_ptr->modem_id;

  if ((instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (slot_index  >= qcril_gstk_get_num_slots()) ||
      (modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, slot_index: 0x%x, modem_id: 0x%x",
                     instance_id, slot_index, modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_send_envelope_command\n");

  memset(&env_cmd_resp, 0x00, sizeof(cat_send_envelope_cmd_resp_msg_v02));
  memset(&resp,         0x00, sizeof(qcril_request_resp_params_type));

  /* Allocate request pointer on the heap */
  env_cmd_req_ptr = (cat_send_envelope_cmd_req_msg_v02 *)
                                qcril_malloc(sizeof(cat_send_envelope_cmd_req_msg_v02));
  if (env_cmd_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for env_cmd_req_ptr!");
    qcril_default_request_resp_params(instance_id, params_ptr->t,
                                      params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response(&resp);
    return;
  }

  memset(env_cmd_req_ptr, 0x00, sizeof(cat_send_envelope_cmd_req_msg_v02));

  /* Allocate memory for the raw envelope */
  len = strlen((char *) params_ptr->data);
  QCRIL_LOG_INFO("Envelope data length is %d\n",len);

  env_cmd_req_ptr->slot_valid = TRUE;
  env_cmd_req_ptr->slot.slot  = qcril_gstk_qmi_convert_slot_index_to_slot_type(slot_index);

  env_cmd_req_ptr->envelope_cmd.env_cmd_type      = CAT_ENVELOPE_CMD_TYPE_UNKNOWN_V02;
  env_cmd_req_ptr->envelope_cmd.envelope_data_len = QCRIL_GSTK_QMI_ENVCMD_DATA_SIZE(len);
  if (env_cmd_req_ptr->envelope_cmd.envelope_data_len > QMI_CAT_ENVELOPE_DATA_MAX_LENGTH_V02)
  {
    QCRIL_LOG_ERROR("Length of Envelope too long: 0x%x\n",
                     env_cmd_req_ptr->envelope_cmd.envelope_data_len);

    qcril_default_request_resp_params(instance_id, params_ptr->t, params_ptr->event_id,
                                      RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response(&resp);
  }
  else
  {
    /* Copy over the command and send to the card */
    QCRIL_GSTK_QMI_ENVCMD_COPY(env_cmd_req_ptr->envelope_cmd.envelope_data,
                               params_ptr->data,
                               len);

    /* Primary will always send envelope commands to card */
    QCRIL_LOG_QMI( modem_id, "qmi_gstk_service", "send_envelope" );
    qmi_err_code = qmi_client_send_msg_sync_with_shm(qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                                            QMI_CAT_SEND_ENVELOPE_CMD_REQ_V02,
                                            (void *) env_cmd_req_ptr,
                                            sizeof(cat_send_envelope_cmd_req_msg_v02),
                                            (void *) &env_cmd_resp,
                                            sizeof(cat_send_envelope_cmd_resp_msg_v02),
                                            QMI_CAT_DEFAULT_TIMEOUT);
    if (qmi_err_code != 0)
    {
      QCRIL_LOG_ERROR("Error for SEND_ENVELOPE_CMD_REQ, client_err: 0x%x, error_code: 0x%x\n",
                      qmi_err_code, env_cmd_resp.resp.error);

      qcril_default_request_resp_params(instance_id, params_ptr->t, params_ptr->event_id,
                                        RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response(&resp);
    }
    else
    {
      uint16   env_response_len = 0;
      char   * env_response_ptr = NULL;

      /* Since this is synchronous call to QMI we respond here with the result.
         Fill the default data first and then the response data */
      qcril_default_request_resp_params(instance_id, params_ptr->t, params_ptr->event_id,
                                        RIL_E_SUCCESS, &resp);

      if (env_cmd_resp.env_resp_data_valid &&
          (env_cmd_resp.env_resp_data.env_resp_data_len > 0) &&
          (env_cmd_resp.env_resp_data.env_resp_data_len <= QMI_CAT_RAW_ENV_RSP_DATA_MAX_LENGTH_V02))
      {
        env_response_len = QCRIL_GSTK_QMI_ENVRSP_DATA_SIZE(
                             env_cmd_resp.env_resp_data.env_resp_data_len) + 1;
        env_response_ptr = (char *)qcril_malloc(env_response_len);
        if (env_response_ptr)
        {
          memset(env_response_ptr, 0, env_response_len);
          QCRIL_GSTK_QMI_ENVRSP_COPY((uint8 *)env_response_ptr,
                                     env_cmd_resp.env_resp_data.env_resp_data,
                                     env_cmd_resp.env_resp_data.env_resp_data_len);
          QCRIL_LOG_DEBUG("send_envelope_command, response data=%s\n", env_response_ptr);
          resp.resp_pkt = env_response_ptr;
          resp.resp_len = env_response_len;
        }
      }

      qcril_send_request_response(&resp);

      /* Free the response pointer we allocated earlier */
      if (env_response_ptr)
      {
        QCRIL_GSTK_QMI_FREE_PTR(env_response_ptr);
      }
    }
  }

  /* Free allocated request pointer */
  QCRIL_GSTK_QMI_FREE_PTR(env_cmd_req_ptr);
} /* qcril_gstk_qmi_request_stk_send_envelope_command */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_send_terminal_response
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
void qcril_gstk_qmi_request_stk_send_terminal_response
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type       instance_id     = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type          modem_id        = QCRIL_DEFAULT_MODEM_ID;
  RIL_Errno                      ril_result      = RIL_E_SUCCESS;
  int                            len             = 0;
  int                            qmi_err_code    = 0;
  uint8                          slot_index      = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  cat_send_tr_req_msg_v02      * send_tr_req_ptr = NULL;
  cat_send_tr_resp_msg_v02       send_tr_resp;
  qcril_request_resp_params_type resp;

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_send_terminal_response\n");

  /*-----------------------------------------------------------------------*/

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  if ((params_ptr->data    == NULL) ||
      (params_ptr->datalen == 0))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL data pointer or zero datalen");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
  modem_id = params_ptr->modem_id;

  if ((instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (slot_index  >= qcril_gstk_get_num_slots()) ||
      (modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, slot_index: 0x%x, modem_id: 0x%x",
                     instance_id, slot_index, modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  memset(&resp,0x00, sizeof(qcril_request_resp_params_type));
  memset(&send_tr_resp,0x00, sizeof(cat_send_tr_resp_msg_v02));

  /* Allocate request pointer on the heap */
  send_tr_req_ptr = (cat_send_tr_req_msg_v02 *)
                      qcril_malloc(sizeof(cat_send_tr_req_msg_v02));
  if (send_tr_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for send_tr_req_ptr!");
    qcril_default_request_resp_params(instance_id, params_ptr->t, params_ptr->event_id,
                                      RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response(&resp);
    return;
  }

  memset(send_tr_req_ptr, 0x00, sizeof(cat_send_tr_req_msg_v02));

  QCRIL_LOG_INFO("Data length supplied is %d\n", params_ptr->datalen);

  len = strlen((char *) params_ptr->data);
  QCRIL_LOG_INFO("String length of data buffer is %d\n", len);

  /* Allocate memory for the raw response */
  /* rsp.data_len = QCRIL_GSTK_RESPONSE_DATA_SIZE(params_ptr->datalen); */

  send_tr_req_ptr->slot_valid = TRUE;
  send_tr_req_ptr->slot.slot  = qcril_gstk_qmi_convert_slot_index_to_slot_type(slot_index);
  send_tr_req_ptr->terminal_response.terminal_response_len = QCRIL_GSTK_QMI_RESPONSE_DATA_SIZE(len);
  if (send_tr_req_ptr->terminal_response.terminal_response_len > QMI_CAT_TERMINAL_RESPONSE_MAX_LENGTH_V02)
  {
    QCRIL_LOG_ERROR("Length of TR too long: 0x%x\n",
                     send_tr_req_ptr->terminal_response.terminal_response_len);
    ril_result = RIL_E_GENERIC_FAILURE;
  }
  else
  {
    /* Copy over the response and send to the card */
    QCRIL_GSTK_QMI_RESPONSE_COPY(send_tr_req_ptr->terminal_response.terminal_response,
                                 params_ptr->data,
                                 len);
    if(qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr)
    {
      send_tr_req_ptr->terminal_response.uim_ref_id =
        qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr->uim_ref_id;

      QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "send terminal response" );
      qmi_err_code = qmi_client_send_msg_sync_with_shm(
                       qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr->handle,
                       QMI_CAT_SEND_TR_REQ_V02,
                       (void *) send_tr_req_ptr,
                       sizeof(cat_send_tr_req_msg_v02),
                       (void *) &send_tr_resp,
                       sizeof(cat_send_tr_resp_msg_v02),
                       QMI_CAT_DEFAULT_TIMEOUT);
      if (qmi_err_code != 0)
      {
        QCRIL_LOG_ERROR("Error for SEND_TR_REQ, client_err: 0x%x, error_code: 0x%x\n",
                        qmi_err_code, send_tr_resp.resp.error);
        ril_result = RIL_E_GENERIC_FAILURE;
      }

      /* Clear the global proactive command cache, irrespective of the result */
      QCRIL_LOG_INFO("%s", "Dismiss pending proactive cmd!\n");
      QCRIL_GSTK_QMI_FREE_PTR(qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr);
    }
    else
    {
      QCRIL_LOG_ERROR("%s\n","qcril_gstk_qmi sending TR Error no matching cmd cached");
      ril_result = RIL_E_GENERIC_FAILURE;
    }
  }

  /* Notify the caller of the result */
  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_result, &resp );
  qcril_send_request_response( &resp );

  /* Free allocated request pointer */
  QCRIL_GSTK_QMI_FREE_PTR(send_tr_req_ptr);
} /* qcril_gstk_qmi_request_stk_send_terminal_response */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim
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
void qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type            instance_id   = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type               modem_id      = QCRIL_DEFAULT_MODEM_ID;
  int                                 user_accepted = 0;
  RIL_Errno                           ril_result    = RIL_E_SUCCESS;
  int                                 qmi_err_code  = 0;
  uint8                               slot_index    = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  cat_event_confirmation_req_msg_v02  event_conf_req;
  cat_event_confirmation_resp_msg_v02 event_conf_resp;
  qcril_request_resp_params_type      resp;

  /*-----------------------------------------------------------------------*/

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    return;
  }

  if ((params_ptr->data    == NULL) ||
      (params_ptr->datalen != sizeof(int)))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL data pointer or params_ptr->datalen: 0x%x");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  slot_index  = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
  modem_id = params_ptr->modem_id;

  if ((instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (slot_index  >= qcril_gstk_get_num_slots()) ||
      (modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, slot_index: 0x%x, modem_id: 0x%x",
                     instance_id, slot_index, modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim\n");

  memset(&event_conf_req, 0x00, sizeof(cat_event_confirmation_req_msg_v02));
  memset(&event_conf_resp, 0x00, sizeof(cat_event_confirmation_resp_msg_v02));
  memset(&resp, 0x00, sizeof(qcril_request_resp_params_type));

  user_accepted = *(int *) params_ptr->data;
  QCRIL_LOG_INFO("User %s call\n",(user_accepted ? "ACCEPTED" : "REJECTED"));

  /* Limitation in UI: UI has no knowledge if icon is being displayed
     successfully. Assume icon displayed and send icon conf = TRUE */
  event_conf_req.display_valid   = TRUE;
  event_conf_req.display.display = 0x01;
  event_conf_req.confirm_valid   = TRUE;
  event_conf_req.confirm.confirm = (user_accepted ? 0x01 : 0x00);
  event_conf_req.slot_valid      = TRUE;
  event_conf_req.slot.slot       = qcril_gstk_qmi_convert_slot_index_to_slot_type(slot_index);

  /* Pass into GSTK and ensure it was well received */
  QCRIL_LOG_QMI( modem_id, "qmi_cat_service", "setup_call_user_cnf_alpha_rsp" );

  /* Use QMI handle from the command cache */
  if((qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr) &&
     ((qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr->command_type ==
         QCRIL_GSTK_QMI_CMD_STK_SET_UP_CALL)||
      (qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr->command_type ==
         QCRIL_GSTK_QMI_CMD_STK_OPEN_CHANNEL)))
  {
    QCRIL_LOG_QMI( modem_id, "qmi_uim_service", "event confirmation" );
    qmi_err_code = qmi_client_send_msg_sync_with_shm(
                     qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr->handle,
                     QMI_CAT_EVENT_CONFIRMATION_REQ_V02,
                     (void *) &event_conf_req,
                     sizeof(cat_event_confirmation_req_msg_v02),
                     (void *) &event_conf_resp,
                     sizeof(cat_event_confirmation_resp_msg_v02),
                     QMI_CAT_DEFAULT_TIMEOUT);
    if (qmi_err_code != 0)
    {
      QCRIL_LOG_ERROR("Error for EVENT_CONFIRMATION_REQ, client_err: 0x%x, error_code: 0x%x\n",
                      qmi_err_code, event_conf_resp.resp.error);
      ril_result = RIL_E_GENERIC_FAILURE;
    }

    /* Clear the global proactive command cache, irrespective of the result */
    QCRIL_LOG_INFO("%s", "Dismiss pending proactive cmd!\n");
    QCRIL_GSTK_QMI_FREE_PTR(qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr);
  }
  else
  {
    QCRIL_LOG_ERROR("%s", "set up call or bip command not cached");
    ril_result = RIL_E_GENERIC_FAILURE;
  }

  /* Notify the caller of the result */
  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id,
                                      ril_result, &resp );
  qcril_send_request_response( &resp );

} /* qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim */

/*=========================================================================
  FUNCTION:  qcril_gstk_process_notify_ril_is_ready
===========================================================================*/
/*!
  @brief
    Handle the QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY event that is passed to
    qcril_event_queue by getVersion() in qcril.c file.

    If QCRIL_GSTK_QMI_RIL_IS_READY_BMSK is not set, the function will set the
    flag and try to send terminal profile.

  @param[in]  params_ptr  Should be NULL
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
*/
/*=========================================================================*/
void qcril_gstk_qmi_process_notify_ril_is_ready
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  uint32                    timer_id;
  uint8                     slot_index = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;

  QCRIL_LOG_INFO("%s", "qcril_gstk_process_notify_ril_is_ready\n");

  /*-----------------------------------------------------------------------*/

  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
  modem_id = params_ptr->modem_id;

  if ((instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (slot_index  >= qcril_gstk_get_num_slots()) ||
      (modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, slot_index: 0x%x, modem_id: 0x%x",
                     instance_id, slot_index, modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  /*-----------------------------------------------------------------------*/

  qcril_gstk_qmi_info.gstk_command_info[slot_index].flags |= QCRIL_GSTK_QMI_RIL_IS_READY_BMSK;

  /* Setup timer setup only if:
     1. Cache is present, in this case we need to send to Telephony STK App, AND
     2. There is no timer already running */
  if ((qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr != NULL) &&
      (qcril_gstk_qmi_info.timer_id == 0))
  {
    qcril_setup_timed_callback( instance_id, modem_id,
                                qcril_gstk_qmi_resend_proactive_cmd,
                                &QCRIL_GSTK_QMI_TIMER_RESEND,
                                &timer_id );
    qcril_gstk_qmi_info.timer_id = timer_id;
  }

  /* Get supported proactive commands from QMI CAT for STK recovery */
  qcril_gstk_qmi_get_recovery_proactive_cache(instance_id);
} /* qcril_gstk_process_notify_ril_is_ready */

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_service_is_running
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
void qcril_gstk_qmi_request_stk_service_is_running
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  if (params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }
  instance_id = params_ptr->instance_id;
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    QCRIL_LOG_ERROR("Invalid instance_id %d", instance_id);
    QCRIL_ASSERT(0);
    return;
  }

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_service_is_running\n");

  /* Notify GSTK that RIL STK is ready */
  /* This is to solve the issue of SIM card sending proactive cmd too early to RIL */
  (void)qcril_event_queue( instance_id, modem_id, QCRIL_DATA_ON_STACK,
                           QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY, NULL, 0, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  qcril_send_request_response( &resp );

} /* qcril_gstk_qmi_request_stk_service_is_running */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_scws_data_available_callback
 *   ======================================================================*/
/*!
 *   @brief
 *     Callback that is executed by the SCWS agent when it needs to send any
 *     data to the modem.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
void qcril_gstk_qmi_scws_data_available_callback
(
  uint32                      bip_id,
  qcril_scws_slot_enum_type   slot_id,
  uint16                      data_len,
  uint8                     * data_ptr,
  uint16                      remaining_data_len
)
{
  int                                     i                       = 0;
  int                                     qmi_err_code            = 0;
  qcril_gstk_original_request_type      * orig_ptr                = NULL;
  cat_scws_data_available_req_msg_v02   * data_available_req_ptr  = NULL;
  cat_scws_data_available_resp_msg_v02  * data_available_resp_ptr = NULL;
  qmi_txn_handle                          txn_handle;

  QCRIL_LOG_INFO( "%s, for bip_id: 0x%X, scws_slot_id: %d, data_len: 0x%X, remaining_data_len: 0x%X \n",
                 __FUNCTION__, bip_id, slot_id, data_len, remaining_data_len);

  /* Allocate request pointer on the heap */
  data_available_req_ptr = (cat_scws_data_available_req_msg_v02 *)
                      qcril_malloc(sizeof(cat_scws_data_available_req_msg_v02));
  if (data_available_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for data_available_req_ptr!");
    return;
  }

  /* Init data available params */
  memset(data_available_req_ptr, 0x00, sizeof(cat_scws_data_available_req_msg_v02));
  data_available_req_ptr->result.data_len    = data_len;
  if (data_available_req_ptr->result.data_len > QMI_CAT_SCWS_DATA_MAX_LENGTH_V02)
  {
    QCRIL_LOG_ERROR("Data length exceeds limt, data_len: 0x%x!",
                    data_available_req_ptr->result.data_len);
    /* Free allocated request pointer */
    QCRIL_GSTK_QMI_FREE_PTR(data_available_req_ptr);
    return;
  }

  data_available_req_ptr->remaining_data_len = remaining_data_len;
  data_available_req_ptr->result.ch_id       = bip_id;
  data_available_req_ptr->slot_valid         = TRUE;
  data_available_req_ptr->slot.slot          = qcril_gstk_qmi_convert_scws_slot_id(slot_id);

  /* Send data in packets of size QCRIL_GSTK_QMI_SCWS_DATA_PKT_SIZE or less */
  for(i = 0; i < data_len; i += QCRIL_GSTK_QMI_SCWS_DATA_PKT_SIZE)
  {
    uint16 len_to_send = 0;

    /* Allocate original request */
    orig_ptr = (qcril_gstk_original_request_type*)
                  qcril_malloc(sizeof(qcril_gstk_original_request_type));
    if (orig_ptr == NULL)
    {
      QCRIL_LOG_ERROR("%s", "error allocating memory for original_request_type!");
      /* Free allocated request pointer */
      QCRIL_GSTK_QMI_FREE_PTR(data_available_req_ptr);
      return;
    }

    /* Update request parameters */
    memset(orig_ptr, 0, sizeof(qcril_gstk_original_request_type));

    /* Since we support on both RILs */
  #ifdef FEATURE_QCRIL_UIM_QMI_RPC_QCRIL
    switch (data_available_req_ptr->slot.slot)
    {
      case CAT_SLOT3_V02:
        orig_ptr->instance_id = QCRIL_THIRD_INSTANCE_ID;
        break;
      case CAT_SLOT2_V02:
        orig_ptr->instance_id = QCRIL_SECOND_INSTANCE_ID;
        break;
      case CAT_SLOT1_V02:
      default:
        orig_ptr->instance_id = QCRIL_DEFAULT_INSTANCE_ID;
        QCRIL_LOG_INFO( "%s\n", "Using QCRIL_DEFAULT_INSTANCE_ID");
        break;
    }
  #else
    orig_ptr->instance_id = qmi_ril_get_process_instance_id();
  #endif
    orig_ptr->modem_id     = QCRIL_DEFAULT_MODEM_ID;
    orig_ptr->bip_id       = bip_id;
    orig_ptr->slot_id      = slot_id;

    if (i+QCRIL_GSTK_QMI_SCWS_DATA_PKT_SIZE < data_len)
    {
      len_to_send = QCRIL_GSTK_QMI_SCWS_DATA_PKT_SIZE;
    }
    else
    {
      len_to_send = data_len-i;
    }

    /* Update data & length */
    data_available_req_ptr->result.data_len = len_to_send;
    memcpy(data_available_req_ptr->result.data,
           data_ptr + i,
           data_available_req_ptr->result.data_len);

    /* Allocate response pointer since the command is an async one */
    data_available_resp_ptr = (cat_scws_data_available_resp_msg_v02 *)
                                qcril_malloc(sizeof(cat_scws_data_available_resp_msg_v02));
    if (data_available_resp_ptr == NULL)
    {
      QCRIL_LOG_ERROR("%s", "error allocating memory for data_available_resp_ptr!");
      QCRIL_GSTK_QMI_FREE_PTR(orig_ptr);
      QCRIL_GSTK_QMI_FREE_PTR(data_available_req_ptr);
      return;
    }

    memset(data_available_resp_ptr, 0x00, sizeof(cat_scws_data_available_resp_msg_v02));

    QCRIL_LOG_QMI( orig_ptr->modem_id, "qmi_uim_service", "event confirmation" );
    qmi_err_code = qmi_client_send_msg_async(
                     qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                     QMI_CAT_SCWS_DATA_AVAILABLE_REQ_V02,
                     (void *) data_available_req_ptr,
                     sizeof(cat_scws_data_available_req_msg_v02),
                     (void *) data_available_resp_ptr,
                     sizeof(cat_scws_data_available_resp_msg_v02),
                     qcril_gstk_qmi_command_cb,
                     orig_ptr,
                     &txn_handle);
    if (qmi_err_code != 0)
    {
      QCRIL_LOG_ERROR("Error for DATA_AVAILABLE_REQ, client_err: 0x%x, error_code: 0x%x\n",
                      qmi_err_code, data_available_resp_ptr->resp.error);
      QCRIL_GSTK_QMI_FREE_PTR(orig_ptr);
      QCRIL_GSTK_QMI_FREE_PTR(data_available_resp_ptr);
      break;
    }
  }
  /* Free allocated request pointer */
  QCRIL_GSTK_QMI_FREE_PTR(data_available_req_ptr);
} /* qcril_gstk_qmi_scws_data_available_callback */


/*===========================================================================
 *   FUNCTION:  qcril_gstk_qmi_scws_channel_status_callback
 *   ======================================================================*/
/*!
 *   @brief
 *     Callback that is executed by the SCWS agent when it needs to send any
 *     channel status information to the modem.
 *
 *   @return
 *                                                                         */
/*=========================================================================*/
void qcril_gstk_qmi_scws_channel_status_callback
(
  uint32                              bip_id,
  qcril_scws_slot_enum_type           slot_id,
  qcril_scws_socket_state_enum_type   socket_state
)
{
  int                                   qmi_err_code = 0;
  cat_scws_channel_status_req_msg_v02   channel_status_req;
  cat_scws_channel_status_resp_msg_v02  channel_status_resp;

  QCRIL_LOG_INFO( "%s, for bip_id: 0x%X, scws_slot_id: %d, socket_state: 0x%X \n",
                  __FUNCTION__, bip_id, slot_id, socket_state);

  memset(&channel_status_req, 0x00, sizeof(cat_scws_channel_status_req_msg_v02));
  memset(&channel_status_resp, 0x00, sizeof(cat_scws_channel_status_resp_msg_v02));

  /* Init channel status params */
  channel_status_req.slot_valid           = TRUE;
  channel_status_req.slot.slot            = qcril_gstk_qmi_convert_scws_slot_id(slot_id);
  channel_status_req.channel_status.ch_id = bip_id;
  channel_status_req.channel_status.state = qcril_gstk_qmi_convert_scws_socket_state(socket_state);;

  QCRIL_LOG_QMI( QCRIL_DEFAULT_MODEM_ID, "qmi_uim_service", "scws channel status" );
  qmi_err_code = qmi_client_send_msg_sync_with_shm(
                   qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                   QMI_CAT_SCWS_CHANNEL_STATUS_REQ_V02,
                   (void *) &channel_status_req,
                   sizeof(cat_scws_channel_status_req_msg_v02),
                   (void *) &channel_status_resp,
                   sizeof(cat_scws_channel_status_resp_msg_v02),
                   QMI_CAT_DEFAULT_TIMEOUT);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_ERROR("Error for SCWS_CHANNEL_STATUS_REQ, client_err: 0x%x, error_code: 0x%x\n",
                    qmi_err_code, channel_status_resp.resp.error);
  }
} /* qcril_gstk_qmi_scws_channel_status_callback */


/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_send_envelope_with_status
===========================================================================*/
/*!
  @brief
    Handles RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS.

  @param[in]  params_ptr  Pointer to a struct containing the request data
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @msc
  @endmsc

  @see
*/
/*=========================================================================*/
void qcril_gstk_qmi_request_stk_send_envelope_with_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_instance_id_e_type            instance_id     = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type               modem_id        = QCRIL_DEFAULT_MODEM_ID;
  RIL_Errno                           ril_result      = RIL_E_GENERIC_FAILURE;
  int                                 len             = 0;
  int                                 qmi_err_code    = 0;
  uint8                               slot_index      = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  cat_send_envelope_cmd_req_msg_v02 * env_cmd_req_ptr = NULL;;
  cat_send_envelope_cmd_resp_msg_v02  env_cmd_resp;
  qcril_request_resp_params_type      resp;
  RIL_SIM_IO_Response                 ril_response;

  QCRIL_LOG_INFO("%s", "qcril_gstk_qmi_request_stk_send_envelope_with_status\n");

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  if ((params_ptr->data    == NULL) ||
      (params_ptr->datalen == 0))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL data pointer or zero params_ptr->datalen");
    QCRIL_ASSERT(0);
    return;
  }

  instance_id = params_ptr->instance_id;
  slot_index  = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);
  modem_id = params_ptr->modem_id;

  if ((instance_id >= QCRIL_MAX_INSTANCE_ID) ||
      (slot_index  >= QCRIL_GSTK_MAX_CARD_COUNT) ||
      (modem_id    >= QCRIL_MAX_MODEM_ID))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, slot_index: 0x%x, modem_id: 0x%x",
                     instance_id, slot_index, modem_id);
    QCRIL_ASSERT(0);
    return;
  }

  memset(&env_cmd_resp, 0x00, sizeof(cat_send_envelope_cmd_resp_msg_v02));
  memset(&resp      , 0x00, sizeof(qcril_request_resp_params_type));
  memset(&ril_response, 0, sizeof(RIL_SIM_IO_Response));

  /* Allocate request pointer on the heap */
  env_cmd_req_ptr = (cat_send_envelope_cmd_req_msg_v02 *)
                      qcril_malloc(sizeof(cat_send_envelope_cmd_req_msg_v02));
  if (env_cmd_req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "error allocating memory for env_cmd_req_ptr!");
    goto send_envelope_response;
    return;
  }

  memset(env_cmd_req_ptr, 0x00, sizeof(cat_send_envelope_cmd_req_msg_v02));

  /* Allocate memory for the raw envelope */
  len = strlen((char *) params_ptr->data);
  QCRIL_LOG_INFO("Envelope data length is %d\n",len);

  env_cmd_req_ptr->slot_valid = TRUE;
  env_cmd_req_ptr->slot.slot  = qcril_gstk_qmi_convert_slot_index_to_slot_type(slot_index);

  env_cmd_req_ptr->envelope_cmd.env_cmd_type      = CAT_ENVELOPE_CMD_TYPE_UNKNOWN_V02;
  env_cmd_req_ptr->envelope_cmd.envelope_data_len = QCRIL_GSTK_QMI_ENVCMD_DATA_SIZE(len);
  if (env_cmd_req_ptr->envelope_cmd.envelope_data_len > QMI_CAT_ENVELOPE_DATA_MAX_LENGTH_V02)
  {
    QCRIL_LOG_ERROR("Length of Envelope too long: 0x%x\n",
                     env_cmd_req_ptr->envelope_cmd.envelope_data_len);
    goto send_envelope_response;
  }

  /* Copy over the command and send to the card */
  QCRIL_GSTK_QMI_ENVCMD_COPY(env_cmd_req_ptr->envelope_cmd.envelope_data,
                             params_ptr->data,
                             len);

  /* Primary will always send envelope commands to card */
  QCRIL_LOG_QMI( modem_id, "qmi_gstk_service", "send_envelope" );
  qmi_err_code = qmi_client_send_msg_sync_with_shm(qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                                          QMI_CAT_SEND_ENVELOPE_CMD_REQ_V02,
                                          (void *) env_cmd_req_ptr,
                                          sizeof(cat_send_envelope_cmd_req_msg_v02),
                                          (void *) &env_cmd_resp,
                                          sizeof(cat_send_envelope_cmd_resp_msg_v02),
                                          QMI_CAT_DEFAULT_TIMEOUT);
  if (qmi_err_code != 0)
  {
    QCRIL_LOG_ERROR("Error for SEND_ENVELOPE_CMD_REQ, client_err: 0x%x, error_code: 0x%x\n",
                    qmi_err_code, env_cmd_resp.resp.error);
    goto send_envelope_response;
  }

  if (env_cmd_resp.env_resp_data_valid != TRUE)
  {
    QCRIL_LOG_ERROR("%s", "Raw envelope response TLV is invalid !\n");
    goto send_envelope_response;
  }

  /* Update the response upon success */
  ril_result       = RIL_E_SUCCESS;
  ril_response.sw1 = env_cmd_resp.env_resp_data.sw1;
  ril_response.sw2 = env_cmd_resp.env_resp_data.sw2;

  /* Allocate the ASCII string for response, if needed */
  if ((env_cmd_resp.env_resp_data.env_resp_data_len > 0) &&
      (env_cmd_resp.env_resp_data.env_resp_data_len <=
         QMI_CAT_RAW_ENV_RSP_DATA_MAX_LENGTH_V02))
  {
    QCRIL_LOG_DEBUG( "send_envelope_with_status, env_resp_data_len: 0x%x\n",
                    env_cmd_resp.env_resp_data.env_resp_data_len);

    len = QCRIL_GSTK_QMI_ENVRSP_DATA_SIZE(env_cmd_resp.env_resp_data.env_resp_data_len) + 1;
    ril_response.simResponse = (char *)qcril_malloc(len);
    if (ril_response.simResponse == NULL)
    {
      QCRIL_LOG_ERROR("%s", "Unable to allocate memory for Envelope response!");
      goto send_envelope_response;
    }

    memset(ril_response.simResponse, 0, len);
    QCRIL_GSTK_QMI_ENVRSP_COPY((uint8 *)ril_response.simResponse,
                               env_cmd_resp.env_resp_data.env_resp_data,
                               env_cmd_resp.env_resp_data.env_resp_data_len);
  }

  QCRIL_LOG_DEBUG( "send_envelope_with_status, RIL_SIM_IO_Response: sw1=%d sw2=%d data=%s\n",
                  ril_response.sw1, ril_response.sw2,
                  ril_response.simResponse != NULL ? ril_response.simResponse : "NULL");

send_envelope_response:
  /* Since this is synchronous call to QMI we respond here with the result.
     Fill the default data first and then the response data */

  qcril_default_request_resp_params( instance_id,
                                     params_ptr->t,
                                     params_ptr->event_id,
                                     ril_result,
                                     &resp );

  resp.resp_pkt = (void *)&ril_response;
  resp.resp_len = sizeof(RIL_SIM_IO_Response);

  qcril_send_request_response( &resp );

  /* Free the response pointer we allocated earlier */
  if (ril_response.simResponse)
  {
    QCRIL_GSTK_QMI_FREE_PTR(ril_response.simResponse);
  }

  /* Free allocated request pointer */
  QCRIL_GSTK_QMI_FREE_PTR(env_cmd_req_ptr);
} /* qcril_gstk_qmi_request_stk_send_envelope_with_status */


/*=========================================================================
  FUNCTION:  qcril_gstk_qmi_process_card_error
===========================================================================*/
/*!
  @brief
    Handles the QCRIL_EVT_GSTK_QMI_NOTIFY_CARD_ERROR event that is posted by
    QCRIL_UIM module. If any proactive command was cached in the global,
    this function will clear it.

  @param[in]  params_ptr  Should be NULL
  @param[in]  ret_ptr     Return argument by which the function can indicate
                          whether the radio or SIM state was changed

  @return
    Nothing

  @see
*/
/*=========================================================================*/
void qcril_gstk_qmi_process_card_error
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  int slot_index = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;

  /* Sanity checks */
  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, cannot process request");
    QCRIL_ASSERT(0);
    return;
  }

  if ((params_ptr->data == NULL) || (params_ptr->datalen == 0))
  {
    QCRIL_LOG_ERROR("%s", "Invalid input, NULL data pointer or zero datalen");
    QCRIL_ASSERT(0);
    return;
  }

  /* Extract slot index from incoming data */
  slot_index = *((int*)params_ptr->data);
  if ((slot_index < 0) ||(slot_index >= QCRIL_GSTK_MAX_CARD_COUNT))
  {
    QCRIL_LOG_ERROR("Invalid slot index for cleanup: 0x%x", slot_index);
    return;
  }

  QCRIL_LOG_INFO("qcril_gstk_process_card_error for slot_id: 0x%x", slot_index);

  /* Clear the global proactive command cache, if present */
  if (qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr != NULL)
  {
    QCRIL_LOG_INFO("%s", "Dismiss pending proactive cmd!\n");
    qcril_free(qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr);
    qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr = NULL;
    /* Also de-activate the timer, if running */
    qcril_gstk_qmi_deactivate_timer();
  }
} /* qcril_gstk_qmi_process_card_error */


/*=========================================================================
  FUNCTION:  qcril_gstk_qmi_get_recovery_proactive_cache
===========================================================================*/
/*!
 *   @brief
 *     Requests recovery proactive commands from QMI CAT and performs the
 *     necessary data parsing, packaging the data and sends the UNSOL event
 *     to Android Telephony.
 *
 *   @return
 *     None
 *                                                                         */
/*=========================================================================*/
static void qcril_gstk_qmi_get_recovery_proactive_cache
(
  qcril_instance_id_e_type  instance_id
)
{
  qcril_gstk_qmi_unsol_info_type              ril_unsol_resp;
  cat_get_cached_proactive_cmd_req_msg_v02    get_cache_req;
  cat_get_cached_proactive_cmd_resp_msg_v02  *get_cache_resp_ptr = NULL;
  qmi_client_error_type                       qmi_err_code       = QMI_NO_ERR;
  uint8                                       slot_index         = QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE;
  uint8                                       i                  = 0;
  /* Map STK_CMD to CAT_CACHED_COMMAND_ID */
  qcril_gstk_cmd_id_qmi_req_id_map_type recovery_proactive_commands[] =
  {
    {QCRIL_GSTK_QMI_CMD_STK_SET_UP_MENU,           CAT_CACHED_COMMAND_ID_SETUP_MENU_V02},
    {QCRIL_GSTK_QMI_CMD_STK_SET_UP_EVENT_LIST,     CAT_CACHED_COMMAND_ID_SETUP_EVENT_LIST_V02},
    {QCRIL_GSTK_QMI_CMD_STK_SET_UP_IDLE_MODE_TEXT, CAT_CACHED_COMMAND_ID_SETUP_IDLE_TEXT_V02}
  };

  QCRIL_LOG_INFO("Enter: instance_id=0x%x", instance_id);

  slot_index = qcril_gstk_qmi_convert_instance_to_slot_index(instance_id);

  if ((instance_id >= QCRIL_MAX_INSTANCE_ID)               ||
      (slot_index  == QCRIL_GSTK_INVALID_SLOT_INDEX_VALUE) ||
      (slot_index  >= qcril_gstk_get_num_slots()))
  {
    QCRIL_LOG_ERROR("Invalid values, instance_id: 0x%x, slot_index: 0x%x",
                     instance_id, slot_index);
    QCRIL_ASSERT(0);
    return;
  }

  get_cache_resp_ptr = (cat_get_cached_proactive_cmd_resp_msg_v02 *)qcril_malloc(
                        sizeof(cat_get_cached_proactive_cmd_resp_msg_v02));
  if(NULL == get_cache_resp_ptr)
  {
    QCRIL_LOG_ERROR("qcril_malloc fail get_cache_resp_ptr\n");
    QCRIL_ASSERT(0);
    return;
  }

  memset(&get_cache_req, 0x00, sizeof(get_cache_req));
  get_cache_req.slot_valid = TRUE;
  get_cache_req.slot.slot  = qcril_gstk_qmi_convert_slot_index_to_slot_type(slot_index);

  for (i = 0;
       i < sizeof(recovery_proactive_commands)/sizeof(recovery_proactive_commands[0]);
       i++)
  {
    /* Get command from QMI only if it is not in QCRIL cache */
    if (qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr &&
        recovery_proactive_commands[i].gstk_cmd_id ==
          qcril_gstk_qmi_info.gstk_command_info[slot_index].cmd_ptr->command_type)
    {
      continue;
    }

    QCRIL_LOG_INFO("Requesting qmi_req_cmd_id=0x%x", recovery_proactive_commands[i].qmi_req_cmd_id);

    get_cache_req.command_id = recovery_proactive_commands[i].qmi_req_cmd_id;
    memset(get_cache_resp_ptr, 0x00, sizeof(*get_cache_resp_ptr));
    qmi_err_code = qmi_client_send_msg_sync(
                          qcril_gstk_qmi_info.qmi_cat_svc_client_primary,
                          QMI_CAT_GET_CACHED_PROACTIVE_CMD_REQ_V02,
                          (void *)&get_cache_req,
                          sizeof(get_cache_req),
                          (void *)get_cache_resp_ptr,
                          sizeof(*get_cache_resp_ptr),
                          QMI_CAT_DEFAULT_TIMEOUT);
    if (qmi_err_code               != QMI_NO_ERR ||
        get_cache_resp_ptr->resp.result != QMI_RESULT_SUCCESS_V01 ||
        get_cache_resp_ptr->resp.error  != QMI_ERR_NONE_V01)
    {
      QCRIL_LOG_ERROR("Error GET_CACHED_PROACTIVE_CMD_REQ, qmi_err_code: 0x%x, result: 0x%x, error: 0x%x\n",
                      qmi_err_code, get_cache_resp_ptr->resp.result, get_cache_resp_ptr->resp.error);
      continue;
    }

    /* Prepare qcril_gstk_qmi_unsol_info_type */
    memset(&ril_unsol_resp, 0x00, sizeof(ril_unsol_resp));
    ril_unsol_resp.ril_unsol_type = RIL_UNSOL_STK_EVENT_NOTIFY;
    switch (get_cache_req.command_id)
    {
      case CAT_CACHED_COMMAND_ID_SETUP_MENU_V02:
        if (get_cache_resp_ptr->setup_menu_valid)
        {
          if (get_cache_resp_ptr->setup_menu.pc_setup_menu_len < QCRIL_GSTK_QMI_COMMAND_MIN_SIZE ||
              get_cache_resp_ptr->setup_menu.pc_setup_menu_len > QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02)
          {
            QCRIL_LOG_ERROR("Length out of range: 0x%x, discarding TLV",
                            get_cache_resp_ptr->setup_menu.pc_setup_menu_len);
            return;
          }

          /* Allocate & copy response packet's data & length */
          QCRIL_GSTK_QMI_MALLOC_AND_CPY_CMD(ril_unsol_resp.ril_unsol_resp.resp_pkt,
                                            ril_unsol_resp.ril_unsol_resp.resp_len,
                                            get_cache_resp_ptr->setup_menu.pc_setup_menu,
                                            get_cache_resp_ptr->setup_menu.pc_setup_menu_len);
        }
        break;

      case CAT_CACHED_COMMAND_ID_SETUP_EVENT_LIST_V02:
        if (get_cache_resp_ptr->setup_event_list_raw_valid)
        {
          if (get_cache_resp_ptr->setup_event_list_raw.pc_setup_event_list_len < QCRIL_GSTK_QMI_COMMAND_MIN_SIZE ||
              get_cache_resp_ptr->setup_event_list_raw.pc_setup_event_list_len > QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02)
          {
            QCRIL_LOG_ERROR("Length out of range: 0x%x, discarding TLV",
                            get_cache_resp_ptr->setup_event_list_raw.pc_setup_event_list_len);
            return;
          }

          /* Allocate & copy response packet's data & length */
          QCRIL_GSTK_QMI_MALLOC_AND_CPY_CMD(ril_unsol_resp.ril_unsol_resp.resp_pkt,
                                            ril_unsol_resp.ril_unsol_resp.resp_len,
                                            get_cache_resp_ptr->setup_event_list_raw.pc_setup_event_list,
                                            get_cache_resp_ptr->setup_event_list_raw.pc_setup_event_list_len);
        }
        break;

      case CAT_CACHED_COMMAND_ID_SETUP_IDLE_TEXT_V02:
        if (get_cache_resp_ptr->idle_mode_text_valid)
        {
          if (get_cache_resp_ptr->idle_mode_text.pc_setup_idle_mode_text_len < QCRIL_GSTK_QMI_COMMAND_MIN_SIZE ||
              get_cache_resp_ptr->idle_mode_text.pc_setup_idle_mode_text_len > QMI_CAT_RAW_PROACTIVE_CMD_MAX_LENGTH_V02)
          {
            QCRIL_LOG_ERROR("Length out of range: 0x%x, discarding TLV",
                            get_cache_resp_ptr->idle_mode_text.pc_setup_idle_mode_text_len);
            return;
          }

          /* Allocate & copy response packet's data & length */
          QCRIL_GSTK_QMI_MALLOC_AND_CPY_CMD(ril_unsol_resp.ril_unsol_resp.resp_pkt,
                                            ril_unsol_resp.ril_unsol_resp.resp_len,
                                            get_cache_resp_ptr->idle_mode_text.pc_setup_idle_mode_text,
                                            get_cache_resp_ptr->idle_mode_text.pc_setup_idle_mode_text_len);
        }
        break;

      default:
        break;
    }

    QCRIL_LOG_INFO("Received response: resp_pkt=0x%x, resp_len=0x%x",
                   ril_unsol_resp.ril_unsol_resp.resp_pkt, ril_unsol_resp.ril_unsol_resp.resp_len);

    /* Send qcril_unsol_resp_params_type to STK */
    if (ril_unsol_resp.ril_unsol_resp.resp_pkt &&
        ril_unsol_resp.ril_unsol_resp.resp_len)
    {
      QCRIL_LOG_INFO("Sending RIL_UNSOL_STK_EVENT_NOTIFY for 0x%x",
                     get_cache_req.command_id);

      qcril_gstk_qmi_send_unsol_resp(instance_id,
                                     ril_unsol_resp.ril_unsol_type,
                                     ril_unsol_resp.ril_unsol_resp.resp_pkt,
                                     ril_unsol_resp.ril_unsol_resp.resp_len);

      /* Free buffer allocated for the UNSOL response */
      QCRIL_GSTK_QMI_FREE_PTR(ril_unsol_resp.ril_unsol_resp.resp_pkt);
    }
  }

  /* Free  allocated get_cache_resp_ptr */
  QCRIL_GSTK_QMI_FREE_PTR(get_cache_resp_ptr);
}/* qcril_gstk_qmi_get_recovery_proactive_cache */


#else  /* FEATURE_CDMA_NON_RUIM  */

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
static void qcril_gstk_qmi_not_supported
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

} /* qcril_gstk_qmi_not_supported */

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_init
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
void qcril_gstk_qmi_init( void )
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_qmi_init(): Not Supported\n");
} /* qcril_gstk_qmi_init */



/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_set_profile
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
void qcril_gstk_qmi_request_stk_set_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_qmi_not_supported(
    "qcril_gstk_request_stk_set_profile()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_qmi_request_stk_set_profile */

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_get_profile
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
void qcril_gstk_qmi_request_stk_get_profile
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_qmi_not_supported(
    "qcril_gstk_qmi_request_stk_set_profile()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_request_stk_get_profile */

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_send_envelope_command
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
void qcril_gstk_qmi_request_stk_send_envelope_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_qmi_not_supported(
    "qcril_gstk_qmi_request_stk_send_envelope_command()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_qmi_request_stk_send_envelope_command */

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
void qcril_gstk_qmi_request_stk_send_terminal_response
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_qmi_not_supported(
    "qcril_gstk_qmi_request_stk_send_terminal_response()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_qmi_request_stk_send_terminal_response */

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim
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
void qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  qcril_gstk_qmi_not_supported(
    "qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim()",
    params_ptr,
    ret_ptr);
} /* qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim */

/*===========================================================================
  FUNCTION:  qcril_gstk_qmi_process_notify_ril_is_ready
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
void qcril_gstk_qmi_process_notify_ril_is_ready
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  /* do nothing */
  QCRIL_LOG_INFO( "%s", "qcril_gstk_qmi_process_notify_ril_is_ready(): nothing to do.\n");
} /* qcril_gstk_qmi_process_notify_ril_is_ready */


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
void qcril_gstk_qmi_process_send_raw_envelope_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr
)
{
  QCRIL_LOG_INFO( "%s", "qcril_gstk_qmi_process_send_raw_envelope_callback(): Not Supported\n");
} /* qcril_gstk_qmi_process_send_raw_envelope_callback */

#endif /* FEATURE_CDMA_NON_RUIM */
#endif /* (FEATURE_QCRIL_QMI_CAT) */
