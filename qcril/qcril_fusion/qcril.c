/*!
  @file
  qcril.c

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril.c#67 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/12/10   at      Removed QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK for UIM QMI
09/30/10   at      Added secondary gw/cdma state checks for SIM/RUIM access
09/20/10   at      Allow RIL_REQUEST_SIM_IO in all active radio states
09/09/10   js      Allow STK commands to be handled in correct radio states
08/30/10   at      Removed QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE
08/27/10   js      Allow RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING to be
                   processed in all radio states
07/14/10   at      Added RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE
06/28/10   js      Support for GSTK QMI RIL
05/13/10   at      Added respective callbacks for FEATURE_QCRIL_UIM_QMI
03/05/10   sm      Added support for CM_PH_EVENT_PRL_INIT handling.
03/01/10   fc      Re-architecture to support split modem.
03/04/10   sk      Added DSAC changes with Featurization
02/09/10   pg      Added support to indicate call re-establishment to UI.
11/19/09   js      Support for illegal card state
10/02/09   sb      Added support for getting and setting the SMSC address.
09/15/09   sb      Added support for Extended Burst Type International DBM.
08/10/09   sb      Added support for RUIM SMS.
08/10/09   xz      Added support of RIL_REQUEST_STK_SERVICE_IS_RUNNING
                   Fixed the issue of unable to send terminal response
                   Added names of new request into qcril_request_name
08/07/09   sb      Added support for reporting memory status.
07/28/09   pg      At power up time, return the radio state associated with
                   the previously acquired network mode toavoid constant
                   phone images swapping.
                   Do not return both UNSOL_RADIO_STATE_CHANGE and
                   UNSOL_NETWORK_STATE_CHANGE for the same CM event.
07/24/09   sb      Added qcril_response_error2, so that a response can be sent
                   on the payload in the case of a failure.
07/22/09   sb      Added support for latest ril.h under FEATURE_NEW_RIL_API.
07/10/09   tml     Added support for intermediate pin2 verification steps
06/26/09   fc      Changes in call flow log packet.
06/15/09   nrn     Adding support for NAM programming
06/06/09   nrn     Adding support for Authentication and Registration Reject
06/03/09   nd      Added support for Otasp/Otapa.
06/01/09   sk      Changes related to pbm initialization, pbm event handling,
                   CM event CM_SS_EVENT_EMERG_NUM_LIST handling
05/29/09   fc      Added support for FTM mode.
05/26/09   fc      Removed the unnecessary de-init of Diag LSM.
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
                   Changes to support profiling of AMSS event.
05/14/09   pg      Added support for CDMA phase II under FEATURE_MULTIMODE_ANDROID_2.
                   Mainlined FEATURE_MULTIMODE_ANDROID.
04/28/09   fc      Added support to perform card powerup/powerdown for
                   LPM to ONLINE or vice versa transition.
04/14/09   fc      Changes to delay the reporting of subscription available
                   till modem back to online mode.
04/13/09   fc      Changes to de-register noisy CM events whenever screen off
                   for power optimization.
04/05/09   fc      Cleanup log macros and mutex macros.
03/17/09   fc      Added ONS support for NITZ.
02/23/09   fc      Disable subscription when SIM/RUIM state change to
                   QCRIL_SIM_STATE_NOT_READY.
02/20/09   fc      Fixed the issue of wrong states being used for Data events.
02/11/09   xz      Replace qcril_process_event() with qcril_event_queue()
02/09/09   fc      Changed to force RTRE config to NV subscription in case of
                   NV RTRE config set to CM_RTRE_CONFIG_RUIM_OR_DROP_BACK, but
                   runtime control of RTRE config is not enabled.
02/09/09   fc      Changed the power up sequence to start with Radio State
                   Unavailable, to command CM to LPM and subscription
                   unavailable, and to wait for the very first CM_PH_EVENT_INFO
                   before reading RTRE configuration and transition to Radio
                   State Off.
                   Changed all internal RPC calls to CM to use internal token ID.
                   Fixed incorrect call end params being set for
                   RIL_REQUEST_HANDUP_WAITING_OR_BACKGROUD in fading scenario.
01/30/09   pg      Get band/mode capability from CM_PH_EVENT_INFO.
01/28/09   pg      Added code to support CDMA only modem with GW Android UI
                   under FEATURE_CDMA_ONLY_HACK.
01/28/09   fc      Fixed invalid memory dereference and overflow issue in
                   processing the QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY.
01/27/09   xz      Added support of new event QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY
01/26/08   fc      Logged assertion info.
01/19/09   pg      Activate QCRIL CM client regardless if any of cm event
                   registration fail.
01/09/09   adg     Added initial GSTK support
01/09/09   tml     Featurize for 1x non ruim build
12/23/08   fc      Added the initialization of modem_state_changed and
                   acquired_radio_tech in qcril_request_return_type to fix the array
                   out-of-bound access issue.
12/22/08   tml     Added qcril_queue_event2 for unit test framework
12/16/08   fc      Added API to support the release of AMSS objects for ONCRPC.
12/08/08   pg      Added multi-mode data call hook up.
                   Added support to pass CDMA voice privacy mode to RILD.
                   For multi-mode, Radio State is now changed from
                   RADIO_STATE_NV_READY to RADIO_STATE_SIM_READY when modem
                   acquires GW system.
                   Initialized rtre_config to NV only when in multimode or
                   CDMA only mode.
                   Corrected RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED response
                   format.
12/04/08   fc      Added ONCRPC sanity check for NV and SND.
11/14/08   pg      Added temp CDMA data support.
11/05/08   fc      Added comments about RIL initializations.
10/29/08   pg      Fixed a Mutex deadlock problem in qcril_init().
10/28/08   pg      Added support for RIL_REQUEST_SET_LOCATION_UPDATES.
10/10/08   pg      Change token_id used internally from NULL to 0xFFFFFFFF.
10/06/08   pg      Added support for RIL_REQUEST_CDMA_SUBSCRIPTION.
09/30/09   pg      Moved code to activate and disactivate GW/CDMA subscription
                   out FEATURE_MULTIMODE_ANDROID.
09/30/08   pg      Reverted the previous change.  RIL_REQUEST_DEVICE_ID can be
                   handled in all active states now.
09/22/08   pg      Restrict RIL_REQUEST_DEVICE_ID to subscription available
                   states only.
09/11/08   asn     Added data support
08/07/08   pg      Changed CM event that request DTMF wait on.
08/04/08   pg      Added support for GET IMEI, IMEISV.
06/11/08   jod     Added support for GET IMSI function.
05/28/08   tml     Added FDN, PIN enabling/disabling and Perso supports
05/22/08   tml     Fixed compilation issue with LTK
05/20/08   tml     Added internal command support for qcril mmgsdi
05/19/08   tml     Separate UIM into mmgsdi and gstk
05/07/08   fc      Added CM events.
05/04/08   da      Initial framework.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#ifdef FEATURE_QC_TIME_SERVICES
#include <cutils/properties.h>
#include <time_genoff.h>
#endif
#include <time.h>
#include "comdef.h"
#include "oncrpc.h"
#include "cm.h"
#include "cm_rpc.h"
#include "wms.h"
#include "wms_rpc.h"
#include "nv.h"
#include "nv_rpc.h"
#include "pbmlib.h"
#include "pbmlib_rpc.h"

#ifdef FEATURE_QCRIL_FUSION
#include "cm_fusion_rpc.h"
#include "wms_fusion_rpc.h"
#include "pbmlib_fusion_rpc.h"
#endif /* FEATURE_QCRIL_FUSION */

#ifndef FEATURE_CDMA_NON_RUIM
#ifndef FEATURE_QCRIL_UIM_QMI
#include "gsdi_exp.h"
#include "gsdi_exp_rpc.h"
#include "mmgsdilib.h"
#include "mmgsdilib_rpc.h"
#endif /* !FEATURE_QCRIL_UIM_QMI */
#ifndef FEATURE_QCRIL_QMI_CAT
#include "gstk_exp.h"
#include "gstk_exp_rpc.h"
#endif /* !FEATURE_QCRIL_QMI_CAT */
#endif /* FEATURE_CDMA_NON_RUIM */

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_arb.h"
#include "qcril_cm_clist.h"
#include "qcril_cm_ons.h"
#include "qcril_other.h"
#include "qcrilhook_oem.h"
#include "qcril_pbm.h"

#ifdef FEATURE_QCRIL_SAR
#include "rfm_sar.h"
#endif /* FEATURE_QCRIL_SAR */

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

extern void RIL_removeTimedCallback( void *param );

typedef void qcril_req_handler_type
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

/*! Hold general information about RIL_REQUESTs */
typedef struct qcril_dispatch_tag
{
  /*! field to link entries together in hash table */
  struct qcril_dispatch_tag *next_ptr;
  uint32 event_id;
  qcril_req_handler_type *handler;
  uint16 allowed_radio_states_mask;
} qcril_dispatch_table_entry_type;

/*! Hash table size, should be roughly equal to number of QCRIL events */
#define QCRIL_HT_ENTRIES_MAX 150

/* Bitmask represents the radio states
   Bit 5  - SIM or RUIM ready
   Bit 3  - Modem On
   Bit 2  - Modem Unavailable
   Bit 1  - Modem Off

   In case of ICS
   Bit 10 - Modem On   
*/
#ifndef FEATURE_ICS
#define QCRIL_STATE_MASK_ALL_STATES                    0x07
#define QCRIL_STATE_MASK_ALL_ACTIVE_STATES             0x05
#define QCRIL_STATE_MASK_SIM_OR_RUIM_READY_STATES      0x10
#else
#define QCRIL_STATE_MASK_ALL_STATES                    0x403
#define QCRIL_STATE_MASK_ALL_ACTIVE_STATES             0x401
#define QCRIL_STATE_MASK_SIM_OR_RUIM_READY_STATES      0x10
#endif

#define QCRIL_REG_ALL_STATES( event_id, handler )          \
  NULL, (uint32) event_id, handler, QCRIL_STATE_MASK_ALL_STATES

#define QCRIL_REG_ALL_ACTIVE_STATES( event_id, handler ) \
  NULL, (uint32) event_id, handler, QCRIL_STATE_MASK_ALL_ACTIVE_STATES

#define QCRIL_REG_SIM_OR_RUIM_READY_STATES( event_id, handler ) \
  NULL, (uint32) event_id, handler, QCRIL_STATE_MASK_SIM_OR_RUIM_READY_STATES

static qcril_dispatch_table_entry_type qcril_event_table[] =
{
  /**********************************************
   *                SIM (MMGSDI)                *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/

#ifdef FEATURE_QCRIL_UIM_QMI

  /* QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_QMI_COMMAND_CALLBACK, qcril_uim_process_qmi_callback ) },

  /* QCRIL_EVT_UIM_QMI_INDICATION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_UIM_QMI_INDICATION, qcril_uim_process_qmi_indication ) },

  /* QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_UIM_VERIFY_PIN_COMMAND_CALLBACK, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS, qcril_uim_process_internal_command ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUBS, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUBS, qcril_uim_process_internal_command ) },
  #endif /* FEATURE_QCRIL_DSDS */

  /* QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_START, qcril_uim_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_MODEM_RESTART_COMPLETE, qcril_uim_process_internal_command ) },

#else

  /* QCRIL_EVT_MMGSDI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_MMGSDI_COMMAND_CALLBACK, qcril_mmgsdi_process_command_callback ) },

  /* QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK, qcril_mmgsdi_process_imsi_command_callback ) },

  /* QCRIL_EVT_MMGSDI_INTERNAL_VERIFY_PIN_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_MMGSDI_INTERNAL_VERIFY_PIN_COMMAND_CALLBACK, qcril_mmgsdi_process_internal_verify_pin_command_callback ) },

  /* QCRIL_EVT_MMGSDI_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_MMGSDI_EVENT_CALLBACK, qcril_mmgsdi_process_event_callback ) },

  /* QCRIL_EVT_MMGSDI_GSDI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_MMGSDI_GSDI_COMMAND_CALLBACK, qcril_mmgsdi_process_gsdi_command_callback ) },

  /* QCRIL_EVT_MMGSDI_PERSO_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_MMGSDI_PERSO_EVENT_CALLBACK, qcril_mmgsdi_process_perso_event_callback ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUB */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_ACTIVATE_SUB, qcril_mmgsdi_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUB */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_DEACTIVATE_SUB, qcril_mmgsdi_process_internal_command ) },
  #endif /* FEATURE_QCRIL_DSDS */

  /* QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP, qcril_mmgsdi_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN, qcril_mmgsdi_process_internal_command ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_QUERY_FACILITY_FD */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS, qcril_mmgsdi_request_get_fdn_status ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_SET_FACILITY_FD */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS, qcril_mmgsdi_request_set_fdn_status ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_QUERY_FACILITY_SC */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS, qcril_mmgsdi_request_get_pin_status ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_SET_FACILITY_SC */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS, qcril_mmgsdi_request_set_pin_status ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_FDN_PBM_RECORD_UPDATE, qcril_mmgsdi_process_fdn_record_update_from_pbm ) },

  /* QCRIL_EVT_INTERNAL_MMGSDI_READ_UST_VALUE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_INTERNAL_MMGSDI_READ_UST_VALUE, qcril_mmgsdi_process_internal_read_ust_callback ) },

#endif /* FEATURE_QCRIL_UIM_QMI */

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

#ifdef FEATURE_QCRIL_UIM_QMI

  /* 1 - RIL_REQUEST_GET_SIM_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_SIM_STATUS, qcril_uim_request_get_sim_status ) },

  /* 2 - RIL_REQUEST_ENTER_SIM_PIN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PIN, qcril_uim_request_enter_pin ) },

  /* 3 - RIL_REQUEST_ENTER_SIM_PUK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PUK, qcril_uim_request_enter_puk ) },

  /* 4 - RIL_REQUEST_ENTER_SIM_PIN2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PIN2, qcril_uim_request_enter_pin ) },

  /* 5 - RIL_REQUEST_ENTER_SIM_PUK2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PUK2, qcril_uim_request_enter_puk ) },

  /* 6 - RIL_REQUEST_CHANGE_SIM_PIN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_SIM_PIN, qcril_uim_request_change_pin ) },

  /* 7 - RIL_REQUEST_CHANGE_SIM_PIN2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_SIM_PIN2, qcril_uim_request_change_pin ) },

  /* 8 - RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE, qcril_uim_request_enter_perso_key ) },

  /* 11 - RIL_REQUEST_GET_IMSI */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_IMSI, qcril_uim_request_get_imsi ) },

  /* 28 - RIL_REQUEST_SIM_IO */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIM_IO, qcril_uim_request_sim_io ) },

#else

  /* 1 - RIL_REQUEST_GET_SIM_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_SIM_STATUS, qcril_mmgsdi_request_get_sim_status ) },

  /* 2 - RIL_REQUEST_ENTER_SIM_PIN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PIN, qcril_mmgsdi_request_enter_pin ) },

  /* 3 - RIL_REQUEST_ENTER_SIM_PUK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PUK, qcril_mmgsdi_request_enter_puk ) },

  /* 4 - RIL_REQUEST_ENTER_SIM_PIN2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PIN2, qcril_mmgsdi_request_enter_pin ) },

  /* 5 - RIL_REQUEST_ENTER_SIM_PUK2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_SIM_PUK2, qcril_mmgsdi_request_enter_puk ) },

  /* 6 - RIL_REQUEST_CHANGE_SIM_PIN */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_SIM_PIN, qcril_mmgsdi_request_change_pin ) },

  /* 7 - RIL_REQUEST_CHANGE_SIM_PIN2 */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_SIM_PIN2, qcril_mmgsdi_request_change_pin ) },

  /* 8 - RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ENTER_DEPERSONALIZATION_CODE, qcril_mmgsdi_request_enter_perso_key ) },

  /* 11 - RIL_REQUEST_GET_IMSI */
  { QCRIL_REG_SIM_OR_RUIM_READY_STATES( RIL_REQUEST_GET_IMSI, qcril_mmgsdi_request_get_imsi ) },

  /* 28 - RIL_REQUEST_SIM_IO */
  { QCRIL_REG_SIM_OR_RUIM_READY_STATES( RIL_REQUEST_SIM_IO, qcril_mmgsdi_request_sim_io ) },

  /* 0x80004 - QCRIL_EVT_HOOK_ME_DEPERSONALIZATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_ME_DEPERSONALIZATION, qcril_mmgsdi_request_oem_hook_me_depersonalization ) },

#endif /* FEATURE_QCRIL_UIM_QMI */

  /**********************************************
   *                  SIM (GSTK)                *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/
#ifdef FEATURE_QCRIL_QMI_CAT
  /* QCRIL_EVT_GSTK_QMI_CAT_INDICATION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_CAT_INDICATION, qcril_gstk_qmi_process_qmi_indication ) },

  /* QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_NOTIFY_RIL_IS_READY, qcril_gstk_qmi_process_notify_ril_is_ready ) },

  /* QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_RAW_COMMAND_CALLBACK, qcril_gstk_qmi_process_raw_command_callback ) },

  /* QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_QMI_COMMAND_CALLBACK, qcril_gstk_qmi_process_qmi_response ) },

#else
  /* QCRIL_EVT_GSTK_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_COMMAND_CALLBACK, qcril_gstk_process_command_callback ) },

  /* QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_CLIENT_INIT_CALLBACK, qcril_gstk_process_client_init_callback ) },

  /* QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_CLIENT_REG_CALLBACK, qcril_gstk_process_client_reg_callback ) },

  /* QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_RAW_COMMAND_CALLBACK, qcril_gstk_process_raw_command_callback ) },

  /* QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_SEND_RAW_ENVELOPE_CALLBACK, qcril_gstk_process_send_raw_envelope_callback ) },

  /* QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_GSTK_NOTIFY_RIL_IS_READY, qcril_gstk_process_notify_ril_is_ready ) },

#endif /* FEATURE_QCRIL_QMI_CAT */


  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/
#ifdef FEATURE_QCRIL_QMI_CAT
  /* 67 - RIL_REQUEST_STK_GET_PROFILE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_GET_PROFILE, qcril_gstk_qmi_request_stk_get_profile ) },

  /* 68 - RIL_REQUEST_STK_SET_PROFILE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SET_PROFILE, qcril_gstk_qmi_request_stk_set_profile ) },

  /* 69 - RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, qcril_gstk_qmi_request_stk_send_envelope_command ) },

  /* 70 - RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, qcril_gstk_qmi_request_stk_send_terminal_response ) },

  /* 71 - RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim ) },

  /* 103 - RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, qcril_gstk_qmi_request_stk_service_is_running ) },

#else

  /* 67 - RIL_REQUEST_STK_GET_PROFILE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_GET_PROFILE, qcril_gstk_request_stk_get_profile ) },

  /* 68 - RIL_REQUEST_STK_SET_PROFILE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SET_PROFILE, qcril_gstk_request_stk_set_profile ) },

  /* 69 - RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, qcril_gstk_request_stk_send_envelope_command ) },

  /* 70 - RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, qcril_gstk_request_stk_send_terminal_response ) },

  /* 71 - RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, qcril_gstk_request_stk_handle_call_setup_requested_from_sim ) },

  /* 103 - RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING */
  { QCRIL_REG_ALL_STATES( RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING, qcril_gstk_request_stk_service_is_running ) },
#endif /* FEATURE_QCRIL_QMI_CAT */

  /**********************************************
   *                    SMS                     *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/

  /* QCRIL_EVT_SMS_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_COMMAND_CALLBACK, qcril_sms_command_event_callback ) },

  /* QCRIL_EVT_SMS_SUBSCRIPTION_PREF_INFO */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_SUBSCRIPTION_PREF_INFO, qcril_sms_subscription_pref_info ) },

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 25 - RIL_REQUEST_SEND_SMS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEND_SMS, qcril_sms_request_send_sms ) },

  /* 26 - RIL_REQUEST_SEND_SMS_EXPECT_MORE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEND_SMS_EXPECT_MORE, qcril_sms_request_send_sms_expect_more ) },

  /* 37 - RIL_REQUEST_SMS_ACKNOWLEDGE  */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SMS_ACKNOWLEDGE , qcril_sms_request_sms_acknowledge ) },

  /* 63 - RIL_REQUEST_WRITE_SMS_TO_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_WRITE_SMS_TO_SIM, qcril_sms_request_write_sms_to_sim ) },

  /* 64 - RIL_REQUEST_DELETE_SMS_ON_SIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DELETE_SMS_ON_SIM, qcril_sms_request_delete_sms_on_sim ) },

  /* 100 - RIL_REQUEST_GET_SMSC_ADDRESS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_SMSC_ADDRESS, qcril_sms_request_get_smsc_address ) },

  /* 101 - RIL_REQUEST_SET_SMSC_ADDRESS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_SMSC_ADDRESS, qcril_sms_request_set_smsc_address ) },

  /* 102 - RIL_REQUEST_REPORT_SMS_MEMORY_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, qcril_sms_request_report_sms_memory_status ) },

  /* 89 - RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG, qcril_sms_request_gsm_get_broadcast_sms_config ) },

  /* 90 - RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, qcril_sms_request_gsm_set_broadcast_sms_config ) },

  /* 91 - RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, qcril_sms_request_gsm_sms_broadcast_activation ) },

  /* 87 - RIL_REQUEST_CDMA_SEND_SMS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SEND_SMS, qcril_sms_request_cdma_send_sms ) },

  /* 88 - RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, qcril_sms_request_cdma_sms_acknowledge ) },

  /* 96 - RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, qcril_sms_request_cdma_write_sms_to_ruim ) },

  /* 97 - RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, qcril_sms_request_cdma_delete_sms_on_ruim ) },

  /* 92 - RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG, qcril_sms_request_cdma_get_broadcast_sms_config ) },

  /* 93 - RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, qcril_sms_request_cdma_set_broadcast_sms_config ) },

  /* 94 - RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, qcril_sms_request_cdma_sms_broadcast_activation ) },

  /* 108 - RIL_REQUEST_IMS_REGISTRATION_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_IMS_REGISTRATION_STATE, qcril_sms_request_ims_registration_state ) },

  /* 109 - RIL_REQUEST_IMS_SEND_SMS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_IMS_SEND_SMS, qcril_sms_request_ims_send_sms ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_SMS_CFG_MS_MEMORY_FULL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_CFG_MS_MEMORY_FULL, qcril_sms_cfg_event_memory_full ) },
  #else
  /* QCRIL_EVT_SMS_CFG_MEMORY_FULL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_CFG_MEMORY_FULL, qcril_sms_cfg_event_memory_full ) },
  #endif /* FEATURE_QCRIL_DSDS */

  /* QCRIL_EVT_SMS_SUBMIT_RPT */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_SUBMIT_RPT, qcril_sms_msg_event_submit_report ) },

  /* QCRIL_EVT_SMS_WRITE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_WRITE, qcril_sms_msg_event_write ) },

  /* QCRIL_EVT_SMS_DELETE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_DELETE, qcril_sms_msg_event_delete ) },

  /* QCRIL_EVT_SMS_RECEIVED_MESSAGE */
  { QCRIL_REG_ALL_STATES (QCRIL_EVT_SMS_RECEIVED_MESSAGE, qcril_sms_msg_event_received_message ) },

  /* QCRIL_EVT_SMS_STATUS_RPT */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_STATUS_RPT, qcril_sms_msg_event_status_report ) },

  /* QCRIL_EVT_SMS_SEND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_SEND, qcril_sms_msg_event_send ) },

  /* QCRIL_EVT_SMS_BC_MM_PREF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_BC_MM_PREF, qcril_sms_bc_mm_event_pref ) },

  /* QCRIL_EVT_SMS_BC_MM_ADD_SRVS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_BC_MM_ADD_SRVS, qcril_sms_bc_mm_event_add_services ) },

  /* QCRIL_EVT_SMS_BC_MM_TABLE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_BC_MM_TABLE, qcril_sms_bc_mm_event_table ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_CFG_MS_MEMORY_STATUS_SET, qcril_sms_cfg_event_mem_status_set ) },
  #else
  /* QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_CFG_MEMORY_STATUS_SET, qcril_sms_cfg_event_mem_status_set ) },
  #endif /* FEATURE_QCRIL_DSDS */

  /* QCRIL_EVT_SMS_CFG_MESSAGE_LIST */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_CFG_MESSAGE_LIST, qcril_sms_cfg_event_message_list ) },

  /* QCRIL_EVT_SMS_READ_TEMPLATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_READ_TEMPLATE, qcril_sms_msg_event_read_template ) },

  /* QCRIL_EVT_SMS_WRITE_TEMPLATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_WRITE_TEMPLATE, qcril_sms_msg_event_write_template ) },

  #ifdef FEATURE_QCRIL_IMS
  /* QCRIL_EVT_SMS_TRANSPORT_REG */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_TRANSPORT_REG, qcril_sms_msg_event_transport_reg ) },
  #endif /* FEATURE_QCRIL_IMS */

  #ifdef FEATURE_QCRIL_WMS_ETWS
  /* QCRIL_EVT_SMS_ETWS_NOTIFICATION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_SMS_ETWS_NOTIFICATION, qcril_sms_msg_event_etws_notification ) },
  #endif /* FEATURE_QCRIL_WMS_ETWS */
  /**********************************************
   *                     DATA                   *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/

  /* QCRIL_EVT_DATA_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES(  QCRIL_EVT_DATA_COMMAND_CALLBACK, qcril_data_command_hdlr ) },

  /* QCRIL_EVT_DATA_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_DATA_EVENT_CALLBACK, qcril_data_event_hdlr ) },

#ifdef FEATURE_QCRIL_USE_NETCTRL
  /* QCRIL_EVT_DATA_WDS_EVENT_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_DATA_WDS_EVENT_CALLBACK, qcril_data_wds_event_hdlr ) },
#endif /*FEATURE_QCRIL_USE_NETCTRL*/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/
  /* 27 - RIL_REQUEST_SETUP_DATA_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SETUP_DATA_CALL, qcril_data_request_setup_data_call ) },

  /* 41 - RIL_REQUEST_DEACTIVATE_DATA_CALL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DEACTIVATE_DATA_CALL, qcril_data_request_deactivate_data_call ) },

  /* The following event can come after switched to LPM. So honor it even Modem is in OFF state */
  /* 56 - RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, qcril_data_request_last_data_call_fail_cause ) },

  /* 57 - RIL_REQUEST_DATA_CALL_LIST */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DATA_CALL_LIST, qcril_data_request_data_call_list ) },

  /* 109 - RIL_REQUEST_GET_DATA_CALL_PROFILE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_DATA_CALL_PROFILE, qcril_data_request_omh_profile_info ) },

  /* - RIL_REQUEST_DATA_GO_DORMANT*/
  { QCRIL_REG_ALL_ACTIVE_STATES(QCRIL_EVT_HOOK_DATA_GO_DORMANT, qcril_data_process_qcrilhook_go_dormant)},

  /**********************************************
   *                    PBM                     *
   **********************************************/

  /* -------------------*
   *        EVENTS      *
   * -------------------*/
  /* QCRIL_EVT_PBM_REFRESH_START */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_REFRESH_START, qcril_pbm_event_handler ) },

  /* QCRIL_EVT_PBM_REFRESH_DONE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_REFRESH_DONE, qcril_pbm_event_handler ) },

  /* QCRIL_EVT_PBM_SIM_INIT_DONE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_SIM_INIT_DONE, qcril_pbm_event_handler ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_PBM_SESSION_INIT_DONE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_SESSION_INIT_DONE, qcril_pbm_event_handler ) },
  #endif /* FEATURE_QCRIL_DSDS */

  /* QCRIL_EVT_PBM_PB_READY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_PB_READY, qcril_pbm_event_handler ) },

  /* QCRIL_EVT_PBM_CARD_INSERTED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_CARD_INSERTED, qcril_pbm_event_card_state_changed ) },

  /* QCRIL_EVT_PBM_CARD_INIT_COMPLETED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_CARD_INIT_COMPLETED, qcril_pbm_event_card_state_changed ) },

  /* QCRIL_EVT_PBM_CARD_ERROR */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_CARD_ERROR, qcril_pbm_event_card_state_changed ) },

  /* QCRIL_EVT_INTERNAL_PBM_UPDATE_OTA_ECC_LIST */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_PBM_UPDATE_OTA_ECC_LIST, qcril_pbm_event_update_ota_ecc_list ) },

  /**********************************************
   *           CM                               *
   **********************************************/

  /* ---------------------------*
   *      COMMAND CALLBACKS     *
   * ---------------------------*/

  /* QCRIL_EVT_CM_COMMAND_CALLBACK */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_COMMAND_CALLBACK, qcril_cm_event_command_callback ) },


  /**********************************************
   *           CM - Phone Services              *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 23 - RIL_REQUEST_RADIO_POWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_RADIO_POWER, qcril_cm_phonesvc_request_radio_power ) },

  /* 45 - RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE, qcril_cm_phonesvc_request_query_network_selection_mode ) },

  /* 46 - RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC, qcril_cm_phonesvc_request_set_network_selection_automatic ) },

  /* 47 - RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, qcril_cm_phonesvc_request_set_network_selection_manual ) },

  /* 48 - RIL_REQUEST_QUERY_AVAILABLE_NETWORKS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_AVAILABLE_NETWORKS, qcril_cm_phonesvc_request_query_available_networks ) },

  /* 58 - RIL_REQUEST_RESET_RADIO - Deprecated per ril.h */

  /* 65 - RIL_REQUEST_SET_BAND_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_BAND_MODE, qcril_cm_phonesvc_request_set_band_mode ) },

  /* 66 - RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE, qcril_cm_phonesvc_request_query_available_band_mode ) },

  /* 73 - RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, qcril_cm_phonesvc_request_set_preferred_network_type ) },

  /* 74 - RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE, qcril_cm_phonesvc_request_get_preferred_network_type ) },

  /* 77 - RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, qcril_cm_phonesvc_request_cdma_set_subscription_source ) },

  /* 106 - RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE, qcril_cm_phonesvc_request_cdma_get_subscription_source ) },

  /* 78 - RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, qcril_cm_phonesvc_request_cdma_set_roaming_preference ) },

  /* 79 - RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE, qcril_cm_phonesvc_request_cdma_query_roaming_preference ) },

  /* 103 - RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE, qcril_cm_phonesvc_request_exit_emergency_callback_mode ) },

  /* 95 - RIL_REQUEST_CDMA_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SUBSCRIPTION, qcril_cm_phonesvc_request_cdma_subscription ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* 109 - RIL_REQUEST_SET_UICC_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_UICC_SUBSCRIPTION, qcril_cm_phonesvc_request_set_uicc_subscription ) },

  /* 110 - RIL_REQUEST_SET_DATA_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_DATA_SUBSCRIPTION, qcril_cm_phonesvc_request_set_data_subscription ) },

  /* 111 - RIL_REQUEST_GET_UICC_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_UICC_SUBSCRIPTION, qcril_cm_phonesvc_request_get_uicc_subscription ) },

  /* 112 - RIL_REQUEST_GET_DATA_SUBSCRIPTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_DATA_SUBSCRIPTION, qcril_cm_phonesvc_request_get_data_subscription ) },

  /* 113 - RIL_REQUEST_SET_SUBSCRIPTION_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_SUBSCRIPTION_MODE, qcril_cm_phonesvc_request_set_subscription_mode ) },
  #endif /* FEATURE_QCRIL_DSDS */

  #ifdef FEATURE_QCRIL_SAR
  /* 117 - RIL_REQUEST_SET_TRANSMIT_POWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_TRANSMIT_POWER, qcril_sar_request_set_transmit_power ) },
  #endif /* FEATURE_QCRIL_SAR */

  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  /* QCRIL_EVT_CM_UPDATE_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_UPDATE_FDN_STATUS, qcril_cm_event_update_fdn_status ) },

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  /* QCRIL_EVT_CM_ENABLE_SUBSCRIPTION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_ENABLE_SUBSCRIPTION, qcril_cm_event_enable_subscription ) },

  /* QCRIL_EVT_CM_DISABLE_SUBSCRIPTION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_DISABLE_SUBSCRIPTION, qcril_cm_event_disable_subscription ) },
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  /* QCRIL_EVT_CM_CARD_STATUS_UPDATED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CARD_STATUS_UPDATED, qcril_cm_event_card_status_updated ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_ACTIVATE_PROVISION_STATUS, qcril_cm_event_activate_provision_status ) },

  /* QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_DEACTIVATE_PROVISION_STATUS, qcril_cm_event_deactivate_provision_status ) },
  #endif /* FEATURE_QCRIL_DSDS */

  /* QCRIL_EVT_CM_UPDATE_FDN_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_UPDATE_FDN_STATUS, qcril_cm_event_update_fdn_status ) },

  /* QCRIL_EVT_CM_PH_OPRT_MODE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_OPRT_MODE, qcril_cm_phonesvc_event_oprt_mode ) },

  /* QCRIL_EVT_CM_PH_INFO */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_INFO, qcril_cm_phonesvc_event_info ) },

  /* QCRIL_EVT_CM_PH_SYS_SEL_PREF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_SYS_SEL_PREF, qcril_cm_phonesvc_event_sys_sel_pref ) },

  #ifdef FEATURE_QCRIL_DSDS
  /* QCRIL_EVT_CM_PH_DUAL_STANDBY_PREF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_DUAL_STANDBY_PREF, qcril_cm_phonesvc_event_dual_standby_pref ) },

  /* QCRIL_EVT_CM_PH_SUBSCRIPTION_PREF_INFO */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_SUBSCRIPTION_PREF_INFO, qcril_cm_phonesvc_event_subscription_pref_info ) },
  #endif /* FEATURE_QCRIL_DSDS */

  #ifdef FEATURE_QCRIL_PRL_INIT
  /* QCRIL_EVT_CM_PH_PRL_INIT */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_PRL_INIT, qcril_cm_phonesvc_event_prl_init ) },
  #endif /* FEATURE_QCRIL_PRL_INIT */

  /* QCRIL_EVT_CM_PH_SUBSCRIPTION_AVAILABLE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_SUBSCRIPTION_AVAILABLE, qcril_cm_phonesvc_event_subscription_available ) },

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  /* QCRIL_EVT_CM_PH_SUBSCRIPTION_NOT_AVAILABLE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_SUBSCRIPTION_NOT_AVAILABLE, qcril_cm_phonesvc_event_subscription_not_available ) },
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  /* QCRIL_EVT_CM_PH_AVAILABLE_NETWORKS_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_AVAILABLE_NETWORKS_CONF, qcril_cm_phonesvc_event_available_networks_conf ) },

  /* QCRIL_EVT_CM_PH_TERMINATE_GET_NETWORKS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_TERMINATE_GET_NETWORKS, qcril_cm_phonesvc_event_terminate_get_networks ) },

  /* QCRIL_EVT_CM_PH_NVRUIM_CONFIG_CHANGED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_PH_NVRUIM_CONFIG_CHANGED, qcril_cm_phonesvc_event_nvruim_config_changed ) },


  /**********************************************
   *       CM - Serving System Management       *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 104 - RIL_REQUEST_VOICE_RADIO_TECH */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_VOICE_RADIO_TECH, qcril_cm_srvsys_request_voice_radio_tech ) },

  /* 19 - RIL_REQUEST_SIGNAL_STRENGTH */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SIGNAL_STRENGTH, qcril_cm_srvsys_request_signal_strength ) },

  /* 20 - RIL_REQUEST_REGISTRATION_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_REGISTRATION_STATE, qcril_cm_srvsys_request_registration_state ) },

  /* 21 - RIL_REQUEST_DATA_REGISTRATION_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DATA_REGISTRATION_STATE, qcril_cm_srvsys_request_data_registration_state ) },

  /* 22 - RIL_REQUEST_OPERATOR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_OPERATOR, qcril_cm_srvsys_request_operator ) },

  /* 61 - RIL_REQUEST_SCREEN_STATE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SCREEN_STATE, qcril_cm_srvsys_request_screen_state ) },

  /* 75 - RIL_REQUEST_GET_NEIGHBORING_CELL_IDS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_NEIGHBORING_CELL_IDS, qcril_cm_srvsys_request_get_neighboring_cell_ids ) },

  /* 76 - RIL_REQUEST_SET_LOCATION_UPDATES */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_LOCATION_UPDATES, qcril_cm_srvsys_request_set_location_updates ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  /* QCRIL_EVT_CM_SS_SRV_CHANGED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_SRV_CHANGED, qcril_cm_srvsys_event_srv_changed ) },

  /* QCRIL_EVT_CM_SS_RSSI */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_RSSI, qcril_cm_srvsys_event_rssi ) },

  /* QCRIL_EVT_CM_SS_INFO */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_INFO, qcril_cm_srvsys_event_info ) },

  /* QCRIL_EVT_CM_SS_REG_REJECT */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_REG_REJECT, qcril_cm_srvsys_event_reg_reject ) },

  /* QCRIL_EVT_CM_SS_HDR_RSSI */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_HDR_RSSI, qcril_cm_srvsys_event_rssi ) },

  /* QCRIL_EVT_CM_SS_EMERG_NUM_LIST */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_EMERG_NUM_LIST, qcril_cm_srvsys_event_emerg_num_list ) },

  #ifdef FEATURE_QCRIL_DSAC
  /* QCRIL_EVT_CM_SS_CELL_ACCESS_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SS_CELL_ACCESS_IND, qcril_cm_srvsys_event_cell_access_ind ) },
  #endif /* FEATURE_QCRIL_DSAC */

  /**********************************************
   *             CM - Call Services             *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 9 - RIL_REQUEST_GET_CURRENT_CALLS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_CURRENT_CALLS, qcril_cm_callsvc_request_get_current_calls ) },

  /* 10 - RIL_REQUEST_DIAL */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DIAL, qcril_cm_callsvc_request_dial ) },

  /* 12 - RIL_REQUEST_HANGUP */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_HANGUP, qcril_cm_callsvc_request_hangup ) },

  /* 18 - RIL_REQUEST_LAST_CALL_FAIL_CAUSE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_LAST_CALL_FAIL_CAUSE, qcril_cm_callsvc_request_last_call_fail_cause ) },

  /* 40 - RIL_REQUEST_ANSWER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_ANSWER, qcril_cm_callsvc_request_answer ) },

  /* 80 - RIL_REQUEST_SET_TTY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_TTY_MODE, qcril_cm_callsvc_request_set_tty_mode ) },

  /* 81 - RIL_REQUEST_QUERY_TTY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_TTY_MODE, qcril_cm_callsvc_request_query_tty_mode ) },

  /* 82 - RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, qcril_cm_callsvc_request_cdma_set_preferred_voice_privacy_mode ) },

  /* 83 - RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE, qcril_cm_callsvc_request_cdma_query_preferred_voice_privacy_mode ) },

  /* 84 - RIL_REQUEST_CDMA_FLASH */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_FLASH, qcril_cm_callsvc_request_cdma_flash ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  /* QCRIL_EVT_CM_CALL_ORIG */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_ORIG, qcril_cm_callsvc_event_orig ) },

  /* QCRIL_EVT_CM_CALL_SETUP_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_SETUP_IND, qcril_cm_callsvc_event_setup_ind ) },

  /* QCRIL_EVT_CM_CALL_USER_DATA_IND  */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_USER_DATA_IND, qcril_cm_callsvc_event_user_data_ind ) },

  /* QCRIL_EVT_CM_CALL_PROGRESS_INFO_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_PROGRESS_INFO_IND, qcril_cm_callsvc_event_progress_info_ind ) },

  /* QCRIL_EVT_CM_CALL_INCOM */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_INCOM, qcril_cm_callsvc_event_incom ) },

  /* QCRIL_EVT_CM_CALL_CONNECT */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_CONNECT, qcril_cm_callsvc_event_connect ) },

  /* QCRIL_EVT_CM_CALL_ANSWER */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_ANSWER, qcril_cm_callsvc_event_answer ) },

  /* QCRIL_EVT_CM_CALL_END */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_END, qcril_cm_callsvc_event_end ) },

  /* QCRIL_EVT_CM_CALL_MNG_CALLS_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_MNG_CALLS_CONF, qcril_cm_callsvc_event_mng_calls_conf ) },

  /* QCRIL_EVT_CM_CALL_ORIG_FWD_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_ORIG_FWD_STATUS, qcril_cm_callsvc_event_orig_fwd_status ) },

  /* QCRIL_EVT_CM_CALL_BEING_FORWARDED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_BEING_FORWARDED, qcril_cm_callsvc_event_call_being_forwarded ) },

  /* QCRIL_EVT_CM_CALL_IS_WAITING */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_IS_WAITING, qcril_cm_callsvc_event_call_is_waiting ) },

  /* QCRIL_EVT_CM_CALL_BARRED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_BARRED, qcril_cm_callsvc_event_call_barred ) },

  /* QCRIL_EVT_CM_CALL_RESTRICTED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_RESTRICTED, qcril_cm_callsvc_event_call_restricted ) },

  /* QCRIL_EVT_CM_CALL_INCOM_FWD_CALL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_INCOM_FWD_CALL, qcril_cm_callsvc_event_incom_fwd_call ) },

  /* QCRIL_EVT_CM_CALL_CUG_INFO_RECEIVED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_CUG_INFO_RECEIVED, qcril_cm_callsvc_event_cug_info_received ) },

  /* QCRIL_EVT_CM_CALL_ON_HOLD  */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_ON_HOLD, qcril_cm_callsvc_event_call_on_hold ) },

  /* QCRIL_EVT_CM_CALL_RETRIEVED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_RETRIEVED, qcril_cm_callsvc_event_call_retrieved ) },

  /* QCRIL_EVT_CM_CALL_FORWARDED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_FORWARDED, qcril_cm_callsvc_event_call_forwarded ) },

  /* QCRIL_EVT_CM_CALL_TRANSFERRED_CALL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_TRANSFERRED_CALL, qcril_cm_callsvc_event_transferred_call ) },

  /* QCRIL_EVT_CM_CALL_CALL_DEFLECTION */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_DEFLECTION, qcril_cm_callsvc_event_call_deflection ) },

  /* QCRIL_EVT_CM_CALL_CNAP_INFO_RECEIVED */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_CNAP_INFO_RECEIVED, qcril_cm_callsvc_event_cnap_info_received ) },

  /*  QCRIL_EVT_CM_CALL_OTASP_STATUS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_OTASP_STATUS, qcril_cm_callsvc_event_otasp_status ) },

  /* QCRIL_EVT_CM_CALL_SUPS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_SUPS, qcril_cm_callsvc_event_sups ) },

  /* QCRIL_EVT_CM_CALL_PRIVACY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_PRIVACY, qcril_cm_callsvc_event_privacy ) },

  /* QCRIL_EVT_CM_CALL_PRIVACY_PREF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_PRIVACY_PREF, qcril_cm_callsvc_event_privacy_pref ) },

  /* QCRIL_EVT_CM_CALL_CALLER_ID */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_CALLER_ID, qcril_cm_callsvc_event_caller_id )},

  /* QCRIL_EVT_CM_CALL_SIGNAL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_SIGNAL, qcril_cm_callsvc_event_signal )},

  /* QCRIL_EVT_CM_CALL_DISPLAY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_DISPLAY, qcril_cm_callsvc_event_display )},

  /* QCRIL_EVT_CM_CALL_CALLED_PARTY */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_CALLED_PARTY, qcril_cm_callsvc_event_called_party )},

  /* QCRIL_EVT_CM_CALL_CONNECTED_NUM */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_CONNECTED_NUM, qcril_cm_callsvc_event_connected_num )},

  /* QCRIL_EVT_CM_CALL_EXT_DISP */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_EXT_DISP, qcril_cm_callsvc_event_ext_display )},

  /* QCRIL_EVT_CM_CALL_EXT_BRST_INTL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_EXT_BRST_INTL, qcril_cm_callsvc_event_ext_brst_intl )},

  /* QCRIL_EVT_CM_CALL_NSS_CLIR_REC */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_NSS_CLIR_REC, qcril_cm_callsvc_event_nss_clir )},

  /* QCRIL_EVT_CM_CALL_NSS_REL_REC */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_NSS_REL_REC, qcril_cm_callsvc_event_nss_rel )},

  /* QCRIL_EVT_CM_CALL_NSS_AUD_CTRL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_NSS_AUD_CTRL, qcril_cm_callsvc_event_nss_aud_ctrl )},

  /* QCRIL_EVT_CM_CALL_REDIRECTING_NUMBER */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_REDIRECTING_NUMBER, qcril_cm_callsvc_event_redirecting_number )},

  /* QCRIL_EVT_CM_CALL_LINE_CTRL */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_CALL_LINE_CTRL, qcril_cm_callsvc_event_line_ctrl )},

  /**********************************************
   *           CM - In Band Services            *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 24 - RIL_REQUEST_DTMF */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DTMF, qcril_cm_inbandsvc_request_dtmf ) },

  /* 49 - RIL_REQUEST_DTMF_START */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DTMF_START, qcril_cm_inbandsvc_request_dtmf_start ) },

  /* 50 - RIL_REQUEST_DTMF_STOP */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DTMF_STOP, qcril_cm_inbandsvc_request_dtmf_stop ) },

  /* 85 - RIL_REQUEST_CDMA_BURST_DTMF */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_BURST_DTMF, qcril_cm_inbandsvc_request_cdma_burst_dtmf ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/

  /* QCRIL_EVT_CM_INBAND_REV_BURST_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_REV_BURST_DTMF, qcril_cm_inbandsvc_event_rev_burst_dtmf ) },

  /* QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF, qcril_cm_inbandsvc_event_rev_start_cont_dtmf ) },

  /* QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_REV_START_CONT_DTMF_CONF, qcril_cm_inbandsvc_event_rev_start_cont_dtmf_conf ) },

  /* QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF, qcril_cm_inbandsvc_event_rev_stop_cont_dtmf ) },

  /* QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_REV_STOP_CONT_DTMF_CONF, qcril_cm_inbandsvc_event_rev_stop_cont_dtmf_conf ) },

  /* QCRIL_EVT_CM_INBAND_FWD_BURST_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_FWD_BURST_DTMF, qcril_cm_inbandsvc_event_fwd_burst_dtmf ) },

  /* QCRIL_EVT_CM_INBAND_FWD_START_CONT_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_FWD_START_CONT_DTMF, qcril_cm_inbandsvc_event_fwd_start_cont_dtmf ) },

  /* QCRIL_EVT_CM_INBAND_FWD_STOP_CONT_DTMF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_INBAND_FWD_STOP_CONT_DTMF, qcril_cm_inbandsvc_event_fwd_stop_cont_dtmf ) },

  /**********************************************
   *          CM - Supplemental Services        *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 13 - RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, qcril_cm_supsvc_request_hangup_waiting_or_background ) },

  /* 14 - RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, qcril_cm_supsvc_request_hangup_foreground_resume_background ) },

  /* 15 - RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, qcril_cm_supsvc_request_switch_waiting_or_holding_and_active ) },

  /* 16 - RIL_REQUEST_CONFERENCE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CONFERENCE, qcril_cm_supsvc_request_conference ) },

  /* 17 - RIL_REQUEST_UDUB */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_UDUB, qcril_cm_supsvc_request_udub ) },

  /* 29 - RIL_REQUEST_SEND_USSD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEND_USSD, qcril_cm_supsvc_request_send_ussd ) },

  /* 30 - RIL_REQUEST_CANCEL_USSD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CANCEL_USSD, qcril_cm_supsvc_request_cancel_ussd ) },

  /* 31 - RIL_REQUEST_GET_CLIR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_CLIR, qcril_cm_supsvc_request_get_clir ) },

  /* 32 - RIL_REQUEST_SET_CLIR */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_CLIR, qcril_cm_supsvc_request_set_clir ) },

  /* 33 - RIL_REQUEST_QUERY_CALL_FORWARD_STATUS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, qcril_cm_supsvc_request_query_call_forward_status ) },

  /* 34 - RIL_REQUEST_SET_CALL_FORWARD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_CALL_FORWARD, qcril_cm_supsvc_request_set_call_forward ) },

  /* 35 - RIL_REQUEST_QUERY_CALL_WAITING */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_CALL_WAITING, qcril_cm_supsvc_request_query_call_waiting ) },

  /* 36 - RIL_REQUEST_SET_CALL_WAITING */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_CALL_WAITING, qcril_cm_supsvc_request_set_call_waiting ) },

  /* 42 - RIL_REQUEST_QUERY_FACILITY_LOCK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_FACILITY_LOCK, qcril_cm_supsvc_request_query_facility_lock ) },

  /* 43 - RIL_REQUEST_SET_FACILITY_LOCK */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_FACILITY_LOCK, qcril_cm_supsvc_request_set_facility_lock ) },

  /* 44 - RIL_REQUEST_CHANGE_BARRING_PASSWORD */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CHANGE_BARRING_PASSWORD, qcril_cm_supsvc_request_change_barring_password ) },

  /* 52 - RIL_REQUEST_SEPARATE_CONNECTION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SEPARATE_CONNECTION, qcril_cm_supsvc_request_separate_connection ) },

  /* 55 - RIL_REQUEST_QUERY_CLIP */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_QUERY_CLIP, qcril_cm_supsvc_request_query_clip ) },

  /* 62 - RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION, qcril_cm_supsvc_request_set_supp_svc_notification ) },

  /* 72 - RIL_REQUEST_EXPLICIT_CALL_TRANSFER */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_EXPLICIT_CALL_TRANSFER, qcril_cm_supsvc_request_explicit_call_transfer ) },

  /* -------------------*
   *        EVENTS      *
   * -------------------*/
  /* QCRIL_EVT_CM_SUPS_ACTIVATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_ACTIVATE, qcril_cm_supsvc_event_ack ) },

  /* QCRIL_EVT_CM_SUPS_ACTIVATE_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_ACTIVATE_CONF, qcril_cm_supsvc_event_activate_conf ) },

  /* QCRIL_EVT_CM_SUPS_DEACTIVATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_DEACTIVATE, qcril_cm_supsvc_event_ack ) },

  /* QCRIL_EVT_CM_SUPS_DEACTIVATE_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_DEACTIVATE_CONF, qcril_cm_supsvc_event_deactivate_conf ) },

  /* QCRIL_EVT_CM_SUPS_ERASE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_ERASE, qcril_cm_supsvc_event_ack ) },

  /* QCRIL_EVT_CM_SUPS_ERASE_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_ERASE_CONF, qcril_cm_supsvc_event_erase_conf ) },

  /* QCRIL_EVT_CM_SUPS_INTERROGATE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_INTERROGATE, qcril_cm_supsvc_event_ack ) },

  /* QCRIL_EVT_CM_SUPS_INTERROGATE_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_INTERROGATE_CONF, qcril_cm_supsvc_event_interrogate_conf ) },

  /* QCRIL_EVT_CM_SUPS_REGISTER */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_REGISTER, qcril_cm_supsvc_event_ack ) },

  /* QCRIL_EVT_CM_SUPS_REGISTER_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_REGISTER_CONF, qcril_cm_supsvc_event_register_conf ) },

  /* QCRIL_EVT_CM_SUPS_REG_PASSWORD */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_REG_PASSWORD, qcril_cm_supsvc_event_ack ) },

  /* QCRIL_EVT_CM_SUPS_REG_PASSWORD_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_REG_PASSWORD_CONF, qcril_cm_supsvc_event_reg_password_conf ) },

  /* QCRIL_EVT_CM_SUPS_PROCESS_USS */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_PROCESS_USS, qcril_cm_supsvc_event_process_uss ) },

  /* QCRIL_EVT_CM_SUPS_PROCESS_USS_CONF */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_PROCESS_USS_CONF, qcril_cm_supsvc_event_process_uss_conf ) },

  /* QCRIL_EVT_CM_SUPS_EVENT_FWD_CHECK_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_EVENT_FWD_CHECK_IND, qcril_cm_supsvc_event_fwd_check_ind ) },

  /* QCRIL_EVT_CM_SUPS_USS_NOTIFY_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_USS_NOTIFY_IND, qcril_cm_supsvc_event_uss_notify_ind ) },

  /* QCRIL_EVT_CM_SUPS_USS_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_USS_IND, qcril_cm_supsvc_event_uss_ind ) },

  /* QCRIL_EVT_CM_SUPS_RELEASE_USS_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_RELEASE_USS_IND, qcril_cm_supsvc_event_release_uss_ind ) },

  /* QCRIL_EVT_CM_SUPS_GET_PASSWORD_IND */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_SUPS_GET_PASSWORD_IND, qcril_cm_supsvc_event_get_password_ind ) },


  /**********************************************
   *          CM - Stats Services               *
   **********************************************/

  #ifdef FEATURE_QCRIL_NCELL
  /* QCRIL_EVT_CM_STATS_MODEM_INFO */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_CM_STATS_MODEM_INFO, qcril_cm_stats_event_modem_info ) },
  #endif /* FEATURE_QCRIL_NCELL */


   /**********************************************
   *                  Other                     *
   **********************************************/

  /* -------------------*
   *     RIL_REQUESTS   *
   * -------------------*/

  /* 38 - RIL_REQUEST_GET_IMEI */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_IMEI, qcril_other_request_get_imei ) },

  /* 39 - RIL_REQUEST_GET_IMEISV */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_IMEISV, qcril_other_request_get_imeisv ) },

  /* 51 - RIL_REQUEST_BASEBAND_VERSION */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_BASEBAND_VERSION, qcril_other_request_baseband_version ) },

  /* 53 - RIL_REQUEST_SET_MUTE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_SET_MUTE, qcril_other_request_set_mute ) },

  /* 54 - RIL_REQUEST_GET_MUTE */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_GET_MUTE, qcril_other_request_get_mute ) },

  /* 59 - RIL_REQUEST_OEM_HOOK_RAW */
  /* RIL_REQUEST_OEM_HOOK_RAW is handled explicity in onRequest() and  appropriately dispatched */

  /* 60 - RIL_REQUEST_OEM_HOOK_STRINGS */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_OEM_HOOK_STRINGS, qcril_other_request_oem_hook_strings ) },

  /* 0x80001 - QCRIL_EVT_HOOK_NV_READ */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_HOOK_NV_READ, qcril_other_request_oem_hook_nv_read ) },

  /* 0x80002 - QCRIL_EVT_HOOK_NV_WRITE */
  { QCRIL_REG_ALL_STATES( QCRIL_EVT_HOOK_NV_WRITE, qcril_other_request_oem_hook_nv_write ) },

#ifdef FEATURE_QCRIL_DSDS
  /* 0x80005 - QCRIL_EVT_HOOK_SET_TUNE_AWAY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_TUNE_AWAY, qcril_cm_phonesvc_request_set_tune_away ) },

  /* 0x80006 - QCRIL_EVT_HOOK_GET_TUNE_AWAY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_TUNE_AWAY, qcril_cm_phonesvc_request_get_tune_away ) },

  /* 0x80007 - QCRIL_EVT_HOOK_SET_PAGING_PRIORITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_SET_PAGING_PRIORITY, qcril_cm_phonesvc_request_set_paging_priority ) },

  /* 0x80008 - QCRIL_EVT_HOOK_GET_PAGING_PRIORITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( QCRIL_EVT_HOOK_GET_PAGING_PRIORITY, qcril_cm_phonesvc_request_get_paging_priority ) },

#endif

  /* 76 - RIL_REQUEST_DEVICE_IDENTITY */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_DEVICE_IDENTITY, qcril_other_request_device_identity ) },

  /* 86 - RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY */
  { QCRIL_REG_ALL_ACTIVE_STATES( RIL_REQUEST_CDMA_VALIDATE_AND_WRITE_AKEY, qcril_other_request_cdma_validate_and_write_akey ) },

  /* -------------------*
   *      CALLBACKS     *
   * -------------------*/

  /* -------------------*
   *     UNSOLICITED    *
   * -------------------*/
};

/* Stays in GW radio technology if fullfill one of the following conditions
  (1) Modem supporte GW mode only or
  (2) Modem supports GW mode and
      (a) GW is the only preferred mode or
      (b) GW is acquired or
      (c) Currently has no service and GW is the last acquired system
*/
#define QCRIL_STAYS_IN_GW_RADIO_TECH( mode_config_mask, mode_pref, acquired_radio_tech, prev_acquired_radio_tech )       \
  ( ( mode_config_mask == QCRIL_MODE_GW_CONFIG_MASK ) ||                                                                 \
    ( ( mode_config_mask & QCRIL_MODE_GW_CONFIG_MASK ) &&                                                                \
      ( ( mode_pref == QCRIL_NETWORK_MODE_PREF_GW ) || ( acquired_radio_tech == QCRIL_ACQUIRED_GW ) ||                   \
        ( ( acquired_radio_tech == QCRIL_ACQUIRED_NONE ) && ( prev_acquired_radio_tech == QCRIL_ACQUIRED_GW ) ) ) ) )

/* Stays in 1XEVDO radio technology if fullfill one of the follwoing conditions
  (1) Modem supports 1XEVDO mode only or
  (2) Modem supports 1XEVDO mode and
      (a) 1XEVDO is the only preferred mode or
      (b) 1XEVDO is acquired or
      (c) Currently has no service and GW is not the last acquired system
*/
#define QCRIL_STAYS_IN_1XEVDO_RADIO_TECH( mode_config_mask, mode_pref, acquired_radio_tech, prev_acquired_radio_tech )   \
  ( ( mode_config_mask == QCRIL_MODE_1XEVDO_CONFIG_MASK ) ||                                                             \
    ( ( mode_config_mask & QCRIL_MODE_1XEVDO_CONFIG_MASK ) &&                                                            \
      ( ( mode_pref == QCRIL_NETWORK_MODE_PREF_1XEVDO ) || ( acquired_radio_tech == QCRIL_ACQUIRED_1XEVDO ) ||           \
        ( ( acquired_radio_tech == QCRIL_ACQUIRED_NONE ) && ( prev_acquired_radio_tech != QCRIL_ACQUIRED_GW ) ) ) ) )


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* QCRIL internal info */
static qcril_arb_state_struct_type *qcril_state;

/* QCRIL timer id */
pthread_mutex_t qcril_timer_id_mutex; /*!< Mutex to control access/update of QCRIL Timer ID*/
static uint16 qcril_timer_id;         /*!< Next QCRIL Timer ID */

/* QCRIL timer list */
pthread_mutex_t qcril_timed_callback_list_mutex;
static qcril_timed_callback_info *qcril_timed_callback_list;

/*! Hash table for dispatching inputs to QCRIL handlers */
static qcril_dispatch_table_entry_type *qcril_hash_table[ QCRIL_HT_ENTRIES_MAX ];

/* Radio state name */
#ifndef FEATURE_ICS
static char *radio_state_name[] = { "Radio Off", "Radio Unavailable", "Radio On" };
#else
static char *radio_state_name[] = { "Radio Off", "Radio Unavailable","","","","","","","","", "Radio On" };
#endif

/* Time (1 second) to wait for the completion of modem restart before re-initiate QCRIL */
static const struct timeval TIMEVAL_DELAY = {1,0};

#ifdef FEATURE_QC_TIME_SERVICES
#define TIME_SERVICES_PROPERTY "persist.timed.enable"
static time_bases_type qcril2ats[] =
{
  ATS_WCDMA,
  ATS_1X,
  ATS_USER,
};
#endif


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES


===========================================================================*/

static void onRequest_rid0( int request, void *data, size_t datalen, RIL_Token t );
static RIL_RadioState currentState_rid0();
static int onSupports_rid0( int request );
static void onCancel_rid0( RIL_Token t );
static const char *getVersion_rid0( void );

#ifdef FEATURE_QCRIL_DSDS
static void onRequest_rid1( int request, void *data, size_t datalen, RIL_Token t );
static RIL_RadioState currentState_rid1();
static int onSupports_rid1( int request );
static void onCancel_rid1( RIL_Token t );
static const char *getVersion_rid1( void );
#endif /* FEATURE_QCRIL_DSDS */

static const RIL_RadioFunctions qcril_request_api[ QCRIL_MAX_INSTANCE_ID ] = {
  { RIL_VERSION, onRequest_rid0, currentState_rid0, onSupports_rid0, onCancel_rid0, getVersion_rid0 }
  #ifdef FEATURE_QCRIL_DSDS
  , { RIL_VERSION, onRequest_rid1, currentState_rid1, onSupports_rid1, onCancel_rid1, getVersion_rid1 }
  #endif /* FEATURE_QCRIL_DSDS */
};

#ifdef RIL_SHLIB
struct RIL_Env *qcril_response_api[ QCRIL_MAX_INSTANCE_ID ]; /*!< Functions for ril to call */
#endif /* RIL_SHLIB */


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

qcril_timed_callback_info **_qcril_find_timed_callback_locked(uint32 timer_id)
{
  qcril_timed_callback_info **i, *ret;

  for (i = &qcril_timed_callback_list;
        *i ; i = &((*i)->next)) {
    if ((*i)->timer_id == timer_id) {
      break;
    }
  }

  return i;

}
qcril_timed_callback_info *qcril_find_and_remove_timed_callback(uint32 timer_id)
{
  qcril_timed_callback_info **i, *ret;
  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");

  i = _qcril_find_timed_callback_locked(timer_id);

  ret = *i;
  if (ret) {
    *i = ret->next;
    ret->next = NULL;
  }

  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");
  return ret;

}

void qcril_add_timed_callback(qcril_timed_callback_info *info)
{
  qcril_timed_callback_info **i;
  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");

  for (i = &qcril_timed_callback_list; *i; i = &((*i)->next))
      /*EMPTY*/;
  *i = info;
  info->next = NULL;
  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex");
}

/*=========================================================================
  FUNCTION:  qcril_timed_callback_dispatch

===========================================================================*/
/*!
    @brief
    Dispatch function for all timed callbacks

    @return
    void
*/
/*=========================================================================*/
void qcril_timed_callback_dispatch
(
  void *param
)
{
  uint32 timer_id = (uint32) param;
  qcril_timed_callback_info *info = qcril_find_and_remove_timed_callback(timer_id);

  if (info) {
    info->callback((void *)timer_id);
    qcril_free(info);
  }
} /* qcril_timed_callback_dispatch */

/*=========================================================================
  FUNCTION:  qcril_setup_timed_callback

===========================================================================*/
/*!
    @brief
    Setup RIL callback timer

    @return
    0 on success.
*/
/*=========================================================================*/
int qcril_setup_timed_callback
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  RIL_TimedCallback callback,
  const struct timeval *relativeTime,
  uint32 *timer_id
)
{
  qcril_timed_callback_info *tcbinfo;
  int ret = -1;
  uint32 the_timer_id;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  tcbinfo = qcril_malloc(sizeof(qcril_timed_callback_info));
  if (tcbinfo) {

    QCRIL_MUTEX_LOCK( &qcril_timer_id_mutex, "qcril_timer_id_mutex" );

    /* Most Significant 16 bits are the Instance ID + Modem ID and Least Significant 16 bits are the QCRIL Timer ID */
    the_timer_id = ( uint32 ) QCRIL_COMPOSE_USER_DATA( instance_id, modem_id, qcril_timer_id );
    qcril_timer_id++;

    QCRIL_MUTEX_UNLOCK( &qcril_timer_id_mutex, "qcril_timer_id_mutex" );

    tcbinfo->timer_id = the_timer_id;
    tcbinfo->callback = callback;

    qcril_add_timed_callback(tcbinfo);

    qcril_response_api[ instance_id ]->RequestTimedCallback( qcril_timed_callback_dispatch,
                                                             (void *)the_timer_id, relativeTime );

    QCRIL_LOG_DEBUG( "Set timer with ID %d", the_timer_id );

    if (timer_id) {
      *timer_id = the_timer_id;
    }
    ret = 0;
  }

  return ret;
} /* qcril_setup_timed_callback */


/*=========================================================================
  FUNCTION:  qcril_cancel_timed_callback

===========================================================================*/
/*!
    @brief
    Cancel RIL callback timer

    @return
    0 on success.
*/
/*=========================================================================*/
int qcril_cancel_timed_callback
(
  void *param
)
{
  uint32 timer_id = (uint32) param;
  qcril_timed_callback_info *info = qcril_find_and_remove_timed_callback(timer_id);
  int ret = -1;
  /*-----------------------------------------------------------------------*/

  if (info) {
    ret = 0;
    QCRIL_LOG_DEBUG( "Cancel timer with ID %d", info->timer_id );
    qcril_free(info);
  }

  return ret;
} /* qcril_cancel_timed_callback */


/*=========================================================================
  FUNCTION:  qcril_timed_callback_active

===========================================================================*/
/*!
    @brief
    Query state of the timed callback

    @return
    0 if timer is inactive. Non-zero Otherwise
*/
/*=========================================================================*/
int qcril_timed_callback_active
(
  uint32 timer_id
)
{
  /*-----------------------------------------------------------------------*/
  qcril_timed_callback_info **info = NULL;

  QCRIL_ASSERT( info );

  QCRIL_MUTEX_LOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex" );

  info = _qcril_find_timed_callback_locked(timer_id);

  QCRIL_MUTEX_UNLOCK( &qcril_timed_callback_list_mutex, "timed callback list mutex" );

  QCRIL_ASSERT( info != NULL );

  return *info !=NULL;
} /* qcril_timed_callback_active */


/*=========================================================================
  FUNCTION:  qcril_default_request_resp_params

===========================================================================*/
/*!
    @brief
    Set default values for parameters in RIL request's response

    @return
    None
*/
/*=========================================================================*/
void qcril_default_request_resp_params
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request_id,
  RIL_Errno ril_err_no,
  qcril_request_resp_params_type *param_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( param_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  param_ptr->instance_id = instance_id;
  param_ptr->t = t;
  param_ptr->request_id = request_id;
  param_ptr->ril_err_no = ril_err_no;
  param_ptr->resp_pkt = NULL;
  param_ptr->resp_len = 0;
  param_ptr->logstr = NULL;

} /* qcril_default_request_resp_params */


/*=========================================================================
  FUNCTION:  qcril_send_request_response

===========================================================================*/
/*!
    @brief
    Remove the entry from the ReqList. Send response for RIL request.

    @return
    None
*/
/*=========================================================================*/
void qcril_send_request_response
(
  qcril_request_resp_params_type *param_ptr
)
{
  qcril_instance_id_e_type instance_id;
  char label[ 300 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( param_ptr != NULL );
  instance_id = param_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  /* Remove entry from Reqlist if applicable */
  ( void ) qcril_reqlist_free( param_ptr->instance_id, param_ptr->t );

  /* Log the event packet for the response to RIL request */
  if ( param_ptr->logstr != NULL )
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s - %s, RID %d, Token %d, %s",
                    qcril_log_lookup_event_name( param_ptr->request_id ), param_ptr->logstr, param_ptr->instance_id,
                    qcril_log_get_token_id( param_ptr->t ), qcril_log_lookup_errno_name( param_ptr->ril_err_no ) );
  }
  else
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s - RID %d, Token %d, %s",
                    qcril_log_lookup_event_name( param_ptr->request_id ), param_ptr->instance_id,
                    qcril_log_get_token_id( param_ptr->t ), qcril_log_lookup_errno_name( param_ptr->ril_err_no ) );
  }

  QCRIL_LOG_CF_PKT_RIL_RES( instance_id, label );

  /* Send response to the RIL request */
  QCRIL_LOG_DEBUG( "UI <--- %s (%d) Complete --- RIL [RID %d, Token %d, %s, Len %d %s]\n",
                   qcril_log_lookup_event_name( param_ptr->request_id ), param_ptr->request_id, param_ptr->instance_id,
                   qcril_log_get_token_id( param_ptr->t ), qcril_log_lookup_errno_name( param_ptr->ril_err_no ), param_ptr->resp_len,
                   ( param_ptr->logstr == NULL )? "" : param_ptr->logstr );

  qcril_response_api[ param_ptr->instance_id ]->OnRequestComplete( param_ptr->t, param_ptr->ril_err_no,
                                                                   param_ptr->resp_pkt, param_ptr->resp_len );

} /* qcril_send_request_response */


/*=========================================================================
  FUNCTION:  qcril_default_unsol_resp_params

===========================================================================*/
/*!
    @brief
    Set default values for unsolicted response parameters.

    @return
    None
*/
/*=========================================================================*/
void qcril_default_unsol_resp_params
(
  qcril_instance_id_e_type instance_id,
  int response_id,
  qcril_unsol_resp_params_type *param_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( param_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  param_ptr->instance_id = instance_id;
  param_ptr->response_id = response_id;
  param_ptr->resp_pkt = NULL;
  param_ptr->resp_len = 0;
  param_ptr->logstr = NULL;

} /* qcril_default_unsol_resp_params */


/*=========================================================================
  FUNCTION:  qcril_send_unsol_response

===========================================================================*/
/*!
    @brief
    Send RIL_onUnsolicitedResponse.

    @return
    None
*/
/*=========================================================================*/
void qcril_send_unsol_response
(
  qcril_unsol_resp_params_type *param_ptr
)
{
  qcril_instance_id_e_type instance_id;
  char label[ 300 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( param_ptr!= NULL );
  instance_id = param_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  /* Log event packet for Unsolicited response */
  if ( param_ptr->logstr != NULL)
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s, %s", qcril_log_lookup_event_name( param_ptr->response_id ), param_ptr->logstr );
  }
  else
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s", qcril_log_lookup_event_name( param_ptr->response_id ) );
  }

  QCRIL_LOG_CF_PKT_RIL_UNSOL_RES( instance_id, label );

  /* Send Unsolicted RIL response */
  QCRIL_LOG_DEBUG( "UI <--- %s (%d) --- RIL [RID %d, Len %d, %s]\n",
                   qcril_log_lookup_event_name( param_ptr->response_id ), param_ptr->response_id,
                   instance_id, param_ptr->resp_len, param_ptr->logstr );

  qcril_response_api[ instance_id ]->OnUnsolicitedResponse( param_ptr->response_id, param_ptr->resp_pkt,
                                                            param_ptr->resp_len );

} /* qcril_send_unsol_response */


/*=========================================================================
  FUNCTION:  qcril_hook_unsol_response

===========================================================================*/
/*!
    @brief
    Sending the unsolicited oem hook indication including header.

    @return
    None
*/
/*=========================================================================*/
void qcril_hook_unsol_response
(
  qcril_instance_id_e_type instance_id,
  uint32  unsol_event,
  char *  data,
  uint32  data_len
)
{
  char *payload;
  uint32 index = 0;
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  payload = (char *) qcril_malloc( QCRIL_OTHER_OEM_NAME_LENGTH + sizeof(unsol_event) + sizeof(data_len) + data_len );

  if( payload != NULL )
  {
    memcpy( payload, QCRIL_HOOK_OEM_NAME, QCRIL_OTHER_OEM_NAME_LENGTH );
    index += QCRIL_OTHER_OEM_NAME_LENGTH;

    memcpy( &payload[index], &unsol_event, sizeof(unsol_event) );
    index += sizeof(unsol_event);

    memcpy( &payload[index], &data_len, sizeof(data_len) );
    index += sizeof(data_len);

    memcpy( &payload[index], data, data_len );
    index += data_len;

    qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_OEM_HOOK_RAW, &unsol_resp );
    unsol_resp.resp_pkt = ( void * ) payload;
    unsol_resp.resp_len = index;
    qcril_send_unsol_response( &unsol_resp );
    qcril_free( payload );
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "qcril_malloc returned NULL in qcril_hook_unsol_response()\n" );
  }

} /* qcril_hook_unsol_response */


/*===========================================================================

FUNCTION qcril_hash

DESCRIPTION
  Return a hash of the mobile ESN and other selected inputs ( see CAI 2-216 )
    R = floor( N* (( 40503 * ( L ^ H ^ DECORR )) % 65536 ) / 65536 )

DEPENDENCIES
  None.

RETURN VALUE
  A number between 0 and N-1.  Note that in some cases the CAI requires that
  1 be added to this.  This is up to the caller.

SIDE EFFECTS
  None.

===========================================================================*/
static uint16 qcril_hash
(
  uint32 hash_key,
  uint16  n, /* Range */
  uint16  decorr
)
{
  uint16 temp;

  /*-----------------------------------------------------------------------*/

  temp = (decorr ^ ((uint16)( hash_key & 0xFFFF )) ^ ((uint16)(( hash_key>>16 ) & 0xFFFF )));
  temp = ( temp * 40503L ) & 0xFFFF;
  temp = (uint16)(( (long) temp * n ) >> 16);

  return( temp );

} /* qcril_hash() */

/*===========================================================================

  FUNCTION:  qcril_hash_table_lookup

===========================================================================*/
/*!
    @brief
    Finds the dispatch table entry for a given event_id

    @return
    E_SUCCESS if an entry is found
    E_FAILURE if no entry is found

*/
/*=========================================================================*/
static IxErrnoType qcril_hash_table_lookup
(
  uint32 id,
  qcril_dispatch_table_entry_type **entry_ptr_ptr /*!< OUT table entry ptr, if found */
)
{
  uint32 hash_index; /*!< Indicies into hash table */
  IxErrnoType status = E_SUCCESS;
  qcril_dispatch_table_entry_type *temp_entry_ptr;

  /*-----------------------------------------------------------------------*/

  hash_index = qcril_hash( id, QCRIL_HT_ENTRIES_MAX, 0 );
  QCRIL_ASSERT( hash_index < QCRIL_HT_ENTRIES_MAX );

  temp_entry_ptr = (qcril_dispatch_table_entry_type *) qcril_hash_table[hash_index];

  /* Look through the hash table collision list for this entry. */

  while (temp_entry_ptr != NULL)
  {
    if (temp_entry_ptr->event_id == id)
    {
      if ( entry_ptr_ptr != NULL )
      {
        *entry_ptr_ptr = temp_entry_ptr;
      }
      break;
    }
    temp_entry_ptr = temp_entry_ptr->next_ptr;
  }

  if (temp_entry_ptr == NULL)
  {
    status = E_FAILURE;
  }

  return (status);

} /* qcril_hash_table_lookup() */


/*===========================================================================

  FUNCTION:  qcril_state_transition

===========================================================================*/
/*!
    @brief
    Called each time an event handler returns, to process the next state
    of qcril.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_state_transition
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  int event_id,
  const qcril_request_return_type *ret_ptr
)
{
  qcril_arb_state_info_struct_type *s_ptr;
  #ifdef FEATURE_QCRIL_DSDS
  qcril_arb_state_info_struct_type *sec_s_ptr;
  #endif /* FEATURE_QCRIL_DSDS */
  boolean radio_state_changed = FALSE;
  RIL_RadioState next_radio_state, current_radio_state;
  boolean modem_state_changed = FALSE;
  qcril_modem_state_e_type next_modem_state;
  boolean pri_gw_sim_state_changed = FALSE, pri_cdma_sim_state_changed = FALSE;
  boolean sec_gw_sim_state_changed = FALSE, sec_cdma_sim_state_changed = FALSE;
  qcril_sim_state_e_type current_pri_gw_sim_state, next_pri_gw_sim_state, current_pri_cdma_sim_state, next_pri_cdma_sim_state;
  qcril_sim_state_e_type current_sec_gw_sim_state, next_sec_gw_sim_state, current_sec_cdma_sim_state, next_sec_cdma_sim_state;

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  IxErrnoType err_no;
  boolean subscription_config_changed = FALSE;
  uint16 current_subscription_config_mask, new_subscription_config_mask, subscription_enable_mask = 0, subscription_disable_mask = 0;
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  boolean voice_radio_tech_changed = FALSE;
  #ifndef FEATURE_ICS
  qcril_radio_tech_family_e_type new_voice_radio_tech;
  #else
  qcril_radio_tech_e_type new_voice_radio_tech;
  #endif
  qcril_unsol_resp_params_type unsol_resp;

  char *modem_state_name[] = { "Unavailable", "Off", "On" };
  char *sim_state_name[] = { "Absent", "Not ready", "Ready", "PIN", "PUK", "Network personalization", "Error", "Illegal" };
  #ifndef FEATURE_ICS
  char *radio_tech_name[] = { "Unknown", "3GPP", "3GPP2", "Global" };
  #else
  char *radio_tech_name[] = { "Unknown", "GPRS", "EDGE", "UMTS", "IS95A", "IS95B", "1xRTT", "EVDO_0", "EVDO_A", "HSDPA", "HSUPA", "HSPA", "EVDO_B", "EHRPD", "LTE", "GLOBAL" };
  #endif
  char label[ 300 ];
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  s_ptr = &qcril_state->info[ instance_id ];
  QCRIL_ASSERT( ret_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_state->mutex, "qcril_state_mutex" );

  current_radio_state = s_ptr->radio_state;
  next_radio_state = current_radio_state;
  next_modem_state = s_ptr->modem_state;
  current_pri_gw_sim_state = next_pri_gw_sim_state = s_ptr->pri_gw_sim_state;
  current_pri_cdma_sim_state = next_pri_cdma_sim_state = s_ptr->pri_cdma_sim_state;
  current_sec_gw_sim_state = next_sec_gw_sim_state = s_ptr->sec_gw_sim_state;
  current_sec_cdma_sim_state = next_sec_cdma_sim_state = s_ptr->sec_cdma_sim_state;
  new_voice_radio_tech = s_ptr->voice_radio_tech;

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  current_subscription_config_mask = new_subscription_config_mask = s_ptr->subscription_config_mask;
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  /* Check whether the Modem state is changed */
  if ( ret_ptr->modem_state_changed && ( ret_ptr->next_modem_state !=  s_ptr->modem_state ) )
  {
    modem_state_changed = TRUE;
    next_modem_state = ret_ptr->next_modem_state;
  }

  /* Check whether the primary GSM/WCDMA SIM state is changed */
  if ( ret_ptr->pri_gw_sim_state_changed && ( ret_ptr->next_pri_gw_sim_state !=  s_ptr->pri_gw_sim_state ) )
  {
    pri_gw_sim_state_changed = TRUE;
    next_pri_gw_sim_state = ret_ptr->next_pri_gw_sim_state;
  }

  /* Check whether the primary CDMA SIM state is changed */
  if ( ret_ptr->pri_cdma_sim_state_changed && ( ret_ptr->next_pri_cdma_sim_state !=  s_ptr->pri_cdma_sim_state ) )
  {
    pri_cdma_sim_state_changed = TRUE;
    next_pri_cdma_sim_state = ret_ptr->next_pri_cdma_sim_state;
  }

  /* Check whether the secondary GSM/WCDMA SIM state is changed */
  if ( ret_ptr->sec_gw_sim_state_changed && ( ret_ptr->next_sec_gw_sim_state !=  s_ptr->sec_gw_sim_state ) )
  {
    sec_gw_sim_state_changed = TRUE;
    next_sec_gw_sim_state = ret_ptr->next_sec_gw_sim_state;
  }

  /* Check whether the secondary CDMA SIM state is changed */
  if ( ret_ptr->sec_cdma_sim_state_changed && ( ret_ptr->next_sec_cdma_sim_state !=  s_ptr->sec_cdma_sim_state ) )
  {
    sec_cdma_sim_state_changed = TRUE;
    next_sec_cdma_sim_state = ret_ptr->next_sec_cdma_sim_state;
  }

  /* Check whether the network mode is changed */
  if ( ( ( event_id == QCRIL_EVT_CM_SS_SRV_CHANGED ) || ( event_id == QCRIL_EVT_CM_SS_INFO ) )&&
       ret_ptr->voice_radio_tech_changed &&
       ( ret_ptr->new_voice_radio_tech != s_ptr->voice_radio_tech ) )
  {
    /* Not global mode */
    if ( ret_ptr->new_voice_radio_tech != QCRIL_RADIO_TECH_GLOBAL )
    {
      voice_radio_tech_changed = TRUE;
      new_voice_radio_tech = ret_ptr->new_voice_radio_tech;
    }
    /* Global mode, not camped on any system before */
    else if ( new_voice_radio_tech == QCRIL_RADIO_TECH_NONE )
    {
      voice_radio_tech_changed = TRUE;
      #ifndef FEATURE_ICS
      new_voice_radio_tech = QCRIL_RADIO_TECH_3GPP2;
      #else
      new_voice_radio_tech = QCRIL_RADIO_TECH_1xRTT;
      #endif
    }
  }

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  /* Check whether the subscription configuration is changed */
  if ( ret_ptr->subscription_config_changed && ( ret_ptr->new_subscription_config_mask !=  s_ptr->subscription_config_mask ) )
  {
    subscription_config_changed = TRUE;
    new_subscription_config_mask = ret_ptr->new_subscription_config_mask;
  }
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  QCRIL_LOG_DEBUG( "[%s(%d)] Modem: %s --> %s, Voice Tech: %s --> %s\n",
                   qcril_log_lookup_event_name( event_id ), event_id,
                   modem_state_name[ s_ptr->modem_state ], modem_state_name[ next_modem_state ],
                   radio_tech_name[ s_ptr->voice_radio_tech ], radio_tech_name[ new_voice_radio_tech ] );

  QCRIL_LOG_DEBUG( "[%s(%d)] GW SIM(pri): %s --> %s, CDMA SIM(pri): %s --> %s, GW SIM(sec): %s --> %s, CDMA SIM(sec): %s --> %s\n",
                   qcril_log_lookup_event_name( event_id ), event_id,
                   sim_state_name[ s_ptr->pri_gw_sim_state ], sim_state_name[ next_pri_gw_sim_state ],
                   sim_state_name[ s_ptr->pri_cdma_sim_state ], sim_state_name[ next_pri_cdma_sim_state ],
                   sim_state_name[ s_ptr->sec_gw_sim_state ], sim_state_name[ next_sec_gw_sim_state ],
                   sim_state_name[ s_ptr->sec_cdma_sim_state ], sim_state_name[ next_sec_cdma_sim_state ] );

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  QCRIL_LOG_DEBUG( "[%s(%d)] Subscription: 0x%x --> 0x%x\n",
                   qcril_log_lookup_event_name( event_id ), event_id, current_subscription_config_mask,
                   new_subscription_config_mask );
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  /* Change in Modem State, SIM state or Subscription Config mask. Figure out the states update */
  if ( modem_state_changed || pri_gw_sim_state_changed || pri_cdma_sim_state_changed || voice_radio_tech_changed
       #ifdef FEATURE_QCRIL_SUBS_CTRL
       || subscription_config_changed
       #endif /* FEATURE_QCRIL_SUBS_CTRL */
       #ifdef FEATURE_QCRIL_DSDS
       || sec_gw_sim_state_changed || sec_cdma_sim_state_changed
       #endif /* FEATURE_QCRIL_DSDS */
     )
  {
    switch ( next_modem_state )
    {
      /* Modem is resetting or rebooting */
      case QCRIL_MODEM_STATE_UNAVAILABLE:
        next_radio_state = RADIO_STATE_UNAVAILABLE;
        break;

      /* Modem is in low power mode */
      case QCRIL_MODEM_STATE_OFF:
        next_radio_state = RADIO_STATE_OFF;
        break;

      /* Modem is in online mode */
      default:
        next_radio_state = RADIO_STATE_ON;
        break;

    } /* end switch */

    /* Save the Modem State, SIM state */
    s_ptr->modem_state = next_modem_state;
    s_ptr->pri_gw_sim_state = next_pri_gw_sim_state;
    s_ptr->pri_cdma_sim_state = next_pri_cdma_sim_state;
    s_ptr->sec_gw_sim_state = next_sec_gw_sim_state;
    s_ptr->sec_cdma_sim_state = next_sec_cdma_sim_state;
    s_ptr->voice_radio_tech = new_voice_radio_tech;

    #ifdef FEATURE_QCRIL_SUBS_CTRL
    s_ptr->subscription_config_mask = new_subscription_config_mask;
    #endif /* FEATURE_QCRIL_SUBS_CTRL */

    #ifdef FEATURE_QCRIL_DSDS
    /* Sync modem state and sim states among instances */
    if ( qcril_arb_ma_is_dsds() )
    {
      if ( modem_state_changed || pri_gw_sim_state_changed || pri_cdma_sim_state_changed || sec_gw_sim_state_changed ||
           sec_cdma_sim_state_changed )
      {
        QCRIL_LOG_DEBUG( "%s\n", "Sync modem state and SIM state among instances" );
        sec_s_ptr = &qcril_state->info[ QCRIL_DSDS_INSTANCE_PAIR( instance_id ) ];
        sec_s_ptr->modem_state = next_modem_state;
        sec_s_ptr->pri_gw_sim_state = next_pri_gw_sim_state;
        sec_s_ptr->pri_cdma_sim_state = next_pri_cdma_sim_state;
        sec_s_ptr->sec_gw_sim_state = next_sec_gw_sim_state;
        sec_s_ptr->sec_cdma_sim_state = next_sec_cdma_sim_state;
      }
    }
    #endif /* FEATURE_QCRIL_DSDS */

    if ( next_radio_state != current_radio_state )
    {
      radio_state_changed = TRUE;
      s_ptr->radio_state = next_radio_state;

      QCRIL_SNPRINTF( label, sizeof( label ), "Radio State Change: %s -> %s", radio_state_name[ current_radio_state],
                      radio_state_name[next_radio_state] );
      QCRIL_LOG_CF_PKT_RIL_ST_CHG( instance_id, label );

      /* Report the radio state change */
      QCRIL_LOG_DEBUG( "RID %d [%s(%d)] RadioState %s --> %s\n",
                       instance_id, qcril_log_lookup_event_name( event_id ), event_id ,
                       radio_state_name[ current_radio_state ], radio_state_name[ s_ptr->radio_state ] );

      /* Notify Android for the Radio State Change */
      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, &unsol_resp );
      qcril_send_unsol_response( &unsol_resp );

      #ifdef FEATURE_QCRIL_DSDS
      /* Sync radio state reporting among instances */
      if ( qcril_arb_ma_is_dsds() )
      {
         sec_s_ptr->radio_state = next_radio_state;
        /* Report the radio state change */
        QCRIL_LOG_DEBUG( "RID %d [%s(%d)] RadioState %s --> %s\n",
                         QCRIL_DSDS_INSTANCE_PAIR( instance_id ),qcril_log_lookup_event_name( event_id ), event_id ,
                         radio_state_name[ current_radio_state ], radio_state_name[ sec_s_ptr->radio_state ] );

        /* Notify Android for the Radio State Change */
        qcril_default_unsol_resp_params( QCRIL_DSDS_INSTANCE_PAIR( instance_id ), (int) RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, &unsol_resp );
        qcril_send_unsol_response( &unsol_resp );
      }
      #endif /* FEATURE_QCRIL_DSDS */
    }

    if ( voice_radio_tech_changed )
    {
      /* Notify Android for the Voice Radio Tech Change */
      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, &unsol_resp );
      #ifdef FEATURE_ICS
      unsol_resp.resp_pkt = ( void * ) &new_voice_radio_tech;
      unsol_resp.resp_len = sizeof(new_voice_radio_tech);
      #endif /* FEATURE_ICS */
      qcril_send_unsol_response( &unsol_resp );

      #ifdef FEATURE_QCRIL_DSDS
      if ( !qcril_arb_ma_is_dsds() )
      {
        /* Voice tech changed, need to update ECC cache and property if using session API */
        (void) qcril_pbm_get_ecc( instance_id, modem_id, PBM_FIELD_NONE, TRUE );
      }
      #endif /* FEATURE_QCRIL_DSDS */
    }
  }

  QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );

  if ( ret_ptr->ssic_notification_status != QCRIL_SSIC_NOTIFICATION_STATUS_NO_CHANGE )
  {
    qcril_cm_process_network_info( instance_id, modem_id, ret_ptr->ssic_notification_status );
  }

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  /* Changes in Radio State, GW SIM State, CDMA SIM State, Subscription Configuration, may trigger a change in subscription i
     state */
  if ( ( radio_state_changed || pri_gw_sim_state_changed || pri_cdma_sim_state_changed || subscription_config_changed ) &&
       ( next_radio_state != RADIO_STATE_UNAVAILABLE ) )
  {
    /* Check for changes in GW Subscription state */
    if ( new_subscription_config_mask & QCRIL_SUBSCRIPTION_SIM_MASK )
    {
      /* GW SIM is ready and Modem is ON. Time to report GW subscription available.
         Note: Subscription available will trigger modem operations (e.g. RAT Balancing Test Scenario) that can only be executed
         successfully if modem is in ONLINE mode */
      if ( ( next_modem_state == QCRIL_MODEM_STATE_ON ) && ( next_pri_gw_sim_state == QCRIL_SIM_STATE_READY ) )
      {
        QCRIL_LOG_INFO( "RID %d MID %d Modem is on, GW SIM ready, check Pri GW subscription available reporting\n",
                        instance_id, modem_id );
        subscription_enable_mask |= QCRIL_MODE_GW_CONFIG_MASK;
      }
      /* Modem is OFF or GW SIM State changes to ERROR, ABSENT, PUK or NOT READY. Need to report subscription not available */
      else if ( ( next_modem_state == QCRIL_MODEM_STATE_OFF ) || ( next_pri_gw_sim_state == QCRIL_SIM_STATE_ABSENT ) ||
                ( next_pri_gw_sim_state == QCRIL_SIM_STATE_CARD_ERROR ) || ( next_pri_gw_sim_state == QCRIL_SIM_STATE_PUK ) ||
                ( next_pri_gw_sim_state == QCRIL_SIM_STATE_NOT_READY ) || ( next_pri_gw_sim_state == QCRIL_SIM_STATE_ILLEGAL ) )
      {
        QCRIL_LOG_INFO( "RID %d MID %d Modem state is off or GW SIM state changes to Absent/Error/PUK/NotReady, check Pri GW subscription not available reporting\n",
                        instance_id, modem_id );
        subscription_disable_mask |= QCRIL_MODE_GW_CONFIG_MASK;
      }
    }

    /* Check for changes in CDMA Subscription state */
    /* CDMA subscription is NV */
    if ( new_subscription_config_mask & QCRIL_SUBSCRIPTION_NV_MASK )
    {
      /* CDMA subscription is NV and Modem is ON. Time to report CDMA subscription available.
         Note: Subscription available will trigger modem operations that can only be executed successfully if modem is in
         ONLINE mode */
      if ( next_modem_state == QCRIL_MODEM_STATE_ON )
      {
        QCRIL_LOG_INFO( "RID %d MID %d CDMA subscription is NV, Modem is on, check Pri CDMA subscription available reporting\n",
                        instance_id, modem_id );
        subscription_enable_mask |= QCRIL_MODE_1XEVDO_CONFIG_MASK;
      }
      /* CDMA subscription is NV and Modem is OFF. Time to report CDMA subscription not available. */
      else if ( next_modem_state == QCRIL_MODEM_STATE_OFF )
      {
        QCRIL_LOG_INFO( "RID %d MID %d CDMA subscription is NV, Modem is off, check Pri CDMA subscription unavailable reporting\n",
                        instance_id, modem_id );
        subscription_disable_mask |= QCRIL_MODE_1XEVDO_CONFIG_MASK;
      }
    }

    /* CDMA subscription is RUIM */
    else if ( new_subscription_config_mask & QCRIL_SUBSCRIPTION_RUIM_MASK )
    {
      /* CDMA subscription is RUIM and Modem is ON. Time to report CDMA subscription available.
         Note: Subscription available will trigger modem operations that can only be executed successfully if modem is in
         ONLINE mode */
      if ( ( next_modem_state == QCRIL_MODEM_STATE_ON ) && ( next_pri_cdma_sim_state == QCRIL_SIM_STATE_READY ) )
      {
        QCRIL_LOG_INFO( "RID %d MID %d CDMA subscription is RUIM, Modem is on, RUIM state is ready, check CDMA subscription available reporting\n",
                        instance_id, modem_id );
        subscription_enable_mask |= QCRIL_MODE_1XEVDO_CONFIG_MASK;
      }
      /* Modem is OFF or RUIM State changes to ERROR, ABSENT, PUK or NOT_READY. Need to report subscription not available */
      else if ( ( next_modem_state == QCRIL_MODEM_STATE_OFF ) || ( next_pri_cdma_sim_state == QCRIL_SIM_STATE_ABSENT ) ||
                ( next_pri_cdma_sim_state == QCRIL_SIM_STATE_CARD_ERROR ) || ( next_pri_cdma_sim_state == QCRIL_SIM_STATE_PUK ) ||
                ( next_pri_cdma_sim_state == QCRIL_SIM_STATE_NOT_READY ) || ( next_pri_cdma_sim_state == QCRIL_SIM_STATE_ILLEGAL ) )
      {
        QCRIL_LOG_INFO( "RID %d MID %d CDMA subscription is RUIM, Modem is off or RUIM state changes Absent/Error/PUK/NotReady, check CDMA subscription not available reporting\n",
                        instance_id, modem_id );
        subscription_disable_mask |= QCRIL_MODE_1XEVDO_CONFIG_MASK;
      }
    }

    /* Trigger the processing of subscription available */
    if ( subscription_enable_mask != 0 )
    {
      err_no = qcril_process_event( instance_id, modem_id, QCRIL_EVT_CM_ENABLE_SUBSCRIPTION, (void *) &subscription_enable_mask,
                                    sizeof( subscription_enable_mask ), NULL );
      QCRIL_ASSERT( err_no == E_SUCCESS );
    }

    /* Trigger the processing of subscription unavailable */
    if ( subscription_disable_mask != 0 )
    {
      err_no = qcril_process_event( instance_id, modem_id, QCRIL_EVT_CM_DISABLE_SUBSCRIPTION, (void *) &subscription_disable_mask,
                                     sizeof( subscription_disable_mask ), NULL );
      QCRIL_ASSERT( err_no == E_SUCCESS );
    }
  }
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

} /* qcril_state_transition() */


/*===========================================================================

  FUNCTION:  qcril_dispatch_event

===========================================================================*/
/*!
    @brief
    Does processing common to onRequest and qcril_process_event with
    respect to calling the event handler and processing the return value
    from the event handler.

    @return
    E_SUCCESS of the event was handled normally
    E_NOT_ALLOWED if the event is not supported in the current state

*/
/*=========================================================================*/
static IxErrnoType qcril_dispatch_event
(
  qcril_dispatch_table_entry_type *entry_ptr,
  qcril_request_params_type *params_ptr
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_arb_state_info_struct_type *s_ptr;
  uint16 allowed_radio_states_mask = 0;
  boolean request_is_allowed_in_current_state = FALSE;
  qcril_request_return_type ret;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  s_ptr = &qcril_state->info[ instance_id ];
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_MUTEX_LOCK( &qcril_state->mutex, "qcril_state_mutex" );

  /* Ignore all requests received in an unsupported state */
  allowed_radio_states_mask = ( 1 << (int) s_ptr->radio_state );

  if ( entry_ptr->allowed_radio_states_mask & allowed_radio_states_mask )
  {
    request_is_allowed_in_current_state = TRUE;
  }
  /* For the power up scenario started in Airplane mode, allow SIM/RUIM access  */
  else if ( ( entry_ptr->allowed_radio_states_mask == QCRIL_STATE_MASK_SIM_OR_RUIM_READY_STATES ) &&
            ( allowed_radio_states_mask & QCRIL_STATE_MASK_ALL_ACTIVE_STATES ) &&
            ( ( s_ptr->pri_gw_sim_state   ==  QCRIL_SIM_STATE_READY ) ||
              ( s_ptr->sec_gw_sim_state   ==  QCRIL_SIM_STATE_READY ) ||
              ( s_ptr->pri_cdma_sim_state ==  QCRIL_SIM_STATE_READY ) ||
              ( s_ptr->sec_cdma_sim_state ==  QCRIL_SIM_STATE_READY ) ||
              ( s_ptr->pri_gw_sim_state   ==  QCRIL_SIM_STATE_ILLEGAL ) ||
              ( s_ptr->sec_gw_sim_state   ==  QCRIL_SIM_STATE_ILLEGAL ) ||
              ( s_ptr->pri_cdma_sim_state ==  QCRIL_SIM_STATE_ILLEGAL) ||
              ( s_ptr->sec_cdma_sim_state ==  QCRIL_SIM_STATE_ILLEGAL ) ) )
  {
    request_is_allowed_in_current_state = TRUE;
  }

  if ( !request_is_allowed_in_current_state )
  {
    QCRIL_LOG_DEBUG( "Ignore %s, unsupported state.\n", qcril_log_lookup_event_name( params_ptr->event_id ) );
    QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );
    return (E_NOT_ALLOWED);
  }

  /* Initialize the structure that the request handler will use to return
     information about the status of the request */
  ret.modem_state_changed = FALSE;
  ret.next_modem_state = s_ptr->modem_state;
  ret.pri_gw_sim_state_changed = FALSE;
  ret.next_pri_gw_sim_state = s_ptr->pri_gw_sim_state;
  ret.pri_cdma_sim_state_changed = FALSE;
  ret.next_pri_cdma_sim_state = s_ptr->pri_cdma_sim_state;
  ret.sec_gw_sim_state_changed = FALSE;
  ret.next_sec_gw_sim_state = s_ptr->sec_gw_sim_state;
  ret.sec_cdma_sim_state_changed = FALSE;
  ret.next_sec_cdma_sim_state = s_ptr->sec_cdma_sim_state;
  ret.voice_radio_tech_changed = FALSE;
  ret.new_voice_radio_tech = s_ptr->voice_radio_tech;
  ret.ssic_notification_status = QCRIL_SSIC_NOTIFICATION_STATUS_NO_CHANGE;

  #ifdef FEATURE_QCRIL_SUBS_CTRL
  ret.subscription_config_changed = FALSE;
  ret.new_subscription_config_mask = s_ptr->subscription_config_mask;
  #endif /* FEATURE_QCRIL_SUBS_CTRL */

  QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );

  /* Dispatch the request to the appropriate handler */
  (entry_ptr->handler)(params_ptr, &ret);

  /* Handle state transition */
  if ( ret.modem_state_changed || ret.pri_gw_sim_state_changed || ret.pri_cdma_sim_state_changed ||
       ret.sec_gw_sim_state_changed || ret.sec_cdma_sim_state_changed || ret.voice_radio_tech_changed ||
       ( ret.ssic_notification_status != QCRIL_SSIC_NOTIFICATION_STATUS_NO_CHANGE )
       #ifdef FEATURE_QCRIL_SUBS_CTRL
       || ret.subscription_config_changed
       #endif /* FEATURE_QCRIL_SUBS_CTRL */
     )
  {
    qcril_state_transition( instance_id, modem_id, params_ptr->event_id, &ret );
  }

  return E_SUCCESS;

} /* qcril_dispatch_event() */


/*===========================================================================

  FUNCTION:  qcril_parse_oem_hook_header

===========================================================================*/
/*!
    @brief
    Does parsing of oem hook request for routing it to appropriate module.

    @return
    TRUE if header is encoded properly and request id is valid.
    FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_parse_oem_hook_header
(
  char   *data,
  uint32 *hook_req,
  uint32 *hook_req_len
)
{
  char oem_name[9];
  uint32 index = 0, cmd_id = 0;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( data != NULL );

  /*-----------------------------------------------------------------------*/

  /* decode the raw string to find out the oem name string data[0 - 7], 8 bytes*/
  if( strncmp( data, QCRIL_HOOK_OEM_NAME, QCRIL_OTHER_OEM_NAME_LENGTH ) != 0 )
  {
     memcpy( oem_name, &data[index], QCRIL_OTHER_OEM_NAME_LENGTH );
     oem_name[QCRIL_OTHER_OEM_NAME_LENGTH] = '\0';
     QCRIL_LOG_DEBUG( "Mismatch in oem_name between received=%s and expected=%s \n", oem_name, QCRIL_HOOK_OEM_NAME);
     return FALSE;
  }

  /* incrementing the index by OEM name size i.e 9 bytes */
  index += (uint32)QCRIL_OTHER_OEM_NAME_LENGTH;

  /* decode the raw string to find out command id, data[9 - 12], 4 bytes */
  memcpy( &cmd_id, &data[index], QCRIL_OTHER_OEM_REQUEST_ID_LEN );
  if( cmd_id >= QCRIL_EVT_OEM_MAX )
  {
     QCRIL_LOG_DEBUG( "Received un expected command id = %lu\n", cmd_id );
     return FALSE;
  }

  switch ( cmd_id )
  {
    case QCRIL_EVT_HOOK_NV_READ:
    case QCRIL_EVT_HOOK_NV_WRITE:
    case QCRIL_EVT_HOOK_DATA_GO_DORMANT:
    case QCRIL_EVT_HOOK_ME_DEPERSONALIZATION:
    case QCRIL_EVT_HOOK_SET_TUNE_AWAY:
    case QCRIL_EVT_HOOK_GET_TUNE_AWAY:
    case QCRIL_EVT_HOOK_SET_PAGING_PRIORITY:
    case QCRIL_EVT_HOOK_GET_PAGING_PRIORITY:
      *hook_req = cmd_id;
      break;

    default:
      QCRIL_LOG_DEBUG( "Received un-handled/oem command id = %lu\n", cmd_id );
      *hook_req = cmd_id;
  }

  /* incrementing the index by command id size i.e 4 bytes */
  index += (uint32)QCRIL_OTHER_OEM_REQUEST_ID_LEN;

  /* decode the raw string to find the length of the payload, data[13 - 16],  4 bytes*/
  memcpy( hook_req_len, &data[index], QCRIL_OTHER_OEM_REQUEST_DATA_LEN );

  return TRUE;

} /* qcril_parse_oem_hook_header() */


/*===========================================================================

  FUNCTION:  onRequest

===========================================================================*/
/*!
    @brief
    Call from RIL to us to make a RIL_REQUEST
    Must be completed with a call to RIL_onRequestComplete()
    RIL_onRequestComplete() may be called from any thread, before or after
    this function returns.
    Returning from this routine implies the radio is ready to process another
    command (whether or not the previous command has completed).

    @return
    None.
*/
/*=========================================================================*/
static void onRequest
(
  qcril_instance_id_e_type instance_id,
  int request,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  qcril_request_params_type param;
  qcril_dispatch_table_entry_type *entry_ptr; /*!< Dispatch table entry */
  int *in_data_ptr, in_data_val;
  char label[ 300 ];
  uint32 oem_hook_req, oem_hook_datalen;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  QCRIL_ASSERT( t != (void *) QCRIL_TOKEN_ID_INTERNAL );

  /*-----------------------------------------------------------------------*/

  /* Initialize the input parameters for the request handler.  This allows for
     easy addition or deletion of parameters in the future. */
  param.event_id = request;
  param.data = data;
  param.datalen = datalen;
  param.t = t;
  param.instance_id = instance_id;
  param.modem_id = QCRIL_DEFAULT_MODEM_ID;

  QCRIL_LOG_DEBUG( "UI --- %s (%d) ---> RIL [RID %d, token id %d, data len %d]\n",
                   qcril_log_lookup_event_name( param.event_id ), param.event_id, param.instance_id,
                   qcril_log_get_token_id( param.t ), param.datalen );

  if ( ( param.event_id == RIL_REQUEST_DIAL ) || ( param.event_id == RIL_REQUEST_SETUP_DATA_CALL ) )
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s - RID %d, Token %d", qcril_log_lookup_event_name( param.event_id ),
                    param.instance_id, qcril_log_get_token_id( param.t ) );
    /* Use bold arrows for really interesting events */
    QCRIL_LOG_CF_PKT_RIL_REQ2( param.instance_id, label );
  }
  else if ( ( ( param.event_id == RIL_REQUEST_RADIO_POWER ) ||
              ( param.event_id == RIL_REQUEST_SCREEN_STATE ) ||
              ( param.event_id == RIL_REQUEST_SET_MUTE ) ) && ( param.data != NULL ) )
  {
    in_data_ptr = ( int * ) param.data;
    if ( in_data_ptr != NULL )
    {
      in_data_val = *in_data_ptr;
    }
    else
    {
      in_data_val = 1;
    }

    QCRIL_SNPRINTF( label, sizeof( label ), "%s - %s, Token %d",
                    qcril_log_lookup_event_name( param.event_id ), ( in_data_val == 0 ) ? "Off" : "On" ,
                    qcril_log_get_token_id( param.t ) );
    QCRIL_LOG_CF_PKT_RIL_REQ( param.instance_id, label );
  }
  else
  {
    QCRIL_SNPRINTF( label, sizeof( label ), "%s - Token %d",
                    qcril_log_lookup_event_name( param.event_id ), qcril_log_get_token_id( param.t ) );
    QCRIL_LOG_CF_PKT_RIL_REQ( param.instance_id, label );
  }

  if ( param.event_id >= (int) QCRIL_EVT_BASE )
  {
    /* The request is out of range */
    qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  if ( param.event_id  == RIL_REQUEST_OEM_HOOK_RAW )
  {
    /* parse the OEM hook request to distinguish between internal or customer specific requests */
    if ( qcril_parse_oem_hook_header( data, &oem_hook_req, &oem_hook_datalen ) )
    {
      if ( ( ( oem_hook_req > QCRIL_EVT_HOOK_BASE ) && ( oem_hook_req < QCRIL_EVT_HOOK_MAX ) ) ||
           ( ( oem_hook_req > QCRIL_EVT_OEM_BASE ) && ( oem_hook_req < QCRIL_EVT_OEM_MAX ) ) )
      {
        /* This is an OEM_HOOK request, Convert it to look like a internal RIL REQUEST */
        /* Move data pointer past the QCRILHook header and re-adjusting the length */
        param.event_id = oem_hook_req;
        param.data = (char *)data + QCRIL_HOOK_HEADER_SIZE;
        param.datalen = oem_hook_datalen;
        if ( ( oem_hook_req > QCRIL_EVT_OEM_BASE ) && ( oem_hook_req < QCRIL_EVT_OEM_MAX ) )
        {
          /* For OEM commands call out to qcrilhook */
          qcrilhook_oem( instance_id, param.event_id, data, datalen, t );
          return;
        }
      }
      else
      {
        /* The request is not supported */
        qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
        qcril_send_request_response( &resp );
        return;
      }
    }
    else
    {
      /* The request is not supported */
      qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
      qcril_send_request_response( &resp );
      return;
    }
  }

  /* Do a lookup for the entry */
  if ( qcril_hash_table_lookup( (uint32) param.event_id, &entry_ptr ) != E_SUCCESS )
  {
    /* The request is not supported */
    qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
    qcril_send_request_response( &resp );
    return;
  }

  QCRIL_ASSERT( entry_ptr != NULL );

  if ( qcril_dispatch_event( entry_ptr, &param ) == E_NOT_ALLOWED )
  {
    qcril_default_request_resp_params( param.instance_id, param.t, param.event_id, RIL_E_RADIO_NOT_AVAILABLE, &resp );
    qcril_send_request_response( &resp );
  }

} /* onRequest() */


/*===========================================================================

  FUNCTION:  onRequest_rid0

===========================================================================*/
/*!
    @brief
    Call from RIL instance RID0 to us to make a RIL_REQUEST
    Must be completed with a call to RIL_onRequestComplete()
    RIL_onRequestComplete() may be called from any thread, before or after
    this function returns.
    Returning from this routine implies the radio is ready to process another
    command (whether or not the previous command has completed).

    @return
    None.
*/
/*=========================================================================*/
static void onRequest_rid0
(
  int request,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  onRequest( QCRIL_DEFAULT_INSTANCE_ID, request, data, datalen, t );

} /* onRequest_rid0 */


#ifdef FEATURE_QCRIL_DSDS
/*===========================================================================

  FUNCTION:  onRequest_rid1

===========================================================================*/
/*!
    @brief
    Call from RIL instance RID1 to us to make a RIL_REQUEST
    Must be completed with a call to RIL_onRequestComplete()
    RIL_onRequestComplete() may be called from any thread, before or after
    this function returns.
    Returning from this routine implies the radio is ready to process another
    command (whether or not the previous command has completed).

    @return
    None.
*/
/*=========================================================================*/
static void onRequest_rid1
(
  int request,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  onRequest( QCRIL_SECOND_INSTANCE_ID, request, data, datalen, t );

} /* onRequest_rid1 */
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

  FUNCTION:  qcril_process_event

===========================================================================*/
/*!
    @brief
    Dispatches all RIL events except RIL_REQUESTs, which are handled by
    onRequest.  This routine may be called from multiple threads but it
    blocks on a single semaphore that is shared with onRequest such that
    only one event is handled at a time by the RIL.

    @return
    E_SUCCESS of the event was handled normally
    E_NOT_SUPPORTED if the event_id was invalid
    E_NOT_ALLOWED if the event is not supported in the current state

*/
/*=========================================================================*/
IxErrnoType qcril_process_event
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  IxErrnoType err_no;
  qcril_request_params_type params;
  qcril_dispatch_table_entry_type *entry_ptr; /*!< Dispatch table entry */
  char event_name[ 100 ] = "";;

  /*-----------------------------------------------------------------------*/

  QCRIL_SNPRINTF( event_name, sizeof( event_name ), "%s(%d), RID %d, MID %d", qcril_log_lookup_event_name( (int) event_id ),
                  event_id, instance_id, modem_id );

  if ( ( ( event_id > QCRIL_EVT_CM_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_CM_MAX ) ) ||
       ( ( event_id > QCRIL_EVT_SMS_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_SMS_MAX ) ) ||
       ( ( event_id > QCRIL_EVT_MMGSDI_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_MMGSDI_MAX ) ) ||
       ( ( event_id > QCRIL_EVT_GSTK_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_GSTK_MAX ) ) ||
       ( ( event_id > QCRIL_EVT_PBM_INTERNAL_BASE ) && ( event_id < QCRIL_EVT_PBM_MAX ) ) )
  {
    /* Internal events */
    QCRIL_LOG_CF_PKT_RIL_EVT( instance_id, event_name );
    QCRIL_LOG_DEBUG( "RIL --- %s ---> RIL\n", event_name );
  }
  else
  {
    /* AMSS events or callbacks */
    QCRIL_LOG_CF_PKT_MODEM_EVT( modem_id, event_name );

    /* Note: Below debug messages are used for profiling AMSS events/callbacks during screen on/off. Don't change them */
    if ( qcril_cm_screen_is_off( instance_id ) )
    {
      QCRIL_LOG_DEBUG( "RIL <=== %s === AMSS\n", event_name, event_id, instance_id, modem_id );
    }
    else
    {
      QCRIL_LOG_DEBUG( "RIL <--- %s --- AMSS\n", event_name, event_id, instance_id, modem_id );
    }
  }

  if (event_id <= QCRIL_EVT_BASE)
  {
    /* The event_id is out of range */
    return (E_NOT_SUPPORTED);
  }

  /* Do a lookup for the entry */
  if (qcril_hash_table_lookup( (uint32) event_id, &entry_ptr ) != E_SUCCESS)
  {
    /* The request is not supported */
    return (E_NOT_SUPPORTED);
  }

  QCRIL_ASSERT( entry_ptr != NULL );

  /* Initialize the input parameters for the request handler.  This allows for
     easy addition or deletion of parameters in the future. */
  params.instance_id = instance_id;
  params.modem_id = modem_id;
  params.event_id = (int) event_id;
  params.data = data;
  params.datalen = datalen;
  params.t = t;

  err_no = qcril_dispatch_event( entry_ptr, &params );

  QCRIL_LOG_INFO( "Exit qcril_process_event() : %s, err_no %d\n", qcril_log_lookup_event_name((int) event_id ), err_no );

  return err_no;

} /* qcril_process_event() */


/*===========================================================================

  FUNCTION:  currentState

===========================================================================*/
/*!
    @brief
    Return current radio state of an instance.

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static RIL_RadioState currentState
(
  qcril_instance_id_e_type instance_id
)
{
  qcril_arb_state_info_struct_type *s_ptr;
  char label[ 300 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  s_ptr = &qcril_state->info[ instance_id ];

  QCRIL_LOG_DEBUG( "RID %d currentState() -> %s(%d)\n", instance_id, radio_state_name[ s_ptr->radio_state ],
                   s_ptr->radio_state );

  QCRIL_SNPRINTF( label, sizeof( label ), "currentState() - %s", radio_state_name[ s_ptr->radio_state ] );
  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, label );

  return s_ptr->radio_state;

} /* currentState() */


/*===========================================================================

  FUNCTION:  currentState_rid0

===========================================================================*/
/*!
    @brief
    Synchronous call from the RIL to us to return current radio state of
    RIL instance RID0.
    RADIO_STATE_UNAVAILABLE should be the initial state.

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static RIL_RadioState currentState_rid0
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  return currentState( QCRIL_DEFAULT_INSTANCE_ID );

} /* currentState_rid0() */


#ifdef FEATURE_QCRIL_DSDS
/*===========================================================================

  FUNCTION:  currentState_rid1

===========================================================================*/
/*!
    @brief
    Synchronous call from the RIL to us to return current radio state of
    RIL instance RID1.
    RADIO_STATE_UNAVAILABLE should be the initial state.

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static RIL_RadioState currentState_rid1
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  return currentState( QCRIL_SECOND_INSTANCE_ID );

} /* currentState_rid1() */
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

  FUNCTION:  onSupports

===========================================================================*/
/*!
    @brief
    Determines if the RIL supports the given RIL_REQUEST by a RIL instance

    @return
    1 if the given RIL_REQUEST is supported
*/
/*=========================================================================*/
static int onSupports
(
  qcril_instance_id_e_type instance_id,
  int request
)
{
  int supported = 1;
  char label[ 80 ];

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  /* Do a lookup for the entry */
  if ( qcril_hash_table_lookup( (uint32) request, NULL ) != E_SUCCESS )
  {
    supported = 0;
  }

  QCRIL_LOG_DEBUG( "onSupports() ->: %s (%d), %s\n", qcril_log_lookup_event_name( request ), request,
                   (supported>0)?"Supported":"Not Supported");

  QCRIL_SNPRINTF( label, sizeof( label ), "onSupports() -` %s %s",
                  qcril_log_lookup_event_name( request ), ( supported > 0 ) ? "Supported" : "Not Supported" );
  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, label );

  return supported;

} /* onSupports() */


/*===========================================================================

  FUNCTION:  onSupports_rid0

===========================================================================*/
/*!
    @brief
    Determines if the RIL supports the given RIL_REQUEST by
    RIL instance RID0.

    @return
    1 if the given RIL_REQUEST is supported
*/
/*=========================================================================*/
static int onSupports_rid0
(
  int request
)
{
  /*-----------------------------------------------------------------------*/

  return onSupports( QCRIL_DEFAULT_INSTANCE_ID, request );

} /* onSupports_rid0() */


#ifdef FEATURE_QCRIL_DSDS
/*===========================================================================

  FUNCTION:  onSupports_rid1

===========================================================================*/
/*!
    @brief
    Determines if the RIL supports the given RIL_REQUEST by
    RIL instance RID1.

    @return
    1 if the given RIL_REQUEST is supported
*/
/*=========================================================================*/
static int onSupports_rid1
(
  int request
)
{
  /*-----------------------------------------------------------------------*/

  return onSupports( QCRIL_SECOND_INSTANCE_ID, request );

} /* onSupport_rid1() */
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

  FUNCTION:  onCancel

===========================================================================*/
/*!
    @brief
    The RIL is supposed to make a "best effort" to cancel the outstanding
    RIL_REQUEST with the given RIL_Token.  For now the hope is that out
    "best effort" can be "no effort".

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static void onCancel
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_NOTUSED( t );

  QCRIL_LOG_DEBUG( "%s", "onCancel()\n" );

  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, "onCancel()" );

} /* onCancel() */


/*===========================================================================

  FUNCTION:  onCancel_rid0

===========================================================================*/
/*!
    @brief
    The RIL is supposed to make a "best effort" to cancel the outstanding
    RIL_REQUEST with the given RIL_Token for RIL instance RID0.
    For now the hope is that out "best effort" can be "no effort".

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static void onCancel_rid0
(
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  onCancel( QCRIL_DEFAULT_INSTANCE_ID, t );

} /* onCancel_rid0()*/


#ifdef FEATURE_QCRIL_DSDS
/*===========================================================================

  FUNCTION:  onCancel_rid1

===========================================================================*/
/*!
    @brief
    The RIL is supposed to make a "best effort" to cancel the outstanding
    RIL_REQUEST with the given RIL_Token for RIL instance RID1.
    For now the hope is that out "best effort" can be "no effort".

    @return
    The current state of the RIL
*/
/*=========================================================================*/
static void onCancel_rid1
(
  RIL_Token t
)
{
  /*-----------------------------------------------------------------------*/

  onCancel( QCRIL_SECOND_INSTANCE_ID, t );

} /* onCancel_rid1()*/
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

  FUNCTION:  getVersion

===========================================================================*/
/*!
    @brief
    Used to query what version of the RIL is present

    @return
    A string describing this RIL version
*/
/*=========================================================================*/
static const char *getVersion
(
  qcril_instance_id_e_type instance_id
)
{
  char *version = "Qualcomm RIL 1.0";

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/


  QCRIL_LOG_DEBUG( "getVersion() -> %s\n", version );

  QCRIL_LOG_CF_PKT_RIL_FN( instance_id, "getVersion() - Qualcomm RIL 2.0" );

  return version;

} /* getVersion() */


/*===========================================================================

  FUNCTION:  getVersion_rid0

===========================================================================*/
/*!
    @brief
    Used to query what version of the RIL is present in RID0

    @return
    A string describing this RIL version
*/
/*=========================================================================*/
static const char *getVersion_rid0
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  return getVersion( QCRIL_DEFAULT_INSTANCE_ID );

} /* getVersion_rid0() */


#ifdef FEATURE_QCRIL_DSDS
/*===========================================================================

  FUNCTION:  getVersion_rid1

===========================================================================*/
/*!
    @brief
    Used to query what version of the RIL is present in RID1.

    @return
    A string describing this RIL version
*/
/*=========================================================================*/
static const char *getVersion_rid1
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  return getVersion( QCRIL_SECOND_INSTANCE_ID );

} /* getVersion_rid1() */
#endif /* FEATURE_QCRIL_DSDS */


/*===========================================================================

  FUNCTION:  qcril_init_hash_table

===========================================================================*/
/*!
    @brief
    Initializes the hash table of events

    @return
    None.
*/
/*=========================================================================*/
static void qcril_init_hash_table( void )
{
  uint32 reg_index, hash_index; /*!< index into hash table */
  qcril_dispatch_table_entry_type *temp_entry_ptr;

  /*-----------------------------------------------------------------------*/

  /* Initialize hash table */
  for (reg_index = 0; reg_index < QCRIL_ARR_SIZE( qcril_event_table ); reg_index++)
  {
    hash_index = qcril_hash( qcril_event_table[reg_index].event_id,
                             QCRIL_HT_ENTRIES_MAX, 0 );
    QCRIL_ASSERT( hash_index < QCRIL_HT_ENTRIES_MAX );

    if (qcril_hash_table[hash_index] == NULL)
    {
      /* No collision, just assign the new entry */
      qcril_hash_table[hash_index] = &qcril_event_table[reg_index];
    }
    else
    {
      /* Link the entry at the end of the collision list */
      temp_entry_ptr = qcril_hash_table[hash_index];

      while (temp_entry_ptr->next_ptr != NULL)
      {
        temp_entry_ptr = temp_entry_ptr->next_ptr;
      }
      temp_entry_ptr->next_ptr = &qcril_event_table[reg_index];
    }
  }

} /* qcril_init_hash_table() */


/*===========================================================================

  FUNCTION:  qcril_delay_timed_cb

===========================================================================*/
/*!
    @brief
    Handle delay timer expiration.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_delay_timed_cb
(
  void *param
)
{
  QCRIL_LOG_DEBUG( "Delay Timer expired with ID %d\n", (uint32) param );

}; /* qcril_delay_timed_cb */


/*===========================================================================

  FUNCTION:  qcril_init_state

===========================================================================*/
/*!
    @brief
    Initialize states of QCRIL.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_init_state
(
  void
)
{
  qcril_instance_id_e_type instance_id;
  uint32 timer_id;
  qcril_arb_state_info_struct_type *s_ptr;

  /*-----------------------------------------------------------------------*/

  /* Initialize TIMER ID */
  qcril_timer_id = 0;

  /* Allow cache */
  qcril_state = (qcril_arb_state_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_STATE );
  QCRIL_ASSERT( qcril_state != NULL );

  /* Initialize Timer ID mutex */
  pthread_mutex_init( &qcril_state->mutex, NULL );

  /* initialize Timed Callback list */
  pthread_mutex_init( &qcril_timed_callback_list_mutex, NULL);
  qcril_timed_callback_list = NULL;

  /* Initialize internal data */
  for ( instance_id = 0; instance_id < QCRIL_MAX_INSTANCE_ID; instance_id++ )
  {
    s_ptr = &qcril_state->info[ instance_id ];

    #ifdef FEATURE_QCRIL_SUBS_CTRL
    /* Unknown subscription config till Modem reports it */
    s_ptr->subscription_config_mask = 0;
    #endif /* FEATURE_QCRIL_SUBS_CTRL */

    /* Stay in Radio Unavailable State till Modem reports its mode band capability */
    s_ptr->radio_state = RADIO_STATE_UNAVAILABLE;

    /* Stay in Modem Unavailable State till Modem reports its mode band capability */
    s_ptr->modem_state = QCRIL_MODEM_STATE_UNAVAILABLE;

    /* Stay in GW SIM Not Ready State till Modem reports an update on GW SIM State */
    s_ptr->pri_gw_sim_state = QCRIL_SIM_STATE_NOT_READY;
    s_ptr->sec_gw_sim_state = QCRIL_SIM_STATE_NOT_READY;

    /* Stay in CDMA SIM Not Ready State till Modem reports an update on CDMA SIM State */
    s_ptr->pri_cdma_sim_state = QCRIL_SIM_STATE_NOT_READY;
    s_ptr->sec_cdma_sim_state = QCRIL_SIM_STATE_NOT_READY;

    /* Initialize acquired radio technology */
    s_ptr->voice_radio_tech = QCRIL_RADIO_TECH_NONE;

    /* Report changes in modem status and SIM status */
    QCRIL_LOG_DEBUG( "RID %d: RadioState Uninitialized --> %s [Event PowerUpInit]\n", instance_id,
                     radio_state_name[ s_ptr->radio_state ] );

    /* No RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED needed to be reported at power-up.
    ** RILD itself will invoke the unsol event when it's done register and query
    ** QCRIL for the Radio State. */
  }

  /* This is a workaround for a bug in ril.cpp where it starts ril_event_loop()
     before adding s_fdlisten to readFds.  When ril_event_loop first calls
     select() it is only waiting on s_fdWakeupRead.  Setting this timer wakes up
     the select, and when it blocks again s_fdlisten is in the fd_set.  Otherwise
     ril_event_loop() is blocked forever, even if Java connects to the socket. */
  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, qcril_delay_timed_cb,
                                         &TIMEVAL_DELAY, NULL );

} /* qcril_init_state */


/*===========================================================================

  FUNCTION:  qcril_init

===========================================================================*/
/*!
    @brief
    Initializes all QCRIL modules

    @return
    None.
*/
/*=========================================================================*/
void qcril_init
(
  void
)
{

  /*-----------------------------------------------------------------------*/

  /* Initialize QCRIL states */
  qcril_init_state();


  /* #########################################################################################################
                                           !!!IMPORTANT!!!
     #########################################################################################################

     (1) Use the state mutex to block QCRIL states update that could possibily triggered by any AMSS
         command callback, or AMSS event before the completion of radio state initialization.

     (2) Don't call qcril_process_event inside this block. Doing so, will end up in mutex deadlock.

     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                                            QCRIL STATES INITIALIZATION BEGIN
     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */


  QCRIL_MUTEX_LOCK( &qcril_state->mutex, "qcril_state_mutex" );

  /* Initialize QCRIL internal data structures */
  qcril_init_hash_table();

  qcril_reqlist_init();

  qcril_cm_clist_init();

  qcril_cm_ons_init();

  /* Initialize QCRIL as AMSS client */
  if ( qcril_cm_init() != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s", "Error in initializing QCRIL CM\n" );
  }

  if ( qcril_sms_init() != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s", "Failed to initialize QCRIL SMS\n" );
  }

  qcril_other_init();

  qcril_pbm_init();

  #ifdef FEATURE_QCRIL_UIM_QMI
  qcril_uim_init();
  #else
  qcril_mmgsdi_init();
  #endif /* FEATURE_QCRIL_UIM_QMI */

  #ifdef FEATURE_QCRIL_QMI_CAT
  qcril_gstk_qmi_init();
  #else
  qcril_gstk_init();
  #endif /* FEATURE_QCRIL_QMI_CAT */

  qcril_data_init();

  QCRIL_MUTEX_UNLOCK( &qcril_state->mutex, "qcril_state_mutex" );

  /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                                         QCRIL STATES INITIALIZATION END
     <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

  /* Get the modem capability */
  qcril_cm_get_modem_capability();

} /* qcril_init() */


/*===========================================================================

  FUNCTION:  qcril_rpc_init

===========================================================================*/
/*!
    @brief
    Handle RPC initialization

    @return
    none
*/
/*=========================================================================*/
void qcril_rpc_init
(
  void
)
{
  int i, retry_limit = 20;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_rpc_init()\n" );

  oncrpc_init();
  oncrpc_task_start();

  /* Sanity checks to make sure MSM RPC are working */
  for ( i = 0; i < retry_limit; i++ )
  {
    if ( cm_null() )
    {
      break;
    }
    else
    {
      QCRIL_LOG_ERROR( "Waiting for CM services count = %d\n", i );
    }
    sleep( 2 );
  }

  /* Initialize the RPC callbacks */
  cmcb_app_init();
  wmscb_app_init();
  pbmlibcb_app_init();

  #ifndef FEATURE_CDMA_NON_RUIM
  #ifndef FEATURE_QCRIL_UIM_QMI
  gsdi_expcb_app_init();
  mmgsdilibcb_app_init();
  #endif /* !FEATURE_QCRIL_UIM_QMI */
  #ifndef FEATURE_QCRIL_QMI_CAT
  gstk_expcb_app_init();
  #endif /* !FEATURE_QCRIL_QMI_CAT */
  #endif /* !FEATURE_CDMA_NON_RUIM */

  if ( !cm_null() )
  {
    QCRIL_LOG_ERROR( "%s", "CM RPC calls are not working!\n" );
  }

  if ( !pbmlib_null() )
  {
    QCRIL_LOG_ERROR( "%s", "PBM RPC calls are not working!\n" );
  }

  if ( !wms_null() )
  {
    QCRIL_LOG_ERROR( "%s", "WMS RPC calls are not working!\n" );
  }

  #ifndef FEATURE_CDMA_NON_RUIM
  #ifndef FEATURE_QCRIL_UIM_QMI
  if ( !gsdi_exp_null() )
  {
    QCRIL_LOG_ERROR( "%s", "GSDI RPC calls are not working!\n" );
  }

  if ( !mmgsdilib_null() )
  {
    QCRIL_LOG_ERROR( "%s", "MMGSDI RPC calls are not working!\n" );
  }

  if ( !gstk_exp_null() )
  {
    QCRIL_LOG_ERROR( "%s", "GSTK RPC calls are not working!\n" );
  }
  #endif /* FEATURE_QCIL_UIM_QMI */
  #endif /* !FEATURE_CDMA_NON_RUIM */

  if ( !nv_null() )
  {
    QCRIL_LOG_ERROR( "%s", "NV RPC calls are not working!\n" );
  }

  #ifdef FEATURE_QCRIL_SAR
  rfm_sarcb_app_init();
  if ( !rfm_sar_null() )
  {
    QCRIL_LOG_ERROR( "%s", "RFM_SAR RPC calls are not working!\n" );
  }
  #endif /* FEATURE_QCIL_SAR */

  #ifdef FEATURE_QCRIL_FUSION
  /* Sanity checks to make sure MSM RPC are working */
  for ( i = 0; i < retry_limit; i++ )
  {
    if ( cm_fusion_null() )
    {
      break;
    }
    else
    {
      QCRIL_LOG_ERROR( "Waiting for MDM CM services count = %d\n", i );
    }
    sleep( 2 );
  }

  /* Initialize the MDM RPC callbacks */
  cm_fusioncb_app_init();
  wms_fusioncb_app_init();
  pbmlib_fusioncb_app_init();

  /* Sanity checks to make sure MDM RPC are working */
  if ( !cm_fusion_null() )
  {
    QCRIL_LOG_ERROR( "%s", "MDM CM RPC calls are not working!\n");
  }

  if ( !pbmlib_fusion_null() )
  {
    QCRIL_LOG_ERROR( "%s", "MDM PBM RPC calls are not working!\n" );
  }

  if ( !wms_fusion_null() )
  {
    QCRIL_LOG_ERROR( "%s", "MDM WMS RPC calls are not working!\n");
  }
  #endif /* FEATURE_QCRIL_FUSION */

} /* qcril_rpc_init */


/*===========================================================================

  FUNCTION:  qcril_release

===========================================================================*/
/*!
    @brief
    Release AMSS client objects.

    @return
    None.
*/
/*=========================================================================*/
void qcril_release
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s", "qcril_rpc_release()\n" );

  qcril_cm_release();
  qcril_sms_release();
#ifdef FEATURE_QCRIL_UIM_QMI
  qcril_uim_release();
#else
  qcril_mmgsdi_release();
#endif /* FEATURE_QCRIL_UIM_QMI */
  qcril_pbm_release();

} /* qcril_release()*/


/*===========================================================================

  FUNCTION:  qcril_malloc

===========================================================================*/
/*!
    @brief
    Allocate memory from heap.

    @return
    Pointer to allocated memory region.
*/
/*=========================================================================*/
void *qcril_malloc
(
  size_t size
)
{
  void *mem_ptr;

  /*-----------------------------------------------------------------------*/

  mem_ptr = malloc( size );
  if ( mem_ptr != NULL )
  {
    memset( mem_ptr, 0, size );
  }
  else
  {
    QCRIL_LOG_ERROR( "%s", "Fail to allocate memory\n" );
  }

  return mem_ptr;

} /* qcril_malloc */


/*===========================================================================

  FUNCTION:  qcril_free

===========================================================================*/
/*!
    @brief
    Free specified memory region.

    @return
    None.
*/
/*=========================================================================*/
void qcril_free
(
  void *mem_ptr
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( mem_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  free( mem_ptr );

} /* qcril_free */

static int qcril_get_time_valid_args(qcril_time_base base, struct timeval *tv)
{
    return ( (base <= QCRIL_TIME_BASE_LAST) && (tv != NULL) );
}

int qcril_get_time(qcril_time_base base, struct timeval *tv)
{
  int ret = -1;
  if (qcril_get_time_valid_args(base, tv)) {
#ifdef FEATURE_QC_TIME_SERVICES
    int time_services_enabled = 0;
    char value[PROPERTY_VALUE_MAX];
    uint64_t seconds;

    if (property_get(TIME_SERVICES_PROPERTY, value, "false")) {
      time_services_enabled = !strncmp(value, "true", PROPERTY_VALUE_MAX);

      if (time_services_enabled) {
        time_genoff_info_type time_get;
        time_get.base = qcril2ats[base];
        time_get.unit = TIME_SECS;
        time_get.operation = T_GET;
        time_get.ts_val = (void *) &seconds;
        ret = time_genoff_operation(&time_get);
        if (!ret)
          tv->tv_sec = (__time_t)seconds;
      } else {
#endif
        ret = gettimeofday(tv, NULL);
#ifdef FEATURE_QC_TIME_SERVICES
      }
    }
#endif
  } else {
    QCRIL_LOG_DEBUG( "%s: Wrong parameters. Base: %d, tv: 0x%x\n", __func__, base, tv);
  }
  return ret;
}

#ifdef RIL_SHLIB
/*===========================================================================

  FUNCTION:  RIL_Init

===========================================================================*/
/*!
    @brief
    Returns the current state of the RIL

    @return
    The current state of the RIL
*/
/*=========================================================================*/
const RIL_RadioFunctions *RIL_Init
(
  const struct RIL_Env *env,
  int argc,
  char **argv
)
{
  int client_id = 0;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  /*-----------------------------------------------------------------------*/

  /* The first QCRIL instance initialize the logging and Arbitration module */
  if ( QCRIL_PRIMARY_INSTANCE( instance_id ) )
  {
    /* Initialize logging */
    qcril_log_init();

    /* Initialize the Arbitration module. Should be done before any other initialization */
    qcril_arb_init();
  }

  #ifdef FEATURE_QCRIL_DSDS
  if ( qcril_arb_ma_is_dsds() )
  {
      int opt = -1;
      optind = 1;
      while ( -1 != (opt = getopt(argc, argv, "p:d:s:c:"))) {
         switch (opt) {
             case 'c':
                 client_id = atoi(optarg);
                 QCRIL_LOG_DEBUG( "RIL client opt: %d, running RIL_Init()\n", client_id);
                 break;
             default:
                 QCRIL_LOG_DEBUG("RIL client opt ignored= %d\n",opt);
                 break;
         }
      }
    instance_id = ( qcril_instance_id_e_type ) client_id;
  }
  #endif /* FEATURE_QCRIL_DSDS */

  QCRIL_LOG_DEBUG( "RIL %d, client id %d, running RIL_Init()\n", instance_id, client_id );

  /* First of all, save the RILD response function pointers to have them ready for use */
  qcril_response_api[ instance_id ] = (struct RIL_Env *) env;

  /* The last QCRIL instance kick off the QCRIL initialization */
  if ( !qcril_arb_ma_is_dsds() || QCRIL_LAST_INSTANCE( instance_id ) )
  {
    /* Initialize the event thread */
    qcril_event_init();

    /* Initialize RPC */
    qcril_rpc_init();

    /* Initialize QCRIL */
    qcril_init();

    /* Start event thread */
    qcril_event_start();
  }

  return &qcril_request_api[ instance_id ];

} /* RIL_Init() */
#elif defined (PLATFORM_LTK)
int qcril_main()
{
  const RIL_RadioFunctions *rilapi = &qcril_api;

  /*-----------------------------------------------------------------------*/

  qcril_init();

  #ifndef FEATURE_CDMA_NON_RUIM
  qcril_mmgsdi_unit_test(rilapi);
  #endif /* FEATURE_CDMA_NON_RUIM */

} /* qcril_main() */
#else
int main(int argc, char *argv[])
{
  const RIL_RadioFunctions *rilapi = &qcril_api;

  /*-----------------------------------------------------------------------*/
  QCRIL_NOTUSED( argv );
  QCRIL_NOTUSED( argc );

  qcril_init();

  #ifndef FEATURE_CDMA_NON_RUIM
  qcril_mmgsdi_unit_test(rilapi);
  qcril_gstk_unit_test(rilapi);
  #endif /* FEATURE_CDMA_NON_RUIM */

  /* Verify QCRIL access function */
  qcril_unit_test( rilapi );

  /* Verify QCRIL Powerup Unit Test Suite */
  qcril_powerup_unit_test();

  /* Verify QCRIL State Transition Test Suite */
  qcril_state_transition_unit_test();

  /* Run ReqList Unit Test Suite */
  qcril_reqlist_unit_test();

  /* Run CM CList Unit Test Suite */
  qcril_cm_clist_unit_test();

  /* Run CM Phone Services Unit Test Suite */
  qcril_cm_phonesvc_unit_test( rilapi );

  /* Run CM Serving System Services Unit Test Suite */
  qcril_cm_srvsys_unit_test( rilapi );

  /* Run CM Call Services Unit Test Suite */
  qcril_cm_callsvc_unit_test( rilapi );

  /* Run CM Inband Services Unit Test Suite */
  qcril_cm_inband_unit_test( rilapi );

  /* Run Other Unit Test Suite */
  qcril_other_unit_test( rilapi );

  /* Run Oem Hook Unit Test Suite */
  qcril_oem_hook_unit_test( rilapi );

  return 0;

} /* qcril_main */
#endif /* RIL_SHLIB */
