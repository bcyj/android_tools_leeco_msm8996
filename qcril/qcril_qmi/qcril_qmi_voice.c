/******************************************************************************
  @file    qcril_qmi_voice.c
  @brief   qcril qmi - voice

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI VOICE.

  ---------------------------------------------------------------------------

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <errno.h>
#include <cutils/memory.h>
#include <cutils/properties.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include "ril.h"
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_reqlist.h"
#include "qcril_arb.h"
#include "qcril_qmi_voice.h"
#include "qcril_qmi_client.h"
#include "qcril_cm_ss.h"
#include "qcril_cmi.h"
#include "qcril_pbm.h"
#include "qcril_qmi_nas.h"
#include "qcril_qmi_ims_socket.h"
#include "qcril_qmi_ims_misc.h"
#include "qcril_qmi_pil_monitor.h"
#include "qcril_am.h"

// required for glibc compile
#include <limits.h>

#ifdef QMI_RIL_UTF
#include <netdb.h>
#endif

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
#define VOICE_NIL                     (0)
#define QCRIL_QMI_VOICE_CALLED_PARTY_BCD_NO_LENGTH       41
#define QCRIL_QMI_SUPS_SERVICE_PASSWORD_LENGTH 4

#define QCRIL_QMI_VOICE_SS_TA_UNKNOWN       129 /* 0x80|CM_TON_UNKNOWN      |CM_NPI_ISDN */
#define QCRIL_QMI_VOICE_SS_TA_INTERNATIONAL 145 /* 0x80|CM_TON_INTERNATIONAL|CM_NPI_ISDN */
#define QCRIL_QMI_VOICE_SS_TA_INTER_PREFIX  '+' /* ETSI international call dial prefix */

#define  QCRIL_QMI_VOICE_UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )
#define QCRIL_QMI_VOICE_MAX_USS_CHAR        182

#define QCRIL_QMI_VOICE_CLASS_NONE              0X00
#define QCRIL_QMI_VOICE_CLASS_VOICE             0X01
#define QCRIL_QMI_VOICE_CLASS_DATA              0X02
#define QCRIL_QMI_VOICE_CLASS_FAX               0X04
#define QCRIL_QMI_VOICE_CLASS_SMS               0X08
#define QCRIL_QMI_VOICE_CLASS_DATACIRCUITSYNC   0X10
#define QCRIL_QMI_VOICE_CLASS_DATACIRCUITASYNC  0X20
#define QCRIL_QMI_VOICE_CLASS_PACKETACCESS      0X40
#define QCRIL_QMI_VOICE_CLASS_PADACCESS         0X80

#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL        ( QCRIL_QMI_VOICE_CLASS_VOICE | QCRIL_QMI_VOICE_CLASS_FAX | QCRIL_QMI_VOICE_CLASS_SMS )
#define QCRIL_QMI_VOICE_TELESEFRVICE_TELEPHONY  ( QCRIL_QMI_VOICE_CLASS_VOICE  )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_DATA   ( QCRIL_QMI_VOICE_CLASS_FAX | QCRIL_QMI_VOICE_CLASS_SMS )
#define QCRIL_QMI_VOICE_TELESEFRVICE_FAX        ( QCRIL_QMI_VOICE_CLASS_FAX )
#define QCRIL_QMI_VOICE_TELESEFRVICE_SMS        ( QCRIL_QMI_VOICE_CLASS_SMS )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_EXCEPT_SMS  ( QCRIL_QMI_VOICE_CLASS_VOICE | QCRIL_QMI_VOICE_CLASS_SMS )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_BEARER_SVC  ( QCRIL_QMI_VOICE_CLASS_DATACIRCUITSYNC | QCRIL_QMI_VOICE_CLASS_DATACIRCUITASYNC )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_ASYNC_SVC   ( QCRIL_QMI_VOICE_CLASS_DATACIRCUITASYNC | QCRIL_QMI_VOICE_CLASS_PADACCESS )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_SYNC_SVC    ( QCRIL_QMI_VOICE_CLASS_DATACIRCUITSYNC | QCRIL_QMI_VOICE_CLASS_PACKETACCESS )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_DATA_CIRCUIT_SYNC_SVC    ( QCRIL_QMI_VOICE_CLASS_DATACIRCUITSYNC )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_DATA_CIRCUIT_ASYNC_SVC    ( QCRIL_QMI_VOICE_CLASS_DATACIRCUITASYNC )
#define QCRIL_QMI_VOICE_TELESEFRVICE_TELEPHONY_AND_ALL_SYNC_SVC    ( QCRIL_QMI_VOICE_CLASS_VOICE | QCRIL_QMI_VOICE_CLASS_DATACIRCUITSYNC )
#define QCRIL_QMI_VOICE_TELESEFRVICE_ALL_GPRS_BEARER_SVC    ( QCRIL_QMI_VOICE_CLASS_PADACCESS )

#define QCRIL_QMI_VOICE_SERVICE_STATUS_INACTIVE    0x00
#define QCRIL_QMI_VOICE_SERVICE_STATUS_ACTIVE      0x01

#define QCRIL_QMI_VOICE_RIL_CF_STATUS_NOT_ACTIVE    0
#define QCRIL_QMI_VOICE_RIL_CF_STATUS_ACTIVE        1

#define QCRIL_QMI_VOICE_RIL_CF_STATUS_DISABLE       0
#define QCRIL_QMI_VOICE_RIL_CF_STATUS_ENABLE        1
#define QCRIL_QMI_VOICE_RIL_CF_STATUS_INTERROGATE   2
#define QCRIL_QMI_VOICE_RIL_CF_STATUS_REGISTRATION  3
#define QCRIL_QMI_VOICE_RIL_CF_STATUS_ERASURE       4

#define QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER_PREFIX  '+'
#define QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER         145
#define QCRIL_QMI_VOICE_DOMESTIC_NUMBER              129

/* OEM HOOK DTMF forward burst payload length (72 bytes)
on_length   : 4 bytes
off_length  : 4 bytes
dtmf_digits: 64 bytes */
#define QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH 72
#define QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH 5

#define QCRIL_QMI_MAX_PENDING_MNG_CALL_REQUESTS 3


#define QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ   7

#define QCRIL_QMI_VOICE_SPEECH_CODEC_BACKLOG            10
#define QCRIL_QMI_VOICE_SPEECH_CODEC_PORT               "5001"
#define QCRIL_QMI_VOICE_SPEECH_CODEC_BUFFER_MAX_SIZE    16
#define SPEECH_CODEC_LOCK()      pthread_mutex_lock(&qcril_qmi_voice_speech_codec_info.speech_codec_mutex);
#define SPEECH_CODEC_UNLOCK()    pthread_mutex_unlock(&qcril_qmi_voice_speech_codec_info.speech_codec_mutex);
#define SPEECH_CODEC_WAIT()      qcril_qmi_voice_speech_codec_condition_wait_helper();
#define SPEECH_CODEC_SIGNAL()    pthread_cond_signal(&qcril_qmi_voice_speech_codec_info.speech_codec_cond_var);

/* First Call Barring Facility Type Ims__SuppSvcFacilityType */
#define IMS__SUPP_SVC_FACILITY_TYPE__CB_MIN IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOC

/* Last Call Barring Facility Type Ims__SuppSvcFacilityType */
#define IMS__SUPP_SVC_FACILITY_TYPE__CB_MAX IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MT

/* Min Facility Type Ims__SuppSvcFacilityType */
#define IMS__SUPP_SVC_FACILITY_TYPE__MIN IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_CLIP

/* Max Facility Type Ims__SuppSvcFacilityType */
#define IMS__SUPP_SVC_FACILITY_TYPE__MAX IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICa

/* Max Operation Type Ims__SuppSvcOperationType */
#define IMS__SUPP_SVC_OPERATION_TYPE__MAX IMS__SUPP_SVC_OPERATION_TYPE__QUERY

/*===========================================================================

                    QMI VOICE GLOBALS

===========================================================================*/

static qcril_qmi_voice_info_type  qcril_qmi_voice_info;

static qcril_qmi_pending_emergency_call_info_type qcril_qmi_pending_emergency_call_info;

voice_dial_call_resp_msg_v02               dial_call_resp_msg;

static qcril_qmi_voice_voip_overview_type qmi_voice_voip_overview;

// STK CC info
qcril_qmi_voice_stk_cc_info_type              stk_cc_info;

int qmi_ril_voice_is_voice_calls_supressed  = FALSE;

call_type_enum_v02 qcril_qmi_voice_cdma_call_type_to_be_considered;

RIL_Call_Details* __vt_volte_emulation_call_details_dummy_ptr;

qcril_qmi_voice_speech_codec_info_type qcril_qmi_voice_speech_codec_info;

static int feature_subaddress_support = 0;

static int feature_subaddress_support_amp = 1;

static int reject_cause_21_supported = FALSE;

static int feature_subaddress_ia5_id_support = 0;

static int feature_redir_party_num_support = 1;

static int dtmf_rtp_event_interval = QCRIL_QMI_VOICE_DTMF_INTERVAL_VAL;

static boolean disabled_screen_off_ind = FALSE;

static void qcril_qmi_voice_request_set_supp_svc
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

static void qcril_qmi_voice_request_query_colp
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

static void qcril_qmi_voice_send_ims_unsol_resp_handover(RIL_SrvccState ril_srvccstate);

/*  Function handler table for QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS
    Handler functions for operations IMS__SUPP_SVC_OPERATION_TYPE__ACTIVATE
    and IMS__SUPP_SVC_OPERATION_TYPE__DEACTIVATE for each SuppSvcFacilityType.
*/
static qcril_req_handler_type * supp_srv_status_set_handler_table\
[IMS__SUPP_SVC_FACILITY_TYPE__MAX + 1] =
{
  NULL,                             /* Invalid facility type value = 0 */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_CLIP = 1   */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_COLP = 2   */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BAOC = 3   */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BAOIC = 4  */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BAOICxH = 5 */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BAIC = 6   */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BAICr = 7  */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BA_ALL = 8 */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BA_MO = 9  */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BA_MT = 10 */
  qcril_qmi_voice_request_set_supp_svc,       /* FACILITY_BS_MT = 11 */
  qcril_qmi_voice_request_set_supp_svc        /* FACILITY_BAICa = 12 */
};

/*  Function handler table for QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS
    Handler function for IMS__SUPP_SVC_OPERATION_TYPE__QUERY for each
    SuppSvcFacilityType.
*/
static qcril_req_handler_type * supp_srv_status_query_handler_table\
[IMS__SUPP_SVC_FACILITY_TYPE__MAX + 1] =
{
  NULL,                             /* Invalid facility type value = 0 */
  NULL,                   /* Query not supported for FACILITY_CLIP = 1 */
                          /* use REQUEST_QUERY_CALL_CLIP               */
  qcril_qmi_voice_request_query_colp,           /* FACILITY_COLP = 2   */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BAOC = 3   */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BAOIC = 4  */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BAOICxH = 5 */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BAIC = 6   */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BAICr = 7  */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BA_ALL = 8 */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BA_MO = 9  */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BA_MT = 10 */
  qcril_qmi_voice_request_query_facility_lock,  /* FACILITY_BS_MT = 11 */
  qcril_qmi_voice_request_query_facility_lock   /* FACILITY_BAICa = 12 */
};

/* Lookup table to convert IMS facility of type Ims__SuppSvcFacilityType
    to QCRIL facility of type qcril_qmi_voice_facility_e_type.
*/
static qcril_qmi_voice_facility_e_type
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__MAX + 1];

/* Enum consisting of constants to determine
   when the conference is established.
   Fixed Enum do-not add any items.
*/
typedef enum {
  QCRIL_QMI_IMS_VOICE_CONF_NO_CONF     = 0x00, /*used when there is no conference*/
  QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS = 0x0F, /*used when conference is in progress*/
  QCRIL_QMI_IMS_VOICE_CONF_ESTABLISHED = 0xFF /*used when conference is established*/
}qcril_qmi_ims_voice_conf_state;

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

static void qcril_qmi_voice_handle_mng_call_req
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int event_id,
  voice_manage_calls_req_msg_v02*  manage_calls_req,
  uint32 user_data,
  boolean send_async_ex
);

static void qcril_qmi_voice_send_management_call_request
(
  qcril_instance_id_e_type instance_id,
  struct qcril_reqlist_buf_tag *req,
  void *data,
  size_t datalen
);

static RIL_Errno qcril_qmi_voice_post_manage_voip_calls_request( voip_sups_type_enum_v02 request, uint16 req_id  );

static void qcril_qmi_voice_store_last_call_failure_cause(RIL_LastCallFailCause reason);
static void qcril_qmi_voice_invalid_last_call_failure_cause();
static void qcril_qmi_voice_respond_ril_last_call_failure_request();
static void qcril_qmi_voice_handle_new_last_call_failure_cause(int reason, boolean is_qmi_reason, qcril_qmi_voice_voip_call_info_entry_type *call_obj);
static int qcril_qmi_voice_nas_control_is_call_mode_reported_voice_radio_tech_different(call_mode_enum_v02 call_mode);
static RIL_RadioTechnologyFamily qcril_qmi_voice_nas_control_get_reported_voice_radio_tech();
static unsigned int qcril_qmi_voice_convert_call_mode_to_radio_tech_family(call_mode_enum_v02 call_mode);
static void qcril_qmi_voice_make_incoming_call_ring (qcril_timed_callback_handler_params_type *param);
static boolean qcril_qmi_voice_create_mpty_voip_call_vcl(call_state_enum_v02 *current_call_state);
static boolean qcril_qmi_voice_add_call_to_existing_mpty_voip_call_vcl(qcril_qmi_voice_voip_call_info_entry_type* call_entry_ptr);
static void qcril_qmi_voice_add_call_to_existing_mpty_voip_call_failure_cleanup_vcl();
static void qcril_qmi_voice_nas_control_set_current_calls_number(unsigned int number_of_calls);
static void qcril_qmi_voice_auto_answer_timeout_handler( void *param );
static int qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate( void );
static void qmi_ril_voice_extended_dialing_over( void );
static void qmi_ril_voice_drop_homeless_incall_reqs( void );
static void qmi_ril_voice_drop_homeless_incall_reqs_main_threaded(void * param);
static void qmi_ril_voice_cleanup_reqs_after_call_completion(void);
static void qmi_ril_voice_cleanup_reqs_after_call_completion_main_threaded(void * param);
uint32_t qcril_qmi_voice_call_num_copy_with_toa_check(char *src, uint32_t src_size, char* dest,
                                                      uint32_t dest_buffer_size, voice_num_type_enum_v02 num_type);

static void qcril_qmi_voice_create_emer_voice_entry(qcril_qmi_voice_emer_voice_feature_info_type * emer_voice_number, voice_remote_party_number2_type_v02 * remote_party_number);
static int qcril_qmi_voice_is_emer_voice_entry_valid(qcril_qmi_voice_emer_voice_feature_info_type * emer_voice_number);
static int qcril_qmi_voice_handle_ril_call_entry(RIL_Call *info_ptr);
static int qcril_qmi_voice_is_cdma_voice_emergency_calls_present(qcril_qmi_voice_voip_call_info_entry_type **cdma_voice_call_info_entry,
                                                                 qcril_qmi_voice_voip_call_info_entry_type **cdma_no_srv_emer_call_info_entry);


static void qcril_qmi_voice_respond_pending_hangup_ril_response(uint8_t call_id);
static void qmi_ril_voice_pending_1x_num_timeout(void * param);

static void qcril_qmi_voice_set_last_call_fail_request_timeout();
static void qcril_qmi_voice_last_call_fail_request_timeout_handler(void *param);
static void qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( qcril_qmi_voice_voip_call_info_entry_type* entry );

static void qmi_ril_succeed_on_pending_hangup_req_on_no_calls_left(void);

static void qcril_qmi_voice_transfer_sim_ucs2_alpha_to_std_ucs2_alpha(const voice_alpha_ident_type_v02 *sim_alpha, voice_alpha_ident_type_v02 *std_alpha);

static void qcril_qmi_voice_ims_send_unsol_ringback_tone(boolean local_ringback_payload);

static void qmi_ril_voice_revoke_kill_immunity_priveledge( void );

static void qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl( void );

static void qmi_ril_voice_review_call_objs_after_last_call_failure_response_vcl( void );

static void qmi_ril_voice_ended_call_obj_phase_out(void * param);

static boolean qcril_qmi_voice_call_to_ims(const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry);
static boolean qcril_qmi_voice_call_to_atel(const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry);

static void qmi_ril_voice_ims_command_oversight_timeout_handler(void * param);

static void qcril_qmi_voice_ims_conf_req_state_reset_vcl();
static void qcril_qmi_voice_ims_conf_req_state_start_vcl();
static void qcril_qmi_voice_ims_conf_add_call_req_state_start_vcl();
static qcril_qmi_voice_ims_conf_req_state qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl();
static void qcril_qmi_voice_set_ims_conf_req_txn_state_vcl(qcril_qmi_voice_ims_conf_req_state state);
static void qcril_qmi_voice_set_ims_conf_call_req_txn_state_to_next_vcl();

static boolean qmi_ril_voice_is_calls_supressed_by_pil_vcl();
static void qcril_qmi_voice_handle_pil_state_changed(const qcril_qmi_pil_state* cur_state);
static void qcril_qmi_voice_hangup_all_non_emergency_calls_vcl();
static void qcril_qmi_voice_ims_dial_call_handler (qcril_timed_callback_handler_params_type *param);

static boolean qcril_qmi_voice_get_atel_call_type_info_by_call_info
(
   const qcril_qmi_voice_voip_call_info_entry_type *call_info,
   RIL_Call_Details *call_details
);

static boolean qcril_qmi_voice_get_atel_call_type_info
(
   call_type_enum_v02 call_type,
   boolean video_attrib_valid,
   voice_call_attribute_type_mask_v02 video_attrib,
   boolean audio_attrib_valid,
   voice_call_attribute_type_mask_v02 audio_attrib,
   boolean attrib_status_valid,
   voice_call_attrib_status_enum_v02 attrib_status,
   boolean call_info_elab_valid,
   qcril_qmi_voice_voip_call_info_elaboration_type call_info_elab,
   boolean cached_call_type_valid,
   Ims__CallType cached_call_type,
   RIL_Call_Details *call_details
);

static boolean qcril_qmi_voice_get_modem_call_type_info
(
   RIL_Call_Details                      *call_details,
   call_type_enum_v02                    *call_type,
   uint8_t                               *audio_attrib_valid,
   voice_call_attribute_type_mask_v02    *audio_attrib,
   uint8_t                               *video_attrib_valid,
   voice_call_attribute_type_mask_v02    *video_attrib
);

void qcril_qmi_voice_reset_conf_info_xml();
static RIL_Errno qcril_qmi_voice_send_request_answer(
  const qcril_request_params_type             *const params_ptr,
  int                                         reject_this_call
);

void qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp
(
   qmi_sups_errors_enum_v02 sups_failure_cause,
   uint8_t call_id
);

static boolean qcril_qmi_voice_is_ims_send_calls(int event_id);
static void qcril_qmi_voice_send_ims_unsol_call_state_changed();

void qcril_qmi_voice_set_audio_call_type
(
    const voice_call_info2_type_v02* iter_call_info,
    qcril_qmi_voice_voip_call_info_entry_type *call_info_entry
);

void qcril_qmi_voice_send_hangup_on_call
(
    int conn_index
);

void qcril_qmi_voice_get_colp_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
);


static void qcril_initialize_ims_to_ril_facility_lookup_table( void )
{
  /* Unused IMS Facility Type value = 0 */
  qcril_ims_to_ril_facility_lookup_table[0] =
    QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_CLIP = 1 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_CLIP] =
    QCRIL_QMI_VOICE_FACILITY_CLIP;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_COLP = 2 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_COLP] =
    QCRIL_QMI_VOICE_FACILITY_COLP;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOC = 3 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOC] =
    QCRIL_QMI_VOICE_CB_FACILITY_ALLOUTGOING;

  /*  IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOIC = 4 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOIC] =
    QCRIL_QMI_VOICE_CB_FACILITY_OUTGOINGINT;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOICxH = 5 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAOICxH]
    = QCRIL_QMI_VOICE_CB_FACILITY_OUTGOINGINTEXTOHOME;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAIC = 6 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAIC] =
    QCRIL_QMI_VOICE_CB_FACILITY_ALLINCOMING;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICr = 7 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICr] =
    QCRIL_QMI_VOICE_CB_FACILITY_INCOMINGROAMING;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_ALL = 8 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_ALL] =
    QCRIL_QMI_VOICE_CB_FACILITY_ALLBARRING;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MO = 9 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MO] =
    QCRIL_QMI_VOICE_CB_FACILITY_ALLOUTGOINGBARRING;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MT = 10 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BA_MT] =
    QCRIL_QMI_VOICE_CB_FACILITY_ALLINCOMINGBARRING;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BS_MT = 11 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BS_MT] =
    VOICE_REASON_BARR_INCOMING_NUMBER_V02;

  /* IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICa = 12 */
  qcril_ims_to_ril_facility_lookup_table[IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BAICa] =
    VOICE_REASON_BARR_INCOMING_ANONYMOUS_V02;

};

static void qcril_qmi_voice_setup_answer_resp_hdlr
(
   const qcril_request_params_type *const params_ptr
);

static qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state(call_state_enum_v02 qmi_call_state);
static RIL_Errno qcril_qmi_voice_perform_null_check_and_reqlist_entry
(
   const qcril_request_params_type *const params_ptr,
   uint32_t *user_data_ptr,
   qcril_reqlist_public_type *reqlist_entry_ptr
);
static RIL_Errno qcril_qmi_voice_gather_current_call_information
(
   unsigned int iter,
   const qcril_request_params_type *const params_ptr,
   qcril_qmi_voice_current_calls_type *payload_ptr,
   const qcril_qmi_voice_voip_call_info_entry_type *const call_info_entry
);

//===========================================================================
// qcril_qmi_voice_pre_init
//===========================================================================
RIL_Errno qcril_qmi_voice_pre_init(void)
{
  RIL_Errno res = RIL_E_GENERIC_FAILURE;
  pthread_mutexattr_t mtx_atr;
  char args[ PROPERTY_VALUE_MAX ];
  int len;
  char *end_ptr;
  unsigned long ret_val;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    memset( &qcril_qmi_voice_info, 0, sizeof(qcril_qmi_voice_info));

#ifdef QCRIL_PROTOBUF_BUILD_ENABLED
    qcril_qmi_voice_info.jbims = TRUE;
#else
    qcril_qmi_voice_info.jbims = FALSE;
#endif

    QCRIL_LOG_INFO("qcril_qmi_voice_info.jbims set to: %d", qcril_qmi_voice_info.jbims);

    property_get( QCRIL_PROCESS_SUPS_IND, args, "" );

    len = strlen( args );
    if ( len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 );
      if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
      {
        QCRIL_LOG_ERROR( "Fail to convert process_sups_ind setting %s", args );
      }
      else if ( ret_val > 1 )
      {
        QCRIL_LOG_ERROR( "Invalid saved process_sups_ind setting %ld, use default", ret_val );
      }
      else
      {
        qcril_qmi_voice_info.process_sups_ind = ( boolean ) ret_val;
      }
    }

    QCRIL_LOG_INFO("qcril_qmi_voice_info.process_sups_ind set to: %d", qcril_qmi_voice_info.process_sups_ind);

    property_get( QCRIL_INFORCE_0X9E_TREAT_AS_TAG, args, "" );

    len = strlen( args );
    if ( len > 0 )
    {
      ret_val = strtoul( args, &end_ptr, 0 );
      if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
      {
        QCRIL_LOG_ERROR( "Fail to convert is_0x9e_not_treat_as_name setting %s", args );
      }
      else if ( ret_val > 1 )
      {
        QCRIL_LOG_ERROR( "Invalid saved is_0x9e_not_treat_as_name setting %ld, use default", ret_val );
      }
      else
      {
        qcril_qmi_voice_info.is_0x9e_not_treat_as_name = ( boolean ) ret_val;
      }
    }
    QCRIL_LOG_INFO("qcril_qmi_voice_info.is_0x9e_not_treat_as_name set to: %d", qcril_qmi_voice_info.is_0x9e_not_treat_as_name);

    pthread_mutexattr_init( &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex_atr );
    pthread_mutex_init( &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex,
                        &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex_atr );

    // voip
    memset( &qmi_voice_voip_overview, 0, sizeof( qmi_voice_voip_overview ) );
    pthread_mutexattr_init( &qmi_voice_voip_overview.overview_lock_mtx_atr );
    pthread_mutexattr_settype( &qmi_voice_voip_overview.overview_lock_mtx_atr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &qmi_voice_voip_overview.overview_lock_mutex, &qmi_voice_voip_overview.overview_lock_mtx_atr );

    //emergency call init
    qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data = malloc(sizeof(RIL_Dial));
    if( NULL == qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data )
    {
      QCRIL_LOG_ERROR("malloc failed");
      break;
    }
    memset(qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data, 0, sizeof(RIL_Dial));
    ((RIL_Dial*) qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data)->address = malloc(QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN);
    if( NULL == ((RIL_Dial*) qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data)->address )
    {
      QCRIL_LOG_ERROR("malloc failed");
      break;
    }

    pthread_mutexattr_init( &qcril_qmi_voice_info.voice_info_lock_mutex_atr );
    pthread_mutex_init( &qcril_qmi_voice_info.voice_info_lock_mutex, &qcril_qmi_voice_info.voice_info_lock_mutex_atr );

    res = RIL_E_SUCCESS;

  } while (FALSE);

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int) res );

  return res;
} // void

//===========================================================================
// qcril_qmi_voice_ind_registrations
//===========================================================================

void qcril_qmi_voice_ind_registrations
(
  void
)
{
  voice_indication_register_req_msg_v02  indication_req;
  voice_indication_register_resp_msg_v02 indication_resp_msg;

  QCRIL_LOG_FUNC_ENTRY();

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));

  indication_req.reg_voice_privacy_events_valid = TRUE;
  indication_req.reg_voice_privacy_events = 0x01;

  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                      &indication_req,
                                      sizeof(indication_req),
                                      &indication_resp_msg,
                                      sizeof(indication_resp_msg)
                                    ) !=E_SUCCESS )
  {
    QCRIL_LOG_INFO("Voice_privacy events indication register failed!");
  }
  else
  {
    QCRIL_LOG_INFO("Voice_privacy events registration error code: %d", indication_resp_msg.resp.error);
  }

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));

  indication_req.ext_brst_intl_events_valid = TRUE;
  indication_req.ext_brst_intl_events = 0x01;

  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                      &indication_req,
                                      sizeof(indication_req),
                                      &indication_resp_msg,
                                      sizeof(indication_resp_msg)
                                    ) !=E_SUCCESS )
  {
    QCRIL_LOG_INFO("Extended_burst events indication register failed!");
  }
  else
  {
    QCRIL_LOG_INFO("Extended_burst events registration error code: %d", indication_resp_msg.resp.error);
  }

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));

  indication_req.speech_events_valid = TRUE;
  indication_req.speech_events = 0x01;

  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                      &indication_req,
                                      sizeof(indication_req),
                                      &indication_resp_msg,
                                      sizeof(indication_resp_msg)
                                    ) !=E_SUCCESS )
  {
    QCRIL_LOG_INFO("Speech events indication register failed!");
  }
  else
  {
    QCRIL_LOG_INFO("Speech events registration error code: %d", indication_resp_msg.resp.error);
  }

  if (qcril_qmi_voice_info.jbims)
  {
    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
    indication_req.handover_events_valid = TRUE;
    indication_req.handover_events = 0x01;
    if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                        &indication_req,
                                        sizeof(indication_req),
                                        &indication_resp_msg,
                                        sizeof(indication_resp_msg)
                                      ) !=E_SUCCESS )
    {
      QCRIL_LOG_INFO("Handover events indication register failed!");
    }
    else
    {
      QCRIL_LOG_INFO("Handover events registration error code: %d", indication_resp_msg.resp.error);
    }

    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
    indication_req.conference_events_valid = TRUE;
    indication_req.conference_events = 0x01;
    if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                        &indication_req,
                                        sizeof(indication_req),
                                        &indication_resp_msg,
                                        sizeof(indication_resp_msg)
                                      ) !=E_SUCCESS )
    {
      QCRIL_LOG_INFO("Conference events indication register failed!");
    }
    else
    {
      QCRIL_LOG_INFO("Conference events registration error code: %d", indication_resp_msg.resp.error);
    }

    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
    indication_req.tty_info_events_valid = TRUE;
    indication_req.tty_info_events = 0x01;
    if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                        &indication_req,
                                        sizeof(indication_req),
                                        &indication_resp_msg,
                                        sizeof(indication_resp_msg)
                                      ) !=E_SUCCESS )
    {
      QCRIL_LOG_INFO("tty_info events indication register failed!");
    }
    else
    {
      QCRIL_LOG_INFO("tty_info events registration error code: %d", indication_resp_msg.resp.error);
    }
  }

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
  indication_req.cc_result_events_valid = TRUE;
  indication_req.cc_result_events = 0x01;
  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                       QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                       &indication_req,
                                       sizeof(indication_req),
                                       &indication_resp_msg,
                                       sizeof(indication_resp_msg)
                                     ) !=E_SUCCESS )
  {
     QCRIL_LOG_INFO("cc_result events indication register failed!");
  }
  else
  {
     QCRIL_LOG_INFO("cc_result events registration error code: %d", indication_resp_msg.resp.error);
  }

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
  indication_req.additional_call_info_events_valid = TRUE;
  indication_req.additional_call_info_events = 0x01;
  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                       QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                       &indication_req,
                                       sizeof(indication_req),
                                       &indication_resp_msg,
                                       sizeof(indication_resp_msg)
                                     ) !=E_SUCCESS )
  {
     QCRIL_LOG_INFO("additional_call_info events indication register failed!");
  }
  else
  {
     QCRIL_LOG_INFO("additional_call_info events registration error code: %d",
                    indication_resp_msg.resp.error);
  }

  memset(&indication_req, 0, sizeof(indication_req));
  memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
  indication_req.audio_rat_change_events_valid = TRUE;
  indication_req.audio_rat_change_events = 0x01;
  if ( qcril_qmi_client_send_msg_sync( QCRIL_QMI_CLIENT_VOICE,
                                       QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                       &indication_req,
                                       sizeof(indication_req),
                                       &indication_resp_msg,
                                       sizeof(indication_resp_msg)
                                     ) !=E_SUCCESS )
  {
     QCRIL_LOG_INFO("audio_rat_change events indication register failed!");
  }
  else
  {
     QCRIL_LOG_INFO("audio_rat_change events registration error code: %d",
                    indication_resp_msg.resp.error);
  }

  disabled_screen_off_ind = FALSE;

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_ind_registrations

//===========================================================================
// qcril_qmi_voice_is_active_ims_call
//===========================================================================
static boolean qcril_qmi_voice_is_active_ims_call
(
 const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry
)
{
  boolean ret = FALSE;

  if (call_info_entry)
  {
    ret = (qcril_qmi_voice_call_to_ims(call_info_entry) &&
            (call_info_entry->voice_scv_info.call_state != CALL_STATE_END_V02));
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET((int)ret);
  return ret;
} // qcril_qmi_voice_is_active_ims_call

//===========================================================================
// qcril_qmi_voice_toggle_ind_reg_on_screen_state
//===========================================================================
void qcril_qmi_voice_toggle_ind_reg_on_screen_state
(
  boolean enable
)
{
  boolean need_to_register = FALSE;
  voice_indication_register_req_msg_v02  indication_req;
  voice_indication_register_resp_msg_v02 indication_resp_msg;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_INFO("disabled_screen_off_ind = %d\n", disabled_screen_off_ind);

  if (enable == FALSE)
  {
    // De-register for the indications if PS calls are not present
    if (!disabled_screen_off_ind)
    {
      need_to_register = !qcril_qmi_voice_has_specific_call(qcril_qmi_voice_is_active_ims_call);
      disabled_screen_off_ind = need_to_register;
    }
  }
  else
  {
    // Register again only if previously de-registered the indications
    need_to_register = disabled_screen_off_ind;
    disabled_screen_off_ind = FALSE;
  }

  QCRIL_LOG_INFO("enable = %d, need_to_register = %d, disabled_screen_off_ind = %d\n",
                 enable, need_to_register, disabled_screen_off_ind);

  if (need_to_register)
  {
    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));

    indication_req.speech_events_valid = TRUE;
    indication_req.speech_events = enable;

    indication_req.handover_events_valid = TRUE;
    indication_req.handover_events = enable;

    if (qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_VOICE,
                                       QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                       &indication_req,
                                       sizeof(indication_req),
                                       &indication_resp_msg,
                                       sizeof(indication_resp_msg)) != E_SUCCESS)
    {
      QCRIL_LOG_INFO("Indication register failed!");
    }
    else
    {
      QCRIL_LOG_INFO("Events registration error code: %d", indication_resp_msg.resp.error);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_toggle_ind_reg_on_screen_state


//===========================================================================
// qcril_qmi_voice_init
//===========================================================================
RIL_Errno qcril_qmi_voice_init
(
  void
)
{
  char property_name[ 40 ];
  char args[ PROPERTY_VALUE_MAX ];
  int property_param_len;
  char *end_ptr;
  unsigned long ret_val = 0;
  RIL_Errno res = RIL_E_SUCCESS;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_ind_registrations();

  /* Default CLIR */
  qcril_qmi_voice_info.clir = ( uint8 ) QCRIL_QMI_VOICE_SS_CLIR_PRESENTATION_INDICATOR;

  /* Use saved CLIR setting if available */
  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s%d", QCRIL_QMI_VOICE_CLIR,qmi_ril_get_process_instance_id());
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert CLIR %s", args );
    }
    else if ( ret_val > QCRIL_QMI_VOICE_SS_CLIR_SUPPRESSION_OPTION )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Invalid saved CLIR %ld, use default", ret_val );
    }
    else
    {
      qcril_qmi_voice_info.clir = ( uint8 ) ret_val;
    }
  }
  QCRIL_LOG_DEBUG( "CLIR=%d", qcril_qmi_voice_info.clir );

  /* Default Auto Answer timerID */
  qmi_voice_voip_overview.auto_answer_timer_id = QMI_RIL_ZERO;

  /* Default Speech Codec */
  qcril_qmi_voice_speech_codec_info.report_speech_codec = FALSE;

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_QMI_VOICE_REPORT_SPEECH_CODEC);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QCRIL_QMI_VOICE_REPORT_SPEECH_CODEC %s", args );
    }
    else
    {
      QCRIL_LOG_INFO("QCRIL_QMI_VOICE_REPORT_SPEECH_CODEC %d",ret_val);
      qcril_qmi_voice_speech_codec_info.report_speech_codec = ret_val;
    }
  }

  qcril_qmi_voice_reset_stk_cc();

  QCRIL_LOG_DEBUG( "REPORT_SPEECH_CODEC=%d", qcril_qmi_voice_speech_codec_info.report_speech_codec );
  qcril_qmi_voice_speech_codec_info_init();

  qcril_qmi_ims_socket_init();
  qcril_qmi_voice_reset_conf_info_xml();

  qmi_voice_voip_overview.cdma_call_id = VOICE_INVALID_CALL_ID;

  qcril_qmi_voice_info.pil_state.state = QCRIL_QMI_PIL_STATE_UNKNOWN;
  if ( 0 == qcril_qmi_pil_init_monitor() )
  {
    qcril_qmi_pil_register_for_state_change(qcril_qmi_voice_handle_pil_state_changed);
    qcril_qmi_voice_info.pil_state = *(qcril_qmi_pil_get_pil_state());
  }

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QMI_RIL_SYS_PROP_NAME_SUBADDRESS);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QMI_RIL_SYS_PROP_NAME_SUBADDRESS %s", args );
    }
    else
    {
      feature_subaddress_support = ret_val;
    }
  }
  QCRIL_LOG_DEBUG("QMI_RIL_SYS_PROP_NAME_SUBADDRESS=%d", feature_subaddress_support);

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QMI_RIL_SYS_PROP_NAME_SUBADDRESS_AMPERSAND);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QMI_RIL_SYS_PROP_NAME_SUBADDRESS_AMPERSAND %s", args );
    }
    else
    {
      feature_subaddress_support_amp = ret_val;
    }
  }
  QCRIL_LOG_DEBUG("QMI_RIL_SYS_PROP_NAME_SUBADDRESS_AMPERSAND=%d", feature_subaddress_support_amp);

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_REJECT_CAUSE_21_SUPPORTED);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Failed to convert QCRIL_REJECT_CAUSE_21_SUPPORTED %s", args );
    }
    else
    {
      reject_cause_21_supported = ret_val;
    }
  }
  QCRIL_LOG_DEBUG("QCRIL_REJECT_CAUSE_21_SUPPORTED=%d", reject_cause_21_supported);

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s",
      QMI_RIL_SYS_PROP_NAME_SUBADDRESS_IA5_IDENTIFIER);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QMI_RIL_SYS_PROP_NAME_SUBADDRESS_IA5_IDENTIFIER %s",
          args );
    }
    else
    {
      feature_subaddress_ia5_id_support = ret_val;
    }
  }

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s",
      QMI_RIL_SYS_PROP_NAME_REDIR_PARTY_NUM);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QMI_RIL_SYS_PROP_NAME_REDIR_PARTY_NUM %s",
          args );
    }
    else
    {
      feature_redir_party_num_support = ret_val;
    }
  }
  QCRIL_LOG_DEBUG("QMI_RIL_SYS_PROP_NAME_REDIR_PARTY_NUM=%d", feature_redir_party_num_support);

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s",
      QCRIL_QMI_VOICE_DTMF_INTERVAL);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QCRIL_QMI_VOICE_DTMF_INTERVAL %s",
          args );
    }
    else
    {
      dtmf_rtp_event_interval = ret_val;
    }
  }
  QCRIL_LOG_DEBUG("QCRIL_QMI_VOICE_DTMF_INTERVAL=%d", dtmf_rtp_event_interval);

  qcril_initialize_ims_to_ril_facility_lookup_table();
  QCRIL_LOG_DEBUG("Initialized IMS to RIL facility lookup table");

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)res );

  qcril_qmi_voice_cdma_call_type_to_be_considered = CALL_TYPE_VOICE_V02;
  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s",
      QCRIL_QMI_CDMA_VOICE_EMER_VOICE);
  property_get( property_name, args, "" );
  property_param_len = strlen( args );
  if ( property_param_len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == ULONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QCRIL_QMI_CDMA_VOICE_EMER_VOICE %s",
          args );
    }
    else
    {
      QCRIL_LOG_DEBUG("QCRIL_QMI_CDMA_VOICE_EMER_VOICE=%d", ret_val);
      if (ret_val == 0)
      {
        qcril_qmi_voice_cdma_call_type_to_be_considered = CALL_TYPE_EMERGENCY_V02;
      }
    }
  }
  QCRIL_LOG_DEBUG("qcril_qmi_voice_cdma_call_type_to_be_considered=%d",
      qcril_qmi_voice_cdma_call_type_to_be_considered);

  QCRIL_LOG_FUNC_RETURN_WITH_RET( (int)res );

  return res;

} // qcril_qmi_voice_init

//===========================================================================
// qcril_qmi_voice_post_cleanup
//===========================================================================
void qcril_qmi_voice_post_cleanup( void )
{
  // mutex deletion
  pthread_mutex_destroy( &qmi_voice_voip_overview.overview_lock_mutex );
  pthread_mutexattr_destroy( &qmi_voice_voip_overview.overview_lock_mtx_atr );
  pthread_mutex_destroy( &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex );
  pthread_mutexattr_destroy( &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex_atr);
  pthread_mutex_destroy( &qcril_qmi_voice_info.voice_info_lock_mutex );
  pthread_mutexattr_destroy( &qcril_qmi_voice_info.voice_info_lock_mutex_atr);
  qcril_qmi_voice_speech_codec_info_cleanup();

  //emergency call cleanup
  if( qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data != NULL )
  {
    if( ((RIL_Dial*) qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data)->address != NULL )
    {
      free(((RIL_Dial*) qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data)->address);
      ((RIL_Dial*) qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data)->address = NULL;
    }
    free(qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data);
    qcril_qmi_pending_emergency_call_info.emergency_params_ptr.data = NULL;
  }

} // qcril_qmi_voice_post_cleanup

boolean qcril_qmi_voice_get_jbims() {return qcril_qmi_voice_info.jbims;}

//===========================================================================
// qcril_qmi_voice_speech_codec_info_init
//===========================================================================
void qcril_qmi_voice_speech_codec_info_init( void )
{
  pthread_attr_t attr;

  QCRIL_LOG_FUNC_ENTRY();

  signal(SIGUSR1,qcril_qmi_voice_speech_codec_thread_signal_handler_sigusr1);
  if( TRUE == qcril_qmi_voice_speech_codec_info.report_speech_codec )
  {
    pthread_mutex_init(&qcril_qmi_voice_speech_codec_info.speech_codec_mutex, NULL);
    pthread_cond_init (&qcril_qmi_voice_speech_codec_info.speech_codec_cond_var, NULL);
#ifdef QMI_RIL_UTF
    pthread_attr_init(&attr);
    utf_pthread_create_handler(&qcril_qmi_voice_speech_codec_info.speech_codec_thread_id, &attr, qcril_qmi_voice_speech_codec_info_thread_proc, NULL);
#else
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&qcril_qmi_voice_speech_codec_info.speech_codec_thread_id, &attr, qcril_qmi_voice_speech_codec_info_thread_proc, NULL);
#endif
    qmi_ril_set_thread_name(qcril_qmi_voice_speech_codec_info.speech_codec_thread_id, QMI_RIL_VOICE_SPEECH_CODEC_THREAD_NAME);
    pthread_attr_destroy(&attr);
  }

  QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_voice_speech_codec_info_init

//===========================================================================
// qcril_qmi_voice_speech_codec_info_cleanup
//===========================================================================
void qcril_qmi_voice_speech_codec_info_cleanup( void )
{
  if( TRUE == qcril_qmi_voice_speech_codec_info.report_speech_codec )
  {
    SPEECH_CODEC_LOCK();
    qcril_qmi_voice_speech_codec_info.is_exit_enabled = TRUE;
    pthread_kill(qcril_qmi_voice_speech_codec_info.speech_codec_thread_id,SIGUSR1);
    SPEECH_CODEC_SIGNAL();
    SPEECH_CODEC_UNLOCK();
    pthread_join(qcril_qmi_voice_speech_codec_info.speech_codec_thread_id,NULL);

    qcril_qmi_voice_speech_codec_info.is_exit_enabled = FALSE;
    qcril_qmi_voice_speech_codec_info.speech_codec_info_valid = FALSE;
    pthread_mutex_destroy( &qcril_qmi_voice_speech_codec_info.speech_codec_mutex );
    pthread_cond_destroy( &qcril_qmi_voice_speech_codec_info.speech_codec_cond_var );
  }
} //qcril_qmi_voice_speech_codec_info_cleanup

//===========================================================================
// qcril_qmi_voice_speech_codec_info_thread_proc
//===========================================================================
void* qcril_qmi_voice_speech_codec_info_thread_proc(void * param)
{
    QCRIL_LOG_FUNC_ENTRY();

    struct addrinfo hints, *servinfo;
    char send_buffer[QCRIL_QMI_VOICE_SPEECH_CODEC_BUFFER_MAX_SIZE], recv_buffer[QCRIL_QMI_VOICE_SPEECH_CODEC_BUFFER_MAX_SIZE];
    int recv_bytes = 0;
    int send_bytes = 0;
    int temp_sockopt = 0;
    struct sockaddr_storage their_addr;
    socklen_t their_addr_size;
    int sockid = 0;
    int new_sockid = 0;
    int ret = 0;
    int wait_res = 0;
    int invalid_case = 0;

    QCRIL_NOTUSED(param);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNIX;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, QCRIL_QMI_VOICE_SPEECH_CODEC_PORT, &hints, &servinfo) != 0)
    {
        return NULL;
    }
    if ((sockid = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1)
    {
        freeaddrinfo(servinfo);
        return NULL;
    }
    if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &temp_sockopt, sizeof(temp_sockopt)) == -1)
    {
        freeaddrinfo(servinfo);
        close(sockid);
        return NULL;
    }
    if (bind(sockid, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        freeaddrinfo(servinfo);
        close(sockid);
        return NULL;
    }
    if (listen(sockid, QCRIL_QMI_VOICE_SPEECH_CODEC_BACKLOG) == -1)
    {
        freeaddrinfo(servinfo);
        close(sockid);
        return NULL;
    }
    freeaddrinfo(servinfo);

    while(ret != -1)
    {
        their_addr_size = sizeof their_addr;
        new_sockid = accept(sockid, (struct sockaddr *)&their_addr, &their_addr_size);
        if (new_sockid == -1)
        {
            break;
        }

        QCRIL_LOG_INFO("client connected");
        while(1)
        {
            invalid_case = TRUE;
            memset(send_buffer, 0, sizeof(send_buffer));
            memset(recv_buffer, 0, sizeof(recv_buffer));

            if ((recv_bytes = recv(new_sockid, recv_buffer, sizeof(recv_buffer) , 0)) <= 0)
            {
                ret = -1;
                break;
            }
            QCRIL_LOG_INFO("number of bytes received %d, message_id %02x info requested by client", recv_bytes, recv_buffer[0]);

            if( QMI_VOICE_SPEECH_CODEC_INFO_IND_V02 == recv_buffer[0] )
            {
              SPEECH_CODEC_LOCK();
              if( FALSE == qcril_qmi_voice_speech_codec_info.speech_codec_info_valid )
              {
                wait_res = SPEECH_CODEC_WAIT();
                if( TRUE == qcril_qmi_voice_speech_codec_info.is_exit_enabled )
                {
                  ret = -1;
                  SPEECH_CODEC_UNLOCK();
                  break;
                }
              }

              if( TRUE == qcril_qmi_voice_speech_codec_info.speech_codec_info_valid )
              {
                invalid_case = FALSE;
                send_buffer[0] = recv_buffer[0];
                send_buffer[1] = 2; //length of the payload in little endian order - network_mode + speech_codec
                send_buffer[2] = 0;
                send_buffer[3] = qcril_qmi_voice_speech_codec_info.network_mode;
                send_buffer[4] = qcril_qmi_voice_speech_codec_info.speech_codec;
                send_bytes = 5;
                qcril_qmi_voice_speech_codec_info.speech_codec_info_valid = FALSE;
              }
              SPEECH_CODEC_UNLOCK();
            }

            if( TRUE == invalid_case )
            {
              QCRIL_LOG_INFO("unsupported message or speech codec info not available within specified timeframe");
              send_buffer[0] = recv_buffer[0];
              send_buffer[1] = 0; //length of the payload in little endian order - '0' since it is a unsupported message
              send_buffer[2] = 0;
              send_bytes = 3;
            }
            send(new_sockid, send_buffer, send_bytes, 0);
        }
        close(new_sockid);
    }
    close(sockid);

    qmi_ril_clear_thread_name(pthread_self());

    QCRIL_LOG_FUNC_RETURN();
    return NULL;
} //qcril_qmi_voice_speech_codec_info_thread_proc

//===========================================================================
// qcril_qmi_voice_speech_codec_thread_signal_handler_sigusr1
//===========================================================================
void qcril_qmi_voice_speech_codec_thread_signal_handler_sigusr1(int arg)
{

  QCRIL_NOTUSED(arg);
  return;
} //qcril_qmi_voice_speech_codec_thread_signal_handler_sigusr1

//===========================================================================
//qcril_qmi_voice_speech_codec_condition_wait_helper
//===========================================================================
IxErrnoType qcril_qmi_voice_speech_codec_condition_wait_helper()
{
    IxErrnoType res;
    struct timeval tp;
    struct timespec ts;

    gettimeofday(&tp, NULL);
    ts.tv_sec = tp.tv_sec + 15;     //15 seconds
    ts.tv_nsec = tp.tv_usec * 1000;
    res = pthread_cond_timedwait(&qcril_qmi_voice_speech_codec_info.speech_codec_cond_var, &qcril_qmi_voice_speech_codec_info.speech_codec_mutex, &ts);
    return res;
} //qcril_qmi_voice_speech_codec_condition_wait_helper

//===========================================================================
// qcril_qmi_voice_cleanup
//===========================================================================
void qcril_qmi_voice_cleanup(void)
{
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
  boolean unsol_resp = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  // clean call objects, will have to extend

  qcril_qmi_voice_voip_lock_overview();
  call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
  while ( NULL != call_info_entry )
  {
      if(qcril_qmi_voice_call_to_ims(call_info_entry))
      {
         unsol_resp = TRUE;
      }
      call_info_entry->voice_scv_info.call_state = CALL_STATE_END_V02;
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
  }
  qcril_qmi_voice_voip_unlock_overview();
  if(unsol_resp)
  {
      qcril_qmi_voice_send_ims_unsol_call_state_changed();
  }
  qcril_qmi_voice_voip_lock_overview();
  call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
  while ( NULL != call_info_entry )
  {
      qcril_qmi_voice_voip_destroy_call_info_entry( call_info_entry );
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
  }
  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_cleanup

//===========================================================================
// qcril_qmi_voice_call_to_ims
//===========================================================================
boolean qcril_qmi_voice_call_to_ims(const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry)
{
   boolean ret = FALSE;



   if (call_info_entry)
   {
     ret = qcril_qmi_voice_info.jbims && !qcril_qmi_voice_call_to_atel(call_info_entry);
   }
   QCRIL_LOG_FUNC_RETURN_WITH_RET((int)ret);
   return ret;
} // qcril_qmi_voice_calls_to_ims

//===========================================================================
// qcril_qmi_voice_is_call_has_ims_audio
//===========================================================================
boolean qcril_qmi_voice_is_call_has_ims_audio
(
    const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry
)
{
    boolean ret = FALSE;
    if (call_info_entry)
    {
        boolean is_cs = ( (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN) ||
                          (call_info_entry->audio_call_type == QMI_RIL_VOICE_IMS_AUDIO_CALL_TYPE_VOICE) )
                        ? TRUE : FALSE;
        QCRIL_LOG_INFO( "qcril_qmi_voice_info.jbims: %d, is cs call: %d",
                        qcril_qmi_voice_info.jbims,
                        is_cs );
        ret = qcril_qmi_voice_info.jbims && !is_cs && !(call_info_entry->srvcc_in_progress);
   }
   return ret;
} // qcril_qmi_voice_is_call_has_ims_audio

//===========================================================================
// qcril_qmi_voice_is_call_has_voice_audio
//===========================================================================
boolean qcril_qmi_voice_is_call_has_voice_audio
(
    const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry
)
{
    boolean ret = FALSE;
    if (call_info_entry)
    {
        ret = call_info_entry->srvcc_in_progress || !qcril_qmi_voice_is_call_has_ims_audio(call_info_entry);
   }
   return ret;
} // qcril_qmi_voice_is_call_has_voice_audio

//===========================================================================
// qcril_qmi_voice_get_answer_am_event
//===========================================================================
qcril_am_vs_type qcril_qmi_voice_get_answer_am_event
(
    const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry
)
{
    qcril_am_event_type am_event = QCRIL_AM_EVENT_INVALID;
    if (call_info_entry)
    {
        if ( qcril_qmi_voice_is_call_has_ims_audio(call_info_entry) )
        {
            am_event = QCRIL_AM_EVENT_IMS_ANSWER;
        }
        else
        {
            am_event = QCRIL_AM_EVENT_VOICE_ANSWER;
        }
    }
    return am_event;
} // qcril_qmi_voice_get_answer_am_event

//===========================================================================
// qcril_qmi_voice_call_to_atel
//===========================================================================
boolean qcril_qmi_voice_call_to_atel(const qcril_qmi_voice_voip_call_info_entry_type * call_info_entry)
{
   boolean ret = FALSE;
   if (call_info_entry)
   {
     ret = (!qcril_qmi_voice_info.jbims) || (QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN & call_info_entry->elaboration);
     QCRIL_LOG_INFO("call_to_atel: %d", ret);

   }
   return ret;
} // qcril_qmi_voice_call_to_atel

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_lookup_command_name

===========================================================================*/
/*!
    @brief
    Lookup state name.

    @return
    A pointer to the state name.
*/
/*=========================================================================*/
static char *qcril_qmi_voice_lookup_command_name
(
  unsigned long msg
)
{
  switch( msg )
  {
    case QMI_VOICE_DIAL_CALL_RESP_V02:
      return "DIAL CALL RESP";
    case QMI_VOICE_ANSWER_CALL_RESP_V02:
      return "ANSWER CALL RESP";
    case QMI_VOICE_END_CALL_RESP_V02:
      return "END CALL RESP";
    case QMI_VOICE_BURST_DTMF_RESP_V02:
      return "BURST DTMF RESP";
    case QMI_VOICE_START_CONT_DTMF_RESP_V02:
      return "START CONT DTMF RESP";
    case QMI_VOICE_SEND_FLASH_RESP_V02:
      return "SEND FLASH RESP";
    case QMI_VOICE_MANAGE_CALLS_RESP_V02:
      return "MNG CALLS RESP";
    case QMI_VOICE_GET_CLIP_RESP_V02:
      return "GET CLIP RESP";
    case QMI_VOICE_MANAGE_IP_CALLS_RESP_V02:
      return "MNG IP CALLS RESP";
    case QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02:
      return "SET SUPS RESP";
    case QMI_VOICE_GET_COLR_RESP_V02:
      return "GET COLR RESP";
    default:
      return "Unknown";
  } /* end switch */
} /* qcril_qmi_voice_lookup_command_name */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_map_qmi_to_ril_num_pi

===========================================================================*/
/*!
    @brief
    Maps qmi num presentation enum to RIL pi values.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_voice_map_qmi_to_ril_num_pi
(
  pi_num_enum_v02  qmi_num_pi
)
{
  int ril_num_pi = QCRIL_QMI_VOICE_RIL_PI_ALLOWED;

  switch(qmi_num_pi)
  {
    case PRESENTATION_NUM_ALLOWED_V02:
      ril_num_pi = QCRIL_QMI_VOICE_RIL_PI_ALLOWED;
      break;
    case PRESENTATION_NUM_RESTRICTED_V02:
      ril_num_pi = QCRIL_QMI_VOICE_RIL_PI_RESTRICTED;
      break;
    case PRESENTATION_NUM_RESERVED_V02:               // fallthough
    case PRESENTATION_NUM_NUM_UNAVAILABLE_V02:
      ril_num_pi = QCRIL_QMI_VOICE_RIL_PI_UNKNOWN;
      break;
    case PRESENTATION_NUM_PAYPHONE_V02:
      ril_num_pi = QCRIL_QMI_VOICE_RIL_PI_PAYPHONE;
      break;
    default:
      QCRIL_LOG_INFO("Invalid num presentation %d",qmi_num_pi);
      break;
  }
  return ril_num_pi;
}/* qcril_qmi_voice_map_qmi_to_ril_num_pi */

boolean qcril_qmi_voice_map_qmi_to_ril_provision_status
(
 provision_status_enum_v02 qmi_provision_status,
 int *ril_provision_status
)
{
  boolean result = FALSE;

  if (ril_provision_status)
  {
    result = TRUE;
    switch(qmi_provision_status)
    {
      case PROVISION_STATUS_NOT_PROVISIONED_V02 :
        *ril_provision_status = QCRIL_QMI_VOICE_CLIR_SRV_NOT_PROVISIONED;
        break;
      case PROVISION_STATUS_PRESENTATION_ALLOWED_V02 :
        *ril_provision_status = QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_ALLOWED;
        break;
      case PROVISION_STATUS_PROVISIONED_PERMANENT_V02 :
        *ril_provision_status = QCRIL_QMI_VOICE_CLIR_SRV_PROVISIONED_PERMANENT;
        break;
      case PROVISION_STATUS_PRESENTATION_RESTRICTED_V02:
        *ril_provision_status = QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_RESTRICTED;
        break;
      default :
        QCRIL_LOG_INFO("Invalid provision status %d", qmi_provision_status);
        result = FALSE;
        break;
    }
  }

  return result;
}

boolean qcril_qmi_voice_map_qmi_status_to_ims_provision_status
(
 provision_status_enum_v02 qmi_provision_status,
 active_status_enum_v02 qmi_activation_status,
 int *ril_provision_status
)
{
  boolean result = FALSE;

  if(qmi_provision_status != PROVISION_STATUS_NOT_PROVISIONED_V02)
  {
    if (ril_provision_status)
    {
      result = TRUE;
      switch(qmi_activation_status)
      {
        case ACTIVE_STATUS_INACTIVE_V02:
          *ril_provision_status = QCRIL_QMI_VOICE_SUPS_SRV_INACTIVE;
          break;
        case ACTIVE_STATUS_ACTIVE_V02:
          *ril_provision_status = QCRIL_QMI_VOICE_SUPS_SRV_ACTIVE;
          break;
        default :
          QCRIL_LOG_INFO("Invalid avtivation status %d", qmi_activation_status);
          result = FALSE;
          break;
      }
    }
  }
  else
  {
    *ril_provision_status = QCRIL_QMI_VOICE_SUPS_SRV_INACTIVE;
  }

  return result;
}

int  qcril_qmi_voice_get_facility_value
(
  const char * facility,
  char* facility_name )
{
  char temp[ 3 ];

  if( facility != NULL )
  {
  if ( strlen( facility ) != sizeof( char ) * 2 )
    return ( (int) QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED );

  memcpy( &temp[ 0 ], facility, sizeof( char ) * 2 );
  temp[ 2 ] = '\0';

  temp[ 0 ] = QCRIL_QMI_VOICE_UPCASE( temp[ 0 ] );
  temp[ 1 ] = QCRIL_QMI_VOICE_UPCASE( temp[ 1 ] );

  memcpy( facility_name, temp, 3 );

  if ( strcmp( temp, "SC" ) == FALSE )
   return ( (int) QCRIL_QMI_VOICE_FACILITY_LOCK_SC );

  if ( strcmp( temp, "AO" ) == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_ALLOUTGOING );

  if ( strcmp( temp, "OI" ) == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_OUTGOINGINT);

  if ( strcmp( temp, "OX" ) == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_OUTGOINGINTEXTOHOME);

  if ( strcmp( temp, "AI") == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_ALLINCOMING);

  if ( strcmp( temp, "IR" ) == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_INCOMINGROAMING);

  if ( strcmp( temp, "AB" ) == FALSE )
    return( (int) QCRIL_QMI_VOICE_CB_FACILITY_ALLBARRING);

  if ( strcmp( temp, "AG" ) == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_ALLOUTGOINGBARRING);

  if ( strcmp( temp, "AC" ) == FALSE )
    return ( (int) QCRIL_QMI_VOICE_CB_FACILITY_ALLINCOMINGBARRING);

  if (strcmp( temp, "FD" ) == FALSE)
    return ( (int) QCRIL_QMI_VOICE_FACILITY_LOCK_FD );

  return ( (int) QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED );
  }
  else
  {
    return ( (int) QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED );
  }

} /* qcril_qmi_voice_get_facility_value */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_convert_ims_to_ril_facility_type

===========================================================================*/
/*!
    @brief
    Converts IMS facility of type Ims__SuppSvcFacilityType to QCRIL
    facility of type qcril_qmi_voice_facility_e_type using lookup table
    qcril_ims_to_ril_facility_lookup_table.

    @return
    Success: qcril_qmi_voice_facility_e_type
    Error:   QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED
*/
/*=========================================================================*/
static qcril_qmi_voice_facility_e_type
  qcril_qmi_voice_convert_ims_to_ril_facility_type
(
  /* Facility type as definied in Ims__SuppSvcRequest */
  Ims__SuppSvcFacilityType ims_facility_type
)
{
  if (ims_facility_type >= IMS__SUPP_SVC_FACILITY_TYPE__MIN &&
      ims_facility_type <= IMS__SUPP_SVC_FACILITY_TYPE__MAX)
  {
    return qcril_ims_to_ril_facility_lookup_table[ims_facility_type];
  }
  else
  {
    return QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED;
  }
} /* qcril_qmi_voice_convert_ims_to_ril_facility_type */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_map_qmi_cfw_reason_to_ril_reason

===========================================================================*/
/*!
    @brief
    Maps qmi CFW Reason to QCRIL CFW Reason.

    @return
    None.
*/
/*=========================================================================*/

uint8 qcril_qmi_voice_map_qmi_cfw_reason_to_ril_reason(int qmi_cfw_reason)
{
  uint8 ril_reason=0;

  switch(qmi_cfw_reason)
  {
    case VOICE_REASON_FWD_UNCONDITIONAL_V02 :
     ril_reason = (uint8)QCRIL_QMI_VOICE_CCFC_REASON_UNCOND;
     break;

    case VOICE_REASON_FWD_MOBILEBUSY_V02:
      ril_reason = (uint8)QCRIL_QMI_VOICE_CCFC_REASON_BUSY ;
      break;

    case VOICE_REASON_FWD_NOREPLY_V02 :
      ril_reason = (uint8)QCRIL_QMI_VOICE_CCFC_REASON_NOREPLY;
      break;

    case VOICE_REASON_FWD_UNREACHABLE_V02:
      ril_reason = (uint8) QCRIL_QMI_VOICE_CCFC_REASON_NOTREACH ;
      break;

    case VOICE_REASON_FWD_ALLFORWARDING_V02:
      ril_reason = (uint8)QCRIL_QMI_VOICE_CCFC_REASON_ALLCALL ;
      break;

    case VOICE_REASON_FWD_ALLCONDITIONAL_V02:
      ril_reason = (uint8)QCRIL_QMI_VOICE_CCFC_REASON_ALLCOND ;
      break;

    default:
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("qmi_reason : %d",qmi_cfw_reason);
      ril_reason = (uint8)QCRIL_QMI_VOICE_CCFC_REASON_UNCOND;
      break;
  }

  return ril_reason;
}/*qcril_qmi_voice_map_qmi_cfw_reason_to_ril_reason*/

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_map_qmi_to_ril_name_pi

===========================================================================*/
/*!
    @brief
    Maps qmi name presentation enum to RIL pi values.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_voice_map_qmi_to_ril_name_pi
(
  pi_name_enum_v02  qmi_name_pi
)
{
  int ril_name_pi = QCRIL_QMI_VOICE_RIL_PI_ALLOWED;

  switch(qmi_name_pi)
  {
    case PRESENTATION_NAME_PRESENTATION_ALLOWED_V02:
      ril_name_pi = QCRIL_QMI_VOICE_RIL_PI_ALLOWED;
      break;
    case PRESENTATION_NAME_PRESENTATION_RESTRICTED_V02:
      ril_name_pi = QCRIL_QMI_VOICE_RIL_PI_RESTRICTED;
      break;
    case PRESENTATION_NAME_UNAVAILABLE_V02:
      ril_name_pi = QCRIL_QMI_VOICE_RIL_PI_UNKNOWN;
      break;
    case PRESENTATION_NAME_NAME_PRESENTATION_RESTRICTED_V02:
      ril_name_pi = QCRIL_QMI_VOICE_RIL_PI_RESTRICTED;
      break;
    default:
      QCRIL_LOG_INFO("Invalid name presentation %d",qmi_name_pi);
      break;
  }
  return ril_name_pi;
}/* qcril_qmi_voice_map_qmi_to_ril_name_pi */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_map_qmi_to_ril_last_call_failure_cause

===========================================================================*/
/*!
    @brief
    Maps qmi last call failure cause enum to RIL last call failure cause values.

    @return
    RIL last call failure cause values.
*/
/*=========================================================================*/
RIL_LastCallFailCause qcril_qmi_voice_map_qmi_to_ril_last_call_failure_cause
(
  call_end_reason_enum_v02  reason
)
{

  RIL_LastCallFailCause ret;
  switch ( reason )
  {
    case CALL_END_CAUSE_CLIENT_END_V02:
    case CALL_END_CAUSE_REL_NORMAL_V02:
      ret = CALL_FAIL_NORMAL;
      break;

    case CALL_END_CAUSE_NETWORK_END_V02:
      ret = CALL_FAIL_NORMAL;
      break;

    case CALL_END_CAUSE_IMSI_UNKNOWN_IN_VLR_V02:
      ret = CALL_FAIL_IMSI_UNKNOWN_IN_VLR;
      break;

    case CALL_END_CAUSE_IMEI_NOT_ACCEPTED_V02:
      ret = CALL_FAIL_IMEI_NOT_ACCEPTED;
      break;

    case CALL_END_CAUSE_INCOM_REJ_V02:
    case CALL_END_CAUSE_SETUP_REJ_V02:
      ret = CALL_FAIL_BUSY;
      break;

    case CALL_END_CAUSE_NO_SRV_V02:
    case CALL_END_CAUSE_NO_GW_SRV_V02:
    case CALL_END_CAUSE_NO_FULL_SRV_V02:
    case CALL_END_CAUSE_MAX_PS_CALLS_V02:
      ret = CALL_FAIL_ERROR_UNSPECIFIED;
      break;

    case CALL_END_CAUSE_NETWORK_CONGESTION_V02:
      ret = CALL_FAIL_CONGESTION;
      break;

    case CALL_END_CAUSE_NO_FUNDS_V02:
      ret = CALL_FAIL_ACM_LIMIT_EXCEEDED;
      break;

    case CALL_END_CAUSE_CDMA_LOCK_V02:
      ret = CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE;
      break;

    case CALL_END_CAUSE_FADE_V02:
      ret = CALL_FAIL_CDMA_DROP;
      break;

    case CALL_END_CAUSE_INTERCEPT_V02:
      ret = CALL_FAIL_CDMA_INTERCEPT;
      break;

    case CALL_END_CAUSE_REORDER_V02:
      ret = CALL_FAIL_CDMA_REORDER;
      break;

    case CALL_END_CAUSE_REL_SO_REJ_V02:
      ret = CALL_FAIL_CDMA_SO_REJECT;
      break;

    case CALL_END_CAUSE_RETRY_ORDER_V02:
      ret = CALL_FAIL_CDMA_RETRY_ORDER;
      break;

    case CALL_END_CAUSE_ACC_FAIL_V02:
    case CALL_END_CAUSE_MAX_ACCESS_PROBE_V02:
      ret = CALL_FAIL_CDMA_ACCESS_FAILURE;
      break;

    case CALL_END_CAUSE_INCOM_CALL_V02:
      ret = CALL_FAIL_CDMA_PREEMPTED;
      break;

    case CALL_END_CAUSE_EMERGENCY_FLASHED_V02:
      ret = CALL_FAIL_CDMA_NOT_EMERGENCY;
      break;

    case CALL_END_CAUSE_ACCESS_BLOCK_ALL_V02:
    case CALL_END_CAUSE_PSIST_N_V02:
      ret = CALL_FAIL_CDMA_ACCESS_BLOCKED;
      break;

    case CALL_END_CAUSE_UNASSIGNED_NUMBER_V02:
      ret = CALL_FAIL_UNOBTAINABLE_NUMBER;
      break;

    case CALL_END_CAUSE_USER_BUSY_V02:
      ret = CALL_FAIL_BUSY;
      break;

    case CALL_END_CAUSE_NORMAL_CALL_CLEARING_V02:
    case CALL_END_CAUSE_RINGING_RINGBACK_TIMEOUT_V02:
      ret = CALL_FAIL_NORMAL;
      break;

#ifdef EXTENDED_FAIL_ERROR_CAUSE_FOR_VOICE_CALL
    case CALL_END_CAUSE_NO_ROUTE_TO_DESTINATION_V02:
      ret = CALL_FAIL_NO_ROUTE_TO_DESTINATION;
      break;

    case CALL_END_CAUSE_CHANNEL_UNACCEPTABLE_V02:
      ret = CALL_FAIL_CHANNEL_UNACCEPTABLE;
      break;

    case CALL_END_CAUSE_OPERATOR_DETERMINED_BARRING_V02:
      ret = CALL_FAIL_OPERATOR_DETERMINED_BARRING;
      break;

    case CALL_END_CAUSE_NO_USER_RESPONDING_V02:
      ret = CALL_FAIL_NO_USER_RESPONDING;
      break;

    case CALL_END_CAUSE_USER_ALERTING_NO_ANSWER_V02:
      ret = CALL_FAIL_NO_ANSWER_FROM_USER;
      break;

    case CALL_END_CAUSE_CALL_REJECTED_V02:
      ret = CALL_FAIL_CALL_REJECTED;
      break;

    case CALL_END_CAUSE_NUMBER_CHANGED_V02:
      ret = CALL_FAIL_NUMBER_CHANGED;
      break;

    case CALL_END_CAUSE_PREEMPTION_V02:
      ret = CALL_FAIL_PREEMPTION;
      break;

    case CALL_END_CAUSE_DESTINATION_OUT_OF_ORDER_V02:
      ret = CALL_FAIL_DESTINATION_OUT_OF_ORDER;
      break;

    case CALL_END_CAUSE_INVALID_NUMBER_FORMAT_V02:
      ret = CALL_FAIL_INVALID_NUMBER_FORMAT;
      break;

    case CALL_END_CAUSE_FACILITY_REJECTED_V02:
      ret = CALL_FAIL_FACILITY_REJECTED;
      break;

    case CALL_END_CAUSE_RESP_TO_STATUS_ENQUIRY_V02:
      ret = CALL_FAIL_RESP_TO_STATUS_ENQUIRY;
      break;

    case CALL_END_CAUSE_NORMAL_UNSPECIFIED_V02:
      ret = CALL_FAIL_NORMAL_UNSPECIFIED;
      break;

    case CALL_END_CAUSE_NO_CIRCUIT_OR_CHANNEL_AVAILABLE_V02:
      ret = CALL_FAIL_CONGESTION;
      break;

    case CALL_END_CAUSE_NETWORK_OUT_OF_ORDER_V02:
      ret = CALL_FAIL_NETWORK_OUT_OF_ORDER;
      break;

    case CALL_END_CAUSE_TEMPORARY_FAILURE_V02:
      ret = CALL_FAIL_TEMPORARY_FAILURE;
      break;

    case CALL_END_CAUSE_SWITCHING_EQUIPMENT_CONGESTION_V02:
      ret = CALL_FAIL_SWITCHING_EQUIPMENT_CONGESTION;
      break;

    case CALL_END_CAUSE_ACCESS_INFORMATION_DISCARDED_V02:
      ret = CALL_FAIL_ACCESS_INFORMATION_DISCARDED;
      break;

    case CALL_END_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE_V02:
      ret = CALL_FAIL_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;
      break;

    case CALL_END_CAUSE_RESOURCES_UNAVAILABLE_OR_UNSPECIFIED_V02:
      ret = CALL_FAIL_RESOURCES_UNAVAILABLE_OR_UNSPECIFIED;
      break;

    case CALL_END_CAUSE_QOS_UNAVAILABLE_V02:
      ret = CALL_FAIL_QOS_UNAVAILABLE;
      break;

    case CALL_END_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED_V02:
      ret = CALL_FAIL_REQUESTED_FACILITY_NOT_SUBSCRIBED;
      break;

    case CALL_END_CAUSE_INCOMING_CALLS_BARRED_WITHIN_CUG_V02:
      ret = CALL_FAIL_INCOMING_CALLS_BARRED_WITHIN_CUG;
      break;

    case CALL_END_CAUSE_BEARER_CAPABILITY_NOT_AUTH_V02:
      ret = CALL_FAIL_BEARER_CAPABILITY_NOT_AUTHORIZED;
      break;

    case CALL_END_CAUSE_BEARER_CAPABILITY_UNAVAILABLE_V02:
      ret = CALL_FAIL_BEARER_CAPABILITY_UNAVAILABLE;
      break;

    case CALL_END_CAUSE_SERVICE_OPTION_NOT_AVAILABLE_V02:
      ret = CALL_FAIL_SERVICE_OPTION_NOT_AVAILABLE;
      break;

    case CALL_END_CAUSE_BEARER_SERVICE_NOT_IMPLEMENTED_V02:
      ret = CALL_FAIL_BEARER_SERVICE_NOT_IMPLEMENTED;
      break;

    case CALL_END_CAUSE_ACM_LIMIT_EXCEEDED_V02:
      ret = CALL_FAIL_ACM_LIMIT_EXCEEDED;
      break;

    case CALL_END_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED_V02:
      ret = CALL_FAIL_REQUESTED_FACILITY_NOT_IMPLEMENTED;
      break;

    case CALL_END_CAUSE_ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE_V02:
      ret = CALL_FAIL_ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE;
      break;

    case CALL_END_CAUSE_SERVICE_OR_OPTION_NOT_IMPLEMENTED_V02:
      ret = CALL_FAIL_SERVICE_OR_OPTION_NOT_IMPLEMENTED;
      break;

    case CALL_END_CAUSE_INVALID_TRANSACTION_IDENTIFIER_V02:
      ret = CALL_FAIL_INVALID_TRANSACTION_IDENTIFIER;
      break;

    case CALL_END_CAUSE_USER_NOT_MEMBER_OF_CUG_V02:
      ret = CALL_FAIL_USER_NOT_MEMBER_OF_CUG;
      break;

    case CALL_END_CAUSE_INCOMPATIBLE_DESTINATION_V02:
      ret = CALL_FAIL_INCOMPATIBLE_DESTINATION;
      break;

    case CALL_END_CAUSE_INVALID_TRANSIT_NW_SELECTION_V02:
      ret = CALL_FAIL_INVALID_TRANSIT_NW_SELECTION;
      break;

    case CALL_END_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE_V02:
      ret = CALL_FAIL_SEMANTICALLY_INCORRECT_MESSAGE;
      break;

    case CALL_END_CAUSE_INVALID_MANDATORY_INFORMATION_V02:
      ret = CALL_FAIL_INVALID_MANDATORY_INFORMATION;
      break;

    case CALL_END_CAUSE_MESSAGE_TYPE_NON_IMPLEMENTED_V02:
      ret = CALL_FAIL_MESSAGE_TYPE_NON_IMPLEMENTED;
      break;

    case CALL_END_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE_V02:
      ret = CALL_FAIL_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
      break;

    case CALL_END_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_V02:
      ret = CALL_FAIL_INFORMATION_ELEMENT_NON_EXISTENT;
      break;

    case CALL_END_CAUSE_CONDITONAL_IE_ERROR_V02:
      ret = CALL_FAIL_CONDITIONAL_IE_ERROR;
      break;

    case CALL_END_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE_V02:
      ret = CALL_FAIL_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
      break;

    case CALL_END_CAUSE_RECOVERY_ON_TIMER_EXPIRED_V02:
      ret = CALL_FAIL_RECOVERY_ON_TIMER_EXPIRED;
      break;

    case CALL_END_CAUSE_PROTOCOL_ERROR_UNSPECIFIED_V02:
      ret = CALL_FAIL_PROTOCOL_ERROR_UNSPECIFIED;
      break;

    case CALL_END_CAUSE_INTERWORKING_UNSPECIFIED_V02:
      ret = CALL_FAIL_INTERWORKING_UNSPECIFIED;
      break;
#endif

    // DSDS: To enable emergency redial. Android Telephony
    // shall redial emergency call on the other sub on receiving
    // the fail cause. This cause is specific to DSDS and not
    // per spec. Telephony has locally defined the same cause codes.
    case CALL_END_CAUSE_TEMP_REDIAL_ALLOWED_V02:
    case CALL_END_CAUSE_PERM_REDIAL_NOT_NEEDED_V02:
      ret = reason;
      break;

    default:
      ret = CALL_FAIL_ERROR_UNSPECIFIED;
      break;
  } /* end switch */
  QCRIL_LOG_ESSENTIAL("map qmi reason: %d to ril reason: %d", reason, ret);
  return ret;
}/* qcril_qmi_voice_map_qmi_to_ril_last_call_failure_cause */

//===========================================================================
// qcril_qmi_voice_last_call_failure_cause_lock
//===========================================================================
inline void qcril_qmi_voice_last_call_failure_cause_lock()
{
  pthread_mutex_lock( &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex );
} // qcril_qmi_voice_last_call_failure_cause_lock

//===========================================================================
// qcril_qmi_voice_last_call_failure_cause_unlock
//===========================================================================
inline void qcril_qmi_voice_last_call_failure_cause_unlock()
{
  pthread_mutex_unlock( &qcril_qmi_voice_info.last_call_failure_cause.call_failure_cause_lock_mutex );
} // qcril_qmi_voice_last_call_failure_cause_unlock

//===========================================================================
// qcril_qmi_voice_info_lock
//===========================================================================
inline void qcril_qmi_voice_info_lock()
{
  pthread_mutex_lock( &qcril_qmi_voice_info.voice_info_lock_mutex );
} // qcril_qmi_voice_info_lock

//===========================================================================
// qcril_qmi_voice_info_unlock
//===========================================================================
inline void qcril_qmi_voice_info_unlock()
{
  pthread_mutex_unlock( &qcril_qmi_voice_info.voice_info_lock_mutex );
} // qcril_qmi_voice_info_unlock

//===========================================================================
// qcril_qmi_voice_store_last_call_failure_cause_ex
//===========================================================================
void qcril_qmi_voice_store_last_call_failure_cause_ex
(
  RIL_LastCallFailCause  reason,
  uint8 *extended_codes,
  int extended_codes_len
)
{
  QCRIL_LOG_INFO("store ril reason: %d", reason);
  qcril_qmi_voice_last_call_failure_cause_lock();
  qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause_valid = TRUE;
  qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause = reason;

  if (qcril_qmi_voice_info.last_call_failure_extended_codes)
  {
    qcril_free(qcril_qmi_voice_info.last_call_failure_extended_codes);
    qcril_qmi_voice_info.last_call_failure_extended_codes_len = 0;
  }

  if (extended_codes_len && extended_codes)
  {
    qcril_qmi_voice_info.last_call_failure_extended_codes = qcril_malloc(extended_codes_len);
    if (qcril_qmi_voice_info.last_call_failure_extended_codes)
    {
       memcpy(qcril_qmi_voice_info.last_call_failure_extended_codes, extended_codes, extended_codes_len);
       qcril_qmi_voice_info.last_call_failure_extended_codes_len = extended_codes_len;
    }
    else
    {
       QCRIL_LOG_ERROR("malloc failed");
    }
  }
  qcril_qmi_voice_last_call_failure_cause_unlock();
} // qcril_qmi_voice_store_last_call_failure_cause_ex

//===========================================================================
// qcril_qmi_voice_store_last_call_failure_cause
//===========================================================================
void qcril_qmi_voice_store_last_call_failure_cause
(
  RIL_LastCallFailCause  reason
)
{
  QCRIL_LOG_INFO("store ril reason: %d", reason);
  qcril_qmi_voice_store_last_call_failure_cause_ex(reason, NULL, 0);
} // qcril_qmi_voice_store_last_call_failure_cause

//===========================================================================
// qcril_qmi_voice_invalid_last_call_failure_cause
//===========================================================================
void qcril_qmi_voice_invalid_last_call_failure_cause()
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_voice_last_call_failure_cause_lock();
  qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause_valid = FALSE;
  qcril_qmi_voice_last_call_failure_cause_unlock();
} // qcril_qmi_voice_invalid_last_call_failure_cause

//===========================================================================
// qcril_qmi_voice_set_last_call_fail_request_timeout
//===========================================================================
void qcril_qmi_voice_set_last_call_fail_request_timeout()
{
  const struct timeval timeout_value = {3,0}; // 3 seconds
  uint32 timer_id;

  QCRIL_LOG_FUNC_ENTRY();
  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                              QCRIL_DEFAULT_MODEM_ID,
                              qcril_qmi_voice_last_call_fail_request_timeout_handler,
                              &timeout_value,
                              &timer_id );

  qcril_qmi_voice_last_call_failure_cause_lock();
  qcril_qmi_voice_info.last_call_failure_cause.pending_req = TRUE;
  qcril_qmi_voice_info.last_call_failure_cause.pending_request_timeout_timer_id = timer_id;
  qcril_qmi_voice_last_call_failure_cause_unlock();

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_set_last_call_fail_request_timeout

//===========================================================================
// qcril_qmi_voice_last_call_fail_request_timeout_handler
//===========================================================================
void qcril_qmi_voice_last_call_fail_request_timeout_handler(void * param)
{
  qcril_request_resp_params_type resp;
  boolean response_sent = FALSE;

  QCRIL_NOTUSED( param );

  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_voice_last_call_failure_cause_lock();
  if (qcril_qmi_voice_info.last_call_failure_cause.pending_req)
  {
    qcril_reqlist_public_type request_info;
    if (E_SUCCESS == qcril_reqlist_query_by_request(QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_LAST_CALL_FAIL_CAUSE, &request_info))
    {
      QCRIL_LOG_DEBUG( "Reply to RIL --> Last call fail cause : %d", qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause);
      qcril_default_request_resp_params(QCRIL_DEFAULT_INSTANCE_ID, request_info.t, request_info.request,
                                        RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      response_sent = TRUE;
    }

    if (E_SUCCESS == qcril_reqlist_query_by_request(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE, &request_info))
    {
      qcril_qmi_ims_socket_send(request_info.t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE, IMS__ERROR__E_GENERIC_FAILURE, NULL, 0);
      response_sent = TRUE;
    }

    qcril_qmi_voice_info.last_call_failure_cause.pending_req = FALSE;
  }
  qcril_qmi_voice_last_call_failure_cause_unlock();

  if (response_sent)
  {

    qcril_qmi_voice_voip_lock_overview();
    qmi_ril_voice_review_call_objs_after_last_call_failure_response_vcl();
    qcril_qmi_voice_voip_unlock_overview();

  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_last_call_fail_request_timeout_handler

//===========================================================================
// qcril_qmi_voice_respond_ril_last_call_failure_request
//===========================================================================
void qcril_qmi_voice_respond_ril_last_call_failure_request()
{
  int ril_request = RIL_REQUEST_LAST_CALL_FAIL_CAUSE;

  qcril_request_resp_params_type resp;

  int                       call_obj_review_needed;
  qcril_reqlist_public_type request_info;

  QCRIL_LOG_FUNC_ENTRY();

  call_obj_review_needed    = FALSE;

  qcril_qmi_voice_last_call_failure_cause_lock();

  if ( TRUE == qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause_valid )
  {
    // find all pending request if we have and send the resoponse
    QCRIL_LOG_INFO("last_call_failure_cause is valid. Will send the response if there is any pending ril request.");
    while ( E_SUCCESS == qcril_reqlist_query_by_request(QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_LAST_CALL_FAIL_CAUSE, &request_info) ||
            E_SUCCESS == qcril_reqlist_query_by_request(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_IMS_SOCKET_REQ_LAST_CALL_FAIL_CAUSE, &request_info)
          )
    {
      call_obj_review_needed = TRUE;

      if ( RIL_REQUEST_LAST_CALL_FAIL_CAUSE == request_info.request )
      {
        QCRIL_LOG_DEBUG( "Reply to RIL --> Last call fail cause : %d", qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause);
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, request_info.t, request_info.request,
                                          RIL_E_SUCCESS, &resp );
        resp.resp_pkt = (void *) &qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause;
        resp.resp_len = sizeof( qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause );
        qcril_send_request_response( &resp );
      }
      else
      {
        Ims__CallFailCauseResponse lfc = IMS__CALL_FAIL_CAUSE_RESPONSE__INIT;
        lfc.has_failcause = TRUE;
        lfc.failcause = qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause;

        if (qcril_qmi_voice_info.last_call_failure_extended_codes_len > 0)
        {
          lfc.failcause = qcril_qmi_ims_map_ril_failcause_to_ims_failcause ( qcril_qmi_voice_info.last_call_failure_cause.last_call_failure_cause,
                                                                             atoi( (char*)qcril_qmi_voice_info.last_call_failure_extended_codes ) );
          lfc.has_errorinfo = TRUE;
          lfc.errorinfo.len = qcril_qmi_voice_info.last_call_failure_extended_codes_len;
          lfc.errorinfo.data = qcril_qmi_voice_info.last_call_failure_extended_codes;
        }

        qcril_qmi_ims_socket_send(request_info.t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE, RIL_E_SUCCESS, &lfc, sizeof(lfc));
      }

      qcril_qmi_voice_stk_cc_relay_alpha_if_necessary( QCRIL_DEFAULT_INSTANCE_ID, FALSE );
    }

    if ( call_obj_review_needed )
    {
      qmi_ril_voice_review_call_objs_after_last_call_failure_response_vcl();
    }

    if ( qcril_qmi_voice_info.last_call_failure_cause.pending_req )
    {
      qcril_qmi_voice_info.last_call_failure_cause.pending_req = FALSE;
      qcril_cancel_timed_callback((void *)(uintptr_t) qcril_qmi_voice_info.last_call_failure_cause.pending_request_timeout_timer_id);
    }
  }
  else
  {
    QCRIL_LOG_INFO("last_call_failure_cause is not valid. The response will be delayed after we get the valid value.");
  }
  qcril_qmi_voice_last_call_failure_cause_unlock();

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_respond_ril_last_call_failure_request

//===========================================================================
// qmi_ril_voice_review_call_objs_after_last_call_failure_response_vcl
//===========================================================================
void qmi_ril_voice_review_call_objs_after_last_call_failure_response_vcl( void )
{
  qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;

  int                       cleanup_evaluation_needed;

  QCRIL_LOG_FUNC_ENTRY();

  cleanup_evaluation_needed = FALSE;

  call_info = qcril_qmi_voice_voip_call_info_entries_enum_first();
  while ( NULL != call_info )
  {
    if ( VOICE_INVALID_CALL_ID != call_info->android_call_id &&
         CALL_STATE_END_V02 == call_info->voice_scv_info.call_state &&
         !( call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_LAST_CALL_FAILURE_REPORTED )
       )
    {
      call_info->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_LAST_CALL_FAILURE_REPORTED;

      cleanup_evaluation_needed = TRUE;
    }
    call_info = qcril_qmi_voice_voip_call_info_entries_enum_next();
  }

  if ( cleanup_evaluation_needed )
  {
    qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl();
  }
  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_review_call_objs_after_last_call_failure_response_vcl

//===========================================================================
// qcril_qmi_voice_handle_new_last_call_failure_cause
//===========================================================================
void qcril_qmi_voice_handle_new_last_call_failure_cause(int reason, boolean is_qmi_reason, qcril_qmi_voice_voip_call_info_entry_type *call_obj)
{
  QCRIL_LOG_FUNC_ENTRY();
  RIL_LastCallFailCause ril_reason;
  char extended_codes[MAX_DEC_INT_STR + 1] = {0};

  if ( is_qmi_reason )
  {
    ril_reason = qcril_qmi_voice_map_qmi_to_ril_last_call_failure_cause((call_end_reason_enum_v02)reason);
  }
  else
  {
    ril_reason = (call_end_reason_enum_v02)reason;
  }
  snprintf(extended_codes, MAX_DEC_INT_STR, "%d", reason);
  qcril_qmi_voice_store_last_call_failure_cause_ex(ril_reason, (uint8_t*)extended_codes, strlen(extended_codes));
  qcril_qmi_voice_respond_ril_last_call_failure_request();
  if (call_obj)
  {
     call_obj->lcf_valid = TRUE;
     call_obj->lcf = ril_reason;
     snprintf(call_obj->lcf_extended_codes, MAX_DEC_INT_STR, "%d", reason);
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_handle_new_last_call_failure_cause

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_send_unsol_call_state_changed

===========================================================================*/
/*!
    @brief
    Send RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED.

    @return
    none
*/
/*=========================================================================*/
void qcril_qmi_voice_send_unsol_call_state_changed
(
  qcril_instance_id_e_type instance_id
)
{
  qcril_unsol_resp_params_type unsol_resp;

  if( instance_id < QCRIL_MAX_INSTANCE_ID )
  {
    /* Report the call state change for voice or CS data calls */
    qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED, &unsol_resp );
    qcril_send_unsol_response( &unsol_resp );
  }
} /* qcril_qmi_voice_send_unsol_call_state_changed */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_make_incoming_call_ring
===========================================================================*/
void qcril_qmi_voice_make_incoming_call_ring (qcril_timed_callback_handler_params_type *param)
{
  qcril_qmi_voice_voip_call_info_entry_type*    call_info = NULL;
  qcril_unsol_resp_params_type unsol_resp;
  int                                           need_ring_to_ui;
  int                                           need_reiterate;
  int                                           is_1x;
  int                                           is_ims_call;

  static const struct timeval TIMEVAL_DELAY = {3,0}; // 3 seconds

  QCRIL_LOG_FUNC_ENTRY();

  need_ring_to_ui = FALSE;
  need_reiterate  = FALSE;
  is_1x           = FALSE;
  qcril_qmi_voice_voip_lock_overview();

  call_info = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING, TRUE );
  QCRIL_LOG_INFO(".. call obj %p", call_info );
  if ( call_info )
  {
    qcril_qmi_voice_voip_call_info_dump( call_info );

    call_info->ringing_time_id = QMI_RIL_ZERO;

    is_1x = (CALL_MODE_CDMA_V02 == call_info->voice_scv_info.mode) ? TRUE : FALSE;

    is_ims_call = qcril_qmi_voice_call_to_ims(call_info);

    if ( call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_RING_DONE || !is_1x )
    {
      call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_RING_DONE;
      need_ring_to_ui = TRUE;
    }

    if ( !is_1x && (call_info->voice_scv_info.call_state == CALL_STATE_SETUP_V02) ) // no follow up will be needed for 1x or non-setup call
    {
      need_reiterate = TRUE;
    }

    if ( !need_reiterate )
    {
      call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING;
    }
    else
    {
      call_info->elaboration |= QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID;
      qcril_setup_timed_callback_ex_params_adv( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                qcril_qmi_voice_make_incoming_call_ring,
                                                NULL, // nothing to pass through as no param for non 1x
                                                TRUE,
                                                &TIMEVAL_DELAY,
                                                &call_info->ringing_time_id );
    }
  }

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_INFO(".. need ring ui %d, need reiterate %d", (int) need_ring_to_ui, (int) need_reiterate );
  if ( need_ring_to_ui )
  {
    if (!is_ims_call)
    {
      qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_CALL_RING, &unsol_resp );
      if ( is_1x && NULL != param->custom_param )
      {
        QCRIL_LOG_INFO("signal_info signal_type: %d, alert_pitch: %d, signal: %d",
                      ((RIL_CDMA_SignalInfoRecord *)param->custom_param)->signalType,
                      ((RIL_CDMA_SignalInfoRecord *)param->custom_param)->alertPitch,
                      ((RIL_CDMA_SignalInfoRecord *)param->custom_param)->signal);

        unsol_resp.resp_pkt = ( void * ) param->custom_param;
        unsol_resp.resp_len = sizeof( RIL_CDMA_SignalInfoRecord );
      }
      qcril_send_unsol_response( &unsol_resp );
    }
    else
    {
      qcril_qmi_ims_socket_send_empty_payload_unsol_resp(IMS__MSG_ID__UNSOL_CALL_RING);
    }
  }

  if ( param->custom_param )
  {
    qcril_free( param->custom_param );
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_make_incoming_call_ring */

//===========================================================================
// qmi_ril_succeed_on_pending_hangup_req_on_no_calls_left
//===========================================================================
void qmi_ril_succeed_on_pending_hangup_req_on_no_calls_left(void)
{
  qcril_reqlist_public_type       request_info;
  IxErrnoType                     lookup_res;
  qcril_request_resp_params_type  resp;


  QCRIL_LOG_FUNC_ENTRY();

  do
  {
     lookup_res = qcril_reqlist_query_by_request( QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_HANGUP, &request_info );
     if ( E_SUCCESS == lookup_res )
       break;

     lookup_res = qcril_reqlist_query_by_request( QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, &request_info );
     if ( E_SUCCESS == lookup_res )
       break;

     lookup_res = qcril_reqlist_query_by_request( QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND, &request_info );
  } while (FALSE);

  if ( E_SUCCESS == lookup_res )
  { // a pending HANGUP_xxx request
      qcril_default_request_resp_params(QCRIL_DEFAULT_INSTANCE_ID,
                                        request_info.t,
                                        request_info.request,
                                        RIL_E_SUCCESS,
                                        &resp );
      qcril_send_request_response( &resp );
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_succeed_on_pending_hangup_req_on_no_calls_left

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_otasp_status_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_QMI_VOICE_OTASP_STATUS_IND

    @return
    RIL_CDMA_OTA_ProvisionStatus.
*/
/*=========================================================================*/
void qcril_qmi_voice_otasp_status_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_otasp_status_ind_msg_v02* otasp_status_ind_ptr = NULL;
  RIL_CDMA_OTA_ProvisionStatus response;
  qcril_unsol_resp_params_type unsol_resp;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;


  if( ind_data_ptr != NULL )
  {

      ind_data_len = ind_data_len;
      otasp_status_ind_ptr = (voice_otasp_status_ind_msg_v02*)ind_data_ptr;

      QCRIL_LOG_DEBUG( "QCRIL_EVT_QMI_VOICE_OTASP_STATUS_IND status = %d for conn id %d",
                           otasp_status_ind_ptr->otasp_status_info.otasp_status,
                           otasp_status_ind_ptr->otasp_status_info.call_id);

      response = (RIL_CDMA_OTA_ProvisionStatus)otasp_status_ind_ptr->otasp_status_info.otasp_status;
      qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_CDMA_OTA_PROVISION_STATUS, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) &response;
      unsol_resp.resp_len = sizeof( response );
      qcril_send_unsol_response( &unsol_resp );

      qcril_qmi_voice_voip_lock_overview();
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( otasp_status_ind_ptr->otasp_status_info.call_id );
      if ( call_info_entry )
      {
        call_info_entry->voice_svc_otasp_status = otasp_status_ind_ptr->otasp_status_info.otasp_status;
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_OTASP_STATUS_VALID;
        QCRIL_LOG_INFO( ".. caching otasp" );
      }
      qcril_qmi_voice_voip_unlock_overview();
  }

} /* qcril_qmi_voice_otasp_status_ind_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_privacy_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_VOICE_PRIVACY_IND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_privacy_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_privacy_ind_msg_v02* privacy_ind_ptr = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
  boolean unsol_call_state_change_to_atel = FALSE;
  boolean unsol_call_state_change_to_ims = FALSE;

  if( ind_data_ptr != NULL )
  {
      ind_data_len = ind_data_len;
      privacy_ind_ptr = (voice_privacy_ind_msg_v02*)ind_data_ptr;

      QCRIL_LOG_INFO("Privacy indication received with privacy %d for conn id %d",
                                   privacy_ind_ptr->voice_privacy_info.voice_privacy,
                                   privacy_ind_ptr->voice_privacy_info.call_id);

      qcril_qmi_voice_voip_lock_overview();
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( privacy_ind_ptr->voice_privacy_info.call_id );
      if ( call_info_entry )
      {
        call_info_entry->voice_svc_voice_privacy = privacy_ind_ptr->voice_privacy_info.voice_privacy;
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VOICE_PRIVACY_VALID;
        QCRIL_LOG_INFO( ".. caching voice privacy" );
        unsol_call_state_change_to_atel = qcril_qmi_voice_call_to_atel(call_info_entry);
        unsol_call_state_change_to_ims  = qcril_qmi_voice_call_to_ims (call_info_entry);
      }
      qcril_qmi_voice_voip_unlock_overview();

      if (unsol_call_state_change_to_atel)
      {
         qcril_qmi_voice_send_unsol_call_state_changed( QCRIL_DEFAULT_INSTANCE_ID );
      }

      if (unsol_call_state_change_to_ims)
      {
         qcril_qmi_voice_send_ims_unsol_call_state_changed();
      }
  }

}/* qcril_qmi_voice_privacy_ind_hdlr */

//===========================================================================
// qcril_qmi_voice_uus_ind_hdlr
//===========================================================================
void qcril_qmi_voice_uus_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_uus_ind_msg_v02* uus_ind_ptr = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  if( ind_data_ptr != NULL )
  {
      ind_data_len = ind_data_len;
      uus_ind_ptr = (voice_uus_ind_msg_v02*)ind_data_ptr;

      QCRIL_LOG_INFO("UUS indication received for call id %d with type %d, dcs %d, data len %d ",
                                   (int)uus_ind_ptr->uus_information.call_id,
                                   (int)uus_ind_ptr->uus_information.uus_type,
                                   (int)uus_ind_ptr->uus_information.uus_dcs,
                                   (int)uus_ind_ptr->uus_information.uus_data_len
                     );

      qcril_qmi_voice_voip_lock_overview();
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( uus_ind_ptr->uus_information.call_id );
      if ( call_info_entry )
      {
        qcril_qmi_voice_voip_update_call_info_uus(
                                                  call_info_entry,
                                                  uus_ind_ptr->uus_information.uus_type,
                                                  uus_ind_ptr->uus_information.uus_dcs,
                                                  uus_ind_ptr->uus_information.uus_data_len,
                                                  uus_ind_ptr->uus_information.uus_data
                                                 );
      }
      qcril_qmi_voice_voip_unlock_overview();
  }

} // qcril_qmi_voice_uus_ind_hdlr

//===========================================================================
// qcril_qmi_voice_respond_pending_hangup_ril_response
//===========================================================================
void qcril_qmi_voice_respond_pending_hangup_ril_response(uint8_t call_id)
{
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  qcril_reqlist_public_type request_info;

  QCRIL_LOG_FUNC_ENTRY();

  call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( call_id );

  if ( (NULL != call_info_entry) && (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_HANGUP_RESP) )
  {
    if (E_SUCCESS == qcril_reqlist_query(QCRIL_DEFAULT_INSTANCE_ID, call_info_entry->pending_end_call_req_tid, &request_info))
    {
      qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, request_info.t, request_info.request, RIL_E_SUCCESS );
      call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_HANGUP_RESP;
    }
    else
    {
      QCRIL_LOG_ERROR("no pending Telphony end call request is found");
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_respond_pending_hangup_ril_response

//===========================================================================
// qmi_ril_voice_pending_1x_num_timeout
//===========================================================================
void qmi_ril_voice_pending_1x_num_timeout(void * param)
{
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qcril_qmi_voice_voip_lock_overview();

    if ( QMI_RIL_ZERO != qmi_voice_voip_overview.num_1x_wait_timer_id )
    {
      qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.num_1x_wait_timer_id );
      qmi_voice_voip_overview.num_1x_wait_timer_id = QMI_RIL_ZERO;
    }

    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING, TRUE );
    if ( call_info_entry )
    {
      call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING;

      qcril_qmi_voice_send_unsol_call_state_changed( QCRIL_DEFAULT_INSTANCE_ID );
    }

    qcril_qmi_voice_voip_unlock_overview();

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_pending_1x_num_timeout

//===========================================================================
// qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl
//===========================================================================
void qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl(const voice_call_info2_type_v02* call_info)
{
   if ( CALL_MODE_CDMA_V02 == call_info->mode &&
        ( CALL_TYPE_VOICE_V02 == call_info->call_type || CALL_TYPE_VOICE_FORCED_V02 == call_info->call_type ) &&
        VOICE_INVALID_CALL_ID == qmi_voice_voip_overview.cdma_call_id
      )
   {
      qmi_voice_voip_overview.cdma_call_id = call_info->call_id;
      QCRIL_LOG_INFO("set cdma call id to %d", qmi_voice_voip_overview.cdma_call_id);
   }
} // qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl

//===========================================================================
// qcril_qmi_voice_reset_cdma_call_id_if_applicable_vcl
//===========================================================================
void qcril_qmi_voice_reset_cdma_call_id_if_applicable_vcl(const voice_call_info2_type_v02* call_info)
{
   if ( CALL_MODE_CDMA_V02 == call_info->mode &&
        ( CALL_TYPE_VOICE_V02 == call_info->call_type || CALL_TYPE_VOICE_FORCED_V02 == call_info->call_type ) &&
        call_info->call_id == qmi_voice_voip_overview.cdma_call_id &&
        CALL_STATE_END_V02 == call_info->call_state
      )
   {
      qmi_voice_voip_overview.cdma_call_id = VOICE_INVALID_CALL_ID;
      QCRIL_LOG_INFO("reset cdma call id to VOICE_INVALID_CALL_ID");
   }
} // qcril_qmi_voice_reset_cdma_call_id_if_applicable_vcl

//===========================================================================
// qcril_qmi_voice_set_ps_cs_call_elab_vcl
//===========================================================================
void qcril_qmi_voice_set_ps_cs_call_elab_vcl(const voice_call_info2_type_v02* iter_call_info, qcril_qmi_voice_voip_call_info_entry_type *call_info_entry)
{
   call_mode_enum_v02 call_mode;
   call_type_enum_v02 call_type;

   if (NULL == call_info_entry)
   {
      QCRIL_LOG_ERROR("call_info_entry is NULL");
   }
   else
   {
      if (NULL != iter_call_info)
      {
         call_type = iter_call_info->call_type;
         call_mode = iter_call_info->mode;
      }
      else
      {
         call_type = call_info_entry->voice_scv_info.call_type;
         call_mode = call_info_entry->voice_scv_info.mode;
      }

      if ( CALL_TYPE_VOICE_IP_V02 == call_type ||
           CALL_TYPE_VT_V02 == call_type ||
           CALL_TYPE_EMERGENCY_IP_V02 == call_type
         )
      {
         if ( !(call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN) ||
              CALL_MODE_NO_SRV_V02 != call_mode
            )
         {
            // if the call type is a PS call, and we either get a determined call_mode or it is not a dialed call on auto domain, set it as a PS call
            QCRIL_LOG_INFO("set the call as a PS call");
            call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
            call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
         }
      }
      else if (CALL_MODE_NO_SRV_V02 != call_mode && CALL_MODE_UNKNOWN_V02 != call_mode && CALL_MODE_LTE_V02 != call_mode)
      {
         // if the call type is a CS call and the call_mode is determined(not no_srv), set it as a CS call
         QCRIL_LOG_INFO("set the call as a CS call");
         call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
         call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
         call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
      }
   }
} // qcril_qmi_voice_set_ps_cs_call_elab_vcl

void qcril_qmi_voice_set_domain_elab_from_call_type
(
  call_type_enum_v02 call_type,
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry
)
{
    if ( CALL_TYPE_VOICE_IP_V02 == call_type ||
         CALL_TYPE_VT_V02 == call_type ||
         CALL_TYPE_EMERGENCY_IP_V02 == call_type
       )
    {
        QCRIL_LOG_INFO("set the call as a PS call");
        call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
        call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
    }
    else
    {
        QCRIL_LOG_INFO("set the call as a CS call");
        call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
        call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
    }
}

//===========================================================================
// qcril_qmi_voice_auto_answer_if_needed
//===========================================================================
void qcril_qmi_voice_auto_answer_if_needed()
{
  char                                        property_name[ 40 ];
  char                                        args[ PROPERTY_VALUE_MAX ];
  int                                         auto_ans_len;
  char                                        *end_ptr;
  long int                                    ret_val = 0;
  uint32                                      auto_answer_timerid;
  int                                         auto_answer_tmr_res;
  struct timeval auto_answer_timeout = { 0 , 0 };

  QCRIL_SNPRINTF( property_name, sizeof( property_name ), "%s", QCRIL_QMI_AUTO_ANSWER);
  property_get( property_name, args, "" );
  auto_ans_len = strlen( args );
  if ( auto_ans_len > 0 )
  {
    ret_val = strtol( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ret_val == LONG_MAX ) )
    {
      QCRIL_LOG_ERROR( "QCRIL QMI VOICE Fail to convert QCRIL_QMI_AUTO_ANSWER %s", args );
    }
    else
    {
      QCRIL_LOG_INFO("QCRIL_QMI_AUTO_ANSWER %d",ret_val);
    }
  }
  if( ret_val > 0 )
  {
    if ( QMI_RIL_ZERO != qmi_voice_voip_overview.auto_answer_timer_id )
    {
      QCRIL_LOG_INFO("Cancel Auto Answer timed callback");
      qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.auto_answer_timer_id );
      qmi_voice_voip_overview.auto_answer_timer_id = QMI_RIL_ZERO;
    }
    auto_answer_timerid = QMI_RIL_ZERO;
    auto_answer_timeout.tv_sec = ret_val/1000;
    QCRIL_LOG_INFO("Auto answer timeout %d", (int)auto_answer_timeout.tv_sec);
    auto_answer_tmr_res = qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                                  QCRIL_DEFAULT_MODEM_ID,
                                                  qcril_qmi_voice_auto_answer_timeout_handler,
                                                  &auto_answer_timeout,
                                                  &auto_answer_timerid );
    QCRIL_LOG_INFO("Auto answer tmr post res %d, id %d", (int) auto_answer_tmr_res, (int)auto_answer_timerid );
    if ( QMI_RIL_ZERO != auto_answer_timerid )
    {
      qmi_voice_voip_overview.auto_answer_timer_id = auto_answer_timerid;
    }
  }
  else
  {
    QCRIL_LOG_INFO("Auto answer disabled!");
  }
} // qcril_qmi_voice_auto_answer_if_needed

//===========================================================================
// qcril_qmi_voice_is_qmi_call_emergency
//===========================================================================
boolean qcril_qmi_voice_is_qmi_call_emergency(const voice_call_info2_type_v02* iter_call_info)
{
   return (CALL_TYPE_EMERGENCY_V02 == iter_call_info->call_type || CALL_TYPE_EMERGENCY_IP_V02 == iter_call_info->call_type);
} // qcril_qmi_voice_is_qmi_call_emergency

//===========================================================================
// qcril_qmi_voice_voip_change_call_elab_when_first_call_ind_received
//===========================================================================
void qcril_qmi_voice_voip_change_call_elab_when_first_call_ind_received(qcril_qmi_voice_voip_call_info_entry_type *call_info_entry)
{
   if (call_info_entry)
   {
      if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_QMI_ID_RECEIVED)
      {
         call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_QMI_ID_RECEIVED;
         call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;
      }
   }
   else
   {
      QCRIL_LOG_ERROR("call_info_entry is NULL");
   }
} // qcril_qmi_voice_voip_change_call_elab_when_first_call_ind_received

//===========================================================================
// qcril_qmi_voice_check_and_update_srvcc_no_mid_call_support
//===========================================================================
static void qcril_qmi_voice_check_and_update_srvcc_no_mid_call_support
(
 qcril_qmi_voice_voip_call_info_entry_type *call_info_entry,
 voice_call_info2_type_v02                 *iter_call_info
)
{
  qcril_qmi_voice_voip_call_info_entry_type * mpty_call_info = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  // In case of conference call SRVCC without mid-call SRVCC support, modem will
  // report only one call after SRVCC with the call id same as the previous call.
  // In this case, our call_info_entry which got updated for this indications
  // would be a MPTY_CALL with list of participants in the mpty_voip_call_list.
  if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID &&
      call_info_entry->mpty_voip_call_list )
  {
    QCRIL_LOG_INFO("Conference call SRVCC without mid-call SRVCC support");

    // Ensure the call is set as mpty. After SRVCC, if mid-call SRVCC is not supported,
    // modem will be notifying the call with mpty set to FALSE.
    if (!call_info_entry->voice_scv_info.is_mpty)
    {
      call_info_entry->voice_scv_info.is_mpty = TRUE;
    }

    mpty_call_info = call_info_entry->mpty_voip_call_list;

    // Set first participant as CS DOMAIN - to make sure this call is report to rild socket
    qcril_qmi_voice_set_ps_cs_call_elab_vcl(iter_call_info, mpty_call_info);

    // Set first participant qmi_call_id as parent qmi_call_id - to make sure any
    // operation on the android_call_id to map to actual qmi_call_id
    mpty_call_info->qmi_call_id = call_info_entry->qmi_call_id;
    mpty_call_info->media_id = call_info_entry->media_id;

    // Reset the MEMBER_OF_MPTY_VOIP_CALL ELAB from remaining participats
    // But keep the list - these are dummy calls to report to the IMS socket.
    mpty_call_info = mpty_call_info->mpty_voip_call_list;
    while (mpty_call_info &&
           mpty_call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_MEMBER_OF_MPTY_VOIP_CALL)
    {
      mpty_call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_MEMBER_OF_MPTY_VOIP_CALL;
      mpty_call_info = mpty_call_info->mpty_voip_call_list;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_check_and_update_srvcc_no_mid_call_support

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_all_call_status_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_VOICE_ALL_CALL_STATUS_IND_V02.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_all_call_status_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_all_call_status_ind_msg_v02* call_status_ind_ptr = NULL;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  unsigned int i,j;
  int local_ringback_payload = FALSE;
  qcril_unsol_resp_params_type unsol_resp;
  RIL_CDMA_SignalInfoRecord signal_info_rec;
  uint32_t nof_ims_calls = 0;
  uint32_t nof_atel_calls = 0;
  boolean  hangup_call    = FALSE;
  int res = 0;

  /* variables defined to track modem initiated silent redial */
  boolean is_request_on_autodomain = FALSE;
  boolean is_call_on_cs_domain     = FALSE;
  boolean is_silent_redialed       = FALSE;

  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  qcril_qmi_voice_voip_call_info_entry_type * parent_call_info_entry = NULL;
  qcril_qmi_voice_voip_call_info_entry_type * mpty_call_info = NULL;
  voice_call_info2_type_v02*                  iter_call_info = NULL;
  uint8_t                                     remote_party_number_valid;
  voice_remote_party_number2_type_v02*        remote_party_number = NULL;
  uint8_t                                     remote_party_name_valid;
  voice_remote_party_name2_type_v02*          remote_party_name = NULL;
  uint8_t                                     alerting_type_valid;
  voice_alerting_type_type_v02*               alerting_type = NULL;
  uint8_t                                     srv_opt_valid;
  voice_srv_opt_type_v02*                     srv_opt = NULL;
  uint8_t                                     call_end_reason_valid;
  voice_call_end_reason_type_v02*             call_end_reason = NULL;
  uint8_t                                     alpha_id_valid;
  voice_alpha_ident_with_id_type_v02*         alpha_id = NULL;
  uint8_t                                     conn_party_num_valid;
  voice_conn_num_with_id_type_v02*            conn_party_num = NULL;
  uint8_t                                     diagnostic_info_valid;
  voice_diagnostic_info_with_id_type_v02*     diagnostic_info = NULL;
  uint8_t                                     called_party_num_valid;
  voice_num_with_id_type_v02*                 called_party_num = NULL;
  uint8_t                                     redirecting_party_num_valid;
  voice_num_with_id_type_v02*                 redirecting_party_num = NULL;
  uint8_t                                     ip_num_info_valid;
  voice_ip_num_id_type_v02*                   ip_num_info = NULL;
  uint8_t                                     conn_ip_num_info_valid;
  voice_conn_ip_num_with_id_type_v02*         conn_ip_num_info = NULL;
  uint8_t                                     is_add_info_present_valid;
  voice_is_add_info_present_with_id_type_v02* is_add_info_present = NULL;
  uint8_t                                     ip_caller_name_valid;
  voice_ip_caller_name_info_type_v02*         ip_caller_name = NULL;
  uint8_t                                     end_reason_text_valid;
  voice_ip_end_reason_text_type_v02*          end_reason_text = NULL;

  uint8_t                                     ril_call_state_valid;
  RIL_CallState                               ril_call_state;
  uint8_t                                     iter_call_id;
  int                                         calls_nof_remaining;
  int                                         calls_iter;
  int                                         is_deviant_call;
  int                                         is_mode_resonable;

  int                                         imperative_report;
  int                                         post_call_cleanup_may_be_necessary;

  uint8_t                                     audio_attrib_valid;
  voice_call_attributes_type_v02              *audio_attrib = NULL;
  uint8_t                                     video_attrib_valid;
  voice_call_attributes_type_v02              *video_attrib = NULL;

  uint8_t                                     call_attrib_status_valid;
  voice_call_attrib_status_type_v02           *call_attrib_status = NULL;
  qmi_ril_nw_reg_rte_type                     current_voice_rte;
  int                                         reported_voice_radio_tech;
  qcril_qmi_ims_voice_conf_state ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_NO_CONF;

  qcril_qmi_voice_voip_call_info_entry_type * cdma_voice_call_info_entry = NULL;

  qcril_qmi_voice_voip_current_call_summary_type
                                              calls_summary_beginning;
  qcril_qmi_voice_voip_current_call_summary_type
                                              calls_summary_end;
  int                                         need_1x_num_pending_timeout;
  const struct timeval                        num_1x_wait_timeout = { 1 , 0 }; // 1 second
  uint32                                      num_1x_wait_timer_id;

  const struct timeval                        call_obj_phaseout_auto_delay = { 3 , 0 }; // 3 seconds

  char                                        log_essence[ QCRIL_MAX_LOG_MSG_SIZE ];
  char                                        log_addon[ QCRIL_MAX_LOG_MSG_SIZE ];

  uint8_t is_srvcc_valid;
  voice_is_srvcc_call_with_id_type_v02 *is_srvcc = NULL;

  uint8_t srvcc_parent_call_info_valid;
  voice_srvcc_parent_call_id_type_v02 *srvcc_parent_call_info = NULL;

  uint8_t local_call_capabilities_info_valid;
  voice_ip_call_capabilities_info_type_v02 *local_call_capabilities_info = NULL;

  uint8_t peer_call_capabilities_info_valid;
  voice_ip_call_capabilities_info_type_v02 *peer_call_capabilities_info = NULL;

  uint8_t child_number_valid;
  voice_child_number_info_type_v02 *child_number = NULL;

  uint8_t display_text_valid;
  voice_display_text_info_type_v02 *display_text = NULL;

  char  parentCallID[25] = "\0";
  char  callID[5] = "\0";

  int32_t iter_media_id = INVALID_MEDIA_ID;

  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type
                                              oversight_event_params;

  QCRIL_LOG_FUNC_ENTRY();

  memset( &calls_summary_beginning, 0, sizeof( calls_summary_beginning ) );
  memset( &calls_summary_end, 0, sizeof( calls_summary_end ) );

  snprintf( log_essence, QCRIL_MAX_LOG_MSG_SIZE,
            "RILVIMS: update" );

  if( ind_data_ptr != NULL )
  {
      call_status_ind_ptr = (voice_all_call_status_ind_msg_v02*)ind_data_ptr;
      ind_data_len = ind_data_len;

      need_1x_num_pending_timeout = FALSE;

      post_call_cleanup_may_be_necessary = FALSE;

      snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE,
                " %d calls", call_status_ind_ptr->call_info_len );
      strlcat( log_essence, log_addon, sizeof( log_essence ) );

      current_voice_rte           = qmi_ril_nw_reg_get_current_rte_of_kind( QMI_RIL_RTE_KIND_VOICE );
      reported_voice_radio_tech   = qcril_qmi_nas_get_reported_voice_radio_tech(TRUE);

      cdma_voice_call_info_entry = NULL;

      qcril_qmi_voice_voip_lock_overview();

      qcril_qmi_voice_voip_generate_summary( &calls_summary_beginning );

      for ( i = 0; i < call_status_ind_ptr->call_info_len; i++ )
      {
        // call info
        iter_call_info = &call_status_ind_ptr->call_info[i];
        iter_call_id   = iter_call_info->call_id;

        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
        if ( call_info_entry )
        {
          snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE,
                    "[a-c-id: %d, q-c-id %d, c-s %d -> %d]",
                        (int)call_info_entry->android_call_id,
                        (int)iter_call_info->call_id,
                        (int)call_info_entry->voice_scv_info.call_state,
                        (int)iter_call_info->call_state
                 );
        }
        else
        {
          snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE,
                    "[uncached qmi call id %d, call state %d]", (int)iter_call_info->call_id, (int)iter_call_info->call_state );
        }
        strlcat( log_essence, log_addon, sizeof( log_essence ) );


        // remote party number
        remote_party_number_valid = FALSE;
        remote_party_number       = NULL;
        for ( j = 0; j < call_status_ind_ptr->remote_party_number_len && !remote_party_number_valid; j++ )
        {
          if ( call_status_ind_ptr->remote_party_number[ j ].call_id == iter_call_id )
          {
            remote_party_number_valid = TRUE;
            remote_party_number       = &call_status_ind_ptr->remote_party_number[ j ];
          }
        }
        // remote party name
        remote_party_name_valid = FALSE;
        remote_party_name = NULL;
        for ( j = 0; j < call_status_ind_ptr->remote_party_name_len && !remote_party_name_valid; j++ )
        {
          if ( call_status_ind_ptr->remote_party_name[ j ].call_id == iter_call_id )
          {
            remote_party_name_valid = TRUE;
            remote_party_name = &call_status_ind_ptr->remote_party_name[ j ];
          }
        }
        // altering type
        alerting_type_valid = FALSE;
        alerting_type = NULL;
        for ( j = 0; j < call_status_ind_ptr->alerting_type_len && !alerting_type_valid; j++ )
        {
          if ( call_status_ind_ptr->alerting_type[ j ].call_id == iter_call_id )
          {
             alerting_type_valid = TRUE;
             alerting_type = &call_status_ind_ptr->alerting_type[ j ];
          }
        }
        // srv op
        srv_opt_valid = FALSE;
        srv_opt = NULL;
        for ( j = 0; j < call_status_ind_ptr->srv_opt_len && !srv_opt_valid; j++  )
        {
          if ( call_status_ind_ptr->srv_opt[ j ].call_id == iter_call_id )
          {
            srv_opt_valid = TRUE;
            srv_opt = &call_status_ind_ptr->srv_opt[ j ];
          }
        }
        // call end reason
        call_end_reason_valid = FALSE;
        call_end_reason = NULL;
        for ( j = 0; j < call_status_ind_ptr->call_end_reason_len && !call_end_reason_valid; j++ )
        {
          if ( call_status_ind_ptr->call_end_reason[ j ].call_id == iter_call_id )
          {
            call_end_reason_valid = TRUE;
            call_end_reason = &call_status_ind_ptr->call_end_reason[ j ];
          }
        }
        // alpha id
        alpha_id_valid = FALSE;
        alpha_id = NULL;
        for ( j = 0; j < call_status_ind_ptr->alpha_id_len && !alpha_id_valid; j++ )
        {
          if ( call_status_ind_ptr->alpha_id[ j ].call_id == iter_call_id )
          {
            alpha_id = &call_status_ind_ptr->alpha_id[ j ];
            alpha_id_valid = TRUE;
          }
        }
        // conn party num
        conn_party_num_valid = FALSE;
        conn_party_num = NULL;
        for ( j = 0; j < call_status_ind_ptr->conn_party_num_len && !conn_party_num_valid; j++ )
        {
          if ( call_status_ind_ptr->conn_party_num[ j ].call_id == iter_call_id )
          {
            conn_party_num_valid = TRUE;
            conn_party_num = &call_status_ind_ptr->conn_party_num[ j ];
          }
        }
        // diagnostic info
        diagnostic_info_valid = FALSE;
        diagnostic_info = NULL;
        for ( j = 0; j < call_status_ind_ptr->diagnostic_info_len && !diagnostic_info_valid; j++ )
        {
          if ( call_status_ind_ptr->diagnostic_info[ j ].call_id == iter_call_id )
          {
            diagnostic_info_valid = TRUE;
            diagnostic_info = &call_status_ind_ptr->diagnostic_info[ j ];
          }
        }
        // called party num
        called_party_num_valid = FALSE;
        called_party_num = NULL;
        for ( j = 0; j < call_status_ind_ptr->called_party_num_len && !called_party_num_valid; j++ )
        {
          if ( call_status_ind_ptr->called_party_num[ j ].call_id == iter_call_id )
          {
            called_party_num_valid = TRUE;
            called_party_num = &call_status_ind_ptr->called_party_num[ j ];
          }
        }
        // redirecting party num
        redirecting_party_num_valid = FALSE;
        redirecting_party_num = NULL;
        for ( j = 0; j < call_status_ind_ptr->redirecting_party_num_len && !redirecting_party_num_valid; j++ )
        {
          if ( call_status_ind_ptr->redirecting_party_num[ j ].call_id == iter_call_id )
          {
            redirecting_party_num_valid = TRUE;
            redirecting_party_num = &call_status_ind_ptr->redirecting_party_num[ j ];
          }
        }
        // audio attributes
        audio_attrib_valid = FALSE;
        audio_attrib = NULL;
        for ( j = 0; j < call_status_ind_ptr->audio_attrib_len && !audio_attrib_valid; j++ )
        {
          if ( call_status_ind_ptr->audio_attrib[ j ].call_id == iter_call_id )
          {
            audio_attrib_valid = TRUE;
            audio_attrib = &call_status_ind_ptr->audio_attrib[ j ];
          }
        }
        // video attributes
        video_attrib_valid = FALSE;
        video_attrib = NULL;
        for ( j = 0; j < call_status_ind_ptr->video_attrib_len && !video_attrib_valid; j++ )
        {
          if ( call_status_ind_ptr->video_attrib[ j ].call_id == iter_call_id )
          {
            video_attrib_valid = TRUE;
            video_attrib = &call_status_ind_ptr->video_attrib[ j ];
          }
        }
        // call attribute status
        call_attrib_status_valid = FALSE;
        call_attrib_status = NULL;
        for ( j = 0; j < call_status_ind_ptr->call_attrib_status_len && ! call_attrib_status_valid; j++)
        {
           if ( call_status_ind_ptr->call_attrib_status[j].call_id == iter_call_id)
           {
              call_attrib_status_valid = TRUE;
              call_attrib_status = &call_status_ind_ptr->call_attrib_status[j];
           }
        }

        // SRVCC Call
        is_srvcc_valid = FALSE;
        is_srvcc = NULL;
        for ( j = 0; j < call_status_ind_ptr->is_srvcc_len && !is_srvcc_valid; j++ )
        {
          if ( call_status_ind_ptr->is_srvcc[ j ].call_id == iter_call_id )
          {
            is_srvcc_valid = TRUE;
            is_srvcc = &call_status_ind_ptr->is_srvcc[ j ];
          }
        }

        // Mid Call SRVCC
        srvcc_parent_call_info_valid = FALSE;
        srvcc_parent_call_info = NULL;
        for ( j = 0; j < call_status_ind_ptr->srvcc_parent_call_info_len && !srvcc_parent_call_info_valid; j++ )
        {
          if ( call_status_ind_ptr->srvcc_parent_call_info[ j ].call_id == iter_call_id )
          {
            srvcc_parent_call_info_valid = TRUE;
            srvcc_parent_call_info = &call_status_ind_ptr->srvcc_parent_call_info[ j ];
          }
        }

        // Local Call Capabilities
        local_call_capabilities_info_valid = FALSE;
        local_call_capabilities_info = NULL;
        for ( j = 0; j < call_status_ind_ptr->local_call_capabilities_info_len && !local_call_capabilities_info_valid; j++ )
        {
          if ( call_status_ind_ptr->local_call_capabilities_info[ j ].call_id == iter_call_id )
          {
            local_call_capabilities_info_valid = TRUE;
            local_call_capabilities_info = &call_status_ind_ptr->local_call_capabilities_info[ j ];
          }
        }

        // Peer Call Capabilities
        peer_call_capabilities_info_valid = FALSE;
        peer_call_capabilities_info = NULL;
        for ( j = 0; j < call_status_ind_ptr->peer_call_capabilities_info_len && !peer_call_capabilities_info_valid; j++ )
        {
          if ( call_status_ind_ptr->peer_call_capabilities_info[ j ].call_id == iter_call_id )
          {
            peer_call_capabilities_info_valid = TRUE;
            peer_call_capabilities_info = &call_status_ind_ptr->peer_call_capabilities_info[ j ];
          }
        }

        // Child Number
        child_number_valid = FALSE;
        child_number = NULL;
        for ( j = 0; j < call_status_ind_ptr->child_number_len && !child_number_valid; j++ )
        {
          if ( call_status_ind_ptr->child_number[ j ].call_id == iter_call_id )
          {
            child_number_valid = TRUE;
            child_number = &call_status_ind_ptr->child_number[ j ];
          }
        }

        // Display Text
        display_text_valid = FALSE;
        display_text = NULL;
        for ( j = 0; j < call_status_ind_ptr->display_text_len && !display_text_valid; j++ )
        {
          if ( call_status_ind_ptr->display_text[ j ].call_id == iter_call_id )
          {
            display_text_valid = TRUE;
            display_text = &call_status_ind_ptr->display_text[ j ];
          }
        }

        // Remote Party Number Extension
        ip_num_info_valid = FALSE;
        ip_num_info = NULL;
        for ( j = 0; j < call_status_ind_ptr->ip_num_info_len && !ip_num_info_valid; j++ )
        {
          if ( call_status_ind_ptr->ip_num_info[ j ].call_id == iter_call_id )
          {
            ip_num_info_valid = TRUE;
            ip_num_info = &call_status_ind_ptr->ip_num_info[ j ];
          }
        }

        // Connected Party Number Extension
        conn_ip_num_info_valid = FALSE;
        conn_ip_num_info = NULL;
        for ( j = 0; j < call_status_ind_ptr->conn_ip_num_info_len && !conn_ip_num_info_valid; j++ )
        {
          if ( call_status_ind_ptr->conn_ip_num_info[ j ].call_id == iter_call_id )
          {
            conn_ip_num_info_valid = TRUE;
            conn_ip_num_info = &call_status_ind_ptr->conn_ip_num_info[ j ];
          }
        }

        // Additional Call Info
        is_add_info_present_valid = FALSE;
        is_add_info_present = NULL;
        for ( j = 0; j < call_status_ind_ptr->is_add_info_present_len && !is_add_info_present_valid; j++ )
        {
          if ( call_status_ind_ptr->is_add_info_present[ j ].call_id == iter_call_id )
          {
            is_add_info_present_valid = TRUE;
            is_add_info_present = &call_status_ind_ptr->is_add_info_present[ j ];
          }
        }

        // media_id
        iter_media_id       = INVALID_MEDIA_ID;
        for ( j = 0; j < call_status_ind_ptr->media_id_len; j++ )
        {
          if ( call_status_ind_ptr->media_id[ j ].call_id == iter_call_id )
          {
            iter_media_id       = call_status_ind_ptr->media_id[ j ].media_id;
            break;
          }
        }


        // IP Caller name
        ip_caller_name_valid = FALSE;
        ip_caller_name = NULL;
        for ( j = 0; j < call_status_ind_ptr->ip_caller_name_len && !ip_caller_name_valid; j++ )
        {
          if ( call_status_ind_ptr->ip_caller_name[ j ].call_id == iter_call_id )
          {
            ip_caller_name_valid = TRUE;
            ip_caller_name = &call_status_ind_ptr->ip_caller_name[ j ];
          }
        }

        // End Reason Text for IP call
        end_reason_text_valid = FALSE;
        end_reason_text = NULL;
        for ( j = 0; j < call_status_ind_ptr->end_reason_text_len && !end_reason_text_valid; j++ )
        {
          if ( call_status_ind_ptr->end_reason_text[ j ].call_id == iter_call_id )
          {
            end_reason_text_valid = TRUE;
            end_reason_text = &call_status_ind_ptr->end_reason_text[ j ];
          }
        }

        // ril call state
        ril_call_state_valid = FALSE;
        ril_call_state = 0;

        call_info_entry = NULL;

        QCRIL_LOG_INFO("call state %d, IsMT=%d", iter_call_info->call_state, iter_call_info->direction);
        switch ( iter_call_info->call_state )
        {
          case CALL_STATE_INCOMING_V02:
            QCRIL_LOG_ESSENTIAL("call state INCOMING for conn id %d", iter_call_info->call_id);
            ril_call_state_valid = TRUE;
            ril_call_state = RIL_CALL_INCOMING;

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
            if ( NULL == call_info_entry )
            { // fresh
              call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                      iter_call_info->call_id,
                      iter_media_id,
                      TRUE,
                      QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING );
            }
            else
            { // what we got?
              call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING;
              qcril_qmi_voice_voip_call_info_dump( call_info_entry );
            }

            if ( call_info_entry )
            {
              if ( CALL_STATE_SETUP_V02 == call_info_entry->voice_scv_info.call_state &&
                   call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID )
              {
                qcril_cancel_timed_callback((void *)(uintptr_t) call_info_entry->ringing_time_id);
                call_info_entry->ringing_time_id = QMI_RIL_ZERO;
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID;
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING;
              }

              if ( CALL_TYPE_VOICE_IP_V02 == iter_call_info->call_type || CALL_TYPE_VT_V02 == iter_call_info->call_type )
              {
                 qcril_qmi_ims_socket_send_empty_payload_unsol_resp(IMS__MSG_ID__UNSOL_CALL_RING);
              }

              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );

              qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( call_info_entry );

              qcril_qmi_voice_set_domain_elab_from_call_type(iter_call_info->call_type, call_info_entry);
              qcril_qmi_voice_set_audio_call_type(iter_call_info, call_info_entry);

              if ( ( QMI_RIL_RTE_SUB_LTE == current_voice_rte || RADIO_TECH_LTE == reported_voice_radio_tech )
                    && CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type
                 )
              { // 1x CSFB MT call
                call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL;
              }
              else if ( CALL_MODE_NO_SRV_V02 == call_info_entry->voice_scv_info.mode &&
                        CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type  )
              { // 1x or GW CSFB MT call
                call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL;
              }

              switch ( iter_call_info->call_type )
              {
                case CALL_TYPE_VOICE_V02:
                case CALL_TYPE_VOICE_FORCED_V02:
                  if ( CALL_MODE_CDMA_V02 == iter_call_info->mode && ( ! ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID ) ) )
                  {
                    call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING;

                    if ( QMI_RIL_ZERO != qmi_voice_voip_overview.num_1x_wait_timer_id )
                    {
                      qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.num_1x_wait_timer_id );
                      qmi_voice_voip_overview.num_1x_wait_timer_id = QMI_RIL_ZERO;
                    }

                    num_1x_wait_timer_id = QMI_RIL_ZERO;
                    qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                qmi_ril_voice_pending_1x_num_timeout,
                                                &num_1x_wait_timeout,
                                                &num_1x_wait_timer_id );

                    if ( QMI_RIL_ZERO != num_1x_wait_timer_id )
                    {
                      qmi_voice_voip_overview.num_1x_wait_timer_id = num_1x_wait_timer_id;
                    }

                  }
                  break;

                default:
                  // nothing
                  break;

              }

            }

            qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl(iter_call_info);

            // Answer call if auto answer is enabled
            qcril_qmi_voice_auto_answer_if_needed();

            // Register for the handover indicatons if the incoming call is on IMS
            if (qcril_qmi_voice_call_to_ims(call_info_entry))
            {
              qcril_qmi_voice_toggle_ind_reg_on_screen_state(TRUE);
            }
            break; // end of case CALL_STATE_INCOMING_V02

          case CALL_STATE_ALERTING_V02:
            QCRIL_LOG_ESSENTIAL("call state ALERTING for conn id %d", iter_call_info->call_id);
            ril_call_state_valid = TRUE;
            ril_call_state = RIL_CALL_ALERTING;

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
            if ( call_info_entry )
            {
              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );

              if ( CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type ||
                   CALL_TYPE_EMERGENCY_IP_V02 == call_info_entry->voice_scv_info.call_type )
              {
#ifndef QMI_RIL_UTF
                if (call_info_entry->voice_scv_info.mode == CALL_MODE_LTE_V02 ||
                    call_info_entry->voice_scv_info.mode == CALL_MODE_WLAN_V02)
                {
                  qcril_am_set_emergency_on_lte((call_info_entry->voice_scv_info.mode ==
                                               CALL_MODE_LTE_V02));
                }
#endif
              }


              if ( ( NULL != call_info_entry ) && ( TRUE == is_srvcc_valid ) &&
                   ( NULL != is_srvcc ) && ( TRUE == is_srvcc->is_srvcc_call ) )
              {
                /* Call domain changes are not expected in normal scenario and even
                   if it changes we have to handle them only in conversation state.
                   This is due to the limitation in telephony that CS->PS transition
                   is not handled. So we need evaluate CS/PS call elaboration only in
                   case of SRVCC where call domain is changed intentionally to CS*/
                qcril_qmi_voice_set_ps_cs_call_elab_vcl(iter_call_info, call_info_entry);
              }
              qcril_qmi_voice_set_audio_call_type(iter_call_info, call_info_entry);

              if ( ( NULL != call_info_entry ) &&
                   !( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN ) &&
                   ( QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_ALERTING_OR_CONVERSATION_CALL_PENDING == qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl() ) &&
                   ( (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL) || (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL) )
                 )
              {
                 //ims mpty conf call
                 if(ims_conf_status == QCRIL_QMI_IMS_VOICE_CONF_NO_CONF)
                    ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
                 else
                    ims_conf_status &= QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
                 qcril_qmi_voice_set_ims_conf_call_req_txn_state_to_next_vcl();
                 if (iter_call_info->is_mpty)
                 {
                    call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL;
                    call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL;
                 }
              }
            }

            /* Ringback tone handling */
            if( alerting_type_valid )
            {
              if(alerting_type->alerting_type == ALERTING_REMOTE_V02)
              {

                QCRIL_LOG_INFO("Ringback Tone started with QMI id %d", iter_call_id);
                local_ringback_payload = TRUE;
                qcril_qmi_voice_info.last_call_is_local_ringback = TRUE;
                qcril_qmi_voice_info.last_local_ringback_call_id = iter_call_id;

                if (!qcril_qmi_voice_call_to_ims(call_info_entry))
                {
                  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RINGBACK_TONE, &unsol_resp );
                  unsol_resp.resp_pkt = ( void * ) &local_ringback_payload;
                  unsol_resp.resp_len = sizeof( local_ringback_payload );
                  qcril_send_unsol_response( &unsol_resp );
                }
                else
                {
                  qcril_qmi_voice_ims_send_unsol_ringback_tone(local_ringback_payload);
                }
              }
              else if(qcril_qmi_voice_info.last_call_is_local_ringback &&
                       qcril_qmi_voice_info.last_local_ringback_call_id == iter_call_id &&
                      (alerting_type->alerting_type == ALERTING_LOCAL_V02))
              {
                QCRIL_LOG_INFO("Ringback Tone stopped with QMI id %d", iter_call_id);
                local_ringback_payload = FALSE;
                qcril_qmi_voice_info.last_call_is_local_ringback = FALSE;
                qcril_qmi_voice_info.last_local_ringback_call_id = VOICE_INVALID_CALL_ID;

                if (!qcril_qmi_voice_call_to_ims(call_info_entry))
                {
                  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RINGBACK_TONE, &unsol_resp );
                  unsol_resp.resp_pkt = ( void * ) &local_ringback_payload;
                  unsol_resp.resp_len = sizeof( local_ringback_payload );
                  qcril_send_unsol_response( &unsol_resp );
                }
                else
                {
                  qcril_qmi_voice_ims_send_unsol_ringback_tone(local_ringback_payload);
                }
              }
            }

            qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl(iter_call_info);

            break; // end of case CALL_STATE_ALERTING_V02

          case CALL_STATE_CONVERSATION_V02:
            QCRIL_LOG_ESSENTIAL("call state CONVERSATION for conn id %d", iter_call_info->call_id);
            ril_call_state_valid = TRUE;
            ril_call_state = RIL_CALL_ACTIVE;

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
            if ( ( NULL == call_info_entry ) && ( TRUE == srvcc_parent_call_info_valid ) && ( NULL != srvcc_parent_call_info ) )
            {
              // This is the case of conference call SRVCC
              // Create call_info_entry with new call_id.
              call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                      srvcc_parent_call_info->call_id,
                      INVALID_MEDIA_ID,
                      TRUE,
                      QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID );

              if( (NULL != call_info_entry) && (srvcc_parent_call_info->is_parent_id_cleared == TRUE) )
              {
                //Delete the call_info_entry with parent call_id
                parent_call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( srvcc_parent_call_info->parent_call_id );
                if( parent_call_info_entry )
                {
                  memset( &parentCallID, 0, sizeof( parentCallID ) );
                  memset( &callID, 0, sizeof( callID ) );
                  mpty_call_info = parent_call_info_entry;
                  if( NULL != mpty_call_info->mpty_voip_call_list )
                  {
                    mpty_call_info = mpty_call_info->mpty_voip_call_list;
                    snprintf(parentCallID, sizeof(parentCallID), QCRIL_PARENT_CALL_ID_STR"%d", mpty_call_info->android_call_id );

                    mpty_call_info = mpty_call_info->mpty_voip_call_list;
                    while ( mpty_call_info )
                    {
                      snprintf(callID, 5, ",%d", mpty_call_info->android_call_id);
                      strlcat( parentCallID, callID, sizeof(parentCallID) );
                      mpty_call_info = mpty_call_info->mpty_voip_call_list;
                    }
                  }
                  else
                  {
                    snprintf(parentCallID, sizeof(parentCallID), QCRIL_PARENT_CALL_ID_STR"%d", mpty_call_info->android_call_id );
                  }
                  strlcpy(call_info_entry->parent_call_id, parentCallID, strlen(parentCallID)+1);

                  parent_call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_AFTER_SRVCC;
                }
                else
                {
                  QCRIL_LOG_INFO( "Call info with parent_call_id doesn't exist... " );
                }
              }
            }
            else if ( ( NULL == call_info_entry ) && ( TRUE == is_srvcc_valid ) && ( NULL != is_srvcc ) && ( TRUE == is_srvcc->is_srvcc_call ) )
            { // fresh creation of call_info_entry in case of SRVCC call from 3rd party
                call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                        is_srvcc->call_id,
                        INVALID_MEDIA_ID,
                        TRUE,
                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID );
            }

            if ( call_info_entry )
            {
              if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING )
              {
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING;

                if ( QMI_RIL_ZERO != qmi_voice_voip_overview.num_1x_wait_timer_id )
                {
                  qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.num_1x_wait_timer_id );
                  qmi_voice_voip_overview.num_1x_wait_timer_id = QMI_RIL_ZERO;
                }
              }

              switch ( call_info_entry->voice_scv_info.call_state ) // check previous state
              {
                case CALL_STATE_ORIGINATING_V02:
                case CALL_STATE_CONVERSATION_V02:
                case CALL_STATE_CC_IN_PROGRESS_V02:
                case CALL_STATE_ALERTING_V02:
                  // no action
                  break;

                default:
                  qmi_ril_voice_drop_homeless_incall_reqs();
                  break;
              }

              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );
              call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_GOT_CONNECTED;

              // set the domain flag before ELA is reset by call_type
              is_request_on_autodomain = (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN)? TRUE:FALSE;
              qcril_qmi_voice_set_ps_cs_call_elab_vcl(iter_call_info, call_info_entry);
              // set the calltype flag after ELA is reset by call_type
              is_call_on_cs_domain = (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN)? TRUE:FALSE;
              is_silent_redialed = is_request_on_autodomain && is_call_on_cs_domain;
              QCRIL_LOG_INFO("is_request_on_autodomain:%d is_call_on_cs_domain:%d is_silent_redialed:%d", is_request_on_autodomain, is_call_on_cs_domain, is_silent_redialed);

              if ( CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type ||
                   CALL_TYPE_EMERGENCY_IP_V02 == call_info_entry->voice_scv_info.call_type )
              {
#ifndef QMI_RIL_UTF
                qcril_am_set_emergency_on_lte((call_info_entry->voice_scv_info.mode ==
                                               CALL_MODE_LTE_V02));
#endif
              }

              qcril_qmi_voice_set_audio_call_type(iter_call_info, call_info_entry);

              if ( ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN ) &&
                     (QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_CONVERSATION_CALL_PENDING == qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl() ||
                     QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_ALERTING_OR_CONVERSATION_CALL_PENDING == qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl() ||
                     QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_MERGING_CALLS == qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl() )  &&
                   ( (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL) ||
                     (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL) ) )
              {
                 if (iter_call_info->is_mpty)
                 {
                    //ims mpty conf call
                    if(ims_conf_status == QCRIL_QMI_IMS_VOICE_CONF_NO_CONF)
                       ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_ESTABLISHED; //conference is established
                    else
                       ims_conf_status &= QCRIL_QMI_IMS_VOICE_CONF_ESTABLISHED; //conference is established

                    qcril_qmi_voice_set_ims_conf_req_txn_state_vcl(QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_MERGING_CALLS);
                    call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL;
                    call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL;
                 }
                 else
                 {
                   QCRIL_LOG_ERROR("iter_call_info->is_mpty is not set");
                 }

              }

              qcril_qmi_voice_check_and_update_srvcc_no_mid_call_support(call_info_entry, iter_call_info);
            }

            /* Ringback tone handling */
            if( alerting_type_valid )
            {
              if(qcril_qmi_voice_info.last_call_is_local_ringback &&
                 qcril_qmi_voice_info.last_local_ringback_call_id == iter_call_id &&
                 (alerting_type->alerting_type == ALERTING_LOCAL_V02) )
              {
                QCRIL_LOG_INFO("Ringback Tone stopped with QMI call id %d", iter_call_id);
                local_ringback_payload = FALSE;
                qcril_qmi_voice_info.last_call_is_local_ringback = FALSE;
                qcril_qmi_voice_info.last_local_ringback_call_id = VOICE_INVALID_CALL_ID;

                if (!qcril_qmi_voice_call_to_ims(call_info_entry))
                {
                  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RINGBACK_TONE, &unsol_resp );
                  unsol_resp.resp_pkt = ( void * ) &local_ringback_payload;
                  unsol_resp.resp_len = sizeof( local_ringback_payload );
                  qcril_send_unsol_response( &unsol_resp );
                }
                else
                {
                  qcril_qmi_voice_ims_send_unsol_ringback_tone(local_ringback_payload);
                }
              }
            }

            qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl(iter_call_info);

            // Reset the PENDING_INCOMING and ANSWERING_CALL elabs
            call_info_entry->elaboration &= ~(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING |
                                              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ANSWERING_CALL);

            break; // end of case CALL_STATE_CONVERSATION_V02

          case CALL_STATE_END_V02:
            QCRIL_LOG_ESSENTIAL("call state END for conn id %d", iter_call_info->call_id);

            post_call_cleanup_may_be_necessary = TRUE;

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );

            // Cancel auto answer timed callback if previous state is INCOMING
            if( call_info_entry )
            {
              if ( ( CALL_STATE_INCOMING_V02 == call_info_entry->voice_scv_info.call_state ) &&
                   ( CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type ) )
              {
                if ( QMI_RIL_ZERO != qmi_voice_voip_overview.auto_answer_timer_id )
                {
                  QCRIL_LOG_INFO("Cancel Auto Answer timed callback");
                  qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.auto_answer_timer_id );
                  qmi_voice_voip_overview.auto_answer_timer_id = QMI_RIL_ZERO;
                }
              }
            }

            // Ringback tone handling
            if(alerting_type_valid)
            {
              if(qcril_qmi_voice_info.last_call_is_local_ringback &&
                 qcril_qmi_voice_info.last_local_ringback_call_id == iter_call_id &&
                 (alerting_type->alerting_type == ALERTING_LOCAL_V02) )
              {
                QCRIL_LOG_INFO("Ringback Tone stopped with QMI call id %d", iter_call_id);
                local_ringback_payload = FALSE;
                qcril_qmi_voice_info.last_call_is_local_ringback = FALSE;
                qcril_qmi_voice_info.last_local_ringback_call_id = VOICE_INVALID_CALL_ID;
                if (!qcril_qmi_voice_call_to_ims(call_info_entry))
                {
                  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RINGBACK_TONE, &unsol_resp );
                  unsol_resp.resp_pkt = ( void * ) &local_ringback_payload;
                  unsol_resp.resp_len = sizeof( local_ringback_payload );
                  qcril_send_unsol_response( &unsol_resp );
                }
                else
                {
                  qcril_qmi_voice_ims_send_unsol_ringback_tone(local_ringback_payload);
                }
              }
            }

            if ( call_info_entry )
            {
              qcril_qmi_voice_voip_change_call_elab_when_first_call_ind_received(call_info_entry);
              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );

              QCRIL_LOG_INFO( "call mode %d, call type %d, call got connected %d",
                              (int)iter_call_info->mode,
                              (int)iter_call_info->call_type,
                              (int) (QMI_RIL_ZERO != (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_GOT_CONNECTED))  );

              if (CALL_TYPE_EMERGENCY_V02 == iter_call_info->call_type ||
                     CALL_TYPE_EMERGENCY_IP_V02 == iter_call_info->call_type)
              {
                 if(call_info_entry->elaboration &
                    QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_GOT_CONNECTED)
                 {
                   int is_eme_ip = (CALL_TYPE_EMERGENCY_IP_V02 == iter_call_info->call_type);
                   qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                                      QCRIL_DEFAULT_MODEM_ID,
                                      QCRIL_DATA_ON_STACK,
                                      QCRIL_EVT_QMI_RIL_CONNECTED_EMEGENCY_CALL_END,
                                      &is_eme_ip,
                                      sizeof(is_eme_ip),
                                      (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
                 }
#ifndef QMI_RIL_UTF
                 qcril_am_reset_emergency_on_lte();
#endif
              }

             if ( !(call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_MEMBER_OF_MPTY_VOIP_CALL)
                  && !qcril_qmi_voice_call_to_ims(call_info_entry))
             {
                (void)qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                                            QCRIL_DEFAULT_MODEM_ID,
                                            qmi_ril_voice_ended_call_obj_phase_out,
                                            &call_obj_phaseout_auto_delay,
                                            &call_info_entry->call_obj_phase_out_timer_id
                                           );
              }
            }

            // Last call failure cause
            if( call_status_ind_ptr->call_end_reason_valid )
            {
              QCRIL_LOG_INFO("call failure cause %d", call_status_ind_ptr->call_end_reason[0].call_end_reason);
              qcril_qmi_voice_handle_new_last_call_failure_cause(call_status_ind_ptr->call_end_reason[0].call_end_reason, TRUE, call_info_entry);
            }

            qcril_qmi_voice_respond_pending_hangup_ril_response(iter_call_info->call_id);

            if ( NULL != call_info_entry )
            { // qmi call id will no longer be valid for this call object
              call_info_entry->qmi_call_id = VOICE_INVALID_CALL_ID;
            }

            if ( (NULL != call_info_entry) &&
                 call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN &&
                 QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_MERGING_CALLS == qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl() &&
                 call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL
               )
            {
                //This call got added to an ims mpty conference.
                if(ims_conf_status == QCRIL_QMI_IMS_VOICE_CONF_NO_CONF)
                    ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_ESTABLISHED; //conference is established
               else
                  ims_conf_status &= QCRIL_QMI_IMS_VOICE_CONF_ESTABLISHED; //conference is established
               //qcril_qmi_voice_add_call_to_existing_mpty_voip_call_vcl(call_info_entry);
            }
            qcril_qmi_voice_reset_cdma_call_id_if_applicable_vcl(iter_call_info);
            if (qcril_qmi_ril_domestic_service_is_screen_off())
            {
              qcril_qmi_voice_toggle_ind_reg_on_screen_state(FALSE);
            }
            break; // end of case CALL_STATE_END_V02

          case CALL_STATE_ORIGINATING_V02:
            QCRIL_LOG_ESSENTIAL("call state ORIGINATING for conn id %d", iter_call_info->call_id);
            ril_call_state_valid = TRUE;
            ril_call_state = RIL_CALL_DIALING;

            // update call info entry
            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );

            if ( NULL == call_info_entry )
            { // ghost call
              call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                      iter_call_info->call_id,
                      iter_media_id,
                      TRUE,
                      QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NONE );
              if ( call_info_entry )
              {
                call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PHANTOM_CALL;
                qcril_qmi_voice_set_domain_elab_from_call_type(iter_call_info->call_type, call_info_entry);

                if ( !( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN ) &&
                     ( QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_ORIGINATING_CALL_PENDING == qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl() ) )
                {
                    //ims conference call
                    if(ims_conf_status == QCRIL_QMI_IMS_VOICE_CONF_NO_CONF)
                    {
                       ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
                    }
                    else
                    {
                       ims_conf_status &= QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
                    }

                    qcril_qmi_voice_set_ims_conf_call_req_txn_state_to_next_vcl();
                    if (iter_call_info->is_mpty)
                    {
                       call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL;
                    }
                    else
                    {
                       call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL;
                    }
                }
              }
            }
            else
            {
               qcril_qmi_voice_voip_change_call_elab_when_first_call_ind_received(call_info_entry);
            }

            if ( call_info_entry )
            {
              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );
              qcril_qmi_voice_set_audio_call_type(iter_call_info, call_info_entry);

              qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( call_info_entry );

              // related eme from non oos or 1xcsfb
              if ( CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type ||
                   CALL_TYPE_EMERGENCY_IP_V02 == call_info_entry->voice_scv_info.call_type )
              {
                call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL;
                if ( !( call_info_entry->elaboration & ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING )) )
                { // related emergency call from non oos
                  call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING;
                }

#ifndef QMI_RIL_UTF
                qcril_am_set_emergency_on_lte((call_info_entry->voice_scv_info.mode ==
                                               CALL_MODE_LTE_V02));
#endif
              }
              else
              { // non emergency
                if ( (QMI_RIL_RTE_SUB_LTE == current_voice_rte || RADIO_TECH_LTE == reported_voice_radio_tech) &&
                     (CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type) )
                {
                  call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL;
                  if ( !( call_info_entry->elaboration & ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING )) )
                  { // 1x CSFB call
                    call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING;
                  }
                }
                else if ( (CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type ||
                           CALL_TYPE_STD_OTASP_V02 == call_info_entry->voice_scv_info.call_type ||
                           CALL_TYPE_NON_STD_OTASP_V02 == call_info_entry->voice_scv_info.call_type)
                          &&
                          (CALL_MODE_NO_SRV_V02 == call_info_entry->voice_scv_info.mode)
                        )
                { // call type is voice or OTASP, but call mode is not yet known -> we may not identify if calls' RAT matches to current voice RAT, and that will hide call from UI
                  // mark call as "DIAL_FROM_OOS" to reveal the call info to UI
                  call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAL_FROM_OOS;
                  if ( !( call_info_entry->elaboration & ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING )) ) // also set extended dialing flag, this enabling RIL reports DIALING state
                                                                                                                   // at least once, that way ensuring call info is rendered if we change RAT
                                                                                                                   // during call setup
                  { // call originated from unknown RAT
                    call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING;
                  }
                }
              }
            }

            if ( qcril_qmi_voice_is_stk_cc_in_progress() && stk_cc_info.call_id_info == iter_call_info->call_id )
            {  // stk cc for this call
              qcril_qmi_voice_stk_cc_relay_alpha_if_necessary(instance_id, FALSE);
              qcril_qmi_voice_reset_stk_cc();
            }

            qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl(iter_call_info);

            break; // end of case CALL_STATE_ORIGINATING_V02

          case CALL_STATE_DISCONNECTING_V02:
            QCRIL_LOG_ESSENTIAL("call state DISCONNECTING for conn id %d", iter_call_info->call_id);

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
            if ( call_info_entry )
            {
              qcril_qmi_voice_voip_change_call_elab_when_first_call_ind_received(call_info_entry);
              switch ( call_info_entry->voice_scv_info.call_state ) // check previous state
              {
                case CALL_STATE_ORIGINATING_V02:
                case CALL_STATE_CONVERSATION_V02:
                case CALL_STATE_CC_IN_PROGRESS_V02:
                case CALL_STATE_ALERTING_V02:
                  qmi_ril_voice_drop_homeless_incall_reqs();
                  break;

                case CALL_STATE_INCOMING_V02:
                  if ( ( CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type ) &&
                       ( QMI_RIL_ZERO != qmi_voice_voip_overview.auto_answer_timer_id ) )
                  {
                    QCRIL_LOG_INFO("Cancel Auto Answer timed callback");
                    qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.auto_answer_timer_id );
                    qmi_voice_voip_overview.auto_answer_timer_id = QMI_RIL_ZERO;
                  }
                  break;

                default:
                  // no action
                  break;
              }

              if ( CALL_STATE_SETUP_V02 == call_info_entry->voice_scv_info.call_state &&
                   call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID )
              {
                qcril_cancel_timed_callback((void *)(uintptr_t) call_info_entry->ringing_time_id);
                call_info_entry->ringing_time_id = QMI_RIL_ZERO;
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING;
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID;
              }

              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );
            }
            qcril_qmi_voice_invalid_last_call_failure_cause();

            /* Ringback tone handling */
            if( alerting_type_valid )
            {
              if(qcril_qmi_voice_info.last_call_is_local_ringback &&
                 (alerting_type->alerting_type == ALERTING_LOCAL_V02) &&
                 qcril_qmi_voice_info.last_local_ringback_call_id == iter_call_id)
              {
                QCRIL_LOG_INFO("Stop Ringback Tone for QMI id %d", iter_call_id);
                local_ringback_payload = FALSE;
                qcril_qmi_voice_info.last_call_is_local_ringback = FALSE;
                qcril_qmi_voice_info.last_local_ringback_call_id = VOICE_INVALID_CALL_ID;
                if (!qcril_qmi_voice_call_to_ims(call_info_entry))
                {

                  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RINGBACK_TONE, &unsol_resp );
                  unsol_resp.resp_pkt = ( void * ) &local_ringback_payload;
                  unsol_resp.resp_len = sizeof( local_ringback_payload );
                  qcril_send_unsol_response( &unsol_resp );
                }
                else
                {
                  qcril_qmi_voice_ims_send_unsol_ringback_tone(local_ringback_payload);
                }
              }
            }
            //TODO: check the race condition in real time and uncomment the commented code if needed

            /*Race condition(example, could be many other): window betwee {HOLD, END, CONVERSATION}
             *  and {END, Conversation} if we get a disconnecting status for the second call like
             * {DISCONNECTING, CONVERSATION} we shouldnt report the indication back to atel.
             * This should be considered as conference in progress.
             * Uncomment the code below if such race condition exisits in real time.*/

            //if(NULL != call_info_entry &&
               //call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN &&
               //call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL )
            //{
              // /* This call id will be ended and
                  //the call will be added to an ims conference*/
               //if(ims_conf_status == QCRIL_QMI_IMS_VOICE_CONF_NO_CONF)
                  //ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
               //else
                  //ims_conf_status &= QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
            //}

            break; // end of case CALL_STATE_DISCONNECTING_V02

          case CALL_STATE_WAITING_V02:
            QCRIL_LOG_ESSENTIAL("call state WAITING for conn id %d", iter_call_info->call_id);
            ril_call_state_valid = TRUE;
            ril_call_state = RIL_CALL_WAITING;

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
            if ( NULL == call_info_entry )
            { // fresh
              call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                      iter_call_info->call_id,
                      iter_media_id,
                      TRUE,
                      QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING );
            }
            else
            {
              call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING;
              qcril_qmi_voice_voip_call_info_dump( call_info_entry );
            }
            if ( call_info_entry )
            {
              if ( CALL_STATE_SETUP_V02 == call_info_entry->voice_scv_info.call_state &&
                   call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID )
              {
                qcril_cancel_timed_callback((void *)(uintptr_t) call_info_entry->ringing_time_id);
                call_info_entry->ringing_time_id = QMI_RIL_ZERO;
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_RINING_TIME_ID_VALID;
                call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING;
              }

              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );
              qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( call_info_entry );

              qcril_qmi_voice_set_ps_cs_call_elab_vcl(iter_call_info, call_info_entry);
              qcril_qmi_voice_set_audio_call_type(iter_call_info, call_info_entry);
            }

            qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                               QCRIL_DEFAULT_MODEM_ID,
                               QCRIL_DATA_ON_STACK,
                               QCRIL_EVT_QMI_VOICE_GET_WAITING_CALL,
                               &(iter_call_info->call_id),
                               sizeof(iter_call_info->call_id),
                               (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

            qcril_qmi_voice_set_cdma_call_id_if_applicable_vcl(iter_call_info);

            break; // end of case CALL_STATE_WAITING_V02

          case CALL_STATE_HOLD_V02:
            QCRIL_LOG_ESSENTIAL("call state HOLD for conn id %d", iter_call_info->call_id);
            ril_call_state_valid = TRUE;
            ril_call_state = RIL_CALL_HOLDING;

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
            if ( ( NULL == call_info_entry ) && ( TRUE == srvcc_parent_call_info_valid ) && ( NULL != srvcc_parent_call_info ) )
            {
              // This is the case of conference call SRVCC
              // Create call_info_entry with new call_id.
              call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                      srvcc_parent_call_info->call_id,
                      INVALID_MEDIA_ID,
                      TRUE,
                      QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID );

              if( (NULL != call_info_entry) && (srvcc_parent_call_info->is_parent_id_cleared == TRUE) )
              {
                //Delete the call_info_entry with parent call_id
                parent_call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( srvcc_parent_call_info->parent_call_id );
                if( parent_call_info_entry )
                {
                  memset( &parentCallID, 0, sizeof( parentCallID ) );
                  memset( &callID, 0, sizeof( callID ) );
                  mpty_call_info = parent_call_info_entry;
                  if( NULL != mpty_call_info->mpty_voip_call_list )
                  {
                    mpty_call_info = mpty_call_info->mpty_voip_call_list;
                    snprintf(parentCallID, sizeof(parentCallID), QCRIL_PARENT_CALL_ID_STR"%d", mpty_call_info->android_call_id );

                    mpty_call_info = mpty_call_info->mpty_voip_call_list;
                    while ( mpty_call_info )
                    {
                      snprintf(callID, 5, ",%d", mpty_call_info->android_call_id);
                      strlcat( parentCallID, callID, sizeof(parentCallID) );
                      mpty_call_info = mpty_call_info->mpty_voip_call_list;
                    }
                  }
                  else
                  {
                    snprintf(parentCallID, sizeof(parentCallID), QCRIL_PARENT_CALL_ID_STR"%d", mpty_call_info->android_call_id );
                  }
                  strlcpy(call_info_entry->parent_call_id, parentCallID, strlen(parentCallID)+1);

                  parent_call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_AFTER_SRVCC;
                }
                else
                {
                  QCRIL_LOG_INFO( "Call info with parent_call_id doesn't exist... " );
                }
              }
            }

            if ( call_info_entry )
            {
              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );
              qcril_qmi_voice_set_ps_cs_call_elab_vcl(iter_call_info, call_info_entry);
              qcril_qmi_voice_set_audio_call_type(iter_call_info, call_info_entry);

              qcril_qmi_voice_check_and_update_srvcc_no_mid_call_support(call_info_entry, iter_call_info);
            }

            if(NULL != call_info_entry &&
               call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN &&
               call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL )
            {
              /* This call id will be ended and
                  the call will be added to an ims conference*/
               if(ims_conf_status == QCRIL_QMI_IMS_VOICE_CONF_NO_CONF)
                  ims_conf_status = QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
               else
                  ims_conf_status &= QCRIL_QMI_IMS_VOICE_CONF_IN_PROGRESS; //conference establishment is in progress
            }

            break; // end of case CALL_STATE_HOLD_V02

          case CALL_STATE_CC_IN_PROGRESS_V02:
            QCRIL_LOG_ESSENTIAL("call state CC IN PROGRESS for conn id %d", iter_call_info->call_id);

            call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
                                                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP,
                                                TRUE );

            if (call_info_entry)
            {
                call_info_entry->qmi_call_id = iter_call_info->call_id;
                call_info_entry->media_id = iter_media_id;

                QCRIL_LOG_INFO("Call qmi id is %d, Call media id is %d",
                               call_info_entry->qmi_call_id,
                               call_info_entry->media_id);

                if (QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_HANGUP_AFTER_VALID_QMI_ID &
                                                     call_info_entry->elaboration)
                {
                    call_info_entry->elaboration &=
                                     (~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_HANGUP_AFTER_VALID_QMI_ID);
                    hangup_call = TRUE;
                    QCRIL_LOG_INFO("Call needs to be hung up");
                }
            }
            else
            {
                call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id(
                                                            iter_call_info->call_id);
            }

            if (call_info_entry)
            {
              qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                  call_info_entry,
                                                  iter_call_info,
                                                  remote_party_number_valid,
                                                  remote_party_number,
                                                  remote_party_name_valid,
                                                  remote_party_name,
                                                  alerting_type_valid,
                                                  alerting_type,
                                                  srv_opt_valid,
                                                  srv_opt,
                                                  call_end_reason_valid,
                                                  call_end_reason,
                                                  alpha_id_valid,
                                                  alpha_id,
                                                  conn_party_num_valid,
                                                  conn_party_num,
                                                  diagnostic_info_valid,
                                                  diagnostic_info,
                                                  called_party_num_valid,
                                                  called_party_num,
                                                  redirecting_party_num_valid,
                                                  redirecting_party_num,
                                                  ril_call_state_valid,
                                                  ril_call_state,
                                                  audio_attrib_valid,
                                                  audio_attrib,
                                                  video_attrib_valid,
                                                  video_attrib,
                                                  call_attrib_status_valid,
                                                  call_attrib_status,
                                                  is_srvcc_valid,
                                                  is_srvcc,
                                                  srvcc_parent_call_info_valid,
                                                  srvcc_parent_call_info,
                                                  local_call_capabilities_info_valid,
                                                  local_call_capabilities_info,
                                                  peer_call_capabilities_info_valid,
                                                  peer_call_capabilities_info,
                                                  child_number_valid,
                                                  child_number,
                                                  display_text_valid,
                                                  display_text,
                                                  ip_num_info_valid,
                                                  ip_num_info,
                                                  conn_ip_num_info_valid,
                                                  conn_ip_num_info,
                                                  is_add_info_present_valid,
                                                  is_add_info_present,
                                                  ip_caller_name_valid,
                                                  ip_caller_name,
                                                  end_reason_text_valid,
                                                  end_reason_text
                                                  );
            }

            break; // end of case CALL_STATE_CC_IN_PROGRESS_V02

          case CALL_STATE_SETUP_V02:
            QCRIL_LOG_ESSENTIAL("call state SETUP for conn id %d", iter_call_info->call_id);
            if ( !qmi_ril_voice_is_voice_calls_supressed )
            {

              call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( iter_call_info->call_id );
              if ( NULL == call_info_entry )
              { // fresh
                call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                        iter_call_info->call_id,
                        iter_media_id,
                        TRUE,
                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NONE );
              }
              else
              { // what we got?
                qcril_qmi_voice_voip_call_info_dump( call_info_entry );
              }
              if ( call_info_entry )
              {
                qcril_qmi_voice_set_domain_elab_from_call_type(iter_call_info->call_type, call_info_entry);

                qcril_qmi_voice_voip_update_call_info_entry_mainstream(
                                                    call_info_entry,
                                                    iter_call_info,
                                                    remote_party_number_valid,
                                                    remote_party_number,
                                                    remote_party_name_valid,
                                                    remote_party_name,
                                                    alerting_type_valid,
                                                    alerting_type,
                                                    srv_opt_valid,
                                                    srv_opt,
                                                    call_end_reason_valid,
                                                    call_end_reason,
                                                    alpha_id_valid,
                                                    alpha_id,
                                                    conn_party_num_valid,
                                                    conn_party_num,
                                                    diagnostic_info_valid,
                                                    diagnostic_info,
                                                    called_party_num_valid,
                                                    called_party_num,
                                                    redirecting_party_num_valid,
                                                    redirecting_party_num,
                                                    ril_call_state_valid,
                                                    ril_call_state,
                                                    audio_attrib_valid,
                                                    audio_attrib,
                                                    video_attrib_valid,
                                                    video_attrib,
                                                    call_attrib_status_valid,
                                                    call_attrib_status,
                                                    is_srvcc_valid,
                                                    is_srvcc,
                                                    srvcc_parent_call_info_valid,
                                                    srvcc_parent_call_info,
                                                    local_call_capabilities_info_valid,
                                                    local_call_capabilities_info,
                                                    peer_call_capabilities_info_valid,
                                                    peer_call_capabilities_info,
                                                    child_number_valid,
                                                    child_number,
                                                    display_text_valid,
                                                    display_text,
                                                    ip_num_info_valid,
                                                    ip_num_info,
                                                    conn_ip_num_info_valid,
                                                    conn_ip_num_info,
                                                    is_add_info_present_valid,
                                                    is_add_info_present,
                                                    ip_caller_name_valid,
                                                    ip_caller_name,
                                                    end_reason_text_valid,
                                                    end_reason_text
                                                    );

                if ( !( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING ) )
                { // got to initiate ringing
                  call_info_entry->elaboration |= ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING | QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_RING_DONE );
                  QCRIL_LOG_INFO( "launch ringer" );
                  qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                        QCRIL_DEFAULT_MODEM_ID,
                                        qcril_qmi_voice_make_incoming_call_ring,
                                        NULL,   // no param
                                        NULL,   // immediate
                                        NULL );
                }
              }
            }
            break; // end of case CALL_STATE_SETUP_V02

          default:
            QCRIL_LOG_ESSENTIAL("unexpected call state(%d)  for conn id %d", iter_call_info->call_state,
                                                                            iter_call_info->call_id);
            call_info_entry = NULL;
            break;
        } // switch ( iter_call_info->call_state )

        // eme oos - extended dialing
        if ( NULL != call_info_entry )
        {
          switch ( iter_call_info->call_state )
          {
            case CALL_STATE_CONVERSATION_V02: // fallthrough
            case CALL_STATE_DISCONNECTING_V02:
            case CALL_STATE_ALERTING_V02:
            case CALL_STATE_END_V02:
              if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING &&
                   !( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING_ENDING ) )
              {
                call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING_ENDING;
                call_info_entry->elaboration &= ~( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING_ENDING | QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING );
              }
              break;

            default:
              // nothing
              break;
          }
        }
        //Evaluating calls that need to be skipped for now
        if (iter_call_info->call_state == CALL_STATE_CC_IN_PROGRESS_V02 ||
                (iter_call_info->call_type == CALL_TYPE_VOICE_IP_V02 ||
                 iter_call_info->call_type == CALL_TYPE_VT_V02 ||
                 iter_call_info->call_type == CALL_TYPE_EMERGENCY_IP_V02))
        {
           is_deviant_call = FALSE;
        }
        else
        {
           is_deviant_call = qcril_qmi_voice_nas_control_is_call_mode_reported_voice_radio_tech_different( iter_call_info->mode );
        }
        QCRIL_LOG_INFO(".. is deviant call pre %d ", (int) is_deviant_call );
        if ( is_deviant_call )
        {
          switch ( iter_call_info->call_state )
          {
            case CALL_STATE_ORIGINATING_V02:
            case CALL_STATE_INCOMING_V02:
            case CALL_STATE_CC_IN_PROGRESS_V02:
            case CALL_STATE_SETUP_V02:
              // keep is_deviant_call as is
              break;

            default: // do not allow hiding call for post connecetd call states
              is_deviant_call = FALSE;
              break;
          }
        }
        is_mode_resonable = qmi_ril_nw_reg_voice_is_voice_call_mode_reasonable_against_dev_cfg( iter_call_info->mode );
        QCRIL_LOG_INFO(".. is deviant call final %d, is mode reasonable %d", (int) is_deviant_call, (int)is_mode_resonable );


        if ( call_info_entry )
        {
          if ( call_info_entry->elaboration &
               (QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS |
                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL |
                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL |
                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAL_FROM_OOS) )
          {
            imperative_report = TRUE;
          }
          else
          {
            imperative_report = FALSE;
          }
        }
        else
        {
          imperative_report = FALSE;
        }
        QCRIL_LOG_INFO(".. imperative report %d ", (int) imperative_report );

        if ( !( CALL_STATE_SETUP_V02 == iter_call_info->call_state ||
                CALL_STATE_DISCONNECTING_V02 == iter_call_info->call_state ||
                ( is_deviant_call && !imperative_report && is_mode_resonable )
              )
           )
        {  // need to report the given call at this time
           if ( qcril_qmi_voice_call_to_ims(call_info_entry) )
           {
             nof_ims_calls++;
           }

           if ( qcril_qmi_voice_call_to_atel(call_info_entry) )
           {
             nof_atel_calls++;
           }
        }

        if ( call_info_entry )
        {
          if ( is_deviant_call && is_mode_resonable && !imperative_report )
          {
            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_WAITING_FOR_MATCHING_VOICE_RTE;
            QCRIL_LOG_INFO("skipping call with android call id %d / qmi call id %d as call mode does not match with the current voice radio tech",
                           call_info_entry->android_call_id,
                           call_info_entry->qmi_call_id );
          }
          else
          {
            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_WAITING_FOR_MATCHING_VOICE_RTE )
            { // "shadowing" no longer needed
              call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_WAITING_FOR_MATCHING_VOICE_RTE;
            }
          }
        }

        // oversight update -- start
        memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
        oversight_event_params.new_call_state      = iter_call_info->call_state;
        oversight_event_params.locator.qmi_call_id = iter_call_info->call_id;
        (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_IND, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID, &oversight_event_params );

        if ( NULL != call_info_entry )
        {
          if ( VOICE_INVALID_CALL_ID != call_info_entry->android_call_id )
          {
            memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
            oversight_event_params.new_call_state             = iter_call_info->call_state;
            oversight_event_params.locator.android_call_id    = call_info_entry->android_call_id;
            (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_IND, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ANDROID_CALL_ID, &oversight_event_params );
          }

          memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
          oversight_event_params.new_call_state               = iter_call_info->call_state;
          oversight_event_params.locator.elaboration_pattern  = call_info_entry->elaboration;
          (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_IND, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN, &oversight_event_params );
        }

        if ( CALL_STATE_END_V02 == iter_call_info->call_state )
        {
          memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
          oversight_event_params.locator.qmi_call_id = iter_call_info->call_id;
          (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_CALL_ENDED, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID, &oversight_event_params );

          if ( NULL != call_info_entry )
          {
            if ( VOICE_INVALID_CALL_ID != call_info_entry->android_call_id )
            {
              memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
              oversight_event_params.locator.android_call_id   = call_info_entry->android_call_id;
              (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_CALL_ENDED, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ANDROID_CALL_ID, &oversight_event_params );
            }

            memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
            oversight_event_params.locator.elaboration_pattern  = call_info_entry->elaboration;
            (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_CALL_ENDED, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN, &oversight_event_params );
          }
        }
        // oversight update -- end

      } // for ( i = 0; i < call_status_ind_ptr->call_info_len; i++ )

      //Destroy the call info objects for the calls ended as part of srvcc
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_AFTER_SRVCC, TRUE );
      if ( NULL != call_info_entry )
      {
        qcril_qmi_voice_voip_destroy_call_info_entry( call_info_entry );
      }

      // dump call objs to log
      calls_nof_remaining = 0;
      calls_iter          = 0;
      QCRIL_LOG_INFO( "-- final call dump start --" );
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
      while ( NULL != call_info_entry )
      {
        calls_nof_remaining++;
        call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
      }
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
      while ( NULL != call_info_entry )
      {
        calls_iter++;
        QCRIL_LOG_INFO( "- dumping call %d out of %d", calls_iter, calls_nof_remaining );
        qcril_qmi_voice_voip_call_info_dump( call_info_entry );
        call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
      }
      QCRIL_LOG_INFO( "-- final call dump end --" );

      if( TRUE == qcril_qmi_voice_is_cdma_voice_emergency_calls_present(&cdma_voice_call_info_entry, NULL) )
      {
        if( NULL != cdma_voice_call_info_entry )
        {
          cdma_voice_call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_REPORT_CACHED_RP_NUMBER;
        }
        QCRIL_LOG_INFO( "flagged cached RP num %p", cdma_voice_call_info_entry );
      }

      // post cleanup
      if ( post_call_cleanup_may_be_necessary && QMI_RIL_ZERO == calls_nof_remaining )
      {
        qmi_ril_succeed_on_pending_hangup_req_on_no_calls_left();
        qmi_ril_voice_cleanup_reqs_after_call_completion();
      }

      qcril_qmi_voice_voip_generate_summary( &calls_summary_end );

      if (qmi_ril_voice_is_calls_supressed_by_pil_vcl())
      {
         qcril_qmi_voice_hangup_all_non_emergency_calls_vcl();
      }

#ifndef QMI_RIL_UTF
      qcril_am_handle_event(QCRIL_AM_EVENT_CALL_STATE_CHANGED, NULL);
#endif

      qcril_qmi_voice_voip_unlock_overview();

      if ( nof_atel_calls && !qmi_ril_voice_is_voice_calls_supressed )
      {
        // send SRVCC ind to RILD if IMS dial request if silent redialed
        if (is_silent_redialed)
        {
          QCRIL_LOG_ERROR("modem silent redialed, sending SRVCC IND");
          // send HANDOVER STARTED and COMPLETED
          qcril_default_unsol_resp_params(instance_id, RIL_UNSOL_SRVCC_STATE_NOTIFY ,&unsol_resp);
          RIL_SrvccState ril_srvccstate;
          unsol_resp.resp_pkt = &ril_srvccstate;
          unsol_resp.resp_len = sizeof(RIL_SrvccState);

          ril_srvccstate = HANDOVER_STARTED;
          qcril_send_unsol_response(&unsol_resp);
          qcril_qmi_voice_send_ims_unsol_resp_handover(ril_srvccstate);

          ril_srvccstate = HANDOVER_COMPLETED;
          qcril_send_unsol_response(&unsol_resp);
          qcril_qmi_voice_send_ims_unsol_resp_handover(ril_srvccstate);
        }
        qcril_qmi_voice_send_unsol_call_state_changed( instance_id );
      }

      if ( nof_ims_calls && !qmi_ril_voice_is_voice_calls_supressed )
      {
        qcril_qmi_voice_send_ims_unsol_call_state_changed();
      }


      if ( calls_summary_end.nof_voice_calls != calls_summary_beginning.nof_voice_calls )
      {
        qcril_qmi_nas_initiate_voice_rte_change_propagation();
      }

      if (hangup_call)
      {
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
                                            QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP,
                                            TRUE );
        if (call_info_entry)
        {
            qcril_qmi_voice_send_hangup_on_call(call_info_entry->android_call_id);
        }
      }
  }

  QCRIL_LOG_ESSENTIAL ( log_essence );

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_all_call_status_ind_hdlr

//===========================================================================
// qcril_qmi_voice_is_cdma_voice_emergency_calls_present
//
//
// Passes back the pointer to the voice call info entry
// returns TRUE if a emergency call has been dialled on top of a cdma voice call
//===========================================================================
int qcril_qmi_voice_is_cdma_voice_emergency_calls_present(qcril_qmi_voice_voip_call_info_entry_type **cdma_voice_call_info_entry,
                                                                 qcril_qmi_voice_voip_call_info_entry_type **cdma_no_srv_emer_call_info_entry)
{
  int ret;
  int nof_calls;
  int nof_1x_voice_calls;
  int nof_1x_no_srv_emergency_calls;
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  ret = FALSE;
  nof_calls = QMI_RIL_ZERO;
  nof_1x_voice_calls = QMI_RIL_ZERO;
  nof_1x_no_srv_emergency_calls = QMI_RIL_ZERO;
  call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();

  while ( NULL != call_info_entry )
  {
    if ( call_info_entry->android_call_id != VOICE_INVALID_CALL_ID )
    {
      nof_calls++;
      if ( ( CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type ||
             ( CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type &&
               (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_REPORT_CACHED_RP_NUMBER) ) ) &&
           CALL_MODE_CDMA_V02 == call_info_entry->voice_scv_info.mode )
      {
        nof_1x_voice_calls++;
        if( cdma_voice_call_info_entry )
        {
          *cdma_voice_call_info_entry = call_info_entry;
        }
      }
      else if ( CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type &&
                ( CALL_MODE_CDMA_V02 == call_info_entry->voice_scv_info.mode ||
                  CALL_MODE_NO_SRV_V02 == call_info_entry->voice_scv_info.mode ) )
      {
        nof_1x_no_srv_emergency_calls++;
        if( cdma_no_srv_emer_call_info_entry )
        {
          *cdma_no_srv_emer_call_info_entry = call_info_entry;
        }
      }
    }
    call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
  }
  QCRIL_LOG_INFO( "nof calls %d, nof cdma voice calls %d, nof 1x/no_srv emergency calls %d", nof_calls, nof_1x_voice_calls, nof_1x_no_srv_emergency_calls );

  if ( 2 == nof_calls &&
       1 == nof_1x_voice_calls &&
       1 == nof_1x_no_srv_emergency_calls )
  {
    ret= TRUE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
} //qcril_qmi_voice_is_cdma_voice_emergency_calls_present

//===========================================================================
// qmi_ril_voice_ended_call_obj_phaseout
//===========================================================================
void qmi_ril_voice_ended_call_obj_phase_out(void * param)
{
    qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;
    uint32                                     timer_id;

    QCRIL_LOG_FUNC_ENTRY();

    timer_id = ( uint32 )(uintptr_t)param;

    qcril_qmi_voice_voip_lock_overview();

    call_info = qcril_qmi_voice_voip_call_info_entries_enum_first();
    while ( NULL != call_info )
    {
      if ( VOICE_INVALID_CALL_ID != call_info->android_call_id &&
           CALL_STATE_END_V02 == call_info->voice_scv_info.call_state &&
           timer_id == call_info->call_obj_phase_out_timer_id &&
          ! ( call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL )
         )
      { // timeout on keeping call obj, delete it now
        qcril_qmi_voice_voip_destroy_call_info_entry( call_info );
      }

      call_info = qcril_qmi_voice_voip_call_info_entries_enum_next();
    }

    qcril_qmi_voice_voip_unlock_overview();

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_ended_call_obj_phaseout

//===========================================================================
// qmi_ril_extended_dialing_over
//===========================================================================
void qmi_ril_voice_extended_dialing_over( void )
{
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  int                                         need_update_atel;
  int                                         need_update_ims;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();
  call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING, TRUE );
  QCRIL_LOG_INFO( ".. call obj %p", call_info_entry );
  if ( NULL != call_info_entry )
  {
    call_info_entry->elaboration &= ~( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING_ENDING | QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING );
    need_update_atel = qcril_qmi_voice_call_to_atel(call_info_entry);
    need_update_ims  = qcril_qmi_voice_call_to_ims (call_info_entry);
  }
  else
  {
    need_update_atel = FALSE;
    need_update_ims  = FALSE;
  }
  qcril_qmi_voice_voip_unlock_overview();

  if ( need_update_atel )
  {
    qcril_qmi_voice_send_unsol_call_state_changed( QCRIL_DEFAULT_INSTANCE_ID );
  }

  if ( need_update_ims )
  {
    qcril_qmi_voice_send_ims_unsol_call_state_changed();
  }

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_extended_dialing_over

//===========================================================================
// qmi_ril_voice_revoke_kill_immunity_priveledge
//===========================================================================
void qmi_ril_voice_revoke_kill_immunity_priveledge( void )
{
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  int                                         any_found;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();
  do
  {
    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_KILL_IMMUNITY, TRUE );
    QCRIL_LOG_INFO( ".. call obj %p", call_info_entry );
    if ( NULL != call_info_entry )
    {
      call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_KILL_IMMUNITY;

      any_found = TRUE;
    }
    else
    {
      any_found = FALSE;
    }
  } while ( call_info_entry != NULL );
  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_revoke_kill_immunity_priveledge

//===========================================================================
// qmi_ril_voice_post_rte_change_propagation_follow_up
//===========================================================================
void qmi_ril_voice_post_rte_change_propagation_follow_up(void)
{
  QCRIL_LOG_FUNC_ENTRY();

  qmi_ril_voice_revoke_kill_immunity_priveledge();
  qmi_ril_voice_extended_dialing_over();

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_post_rte_change_propagation_follow_up

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_modified_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_VOICE_MODIFIED_IND_V02.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_modified_ind_hdlr
(
  void   *data_ptr,
  uint32 data_len
)
{
  qcril_instance_id_e_type        instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type           modem_id = QCRIL_DEFAULT_MODEM_ID;
  voice_modified_ind_msg_v02 *modify_ind_ptr;
  qcril_reqlist_public_type       req_info;
  IxErrnoType                     result;
  qcril_qmi_voice_voip_call_info_entry_type   *call_info = NULL;
  qcril_request_resp_params_type  resp;
  RIL_Call_Modify unsol_modify_data;
  qcril_unsol_resp_params_type    unsol_resp;
  voice_call_attributes_type_v02  audio_attrib;
  voice_call_attributes_type_v02  video_attrib;
  voice_call_attrib_status_type_v02 call_attrib_status;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(data_len);

  if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) || qcril_qmi_voice_info.jbims )
  {
      memset(&unsol_modify_data, 0, sizeof(unsol_modify_data));

      if( data_ptr != NULL )
      {
        modify_ind_ptr = (voice_modified_ind_msg_v02 *)data_ptr;

        call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( modify_ind_ptr->call_id );

        if( call_info != NULL )
        {
           /* udpate the call type */
           if( modify_ind_ptr->call_type_valid )
           {
              call_info->voice_scv_info.call_type = modify_ind_ptr->call_type;
           }

           if( modify_ind_ptr->audio_attrib_valid )
           {
              audio_attrib.call_attributes = modify_ind_ptr->audio_attrib;
              audio_attrib.call_id = modify_ind_ptr->call_id;
           }

           if( modify_ind_ptr->video_attrib_valid )
           {
              video_attrib.call_attributes = modify_ind_ptr->video_attrib;
              video_attrib.call_id = modify_ind_ptr->call_id;
           }

           if( modify_ind_ptr->call_attrib_status_valid )
           {
              call_attrib_status.call_attrib_status = modify_ind_ptr->call_attrib_status;
              call_attrib_status.call_id = modify_ind_ptr->call_id;
           }
           /* update the audio/video parameters */
           qcril_qmi_voice_voip_update_call_info_entry_mainstream (
                                               call_info,
                                               &call_info->voice_scv_info,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               call_info->ril_call_state,
                                               modify_ind_ptr->audio_attrib_valid,
                                               &audio_attrib,
                                               modify_ind_ptr->video_attrib_valid,
                                               &video_attrib,
                                               modify_ind_ptr->call_attrib_status_valid,
                                               &call_attrib_status,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL,
                                               FALSE,
                                               NULL);

           // Reset the answered_call_type flag
           qcril_qmi_voice_voip_reset_answered_call_type(call_info, modify_ind_ptr);

           /* check for modify initiate pending request */
           result  = qcril_reqlist_query_by_event( instance_id,
                                                   modem_id,
                                                   QCRIL_EVT_QMI_REQUEST_MODIFY_INITIATE,
                                                   &req_info );

           /* now check for modify confirm pending request as both of them wait for same indication */
           if( result != E_SUCCESS )
           {
              result  = qcril_reqlist_query_by_event( instance_id,
                                                      modem_id,
                                                      QCRIL_EVT_QMI_REQUEST_MODIFY_CONFIRM,
                                                      &req_info );
           }

           if( result == E_SUCCESS )
           {
              /* modify was initiated/accepted(upgrade) by user, send the response now */
              qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID, req_info.t, req_info.request,
                                                        modify_ind_ptr->failure_cause_valid ? RIL_E_GENERIC_FAILURE : RIL_E_SUCCESS);
           }
           else
           {
              if (qcril_qmi_voice_info.jbims)
              {
                 if (modify_ind_ptr->failure_cause_valid && modify_ind_ptr->failure_cause)
                 {
                    Ims__CallModify ims_unsol_modify_data = IMS__CALL_MODIFY__INIT;

                    ims_unsol_modify_data.has_callindex = TRUE;
                    ims_unsol_modify_data.callindex = modify_ind_ptr->call_id;

                    Ims__CallDetails ims_call_details = IMS__CALL_DETAILS__INIT;
                    if ( call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_MODIFY_CONFIRM_PENDING )
                    {
                       ims_unsol_modify_data.calldetails = &ims_call_details;
                       ims_call_details.has_calltype = TRUE;
                       ims_call_details.calltype = call_info->to_modify_call_type;
                       ims_call_details.has_calldomain = TRUE;
                       ims_call_details.calldomain = call_info->to_modify_call_domain;
                       ims_call_details.has_mediaid = TRUE;
                       ims_call_details.mediaid = call_info->media_id;
                       call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_MODIFY_CONFIRM_PENDING;
                    }
                    else
                    {
                       QCRIL_LOG_DEBUG("QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_MODIFY_CONFIRM_PENDING is not set");
                    }

                    ims_unsol_modify_data.has_error = TRUE;
                    ims_unsol_modify_data.error = IMS__ERROR__E_CANCELLED; // E_CANCELLED is the only valid value to IMS

                    qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_MODIFY_CALL, IMS__ERROR__E_SUCCESS, &ims_unsol_modify_data, sizeof(ims_unsol_modify_data));
                 }
                 else
                 {
                    QCRIL_LOG_DEBUG("Received a unexpected QMI_VOICE_MODIFIED_IND with call_id: %d", modify_ind_ptr->call_id);
                 }
              }
           }

           if (qcril_qmi_voice_call_to_atel(call_info))
           {
              /* send unsol indication to telephony for indicating change in call type*/
              qcril_qmi_voice_send_unsol_call_state_changed( instance_id );
           }
           if (qcril_qmi_voice_call_to_ims(call_info))
           {
              /* send unsol indication to IMS for indicating change in call type*/
              qcril_qmi_voice_send_ims_unsol_call_state_changed();
           }
        }
        else
        {
           QCRIL_LOG_DEBUG("could not find call-id = %d, ignoring modify ind", modify_ind_ptr->call_id);
        }
      }
      else
      {
        QCRIL_LOG_DEBUG("received null data, ignoring modify ind");
      }
  }

  QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_modify_accept_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_VOICE_MODIFY_ACCEPT_IND_V02.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_modify_accept_ind_hdlr
(
  void   *data_ptr,
  uint32 data_len
)
{
  qcril_instance_id_e_type        instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_modem_id_e_type           modem_id = QCRIL_DEFAULT_MODEM_ID;
  voice_modify_accept_ind_msg_v02 *modify_ind_ptr = NULL;
  qcril_qmi_voice_voip_call_info_entry_type   *call_info = NULL;
  RIL_Call_Modify unsol_modify_data;
  qcril_unsol_resp_params_type    unsol_resp;
  boolean result;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(data_len);

  if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) || qcril_qmi_voice_info.jbims )
  {
      if( data_ptr != NULL )
      {
        modify_ind_ptr = (voice_modify_accept_ind_msg_v02 *)data_ptr;

        call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( modify_ind_ptr->call_id );

        if( call_info != NULL )
        {
           /* fill the unsol modify data */
           unsol_modify_data.callIndex = call_info->android_call_id;
           unsol_modify_data.callDetails = qcril_malloc( sizeof( *unsol_modify_data.callDetails ) );

           if( unsol_modify_data.callDetails != NULL )
           {
             result = !qcril_qmi_voice_get_atel_call_type_info( modify_ind_ptr->call_type,
                                                 modify_ind_ptr->video_attrib_valid,
                                                 modify_ind_ptr->video_attrib,
                                                 modify_ind_ptr->audio_attrib_valid,
                                                 modify_ind_ptr->audio_attrib,
                                                 FALSE,
                                                 0,
                                                 FALSE,
                                                 0,
                                                 FALSE,
                                                 0,
                                                 unsol_modify_data.callDetails );

             if( RIL_E_SUCCESS == result )
             {
                if (!qcril_qmi_voice_info.jbims)
                {
                   /* modify was initiated by network(other party), send indication to telephony */
                   qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_MODIFY_CALL, &unsol_resp );

                   unsol_resp.resp_pkt = &unsol_modify_data;
                   unsol_resp.resp_len = sizeof(unsol_modify_data);
                   qcril_send_unsol_response( &unsol_resp );
                }
                else
                {
                   /* modify was initiated by network(other party), send indication to ims */
                   Ims__CallModify ims_unsol_modify_data = IMS__CALL_MODIFY__INIT;
                   Ims__CallDetails ims_call_details = IMS__CALL_DETAILS__INIT;
                   ims_unsol_modify_data.calldetails = &ims_call_details;
                   qcril_qmi_ims_translate_ril_callmodify_to_ims_callmodify(&unsol_modify_data, &ims_unsol_modify_data);
                   qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_MODIFY_CALL, IMS__ERROR__E_SUCCESS, &ims_unsol_modify_data, sizeof(ims_unsol_modify_data));
                   call_info->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_MODIFY_CONFIRM_PENDING;
                   call_info->to_modify_call_domain = ims_call_details.calldomain;
                   call_info->to_modify_call_type = ims_call_details.calltype;
                }
             }
             else
             {
                QCRIL_LOG_DEBUG("could not convert modem call type to atel call type");
             }
             qcril_free( unsol_modify_data.callDetails );
           }
           else
           {
             QCRIL_LOG_ERROR("memory malloc failed");
           }
        }
        else
        {
           QCRIL_LOG_DEBUG("could not find call-id = %d, ignoring modify accept ind", modify_ind_ptr->call_id);
        }
      }
      else
      {
        QCRIL_LOG_DEBUG("received null data, ignoring modify accept ind");
      }
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================*/
/*!
    @brief
    Handle QMI_VOICE_SPEECH_CODEC_INFO_IND_V02.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_speech_codec_info_ind_hdlr
(
  void   *data_ptr,
  uint32 data_len
)
{
  voice_speech_codec_info_ind_msg_v02* speech_codec_ptr;
  boolean call_to_atel = FALSE;
  boolean call_to_ims  = FALSE;
  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED(data_len);

  if( data_ptr )
  {
     speech_codec_ptr = (voice_speech_codec_info_ind_msg_v02*) data_ptr;

     if ( TRUE == qcril_qmi_voice_speech_codec_info.report_speech_codec )
     {
       SPEECH_CODEC_LOCK();
       qcril_qmi_voice_speech_codec_info.speech_codec_info_valid = TRUE;
       qcril_qmi_voice_speech_codec_info.network_mode = VOICE_NETWORK_MODE_NONE_V02;
       qcril_qmi_voice_speech_codec_info.speech_codec = VOICE_SPEECH_CODEC_NONE_V02;
       if( speech_codec_ptr->network_mode_valid )
       {
         qcril_qmi_voice_speech_codec_info.network_mode = speech_codec_ptr->network_mode;
       }
       if( speech_codec_ptr->speech_codec_valid )
       {
         qcril_qmi_voice_speech_codec_info.speech_codec = speech_codec_ptr->speech_codec;
       }
       SPEECH_CODEC_SIGNAL();
       SPEECH_CODEC_UNLOCK();
     }

     // add codec info to the associated call
     if (speech_codec_ptr->call_id_valid && speech_codec_ptr->speech_codec_valid)
     {
        qcril_qmi_voice_voip_lock_overview();
        qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id(speech_codec_ptr->call_id);
        if (NULL == call_info_entry)
        {
           QCRIL_LOG_ERROR("did not find the call with qmi id: %d", speech_codec_ptr->call_id);
        }
        else
        {
           call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CODEC_VALID;
           call_info_entry->codec = speech_codec_ptr->speech_codec;
           call_to_atel = qcril_qmi_voice_call_to_atel(call_info_entry);
           call_to_ims  = qcril_qmi_voice_call_to_ims (call_info_entry);
        }
        qcril_qmi_voice_voip_unlock_overview();
        if (call_to_atel)
        {
          qcril_qmi_voice_send_unsol_call_state_changed( QCRIL_DEFAULT_INSTANCE_ID );
        }
        if (call_to_ims)
        {
          qcril_qmi_voice_send_ims_unsol_call_state_changed();
        }
     }
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_nas_control_is_call_mode_reported_voice_radio_tech_different

===========================================================================*/
/*!
    @brief
    Compares call_mode to reported voice radio tech and
    returns TRUE if they do not belong to the same radio_technology family

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_voice_nas_control_is_call_mode_reported_voice_radio_tech_different(call_mode_enum_v02 call_mode)
{
  unsigned int res = FALSE;
  unsigned int call_radio_tech = RADIO_TECH_UNKNOWN;
  unsigned int old_call_radio_tech = RADIO_TECH_UNKNOWN;
  unsigned int old_call_radio_tech_family = RADIO_TECH_UNKNOWN;

  int is_ics;

  is_ics = qmi_ril_is_feature_supported( QMI_RIL_FEATURE_ICS );
  call_radio_tech = qcril_qmi_voice_convert_call_mode_to_radio_tech_family(call_mode);

  old_call_radio_tech = qcril_qmi_voice_nas_control_get_reported_voice_radio_tech();
  old_call_radio_tech_family = qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family(old_call_radio_tech);

  if (
      ( call_radio_tech != old_call_radio_tech_family ) &&
      ( ( is_ics && RADIO_TECH_UNKNOWN != old_call_radio_tech ) || ( !is_ics && QMI_RIL_ZERO != old_call_radio_tech ) )
     )
  {
    res = TRUE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}/* qcril_qmi_voice_nas_control_is_call_mode_reported_voice_radio_tech_different */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family

===========================================================================*/
/*!
    @brief
    Converts the reported voice radio tech to voice radio tech family in case
    of ICS


    @return
    Voice radio tech family.
*/
/*=========================================================================*/
unsigned int qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family(unsigned int voice_radio_tech)
{
  int is_ics = qmi_ril_is_feature_supported(  QMI_RIL_FEATURE_ICS );
  unsigned int voice_radio_tech_family = RADIO_TECH_UNKNOWN;

  QCRIL_LOG_INFO("entered voice_radio_tech %d", (int) voice_radio_tech);

  /* convert to the radio tech family in case of ics as it will be computed
      in terms of radio tech*/
  if( is_ics )
  {
    switch( voice_radio_tech )
    {
      case RADIO_TECH_GPRS:
      case RADIO_TECH_EDGE:
      case RADIO_TECH_UMTS:
      case RADIO_TECH_TD_SCDMA:
      case RADIO_TECH_HSDPA:
      case RADIO_TECH_HSUPA:
      case RADIO_TECH_HSPA:
      case RADIO_TECH_LTE:
          voice_radio_tech_family = RADIO_TECH_3GPP;
          break;

      case RADIO_TECH_IS95A:
      case RADIO_TECH_IS95B:
      case RADIO_TECH_1xRTT:
      case RADIO_TECH_EVDO_0:
      case RADIO_TECH_EVDO_A:
      case RADIO_TECH_EVDO_B:
      case RADIO_TECH_EHRPD:
          voice_radio_tech_family = RADIO_TECH_3GPP2;
          break;

      default:
       QCRIL_LOG_ERROR("invalid radio tech = %d", voice_radio_tech);
       break;
    }
  }
  else
  {
    voice_radio_tech_family = voice_radio_tech;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(voice_radio_tech_family);
  return voice_radio_tech_family;
}/* qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_convert_call_mode_to_radio_tech_family

===========================================================================*/
/*!
    @brief
    Converts the reported call mode to voice radio tech family


    @return
    Voice radio tech family.
*/
/*=========================================================================*/
unsigned int qcril_qmi_voice_convert_call_mode_to_radio_tech_family(call_mode_enum_v02 call_mode)
{
  unsigned int call_radio_tech = RADIO_TECH_UNKNOWN;

  QCRIL_LOG_INFO("entered call_mode %d", (int) call_mode);

  switch( call_mode )
  {
    case CALL_MODE_CDMA_V02:
      call_radio_tech = RADIO_TECH_3GPP2;
      break;

    case CALL_MODE_GSM_V02:
    case CALL_MODE_UMTS_V02:
    case CALL_MODE_TDS_V02:
    case CALL_MODE_LTE_V02:
      call_radio_tech = RADIO_TECH_3GPP;
      break;

    default:
      call_radio_tech = RADIO_TECH_UNKNOWN;
      break;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(call_radio_tech);
  return call_radio_tech;
}/* qcril_qmi_voice_convert_call_mode_to_radio_tech_family */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_is_emergency_call_pending

===========================================================================*/
/*!
    @brief
    Checks If emergency call has been put on hold for PRL to be loaded in 3GPP2
    mode.


    @return
    TRUE if emergency call is pending.
*/
/*=========================================================================*/
int qcril_qmi_voice_is_emergency_call_pending()
{
  int ret;
  QCRIL_LOG_FUNC_ENTRY();

  ret = qcril_qmi_pending_emergency_call_info.is_emergency_call_pending;

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
}/* qcril_qmi_voice_is_emergency_call_pending */

//===========================================================================
//qcril_qmi_voice_nas_control_get_reported_voice_radio_tech
//===========================================================================
RIL_RadioTechnologyFamily qcril_qmi_voice_nas_control_get_reported_voice_radio_tech()
{
    RIL_RadioTechnologyFamily voice_radio_tech;

    QCRIL_LOG_FUNC_ENTRY();

    voice_radio_tech = qcril_qmi_nas_get_reported_voice_radio_tech(TRUE);

    QCRIL_LOG_INFO("completed with voice_radio_tech %d", voice_radio_tech);
    return voice_radio_tech;
} //qcril_qmi_voice_nas_control_get_reported_voice_radio_tech

//===========================================================================
//qcril_qmi_voice_nas_control_process_calls_pending_for_right_voice_rte
//===========================================================================
void qcril_qmi_voice_nas_control_process_calls_pending_for_right_voice_rte()
{
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  unsigned int call_radio_tech = RADIO_TECH_UNKNOWN;
  unsigned int call_radio_tech_family = RADIO_TECH_UNKNOWN;
  int res = FALSE;
  boolean call_to_atel = FALSE;
  boolean call_to_ims  = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  call_radio_tech = qcril_qmi_voice_nas_control_get_reported_voice_radio_tech();
  call_radio_tech_family = qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family(call_radio_tech);

  qcril_qmi_voice_voip_lock_overview();
  call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
  while ( NULL != call_info_entry )
  {
    qcril_qmi_voice_voip_call_info_dump( call_info_entry );
    if( (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_WAITING_FOR_MATCHING_VOICE_RTE) &&
        (call_radio_tech_family == qcril_qmi_voice_convert_call_mode_to_radio_tech_family(call_info_entry->voice_scv_info.mode)) )
    {
      call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_WAITING_FOR_MATCHING_VOICE_RTE; //Resetting the WAITING_FOR_MATCHING_VOICE_RTE bit
      res = TRUE;
      call_to_atel = qcril_qmi_voice_call_to_atel(call_info_entry);
      call_to_ims  = qcril_qmi_voice_call_to_ims (call_info_entry);

      QCRIL_LOG_INFO("Resuming android call id %d as call mode matches with the current voice radio tech",call_info_entry->android_call_id);
    }
    call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
  }
  qcril_qmi_voice_voip_unlock_overview();

  if( res )
  {
     if (call_to_atel)
     {
       qcril_qmi_voice_send_unsol_call_state_changed( QCRIL_DEFAULT_INSTANCE_ID );
     }
     if (call_to_ims)
     {
       qcril_qmi_voice_send_ims_unsol_call_state_changed();
     }
  }

  QCRIL_LOG_FUNC_RETURN();

} //qcril_qmi_voice_nas_control_process_calls_pending_for_right_voice_rte

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_sups_notification_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_VOICE_SUPS_NOTIFICATION_IND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_sups_notification_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_sups_notification_ind_msg_v02* sups_notify_ind;
  qcril_instance_id_e_type instance_id= QCRIL_DEFAULT_INSTANCE_ID;;
  RIL_SuppSvcNotification response;
  boolean success = FALSE;
  qcril_unsol_resp_params_type unsol_resp;
  char number[ 2 * QCRIL_QMI_VOICE_CALLED_PARTY_BCD_NO_LENGTH ];
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  char ip_hist_info_utf8_str[QCRIL_QMI_VOICE_MAX_IP_HISTORY_INFO_LEN*2];
  int  utf8_len = 0;
  int  i = 0;

  if( ind_data_ptr != NULL )
  {
    ind_data_len = ind_data_len;
    sups_notify_ind = (voice_sups_notification_ind_msg_v02*)ind_data_ptr;

    memset( ( void* )&response, 0, sizeof( response ) );
    memset( ( void* )&number, 0, sizeof( number ) );
    /* Add event processing here */
    switch(sups_notify_ind->notification_info.notification_type)
    {
      case NOTIFICATION_TYPE_UNCOND_CALL_FORWARD_ACTIVE_V02:
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_UNCOND_FWD_ACTIVE;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_COND_CALL_FORWARD_ACTIVE_V02:
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_COND_FWD_ACTIVE;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_OUTGOING_CALL_IS_FORWARDED_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_CALL_FORWARDED;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_OUTGOING_CALL_IS_WAITING_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_CALL_WAITING;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_OUTGOING_CALLS_BARRED_V02:
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_OUTGOING_CALLS_BARRED;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_INCOMING_CALLS_BARRED_V02:
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_INCOMING_CALLS_BARRED;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_CLIR_SUPPRSESION_REJECTED_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_CLIR_SUPPRESSION_REJ;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_INCOMING_FORWARDED_CALL_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_FORWARDED_CALL;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_OUTGOING_CUG_CALL_V02:
        if(sups_notify_ind->index_valid)
        {
          response.index = sups_notify_ind->index;
        }
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_CUG_CALL;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_INCOMING_CUG_CALL_V02:
        if(sups_notify_ind->index_valid)
        {
          response.index = sups_notify_ind->index;
        }
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_CUG_CALL;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_CALL_IS_ON_HOLD_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_CALL_HOLD;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_CALL_IS_RETRIEVED_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_CALL_RETRIEVED;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_INCOMING_CALL_IS_FORWARDED_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_ADDITIONAL_INCOM_CALL_FWD;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_INCOMING_CALL_IS_ECT_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        if(sups_notify_ind->ect_number_valid == TRUE)
        {
          if(sups_notify_ind->ect_number.ect_call_state == ECT_CALL_STATE_ALERTING_V02)
          {
            response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_ECT_CALL_REMOTE_PTY_ALERT;
          }
          else if(sups_notify_ind->ect_number.ect_call_state == ECT_CALL_STATE_ACTIVE_V02)
          {
            response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_ECT_CALL_REMOTE_PTY_CONNECTED;
            if(sups_notify_ind->ect_number.number_len != 0 )
            {
              memset(number,0,sizeof(number));
              if( sups_notify_ind->ect_number.number_len < sizeof(number) )
              {
                memcpy(number,sups_notify_ind->ect_number.number,sups_notify_ind->ect_number.number_len);
              }
              else
              {
                memcpy(number,sups_notify_ind->ect_number.number,(sizeof(number)-1));
              }

              /* Set Type based on '+' prefix */
              response.type = ( QCRIL_QMI_VOICE_SS_TA_INTER_PREFIX == number[ 0 ] ) ? QCRIL_QMI_VOICE_SS_TA_INTERNATIONAL : QCRIL_QMI_VOICE_SS_TA_UNKNOWN;
              /* number[0] contains type and the rest contains the number in ASCII */
              if ( response.type == QCRIL_QMI_VOICE_SS_TA_INTERNATIONAL )
              {
                response.number = (char * )&number[ 1 ];
              }
              else
              {
                response.number = (char * )&number[ 0 ];
              }
              QCRIL_LOG_DEBUG( "ECT notification has number = %s",response.number);
            }
          }
          else
          {
            QCRIL_LOG_DEBUG( "Invalid ECT notification call state  = %d",sups_notify_ind->ect_number.ect_call_state);
          }

        }
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_OUTGOING_CALL_IS_DEFLECTED_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MO_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSI_CALL_DEFLECTED;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_INCOMING_DEFLECTED_CALL_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_DEFLECTED_CALL;
        success = TRUE;
        break;
      case NOTIFICATION_TYPE_CALL_IS_IN_MPTY_V02:
        response.number = NULL;
        response.notificationType = (int) QCRIL_QMI_VOICE_SS_MT_NOTIFICATION;
        response.code = (int) QCRIL_QMI_VOICE_SS_CSSU_MPTY_CALL;
        success = TRUE;
        break;
      default :
        QCRIL_LOG_ERROR( "Invalid sups notification type recieved = %d",sups_notify_ind->notification_info.notification_type);
        break;
    }

    if ( success )
    {
      QCRIL_LOG_DEBUG( "QCRIL_EVT_CM_CALL_ORIG_FWD_STATUS notification type %d, response code %d",
                       response.notificationType, response.code);

      /* Call related notifications to RIL */
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( sups_notify_ind->notification_info.call_id );
      if (!qcril_qmi_voice_call_to_ims(call_info_entry))
      {
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_SUPP_SVC_NOTIFICATION, &unsol_resp );
        unsol_resp.resp_pkt = ( void * ) &response;
        unsol_resp.resp_len = sizeof( response );
        qcril_send_unsol_response( &unsol_resp );
      }
      else
      {
        Ims__SuppSvcNotification ims_unsol_supp_svc_notification = IMS__SUPP_SVC_NOTIFICATION__INIT;
        qcril_qmi_ims_translate_ril_suppsvcnotification_to_ims_suppsvcnotification(&response, &ims_unsol_supp_svc_notification);

        if (call_info_entry)
        {
            ims_unsol_supp_svc_notification.has_connid = TRUE;
            ims_unsol_supp_svc_notification.connid = call_info_entry->android_call_id;
        }

        if(sups_notify_ind->ip_forward_hist_info_valid)
        {
            //Convert UTF16 to UTF8 string
            utf8_len = qcril_cm_ss_convert_ucs2_to_utf8(
                                 (char *) sups_notify_ind->ip_forward_hist_info,
                                 sups_notify_ind->ip_forward_hist_info_len * 2,
                                 ip_hist_info_utf8_str );

            QCRIL_LOG_DEBUG ("ip_hist_info (utf8 data) : %s", ip_hist_info_utf8_str);
            if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_IP_HISTORY_INFO_LEN * 2 ) )
            {
              QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_IP_HISTORY_INFO_LEN");
              utf8_len = (int) (QCRIL_QMI_VOICE_MAX_IP_HISTORY_INFO_LEN*2);
              ip_hist_info_utf8_str[ utf8_len - 1] = '\0';
            }
            ims_unsol_supp_svc_notification.history_info = qmi_ril_util_str_clone(ip_hist_info_utf8_str);
        }

        qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_SUPP_SVC_NOTIFICATION, IMS__ERROR__E_SUCCESS, &ims_unsol_supp_svc_notification, sizeof(ims_unsol_supp_svc_notification));
        if (ims_unsol_supp_svc_notification.number)
        {
          qcril_free( ims_unsol_supp_svc_notification.number );
        }
        if (ims_unsol_supp_svc_notification.history_info)
        {
          qcril_free( ims_unsol_supp_svc_notification.history_info );
        }
      }
    }
  }

}/* qcril_qmi_voice_sups_notification_ind_hdlr */

//===========================================================================
// qcril_qmi_voice_call_num_copy_with_toa_check
//===========================================================================
uint32_t qcril_qmi_voice_call_num_copy_with_toa_check(char *src, uint32_t src_size, char* dest,
                                                      uint32_t dest_buffer_size, voice_num_type_enum_v02 num_type)
{
  uint32_t ret_size = 0;
  int offset = 0;

  if ( NULL == src || NULL == dest || src_size + 1 >= dest_buffer_size )
  {
    QCRIL_LOG_ERROR("function paramenter incorrect");
  }
  else
  {
    if ( QMI_VOICE_NUM_TYPE_INTERNATIONAL_V02 != num_type )
    {
      ret_size = src_size;
      memcpy(dest, src, src_size);
    }
    else
    {
      if ( QCRIl_QMI_VOICE_SS_TA_INTER_PREFIX == src[0] )
      {
        ret_size = src_size;
        memcpy(dest, src, src_size);
      }
      else
      {
        if (src_size > 1 && src[0] == '0' && src[1] == '0')
        {
          QCRIL_LOG_INFO("Removing 00 prefix");
          offset = 2;
          src_size-=2; //src_size can not turn negative since we have already checked "if (src_size > 1" above
        }
        ret_size = src_size + 1;
        dest[0] = QCRIl_QMI_VOICE_SS_TA_INTER_PREFIX;
        memcpy(dest+1, src + offset, src_size);
      }
    }
    dest [ret_size] = 0;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET( ret_size );

  return ret_size;
} // qcril_qmi_voice_call_num_copy_with_toa_check

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_info_rec_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI_VOICE_INFO_REC_IND.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_info_rec_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  RIL_CDMA_CallWaiting            call_waiting;
  RIL_CDMA_InformationRecords     info_recs;
  RIL_CDMA_SignalInfoRecord       signal_info_rec;
  voice_info_rec_ind_msg_v02 *    info_rec_ind;
  qcril_unsol_resp_params_type    unsol_resp;
  int                             idx=0;
  RIL_CDMA_InformationRecord *    info_rec_ptr;
  uint8                           display_tag;
  uint8                           display_len;
  boolean                         call_is_in_call_waiting_state = FALSE;
  boolean                         ignore_caller_name = FALSE;
  unsigned int                    buf_len = 0;
  struct
  {
    char buf[QMI_VOICE_CALLER_ID_MAX_V02+1];
    char pi;
  } number;
  char                            name[QMI_VOICE_CALLER_NAME_MAX_V02+1];
  boolean                         name_changed = FALSE,
                                  number_changed = FALSE;
  int                             iter_idx;
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = NULL;
  RIL_CDMA_SignalInfoRecord*      info_rec_pass_over;

  int                             need_reset_1x_num_wait_tmr;

  QCRIL_LOG_FUNC_ENTRY();

  need_reset_1x_num_wait_tmr = FALSE;

  if( ind_data_ptr != NULL )
  {
    ind_data_len = ind_data_len;
    info_rec_ind = (voice_info_rec_ind_msg_v02*)ind_data_ptr;
    memset( &info_recs, 0, sizeof( info_recs ) );
    memset( &number, 0, sizeof(number) );
    memset( &name, 0, sizeof(name) );
    memset( &signal_info_rec, 0, sizeof(signal_info_rec) );


    QCRIL_LOG_INFO(".. call_waiting_valid, state %d, %d", (int)info_rec_ind->call_waiting_valid, (int)info_rec_ind->call_waiting );
    if (info_rec_ind->call_waiting_valid)
    {
      call_is_in_call_waiting_state = (info_rec_ind->call_waiting == CALL_WAITING_NEW_CALL_V02) ?
                                       TRUE: FALSE;
    }
    QCRIL_LOG_INFO(".. call in waiting state org %d", (int)call_is_in_call_waiting_state );

    QCRIL_LOG_INFO(".. caller_id_info_valid %d", (int)info_rec_ind->caller_id_info_valid );
    QCRIL_LOG_INFO(".. calling_party_info_valid %d", (int)info_rec_ind->calling_party_info_valid );
    if( info_rec_ind->caller_id_info_valid && info_rec_ind->calling_party_info_valid )
    {
      QCRIL_LOG_INFO("caller_id caller_id_len: %d, pi: %d", info_rec_ind->caller_id_info.caller_id_len, info_rec_ind->caller_id_info.pi);
      QCRIL_LOG_INFO("calling_party_info num_type: %d, num_plan: %d, si: %d",
                     info_rec_ind->calling_party_info.num_type, info_rec_ind->calling_party_info.num_plan,
                     info_rec_ind->calling_party_info.si);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC;

      info_rec_ptr->rec.number.len = qcril_qmi_voice_call_num_copy_with_toa_check(info_rec_ind->caller_id_info.caller_id,
                                                                                  info_rec_ind->caller_id_info.caller_id_len,
                                                                                  info_rec_ptr->rec.number.buf,
                                                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH,
                                                                                  info_rec_ind->calling_party_info.num_type);

      info_rec_ptr->rec.number.pi = (char)info_rec_ind->caller_id_info.pi;
      info_rec_ptr->rec.number.number_type = info_rec_ind->calling_party_info.num_type;
      info_rec_ptr->rec.number.number_plan = info_rec_ind->calling_party_info.num_plan;
      info_rec_ptr->rec.number.si = info_rec_ind->calling_party_info.si;
      idx++;

      memcpy(number.buf,
              info_rec_ptr->rec.number.buf,
               info_rec_ptr->rec.number.len);
      number.pi = info_rec_ptr->rec.number.pi ;
      number_changed = TRUE;
    }
    else if ( info_rec_ind->caller_id_info_valid && !info_rec_ind->calling_party_info_valid )
    {
      QCRIL_LOG_INFO("caller_id caller_id_len: %d, pi: %d", info_rec_ind->caller_id_info.caller_id_len, info_rec_ind->caller_id_info.pi);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC;
      info_rec_ptr->rec.number.len = info_rec_ind->caller_id_info.caller_id_len;
      memcpy(info_rec_ptr->rec.number.buf,
             info_rec_ind->caller_id_info.caller_id,
             info_rec_ptr->rec.number.len);
      info_rec_ptr->rec.number.pi = (char)info_rec_ind->caller_id_info.pi;
      info_rec_ptr->rec.number.number_type = QMI_VOICE_NUM_TYPE_UNKNOWN_V02;
      info_rec_ptr->rec.number.number_plan = QMI_VOICE_NUM_PLAN_UNKNOWN_V02;
      info_rec_ptr->rec.number.si = QMI_VOICE_SI_USER_PROVIDED_NOT_SCREENED_V02;
      idx++;

      memcpy(number.buf,
              info_rec_ptr->rec.number.buf,
               info_rec_ptr->rec.number.len);
      number.pi = info_rec_ptr->rec.number.pi ;
      number_changed = TRUE;
    }
    else if ( info_rec_ind->calling_party_info_valid )
    {
      QCRIL_LOG_INFO("calling_party_info num_len: %d, pi: %d, num_type: %d, num_plan: %d, si: %d",
                     info_rec_ind->calling_party_info.num_len, info_rec_ind->calling_party_info.pi,
                     info_rec_ind->calling_party_info.num_type, info_rec_ind->calling_party_info.num_plan,
                     info_rec_ind->calling_party_info.si);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC;

      info_rec_ptr->rec.number.len = qcril_qmi_voice_call_num_copy_with_toa_check(info_rec_ind->calling_party_info.num,
                                                                                  info_rec_ind->calling_party_info.num_len,
                                                                                  info_rec_ptr->rec.number.buf,
                                                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH,
                                                                                  info_rec_ind->calling_party_info.num_type);

      info_rec_ptr->rec.number.pi = (char)info_rec_ind->calling_party_info.pi;
      info_rec_ptr->rec.number.number_type = info_rec_ind->calling_party_info.num_type;
      info_rec_ptr->rec.number.number_plan = info_rec_ind->calling_party_info.num_plan;
      info_rec_ptr->rec.number.si = info_rec_ind->calling_party_info.si;

      idx++;

      memcpy(number.buf,
              info_rec_ptr->rec.number.buf,
               info_rec_ptr->rec.number.len);
      number.pi = info_rec_ptr->rec.number.pi ;
      number_changed = TRUE;
    }

    QCRIL_LOG_INFO(".. display_buffer_valid %d", (int)info_rec_ind->display_buffer_valid );
    if ( info_rec_ind->display_buffer_valid )
    {
      buf_len = strlen(info_rec_ind->display_buffer);
      QCRIL_LOG_INFO("display_buffer length: %d", buf_len);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_DISPLAY_INFO_REC;
      info_rec_ptr->rec.display.alpha_len = buf_len;
      if ( info_rec_ptr->rec.display.alpha_len > CDMA_ALPHA_INFO_BUFFER_LENGTH )
      {
        info_rec_ptr->rec.display.alpha_len = CDMA_ALPHA_INFO_BUFFER_LENGTH;
      }
      memcpy(info_rec_ptr->rec.display.alpha_buf,
             info_rec_ind->display_buffer,
             info_rec_ptr->rec.display.alpha_len);
      idx++;
    }

    QCRIL_LOG_INFO(".. ext_display_record_valid %d", (int)info_rec_ind->ext_display_record_valid );
    QCRIL_LOG_INFO(".. ext_display_buffer_valid %d", (int)info_rec_ind->ext_display_buffer_valid );
    if ( info_rec_ind->ext_display_record_valid )
    {
      display_tag = info_rec_ind->ext_display_record.ext_display_info[0];
      display_len = info_rec_ind->ext_display_record.ext_display_info[1];
      QCRIL_LOG_INFO("ext_display_record display_type: %d, ext_display_info_len: %d, is_0x9e_not_treat_as_name: %d",
                     display_tag, display_len, qcril_qmi_voice_info.is_0x9e_not_treat_as_name);
      if ( display_tag == 0x8D || display_tag == 0x8F || (display_tag == 0x9E && !qcril_qmi_voice_info.is_0x9e_not_treat_as_name))
      {
        memcpy(name, info_rec_ind->ext_display_record.ext_display_info+2, display_len);
        name_changed = TRUE;
      }
      else
      {
        info_rec_ptr = &info_recs.infoRec[idx];
        info_rec_ptr->name = RIL_CDMA_EXTENDED_DISPLAY_INFO_REC;

        info_rec_ptr->rec.display.alpha_len = info_rec_ind->ext_display_record.ext_display_info_len;
        if ( info_rec_ptr->rec.display.alpha_len > CDMA_ALPHA_INFO_BUFFER_LENGTH )
        {
          info_rec_ptr->rec.display.alpha_len = CDMA_ALPHA_INFO_BUFFER_LENGTH;
        }
        memcpy(info_rec_ptr->rec.display.alpha_buf,
               info_rec_ind->ext_display_record.ext_display_info,
               info_rec_ptr->rec.display.alpha_len);
        idx++;
        if ( display_tag == 0x9E && qcril_qmi_voice_info.is_0x9e_not_treat_as_name )
        {
          ignore_caller_name = TRUE;
        }
      }
    }
    else if(info_rec_ind->ext_display_buffer_valid)
    {
      buf_len = strlen(info_rec_ind->ext_display_buffer);
      QCRIL_LOG_INFO("ext_display_buffer length: %d", buf_len);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_EXTENDED_DISPLAY_INFO_REC;
      info_rec_ptr->rec.display.alpha_len = buf_len;
      if ( info_rec_ptr->rec.display.alpha_len > CDMA_ALPHA_INFO_BUFFER_LENGTH )
      {
        info_rec_ptr->rec.display.alpha_len = CDMA_ALPHA_INFO_BUFFER_LENGTH;
      }
      memcpy(info_rec_ptr->rec.display.alpha_buf,
             info_rec_ind->ext_display_buffer,
             info_rec_ptr->rec.display.alpha_len);
      idx++;
    }

    QCRIL_LOG_INFO(".. caller_name_valid %d", (int)info_rec_ind->caller_name_valid );
    QCRIL_LOG_INFO(".. name_changed %d", (int)name_changed);
    if(info_rec_ind->caller_name_valid && !name_changed && !ignore_caller_name)
    {
      buf_len = strlen(info_rec_ind->ext_display_buffer);
      QCRIL_LOG_INFO("caller_name length: %d", buf_len);
      memcpy(name, info_rec_ind->caller_name, buf_len);
      name_changed = TRUE;
    }

    QCRIL_LOG_INFO(".. audio_control_valid %d", (int)info_rec_ind->audio_control_valid );
    if(info_rec_ind->audio_control_valid)
    {
      QCRIL_LOG_INFO("audio control downlink: %d, uplink: %d",
                     info_rec_ind->audio_control.down_link, info_rec_ind->audio_control.up_link);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC;
      info_rec_ptr->rec.audioCtrl.downLink = info_rec_ind->audio_control.down_link;
      info_rec_ptr->rec.audioCtrl.upLink = info_rec_ind->audio_control.up_link;
      idx++;
    }

    QCRIL_LOG_INFO(".. clir_cause_valid %d", (int)info_rec_ind->clir_cause_valid );
    if (info_rec_ind->clir_cause_valid )
    {
      QCRIL_LOG_INFO("clir_cause: %d", info_rec_ind->clir_cause);
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_T53_CLIR_INFO_REC;
      info_rec_ptr->rec.clir.cause = (char)info_rec_ind->clir_cause;
      number_changed = TRUE;
      idx++;
    }

    QCRIL_LOG_INFO(".. nss_release_valid %d", (int)info_rec_ind->nss_release_valid);
    if ( info_rec_ind->nss_release_valid )
    {
      QCRIL_LOG_INFO("nss_release: %d", info_rec_ind->nss_release);
      uint32 nss_release_size = sizeof(info_rec_ind->nss_release);
      uint32 call_id_size = sizeof(info_rec_ind->call_id);
      char info[(nss_release_size + call_id_size)];
      memcpy(info, &info_rec_ind->nss_release, nss_release_size);
      memcpy((info + nss_release_size), &info_rec_ind->call_id, call_id_size);
      qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_NSS_RELEASE,
                                info, (nss_release_size + call_id_size) );

      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_T53_RELEASE_INFO_REC;
      idx++;
    }

    QCRIL_LOG_INFO(".. redirecting_num_info_valid %d", (int)info_rec_ind->redirecting_num_info_valid );
    if ( info_rec_ind->redirecting_num_info_valid )
    {
      QCRIL_LOG_INFO("redirecting reason: %d, pi: %d, si: %d, num_plan: %d, num_type: %d, len: %d",
                     info_rec_ind->redirecting_num_info.reason, info_rec_ind->redirecting_num_info.pi,
                     info_rec_ind->redirecting_num_info.si, info_rec_ind->redirecting_num_info.num_plan,
                     info_rec_ind->redirecting_num_info.num_type, info_rec_ind->redirecting_num_info.num_len );

      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_REDIRECTING_NUMBER_INFO_REC;
      info_rec_ptr->rec.redir.redirectingReason = info_rec_ind->redirecting_num_info.reason;
      info_rec_ptr->rec.redir.redirectingNumber.pi = info_rec_ind->redirecting_num_info.pi;
      info_rec_ptr->rec.redir.redirectingNumber.si = info_rec_ind->redirecting_num_info.si;
      info_rec_ptr->rec.redir.redirectingNumber.number_plan = info_rec_ind->redirecting_num_info.num_plan;
      info_rec_ptr->rec.redir.redirectingNumber.number_type = info_rec_ind->redirecting_num_info.num_type;
      info_rec_ptr->rec.redir.redirectingNumber.len = qcril_qmi_voice_call_num_copy_with_toa_check(info_rec_ind->redirecting_num_info.num,
                                                                                                   info_rec_ind->redirecting_num_info.num_len,
                                                                                                   info_rec_ptr->rec.redir.redirectingNumber.buf,
                                                                                                   CDMA_NUMBER_INFO_BUFFER_LENGTH,
                                                                                                   info_rec_ind->redirecting_num_info.num_type);
      idx++;
    }

    QCRIL_LOG_INFO(".. line_control_valid %d", (int)info_rec_ind->line_control_valid);
    if ( info_rec_ind->line_control_valid )
    {
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_LINE_CONTROL_INFO_REC;
      info_rec_ptr->rec.lineCtrl.lineCtrlPolarityIncluded = (char) info_rec_ind->line_control.polarity_included;
      info_rec_ptr->rec.lineCtrl.lineCtrlToggle = info_rec_ind->line_control.toggle_mode;
      info_rec_ptr->rec.lineCtrl.lineCtrlReverse = info_rec_ind->line_control.reverse_polarity;
      info_rec_ptr->rec.lineCtrl.lineCtrlPowerDenial = info_rec_ind->line_control.power_denial_time;
      idx++;
    }

    QCRIL_LOG_INFO(".. conn_num_info_valid %d", (int)info_rec_ind->conn_num_info_valid);
    if ( info_rec_ind->conn_num_info_valid )
    {
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_CONNECTED_NUMBER_INFO_REC;
      info_rec_ptr->rec.number.number_plan = info_rec_ind->conn_num_info.num_plan;
      info_rec_ptr->rec.number.number_type = info_rec_ind->conn_num_info.num_type;
      info_rec_ptr->rec.number.pi = info_rec_ind->conn_num_info.pi;
      info_rec_ptr->rec.number.si = info_rec_ind->conn_num_info.si;

      info_rec_ptr->rec.number.len = qcril_qmi_voice_call_num_copy_with_toa_check(info_rec_ind->conn_num_info.num,
                                                                                  info_rec_ind->conn_num_info.num_len,
                                                                                  info_rec_ptr->rec.number.buf,
                                                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH,
                                                                                  info_rec_ind->conn_num_info.num_type);
      idx++;
    }

    QCRIL_LOG_INFO(".. called_party_info_valid %d", (int)info_rec_ind->called_party_info_valid);
    if ( info_rec_ind->called_party_info_valid )
    {
      info_rec_ptr = &info_recs.infoRec[idx];
      info_rec_ptr->name = RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC;
      info_rec_ptr->rec.number.number_plan = info_rec_ind->called_party_info.num_plan;
      info_rec_ptr->rec.number.number_type = info_rec_ind->called_party_info.num_type;
      info_rec_ptr->rec.number.pi = info_rec_ind->called_party_info.pi;
      info_rec_ptr->rec.number.si = info_rec_ind->called_party_info.si;

      info_rec_ptr->rec.number.len = qcril_qmi_voice_call_num_copy_with_toa_check(info_rec_ind->called_party_info.num,
                                                                                  info_rec_ind->called_party_info.num_len,
                                                                                  info_rec_ptr->rec.number.buf,
                                                                                  CDMA_NUMBER_INFO_BUFFER_LENGTH,
                                                                                  info_rec_ind->called_party_info.num_type);
      idx++;
    }

    QCRIL_LOG_INFO(".. signal_info_valid %d", (int)info_rec_ind->signal_info_valid );
    if( info_rec_ind->signal_info_valid )
    {
      signal_info_rec.isPresent = TRUE;
      signal_info_rec.signalType = (char)info_rec_ind->signal_info.signal_type;
      signal_info_rec.alertPitch = (char)info_rec_ind->signal_info.alert_pitch;
      signal_info_rec.signal = (char)info_rec_ind->signal_info.signal;
    }

    QCRIL_LOG_INFO(".. is waiting state %d, name changed %d, number changed %d  ", (int)call_is_in_call_waiting_state, (int)name_changed, (int)number_changed );

    if( call_is_in_call_waiting_state )
    {
      /* Fill in the Call Waiting information */
      call_waiting.number = number.buf;
      call_waiting.numberPresentation = number.pi;
      call_waiting.name = name;
      call_waiting.signalInfoRecord = signal_info_rec;

      QCRIL_LOG_INFO( "call is in waiting state. Number : %s; Number Presentation: %d; Name : %s; Signal Info Rec.isPresent : %d",
                      call_waiting.number,
                      call_waiting.numberPresentation,
                      call_waiting.name,
                      call_waiting.signalInfoRecord.isPresent
                    );

      qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_CDMA_CALL_WAITING, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) &call_waiting;
      unsol_resp.resp_len = sizeof( RIL_CDMA_CallWaiting );
      qcril_send_unsol_response( &unsol_resp );

      qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                         QCRIL_DEFAULT_MODEM_ID,
                         QCRIL_DATA_ON_STACK,
                         QCRIL_EVT_QMI_VOICE_GET_WAITING_CALL,
                         &(info_rec_ind->call_id),
                         sizeof(info_rec_ind->call_id),
                         (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
    }
    else
    {
      qcril_qmi_voice_voip_lock_overview();

      if ( info_rec_ind->caller_id_info_valid || info_rec_ind->caller_name_valid || info_rec_ind->calling_party_info_valid || info_rec_ind->clir_cause_valid)
      {
        //  workaround for unknown incoming call number in 3gpp2
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( info_rec_ind->call_id );
        if ( NULL == call_info_entry && info_rec_ind->call_id != 254 )
        { // fresh, this should be an incoming call and the info_rec_ind comes before all_call_status_ind
          // Create a call entry only if call_id is not 254 which is used by modem to indicate a unknown call type.
          call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
                  info_rec_ind->call_id,
                  INVALID_MEDIA_ID,
                  TRUE,
                  QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING );
          call_info_entry->ril_call_state = RIL_CALL_INCOMING;
          call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_RIL_CALL_STATE_VALID;
        }
        else
        { // what we got?
          qcril_qmi_voice_voip_call_info_dump( call_info_entry );
        }

        if ( call_info_entry )
        {
          if ( info_rec_ind->caller_id_info_valid )
          {
            call_info_entry->voice_svc_remote_party_number.call_id = info_rec_ind->call_id;
            call_info_entry->voice_svc_remote_party_number.number_pi = info_rec_ind->caller_id_info.pi;

            QCRIL_LOG_INFO(".. store caller id in call obj" );

            uint32_t number_len = qcril_qmi_voice_call_num_copy_with_toa_check(info_rec_ind->caller_id_info.caller_id,
                                                                               info_rec_ind->caller_id_info.caller_id_len,
                                                                               call_info_entry->voice_svc_remote_party_number.number,
                                                                               QMI_VOICE_NUMBER_MAX_V02,
                                                                               (info_rec_ind->calling_party_info_valid) ? info_rec_ind->calling_party_info.num_type : QMI_VOICE_NUM_TYPE_UNKNOWN_V02);
            call_info_entry->voice_svc_remote_party_number.number_len = number_len;

            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;

            qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( call_info_entry );

            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING )
            {
              call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING;
              need_reset_1x_num_wait_tmr = TRUE;
            }

          }
          else if ( info_rec_ind->calling_party_info_valid )
          {
            call_info_entry->voice_svc_remote_party_number.call_id = info_rec_ind->call_id;
            call_info_entry->voice_svc_remote_party_number.number_pi = info_rec_ind->calling_party_info.pi;

            QCRIL_LOG_INFO(".. store calling_party_info in call obj" );

            uint32_t number_len = qcril_qmi_voice_call_num_copy_with_toa_check(
                                  info_rec_ind->calling_party_info.num,
                                  info_rec_ind->calling_party_info.num_len,
                                  call_info_entry->voice_svc_remote_party_number.number,
                                  QMI_VOICE_NUMBER_MAX_V02,
                                  info_rec_ind->calling_party_info.num_type);
            call_info_entry->voice_svc_remote_party_number.number_len = number_len;

            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;

            qcril_qmi_voice_consider_shadow_remote_number_cpy_creation (call_info_entry );

            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING )
            {
              call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING;
              need_reset_1x_num_wait_tmr = TRUE;
            }

          }

          if ( info_rec_ind->caller_name_valid && !ignore_caller_name)
          {
            QCRIL_LOG_INFO(".. store caller name in call obj" );

            call_info_entry->voice_svc_remote_party_name.call_id = info_rec_ind->call_id;
            call_info_entry->voice_svc_remote_party_name.name_pi = 0x00; // set to Allowed Presentation
            // convert ascii to utf8
            call_info_entry->voice_svc_remote_party_name.name_len = qcril_cm_ss_ascii_to_utf8((unsigned char*) info_rec_ind->caller_name,
                                                                                              strlen(info_rec_ind->caller_name),
                                                                                              call_info_entry->voice_svc_remote_party_name.name,
                                                                                              sizeof(call_info_entry->voice_svc_remote_party_name.name));
            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NAME_VALID;
          }

          if ( info_rec_ind->clir_cause_valid )
          {
            if((!info_rec_ind->calling_party_info_valid) && (info_rec_ind->clir_cause == 0))
            {
              call_info_entry->voice_svc_remote_party_number.number_pi = PRESENTATION_NUM_NUM_UNAVAILABLE_V02; // default value
            }
            else
            {
              switch((uint8)info_rec_ind->clir_cause)
              {
                case QMI_VOICE_CLIR_CAUSE_NO_CAUSE_V02:
                  call_info_entry->voice_svc_remote_party_number.number_pi = PRESENTATION_NUM_ALLOWED_V02;
                  break;
                case QMI_VOICE_CLIR_CAUSE_REJECTED_BY_USER_V02:
                  call_info_entry->voice_svc_remote_party_number.number_pi = PRESENTATION_NUM_RESTRICTED_V02;
                  break;
                case QMI_VOICE_CLIR_CAUSE_COIN_LINE_V02:
                  call_info_entry->voice_svc_remote_party_number.number_pi = PRESENTATION_NUM_PAYPHONE_V02;
                  break;
                default:
                  call_info_entry->voice_svc_remote_party_number.number_pi = PRESENTATION_NUM_NUM_UNAVAILABLE_V02;
                  break;
               }
              QCRIL_LOG_INFO("Mapped Clir=%d, PI=%d",info_rec_ind->clir_cause,call_info_entry->voice_svc_remote_party_number.number_pi );
            }
            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;

            qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( call_info_entry );

            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING )
            {
              call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING;
              need_reset_1x_num_wait_tmr = TRUE;
            }

            QCRIL_LOG_INFO( "clir_cause_valid voice_svc_remote_party_number.number_pi = %d",
                            call_info_entry->voice_svc_remote_party_number.number_pi);
          }

          qcril_qmi_voice_voip_call_info_dump( call_info_entry );
        }
        else
        {
          QCRIL_LOG_ERROR("call_info_entry is NULL");
        }
      }
      else
      {
        QCRIL_LOG_INFO("both caller_id_info and caller_name are not valid");
      }

      qcril_qmi_voice_voip_unlock_overview();
    }

    QCRIL_LOG_INFO(".. signal_info_valid %d", (int)info_rec_ind->signal_info_valid );
    if( info_rec_ind->signal_info_valid )
    {
      QCRIL_LOG_INFO("signal_info signal_type: %d, alert_pitch: %d, signal: %d", info_rec_ind->signal_info.signal_type,
                     info_rec_ind->signal_info.alert_pitch, info_rec_ind->signal_info.signal);

      qcril_qmi_voice_voip_lock_overview();

      if ( call_is_in_call_waiting_state )
      { // there must be call waiting obj already
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state( RIL_CALL_WAITING );
      }
      else
      { // expecting call obj in coming state
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state( RIL_CALL_INCOMING );
      }
      QCRIL_LOG_INFO(".. call obj %p", call_info_entry );
      if ( call_info_entry )
      {
        qcril_qmi_voice_voip_call_info_dump( call_info_entry );
        call_info_entry->elaboration |= ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING | QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_RING_DONE );

        QCRIL_LOG_INFO(".. launch ringer" );
        info_rec_pass_over = qcril_malloc( sizeof( *info_rec_pass_over ) );
        if ( info_rec_pass_over )
        {
          *info_rec_pass_over = signal_info_rec;

          qcril_setup_timed_callback_ex_params_adv( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                qcril_qmi_voice_make_incoming_call_ring,
                                                info_rec_pass_over,
                                                TRUE,
                                                NULL,   // immediate
                                                NULL );
        }
        else
        { // rollback
          call_info_entry->elaboration &= ~( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NEED_FOR_RING_PENDING | QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_RING_DONE );
        }
      }
      else
      { // neither a waiting call nor an incoming call
        info_rec_ptr = &info_recs.infoRec[idx];
        info_rec_ptr->name = RIL_CDMA_SIGNAL_INFO_REC;
        info_rec_ptr->rec.signal.isPresent = TRUE;
        info_rec_ptr->rec.signal.signalType = (char)info_rec_ind->signal_info.signal_type;
        info_rec_ptr->rec.signal.alertPitch = (char)info_rec_ind->signal_info.alert_pitch;
        info_rec_ptr->rec.signal.signal = (char)info_rec_ind->signal_info.signal;
        idx++;
      }
      qcril_qmi_voice_voip_unlock_overview();
    }

    info_recs.numberOfInfoRecs = idx;
    QCRIL_LOG_INFO( ".. number of info recs to be sent in event is %d", info_recs.numberOfInfoRecs);
    for ( iter_idx = 0; iter_idx < info_recs.numberOfInfoRecs; iter_idx++ )
    {
      QCRIL_LOG_INFO( ".. .. rec# %d, name %d ", iter_idx, (int)info_recs.infoRec[ iter_idx ].name );
    }

    if (number_changed || name_changed)
    {
      qcril_qmi_voice_send_unsol_call_state_changed( QCRIL_DEFAULT_INSTANCE_ID );
    }

    if ( idx > 0 )
    {
      /* If there are any info recs remaining after extracting the name, number
         and signal info, send them to the UI. */
      qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_CDMA_INFO_REC, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) &info_recs;
      unsol_resp.resp_len = sizeof( RIL_CDMA_InformationRecords );
      qcril_send_unsol_response( &unsol_resp );
    }

    if ( need_reset_1x_num_wait_tmr )
    {
      qcril_qmi_voice_voip_lock_overview();
      if ( QMI_RIL_ZERO != qmi_voice_voip_overview.num_1x_wait_timer_id )
      {
        qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.num_1x_wait_timer_id );
        qmi_voice_voip_overview.num_1x_wait_timer_id = QMI_RIL_ZERO;
      }
      qcril_qmi_voice_voip_unlock_overview();
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_info_rec_ind_hdlr
/*=========================================================================
  FUNCTION:  qcril_qmi_voice_dial_call_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle DIAL_CALL_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_dial_call_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_dial_call_resp_msg_v02    *dial_call_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  boolean destroy_call_info_entry = FALSE;

  RIL_LastCallFailCause last_call_fail_cause;
  qcril_qmi_voice_stk_cc_modification_e_type stk_cc_modification;

  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type oversight_exec_event;


  qcril_qmi_voice_voip_lock_overview();

  call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP, TRUE );

  QCRIL_LOG_INFO( "call_info_entry %p", call_info_entry );

  if( NULL != params_ptr->data && NULL != call_info_entry )
  {
    dial_call_resp = (voice_dial_call_resp_msg_v02    *)params_ptr->data;
    qmi_result = dial_call_resp->resp.result;
    qmi_error = dial_call_resp->resp.error;

    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex( QMI_NO_ERR,
                                                                        &dial_call_resp->resp,
                                                                        QCRIL_QMI_ERR_CTX_DIAL_TXN,
                                                                        dial_call_resp );


    QCRIL_LOG_INFO("DIAL CALL RESP : ril_err=%d, dial_call_resp_result=%d, dial_call_resp_error=%d, cc_sups_result_valid=%d, cc_result_type_valid=%d, cc_result_type=%d ",
                   (int)ril_err,
                   (int)dial_call_resp->resp.result,
                   (int)dial_call_resp->resp.error,
                   (int)dial_call_resp->cc_sups_result_valid,
                   (int)dial_call_resp->cc_result_type_valid,
                   (int)dial_call_resp->cc_result_type
                   );

    switch ( (int)ril_err )
    {
      case RIL_E_SUCCESS:
      case RIL_E_DIAL_MODIFIED_TO_DIAL:
        call_info_entry->qmi_call_id = dial_call_resp->call_id;

        if(dial_call_resp->media_id_valid)
        {
          call_info_entry->media_id = dial_call_resp->media_id;
        }
        else
        {
          call_info_entry->media_id = INVALID_MEDIA_ID;
        }
        QCRIL_LOG_INFO( "call qmi id recorded %d", (int)call_info_entry->qmi_call_id );
        break;

      default:
        qcril_qmi_voice_respond_pending_hangup_ril_response(call_info_entry->qmi_call_id);
        destroy_call_info_entry = TRUE;
        break;
    }

    if (!qmi_ril_is_feature_supported(QMI_RIL_FEATURE_ATEL_STKCC))
    {
      if (RIL_E_DIAL_MODIFIED_TO_DIAL == ril_err)
      {
        ril_err = RIL_E_SUCCESS;
      }
    }

    if( RIL_E_SUCCESS != ril_err && dial_call_resp->cc_sups_result_valid )
    {
      QCRIL_LOG_INFO("Error Details : cc_sups_result_reason=%d, cc_sups_result_service=%d",
                      dial_call_resp->cc_sups_result.reason,
                      dial_call_resp->cc_sups_result.service_type);

    }
    switch ( (int)ril_err )
    {
      case RIL_E_DIAL_MODIFIED_TO_DIAL:
        last_call_fail_cause = CALL_FAIL_DIAL_MODIFIED_TO_DIAL;
        stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_DIAL_TO_DIAL;
        break;

      case RIL_E_DIAL_MODIFIED_TO_USSD:
        last_call_fail_cause = CALL_FAIL_DIAL_MODIFIED_TO_USSD;
        stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_DIAL_TO_USSD;
        break;

      case RIL_E_DIAL_MODIFIED_TO_SS:
        last_call_fail_cause = CALL_FAIL_DIAL_MODIFIED_TO_SS;
        stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_DIAL_TO_SS;
        break;

      default:
        last_call_fail_cause = CALL_FAIL_ERROR_UNSPECIFIED;
        stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE;
        break;
    }

    if ( CALL_FAIL_ERROR_UNSPECIFIED != last_call_fail_cause )
    { // STK CC session started
      qcril_qmi_voice_reset_stk_cc();

      stk_cc_info.modification                      = stk_cc_modification;
      stk_cc_info.is_alpha_relayed                  = FALSE;

      if ( dial_call_resp->call_id_valid )
      {
        stk_cc_info.call_id_info      = dial_call_resp->call_id;
      }

      if ( dial_call_resp->cc_sups_result_valid )
      {
        stk_cc_info.ss_ussd_info = dial_call_resp->cc_sups_result;
      }

      if ( dial_call_resp->alpha_ident_valid )
      {
        if ( ALPHA_DCS_UCS2_V02 == dial_call_resp->alpha_ident.alpha_dcs )
        {
          qcril_qmi_voice_transfer_sim_ucs2_alpha_to_std_ucs2_alpha(&dial_call_resp->alpha_ident, &stk_cc_info.alpha_ident);
        }
        else
        {
          stk_cc_info.alpha_ident = dial_call_resp->alpha_ident;
        }
      }
      else
      {
        memset( &stk_cc_info.alpha_ident, 0, sizeof( stk_cc_info.alpha_ident ) );
      }
    }


    QCRIL_LOG_INFO( "DIAL CALL RESP COMPLETE received with result %d for call id %d", (int)ril_err, (int)dial_call_resp->call_id );

    qcril_qmi_voice_stk_cc_dump();
  }
  else
  {  // fault
    if ( NULL != call_info_entry )
    {
      qcril_qmi_voice_voip_destroy_call_info_entry( call_info_entry );
    }
  }

  if ( RIL_E_FDN_CHECK_FAILURE == ril_err )
  {
    qcril_qmi_voice_handle_new_last_call_failure_cause(CALL_FAIL_FDN_BLOCKED, FALSE, call_info_entry);
  }
  else
  {
    qcril_qmi_voice_handle_new_last_call_failure_cause(last_call_fail_cause, FALSE, call_info_entry);
  }

  memset( &oversight_exec_event, 0, sizeof( oversight_exec_event ) );
  oversight_exec_event.locator.elaboration_pattern = QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP;

  if ( RIL_E_SUCCESS != ril_err &&
          RIL_E_DIAL_MODIFIED_TO_DIAL != ril_err )
  {
    if (call_info_entry &&
        (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING))
    {
        call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING;
    }
    qcril_send_empty_payload_request_response(instance_id, params_ptr->t, params_ptr->event_id, ril_err);
    if (call_info_entry && qcril_qmi_voice_call_to_ims(call_info_entry))
    {
        call_info_entry->voice_scv_info.call_state = CALL_STATE_END_V02;
        qcril_qmi_voice_send_ims_unsol_call_state_changed();
    }
    qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_ABANDON,
                                                      QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN,
                                                      &oversight_exec_event );
  }
  else
  {
    if (RIL_E_DIAL_MODIFIED_TO_DIAL == ril_err)
    {
      oversight_exec_event.successful_response_payload = ril_err;
      oversight_exec_event.successful_response_payload_len = sizeof(ril_err);
    }
    qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_SUCCESS,
                                                      QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN,
                                                      &oversight_exec_event );
  }

  if (destroy_call_info_entry)
  {
    qcril_qmi_voice_voip_destroy_call_info_entry( call_info_entry );
  }

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_voice_dial_call_resp_hdlr */

//===========================================================================
// qcril_qmi_voice_answer_call_resp_hdlr
//===========================================================================
void qcril_qmi_voice_answer_call_resp_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qmi_client_error_type                  transp_err
)
{
  voice_answer_call_resp_msg_v02  *ans_call_resp;
  RIL_Errno ril_err;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;

  qcril_qmi_voice_voip_call_info_entry_type*  call_info_entry = NULL;
  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type oversight_event_params;
  qmi_ril_voice_ims_command_exec_oversight_type *command_oversight;
  int covered_by_oversight_handling;

  QCRIL_LOG_FUNC_ENTRY();

  ans_call_resp = (voice_answer_call_resp_msg_v02*) params_ptr->data;
  if ( NULL != ans_call_resp )
  {
    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( transp_err, &ans_call_resp->resp );
    QCRIL_LOG_INFO( ".. transp err %d, resp err %d, ril err %d", (int)transp_err, (int)ans_call_resp->resp.error, (int)ril_err );
    QCRIL_LOG_INFO( ".. call id valid %d, call id %d", (int)ans_call_resp->call_id_valid, (int)ans_call_resp->call_id );

    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE );
    QCRIL_LOG_INFO( ".. call info entry %p", call_info_entry );

    qmi_result = ans_call_resp->resp.result;
    qmi_error = ans_call_resp->resp.error;
    covered_by_oversight_handling = FALSE;
    qcril_qmi_voice_voip_lock_overview();
    command_oversight = qmi_ril_voice_ims_find_command_oversight_by_token (params_ptr->t);
    if (NULL != command_oversight)
    {
        memset (&oversight_event_params, 0, sizeof (oversight_event_params));
        oversight_event_params.locator.command_oversight = command_oversight;
        covered_by_oversight_handling = qmi_ril_voice_ims_command_oversight_handle_event
                                        (
                                           (QMI_RESULT_SUCCESS_V01 == qmi_result && QMI_ERR_NONE_V01 == qmi_error)?
                                           QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_SUCCESS :
                                           QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_FAILURE,
                                           QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                           &oversight_event_params
                                        );
    } // if (NULL != command_oversight)
    qcril_qmi_voice_voip_unlock_overview();

    if (( NULL == command_oversight) || (!covered_by_oversight_handling))
    {
        if ( NULL != call_info_entry )
        {
            call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ANSWERING_CALL;
            if (RIL_E_SUCCESS == ril_err)
            {
               call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING;
            }
        }

        if (RIL_E_SUCCESS != ril_err)
        {
            if (QCRIL_EVT_IMS_SOCKET_REQ_ANSWER == params_ptr->event_id)
            {
                qcril_am_event_type(QCRIL_AM_EVENT_IMS_ANSWER_FAIL);
            }
            else
            {
                qcril_am_event_type(QCRIL_AM_EVENT_VOICE_ANSWER_FAIL);
            }
        }

        qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_answer_call_resp_hdlr

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_end_call_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle END_CALL_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_end_call_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_end_call_resp_msg_v02  *end_call_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err = RIL_E_SUCCESS;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_LOG_FUNC_ENTRY();

  if( params_ptr->data != NULL )
  {
    end_call_resp = (voice_end_call_resp_msg_v02 *)params_ptr->data;
    qmi_result = end_call_resp->resp.result;
    qmi_error = end_call_resp->resp.error;

    if(qmi_result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_INFO("END CALL RESP SUCCESS received with call id %d",end_call_resp->call_id);
      qcril_qmi_voice_voip_lock_overview();
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( end_call_resp->call_id );
      if ( NULL != call_info_entry && CALL_STATE_END_V02 != call_info_entry->voice_scv_info.call_state)
      {
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_HANGUP_RESP;
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER;
        call_info_entry->pending_end_call_req_tid = params_ptr->t;
      }
      else
      {
        QCRIL_LOG_DEBUG("Didn't have call id %d in the call entry list", end_call_resp->call_id);
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
      }
      qcril_qmi_voice_voip_unlock_overview();
    }
    else
    {
      QCRIL_LOG_INFO("END CALL RESP FAILURE received with error %d",qmi_error);
      ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, ril_err );
    }
  }
  else
  {
    ril_err = RIL_E_GENERIC_FAILURE;
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, ril_err );
  }


} /* qcril_qmi_voice_end_call_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_burst_dtmf_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle BURST_DTMF_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_burst_dtmf_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_burst_dtmf_resp_msg_v02  *burst_dtmf_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {
  burst_dtmf_resp = (voice_burst_dtmf_resp_msg_v02 *)params_ptr->data;
  qmi_result = burst_dtmf_resp->resp.result;
  qmi_error = burst_dtmf_resp->resp.error;

  if(qmi_result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("BURST DTMF RESP SUCCESS received with call id %d", burst_dtmf_resp->call_id);
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("BURST DTMF RESP FAILURE received with error %d",qmi_error);
    ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
    /* Send FAILURE response */
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_err, &resp );
    qcril_send_request_response( &resp );
  }
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
} /* qcril_qmi_voice_burst_dtmf_resp_hdlr */

static void qcril_qmi_voice_ims_req_dtmf_stop_tmr_handler (qcril_timed_callback_handler_params_type *handler_params)
{
  qcril_instance_id_e_type instance_id;
  RIL_Token t;

  QCRIL_LOG_FUNC_ENTRY();

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  t = (RIL_Token)handler_params->custom_param;

  qcril_event_queue( instance_id, QCRIL_DEFAULT_MODEM_ID,
                     QCRIL_DATA_ON_STACK, QCRIL_EVT_IMS_SOCKET_REQ_STOP_CONT_DTMF, NULL,
                     VOICE_NIL, t );

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_nas_ims_req_dtmf_stop_tmr_handler

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_start_cont_dtmf_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle START_CONT_DTMF_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_start_cont_dtmf_resp_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_evt_e_type pending_event
)
{
  voice_start_cont_dtmf_resp_msg_v02  *start_cont_dtmf_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  struct timeval  tmr_delay_ims_req_dtmf_stop;

  tmr_delay_ims_req_dtmf_stop.tv_sec = QMI_RIL_ZERO;
  tmr_delay_ims_req_dtmf_stop.tv_usec = dtmf_rtp_event_interval * 1000; /* default: 85ms * 1000 */


  qcril_qmi_voice_info_lock();
  qcril_qmi_voice_info.pending_dtmf_req_id = QMI_RIL_ZERO;
  qcril_qmi_voice_info_unlock();

  if( params_ptr->data != NULL )
  {
    start_cont_dtmf_resp = (voice_start_cont_dtmf_resp_msg_v02 *)params_ptr->data;
    qmi_result = start_cont_dtmf_resp->resp.result;
    qmi_error = start_cont_dtmf_resp->resp.error;

    if(qmi_result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_INFO("START CONT DTMF RESP SUCCESS received with call id %d", start_cont_dtmf_resp->call_id);
      if(pending_event == QCRIL_EVT_QMI_VOICE_BURST_START_CONT_DTMF)
      {
        qcril_reqlist_free( params_ptr->instance_id, params_ptr->t );
        QCRIL_LOG_INFO("Queueing up STOP_CONT_DTMF request for completing simulation of BURST DTMF");
        if( RIL_REQUEST_DTMF == params_ptr->event_id )
        {
          qcril_event_queue( instance_id, QCRIL_DEFAULT_MODEM_ID,
                             QCRIL_DATA_ON_STACK, QCRIL_EVT_QMI_VOICE_BURST_STOP_CONT_DTMF, NULL,
                             VOICE_NIL, params_ptr->t );
        }
        else
        {
          qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                qcril_qmi_voice_ims_req_dtmf_stop_tmr_handler,
                                                params_ptr->t,
                                                &tmr_delay_ims_req_dtmf_stop,
                                                NULL );
        }
      }
      else
      {
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
      }
    }
    else
    {
      QCRIL_LOG_INFO("START CONT DTMF RESP FAILURE received with error %d",qmi_error);
      ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
      /* Send FAILURE response */
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, ril_err );
    }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }
} /* qcril_qmi_voice_start_cont_dtmf_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_stop_cont_dtmf_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle STOP_CONT_DTMF_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_stop_cont_dtmf_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_stop_cont_dtmf_resp_msg_v02  *stop_cont_dtmf_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  qcril_qmi_voice_info_lock();
  qcril_qmi_voice_info.pending_dtmf_req_id = QMI_RIL_ZERO;
  qcril_qmi_voice_info_unlock();

  if( params_ptr->data != NULL )
  {
    stop_cont_dtmf_resp = (voice_stop_cont_dtmf_resp_msg_v02 *)params_ptr->data;
    qmi_result = stop_cont_dtmf_resp->resp.result;
    qmi_error = stop_cont_dtmf_resp->resp.error;

    if(qmi_result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_INFO("STOP CONT DTMF RESP SUCCESS received with call id %d", stop_cont_dtmf_resp->call_id);
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
    }
    else
    {
      QCRIL_LOG_INFO("STOP CONT DTMF RESP FAILURE received with error %d",qmi_error);
      ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
      /* Send FAILURE response */
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, ril_err );
    }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }
} /* qcril_qmi_voice_stop_cont_dtmf_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_send_flash_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle SEND_FLASH_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_send_flash_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_send_flash_resp_msg_v02      *send_flash_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {
  send_flash_resp = (voice_send_flash_resp_msg_v02 *)params_ptr->data;
  qmi_result = send_flash_resp->resp.result;
  qmi_error = send_flash_resp->resp.error;

  if(qmi_result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("SEND FLASH RESP SUCCESS received with call id %d", send_flash_resp->call_id);
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("SEND FLASH RESP FAILURE received with error %d",qmi_error);
    ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
    /* Send FAILURE response */
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_err, &resp );
    qcril_send_request_response( &resp );
  }
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
} /* qcril_qmi_voice_send_flash_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_set_supp_svc_notification_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Set Supps Notification Request.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_set_supp_svc_notification_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_indication_register_resp_msg_v02      *set_supp_svc_notification_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  if( params_ptr->data != NULL )
  {
    set_supp_svc_notification_resp = (voice_indication_register_resp_msg_v02 *)params_ptr->data;
    qmi_result = set_supp_svc_notification_resp->resp.result;
    qmi_error = set_supp_svc_notification_resp->resp.error;

    if(qmi_result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_INFO("Set Supps SVC notification RESP: SUCCESS");
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
    }
    else
    {
      QCRIL_LOG_INFO("Set Supps SVC notification RESP:FAILURE received with error %d",qmi_error);
      ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
      /* Send FAILURE response */
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, ril_err );
    }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }
} /* qcril_qmi_voice_set_supp_svc_notification_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_ussd_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handle USSD Indications.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_ussd_ind_hdlr
(
void *ind_data_ptr,
uint32 ind_data_len
)
{
  qcril_instance_id_e_type instance_id=QCRIL_DEFAULT_INSTANCE_ID;
  char ussd_utf8_str[QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2];
  char *response[ 2 ];
  char type_code[ 2 ];
  int utf8_len = 0;
  uint8 uss_dcs=QCRIL_QMI_VOICE_USSD_DCS_UNSPECIFIED;
  qcril_unsol_resp_params_type unsol_resp;
  voice_ussd_ind_msg_v02  *ussd_ind;
  further_user_action_enum_v02 notification_type;
  boolean success=TRUE;
  int i=0;

  QCRIL_LOG_FUNC_ENTRY();

  if( ind_data_ptr != NULL )
  {
    ussd_ind = (voice_ussd_ind_msg_v02*)ind_data_ptr;
    ind_data_len = ind_data_len;
    notification_type = ussd_ind->notification_type;
    memset( ussd_utf8_str, '\0', sizeof( ussd_utf8_str ) );

    QCRIL_LOG_INFO ("ind_data_ptr is not NULL");

    switch(notification_type)
    {
      case FURTHER_USER_ACTION_REQUIRED_V02:
        qcril_qmi_voice_info.ussd_user_action_required = TRUE;
        break;
      case FURTHER_USER_ACTION_NOT_REQUIRED_V02 :
        qcril_qmi_voice_info.ussd_user_action_required = FALSE;
        break;
      default :
        break;
    }

    if( TRUE == ussd_ind->uss_info_valid || TRUE == ussd_ind->uss_info_utf16_valid )
    {
      if ( TRUE == ussd_ind->uss_info_utf16_valid ) // using uss_info_utf16 instead of uss_info if it is available
      {
        utf8_len = qcril_cm_ss_convert_ucs2_to_utf8( (char *) ussd_ind->uss_info_utf16, ussd_ind->uss_info_utf16_len * 2, ussd_utf8_str );

        for(i=0 ; i< utf8_len ; i++ )
        {
          QCRIL_LOG_DEBUG ("utf8 data bytes : %x ", ussd_utf8_str[ i ]);
        }
        if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
        {
          QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR");
          utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
          ussd_utf8_str[ utf8_len - 1] = '\0';
        }
      }
      else
      {
        switch(ussd_ind->uss_info.uss_dcs)
        {
          case USS_DCS_ASCII_V02 :
            if( ussd_ind->uss_info.uss_data_len < QMI_VOICE_USS_DATA_MAX_V02 )
            {
              qcril_cm_ss_ascii_to_utf8((unsigned char *)ussd_ind->uss_info.uss_data, ussd_ind->uss_info.uss_data_len,
                                      ussd_utf8_str, sizeof(ussd_utf8_str));
            }
            break;
          case USS_DCS_8BIT_V02 :
            uss_dcs = QCRIL_QMI_VOICE_USSD_DCS_8_BIT;
            utf8_len = qcril_cm_ss_convert_ussd_string_to_utf8( uss_dcs,
                                                                ussd_ind->uss_info.uss_data_len,
                                                                ussd_ind->uss_info.uss_data,
                                                                ussd_utf8_str );
            if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
            {
              QCRIL_LOG_ERROR ( "ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR" );
              utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
              ussd_utf8_str[ utf8_len - 1] = '\0';
            }
            break;
          case USS_DCS_UCS2_V02 :
            uss_dcs = QCRIL_QMI_VOICE_USSD_DCS_UCS2;
            utf8_len = qcril_cm_ss_convert_ussd_string_to_utf8( uss_dcs,
                                                                ussd_ind->uss_info.uss_data_len,
                                                                ussd_ind->uss_info.uss_data,
                                                                ussd_utf8_str );
            if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
            {
              QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR");
              utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
              ussd_utf8_str[ utf8_len - 1] = '\0';
            }
            break;
          default :
            QCRIL_LOG_ERROR ("Invalid USSD dcs : %d", ussd_ind->uss_info.uss_dcs );
            success = FALSE;
            break;
        }
      }

      if(success == TRUE)
      {
        if(notification_type == FURTHER_USER_ACTION_REQUIRED_V02)
        {
          type_code[ 0 ] = '1';   /* QCRIL_CM_SS_CUSD_RESULT_MORE */
          type_code[ 1 ] = '\0';
        }
        else
        {
          type_code[ 0 ] = '0'; /* QCRIL_CM_SS_CUSD_RESULT_MORE */
          type_code[ 1 ] = '\0';
        }
        response[ 0 ] = type_code;
        response[ 1 ] = ussd_utf8_str;

        /* sending the response received from the network for the USSD request */
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_ON_USSD, &unsol_resp );
        unsol_resp.resp_pkt = ( void * ) &response;
        unsol_resp.resp_len = sizeof( response );
        unsol_resp.logstr = NULL;
        qcril_send_unsol_response( &unsol_resp );
      }

    }
    else
    {
      QCRIL_LOG_ERROR("Received USSD Indication with no USSD string");
    }
  }

  QCRIL_LOG_FUNC_RETURN();

}/*qcril_qmi_voice_ussd_ind_hdlr*/

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_ussd_release_ind_hdlr

===========================================================================*/
/*!
    @brief
    Handles USSD Release indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_ussd_release_ind_hdlr
(
void *ind_data_ptr,
uint32 ind_data_len
)
{
  qcril_reqlist_public_type req_info;
  qcril_request_resp_params_type resp;
  char type_code[2];
  char *response_buff[2];
  qcril_unsol_resp_params_type unsol_resp;

  ind_data_ptr = ind_data_ptr;
  ind_data_len = ind_data_len;
  QCRIL_LOG_FUNC_ENTRY();
  if ( qcril_reqlist_query_by_request( QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_SEND_USSD, &req_info ) == E_SUCCESS )
  {
    QCRIL_LOG_INFO("cleaning the uss_cnf after receiving release_uss_ind");
    /* send RIL_E_GENERIC_FAILURE response */
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

  qcril_qmi_voice_info.ussd_user_action_required = FALSE;

  memset( type_code, '\0', sizeof( type_code ) );
  memset( response_buff, 0, sizeof( response_buff ) );
  memset( &unsol_resp, 0 , sizeof(unsol_resp) );

  QCRIL_LOG_DEBUG ("USSD Release triggered, Sending ABORT in case if any pending transaction exists");
  type_code[ 0 ] = '0';  /* QCRIL_CM_SS_CUSD_RESULT_DONE */
  type_code[ 1 ] = '\0';
  response_buff[ 0 ] = type_code;
  response_buff[ 1 ] = NULL;
  qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_ON_USSD, &unsol_resp );
  unsol_resp.resp_pkt = (void *) response_buff;
  unsol_resp.resp_len = sizeof( response_buff );
  unsol_resp.logstr = NULL;
  qcril_send_unsol_response( &unsol_resp );

}/*qcril_qmi_voice_ussd_release_ind_hdlr*/

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_command_cb

===========================================================================*/
/*!
    @brief
    Common Callback for all the QMI voice commands.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_command_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  qmi_resp_callback_type qmi_resp_callback;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_INFO(".. msg id %d", (int) msg_id );

  qmi_resp_callback.user_handle = user_handle;
  qmi_resp_callback.msg_id = msg_id;
  qmi_resp_callback.data_buf = (void*) resp_c_struct;
  qmi_resp_callback.data_buf_len = resp_c_struct_len;
  qmi_resp_callback.cb_data = resp_cb_data;
  qmi_resp_callback.transp_err = transp_err;

  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                 QCRIL_DEFAULT_MODEM_ID,
                 QCRIL_DATA_ON_STACK,
                 QCRIL_EVT_QMI_VOICE_HANDLE_COMM_CALLBACKS,
                 (void*) &qmi_resp_callback,
                 sizeof(qmi_resp_callback),
                 (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_voice_command_cb */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_command_cb_helper

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_VOICE_HANDLE_COMM_CALLBACKS

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_command_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  uint16 req_id;
  uint16 dtmf_req_id;
  qcril_reqlist_public_type req_info;
  qcril_request_params_type req_data;
  qmi_resp_callback_type * qmi_resp_callback;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  qmi_resp_callback = (qmi_resp_callback_type *) params_ptr->data;
  if( qmi_resp_callback )
  {
    /*-----------------------------------------------------------------------*/
    QCRIL_ASSERT( qmi_resp_callback->data_buf != NULL );
    user_data = ( uint32 )(uintptr_t) qmi_resp_callback->cb_data;
    instance_id = QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
    req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data );
    req_data.modem_id = QCRIL_DEFAULT_MODEM_ID;
    req_data.instance_id = instance_id;
    req_data.datalen = qmi_resp_callback->data_buf_len;
    req_data.data = qmi_resp_callback->data_buf;
    /*-----------------------------------------------------------------------*/

    QCRIL_LOG_INFO("msg_id %.2x (%s)", qmi_resp_callback->msg_id, qcril_qmi_voice_lookup_command_name(qmi_resp_callback->msg_id));

    // DTMF add-on
    if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) != E_SUCCESS )
    {
      switch ( qmi_resp_callback->msg_id )
      {
        case QMI_VOICE_START_CONT_DTMF_RESP_V02:  // fall through
        case QMI_VOICE_STOP_CONT_DTMF_RESP_V02:
          qcril_qmi_voice_info_lock();
          dtmf_req_id = qcril_qmi_voice_info.pending_dtmf_req_id;
          qcril_qmi_voice_info_unlock();
          if ( qcril_reqlist_query_by_req_id( dtmf_req_id, &instance_id, &req_info ) == E_SUCCESS )
          {
            req_id = dtmf_req_id;
            QCRIL_LOG_INFO("dtmf req id overruled %d", req_id );
          }
          break;

        default: // no action
          break;
      }
    }

    /* Lookup the Token ID */
    if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
    {
      if( qmi_resp_callback->transp_err != QMI_NO_ERR )
      {
        QCRIL_LOG_INFO("Transp error (%d) recieved from QMI for RIL request %d", qmi_resp_callback->transp_err, req_info.request);
        /* Send GENERIC_FAILURE response */
        qcril_send_empty_payload_request_response( instance_id, req_info.t, req_info.request, RIL_E_GENERIC_FAILURE );
      }
      else
      {
        req_data.t = req_info.t;
        req_data.event_id = req_info.request;
        switch(qmi_resp_callback->msg_id)
        {
        case QMI_VOICE_DIAL_CALL_RESP_V02:
          qcril_qmi_voice_dial_call_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_ANSWER_CALL_RESP_V02:
          qcril_qmi_voice_answer_call_resp_hdlr( &req_data , qmi_resp_callback->transp_err );
          break;

        case QMI_VOICE_END_CALL_RESP_V02:
          qcril_qmi_voice_end_call_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_BURST_DTMF_RESP_V02:
          qcril_qmi_voice_burst_dtmf_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_START_CONT_DTMF_RESP_V02:
          qcril_qmi_voice_start_cont_dtmf_resp_hdlr( &req_data, req_info.pending_event_id[ QCRIL_DEFAULT_MODEM_ID ]);
          break;

        case QMI_VOICE_STOP_CONT_DTMF_RESP_V02:
          qcril_qmi_voice_stop_cont_dtmf_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_SEND_FLASH_RESP_V02:
          qcril_qmi_voice_send_flash_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_INDICATION_REGISTER_RESP_V02:
          qcril_qmi_voice_set_supp_svc_notification_resp_hdlr( &req_data );
          break;

        case QMI_VOICE_MANAGE_CALLS_RESP_V02:
          qcril_qmi_voice_sups_cmd_mng_calls_resp_hdlr( &req_data );
          break;

        case QMI_VOICE_SET_SUPS_SERVICE_RSEP_V02:
          qcril_qmi_voice_set_sups_service_resp_hdlr( &req_data );
          break;

        case QMI_VOICE_GET_CLIR_RESP_V02:
          qcril_qmi_voice_get_clir_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_GET_CLIP_RESP_V02:
          qcril_qmi_voice_get_clip_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_GET_COLP_RESP_V02:
          qcril_qmi_voice_get_colp_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_SET_CALL_BARRING_PASSWORD_RESP_V02:
          qcril_qmi_voice_change_call_barring_password_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_GET_CALL_WAITING_RESP_V02:
          qcril_qmi_voice_query_call_waiting_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_GET_CALL_BARRING_RESP_V02:
          qcril_qmi_voice_query_facility_lock_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_GET_CALL_FORWARDING_RESP_V02:
          qcril_qmi_voice_query_call_forward_status_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_ORIG_USSD_RESP_V02:
          qcril_qmi_voice_orig_ussd_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_ANSWER_USSD_RESP_V02:
          qcril_qmi_voice_answer_ussd_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_CANCEL_USSD_RESP_V02:
          qcril_qmi_voice_cancel_ussd_resp_hdlr(&req_data);
          break;

        case QMI_VOICE_MANAGE_IP_CALLS_RESP_V02:
          qcril_qmi_voice_voip_manage_ip_calls_resp_hdlr( &req_data );
          break;

        case QMI_VOICE_SETUP_ANSWER_RESP_V02:
          qcril_qmi_voice_setup_answer_resp_hdlr( &req_data );
          break;

        case QMI_VOICE_GET_COLR_RESP_V02:
          qcril_qmi_voice_get_colr_resp_hdlr( &req_data );
          break;

        default:
            QCRIL_LOG_INFO("Unsupported QMI VOICE message %d", qmi_resp_callback->msg_id);
            break;
        }
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "Req ID: %d not found for qcril_qmi_client_voice_svc_cb", req_id );
    }

    if ( QMI_VOICE_DIAL_CALL_RESP_V02 != qmi_resp_callback->msg_id )
    {
      qcril_free( qmi_resp_callback->data_buf );
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_voice_command_cb_helper */

/*=========================================================================
  HELPER FUNCTION: on_length_enum_to_str
===========================================================================*/
void on_length_enum_to_str(dtmf_onlength_enum_v02 on_enum, char* str, int len) {
        if(len >= 4)
        {
        switch(on_enum)
        {
                case DTMF_ONLENGTH_95MS_V02:
                        strlcpy(str, "95", len);
                        break;
                case DTMF_ONLENGTH_150MS_V02:
                        strlcpy(str, "150", len);
                        break;
                case DTMF_ONLENGTH_200MS_V02:
                        strlcpy(str, "200", len);
                        break;
                case DTMF_ONLENGTH_250MS_V02:
                        strlcpy(str, "250", len);
                        break;
                case DTMF_ONLENGTH_300MS_V02:
                        strlcpy(str, "300", len);
                        break;
                case DTMF_ONLENGTH_350MS_V02:
                        strlcpy(str, "350", len);
                        break;
                case DTMF_ONLENGTH_SMS_V02:
                        strlcpy(str, "SMS", len);
                        break;
                default:
                        break;
        }
        }
}

/*=========================================================================
  HELPER FUNCTION: off_length_enum_to_str
===========================================================================*/
void off_length_enum_to_str(dtmf_offlength_enum_v02 off_enum, char* str, int len) {
        if(len >= 4)
        {
        switch(off_enum)
        {
                case DTMF_OFFLENGTH_60MS_V02:
                        strlcpy(str, "60", len);
                        break;
                case DTMF_OFFLENGTH_100MS_V02:
                        strlcpy(str, "100", len);
                        break;
                case DTMF_OFFLENGTH_150MS_V02:
                        strlcpy(str, "150", len);
                        break;
                case DTMF_OFFLENGTH_200MS_V02:
                        strlcpy(str, "200", len);
                        break;
                default:
                        break;
        }
        }

}
/*=========================================================================
  FUNCTION: qcril_qmi_voice_dtmf_ind_hdlr

    @brief
    Handle QMI_VOICE_DTMF_IND_V02.

    @return
    None.
=========================================================================*/
void qcril_qmi_voice_dtmf_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{

  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_unsol_resp_params_type unsol_resp;

  voice_dtmf_ind_msg_v02* dtmf_ind;
  voice_dtmf_info_type_v02* dtmf_info;

  char payload[QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH];
  char on_length[4];
  char off_length[4];
  int digit_buf_len = 0;

  //initalize all our arrays to zero incase we don't set all bytes
  memset(on_length, 0, sizeof(on_length));
  memset(off_length, 0, sizeof(off_length));
  memset(payload, 0, sizeof(payload));

  if( ind_data_ptr != NULL )
  {
  dtmf_ind = (voice_dtmf_ind_msg_v02*) ind_data_ptr;
  ind_data_len = ind_data_len;

  dtmf_info = &dtmf_ind->dtmf_info;

  QCRIL_LOG_FUNC_ENTRY();

  switch(dtmf_info->dtmf_event)
  {
        case DTMF_EVENT_FWD_BURST_V02:

                if(dtmf_ind->on_length_valid) {
                        on_length_enum_to_str(dtmf_ind->on_length, on_length, sizeof(on_length));
                }
                if(dtmf_ind->off_length_valid) {
                        off_length_enum_to_str(dtmf_ind->off_length, off_length, sizeof(off_length));
                }

                digit_buf_len = (dtmf_info->digit_buffer_len < (QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH - 8)) ?
                        dtmf_info->digit_buffer_len :
                        QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH - 8;

                memcpy(payload, on_length, sizeof(on_length));
                memcpy(payload + sizeof(on_length), off_length, sizeof(off_length));
                memcpy(payload + sizeof(on_length) + sizeof(off_length), (void*) dtmf_info->digit_buffer, digit_buf_len);
                qcril_hook_unsol_response( instance_id, QCRIL_EVT_HOOK_UNSOL_CDMA_BURST_DTMF, payload, sizeof(payload));
                break;

        case DTMF_EVENT_FWD_START_CONT_V02:
                  if(dtmf_info->digit_buffer_len != 0) {
                        qcril_hook_unsol_response(instance_id, QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_START, &dtmf_info->digit_buffer[0], sizeof(dtmf_info->digit_buffer[0]));
                  }
                  break;

        case DTMF_EVENT_FWD_STOP_CONT_V02:
                  if(dtmf_info->digit_buffer_len != 0) {
                        qcril_hook_unsol_response(instance_id, QCRIL_EVT_HOOK_UNSOL_CDMA_CONT_DTMF_STOP, NULL, 0);
                  }
                  break;
        default:
                QCRIL_LOG_INFO ("Got unknown DTMF_EVENT in DTMF indication handler");
  }
}
}

/*=========================================================================
  FUNCTION: qcril_qmi_voice_ext_brst_intl_ind_hdlr

    @brief
    Handle QMI_VOICE_EXT_BRST_INTL_IND_V02.

    @return
    None.
=========================================================================*/
void qcril_qmi_voice_ext_brst_intl_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{

  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  voice_ext_brst_intl_ind_msg_v02* ext_brst_intl_ind;
  int payload[QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH];

  QCRIL_LOG_FUNC_ENTRY();

  if( ind_data_ptr != NULL )
  {
    memset(payload, 0, sizeof(payload));
    ext_brst_intl_ind = (voice_ext_brst_intl_ind_msg_v02*) ind_data_ptr;
    ind_data_len = ind_data_len;

    payload[ 0 ] = ext_brst_intl_ind->ext_burst_data.mcc;
    payload[ 1 ] = ext_brst_intl_ind->ext_burst_data.db_subtype;
    payload[ 2 ] = ext_brst_intl_ind->ext_burst_data.chg_ind;
    payload[ 3 ] = ext_brst_intl_ind->ext_burst_data.sub_unit;
    payload[ 4 ] = ext_brst_intl_ind->ext_burst_data.unit;

    qcril_hook_unsol_response( instance_id, QCRIL_EVT_HOOK_UNSOL_EXTENDED_DBM_INTL, (char *) payload, sizeof(payload) );
  }

  QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_voice_ext_brst_intl_ind_hdlr

//===========================================================================
// qcril_qmi_voice_mark_calls_srvcc_in_progress
//===========================================================================
void qcril_qmi_voice_mark_calls_srvcc_in_progress()
{
    qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;

    qcril_qmi_voice_voip_lock_overview();
    iter = qmi_voice_voip_overview.call_info_root;
    while ( iter != NULL  )
    {
        if ( !(iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN) )
        {
            iter->srvcc_in_progress = TRUE;
        }
        iter = iter->next;
    }
    qcril_qmi_voice_voip_unlock_overview();
} // qcril_qmi_voice_mark_calls_srvcc_in_progress

//===========================================================================
// qcril_qmi_voice_unmark_calls_srvcc_in_progress
//===========================================================================
void qcril_qmi_voice_unmark_calls_srvcc_in_progress()
{
    qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;

    qcril_qmi_voice_voip_lock_overview();
    iter = qmi_voice_voip_overview.call_info_root;
    while ( iter != NULL  )
    {
        iter->srvcc_in_progress = FALSE;
        iter = iter->next;
    }
    qcril_qmi_voice_voip_unlock_overview();
} // qcril_qmi_voice_unmark_calls_srvcc_in_progress

//===========================================================================
// qcril_qmi_voice_reset_all_calls_from_auto_to_cs_domain_elab
//===========================================================================
void qcril_qmi_voice_reset_all_calls_from_auto_to_cs_domain_elab()
{
    qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    qcril_qmi_voice_voip_lock_overview();
    call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
    while ( call_info_entry != NULL  )
    {
        call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
        call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
    }
    qcril_qmi_voice_voip_unlock_overview();

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_reset_all_calls_from_auto_to_cs_domain_elab

//===========================================================================
// qcril_qmi_voice_handover_info_ind_hdlr
//===========================================================================
void qcril_qmi_voice_handover_info_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_handover_ind_msg_v02*    qmi_handover_ind_msg_ptr;
  RIL_Errno                      ret_val = RIL_E_GENERIC_FAILURE;
  qcril_unsol_resp_params_type   unsol_resp_params;
  RIL_SrvccState                 ril_srvccstate = HANDOVER_STARTED;

  QCRIL_LOG_FUNC_ENTRY();

  if( ind_data_ptr != NULL && ind_data_len != 0 )
  {
    if (qcril_qmi_voice_info.jbims)
    {
       qmi_handover_ind_msg_ptr = (voice_handover_ind_msg_v02*) ind_data_ptr;

       QCRIL_LOG_INFO("qmi handover ind ho_type_valid: %d, ho_type: %d",
               qmi_handover_ind_msg_ptr->ho_type_valid, qmi_handover_ind_msg_ptr->ho_type);

       // Need to send the UNSOL_SRVCC_STATE_NOTIFY only
       // in case of handover type is SRVCC L_2_G/L_2_W
       if (qmi_handover_ind_msg_ptr->ho_type_valid)
       {
         switch (qmi_handover_ind_msg_ptr->ho_type)
         {
           case VOICE_HO_SRVCC_L_2_G_V02:
           case VOICE_HO_SRVCC_L_2_W_V02:
             ret_val = RIL_E_SUCCESS;
             break;

           default:
             break;
         }
       }

       if (ret_val == RIL_E_SUCCESS)
       {
         QCRIL_LOG_INFO("qmi handover ind ho_state: %d", qmi_handover_ind_msg_ptr->ho_state);
         switch (qmi_handover_ind_msg_ptr->ho_state)
         {
           case VOICE_HANDOVER_START_V02:
             ril_srvccstate = HANDOVER_STARTED;
             qcril_qmi_voice_mark_calls_srvcc_in_progress();
#ifndef QMI_RIL_UTF
             qcril_am_handle_event(QCRIL_AM_EVENT_SRVCC_START, NULL);
#endif
             break;
           case VOICE_HANDOVER_FAIL_V02:
             ril_srvccstate = HANDOVER_FAILED;
             qcril_qmi_voice_unmark_calls_srvcc_in_progress();
#ifndef QMI_RIL_UTF
             qcril_am_handle_event(QCRIL_AM_EVENT_SRVCC_FAIL, NULL);
#endif
             break;
           case VOICE_HANDOVER_COMPLETE_V02:
             ril_srvccstate = HANDOVER_COMPLETED;
             qcril_qmi_voice_reset_all_calls_from_auto_to_cs_domain_elab();
#ifndef QMI_RIL_UTF
             qcril_am_handle_event(QCRIL_AM_EVENT_SRVCC_COMPLETE, NULL);
#endif
             if (qcril_qmi_ril_domestic_service_is_screen_off())
             {
               qcril_qmi_voice_toggle_ind_reg_on_screen_state(FALSE);
             }
             break;
           case VOICE_HANDOVER_CANCEL_V02:
             ril_srvccstate = HANDOVER_CANCELED;
             qcril_qmi_voice_unmark_calls_srvcc_in_progress();
#ifndef QMI_RIL_UTF
             qcril_am_handle_event(QCRIL_AM_EVENT_SRVCC_CANCEL, NULL);
#endif
             break;
           default:
             ret_val = RIL_E_GENERIC_FAILURE;
             break;
         }/* switch */
         if (RIL_E_SUCCESS == ret_val)
         {
           // Send SRVCC handover indication on RILD socket
           qcril_default_unsol_resp_params(QCRIL_DEFAULT_INSTANCE_ID,
                   RIL_UNSOL_SRVCC_STATE_NOTIFY, &unsol_resp_params);
           unsol_resp_params.resp_pkt = &ril_srvccstate;
           unsol_resp_params.resp_len = sizeof(RIL_SrvccState);
           qcril_send_unsol_response(&unsol_resp_params);

           // Send SRVCC handover indication on IMS socket
           qcril_qmi_voice_send_ims_unsol_resp_handover(ril_srvccstate);
         }
       }
    }/* if (qcril_qmi_voice_info.jbims) */
  }
  else
  {
    QCRIL_LOG_ERROR("ind_data_ptr is NULL");
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_handover_info_ind_hdlr

//===========================================================================
// qcril_qmi_voice_reset_conf_info_xml
//===========================================================================
void qcril_qmi_voice_reset_conf_info_xml()
{
   QCRIL_LOG_FUNC_ENTRY();
   qcril_qmi_voice_info.conf_xml.last_sequence_number = -1;
   qcril_qmi_voice_info.conf_xml.total_size = 0;
   qcril_qmi_voice_info.conf_xml.filled_size = 0;

   if (qcril_qmi_voice_info.conf_xml.buffer)
   {
      qcril_free(qcril_qmi_voice_info.conf_xml.buffer);
      qcril_qmi_voice_info.conf_xml.buffer = NULL;
   }

   qcril_qmi_voice_info.conf_xml.call_id_valid = FALSE;
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_reset_conf_info_xml

//===========================================================================
// qcril_qmi_voice_conference_info_ind_hdlr
//===========================================================================
void qcril_qmi_voice_conference_info_ind_hdlr
(
   void *ind_data_ptr,
   uint32 ind_data_len
)
{
   voice_conference_info_ind_msg_v02* qmi_ind_msg_ptr;

   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_voice_info_lock();

   do
   {
      if (!qcril_qmi_voice_info.jbims)
      {
         QCRIL_LOG_ERROR("jbims is not set");
         break;
      }

      if (NULL == ind_data_ptr || 0 == ind_data_len)
      {
         QCRIL_LOG_ERROR("ind_data_ptr is NULL or ind_data_len is 0");
         break;
      }
      qmi_ind_msg_ptr = (voice_conference_info_ind_msg_v02*) ind_data_ptr;
      QCRIL_LOG_INFO("sequence: %d, total_size_valid: %d, total_size: %d, conference_xml_len: %d",
                     qmi_ind_msg_ptr->sequence, qmi_ind_msg_ptr->total_size_valid, qmi_ind_msg_ptr->total_size, qmi_ind_msg_ptr->conference_xml_len);

      if (0 == qmi_ind_msg_ptr->sequence)
      {
         if (qmi_ind_msg_ptr->total_size_valid)
         {
            qcril_qmi_voice_reset_conf_info_xml();
            qcril_qmi_voice_info.conf_xml.total_size = qmi_ind_msg_ptr->total_size;
            qcril_qmi_voice_info.conf_xml.buffer = qcril_malloc(qcril_qmi_voice_info.conf_xml.total_size);
            if (NULL == qcril_qmi_voice_info.conf_xml.buffer)
            {
               QCRIL_LOG_ERROR("malloc failed");
               qcril_qmi_voice_reset_conf_info_xml();
               break;
            }
         }
         else
         {
            QCRIL_LOG_ERROR("no total size in the first sequence indication");
            break;
         }
      }

      if (qcril_qmi_voice_info.conf_xml.filled_size + qmi_ind_msg_ptr->conference_xml_len > qcril_qmi_voice_info.conf_xml.total_size)
      {
         QCRIL_LOG_ERROR("filled_size (%d) + new conference_xml_len (%d) > total_size (%d)",
                         qcril_qmi_voice_info.conf_xml.filled_size, qmi_ind_msg_ptr->conference_xml_len, qcril_qmi_voice_info.conf_xml.total_size);
         qcril_qmi_voice_reset_conf_info_xml();
         break;
      }

      if (qmi_ind_msg_ptr->sequence !=
             qcril_qmi_voice_info.conf_xml.last_sequence_number + (unsigned int)1)
      {
         QCRIL_LOG_ERROR("sequence out of order! new msg seq#: %d, last_seq#: %d",
                         qmi_ind_msg_ptr->sequence, qcril_qmi_voice_info.conf_xml.last_sequence_number);
         qcril_qmi_voice_reset_conf_info_xml();
         break;
      }

      if (NULL == qcril_qmi_voice_info.conf_xml.buffer)
      {
         QCRIL_LOG_ERROR("qcril_qmi_voice_info.conf_xml.buffer is NULL");
         break;
      }

      memcpy( &(qcril_qmi_voice_info.conf_xml.buffer[qcril_qmi_voice_info.conf_xml.filled_size]),
              qmi_ind_msg_ptr->conference_xml, qmi_ind_msg_ptr->conference_xml_len );
      qcril_qmi_voice_info.conf_xml.filled_size += qmi_ind_msg_ptr->conference_xml_len;
      qcril_qmi_voice_info.conf_xml.last_sequence_number = qmi_ind_msg_ptr->sequence;

      if (qmi_ind_msg_ptr->call_id_valid)
      {
        qcril_qmi_voice_info.conf_xml.call_id_valid = TRUE;
        qcril_qmi_voice_info.conf_xml.call_id = qmi_ind_msg_ptr->call_id;
      }

      if (qcril_qmi_voice_info.conf_xml.filled_size == qcril_qmi_voice_info.conf_xml.total_size)
      {
         Ims__ConfInfo conf_info = IMS__CONF_INFO__INIT;
         conf_info.has_conf_info_uri = TRUE;
         conf_info.conf_info_uri.len = qcril_qmi_voice_info.conf_xml.total_size;
         conf_info.conf_info_uri.data = qcril_qmi_voice_info.conf_xml.buffer;

         if (qcril_qmi_voice_info.conf_xml.call_id_valid)
         {
           qcril_qmi_voice_voip_call_info_entry_type *call_info_entry =
             qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id(qcril_qmi_voice_info.conf_xml.call_id);

           if (call_info_entry)
           {
             Ims__ConfCallState ims_state;
             if (qcril_qmi_ims_map_qmi_call_state_to_ims_conf_call_state(call_info_entry->voice_scv_info.call_state, &ims_state))
             {
               conf_info.has_confcallstate = TRUE;
               conf_info.confcallstate = ims_state;
             }
           }
         }

         qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_REFRESH_CONF_INFO, IMS__ERROR__E_SUCCESS, &conf_info, sizeof(conf_info));
         qcril_qmi_voice_send_ims_unsol_call_state_changed();
         qcril_qmi_voice_reset_conf_info_xml();
      }

   } while (FALSE);

   qcril_qmi_voice_info_unlock();

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_conference_info_ind_hdlr

//===========================================================================
// qcril_qmi_voice_tty_ind_hdlr
//===========================================================================
void qcril_qmi_voice_tty_ind_hdlr
(
   void *ind_data_ptr,
   uint32 ind_data_len
)
{
   voice_tty_ind_msg_v02* qmi_ind_msg_ptr;

   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_voice_info_lock();

   do
   {
      if (!qcril_qmi_voice_info.jbims)
      {
         QCRIL_LOG_ERROR("jbims is not set");
         break;
      }

      if (NULL == ind_data_ptr || 0 == ind_data_len)
      {
         QCRIL_LOG_ERROR("ind_data_ptr is NULL or ind_data_len is 0");
         break;
      }

      qmi_ind_msg_ptr = (voice_tty_ind_msg_v02*) ind_data_ptr;
      QCRIL_LOG_INFO("tty_mode: %d", qmi_ind_msg_ptr->tty_mode);

      Ims__TtyNotify ims_tty_info = IMS__TTY_NOTIFY__INIT;
      ims_tty_info.has_mode = TRUE;
      switch(qmi_ind_msg_ptr->tty_mode)
      {
         case TTY_MODE_FULL_V02:
            ims_tty_info.mode = IMS__TTY__MODE__TYPE__TTY_MODE_FULL;
            break;

         case TTY_MODE_VCO_V02:
            ims_tty_info.mode = IMS__TTY__MODE__TYPE__TTY_MODE_VCO;
            break;

         case TTY_MODE_HCO_V02:
            ims_tty_info.mode = IMS__TTY__MODE__TYPE__TTY_MODE_HCO;
            break;

         case TTY_MODE_OFF_V02:
            ims_tty_info.mode = IMS__TTY__MODE__TYPE__TTY_MODE_OFF;
            break;

         default:
            ims_tty_info.mode = IMS__TTY__MODE__TYPE__TTY_MODE_OFF;
            break;
      }

      qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_TTY_NOTIFICATION, IMS__ERROR__E_SUCCESS, &ims_tty_info, sizeof(ims_tty_info));

   } while (FALSE);

   qcril_qmi_voice_info_unlock();

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_tty_ind_hdlr

/*=========================================================================
  FUNCTION: qcril_qmi_voice_call_control_result_info_ind_hdlr

    @brief
    Handle QMI_VOICE_CALL_CONTROL_RESULT_INFO_IND_V02.

    @return
    None.
=========================================================================*/
void qcril_qmi_voice_call_control_result_info_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
   voice_call_control_result_info_ind_msg_v02* qmi_ind_msg_ptr;
   qcril_unsol_resp_params_type unsol_resp;
   qcril_instance_id_e_type instance_id;
   char buf_str[QMI_VOICE_CC_ALPHA_TEXT_MAX_V02 + 2];

   QCRIL_LOG_FUNC_ENTRY();
   instance_id = qmi_ril_get_process_instance_id();
   memset(buf_str, 0 , QMI_VOICE_CC_ALPHA_TEXT_MAX_V02 + 2);

   if( ind_data_ptr != NULL && ind_data_len != 0 )
   {
       qmi_ind_msg_ptr = (voice_call_control_result_info_ind_msg_v02*) ind_data_ptr;

        if( ( VOICE_CC_ALPHA_NOT_PRESENT_V02 == qmi_ind_msg_ptr->alpha_presence ) ||
            ( VOICE_CC_ALPHA_NULL_V02 == qmi_ind_msg_ptr->alpha_presence )
          )
        {
            QCRIL_LOG_INFO("Either Alhpa is absent in cc result or Alpha is present but length is zero");
        }
        else
        {
            if( TRUE == qmi_ind_msg_ptr->alpha_text_gsm8_valid )
            {
                QCRIL_LOG_INFO("Alpha text message is present in gsm8 bit format");
                if(qmi_ind_msg_ptr->alpha_text_gsm8_len < QMI_VOICE_CC_ALPHA_TEXT_MAX_V02 )
                    qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( (char*) qmi_ind_msg_ptr->alpha_text_gsm8,
                                                                qmi_ind_msg_ptr->alpha_text_gsm8_len,
                                                                buf_str );
            }
            else
            {
                QCRIL_LOG_INFO("Alpha text message is present in UTF16 format");
                qcril_cm_ss_convert_ucs2_to_utf8( (char *) qmi_ind_msg_ptr->alpha_text_utf16,
                                                             qmi_ind_msg_ptr->alpha_text_utf16_len * 2, buf_str );
            }
        }
   }
   else
   {
       QCRIL_LOG_ERROR("ind_data_ptr is NULL");
   }

   if ( *buf_str )
   {
       qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_STK_CC_ALPHA_NOTIFY, &unsol_resp );
       unsol_resp.resp_pkt    = (void*)buf_str;
       unsol_resp.resp_len    = sizeof( buf_str );

       qcril_send_unsol_response( &unsol_resp );
   }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_voice_call_control_result_info_ind_hdlr

//===========================================================================
// qcril_qmi_voice_reset_additional_call_info
//===========================================================================
void qcril_qmi_voice_reset_additional_call_info
(
    qcril_qmi_voice_voip_call_info_entry_type *entry
)
{
   QCRIL_LOG_FUNC_ENTRY();

   if (entry)
   {
      entry->additional_call_info.last_sequence_number = -1;
      entry->additional_call_info.total_size = 0;
      entry->additional_call_info.filled_size = 0;

      if (entry->additional_call_info.buffer)
      {
         qcril_free(entry->additional_call_info.buffer);
         entry->additional_call_info.buffer = NULL;
      }
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_reset_additional_call_info

//===========================================================================
// qcril_qmi_voice_is_additional_call_info_available
//===========================================================================
boolean qcril_qmi_voice_is_additional_call_info_available
(
    qcril_qmi_voice_voip_call_info_entry_type *entry
)
{
   boolean add_info_present = FALSE;

   QCRIL_LOG_FUNC_ENTRY();

   if (entry)
   {
      if (entry->additional_call_info.is_add_info_present &&
          (entry->additional_call_info.total_size > 0)&&
          (entry->additional_call_info.filled_size ==
           entry->additional_call_info.total_size))
      {
         add_info_present = TRUE;
      }
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(add_info_present);

   return add_info_present;
} // qcril_qmi_voice_is_additional_call_info_available


//===========================================================================
// qcril_qmi_voice_additional_call_info_ind_hdlr
//===========================================================================
void qcril_qmi_voice_additional_call_info_ind_hdlr
(
   void *ind_data_ptr,
   uint32 ind_data_len
)
{
   voice_additional_call_info_ind_msg_v02    *qmi_ind_msg_ptr = NULL;
   voice_additional_call_info_type_v02       *add_call_info   = NULL;
   qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = NULL;

   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_voice_info_lock();

   do
   {
      if (!qcril_qmi_voice_info.jbims)
      {
         QCRIL_LOG_ERROR("jbims is not set");
         break;
      }

      if (NULL == ind_data_ptr || 0 == ind_data_len)
      {
         QCRIL_LOG_ERROR("ind_data_ptr is NULL or ind_data_len is 0");
         break;
      }
      qmi_ind_msg_ptr = (voice_additional_call_info_ind_msg_v02*) ind_data_ptr;
      if (qmi_ind_msg_ptr->extension_header_info_valid)
      {
         add_call_info   = &(qmi_ind_msg_ptr->extension_header_info);
      }
      else
      {
         QCRIL_LOG_ERROR("extension_header_info is not valid");
         break;
      }
      QCRIL_LOG_INFO("call_id: %d, sequence: %d, total_size: %d, additional_call_info_len: %d",
              qmi_ind_msg_ptr->call_id, add_call_info->sequence,
              add_call_info->total_size, add_call_info->additional_call_info_len);

      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id(
              qmi_ind_msg_ptr->call_id);
      if (NULL == call_info_entry)
      {
         QCRIL_LOG_ERROR("Unable to find call info entry for call_id: %d",
                 qmi_ind_msg_ptr->call_id);
         break;
      }

      if (0 == add_call_info->sequence)
      {
         qcril_qmi_voice_reset_additional_call_info(call_info_entry);
         call_info_entry->additional_call_info.total_size = add_call_info->total_size;
         call_info_entry->additional_call_info.buffer     =
               qcril_malloc(call_info_entry->additional_call_info.total_size);
         if (NULL == call_info_entry->additional_call_info.buffer)
         {
            QCRIL_LOG_ERROR("malloc failed");
            qcril_qmi_voice_reset_additional_call_info(call_info_entry);
            break;
         }
      }

      if ((call_info_entry->additional_call_info.filled_size +
            add_call_info->additional_call_info_len) >
          call_info_entry->additional_call_info.total_size)
      {
         QCRIL_LOG_ERROR("filled_size (%d) + new additional_call_info_len (%d) > total_size (%d)",
                         call_info_entry->additional_call_info.filled_size,
                         add_call_info->additional_call_info_len,
                         call_info_entry->additional_call_info.total_size);
         qcril_qmi_voice_reset_additional_call_info(call_info_entry);
         break;
      }

      if (add_call_info->sequence !=
          call_info_entry->additional_call_info.last_sequence_number+1)
      {
         QCRIL_LOG_ERROR("sequence out of order! new msg seq#: %d, last_seq#: %d",
                         add_call_info->sequence,
                         call_info_entry->additional_call_info.last_sequence_number);
         qcril_qmi_voice_reset_additional_call_info(call_info_entry);
         break;
      }

      if (NULL == call_info_entry->additional_call_info.buffer)
      {
         QCRIL_LOG_ERROR("call_info_entry->additional_call_info.buffer is NULL");
         break;
      }

      memcpy(&(call_info_entry->additional_call_info.buffer[
                    call_info_entry->additional_call_info.filled_size]),
              add_call_info->additional_call_info, add_call_info->additional_call_info_len);
      call_info_entry->additional_call_info.filled_size += add_call_info->additional_call_info_len;
      call_info_entry->additional_call_info.last_sequence_number = add_call_info->sequence;
      if (call_info_entry->additional_call_info.filled_size ==
          call_info_entry->additional_call_info.total_size)
      {
        qcril_qmi_voice_send_ims_unsol_call_state_changed();
      }
   } while (FALSE);

   qcril_qmi_voice_info_unlock();

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_additional_call_info_ind_hdlr

//===========================================================================
// qcril_qmi_voice_audio_rat_change_info_ind_hdlr
//===========================================================================
void qcril_qmi_voice_audio_rat_change_info_ind_hdlr
(
  void *ind_data_ptr,
  uint32 ind_data_len
)
{
  voice_audio_rat_change_info_ind_msg_v02 *qmi_ind_msg_ptr = NULL;
  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( NULL == ind_data_ptr || 0 == ind_data_len )
    {
      QCRIL_LOG_ERROR("ind_data_ptr is NULL or ind_data_len is 0");
      break;
    }
    qmi_ind_msg_ptr = (voice_audio_rat_change_info_ind_msg_v02*)ind_data_ptr;

#ifndef QMI_RIL_UTF
    qcril_am_handle_event(QCRIL_AM_EVENT_AUDIO_RAT_CHANGED, qmi_ind_msg_ptr);
#endif
  } while (0);

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_audio_rat_change_info_ind_hdlr

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_unsol_ind_cb

===========================================================================*/
/*!
    @brief
    Callback for QMI VOICE indications

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_unsol_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                          *ind_buf,
  unsigned int                   ind_buf_len,
  void                          *ind_cb_data
)
{
  qmi_ind_callback_type qmi_callback;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_callback.data_buf = qcril_malloc(ind_buf_len);

  if( qmi_callback.data_buf )
  {
    qmi_callback.user_handle = user_handle;
    qmi_callback.msg_id = msg_id;
    memcpy(qmi_callback.data_buf,ind_buf,ind_buf_len);
    qmi_callback.data_buf_len = ind_buf_len;
    qmi_callback.cb_data = ind_cb_data;

    qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_ON_STACK,
                   QCRIL_EVT_QMI_VOICE_HANDLE_INDICATIONS,
                   (void*) &qmi_callback,
                   sizeof(qmi_callback),
                   (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  }
  else
  {
    QCRIL_LOG_FATAL("malloc failed");
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_voice_unsol_ind_cb */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_unsol_ind_cb_helper

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_VOICE_HANDLE_INDICATIONS

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_unsol_ind_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  qmi_ind_callback_type * qmi_callback;

  qmi_callback = (qmi_ind_callback_type*) params_ptr->data;

  uint32_t decoded_payload_len = 0;
  qmi_client_error_type qmi_err = QMI_NO_ERR;
  void* decoded_payload = NULL;

  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  if( qmi_callback )
  {
    qmi_idl_get_message_c_struct_len(
                     qcril_qmi_client_get_service_object(QCRIL_QMI_CLIENT_VOICE),
                     QMI_IDL_INDICATION,
                     qmi_callback->msg_id,
                     &decoded_payload_len);

    if(decoded_payload_len)
    {
      decoded_payload = qcril_malloc(decoded_payload_len);
    }

    if ( decoded_payload || !decoded_payload_len )
    {
        if( decoded_payload_len )
        {
          qmi_err = qmi_client_message_decode(qcril_qmi_client_get_user_handle(QCRIL_QMI_CLIENT_VOICE),
                                              QMI_IDL_INDICATION,
                                              qmi_callback->msg_id,
                                              qmi_callback->data_buf,
                                              qmi_callback->data_buf_len,
                                              decoded_payload,
                                              (int)decoded_payload_len);
        }



        QCRIL_LOG_INFO(".. operational state %d", (int) qmi_ril_get_operational_status() );
        // In DSDS QCRIL state will be UNRESTRICTED only when subscriptions are
        // ACTIVE. But without SIM subscriptions cannot be ACTIVE. To allow
        // emergency calls in this state we need to check if there is any
        // pending emergency call in non UNRESTRICTED state.
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset
                          ( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS|
                            QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL);
        if ( ( qmi_ril_get_operational_status() == QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED
               || call_info_entry != NULL ) && QMI_NO_ERR == qmi_err )
        {
            switch(qmi_callback->msg_id)
          {
              case QMI_VOICE_ALL_CALL_STATUS_IND_V02:
                qcril_qmi_voice_all_call_status_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_OTASP_STATUS_IND_V02:
                qcril_qmi_voice_otasp_status_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_PRIVACY_IND_V02:
                qcril_qmi_voice_privacy_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_SUPS_NOTIFICATION_IND_V02:
                qcril_qmi_voice_sups_notification_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_INFO_REC_IND_V02:
                qcril_qmi_voice_info_rec_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_USSD_IND_V02:
                qcril_qmi_voice_ussd_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_USSD_RELEASE_IND_V02:
                qcril_qmi_voice_ussd_release_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_SUPS_IND_V02:
                qcril_qmi_voice_stk_cc_handle_voice_sups_ind( (voice_sups_ind_msg_v02*) decoded_payload );
                break;

              case QMI_VOICE_DTMF_IND_V02:
                qcril_qmi_voice_dtmf_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_EXT_BRST_INTL_IND_V02:
                qcril_qmi_voice_ext_brst_intl_ind_hdlr(decoded_payload, decoded_payload_len);
                break;

              case QMI_VOICE_UUS_IND_V02:
                qcril_qmi_voice_uus_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_MODIFIED_IND_V02:
                qcril_qmi_voice_modified_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_MODIFY_ACCEPT_IND_V02:
                qcril_qmi_voice_modify_accept_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_SPEECH_CODEC_INFO_IND_V02:
                qcril_qmi_voice_speech_codec_info_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_HANDOVER_IND_V02:
                qcril_qmi_voice_handover_info_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_CONFERENCE_INFO_IND_V02:
                qcril_qmi_voice_conference_info_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_TTY_IND_V02:
                qcril_qmi_voice_tty_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_CALL_CONTROL_RESULT_INFO_IND_V02:
                qcril_qmi_voice_call_control_result_info_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_ADDITIONAL_CALL_INFO_IND_V02:
                qcril_qmi_voice_additional_call_info_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              case QMI_VOICE_AUDIO_RAT_CHANGE_INFO_IND_V02:
                qcril_qmi_voice_audio_rat_change_info_ind_hdlr( decoded_payload, decoded_payload_len );
                break;

              default:
                QCRIL_LOG_INFO("Unknown QMI VOICE indication %d", qmi_callback->msg_id);
                break;
            }
          }
          else
          {
              QCRIL_LOG_INFO("Indication decode failed for msg %d of svc %d with error %d", qmi_callback->msg_id, QCRIL_QMI_CLIENT_VOICE, qmi_err );
          }

          if( decoded_payload_len )
          {
            qcril_free(decoded_payload);
          }
    }

    if( qmi_callback->data_buf )
    {
      qcril_free(qmi_callback->data_buf);
    }
  }

  QCRIL_LOG_FUNC_RETURN();

}/* qcril_qmi_voice_unsol_ind_cb_helper */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_sups_cmd_mng_calls_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle MNG_CALLS_RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_sups_cmd_mng_calls_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_manage_calls_resp_msg_v02    *mng_calls_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err = RIL_E_SUCCESS;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;

  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type oversight_event_params;
  qmi_ril_voice_ims_command_exec_oversight_type *                   command_oversight;
  int                                                               covered_by_oversight_handling;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {
    //memset(mng_calls_resp,0,sizeof(voice_manage_calls_resp_msg_v02));
    /* Entry found in the ReqList */

    QCRIL_LOG_INFO("params_ptr->data is not NULL");

    mng_calls_resp = (voice_manage_calls_resp_msg_v02 *)params_ptr->data;
    qmi_result = mng_calls_resp->resp.result;
    qmi_error = mng_calls_resp->resp.error;

    covered_by_oversight_handling = FALSE;
    qcril_qmi_voice_voip_lock_overview();
    command_oversight = qmi_ril_voice_ims_find_command_oversight_by_token( params_ptr->t );
    if ( NULL != command_oversight )
    {
      memset( &oversight_event_params, 0, sizeof( oversight_event_params ) );
      oversight_event_params.locator.command_oversight = command_oversight;
      covered_by_oversight_handling = qmi_ril_voice_ims_command_oversight_handle_event
                                      (
                                          ( QMI_RESULT_SUCCESS_V01 == qmi_result && QMI_ERR_NONE_V01 == qmi_error ) ?
                                              QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_SUCCESS : QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_FAILURE ,
                                          QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                          &oversight_event_params
                                      );
    } // if ( NULL != command_oversight )
    qcril_qmi_voice_voip_unlock_overview();

    if ( !covered_by_oversight_handling )
    {
      if(qmi_result == QMI_RESULT_SUCCESS_V01)
      {
        QCRIL_LOG_INFO("QCRIL QMI VOICE MNG CALLS RESP: SUCCESS");
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, ril_err);
      }
      else
      {
        QCRIL_LOG_INFO("QCRIL QMI VOICE MNG CALLS RESP: FAILURE");
        if(mng_calls_resp->failure_cause_valid == TRUE)
        {
          QCRIL_LOG_ERROR("QCRIL QMI VOICE MNG CALLS RESP sups_failure_cause=%d, for Token ID= %d",
              mng_calls_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
          /* Send UNSOL msg with SS error code first */
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( mng_calls_resp->failure_cause,
              VOICE_INVALID_CALL_ID );
        }
        ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
        /* Send FAILURE response */
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t,params_ptr->event_id, ril_err );
      } // if(qmi_result == QMI_RESULT_SUCCESS_V01)
    } // if ( !covered_by_oversight_handling )

    if ( QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND == params_ptr->event_id ||
         QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == params_ptr->event_id ||
         RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND == params_ptr->event_id ||
         RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == params_ptr->event_id ||
         QCRIL_EVT_IMS_SOCKET_REQ_RESUME == params_ptr->event_id )
    {
      qcril_qmi_voice_voip_unmark_all_with(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE);
      if (RIL_E_SUCCESS != ril_err)
      {
#ifndef QMI_RIL_UTF
        qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL_FAIL, NULL);
#endif
      }
    }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }
}/* qcril_qmi_voice_sups_cmd_mng_calls_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_set_sups_service_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle SET SUPS Service RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_set_sups_service_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_set_sups_service_resp_msg_v02    *set_sups_service_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  char sups_service_resp_utf8_str[QCRIL_QMI_VOICE_MAX_SUPS_FAILURE_STR_LEN];
  int utf8_len = 0;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_LOG_FUNC_ENTRY();

  if( params_ptr->data != NULL )
  {
      //memset(set_sups_service_resp,0,sizeof(voice_set_sups_service_resp_msg_v02));
      /* Entry found in the ReqList */

      QCRIL_LOG_INFO("params_ptr->data is not NULL");

      set_sups_service_resp = (voice_set_sups_service_resp_msg_v02 *)params_ptr->data;
      qmi_result = set_sups_service_resp->resp.result;
      qmi_error = set_sups_service_resp->resp.error;

      if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                               instance_id,
                                               &set_sups_service_resp->resp,
                                               NULL,
                                               set_sups_service_resp->alpha_ident_valid,
                                               &set_sups_service_resp->alpha_ident,
                                               set_sups_service_resp->call_id_valid,
                                               set_sups_service_resp->call_id,
                                               set_sups_service_resp->cc_sups_result_valid,
                                               &set_sups_service_resp->cc_sups_result,
                                               set_sups_service_resp->cc_result_type_valid,
                                               &set_sups_service_resp->cc_result_type
                                               )
           )
      {
        if((qmi_result == QMI_RESULT_SUCCESS_V01) && (qmi_error == QMI_ERR_NONE_V01))
        {
          QCRIL_LOG_INFO("QCRIL QMI VOICE SET SUPS RESP: SUCCESS");
          qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
        }
        else
        {
          QCRIL_LOG_INFO("QCRIL QMI VOICE SET SUPS RESP: FAILURE");

          if(set_sups_service_resp->failure_cause_valid == TRUE)
          {
            QCRIL_LOG_ERROR("QCRIL QMI VOICE SET SUPS RESP sups_failure_cause=%d, for Token ID= %d",
                set_sups_service_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));

            /* Send oem_hook_unsol_resp if not an IMS request */
            if ( !(params_ptr->event_id > QCRIL_EVT_IMS_SOCKET_REQ_BASE &&
              params_ptr->event_id < QCRIL_EVT_IMS_SOCKET_REQ_MAX ) )
            {
              /* Send UNSOL msg with SS error code first */
              qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp (
                set_sups_service_resp->failure_cause,
                (set_sups_service_resp->call_id_valid ?
                set_sups_service_resp->call_id : VOICE_INVALID_CALL_ID) );
            }
          }

          ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);

          //Failure cause string is supported for ims supp svc requests only
          if( (set_sups_service_resp->failure_cause_description_valid == TRUE) &&
              ( ( QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS == params_ptr->event_id ) ||
                ( QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING == params_ptr->event_id ) ||
                ( QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS == params_ptr->event_id ) ) )
          {
            QCRIL_LOG_ERROR("QCRIL QMI VOICE SET SUPS RESP sups_failure_description_len=%d", set_sups_service_resp->failure_cause_description_len);

            memset(sups_service_resp_utf8_str, 0x0, sizeof(sups_service_resp_utf8_str));
            utf8_len = qcril_cm_ss_convert_ucs2_to_utf8( (char *) set_sups_service_resp->failure_cause_description,
                                                         set_sups_service_resp->failure_cause_description_len * 2,
                                                         sups_service_resp_utf8_str );
            if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_SUPS_FAILURE_STR_LEN ) )
            {
              QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR" );
              utf8_len = (int) (QCRIL_QMI_VOICE_MAX_SUPS_FAILURE_STR_LEN);
              sups_service_resp_utf8_str[ utf8_len - 1] = '\0';
            }
            QCRIL_LOG_ERROR("QCRIL QMI VOICE SET SUPS RESP len =%d, sups_failure_description=%s", utf8_len, sups_service_resp_utf8_str);

            Ims__SuppSvcResponse supp_svc_resp = IMS__SUPP_SVC_RESPONSE__INIT;
            supp_svc_resp.failurecause = qcril_malloc(utf8_len);
            if( supp_svc_resp.failurecause != NULL )
            {
              strlcpy( supp_svc_resp.failurecause, sups_service_resp_utf8_str, utf8_len );

              qcril_qmi_ims_socket_send( params_ptr->t,
                                         IMS__MSG_TYPE__RESPONSE,
                                         qcril_qmi_ims_map_event_to_request(params_ptr->event_id),
                                         qcril_qmi_ims_map_ril_error_to_ims_error(ril_err),
                                         (void *)&supp_svc_resp,
                                         sizeof(supp_svc_resp));
              qcril_free(supp_svc_resp.failurecause);
            }
            else
            {
              /* Send FAILURE response */
              qcril_send_empty_payload_request_response( instance_id, params_ptr->t,params_ptr->event_id, ril_err );
            }
          }
          else
          {
            /* Send FAILURE response */
            qcril_send_empty_payload_request_response( instance_id, params_ptr->t,params_ptr->event_id, ril_err );
          }
        }
      }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }

  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_voice_set_sups_service_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_get_clip_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle GET CLIP RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_get_clip_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_clip_resp_msg_v02 *get_clip_resp;
  RIL_Errno ril_err;
  RIL_Errno ril_result;
  qcril_request_resp_params_type resp;
  qmi_error_type_v01  qmi_error;
  int response[ 1 ];

  memset( response, 0, sizeof ( response ) );
  memset( &resp, 0, sizeof ( resp ) );

  if( params_ptr->data != NULL )
  {
    // memset(get_clip_resp,0,sizeof(voice_get_clip_resp_msg_v02));
    /* Entry found in the ReqList */

    QCRIL_LOG_INFO("params_ptr->data is not NULL");
    get_clip_resp = (voice_get_clip_resp_msg_v02 *)params_ptr->data;
    qmi_error = get_clip_resp->resp.error;
    ril_result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                   QMI_NO_ERR,
                   &get_clip_resp->resp);

    if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                             QCRIL_DEFAULT_INSTANCE_ID,
                                             &get_clip_resp->resp,
                                             NULL,
                                             get_clip_resp->alpha_id_valid,
                                             &get_clip_resp->alpha_id,
                                             get_clip_resp->call_id_valid,
                                             get_clip_resp->call_id,
                                             get_clip_resp->cc_sups_result_valid,
                                             &get_clip_resp->cc_sups_result,
                                             get_clip_resp->cc_result_type_valid,
                                             &get_clip_resp->cc_result_type
                                             )
         )
    { // not stk cc case
      response[0] = QCRIL_QMI_VOICE_CLIR_SRV_NO_NETWORK;
      if( get_clip_resp->clip_response_valid == TRUE )
      {
       qcril_qmi_voice_map_qmi_status_to_ims_provision_status(
                get_clip_resp->clip_response.provision_status,
                get_clip_resp->clip_response.active_status,
                &response[0]);
      }
      QCRIL_LOG_DEBUG("QCRIL QMI VOICE GET CLIP RESP response[0]=%d, for Token ID= %d", response[0],qcril_log_get_token_id( params_ptr->t ));

      if( RIL_E_SUCCESS == ril_result )
      {
        QCRIL_LOG_INFO("QCRIL QMI VOICE GET CLIP RESP: SUCCESS");
        if (RIL_REQUEST_QUERY_CLIP == params_ptr->event_id)
        {
          qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                             params_ptr->t,
                                             params_ptr->event_id,
                                             RIL_E_SUCCESS,
                                             &resp );
          resp.resp_pkt = (void *) response;
          resp.resp_len = sizeof( response );
          qcril_send_request_response( &resp );
        }
        else
        {
           Ims__ClipProvisionStatus clip = IMS__CLIP_PROVISION_STATUS__INIT;
           clip.has_clip_status = TRUE;
           clip.clip_status = response[0];
           qcril_qmi_ims_socket_send(params_ptr->t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_QUERY_CLIP, IMS__ERROR__E_SUCCESS, (void *)&clip, sizeof(clip));
        }
      }
      else
      {
        QCRIL_LOG_INFO("QCRIL QMI VOICE GET CLIP RESP: FAILURE");

        if(get_clip_resp->failure_cause_valid == TRUE)
        {
          QCRIL_LOG_ERROR("QCRIL QMI VOICE GET CLIP RESP sups_failure_cause=%d, for Token ID= %d",
              get_clip_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
          if (RIL_REQUEST_QUERY_CLIP == params_ptr->event_id)
          {
            /* Send UNSOL msg with SS error code first */
            qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( get_clip_resp->failure_cause,
              (get_clip_resp->call_id_valid ? get_clip_resp->call_id : VOICE_INVALID_CALL_ID) );
          }
        }

        ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
        /* Send FAILURE response */
        qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                   params_ptr->t,
                                                   params_ptr->event_id,
                                                   ril_err );
      }
    }
  }
  else
  {
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       RIL_E_GENERIC_FAILURE,
                                       &resp );
    qcril_send_request_response( &resp );
  }
}/* qcril_qmi_voice_get_clip_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_get_colp_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI VOICE GET COLP RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_get_colp_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_colp_resp_msg_v02 *get_colp_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  RIL_Errno ril_result;
  qcril_request_resp_params_type resp;
  qmi_error_type_v01  qmi_error;
  qmi_sups_errors_enum_v02 sups_failure_cause;
  int response[ 1 ];

  memset( response, 0, sizeof ( response ) );
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  if( params_ptr->data != NULL )
  {
    QCRIL_LOG_INFO("params_ptr->data is not NULL");

    get_colp_resp = (voice_get_colp_resp_msg_v02 *)params_ptr->data;
    qmi_error = get_colp_resp->resp.error;
    ril_result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                   QMI_NO_ERR,
                   &get_colp_resp->resp);

    if(get_colp_resp->failure_cause_valid == TRUE)
    {
      sups_failure_cause = get_colp_resp->failure_cause;
      QCRIL_LOG_ERROR("QCRIL QMI VOICE GET COLP RESP sups_failure_cause=%d, for Token ID= %d",
                      sups_failure_cause,
                      qcril_log_get_token_id( params_ptr->t ));
    }

    response[0] = QCRIL_QMI_VOICE_CLIR_SRV_NO_NETWORK;
    if( get_colp_resp->colp_response_valid == TRUE )
    {
      qcril_qmi_voice_map_qmi_status_to_ims_provision_status(
              get_colp_resp->colp_response.provision_status,
              get_colp_resp->colp_response.active_status,
              &response[0]);
    }
    QCRIL_LOG_DEBUG("QCRIL QMI VOICE GET COLP RESP response[0]=%d, for Token ID= %d",
                    response[0],
                    qcril_log_get_token_id( params_ptr->t ));

    if( RIL_E_SUCCESS == ril_result )
    {
      QCRIL_LOG_INFO("QCRIL QMI VOICE GET COLP RESP: SUCCESS");
      {
        Ims__SuppSvcResponse supp_svc_response = IMS__SUPP_SVC_RESPONSE__INIT;
        supp_svc_response.has_status = TRUE;
        supp_svc_response.status = response[0];
        supp_svc_response.has_facilitytype = TRUE;
        supp_svc_response.facilitytype = IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_COLP;

        qcril_qmi_ims_socket_send(params_ptr->t,
                                   IMS__MSG_TYPE__RESPONSE,
                                   IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS,
                                   IMS__ERROR__E_SUCCESS,
                                   (void *)&supp_svc_response,
                                   sizeof(supp_svc_response));
      }
    }
    else
    {
      QCRIL_LOG_INFO("QCRIL QMI VOICE GET COLP RESP: FAILURE");

      ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
      /* Send FAILURE response */
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 ril_err );
    }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id,
                                               params_ptr->t,
                                               params_ptr->event_id,
                                               RIL_E_GENERIC_FAILURE );
  }
}/* qcril_qmi_voice_get_colp_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_get_clir_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle GET CLIR RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_get_clir_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_clir_resp_msg_v02 *get_clir_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  int response[ 2 ];
  boolean success=FALSE;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_LOG_FUNC_ENTRY();

  if( params_ptr->data != NULL )
  {
    /* Entry found in the ReqList */

    QCRIL_LOG_INFO("params_ptr->data is not NULL");

    get_clir_resp = (voice_get_clir_resp_msg_v02 *)params_ptr->data;
    qmi_result = get_clir_resp->resp.result;
    qmi_error = get_clir_resp->resp.error;
    if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                             instance_id,
                                             &get_clir_resp->resp,
                                             NULL,
                                             get_clir_resp->alpha_id_valid,
                                             &get_clir_resp->alpha_id,
                                             get_clir_resp->call_id_valid,
                                             get_clir_resp->call_id,
                                             get_clir_resp->cc_sups_result_valid,
                                             &get_clir_resp->cc_sups_result,
                                             get_clir_resp->cc_result_type_valid,
                                             &get_clir_resp->cc_result_type
                                             )
         )
    {
      if( (qmi_result == QMI_RESULT_SUCCESS_V01) && (qmi_error == QMI_ERR_SUPS_FAILURE_CAUSE_V01))
      {
        if(get_clir_resp->clir_response_valid == TRUE)
        {
          response[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_NO_NETWORK;
          response[0] = qcril_qmi_voice_info.clir;
          success = TRUE;
          QCRIL_LOG_DEBUG("QCRIL QMI VOICE GET CLIR RESP response[0]=%d, response[1]= %d", response[0],response[1]);
        }
      }
      else
      {
        if ( (QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR == params_ptr->event_id) &&
             (get_clir_resp->failure_cause_valid == TRUE) &&
             (get_clir_resp->failure_cause == QMI_FAILURE_CAUSE_FACILITY_NOT_SUPPORTED_V02) )
        {
            success = TRUE;
            //In case get_clir not supported by IMS at modem, then return cached value.
            if ( qcril_qmi_voice_info.clir == QCRIL_QMI_VOICE_SS_CLIR_INVOCATION_OPTION )
            {
              response[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_ALLOWED;
            }
            else if ( qcril_qmi_voice_info.clir == QCRIL_QMI_VOICE_SS_CLIR_SUPPRESSION_OPTION )
            {
              response[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_RESTRICTED;
            }
            else
            {
              response[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_ALLOWED;
            }
        }
        else if ((get_clir_resp->clir_response_valid == TRUE) && (qmi_error == QMI_ERR_NONE_V01))
        {
          response[0] = qcril_qmi_voice_info.clir;
          success = qcril_qmi_voice_map_qmi_to_ril_provision_status(
                  get_clir_resp->clir_response.provision_status,
                  &response[1]);
          QCRIL_LOG_DEBUG("QCRIL QMI VOICE GET CLIR RESP response[0]=%d, response[1]= %d", response[0],response[1]);
        }
        else
        {
          QCRIL_LOG_DEBUG("QCRIL QMI VOICE GET CLIR RESP : FAILURE");
          success = FALSE;
        }
      }

      if( success )
      {
        QCRIL_LOG_INFO("QCRIL QMI VOICE GET CLIR RESP: SUCCESS");
        if (RIL_REQUEST_GET_CLIR == params_ptr->event_id)
        {
           qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
           resp.resp_pkt = (void *) response;
           resp.resp_len = sizeof( response );
           qcril_send_request_response( &resp );
        }
        else
        {
           Ims__Clir clir = IMS__CLIR__INIT;
           clir.has_param_m = TRUE;
           clir.param_m = response[1];
           clir.has_param_n = TRUE;
           clir.param_n = qcril_qmi_voice_info.clir;
           qcril_qmi_ims_socket_send(params_ptr->t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_GET_CLIR, IMS__ERROR__E_SUCCESS, (void *)&clir, sizeof(clir));
        }
      }
      else
      {
        QCRIL_LOG_INFO("QCRIL QMI VOICE GET CLIR RESP: FAILURE");

        if (get_clir_resp->failure_cause_valid)
        {
          QCRIL_LOG_DEBUG("QCRIL QMI VOICE GET CLIR RESP sups_failure_cause=%d, for Token ID= %d",
              get_clir_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
          /* Send UNSOL msg with SS error code first */
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( get_clir_resp->failure_cause,
              (get_clir_resp->call_id_valid ? get_clir_resp->call_id : VOICE_INVALID_CALL_ID) );
        }

        ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
        /* Send FAILURE response */
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t,params_ptr->event_id, ril_err );
      }
    }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }
  QCRIL_LOG_FUNC_RETURN();
}/* qcril_qmi_voice_get_clir_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_query_call_waiting_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Query Call Waiting RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_query_call_waiting_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_call_waiting_resp_msg_v02 *get_cw_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  Ims__CallWaitingInfo ims_resp;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  int response[ 2 ];


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {
      /* Entry found in the ReqList */

      QCRIL_LOG_INFO("params_ptr->data is not NULL");

      get_cw_resp = (voice_get_call_waiting_resp_msg_v02 *)params_ptr->data;
      qmi_result = get_cw_resp->resp.result;
      qmi_error = get_cw_resp->resp.error;
      QCRIL_LOG_INFO("QCRIL QMI VOICE Query Call Waiting qmi_error : %d", qmi_error );

      if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                               instance_id,
                                               &get_cw_resp->resp,
                                               NULL,
                                               get_cw_resp->alpha_id_valid,
                                               &get_cw_resp->alpha_id,
                                               get_cw_resp->call_id_valid,
                                               get_cw_resp->call_id,
                                               get_cw_resp->cc_sups_result_valid,
                                               &get_cw_resp->cc_sups_result,
                                               get_cw_resp->cc_result_type_valid,
                                               &get_cw_resp->cc_result_type
                                               )
           )
      {
          if((qmi_result == QMI_RESULT_SUCCESS_V01) && (get_cw_resp->service_class_valid == TRUE))
          {
            if(get_cw_resp->service_class == 0x00)
            {
              response[0] = FALSE;
              QCRIL_LOG_INFO("QCRIL QMI VOICE Query Call Waiting : Disabled for : %d",get_cw_resp->service_class);
            }
            else
            {
              response[0] = TRUE;
              QCRIL_LOG_INFO("QCRIL QMI VOICE Query Call Waiting : Enabled for : %d",get_cw_resp->service_class);
            }
            response[1] = get_cw_resp->service_class;
            if (RIL_REQUEST_QUERY_CALL_WAITING == params_ptr->event_id)
            {
               qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
               resp.resp_pkt = (void *) response;
               resp.resp_len = sizeof( response );
               qcril_send_request_response( &resp );
            }
            else
            {
               qcril_qmi_ims_translate_ril_service_status_class_to_ims_callwaitinginfo(response[0], response[1], &ims_resp);
               qcril_qmi_ims_socket_send( params_ptr->t,
                                          IMS__MSG_TYPE__RESPONSE,
                                          IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING,
                                          IMS__ERROR__E_SUCCESS,
                                          (void *)&ims_resp,
                                          sizeof(ims_resp)
                                         );
            }
          }
          else
          {
            QCRIL_LOG_INFO("QCRIL QMI VOICE Query Call Waiting RESP: FAILURE");

            if(get_cw_resp->failure_cause_valid == TRUE)
            {
              QCRIL_LOG_ERROR("QCRIL QMI VOICE Query call waiting RESP sups_failure_cause=%d, for Token ID= %d",
                  get_cw_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
              /* Send UNSOL msg with SS error code first */
              qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( get_cw_resp->failure_cause,
                  (get_cw_resp->call_id_valid ? get_cw_resp->call_id : VOICE_INVALID_CALL_ID) );
            }

            ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
            /* Send FAILURE response */
            qcril_send_empty_payload_request_response( instance_id, params_ptr->t,params_ptr->event_id, ril_err );
          }

      }
  }
  else
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }
}/* qcril_qmi_voice_query_call_waiting_resp_hdlr */


/*=========================================================================
  FUNCTION:  qcril_qmi_voice_change_call_barring_password_hdlr

===========================================================================*/
/*!
    @brief
    Handle Change Call Barring Password RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_change_call_barring_password_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_set_call_barring_password_resp_msg_v02 *change_cb_pwd_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {
      /* Entry found in the ReqList */

      QCRIL_LOG_INFO("params_ptr->data is not NULL");

      change_cb_pwd_resp = (voice_set_call_barring_password_resp_msg_v02 *)params_ptr->data;
      qmi_result = change_cb_pwd_resp->resp.result;
      qmi_error = change_cb_pwd_resp->resp.error;
      QCRIL_LOG_INFO("QCRIL QMI VOICE Change Call barring pwd, qmi_error : %d", qmi_error );

      if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                               instance_id,
                                               &change_cb_pwd_resp->resp,
                                               NULL,
                                               change_cb_pwd_resp->alpha_id_valid,
                                               &change_cb_pwd_resp->alpha_id,
                                               change_cb_pwd_resp->call_id_valid,
                                               change_cb_pwd_resp->call_id,
                                               change_cb_pwd_resp->cc_sups_result_valid,
                                               &change_cb_pwd_resp->cc_sups_result,
                                               change_cb_pwd_resp->cc_result_type_valid,
                                               &change_cb_pwd_resp->cc_result_type
                                               )
           )
      {
          if((qmi_result == QMI_RESULT_SUCCESS_V01) && ( qmi_error == QMI_ERR_NONE_V01))
          {
            qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
            qcril_send_request_response( &resp );
          }
          else
          {
            QCRIL_LOG_INFO("QCRIL QMI VOICE Change Call barring pwd: FAILURE");

            if(change_cb_pwd_resp->failure_cause_valid == TRUE)
            {
              QCRIL_LOG_ERROR("QCRIL QMI VOICE Change Call barring pwd RESP, sups_failure_cause=%d, for Token ID= %d",
                  change_cb_pwd_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
              /* Send UNSOL msg with SS error code first */
              qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp (
                  change_cb_pwd_resp->failure_cause,
                  (change_cb_pwd_resp->call_id_valid ?
                   change_cb_pwd_resp->call_id : VOICE_INVALID_CALL_ID) );
            }

            ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
            /* Send FAILURE response */
            if( ( RIL_E_GENERIC_FAILURE == ril_err ) &&
                ( TRUE == change_cb_pwd_resp->failure_cause_valid ) &&
                ( QMI_FAILURE_CAUSE_NEGATIVE_PWD_CHECK_V02 == change_cb_pwd_resp->failure_cause ) )
            {
                ril_err = RIL_E_PASSWORD_INCORRECT;
            }

            qcril_default_request_resp_params( instance_id, params_ptr->t,params_ptr->event_id, ril_err, &resp );
            qcril_send_request_response( &resp );
          }

      }
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
}/* qcril_qmi_voice_change_call_barring_password_resp_hdlr */



/*=========================================================================
  FUNCTION:  qcril_qmi_voice_query_facility_lock_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Query Facility lock RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_query_facility_lock_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_call_barring_resp_msg_v02 *get_cb_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  int response[ 1 ];

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  if( params_ptr->data != NULL )
  {
    /* Entry found in the ReqList */

    QCRIL_LOG_INFO("params_ptr->data is not NULL");

    get_cb_resp = (voice_get_call_barring_resp_msg_v02 *)params_ptr->data;
    qmi_result = get_cb_resp->resp.result;
    qmi_error = get_cb_resp->resp.error;
    QCRIL_LOG_INFO("QCRIL QMI VOICE Query Facility lock qmi_error : %d", qmi_error );

    if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                             instance_id,
                                             &get_cb_resp->resp,
                                             NULL,
                                             get_cb_resp->alpha_id_valid,
                                             &get_cb_resp->alpha_id,
                                             get_cb_resp->call_id_valid,
                                             get_cb_resp->call_id,
                                             get_cb_resp->cc_sups_result_valid,
                                             &get_cb_resp->cc_sups_result,
                                             get_cb_resp->cc_result_type_valid,
                                             &get_cb_resp->cc_result_type
                                             )
         )
    {
        if((qmi_result == QMI_RESULT_SUCCESS_V01) && (get_cb_resp->service_class_valid == TRUE))
        {
          if(get_cb_resp->service_class == 0x00)
          {
            response[0] = get_cb_resp->service_class;
            QCRIL_LOG_INFO("QCRIL QMI VOICE Query Facility Lock : Disabled for All Services, Service_class : %d",get_cb_resp->service_class);
          }
          else
          {
            response[0] = get_cb_resp->service_class;
            QCRIL_LOG_INFO("QCRIL QMI VOICE Query Facility Lock : Enabled for Service_class: %d",get_cb_resp->service_class);
          }

          if( RIL_REQUEST_QUERY_FACILITY_LOCK == params_ptr->event_id )
          {
            qcril_default_request_resp_params( instance_id,
                                               params_ptr->t,
                                               params_ptr->event_id,
                                               RIL_E_SUCCESS,
                                               &resp );
            resp.resp_pkt = (void *) &response[ 0 ];
            resp.resp_len = sizeof( response[ 0 ] );
            qcril_send_request_response( &resp );
          }
          else /* QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS */
          {
            Ims__SuppSvcResponse supp_svc_response = IMS__SUPP_SVC_RESPONSE__INIT;
            supp_svc_response.has_status = TRUE;
            supp_svc_response.status = response[0] ?\
              IMS__SERVICE_CLASS_STATUS__ENABLED : IMS__SERVICE_CLASS_STATUS__DISABLED;

            if ((get_cb_resp->cc_sups_result_valid == TRUE))
            {
              supp_svc_response.facilitytype =
                qcril_qmi_voice_map_qmi_reason_to_ims_facility\
                (get_cb_resp->cc_sups_result.reason);
              supp_svc_response.has_facilitytype =
                supp_svc_response.facilitytype ? TRUE : FALSE;
            }
            else
            {
              supp_svc_response.has_facilitytype = FALSE;
              QCRIL_LOG_INFO("QCRIL QMI VOICE Get Call Barring No Reason Type");
            }

            Ims__CbNumList *cb_num_list_ptr = NULL;
            Ims__CbNumList **cb_num_list_dptr = NULL;

            Ims__ServiceClass *serv_class_ptr = NULL;

            Ims__CbNumListType *cb_num_list_type_ptr = NULL;

            if (get_cb_resp->sc_barred_numbers_status_list_valid)
            {
              uint32_t cb_num_list_len = 0;
              uint32_t cb_num_list_idx = 0;
              uint32_t i, j;
              for (i=0; i<get_cb_resp->sc_barred_numbers_status_list_len; i++)
              {
                cb_num_list_len += get_cb_resp->sc_barred_numbers_status_list[i].call_barring_numbers_list_len;
              }
              QCRIL_LOG_INFO("cb_num_list_len: %d", cb_num_list_len);

              cb_num_list_ptr = qcril_malloc(sizeof(Ims__CbNumList) * cb_num_list_len);
              cb_num_list_dptr = qcril_malloc(sizeof(Ims__CbNumList*) * cb_num_list_len);

              serv_class_ptr = qcril_malloc(sizeof(Ims__ServiceClass) * get_cb_resp->sc_barred_numbers_status_list_len);

              cb_num_list_type_ptr = qcril_malloc(sizeof(Ims__CbNumListType) * get_cb_resp->sc_barred_numbers_status_list_len);

              supp_svc_response.cbnumlisttype = qcril_malloc(sizeof(Ims__CbNumListType*) * get_cb_resp->sc_barred_numbers_status_list_len);

              if (cb_num_list_ptr && cb_num_list_dptr && serv_class_ptr && cb_num_list_type_ptr && supp_svc_response.cbnumlisttype)
              {
                supp_svc_response.n_cbnumlisttype = get_cb_resp->sc_barred_numbers_status_list_len;

                for (i=0; i<supp_svc_response.n_cbnumlisttype; i++)
                {
                  supp_svc_response.cbnumlisttype[i] = &cb_num_list_type_ptr[i];
                  qcril_qmi_ims__cb_num_list_type__init(&cb_num_list_type_ptr[i]);

                  cb_num_list_type_ptr[i].serviceclass = &serv_class_ptr[i];
                  qcril_qmi_ims__service_class__init(&serv_class_ptr[i]);
                  serv_class_ptr[i].has_service_class = TRUE;
                  serv_class_ptr[i].service_class = get_cb_resp->sc_barred_numbers_status_list[i].service_class_ext;

                  cb_num_list_type_ptr[i].n_cb_num_list = get_cb_resp->sc_barred_numbers_status_list[i].call_barring_numbers_list_len;
                  cb_num_list_type_ptr[i].cb_num_list = &cb_num_list_dptr[cb_num_list_idx];

                  for (j=0; j<cb_num_list_type_ptr[i].n_cb_num_list; j++)
                  {
                      cb_num_list_dptr[cb_num_list_idx] = &cb_num_list_ptr[cb_num_list_idx];
                      qcril_qmi_ims__cb_num_list__init(&cb_num_list_ptr[cb_num_list_idx]);

                      cb_num_list_ptr[cb_num_list_idx].has_status = TRUE;
                      cb_num_list_ptr[cb_num_list_idx].status = get_cb_resp->sc_barred_numbers_status_list[i].call_barring_numbers_list[j].active_status;
                      cb_num_list_ptr[cb_num_list_idx].number = get_cb_resp->sc_barred_numbers_status_list[i].call_barring_numbers_list[j].barred_number;
                      cb_num_list_idx++;
                  }
                }
              }
              else
              {
                QCRIL_LOG_ERROR("malloc failed");
              }
            }
            qcril_qmi_ims_socket_send(params_ptr->t,
                                      IMS__MSG_TYPE__RESPONSE,
                                      IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS,
                                      IMS__ERROR__E_SUCCESS,
                                      (void *)&supp_svc_response,
                                      sizeof(supp_svc_response));

            if (cb_num_list_ptr)
            {
              qcril_free(cb_num_list_ptr);
            }
            if (cb_num_list_dptr)
            {
              qcril_free(cb_num_list_dptr);
            }
            if (serv_class_ptr)
            {
              qcril_free(serv_class_ptr);
            }
            if (cb_num_list_type_ptr)
            {
              qcril_free(cb_num_list_type_ptr);
            }
            if (supp_svc_response.cbnumlisttype)
            {
              qcril_free(supp_svc_response.cbnumlisttype);
            }
          }
        }
        else
        {
          QCRIL_LOG_INFO("QCRIL QMI VOICE Query Facility Lock RESP: FAILURE");

          if(get_cb_resp->failure_cause_valid == TRUE)
          {
            QCRIL_LOG_ERROR("QCRIL QMI VOICE Query Facility Lock RESP sups_failure_cause=%d, for Token ID= %d",
                get_cb_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
            if( RIL_REQUEST_QUERY_FACILITY_LOCK == params_ptr->event_id )
            {
              /* Send UNSOL msg with SS error code first */
              qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( get_cb_resp->failure_cause,
                (get_cb_resp->call_id_valid ? get_cb_resp->call_id : VOICE_INVALID_CALL_ID) );
            }
          }
          ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
          /* Send FAILURE response */
          qcril_send_empty_payload_request_response( instance_id,
                                                     params_ptr->t,
                                                     params_ptr->event_id,
                                                     ril_err );
        }
    }
  }
  else
  {
    if( RIL_REQUEST_QUERY_FACILITY_LOCK == params_ptr->event_id )
    {
      qcril_default_request_resp_params( instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         RIL_E_GENERIC_FAILURE,
                                         &resp );
      qcril_send_request_response( &resp );
    }
    else /* QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS */
    {
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
    }
  }
}/* qcril_qmi_voice_query_facility_lock_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_query_call_forward_status_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Query Call Forwarding RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_query_call_forward_status_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_call_forwarding_resp_msg_v02 *get_cf_resp;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  qcril_qmi_voice_callforwd_info_param_u_type response_buffer[ QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ ];
  qcril_qmi_voice_callforwd_info_param_u_type *response[ QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ ];
  Ims__CallForwardInfoList ims_resp = IMS__CALL_FORWARD_INFO_LIST__INIT;
  uint32_t call_forwarding_info_len = 0;
  voice_get_call_forwarding_info_type_v02 *call_fwd_info_ptr;
  uint8_t service_class;

  char * resp_num_ptr;
  uint32 interm_num_len;
  voice_get_call_forwarding_info_type_v02 *cur_fwd_info_slot;
  qcril_qmi_voice_callforwd_info_param_u_type* resp_buf_slot;
  voice_time_type_v02 *call_fwd_start_time = NULL;;
  voice_time_type_v02 *call_fwd_end_time = NULL;;

  QCRIL_LOG_FUNC_ENTRY()

  memset( (void*) response_buffer, 0, sizeof( response_buffer ) );
  memset( (void*) response, 0, sizeof( response ) );


  if( params_ptr->data != NULL )
  {
        QCRIL_LOG_INFO("params_ptr->data is not NULL");

        get_cf_resp = (voice_get_call_forwarding_resp_msg_v02 *)params_ptr->data;
        qmi_result = get_cf_resp->resp.result;
        qmi_error = get_cf_resp->resp.error;
        QCRIL_LOG_INFO("qmi result %d, qmi_error %d", qmi_result, qmi_error );

        ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( QMI_NO_ERR, &get_cf_resp->resp );
        QCRIL_LOG_INFO("resp ril err : %d", ril_err );

        if ( !qcril_qmi_voice_stk_ss_resp_handle(params_ptr,
                                                 QCRIL_DEFAULT_INSTANCE_ID,
                                                 &get_cf_resp->resp,
                                                 NULL,
                                                 get_cf_resp->alpha_id_valid,
                                                 &get_cf_resp->alpha_id,
                                                 get_cf_resp->call_id_valid,
                                                 get_cf_resp->call_id,
                                                 get_cf_resp->cc_sups_result_valid,
                                                 &get_cf_resp->cc_sups_result,
                                                 get_cf_resp->cc_result_type_valid,
                                                 &get_cf_resp->cc_result_type
                                                 )
             )
        {
              call_fwd_info_ptr = get_cf_resp->get_call_forwarding_info;
              QCRIL_LOG_INFO("cf info valid %d", (int)get_cf_resp->get_call_forwarding_info_valid );
              if ( RIL_E_SUCCESS == ril_err &&
                   get_cf_resp->get_call_forwarding_info_valid
                  )
              {
                    uint8 i=0, j=0;
                    uint32_t num_of_instances = 0;
                    call_forwarding_info_len = get_cf_resp->get_call_forwarding_info_len;
                    int instance_merged = FALSE;
                    QCRIL_LOG_INFO("cf info len %d", (int)call_forwarding_info_len );

                    for (i = 0;  i < call_forwarding_info_len && i < GET_CALL_FORWARDING_INFO_MAX_V02 && i < QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ; i++ )
                    {
                      cur_fwd_info_slot = &get_cf_resp->get_call_forwarding_info [ i ];

                      // Try to merge this instance with previous instances
                      instance_merged = FALSE;
                      for ( j = 0 ; j < num_of_instances; j++ )
                      {
                        resp_buf_slot = &response_buffer[ j ];

                        if ( resp_buf_slot->number != NULL  )
                        {
                          interm_num_len = strlen( resp_buf_slot->number );
                        }
                        else
                        {
                          interm_num_len = QMI_RIL_ZERO;
                        }
                        if ( ( interm_num_len == cur_fwd_info_slot->number_len ) &&
                             ( QMI_RIL_ZERO == cur_fwd_info_slot->number_len ||
                                   strncmp( cur_fwd_info_slot->number, resp_buf_slot->number,
                                            cur_fwd_info_slot->number_len ) == 0) &&
                             ( cur_fwd_info_slot->service_status == resp_buf_slot->status ) &&
                             ( cur_fwd_info_slot->no_reply_timer == resp_buf_slot->no_reply_timer ) )
                        {
                          // Number, status and timer are same, merge the service_class
                          resp_buf_slot->service_class |= cur_fwd_info_slot->service_class;
                          instance_merged = TRUE;
                          QCRIL_LOG_INFO( "service class 0x%x is merged with %d",
                                          cur_fwd_info_slot-->service_class, j );
                        }
                      }

                      if ( !instance_merged )
                      {
                        resp_buf_slot = &response_buffer[num_of_instances];

                        // We can't merge it. So we get a new qcril_qmi_voice_callforwd_info_param_u_type record.
                        resp_buf_slot->status = cur_fwd_info_slot->service_status;

                        service_class = cur_fwd_info_slot->service_class;
                        if ( QMI_RIL_ZERO == service_class )
                        {
                          service_class = QMI_RIL_FF;
                          QCRIL_LOG_INFO( "service_class adjusted to 0xFF for single instance" );
                        }
                        resp_buf_slot->service_class = service_class;

                        qmi_ril_enter_critical_section();
                        if ( (qcril_qmi_voice_info.last_cfw_reason > 0) && (qcril_qmi_voice_info.last_cfw_reason <= VOICE_REASON_FWD_ALLCONDITIONAL_V02) )
                        {
                          resp_buf_slot->reason = qcril_qmi_voice_map_qmi_cfw_reason_to_ril_reason(qcril_qmi_voice_info.last_cfw_reason);
                        }
                        else
                        {
                          resp_buf_slot->reason = QCRIL_QMI_VOICE_CCFC_REASON_UNCOND;
                        }
                        qmi_ril_leave_critical_section();

                        interm_num_len = cur_fwd_info_slot->number_len;
                        if ( interm_num_len > QMI_RIL_ZERO )
                        {
                          resp_num_ptr = qcril_malloc( interm_num_len + 1 );
                          if ( resp_num_ptr )
                          {
                            memcpy( resp_num_ptr, cur_fwd_info_slot->number, interm_num_len );
                            resp_num_ptr[ interm_num_len ] = QMI_RIL_ZERO;

                            resp_buf_slot->toa = ( QCRIl_QMI_VOICE_SS_TA_INTER_PREFIX == *resp_num_ptr ) ?
                                  QCRIL_QMI_VOICE_SS_TA_INTERNATIONAL : QCRIL_QMI_VOICE_SS_TA_UNKNOWN;
                            resp_buf_slot->number = resp_num_ptr;
                            QCRIL_LOG_INFO( "instance %d, response_number = %s, toa = %d", (int) num_of_instances,
                                                                                         resp_buf_slot->number,
                                                                                         (int) resp_buf_slot->toa );
                          }
                          else
                          {
                            QCRIL_LOG_ERROR("out of memory");
                            break;
                          }
                        }

                        if ( cur_fwd_info_slot->no_reply_timer != QMI_RIL_ZERO )
                        {
                          resp_buf_slot->no_reply_timer = cur_fwd_info_slot->no_reply_timer;
                          QCRIL_LOG_INFO( "no_reply_timer = %d", resp_buf_slot->no_reply_timer );
                        }

                        QCRIL_LOG_INFO("Status = %d,Service_class = %d,reason = %d",
                                       resp_buf_slot->status,
                                       resp_buf_slot->service_class,
                                       resp_buf_slot->reason);

                        response[ num_of_instances ] = &response_buffer[num_of_instances];

                        num_of_instances++;
                      }
                    }
                    QCRIL_LOG_INFO("num_of_instances: %d", num_of_instances);

                    if (get_cf_resp->call_fwd_start_time_valid)
                    {
                       call_fwd_start_time = &(get_cf_resp->call_fwd_start_time);
                    }

                    if (get_cf_resp->call_fwd_end_time_valid)
                    {
                       call_fwd_end_time = &(get_cf_resp->call_fwd_end_time);
                    }

                    if ( num_of_instances > 0 )
                    {
                      if (RIL_REQUEST_QUERY_CALL_FORWARD_STATUS == params_ptr->event_id)
                      {
                        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
                        resp.resp_pkt = (void *) response;
                        resp.resp_len = sizeof(qcril_qmi_voice_callforwd_info_param_u_type *) * num_of_instances;
                        qcril_send_request_response( &resp );
                      }
                      else
                      {
                        qcril_qmi_ims_translate_ril_callforwdinfo_to_ims_callforwdinfo(
                                response_buffer, num_of_instances,
                                call_fwd_start_time, call_fwd_end_time,
                                &ims_resp);
                        qcril_qmi_ims_socket_send( params_ptr->t,
                                                   IMS__MSG_TYPE__RESPONSE,
                                                   IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS,
                                                   IMS__ERROR__E_SUCCESS,
                                                   (void *)&ims_resp,
                                                   sizeof(ims_resp)
                                                  );

                      }

                      for ( i=0; i<num_of_instances && i <GET_CALL_FORWARDING_INFO_MAX_V02 && i < QCRIL_QMI_VOICE_CFW_RESPONSE_BUF_SZ ; i++ )
                      {
                        if (response_buffer[ i ].number )
                        {
                          qcril_free( response_buffer[ i ].number );
                        }
                      }
                    }
                    else
                    { // no numbers
                      response_buffer[ 0 ].status = FALSE;
                      /* if call forwarding is interrogated for specific service class, include only that service class in response */
                      if ( call_fwd_info_ptr[0].service_class != 0 )
                      {
                        response_buffer[ 0 ].service_class = call_fwd_info_ptr[0].service_class;
                      }
                      else
                      {
                        response_buffer[ 0 ].service_class = 0xff;
                      }
                      qmi_ril_enter_critical_section();
                      response_buffer[ 0 ].reason = qcril_qmi_voice_map_qmi_cfw_reason_to_ril_reason(qcril_qmi_voice_info.last_cfw_reason);
                      qmi_ril_leave_critical_section();
                      response[ 0 ] = &response_buffer[ 0 ];
                      if (RIL_REQUEST_QUERY_CALL_FORWARD_STATUS == params_ptr->event_id)
                      {
                        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
                        resp.resp_pkt = (void *) response;
                        resp.resp_len = sizeof( qcril_qmi_voice_callforwd_info_param_u_type * );
                        qcril_send_request_response( &resp );
                      }
                      else
                      {
                        qcril_qmi_ims_translate_ril_callforwdinfo_to_ims_callforwdinfo(
                                response_buffer, num_of_instances,
                                call_fwd_start_time, call_fwd_end_time,
                                &ims_resp);
                        qcril_qmi_ims_socket_send( params_ptr->t,
                                                   IMS__MSG_TYPE__RESPONSE,
                                                   IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS,
                                                   IMS__ERROR__E_SUCCESS,
                                                   (void *)&ims_resp,
                                                   sizeof(ims_resp)
                                                  );
                      }
                    }
            }
            else
            {
              if(get_cf_resp->failure_cause_valid == TRUE)
              {
                QCRIL_LOG_ERROR("RESP : sups_failure_cause=%d, for Token ID= %d",
                    get_cf_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));
                /* Send UNSOL msg with SS error code first */
                qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( get_cf_resp->failure_cause,
                    (get_cf_resp->call_id_valid ? get_cf_resp->call_id : VOICE_INVALID_CALL_ID) );
              }

              qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,params_ptr->event_id, ril_err );
            }
            qmi_ril_enter_critical_section();
            qcril_qmi_voice_info.last_cfw_reason = QMI_RIL_ZERO;
            qmi_ril_leave_critical_section();
        }
  }
  else
  {
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
  }

  if (ims_resp.info)
  {
    uint8 i = 0;
    for (i = 0; i < ims_resp.n_info; i++)
    {
      if (ims_resp.info[i] && ims_resp.info[i]->callfwdtimerstart)
      {
        qcril_free(ims_resp.info[i]->callfwdtimerstart);
      }
      if (ims_resp.info[i] && ims_resp.info[i]->callfwdtimerend)
      {
        qcril_free(ims_resp.info[i]->callfwdtimerend);
      }
    }
    qcril_free(ims_resp.info);
  }

  QCRIL_LOG_FUNC_RETURN();

}/* qcril_qmi_voice_query_call_forward_status_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_orig_ussd_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Orig USSD RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_orig_ussd_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_orig_ussd_resp_msg_v02 *orig_ussd_resp;
  qcril_instance_id_e_type instance_id;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;
  char ussd_utf8_str[QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2];
  char type_code[2];
  char *response_buff[2];
  int utf8_len =0;
  qcril_request_resp_params_type resp;
  qcril_unsol_resp_params_type unsol_resp;
  uint8 uss_dcs=QCRIL_QMI_VOICE_USSD_DCS_UNSPECIFIED;
  boolean success = TRUE;
  RIL_Errno ril_err;
  qcril_qmi_voice_stk_cc_modification_e_type stk_cc_modification;
  int i=0;

  qcril_request_resp_params_type ril_response;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  do
  {
    if( params_ptr->data != NULL )
    {
      /* Entry found in the ReqList */

      QCRIL_LOG_INFO("params_ptr->data is not NULL");

      memset( ussd_utf8_str, '\0', sizeof( ussd_utf8_str ) );
      memset( type_code, '\0', sizeof( type_code ) );
      memset( response_buff, 0, sizeof( response_buff ) );

      orig_ussd_resp = (voice_orig_ussd_resp_msg_v02 *)params_ptr->data;
      qmi_result = orig_ussd_resp->resp.result;
      qmi_error = orig_ussd_resp->resp.error;
      QCRIL_LOG_INFO("QCRIL QMI VOICE Orig USSD Response qmi_error : %d", qmi_error );

      ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex( QMI_NO_ERR,
                                                                            &orig_ussd_resp->resp,
                                                                            QCRIL_QMI_ERR_CTX_SEND_USSD_TXN,
                                                                            orig_ussd_resp );
      QCRIL_LOG_INFO("ORIG USSD RESP : ril_err=%d, orig_ussd_resp_result=%d, orig_ussd_resp_error=%d, cc_sups_result_valid=%d, cc_result_type_valid=%d, cc_result_type=%d",
                     (int)ril_err,
                     (int)orig_ussd_resp->resp.result,
                     (int)orig_ussd_resp->resp.error,
                     (int)orig_ussd_resp->cc_sups_result_valid,
                     (int)orig_ussd_resp->cc_result_type_valid,
                     (int)orig_ussd_resp->cc_result_type
                     );

      if( ril_err && orig_ussd_resp->cc_sups_result_valid)
      {
        QCRIL_LOG_INFO("Error Details : cc_sups_result_reason=%d, cc_sups_result_service=%d",orig_ussd_resp->cc_sups_result.reason,
                       orig_ussd_resp->cc_sups_result.service_type);
      }
      switch ( (int)ril_err )
      {
        case RIL_E_USSD_MODIFIED_TO_DIAL:
          stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_USSD_TO_DIAL;
          break;

        case RIL_E_USSD_MODIFIED_TO_USSD:
          stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_USSD_TO_USSD;
          break;

        case RIL_E_USSD_MODIFIED_TO_SS:
          stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_USSD_TO_SS;
          break;

        default:
          stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE;
          if (ril_err != RIL_E_SUCCESS && ril_err != RIL_E_FDN_CHECK_FAILURE)
          {
            QCRIL_LOG_INFO("ril_err(%d) convert to RIL_E_GENERIC_FAILURE", ril_err);
            ril_err = RIL_E_GENERIC_FAILURE;
          }
          break;
      }

      // QMI_VOICE can send valid Alpha in case of QMI_ERR_CARD_CALL_CONTROL_FAILED.
      // Handling Alpha for both success and failure cases.
      if ( orig_ussd_resp->alpha_id_valid )
      {
        stk_cc_info.alpha_ident = orig_ussd_resp->alpha_id;
      }
      else
      {
        memset( &stk_cc_info.alpha_ident, 0, sizeof( stk_cc_info.alpha_ident ) );
      }

      if ( QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE != stk_cc_modification )
      { // STK CC session started
        qcril_qmi_voice_reset_stk_cc();

        stk_cc_info.modification                      = stk_cc_modification;
        stk_cc_info.is_alpha_relayed                  = FALSE;

        if ( orig_ussd_resp->call_id_valid )
        {
          stk_cc_info.call_id_info = orig_ussd_resp->call_id;
        }

        if ( orig_ussd_resp->cc_sups_result_valid )
        {
          stk_cc_info.ss_ussd_info = orig_ussd_resp->cc_sups_result;
        }

        QCRIL_LOG_INFO( "org req altered. ril_err: %d, call_id: %d", (int)ril_err, (int)orig_ussd_resp->call_id );

        qcril_qmi_voice_stk_cc_dump();
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_err, &ril_response );
        qcril_send_request_response( &ril_response );
      }
      else
      {
        if(orig_ussd_resp->failure_cause_valid == TRUE)
        {
          /* Send UNSOL msg with SS error code first */
          qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp ( orig_ussd_resp->failure_cause,
              (orig_ussd_resp->call_id_valid ? orig_ussd_resp->call_id : VOICE_INVALID_CALL_ID) );

          QCRIL_LOG_ERROR("QCRIL QMI VOICE Orig USSD RESP sups_failure_cause=%d, for Token ID= %d",
              orig_ussd_resp->failure_cause, qcril_log_get_token_id( params_ptr->t ));

          if( (orig_ussd_resp->failure_cause == QMI_FAILURE_CAUSE_FACILITY_REJECTED_V02) ||
              (orig_ussd_resp->failure_cause == QMI_FAILURE_CAUSE_REJECTED_BY_NETWORK_V02 ) )
          {
            qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
            qcril_send_request_response( &resp );
            break;
          }
        }

        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err, &resp );
        qcril_send_request_response( &resp );
        if (ril_err != RIL_E_SUCCESS)
        {
          QCRIL_LOG_ERROR("Failure response for SEND_USSD!! : ril_err=%d\n", ril_err);
          break;
        }

        /* send RIL_UNSOL_STK_CC_ALPHA_NOTIFY to Telephony if Alpha is valid */
        qcril_qmi_voice_stk_cc_relay_alpha_if_necessary(QCRIL_DEFAULT_INSTANCE_ID, TRUE);

        if( (qmi_result == QMI_RESULT_SUCCESS_V01) &&
            ( (orig_ussd_resp->uss_info_valid == TRUE) || (orig_ussd_resp->uss_info_utf16_valid == TRUE) )
          )
        {
          if ( orig_ussd_resp->uss_info_utf16_valid == TRUE ) // using uss_info_utf16 instead of uss_info if it is available
          {
            QCRIL_LOG_ERROR ("USSD Orig resp, utf16 len=%d", orig_ussd_resp->uss_info_utf16_len );

            utf8_len = qcril_cm_ss_convert_ucs2_to_utf8( (char *) orig_ussd_resp->uss_info_utf16, orig_ussd_resp->uss_info_utf16_len * 2, ussd_utf8_str );
            for(i=0 ; i< utf8_len ; i++ )
            {
              QCRIL_LOG_DEBUG ("utf8 data bytes : %x\n", ussd_utf8_str[ i ]);
            }
            if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
            {
              QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR");
              utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
              ussd_utf8_str[ utf8_len - 1] = '\0';
            }
          }
          else
          {
            QCRIL_LOG_ERROR ("USSD Orig resp dcs=%d , len=%d", orig_ussd_resp->uss_info.uss_dcs,orig_ussd_resp->uss_info.uss_data_len );
            switch(orig_ussd_resp->uss_info.uss_dcs)
            {
              case USS_DCS_ASCII_V02 :
                utf8_len = qcril_cm_ss_ascii_to_utf8((unsigned char *)orig_ussd_resp->uss_info.uss_data, orig_ussd_resp->uss_info.uss_data_len,
                                                     ussd_utf8_str, sizeof(ussd_utf8_str));
                break;
              case USS_DCS_8BIT_V02 :
                uss_dcs = QCRIL_QMI_VOICE_USSD_DCS_8_BIT;
                utf8_len = qcril_cm_ss_convert_ussd_string_to_utf8( uss_dcs,
                                                                    orig_ussd_resp->uss_info.uss_data_len,
                                                                    orig_ussd_resp->uss_info.uss_data,
                                                                    ussd_utf8_str );
                if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
                {
                  QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR");
                  utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
                  ussd_utf8_str[ utf8_len - 1] = '\0';
                }
                break;
              case USS_DCS_UCS2_V02 :
                uss_dcs = QCRIL_QMI_VOICE_USSD_DCS_UCS2;
                utf8_len = qcril_cm_ss_convert_ussd_string_to_utf8( uss_dcs,
                                                                    orig_ussd_resp->uss_info.uss_data_len,
                                                                    orig_ussd_resp->uss_info.uss_data,
                                                                    ussd_utf8_str );
                if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
                {
                  QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR");
                  utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
                  ussd_utf8_str[ utf8_len - 1] = '\0';
                }
                break;
              default :
                QCRIL_LOG_ERROR ("Invalid USSD dcs : %d", orig_ussd_resp->uss_info.uss_dcs );
                success = FALSE;
                break;
            }
          }

          if ( success )
          {
            type_code[ 0 ] = '0';  /* QCRIL_CM_SS_CUSD_RESULT_DONE */
            type_code[ 1 ] = '\0';
            response_buff[ 0 ] = type_code;

            if ( orig_ussd_resp->uss_info.uss_data_len > 0)
            {
              response_buff[ 1 ] = ussd_utf8_str;
            }
            else
            {
              response_buff[ 1 ] = NULL;
            }

            /* Sending the response received from the network for the USSD request */
            QCRIL_LOG_DEBUG ( "USSD Conf Success, data_len : %d", orig_ussd_resp->uss_info.uss_data_len );
            QCRIL_LOG_DEBUG ("USSD : type_code=%s",type_code);
            if ( NULL != response_buff[ 1 ] )
            {
              QCRIL_LOG_DEBUG ("USSD : response_buff[1]=%s",response_buff[ 1 ]);
              QCRIL_LOG_DEBUG ("USSD : strlen=%d",strlen(response_buff[ 1 ]));
            }
            else
            {
              QCRIL_LOG_DEBUG ("USSD : response_buff[1] is NULL");
            }
          }
          else
          {
             QCRIL_LOG_DEBUG ("USSD abort");
             type_code[ 0 ] = '2';  /* QCRIL_CM_SS_CUSD_RESULT_ABORT */
             type_code[ 1 ] = '\0';
             response_buff[ 0 ] = type_code;
             response_buff[ 1 ] = NULL;
          }

          qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_ON_USSD, &unsol_resp );
          unsol_resp.resp_pkt = (void *) response_buff;
          unsol_resp.resp_len = sizeof( response_buff );
          unsol_resp.logstr = NULL;
          qcril_send_unsol_response( &unsol_resp );
        }
        else
        {
          /* sending the unsol indication so that RIL can close the USSD session */
          if ( (orig_ussd_resp->failure_cause_valid == TRUE) &&
             (orig_ussd_resp->failure_cause == QMI_FAILURE_CAUSE_FACILITY_NOT_SUPPORTED_V02) ) /*facilityNotSupported*/
          {
            type_code[ 0 ] = '4';  /*QCRIL_CM_SS_CUSD_RESULT_NOSUP */
            type_code[ 1 ] = '\0';
          }
          else
          {
            type_code[ 0 ] = '2';  /* QCRIL_CM_SS_CUSD_RESULT_ABORT */
            type_code[ 1 ] = '\0';
          }
          response_buff[ 0 ] = type_code;
          response_buff[ 1 ] = NULL;
          QCRIL_LOG_DEBUG ("USSD Failure: type_code=%s",type_code);
          qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_ON_USSD, &unsol_resp );
          unsol_resp.resp_pkt = (void *) response_buff;
          unsol_resp.resp_len = sizeof( response_buff );
          unsol_resp.logstr = NULL;
          qcril_send_unsol_response( &unsol_resp );
        }
      }
    }
    else
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }while(0);
}/* qcril_qmi_voice_orig_ussd_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_answer_ussd_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Answer USSD RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_answer_ussd_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_answer_ussd_resp_msg_v02 *ans_ussd_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {

  /* Entry found in the ReqList */

  QCRIL_LOG_INFO("params_ptr->data is not NULL");

  ans_ussd_resp = (voice_answer_ussd_resp_msg_v02 *)params_ptr->data;
  qmi_result = ans_ussd_resp->resp.result;
  qmi_error = ans_ussd_resp->resp.error;
  QCRIL_LOG_INFO("QCRIL QMI VOICE Answer USSD qmi_error : %d", qmi_error );

  if(qmi_result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QCRIL QMI VOICE Answer USSD Success: user_act_req = %d",qcril_qmi_voice_info.ussd_user_action_required);
    qcril_qmi_voice_info.ussd_user_action_required = FALSE;
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QCRIL QMI VOICE Answer USSD Failure: user_act_req = %d",qcril_qmi_voice_info.ussd_user_action_required);
    qcril_qmi_voice_info.ussd_user_action_required = FALSE;
    ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
    /* Send FAILURE response */
    qcril_default_request_resp_params( instance_id, params_ptr->t,params_ptr->event_id, ril_err, &resp );
    qcril_send_request_response( &resp );
  }
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

}/* qcril_qmi_voice_answer_ussd_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_cancel_ussd_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle Cancel USSD RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_cancel_ussd_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_cancel_ussd_resp_msg_v02 *cancel_ussd_resp;
  qcril_instance_id_e_type instance_id;
  RIL_Errno ril_err;
  qcril_request_resp_params_type resp;
  qmi_result_type_v01 qmi_result;
  qmi_error_type_v01  qmi_error;


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  if( params_ptr->data != NULL )
  {
  /* Entry found in the ReqList */

  QCRIL_LOG_INFO("params_ptr->data is not NULL");

  cancel_ussd_resp = (voice_cancel_ussd_resp_msg_v02 *)params_ptr->data;
  qmi_result = cancel_ussd_resp->resp.result;
  qmi_error = cancel_ussd_resp->resp.error;
  QCRIL_LOG_INFO("QCRIL QMI VOICE Cancel USSD qmi_error : %d", qmi_error );

  if(qmi_result == QMI_RESULT_SUCCESS_V01)
  {
    QCRIL_LOG_INFO("QCRIL QMI VOICE Cancel USSD Success: user_act_req = %d",qcril_qmi_voice_info.ussd_user_action_required);
    qcril_qmi_voice_info.ussd_user_action_required = FALSE;
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
    QCRIL_LOG_INFO("QCRIL QMI VOICE Cancel USSD Failure: user_act_req = %d",qcril_qmi_voice_info.ussd_user_action_required);
    qcril_qmi_voice_info.ussd_user_action_required = FALSE;
    ril_err = qcril_qmi_client_map_qmi_err_to_ril_err(qmi_error);
    /* Send FAILURE response */
    qcril_default_request_resp_params( instance_id, params_ptr->t,params_ptr->event_id, ril_err, &resp );
    qcril_send_request_response( &resp );
  }
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }

}/*qcril_qmi_voice_cancel_ussd_resp_hdlr*/

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_dial

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DIAL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_dial
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  RIL_Dial *in_data_ptr = NULL;
  Ims__Dial *ims_in_data_ptr = NULL;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_dial_call_req_msg_v02  dial_call_req_msg;
  int is_emergency_call;
  int is_non_std_otasp;
  int is_conf_uri = FALSE;
  errno_enum_type req_res;

  RIL_LastCallFailCause last_call_fail_cause;

  uint32_t                                    nw_reg_status_overview;
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  RIL_Errno                                   call_setup_result;
  qmi_client_error_type                       qmi_client_error;

  qmi_txn_handle                              txn_handle;

  char * subaddr = NULL;
  int subaddr_len = 0;
  int addr_len = 0;
  int clir = QCRIL_QMI_VOICE_SS_CLIR_PRESENTATION_INDICATOR;
  RIL_UUS_Info *  uusInfo = NULL;    /* NULL or Pointer to User-User Signaling Information */
  RIL_Call_Details *callDetails = NULL;
  char *extras_str = NULL;
  char *token = NULL, *lastPtr = NULL;
  char *displayText = NULL;
  int i=0;

  int need_enforce_emergency_directly;

  qmi_ril_voice_ims_command_exec_oversight_type*                      command_oversight;
  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type   oversight_cmd_params;

  char voice_dial_address[QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN];
  qcril_qmi_voice_emer_num_ims_addr_info_type emer_num_ims_addr_info;
  int is_emer_num_to_ims_addr = FALSE;

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  memset(voice_dial_address, 0, sizeof(voice_dial_address));
  memset(&emer_num_ims_addr_info, 0, sizeof(emer_num_ims_addr_info));

  if (NULL == params_ptr || NULL == params_ptr->data)
  {
    QCRIL_LOG_INFO("params_ptr or params_ptr is NULL");
    return;
  }

  if ( RIL_REQUEST_DIAL == params_ptr->event_id )
  {
    in_data_ptr = (RIL_Dial *)params_ptr->data;
    memcpy(voice_dial_address, in_data_ptr->address, strlen(in_data_ptr->address));
    clir = in_data_ptr->clir;
    uusInfo = in_data_ptr->uusInfo;
    if( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
    {
      callDetails = VT_VOLTE_CALL_DETAILS( in_data_ptr->callDetails );
    }
  }
  else
  {
    ims_in_data_ptr = (Ims__Dial *)params_ptr->data;
    memcpy(voice_dial_address, ims_in_data_ptr->address, strlen(ims_in_data_ptr->address));
    if ( ims_in_data_ptr->has_clir )
    {
      clir = ims_in_data_ptr->clir;
    }
    if ( ims_in_data_ptr->calldetails )
    {
      callDetails = qcril_malloc(sizeof(*callDetails));
      if ( callDetails )
      {
        callDetails->callDomain = ims_in_data_ptr->calldetails->calldomain;
        callDetails->callType   = ims_in_data_ptr->calldetails->calltype;
        callDetails->extrasLength = ims_in_data_ptr->calldetails->extraslength;
        callDetails->n_extras = ims_in_data_ptr->calldetails->n_extras;
        callDetails->extras     = (const char **)ims_in_data_ptr->calldetails->extras;
        QCRIL_LOG_INFO("call domain: %d, call type: %d", callDetails->callDomain, callDetails->callType);

        for(i=0; i<callDetails->n_extras; i++)
        {
          extras_str = (char*)callDetails->extras[i];
          token = strtok_r(extras_str, "=", &lastPtr);
          if (token == NULL)
              continue;

          if(0 == strcmp(token, "DisplayText"))
          {
            //Retrieve the remaining string after DisplayText=<str>
            displayText = strtok_r(NULL, "\0", &lastPtr);
            QCRIL_LOG_INFO("%s: %s", token, displayText);
          }
        }

      }
    }

    if (ims_in_data_ptr->has_isconferenceuri)
    {
      is_conf_uri = ims_in_data_ptr->isconferenceuri;
    }
    QCRIL_LOG_INFO("is_conf_uri: %d", is_conf_uri);
  }

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  is_emergency_call = FALSE;
  need_enforce_emergency_directly = FALSE;
  unsigned int escv_type = 0;

  if (!is_conf_uri)
  {
    int is_number_part_of_ril_ecclist = qmi_ril_phone_number_is_emergency( voice_dial_address );
    int is_number_emergency_for_display_purpose_only = qmi_ril_nwreg_is_designated_number_emergency_for_display_purposes_only( voice_dial_address );
    if( TRUE == is_number_part_of_ril_ecclist && FALSE == is_number_emergency_for_display_purpose_only )
    {
      is_emergency_call = TRUE;
      need_enforce_emergency_directly = qmi_ril_nwreg_is_designated_number_enforcable_ncl( voice_dial_address );

      if( QCRIL_EVT_IMS_SOCKET_REQ_DIAL == params_ptr->event_id &&
          TRUE == need_enforce_emergency_directly )
      {//IMS DIAL request - Convert the emergency number to IMS address If needed
        strlcpy(emer_num_ims_addr_info.emergency_number,
                voice_dial_address,
                sizeof(emer_num_ims_addr_info.emergency_number));

        is_emer_num_to_ims_addr = qmi_ril_nwreg_convert_emergency_number_to_ims_address(voice_dial_address);

        strlcpy(emer_num_ims_addr_info.ims_address,
                voice_dial_address,
                sizeof(emer_num_ims_addr_info.ims_address));
      }
    }
  }
  QCRIL_LOG_ESSENTIAL(".. is_emergency %d", is_emergency_call);
  QCRIL_LOG_ESSENTIAL(".. need_enforce_emergency_directly %d", need_enforce_emergency_directly );
  QCRIL_LOG_ESSENTIAL(".. is_emer_num_to_ims_addr %d", is_emer_num_to_ims_addr );

  qcril_qmi_voice_voip_lock_overview();
  nw_reg_status_overview = qmi_ril_nw_reg_get_status_overview();
  QCRIL_LOG_INFO(".. nw reg status overview %d", (int)nw_reg_status_overview);
  call_info_entry        = NULL;
  call_setup_result      = RIL_E_GENERIC_FAILURE;
  command_oversight      = NULL;
  do
  {
      if( TRUE == is_emergency_call )
      {
        call_setup_result = qcril_qmi_nas_voice_move_device_to_online_for_emer_call_conditionally();
        if( RIL_E_SUCCESS != call_setup_result )
        {
          break;
        }
      }

      if ( qmi_ril_voice_is_voice_calls_supressed ||
           ( qmi_ril_voice_is_calls_supressed_by_pil_vcl() && !is_emergency_call )
         )
      {
        call_setup_result = RIL_E_GENERIC_FAILURE;
        break;
      }

      if ( params_ptr->datalen == 0 || params_ptr->data == NULL )
      {
        call_setup_result = RIL_E_GENERIC_FAILURE;
        break;
      }

      // check address/subaddress length overflow
      addr_len = strlen( voice_dial_address );

      if( addr_len > 0 )
      {
        if ( feature_subaddress_support_amp )
        {
          subaddr = strchr(voice_dial_address,'&');
        }

        if ( feature_subaddress_support && '*' != voice_dial_address[0] && subaddr == NULL )
        {
           subaddr = strchr(voice_dial_address, '*');
        }

        if (subaddr)
        {
           subaddr++;
           subaddr_len = addr_len - (subaddr - voice_dial_address);
           if (subaddr_len > 0)
           {

              const uint SUBADDR_LEN_MAX = QMI_VOICE_SUBADDRESS_LEN_MAX_V02 - 2; // by the limitation on QMI VOICE, need to reserve two bytes for them to add padding 0x50 and the NULL terminator
              if ( ((unsigned int)subaddr_len) > SUBADDR_LEN_MAX )
              {
                 QCRIL_LOG_ERROR("subaddr_len: %d is greater than SUBADDRESS_LEN_MAX(%d)",
                                 subaddr_len, SUBADDR_LEN_MAX );
                 call_setup_result = RIL_E_GENERIC_FAILURE;
                 break;
              }
              else
              {
                 addr_len = addr_len - 1 - subaddr_len;
              }
           }
        }
      }

      if ( addr_len == 0  && !is_conf_uri )
      {
        QCRIL_LOG_ERROR("Calling number is null");
        call_setup_result = RIL_E_GENERIC_FAILURE;
        break;
      }


      if (is_conf_uri)
      {
        if ( addr_len > QMI_VOICE_CONF_URI_LIST_MAX_LEN_V02 )
        {
          call_setup_result = RIL_E_GENERIC_FAILURE;
          break;
        }
      }
      else if( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
      {
         /* In case of VoIP/VT, number(URI) can be larger than QMI_VOICE_NUMBER_MAX_V02 */
         if ( addr_len > ( QMI_VOICE_NUMBER_MAX_V02 + QMI_VOICE_SIP_URI_OVERFLOW_MAX_V02 ) )
         {
           call_setup_result = RIL_E_GENERIC_FAILURE;
           break;
         }
      }
      else
      {
         if ( addr_len > QMI_VOICE_NUMBER_MAX_V02 )
         {
           call_setup_result = RIL_E_GENERIC_FAILURE;
           break;
         }
      }

      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        call_setup_result = RIL_E_GENERIC_FAILURE;
        break;
      }

      call_info_entry = qcril_qmi_voice_voip_create_call_info_entry(
              VOICE_INVALID_CALL_ID,
              INVALID_MEDIA_ID,
              TRUE,
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP );
      if ( NULL == call_info_entry )
      {
        call_setup_result = RIL_E_GENERIC_FAILURE;
        break;
      }

      call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_QMI_ID_RECEIVED;

      call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_RIL_CALL_STATE_VALID;
      call_info_entry->ril_call_state = RIL_CALL_DIALING;

      if (!is_conf_uri)
      {
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;
        call_info_entry->voice_svc_remote_party_number.number_pi = PRESENTATION_NUM_ALLOWED_V02;
        call_info_entry->voice_svc_remote_party_number.number_len = (addr_len < QMI_VOICE_NUMBER_MAX_V02) ? addr_len : QMI_VOICE_NUMBER_MAX_V02;
        memcpy(call_info_entry->voice_svc_remote_party_number.number, voice_dial_address, call_info_entry->voice_svc_remote_party_number.number_len);

        call_info_entry->voice_scv_info.is_mpty = FALSE;
      }
      else
      {
        call_info_entry->voice_scv_info.is_mpty = TRUE;
      }

      call_info_entry->voice_scv_info.direction = CALL_DIRECTION_MO_V02;

      call_info_entry->voice_scv_info.als = ALS_LINE1_V02;

      if( TRUE == is_emer_num_to_ims_addr )
      {
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMER_NUM_TO_IMS_ADDR;
        memcpy(&call_info_entry->emer_num_ims_addr_info,
               &emer_num_ims_addr_info,
               sizeof(call_info_entry->emer_num_ims_addr_info));
      }

      if ( RIL_REQUEST_DIAL == params_ptr->event_id ||
           QCRIL_EVT_IMS_SOCKET_REQ_DIAL == params_ptr->event_id )
      {
        command_oversight = qmi_ril_voice_ims_create_command_oversight( params_ptr->t, RIL_REQUEST_DIAL, TRUE );
        if ( NULL != command_oversight )
        {
          qmi_ril_voice_ims_command_oversight_add_call_link( command_oversight,
                                                             QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN,
                                                             QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP,
                                                             VOICE_INVALID_CALL_ID,
                                                             CALL_STATE_ORIGINATING_V02 );

          memset( &oversight_cmd_params, 0, sizeof( oversight_cmd_params ) );
          oversight_cmd_params.locator.command_oversight = command_oversight;

          qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_COMMENCE_AWAIT_RESP_IND,
                                                            QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                                            &oversight_cmd_params );
        } // if ( NULL != command_oversight )
        else
          break;
      } // if ( RIL_REQUEST_DIAL == params_ptr->event_id )


      if ( is_emergency_call &&
           ( ( nw_reg_status_overview & QMI_RIL_NW_REG_VOICE_CALLS_AVAILABLE ) &&
           !(nw_reg_status_overview & QMI_RIL_NW_REG_FULL_SERVICE) )
          )
      {
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS |
                                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING;
      }

      if ( is_emergency_call )
      {
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL;
      }

      if ( !is_emergency_call && !(nw_reg_status_overview & QMI_RIL_NW_REG_FULL_SERVICE) )
      { // non emergency ialed from OOS (specific to certain customer extensions)
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAL_FROM_OOS |
                                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING;
      }

      unsigned int old_call_radio_tech = qcril_qmi_voice_nas_control_get_reported_voice_radio_tech();;
      unsigned int old_call_radio_tech_family = qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family(old_call_radio_tech);

      is_non_std_otasp = qmi_ril_phone_number_is_non_std_otasp( voice_dial_address ) && (RADIO_TECH_3GPP2 == old_call_radio_tech_family);
      QCRIL_LOG_INFO(".. is_non_std_otasp %d", is_non_std_otasp);

      memset(&dial_call_req_msg, 0, sizeof(dial_call_req_msg));


      QCRIL_LOG_INFO(".. Number sent %s",voice_dial_address);

      if (!is_conf_uri)
      {
        /* Copy the Calling address and subaddress*/
        if ( NULL == subaddr || 0 == subaddr_len )
        {
          memcpy(dial_call_req_msg.calling_number, voice_dial_address, addr_len);
        }
        else
        {
          // address
          memcpy(dial_call_req_msg.calling_number, voice_dial_address, addr_len);

          // subaddress
          dial_call_req_msg.called_party_subaddress_valid = TRUE;
          dial_call_req_msg.called_party_subaddress.extension_bit = 1; // Always set to 1 according to spec Table 10.5.119/3GPP TS 24.008
          dial_call_req_msg.called_party_subaddress.subaddress_type = SUBADDRESS_TYPE_NSAP_V02;
          dial_call_req_msg.called_party_subaddress.odd_even_ind = subaddr_len % 2;
          if(feature_subaddress_ia5_id_support)
          {
            dial_call_req_msg.called_party_subaddress.subaddress_len = subaddr_len + 1;
            memset(&dial_call_req_msg.called_party_subaddress.subaddress[0],'\0',QMI_VOICE_SUBADDRESS_LEN_MAX_V02);
            dial_call_req_msg.called_party_subaddress.subaddress[0] = QCRIL_QMI_VOICE_SUBADDRESS_IA5_IDENTIFIER;
            memcpy(&dial_call_req_msg.called_party_subaddress.subaddress[1], subaddr, subaddr_len);
          }
          else
          {
            memcpy( dial_call_req_msg.called_party_subaddress.subaddress, subaddr, subaddr_len);
            dial_call_req_msg.called_party_subaddress.subaddress_len = subaddr_len;
          }
        }
      }
      else
      {
        memcpy(dial_call_req_msg.calling_number, "Conference Call", strlen("Conference Call"));
        /* Copy the conf_uri_list */
        dial_call_req_msg.conf_uri_list_valid = TRUE;
        memcpy(dial_call_req_msg.conf_uri_list, voice_dial_address, addr_len);
      }

      if( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
      {
         /* In case of VoIP/VT, number(URI) can be greater than QMI_VOICE_NUMBER_MAX_V02,
              in which copy the remaining to URI tlv */
         if( addr_len > QMI_VOICE_NUMBER_MAX_V02 )
         {
            memcpy(dial_call_req_msg.sip_uri_overflow, voice_dial_address + QMI_VOICE_NUMBER_MAX_V02, addr_len - QMI_VOICE_NUMBER_MAX_V02 );
            dial_call_req_msg.sip_uri_overflow_valid = TRUE;
         }
      }

      if ( is_non_std_otasp )
      {
        dial_call_req_msg.call_type_valid = TRUE;
        dial_call_req_msg.call_type       = CALL_TYPE_NON_STD_OTASP_V02;

        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
      }
      else
      {

        /* Set the clir type */
        /* Use CLIR setting specified in DIAL request */
        if ( clir == QCRIL_QMI_VOICE_SS_CLIR_INVOCATION_OPTION )
        {
            dial_call_req_msg.clir_type_valid = TRUE;
            dial_call_req_msg.clir_type = CLIR_INVOCATION_V02;
        }
        /* Use CLIR setting specified in DIAL request */
        else if ( clir == QCRIL_QMI_VOICE_SS_CLIR_SUPPRESSION_OPTION )
        {
            dial_call_req_msg.clir_type_valid = TRUE;
            dial_call_req_msg.clir_type = CLIR_SUPPRESSION_V02;
        }
        /* Use the default CLIR setting */
        else
        {
          clir = qcril_qmi_voice_info.clir;

          if ( clir == QCRIL_QMI_VOICE_SS_CLIR_INVOCATION_OPTION )
          {
            dial_call_req_msg.clir_type_valid = TRUE;
            dial_call_req_msg.clir_type = CLIR_INVOCATION_V02;
          }
          else if ( clir == QCRIL_QMI_VOICE_SS_CLIR_SUPPRESSION_OPTION )
          {
            dial_call_req_msg.clir_type_valid = TRUE;
            dial_call_req_msg.clir_type = CLIR_SUPPRESSION_V02;
          }
          else
          {
            dial_call_req_msg.clir_type_valid = FALSE;
          }
        }

        QCRIL_LOG_INFO(".. Clir type sent %d",dial_call_req_msg.clir_type);

        if ( ( QCRIL_EVT_IMS_SOCKET_REQ_DIAL == params_ptr->event_id )  && dial_call_req_msg.clir_type_valid )
        {
           dial_call_req_msg.pi_valid = TRUE;
           dial_call_req_msg.pi = ( dial_call_req_msg.clir_type == CLIR_INVOCATION_V02 ) ? IP_PRESENTATION_NUM_RESTRICTED_V02 : IP_PRESENTATION_NUM_ALLOWED_V02;
        }

        /* If not set call type will be assumed to be VOICE ---- but we set it anyway */
        dial_call_req_msg.call_type_valid = TRUE;

        if( callDetails )
        {
           qcril_qmi_voice_get_modem_call_type_info( callDetails,
                                               &dial_call_req_msg.call_type,
                                               &dial_call_req_msg.audio_attrib_valid,
                                               &dial_call_req_msg.audio_attrib,
                                               &dial_call_req_msg.video_attrib_valid,
                                               &dial_call_req_msg.video_attrib );

           if (RIL_CALL_DOMAIN_AUTOMATIC == callDetails->callDomain &&
               (RIL_CALL_TYPE_VOICE == callDetails->callType || CALL_TYPE_VT_V02 == dial_call_req_msg.call_type) )
           {
             call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN;
           }
           else if ( ( CALL_TYPE_VOICE_IP_V02 == dial_call_req_msg.call_type ) ||
                     ( CALL_TYPE_VT_V02 == dial_call_req_msg.call_type ) ||
                     ( CALL_TYPE_EMERGENCY_IP_V02 == dial_call_req_msg.call_type ) )
           {
             call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
           }
           else
           {
             call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
           }

           if (CALL_TYPE_VT_V02 == dial_call_req_msg.call_type)
           {
              call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUDIO_ATTR_VALID;
              call_info_entry->voice_audio_attrib.call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
              call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VIDEO_ATTR_VALID;
              call_info_entry->voice_video_attrib.call_attributes = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
           }
        }
        else
        {
           dial_call_req_msg.call_type   = CALL_TYPE_VOICE_V02;
           call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN;
        }

        if (qcril_qmi_voice_info.jbims)
        {
           /* set service type for dial request only if domain is other than PS (CS/AUTO)*/
           if (!(call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN))
           {
              dial_call_req_msg.service_type_valid = TRUE;
              dial_call_req_msg.service_type =
                qcril_qmi_nas_setting_srv_type_based_on_elaboration_and_rat(call_info_entry->elaboration);
           }
        }

        /* Set the UUS Info if present */
        if ( uusInfo != NULL )
        {
          dial_call_req_msg.uus_valid = TRUE;

          if( uusInfo->uusData != NULL )
          {
             dial_call_req_msg.uus.uus_data_len = uusInfo->uusLength;
             memcpy(dial_call_req_msg.uus.uus_data, uusInfo->uusData, uusInfo->uusLength);
          }

          switch ( uusInfo->uusType )
          {
            case RIL_UUS_TYPE1_IMPLICIT:
              dial_call_req_msg.uus.uus_type = UUS_TYPE1_IMPLICIT_V02;
              break;

            case RIL_UUS_TYPE1_REQUIRED:
              dial_call_req_msg.uus.uus_type = UUS_TYPE1_REQUIRED_V02;
              break;

            case RIL_UUS_TYPE1_NOT_REQUIRED:
              dial_call_req_msg.uus.uus_type = UUS_TYPE1_NOT_REQUIRED_V02;
              break;

            case RIL_UUS_TYPE2_REQUIRED:
              dial_call_req_msg.uus.uus_type = UUS_TYPE2_REQUIRED_V02;
              break;

            case RIL_UUS_TYPE2_NOT_REQUIRED:
              dial_call_req_msg.uus.uus_type = UUS_TYPE2_NOT_REQUIRED_V02;
              break;

            case RIL_UUS_TYPE3_REQUIRED:
              dial_call_req_msg.uus.uus_type = UUS_TYPE3_REQUIRED_V02;
              break;

            case RIL_UUS_TYPE3_NOT_REQUIRED:
              dial_call_req_msg.uus.uus_type = UUS_TYPE3_NOT_REQUIRED_V02;
              break;

            default:
              dial_call_req_msg.uus.uus_type = UUS_TYPE_DATA_V02;
              break;
          }
          switch ( uusInfo->uusDcs )
          {
            case RIL_UUS_DCS_USP:
              dial_call_req_msg.uus.uus_dcs  = UUS_DCS_USP_V02;
              break;

            case RIL_UUS_DCS_OSIHLP:
              dial_call_req_msg.uus.uus_dcs  = UUS_DCS_OHLP_V02;
              break;

            case RIL_UUS_DCS_X244:
              dial_call_req_msg.uus.uus_dcs  = UUS_DCS_X244_V02;
              break;

            case RIL_UUS_DCS_IA5c:
              dial_call_req_msg.uus.uus_dcs  = UUS_DCS_IA5_V02;
              break;

            case RIL_UUS_DCS_RMCF:  // todo: mapping
            default:
              break;
          }

        QCRIL_LOG_INFO("..  UUS info sent type %d, dcs %d, length %d",
                             dial_call_req_msg.uus.uus_type, dial_call_req_msg.uus.uus_dcs,
                             dial_call_req_msg.uus.uus_data_len);
        }
      }

      if ( need_enforce_emergency_directly )
      {
         dial_call_req_msg.call_type_valid = TRUE;
         dial_call_req_msg.call_type   = CALL_TYPE_EMERGENCY_V02;
         escv_type = qcril_qmi_nas_get_escv_type(voice_dial_address);
         if (escv_type > 0)
         {
             dial_call_req_msg.emer_cat_valid = TRUE;
             dial_call_req_msg.emer_cat = escv_type;
         }
      }

      if ( ( qcril_qmi_voice_info.jbims ) && ( NULL != displayText) )
      {
        dial_call_req_msg.display_text_valid = TRUE;
        qcril_cm_ss_convert_utf8_to_ucs2(displayText, dial_call_req_msg.display_text,
                &dial_call_req_msg.display_text_len);
        if (dial_call_req_msg.display_text_len)
        {
          dial_call_req_msg.display_text_len /= 2;
        }
        QCRIL_LOG_ESSENTIAL(".. display text len: %d str: %s ",
                dial_call_req_msg.display_text_len, dial_call_req_msg.display_text);
      }

      QCRIL_LOG_DEBUG(".. final elaboration %x, %x hex", (uint32)(call_info_entry->elaboration >> 32),(uint32)call_info_entry->elaboration );
      QCRIL_LOG_DEBUG(".. call type set %d emer cat %x", (int)dial_call_req_msg.call_type, dial_call_req_msg.emer_cat);

      user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
      memset(&dial_call_resp_msg, 0, sizeof(dial_call_resp_msg));

      last_call_fail_cause = CALL_END_CAUSE_REL_NORMAL_V02;
      /* Send QMI VOICE DIAL CALL REQ */

      qmi_client_error = qmi_client_send_msg_async ( qcril_qmi_client_get_user_handle ( QCRIL_QMI_CLIENT_VOICE ),
                                              QMI_VOICE_DIAL_CALL_REQ_V02,
                                              &dial_call_req_msg,
                                              sizeof(dial_call_req_msg),
                                              &dial_call_resp_msg,
                                              sizeof(dial_call_resp_msg),
                                              qcril_qmi_voice_command_cb,
                                              (void*)(uintptr_t)user_data,
                                              &txn_handle );

      QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );
      call_setup_result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, NULL );
  } while (FALSE);

  if ( RIL_E_SUCCESS != call_setup_result )
  { // rollback
    QCRIL_LOG_INFO(".. rolling back with %d", (int) call_setup_result);
    qcril_send_empty_payload_request_response(instance_id, params_ptr->t, params_ptr->event_id, call_setup_result);
    last_call_fail_cause = CALL_END_CAUSE_CLIENT_END_V02;

    if ( call_info_entry )
    {
      qcril_qmi_voice_voip_destroy_call_info_entry( call_info_entry );
    }
    if ( NULL != command_oversight )
    {
      qmi_ril_voice_ims_destroy_command_oversight( command_oversight );
    }
  }
  qcril_qmi_voice_voip_unlock_overview();
  qcril_qmi_voice_handle_new_last_call_failure_cause( last_call_fail_cause , TRUE, call_info_entry);
  QCRIL_LOG_INFO(".. last_call_fail_cause %d", (int) last_call_fail_cause);

  if ( QCRIL_EVT_IMS_SOCKET_REQ_DIAL == params_ptr->event_id )
  {
    qcril_qmi_ims__dial__free_unpacked(ims_in_data_ptr, NULL);
    if ( callDetails )
    {
      qcril_free(callDetails);
    }
  }
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_cm_callsvc_request_dial() */

//===========================================================================
// qcril_qmi_voice_emergency_call_pending_handler
//===========================================================================
void qcril_qmi_voice_emergency_call_pending_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);
  QCRIL_NOTUSED(params_ptr);

  qcril_qmi_voice_request_dial(&qcril_qmi_pending_emergency_call_info.emergency_params_ptr, NULL);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_voice_set_emergency_call_pending
//===========================================================================
void qcril_qmi_voice_set_emergency_call_pending(int emergency_call_pending)
{
  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_pending_emergency_call_info.is_emergency_call_pending = emergency_call_pending;

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_voice_gather_current_call_information
//===========================================================================
RIL_Errno qcril_qmi_voice_gather_current_call_information
(
   unsigned int iter,
   const qcril_request_params_type *const params_ptr,
   qcril_qmi_voice_current_calls_type *payload_ptr,
   const qcril_qmi_voice_voip_call_info_entry_type *const call_info_entry
)
{
    uint32_t   number_len;
    RIL_Errno result = RIL_E_SUCCESS;
    do
    {
        if (NULL == params_ptr ||
            NULL == payload_ptr ||
            NULL == call_info_entry)
        {
            result = RIL_E_GENERIC_FAILURE;
            QCRIL_LOG_ERROR("Null pointer: params_ptr %p,"
                            " payload_ptr %p,"
                            " call_info_entry %p",
                            params_ptr,
                            payload_ptr,
                            call_info_entry);
            break;
        }
        if (CM_CALL_ID_MAX <= iter)
        {
            result = RIL_E_GENERIC_FAILURE;
            QCRIL_LOG_ERROR("out of range: iter %u, max %u", iter, CM_CALL_ID_MAX);
            break;
        }
        // call state
        if( CALL_STATE_END_V02 == call_info_entry->voice_scv_info.call_state &&
                 qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id) )
        {
            payload_ptr->info[ iter ].state = IMS__CALL_STATE__CALL_END;
        }
        else if ( call_info_entry->elaboration &
                  QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EXTENDED_DIALING )
        {
            payload_ptr->info[iter].state = RIL_CALL_DIALING;
        }
        else
        {
            payload_ptr->info[iter].state = call_info_entry->ril_call_state;
        }

        // call id
        payload_ptr->info[iter].index = call_info_entry->android_call_id;

        //Media id
        payload_ptr->media_id[iter] = call_info_entry->media_id;

        // remote party number and number
        if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONN_PARTY_NUM_VALID ||
           call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONN_PARTY_IP_NUM_VALID )
        {
            if( RIL_REQUEST_GET_CURRENT_CALLS == params_ptr->event_id )
            {
                if ( call_info_entry->voice_svc_conn_party_num.conn_num[0] == QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER_PREFIX )
                {
                    payload_ptr->info[iter].toa = QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER;
                }
                else
                {
                    payload_ptr->info[iter].toa = QCRIL_QMI_VOICE_DOMESTIC_NUMBER;
                }

                payload_ptr->info[iter].number = qcril_malloc( call_info_entry->voice_svc_conn_party_num.conn_num_len + 1 );
                if ( payload_ptr->info[iter].number )
                {
                    memcpy( payload_ptr->info[iter].number, call_info_entry->voice_svc_conn_party_num.conn_num,
                        call_info_entry->voice_svc_conn_party_num.conn_num_len + 1 );
                    payload_ptr->info[iter].numberPresentation = qcril_qmi_voice_map_qmi_to_ril_num_pi( call_info_entry->voice_svc_conn_party_num.conn_num_pi );
                    payload_ptr->info[iter].namePresentation = QCRIL_QMI_VOICE_RIL_PI_UNKNOWN;
                }
            }
            else
            {
                if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONN_PARTY_IP_NUM_VALID)
                {
                    payload_ptr->info[iter].name = qcril_malloc(strlen(call_info_entry->voice_svc_conn_party_ip_num.conn_ip_num) + 1);
                    if ( payload_ptr->info[iter].name )
                    {
                        memcpy( payload_ptr->info[iter].name, call_info_entry->voice_svc_conn_party_ip_num.conn_ip_num,
                              strlen(call_info_entry->voice_svc_conn_party_ip_num.conn_ip_num) + 1 );
                        payload_ptr->info[iter].namePresentation = qcril_qmi_voice_map_qmi_to_ril_name_pi(call_info_entry->voice_svc_conn_party_ip_num.conn_ip_num_pi);
                        payload_ptr->info[iter].numberPresentation = QCRIL_QMI_VOICE_RIL_PI_UNKNOWN;
                    }
                }
                else
                {
                    payload_ptr->info[iter].name = qcril_malloc(call_info_entry->voice_svc_conn_party_num.conn_num_len + 1);
                    if ( payload_ptr->info[iter].name )
                    {
                        memcpy( payload_ptr->info[iter].name, call_info_entry->voice_svc_conn_party_num.conn_num,
                              call_info_entry->voice_svc_conn_party_num.conn_num_len + 1 );
                        payload_ptr->info[iter].namePresentation = qcril_qmi_voice_map_qmi_to_ril_name_pi(call_info_entry->voice_svc_conn_party_num.conn_num_pi);
                        payload_ptr->info[iter].numberPresentation = QCRIL_QMI_VOICE_RIL_PI_UNKNOWN;
                    }
                }
            }
        }
        else
        {
            // remote party number
            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID ||
               call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_IP_NUMBER_VALID )
            {
                pi_num_enum_v02 rp_number_pi;
                uint32_t rp_number_len;
                char rp_number[QMI_VOICE_SIP_URI_MAX_V02];
                memset(rp_number, 0, sizeof(rp_number));

                if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_IP_NUMBER_VALID)
                {
                    rp_number_pi = call_info_entry->voice_svc_remote_party_ip_number.ip_num_pi;
                    rp_number_len = strlen(call_info_entry->voice_svc_remote_party_ip_number.ip_num);
                    memcpy(rp_number, call_info_entry->voice_svc_remote_party_ip_number.ip_num, rp_number_len);
                }
                else
                {
                    rp_number_pi = call_info_entry->voice_svc_remote_party_number.number_pi;
                    rp_number_len = call_info_entry->voice_svc_remote_party_number.number_len;
                    strlcpy(rp_number,
                            call_info_entry->voice_svc_remote_party_number.number,
                            sizeof(rp_number));
                }

                if( qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id) &&
                    (QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMER_NUM_TO_IMS_ADDR & call_info_entry->elaboration) &&
                    (CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type ||
                     CALL_TYPE_EMERGENCY_IP_V02 == call_info_entry->voice_scv_info.call_type)
                  )
                {//convert the ims emergency address back to corresponding emergency number If needed
                    if(!strcmp(call_info_entry->emer_num_ims_addr_info.ims_address,
                             rp_number))
                    {
                        strlcpy(rp_number,
                                call_info_entry->emer_num_ims_addr_info.emergency_number,
                                sizeof(rp_number));
                        rp_number_len = strlen(rp_number);
                        rp_number_pi = PRESENTATION_NUM_ALLOWED_V02;
                    }
                }

                if ( rp_number[0] == QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER_PREFIX )
                {
                    payload_ptr->info[iter].toa = QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER;
                }
                else
                {
                    payload_ptr->info[iter].toa = QCRIL_QMI_VOICE_DOMESTIC_NUMBER;
                }
                    number_len = rp_number_len + 1;
                if (feature_redir_party_num_support)
                {
                    number_len += (call_info_entry->voice_svc_redirecting_party_num.num_len + 1);
                }
                else
                {
                    QCRIL_LOG_INFO("feature_redir_party_num_support not enabled");
                }
                    payload_ptr->info[iter].number = qcril_malloc( number_len );
                if ( payload_ptr->info[iter].number )
                {
                    memcpy( payload_ptr->info[iter].number,
                    rp_number,
                    rp_number_len + 1 );
                    if ( feature_redir_party_num_support &&
                         call_info_entry->voice_svc_redirecting_party_num.num_len > 0 )
                    {
                        payload_ptr->info[iter].number[rp_number_len] = '&';
                        memcpy( &payload_ptr->info[iter].number[rp_number_len + 1],
                        call_info_entry->voice_svc_redirecting_party_num.num,
                        call_info_entry->voice_svc_redirecting_party_num.num_len + 1 );
                    }
                    payload_ptr->info[iter].numberPresentation = qcril_qmi_voice_map_qmi_to_ril_num_pi(
                    rp_number_pi );
                }
                else
                {
                    result = RIL_E_GENERIC_FAILURE;
                    break;
                }
            }
            else
            {
                payload_ptr->info[iter].numberPresentation = QCRIL_QMI_VOICE_RIL_PI_UNKNOWN;
            }

            // remote party name
            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NAME_VALID )
            {
                QCRIL_LOG_INFO("remote name len %d, str %s", call_info_entry->voice_svc_remote_party_name.name_len,
                               call_info_entry->voice_svc_remote_party_name.name);
                if ( *(call_info_entry->voice_svc_remote_party_name.name) &&
                     call_info_entry->voice_svc_remote_party_name.name_len < QCRIL_QMI_VOICE_INTERCODING_BUF_LEN )
                {
                    payload_ptr->info[iter].name = qcril_malloc( call_info_entry->voice_svc_remote_party_name.name_len + 1 );
                    if ( payload_ptr->info[iter].name )
                    {
                        memcpy( payload_ptr->info[iter].name, call_info_entry->voice_svc_remote_party_name.name,
                                call_info_entry->voice_svc_remote_party_name.name_len + 1 );
                    }
                }
                else
                {
                    QCRIL_LOG_ERROR("remote party name is null, or remote party name len >= QCRIL_QMI_VOICE_INTERCODING_BUF_LEN");
                }
                payload_ptr->info[iter].namePresentation = qcril_qmi_voice_map_qmi_to_ril_name_pi(call_info_entry->voice_svc_remote_party_name.name_pi);
            }
            else
            {
                payload_ptr->info[iter].namePresentation = payload_ptr->info[iter].numberPresentation;
            }
        }

        // If IP call and ip_caller_name valid, then set the caller name in 'name' field
        if ((call_info_entry->voice_scv_info.call_type == CALL_TYPE_VOICE_IP_V02 ||
             call_info_entry->voice_scv_info.call_type == CALL_TYPE_VT_V02 ||
             call_info_entry->voice_scv_info.call_type == CALL_TYPE_EMERGENCY_IP_V02) &&
            (call_info_entry->ip_caller_name_valid))
        {
            uint8 caller_name_len = 0;
            char  caller_name[(QMI_VOICE_IP_CALLER_NAME_MAX_LEN_V02 * 2)] = "\0";

            QCRIL_LOG_INFO("ip caller name len: %d, caller name:",
                           call_info_entry->ip_caller_name.ip_caller_name_len);
            qcril_qmi_print_hex((unsigned char *)call_info_entry->ip_caller_name.ip_caller_name,
                                call_info_entry->ip_caller_name.ip_caller_name_len*2);

            // ip caller name TLV is valid. Set the name field
            caller_name_len = qcril_cm_ss_convert_ucs2_to_utf8(
                                             call_info_entry->ip_caller_name.ip_caller_name,
                                             call_info_entry->ip_caller_name.ip_caller_name_len*2,
                                             caller_name);

            if (caller_name_len > 0)
            {
                if (payload_ptr->info[iter].name)
                {
                    qcril_free(payload_ptr->info[iter].name);
                }
                payload_ptr->info[iter].name = qcril_malloc(caller_name_len + 1);
                if (payload_ptr->info[iter].name)
                {
                    strlcpy(payload_ptr->info[iter].name, caller_name, caller_name_len + 1);
                    payload_ptr->info[iter].namePresentation = QCRIL_QMI_VOICE_RIL_PI_ALLOWED;
                }
            }
        }

        // is multiparty
        payload_ptr->info[iter].isMpty = (call_info_entry->voice_scv_info.is_mpty) ? TRUE : FALSE;

        // is mobile terminated
        payload_ptr->info[iter].isMT = ( call_info_entry->voice_scv_info.direction == CALL_DIRECTION_MT_V02 ) ? TRUE : FALSE;
        QCRIL_LOG_INFO("Call state %d, IsMT=%d", call_info_entry->ril_call_state, call_info_entry->voice_scv_info.is_mpty );

        // ALS
        payload_ptr->info[iter].als = ( call_info_entry->voice_scv_info.als == ALS_LINE2_V02 ) ? TRUE : FALSE;

        // privacy
        if ( call_info_entry->voice_scv_info.call_type != CALL_TYPE_SUPS_V02)
        {
            payload_ptr->info[iter].isVoice = TRUE;
            if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VOICE_PRIVACY_VALID )
            {
                payload_ptr->info[iter].isVoicePrivacy = ( call_info_entry->voice_svc_voice_privacy == VOICE_PRIVACY_ENHANCED_V02) ? TRUE : FALSE;
            }
        }
        else
        {
            payload_ptr->info[iter].isVoice = FALSE;
        }

        // codec and call capabilities
        if ( qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id) )
        {
            if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CODEC_VALID)
            {
                payload_ptr->codec_valid[iter] = TRUE;
                payload_ptr->codec[iter] = call_info_entry->codec;
            }

            if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_LOCAL_CALL_CAPBILITIES_VALID)
            {
                payload_ptr->local_call_capabilities_info_valid[iter] = TRUE;
                payload_ptr->local_call_capabilities_info[iter] = call_info_entry->local_call_capabilities_info;
            }

            if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PEER_CALL_CAPBILITIES_VALID)
            {
                payload_ptr->peer_call_capabilities_info_valid[iter] = TRUE;
                payload_ptr->peer_call_capabilities_info[iter] = call_info_entry->peer_call_capabilities_info;
            }

            if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_CHILD_NUMBER_VALID)
            {
                char  child_number[QMI_VOICE_SIP_URI_MAX_V02+25] = "\0"; //Extra len 25 - to put the string "ChildNum="

                payload_ptr->child_number_valid[iter] = TRUE;
                snprintf(child_number, sizeof(child_number), "ChildNum=%s", call_info_entry->child_number.number);

                payload_ptr->child_number[iter] = qcril_malloc( strlen(child_number)+1 );
                strlcpy(payload_ptr->child_number[iter], child_number, strlen(child_number)+1);
            }

            if (call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_DISPLAY_TEXT_VALID)
            {
                char  display_text[(QMI_VOICE_DISPLAY_TEXT_MAX_LEN_V02 * 2)+25] = "\0"; //Extra len 25 - to put the string "DisplayText="

                payload_ptr->display_text_valid[iter] = TRUE;
                snprintf(display_text, sizeof(display_text), QCRIL_DISPLAY_TEXT_STR);

                qcril_cm_ss_convert_ucs2_to_utf8(call_info_entry->display_text.display_text,
                        call_info_entry->display_text.display_text_len*2,
                        display_text + strlen(QCRIL_DISPLAY_TEXT_STR));

                payload_ptr->display_text[iter] = qcril_malloc( strlen(display_text)+1 );
                strlcpy(payload_ptr->display_text[iter], display_text, strlen(display_text)+1);
            }
            if (qcril_qmi_voice_is_additional_call_info_available(call_info_entry))
            {
                int total_size = strlen(QCRIL_ADD_CALL_INFO_STR) +
                                 call_info_entry->additional_call_info.total_size + 1;
                payload_ptr->additional_call_info_valid[iter] = TRUE;
                payload_ptr->additional_call_info[iter]       = qcril_malloc(total_size);
                snprintf(payload_ptr->additional_call_info[iter],  total_size, "%s%s",
                         QCRIL_ADD_CALL_INFO_STR,
                         call_info_entry->additional_call_info.buffer);
            }
        }

        //parentCallID
        if( ( call_info_entry->srvcc_parent_call_info_valid ) &&
               ( call_info_entry->android_call_id != call_info_entry->srvcc_parent_call_info.parent_call_id ) )
        {
            if(strlen(call_info_entry->parent_call_id) > 0)
            {
                payload_ptr->parentCallID[iter] = qcril_malloc( strlen(call_info_entry->parent_call_id)+1 );
                if( NULL != payload_ptr->parentCallID[iter] )
                {
                    payload_ptr->parentCallID_valid[iter] = TRUE;
                    strlcpy(payload_ptr->parentCallID[iter], call_info_entry->parent_call_id, strlen(call_info_entry->parent_call_id)+1);
                    QCRIL_LOG_INFO("payload_ptr->parentCallID[%d]: %s", iter, payload_ptr->parentCallID[iter]);
                }
            }
        }

        // lcf
        if (call_info_entry->lcf_valid && qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id))
        {
            payload_ptr->lcf_valid[iter] = TRUE;
            payload_ptr->lcf[iter] = call_info_entry->lcf;
            memcpy(payload_ptr->lcf_extended_codes[iter], call_info_entry->lcf_extended_codes, MAX_DEC_INT_STR);
        }

        // end_reason_text
        QCRIL_LOG_INFO("call_info_entry->end_reason_text_valid = %d\n", call_info_entry->end_reason_text_valid);
        if (call_info_entry->end_reason_text_valid &&
                qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id))
        {
            uint8 end_reason_text_len = 0;
            char end_reason_text[QMI_VOICE_END_REASON_TEXT_MAX_LEN_V02*2];

            QCRIL_LOG_INFO("end_reason_text_len: %d, end_reason_text (UTF-16):",
                    call_info_entry->end_reason_text.end_reason_text_len);
            qcril_qmi_print_hex((unsigned char *)call_info_entry->end_reason_text.end_reason_text,
                    call_info_entry->end_reason_text.end_reason_text_len*2);

            end_reason_text_len = qcril_cm_ss_convert_ucs2_to_utf8(
                    call_info_entry->end_reason_text.end_reason_text,
                    call_info_entry->end_reason_text.end_reason_text_len*2,
                    end_reason_text);

            if (end_reason_text_len > 0)
            {
                QCRIL_LOG_INFO("end_reason_text (UTF-8): %s", end_reason_text);
                payload_ptr->end_reason_text[iter] = qcril_malloc(end_reason_text_len + 1);
                if (payload_ptr->end_reason_text[iter])
                {
                    strlcpy(payload_ptr->end_reason_text[iter], end_reason_text,
                            end_reason_text_len + 1);
                    payload_ptr->end_reason_text_valid[iter] = TRUE;
                }
            }
        }

        // uus
        payload_ptr->info[iter].uusInfo = NULL;
        if ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_UUS_VALID )
        {
            payload_ptr->uus_info[iter].uusType    = call_info_entry->voice_svc_uus.uus_type;
            payload_ptr->uus_info[iter].uusDcs     = call_info_entry->voice_svc_uus.uus_dcs;
            payload_ptr->uus_info[iter].uusLength  = call_info_entry->voice_svc_uus.uus_data_len;
            payload_ptr->uus_info[iter].uusData    =  (char*)call_info_entry->voice_svc_uus.uus_data;
            payload_ptr->info[iter].uusInfo        = &payload_ptr->uus_info[iter];
        }

        // Call mode
        payload_ptr->mode[iter] = call_info_entry->voice_scv_info.mode;
    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
    return result;
} // qcril_qmi_voice_gather_current_call_information

//===========================================================================
// qcril_qmi_get_call_list_to_send
//===========================================================================
void qcril_qmi_get_call_list_to_send
(
   const qcril_request_params_type *const params_ptr,
   qcril_qmi_voice_current_calls_type **payload_ptr_ptr,
   RIL_Call_Details call_details_copy[CM_CALL_ID_MAX],
   RIL_Errno *ril_req_res_ptr,
   int *is_ril_number_already_freed
)
{
   unsigned int k;
   int need_consider_voice_call_obj_cleanup = FALSE;
   qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

   unsigned int call_radio_tech = RADIO_TECH_UNKNOWN;
   unsigned int call_radio_tech_family = RADIO_TECH_UNKNOWN;

   qcril_qmi_voice_current_calls_type *payload_ptr;
   uint32 num_of_calls = 0;
   uint32 nof_cs_calls = 0;

   int   log_nof_reported_calls = 0;
   int   log_nof_skipped_calls = 0;
   int   log_nof_call_objects = 0;
   char  log_essence[ QCRIL_MAX_LOG_MSG_SIZE ];
   char  log_addon[ QCRIL_MAX_LOG_MSG_SIZE ];

   QCRIL_LOG_FUNC_ENTRY();

   call_radio_tech = qcril_qmi_voice_nas_control_get_reported_voice_radio_tech();
   call_radio_tech_family = qcril_qmi_voice_nas_control_convert_radio_tech_to_radio_tech_family(call_radio_tech);

   snprintf( log_essence, QCRIL_MAX_LOG_MSG_SIZE,
                "RILVI: calls rep:" );

   *ril_req_res_ptr = RIL_E_GENERIC_FAILURE;
   *is_ril_number_already_freed = FALSE;

   qcril_qmi_voice_voip_lock_overview();

   if (RADIO_TECH_3GPP2 == call_radio_tech_family)
   {
      qcril_qmi_voice_voip_current_call_summary_type calls_summary;
      qcril_qmi_voice_voip_generate_summary( &calls_summary );
      nof_cs_calls = calls_summary.nof_voice_calls;
   }

   do
   {
      payload_ptr = ( qcril_qmi_voice_current_calls_type * ) qcril_malloc( sizeof ( qcril_qmi_voice_current_calls_type ) )  ;
      if ( NULL == payload_ptr )
      {
        break;
      }

      memset(payload_ptr, 0, sizeof(qcril_qmi_voice_current_calls_type));
      memset(call_details_copy, 0, sizeof(*call_details_copy) * CM_CALL_ID_MAX);
      QCRIL_LOG_INFO( "iteration through call objects start" );
      k = 0;
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
      while ( NULL != call_info_entry )
      {
        log_nof_call_objects++;

        /* In case of VoiIP conference(mpty) call, only the conference call is considered  */
        /* Telephony is only aware of the associated call */
        if (VOICE_INVALID_CALL_ID != call_info_entry->android_call_id &&        // not a shadow call
            ((VOICE_INVALID_CALL_ID != call_info_entry->qmi_call_id) ||
             (call_info_entry->elaboration &
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_QMI_ID_RECEIVED) ||
             (CALL_STATE_END_V02 == call_info_entry->voice_scv_info.call_state &&
              qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id))) &&      // not a call without a valid qmi call ID
            CALL_STATE_SETUP_V02 != call_info_entry->voice_scv_info.call_state && // not a setup call
            CALL_TYPE_OTAPA_V02 != call_info_entry->voice_scv_info.call_type && // notan OTAPA call
            (CALL_STATE_END_V02 != call_info_entry->voice_scv_info.call_state ||
             qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id)) &&        // not an ended CS call
            (!(call_info_entry->elaboration &
               QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_WAITING_FOR_MATCHING_VOICE_RTE) || // call mode matches with reported voice radio tech
             (call_info_entry->elaboration &
              (QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS |
               QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL |
               QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL))) &&              // emergency call originated from limited service or emergency call or 1x CSFB call
            (call_info_entry->elaboration &
             QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_RIL_CALL_STATE_VALID) &&
            !(call_info_entry->elaboration &
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1X_REMOTE_NUM_PENDING) &&
            (qcril_qmi_voice_is_qmi_call_emergency(&call_info_entry->voice_scv_info) ||
             !qmi_ril_voice_is_calls_supressed_by_pil_vcl() ||
             (CALL_STATE_END_V02 == call_info_entry->voice_scv_info.call_state &&
              qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id))) &&
            !((call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PHANTOM_CALL) &&
              (call_info_entry->voice_scv_info.call_state == CALL_STATE_ORIGINATING_V02 ||
               call_info_entry->voice_scv_info.call_state == CALL_STATE_ALERTING_V02) &&
              (qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state(RIL_CALL_ACTIVE) != NULL)))
          {
            if ( (qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id) && qcril_qmi_voice_call_to_ims(call_info_entry)) ||
                 (RIL_REQUEST_GET_CURRENT_CALLS == params_ptr->event_id && qcril_qmi_voice_call_to_atel(call_info_entry))
               )
            {
              num_of_calls++;
              snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE,
                      "[qmi call id %d, android call id %d, qmi call state %d]",
                      call_info_entry->qmi_call_id,
                      call_info_entry->android_call_id,
                      call_info_entry->voice_scv_info.call_state
                      );
              strlcat( log_essence, log_addon, sizeof( log_essence ) );
              log_nof_reported_calls++;
              if (RIL_E_SUCCESS != qcril_qmi_voice_gather_current_call_information
                                                        (k,
                                                         params_ptr,
                                                         payload_ptr,
                                                         call_info_entry))
              {
                  break;
              }
              // call type
              if( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
              {
                VT_VOLTE_CALL_DETAILS(payload_ptr->info[k].callDetails) = qcril_malloc( sizeof(RIL_Call_Details) );

                if ( NULL != VT_VOLTE_CALL_DETAILS( payload_ptr->info[k].callDetails ) )
                {
                  qcril_qmi_voice_get_atel_call_type_info_by_call_info( call_info_entry, VT_VOLTE_CALL_DETAILS( payload_ptr->info[k].callDetails ));
                }
              }

              if ( qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id) )
              {
                qcril_qmi_voice_get_atel_call_type_info_by_call_info( call_info_entry, &call_details_copy[k] );
              }

              payload_ptr->info_ptr[ k ] = &payload_ptr->info[ k ];
              payload_ptr->num_of_calls++;
              k++;
            }
          }
          else
          { // call not reported
            log_nof_skipped_calls++;
          }

          if ( CALL_STATE_END_V02 == call_info_entry->voice_scv_info.call_state &&
                  VOICE_INVALID_CALL_ID != call_info_entry->android_call_id )
          {
             if ((qcril_qmi_voice_call_to_ims(call_info_entry) &&
                         qcril_qmi_voice_is_ims_send_calls(params_ptr->event_id)) ||
                     (qcril_qmi_voice_call_to_atel(call_info_entry) &&
                      params_ptr->event_id == RIL_REQUEST_GET_CURRENT_CALLS))
             {
                // Need to add this to destroy the call object.
                // Temporary until the mpty_call_list is removed
                // TODO: remove this loop when mpty_call_list is removed.
                qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;
                call_info = call_info_entry;
                do
                {
                   call_info->elaboration |=  QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_ENDED_REPORTED;
                } while (call_info = call_info->mpty_voip_call_list);

                // - For IMS call, failure is sent along with
                //   UNSOL_RESPONSE_CALL_STATE_CHANGED. so set the elab here.
                // - For CS call, call is cleared after LAST_CALL_FAILURE
                //   req is handled, elab is set by then
                if (qcril_qmi_voice_call_to_ims(call_info_entry) &&
                        params_ptr->event_id == QCRIL_EVT_IMS_SOCKET_SEND_UNSOL_CURRENT_CALLS)
                {
                   call_info_entry->elaboration |=
                       QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_LAST_CALL_FAILURE_REPORTED;
                }
                need_consider_voice_call_obj_cleanup = TRUE;
             }
          }
          call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
      }
      *ril_req_res_ptr = RIL_E_SUCCESS;

  } while ( FALSE );


  if ( NULL != payload_ptr && 1 == payload_ptr->num_of_calls )
  {
    *is_ril_number_already_freed = qcril_qmi_voice_handle_ril_call_entry(payload_ptr->info_ptr[ QMI_RIL_ZERO ]);
  }

  if ( need_consider_voice_call_obj_cleanup )
  {
    qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl();
  }

  qcril_qmi_voice_voip_unlock_overview();

  *payload_ptr_ptr = payload_ptr;

  if (RIL_REQUEST_GET_CURRENT_CALLS == params_ptr->event_id)
  {
     qcril_qmi_voice_nas_control_set_current_calls_number(num_of_calls);
     if( !num_of_calls )
     {
       qcril_qmi_nas_control_signal_nas_on_current_calls_change();
     }
  }

  snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE,
            " reported/skipped %d/%d calls",
           log_nof_reported_calls, log_nof_skipped_calls);
  strlcat( log_essence, log_addon, sizeof( log_essence ) );

  QCRIL_LOG_ESSENTIAL( log_essence );
} // qcril_qmi_get_call_list_to_send

//===========================================================================
// qcril_qmi_voice_get_current_calls_cleanup
//===========================================================================
void qcril_qmi_voice_get_current_calls_cleanup
(
  qcril_qmi_voice_current_calls_type *payload_ptr,
  int is_ril_number_already_freed
)
{
  uint32 i;
  QCRIL_LOG_FUNC_ENTRY();
  if ( NULL != payload_ptr )
  {
    for ( i=0; i < payload_ptr->num_of_calls; i++ )
    {
      if ( payload_ptr->info[ i ].number && FALSE == is_ril_number_already_freed )
      {
        qcril_free( payload_ptr->info[ i ].number );
      }
      if( payload_ptr->info[ i ].name )
      {
        qcril_free(payload_ptr->info[ i ].name);
      }
      if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
      {
          if( VT_VOLTE_CALL_DETAILS ( payload_ptr->info[i].callDetails ) )
          {
             qcril_free( VT_VOLTE_CALL_DETAILS ( payload_ptr->info[i].callDetails ) );
          }
      }
      if( NULL != payload_ptr->parentCallID[i])
      {
        qcril_free(payload_ptr->parentCallID[i]);
      }
      if( NULL != payload_ptr->child_number[i])
      {
        qcril_free(payload_ptr->child_number[i]);
      }
      if( NULL != payload_ptr->display_text[i])
      {
        qcril_free(payload_ptr->display_text[i]);
      }
      if( NULL != payload_ptr->additional_call_info[i])
      {
        qcril_free(payload_ptr->additional_call_info[i]);
      }
      if( NULL != payload_ptr->end_reason_text[i])
      {
        qcril_free(payload_ptr->end_reason_text[i]);
      }
    }
    qcril_free( payload_ptr );
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_get_current_calls_cleanup

//===========================================================================
// RIL_REQUEST_GET_CURRENT_CALLS
//===========================================================================
void qcril_qmi_voice_request_get_current_atel_calls
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_qmi_voice_current_calls_type *payload_ptr;
  qcril_request_resp_params_type resp;
  unsigned int i;
  char *call_state[ 6 ] = { "Active", "Holding", "Dialing", "Alerting", "Incoming", "Waiting" };
  RIL_Errno ril_req_res;
  int is_ril_number_already_freed;
  RIL_Call_Details call_details_copy[CM_CALL_ID_MAX];

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_get_call_list_to_send(params_ptr, &payload_ptr, call_details_copy, &ril_req_res, &is_ril_number_already_freed);

  // respond
  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
  if ( RIL_E_SUCCESS == ril_req_res )
  {
    resp.resp_pkt = (void *) &payload_ptr->info_ptr;
    resp.resp_len = sizeof( RIL_Call * ) * payload_ptr->num_of_calls;

    QCRIL_LOG_INFO( "Reply to RIL --> Number of calls : %ld", payload_ptr->num_of_calls );
    for ( i = 0; i < payload_ptr->num_of_calls; i++ )
    {
      QCRIL_LOG_INFO( "Reply to RIL --> call[%ld] :state %s index %d, toa %d, isMpty %d, isMT %d, als %d, isVoice %d, isVoicePrivacy %d",
                       i,call_state[ payload_ptr->info_ptr[ i ]->state ], payload_ptr->info_ptr[ i ]->index,
                       payload_ptr->info_ptr[ i ]->toa,
                       payload_ptr->info_ptr[ i ]->isMpty,
                       payload_ptr->info_ptr[ i ]->isMT,
                       payload_ptr->info_ptr[ i ]->als,
                       payload_ptr->info_ptr[ i ]->isVoice,
                       payload_ptr->info_ptr[ i ]->isVoicePrivacy );

      QCRIL_LOG_INFO( "...num %s, num presentation %d, name %s, name presentation %d",
                       payload_ptr->info_ptr[ i ]->number,
                       payload_ptr->info_ptr[ i ]->numberPresentation,
                       payload_ptr->info_ptr[ i ]->name,
                       payload_ptr->info_ptr[ i ]->namePresentation);

      if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_ICS ) && qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
      {
          QCRIL_LOG_DEBUG( "call_type = %d, call_domain = %d",
                           VT_VOLTE_CALL_DETAILS(payload_ptr->info_ptr[i]->callDetails)->callType,
                           VT_VOLTE_CALL_DETAILS(payload_ptr->info_ptr[i]->callDetails)->callDomain );
      }
    }
  }
  qcril_send_request_response( &resp );

  qcril_qmi_voice_get_current_calls_cleanup(payload_ptr, is_ril_number_already_freed);

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_get_current_atel_calls

//===========================================================================
// qcril_qmi_voice_send_current_ims_calls
//===========================================================================
void qcril_qmi_voice_send_current_ims_calls
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
   qcril_qmi_voice_current_calls_type *payload_ptr;
   qcril_request_resp_params_type resp;
   unsigned int i,j;
   char *call_state[ 7 ] = { "Active", "Holding", "Dialing", "Alerting", "Incoming", "Waiting", "END" };
   RIL_Errno ril_req_res;
   int is_ril_number_already_freed;
   RIL_Call_Details call_details_copy[CM_CALL_ID_MAX];

   QCRIL_NOTUSED( ret_ptr );

   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_get_call_list_to_send(params_ptr, &payload_ptr, call_details_copy, &ril_req_res, &is_ril_number_already_freed);

   char *codec_str[] = { "Codec=NONE",
                         "Codec=QCELP13K",
                         "Codec=EVRC",
                         "Codec=EVRC_B",
                         "Codec=EVRC_WB",
                         "Codec=EVRC_NW",
                         "Codec=AMR_NB",
                         "Codec=AMR_WB",
                         "Codec=GSM_EFR",
                         "Codec=GSM_FR",
                         "Codec=GSM_HR"
                       };


   // respond
   if ( RIL_E_SUCCESS == ril_req_res )
   {
      QCRIL_LOG_INFO( "Reply to RIL --> Number of calls : %ld", payload_ptr->num_of_calls );

      Ims__CallList call_list = IMS__CALL_LIST__INIT;
      call_list.n_callattributes = payload_ptr->num_of_calls;
      call_list.callattributes =  qcril_malloc(sizeof (Ims__CallList__Call*) * payload_ptr->num_of_calls);
      Ims__CallList__Call *calls = qcril_malloc(sizeof(Ims__CallList__Call) * payload_ptr->num_of_calls);
      Ims__CallDetails *call_details = qcril_malloc(sizeof(Ims__CallDetails) * payload_ptr->num_of_calls);
      const uint8 MAX_EXTRAS_PER_CALL = 5;
      char** call_details_extras = qcril_malloc(sizeof(char*) * payload_ptr->num_of_calls * MAX_EXTRAS_PER_CALL);
      Ims__SrvStatusList *localAbility = qcril_malloc(sizeof(Ims__SrvStatusList) * payload_ptr->num_of_calls);
      Ims__SrvStatusList *peerAbility = qcril_malloc(sizeof(Ims__SrvStatusList) * payload_ptr->num_of_calls);
      Ims__CallFailCauseResponse *failcause = qcril_malloc(sizeof(Ims__CallFailCauseResponse) * payload_ptr->num_of_calls);

      if ( NULL == call_list.callattributes || NULL == calls || NULL == call_details || NULL == call_details_extras || NULL == localAbility || NULL == peerAbility || NULL == failcause )
      {
        QCRIL_LOG_FATAL("malloc failed");
        if ( call_list.callattributes )
        {
          qcril_free(call_list.callattributes);
        }
        if ( calls )
        {
          qcril_free(calls);
        }
        if ( call_details )
        {
          qcril_free(call_details);
        }
        if ( call_details_extras )
        {
          qcril_free(call_details_extras);
        }
        if ( localAbility )
        {
          qcril_free(localAbility);
        }
        if ( peerAbility )
        {
          qcril_free(peerAbility);
        }
        if ( failcause )
        {
          qcril_free(failcause);
        }

        ril_req_res = RIL_E_GENERIC_FAILURE;
      }
      else
      {
        for ( i = 0; i < payload_ptr->num_of_calls; i++ )
        {
          call_list.callattributes[i] = &calls[i];
          QCRIL_LOG_INFO( "Reply to RIL --> call[%ld] :state %s index %d, toa %d, isMpty %d, isMT %d, als %d, isVoice %d, isVoicePrivacy %d",
                           i,call_state[ payload_ptr->info_ptr[ i ]->state ], payload_ptr->info_ptr[ i ]->index,
                           payload_ptr->info_ptr[ i ]->toa,
                           payload_ptr->info_ptr[ i ]->isMpty,
                           payload_ptr->info_ptr[ i ]->isMT,
                           payload_ptr->info_ptr[ i ]->als,
                           payload_ptr->info_ptr[ i ]->isVoice,
                           payload_ptr->info_ptr[ i ]->isVoicePrivacy );

          QCRIL_LOG_INFO( "...num %s, num presentation %d, name %s, name presentation %d",
                           payload_ptr->info_ptr[ i ]->number,
                           payload_ptr->info_ptr[ i ]->numberPresentation,
                           payload_ptr->info_ptr[ i ]->name,
                           payload_ptr->info_ptr[ i ]->namePresentation);

          Ims__CallList__Call call_tmp = IMS__CALL_LIST__CALL__INIT;
          memcpy(&(calls[i]), &call_tmp, sizeof(Ims__CallList__Call));

          calls[i].has_state = TRUE;
          calls[i].state     = payload_ptr->info_ptr[ i ]->state;
          calls[i].has_index = TRUE;
          calls[i].index     = payload_ptr->info_ptr[ i ]->index;
          calls[i].has_toa   = TRUE;
          calls[i].toa       = payload_ptr->info_ptr[ i ]->toa;
          calls[i].has_ismpty = TRUE;
          calls[i].ismpty    = payload_ptr->info_ptr[ i ]->isMpty;
          calls[i].has_ismt  = TRUE;
          calls[i].ismt      = payload_ptr->info_ptr[ i ]->isMT;
          calls[i].has_als   = TRUE;
          calls[i].als       = payload_ptr->info_ptr[ i ]->als;
          calls[i].has_isvoice = TRUE;
          calls[i].isvoice     = payload_ptr->info_ptr[ i ]->isVoice;
          calls[i].has_isvoiceprivacy = TRUE;
          calls[i].isvoiceprivacy     = payload_ptr->info_ptr[ i ]->isVoicePrivacy;
          calls[i].number   = payload_ptr->info_ptr[ i ]->number;
          calls[i].has_numberpresentation = TRUE;
          calls[i].numberpresentation     = payload_ptr->info_ptr[ i ]->numberPresentation;
          calls[i].name   = payload_ptr->info_ptr[ i ]->name;
          calls[i].has_namepresentation = TRUE;
          calls[i].namepresentation     = payload_ptr->info_ptr[ i ]->namePresentation;
          calls[i].calldetails = &(call_details[i]);

          Ims__CallDetails call_details_tmp = IMS__CALL_DETAILS__INIT;
          memcpy(&(call_details[i]), &call_details_tmp, sizeof(Ims__CallDetails));
          call_details[i].has_calltype = TRUE;
          call_details[i].calltype = qcril_qmi_ims_map_ril_call_type_to_ims_call_type(call_details_copy[i].callType);
          call_details[i].has_calldomain = TRUE;
          call_details[i].calldomain = qcril_qmi_ims_map_ril_call_domain_to_ims_call_domain(call_details_copy[i].callDomain);
          // Set call sub state
          call_details[i].has_callsubstate = TRUE;
          call_details[i].callsubstate = qcril_qmi_ims_map_ril_call_substate_to_ims_call_substate(
                call_details_copy[i].callSubState);
          call_details[i].has_mediaid = TRUE;
          call_details[i].mediaid = payload_ptr->media_id[i];
          QCRIL_LOG_INFO("Media id = %d", call_details[i].mediaid);

          if (payload_ptr->lcf_valid[i])
          {
             calls[i].failcause = &failcause[i];
             qcril_qmi_ims__call_fail_cause_response__init(&failcause[i]);
             failcause[i].has_failcause = TRUE;
             failcause[i].failcause = payload_ptr->lcf[i];
             if (strlen(payload_ptr->lcf_extended_codes[i]) > 0)
             {
                failcause[i].failcause = qcril_qmi_ims_map_ril_failcause_to_ims_failcause ( payload_ptr->lcf[i], atoi(payload_ptr->lcf_extended_codes[i]) );
                failcause[i].has_errorinfo = TRUE;
                failcause[i].errorinfo.len = strlen(payload_ptr->lcf_extended_codes[i]);
                failcause[i].errorinfo.data = payload_ptr->lcf_extended_codes[i];
             }

             if (payload_ptr->end_reason_text_valid[i])
             {
                failcause[i].networkerrorstring = payload_ptr->end_reason_text[i];
             }
          }


          //Fill CallDetails extras
          unsigned int call_details_extras_idx = i * MAX_EXTRAS_PER_CALL;
          call_details[i].n_extras = 0;
          call_details[i].extras = &(call_details_extras[call_details_extras_idx]);
          if (payload_ptr->codec_valid[i])
          {
             if (payload_ptr->codec[i] >= 0 &&
                ((unsigned int)payload_ptr->codec[i] < sizeof(codec_str)/sizeof(codec_str[0])))
             {
                if( call_details[i].n_extras < MAX_EXTRAS_PER_CALL )
                {
                   call_details_extras[call_details_extras_idx] = codec_str[payload_ptr->codec[i]];
                   call_details[i].n_extras++;
                   call_details_extras_idx++;
                }
             }
             else
             {
                QCRIL_LOG_ERROR( "invalid payload_ptr->codec[i]: %d", payload_ptr->codec[i]);
             }
          }

          if( TRUE == payload_ptr->parentCallID_valid[i] )
          {
             if( call_details[i].n_extras < MAX_EXTRAS_PER_CALL )
             {
                call_details_extras[call_details_extras_idx] = payload_ptr->parentCallID[i];
                call_details[i].n_extras++;
                call_details_extras_idx++;
             }
          }

          if( TRUE == payload_ptr->child_number_valid[i] )
          {
             if( call_details[i].n_extras < MAX_EXTRAS_PER_CALL )
             {
                call_details_extras[call_details_extras_idx] = payload_ptr->child_number[i];
                call_details[i].n_extras++;
                call_details_extras_idx++;
             }
          }

          if( TRUE == payload_ptr->display_text_valid[i] )
          {
            if( call_details[i].n_extras < MAX_EXTRAS_PER_CALL )
            {
              call_details_extras[call_details_extras_idx] = payload_ptr->display_text[i];
              call_details[i].n_extras++;
              call_details_extras_idx++;
            }
          }

          if( TRUE == payload_ptr->additional_call_info_valid[i] )
          {
            if( call_details[i].n_extras < MAX_EXTRAS_PER_CALL )
            {
              call_details_extras[call_details_extras_idx] = payload_ptr->additional_call_info[i];
              call_details[i].n_extras++;
              call_details_extras_idx++;
            }
          }

          QCRIL_LOG_INFO( "...No. of call_details extras %d ", call_details[i].n_extras);
          QCRIL_LOG_INFO( "...%s, %s, %s, %s, %s",
                           call_details_extras[ i*MAX_EXTRAS_PER_CALL ],
                           call_details_extras[ (i*MAX_EXTRAS_PER_CALL)+1 ],
                           call_details_extras[ (i*MAX_EXTRAS_PER_CALL)+2 ],
                           call_details_extras[ (i*MAX_EXTRAS_PER_CALL)+3 ],
                           call_details_extras[ (i*MAX_EXTRAS_PER_CALL)+4 ]);

          if (payload_ptr->local_call_capabilities_info_valid[i])
          {
             call_details[i].localability = &(localAbility[i]);

             Ims__SrvStatusList tmp_localAbility = IMS__SRV_STATUS_LIST__INIT;
             memcpy(&(localAbility[i]), &tmp_localAbility, sizeof(Ims__SrvStatusList));

             qcril_qmi_ims_translate_ril_callcapabilities_to_ims_srvstatusinfo(
                     &payload_ptr->local_call_capabilities_info[i],
                     &localAbility[i],
                     call_details[i].calltype,
                     payload_ptr->mode[i]);
          }

          if (payload_ptr->peer_call_capabilities_info_valid[i])
          {
             call_details[i].peerability = &(peerAbility[i]);

             Ims__SrvStatusList tmp_peerAbility = IMS__SRV_STATUS_LIST__INIT;
             memcpy(&(peerAbility[i]), &tmp_peerAbility, sizeof(Ims__SrvStatusList));

             qcril_qmi_ims_translate_ril_callcapabilities_to_ims_srvstatusinfo(
                     &payload_ptr->peer_call_capabilities_info[i],
                     &peerAbility[i],
                     IMS__CALL_TYPE__CALL_TYPE_UNKNOWN,
                     CALL_MODE_NO_SRV_V02);
          }
        }
        if (QCRIL_EVT_IMS_SOCKET_SEND_UNSOL_CURRENT_CALLS == params_ptr->event_id)
        {
            qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED, IMS__ERROR__E_SUCCESS, &call_list, sizeof(call_list));
        }
        else
        {
            qcril_qmi_ims_socket_send(params_ptr->t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS, qcril_qmi_ims_map_ril_error_to_ims_error(ril_req_res), &call_list, sizeof(call_list));
        }

        if ( NULL != call_list.callattributes )
        {
          qcril_free(call_list.callattributes);
        }
        if ( NULL != calls )
        {
          qcril_free(calls);
        }
        if ( NULL != call_details )
        {
          qcril_free(call_details);
        }
        if ( NULL != call_details_extras )
        {
          qcril_free(call_details_extras);
        }
        if ( NULL != localAbility )
        {
          for(i=0; i<payload_ptr->num_of_calls; i++)
          {
            if( NULL != localAbility[i].srvstatusinfo )
            {
              for(j=0; j<localAbility[i].n_srvstatusinfo; j++)
              {
                if( NULL != localAbility[i].srvstatusinfo[j])
                {
                  qcril_free(localAbility[i].srvstatusinfo[j]);
                }
              }
              qcril_free(localAbility[i].srvstatusinfo);
            }
          }

          qcril_free(localAbility);
        }
        if ( NULL != peerAbility )
        {
          for(i=0; i<payload_ptr->num_of_calls; i++)
          {
            if( NULL != peerAbility[i].srvstatusinfo )
            {
              for(j=0; j<peerAbility[i].n_srvstatusinfo; j++)
              {
                if( NULL != peerAbility[i].srvstatusinfo[j])
                {
                  qcril_free(peerAbility[i].srvstatusinfo[j]);
                }
              }
              qcril_free(peerAbility[i].srvstatusinfo);
            }
          }

          qcril_free(peerAbility);
        }
        if ( failcause )
        {
          qcril_free(failcause);
        }
      }
   }

   if (RIL_E_SUCCESS != ril_req_res && QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS == params_ptr->event_id)
   {
      qcril_qmi_ims_socket_send(params_ptr->t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS, qcril_qmi_ims_map_ril_error_to_ims_error(ril_req_res), NULL, 0);
   }

   qcril_qmi_voice_get_current_calls_cleanup(payload_ptr, is_ril_number_already_freed);

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_send_current_ims_calls

//===========================================================================
//qcril_qmi_voice_create_emer_voice_entry
//===========================================================================
 void qcril_qmi_voice_create_emer_voice_entry(qcril_qmi_voice_emer_voice_feature_info_type * emer_voice_number, voice_remote_party_number2_type_v02 * remote_party_number)
{
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_INFO("entry %p, remote svc entry %p", emer_voice_number, remote_party_number);
  if( emer_voice_number && remote_party_number)
  {
    emer_voice_number->is_valid = TRUE;
    if ( remote_party_number->number[0] == QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER_PREFIX )
    {
      emer_voice_number->toa = QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER;
    }
    else
    {
      emer_voice_number->toa = QCRIL_QMI_VOICE_DOMESTIC_NUMBER;
    }
    emer_voice_number->number = qcril_malloc( remote_party_number->number_len + 1 );
    if ( emer_voice_number->number )
    {
      memcpy( emer_voice_number->number, remote_party_number->number, remote_party_number->number_len + 1 );
      emer_voice_number->numberPresentation = qcril_qmi_voice_map_qmi_to_ril_num_pi( remote_party_number->number_pi );
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_voice_create_emer_voice_entry

//===========================================================================
//qcril_qmi_voice_is_emer_voice_entry_valid
//===========================================================================
int qcril_qmi_voice_is_emer_voice_entry_valid(qcril_qmi_voice_emer_voice_feature_info_type * emer_voice_number)
{
  int ret = FALSE;

  QCRIL_LOG_FUNC_ENTRY();
  if( emer_voice_number && TRUE == emer_voice_number->is_valid )
  {
    ret = TRUE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
} //qcril_qmi_voice_is_emer_voice_entry_valid

//===========================================================================
//qcril_qmi_voice_handle_ril_call_entry
//===========================================================================
int qcril_qmi_voice_handle_ril_call_entry(RIL_Call *info_ptr)
{
  qcril_qmi_voice_voip_call_info_entry_type *cdma_voice_call_info_entry = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *cdma_no_srv_emer_call_info_entry = NULL;
  qcril_qmi_voice_emer_voice_feature_info_type * temp_ptr = NULL;
  int is_cdma_voice_emergency_calls_present;
  int ret;

  QCRIL_LOG_FUNC_ENTRY();

  cdma_voice_call_info_entry = NULL;
  cdma_no_srv_emer_call_info_entry = NULL;
  temp_ptr = NULL;
  is_cdma_voice_emergency_calls_present = FALSE;
  ret = FALSE;

  QCRIL_LOG_INFO("call type to be considered %d", qcril_qmi_voice_cdma_call_type_to_be_considered);
  is_cdma_voice_emergency_calls_present = qcril_qmi_voice_is_cdma_voice_emergency_calls_present(&cdma_voice_call_info_entry, &cdma_no_srv_emer_call_info_entry);

  if( CALL_TYPE_EMERGENCY_V02 == qcril_qmi_voice_cdma_call_type_to_be_considered )
  {
    if( NULL != cdma_no_srv_emer_call_info_entry &&
        VOICE_LOWEST_CALL_ID < cdma_no_srv_emer_call_info_entry->voice_scv_info.call_id &&
        //mode of the emergency call would be either CDMA or NO_SRV
        //In case of NO_SRV we conclude that it is a CDMA emergency + voice call scenario by checking If there is a CDMA voice call present
        ( CALL_MODE_CDMA_V02 == cdma_no_srv_emer_call_info_entry->voice_scv_info.mode || TRUE == is_cdma_voice_emergency_calls_present ) &&
        CALL_TYPE_EMERGENCY_V02 == cdma_no_srv_emer_call_info_entry->voice_scv_info.call_type &&
        //Should not be considering the ENDed call's number as we not going to destroy and not report this call to Atel
        //QMI Voice replaces the voice call's number with a emergency number by the time emergency call is ENDed
        CALL_STATE_END_V02 != cdma_no_srv_emer_call_info_entry->voice_scv_info.call_state
      )
    {
      temp_ptr = &(cdma_no_srv_emer_call_info_entry->emer_voice_number);
    }
  }
  else
  {
    if( NULL != cdma_voice_call_info_entry &&
        ( CALL_MODE_CDMA_V02 == cdma_voice_call_info_entry->voice_scv_info.mode ) &&
        (cdma_voice_call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_REPORT_CACHED_RP_NUMBER)
      )
    {
      temp_ptr = &(cdma_voice_call_info_entry->emer_voice_number);
    }
  }

  if( temp_ptr )
  {
    QCRIL_LOG_INFO( "Found call entry - valid %d, number %s, presentation %d, toa %d",temp_ptr->is_valid, temp_ptr->number, temp_ptr->numberPresentation, temp_ptr->toa);
    if( temp_ptr->is_valid )
    {
      QCRIL_LOG_INFO("freeing up the original ril call entry %p as we need to report cached entry",info_ptr);
      if( info_ptr )
      {
        ret = TRUE;
        if( info_ptr->number )
        {
          free(info_ptr->number);
        }
        if ( temp_ptr->number[0] == QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER_PREFIX )
        {
          info_ptr->toa = QCRIL_QMI_VOICE_INTERNATIONAL_NUMBER;
        }
        else
        {
          info_ptr->toa = QCRIL_QMI_VOICE_DOMESTIC_NUMBER;
        }
        info_ptr->number = temp_ptr->number;
        info_ptr->numberPresentation = temp_ptr->numberPresentation;
      }
    }
    else
    { // should not be ending up here
      QCRIL_LOG_FATAL("Invalid state in cdma call handling");
    }
  }
  else
  {
    QCRIL_LOG_INFO("ril call entry unchanged");
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
  return ret;
} //qcril_qmi_voice_handle_ril_call_entry


//===========================================================================
//qcril_qmi_voice_nas_control_get_current_calls_number
//===========================================================================
unsigned int qcril_qmi_voice_nas_control_get_current_calls_number()
{
  unsigned int number_of_calls;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_info_lock();
  number_of_calls = qcril_qmi_voice_info.number_of_reported_calls;
  qcril_qmi_voice_info_unlock();

  QCRIL_LOG_FUNC_RETURN_WITH_RET(number_of_calls);
  return number_of_calls;
} //qcril_qmi_voice_nas_control_get_current_calls_number

//===========================================================================
//qcril_qmi_voice_nas_control_set_current_calls_number
//===========================================================================
void qcril_qmi_voice_nas_control_set_current_calls_number(unsigned int number_of_calls)
{
  QCRIL_LOG_INFO("entered %d",number_of_calls);

  qcril_qmi_voice_info_lock();
  qcril_qmi_voice_info.number_of_reported_calls = number_of_calls;
  qcril_qmi_voice_info_unlock();

  QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_voice_nas_control_set_current_calls_number

//===========================================================================
// qcril_qmi_voice_nas_control_is_any_calls_present
//===========================================================================
boolean qcril_qmi_voice_nas_control_is_any_calls_present()
{
  QCRIL_LOG_FUNC_ENTRY();
  boolean result = FALSE;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary;
  memset( &calls_summary, 0, sizeof( calls_summary ) );

  qcril_qmi_voice_voip_lock_overview();
  qcril_qmi_voice_voip_generate_summary( &calls_summary );
  if (calls_summary.nof_calls_overall)
  {
    result = TRUE;
  }
  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN_WITH_RET((int) result);
  return result;
} //qcril_qmi_voice_nas_control_is_any_calls_present

//===========================================================================
// qcril_qmi_voice_auto_answer_timeout_handler
//===========================================================================
void qcril_qmi_voice_auto_answer_timeout_handler( void * param )
{
  voice_answer_call_req_msg_v02               ans_call_req_msg;
  voice_answer_call_resp_msg_v02*             ans_call_resp_msg_ptr = NULL;
  IxErrnoType                                 res = E_FAILURE;
  qcril_qmi_voice_voip_call_info_entry_type   *call_info_entry = NULL;
  uint32 user_data;
  qmi_client_error_type client_err = QMI_NO_ERR;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( param );

  do
  {
    qcril_qmi_voice_voip_lock_overview();
    qmi_voice_voip_overview.auto_answer_timer_id = QMI_RIL_ZERO;

    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE );
    if ( NULL == call_info_entry ){
      QCRIL_LOG_INFO(".. pending incoming call record entry not found");
      break;
    }

    memset( &ans_call_req_msg, 0, sizeof( ans_call_req_msg ) );
    ans_call_req_msg.call_id = call_info_entry->qmi_call_id;
    QCRIL_LOG_INFO(".. call id qmi %d", (int) ans_call_req_msg.call_id );

    if ( call_info_entry->qmi_call_id == VOICE_INVALID_CALL_ID )
    {
      QCRIL_LOG_INFO(".. Invalid Call id!");
      break;
    }

    if ( qcril_qmi_voice_info.jbims )
    {
      ans_call_req_msg.call_type_valid = TRUE;
      ans_call_req_msg.call_type = call_info_entry->voice_scv_info.call_type;

      //Fill call type details for VT & VOIP calls from the call_info_entry
      if(call_info_entry->voice_scv_info.call_type == CALL_TYPE_VT_V02)
      {
        ans_call_req_msg.audio_attrib_valid = TRUE;
        ans_call_req_msg.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

        ans_call_req_msg.video_attrib_valid = TRUE;
        ans_call_req_msg.video_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
      }
    }

    ans_call_resp_msg_ptr = qcril_malloc( sizeof(*ans_call_resp_msg_ptr) );
    if( NULL == ans_call_resp_msg_ptr )
    {
      QCRIL_LOG_INFO(".. failed to allocate reponse buffer");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_ID_INTERNAL );
    // Send QMI VOICE ANSWER CALL REQ
    client_err =  qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_ANSWER_CALL_REQ_V02,
                                      &ans_call_req_msg,
                                      sizeof(ans_call_req_msg),
                                      ans_call_resp_msg_ptr,
                                      sizeof(*ans_call_resp_msg_ptr),
                                      (void*)(uintptr_t)user_data);
    if ( client_err )
    {
      QCRIL_LOG_INFO(".. failed to post qmi answer message");
      if ( NULL != ans_call_resp_msg_ptr )
      {
        qcril_free( ans_call_resp_msg_ptr );
        ans_call_resp_msg_ptr = NULL;
      }
    }
    else
    {
        call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ANSWERING_CALL;
#ifndef QMI_RIL_UTF
        qcril_am_handle_event(qcril_qmi_voice_get_answer_am_event(call_info_entry), NULL);
#endif
    }
  } while (FALSE);

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();
}
//===========================================================================
// RIL_REQUEST_ANSWER
//===========================================================================
void qcril_qmi_voice_request_answer
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  RIL_Errno                                   res = RIL_E_GENERIC_FAILURE;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();
  res = qcril_qmi_voice_send_request_answer(
                                params_ptr,
                                FALSE);

  if ( RIL_E_SUCCESS != res )
  {
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, res );
  }

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();

} //qcril_qmi_voice_request_answer

/*===========================================================================

  FUNCTION:  qcril_qmi_process_hangup_on_call_being_setup

===========================================================================*/
/*!
    @brief
    Retrieve call id for MO call for which RIL has not sent response back
    to RIL client.

    Update call info elaboration if need to wait for CC_IN_PROGRESS

    @return
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_process_hangup_on_call_being_setup
(
    int *conn_index
)
{
    qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = NULL;
    RIL_Errno                                  ril_err         = RIL_E_GENERIC_FAILURE;

    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
                                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_MO_CALL_BEING_SETUP,
                                TRUE );
    QCRIL_LOG_INFO( "call_info_entry %p", call_info_entry );

    if (call_info_entry)
    {
      QCRIL_LOG_INFO( "qmi call id %d android call id %d",
                      call_info_entry->qmi_call_id,
                      call_info_entry->android_call_id);
      if (VOICE_INVALID_CALL_ID == call_info_entry->qmi_call_id)
      {
        call_info_entry->elaboration |=
                          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_HANGUP_AFTER_VALID_QMI_ID;
        ril_err = RIL_E_SUCCESS;
      }
      else
      {
        *conn_index = call_info_entry->android_call_id;
        ril_err = RIL_E_SUCCESS;
      }
    }
    else
    {
       QCRIL_LOG_ERROR("Failed to find call entry, aborting!");
    }

    return ril_err;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_send_hangup_on_call

===========================================================================*/
/*!
    @brief
    Send hangup request for pending call

    @return
*/
/*=========================================================================*/
void qcril_qmi_voice_send_hangup_on_call
(
    int conn_index
)
{
    qcril_request_params_type  req_data = {0};
    qcril_reqlist_public_type  req_info;

    QCRIL_LOG_FUNC_ENTRY();

    /* Lookup the hangup request */
    if (qcril_reqlist_query_by_request(QCRIL_DEFAULT_INSTANCE_ID, RIL_REQUEST_HANGUP,
                &req_info) == E_SUCCESS)
    {
        req_data.modem_id       = QCRIL_DEFAULT_MODEM_ID;
        req_data.instance_id    = QCRIL_DEFAULT_INSTANCE_ID;
        req_data.datalen        = sizeof(conn_index);
        req_data.data           = &conn_index;
        req_data.t              = req_info.t;
        req_data.event_id       = req_info.request;

        qcril_reqlist_free(QCRIL_DEFAULT_INSTANCE_ID , req_data.t);
        qcril_qmi_voice_request_hangup(&req_data, NULL);
    }

    return;
}

//===========================================================================
//RIL_REQUEST_HANGUP
//===========================================================================
void qcril_qmi_voice_request_hangup
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  uint32                                          user_data;
  boolean                                         conn_index_valid = FALSE;
  int                                             conn_index = 0;
  boolean                                         conn_uri_valid = FALSE;
  const char*                                     conn_uri = NULL;
  qcril_reqlist_public_type                       reqlist_entry;
  voice_end_call_req_msg_v02                      call_end_req_msg;
  voice_end_call_resp_msg_v02*                    end_call_resp_msg_ptr = NULL;
  voice_manage_ip_calls_req_msg_v02               manage_ip_calls_req_msg;
  voice_manage_ip_calls_resp_msg_v02*             manage_ip_calls_resp_msg_ptr = NULL;
  voice_answer_call_req_msg_v02                   ans_call_req_msg;
  voice_answer_call_resp_msg_v02*                 ans_call_resp_msg_ptr = NULL;
  qcril_qmi_voice_voip_call_info_entry_type*      call_info_entry = NULL;
  qcril_qmi_voice_voip_current_call_summary_type  calls_summary;
  RIL_Errno                                       ril_err = RIL_E_GENERIC_FAILURE;
  qmi_client_error_type                           client_error;
  boolean                                         need_to_reject_incoming_call = FALSE;
  int32_t                                         ims_call_end_reason = 0;
  const char*                                     temp_buf = NULL;

  Ims__Hangup* ims_hangup_ptr = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  if ( qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate() )
  {
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
  }
  else
  {

    qcril_qmi_voice_voip_lock_overview();

    do
    {
      if ( RIL_REQUEST_HANGUP == params_ptr->event_id )
      {
        if ( NULL == params_ptr->data )
        {
          QCRIL_LOG_ERROR(".. invalid param");
          break;
        }
        conn_index_valid = TRUE;
        conn_index = *(( int *) params_ptr->data);

      }
      else
      {
        ims_hangup_ptr = (Ims__Hangup*) params_ptr->data;
        if (ims_hangup_ptr->has_conn_index)
        {
          conn_index_valid = TRUE;
          conn_index = ims_hangup_ptr->conn_index;
        }

        if (ims_hangup_ptr->conn_uri)
        {
          conn_uri_valid = TRUE;
          conn_uri = ims_hangup_ptr->conn_uri;
          QCRIL_LOG_INFO("conn_uri: %s", conn_uri);
        }

        if( ( FALSE == conn_index_valid ) && ( FALSE == conn_uri_valid ) )
        {
          QCRIL_LOG_ERROR("both conn_index and conn_uri are not available in is hangup request");
          break;
        }
        if (NULL != ims_hangup_ptr->failcauseresponse)
        {
          if (TRUE == ims_hangup_ptr->failcauseresponse->has_failcause)
          {
            if(IMS__CALL_FAIL_CAUSE__CALL_FAIL_MISC !=
               ims_hangup_ptr->failcauseresponse->failcause)
            {
              ims_call_end_reason =
              qcril_qmi_ims_map_ims_failcause_qmi_reject_cause(
              ims_hangup_ptr->failcauseresponse->failcause);

              if(INVALID_NEGATIVE_ONE != ims_call_end_reason)
              {
                need_to_reject_incoming_call = TRUE;
                QCRIL_LOG_INFO ("call end reason is %d", (int) ims_call_end_reason);
              }
            }
            else if (IMS__CALL_FAIL_CAUSE__CALL_FAIL_MISC ==
                ims_hangup_ptr->failcauseresponse->failcause &&
                TRUE == ims_hangup_ptr->failcauseresponse->has_errorinfo &&
                ims_hangup_ptr->failcauseresponse->errorinfo.len > 0)
            {
              temp_buf = (const char*) qcril_malloc(
                      ims_hangup_ptr->failcauseresponse->errorinfo.len + 1);
              if (temp_buf != NULL)
              {
                memcpy(temp_buf, ims_hangup_ptr->failcauseresponse->errorinfo.data,
                        ims_hangup_ptr->failcauseresponse->errorinfo.len);
                // Here no need for appending last byte of temp_buf with null
                // since qcril_malloc internally does the memset to 0.
                need_to_reject_incoming_call = TRUE;
                ims_call_end_reason = atoi(temp_buf);
                QCRIL_LOG_INFO ("call end reason is %d", (int) ims_call_end_reason);
              }
              else
              {
               QCRIL_LOG_ERROR ("qcril malloc failed");
               break;
              }
            }
          }
        }
      }

      QCRIL_LOG_INFO( "conn_index_valid: %d, conn_index: %d", conn_index_valid, conn_index );

      // add entry to ReqList
      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE,
                                   NULL,
                                   &reqlist_entry );
      if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
      {
          // Fail to add entry to ReqList
          QCRIL_LOG_ERROR(".. failed to Add into Req list");
          break;
      }

      if ( RIL_REQUEST_HANGUP == params_ptr->event_id )
      {
        /* Not documented in ril.h, if Hangup is coming with a connection index
           it is assumed that this is for the MO call that is being setup. */
        if (conn_index == -1)
        {
          if (RIL_E_SUCCESS !=
                 qcril_qmi_process_hangup_on_call_being_setup(&conn_index))
          {
            /* Could not retrieve call id */
            break;
          }
          else if (conn_index == -1)
          {
            /* Have not received call id, hence we will wait to receive call id. */
            ril_err = RIL_E_SUCCESS;
            break;
          }
        }
      }

      // preparation
      user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

      qcril_qmi_voice_voip_generate_summary( &calls_summary );

      // find call info
      if (conn_index_valid)
      {
        call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id( conn_index );
        if (NULL == call_info_entry)
        {
           QCRIL_LOG_ERROR("Failed to find call entry, aborting!");
           break;
        }

        if (need_to_reject_incoming_call &&
              !(call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING))
        {
          QCRIL_LOG_ERROR("incoming call not found");
          need_to_reject_incoming_call = FALSE;
        }
      }

      if (call_info_entry && need_to_reject_incoming_call)
      {
        ans_call_resp_msg_ptr = qcril_malloc (sizeof (*ans_call_resp_msg_ptr));
        if (NULL == ans_call_resp_msg_ptr)
        {
          QCRIL_LOG_ERROR ("qcril malloc failed");
          break;
        }

        memset (&ans_call_req_msg, 0, sizeof (ans_call_req_msg));
        ans_call_req_msg.call_id = call_info_entry->qmi_call_id;
        ans_call_req_msg.reject_call_valid = TRUE;
        ans_call_req_msg.reject_call = TRUE;
        ans_call_req_msg.reject_cause_valid = TRUE;
        ans_call_req_msg.reject_cause = ims_call_end_reason;
        ril_err = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_ANSWER_CALL_REQ_V02,
                                          &ans_call_req_msg,
                                          sizeof(ans_call_req_msg),
                                          ans_call_resp_msg_ptr,
                                          sizeof(*ans_call_resp_msg_ptr),
                                          (void*)(uintptr_t)user_data);
      }
      else if (call_info_entry &&  conn_index_valid && !conn_uri_valid )
      {
        end_call_resp_msg_ptr = qcril_malloc( sizeof(*end_call_resp_msg_ptr) );
        if( NULL == end_call_resp_msg_ptr )
        {
          QCRIL_LOG_ERROR("qcril_malloc failed");
          break;
        }

        memset( &call_end_req_msg, 0, sizeof(call_end_req_msg) );
        call_end_req_msg.call_id = call_info_entry->qmi_call_id;

        ril_err = qcril_qmi_client_send_msg_async_ex ( QCRIL_QMI_CLIENT_VOICE,
                                                       QMI_VOICE_END_CALL_REQ_V02,
                                                       &call_end_req_msg,
                                                       sizeof(call_end_req_msg),
                                                       end_call_resp_msg_ptr,
                                                       sizeof(*end_call_resp_msg_ptr),
                                                       (void*)(uintptr_t)user_data  );
      }
      else if (conn_uri_valid && NULL != conn_uri)
      {
        manage_ip_calls_resp_msg_ptr = qcril_malloc( sizeof(*manage_ip_calls_resp_msg_ptr) );
        if( NULL == manage_ip_calls_resp_msg_ptr )
        {
          QCRIL_LOG_ERROR("qcril_malloc failed");
          break;
        }

        if (strlen(conn_uri) > QMI_VOICE_SIP_URI_MAX_V02)
        {
          QCRIL_LOG_ERROR("strlen(conn_uri) > QMI_VOICE_SIP_URI_MAX_V02");
          break;
        }

        if (call_info_entry)
        {
            call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONF_PATICIAPNT_CALL_END_REPORTED;
        }

        memset( &manage_ip_calls_req_msg, 0, sizeof(manage_ip_calls_req_msg) );
        manage_ip_calls_req_msg.sups_type = VOIP_SUPS_TYPE_RELEASE_SPECIFIED_CALL_FROM_CONFERENCE_V02;
        manage_ip_calls_req_msg.sip_uri_valid = TRUE;
        memcpy( manage_ip_calls_req_msg.sip_uri, conn_uri, strlen(conn_uri) );
        client_error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                                       QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                       &manage_ip_calls_req_msg,
                                                       sizeof(manage_ip_calls_req_msg),
                                                       manage_ip_calls_resp_msg_ptr,
                                                       sizeof(*manage_ip_calls_resp_msg_ptr),
                                                       (void*)(uintptr_t)user_data );
        ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(client_error, NULL);

      }

      if ( RIL_E_SUCCESS != ril_err )
        break;

    } while ( FALSE );

    qcril_qmi_voice_voip_unlock_overview();

    // respond
    if ( RIL_E_SUCCESS != ril_err )
    {
        qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err );

        if( NULL != call_info_entry )
        {
          call_info_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONF_PATICIAPNT_CALL_END_REPORTED;
        }
        if ( end_call_resp_msg_ptr )
        {
          qcril_free( end_call_resp_msg_ptr );
        }
        if ( manage_ip_calls_resp_msg_ptr )
        {
          qcril_free( manage_ip_calls_resp_msg_ptr );
        }
        if (ans_call_resp_msg_ptr)
        {
            qcril_free (ans_call_resp_msg_ptr);
        }
    }
  }
  if ( ims_hangup_ptr )
  {
    qcril_qmi_ims__hangup__free_unpacked(ims_hangup_ptr, NULL);
  }
  if (temp_buf)
  {
    qcril_free( temp_buf );
  }
  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_hangup

static void qcril_qmi_voice_set_dtmf_concurrency_requirement(qcril_reqlist_generic_concurency_requests_requirement_type *req_ptr)
{
  static uint32 req_ids[] = {
    RIL_REQUEST_DTMF,
    RIL_REQUEST_DTMF_START,
    RIL_REQUEST_DTMF_STOP
  };
  req_ptr->max_concurrency = 1;
  req_ptr->max_pending = 50;
  req_ptr->req_ids_num = sizeof(req_ids)/sizeof(req_ids[0]);
  req_ptr->req_ids = req_ids;
}

static void qcril_qmi_voice_send_start_cont_dtmf_request
(
  qcril_instance_id_e_type instance_id,
  struct qcril_reqlist_buf_tag *req,
  void *data,
  size_t datalen
)
{
  uint32 user_data;
  voice_start_cont_dtmf_req_msg_v02 *start_cont_dtmf_req_msg_ptr = data;
  voice_start_cont_dtmf_resp_msg_v02* start_cont_dtmf_resp_msg_ptr = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(datalen);

  do
  {
    if( NULL == start_cont_dtmf_req_msg_ptr )
    {
      QCRIL_LOG_DEBUG("start_cont_dtmf_req_msg_ptr is NULL");
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }
    start_cont_dtmf_resp_msg_ptr = qcril_malloc( sizeof(*start_cont_dtmf_resp_msg_ptr) );
    if( NULL == start_cont_dtmf_resp_msg_ptr )
    {
      QCRIL_LOG_DEBUG("start_cont_dtmf_resp_msg_ptr is NULL");
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, req->pub.req_id );

    qcril_qmi_voice_info_lock();
    qcril_qmi_voice_info.pending_dtmf_req_id = req->pub.req_id;
    qcril_qmi_voice_info_unlock();
    QCRIL_LOG_DEBUG("dtmf req id %d", req->pub.req_id);

    /* Send QMI VOICE START CONT DTMF REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_START_CONT_DTMF_REQ_V02,
                                      start_cont_dtmf_req_msg_ptr,
                                      sizeof(*start_cont_dtmf_req_msg_ptr),
                                      start_cont_dtmf_resp_msg_ptr,
                                      sizeof(*start_cont_dtmf_resp_msg_ptr),
                                      (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      qcril_qmi_voice_info_lock();
      qcril_qmi_voice_info.pending_dtmf_req_id = QMI_RIL_ZERO; // reset
      qcril_qmi_voice_info_unlock();

      /* In case of ARM9 reset, the command callback will never be executed. So, need to
         delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( start_cont_dtmf_resp_msg_ptr );
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
    }

    qcril_qmi_voice_info_lock();
    QCRIL_LOG_DEBUG("dtmf req id %d", (int)qcril_qmi_voice_info.pending_dtmf_req_id);
    qcril_qmi_voice_info_unlock();

  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_dtmf

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DTMF.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_dtmf
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  char * in_data_ptr;
  qcril_reqlist_public_type reqlist_entry;
  voice_start_cont_dtmf_req_msg_v02 start_cont_dtmf_req_msg;
  Ims__Dtmf *ims_in_data_ptr = NULL;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/
  QCRIL_LOG_FUNC_ENTRY();

  do
  {
        if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
        {
            qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
            break;
        }

        /* Assuming CM will do DTMF digit range check */

        // fill in the req msg fields
        memset(&start_cont_dtmf_req_msg,0, sizeof(start_cont_dtmf_req_msg));
        /* Modem determines the active call */
        start_cont_dtmf_req_msg.cont_dtmf_info.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;

        if ( RIL_REQUEST_DTMF == params_ptr->event_id )
        {
            in_data_ptr = (char *)params_ptr->data;
            QCRIL_LOG_INFO("Cont dtmf request with digit %c is being sent",in_data_ptr[ 0 ]);
            start_cont_dtmf_req_msg.cont_dtmf_info.digit = in_data_ptr[ 0 ];
        }
        else
        {
            ims_in_data_ptr = (Ims__Dtmf *)params_ptr->data;
            if (NULL != ims_in_data_ptr->dtmf)
            {
               QCRIL_LOG_INFO("Cont dtmf request with digit %c is being sent", ims_in_data_ptr->dtmf[0]);
               start_cont_dtmf_req_msg.cont_dtmf_info.digit = ims_in_data_ptr->dtmf[0];
            }
            else
            {
               QCRIL_LOG_ERROR("no dtmf digit in the request");
               qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
               break;
            }
        }

        /* Add entry to ReqList */
        /* For GW system, we sent start_cont_dtmf followed by stop_cont_dtmf to simulate burst_dtmf */
        qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                     QCRIL_EVT_QMI_VOICE_BURST_START_CONT_DTMF, NULL, &reqlist_entry );
        qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
        qcril_qmi_voice_set_dtmf_concurrency_requirement(&concurency_requirement);
        qcril_reqlist_buf_type *req_buf;

        IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                                &reqlist_entry,
                                                                                &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                                &concurency_requirement,
                                                                                sizeof(concurency_requirement),
                                                                                &qcril_qmi_voice_send_start_cont_dtmf_request,
                                                                                &start_cont_dtmf_req_msg,
                                                                                sizeof(start_cont_dtmf_req_msg),
                                                                                &req_buf );
        if ( E_SUCCESS == new_req_result )
        {
          qcril_qmi_voice_send_start_cont_dtmf_request( instance_id, req_buf, &start_cont_dtmf_req_msg, sizeof(start_cont_dtmf_req_msg) );
        }
        else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
        {
          QCRIL_LOG_INFO("new request is blocked.");
        }
        else
        {
          /* Fail to add to ReqList */
          QCRIL_LOG_INFO("new request is rejected.");
          qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        }

  } while ( FALSE );
  if ( NULL != ims_in_data_ptr )
  {
    qcril_qmi_ims__dtmf__free_unpacked(ims_in_data_ptr, NULL);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_dtmf() */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_dtmf_start

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DTMF_START.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_dtmf_start
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  char *in_data_ptr;
  qcril_reqlist_public_type reqlist_entry;
  voice_start_cont_dtmf_req_msg_v02 start_cont_dtmf_req_msg;
  Ims__Dtmf *ims_in_data_ptr = NULL;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    // fill in the req msg fields
    memset(&start_cont_dtmf_req_msg,0, sizeof(start_cont_dtmf_req_msg));
    /* QMI on Modem determines the active/alerting call */
    start_cont_dtmf_req_msg.cont_dtmf_info.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;

    if ( RIL_REQUEST_DTMF_START == params_ptr->event_id )
    {
      in_data_ptr = (char *)params_ptr->data;
      QCRIL_LOG_INFO("Cont dtmf request with digit %c is being sent",in_data_ptr[ 0 ]);
      start_cont_dtmf_req_msg.cont_dtmf_info.digit = in_data_ptr[ 0 ];
    }
    else
    {
      ims_in_data_ptr = (Ims__Dtmf *)params_ptr->data;
      if (NULL != ims_in_data_ptr->dtmf)
      {
         QCRIL_LOG_INFO("Cont dtmf request with digit %c is being sent", ims_in_data_ptr->dtmf[0]);
         start_cont_dtmf_req_msg.cont_dtmf_info.digit = ims_in_data_ptr->dtmf[0];
      }
      else
      {
         QCRIL_LOG_ERROR("no dtmf digit in the request");
         qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
         break;
      }
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
    qcril_qmi_voice_set_dtmf_concurrency_requirement(&concurency_requirement);
    qcril_reqlist_buf_type *req_buf;

    IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                            &reqlist_entry,
                                                                            &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                            &concurency_requirement,
                                                                            sizeof(concurency_requirement),
                                                                            &qcril_qmi_voice_send_start_cont_dtmf_request,
                                                                            &start_cont_dtmf_req_msg,
                                                                            sizeof(start_cont_dtmf_req_msg),
                                                                            &req_buf );
    if ( E_SUCCESS == new_req_result )
    {
      qcril_qmi_voice_send_start_cont_dtmf_request( instance_id, req_buf, &start_cont_dtmf_req_msg, sizeof(start_cont_dtmf_req_msg) );
    }
    else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
    {
      QCRIL_LOG_INFO("new request is blocked.");
    }
    else
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_INFO("new request is rejected.");
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);

  if ( NULL != ims_in_data_ptr )
  {
      qcril_qmi_ims__dtmf__free_unpacked(ims_in_data_ptr, NULL);
  }
} /* qcril_qmi_voice_request_dtmf_start() */

static void qcril_qmi_voice_send_stop_cont_dtmf_request
(
  qcril_instance_id_e_type instance_id,
  struct qcril_reqlist_buf_tag *req,
  void *data,
  size_t datalen
)
{
  uint32 user_data;
  voice_stop_cont_dtmf_req_msg_v02 *stop_cont_dtmf_req_msg_ptr = data;
  voice_stop_cont_dtmf_resp_msg_v02* stop_cont_dtmf_resp_msg_ptr = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(datalen);

  do
  {
    if( NULL == stop_cont_dtmf_req_msg_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }
    stop_cont_dtmf_resp_msg_ptr = qcril_malloc( sizeof(*stop_cont_dtmf_resp_msg_ptr) );
    if( NULL == stop_cont_dtmf_resp_msg_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, req->pub.req_id );

    qcril_qmi_voice_info_lock();
    qcril_qmi_voice_info.pending_dtmf_req_id = req->pub.req_id;
    qcril_qmi_voice_info_unlock();

    /* Send QMI VOICE START CONT DTMF REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_STOP_CONT_DTMF_REQ_V02,
                                      stop_cont_dtmf_req_msg_ptr,
                                      sizeof(*stop_cont_dtmf_req_msg_ptr),
                                      stop_cont_dtmf_resp_msg_ptr,
                                      sizeof(*stop_cont_dtmf_resp_msg_ptr),
                                      (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      qcril_qmi_voice_info_lock();
      qcril_qmi_voice_info.pending_dtmf_req_id = QMI_RIL_ZERO;
      qcril_qmi_voice_info_unlock();

      /* In case of ARM9 reset, the command callback will never be executed. So, need to
         delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( stop_cont_dtmf_resp_msg_ptr );
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }

    qcril_qmi_voice_info_lock();
    QCRIL_LOG_DEBUG("dtmf req id %d", qcril_qmi_voice_info.pending_dtmf_req_id);
    qcril_qmi_voice_info_unlock();

  } while ( FALSE );
  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_dtmf_stop

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_DTMF_STOP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_dtmf_stop
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_reqlist_public_type reqlist_entry;
  voice_stop_cont_dtmf_req_msg_v02 stop_cont_dtmf_req_msg;

  QCRIL_LOG_FUNC_ENTRY();

  /*-----------------------------------------------------------------------*/

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    // fill in the req msg fields
    memset(&stop_cont_dtmf_req_msg,0, sizeof(stop_cont_dtmf_req_msg));
    /* QMI on Modem determines the active/alerting call */
    stop_cont_dtmf_req_msg.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
    qcril_qmi_voice_set_dtmf_concurrency_requirement(&concurency_requirement);
    qcril_reqlist_buf_type *req_buf;

    IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                            &reqlist_entry,
                                                                            &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                            &concurency_requirement,
                                                                            sizeof(concurency_requirement),
                                                                            &qcril_qmi_voice_send_stop_cont_dtmf_request,
                                                                            &stop_cont_dtmf_req_msg,
                                                                            sizeof(stop_cont_dtmf_req_msg),
                                                                            &req_buf );
    if ( E_SUCCESS == new_req_result )
    {
      qcril_qmi_voice_send_stop_cont_dtmf_request( instance_id, req_buf, &stop_cont_dtmf_req_msg, sizeof(stop_cont_dtmf_req_msg) );
    }
    else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
    {
      QCRIL_LOG_INFO("new request is blocked.");
    }
    else
    {
      /* Fail to add to ReqList */
      QCRIL_LOG_INFO("new request is rejected.");
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_dtmf_stop() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_cdma_burst_dtmf

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_BURST_DTMF.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_cdma_burst_dtmf
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint32 user_data;
  char *dtmf_ptr;
  char *onlength_ptr = NULL;
  char *offlength_ptr = NULL;
  uint8 on_length = 1; /* DTMF pulse width, 150ms */
  uint8 off_length = 2; /* DTMF inter-digit interval, 150ms */
  long ret_val;
  char *end_ptr;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_burst_dtmf_req_msg_v02  cdma_burst_dtmf_req_msg;
  voice_burst_dtmf_resp_msg_v02* dtmf_resp_msg_ptr = NULL;
  int dtmf_str_len = 0;

  /*-----------------------------------------------------------------------*/

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
  /* Verify the parameters are valid */
    if ( params_ptr->datalen == 0 || params_ptr->data == NULL)
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
      break;
    }

    dtmf_ptr =  ((char **)params_ptr->data)[ 0 ];
    onlength_ptr = ((char **)params_ptr->data)[ 1 ];
    offlength_ptr = ((char **)params_ptr->data)[ 2 ];
    if( dtmf_ptr == NULL || onlength_ptr == NULL || offlength_ptr == NULL )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
  }

  dtmf_str_len = strlen(dtmf_ptr);

  if( dtmf_str_len > QMI_VOICE_DIGIT_BUFFER_MAX_V02)
  {
    QCRIL_LOG_ERROR("Length of Dtmf string received:%d, maximum length supported:%d",dtmf_str_len,QMI_VOICE_DIGIT_BUFFER_MAX_V02);
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
      break;
  }

  /* Convert DTMF ON length string to DTMF ON duration */
  ret_val = strtol( onlength_ptr, &end_ptr, 0 );
  if ( ( errno == ERANGE ) && ( ( ret_val == LONG_MAX ) || ( ret_val == LONG_MIN ) ) )
  {
    QCRIL_LOG_ERROR( "Fail to convert DTMF ON length str = %s, default to: DTMF_ONLENGTH_SMS \n", onlength_ptr );
  }

  switch ( ret_val )
  {
    case 95 :
      on_length = DTMF_ONLENGTH_95MS_V02;
      break;
    /* As per the Ril interface - 0 maps to default value */
    case 0   :
    case 150 :
      on_length = DTMF_ONLENGTH_150MS_V02;
      break;
    case 200 :
      on_length = DTMF_ONLENGTH_200MS_V02;
      break;
    case 250 :
      on_length = DTMF_ONLENGTH_250MS_V02;
      break;
    case 300 :
      on_length = DTMF_ONLENGTH_300MS_V02;
      break;
    case 350 :
      on_length = DTMF_ONLENGTH_350MS_V02;
      break;
    default :
      on_length = DTMF_ONLENGTH_SMS_V02;
      break;
  }
  QCRIL_LOG_DEBUG( "DTMF ON length ret_val = %p, on_length_val = %d", onlength_ptr , on_length );

  /* Convert DTMF OFF length string to DTMF OFF duration */
  ret_val = strtol( offlength_ptr, &end_ptr, 0 );
  if ( ( errno == ERANGE ) && ( ( ret_val == LONG_MAX ) || ( ret_val == LONG_MIN ) ) )
  {
    QCRIL_LOG_ERROR( "Fail to convert DTMF OFF length %s, default to: DTMF_OFFLENGTH_150MS (150msecs)", offlength_ptr );
  }

  switch ( ret_val )
  {
    case 60 :
      off_length = DTMF_OFFLENGTH_60MS_V02;
      break;
    case 100 :
      off_length = DTMF_OFFLENGTH_100MS_V02;
      break;
    case 150 :
      off_length = DTMF_OFFLENGTH_150MS_V02;
      break;
    case 200 :
      off_length = DTMF_OFFLENGTH_200MS_V02;
      break;
    default :
      off_length = DTMF_OFFLENGTH_150MS_V02;
      break;
  }
  QCRIL_LOG_DEBUG( "DTMF OFF length ret_val = %p, Off_length_val = %d", offlength_ptr , off_length );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
      break;
  }

  memset(&cdma_burst_dtmf_req_msg,0, sizeof(cdma_burst_dtmf_req_msg));
  QCRIL_LOG_INFO("CDMA Burst dtmf request with string %s is being sent",dtmf_ptr);
  /* Modem determines the active call */
  cdma_burst_dtmf_req_msg.burst_dtmf_info.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;
  cdma_burst_dtmf_req_msg.burst_dtmf_info.digit_buffer_len = (dtmf_str_len < QMI_VOICE_DIGIT_BUFFER_MAX_V02)?dtmf_str_len:QMI_VOICE_DIGIT_BUFFER_MAX_V02;
  memcpy(cdma_burst_dtmf_req_msg.burst_dtmf_info.digit_buffer, dtmf_ptr, cdma_burst_dtmf_req_msg.burst_dtmf_info.digit_buffer_len);

  cdma_burst_dtmf_req_msg.dtmf_lengths_valid = TRUE;
  cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_offlength = off_length;
  cdma_burst_dtmf_req_msg.dtmf_lengths.dtmf_onlength = on_length;

  dtmf_resp_msg_ptr = qcril_malloc( sizeof(*dtmf_resp_msg_ptr) );
  if( NULL == dtmf_resp_msg_ptr )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    break;
  }

  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

  /* Send QMI VOICE BURST DTMF REQ */
  if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                    QMI_VOICE_BURST_DTMF_REQ_V02,
                                    &cdma_burst_dtmf_req_msg,
                                    sizeof(cdma_burst_dtmf_req_msg),
                                    dtmf_resp_msg_ptr,
                                    sizeof(*dtmf_resp_msg_ptr),
                                    (void*)(uintptr_t)user_data) != E_SUCCESS )
  {
    /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
    qcril_free( dtmf_resp_msg_ptr );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
  }while(0);

} /* qcril_qmi_voice_request_cdma_burst_dtmf() */

//===========================================================================
// qcril_qmi_voice_parse_call_modify_param
//===========================================================================
boolean qcril_qmi_voice_parse_call_modify_param(int event_id, const void* data_ptr, int* callIndex, RIL_Call_Details** callDetails)
{
  boolean ret = 0;

  if (NULL != data_ptr && NULL != callIndex && NULL != callDetails)
  {
    if (RIL_REQUEST_MODIFY_CALL_INITIATE == event_id || RIL_REQUEST_MODIFY_CALL_CONFIRM == event_id)
    {
       const RIL_Call_Modify *modify_input_param = data_ptr;
       *callIndex = modify_input_param->callIndex;
       *callDetails = modify_input_param->callDetails;
    }
    else if (QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE == event_id || QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM == event_id)
    {
       const Ims__CallModify *ims_modify_input_ptr = data_ptr;
       *callIndex = ims_modify_input_ptr->callindex;
       *callDetails = qcril_malloc(sizeof(RIL_Call_Details));
       if (NULL != *callDetails)
       {
          qcril_qmi_ims_translate_ims_calldetails_to_ril_calldetails(ims_modify_input_ptr->calldetails, *callDetails);
       }
       else
       {
          QCRIL_LOG_ERROR("callDetails malloc failed");
          ret = 1;
       }
    }
    else
    {
       QCRIL_LOG_ERROR("wrong request %d for parsing", event_id);
       ret = 1;
    }
  }
  else
  {
    ret = 1;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int)ret);
  return ret;
} // qcril_qmi_voice_parse_call_modify_param

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_modify_call_initiate

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_MODIFY_CALL_INITIATE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_modify_call_initiate
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
   RIL_Errno result = RIL_E_SUCCESS;
   int callIndex;
   RIL_Call_Details *callDetails = NULL;
   voice_manage_ip_calls_req_msg_v02   modify_req;
   voice_manage_ip_calls_resp_msg_v02   *modify_resp = NULL;
   qmi_client_error_type client_error;
   uint32 user_data;
   qcril_reqlist_public_type                   reqlist_entry;
   qcril_request_resp_params_type   resp;
   qcril_qmi_voice_voip_call_info_entry_type  *call_info = NULL;

   QCRIL_LOG_FUNC_ENTRY();
   QCRIL_NOTUSED(ret_ptr);

   if( params_ptr->data == NULL || params_ptr->datalen <= 0 || !(qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) || qcril_qmi_voice_info.jbims) )
   {
      result = RIL_E_GENERIC_FAILURE;
   }
   else
   {
      if (qcril_qmi_voice_parse_call_modify_param(params_ptr->event_id, params_ptr->data, &callIndex, &callDetails))
      {
         result = RIL_E_GENERIC_FAILURE;
      }

      if (QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE == params_ptr->event_id)
      {
         qcril_qmi_ims__call_modify__free_unpacked((Ims__CallModify *)params_ptr->data, NULL);
      }

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_QMI_REQUEST_MODIFY_INITIATE,
                                   NULL,
                                   &reqlist_entry );

      if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
      {
        QCRIL_LOG_ERROR("Failed to Add into Req list");
        result = RIL_E_GENERIC_FAILURE;
      }

      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id( (uint8_t)callIndex );

      if( call_info == NULL )
      {
         QCRIL_LOG_DEBUG( "android call-id = %d not found, rejecting modify initiate request", callIndex );
         result = RIL_E_GENERIC_FAILURE;
      }

      if( result == RIL_E_SUCCESS )
      {
         memset(&modify_req, 0, sizeof(modify_req));
         if (qcril_qmi_voice_get_modem_call_type_info( callDetails,
                                             &modify_req.call_type,
                                             &modify_req.audio_attrib_valid,
                                             &modify_req.audio_attrib,
                                             &modify_req.video_attrib_valid,
                                             &modify_req.video_attrib ))
         {
            modify_req.call_type_valid = TRUE;
            modify_req.call_id_valid = TRUE;
            modify_req.call_id = call_info->qmi_call_id;
            modify_req.sups_type = VOIP_SUPS_TYPE_MODIFY_CALL_V02;

            user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
            modify_resp = qcril_malloc( sizeof(*modify_resp) );

            client_error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                                    QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                    &modify_req,
                                                    sizeof(modify_req),
                                                    modify_resp,
                                                    sizeof(*modify_resp),
                                                    (void*)(uintptr_t)user_data );

            QCRIL_LOG_INFO(".. qmi send async res %d", (int) client_error );

            result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( client_error, NULL );
         }
         else
         {
            QCRIL_LOG_DEBUG("qcril_qmi_voice_get_modem_call_type_info() returns error");
            result = RIL_E_GENERIC_FAILURE;
         }
      }

      if (QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE == params_ptr->event_id && NULL != callDetails)
      {
         qcril_free(callDetails);
      }
   }

   QCRIL_LOG_INFO("result is %d", result);
   if( result != RIL_E_SUCCESS )
   {
      if (NULL != modify_resp)
      {
         qcril_free( modify_resp );
      }
      qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_request_modify_call_initiate

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_modify_call_confirm

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_MODIFY_CALL_CONFIRM.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_modify_call_confirm
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
   RIL_Errno result = RIL_E_SUCCESS;
   int callIndex;
   RIL_Call_Details *callDetails = NULL;
   voice_manage_ip_calls_req_msg_v02   modify_confirm_req;
   voice_manage_ip_calls_resp_msg_v02   *modify_confirm_resp = NULL;
   qmi_client_error_type client_error;
   uint32 user_data;
   qcril_reqlist_public_type                   reqlist_entry;
   qcril_request_resp_params_type   resp;
   qcril_qmi_voice_voip_call_info_entry_type  *call_info = NULL;
   boolean is_audio_attrib_valid, is_video_attrib_valid, accept;

   QCRIL_LOG_FUNC_ENTRY();
   QCRIL_NOTUSED(ret_ptr);

   if( params_ptr->data == NULL || params_ptr->datalen <= 0 || !(qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) || qcril_qmi_voice_info.jbims) )
   {
      result = RIL_E_GENERIC_FAILURE;
   }
   else
   {
      if (qcril_qmi_voice_parse_call_modify_param(params_ptr->event_id, params_ptr->data, &callIndex, &callDetails))
      {
         result = RIL_E_GENERIC_FAILURE;
      }

      if (QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM == params_ptr->event_id)
      {
         qcril_qmi_ims__call_modify__free_unpacked((Ims__CallModify *)params_ptr->data, NULL);
      }

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_QMI_REQUEST_MODIFY_CONFIRM,
                                   NULL,
                                   &reqlist_entry );

      if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
      {
        QCRIL_LOG_ERROR("Failed to Add into Req list");
        result = RIL_E_GENERIC_FAILURE;
      }

      call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id( (uint8_t)callIndex );
      if( call_info == NULL )
      {
         QCRIL_LOG_DEBUG("call-id not found, rejecting  modify confirm request");
         result = RIL_E_GENERIC_FAILURE;
      }

      if( result == RIL_E_SUCCESS )
      {
         memset(&modify_confirm_req, 0, sizeof(modify_confirm_req));
         if (qcril_qmi_voice_get_modem_call_type_info( callDetails,
                                             &modify_confirm_req.call_type,
                                             &modify_confirm_req.audio_attrib_valid,
                                             &modify_confirm_req.audio_attrib,
                                             &modify_confirm_req.video_attrib_valid,
                                             &modify_confirm_req.video_attrib )
             )
         {

            is_audio_attrib_valid = (call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUDIO_ATTR_VALID) ? TRUE : FALSE;
            is_video_attrib_valid = (call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VIDEO_ATTR_VALID) ? TRUE : FALSE;

            accept = !qcril_qmi_voice_match_modem_call_type( modify_confirm_req.call_type,
                                                            modify_confirm_req.audio_attrib_valid,
                                                            modify_confirm_req.audio_attrib,
                                                            modify_confirm_req.video_attrib_valid,
                                                            modify_confirm_req.video_attrib,
                                                            call_info->voice_scv_info.call_type,
                                                            is_audio_attrib_valid,
                                                            call_info->voice_audio_attrib.call_attributes,
                                                            is_video_attrib_valid,
                                                            call_info->voice_video_attrib.call_attributes);

            modify_confirm_req.call_type_valid = TRUE;
            modify_confirm_req.call_id_valid = TRUE;
            modify_confirm_req.call_id = call_info->qmi_call_id;

            if( accept )
            {
               modify_confirm_req.sups_type = VOIP_SUPS_TYPE_MODIFY_ACCEPT_V02;
            }
            else
            {
               modify_confirm_req.sups_type = VOIP_SUPS_TYPE_MODIFY_REJECT_V02;
            }

            user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
            modify_confirm_resp = qcril_malloc( sizeof(*modify_confirm_resp) );

            client_error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                                    QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                    &modify_confirm_req,
                                                    sizeof(modify_confirm_req),
                                                    modify_confirm_resp,
                                                    sizeof(*modify_confirm_resp),
                                                    (void*)(uintptr_t)user_data );

            QCRIL_LOG_INFO(".. qmi send async res %d", (int) client_error );

            result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( client_error, NULL );

            if ( RIL_E_SUCCESS == result && QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM == params_ptr->event_id )
            {
               call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_MODIFY_CONFIRM_PENDING;

            }
         }
         else
         {
            QCRIL_LOG_DEBUG("qcril_qmi_voice_get_modem_call_type_info() returns error");
            result = RIL_E_GENERIC_FAILURE;
         }
      }

      if ( QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM == params_ptr->event_id && NULL != callDetails )
      {
         qcril_free(callDetails);
      }
   }

   QCRIL_LOG_INFO("result is %d", result);
   if( result != RIL_E_SUCCESS )
   {
      if (NULL != modify_confirm_resp)
      {
         qcril_free( modify_confirm_resp );
      }
      qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_request_modify_call_confirm

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_cdma_flash

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_FLASH.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_cdma_flash
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  char *flash_ptr;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_send_flash_req_msg_v02  send_flash_req_msg;
  voice_send_flash_resp_msg_v02* send_flash_resp_msg_ptr = NULL;
  int flash_str_len = 0;


  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  flash_ptr = ( char * )params_ptr->data;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  /* Lookup as_id */

  do
  {
  if(flash_ptr != NULL)
  {
    flash_str_len = strlen(flash_ptr);
    if(flash_str_len > QMI_VOICE_FLASH_PAYLOAD_MAX_V02)
    {
      QCRIL_LOG_ERROR("Length of flash string received:%d, maximum length supported:%d",flash_str_len,QMI_VOICE_FLASH_PAYLOAD_MAX_V02);
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
        break;
    }
  }

  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
      break;
  }
  memset(&send_flash_req_msg,0,sizeof(send_flash_req_msg));
  send_flash_req_msg.call_id = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;
  /* Flash with no info */
  if ( flash_ptr != NULL )
  {
    memcpy(send_flash_req_msg.flash_payload, flash_ptr, flash_str_len);
    send_flash_req_msg.flash_payload_valid = TRUE;
  }

  send_flash_resp_msg_ptr = qcril_malloc( sizeof(*send_flash_resp_msg_ptr) );
  if( NULL == send_flash_resp_msg_ptr )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    break;
  }

  /* Command CM to send FLASH */
  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

  /* Send QMI VOICE BURST DTMF REQ */
  if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                    QMI_VOICE_SEND_FLASH_REQ_V02,
                                    &send_flash_req_msg,
                                    sizeof(send_flash_req_msg),
                                    send_flash_resp_msg_ptr,
                                    sizeof(*send_flash_resp_msg_ptr),
                                    (void*)(uintptr_t)user_data) != E_SUCCESS )
  {
    /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
    qcril_free( send_flash_resp_msg_ptr );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
  }while(0);
} /* qcril_qmi_voice_request_cdma_flash() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_set_preferred_voice_privacy_mode

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE.

    @return
    None.
*/
/*=========================================================================*/

void qcril_qmi_voice_request_set_preferred_voice_privacy_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  int *in_data_ptr;
  qcril_request_resp_params_type resp;
  voice_set_preferred_privacy_req_msg_v02 set_preferred_privacy_req_msg;
  voice_set_preferred_privacy_resp_msg_v02 set_preferred_privacy_resp_msg;


  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  in_data_ptr = (int *)params_ptr->data;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
        break;
  }

  memset(&set_preferred_privacy_req_msg, 0, sizeof(set_preferred_privacy_req_msg));
  switch(*in_data_ptr)
  {
      case 0:
          set_preferred_privacy_req_msg.privacy_pref = 0x00;
          break;
      case 1:
          set_preferred_privacy_req_msg.privacy_pref = 0x01;
          break;
      default:
          set_preferred_privacy_req_msg.privacy_pref = 0x00;
  }
  QCRIL_LOG_INFO("voice privacy mode preference set %d",set_preferred_privacy_req_msg.privacy_pref);

  memset(&set_preferred_privacy_resp_msg, 0, sizeof(set_preferred_privacy_resp_msg));
  /* Send QMI VOICE SET PREFERRED PRIVACY REQ */
  if ( qcril_qmi_client_send_msg_sync ( QCRIL_QMI_CLIENT_VOICE,
                                    QMI_VOICE_SET_PREFERRED_PRIVACY_REQ_V02,
                                    &set_preferred_privacy_req_msg,
                                    sizeof(set_preferred_privacy_req_msg),
                                    &set_preferred_privacy_resp_msg,
                                    sizeof(set_preferred_privacy_resp_msg)) != E_SUCCESS )
  {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
  }
  else
  {
      if(set_preferred_privacy_resp_msg.resp.result == QMI_RESULT_FAILURE_V01)
      {
          QCRIL_LOG_INFO("voice privacy mode preference set error %d",set_preferred_privacy_resp_msg.resp.error);
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
          qcril_send_request_response( &resp );
      }
      else
      {
          qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
          qcril_send_request_response( &resp );
      }
  }
  }while(0);
} /* qcril_qmi_voice_request_set_preferred_voice_privacy_mode() */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_query_preferred_privacy_mode

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE.

    @return
    None.
*/
/*=========================================================================*/

void qcril_qmi_voice_request_query_preferred_voice_privacy_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;
  int privacy_preference=0;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  voice_get_config_req_msg_v02 qmi_request;
  voice_get_config_resp_msg_v02 qmi_response;

  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  memset( &qmi_request, 0, sizeof( qmi_request ) );
  memset( &qmi_response, 0, sizeof(qmi_response) );
  qmi_request.voice_privacy_valid= TRUE;
  qmi_request.voice_privacy= 0x01; // as per QMI Voice - 0x01

  ril_req_res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_VOICE,
                                                     QMI_VOICE_GET_CONFIG_REQ_V02,
                                                     (void*)&qmi_request,
                                                     sizeof( qmi_request ),
                                                     (void*) &qmi_response,
                                                     sizeof( qmi_response ),
                                                     QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                      );

  if( ril_req_res == RIL_E_SUCCESS && qmi_response.current_voice_privacy_pref_valid )
  {
    privacy_preference = qmi_response.current_voice_privacy_pref;
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
    resp.resp_pkt = &privacy_preference;
    resp.resp_len = sizeof(privacy_preference);
    QCRIL_LOG_DEBUG("privacy_preferece = %d", privacy_preference);
  }
  else
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
  }

  qcril_send_request_response( &resp );
} /* qcril_qmi_voice_request_query_preferred_privacy_mode() */

static void qcril_qmi_voice_set_management_call_concurrency_requirement(qcril_reqlist_generic_concurency_requests_requirement_type *req_ptr)
{
  static uint32 req_ids[] = {
    RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND,
    RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND,
    RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE,
    RIL_REQUEST_CONFERENCE,
    RIL_REQUEST_EXPLICIT_CALL_TRANSFER,
    RIL_REQUEST_SEPARATE_CONNECTION,
    RIL_REQUEST_UDUB
  };
  req_ptr->max_concurrency = 1;
  req_ptr->max_pending = 1024;
  req_ptr->req_ids_num = sizeof(req_ids)/sizeof(req_ids[0]);
  req_ptr->req_ids = req_ids;
}

static void qcril_qmi_voice_send_management_call_request
(
  qcril_instance_id_e_type instance_id,
  struct qcril_reqlist_buf_tag *req,
  void *data,
  size_t datalen
)
{
  uint32 user_data;
  voice_manage_calls_req_msg_v02* mng_call_req_msg_ptr = data;
  voice_manage_calls_resp_msg_v02* mng_call_resp_msg_ptr = NULL;
  qcril_request_resp_params_type resp;
  int error = 0;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED(datalen);

  do
  {
    if( NULL == mng_call_req_msg_ptr )
    {
      QCRIL_LOG_DEBUG("mng_call_req_msg_ptr is null");
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }
    mng_call_resp_msg_ptr = qcril_malloc( sizeof(*mng_call_resp_msg_ptr) );
    if( NULL == mng_call_resp_msg_ptr )
    {
      QCRIL_LOG_DEBUG("mng_call_resp_msg_ptr is null");
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, req->pub.req_id );

    /* Send QMI VOICE MANAGE CALLS REQ */
    error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_MANAGE_CALLS_REQ_V02,
                                      mng_call_req_msg_ptr,
                                      sizeof(*mng_call_req_msg_ptr),
                                      mng_call_resp_msg_ptr,
                                      sizeof(*mng_call_resp_msg_ptr),
                                      (void*)(uintptr_t)user_data);
    QCRIL_LOG_DEBUG("send_msg_async completed with error code: %d", error);
    if ( E_SUCCESS != error )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
         delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( mng_call_resp_msg_ptr );
      qcril_send_empty_payload_request_response( instance_id, req->pub.t, req->pub.request, RIL_E_GENERIC_FAILURE );
    }
    else
    {
      // update call obj state
      if ( SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02 == mng_call_req_msg_ptr->sups_type )
      {
        qcril_qmi_voice_voip_unmark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
      }
      else if ( SUPS_TYPE_RELEASE_ACTIVE_ACCEPT_HELD_OR_WAITING_V02 == mng_call_req_msg_ptr->sups_type )
      {
        qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_CONVERSATION_V02);
        qcril_qmi_voice_voip_unmark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
      }
      else if ( SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02 == mng_call_req_msg_ptr->sups_type )
      {
        qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_WAITING_V02);
        qcril_qmi_voice_voip_unmark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_WAITING_V02);
        qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_INCOMING_V02);
        qcril_qmi_voice_voip_unmark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, CALL_STATE_INCOMING_V02);
        qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_BY_USER, CALL_STATE_HOLD_V02);
      }
    }
  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND
//===========================================================================
void qcril_qmi_voice_request_manage_calls_hangup_waiting_or_background
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;
  voice_manage_calls_resp_msg_v02* hangup_waiting_or_backgroud_resp_msg_ptr = NULL;
  uint32 user_data;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  voice_get_all_call_info_resp_msg_v02 all_call_info_resp;
  voice_end_call_req_msg_v02           call_end_req_msg;
  voice_end_call_resp_msg_v02* end_call_resp_msg_ptr = NULL;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_iter = NULL;

  qcril_qmi_voice_voip_current_call_summary_type calls_summary;

  qmi_ril_voice_ims_command_exec_oversight_type*                      command_oversight;
  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type   oversight_cmd_params;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  if ( qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate() )
  {
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
  }
  else
  {

    qcril_qmi_voice_voip_lock_overview();
    do
    {
      // find call info
      qcril_qmi_voice_voip_generate_summary( &calls_summary );
      call_info_entry = calls_summary.active_or_single_call;
      if ( NULL == call_info_entry )
      {
        QCRIL_LOG_ERROR( ".. call info not found" );
        break;
      }

      if ((TRUE == reject_cause_21_supported) &&
          (qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE )))
      { // DOCOMO
         ril_err = qcril_qmi_voice_send_request_answer(
                                        params_ptr,
                                        TRUE);
      }
      else if ( QMI_RIL_ZERO == calls_summary.nof_voip_calls && calls_summary.nof_3gpp2_calls )
      { // CDMA forever

        // More than one CDMA calls - possibly Dialing and Incoming states
        // Need to disconnect Incoming call
        if (calls_summary.nof_3gpp2_calls > 1)
        {
          call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
                                QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE );
          if( NULL == call_info_entry )
          {
            QCRIL_LOG_ERROR( ".. 3gpp2 incoming call not found" );
            break;
          }
        }

        memset( &call_end_req_msg, 0, sizeof(call_end_req_msg) );
        end_call_resp_msg_ptr = qcril_malloc( sizeof(*end_call_resp_msg_ptr) );
        if( NULL == end_call_resp_msg_ptr )
        {
            break;
        }

        // add entry to ReqList
        qcril_reqlist_default_entry( params_ptr->t,
                                     params_ptr->event_id,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_REQ_AWAITING_CALLBACK,
                                     QCRIL_EVT_NONE,
                                     NULL,
                                     &reqlist_entry );
        if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
        {
            // Fail to add entry to ReqList
            QCRIL_LOG_ERROR(".. failed to Add into Req list");
            break;
        }

        // preparation
        user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

        call_end_req_msg.call_id = call_info_entry->qmi_call_id;
        ril_err = qcril_qmi_client_send_msg_async_ex ( QCRIL_QMI_CLIENT_VOICE,
                                                    QMI_VOICE_END_CALL_REQ_V02,
                                                    &call_end_req_msg,
                                                    sizeof(call_end_req_msg),
                                                    end_call_resp_msg_ptr,
                                                    sizeof(*end_call_resp_msg_ptr),
                                                    (void*)(uintptr_t)user_data  );
        if ( RIL_E_SUCCESS != ril_err )
        {
          break;
        }
      }
      else
      { // 3gpp or voip
        // for 3gpp we do not need call ID as QMI Voice does not requitre that for this particular operation
        // would be voip if we see any VOIP calls in held or waiting
        call_info_entry = NULL;
        call_info_iter = qcril_qmi_voice_voip_call_info_entries_enum_first();
        while ( NULL != call_info_iter && NULL == call_info_entry )
        {
          if ( VOICE_INVALID_CALL_ID != call_info_iter->android_call_id )
          { // skip shadow calls from CM space
            if ( ( CALL_STATE_HOLD_V02 == call_info_iter->voice_scv_info.call_state || CALL_STATE_WAITING_V02 == call_info_iter->voice_scv_info.call_state || CALL_STATE_INCOMING_V02 == call_info_iter->voice_scv_info.call_state ) )
            {
              call_info_entry = call_info_iter;
            }
            if (
                 ( call_info_iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN ) &&
                 ( CALL_STATE_HOLD_V02 == call_info_iter->voice_scv_info.call_state || CALL_STATE_WAITING_V02 == call_info_iter->voice_scv_info.call_state )
               )
            {
              call_info_entry = call_info_iter;
            }
          }
          call_info_iter = qcril_qmi_voice_voip_call_info_entries_enum_next();
        }
        if ( NULL != call_info_entry )
        {
          if ( ( call_info_entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN ) )
          { // voip
            // add entry to ReqList
            qcril_reqlist_default_entry( params_ptr->t,
                                         params_ptr->event_id,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         QCRIL_REQ_AWAITING_CALLBACK,
                                         QCRIL_EVT_NONE,
                                         NULL,
                                         &reqlist_entry );
            if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
            {
                // Fail to add entry to ReqList
                QCRIL_LOG_ERROR(".. failed to Add into Req list");
                break;
            }

            ril_err = qcril_qmi_voice_post_manage_voip_calls_request( VOIP_SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02, reqlist_entry.req_id  );
          }
          else
          { // 3gpp
            memset(&manage_calls_req, 0, sizeof(manage_calls_req));
            manage_calls_req.sups_type = SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02;
            manage_calls_req.call_id_valid = FALSE;

            qcril_reqlist_default_entry( params_ptr->t,
                                         params_ptr->event_id,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         QCRIL_REQ_AWAITING_CALLBACK,
                                         QCRIL_EVT_NONE, NULL, &reqlist_entry );
            qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
            qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
            qcril_reqlist_buf_type *req_buf;

            IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                                    &reqlist_entry,
                                                                                    &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                                    &concurency_requirement,
                                                                                    sizeof(concurency_requirement),
                                                                                    &qcril_qmi_voice_send_management_call_request,
                                                                                    &manage_calls_req,
                                                                                    sizeof(manage_calls_req),
                                                                                    &req_buf );
            if ( E_SUCCESS == new_req_result )
            {

              if ( RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND == params_ptr->event_id )
              {
                command_oversight = qmi_ril_voice_ims_create_command_oversight( params_ptr->t, RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND, TRUE );
                if ( NULL != command_oversight )
                {
                  qmi_ril_voice_ims_command_oversight_add_call_link( command_oversight,
                                                                     QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID,
                                                                     QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NONE,
                                                                     call_info_entry->qmi_call_id,
                                                                     CALL_STATE_END_V02 );

                  memset( &oversight_cmd_params, 0, sizeof( oversight_cmd_params ) );
                  oversight_cmd_params.locator.command_oversight = command_oversight;

                  qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_COMMENCE_AWAIT_RESP_IND,
                                                                    QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                                                    &oversight_cmd_params );
                } // if ( NULL != command_oversight )

              } // if ( RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND == params_ptr->event_id )

              qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
              ril_err = E_SUCCESS;
            }
            else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
            {
              QCRIL_LOG_INFO("the new request is blocked");
              ril_err = E_SUCCESS;
            }
            else
            {
              QCRIL_LOG_INFO("the new request is rejected");
            }
          }
        }
        else // nothing to end
          break;
      }
    } while ( FALSE );

    qcril_qmi_voice_voip_unlock_overview();

    // respond
    if ( RIL_E_SUCCESS != ril_err )
    {
        qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err );

        if ( end_call_resp_msg_ptr )
        {
          qcril_free( end_call_resp_msg_ptr );
        }
    }

  }

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_manage_calls_hangup_waiting_or_background

//===========================================================================
//RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND
//===========================================================================
void qcril_qmi_voice_request_manage_calls_hangup_foreground_resume_background
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;

  voice_get_all_call_info_resp_msg_v02 all_call_info_resp;
  voice_end_call_req_msg_v02           call_end_req_msg;
  voice_end_call_resp_msg_v02* end_call_resp_msg_ptr = NULL;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  qcril_qmi_voice_voip_current_call_summary_type calls_summary;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  if ( qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate() )
  {
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
  }
  else
  {

    qcril_qmi_voice_voip_lock_overview();

    do
    {
      // find call info
      qcril_qmi_voice_voip_generate_summary( &calls_summary );
      call_info_entry = calls_summary.active_or_single_call;
      if ( NULL == call_info_entry )
      {
        QCRIL_LOG_ERROR( ".. call info not found" );
        break;
      }

     //TODO: mpty voip list replaced with is_mpty in summary also need to add is_mpty check here
      if ( 1 == calls_summary.nof_calls_overall )
      { // one call
        memset( &call_end_req_msg, 0, sizeof(call_end_req_msg) );
        end_call_resp_msg_ptr = qcril_malloc( sizeof(*end_call_resp_msg_ptr) );
        if( NULL == end_call_resp_msg_ptr )
        {
          ril_err = RIL_E_GENERIC_FAILURE;
          break;
        }

        // add entry to ReqList
        qcril_reqlist_default_entry( params_ptr->t,
                                     params_ptr->event_id,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_REQ_AWAITING_CALLBACK,
                                     QCRIL_EVT_NONE,
                                     NULL,
                                     &reqlist_entry );
        if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
        {
            // Fail to add entry to ReqList
            QCRIL_LOG_ERROR(".. failed to Add into Req list");
            ril_err = RIL_E_GENERIC_FAILURE;
            break;
        }

        // preparation
        user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

        call_end_req_msg.call_id = call_info_entry->qmi_call_id;
        ril_err = qcril_qmi_client_send_msg_async_ex ( QCRIL_QMI_CLIENT_VOICE,
                                                    QMI_VOICE_END_CALL_REQ_V02,
                                                    &call_end_req_msg,
                                                    sizeof(call_end_req_msg),
                                                    end_call_resp_msg_ptr,
                                                    sizeof(*end_call_resp_msg_ptr),
                                                    (void*)(uintptr_t)user_data  );
        if ( RIL_E_SUCCESS != ril_err )
          break;
      }
      else
      { // multiple
        if ( calls_summary.nof_voip_calls > QMI_RIL_ZERO )
        { // voip
          // add entry to ReqList
          qcril_reqlist_default_entry( params_ptr->t,
                                       params_ptr->event_id,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       QCRIL_REQ_AWAITING_CALLBACK,
                                       QCRIL_EVT_NONE,
                                       NULL,
                                       &reqlist_entry );
          if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
          {
              // Fail to add entry to ReqList
              QCRIL_LOG_ERROR(".. failed to Add into Req list");
              ril_err = RIL_E_GENERIC_FAILURE;
              break;
          }
          ril_err = qcril_qmi_voice_post_manage_voip_calls_request( VOIP_SUPS_TYPE_RELEASE_ACTIVE_ACCEPT_HELD_OR_WAITING_V02, reqlist_entry.req_id  );
        }
        else
        { // voice
          memset(&manage_calls_req, 0, sizeof(manage_calls_req));
          manage_calls_req.sups_type = SUPS_TYPE_RELEASE_ACTIVE_ACCEPT_HELD_OR_WAITING_V02;
          manage_calls_req.call_id_valid = FALSE;

          qcril_reqlist_default_entry( params_ptr->t,
                                       params_ptr->event_id,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       QCRIL_REQ_AWAITING_CALLBACK,
                                       QCRIL_EVT_NONE, NULL, &reqlist_entry );
          qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
          qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
          qcril_reqlist_buf_type *req_buf;

          IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                                  &reqlist_entry,
                                                                                  &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                                  &concurency_requirement,
                                                                                  sizeof(concurency_requirement),
                                                                                  &qcril_qmi_voice_send_management_call_request,
                                                                                  &manage_calls_req,
                                                                                  sizeof(manage_calls_req),
                                                                                  &req_buf );
          if ( E_SUCCESS == new_req_result )
          {
            qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
            ril_err = E_SUCCESS;
          }
          else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
          {
            QCRIL_LOG_INFO("the new request is blocked");
            ril_err = E_SUCCESS;
          }
          else
          {
            QCRIL_LOG_INFO("the new request is rejected");
          }
        }
      }
    } while ( FALSE );

    qcril_qmi_voice_voip_unlock_overview();

    // respond
    if ( RIL_E_SUCCESS != ril_err )
    {
        qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_err );

        if ( end_call_resp_msg_ptr )
        {
          qcril_free( end_call_resp_msg_ptr );
        }
    }
    else
    {
      qcril_qmi_voice_voip_mark_with_specified_call_state(
         QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE,
         CALL_STATE_WAITING_V02 );
      qcril_qmi_voice_voip_mark_with_specified_call_state(
         QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE,
         CALL_STATE_HOLD_V02 );
#ifndef QMI_RIL_UTF
      qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL, NULL);
#endif
    }
  }

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_manage_calls_hangup_foreground_resume_background


//=========================================================================
//qcril_qmi_voice_post_manage_voip_calls_request
//===========================================================================
RIL_Errno qcril_qmi_voice_post_manage_voip_calls_request( voip_sups_type_enum_v02 request, uint16 req_id )
{
    RIL_Errno                             res = RIL_E_GENERIC_FAILURE;
    voice_manage_ip_calls_req_msg_v02     manage_voip_calls_req;
    voice_manage_ip_calls_resp_msg_v02 *  manage_voip_calls_resp;
    qmi_client_error_type                 qmi_client_error = QMI_NO_ERR;
    uint32                                user_data;

    manage_voip_calls_resp = qcril_malloc( sizeof( *manage_voip_calls_resp ) );
    if ( NULL != manage_voip_calls_resp )
    {
      user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, req_id );

      memset( &manage_voip_calls_req, 0, sizeof( manage_voip_calls_req ));
      manage_voip_calls_req.sups_type = request;

      qmi_client_error =  qcril_qmi_client_send_msg_async( QCRIL_QMI_CLIENT_VOICE,
                                                         QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                         (void*) &manage_voip_calls_req,
                                                         sizeof( manage_voip_calls_req ),
                                                         (void*) manage_voip_calls_resp,
                                                         sizeof( *manage_voip_calls_resp ),
                                                         (void*)(uintptr_t)user_data );
      res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, NULL );

      if ( RIL_E_SUCCESS != res )
      {
        qcril_free( manage_voip_calls_resp );
      }
    }

    QCRIL_LOG_INFO("posted req %d, %d", (int) qmi_client_error, (int) res );

    return res;
} // qcril_qmi_voice_post_manage_voip_calls_request

//===========================================================================
// qcril_qmi_voice_voip_manage_ip_calls_resp_hdlr
//===========================================================================
void qcril_qmi_voice_voip_manage_ip_calls_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
    voice_manage_ip_calls_resp_msg_v02        *qmi_response = NULL;
    RIL_Errno                                  ril_req_res = RIL_E_GENERIC_FAILURE;
    qcril_qmi_voice_ims_conf_req_state         voip_conf_req_call_state;
    qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = NULL;

    if (params_ptr->data != NULL)
    {
        qmi_response = (voice_manage_ip_calls_resp_msg_v02 *) params_ptr->data;

        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR,
                       &qmi_response->resp);

        QCRIL_LOG_INFO(".. ril res %d, qmi res %d",
                       (int) ril_req_res, (int)qmi_response->resp.error);

        if (RIL_REQUEST_CONFERENCE == params_ptr->event_id ||
            QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE == params_ptr->event_id)
        {
           if (RIL_E_SUCCESS == ril_req_res)
           {
#ifndef QMI_RIL_UTF
             if (QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE == params_ptr->event_id)
             {
               qcril_qmi_voice_send_ims_unsol_call_state_changed();
             }
             else
             {
               qcril_qmi_voice_send_unsol_call_state_changed(QCRIL_DEFAULT_INSTANCE_ID);
             }
#endif
          }

          qcril_qmi_voice_voip_lock_overview();

          voip_conf_req_call_state = qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl();
          QCRIL_LOG_INFO("conf_call_txn_call_state: %d", voip_conf_req_call_state);
          qcril_qmi_voice_ims_conf_req_state_reset_vcl();

          qcril_qmi_voice_add_call_to_existing_mpty_voip_call_failure_cleanup_vcl();

          qcril_qmi_voice_voip_unlock_overview();
        }

        if ((RIL_REQUEST_MODIFY_CALL_INITIATE == params_ptr->event_id ||
              RIL_REQUEST_MODIFY_CALL_CONFIRM == params_ptr->event_id ||
              QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE == params_ptr->event_id ||
              QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM == params_ptr->event_id)
            && (ril_req_res == RIL_E_SUCCESS))
        {
           /* wait for modify indication, update state to wait for more events */
           qcril_reqlist_update_state( QCRIL_DEFAULT_INSTANCE_ID,
                                       QCRIL_DEFAULT_MODEM_ID,
                                       params_ptr->t,
                                       QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS );
        }
        else if((QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE == params_ptr->event_id)
                 && (qmi_response->failure_cause_valid == TRUE)
                 && (qmi_response->failure_cause == QMI_FAILURE_CAUSE_UPGRADE_DOWNGRADE_REJ_V02))
        {
              qcril_qmi_ims_socket_send(params_ptr->t,
                                        IMS__MSG_TYPE__RESPONSE,
                                        qcril_qmi_ims_map_event_to_request(params_ptr->event_id),
                                        IMS__ERROR__E_REJECTED_BY_REMOTE,
                                        NULL,
                                        0);
        }
        else
        {
           if ((QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE ==
                 params_ptr->event_id) &&
               QMI_ERR_OP_NETWORK_UNSUPPORTED_V01 == qmi_response->resp.error)
           {
              qcril_qmi_ims_socket_send(params_ptr->t,
                                        IMS__MSG_TYPE__RESPONSE,
                                        qcril_qmi_ims_map_event_to_request(params_ptr->event_id),
                                        IMS__ERROR__E_IMS_DEREGISTERED,
                                        NULL,
                                        0);
           }
           else if (QCRIL_EVT_IMS_SOCKET_REQ_HANGUP == params_ptr->event_id)
           {
               //TODO: Remove all this when the mpty voip structure is removed.
              call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(
                  QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONF_PATICIAPNT_CALL_END_REPORTED, TRUE);
              if( QMI_ERR_NONE_V01 == qmi_response->resp.error )
              {
                 if( NULL != call_info_entry && call_info_entry->voice_scv_info.is_mpty != 1 )
                 {
                    qcril_qmi_voice_voip_destroy_mpty_call_info_entry( call_info_entry );
                    call_info_entry = NULL;
                 }
              }
              if( NULL != call_info_entry )
              {
                call_info_entry->elaboration &=
                  ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONF_PATICIAPNT_CALL_END_REPORTED;
              }
              qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                                                        params_ptr->t,
                                                        params_ptr->event_id,
                                                        ril_req_res );
           }
           else
           {
              qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                                                        params_ptr->t,
                                                        params_ptr->event_id,
                                                        ril_req_res );
           }
        }

        if (QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND == params_ptr->event_id ||
            QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == params_ptr->event_id ||
            RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND == params_ptr->event_id ||
            RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == params_ptr->event_id ||
            QCRIL_EVT_IMS_SOCKET_REQ_RESUME == params_ptr->event_id )
        {
            qcril_qmi_voice_voip_unmark_all_with(QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE);
            if (RIL_E_SUCCESS != ril_req_res)
            {
#ifndef QMI_RIL_UTF
                qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL_FAIL, NULL);
#endif
            }
        }
    }
    else
    {
        qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
                                                  params_ptr->t,
                                                  params_ptr->event_id,
                                                  RIL_E_GENERIC_FAILURE );
    }


    QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_voip_manage_ip_calls_resp_hdlr


//=========================================================================
//RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE
//===========================================================================
void qcril_qmi_voice_request_manage_calls_switch_waiting_or_holding_and_active
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_qmi_voice_voip_current_call_summary_type call_summary;
  Ims__SwitchWaitingOrHoldingAndActive *ims_in_data_ptr = NULL;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();

  do
  {
    qcril_qmi_voice_voip_generate_summary( &call_summary );

    QCRIL_LOG_ERROR(".. nof voice %d, nof voip %d", call_summary.nof_voice_calls, call_summary.nof_voip_calls );
    if ( QMI_RIL_ZERO == call_summary.nof_voip_calls + call_summary.nof_voice_calls )
    {
      break;
    }

    if ( call_summary.nof_voip_calls > QMI_RIL_ZERO )
    { // voip
      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE,
                                   NULL,
                                   &reqlist_entry );
      if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
      {
          // Fail to add entry to ReqList
          QCRIL_LOG_ERROR(".. failed to Add into Req list");
          break;
      }

      Ims__CallType call_type;
      qcril_qmi_voice_voip_call_info_entry_type *call_info = NULL;

      if ( QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == params_ptr->event_id )
      {
         ims_in_data_ptr = (Ims__SwitchWaitingOrHoldingAndActive *)params_ptr->data;
         if (ims_in_data_ptr)
         {
            if (ims_in_data_ptr->has_call_type)
            {
               call_type = ims_in_data_ptr->call_type;
               call_info = qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state(RIL_CALL_WAITING);
               if ( !call_info )
               {
                  QCRIL_LOG_DEBUG(".. do not have waiting call while the call type is specified in IMS request");
               }
            }
            qcril_qmi_ims__switch_waiting_or_holding_and_active__free_unpacked(ims_in_data_ptr, NULL);
         }
         else
         {
            QCRIL_LOG_DEBUG("ims_in_data_ptr is NULL");
         }
      }

      if ( call_info )
      {
         voice_manage_ip_calls_req_msg_v02   manage_voip_calls_req;
         voice_manage_ip_calls_resp_msg_v02 *manage_voip_calls_resp;

         manage_voip_calls_resp = qcril_malloc( sizeof( *manage_voip_calls_resp ) );
         if ( NULL != manage_voip_calls_resp )
         {
            qmi_client_error_type qmi_client_error = QMI_NO_ERR;
            uint32                user_data;

            // Cache the user specified call type in answer.
            call_info->answered_call_type_valid = TRUE;
            call_info->answered_call_type = call_type;

            user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

            memset( &manage_voip_calls_req, 0, sizeof(manage_voip_calls_req) );
            manage_voip_calls_req.sups_type = VOIP_SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02;
            manage_voip_calls_req.call_id_valid = TRUE;
            manage_voip_calls_req.call_id = call_info->voice_scv_info.call_id;

            if ( IMS__CALL_TYPE__CALL_TYPE_VOICE == call_type )
            {
               manage_voip_calls_req.call_type_valid = TRUE;
               manage_voip_calls_req.call_type = CALL_TYPE_VOICE_IP_V02;
            }
            else if ( IMS__CALL_TYPE__CALL_TYPE_VT == call_type )
            {
               manage_voip_calls_req.call_type_valid = TRUE;
               manage_voip_calls_req.call_type = CALL_TYPE_VT_V02;
               manage_voip_calls_req.video_attrib_valid = TRUE;
               manage_voip_calls_req.video_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
            }
            else if ( IMS__CALL_TYPE__CALL_TYPE_VT_TX == call_type )
            {
               manage_voip_calls_req.call_type_valid = TRUE;
               manage_voip_calls_req.call_type = CALL_TYPE_VT_V02;
               manage_voip_calls_req.video_attrib_valid = TRUE;
               manage_voip_calls_req.video_attrib = VOICE_CALL_ATTRIB_TX_V02;
            }
            else if ( IMS__CALL_TYPE__CALL_TYPE_VT_RX == call_type )
            {
               manage_voip_calls_req.call_type_valid = TRUE;
               manage_voip_calls_req.call_type = CALL_TYPE_VT_V02;
               manage_voip_calls_req.video_attrib_valid = TRUE;
               manage_voip_calls_req.video_attrib = VOICE_CALL_ATTRIB_RX_V02;
            }
            else
            {
               QCRIL_LOG_DEBUG("Received unexpected call type. Ignore the type specified.");
               manage_voip_calls_req.call_id_valid = FALSE;
               call_info->answered_call_type_valid = FALSE;
            }

            qmi_client_error =  qcril_qmi_client_send_msg_async( QCRIL_QMI_CLIENT_VOICE,
                                                           QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                           (void*) &manage_voip_calls_req,
                                                           sizeof( manage_voip_calls_req ),
                                                           (void*) manage_voip_calls_resp,
                                                           sizeof( *manage_voip_calls_resp ),
                                                           (void*)(uintptr_t)user_data );

            ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, NULL );
            if ( RIL_E_SUCCESS != ril_req_res )
            {
               qcril_free( manage_voip_calls_resp );
            }
         }
         else
         {
            QCRIL_LOG_ERROR("malloc manage_voip_calls_resp failed.");
         }
      }
      else
      {
         ril_req_res = qcril_qmi_voice_post_manage_voip_calls_request( VOIP_SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02 , reqlist_entry.req_id );
      }
    }
    else
    { // voice
      memset(&manage_calls_req, 0, sizeof(manage_calls_req));
      manage_calls_req.sups_type = SUPS_TYPE_HOLD_ACTIVE_ACCEPT_WAITING_OR_HELD_V02;
      manage_calls_req.call_id_valid = FALSE;

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
      qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
      qcril_reqlist_buf_type *req_buf;

      IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                              &reqlist_entry,
                                                                              &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                              &concurency_requirement,
                                                                              sizeof(concurency_requirement),
                                                                              &qcril_qmi_voice_send_management_call_request,
                                                                              &manage_calls_req,
                                                                              sizeof(manage_calls_req),
                                                                              &req_buf );
      if ( E_SUCCESS == new_req_result )
      {
        qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
        ril_req_res = E_SUCCESS;
      }
      else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
      {
        QCRIL_LOG_INFO("the new request is blocked");
        ril_req_res = E_SUCCESS;
      }
      else
      {
        QCRIL_LOG_INFO("the new request is rejected");
      }
    }

  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // failure
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res );
  }
  else
  {
    qcril_qmi_voice_voip_mark_with_specified_call_state(
       QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE,
       CALL_STATE_WAITING_V02 );
    qcril_qmi_voice_voip_mark_with_specified_call_state(
       QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE,
       CALL_STATE_HOLD_V02 );
#ifndef QMI_RIL_UTF
    qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL, NULL);
#endif
  }

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_manage_calls_switch_waiting_or_holding_and_active

/*===========================================================================
  FUNCTION:  qcril_qmi_voice_request_manage_calls_hold_resume
===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_HOLD and
            QCRIL_EVT_IMS_SOCKET_REQ_RESUME.

    @return:
    None
*/
/*=========================================================================*/
void qcril_qmi_voice_request_manage_calls_hold_resume
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_evt_e_type event = params_ptr->event_id;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_qmi_voice_voip_current_call_summary_type call_summary;
  Ims__Hold *ims_in_data_ptr_hold = NULL;
  Ims__Resume *ims_in_data_ptr_resume = NULL;
  voip_sups_type_enum_v02 supsType;
  uint8_t call_id;
  qcril_qmi_voice_voip_call_info_entry_type *call_info = NULL;
  Ims__CallType call_type;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();

  do
  {
    qcril_qmi_voice_voip_generate_summary( &call_summary );

    QCRIL_LOG_ESSENTIAL("Number of voip calls : %d", call_summary.nof_voip_calls );

    if ( QMI_RIL_ZERO == call_summary.nof_voip_calls )
    {
      break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
    {
        // Fail to add entry to ReqList
        QCRIL_LOG_ERROR(".. failed to Add into Req list");
        break;
    }

    if ( QCRIL_EVT_IMS_SOCKET_REQ_HOLD == event )
    {
      ims_in_data_ptr_hold = (Ims__Hold *)params_ptr->data;
      supsType = VOIP_SUPS_TYPE_CALL_HOLD_V02;

      if (ims_in_data_ptr_hold)
      {
        QCRIL_LOG_DEBUG("Has callid? %d", ims_in_data_ptr_hold->has_callid);
        if (TRUE == ims_in_data_ptr_hold->has_callid)
        {
          call_id = ims_in_data_ptr_hold->callid;
          call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(call_id);
        }
        else
        {
          //Call id is mandatory
          QCRIL_LOG_ERROR("Invalid call id");
        }
        if ( !call_info )
        {
          QCRIL_LOG_ERROR("Do not have any voip active calls or invalid call id in request");
        }
        qcril_qmi_ims__hold__free_unpacked(ims_in_data_ptr_hold, NULL);
      }
      else
      {
        QCRIL_LOG_ERROR("Hold request data is NULL");
      }
    }

    if ( QCRIL_EVT_IMS_SOCKET_REQ_RESUME == event )
    {
      ims_in_data_ptr_resume = (Ims__Resume *)params_ptr->data;
      supsType = VOIP_SUPS_TYPE_CALL_RESUME_V02;

      if (ims_in_data_ptr_resume)
      {
        if (TRUE == ims_in_data_ptr_resume->has_callid)
        {
          call_id = ims_in_data_ptr_resume->callid;
          call_info = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(call_id);
        }
        else
        {
          QCRIL_LOG_ERROR("Invalid call id");
        }

        if ( !call_info )
        {
          QCRIL_LOG_ERROR("Do not have any voip held calls or invalid call id in request");
        }

        qcril_qmi_ims__resume__free_unpacked(ims_in_data_ptr_resume, NULL);
      }
      else
      {
        QCRIL_LOG_ERROR("Resume request data is NULL");
      }
    }

    if ( call_info )
    {
      voice_manage_ip_calls_req_msg_v02   manage_voip_calls_req;
      voice_manage_ip_calls_resp_msg_v02 *manage_voip_calls_resp;

      manage_voip_calls_resp = qcril_malloc( sizeof( *manage_voip_calls_resp ) );
      if ( NULL != manage_voip_calls_resp )
      {
        qmi_client_error_type qmi_client_error = QMI_NO_ERR;
        uint32                user_data;

        user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID,
                                             QCRIL_DEFAULT_MODEM_ID,
                                             reqlist_entry.req_id );

        memset( &manage_voip_calls_req, 0, sizeof(manage_voip_calls_req) );
        manage_voip_calls_req.sups_type = supsType;
        manage_voip_calls_req.call_id_valid = TRUE;
        manage_voip_calls_req.call_id = call_info->voice_scv_info.call_id;

        qmi_client_error =  qcril_qmi_client_send_msg_async( QCRIL_QMI_CLIENT_VOICE,
                                                             QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                             &manage_voip_calls_req,
                                                             sizeof( manage_voip_calls_req ),
                                                             manage_voip_calls_resp,
                                                             sizeof( *manage_voip_calls_resp ),
                                                             (void*)(uintptr_t)user_data );

        ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error,
                                                                               NULL );
        if ( RIL_E_SUCCESS != ril_req_res )
        {
          qcril_free( manage_voip_calls_resp );
        }
      }
      else
      {
        QCRIL_LOG_ERROR("malloc manage_voip_calls_resp failed.");
      }
    }
  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // Failure
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
                                               params_ptr->event_id, ril_req_res );
  }
  else
  {
    if( QCRIL_EVT_IMS_SOCKET_REQ_RESUME == event )
    {
      call_info->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SWITCHING_CALL_TO_ACTIVE;

#ifndef QMI_RIL_UTF
      qcril_am_handle_event(QCRIL_AM_EVENT_SWITCH_CALL, NULL);
#endif
    }
  }

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_manage_calls_request_hold_resume

//===========================================================================
// qcril_qmi_voice_request_add_participant
//===========================================================================
void qcril_qmi_voice_request_add_participant
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_qmi_voice_voip_current_call_summary_type call_summary;
  Ims__Dial *ims_in_data_ptr = (Ims__Dial *)params_ptr->data;
  uint8 call_id = VOICE_INVALID_CALL_ID;
  qcril_qmi_voice_voip_current_call_summary_type calls_summary;
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = NULL;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (!ims_in_data_ptr || strlen(ims_in_data_ptr->address) > QMI_VOICE_SIP_URI_MAX_V02)
    {
      QCRIL_LOG_ERROR("invalid parameter");
      break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
    {
      // Fail to add entry to ReqList
      QCRIL_LOG_ERROR(".. failed to Add into Req list");
      break;
    }

    qcril_qmi_voice_voip_generate_summary(&calls_summary);
    QCRIL_LOG_DEBUG("number of calls = %d", calls_summary.nof_calls_overall);

    if (calls_summary.nof_calls_overall == 1)
    {
      call_info_entry = calls_summary.active_or_single_call;
    }
    else
    {
      // Get the call in conversation state if more than one call
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state(
                                CALL_STATE_CONVERSATION_V02);
    }

    QCRIL_LOG_INFO(".. call_info_entry %p", call_info_entry);

    if (!call_info_entry ||
            (VOICE_INVALID_CALL_ID == call_info_entry->qmi_call_id) ||
            !(CALL_STATE_CONVERSATION_V02 == call_info_entry->voice_scv_info.call_state ||
                CALL_STATE_HOLD_V02 == call_info_entry->voice_scv_info.call_state) ||
            !(CALL_TYPE_VOICE_IP_V02 == call_info_entry->voice_scv_info.call_type ||
                CALL_TYPE_VT_V02 == call_info_entry->voice_scv_info.call_type))
    {
      QCRIL_LOG_ERROR("No valid call info entry");
      qcril_qmi_voice_voip_call_info_dump(call_info_entry);
      break;
    }

    call_id = call_info_entry->voice_scv_info.call_id;

    voice_manage_ip_calls_req_msg_v02   manage_voip_calls_req;
    voice_manage_ip_calls_resp_msg_v02 *manage_voip_calls_resp = qcril_malloc( sizeof( *manage_voip_calls_resp ) );
    if ( NULL != manage_voip_calls_resp )
    {
      qmi_client_error_type qmi_client_error = QMI_NO_ERR;
      uint32                user_data;

      user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

      memset( &manage_voip_calls_req, 0, sizeof(manage_voip_calls_req) );
      manage_voip_calls_req.sups_type = 0x0A; // VOIP_SUPS_TYPE_ADD_PARTICIPANT_V02
      manage_voip_calls_req.call_id_valid = TRUE;
      manage_voip_calls_req.call_id = call_id;
      manage_voip_calls_req.sip_uri_valid = TRUE;
      memcpy(&manage_voip_calls_req.sip_uri, ims_in_data_ptr->address, strlen(ims_in_data_ptr->address));

      qmi_client_error =  qcril_qmi_client_send_msg_async( QCRIL_QMI_CLIENT_VOICE,
                                                     QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                     (void*) &manage_voip_calls_req,
                                                     sizeof( manage_voip_calls_req ),
                                                     (void*) manage_voip_calls_resp,
                                                     sizeof( *manage_voip_calls_resp ),
                                                     (void*)(uintptr_t)user_data );

      ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( qmi_client_error, NULL );
      if ( RIL_E_SUCCESS != ril_req_res )
      {
         qcril_free( manage_voip_calls_resp );
      }
    }
    else
    {
      QCRIL_LOG_ERROR("malloc manage_voip_calls_resp failed.");
    }
  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // failure
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res );
  }

  if (ims_in_data_ptr)
  {
    qcril_qmi_ims__dial__free_unpacked(ims_in_data_ptr, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_add_participant

//=========================================================================
//RIL_REQUEST_CONFERENCE
//===========================================================================
void qcril_qmi_voice_request_manage_calls_conference
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;
  voice_manage_calls_req_msg_v02  manage_calls_req;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_qmi_voice_voip_current_call_summary_type call_summary;
  qcril_qmi_voice_voip_call_info_entry_type* call_entry = NULL;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_voice_voip_lock_overview();

  do
  {
    qcril_qmi_voice_voip_generate_summary( &call_summary );

    QCRIL_LOG_ERROR(".. nof voice %d, nof voip %d", call_summary.nof_voice_calls, call_summary.nof_voip_calls );
    if ( QMI_RIL_ZERO == call_summary.nof_voip_calls + call_summary.nof_voice_calls )
    {
      break;
    }

    if ( call_summary.nof_voip_calls > QMI_RIL_ZERO )
    { // voip
      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE,
                                   NULL,
                                   &reqlist_entry );
      if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
      {
          // Fail to add entry to ReqList
          QCRIL_LOG_ERROR(".. failed to Add into Req list");
          break;
      }
      ril_req_res = qcril_qmi_voice_post_manage_voip_calls_request( VOIP_SUPS_TYPE_MAKE_CONFERENCE_CALL_V02, reqlist_entry.req_id  );

      if (RIL_E_SUCCESS == ril_req_res)
      {
        call_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration(QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL, TRUE);
        if ( NULL == call_entry )
        {
           qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL, CALL_STATE_HOLD_V02);
           qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL, CALL_STATE_CONVERSATION_V02);
           call_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL, TRUE);
           if (call_entry)
           {
              call_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL;
           }

           qcril_qmi_voice_ims_conf_req_state_start_vcl();
        }
        else
        {
           qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL, CALL_STATE_HOLD_V02);
           qcril_qmi_voice_voip_mark_with_specified_call_state(QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL, CALL_STATE_CONVERSATION_V02);
           call_entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL;

           qcril_qmi_voice_ims_conf_add_call_req_state_start_vcl();
        }
      }
    }
    else
    { // voice
      memset(&manage_calls_req, 0, sizeof(manage_calls_req));
      manage_calls_req.sups_type = SUPS_TYPE_MAKE_CONFERENCE_CALL_V02;
      manage_calls_req.call_id_valid = FALSE;

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
      qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
      qcril_reqlist_buf_type *req_buf;

      IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                              &reqlist_entry,
                                                                              &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                              &concurency_requirement,
                                                                              sizeof(concurency_requirement),
                                                                              &qcril_qmi_voice_send_management_call_request,
                                                                              &manage_calls_req,
                                                                              sizeof(manage_calls_req),
                                                                              &req_buf );
      if ( E_SUCCESS == new_req_result )
      {
        qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
        ril_req_res = E_SUCCESS;
      }
      else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
      {
        QCRIL_LOG_INFO("the new request is blocked");
        ril_req_res = E_SUCCESS;
      }
      else
      {
        QCRIL_LOG_INFO("the new request is rejected");
      }
    }

  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // failure
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res );
  }

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_manage_calls_conference

//===========================================================================
//RIL_REQUEST_EXPLICIT_CALL_TRANSFER
//===========================================================================
void qcril_qmi_voice_request_manage_calls_explicit_call_transfer
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;
  qcril_qmi_voice_voip_current_call_summary_type call_summary;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_voice_voip_lock_overview();

  do
  {
    qcril_qmi_voice_voip_generate_summary( &call_summary );

    QCRIL_LOG_ERROR(".. nof voice %d, nof voip %d", call_summary.nof_voice_calls, call_summary.nof_voip_calls );
    if ( QMI_RIL_ZERO == call_summary.nof_voip_calls + call_summary.nof_voice_calls )
    {
      break;
    }

    if ( call_summary.nof_voip_calls > QMI_RIL_ZERO )
    { // voip - not applicable
      QCRIL_LOG_ERROR(".. not supported for voip call");
      break;
    }
    else
    { // voice
      memset(&manage_calls_req, 0, sizeof(manage_calls_req));
      manage_calls_req.sups_type = SUPS_TYPE_EXPLICIT_CALL_TRANSFER_V02;
      manage_calls_req.call_id_valid = FALSE;

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
      qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
      qcril_reqlist_buf_type *req_buf;

      IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                              &reqlist_entry,
                                                                              &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                              &concurency_requirement,
                                                                              sizeof(concurency_requirement),
                                                                              &qcril_qmi_voice_send_management_call_request,
                                                                              &manage_calls_req,
                                                                              sizeof(manage_calls_req),
                                                                              &req_buf );

      if ( E_SUCCESS == new_req_result )
      {
        qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
        ril_req_res = E_SUCCESS;
      }
      else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
      {
        QCRIL_LOG_INFO("the new request is blocked");
        ril_req_res = E_SUCCESS;
      }
      else
      {
        QCRIL_LOG_INFO("the new request is rejected");
      }
    }

  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // failure
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
    qcril_send_request_response( &resp );
  }

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();

}/*qcril_qmi_voice_request_manage_calls_explicit_call_transfer*/

//===========================================================================
//RIL_REQUEST_SEPARATE_CONNECTION
//===========================================================================
void qcril_qmi_voice_request_manage_calls_seperate_connection
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr //Output parameter
)
{
  qcril_reqlist_public_type                       reqlist_entry;
  qcril_request_resp_params_type                  resp;
  voice_manage_calls_req_msg_v02                  manage_calls_req;

  qcril_qmi_voice_voip_current_call_summary_type  call_summary;
  RIL_Errno                                       ril_req_res = RIL_E_GENERIC_FAILURE;
  int                                             must_responds_anyway = FALSE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  qcril_qmi_voice_voip_call_info_entry_type *     call_info_entry = NULL;
  int *                                           param_call_id = NULL;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_voice_voip_lock_overview();

  do
  {
    param_call_id = (int *) params_ptr->data; // Connection Index
    if ( NULL == param_call_id )
    {
      QCRIL_LOG_ERROR(".. invalid param");
      break;
    }

    QCRIL_LOG_INFO( ".. con idx param %d", (int)*param_call_id );
    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id( *param_call_id );
    if ( NULL == call_info_entry )
    {
      QCRIL_LOG_ERROR(".. internal call record not found");
      break;
    }

    qcril_qmi_voice_voip_generate_summary( &call_summary );

    QCRIL_LOG_INFO( ".. nof voice %d, nof voip %d, nof 3gpp %d, nof 3gpp2 %d, nof active %d",
                    call_summary.nof_voice_calls, call_summary.nof_voip_calls,
                    call_summary.nof_3gpp_calls, call_summary.nof_3gpp2_calls,
                    call_summary.nof_active_calls );
    if ( QMI_RIL_ZERO == call_summary.nof_voip_calls + call_summary.nof_voice_calls )
      break;

    if ( call_summary.nof_voip_calls > QMI_RIL_ZERO || call_summary.nof_3gpp2_calls > QMI_RIL_ZERO )
    { // voip - not applicable
      QCRIL_LOG_ERROR(".. not supported for voip or 3gpp2 call");
      break;
    }
    else if ( 1 == call_summary.nof_calls_overall )
    { // single call, return success
      ril_req_res = RIL_E_SUCCESS;
      must_responds_anyway = TRUE;
      break;
    }
    else if ( 1 == call_summary.nof_active_calls &&
              CALL_STATE_CONVERSATION_V02 == call_info_entry->voice_scv_info.call_state )
    { // the call_id passed in is the only active call, return success
      ril_req_res = RIL_E_SUCCESS;
      must_responds_anyway = TRUE;
      break;
    }
    else
    { // 3gpp conf call
      memset(&manage_calls_req, 0, sizeof(manage_calls_req));
      manage_calls_req.sups_type          = SUPS_TYPE_HOLD_ALL_EXCEPT_SPECIFIED_CALL_V02;
      manage_calls_req.call_id_valid      = TRUE;
      manage_calls_req.call_id            = call_info_entry->qmi_call_id;

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
      qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
      qcril_reqlist_buf_type *req_buf;

      IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                              &reqlist_entry,
                                                                              &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                              &concurency_requirement,
                                                                              sizeof(concurency_requirement),
                                                                              &qcril_qmi_voice_send_management_call_request,
                                                                              &manage_calls_req,
                                                                              sizeof(manage_calls_req),
                                                                              &req_buf );
      if ( E_SUCCESS == new_req_result )
      {
        qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
        ril_req_res = E_SUCCESS;
      }
      else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
      {
        QCRIL_LOG_INFO("the new request is blocked");
        ril_req_res = E_SUCCESS;
      }
      else
      {
        QCRIL_LOG_INFO("the new request is rejected");
      }
    }
  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res || must_responds_anyway )
  { // failure
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
    qcril_send_request_response( &resp );
  }

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_manage_calls_seperate_connection

//===========================================================================
//RIL_REQUEST_UDUB
//===========================================================================
void qcril_qmi_voice_request_manage_calls_udub
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;

  qcril_qmi_voice_voip_current_call_summary_type call_summary;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_voice_voip_lock_overview();

  do
  {
    qcril_qmi_voice_voip_generate_summary( &call_summary );

    QCRIL_LOG_ERROR(".. nof voice %d, nof voip %d", call_summary.nof_voice_calls, call_summary.nof_voip_calls );
    if ( QMI_RIL_ZERO == call_summary.nof_voip_calls + call_summary.nof_voice_calls )
    {
      break;
    }

    if ( call_summary.nof_voip_calls > QMI_RIL_ZERO )
    { // voip - not applicable
      QCRIL_LOG_ERROR(".. not supported for voip call");
      break;
    }
    else
    { // voice
      memset(&manage_calls_req, 0, sizeof(manage_calls_req));
      manage_calls_req.sups_type = SUPS_TYPE_RELEASE_HELD_OR_WAITING_V02;
      manage_calls_req.call_id_valid = FALSE;

      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   QCRIL_DEFAULT_MODEM_ID,
                                   QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      qcril_reqlist_generic_concurency_requests_requirement_type concurency_requirement;
      qcril_qmi_voice_set_management_call_concurrency_requirement(&concurency_requirement);
      qcril_reqlist_buf_type *req_buf;

      IxErrnoType new_req_result = qcril_reqlist_new_with_concurency_control( instance_id,
                                                                              &reqlist_entry,
                                                                              &qcril_reqlist_generic_check_concurrency_from_set_of_requests,
                                                                              &concurency_requirement,
                                                                              sizeof(concurency_requirement),
                                                                              &qcril_qmi_voice_send_management_call_request,
                                                                              &manage_calls_req,
                                                                              sizeof(manage_calls_req),
                                                                              &req_buf );
      if ( E_SUCCESS == new_req_result )
      {
        qcril_qmi_voice_send_management_call_request( instance_id, req_buf, &manage_calls_req, sizeof(manage_calls_req) );
        ril_req_res = E_SUCCESS;
      }
      else if ( E_BLOCKED_BY_OUTSTANDING_REQ == new_req_result )
      {
        QCRIL_LOG_INFO("the new request is blocked");
        ril_req_res = E_SUCCESS;
      }
      else
      {
        QCRIL_LOG_INFO("the new request is rejected");
      }
    }
  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // failure
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
    qcril_send_request_response( &resp );
  }

  qcril_qmi_voice_voip_unlock_overview();
  QCRIL_LOG_FUNC_RETURN();

} // qcril_qmi_voice_request_manage_calls_udub

/*===========================================================================

  FUNCTION:  qcril_cm_supsvc_request_set_supp_svc_notification

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION.
    Enables/disables supplementary service related notifications from the network.
    Notifications are reported via RIL_UNSOL_SUPP_SVC_NOTIFICATION

    All the events required for notification are either registered or /de-registered
    based on the input.

    Input Paramerers:
    const int *
    ((const int *)data)[0] to enable or disable the notifications.

    @return:
    None
*/
/*=========================================================================*/
void qcril_qmi_voice_request_set_supp_svc_notification
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  int *in_data_ptr;
  Ims__SuppSvcStatus *ims_in_data_ptr;
  uint32 user_data;
  int supps_notification;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;
  voice_indication_register_req_msg_v02  indication_req;
  voice_indication_register_resp_msg_v02* indication_resp_msg_ptr = NULL;
  RIL_Errno res = RIL_E_SUCCESS;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  if (NULL == params_ptr || NULL == params_ptr->data)
  {
    QCRIL_LOG_INFO("params_ptr or params_ptr is NULL");
    return;
  }

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  if ( RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION == params_ptr->event_id )
  {
    in_data_ptr = (int *)params_ptr->data;
    supps_notification = in_data_ptr[0];
  }
  else
  {
    ims_in_data_ptr = (Ims__SuppSvcStatus *)params_ptr->data;
    supps_notification = (int)ims_in_data_ptr->status;
  }
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
        res = RIL_E_GENERIC_FAILURE;
        break;
    }

    QCRIL_LOG_DEBUG( "RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION %s",
                     ( ( supps_notification == (int) QCRIL_QMI_VOICE_SS_ENABLE_NOTIFICATION) ? "Enable" : "Disable" ) );

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    memset(&indication_req, 0, sizeof(indication_req));
    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
    indication_resp_msg_ptr = qcril_malloc( sizeof(*indication_resp_msg_ptr) );
    if( NULL == indication_resp_msg_ptr )
    {
      res = RIL_E_GENERIC_FAILURE;
      break;
    }

    if ( supps_notification == (int) QCRIL_QMI_VOICE_SS_ENABLE_NOTIFICATION )
    {
      /* Register for Call events */
      /* Send QMI VOICE INDICATION REQ */
      indication_req.supps_notification_events_valid = TRUE;
      indication_req.supps_notification_events = 0x01;
      if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                        &indication_req,
                                        sizeof(indication_req),
                                        indication_resp_msg_ptr,
                                        sizeof(*indication_resp_msg_ptr),
                                        (void*)(uintptr_t)user_data) != E_SUCCESS )
      {
        /* In case of modem reset, the command callback will never be executed. So, need to
           delete the entry from the ReqList, and call OnRequestComplete() */
        QCRIL_LOG_ERROR( "Failed to register Call events for sups notifications" );
        qcril_free(indication_resp_msg_ptr);
        res = RIL_E_GENERIC_FAILURE;
      }
    }
    else if ( supps_notification == (int) QCRIL_QMI_VOICE_SS_DISABLE_NOTIFICATION )
    {
      /* Register for Call events */
      /* Send QMI VOICE INDICATION REQ */
      indication_req.supps_notification_events_valid = TRUE;
      indication_req.supps_notification_events = 0x00;
      if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                        &indication_req,
                                        sizeof(indication_req),
                                        indication_resp_msg_ptr,
                                        sizeof(*indication_resp_msg_ptr),
                                        (void*)(uintptr_t)user_data) != E_SUCCESS )
      {
        /* In case of modem reset, the command callback will never be executed. So, need to
           delete the entry from the ReqList, and call OnRequestComplete() */
        QCRIL_LOG_ERROR( "Failed to register Call events for sups notifications, error %d", res );
        qcril_free( indication_resp_msg_ptr );
        res = RIL_E_GENERIC_FAILURE;
      }
    }
    else
    {
      QCRIL_LOG_ERROR("Received invalid params in RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION");
      qcril_free( indication_resp_msg_ptr );
      res = RIL_E_GENERIC_FAILURE;
    }
  }while(0);

  if( res != RIL_E_SUCCESS )
  {
    qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, res );
  }

  if (QCRIL_EVT_IMS_SOCKET_REQ_SET_SUPP_SVC_NOTIFICATION == params_ptr->event_id)
  {
    qcril_qmi_ims__supp_svc_status__free_unpacked(ims_in_data_ptr, NULL);
  }


} /* qcril_cm_supsvc_request_set_supp_svc_notification() */

//===========================================================================
// qcril_qmi_voice_request_last_call_fail_cause / RIL_REQUEST_LAST_CALL_FAIL_CAUSE
//===========================================================================
void qcril_qmi_voice_request_last_call_fail_cause
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type reqlist_entry;

  QCRIL_NOTUSED( ret_ptr );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, RIL_REQUEST_LAST_CALL_FAIL_CAUSE,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
  {
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE);
  }
  else
  {
    qcril_qmi_voice_set_last_call_fail_request_timeout();

    qcril_qmi_voice_voip_lock_overview();
    qcril_qmi_voice_respond_ril_last_call_failure_request();
    qcril_qmi_voice_voip_unlock_overview();
  }
} // qcril_qmi_voice_request_last_call_fail_cause


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_set_call_forward

===========================================================================*/
/*!
    @brief
    Handles Set call forwaring Requests.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_set_call_forward
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{

  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  qcril_qmi_voice_callforwd_info_param_u_type *in_data_ptr;
  Ims__CallForwardInfoList *ims_in_data_ptr;
  qcril_reqlist_public_type reqlist_entry;
  voice_set_sups_service_req_msg_v02 set_sups_cfw_req;
  voice_set_sups_service_resp_msg_v02* set_sups_cfw_resp_ptr = NULL;
  int cf_num_len=0,return_back=0;
  int  status;
  int  reason;
  int  service_class;
  int  toa;
  char *number;
  int  no_reply_timer;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        break;
    }

    if (RIL_REQUEST_SET_CALL_FORWARD == params_ptr->event_id)
    {
        in_data_ptr = (qcril_qmi_voice_callforwd_info_param_u_type *)params_ptr->data;
        status = in_data_ptr->status;
        reason = in_data_ptr->reason;
        service_class = in_data_ptr->service_class;
        toa = in_data_ptr->toa;
        number = in_data_ptr->number;
        no_reply_timer = in_data_ptr->no_reply_timer;
    }
    else
    {
        ims_in_data_ptr = (Ims__CallForwardInfoList *)params_ptr->data;
        if( ( NULL == ims_in_data_ptr->info ) || ( NULL == ims_in_data_ptr->info[0] ) )
        {
           qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
           qcril_qmi_ims__call_forward_info_list__free_unpacked(ims_in_data_ptr, NULL);
           break;
        }

        status = ims_in_data_ptr->info[0]->status;
        reason = ims_in_data_ptr->info[0]->reason;
        service_class = ims_in_data_ptr->info[0]->service_class;
        toa = ims_in_data_ptr->info[0]->toa;
        number = ims_in_data_ptr->info[0]->number;
        no_reply_timer = ims_in_data_ptr->info[0]->time_seconds;
    }

    QCRIL_LOG_DEBUG( "SET_CALL_FORWARD status = %d, reason = %d, serviceClass = %d",
                     status, reason, service_class );


    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    memset(&set_sups_cfw_req, 0, sizeof(set_sups_cfw_req));

    return_back = 0;
    switch(status)
    {
      case QCRIL_QMI_VOICE_MODE_DISABLE :
        set_sups_cfw_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
        break;

      case QCRIL_QMI_VOICE_MODE_ENABLE :
        set_sups_cfw_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
        break;

      case QCRIL_QMI_VOICE_MODE_REG :
        set_sups_cfw_req.supplementary_service_info.voice_service = VOICE_SERVICE_REGISTER_V02;
        break;

      case QCRIL_QMI_VOICE_MODE_ERASURE:
        set_sups_cfw_req.supplementary_service_info.voice_service = VOICE_SERVICE_ERASE_V02;
        break;

      default:
        /* Fail to add entry to ReqList */
        QCRIL_LOG_ERROR("Invalid status req: %d", status);
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        return_back = 1;
    }
    if( return_back == 1 )
    {
      break;
    }

    switch(reason)
    {
      case QCRIL_QMI_VOICE_CCFC_REASON_UNCOND :
        set_sups_cfw_req.supplementary_service_info.reason = VOICE_REASON_FWD_UNCONDITIONAL_V02;
        break;

      case QCRIL_QMI_VOICE_CCFC_REASON_BUSY:
        set_sups_cfw_req.supplementary_service_info.reason = VOICE_REASON_FWD_MOBILEBUSY_V02;
        break;

      case QCRIL_QMI_VOICE_CCFC_REASON_NOREPLY:
        set_sups_cfw_req.supplementary_service_info.reason = VOICE_REASON_FWD_NOREPLY_V02;
        break;

      case QCRIL_QMI_VOICE_CCFC_REASON_NOTREACH:
        set_sups_cfw_req.supplementary_service_info.reason = VOICE_REASON_FWD_UNREACHABLE_V02;
        break;

      case QCRIL_QMI_VOICE_CCFC_REASON_ALLCALL:
        set_sups_cfw_req.supplementary_service_info.reason = VOICE_REASON_FWD_ALLFORWARDING_V02;
        break;

      case QCRIL_QMI_VOICE_CCFC_REASON_ALLCOND:
        set_sups_cfw_req.supplementary_service_info.reason = VOICE_REASON_FWD_ALLCONDITIONAL_V02;
        break;

      default:
        set_sups_cfw_req.supplementary_service_info.reason = 0xFF;
        break;
    }

    if(service_class > 0)
    {
      set_sups_cfw_req.service_class =  service_class;
      set_sups_cfw_req.service_class_valid = TRUE;
    }
    else
    {
      set_sups_cfw_req.service_class_valid = FALSE;
    }
    set_sups_cfw_req.password_valid = FALSE;

    if(number != NULL)
    {
      cf_num_len = strlen(number);
    }

    if ( (number != NULL) &&
         (cf_num_len > 0) && (cf_num_len <= QMI_VOICE_NUMBER_MAX_V02) &&
         (status == (int) QCRIL_QMI_VOICE_MODE_REG) )
    {
      set_sups_cfw_req.number_valid = TRUE;

      QCRIL_LOG_DEBUG("toa=%d, cf number=%s, len=%d", toa, number, cf_num_len);
      if((toa == 145) && (number[0] != '+'))
      {
        set_sups_cfw_req.number[0] = '+';
        memcpy(&set_sups_cfw_req.number[1],number,cf_num_len);
      }
      else if((toa == 145) && (number[0] == '+'))
      {
        set_sups_cfw_req.number[0] = '+';
        memcpy(&set_sups_cfw_req.number[1],&(number[1]),cf_num_len-1);
      }
      else if((toa == 129) && (number[0] != '+'))
      {
        memcpy(set_sups_cfw_req.number,number,cf_num_len);
      }
      else
      {
        memcpy(set_sups_cfw_req.number,(number),cf_num_len);
      }
    }

    if ( ( ( reason == (int) QCRIL_QMI_VOICE_CCFC_REASON_NOREPLY ) ||
           ( reason == (int) QCRIL_QMI_VOICE_CCFC_REASON_ALLCALL ) ||
           ( reason == (int) QCRIL_QMI_VOICE_CCFC_REASON_ALLCOND ) ) &&
         ( status == (int) QCRIL_QMI_VOICE_MODE_REG ) &&
         ( no_reply_timer != 0 ) )
    {
      set_sups_cfw_req.timer_value_valid = TRUE;
      set_sups_cfw_req.timer_value = no_reply_timer;
    }
    else
    {
      set_sups_cfw_req.timer_value_valid = FALSE;
    }

    if ((params_ptr->event_id == QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS) &&
         (reason == (int) QCRIL_QMI_VOICE_CCFC_REASON_UNCOND))
    {
      if (ims_in_data_ptr->info[0]->callfwdtimerstart)
      {
        QCRIL_LOG_DEBUG("Valid callFwdTimerStart");
        set_sups_cfw_req.call_fwd_start_time_valid = TRUE;
        qcril_qmi_ims_translate_ims_callfwdtimerinfo_to_voice_time_type(
                ims_in_data_ptr->info[0]->callfwdtimerstart,
                &(set_sups_cfw_req.call_fwd_start_time));
      }

      if (ims_in_data_ptr->info[0]->callfwdtimerend)
      {
        QCRIL_LOG_DEBUG("Valid callFwdTimerEnd");
        set_sups_cfw_req.call_fwd_end_time_valid = TRUE;
        qcril_qmi_ims_translate_ims_callfwdtimerinfo_to_voice_time_type(
                ims_in_data_ptr->info[0]->callfwdtimerend,
                &(set_sups_cfw_req.call_fwd_end_time));
      }
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
    set_sups_cfw_resp_ptr = qcril_malloc( sizeof(*set_sups_cfw_resp_ptr) );
    if( NULL == set_sups_cfw_resp_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    /* Send QMI VOICE SET SUPS SERVICE REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_SET_SUPS_SERVICE_REQ_V02,
                                      &set_sups_cfw_req,
                                      sizeof(set_sups_cfw_req),
                                      set_sups_cfw_resp_ptr,
                                      sizeof(*set_sups_cfw_resp_ptr),
                                      (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
         delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( set_sups_cfw_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }

    if (QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS == params_ptr->event_id)
    {
      qcril_qmi_ims__call_forward_info_list__free_unpacked(ims_in_data_ptr, NULL);
    }
  }while(0);
} /* qcril_qmi_voice_request_set_call_forward() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_set_supp_svc

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS with Activate/Deactivate
    for CLIP, COLP, CALL Barring facilities.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_qmi_voice_request_set_supp_svc
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  const char **in_data = (const char **)params_ptr->data;
  int facility = 0;
  int operation = 0;
  voice_set_sups_service_req_msg_v02 set_sups_cb_req;
  voice_set_sups_service_resp_msg_v02* set_sups_cb_resp_ptr = NULL;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  IxErrnoType err_no;
  boolean is_error = FALSE;

  /* IMS SuppSvcRequest message pointer */
  Ims__SuppSvcRequest *ims_in_data_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    memset(&set_sups_cb_req, 0, sizeof(set_sups_cb_req));

    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
      break;
    }

    ims_in_data_ptr = ( Ims__SuppSvcRequest * )params_ptr->data;

    facility =
      (int) qcril_qmi_voice_convert_ims_to_ril_facility_type\
      ( ims_in_data_ptr->facilitytype );

    set_sups_cb_req.supplementary_service_info.reason = facility;

    operation = ( int )ims_in_data_ptr->operationtype;

    switch( operation )
    {
      case IMS__SUPP_SVC_OPERATION_TYPE__DEACTIVATE :
        set_sups_cb_req.supplementary_service_info.voice_service =
          VOICE_SERVICE_DEACTIVATE_V02;
        break;

      case IMS__SUPP_SVC_OPERATION_TYPE__ACTIVATE :
        set_sups_cb_req.supplementary_service_info.voice_service =
          VOICE_SERVICE_ACTIVATE_V02;
        break;

      case IMS__SUPP_SVC_OPERATION_TYPE__REGISTER :
        set_sups_cb_req.supplementary_service_info.voice_service =
          VOICE_SERVICE_REGISTER_V02;
        break;

      case IMS__SUPP_SVC_OPERATION_TYPE__ERASURE :
        set_sups_cb_req.supplementary_service_info.voice_service =
          VOICE_SERVICE_ERASE_V02;
        break;

      default:
        /* Fail to add entry to ReqList */
        QCRIL_LOG_ERROR( "Invalid Mode req : %d", operation );

        qcril_send_empty_payload_request_response( instance_id,
                                                   params_ptr->t,
                                                   params_ptr->event_id,
                                                   RIL_E_GENERIC_FAILURE );
        boolean is_error = TRUE;
        break;
    }

    if (is_error)
    {
      break;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );
    set_sups_cb_resp_ptr = qcril_malloc( sizeof( *set_sups_cb_resp_ptr ) );

    if( NULL == set_sups_cb_resp_ptr )
    {
      QCRIL_LOG_ERROR( "Failed to malloc resp ptr" );
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
        break;
    }

    if (IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BS_MT == ims_in_data_ptr->facilitytype)
    {
        if (ims_in_data_ptr->cbnumlisttype)
        {
            if (ims_in_data_ptr->cbnumlisttype->serviceclass->has_service_class)
            {
                set_sups_cb_req.service_class_valid = TRUE;
                set_sups_cb_req.service_class = ims_in_data_ptr->cbnumlisttype->serviceclass->service_class;
            }

            set_sups_cb_req.call_barring_numbers_list_valid = TRUE;
            set_sups_cb_req.call_barring_numbers_list_len = ims_in_data_ptr->cbnumlisttype->n_cb_num_list;
            if (set_sups_cb_req.call_barring_numbers_list_len > QMI_VOICE_MAX_BARRED_NUMBERS_LIST_V02)
            {
                QCRIL_LOG_DEBUG( "list_len (%d) > QMI MAX_LIST_LEN (%d), reset to MAX_LIST_LEN",
                                 set_sups_cb_req.call_barring_numbers_list_len,
                                 QMI_VOICE_MAX_BARRED_NUMBERS_LIST_V02 );
                set_sups_cb_req.call_barring_numbers_list_len = QMI_VOICE_MAX_BARRED_NUMBERS_LIST_V02;
            }

            size_t i;
            for (i=0; i<set_sups_cb_req.call_barring_numbers_list_len; i++)
            {
                strlcpy( set_sups_cb_req.call_barring_numbers_list[i].barred_number,
                         ims_in_data_ptr->cbnumlisttype->cb_num_list[i]->number,
                         sizeof(set_sups_cb_req.call_barring_numbers_list[i].barred_number) );
            }
        }
    }

    /* Send QMI VOICE SET SUPS SERVICE REQ */
    if( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_SET_SUPS_SERVICE_REQ_V02,
                                          &set_sups_cb_req,
                                          sizeof(set_sups_cb_req),
                                          set_sups_cb_resp_ptr,
                                          sizeof(*set_sups_cb_resp_ptr),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of modem reset, the command callback will never be executed. So, need to
      delete the entry from the ReqList, and call OnRequestComplete() */
      QCRIL_LOG_ERROR( "Send msg async error " );
      qcril_free( set_sups_cb_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
    }
  }while(0);

  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_voice_request_set_supp_svc */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_set_facility_lock

===========================================================================*/
/*!
    @brief
    Handles Set call barring request.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_set_facility_lock
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  const char **in_data = (const char **)params_ptr->data;
  const char *facility_string = NULL;
  const char *facility_status = NULL;
  const char *facility_password = NULL;
  const char *facility_class = NULL;
  int facility, status, service_class,return_back = 0;
  char facility_name[ 3 ];
  voice_set_sups_service_req_msg_v02 set_sups_cb_req;
  voice_set_sups_service_resp_msg_v02* set_sups_cb_resp_ptr = NULL;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  IxErrnoType err_no;


  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
  memset(&set_sups_cb_req, 0, sizeof(set_sups_cb_req));

    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
        break;
    }

    facility_string = in_data[ 0 ];
    facility_status = in_data[ 1 ];
    facility_password = in_data[ 2 ];
    facility_class = in_data [ 3 ];
    if ( facility_string == NULL || facility_status == NULL)
    {
        qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
        qcril_send_request_response( &resp );
        break;
    }


  facility = qcril_qmi_voice_get_facility_value( facility_string, &facility_name[ 0 ] );

  QCRIL_LOG_DEBUG( "RIL_REQUEST_SET_FACILITY_LOCK Facility %s, %s, Facility type = %02x",
                   facility_name, ( ( *facility_status == '0' ) ? "unlock": "lock" ), facility );
  if( facility_password != NULL )
  {
    QCRIL_LOG_DEBUG( "RIL_REQUEST_SET_FACILITY_LOCK Facility pw %s",facility_password );
  }

  if ( facility == (int) QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED )
  {
    /* Call event handler of QCRIL_MMGSDI which will take care of handling and
       sending the response back to RIL */
      QCRIL_LOG_ERROR( "TODO Unsupported facility in set facility lock = %s", facility_name );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
        break;
  }

  if ( facility == (int) QCRIL_QMI_VOICE_FACILITY_LOCK_SC )
  {
    /* Call event handler of QCRIL_MMGSDI which will take care of handling and
       sending the response back to RIL */
    err_no = qcril_process_event( qmi_ril_get_process_instance_id(), QCRIL_DEFAULT_MODEM_ID, QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS,
                                  params_ptr->data, params_ptr->datalen, params_ptr->t );

    /* Event handler not found */
    if ( err_no != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Event handler not found for Event = %d", QCRIL_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }

      break;
  }

  if ( facility == (int) QCRIL_QMI_VOICE_FACILITY_LOCK_FD )
  {
    /* Call event handler of QCRIL MMGSDI which will take care of handling and
       sending the response back to RIL */
    err_no = qcril_process_event( qmi_ril_get_process_instance_id(), QCRIL_DEFAULT_MODEM_ID, QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS,
                                  params_ptr->data, params_ptr->datalen, params_ptr->t );

    /* Event handler not found */
    if ( err_no != E_SUCCESS )
    {
      QCRIL_LOG_ERROR( "Event handler not found for Event = %d", QCRIL_EVT_INTERNAL_MMGSDI_SET_FDN_STATUS );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }

     break;
  }

  set_sups_cb_req.supplementary_service_info.reason = facility;

  if( facility_class != NULL )
  {
    QCRIL_LOG_DEBUG( "class = %s facility = %d", facility_class, facility );
  }

  status = ( ( *facility_status == '0') ? FALSE: ( *facility_status == '1' ) ? TRUE : 2 );

    return_back = 0;
  switch(status)
  {
    case QCRIL_QMI_VOICE_MODE_DISABLE :
      set_sups_cb_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
      break;

    case QCRIL_QMI_VOICE_MODE_ENABLE :
      set_sups_cb_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
      break;

    default:
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Invalid Mode req : %d",status);
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
        return_back = 1;
    }
    if( return_back == 1 )
    {
      break;
  }

  service_class = 0;
  if( facility_class != NULL )
  {
    service_class = atoi( facility_class );
  }

  if(service_class > 0)
  {
  set_sups_cb_req.service_class =  service_class;
  set_sups_cb_req.service_class_valid = TRUE;
  }
  else
  {
    set_sups_cb_req.service_class_valid = FALSE;
  }
  if ( facility_password != NULL )
  {
    set_sups_cb_req.password_valid = TRUE;
    memcpy( set_sups_cb_req.password, facility_password, strlen(facility_password) );
  }

  set_sups_cb_req.timer_value_valid = FALSE;
  set_sups_cb_req.number_valid = FALSE;


  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add entry to ReqList */
    QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
  }

  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
  set_sups_cb_resp_ptr = qcril_malloc( sizeof(*set_sups_cb_resp_ptr) );
  if( NULL == set_sups_cb_resp_ptr )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    break;
  }

/* Send QMI VOICE SET SUPS SERVICE REQ */
  if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_SET_SUPS_SERVICE_REQ_V02,
                                        &set_sups_cb_req,
                                        sizeof(set_sups_cb_req),
                                        set_sups_cb_resp_ptr,
                                        sizeof(*set_sups_cb_resp_ptr),
                                        (void*)(uintptr_t)user_data) != E_SUCCESS )
{
  /* In case of ARM9 reset, the command callback will never be executed. So, need to
   delete the entry from the ReqList, and call OnRequestComplete() */
  qcril_free( set_sups_cb_resp_ptr );
  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
  qcril_send_request_response( &resp );
}
}while(0);

} /* qcril_qmi_voice_request_set_facility_lock() */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_set_call_waiting

===========================================================================*/
/*!
    @brief
    Handles Set call waiting request.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_set_call_waiting
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  int *in_data_ptr;
  Ims__CallWaitingInfo *ims_in_data_ptr;
  unsigned int in_data_len;
  int  status, service_class, return_back = 0;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_set_sups_service_req_msg_v02 set_sups_cw_req;
  voice_set_sups_service_resp_msg_v02* set_sups_cw_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  in_data_len = params_ptr->datalen;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        break;
    }

    if (RIL_REQUEST_SET_CALL_WAITING == params_ptr->event_id)
    {
      in_data_ptr = (int *)params_ptr->data;
      status = in_data_ptr[0];
      service_class = in_data_ptr[ 1 ];
      QCRIL_LOG_INFO( "RIL_REQUEST_SET_CALL_WAITING status = %d, service_class = %d", status, service_class );
    }
    else
    {
      ims_in_data_ptr = (Ims__CallWaitingInfo *)params_ptr->data;
      status = ims_in_data_ptr->service_status;
      if (ims_in_data_ptr->service_class)
      {
        service_class = ims_in_data_ptr->service_class->service_class;
      }
      else
      {
        QCRIL_LOG_DEBUG("no service class in request");
        service_class = 0;
      }
      QCRIL_LOG_INFO( "IMS_REQUEST_SET_CALL_WAITING status = %d, service_class = %d", status, service_class );

      qcril_qmi_ims__call_waiting_info__free_unpacked(ims_in_data_ptr, NULL);
    }

    memset(&set_sups_cw_req, 0, sizeof(set_sups_cw_req));

    return_back = 0;
    switch(status)
    {
      case QCRIL_QMI_VOICE_MODE_DISABLE :
        set_sups_cw_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
        break;

      case QCRIL_QMI_VOICE_MODE_ENABLE :
        set_sups_cw_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
        break;

      default:
        /* Fail to add entry to ReqList */
        QCRIL_LOG_ERROR("Invalid Mode req : %d",status);
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        return_back = 1;
    }
    if( return_back == 1 )
    {
      break;
    }

    set_sups_cw_req.supplementary_service_info.reason = QCRIL_QMI_VOICE_REASON_CALL_WAITING;
    if(service_class > 0)
    {
      set_sups_cw_req.service_class =  service_class;
      set_sups_cw_req.service_class_valid = TRUE;
    }
    else
    {
      set_sups_cw_req.service_class_valid = FALSE;
    }
    set_sups_cw_req.number_valid = FALSE;
    set_sups_cw_req.password_valid = FALSE;
    set_sups_cw_req.timer_value_valid = FALSE;

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
    set_sups_cw_resp_ptr = qcril_malloc( sizeof(*set_sups_cw_resp_ptr) );
    if( NULL == set_sups_cw_resp_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    /* Send QMI VOICE SET SUPS SERVICE REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_SET_SUPS_SERVICE_REQ_V02,
                                          &set_sups_cw_req,
                                          sizeof(set_sups_cw_req),
                                          set_sups_cw_resp_ptr,
                                          sizeof(*set_sups_cw_resp_ptr),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( set_sups_cw_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_set_call_waiting() */

/*===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_QUERY_CLIP.
    Queries for the status of the calling line identity presentation
    supplimentary services present with the network(for MMI code "*#30#").

    Input Paramerers:
    NULL

    Result:
    NULL ( after receiving the appropriate response from AMSS,
           returns the status of CLIP i.e whether it is active or not )
*/
/*=========================================================================*/
void qcril_qmi_voice_request_query_clip
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  voice_get_clip_resp_msg_v02* get_clip_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

    get_clip_resp_ptr = qcril_malloc( sizeof(*get_clip_resp_ptr) );
    if( NULL == get_clip_resp_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    /* Send QMI VOICE GET_CLIP REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_GET_CLIP_REQ_V02,
                                          NULL,
                                          0,
                                          get_clip_resp_ptr,
                                          sizeof(*get_clip_resp_ptr),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( get_clip_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_query_clip() */


/*===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS for COLP.
    Queries for the status of the Connected Line Identification Presentation.

    Input Paramerers:
    NULL

    Result:
    NULL ( after receiving the appropriate response from AMSS,
           returns the status of COLP i.e whether it is active or not )
*/
/*=========================================================================*/
static void qcril_qmi_voice_request_query_colp
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  voice_get_colp_resp_msg_v02* get_colp_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    get_colp_resp_ptr = qcril_malloc( sizeof(*get_colp_resp_ptr) );
    if( NULL == get_colp_resp_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
      break;
    }

    /* Send QMI VOICE GET_COLP REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_GET_COLP_REQ_V02,
                                          NULL,
                                          0,
                                          get_colp_resp_ptr,
                                          sizeof(*get_colp_resp_ptr),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of modem reset, command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( get_colp_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
    }
  }while(0);
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_query_colp() */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_get_clir

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_CLIR.
    Queries for the status of the calling line identity restriction.

    Input Paramerers:
    NULL

    Result:
    NULL ( after receiving the appropriate response from AMSS,
           returns the status of CLIR i.e whether it is active or not )
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_get_clir
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  qcril_reqlist_public_type reqlist_entry;
  voice_get_clir_resp_msg_v02* get_clir_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

    get_clir_resp_ptr = qcril_malloc( sizeof(*get_clir_resp_ptr) );
    if( NULL == get_clir_resp_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    /* Send QMI VOICE DIAL CALL REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                           QMI_VOICE_GET_CLIR_REQ_V02,
                                           NULL,
                                           0,
                                           get_clir_resp_ptr,
                                           sizeof(*get_clir_resp_ptr),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
          delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( get_clir_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_get_clir() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_query_call_waiting

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_QUERY_CALL_WAITING.
    Queries for the status of the call waiting supplimentary services present
    with the network.

    Input Paramerers:
    const int *
    ((const int *)data)[0] is the TS 27.007 service class to query

    though service class is provided as input, it is not required for this SS.

    @return:
    None ( after receiving the appropriate response from AMSS,
           returns the basic service groups list for which call waiting is active )

*/
/*=========================================================================*/
void qcril_qmi_voice_request_query_call_waiting
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  int service_class;
  Ims__ServiceClass *ims_data_ptr;
  qcril_reqlist_public_type reqlist_entry;
  voice_get_call_waiting_req_msg_v02 get_cw_req;
  voice_get_call_waiting_resp_msg_v02* get_cw_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if (RIL_REQUEST_QUERY_CALL_WAITING == params_ptr->event_id)
    {
      service_class = ((int *) params_ptr->data)[ 0 ];
    }
    else
    {
      ims_data_ptr = (Ims__ServiceClass*) params_ptr->data;
      service_class = ims_data_ptr->service_class;
      qcril_qmi_ims__service_class__free_unpacked(ims_data_ptr, NULL);
    }
    /* Though we are checking for service class it is ignored in case of invalid
       class here as service class is not required for interrogation, only if
       status is received in resonse then we use service class */
    QCRIL_LOG_DEBUG( "Received service class = %d", service_class );

    memset(&get_cw_req,0,sizeof(get_cw_req));

    if(service_class > 0)
    {
      get_cw_req.service_class =  service_class;
      get_cw_req.service_class_valid = TRUE;
    }
    else
    {
      get_cw_req.service_class_valid = FALSE;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

    get_cw_resp_ptr = qcril_malloc( sizeof(*get_cw_resp_ptr) );
    if( NULL == get_cw_resp_ptr )
    {
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      break;
    }

    /* Send QMI VOICE DIAL CALL REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_GET_CALL_WAITING_REQ_V02,
                                          &get_cw_req,
                                          sizeof(get_cw_req),
                                          get_cw_resp_ptr,
                                          sizeof(*get_cw_resp_ptr),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( get_cw_resp_ptr );
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);

} /* qcril_qmi_voice_request_query_call_waiting() */


/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_change_barring_password

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CHANGE_BARRING_PASSWORD.
    changes the call barrring password which is registered with the network
    and which is required for activation or deactivation of call barring SS

    input Paramerers:

    const char **
    ((const char **)data)[0] = facility  string code from TS 27.007 7.4
    ((const char **)data)[1] = old password
    ((const char **)data)[1] = new password

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_change_barring_password
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  const char **in_data = (const char **)params_ptr->data;
  unsigned int in_data_len = params_ptr->datalen;
  int facility;
  char facility_name[3];
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_set_call_barring_password_req_msg_v02 change_cb_pwd_req;
  voice_set_call_barring_password_resp_msg_v02* change_cb_pwd_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL || in_data[0] == NULL || in_data[1] == NULL || in_data[2] == NULL)
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
    }

  memset(&change_cb_pwd_req,0,sizeof(change_cb_pwd_req));
  facility = qcril_qmi_voice_get_facility_value( in_data[ 0 ], &facility_name[ 0 ] );

  QCRIL_LOG_DEBUG( "RIL_REQUEST_CHANGE_BARRING_PASSWORD Facility %s (%d), old_pw %s, new_pw %s",
                   facility_name, facility, in_data[1], in_data[2] );

  if ( (facility == QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED ) || (in_data_len == 0))
  {
    QCRIL_LOG_ERROR( "received invalid parameters in RIL_REQUEST_CHANGE_BARRING_PASSWORD");
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
      break;
  }
  else
  {
    change_cb_pwd_req.call_barring_password_info.reason = facility;
    /* filling the password information as accepted by QMI_VOICE svc*/
    memcpy(change_cb_pwd_req.call_barring_password_info.old_password,
           in_data[1],
           sizeof(char)* strlen(in_data[1]));

    memcpy(change_cb_pwd_req.call_barring_password_info.new_password,
           in_data[2],
           sizeof(char)* strlen(in_data[2]));

    memcpy(change_cb_pwd_req.call_barring_password_info.new_password_again,
           in_data[2],
           sizeof(char)* strlen(in_data[2]));

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
        break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
    change_cb_pwd_resp_ptr = qcril_malloc( sizeof(*change_cb_pwd_resp_ptr) );
    if( NULL == change_cb_pwd_resp_ptr )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
    }

    /* Send QMI VOICE DIAL CALL REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                           QMI_VOICE_SET_CALL_BARRING_PASSWORD_REQ_V02,
                                           &change_cb_pwd_req,
                                           sizeof(change_cb_pwd_req),
                                           change_cb_pwd_resp_ptr,
                                           sizeof(*change_cb_pwd_resp_ptr),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
          delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( change_cb_pwd_resp_ptr );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }

  }
  }while(0);

} /* qcril_qmi_voice_request_change_barring_password() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_query_facility_lock

===========================================================================*/
/*!
    @brief
    Handles
      1.  RIL_REQUEST_QUERY_FACILITY_LOCK
      2.  QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS for
          IMS__SUPP_SVC_OPERATION_TYPE__QUERY operation with
          Call Barring facility types.

    Queries for the status of the call barring supplimentary services present
    with the network.

    Input Paramerers:
    When RIL_REQUEST_QUERY_FACILITY_LOCK:
       const char **
        ((const char **)data)[0] is the facility string code from TS 27.007 7.4.
        ((const char **)data)[1] is the password.
        ((const char **)data)[2] is the TS 27.007 service class bit vector of services to query
          ((const char **)data)[3] is the AID (applies only in case of FDN, or "" otherwise)

    When QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS:
       (Ims__SuppSvcRequest *)params_ptr->data

    though service class is provided as input, it is not required for call barring SS.

    Result:
    None ( after receiving the appropriate response from AMSS,
              returns the basic service groups list for which call waiting is active )

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_query_facility_lock
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  const char **in_data = (const char **) params_ptr->data;
  const char *facility_string = NULL;
  const char *facility_password = NULL;
  const char *facility_class = NULL;
  int facility = (int) QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED;
  int service_class = 0;
  unsigned int in_data_len = params_ptr->datalen;
  char facility_name[ 3 ];
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_get_call_barring_req_msg_v02 get_cb_req;
  voice_get_call_barring_resp_msg_v02* get_cb_resp_ptr = NULL;
  IxErrnoType err_no;

  /* IMS SUppSvcRequest message pointer */
  Ims__SuppSvcRequest *ims_in_data_ptr = NULL;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;


  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
      qcril_send_empty_payload_request_response( instance_id,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
      break;
    }

    memset(&get_cb_req,0,sizeof(get_cb_req));

    if ( RIL_REQUEST_QUERY_FACILITY_LOCK == params_ptr->event_id )
    {
      facility_string = in_data[ 0 ];
      facility_password = in_data[ 1 ];
      facility_class = in_data [ 2 ];
      if ( facility_string == NULL)
      {
        qcril_default_request_resp_params( instance_id,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_GENERIC_FAILURE,
                                           &resp );
        qcril_send_request_response( &resp );
        break;
      }
      facility = qcril_qmi_voice_get_facility_value( facility_string, &facility_name[ 0 ] );
    }
    else /* QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS */
    {
      ims_in_data_ptr = ( Ims__SuppSvcRequest * )params_ptr->data;
      facility =
        ( int )qcril_qmi_voice_convert_ims_to_ril_facility_type(
        ( Ims__SuppSvcFacilityType )ims_in_data_ptr->facilitytype );
    }

    service_class = 0;
    if( facility_class != NULL )
    {
      service_class = atoi( facility_class );
    }
    else if ( ims_in_data_ptr &&
              ims_in_data_ptr->cbnumlisttype &&
              ims_in_data_ptr->cbnumlisttype->serviceclass &&
              ims_in_data_ptr->cbnumlisttype->serviceclass->has_service_class )
    {
      service_class = ims_in_data_ptr->cbnumlisttype->serviceclass->service_class;
    }

    QCRIL_LOG_DEBUG( "RIL_REQUEST_QUERY_FACILITY_LOCK Facility %s, \
                      service_class %d, Facility type = %02x", facility_name, service_class, facility );

    if (facility_password != NULL)
    {
      QCRIL_LOG_DEBUG( "RIL_REQUEST_QUERY_FACILITY_LOCK Facility pw %s",facility_password);
    }

    if ( ( facility == QCRIL_QMI_VOICE_FACILITY_UNSUPPORTED ) || (in_data_len == 0) )
    {
      QCRIL_LOG_ERROR( "received invalid parameters in RIL_REQUEST_QUERY_FACILITY_LOCK" );
      qcril_default_request_resp_params( instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         RIL_E_GENERIC_FAILURE,
                                         &resp );
      qcril_send_request_response( &resp );
      break;
    }

    if ( facility == (int) QCRIL_QMI_VOICE_FACILITY_LOCK_SC )
    {
      /* Call event handler of QCRIL MMGSDI which will take care of handling and
       sending the response back to RIL */
      err_no = qcril_process_event( qmi_ril_get_process_instance_id(),
                                    QCRIL_DEFAULT_MODEM_ID,
                                    QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS,
                                    params_ptr->data,
                                    params_ptr->datalen,
                                    params_ptr->t );
      /* Event handler not found */
      if ( err_no != E_SUCCESS )
      {
        QCRIL_LOG_ERROR( "Event handler not found for Event = %d",
                         QCRIL_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS );
        qcril_default_request_resp_params( instance_id,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_GENERIC_FAILURE,
                                           &resp );
        qcril_send_request_response( &resp );
      }
      break;
    }

    if ( facility == (int) QCRIL_QMI_VOICE_FACILITY_LOCK_FD )
    {
      /* Call event handler of QCRIL MMGSDI which will take care of handling and
       sending the response back to RIL */
      err_no = qcril_process_event( qmi_ril_get_process_instance_id(),
                                    QCRIL_DEFAULT_MODEM_ID,
                                    QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS,
                                    params_ptr->data,
                                    params_ptr->datalen,
                                    params_ptr->t );
      /* Event handler not found */
      if ( err_no != E_SUCCESS )
      {
        QCRIL_LOG_ERROR( "Event handler not found for Event = %d",
                         QCRIL_EVT_INTERNAL_MMGSDI_GET_FDN_STATUS );
        qcril_default_request_resp_params( instance_id,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_GENERIC_FAILURE,
                                           &resp );
        qcril_send_request_response( &resp );
      }
      break;
    }

    get_cb_req.reason = facility;

    if ( service_class == 0 )
    {
      get_cb_req.service_class_valid = FALSE;
    }
    else
    {
      get_cb_req.service_class = service_class;
      get_cb_req.service_class_valid = TRUE;
    }

    /* Add entry to ReqList */
    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR( "Failed to Add into Req list" );
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         reqlist_entry.req_id );

    get_cb_resp_ptr = qcril_malloc( sizeof(*get_cb_resp_ptr) );
    if( NULL == get_cb_resp_ptr )
    {
      qcril_default_request_resp_params( instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         RIL_E_GENERIC_FAILURE,
                                         &resp );
      qcril_send_request_response( &resp );
      break;
    }

    /* Send QMI VOICE GET CALL BARRING REQ */
    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                           QMI_VOICE_GET_CALL_BARRING_REQ_V02,
                                           &get_cb_req,
                                           sizeof(get_cb_req),
                                           get_cb_resp_ptr,
                                           sizeof(*get_cb_resp_ptr),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( get_cb_resp_ptr );
      qcril_default_request_resp_params( instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         RIL_E_GENERIC_FAILURE,
                                         &resp );
      qcril_send_request_response( &resp );
    }
  }while(0);

} /* qcril_qmi_voice_request_query_facility_lock() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_set_clir

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SET_CLIR.

    Sets the default CLIR value (i_ptr->clir) as per the received input(n value).

    If RIL_REQUEST_DIAL does not specify valid CLIR parameter, use the default
    clir value.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_set_clir
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  int *in_data_ptr;
  Ims__Clir *ims_in_data_ptr;
  uint8  clir_n_param;
  char args[ PROPERTY_VALUE_MAX ];
  char property_name[ 40 ];

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        break;
    }

    if (RIL_REQUEST_SET_CLIR == params_ptr->event_id)
    {
      in_data_ptr = (int *)params_ptr->data;
      clir_n_param = (uint8) in_data_ptr[0]; /* 27.007, section 7.8 */
    }
    else
    {
      ims_in_data_ptr = (Ims__Clir *)params_ptr->data;
      clir_n_param = (uint8) ims_in_data_ptr->param_n; /* 27.007, section 7.8 */
      qcril_qmi_ims__clir__free_unpacked(ims_in_data_ptr, NULL);
    }

    QCRIL_LOG_DEBUG( "RIL_REQUEST_SET_CLIR input = %d", clir_n_param);

    if ( ( clir_n_param == (uint8) QCRIL_QMI_VOICE_SS_CLIR_PRESENTATION_INDICATOR ) ||
         ( clir_n_param == (uint8) QCRIL_QMI_VOICE_SS_CLIR_INVOCATION_OPTION ) ||
         ( clir_n_param == (uint8) QCRIL_QMI_VOICE_SS_CLIR_SUPPRESSION_OPTION ) )
    {
      qcril_qmi_voice_info.clir = clir_n_param;

      /* Save CLIR setting to system property */
      QCRIL_SNPRINTF( args, sizeof(args), "%d", (int) qcril_qmi_voice_info.clir );

      QCRIL_SNPRINTF( property_name, sizeof(property_name), "%s%d",
                      QCRIL_QMI_VOICE_CLIR, qmi_ril_get_process_instance_id() );
      if ( property_set( property_name, args ) != E_SUCCESS )
      {
        QCRIL_LOG_ERROR( "Fail to save %s to system property", property_name );
      }
      QCRIL_LOG_DEBUG( "SET CLIR=%d", qcril_qmi_voice_info.clir );

      /* sending confirmation as we just need to store the request,
         and use it only when call is initiated */
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS );
    }
    else
    {
      QCRIL_LOG_ERROR("received invalid params in RIL_REQUEST_SET_CLIR");
      qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
    }
  }while(0);
  QCRIL_LOG_FUNC_RETURN();

} /* qcril_qmi_voice_request_set_clir() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_query_call_forward_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_QUERY_CALL_FORWARD_STATUS.

    Queries for the status of the calling forwarding supplimentary services
    present with the network.

    Input Paramerers:
    int *.
    ((int *)data)[0] is "reason" from TS 27.007 7.11

    Result:
    NULL ( after receiving the appropriate response from AMSS,
           returns the status of call forwarding supplimentary services )
*/
/*=========================================================================*/
void qcril_qmi_voice_request_query_call_forward_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  RIL_CallForwardInfo *info = NULL;
  Ims__CallForwardInfoList* ims_info = NULL;
  int reason;
  int service_class;
  qcril_reqlist_public_type reqlist_entry;
  voice_get_call_forwarding_req_msg_v02 query_cf_req;
  voice_get_call_forwarding_resp_msg_v02* query_cf_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/
  instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  QCRIL_NOTUSED( ret_ptr );
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
      if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
      {
          qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
          break;
      }

      if (RIL_REQUEST_QUERY_CALL_FORWARD_STATUS == params_ptr->event_id)
      {
         info = ( RIL_CallForwardInfo * )( params_ptr->data );
         reason = info->reason;
         service_class = info->serviceClass;
      }
      else
      {
         // translate
         ims_info = ( Ims__CallForwardInfoList * )( params_ptr->data );
         if( ( NULL == ims_info->info ) || ( NULL == ims_info->info[0] ) )
         {
           qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
           qcril_qmi_ims__call_forward_info_list__free_unpacked(ims_info, NULL);
           break;
         }
         reason = ims_info->info[0]->reason;
         service_class = ims_info->info[0]->service_class;
         qcril_qmi_ims__call_forward_info_list__free_unpacked(ims_info, NULL);
      }

      memset(&query_cf_req,0,sizeof(query_cf_req));

      if ( !( ( reason >= (int) QCRIL_QMI_VOICE_CCFC_REASON_UNCOND ) && ( reason < (int) QCRIL_QMI_VOICE_CCFC_REASON_MAX ) ) )
      {
        QCRIL_LOG_ERROR( "received invalid reason in RIL_REQUEST_QUERY_CALL_FORWARD_STATUS" );
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        break;
      }

      qmi_ril_enter_critical_section();
      switch ( reason )
      {
        case QCRIL_QMI_VOICE_CCFC_REASON_UNCOND :
         query_cf_req.reason = VOICE_REASON_FWD_UNCONDITIONAL_V02;
         qcril_qmi_voice_info.last_cfw_reason = VOICE_REASON_FWD_UNCONDITIONAL_V02;
         break;

        case QCRIL_QMI_VOICE_CCFC_REASON_BUSY:
          query_cf_req.reason = VOICE_REASON_FWD_MOBILEBUSY_V02;
          qcril_qmi_voice_info.last_cfw_reason = VOICE_REASON_FWD_MOBILEBUSY_V02;
          break;

        case QCRIL_QMI_VOICE_CCFC_REASON_NOREPLY:
          query_cf_req.reason = VOICE_REASON_FWD_NOREPLY_V02;
          qcril_qmi_voice_info.last_cfw_reason = VOICE_REASON_FWD_NOREPLY_V02;
          break;

        case QCRIL_QMI_VOICE_CCFC_REASON_NOTREACH:
          query_cf_req.reason = VOICE_REASON_FWD_UNREACHABLE_V02;
          qcril_qmi_voice_info.last_cfw_reason = VOICE_REASON_FWD_UNREACHABLE_V02;
          break;

        case QCRIL_QMI_VOICE_CCFC_REASON_ALLCALL:
          query_cf_req.reason = VOICE_REASON_FWD_ALLFORWARDING_V02;
          qcril_qmi_voice_info.last_cfw_reason = VOICE_REASON_FWD_ALLFORWARDING_V02;
          break;

        case QCRIL_QMI_VOICE_CCFC_REASON_ALLCOND:
          query_cf_req.reason = VOICE_REASON_FWD_ALLCONDITIONAL_V02;
          qcril_qmi_voice_info.last_cfw_reason = VOICE_REASON_FWD_ALLCONDITIONAL_V02;
          break;

        default:
          /* Fail to add entry to ReqList */
          QCRIL_LOG_ERROR("Invalid status req");
          query_cf_req.reason = 0xFF;
          qcril_qmi_voice_info.last_cfw_reason = 0x0;
          break;
      }
      qmi_ril_leave_critical_section();

      if ( service_class <= 0 )
      {
        query_cf_req.service_class_valid = FALSE;
      }
      else
      {
        query_cf_req.service_class = service_class;
        query_cf_req.service_class_valid = TRUE;
      }

      /* Add entry to ReqList */
      qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                                   QCRIL_EVT_NONE, NULL, &reqlist_entry );
      if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
      {
        /* Fail to add entry to ReqList */
        QCRIL_LOG_ERROR("Failed to Add into Req list");
        qmi_ril_enter_critical_section();
        qcril_qmi_voice_info.last_cfw_reason = 0x00;
        qmi_ril_leave_critical_section();
        break;
      }

      user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

      query_cf_resp_ptr = qcril_malloc( sizeof(*query_cf_resp_ptr) );
      if( NULL == query_cf_resp_ptr )
      {
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
        break;
      }

      /* Send QMI VOICE DIAL CALL REQ */
        if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                              QMI_VOICE_GET_CALL_FORWARDING_REQ_V02,
                                              &query_cf_req,
                                              sizeof(query_cf_req),
                                              query_cf_resp_ptr,
                                              sizeof(*query_cf_resp_ptr),
                                              (void*)(uintptr_t)user_data) != E_SUCCESS )
      {
        /* In case of ARM9 reset, the command callback will never be executed. So, need to
         delete the entry from the ReqList, and call OnRequestComplete() */
        qcril_free( query_cf_resp_ptr );
        qmi_ril_enter_critical_section();
        qcril_qmi_voice_info.last_cfw_reason = 0x0;
        qmi_ril_leave_critical_section();
        qcril_send_empty_payload_request_response( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE );
      }
  } while ( FALSE );
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_query_call_forward_status() */


/*===========================================================================

  FUNCTION:  qcril_cm_supsvc_request_send_ussd

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SEND_USSD.
    Used to send MO USSD request or send response for MT USSD request

    Input:
    const char * containing the USSD request in UTF-8 format

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_supsvc_request_send_ussd
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  const char * rcvd_ussd_str = (const char *) params_ptr->data; /* received ussd string */
  char ussd_str[ QCRIL_QMI_VOICE_MAX_USS_CHAR ];
  int ussd_str_len = 0;
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_answer_ussd_req_msg_v02 answer_ussd_req;
  voice_answer_ussd_resp_msg_v02* answer_ussd_resp_ptr = NULL;
  voice_orig_ussd_req_msg_v02 orig_ussd_req;
  voice_orig_ussd_resp_msg_v02* orig_ussd_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
    if ( ( params_ptr->data == NULL ) || ( params_ptr->datalen == 0 ) )
    {
      QCRIL_LOG_ERROR( "received RIL_REQUEST_SEND_USSD with USSD message set to NULL" );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
    }

  QCRIL_LOG_DEBUG( "RIL_REQUEST_SEND_USSD input = Request Recived");

  memset(&orig_ussd_req,0,sizeof(orig_ussd_req));
  memset(&answer_ussd_req,0,sizeof(answer_ussd_req));


  memset( ussd_str, 0, QCRIL_QMI_VOICE_MAX_USS_CHAR );

  QCRIL_LOG_DEBUG( "USSD string in UTF8 format = %s length = %d\n", rcvd_ussd_str, strlen( rcvd_ussd_str ) );

  if ( qcril_cm_ss_UssdStringIsAscii( rcvd_ussd_str ) )
  {
    ussd_str_len = strlen( rcvd_ussd_str );
    QCRIL_LOG_DEBUG( "Recieved USSD string is in ASCII format = %s",rcvd_ussd_str );
    if ( ussd_str_len > QCRIL_QMI_VOICE_MAX_USS_CHAR )
    {
      QCRIL_LOG_DEBUG( "Received USSD charecters exceed maximum length" );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
        break;
    }
    memcpy(ussd_str,rcvd_ussd_str,ussd_str_len);
    orig_ussd_req.uss_info.uss_dcs = USS_DCS_ASCII_V02;
    answer_ussd_req.uss_info.uss_dcs = USS_DCS_ASCII_V02;
  }
  else
  {
    QCRIL_LOG_DEBUG( "Recieved USSD string is not in Ascii format = %s",rcvd_ussd_str );
    qcril_cm_ss_convert_utf8_to_ucs2( rcvd_ussd_str, ussd_str, &ussd_str_len );
    if ( ussd_str_len == 0 || ussd_str_len > QCRIL_QMI_VOICE_MAX_USS_CHAR )
    {
      QCRIL_LOG_DEBUG( "Illegal UTF8 characters received, length = %d", ussd_str_len );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
        break;
    }
    orig_ussd_req.uss_info.uss_dcs = USS_DCS_UCS2_V02;
    answer_ussd_req.uss_info.uss_dcs = USS_DCS_UCS2_V02;
  }

  orig_ussd_req.uss_info.uss_data_len = ussd_str_len;
  answer_ussd_req.uss_info.uss_data_len = ussd_str_len;
  memcpy(orig_ussd_req.uss_info.uss_data,ussd_str,ussd_str_len);
  memcpy(answer_ussd_req.uss_info.uss_data,ussd_str,ussd_str_len);

  QCRIL_LOG_DEBUG( "USSD string after conversion = %s length = %d", rcvd_ussd_str, strlen( rcvd_ussd_str ) );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add entry to ReqList */
    QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
  }

  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

  /* case where RIL is responding for MT USSD request */
  QCRIL_LOG_INFO("USSD User Action Required = %d",qcril_qmi_voice_info.ussd_user_action_required);
  if ( qcril_qmi_voice_info.ussd_user_action_required == FALSE )
  {
    /* Send QMI VOICE ORIG USSD REQ */
    orig_ussd_resp_ptr = qcril_malloc( sizeof(*orig_ussd_resp_ptr) );
    if( NULL == orig_ussd_resp_ptr )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t,
                          params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
    }

    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                           QMI_VOICE_ORIG_USSD_REQ_V02,
                                           &orig_ussd_req,
                                           sizeof(orig_ussd_req),
                                           orig_ussd_resp_ptr,
                                           sizeof(*orig_ussd_resp_ptr),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free(orig_ussd_resp_ptr);
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
    }
  }
  else /* case where RIL is initiating an MO USSD request */
  {
    /* Send QMI VOICE ANSWER USSD REQ */
    answer_ussd_resp_ptr = qcril_malloc( sizeof(*answer_ussd_resp_ptr) );
    if( NULL == answer_ussd_resp_ptr )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t,
                          params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
    }

    if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                           QMI_VOICE_ANSWER_USSD_REQ_V02,
                                           &answer_ussd_req,
                                           sizeof(answer_ussd_req),
                                           answer_ussd_resp_ptr,
                                           sizeof(*answer_ussd_resp_ptr),
                                           (void*)(uintptr_t)user_data) != E_SUCCESS )
    {
      /* In case of ARM9 reset, the command callback will never be executed. So, need to
       delete the entry from the ReqList, and call OnRequestComplete() */
      qcril_free( answer_ussd_resp_ptr );
      qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_qmi_voice_info.ussd_user_action_required = FALSE;
      qcril_send_request_response( &resp );
    }
  }
  }while(0);

} /* qcril_qmi_voice_supsvc_request_send_ussd() */

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_supsvc_request_cancel_ussd

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CANCEL_USSD.
    Used for cancelling/releasing the ongoing USSD session

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_supsvc_request_cancel_ussd
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;
  uint32 user_data;
  voice_cancel_ussd_resp_msg_v02* cancel_ussd_resp_ptr = NULL;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
  QCRIL_LOG_DEBUG( "RIL_REQUEST_CANCEL_USSD input = Request Recived");


  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, QCRIL_DEFAULT_MODEM_ID, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add entry to ReqList */
    QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
  }

  user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

  cancel_ussd_resp_ptr = qcril_malloc( sizeof(*cancel_ussd_resp_ptr) );
  if( NULL == cancel_ussd_resp_ptr )
  {
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
    break;
  }

/* Send QMI VOICE CANCEL USSD REQ */
  if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                        QMI_VOICE_CANCEL_USSD_REQ_V02,
                                        NULL,
                                        0,
                                        cancel_ussd_resp_ptr,
                                        sizeof(*cancel_ussd_resp_ptr),
                                        (void*)(uintptr_t)user_data) != E_SUCCESS )
  {
    /* In case of ARM9 reset, the command callback will never be executed. So, need to
     delete the entry from the ReqList, and call OnRequestComplete() */
    qcril_free( cancel_ussd_resp_ptr );
    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
    qcril_send_request_response( &resp );
  }
  }while(0);

} /* qcril_qmi_voice_supsvc_request_cancel_ussd() */

//===========================================================================
// ****************************** STK CC ***********************************
//===========================================================================

//===========================================================================
// qcril_qmi_voice_is_stk_cc_in_progress
//===========================================================================
int qcril_qmi_voice_is_stk_cc_in_progress(void)
{
  return ( QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE == stk_cc_info.modification ) ? FALSE : TRUE;
} // qcril_qmi_voice_is_stk_cc_in_progress

//===========================================================================
// qcril_qmi_voice_is_stk_cc_in_progress
//===========================================================================
void qcril_qmi_voice_reset_stk_cc(void)
{
  stk_cc_info.modification = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE;
  stk_cc_info.is_alpha_relayed = FALSE;
  stk_cc_info.call_id_info = QCRIL_QMI_VOICE_UNKNOWN_ACTIVE_CONN_ID;
} // qcril_qmi_voice_reset_stk_cc

//===========================================================================
// qcril_qmi_voice_stk_cc_relay_alpha_if_necessary
//===========================================================================
void qcril_qmi_voice_stk_cc_relay_alpha_if_necessary(qcril_instance_id_e_type instance_id, boolean send_unsol_unconditional)
{
  char buf_str[QMI_VOICE_ALPHA_TEXT_MAX_V02 + 2];
  qcril_unsol_resp_params_type unsol_resp;

  memset(buf_str, 0 , QMI_VOICE_ALPHA_TEXT_MAX_V02 + 2);

  QCRIL_LOG_DEBUG("instance_id = %d, send unsol unconditional = %d", instance_id, send_unsol_unconditional);

  if ( ( qcril_qmi_voice_is_stk_cc_in_progress() && !stk_cc_info.is_alpha_relayed ) ||
       ( send_unsol_unconditional ) )
  {
    if ( stk_cc_info.alpha_ident.alpha_text_len > 0 )
    {
      switch ( stk_cc_info.alpha_ident.alpha_dcs )
      {
        case ALPHA_DCS_GSM_V02:
           if(stk_cc_info.alpha_ident.alpha_text_len < QMI_VOICE_ALPHA_TEXT_MAX_V02)
               qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8( (char*) stk_cc_info.alpha_ident.alpha_text,
                                                                stk_cc_info.alpha_ident.alpha_text_len,
                                                                buf_str );
          break;

        case ALPHA_DCS_UCS2_V02:
          qcril_cm_ss_convert_ucs2_to_utf8( (char *) stk_cc_info.alpha_ident.alpha_text,
                                                            stk_cc_info.alpha_ident.alpha_text_len * 2, buf_str );
          break;

        default:
          buf_str[0] = 0;
          break;
      }

      if ( *buf_str )
      {
        qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_STK_CC_ALPHA_NOTIFY, &unsol_resp );
        unsol_resp.resp_pkt    = (void*)buf_str;
        unsol_resp.resp_len    = sizeof( char* );

        qcril_send_unsol_response( &unsol_resp );
      }

      // RIL_UNSOL_STK_CC_ALPHA_NOTIFY
      stk_cc_info.is_alpha_relayed = TRUE;
    }
  }
} // qcril_qmi_voice_stk_cc_relay_alpha_if_necessary

//===========================================================================
// qcril_qmi_voice_stk_cc_convert_qmi_svc_class_to_ril_teleservice
//===========================================================================
RIL_SsTeleserviceType qcril_qmi_voice_stk_cc_convert_qmi_svc_class_to_ril_teleservice( uint8_t service_class )
{
  RIL_SsTeleserviceType ss_teleservice_type;

  switch ( service_class )
  {
    case QCRIL_QMI_VOICE_TELESEFRVICE_ALL:
      ss_teleservice_type = SS_ALL_TELESEVICES;
      break;

    case QCRIL_QMI_VOICE_TELESEFRVICE_TELEPHONY:
      ss_teleservice_type = SS_TELEPHONY;
      break;

    case QCRIL_QMI_VOICE_TELESEFRVICE_ALL_DATA:
      ss_teleservice_type = SS_ALL_DATA_TELESERVICES;
      break;

    case QCRIL_QMI_VOICE_TELESEFRVICE_SMS:
      ss_teleservice_type = SS_SMS_SERVICES;
      break;

    case QCRIL_QMI_VOICE_TELESEFRVICE_ALL_EXCEPT_SMS:
      ss_teleservice_type = SS_ALL_TELESERVICES_EXCEPT_SMS;
      break;

    case QCRIL_QMI_VOICE_TELESEFRVICE_ALL_BEARER_SVC:
       ss_teleservice_type = SS_ALL_TELE_AND_BEARER_SERVICES;
       break;

    default: // something that we do not support
      ss_teleservice_type = SS_ALL_TELE_AND_BEARER_SERVICES;
      break;
  }

  return ss_teleservice_type;
} // qcril_qmi_voice_stk_cc_convert_qmi_svc_class_to_ril_teleservice

//===========================================================================
// qcril_qmi_voice_stk_cc_handle_voice_sups_ind
//===========================================================================
void qcril_qmi_voice_stk_cc_handle_voice_sups_ind(voice_sups_ind_msg_v02* sups_ind_msg)
{
  RIL_StkCcUnsolSsResponse unsol_ss_response;
  RIL_Errno ril_err;

  qcril_instance_id_e_type instance_id;

  char ussd_utf8_str [QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2];
  char type_code [2];
  char *response_buff [2];
  int  utf8_len = 0;

  uint8 uss_dcs   = QCRIL_QMI_VOICE_USSD_DCS_UNSPECIFIED;
  boolean success = TRUE;

  qcril_unsol_resp_params_type unsol_resp;

  RIL_SsServiceType              ss_service_type;
  int                            ss_service_class;
  RIL_SsRequestType              ss_request_type;
  int                            ss_request_type_available;
  int                            ss_service_type_available;
  int                            ss_service_class_available;

  RIL_StkCcUnsolSsResponse       ril_ss_response;

  int                            idx,i=0;
  int                            ril_cf_reason;
  int                            is_stk_cc_in_progress = qcril_qmi_voice_is_stk_cc_in_progress();
  voice_cc_sups_result_service_type_enum_v02 service_type = stk_cc_info.ss_ussd_info.service_type;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_stk_cc_dump();

  if( sups_ind_msg != NULL )
  {
     QCRIL_LOG_INFO( "..sups_ind call_id %d, %d", (int) sups_ind_msg->call_id_valid, (int) sups_ind_msg->call_id );
     QCRIL_LOG_INFO( "..sups_ind data_source %d, %d", (int) sups_ind_msg->data_source_valid, (int) sups_ind_msg->data_source );

     QCRIL_LOG_INFO( ".. service type, cc_mod %d, %d",
                     (int) sups_ind_msg->supplementary_service_info.service_type,
                     (int) sups_ind_msg->supplementary_service_info.is_modified_by_call_control );

     if ( ( (!is_stk_cc_in_progress && sups_ind_msg->supplementary_service_info.service_type == SERVICE_TYPE_USSD_V02 && qcril_qmi_voice_info.process_sups_ind)// for proactive cases
            || (is_stk_cc_in_progress
               && ( (sups_ind_msg->call_id_valid && sups_ind_msg->call_id == stk_cc_info.call_id_info)// not to confuse with any other SS or USSD activities
                  || (sups_ind_msg->supplementary_service_info.is_modified_by_call_control)
                  )
               )
          ) &&
          sups_ind_msg->data_source_valid && sups_ind_msg->data_source == VOICE_SUPS_DATA_SOURCE_NETWORK_V02 // response from network
        )
     {
       if ( !is_stk_cc_in_progress ) // for proactive cases, we should just use the service type in the qmi message
       {
         service_type = sups_ind_msg->supplementary_service_info.service_type;
       }

       QCRIL_LOG_INFO( "..sups_ind service_type %d", (int) service_type);

       if ( VOICE_CC_SUPS_RESULT_SERVICE_TYPE_USSD_V02 == service_type )
       { //  -- USSD case -- start
         QCRIL_LOG_INFO( "..sups_ind uusd uss valid %d", (int) sups_ind_msg->uss_info_valid );

         memset( ussd_utf8_str, '\0', sizeof( ussd_utf8_str ) );
         memset( type_code, '\0', sizeof( type_code ) );
         memset( response_buff, 0, sizeof( response_buff ) );

         success = FALSE;

         if ( ( sups_ind_msg->uss_info_valid || sups_ind_msg->uss_info_utf16_valid ) &&
              !( sups_ind_msg->alpha_ident_valid &&
              ( QCRIL_QMI_VOICE_ALPHA_LENGTH_IN_NULL_CASE == sups_ind_msg->alpha_ident.alpha_text_len ) )
            )
         {
            if ( sups_ind_msg->uss_info_utf16_valid ) // using uss_info_utf16 instead of uss_info if it is available
            {

              utf8_len = qcril_cm_ss_convert_ucs2_to_utf8( (char *) sups_ind_msg->uss_info_utf16, sups_ind_msg->uss_info_utf16_len * 2, ussd_utf8_str );
              if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
              {
                QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR" );
                utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
                ussd_utf8_str[ utf8_len - 1] = '\0';
              }
              success = TRUE;
            }
            else
            {
              QCRIL_LOG_INFO( "..sups_ind uss_dcs %d", (int) sups_ind_msg->uss_info.uss_dcs );

              switch (sups_ind_msg->uss_info.uss_dcs)
              {
                 case USS_DCS_ASCII_V02 :
                   utf8_len = qcril_cm_ss_ascii_to_utf8((unsigned char *)sups_ind_msg->uss_info.uss_data, sups_ind_msg->uss_info.uss_data_len,
                                                        ussd_utf8_str, sizeof(ussd_utf8_str));
                   success = TRUE;
                   break;

                 case USS_DCS_8BIT_V02 :
                   uss_dcs = QCRIL_QMI_VOICE_USSD_DCS_8_BIT;
                   utf8_len = qcril_cm_ss_convert_ussd_string_to_utf8( uss_dcs,
                                                                       sups_ind_msg->uss_info.uss_data_len,
                                                                       sups_ind_msg->uss_info.uss_data,
                                                                       ussd_utf8_str );
                   if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
                   {
                     QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR" );
                     utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
                     ussd_utf8_str[ utf8_len - 1] = '\0';
                   }

                   success = TRUE;
                   break;

                 case USS_DCS_UCS2_V02 :
                   uss_dcs = QCRIL_QMI_VOICE_USSD_DCS_UCS2;
                   utf8_len = qcril_cm_ss_convert_ussd_string_to_utf8( uss_dcs,
                                                                       sups_ind_msg->uss_info.uss_data_len,
                                                                       sups_ind_msg->uss_info.uss_data,
                                                                       ussd_utf8_str );
                   if ( utf8_len > ( QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR * 2 ) )
                   {
                     QCRIL_LOG_ERROR ("ascii_len exceeds QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR" );
                     utf8_len = (int) (QCRIL_QMI_VOICE_MAX_MT_USSD_CHAR*2);
                     ussd_utf8_str[ utf8_len - 1] = '\0';
                   }

                   success = TRUE;
                   break;

                 default :
                   QCRIL_LOG_ERROR ("Invalid USSD dcs : %d", sups_ind_msg->uss_info.uss_dcs );
                   break;
              }
            }

            if ( success )
            {
               type_code[ 0 ] = '0';  // QCRIL_CM_SS_CUSD_RESULT_DONE
               type_code[ 1 ] = '\0';
               response_buff[ 0 ] = type_code;

               if ( sups_ind_msg->uss_info.uss_data_len > 0)
               {
                 response_buff[ 1 ] = ussd_utf8_str;
               }
               else
               {
                 response_buff[ 1 ] = NULL;
               }

               // Sending the response received from the network for the USSD request
               QCRIL_LOG_DEBUG ("USSD Conf Success, data_len : %d", sups_ind_msg->uss_info.uss_data_len );
               QCRIL_LOG_DEBUG ("USSD : type_code=%s",type_code);
               if ( NULL != response_buff[ 1 ] )
               {
                 QCRIL_LOG_DEBUG ("USSD : response_buff[1]=%s",response_buff[ 1 ]);
                 QCRIL_LOG_DEBUG ("USSD : strlen=%d",strlen(response_buff[ 1 ]));
               }
               else
               {
                 QCRIL_LOG_DEBUG ("USSD : response_buff[1] is NULL");
               }
            }
         }

         if ( !success )
         {
            // sending the unsol indication so that RIL can close the USSD session
            if ( (sups_ind_msg->failure_cause_valid == TRUE) &&
                 (sups_ind_msg->failure_cause == QMI_FAILURE_CAUSE_FACILITY_NOT_SUPPORTED_V02) ) // facilityNotSupported
            {
               type_code[ 0 ] = '4';  // QCRIL_CM_SS_CUSD_RESULT_NOSUP
               type_code[ 1 ] = '\0';
            }
            else
            {
               type_code[ 0 ] = '2';  // QCRIL_CM_SS_CUSD_RESULT_ABORT
               type_code[ 1 ] = '\0';
            }
            response_buff[ 0 ] = type_code;
            response_buff[ 1 ] = NULL;
            QCRIL_LOG_DEBUG ("STK CC USSD Failure: type_code=%s",type_code);
         }

         QCRIL_LOG_INFO( ".. sending RIL_UNSOL_ON_USSD" );
         qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_ON_USSD, &unsol_resp );
         unsol_resp.resp_pkt = (void *) response_buff;
         unsol_resp.resp_len = sizeof( response_buff );
         qcril_send_unsol_response( &unsol_resp );
         QCRIL_LOG_INFO( ".. sending RIL_UNSOL_ON_USSD done" );

         // -- USSD case - end -
       }
       else
       {  // -- SS case -- start
         if( !( sups_ind_msg->alpha_ident_valid &&
            ( QCRIL_QMI_VOICE_ALPHA_LENGTH_IN_NULL_CASE == sups_ind_msg->alpha_ident.alpha_text_len ) )
           )
         {
           QCRIL_LOG_INFO( ".. ss case" );
           QCRIL_LOG_INFO( ".. ss reason %d, %d", (int) sups_ind_msg->reason_valid, (int) sups_ind_msg->reason );
           QCRIL_LOG_INFO( ".. ss service class %d, %d", (int) sups_ind_msg->service_class_valid, (int) sups_ind_msg->service_class );

           // service type
           ss_request_type_available = TRUE;
           switch (sups_ind_msg->supplementary_service_info.service_type)
           {
            case VOICE_CC_SUPS_RESULT_SERVICE_TYPE_ACTIVATE_V02:
              ss_request_type = SS_ACTIVATION;
              break;

            case VOICE_CC_SUPS_RESULT_SERVICE_TYPE_DEACTIVATE_V02:
              ss_request_type = SS_DEACTIVATION;
              break;

            case VOICE_CC_SUPS_RESULT_SERVICE_TYPE_REGISTER_V02:
              ss_request_type = SS_REGISTRATION;
              break;

            case VOICE_CC_SUPS_RESULT_SERVICE_TYPE_ERASE_V02:
              ss_request_type = SS_ERASURE;
              break;

            case VOICE_CC_SUPS_RESULT_SERVICE_TYPE_INTERROGATE_V02:
              ss_request_type = SS_INTERROGATION;
              break;

            // qmi_todo: why VOICE_CC_SUPS_RESULT_SERVICE_TYPE_REGISTER_PASSWORD_V02 ??

            default:
              ss_request_type_available = FALSE;
              ss_request_type = SS_ERASURE; // some default value as we may refer to this value for logging
              break;
           }
           QCRIL_LOG_INFO( ".. ss_request_type %d, %d", (int) ss_request_type_available, (int) ss_request_type );

           // request type
           if ( sups_ind_msg->reason_valid )
           {
            ss_service_type_available = TRUE;
            switch ( sups_ind_msg->reason )
            {
                case VOICE_SUPS_IND_REASON_FWD_UNCONDITIONAL_V02:
                  ss_service_type = SS_CFU;
                  break;

                case VOICE_SUPS_IND_REASON_FWD_MOBILEBUSY_V02:
                  ss_service_type = SS_CF_BUSY;
                  break;

                case VOICE_SUPS_IND_REASON_FWD_NOREPLY_V02:
                  ss_service_type = SS_CF_NO_REPLY;
                  break;

                case VOICE_SUPS_IND_REASON_FWD_UNREACHABLE_V02:
                  ss_service_type = SS_CF_NOT_REACHABLE;
                  break;

                case VOICE_SUPS_IND_REASON_FWD_ALLFORWARDING_V02:
                  ss_service_type = SS_CF_ALL;
                  break;

                case VOICE_SUPS_IND_REASON_FWD_ALLCONDITIONAL_V02:
                  ss_service_type = SS_CF_ALL_CONDITIONAL;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_ALLOUTGOING_V02:
                  ss_service_type = SS_BAOC;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_OUTGOINGINT_V02:
                  ss_service_type = SS_BAOIC;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_OUTGOINGINTEXTOHOME_V02:
                  ss_service_type = SS_BAOIC_EXC_HOME;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_ALLINCOMING_V02:
                  ss_service_type = SS_BAIC;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_INCOMINGROAMING_V02:
                  ss_service_type = SS_BAIC_ROAMING;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_ALLBARRING_V02:
                  ss_service_type = SS_ALL_BARRING;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_ALLOUTGOINGBARRING_V02:
                  ss_service_type = SS_OUTGOING_BARRING;
                  break;

                case VOICE_SUPS_IND_REASON_BARR_ALLINCOMINGBARRING_V02:
                  ss_service_type = SS_INCOMING_BARRING;
                  break;

                case VOICE_SUPS_IND_REASON_CALLWAITING_V02:
                  ss_service_type = SS_WAIT;
                  break;

                case VOICE_SUPS_IND_REASON_CLIP_V02:
                  ss_service_type = SS_CLIP;
                  break;

                case VOICE_SUPS_IND_REASON_CLIR_V02:
                  ss_service_type = SS_CLIR;
                  break;

                // qmi_todo: missing COLP and COLR in QMI Voice

                default:
                  ss_service_type_available = FALSE;
                  ss_service_type = SS_CFU; // some default value as we may refer to this value for logging
                  break;
            }
          }
          else
          {
            ss_service_type_available = FALSE;
            ss_service_type = SS_CFU; // some default value as we may refer to this value for logging
          }
          QCRIL_LOG_INFO( ".. ss_service_type %d, %d", (int) ss_service_type_available, (int) ss_service_type );

          // teleservice type
          ss_service_class_available = TRUE;
          if ( sups_ind_msg->service_class_valid )
          {
            ss_service_class_available = TRUE;
            ss_service_class = sups_ind_msg->service_class;
          }
          else
          {
            ss_service_class_available = FALSE;
            ss_service_class           = 0; // per ril.h - 0 means user doesn't input class
          }
          QCRIL_LOG_INFO( ".. ss_service_class %d, %d", (int) ss_service_class_available, (int) ss_service_class );

          // build response
          success = FALSE;
          memset( &ril_ss_response, 0, sizeof( ril_ss_response ) );
          ril_ss_response.serviceType = ss_service_type;
          ril_ss_response.requestType = ss_request_type;
          ril_ss_response.serviceClass = ss_service_class;

          if ( ss_request_type_available )
          {
            switch ( ss_request_type )
            {
             case SS_ACTIVATION:
             case SS_DEACTIVATION:
             case SS_REGISTRATION:
             case SS_ERASURE:
               // -- ALL SS request types except INTERROGATION -- start
               // -- ALL SS request types except INTERROGATION -- end
               if ( ss_service_type_available )
               {
                 switch ( ss_service_type )
                 {
                   case SS_CFU:
                   case SS_CF_BUSY:
                   case SS_CF_NO_REPLY:
                   case SS_CF_NOT_REACHABLE:
                   case SS_CF_ALL:
                   case SS_CF_ALL_CONDITIONAL:

                   case SS_CLIP:
                   case SS_CLIR:
                   case SS_COLP:
                   case SS_COLR:
                   case SS_WAIT:
                   case SS_BAOC:
                   case SS_BAOIC:
                   case SS_BAOIC_EXC_HOME:
                   case SS_BAIC:
                   case SS_BAIC_ROAMING:
                   case SS_ALL_BARRING:
                   case SS_OUTGOING_BARRING:
                   case SS_INCOMING_BARRING:
                     break;

                   default:
                     // failure case
                     break;
                 }

               }
               else
               {  // failure case
               }
               break;

             case SS_INTERROGATION:
               // -- INTERROGATION -- start
               if ( ss_service_type_available )
               {
                 switch ( ss_service_type )
                 {
                   case SS_CLIP:
                     if ( sups_ind_msg->clip_status_valid )
                     {
                       success = TRUE;
                       ril_ss_response.ssInfo[0] = sups_ind_msg->clip_status.provision_status;
                       if ( (sups_ind_msg->clip_status.active_status == 0) && (sups_ind_msg->clip_status.provision_status == 0))
                       {
                         ril_ss_response.ssInfo[0] = 2;
                       }
                     }
                     QCRIL_LOG_INFO( ".. ss_clip %d, %d", (int) success, (int) ril_ss_response.ssInfo[0] );
                     break;

                   case SS_CLIR:
                     if ( sups_ind_msg->clir_status_valid )
                     {
                       if ( sups_ind_msg->failure_cause_valid )
                       {
                         ril_ss_response.ssInfo[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_NO_NETWORK;
                         ril_ss_response.ssInfo[0] = QCRIL_QMI_VOICE_SS_CLIR_PRESENTATION_INDICATOR;
                       }
                       else
                       {
                         {
                           ril_ss_response.ssInfo[0] = qcril_qmi_voice_info.clir;
                           switch( sups_ind_msg->clir_status.provision_status)
                           {
                             case PROVISION_STATUS_NOT_PROVISIONED_V02 :
                               ril_ss_response.ssInfo[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_NOT_PROVISIONED;
                               success = TRUE;
                               break;

                             case PROVISION_STATUS_PRESENTATION_ALLOWED_V02 :
                               ril_ss_response.ssInfo[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_ALLOWED;
                               success = TRUE;
                               break;

                             case PROVISION_STATUS_PROVISIONED_PERMANENT_V02 :
                               ril_ss_response.ssInfo[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_PROVISIONED_PERMANENT;
                               success = TRUE;
                               break;

                             case PROVISION_STATUS_PRESENTATION_RESTRICTED_V02:
                               ril_ss_response.ssInfo[1] = (int)QCRIL_QMI_VOICE_CLIR_SRV_PRESENTATION_RESTRICTED;
                               success = TRUE;
                               break;

                             default :
                               success = FALSE;
                               break;
                           }
                         }
                       }
                     }
                     break;

                   case SS_COLP:
                   case SS_COLR:
                     // qmi_todo: intor support once QMI Voice supports this
                     break;

                   case SS_WAIT:
                     if (sups_ind_msg->service_class_valid )
                     {
                       if (sups_ind_msg->service_class == 0x00) // we love magic numbers
                       {
                         ril_ss_response.ssInfo[0] = FALSE;
                       }
                       else
                       {
                         ril_ss_response.ssInfo[0] = TRUE;
                       }
                       ril_ss_response.ssInfo[1] = sups_ind_msg->service_class;
                     }
                     else
                     {
                       success = FALSE;
                     }
                     break;

                   case SS_BAOC:
                   case SS_BAOIC:
                   case SS_BAOIC_EXC_HOME:
                   case SS_BAIC:
                   case SS_BAIC_ROAMING:
                   case SS_ALL_BARRING:
                   case SS_OUTGOING_BARRING:
                   case SS_INCOMING_BARRING:
                     if (sups_ind_msg->service_class == 0x00) // magic numbers are inevitable
                     {
                       ril_ss_response.ssInfo[0] = sups_ind_msg->service_class;
                     }
                     break;

                   case SS_CFU:
                   case SS_CF_BUSY:
                   case SS_CF_NO_REPLY:
                   case SS_CF_NOT_REACHABLE:
                   case SS_CF_ALL:
                   case SS_CF_ALL_CONDITIONAL:
                     default:
                     // Call Forward cases
                     if ( sups_ind_msg->call_forwarding_info_valid &&
                          sups_ind_msg->call_forwarding_info_len > 0 &&
                          sups_ind_msg->call_forwarding_info_len < NUM_SERVICE_CLASSES)
                     {
                       success = TRUE;
                       ril_cf_reason = qcril_qmi_voice_map_qmi_cfw_reason_to_ril_reason( sups_ind_msg->reason ); // this must be valid otherwsie we would not get to this point
                       for ( idx = 0; idx < (int)sups_ind_msg->call_forwarding_info_len; idx++ )
                       {
                         ril_ss_response.cfData.cfInfo[idx].status =
                             (sups_ind_msg->call_forwarding_info[idx].service_status == QCRIL_QMI_VOICE_SERVICE_STATUS_ACTIVE) ?
                               QCRIL_QMI_VOICE_RIL_CF_STATUS_ACTIVE : QCRIL_QMI_VOICE_RIL_CF_STATUS_NOT_ACTIVE;

                         ril_ss_response.cfData.cfInfo[idx].reason = ril_cf_reason;

                         ril_ss_response.cfData.cfInfo[idx].serviceClass = sups_ind_msg->call_forwarding_info[idx].service_class;

                         ril_ss_response.cfData.cfInfo[idx].number = sups_ind_msg->call_forwarding_info[idx].number; // 1st is char*, 2nd is char[]

                         ril_ss_response.cfData.cfInfo[idx].toa =
                           (QCRIl_QMI_VOICE_SS_TA_INTER_PREFIX == *sups_ind_msg->call_forwarding_info[idx].number) ?
                             QCRIL_QMI_VOICE_SS_TA_INTERNATIONAL : QCRIL_QMI_VOICE_SS_TA_UNKNOWN;

                         ril_ss_response.cfData.cfInfo[idx].timeSeconds = sups_ind_msg->call_forwarding_info[idx].no_reply_timer;
                       }
                       ril_ss_response.cfData.numValidIndexes = (int)sups_ind_msg->call_forwarding_info_len;
                     }
                     break;
                 }
               }
               else
               {  // just unfortunate - we cannot handle it

               }
               // -- INTERROGATION - end
               break;

               default:
                 break;
             }
           }
           else
           {  // unknown request type
             // qmi_todo: handle
           }
         }
           qcril_default_unsol_resp_params( QCRIL_DEFAULT_INSTANCE_ID, (int) RIL_UNSOL_ON_SS, &unsol_resp );
           unsol_resp.resp_pkt = (void *) &ril_ss_response;
           unsol_resp.resp_len = sizeof( ril_ss_response );
           qcril_send_unsol_response( &unsol_resp );
           // -- SS case -- end
        }
     }

     if ( ( ( sups_ind_msg->data_source_valid  && sups_ind_msg->data_source == VOICE_SUPS_DATA_SOURCE_MS_V02) || !sups_ind_msg->data_source_valid ) &&
          ( sups_ind_msg->alpha_ident_valid ) )
     {
        /* this should cover the following scenarios.
            1) User initiated requests modified by STK CC.
            2) STK initiated requests modified by STK CC. */

        if ( ALPHA_DCS_UCS2_V02 == sups_ind_msg->alpha_ident.alpha_dcs )
        {
          qcril_qmi_voice_transfer_sim_ucs2_alpha_to_std_ucs2_alpha(&sups_ind_msg->alpha_ident, &stk_cc_info.alpha_ident);
        }
        else
        {
          stk_cc_info.alpha_ident = sups_ind_msg->alpha_ident;
        }

        qcril_qmi_voice_stk_cc_relay_alpha_if_necessary(QCRIL_DEFAULT_INSTANCE_ID, TRUE);
     }
  }
} // qcril_qmi_voice_stk_cc_handle_voice_sups_ind

//===========================================================================
// qcril_qmi_voice_transfer_sim_alpha_to_std_alpha
//===========================================================================
void qcril_qmi_voice_transfer_sim_ucs2_alpha_to_std_ucs2_alpha(const voice_alpha_ident_type_v02 *sim_alpha, voice_alpha_ident_type_v02 *std_alpha)
{
  uint8 idx;
  uint8 coding_scheme;
  uint8 num_of_char;
  uint16 base_val;
  uint16 ucs2_val;

  QCRIL_LOG_FUNC_ENTRY();

  memset(std_alpha, 0, sizeof(*std_alpha));

  do
  {
    if ( sim_alpha->alpha_text_len > 0)
    {
      coding_scheme = sim_alpha->alpha_text[0];
    }
    else
    {
      QCRIL_LOG_ERROR("alpha_text_len is 0");
      break;
    }
    QCRIL_LOG_INFO("coding scheme %x", (int)coding_scheme);

    idx = 1;
    switch (coding_scheme)
    {
      case 0x80:
        while ( (uint32)(idx+1) < sim_alpha->alpha_text_len )
        {
          std_alpha->alpha_text[idx-1] = sim_alpha->alpha_text[idx+1];
          std_alpha->alpha_text[idx] = sim_alpha->alpha_text[idx];
          idx += 2;
        }

        if ( (uint32)(idx+1) == sim_alpha->alpha_text_len && 0 != sim_alpha->alpha_text[idx] )
        {
          QCRIL_LOG_ERROR("an unexpected extra non-zero byte in source alpha buffer");
        }

        std_alpha->alpha_dcs = ALPHA_DCS_UCS2_V02;
        std_alpha->alpha_text_len = idx-1;
        break;

      case 0x81:
        if ( sim_alpha->alpha_text_len < 3 )
        {
          QCRIL_LOG_ERROR("sim_alpha->alpha_text_len (%d) less than 3", sim_alpha->alpha_text_len);
        }
        else
        {
          num_of_char = sim_alpha->alpha_text[1];
          base_val = sim_alpha->alpha_text[2];
          base_val <<= 7;
          idx = 3;

          if ( idx + num_of_char > sim_alpha->alpha_text_len )
          {
            QCRIL_LOG_DEBUG("num_of_char > sim_alpha->alpha_text_len - 3");
            num_of_char = sim_alpha->alpha_text_len - idx;
          }

          if (num_of_char * 2 > QMI_VOICE_ALPHA_TEXT_MAX_V02)
          {
            QCRIL_LOG_DEBUG("num_of_char * 2 > QMI_VOICE_ALPHA_TEXT_MAX_V02");
            num_of_char = QMI_VOICE_ALPHA_TEXT_MAX_V02 / 2;
          }

          int i;
          for ( i = 0; i< num_of_char && i < (QMI_VOICE_ALPHA_TEXT_MAX_V02 / 2); i++, idx++ )
          {
            ucs2_val = sim_alpha->alpha_text[idx];

            if ( ucs2_val >= 0x80 )
            {
              ucs2_val &= 0x7F;
              ucs2_val |= base_val;
            }

            std_alpha->alpha_text[2*i]   = (uint8) (ucs2_val);
            std_alpha->alpha_text[2*i+1] = (uint8) (ucs2_val >> 8);
          }
          std_alpha->alpha_dcs = ALPHA_DCS_UCS2_V02;
          std_alpha->alpha_text_len = num_of_char * 2;
        }

        break;

      case 0x82:
        if ( sim_alpha->alpha_text_len < 4 )
        {
          QCRIL_LOG_DEBUG("sim_alpha->alpha_text_len (%d) less than 4", sim_alpha->alpha_text_len);
        }
        else
        {
          num_of_char = sim_alpha->alpha_text[1];
          base_val = sim_alpha->alpha_text[2];
          base_val <<= 8;
          base_val += sim_alpha->alpha_text[3];
          idx = 4;

          if ( idx + num_of_char > sim_alpha->alpha_text_len )
          {
            QCRIL_LOG_DEBUG("num_of_char > sim_alpha->alpha_text_len - 4");
            num_of_char = sim_alpha->alpha_text_len - idx;
          }

          if (num_of_char * 2 > QMI_VOICE_ALPHA_TEXT_MAX_V02)
          {
            QCRIL_LOG_DEBUG("num_of_char * 2 > QMI_VOICE_ALPHA_TEXT_MAX_V02");
            num_of_char = QMI_VOICE_ALPHA_TEXT_MAX_V02 / 2;
          }

          int i;
          for ( i = 0; i< num_of_char && i < (QMI_VOICE_ALPHA_TEXT_MAX_V02 / 2); i++, idx++ )
          {
            ucs2_val = sim_alpha->alpha_text[idx];

            if ( ucs2_val >= 0x80 )
            {
              ucs2_val &= 0x7F;
              ucs2_val += base_val;
            }

            std_alpha->alpha_text[2*i]   = (uint8) (ucs2_val);
            std_alpha->alpha_text[2*i+1] = (uint8) (ucs2_val >> 8);
          }
          std_alpha->alpha_dcs = ALPHA_DCS_UCS2_V02;
          std_alpha->alpha_text_len = num_of_char * 2;
        }

        break;

      default:
        QCRIL_LOG_ERROR("unknown SIM coding scheme");
    }

  } while ( FALSE );

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_transfer_sim_alpha_to_std_alpha

//===========================================================================
// qcril_qmi_voice_stk_cc_dump
//===========================================================================
void qcril_qmi_voice_stk_cc_dump(void)
{
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_INFO( "... modification %d, call_id %d ", (int) stk_cc_info.modification, (int) stk_cc_info.call_id_info );
  QCRIL_LOG_INFO( "... ss svc type %d, ss reason %d ", (int) stk_cc_info.ss_ussd_info.service_type, (int) stk_cc_info.ss_ussd_info.reason );
  QCRIL_LOG_INFO( "... is_alpha_relayed %d ", (int) stk_cc_info.is_alpha_relayed );
} // qcril_qmi_voice_stk_cc_dump

//===========================================================================
// qcril_qmi_voice_stk_ss_resp_handle
//===========================================================================
int qcril_qmi_voice_stk_ss_resp_handle( const qcril_request_params_type *const params_ptr,
                                        qcril_instance_id_e_type instance_id,
                                        qmi_response_type_v01* resp,
                                        void* extra_info,
                                        uint8_t alpha_ident_valid,
                                        voice_alpha_ident_type_v02* alpha_ident,
                                        uint8_t call_id_valid,
                                        uint8_t call_id,
                                        uint8_t cc_sups_result_valid,
                                        voice_cc_sups_result_type_v02* cc_sups_result,
                                        uint8_t cc_result_type_valid,
                                        voice_cc_result_type_enum_v02* cc_result_type)
{
  int res = FALSE;
  RIL_Errno ril_err;

  qcril_qmi_voice_stk_cc_modification_e_type stk_cc_modification;

  qcril_request_resp_params_type ril_response;

  qmi_ril_err_ctx_ss_resp_data_type ss_op_info;

  QCRIL_LOG_INFO( "cidv %d, ccrv %d, ccr %p", (int)call_id_valid, (int)cc_sups_result_valid, cc_sups_result );
  QCRIL_NOTUSED(extra_info);

  memset( &ss_op_info, 0, sizeof( ss_op_info ));
  ss_op_info.cc_sups_result_valid = cc_sups_result_valid;
  ss_op_info.cc_sups_result       = cc_sups_result;
  ss_op_info.cc_result_type_valid = cc_result_type_valid;
  ss_op_info.cc_result_type       = cc_result_type;

  ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result_ex( QMI_NO_ERR,
                                                                        resp,
                                                                        QCRIL_QMI_ERR_CTX_SEND_SS_TXN,
                                                                        (void*)&ss_op_info ); // we disregard extra_info param now
  QCRIL_LOG_INFO("STK SS RESP : ril_err=%d, result=%d, error=%d", ril_err,resp->result, resp->error);


  switch ( (int)ril_err )
  {
    case RIL_E_SS_MODIFIED_TO_DIAL:
      stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_SS_TO_DIAL;
      break;

    case RIL_E_SS_MODIFIED_TO_USSD:
      stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_SS_TO_USSD;
      break;

    case RIL_E_SS_MODIFIED_TO_SS:
      stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_SS_TO_SS;
      break;

    default:
      stk_cc_modification  = QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE;
      break;
  }

  if ( QCRIL_QMI_VOICE_STK_CC_MODIFICATION_NONE != stk_cc_modification )
  { // STK CC session started
    qcril_qmi_voice_reset_stk_cc();

    stk_cc_info.modification                      = stk_cc_modification;
    stk_cc_info.is_alpha_relayed                  = FALSE;

    if ( call_id_valid )
    {
      stk_cc_info.call_id_info = call_id;
    }

    if ( cc_sups_result_valid )
    {
      stk_cc_info.ss_ussd_info = *cc_sups_result;
    }

    if ( alpha_ident_valid )
    {
      stk_cc_info.alpha_ident = *alpha_ident;
    }
    else
    {
      memset( &stk_cc_info.alpha_ident, 0, sizeof( stk_cc_info.alpha_ident ) );
    }

    res = TRUE;

    QCRIL_LOG_INFO( "org req altered ril_err %d, call_id %d ", (int)ril_err, (int)call_id );

    qcril_qmi_voice_stk_cc_dump();

    qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_err, &ril_response );
    qcril_send_request_response( &ril_response );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;
} // qcril_qmi_voice_stk_ss_resp_handle

//===========================================================================
// qmi_ril_voice_is_under_any_emergency_calls
//===========================================================================
int qmi_ril_voice_is_under_any_emergency_calls(void)
{
  int                                         res;
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  res = FALSE;

  qcril_qmi_voice_voip_lock_overview();

  call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
  while ( NULL != call_info_entry && !res )
  {
    if ( VOICE_INVALID_CALL_ID != call_info_entry->android_call_id &&
         CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type )
    {
      res = TRUE;
    }
    else
    {
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
    }
  }

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;
} // qmi_ril_voice_is_under_any_emergency_calls

//===========================================================================
// qmi_ril_voice_is_under_any_voice_calls
//===========================================================================
int qmi_ril_voice_is_under_any_voice_calls(void)
{
  int                                         res;
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  res = FALSE;

  qcril_qmi_voice_voip_lock_overview();

  call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
  while ( NULL != call_info_entry && !res )
  {
    if ( VOICE_INVALID_CALL_ID != call_info_entry->android_call_id &&
         ( CALL_TYPE_VOICE_V02 == call_info_entry->voice_scv_info.call_type ||
           CALL_TYPE_EMERGENCY_V02 == call_info_entry->voice_scv_info.call_type )
       )
    {
      res = TRUE;
    }
    else
    {
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
    }
  }

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);

  return res;
} // qmi_ril_voice_is_under_any_voice_calls

//===========================================================================
// RIL_REQUEST_SET_TTY_MODE
//===========================================================================
void qcril_qmi_voice_request_set_tty_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  uint8 i;
  int *tty_mode_ptr = NULL;
  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;

  tty_mode_enum_v02 qmi_tty_val;

  voice_set_config_req_msg_v02  qmi_request;
  voice_set_config_resp_msg_v02 qmi_response;

  qcril_request_resp_params_type resp;
  Ims__TtyNotify *ims_tty_notify = NULL;
  uint8_t outcome_valid = 0;
  uint8_t outcome = 0;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_NOTUSED( ret_ptr );

  do
  {
    if ( params_ptr->datalen <= 0 || params_ptr->data == NULL )
    {
      QCRIL_LOG_INFO("Invalid data received\n");
      break;
    }

    memset(&qmi_response, 0, sizeof(qmi_response));
    memset( &qmi_request, 0, sizeof( qmi_request ) );

    if ( RIL_REQUEST_SET_TTY_MODE == params_ptr->event_id )
    {
      tty_mode_ptr = ( int * )params_ptr->data;
      if ((*tty_mode_ptr) < QCRIL_CM_TTY_MODE_MIN || (*tty_mode_ptr) > QCRIL_CM_TTY_MODE_MAX )
      {
        QCRIL_LOG_INFO("Invalid tty mode (%d) received\n", *tty_mode_ptr);
        break;
      }
      switch ( *tty_mode_ptr )
      {
        case QCRIL_CM_TTY_MODE_FULL:
          qmi_tty_val = TTY_MODE_FULL_V02;
          break;

        case QCRIL_CM_TTY_MODE_HCO:
          qmi_tty_val = TTY_MODE_HCO_V02;
          break;

        case QCRIL_CM_TTY_MODE_VCO:
          qmi_tty_val = TTY_MODE_VCO_V02;
          break;

        case QCRIL_CM_TTY_MODE_OFF:   // fallthrough
        default:
          qmi_tty_val = TTY_MODE_OFF_V02;
          break;
      }
      QCRIL_LOG_INFO(".. tty val  %d", (int)qmi_tty_val);

      qmi_request.tty_mode_valid = TRUE;
      qmi_request.tty_mode = qmi_tty_val;
    }
    else
    {
      ims_tty_notify = (Ims__TtyNotify *)params_ptr->data;
      if (!qcril_qmi_ims_translate_ims_ttymodetype_to_qmi_tty_mode(ims_tty_notify->mode,
                  &qmi_tty_val))
      {
        QCRIL_LOG_INFO("Invalid ui tty mode(%d) received\n", ims_tty_notify->mode);
         break;
      }
      QCRIL_LOG_INFO(".. tty setting  %d", (int)qmi_tty_val);
      qmi_request.ui_tty_setting_valid = TRUE;
      qmi_request.ui_tty_setting = qmi_tty_val;
    }

    ril_req_res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_VOICE,
                                                       QMI_VOICE_SET_CONFIG_REQ_V02,
                                                       &qmi_request,
                                                       sizeof( qmi_request ),
                                                       (void*) &qmi_response,
                                                       sizeof( qmi_response ),
                                                       QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                        );

    QCRIL_LOG_INFO(".. qmi req got  %d", (int)ril_req_res );

    if ( RIL_E_SUCCESS == ril_req_res )
    {
      if ( qmi_request.tty_mode_valid )
      {
        outcome_valid = qmi_response.tty_mode_outcome_valid;
        outcome = qmi_response.tty_mode_outcome;
      }
      else if ( qmi_request.ui_tty_setting_valid )
      {
        outcome_valid = qmi_response.ui_tty_setting_outcome_valid;
        outcome = qmi_response.ui_tty_setting_outcome;
      }
      QCRIL_LOG_INFO(".. outcome  %d, %d", (int)outcome_valid, (int)outcome );
      if ( outcome_valid && 0x00 != outcome )// 0x00: Information written successfully per QMI Voice
      {
        ril_req_res = RIL_E_GENERIC_FAILURE;
      }
    }
  } while (FALSE);

  qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t,
          params_ptr->event_id, ril_req_res);

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ril_req_res);

} // qcril_qmi_voice_request_set_tty_mode

//===========================================================================
// RIL_REQUEST_QUERY_TTY_MODE
//===========================================================================
void qcril_voice_query_tty_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  qcril_request_resp_params_type resp;

  RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
  int ril_response = 0;

  voice_get_config_req_msg_v02 qmi_request;
  voice_get_config_resp_msg_v02 qmi_response;

  QCRIL_LOG_FUNC_ENTRY();


  QCRIL_NOTUSED( ret_ptr );

  memset( &qmi_request, 0, sizeof( qmi_request ) );
  qmi_request.tty_mode_valid = TRUE;
  qmi_request.tty_mode = 0x01; // as per QMI Voice - 0x01: Include tty configurations status information in response message

  memset( &qmi_response, 0, sizeof( qmi_response ) );

  ril_req_res =  qcril_qmi_client_send_msg_sync_ex( QCRIL_QMI_CLIENT_VOICE,
                                                     QMI_VOICE_GET_CONFIG_REQ_V02,
                                                     &qmi_request,
                                                     sizeof( qmi_request ),
                                                     (void*) &qmi_response,
                                                     sizeof( qmi_response ),
                                                     QCRIL_QMI_SYNC_REQ_UNRESTRICTED_TIMEOUT
                                                      );

  QCRIL_LOG_INFO( ".. qmi req %d", (int)ril_req_res );

  if ( RIL_E_SUCCESS == ril_req_res )
  {
    if ( qmi_response.current_tty_mode_valid )
    {
      switch ( qmi_response.current_tty_mode )
      {
        case TTY_MODE_FULL_V02:
          ril_response = QCRIL_CM_TTY_MODE_FULL;
          break;

        case TTY_MODE_VCO_V02:
          ril_response = QCRIL_CM_TTY_MODE_VCO;
          break;

        case TTY_MODE_HCO_V02:
          ril_response = QCRIL_CM_TTY_MODE_HCO;
          break;

        case TTY_MODE_OFF_V02:  // fallthrough
          ril_response = QCRIL_CM_TTY_MODE_OFF;
        default:
          break;
      }
    }
    else
    {
      ril_req_res = RIL_E_GENERIC_FAILURE;
    }
  }

  qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
  if ( RIL_E_SUCCESS == ril_req_res  )
  {
    resp.resp_pkt = (void *) &ril_response;
    resp.resp_len = sizeof( ril_response );
  }
  qcril_send_request_response( &resp );

  QCRIL_LOG_FUNC_RETURN_WITH_RET(ril_req_res);

} // qcril_voice_query_tty_mode

//===========================================================================
// qcril_qmi_voice_get_waiting_call_handler
//===========================================================================
void qcril_qmi_voice_waiting_call_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED( ret_ptr );
  if (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_KDDI_HOLD_ANSWER))
  {
    // send flash for Hold Answer feature
    voice_send_flash_req_msg_v02  send_flash_req_msg;
    voice_send_flash_resp_msg_v02 send_flash_resp_msg;
    memset(&send_flash_req_msg,0,sizeof(send_flash_req_msg));
    send_flash_req_msg.call_id = *((uint8_t*) (params_ptr->data));
    send_flash_req_msg.flash_type_valid = TRUE;
    send_flash_req_msg.flash_type = 1; // QMI_VOICE_FLASH_TYPE_ACT_ANSWER_ HOLD -- Activate answer hold

    int ret = qcril_qmi_client_send_msg_sync ( QCRIL_QMI_CLIENT_VOICE,
                                               QMI_VOICE_SEND_FLASH_REQ_V02,
                                               &send_flash_req_msg,
                                               sizeof(send_flash_req_msg),
                                               &send_flash_resp_msg,
                                               sizeof(send_flash_resp_msg));
    QCRIL_LOG_INFO("Send flash result: %d, %d", ret, send_flash_resp_msg.resp);
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_get_waiting_call_handler

// ---------------------------------------------- VOIP utilities -------------------------------------------------------------------
//===========================================================================
// qcril_qmi_voice_voip_call_info_entry_type
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_create_call_info_entry(
        uint8_t call_qmi_id,
        int32_t call_media_id,
        int need_allocate_call_android_id,
        qcril_qmi_voice_voip_call_info_elaboration_type initial_elaboration )
{
  uint8_t call_android_id = VOICE_INVALID_CALL_ID;
  qcril_qmi_voice_voip_call_info_entry_type* res = qcril_malloc( sizeof( *res ) );
  RIL_Errno id_alloc_res;
  int   finalized_ok;

  if ( res )
  {
    finalized_ok = FALSE;
    memset( res, 0, sizeof( *res ) );

    do
    {
      res->elaboration = initial_elaboration;

      if ( need_allocate_call_android_id )
      {
        id_alloc_res = qcril_qmi_voice_voip_allocate_call_android_id ( &call_android_id );
        if ( RIL_E_SUCCESS != id_alloc_res )
            break;
      }

      res->android_call_id = call_android_id;

      res->qmi_call_id = call_qmi_id;

      res->media_id = call_media_id;

      res->next = qmi_voice_voip_overview.call_info_root;
      qmi_voice_voip_overview.call_info_root = res;

      finalized_ok = TRUE;
    } while (FALSE);

    if ( !finalized_ok )
    { // rollback
      qcril_free( res );
      res = NULL;
    }
  }

  QCRIL_LOG_ESSENTIAL("Created call info entry %p with call android id %d, qmi id %d, media id %d",
                        res, (int) call_android_id, call_qmi_id, call_media_id);

  return res;
} // qcril_qmi_voice_voip_call_info_entry_type

//===========================================================================
// qcril_qmi_voice_voip_destroy_call_info_entry
//===========================================================================
void qcril_qmi_voice_voip_destroy_call_info_entry( qcril_qmi_voice_voip_call_info_entry_type* entry )
{
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* prev = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* temp = NULL;
  int found;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_INFO(  ".. entry %p", entry );

  if ( entry )
  {

    iter = qmi_voice_voip_overview.call_info_root;
    prev = NULL;
    found = FALSE;

    while ( iter != NULL && !found )
    {
      if ( iter == entry )
      {
        found = TRUE;
      }
      else
      {
        prev = iter;
        iter = iter->next;
      }
    }
    QCRIL_LOG_INFO( ".. entry found ok %d", found );
    if ( found )
    {
      if ( entry == qmi_voice_voip_overview.call_info_enumeration_current )
      {
        qmi_voice_voip_overview.call_info_enumeration_current = entry->next;
      }

      if ( NULL == prev )
      { // first
        qmi_voice_voip_overview.call_info_root = entry->next;
      }
      else
      {
        prev->next = entry->next;
      }

      // destroy
      if ( !(entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_GOLDEN) )
      {
        //TODO: need to remove this when the call list is removed.
        // release the VoIP mpty calls
        iter = entry->mpty_voip_call_list;
        while( iter != NULL )
        {
           temp = iter;
           iter = iter->mpty_voip_call_list;
           QCRIL_LOG_INFO("free mpty_voip_call_list entry: %p", temp);
           qcril_free(temp);
        }

        //destroy the cdma emergency voice call handling entries
        if( entry->emer_voice_number.number )
        {
          qcril_free(entry->emer_voice_number.number);
        }

        // detroy phaseout timer if needed
        if ( QMI_RIL_ZERO != entry->call_obj_phase_out_timer_id )
        {
          qcril_cancel_timed_callback( (void*)(uintptr_t)entry->call_obj_phase_out_timer_id );
        }

        // destroy additional call info buffer
        if( entry->additional_call_info.buffer )
        {
          qcril_free(entry->additional_call_info.buffer);
        }

        qcril_free( entry );
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_voip_destroy_call_info_entry

//===========================================================================
// qcril_qmi_voice_voip_destroy_mpty_call_info_entry
//===========================================================================
//TODO: Remove the function when call list is removed.
void qcril_qmi_voice_voip_destroy_mpty_call_info_entry( qcril_qmi_voice_voip_call_info_entry_type* entry )
{
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* prev = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
  int found;

  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_INFO(  ".. entry %p", entry );

  if ( entry )
  {
    iter = qmi_voice_voip_overview.call_info_root;
    prev = NULL;
    found = FALSE;

    while ( iter != NULL && !found )
    {
      call_info_entry = iter;
      do
      {
        if ( call_info_entry == entry )
        {
          found = TRUE;
          break;
        }
        prev = call_info_entry;
        call_info_entry = call_info_entry->mpty_voip_call_list;
      } while(call_info_entry);
      iter = iter->next;
    }

    QCRIL_LOG_INFO( ".. entry found ok %d", found );
    if ( found )
    {
      if ( NULL != prev && entry == prev->mpty_voip_call_list )
      {
        prev->mpty_voip_call_list = entry->mpty_voip_call_list;

        // detroy phaseout timer if needed
        if ( QMI_RIL_ZERO != entry->call_obj_phase_out_timer_id )
        {
          qcril_cancel_timed_callback( (void*)(uintptr_t)entry->call_obj_phase_out_timer_id );
        }

        qcril_free( entry );
      }
      else
      {
        QCRIL_LOG_INFO( ".. this is not a mpty call_info" );
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_voip_destroy_mpty_call_info_entry

//===========================================================================
// qcril_qmi_voice_voip_allocate_call_android_id
//===========================================================================
RIL_Errno qcril_qmi_voice_voip_allocate_call_android_id( uint8_t* new_call_android_id )
{
  RIL_Errno res = RIL_E_GENERIC_FAILURE;
  uint8_t call_id;
  uint8_t generated_call_id;
  int generated;
  int found;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  generated_call_id = VOICE_INVALID_CALL_ID;

  if ( new_call_android_id )
  {
    generated = FALSE;
    for ( call_id = VOICE_LOWEST_CALL_ID; call_id <= VOICE_HIGHEST_CALL_ID && !generated; call_id++ )
    {
      iter = qmi_voice_voip_overview.call_info_root;
      found = FALSE;
      while ( iter != NULL && !found )
      {
        call_info_entry = iter;
        do
        {
            if ( call_id == call_info_entry->android_call_id )
            {
                found = TRUE;
                break;
            }
           call_info_entry = call_info_entry->mpty_voip_call_list;
        } while (call_info_entry);
        iter = iter->next;
      }
      if ( !found )
      {
        generated_call_id = call_id;
        generated = TRUE;
      }
    }

    if ( generated )
    {
      res = RIL_E_SUCCESS;
    }

    *new_call_android_id = generated_call_id;
  }

  QCRIL_LOG_INFO( "returns res %d and id %d", (int) res, (int)generated_call_id );

  return res;
} // qcril_qmi_voice_voip_allocate_call_android_id

//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id( uint8_t call_qmi_id )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;

  QCRIL_LOG_INFO( "entry with id %d", (int)call_qmi_id );
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    if ( call_qmi_id == iter->qmi_call_id )
    {
        res = iter;
    }
    else
    {
        iter = iter->next;
    }
  }


  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_call_qmi_id

//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id( uint8_t call_andoid_id )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  QCRIL_LOG_INFO( "entry with id %d", (int)call_andoid_id );
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    call_info_entry = iter;
    do
    {
      if ( call_andoid_id == call_info_entry->android_call_id )
      {
        res = call_info_entry;
        break;
      }
      call_info_entry = call_info_entry->mpty_voip_call_list;
    } while(call_info_entry);
    iter = iter->next;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id

//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state( RIL_CallState ril_call_state )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;

  QCRIL_LOG_INFO( "entry with ril call state %d", (int)ril_call_state );
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    if ( ( iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_RIL_CALL_STATE_VALID ) && ( ril_call_state == iter->ril_call_state ) )
    {
        res = iter;
    }
    else
    {
        iter = iter->next;
    }
  }


  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_andoid_call_state
//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_elaboration
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( qcril_qmi_voice_voip_call_info_elaboration_type elaboration_pattern, int pattern_present )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
  qcril_qmi_voice_voip_call_info_elaboration_type expected_after_application;

  QCRIL_LOG_INFO( "entry with elaboration %x, %x hex", (uint32)(elaboration_pattern >> 32),(uint32)elaboration_pattern );

  if ( pattern_present )
  {
    expected_after_application  = elaboration_pattern;
  }
  else
  {
    expected_after_application  = QMI_RIL_ZERO;
  }

  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    call_info_entry = iter;
    do
    {
      if ( expected_after_application == (elaboration_pattern & call_info_entry->elaboration) ) // all bits
      {
        res = call_info_entry;
        break;
      }
      call_info_entry = call_info_entry->mpty_voip_call_list;
    } while (call_info_entry);
    iter = iter->next;
  }

  if ( NULL != res )
  {
    QCRIL_LOG_INFO( ".. found %p - with call android id %d, call qmi id %d", res, (int)res->android_call_id, (int)res->qmi_call_id );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_elaboration
//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset( qcril_qmi_voice_voip_call_info_elaboration_type elaboration_pattern )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;

  QCRIL_LOG_INFO( "entry with pattern %d", (int)elaboration_pattern );


  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    if ( elaboration_pattern & iter->elaboration ) // any of requested bits
    {
        res = iter;
    }
    else
    {
        iter = iter->next;
    }
  }

  if ( NULL != res )
  {
    QCRIL_LOG_INFO( ".. found %p - with call android id %d, call qmi id %d", res, (int)res->android_call_id, (int)res->qmi_call_id );
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset

//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_conn_uri
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_conn_uri( const char *conn_uri )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;

  QCRIL_LOG_INFO( "entry with conn_uri %s", conn_uri);
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    call_info_entry = iter;
    do
    {
      if ( !strcmp( conn_uri, iter->voice_svc_remote_party_number.number ) )
      {
        res = call_info_entry;
        break;
      }
      call_info_entry = call_info_entry->mpty_voip_call_list;
    } while (call_info_entry);
    iter = iter->next;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_conn_uri

//===========================================================================
// qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state( call_state_enum_v02 qmi_call_state )
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;
  qcril_qmi_voice_voip_call_info_entry_type* iter;

  QCRIL_LOG_INFO( "seeking entry with qmi call state %d", (int)qmi_call_state );
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL && NULL == res )
  {
    if ( qmi_call_state == iter->voice_scv_info.call_state )
    {
        res = iter;
    }
    else
    {
        iter = iter->next;
    }
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
} // qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state

//===========================================================================
// qcril_qmi_voice_consider_shadow_remote_number_cpy_creation
//===========================================================================
void qcril_qmi_voice_consider_shadow_remote_number_cpy_creation( qcril_qmi_voice_voip_call_info_entry_type* entry )
{
  QCRIL_LOG_INFO( "param %p", entry );

  if ( NULL != entry )
  {
    if ( ( CALL_TYPE_VOICE_V02 == entry->voice_scv_info.call_type ||
         CALL_TYPE_EMERGENCY_V02 == entry->voice_scv_info.call_type ) &&
         entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID &&
         !qcril_qmi_voice_is_emer_voice_entry_valid(&entry->emer_voice_number) )
    {
      QCRIL_LOG_INFO( "shadow created for voice/emergency call" );
      qcril_qmi_voice_create_emer_voice_entry(&entry->emer_voice_number, &entry->voice_svc_remote_party_number);
    }


  }
} // qcril_qmi_voice_consider_shadow_remote_number_cpy_creation

//===========================================================================
// qcril_qmi_voice_voip_update_call_info_entry_mainstream
//===========================================================================
void qcril_qmi_voice_voip_update_call_info_entry_mainstream(qcril_qmi_voice_voip_call_info_entry_type* entry,
                                                 voice_call_info2_type_v02* call_info,
                                                 uint8_t remote_party_number_valid,
                                                 voice_remote_party_number2_type_v02* remote_party_number,
                                                 uint8_t remote_party_name_valid,
                                                 voice_remote_party_name2_type_v02* remote_party_name,
                                                 uint8_t alerting_type_valid,
                                                 voice_alerting_type_type_v02* alerting_type,
                                                 uint8_t srv_opt_valid,
                                                 voice_srv_opt_type_v02* srv_opt,
                                                 uint8_t call_end_reason_valid,
                                                 voice_call_end_reason_type_v02* call_end_reason,
                                                 uint8_t alpha_id_valid,
                                                 voice_alpha_ident_with_id_type_v02* alpha_id,
                                                 uint8_t conn_party_num_valid,
                                                 voice_conn_num_with_id_type_v02* conn_party_num,
                                                 uint8_t diagnostic_info_valid,
                                                 voice_diagnostic_info_with_id_type_v02* diagnostic_info,
                                                 uint8_t called_party_num_valid,
                                                 voice_num_with_id_type_v02* called_party_num,
                                                 uint8_t redirecting_party_num_valid,
                                                 voice_num_with_id_type_v02* redirecting_party_num,
                                                 uint8_t ril_call_state_valid,
                                                 RIL_CallState ril_call_state,
                                                 uint8_t audio_attrib_valid,
                                                 voice_call_attributes_type_v02 *audio_attrib,
                                                 uint8_t video_attrib_valid,
                                                 voice_call_attributes_type_v02 *video_attrib,
                                                 uint8_t call_attrib_status_valid,
                                                 voice_call_attrib_status_type_v02 *call_attrib_status,
                                                 uint8_t is_srvcc_valid,
                                                 voice_is_srvcc_call_with_id_type_v02 *is_srvcc,
                                                 uint8_t srvcc_parent_call_info_valid,
                                                 voice_srvcc_parent_call_id_type_v02 *srvcc_parent_call_info,
                                                 uint8_t local_call_capabilities_info_valid,
                                                 voice_ip_call_capabilities_info_type_v02 *local_call_capabilities_info,
                                                 uint8_t peer_call_capabilities_info_valid,
                                                 voice_ip_call_capabilities_info_type_v02 *peer_call_capabilities_info,
                                                 uint8_t child_number_valid,
                                                 voice_child_number_info_type_v02 *child_number,
                                                 uint8_t display_text_valid,
                                                 voice_display_text_info_type_v02 *display_text,
                                                 uint8_t ip_num_info_valid,
                                                 voice_ip_num_id_type_v02 *ip_num_info,
                                                 uint8_t conn_ip_num_info_valid,
                                                 voice_conn_ip_num_with_id_type_v02 *conn_ip_num_info,
                                                 uint8_t is_add_info_present_valid,
                                                 voice_is_add_info_present_with_id_type_v02 *is_add_info_present,
                                                 uint8_t ip_caller_name_valid,
                                                 voice_ip_caller_name_info_type_v02 *ip_caller_name,
                                                 uint8_t end_reason_text_valid,
                                                 voice_ip_end_reason_text_type_v02 *end_reason_text
                                                 )
{
  char alpha_remote_name[QMI_VOICE_CALLER_NAME_MAX_V02+1] = {0};
  int alpha_remote_name_len = 0;
  uint8 remote_party_name_len = 0;

  QCRIL_LOG_INFO( "param %p", entry );
  if ( entry )
  {
    QCRIL_LOG_INFO( "call android id %d, call qmi id %d", (int)entry->android_call_id, (int)entry->qmi_call_id );

    if ( call_info )
    {
      entry->voice_scv_info = *call_info;
      QCRIL_LOG_INFO( ".. call state %d, call type %d, call mode %d", (int)entry->voice_scv_info.call_state, (int)entry->voice_scv_info.call_type, (int)entry->voice_scv_info.mode );
    }

    /* If the alpha identifier is provided by the UICC, this is an indication that the terminal should not give any information
       to the user on the changes made by the UICC to the initial user request. So, We have to ignore modified number in remote
       party field if Alpha text is valid.
    */
    if ( ip_num_info_valid && ( NULL != ip_num_info ) )
    {
        if( !alpha_id_valid )
        {
           entry->voice_svc_remote_party_ip_number = *ip_num_info;
           entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_IP_NUMBER_VALID;
        }
        else
        {
           entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_IP_NUMBER_VALID;
        }
    }
    if ( remote_party_number_valid && ( NULL != remote_party_number ) )
    {
        if( !alpha_id_valid )
        {
           entry->voice_svc_remote_party_number = *remote_party_number;
           entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;
        }
        else
        {
           entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NUMBER_VALID;
        }
    }
    if ( remote_party_name_valid && NULL != remote_party_name )
    {
      entry->voice_svc_remote_party_name.call_id = remote_party_name->call_id;
      entry->voice_svc_remote_party_name.name_pi = remote_party_name->name_pi;

      *(entry->voice_svc_remote_party_name.name) = 0;

      QCRIL_LOG_INFO("remote party name - coding scheme %d, len %d", remote_party_name->coding_scheme,
                      remote_party_name->name_len);

      remote_party_name_len = ((remote_party_name->name_len > QMI_VOICE_CALLER_NAME_MAX_V02) ?
              QMI_VOICE_CALLER_NAME_MAX_V02:
              remote_party_name->name_len);

      if (entry->voice_scv_info.mode == CALL_MODE_CDMA_V02)
      {
        // convert ascii to utf8
        QCRIL_LOG_INFO("remote party name - call mode CDMA; convert ascii to utf8\n");
        entry->voice_svc_remote_party_name.name_len =
                    qcril_cm_ss_ascii_to_utf8((unsigned char*) remote_party_name->name,
                            remote_party_name_len,
                            entry->voice_svc_remote_party_name.name,
                            sizeof(entry->voice_svc_remote_party_name.name));
      }
      else
      {
        entry->voice_svc_remote_party_name.name_len =
                    qcril_cm_ss_convert_ussd_string_to_utf8(remote_party_name->coding_scheme,
                            remote_party_name_len,
                            (byte *)remote_party_name->name,
                            entry->voice_svc_remote_party_name.name);
      }

      if ( (! *entry->voice_svc_remote_party_name.name)
           || ( entry->voice_svc_remote_party_name.name_len >= sizeof(entry->voice_svc_remote_party_name.name) ) )
      {
         QCRIL_LOG_ERROR("Invalid conversion results, remote name");
      }
      else
      {
         entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NAME_VALID;
      }
    }

    if ( alerting_type_valid && NULL != alerting_type )
    {
      entry->voice_svc_alerting_type = *alerting_type;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ALERTING_TYPE_VALID;
    }
    if ( srv_opt_valid && NULL != srv_opt )
    {
      entry->voice_svc_srv_opt = *srv_opt;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_SRV_OPT_VALID;
    }
    if ( call_end_reason_valid && NULL != call_end_reason )
    {
      entry->voice_svc_call_end_reason = *call_end_reason;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_END_REASON_VALID;
    }
    if ( alpha_id_valid && NULL != alpha_id )
    {
      alpha_id->alpha_text_len = (alpha_id->alpha_text_len>QMI_VOICE_ALPHA_TEXT_MAX_V02)?
                                 QMI_VOICE_ALPHA_TEXT_MAX_V02:alpha_id->alpha_text_len;
      entry->voice_svc_alpha_id = *alpha_id;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ALPHA_ID_VALID;

      if((!remote_party_name_valid || (NULL == remote_party_name)) &&
          !qmi_ril_is_feature_supported(QMI_RIL_FEATURE_ATEL_STKCC))
      {
        if(ALPHA_DCS_GSM_V02 == alpha_id->alpha_dcs)
        {
          alpha_remote_name_len = qcril_cm_ss_convert_gsm8bit_alpha_string_to_utf8(
                                                           alpha_id->alpha_text,
                                                           alpha_id->alpha_text_len,
                                                           alpha_remote_name);
        }
        else
        {
          alpha_remote_name_len = qcril_cm_ss_convert_ucs2_to_utf8(alpha_id->alpha_text,
                                                               alpha_id->alpha_text_len * 2,
                                                               alpha_remote_name);
        }

        entry->voice_svc_remote_party_name.call_id  = entry->qmi_call_id;
        entry->voice_svc_remote_party_name.name_pi  = PRESENTATION_NAME_PRESENTATION_ALLOWED_V02;
        entry->voice_svc_remote_party_name.name_len = alpha_remote_name_len;
        strlcpy(entry->voice_svc_remote_party_name.name,
                alpha_remote_name,
                sizeof(entry->voice_svc_remote_party_name.name));
        entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_R_PARTY_NAME_VALID;

        QCRIL_LOG_ESSENTIAL("Alpha string %s", entry->voice_svc_remote_party_name.name);
      }
    }
    if ( conn_ip_num_info_valid && NULL != conn_ip_num_info )
    {
      entry->voice_svc_conn_party_ip_num = *conn_ip_num_info;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONN_PARTY_IP_NUM_VALID;
    }
    if ( conn_party_num_valid && NULL != conn_party_num )
    {
      entry->voice_svc_conn_party_num = *conn_party_num;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CONN_PARTY_NUM_VALID;
    }
    if ( diagnostic_info_valid && NULL != diagnostic_info )
    {
      entry->voice_svc_diagnostic_info = *diagnostic_info;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAGNOSTIC_INFO_VALID;
    }
    if ( called_party_num_valid && NULL != called_party_num )
    {
      entry->voice_svc_called_party_num = *called_party_num;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALLED_PARTY_NUM_VALID;
    }
    if ( redirecting_party_num_valid && NULL != redirecting_party_num )
    {
      entry->voice_svc_redirecting_party_num = *redirecting_party_num;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_REDIRECTING_PARTY_NUM_VALID;
    }
    if ( ril_call_state_valid )
    {
      entry->ril_call_state = ril_call_state;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_RIL_CALL_STATE_VALID;
    }
    if( audio_attrib_valid )
    {
       entry->elaboration |=  QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUDIO_ATTR_VALID;
       entry->voice_audio_attrib = *audio_attrib;
    }
    if( video_attrib_valid )
    {
       entry->elaboration |=  QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VIDEO_ATTR_VALID;
       entry->voice_video_attrib = *video_attrib;
    }
    if( call_attrib_status_valid )
    {
       entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_ATTR_VALID;
       entry->call_attrib_status = *call_attrib_status;
    }
    if ( is_srvcc_valid && NULL != is_srvcc )
    {
      if( TRUE == is_srvcc->is_srvcc_call)
      {
        entry->is_srvcc = *is_srvcc;
        entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID;
        entry->srvcc_in_progress = FALSE;
      }
    }

    entry->srvcc_parent_call_info_valid = srvcc_parent_call_info_valid;
    if ( srvcc_parent_call_info_valid && NULL != srvcc_parent_call_info )
    {
      entry->srvcc_parent_call_info = *srvcc_parent_call_info;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID;
    }

    /* In case of SRVCC as call type get modified to voice, reset voip mask */
    if( (entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_SRVCC_VALID) &&
        (entry->voice_scv_info.call_type == CALL_TYPE_VOICE_V02) )
    {
       if( entry->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN )
       {
          entry->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN;
          QCRIL_LOG_DEBUG("resetting the voip mask as call got modified to voice");
       }
    }

    if ( local_call_capabilities_info_valid && NULL != local_call_capabilities_info )
    {
      entry->local_call_capabilities_info = *local_call_capabilities_info;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_LOCAL_CALL_CAPBILITIES_VALID;
    }
    if ( peer_call_capabilities_info_valid && NULL != peer_call_capabilities_info )
    {
      entry->peer_call_capabilities_info = *peer_call_capabilities_info;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PEER_CALL_CAPBILITIES_VALID;
    }
    if ( child_number_valid && NULL != child_number )
    {
      entry->child_number = *child_number;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_CHILD_NUMBER_VALID;
    }
    if ( display_text_valid && NULL != display_text )
    {
      entry->display_text = *display_text;
      entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_IS_DISPLAY_TEXT_VALID;
    }
    if ( is_add_info_present_valid && NULL != is_add_info_present )
    {
      entry->additional_call_info.is_add_info_present = is_add_info_present->is_add_info_present;
    }
    if ( ip_caller_name_valid && NULL != ip_caller_name )
    {
      entry->ip_caller_name_valid = TRUE;
      entry->ip_caller_name = *ip_caller_name;
    }
    if ( end_reason_text_valid && NULL != end_reason_text )
    {
      entry->end_reason_text_valid = TRUE;
      entry->end_reason_text = *end_reason_text;
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_voip_update_call_info_entry_mainstream
//===========================================================================
// qcril_qmi_voice_voip_lock_overview
//===========================================================================
void qcril_qmi_voice_voip_update_call_info_uus(qcril_qmi_voice_voip_call_info_entry_type* entry,
                                               uus_type_enum_v02 uus_type,
                                               uus_dcs_enum_v02 uus_dcs,
                                               uint32_t uus_data_len,
                                               uint8_t *uus_data
                                                 )
{
  QCRIL_LOG_INFO( "param %p", entry );
  if ( entry )
  {
    entry->voice_svc_uus.uus_type = uus_type;
    entry->voice_svc_uus.uus_dcs  = uus_dcs;
    entry->voice_svc_uus.uus_data_len  = uus_data_len;
    memcpy( entry->voice_svc_uus.uus_data, uus_data, MIN( QMI_VOICE_UUS_DATA_MAX_V02, uus_data_len ) );
    entry->voice_svc_uus.uus_data_len  = uus_data_len;

    entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_UUS_VALID;
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_voip_update_call_info_uus

//===========================================================================
// qcril_qmi_voice_voip_lock_overview
//===========================================================================
void qcril_qmi_voice_voip_lock_overview()
{
  pthread_mutex_lock( &qmi_voice_voip_overview.overview_lock_mutex );
} // qcril_qmi_voice_voip_lock_overview
//===========================================================================
// qcril_qmi_voice_voip_unlock_overview
//===========================================================================
void qcril_qmi_voice_voip_unlock_overview()
{
  pthread_mutex_unlock( &qmi_voice_voip_overview.overview_lock_mutex );
} // qcril_qmi_voice_voip_unlock_overview
//===========================================================================
// qcril_qmi_voice_voip_mark_all_with
//===========================================================================
void qcril_qmi_voice_voip_mark_all_with(qcril_qmi_voice_voip_call_info_elaboration_type elaboration_set)
{
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;

  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL  )
  {
    iter->elaboration |= elaboration_set;
    iter = iter->next;
  }
} // qcril_qmi_voice_voip_mark_all_with
//===========================================================================
// qcril_qmi_voice_voip_unmark_all_with
//===========================================================================
void qcril_qmi_voice_voip_unmark_all_with(qcril_qmi_voice_voip_call_info_elaboration_type elaboration_set)
{
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_elaboration_type application_set;

  application_set = ~elaboration_set;
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL  )
  {
    iter->elaboration &= application_set;
    iter = iter->next;
  }
} // qcril_qmi_voice_voip_unmark_all_with

//===========================================================================
// qcril_qmi_voice_voip_unmark_with_specified_call_state
//===========================================================================
void qcril_qmi_voice_voip_unmark_with_specified_call_state
(
  qcril_qmi_voice_voip_call_info_elaboration_type elaboration_set,
  call_state_enum_v02 state
)
{
  qcril_qmi_voice_voip_call_info_entry_type* iter = NULL;
  qcril_qmi_voice_voip_call_info_elaboration_type application_set;

  application_set = ~elaboration_set;
  iter = qmi_voice_voip_overview.call_info_root;
  while ( iter != NULL  )
  {
    if ( state == iter->voice_scv_info.call_state )
    {
      iter->elaboration &= application_set;
    }
    iter = iter->next;
  }
} // qcril_qmi_voice_voip_unmark_with_specified_call_state

//===========================================================================
// qcril_qmi_voice_voip_mark_with_specified_call_state
//===========================================================================
void qcril_qmi_voice_voip_mark_with_specified_call_state
(
  qcril_qmi_voice_voip_call_info_elaboration_type elaboration_set,
  call_state_enum_v02 state
)
{
  qcril_qmi_voice_voip_call_info_entry_type* iter = qmi_voice_voip_overview.call_info_root;

  while ( iter != NULL  )
  {
    if ( state == iter->voice_scv_info.call_state )
    {
      iter->elaboration |= elaboration_set;
    }
    iter = iter->next;
  }
} // qcril_qmi_voice_voip_mark_with_specified_call_state

//===========================================================================
// qcril_qmi_voice_voip_call_info_entries_enum_first
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_call_info_entries_enum_first(void)
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;

  res = qmi_voice_voip_overview.call_info_root;
  qmi_voice_voip_overview.call_info_enumeration_current = res;

  return res;
} //
//===========================================================================
// qcril_qmi_voice_voip_call_info_entries_enum_next
//===========================================================================
qcril_qmi_voice_voip_call_info_entry_type* qcril_qmi_voice_voip_call_info_entries_enum_next(void)
{
  qcril_qmi_voice_voip_call_info_entry_type* res = NULL;

  res = NULL;

  if ( NULL != qmi_voice_voip_overview.call_info_enumeration_current )
  {
    res = qmi_voice_voip_overview.call_info_enumeration_current->next;
    qmi_voice_voip_overview.call_info_enumeration_current = res;
  }

  return res;
}
//===========================================================================
// qcril_qmi_voice_voip_generate_summary
//===========================================================================
void qcril_qmi_voice_voip_generate_summary( qcril_qmi_voice_voip_current_call_summary_type * summary )
{
  qcril_qmi_voice_voip_call_info_entry_type * call_info_entry = NULL;
  qcril_qmi_voice_voip_call_info_entry_type * call_info_iter = NULL;

  int nof_3gpp2_calls;
  int nof_3gpp_calls;
  int nof_active_calls;
  int nof_voip_calls;
  int nof_calls;

  if ( summary )
  {
    memset( summary, 0, sizeof( *summary ) );

    nof_3gpp2_calls  = 0;
    nof_3gpp_calls   = 0;
    nof_active_calls = 0;
    nof_voip_calls   = 0;
    nof_calls        = 0;

    call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
    while ( NULL != call_info_entry )
    {
      call_info_iter = call_info_entry;
      if (call_info_iter->mpty_voip_call_list)
      {
        call_info_iter = call_info_iter->mpty_voip_call_list;
      }

      do
      {
        if ( ( VOICE_INVALID_CALL_ID != call_info_iter->android_call_id ) &&
             ( ( VOICE_INVALID_CALL_ID != call_info_iter->qmi_call_id ) ||
               ( call_info_iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NO_QMI_ID_RECEIVED ) ||
               ( call_info_iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_MEMBER_OF_MPTY_VOIP_CALL ) )
           )
        { // skip shadow calls from CM space
          nof_calls++;
          if ( call_info_iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN )
          {
            nof_voip_calls++;
          }
          else if ( CALL_MODE_CDMA_V02 == call_info_iter->voice_scv_info.mode )
          {
            nof_3gpp2_calls++;
          }
          else
          {
            nof_3gpp_calls++;
          }

          if ( CALL_STATE_CONVERSATION_V02 == call_info_iter->voice_scv_info.call_state )
          {
            nof_active_calls++;
          }
          summary->active_or_single_call = call_info_entry;
        }
        call_info_iter = call_info_iter->mpty_voip_call_list;
      } while (call_info_iter);
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
    }

    summary->nof_calls_overall      = nof_calls;
    summary->nof_voip_calls         = nof_voip_calls;
    summary->nof_3gpp2_calls        = nof_3gpp2_calls;
    summary->nof_3gpp_calls         = nof_3gpp_calls;
    summary->nof_active_calls       = nof_active_calls;
    summary->nof_voice_calls        = nof_3gpp2_calls + nof_3gpp_calls;
  }
} // qcril_qmi_voice_voip_generate_summary

//===========================================================================
// qcril_qmi_voice_voip_call_info_dump
//===========================================================================
void qcril_qmi_voice_voip_call_info_dump(qcril_qmi_voice_voip_call_info_entry_type* call_info_entry)
{
  QCRIL_LOG_INFO( "param %p", call_info_entry );
  if ( call_info_entry )
  {
     do
     {
        QCRIL_LOG_DEBUG( ".. call android id %d, call qmi id %d, elaboration %x, %x hex",
                        (int)call_info_entry->android_call_id,
                        (int)call_info_entry->qmi_call_id,
                        (uint32) (call_info_entry->elaboration >> 32), (uint32)call_info_entry->elaboration);
        QCRIL_LOG_DEBUG( ".. call state %d, call type %d, call mode %d",
                        (int)call_info_entry->voice_scv_info.call_state,
                        (int)call_info_entry->voice_scv_info.call_type,
                        (int)call_info_entry->voice_scv_info.mode );
        call_info_entry = call_info_entry->mpty_voip_call_list;
     } while (call_info_entry);
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_voip_call_info_dump
//===========================================================================
// qmi_ril_set_supress_voice_calls
//===========================================================================
void qmi_ril_set_supress_voice_calls( int suppress )
{
  qmi_ril_voice_is_voice_calls_supressed = suppress;

  QCRIL_LOG_INFO( "is_voice_calls_supressed %d", (int)qmi_ril_voice_is_voice_calls_supressed );
} // qmi_ril_set_supress_voice_calls
//===========================================================================
// qcril_qmi_voice_request_set_local_call_hold
//===========================================================================^M
void qcril_qmi_voice_request_set_local_call_hold
(
const qcril_request_params_type *const params_ptr,
qcril_request_return_type *const ret_ptr
)
{
  qcril_reqlist_public_type reqlist_entry;
  qcril_request_resp_params_type resp;
  voice_manage_calls_req_msg_v02  manage_calls_req;
  voice_manage_calls_resp_msg_v02* mng_call_resp_msg_ptr = NULL;
  RIL_Errno   ril_req_res = RIL_E_SUCCESS;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  int lch = 0;
  uint32 user_data;
  int error = 0;

  QCRIL_NOTUSED( ret_ptr );

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    if ( NULL == params_ptr->data )
    {
      QCRIL_LOG_ERROR(".. invalid param");
      ril_req_res = RIL_E_GENERIC_FAILURE;
      break;
    }
    lch = *(( int *) params_ptr->data);
    memset(&manage_calls_req, 0, sizeof(manage_calls_req));
    manage_calls_req.sups_type = lch ? SUPS_TYPE_LOCAL_HOLD_V02 : SUPS_TYPE_LOCAL_UNHOLD_V02;
    QCRIL_LOG_DEBUG("lch = %d, sups type %d", lch, (int) manage_calls_req.sups_type );
    manage_calls_req.call_id_valid = FALSE;

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_HOOK_SET_LOCAL_CALL_HOLD,
                                 NULL,
                                 &reqlist_entry );

    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) == E_SUCCESS )
    {
      mng_call_resp_msg_ptr = qcril_malloc( sizeof(*mng_call_resp_msg_ptr) );
      if( NULL == mng_call_resp_msg_ptr )
      {
        QCRIL_LOG_DEBUG("mng_call_resp_msg_ptr is null");
        ril_req_res = RIL_E_GENERIC_FAILURE;
        break;
      }

      user_data = QCRIL_COMPOSE_USER_DATA( instance_id, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

      /* Send QMI VOICE MANAGE CALLS REQ */
      error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                      QMI_VOICE_MANAGE_CALLS_REQ_V02,
                                      &manage_calls_req,
                                      sizeof(manage_calls_req),
                                      mng_call_resp_msg_ptr,
                                      sizeof(*mng_call_resp_msg_ptr),
                                      (void*)(uintptr_t)user_data);
      if ( E_SUCCESS != error )
      {
        QCRIL_LOG_DEBUG("send_msg_async failed with error code: %d", error);
        qcril_free( mng_call_resp_msg_ptr );
        ril_req_res = RIL_E_GENERIC_FAILURE;
      }
    }
    else
    {
      QCRIL_LOG_INFO("the new request is rejected");
      ril_req_res = RIL_E_GENERIC_FAILURE;
    }
  } while (FALSE);

  if ( RIL_E_SUCCESS != ril_req_res )
  { // failure
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
    qcril_send_request_response( &resp );
  }
  else
  {
#ifndef QMI_RIL_UTF
    qcril_am_handle_event(lch ? QCRIL_AM_EVENT_LCH : QCRIL_AM_EVENT_UNLCH, NULL);
#endif
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_request_set_local_call_hold
//===========================================================================
// qmi_ril_voice_is_eme_oos
//===========================================================================
int qmi_ril_voice_is_eme_oos( void )
{
  qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;
  int                                        res = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();
  call_info = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset(
                    QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS |
                    QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL |
                    QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL  |
                    QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAL_FROM_OOS );

  res = ( NULL != call_info ) ? TRUE : FALSE;

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int)res);
  return res;
} // qmi_ril_voice_is_eme_oos

//===========================================================================
// qmi_ril_voice_eme_oos_immunity_reload
//===========================================================================
void qmi_ril_voice_eme_oos_immunity_reload(void)
{
  qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_voice_voip_lock_overview();
  call_info = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS |
                                                                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL |
                                                                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL |
                                                                        QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAL_FROM_OOS
                                                                        );

  if ( call_info )
  {
    call_info->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_KILL_IMMUNITY;
    qcril_qmi_voice_voip_call_info_dump( call_info );
  }


  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN();

} // qmi_ril_voice_eme_oos_immunity_reload
//===========================================================================
// qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate
//===========================================================================
int qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate( void )
{
  qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;
  int                                        res;

  QCRIL_LOG_FUNC_ENTRY();

  res = qmi_ril_nwreg_is_vrte_post_change_window();

  if ( res )
  {
    qcril_qmi_voice_voip_lock_overview();
    call_info = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration_any_subset( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_FROM_OOS |
                                                                          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EMERGENCY_CALL |
                                                                          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_1x_CSFB_CALL |
                                                                          QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_DIAL_FROM_OOS
                                                                          );

    if ( NULL != call_info )
    {
      qcril_qmi_voice_voip_call_info_dump( call_info );

      if ( !( call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_KILL_IMMUNITY ) )
      {
        res = FALSE;
      }
      else
      { // immune against being killed but only once
        call_info->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_EME_KILL_IMMUNITY;
      }
    }
    else
    {
      res = FALSE;
    }

    qcril_qmi_voice_voip_unlock_overview();

  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET((int)res);
  return res;

} // qmi_ril_voice_check_eme_oos_handover_in_progress_and_immunity_animate
//===========================================================================
// qmi_ril_voice_drop_homeless_incall_reqs
//===========================================================================
void qmi_ril_voice_drop_homeless_incall_reqs( void )
{
  QCRIL_LOG_FUNC_ENTRY();

  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        qmi_ril_voice_drop_homeless_incall_reqs_main_threaded,
                        NULL,  // immediate
                        NULL );

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_drop_homeless_incall_reqs
//===========================================================================
// qmi_ril_voice_drop_homeless_incall_reqs_main_threaded
//===========================================================================
void qmi_ril_voice_drop_homeless_incall_reqs_main_threaded(void * param)
{
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd( RIL_REQUEST_DTMF, RIL_E_CANCELLED );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_drop_homeless_incall_reqs_main_threaded
//===========================================================================
// qmi_ril_voice_cleanup_reqs_after_call_completion
//===========================================================================
void qmi_ril_voice_cleanup_reqs_after_call_completion(void)
{
  QCRIL_LOG_FUNC_ENTRY();

  qcril_setup_timed_callback( QCRIL_DEFAULT_INSTANCE_ID,
                        QCRIL_DEFAULT_MODEM_ID,
                        qmi_ril_voice_cleanup_reqs_after_call_completion_main_threaded,
                        NULL,  // immediate
                        NULL );

  QCRIL_LOG_FUNC_RETURN();

} // qmi_ril_voice_cleanup_reqs_after_call_completion
//===========================================================================
// qmi_ril_voice_cleanup_reqs_after_call_completion_main_threaded
//===========================================================================
void qmi_ril_voice_cleanup_reqs_after_call_completion_main_threaded(void * param)
{

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED( param );

    qmi_ril_fw_android_request_flow_control_drop_legacy_book_records( TRUE, FALSE );
    qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd( RIL_REQUEST_DTMF, RIL_E_CANCELLED );
    qmi_ril_fw_android_request_flow_control_abandon_requests_family_main_thrd( RIL_REQUEST_CONFERENCE, RIL_E_CANCELLED );

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_cleanup_reqs_after_call_completion_main_threaded


/*===========================================================================

  FUNCTION: qcril_qmi_voice_create_mpty_voip_call

===========================================================================*/
/*!
    @brief
    Should be called once the voip conf request to QMI VOICE returns successfully.
    Add the members of the multiparty voip calls to the call marked as mpty call.
    Set the mpty call state to current_call_state.

    @return
    TRUE, if multi party call is created.
    FALSE, otherwise.
*/
/*=========================================================================*/
boolean qcril_qmi_voice_create_mpty_voip_call_vcl(call_state_enum_v02 *current_call_qmi_state)
{
  qcril_qmi_voice_voip_call_info_entry_type *list = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *mpty_call_info = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *temp = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *prev = NULL;
  boolean result = FALSE;

  if (NULL != current_call_qmi_state)
  {
     *current_call_qmi_state = CALL_STATE_ENUM_MIN_ENUM_VAL_V02;
  }

  QCRIL_LOG_FUNC_ENTRY();

  // find mpty call object
  list = qmi_voice_voip_overview.call_info_root;
  mpty_call_info = NULL;
  while( list != NULL )
  {
     if( list->elaboration & (QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL | QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL) )
     {
        QCRIL_LOG_INFO("found mpty call with call android id %d, call qmi id %d, elaboration %x, %x hex",
                        (int)list->android_call_id,
                        (int)list->qmi_call_id,
                        (uint32) (list->elaboration >> 32), (uint32)list->elaboration);

        list->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MPTY_VOIP_CALL;
        list->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL;
        mpty_call_info = list;
        break;
     }
     list = list->next;
  }

  if (mpty_call_info)
  {
     list = qmi_voice_voip_overview.call_info_root;
     if (NULL != current_call_qmi_state)
     {
        *current_call_qmi_state = mpty_call_info->voice_scv_info.call_state;
     }
     prev = NULL;

     while( list != NULL )
     {
       if( list->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL )
       {
          QCRIL_LOG_INFO("found mpty call member with call android id %d, call qmi id %d, elaboration %x, %x hex",
                        (int)list->android_call_id,
                        (int)list->qmi_call_id,
                        (uint32) (list->elaboration >> 32), (uint32)list->elaboration);
          list->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL;
          list->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_MEMBER_OF_MPTY_VOIP_CALL;

          // append mpty call members to mpty call list
          mpty_call_info->mpty_voip_call_list = list;
          mpty_call_info = list;

          // remove mpty call member from normal call list
          if (list == qmi_voice_voip_overview.call_info_root)
          {
             qmi_voice_voip_overview.call_info_root = list->next;
             temp = list;
             list = list->next;
             temp->next = NULL;
          }
          else
          {
             if (prev)
             {
               prev->next = list->next;
               temp = list;
               list = list->next;
               temp->next = NULL;
             }
             else
             {
               QCRIL_LOG_ERROR("prev is NULL");
               list = list->next;
             }
          }
       }
       else
       {
          prev = list;
          list = list->next;
       }
     }
  }

  else
  {
     QCRIL_LOG_DEBUG("not found any mpty call");
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int) result);
  return result;
} // qcril_qmi_voice_create_mpty_voip_call_vcl

//===========================================================================
// qcril_qmi_voice_add_call_to_existing_mpty_voip_call_vcl
//===========================================================================
//TODO: need to remove the logic for adding mpty call (remove the entire function)
boolean qcril_qmi_voice_add_call_to_existing_mpty_voip_call_vcl(qcril_qmi_voice_voip_call_info_entry_type* call_entry_ptr)
{
  qcril_qmi_voice_voip_call_info_entry_type *iter = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *mpty_call_info = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *temp = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *prev = NULL;
  boolean result = FALSE;

  QCRIL_LOG_FUNC_ENTRY();

  if (NULL != call_entry_ptr)
  {

      call_entry_ptr->elaboration &= ~QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL;

      // find mpty call object
      iter = qmi_voice_voip_overview.call_info_root;
      mpty_call_info = NULL;
      while( iter != NULL )
      {
         if( iter->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_MPTY_VOIP_CALL )
         {
            QCRIL_LOG_INFO("found mpty call with call android id %d, call qmi id %d, elaboration %x, %x hex",
                            (int)iter->android_call_id,
                            (int)iter->qmi_call_id,
                            (uint32) (iter->elaboration >> 32), (uint32)iter->elaboration);
            mpty_call_info = iter;
            break;
         }
         iter = iter->next;
      }

      if ( mpty_call_info )
      {
         // moving mpty_call_info to the last call object
         while ( NULL != mpty_call_info->mpty_voip_call_list )
         {
           mpty_call_info = mpty_call_info->mpty_voip_call_list;
         }

         iter = qmi_voice_voip_overview.call_info_root;
         prev = NULL;

         while( iter != NULL )
         {
           if(iter ==  call_entry_ptr)
           {
              QCRIL_LOG_INFO("found mpty call member with call android id %d, call qmi id %d, elaboration %x, %x hex",
                            (int)iter->android_call_id,
                            (int)iter->qmi_call_id,
                            (uint32) (iter->elaboration >> 32), (uint32)iter->elaboration);
              iter->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_MEMBER_OF_MPTY_VOIP_CALL;

              // append mpty call members to mpty call list
              mpty_call_info->mpty_voip_call_list = iter;
              mpty_call_info = iter;

              // remove mpty call member from normal call list
              if (iter == qmi_voice_voip_overview.call_info_root)
              {
                 qmi_voice_voip_overview.call_info_root = iter->next;
                 temp = iter;
                 iter = iter->next;
                 temp->next = NULL;
              }
              else
              {
                 if (prev)
                 {
                   prev->next = iter->next;
                   temp = iter;
                   iter = iter->next;
                   temp->next = NULL;
                 }
                 else
                 {
                   QCRIL_LOG_ERROR("prev is NULL");
                   iter = iter->next;
                 }
              }

              result = TRUE;
           }
           else
           {
              prev = iter;
              iter = iter->next;
           }
         }
      }
      else
      {
         QCRIL_LOG_DEBUG("call_entry_ptr not found in call list");
      }
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET((int) result);
  return result;
} // qcril_qmi_voice_add_call_to_existing_mpty_voip_call_vcl

//===========================================================================
// qcril_qmi_voice_add_call_to_existing_mpty_voip_call_failure_cleanup_vcl
//===========================================================================
void qcril_qmi_voice_add_call_to_existing_mpty_voip_call_failure_cleanup_vcl()
{
   qcril_qmi_voice_voip_unmark_all_with(QCRIL_QMI_VOICE_VOIP_CALLINFO_TO_BE_MEMBER_OF_MPTY_VOIP_CALL);

   qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl();
} // qcril_qmi_voice_add_call_to_existing_mpty_voip_call_failure_cleanup_vcl

/*===========================================================================

  FUNCTION: qcril_qmi_voice_get_modem_call_type_info

===========================================================================*/
/*!
    @brief
    fetch modem call type information from android telephony information.

    @return
    TRUE, if call type can be derived.
    FALSE, otherwise.
*/
/*=========================================================================*/
boolean qcril_qmi_voice_get_modem_call_type_info
(
   RIL_Call_Details                      *call_details,
   call_type_enum_v02                    *call_type,
   uint8_t                               *audio_attrib_valid,
   voice_call_attribute_type_mask_v02    *audio_attrib,
   uint8_t                               *video_attrib_valid,
   voice_call_attribute_type_mask_v02    *video_attrib
)
{
   boolean result = TRUE;

   if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) || qcril_qmi_voice_info.jbims )
   {
       /* both of following fields are mandatory */
       if( call_details == NULL || call_type == NULL )
       {
          result = FALSE;
          return result;
       }

       switch( call_details->callType )
       {
       case RIL_CALL_TYPE_VOICE:
          if( call_details->callDomain == RIL_CALL_DOMAIN_PS )
          {
             *call_type = CALL_TYPE_VOICE_IP_V02;
          }
          else
          {
             *call_type = CALL_TYPE_VOICE_V02;
          }

          if( audio_attrib_valid != NULL && audio_attrib != NULL )
          {
             *audio_attrib_valid = TRUE;
             *audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
          }

          if( video_attrib_valid != NULL && video_attrib != NULL )
          {
             *video_attrib_valid = TRUE;
             *video_attrib = 0;
          }
          break;

       case RIL_CALL_TYPE_VS_RX:
          /* Video is receive only */
          if( call_details->callDomain == RIL_CALL_DOMAIN_PS || RIL_CALL_DOMAIN_AUTOMATIC == call_details->callDomain )
          {
             if( audio_attrib_valid == NULL || audio_attrib == NULL || video_attrib_valid == NULL || video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *audio_attrib_valid = TRUE;
                *audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                *video_attrib_valid = TRUE;
                *video_attrib = VOICE_CALL_ATTRIB_RX_V02;
                *call_type = CALL_TYPE_VT_V02;
             }
          }
          else
          {
             /* other domain values are not supported currently */
             result = FALSE;
          }
          break;

       case RIL_CALL_TYPE_VS_TX:
          /* Video is transmit only */
          if( call_details->callDomain == RIL_CALL_DOMAIN_PS || RIL_CALL_DOMAIN_AUTOMATIC == call_details->callDomain )
          {
             if( audio_attrib_valid == NULL || audio_attrib == NULL || video_attrib_valid == NULL || video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *audio_attrib_valid = TRUE;
                *audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                *video_attrib_valid = TRUE;
                *video_attrib = VOICE_CALL_ATTRIB_TX_V02;
                *call_type = CALL_TYPE_VT_V02;
             }
          }
          else
          {
             /* other domain values are not supported currently */
             result = FALSE;
          }
          break;

       case RIL_CALL_TYPE_VT:
          /* Video is transmit only */
          if( call_details->callDomain == RIL_CALL_DOMAIN_PS || RIL_CALL_DOMAIN_AUTOMATIC == call_details->callDomain )
          {
             if( audio_attrib_valid == NULL || audio_attrib == NULL || video_attrib_valid == NULL || video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *audio_attrib_valid = TRUE;
                *audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
                *video_attrib_valid = TRUE;
                *video_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
                *call_type = CALL_TYPE_VT_V02;
             }
          }
          else
          {
             /* other domain values are not supported currently */
             result = FALSE;
          }
          break;

       case RIL_CALL_TYPE_VT_NODIR:
          if( call_details->callDomain == RIL_CALL_DOMAIN_PS ||
              RIL_CALL_DOMAIN_AUTOMATIC == call_details->callDomain )
          {
             if( audio_attrib_valid == NULL || audio_attrib == NULL ||
                 video_attrib_valid == NULL || video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *audio_attrib_valid = TRUE;
                *audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
                *video_attrib_valid = TRUE;
                *video_attrib = 0;
                *call_type = CALL_TYPE_VT_V02;
             }
          }
          break;

       default:
          result = FALSE;
          break;
       }

       QCRIL_LOG_DEBUG( "result = %d, modem call type = %d, ril call type = %d, ril call domain = %d",
                        result, *call_type, call_details->callType, call_details->callDomain );

       if( ( audio_attrib != NULL ) && (audio_attrib_valid != NULL ) )
       {
          QCRIL_LOG_DEBUG(" audio_attrib_valid = %d, audio_attrib = %d", *audio_attrib_valid, *audio_attrib);
       }

       if( ( video_attrib != NULL ) && ( video_attrib_valid != NULL ) )
       {
          QCRIL_LOG_DEBUG(" video_attrib_valid = %d, video_attrib = %d", *video_attrib_valid, *video_attrib);
       }

   }
   return result;
}

/*===========================================================================

  FUNCTION: qcril_qmi_voice_get_atel_call_type_info_by_call_info

===========================================================================*/
/*!
    @brief
    fetch RIL call type information from an call info entry.

    @return
    TRUE, if call type can be derived.
    FALSE, otherwise.
*/
/*=========================================================================*/
boolean qcril_qmi_voice_get_atel_call_type_info_by_call_info
(
   const qcril_qmi_voice_voip_call_info_entry_type *call_info,
   RIL_Call_Details *call_details
)
{
   boolean ret;
   call_type_enum_v02 call_type;
   boolean video_attrib_valid;
   voice_call_attribute_type_mask_v02 video_attrib;
   boolean audio_attrib_valid;
   voice_call_attribute_type_mask_v02 audio_attrib;
   boolean attrib_status_valid;
   voice_call_attrib_status_enum_v02 attrib_status;
   qcril_qmi_voice_voip_call_info_elaboration_type call_info_elab;

   if ( NULL == call_info )
   {
      QCRIL_LOG_ERROR("call_info is NULL");
      ret = FALSE;
   }
   else
   {
      call_type = call_info->voice_scv_info.call_type;
      video_attrib_valid = (call_info->elaboration &
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_VIDEO_ATTR_VALID) ? TRUE : FALSE;
      video_attrib = call_info->voice_video_attrib.call_attributes;
      audio_attrib_valid = (call_info->elaboration &
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUDIO_ATTR_VALID) ? TRUE : FALSE;
      audio_attrib = call_info->voice_audio_attrib.call_attributes;
      attrib_status_valid = (call_info->elaboration &
              QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_ATTR_VALID) ? TRUE: FALSE;
      attrib_status = call_info->call_attrib_status.call_attrib_status;
      call_info_elab = call_info->elaboration;
      ret = qcril_qmi_voice_get_atel_call_type_info(call_type, video_attrib_valid, video_attrib,
          audio_attrib_valid, audio_attrib,
          attrib_status_valid, attrib_status,
          TRUE, call_info_elab,
          call_info->answered_call_type_valid, call_info->answered_call_type,
          call_details);
   }

   return ret;
} // qcril_qmi_voice_get_atel_call_type_info_by_call_info

/*===========================================================================

  FUNCTION: qcril_qmi_voice_get_atel_call_type_info

===========================================================================*/
/*!
    @brief
    fetch RIL call type information from call type, video attribute (if available)
    audio attribute (if available) and call elaboration (if available).

    @return
    TRUE, if call type can be derived.
    FALSE, otherwise.
*/
/*=========================================================================*/
boolean qcril_qmi_voice_get_atel_call_type_info
(
   call_type_enum_v02 call_type,
   boolean video_attrib_valid,
   voice_call_attribute_type_mask_v02 video_attrib,
   boolean audio_attrib_valid,
   voice_call_attribute_type_mask_v02 audio_attrib,
   boolean attrib_status_valid,
   voice_call_attrib_status_enum_v02 attrib_status,
   boolean call_info_elab_valid,
   qcril_qmi_voice_voip_call_info_elaboration_type call_info_elab,
   boolean cached_call_type_valid,
   Ims__CallType cached_call_type,
   RIL_Call_Details *call_details
)
{
   QCRIL_LOG_FUNC_ENTRY();
   boolean ret = TRUE;

   if ( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) || qcril_qmi_voice_info.jbims )
   {
      if ( NULL == call_details )
      {
         QCRIL_LOG_ERROR("call_details is NULL ");
         ret = FALSE;
      }
      else
      {
         QCRIL_LOG_INFO( "QMI call_type: %d, video_attrib_valid: %d, video_attrib: %d, "
                         "audio_attrib_valid: %d, audio_attrib: %d, "
                         "call_attrib_valid: %d, call_attrib:%d, "
                         "call_info_elab_valid: %d, elaboration %x, %x hex",
                         call_type, video_attrib_valid, (int)video_attrib,
                         audio_attrib_valid, audio_attrib,
                         attrib_status_valid, attrib_status,
                         call_info_elab_valid,
                         (uint32)(call_info_elab >> 32), (uint32)call_info_elab );

         call_details->callSubState = RIL_CALL_SUB_STATE_UNDEFINED;

         if ( CALL_TYPE_VT_V02 == call_type )
         {
            if( FALSE == video_attrib_valid )
            {
               call_details->callType = RIL_CALL_TYPE_VT;
               call_details->callDomain = RIL_CALL_DOMAIN_PS;
            }
            else
            {
               /* based on call attributes determine video call type */
               if( VOICE_CALL_ATTRIB_TX_V02 == video_attrib )
               {
                  call_details->callType = RIL_CALL_TYPE_VS_TX;
                  call_details->callDomain = RIL_CALL_DOMAIN_PS;
               }
               else if( VOICE_CALL_ATTRIB_RX_V02 == video_attrib )
               {
                  call_details->callType = RIL_CALL_TYPE_VS_RX;
                  call_details->callDomain = RIL_CALL_DOMAIN_PS;
               }
               else if( (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02) == video_attrib )
               {
                  call_details->callType = RIL_CALL_TYPE_VT;
                  call_details->callDomain = RIL_CALL_DOMAIN_PS;
               }
               else if ( TRUE == audio_attrib_valid && 0 == audio_attrib && 0 == video_attrib )
               {
                  call_details->callSubState = (RIL_CALL_SUB_STATE_AUDIO_CONNECTED_SUSPENDED |
                                                RIL_CALL_SUB_STATE_VIDEO_CONNECTED_SUSPENDED);
                  call_details->callType = cached_call_type_valid ?
                                           cached_call_type : RIL_CALL_TYPE_VT;
                  call_details->callDomain = RIL_CALL_DOMAIN_PS;
               }
               else if ( 0 == video_attrib && attrib_status_valid == TRUE )
               {
                  call_details->callType = RIL_CALL_TYPE_VT_NODIR;
                  call_details->callDomain = RIL_CALL_DOMAIN_PS;
                  switch (attrib_status)
                  {
                     case VOICE_CALL_ATTRIB_STATUS_RETRY_NEEDED_V02:
                        call_details->callSubState = RIL_CALL_SUB_STATE_AVP_RETRY;
                        break;
                     case VOICE_CALL_ATTRIB_STATUS_MEDIA_PAUSED_V02:
                        call_details->callSubState = RIL_CALL_SUB_STATE_MEDIA_PAUSED;
                        break;
                     case VOICE_CALL_ATTRIB_STATUS_OK_V02:
                        call_details->callSubState = RIL_CALL_SUB_STATE_UNDEFINED;
                        break;
                  }
               }
               else
               {
                  ret = FALSE;
               }
            }
         }
         else // not a VT call
         {
            call_details->callType = RIL_CALL_TYPE_VOICE;
            if ( TRUE == audio_attrib_valid && 0 == audio_attrib )
            {
               call_details->callSubState = RIL_CALL_SUB_STATE_AUDIO_CONNECTED_SUSPENDED;
            }

            boolean call_domain_set = FALSE;
            if ( call_info_elab_valid )
            {
               call_domain_set = TRUE;
               if (call_info_elab & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CS_DOMAIN)
               {
                  call_details->callDomain = RIL_CALL_DOMAIN_CS;
               }
               else if (call_info_elab & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PS_DOMAIN)
               {
                  call_details->callDomain = RIL_CALL_DOMAIN_PS;
               }
               else if (call_info_elab & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_AUTO_DOMAIN)
               {
                  call_details->callDomain = RIL_CALL_DOMAIN_AUTOMATIC;
               }
               else
               {
                  QCRIL_LOG_DEBUG("did not set call domain in elaboration.");
                  call_domain_set = FALSE;
               }
            }
            if (!call_domain_set)
            {
               switch( call_type )
               {
               case CALL_TYPE_EMERGENCY_IP_V02:
               case CALL_TYPE_VOICE_IP_V02:
                   call_details->callDomain = RIL_CALL_DOMAIN_PS;
                   break;

               default:
                  /* fall back to default voice case */
                  call_details->callDomain = RIL_CALL_DOMAIN_CS;
                  break;
               }
            }
         } // end of "not a VT call"
         QCRIL_LOG_DEBUG( "ril call type = %d, ril call domain = %d, call sub state = %d",
                          call_details->callType, call_details->callDomain,
                          call_details->callSubState );
      }
   }
   else
   {
      ret = FALSE;
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET((int) ret);
   return ret;
} // qcril_qmi_voice_get_atel_call_type_info

/*===========================================================================

  FUNCTION: qcril_qmi_voice_match_modem_call_type

===========================================================================*/
/*!
    @brief
    compare the modem call types provided.

    @return
    TRUE, if call type has matched.
    FALSE, otherwise.
*/
/*=========================================================================*/
boolean qcril_qmi_voice_match_modem_call_type
(
   call_type_enum_v02                    call_type1,
   uint8_t                               audio_attrib_valid1,
   voice_call_attribute_type_mask_v02    audio_attrib1,
   uint8_t                               video_attrib_valid1,
   voice_call_attribute_type_mask_v02    video_attrib1,
   call_type_enum_v02                    call_type2,
   uint8_t                               audio_attrib_valid2,
   voice_call_attribute_type_mask_v02    audio_attrib2,
   uint8_t                               video_attrib_valid2,
   voice_call_attribute_type_mask_v02    video_attrib2
)
{
   boolean result;

   QCRIL_LOG_DEBUG( "call_type1 = %d, audio_attrrib valid/value = %d/%d, video_attrib valid/value = %d/%d",
                    call_type1, audio_attrib_valid1, (int)audio_attrib1, video_attrib_valid1, (int)video_attrib1);

   QCRIL_LOG_DEBUG( "call_type2 = %d, audio_attrrib valid/value = %d/%d, video_attrib valid/value = %d/%d",
                    call_type2, audio_attrib_valid2, (int)audio_attrib2, video_attrib_valid2, (int)video_attrib2);

   if( call_type1 == call_type2 )
   {
      if ( CALL_TYPE_VT_V02 == call_type1 ) // only check video/audio attribute for VT
      {
         if ( ( audio_attrib_valid1 == audio_attrib_valid2 ) &&
              ( ( audio_attrib_valid1 && audio_attrib1 == audio_attrib2 ) || ! audio_attrib_valid1 ) &&
              ( video_attrib_valid1 == video_attrib_valid2 ) &&
              ( ( video_attrib_valid1 && video_attrib1 == video_attrib2 ) || ! video_attrib_valid1 )
            )
         {
            result = TRUE;
         }
         else
         {
            result = FALSE;
         }
      }
      else
      {
         result = TRUE;
      }
   }
   else
   {
      result = FALSE;
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);

   return result;
}

//===========================================================================
// qcril_qmi_voice_ims_send_unsol_ringback_tone
//===========================================================================
void qcril_qmi_voice_ims_send_unsol_ringback_tone(boolean local_ringback_payload)
{
  QCRIL_LOG_FUNC_ENTRY();
  Ims__RingBackTone ring_tone = IMS__RING_BACK_TONE__INIT;
  ring_tone.has_flag = TRUE;
  if ( local_ringback_payload )
  {
    ring_tone.flag = IMS__RING_BACK_TONE__TONE_FLAG__START;
  }
  else
  {
    ring_tone.flag = IMS__RING_BACK_TONE__TONE_FLAG__STOP;
  }

  qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_RINGBACK_TONE, IMS__ERROR__E_SUCCESS, (void *)&ring_tone, sizeof(ring_tone));
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_ims_send_unsol_ringback_tone

//===========================================================================
// qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl
//===========================================================================
void qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl( void )
{
    qcril_qmi_voice_voip_call_info_entry_type* call_info = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    call_info = qcril_qmi_voice_voip_call_info_entries_enum_first();
    while ( NULL != call_info )
    {
      if ( VOICE_INVALID_CALL_ID != call_info->android_call_id &&
           CALL_STATE_END_V02 == call_info->voice_scv_info.call_state &&
           call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_CALL_ENDED_REPORTED &&
           call_info->elaboration & QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_LAST_CALL_FAILURE_REPORTED
         )
      {
        qcril_qmi_voice_voip_destroy_call_info_entry( call_info);
      }

      call_info = qcril_qmi_voice_voip_call_info_entries_enum_next();
    }

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_evaluate_voice_call_obj_cleanup_vcl

//===========================================================================
// qcril_qmi_voice_voip_conf_request_state_reset_vcl
//===========================================================================
void qcril_qmi_voice_ims_conf_req_state_reset_vcl()
{
   qmi_voice_voip_overview.ims_conf_req_state = QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_NO_REQUEST;
   QCRIL_LOG_INFO("state machine reset");
} // qcril_qmi_voice_voip_conf_request_state_reset_vcl

//===========================================================================
// qcril_qmi_voice_voip_conf_request_state_start_vcl
//===========================================================================
void qcril_qmi_voice_ims_conf_req_state_start_vcl()
{
   qmi_voice_voip_overview.ims_conf_req_state = QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_ORIGINATING_CALL_PENDING;
   QCRIL_LOG_INFO("state machine start");
} // qcril_qmi_voice_voip_conf_request_state_start_vcl

//===========================================================================
// qcril_qmi_voice_voip_conf_request_state_start_vcl
//===========================================================================
void qcril_qmi_voice_ims_conf_add_call_req_state_start_vcl()
{
   qmi_voice_voip_overview.ims_conf_req_state = QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_MERGING_CALLS;
   QCRIL_LOG_INFO("state machine start");
} // qcril_qmi_voice_voip_conf_request_state_start_vcl

//===========================================================================
// qcril_qmi_voice_get_voip_conf_call_txn_call_state_vcl
//===========================================================================
qcril_qmi_voice_ims_conf_req_state qcril_qmi_voice_get_ims_conf_call_req_txn_state_vcl()
{
   return qmi_voice_voip_overview.ims_conf_req_state;
} // qcril_qmi_voice_get_voip_conf_call_txn_call_state_vcl

//===========================================================================
// qcril_qmi_voice_set_voip_conf_call_txn_call_state_vcl
//===========================================================================
void qcril_qmi_voice_set_ims_conf_req_txn_state_vcl(qcril_qmi_voice_ims_conf_req_state state)
{
   qmi_voice_voip_overview.ims_conf_req_state = state;
} // qcril_qmi_voice_set_voip_conf_call_txn_call_state_vcl

//===========================================================================
// qcril_qmi_voice_set_voip_conf_call_txn_call_state_to_next_vcl
//===========================================================================
void qcril_qmi_voice_set_ims_conf_call_req_txn_state_to_next_vcl()
{
   qmi_voice_voip_overview.ims_conf_req_state++;
   if (QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_MAX <= qmi_voice_voip_overview.ims_conf_req_state)
   {
      QCRIL_LOG_DEBUG("unexpected call state: %d", qmi_voice_voip_overview.ims_conf_req_state);
      qmi_voice_voip_overview.ims_conf_req_state = QCRIL_QMI_VOICE_IMS_CONF_REQ_STATE_NO_REQUEST;
   }
   QCRIL_LOG_INFO("set voip conf call txn state to: %d", (int) qmi_voice_voip_overview.ims_conf_req_state);
} // qcril_qmi_voice_set_voip_conf_call_txn_call_state_to_next_vcl

//===========================================================================
// qcril_qmi_voice_handle_pil_state_changed
//===========================================================================
void qcril_qmi_voice_handle_pil_state_changed(const qcril_qmi_pil_state* cur_state)
{
   QCRIL_LOG_FUNC_ENTRY();
   qcril_qmi_voice_voip_lock_overview();
   qcril_qmi_voice_info.pil_state.state = cur_state->state;
   if (qmi_ril_voice_is_calls_supressed_by_pil_vcl())
   {
      qcril_qmi_voice_hangup_all_non_emergency_calls_vcl();
   }
   qcril_qmi_voice_voip_unlock_overview();
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_handle_pil_state_changed

//===========================================================================
// qcril_qmi_voice_end_call_internally
//===========================================================================
void qcril_qmi_voice_end_call_internally
(
  qmi_client_type              user_handle,
  unsigned int                 message_id,
  void                        *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
    QCRIL_LOG_FUNC_ENTRY();

    voice_end_call_resp_msg_v02 * qmi_response = (voice_end_call_resp_msg_v02 *) resp_c_struct;
    RIL_Errno ril_req_res;

    QCRIL_NOTUSED(resp_cb_data);
    QCRIL_NOTUSED(resp_c_struct_len);
    QCRIL_NOTUSED(message_id);
    QCRIL_NOTUSED(user_handle);

    QCRIL_LOG_INFO("transp_err: %d", (int) transp_err);
    ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result( transp_err, &qmi_response->resp );
    QCRIL_LOG_INFO(".. ril res %d, qmi res %d", (int) ril_req_res, (int)qmi_response->resp.error );
    qcril_free(qmi_response);

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_end_call_internally

//===========================================================================
// qcril_qmi_voice_hangup_all_non_emergency_calls_vcl
//===========================================================================
void qcril_qmi_voice_hangup_all_non_emergency_calls_vcl()
{
   qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
   boolean has_emergency_call = FALSE;
   boolean has_non_emergency_call = FALSE;

   voice_end_call_req_msg_v02   call_end_req_msg;
   voice_end_call_resp_msg_v02* end_call_resp_msg_ptr = NULL;


   qmi_client_error_type qmi_client_error = QMI_NO_ERR;
   qmi_txn_handle txn_handle;

   QCRIL_LOG_FUNC_ENTRY();

   call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_first();
   while (NULL != call_info_entry)
   {
      if ( (CALL_STATE_DISCONNECTING_V02 != call_info_entry->voice_scv_info.call_state && CALL_STATE_END_V02 != call_info_entry->voice_scv_info.call_state) &&
           !qcril_qmi_voice_is_qmi_call_emergency(&call_info_entry->voice_scv_info)
         )
      {
         // end this call
         end_call_resp_msg_ptr = qcril_malloc( sizeof(*end_call_resp_msg_ptr) );
         if( NULL == end_call_resp_msg_ptr )
         {
            QCRIL_LOG_ERROR("malloc failed");
         }
         else
         {
            memset(&call_end_req_msg, 0, sizeof(call_end_req_msg));
            call_end_req_msg.call_id = call_info_entry->qmi_call_id;

            QCRIL_LOG_INFO("end call with qmi id %d", call_end_req_msg.call_id);
            qmi_client_error = qmi_client_send_msg_async( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_VOICE ),
                                                          QMI_VOICE_END_CALL_REQ_V02,
                                                          (void*) &call_end_req_msg,
                                                          sizeof( call_end_req_msg ),
                                                          (void*) end_call_resp_msg_ptr,
                                                          sizeof( *end_call_resp_msg_ptr ),
                                                          qcril_qmi_voice_end_call_internally,
                                                          NULL,
                                                          &txn_handle
                                                        );
            if (QMI_NO_ERR != qmi_client_error)
            {
               qcril_free( end_call_resp_msg_ptr );
            }
         }
      }
      call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
   }
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_hangup_all_non_emergency_calls_vcl

//===========================================================================
// qmi_ril_voice_is_calls_supressed_by_pil_vcl
//===========================================================================
boolean qmi_ril_voice_is_calls_supressed_by_pil_vcl()
{
   return (QCRIL_QMI_PIL_STATE_OFFLINE == qcril_qmi_voice_info.pil_state.state);
} // qmi_ril_voice_is_calls_supressed_by_pil_vcl

//===========================================================================
// qcril_qmi_voice_ims_send_unsol_radio_state_change
//===========================================================================
void qcril_qmi_voice_ims_send_unsol_radio_state_change(qcril_modem_state_e_type modem_state)
{
  QCRIL_LOG_FUNC_ENTRY();

  Ims__RadioStateChanged ims_unsol_radio_state_change = IMS__RADIO_STATE_CHANGED__INIT;

  ims_unsol_radio_state_change.has_state = TRUE;
  switch ( modem_state )
  {
    case QCRIL_MODEM_STATE_ON:
      ims_unsol_radio_state_change.state = IMS__RADIO_STATE__RADIO_STATE_ON;
      break;

    case QCRIL_MODEM_STATE_OFF:
      ims_unsol_radio_state_change.state = IMS__RADIO_STATE__RADIO_STATE_OFF;
      break;

    default:
      ims_unsol_radio_state_change.state = IMS__RADIO_STATE__RADIO_STATE_UNAVAILABLE;
      break;
  }

  QCRIL_LOG_DEBUG("Modem_state is : %d",ims_unsol_radio_state_change.state);

  qcril_qmi_ims_socket_send(0, IMS__MSG_TYPE__UNSOL_RESPONSE, IMS__MSG_ID__UNSOL_RADIO_STATE_CHANGED, IMS__ERROR__E_SUCCESS, &ims_unsol_radio_state_change, sizeof(ims_unsol_radio_state_change));

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp
//===========================================================================
void qcril_qmi_send_ss_failure_cause_oem_hook_unsol_resp
(
  qmi_sups_errors_enum_v02 sups_failure_cause,
  uint8_t call_id
)
{
    int payload[2] = {0};

    payload[ 0 ] = call_id;
    payload[ 1 ] = sups_failure_cause;

    QCRIL_LOG_INFO("Sending QCRIL_EVT_HOOK_UNSOL_SS_ERR_CODE: call_id - %d sups_failure_cause - %d ", call_id, sups_failure_cause);
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_SS_ERROR_CODE, (char *) payload, sizeof(payload) );

    return;
}

//===========================================================================
// qcril_qmi_voice_send_request_answer
//===========================================================================
RIL_Errno qcril_qmi_voice_send_request_answer(
  const qcril_request_params_type             *const params_ptr,
  int                                         reject_this_call
)
{
   RIL_Errno                                   res;
   uint32                                      user_data;
   qcril_reqlist_public_type                   reqlist_entry;
   voice_answer_call_req_msg_v02               ans_call_req_msg;
   voice_answer_call_resp_msg_v02*             ans_call_resp_msg_ptr = NULL;
   qcril_qmi_voice_voip_call_info_entry_type   *call_info_entry = NULL;
   RIL_Call_Type                               *answer_call_type = params_ptr->data;
   RIL_Call_Details                            answer_call_details;
   qmi_ril_voice_ims_command_exec_oversight_type*                      command_oversight;
   qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type   oversight_cmd_params;
   call_state_enum_v02                         target_call_state;

   QCRIL_LOG_FUNC_ENTRY();

   ans_call_resp_msg_ptr = NULL;
   res = RIL_E_GENERIC_FAILURE;

   do
   {
      call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_PENDING_INCOMING, TRUE );
      if ( NULL == call_info_entry )
      {
         QCRIL_LOG_INFO(".. pending incoming call record entry not found");
         break;
      }
      // Cancel if there is any auto answer timed callback
      if ( QMI_RIL_ZERO != qmi_voice_voip_overview.auto_answer_timer_id )
      {
         QCRIL_LOG_INFO(".. Cancel Auto answer timed callback");
         qcril_cancel_timed_callback( (void*)(uintptr_t) qmi_voice_voip_overview.auto_answer_timer_id );
         qmi_voice_voip_overview.auto_answer_timer_id = QMI_RIL_ZERO;
      }
      qcril_reqlist_default_entry( params_ptr->t,
                                     params_ptr->event_id,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_REQ_AWAITING_CALLBACK,
                                     QCRIL_EVT_NONE,
                                     NULL,
                                     &reqlist_entry );

      reqlist_entry.valid_sub_id = TRUE;
      reqlist_entry.sub_id = call_info_entry->qmi_call_id;
      if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
      {
          QCRIL_LOG_INFO(".. failed to add requrd to req list");
          break;
      }
      memset( &ans_call_req_msg, 0, sizeof( ans_call_req_msg ) );
      ans_call_req_msg.call_id = call_info_entry->qmi_call_id;
      QCRIL_LOG_INFO(".. call id qmi %d", (int) ans_call_req_msg.call_id );

      ans_call_resp_msg_ptr = qcril_malloc( sizeof(*ans_call_resp_msg_ptr) );
      if( NULL == ans_call_resp_msg_ptr )
      {
         QCRIL_LOG_ERROR(".. failed to allocate reponse buffer");
         break;
      }

      if ( QCRIL_EVT_IMS_SOCKET_REQ_ANSWER == params_ptr->event_id )
      {
          Ims__Answer* msg_ptr = (Ims__Answer*) params_ptr->data;
          if (NULL == msg_ptr)
          {
             QCRIL_LOG_ERROR("IMS Answer params_ptr->data is NULL");
             break;
          }
          else
          {
              /* check for these params as they are sent only in case VoIP/VT is supported */
              if( msg_ptr->has_call_type )
              {
                 /* In case of voice call, call type is not expected in answer */
                 if( ( call_info_entry->voice_scv_info.call_type == CALL_TYPE_VT_V02 ) )
                 {
                    if( msg_ptr->call_type == IMS__CALL_TYPE__CALL_TYPE_VT )
                    {
                       ans_call_req_msg.call_type_valid = TRUE;
                       ans_call_req_msg.call_type = CALL_TYPE_VT_V02;

                       ans_call_req_msg.audio_attrib_valid = TRUE;
                       ans_call_req_msg.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

                       ans_call_req_msg.video_attrib_valid = TRUE;
                       ans_call_req_msg.video_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                    }
                    else if ( IMS__CALL_TYPE__CALL_TYPE_VT_TX == msg_ptr->call_type )
                    {
                       ans_call_req_msg.call_type_valid = TRUE;
                       ans_call_req_msg.call_type = CALL_TYPE_VT_V02;

                       ans_call_req_msg.audio_attrib_valid = TRUE;
                       ans_call_req_msg.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

                       ans_call_req_msg.video_attrib_valid = TRUE;
                       ans_call_req_msg.video_attrib = VOICE_CALL_ATTRIB_TX_V02;
                    }
                    else if ( IMS__CALL_TYPE__CALL_TYPE_VT_RX == msg_ptr->call_type )
                    {
                       ans_call_req_msg.call_type_valid = TRUE;
                       ans_call_req_msg.call_type = CALL_TYPE_VT_V02;

                       ans_call_req_msg.audio_attrib_valid = TRUE;
                       ans_call_req_msg.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

                       ans_call_req_msg.video_attrib_valid = TRUE;
                       ans_call_req_msg.video_attrib = VOICE_CALL_ATTRIB_RX_V02;
                    }
                    else
                    {
                       ans_call_req_msg.call_type_valid = TRUE;
                       ans_call_req_msg.call_type = CALL_TYPE_VOICE_IP_V02;
                    }
                    // Cache the user specified call type in answer.
                    call_info_entry->answered_call_type_valid = TRUE;
                    call_info_entry->answered_call_type = msg_ptr->call_type;
                 }
                 QCRIL_LOG_DEBUG("atel ims call type = %d, modem call type = %d", msg_ptr->call_type, ans_call_req_msg.call_type);
              }
              if (msg_ptr->has_presentation)
              {
                 ans_call_req_msg.pi_valid = TRUE;
                 ans_call_req_msg.pi = msg_ptr->presentation;
              }
              qcril_qmi_ims__answer__free_unpacked(msg_ptr, NULL);
          }
      }
      else if( qmi_ril_is_feature_supported( QMI_RIL_FEATURE_VOIP_VT ) )
      {
          /* check for these params as they are sent only in case VoIP/VT is supported */
          if( ( params_ptr->datalen > 0 ) && ( params_ptr->data != NULL ) )
          {
             /* In case of voice call, call type is not expected in answer */
             if( ( call_info_entry->voice_scv_info.call_type == CALL_TYPE_VT_V02 ) )
             {
                if( *answer_call_type == RIL_CALL_TYPE_VT )
                {
                   ans_call_req_msg.call_type_valid = TRUE;
                   ans_call_req_msg.call_type = CALL_TYPE_VT_V02;
                   ans_call_req_msg.audio_attrib_valid = TRUE;
                   ans_call_req_msg.audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);

                   ans_call_req_msg.video_attrib_valid = TRUE;
                   ans_call_req_msg.video_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                }
                else
                {
                   ans_call_req_msg.call_type_valid = TRUE;
                   ans_call_req_msg.call_type = CALL_TYPE_VOICE_IP_V02;
                }
             }
             QCRIL_LOG_DEBUG("atel call type = %d, modem call type = %d", *answer_call_type, ans_call_req_msg.call_type);
          }
      }

      user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );

      if (TRUE == reject_this_call)
      {
          ans_call_req_msg.reject_call_valid = TRUE;
          ans_call_req_msg.reject_call = TRUE;
      }

      if ( RIL_REQUEST_ANSWER == params_ptr->event_id ||
              QCRIL_EVT_IMS_SOCKET_REQ_ANSWER == params_ptr->event_id ||
              RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND == params_ptr->event_id )
      {
          command_oversight = qmi_ril_voice_ims_create_command_oversight( params_ptr->t, params_ptr->event_id , TRUE );
          if ( NULL != command_oversight )
          {
              target_call_state = CALL_STATE_CONVERSATION_V02;
              if ( RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND == params_ptr->event_id )
              {
                 target_call_state = CALL_STATE_END_V02;
              }
              qmi_ril_voice_ims_command_oversight_add_call_link( command_oversight,
                                                             QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID,
                                                             QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NONE,
                                                             call_info_entry->qmi_call_id,
                                                             target_call_state );

              memset( &oversight_cmd_params, 0, sizeof( oversight_cmd_params ) );
              oversight_cmd_params.locator.command_oversight = command_oversight;

              qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_COMMENCE_AWAIT_RESP_IND,
                                                            QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                                            &oversight_cmd_params );
          } // if ( NULL != command_oversight )
      }

      // Send QMI VOICE ANSWER CALL REQ
      if ( qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_ANSWER_CALL_REQ_V02,
                                          &ans_call_req_msg,
                                          sizeof(ans_call_req_msg),
                                          ans_call_resp_msg_ptr,
                                          sizeof(*ans_call_resp_msg_ptr),
                                          (void*)(uintptr_t)user_data) != E_SUCCESS )
      {
          QCRIL_LOG_INFO(".. failed to post qmi answer message");
          break;
      }

      if (FALSE == reject_this_call)
      {
          call_info_entry->elaboration |= QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_ANSWERING_CALL;
#ifndef QMI_RIL_UTF
          qcril_am_handle_event(qcril_qmi_voice_get_answer_am_event(call_info_entry), NULL);
#endif
      }

      res = RIL_E_SUCCESS;
   } while (FALSE);

   if ((RIL_E_SUCCESS != res) && (NULL != ans_call_resp_msg_ptr))
   {
       qcril_free (ans_call_resp_msg_ptr);
   }
   QCRIL_LOG_FUNC_RETURN();
   return res;
} // qcril_qmi_voice_send_request_answer

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_oem_hook_reject_incoming_call_cause_21

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_REJECT_INCOMING_CALL_CAUSE_21
*/
/*=========================================================================*/
void qcril_qmi_voice_oem_hook_reject_incoming_call_cause_21
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr
)
{
    RIL_Errno res = RIL_E_GENERIC_FAILURE;
    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED(ret_ptr);
    qcril_qmi_voice_voip_lock_overview();
    res = qcril_qmi_voice_send_request_answer (params_ptr, TRUE);
    if (RIL_E_SUCCESS != res)
    {
        qcril_send_empty_payload_request_response(
                        QCRIL_DEFAULT_INSTANCE_ID,
                        params_ptr->t,
                        params_ptr->event_id,
                        res);
    }
    qcril_qmi_voice_voip_unlock_overview();
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_oem_hook_reject_incoming_call_cause_21

//===========================================================================
// qmi_ril_voice_ims_command_oversight_timeout_handler
//===========================================================================
void qmi_ril_voice_ims_command_oversight_timeout_handler(void * param)
{
  qmi_ril_voice_ims_command_exec_oversight_type* command_oversight;
  qmi_ril_voice_ims_command_exec_oversight_type* iter;
  uint32                                         cause_tmr;
  qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type
                                                  event_params;

  QCRIL_LOG_FUNC_ENTRY();

  cause_tmr = (uint32)(uintptr_t)param;

  qcril_qmi_voice_voip_lock_overview();

  // find expired
  command_oversight = NULL;
  iter = qmi_ril_voice_ims_command_oversight_first();
  while ( NULL != iter && NULL == command_oversight )
  {
    if ( cause_tmr == iter->timeout_control_timer_id )
    {
      command_oversight = iter;
    }
    else
    {
      iter = qmi_ril_voice_ims_command_oversight_next();
    }
  } // while ( NULL != iter && NULL == command_oversight )

  if ( NULL != command_oversight )
  {
    command_oversight->timeout_control_timer_id = QMI_RIL_ZERO;

    memset( &event_params, 0, sizeof( event_params ) );
    event_params.locator.command_oversight = command_oversight;

    (void)qmi_ril_voice_ims_command_oversight_handle_event( QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_TIME_OUT, QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ, &event_params );
  }

  qcril_qmi_voice_voip_unlock_overview();

  QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_ims_command_oversight_timeout_handler

//===========================================================================
// qmi_ril_voice_ims_create_command_oversight
//===========================================================================
qmi_ril_voice_ims_command_exec_oversight_type* qmi_ril_voice_ims_create_command_oversight( RIL_Token token,  int android_request_id, int launch_timout_control )
{
  struct timeval timeout_delay;

  qmi_ril_voice_ims_command_exec_oversight_type* command_oversight;

  command_oversight = qcril_malloc( sizeof( *command_oversight ) );

  if ( NULL != command_oversight )
  {
    command_oversight->token              = token;
    command_oversight->android_request_id = android_request_id;

    if ( launch_timout_control )
    { // launch timer
      memset( &timeout_delay, 0, sizeof( timeout_delay ) );
      timeout_delay.tv_usec = QMI_RIL_ZERO;

      switch ( android_request_id )
      {
        case RIL_REQUEST_DIAL:
          timeout_delay.tv_sec = 72;
          break;

        case RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        case RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND:
          timeout_delay.tv_sec = 72;
          break;

        case QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER:
          timeout_delay.tv_sec = 90;
          break;

        default:
          timeout_delay.tv_sec = 1; // 1 second by default
          break;
      }
      qcril_setup_timed_callback(   QCRIL_DEFAULT_INSTANCE_ID,
                                    QCRIL_DEFAULT_MODEM_ID,
                                    qmi_ril_voice_ims_command_oversight_timeout_handler,
                                    &timeout_delay,
                                    &command_oversight->timeout_control_timer_id );

    }
    QCRIL_LOG_DEBUG( "cmd oversight created obj %p android req %d, token %"PRIdPTR", under timeout ctrl %d",
                     command_oversight, android_request_id, token, command_oversight->timeout_control_timer_id );

    command_oversight->next                           = qmi_voice_voip_overview.command_exec_oversight_root;
    qmi_voice_voip_overview.command_exec_oversight_root = command_oversight;
  }
  else
  {
    QCRIL_LOG_ERROR("cmd oversight FAILED to create obj: no resources");
  }

  qmi_ril_voice_ims_command_oversight_dump( command_oversight );

  return command_oversight;
} // qmi_ril_voice_ims_create_command_oversight
//===========================================================================
// qmi_ril_voice_ims_destroy_command_oversight
//===========================================================================
void qmi_ril_voice_ims_destroy_command_oversight( qmi_ril_voice_ims_command_exec_oversight_type* command_oversight )
{
  qmi_ril_voice_ims_command_exec_oversight_type** cur_p;
  qmi_ril_voice_ims_command_exec_oversight_type*  cur;
  if ( NULL != command_oversight )
  {
    if ( QMI_RIL_ZERO != command_oversight->timeout_control_timer_id )
    {
      qcril_cancel_timed_callback( (void*)(uintptr_t) command_oversight->timeout_control_timer_id );
      command_oversight->timeout_control_timer_id = QMI_RIL_ZERO;
    }

    cur_p = &qmi_voice_voip_overview.command_exec_oversight_root;
    while ( NULL != ( cur = *cur_p ) && ( command_oversight != cur ) )
    {
      cur_p = &cur->next;
    }
    if ( NULL != cur )
    {
      *cur_p = cur->next;
    }

    if ( command_oversight == qmi_voice_voip_overview.command_exec_oversight_current )
    {
      qmi_voice_voip_overview.command_exec_oversight_current = command_oversight->next;
    }

    qcril_free( command_oversight );
  }
} // qmi_ril_voice_ims_destroy_command_oversight

//===========================================================================
// qmi_ril_voice_ims_command_oversight_add_call_link
//===========================================================================
void qmi_ril_voice_ims_command_oversight_add_call_link(  qmi_ril_voice_ims_command_exec_oversight_type*     command_oversight,
                                                         qmi_ril_voice_ims_command_exec_oversight_call_obj_linkage_e_type link_type,
                                                         qcril_qmi_voice_voip_call_info_elaboration_type    elaboration_pattern,
                                                         int                                                call_id,
                                                         call_state_enum_v02                                target_call_state
                                                       )
{
  qmi_ril_voice_ims_command_exec_oversight_link_type * link;

  if ( NULL != command_oversight && ( command_oversight->nof_impacted < QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_MAX_NOF_CALLS ) )
  {
    link = &command_oversight->impacted [ command_oversight->nof_impacted ];
    command_oversight->nof_impacted++;

    link->linkage_type = link_type;
    switch ( link_type )
    {
      case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_NONE:
        // no action
        break;

      case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID:
        link->linkage.qmi_call_id = call_id;
        break;

      case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ANDROID_CALL_ID:
        link->linkage.android_call_id = call_id;
        break;

      case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN:
        link->linkage.elaboration_pattern                               = elaboration_pattern;
        break;

      default:
        break;
    }
    link->target_call_state = target_call_state;
  }

  qmi_ril_voice_ims_command_oversight_dump( command_oversight );
} // qmi_ril_voice_ims_command_oversight_add_call_link


//===========================================================================
// qmi_ril_voice_ims_command_oversight_first
//===========================================================================
qmi_ril_voice_ims_command_exec_oversight_type* qmi_ril_voice_ims_command_oversight_first()
{
  qmi_ril_voice_ims_command_exec_oversight_type*  command_oversight;

  command_oversight                                       = qmi_voice_voip_overview.command_exec_oversight_root;
  qmi_voice_voip_overview.command_exec_oversight_current  = qmi_voice_voip_overview.command_exec_oversight_root;

  return command_oversight;
}  // qmi_ril_voice_ims_command_oversight_first
//===========================================================================
// qmi_ril_voice_ims_command_oversight_next
//===========================================================================
qmi_ril_voice_ims_command_exec_oversight_type* qmi_ril_voice_ims_command_oversight_next()
{
  qmi_ril_voice_ims_command_exec_oversight_type*  command_oversight;

  command_oversight = qmi_voice_voip_overview.command_exec_oversight_current;
  if ( NULL != qmi_voice_voip_overview.command_exec_oversight_current )
  {
    qmi_voice_voip_overview.command_exec_oversight_current = qmi_voice_voip_overview.command_exec_oversight_current->next;
    command_oversight                                      = qmi_voice_voip_overview.command_exec_oversight_current;
  }
  else
  {
    command_oversight = NULL;
  }
  return command_oversight;
} // qmi_ril_voice_ims_command_oversight_next

//===========================================================================
// qmi_ril_voice_ims_find_command_oversight_by_token
//===========================================================================
qmi_ril_voice_ims_command_exec_oversight_type* qmi_ril_voice_ims_find_command_oversight_by_token( RIL_Token token )
{
  qmi_ril_voice_ims_command_exec_oversight_type*  command_oversight;
  qmi_ril_voice_ims_command_exec_oversight_type*  iter;

  command_oversight = NULL;

  iter = qmi_ril_voice_ims_command_oversight_first();
  while ( ( NULL != iter ) && ( NULL == command_oversight ) )
  {
    if ( token == iter->token )
    {
      command_oversight = iter;
    }
    else
    {
      iter = qmi_ril_voice_ims_command_oversight_next();
    }
  }

  return command_oversight;
} // qmi_ril_voice_ims_find_command_oversight_by_token

//===========================================================================
// qmi_ril_voice_ims_command_oversight_handle_event
//===========================================================================
int qmi_ril_voice_ims_command_oversight_handle_event(   qmi_ril_voice_ims_command_exec_intermediates_event_e_type event,
                                                         qmi_ril_voice_ims_command_exec_oversight_call_obj_linkage_e_type link_type,
                                                         qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type* params
                                                       )
{
  qmi_ril_voice_ims_command_exec_oversight_type*              iter;
  int                                                         idx;
  qmi_ril_voice_ims_command_exec_oversight_link_type *        link;
  int                                                         need_to_conclude_cmd;
  int                                                         is_match;
  qmi_ril_voice_ims_command_exec_intermediates_state_e_type   new_link_state;
  int                                                         need_set_new_link_state;
  int                                                         any_link_changed;
  qmi_ril_voice_ims_command_exec_intermediates_state_e_type   generalized_completion_state;
  RIL_Errno                                                   ril_req_res;
  qcril_reqlist_public_type                                   atel_request;
  IxErrnoType                                                 atel_request_find_res;
  qcril_request_resp_params_type                              atel_resp;
  int                                                         any_action_performed;
  qcril_qmi_voice_voip_call_info_entry_type*                  call_entry;

  QCRIL_LOG_FUNC_ENTRY();

  any_action_performed = FALSE;

  iter = qmi_ril_voice_ims_command_oversight_first();
  while ( NULL != iter )
  {
    qmi_ril_voice_ims_command_oversight_dump( iter );

    link                 = iter->impacted;
    any_link_changed     = FALSE;
    for ( idx = 0; idx < iter->nof_impacted; idx++ )
    {
      is_match = FALSE;
      switch ( link_type )
      {
        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID:
          if ( QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID == link->linkage_type && link->linkage.qmi_call_id == params->locator.qmi_call_id )
          {
            is_match = TRUE;
          }
          break;

        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ANDROID_CALL_ID:
          if ( QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ANDROID_CALL_ID == link->linkage_type && link->linkage.android_call_id == params->locator.android_call_id )
          {
            is_match = TRUE;
          }
          break;

        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN:
          if ( QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN == link->linkage_type &&
               ( link->linkage.elaboration_pattern & params->locator.elaboration_pattern ) == link->linkage.elaboration_pattern
             )
          {
            call_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( link->linkage.elaboration_pattern, TRUE );
            if ( NULL != call_entry )
            {
              is_match = TRUE;
            }
          }
          break;

        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ:
          if ( params->locator.command_oversight == iter )
          {
            is_match = TRUE;
          }
          break;

        default:
          // no action
          break;
      } // switch ( link_type )

      need_set_new_link_state = FALSE;
      new_link_state          = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_NONE;
      if ( is_match )
      { // found record, absorb event
        switch ( event )
        {
          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_COMMENCE_AWAIT_RESP_IND:
            new_link_state          = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_AWAITING_RESP_AND_IND;
            need_set_new_link_state = TRUE;
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_SUCCESS:
            switch ( link->exec_state )
            {
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_AWAITING_RESP_AND_IND:
                new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_RESP_AWAITING_IND;
                need_set_new_link_state = TRUE;
                break;

              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AWAITING_RESP:
                new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION;
                need_set_new_link_state = TRUE;
                break;

              default:
                break;
            }
            if ( 0 == iter->successful_response_payload_len && params->successful_response_payload_len > 0 && need_set_new_link_state )
            {
              iter->successful_response_payload_len = params->successful_response_payload_len;
              memcpy( (void *)&iter->successful_response_payload, (void *)&params->successful_response_payload, params->successful_response_payload_len );
            }
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_FAILURE:
            switch ( link->exec_state )
            {
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_AWAITING_RESP_AND_IND:
                new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION;
                need_set_new_link_state = TRUE;
                break;

              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AWAITING_RESP:
                new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION;
                need_set_new_link_state = TRUE;
                break;

              default:
                break;
            }
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_IND:
            if ( link->target_call_state == params->new_call_state )
            {
              switch ( link->exec_state )
              {
                case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_AWAITING_RESP_AND_IND:
                  new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AWAITING_RESP;
                  need_set_new_link_state = TRUE;
                  break;

                case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_RESP_AWAITING_IND:
                  new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION;
                  need_set_new_link_state = TRUE;
                  break;

                default:
                  break;
              } // ( link->exec_state )
            } // if ( link->target_call_state == params->new_call_state )
            if ( 0 == iter->successful_response_payload_len && params->successful_response_payload_len > 0 && need_set_new_link_state )
            {
              iter->successful_response_payload_len = params->successful_response_payload_len;
              memcpy( (void *)&iter->successful_response_payload, (void *)&params->successful_response_payload, params->successful_response_payload_len );
            }
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_CALL_ENDED:
            if ( CALL_STATE_END_V02 == link->target_call_state )
            {
              switch ( link->exec_state )
              {
                case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_AWAITING_RESP_AND_IND:
                  new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AWAITING_RESP;
                  need_set_new_link_state = TRUE;
                  break;

                case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_RESP_AWAITING_IND:
                  new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION;
                  need_set_new_link_state = TRUE;
                  break;

                default:
                  new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_CALL_ENDED_READY_FOR_COMPLETION;
                  need_set_new_link_state = TRUE;
                  break;
              }
            }
            else
            {
              new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_CALL_ENDED_READY_FOR_COMPLETION;
              need_set_new_link_state = TRUE;
            }
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_TIME_OUT:
            switch ( link->exec_state )
            {
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_CALL_ENDED_READY_FOR_COMPLETION:               // fallthrough
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION:    // fallthrough
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION:
                // no change
                break;

              default:
                if ( iter == params->locator.command_oversight )
                {
                  new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_TIMEOUT_READY_FOR_COMPLETION;
                  need_set_new_link_state = TRUE;
                }
                break;
            }
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_ABANDON:
            new_link_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION;
            need_set_new_link_state = TRUE;
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_NONE:                // fallthrough
          default:
            // no action
            break;
        } // switch ( event )
        if ( need_set_new_link_state )
        {
          link->exec_state      = new_link_state;
          any_link_changed      = TRUE;
          any_action_performed  = TRUE;
        }
      } // if ( is_match ) - call links match with params
      link++;
    } // for ( idx = 0; idx < iter->nof_impacted; idx++ )

    need_to_conclude_cmd = FALSE;
    if ( any_link_changed )
    { // assess if all links awaiting for conclusion
      need_to_conclude_cmd = TRUE;
      generalized_completion_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION;
      link = iter->impacted;
      // get to one result cause or abandon conclusion
      for ( idx = 0; idx < iter->nof_impacted ; idx++ )
      {
        switch ( link->exec_state )
        {
          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION:
            // we may conclude, keeping need_to_conclude_cmd set
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION:
            generalized_completion_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION;
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_TIMEOUT_READY_FOR_COMPLETION:
            generalized_completion_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_TIMEOUT_READY_FOR_COMPLETION;
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_CALL_ENDED_READY_FOR_COMPLETION:
            switch ( generalized_completion_state )
            {
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_TIMEOUT_READY_FOR_COMPLETION:                  // fallthrough
              case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION:
                // no change
                break;

              default:
                generalized_completion_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_CALL_ENDED_READY_FOR_COMPLETION;
                break;
            }
            // we may conclude, keeping need_to_conclude_cmd set
            break;

          case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION:
            generalized_completion_state = QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION;
            need_to_conclude_cmd = TRUE;
            break;

          default:
            if ( QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION != generalized_completion_state )
            {
              need_to_conclude_cmd = FALSE; // not ready for conclusion
            }
            break;
        }
        link++;
      } // for ( idx = 0; idx < iter->nof_impacted && need_to_conclude_cmd ; idx++ )
      if ( need_to_conclude_cmd )
      { // do conclude - need to respond and conclude the pending oversight
        if ( QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION != generalized_completion_state )
        {
          switch ( generalized_completion_state )
          {
            case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_TIMEOUT_READY_FOR_COMPLETION:
              ril_req_res = RIL_E_GENERIC_FAILURE;
              break;

            case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_FAILURE_RESP_READY_FOR_COMPLETION:
              ril_req_res = RIL_E_GENERIC_FAILURE;
              break;

            case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_CALL_ENDED_READY_FOR_COMPLETION:
            case QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_RECEIVED_IND_AND_RESP_READY_FOR_COMPLETION:
            default:
              if (iter->successful_response_payload_len > 0)
              {
                ril_req_res = iter->successful_response_payload;
              }
              else
              {
                ril_req_res = RIL_E_SUCCESS;
              }
              break;
          }

          atel_request_find_res = qcril_reqlist_query( QCRIL_DEFAULT_INSTANCE_ID, iter->token, &atel_request );
          if ( E_SUCCESS == atel_request_find_res )
          {
            qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                       atel_request.t,
                                                       atel_request.request,
                                                       ril_req_res );

          } // if ( E_SUCCESS == atel_request_find_res )
        } // if ( QMI_RIL_VOICE_IMS_EXEC_INTERMED_STATE_ABANDONED_READY_FOR_COMPLETION != generalized_completion_state )

        // reset elaboration from call obj
        link                 = iter->impacted;
        for ( idx = 0; idx < iter->nof_impacted; idx++ )
        {
          if ( QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN == link->linkage_type )
          {
            call_entry = qcril_qmi_voice_voip_find_call_info_entry_by_elaboration( link->linkage.elaboration_pattern, TRUE );
            if ( NULL != call_entry )
            {
              call_entry->elaboration &= ~link->linkage.elaboration_pattern;
            }
          }
          link++;
        } // for ( idx = 0; idx < iter->nof_impacted; idx++ )

        qmi_ril_voice_ims_destroy_command_oversight( iter );
      } // if ( need_to_conclude_cmd )

    } // if ( any_link_changed )

    if ( !need_to_conclude_cmd )
    {
      qmi_ril_voice_ims_command_oversight_dump( iter );
    }
    else
    {
      QCRIL_LOG_INFO("command concluded");
    }

    iter = qmi_ril_voice_ims_command_oversight_next();

  } // while ( NULL != iter ), iteration through oversight command objects

  QCRIL_LOG_FUNC_RETURN_WITH_RET( any_action_performed );

  return any_action_performed;
} // qmi_ril_voice_ims_command_oversight_handle_event

//===========================================================================
// qmi_ril_voice_ims_command_oversight_dump
//===========================================================================
void qmi_ril_voice_ims_command_oversight_dump( qmi_ril_voice_ims_command_exec_oversight_type* command_oversight )
{
  int                                                   idx;
  qmi_ril_voice_ims_command_exec_oversight_link_type*   link;
  char                                                  link_info_buf[ QCRIL_MAX_LOG_MSG_SIZE ];

  if ( NULL != command_oversight )
  {
    QCRIL_LOG_INFO("oversight obj android request id %d, tokenj id %d, timer %d, nof_impacted %d",
                   command_oversight->android_request_id,
                   qcril_log_get_token_id ( command_oversight->token ),
                   command_oversight->timeout_control_timer_id,
                   command_oversight->nof_impacted );

    link = command_oversight->impacted;
    for ( idx = 0; idx < command_oversight->nof_impacted; idx++ )
    {
      switch ( link->linkage_type )
      {
        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID:
          snprintf( link_info_buf, QCRIL_MAX_LOG_MSG_SIZE, "qmi call id %d", link->linkage.qmi_call_id );
          break;

        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ANDROID_CALL_ID:
          snprintf( link_info_buf, QCRIL_MAX_LOG_MSG_SIZE, "atel call id %d", link->linkage.android_call_id );
          break;

        case QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_ELABORATION_PATTERN:
          snprintf( link_info_buf, QCRIL_MAX_LOG_MSG_SIZE, "elaboration %x-%x", (unsigned int)(link->linkage.elaboration_pattern >> 32), (unsigned int)(link->linkage.elaboration_pattern & 0xFFFFFFUL) );
          break;

        default:
          snprintf( link_info_buf, QCRIL_MAX_LOG_MSG_SIZE, "no info" );
          break;
      }
      QCRIL_LOG_INFO(".link#%d, state %d, %s, exp c-state %d",
                     idx,
                     link->exec_state,
                     link_info_buf,
                     link->target_call_state
                     );

      link++;
    }
  }
} // qmi_ril_voice_ims_command_oversight_dump

void qcril_qmi_voice_ims_dial_call_handler (qcril_timed_callback_handler_params_type *param)
{
   qcril_qmi_voice_request_dial (param->custom_param, NULL);
   if (NULL != param->custom_param)
   {
       qcril_free (param->custom_param);
   }
}

void qcril_qmi_voice_process_for_ims_dial
(
   void *data,
   size_t datalen,
   RIL_Token t
)
{
   int res = 0;
   qcril_request_params_type *params_ptr = NULL;
   do
   {
      params_ptr = qcril_malloc (sizeof (*params_ptr));
      if (NULL == params_ptr)
      {
         QCRIL_LOG_ERROR ("qcril_malloc failed");
         if (NULL != data)
         {
             qcril_qmi_ims__dial__free_unpacked(data, NULL);
         }
         break;
      }
      params_ptr->instance_id = QCRIL_DEFAULT_INSTANCE_ID;
      params_ptr->modem_id = QCRIL_DEFAULT_MODEM_ID;
      params_ptr->event_id = (int) QCRIL_EVT_IMS_SOCKET_REQ_DIAL;
      params_ptr->data = data;
      params_ptr->datalen = datalen;
      params_ptr->t = t;
      res = qcril_setup_timed_callback_ex_params (QCRIL_DEFAULT_INSTANCE_ID,
                                            QCRIL_DEFAULT_MODEM_ID,
                                            qcril_qmi_voice_ims_dial_call_handler,
                                            (void *) params_ptr,
                                            NULL,
                                            NULL);
      if (RIL_E_SUCCESS != res)
      {
          QCRIL_LOG_ERROR ("could not post ims dial request to timed callback. res = %d", res);
          qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, res);
          if (NULL != data)
          {
             qcril_qmi_ims__dial__free_unpacked(data, NULL);
          }
          if (NULL != params_ptr)
          {
              qcril_free(params_ptr);
          }
      }
   } while (FALSE);
}

/*===========================================================================

  FUNCTION:  qcril_qmi_voice_request_ims_set_supp_srv_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS, and calls appropriate
    function based on the SuppSvcFacilityType and SuppSvcOperationType.
    Function handler table defined by set_supp_srv_status_handler_table[][].

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_ims_set_supp_srv_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  /* IMS SuppSvcRequest message pointer */
  Ims__SuppSvcRequest *ims_in_data_ptr;

  /* Operation type (activate, deactivate, query) from SuppSvcRequest message */
  Ims__SuppSvcOperationType operation_type;

  /* Facility type (CLIP, COLP, BA*) from SuppSvcRequest message */
  Ims__SuppSvcFacilityType facility_type;

  QCRIL_LOG_FUNC_ENTRY();

  if ( NULL == params_ptr)
  {
    QCRIL_LOG_INFO( "params_ptr is NULL" );
  }
  else if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
  {
    QCRIL_LOG_INFO( "params_ptr->datalen == 0 or params_ptr is NULL" );
    qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                               params_ptr->t,
                                               params_ptr->event_id,
                                               RIL_E_GENERIC_FAILURE );
  }
  else
  {
    ims_in_data_ptr = ( Ims__SuppSvcRequest * )params_ptr->data;
    if ( ims_in_data_ptr->has_operationtype )
    {
      operation_type = ( Ims__SuppSvcOperationType )ims_in_data_ptr->operationtype;
      QCRIL_LOG_INFO( "operation_type: %d", operation_type );

      if ( ims_in_data_ptr->has_facilitytype )
      {
        facility_type = ( Ims__SuppSvcFacilityType )ims_in_data_ptr->facilitytype;
        QCRIL_LOG_INFO( "facility_type: %d", facility_type );

        if ( IMS__SUPP_SVC_FACILITY_TYPE__MAX >= facility_type ||
          IMS__SUPP_SVC_FACILITY_TYPE__MIN <= facility_type )
        {

          switch ( operation_type )
          {
            case   IMS__SUPP_SVC_OPERATION_TYPE__ACTIVATE:
            case   IMS__SUPP_SVC_OPERATION_TYPE__DEACTIVATE:   /* Fall through */
              {
                ( *supp_srv_status_set_handler_table\
                  [facility_type] )( params_ptr, ret_ptr );
                break;
              }

            case  IMS__SUPP_SVC_OPERATION_TYPE__QUERY:
              {
                ( *supp_srv_status_query_handler_table\
                  [facility_type] )( params_ptr, ret_ptr );
                break;
              }

          case IMS__SUPP_SVC_OPERATION_TYPE__REGISTER:
          case IMS__SUPP_SVC_OPERATION_TYPE__ERASURE:
              if (IMS__SUPP_SVC_FACILITY_TYPE__FACILITY_BS_MT == facility_type)
              {
                  qcril_qmi_voice_request_set_supp_svc(params_ptr, ret_ptr);
              }
              else
              {
                  QCRIL_LOG_INFO( "Unsupported facility type %d for reg or erase", facility_type );
                  qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                             params_ptr->t,
                                                             params_ptr->event_id,
                                                             RIL_E_GENERIC_FAILURE );
              }
              break;

            default:
              {
                QCRIL_LOG_INFO( "Unsupported operation type %d", operation_type );
                qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                           params_ptr->t,
                                                           params_ptr->event_id,
                                                           RIL_E_GENERIC_FAILURE );
              }
          }
        }
        else
        {
          QCRIL_LOG_INFO( "Unsupported facilityType %d", facility_type );
          qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                     params_ptr->t,
                                                     params_ptr->event_id,
                                                     RIL_E_GENERIC_FAILURE );
        }
      }
      else
      {
        QCRIL_LOG_INFO( "facilityType is not present" );
        qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                   params_ptr->t,
                                                   params_ptr->event_id,
                                                   RIL_E_GENERIC_FAILURE );
      }
    }
    else
    {
      QCRIL_LOG_INFO( "operationType is not present" );

      qcril_send_empty_payload_request_response( QCRIL_DEFAULT_INSTANCE_ID,
                                                 params_ptr->t,
                                                 params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE );
    }
  }

  if (ims_in_data_ptr)
  {
    qcril_qmi_ims__supp_svc_request__free_unpacked(ims_in_data_ptr, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
}

boolean qcril_qmi_voice_has_specific_call(qcril_qmi_voice_call_filter filter)
{
    boolean ret = FALSE;

    if (filter)
    {
        qcril_qmi_voice_voip_lock_overview();
        qcril_qmi_voice_voip_call_info_entry_type *call_info_entry =
            qcril_qmi_voice_voip_call_info_entries_enum_first();

        while ( NULL != call_info_entry )
        {
            if (filter(call_info_entry))
            {
                ret = TRUE;
                break;
            }
            call_info_entry = qcril_qmi_voice_voip_call_info_entries_enum_next();
        }
        qcril_qmi_voice_voip_unlock_overview();
    }

    return ret;
}
/*=========================================================================
  FUNCTION:  qcril_qmi_voice_reboot_cleanup

===========================================================================*/
/*!
    @brief
    Cleans up globals for reboot
*/
/*=========================================================================*/
int qcril_qmi_voice_reboot_cleanup()
{
  qmi_ril_voice_is_voice_calls_supressed  = FALSE;

  feature_subaddress_support = 1;
  reject_cause_21_supported = FALSE;

  return 0;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_request_call_deflection

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_call_deflection
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  Ims__DeflectCall                          *ims_deflect_call_ptr = NULL;
  const char*                               conn_uri = NULL;
  qcril_reqlist_public_type                 reqlist_entry;
  voice_manage_ip_calls_req_msg_v02         manage_ip_calls_req_msg;
  voice_manage_ip_calls_resp_msg_v02        *manage_ip_calls_resp_msg_ptr = NULL;
  qcril_qmi_voice_voip_call_info_entry_type *call_info_entry = NULL;
  qmi_client_error_type                     client_error;
  uint32_t                                  conn_index;
  uint32                                    user_data;
  RIL_Errno                                 ril_err = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_NOTUSED(ret_ptr);

  if(NULL == params_ptr)
  {
    QCRIL_LOG_ERROR("params_ptr is NULL");
    return;
  }

  do
  {
    if(NULL == params_ptr->data)
    {
      QCRIL_LOG_ERROR("params_ptr->data is NULL");
      break;
    }

    ims_deflect_call_ptr = (Ims__DeflectCall *)params_ptr->data;
    if((ims_deflect_call_ptr->has_conn_index) && (ims_deflect_call_ptr->number))
    {
      conn_index = ims_deflect_call_ptr->conn_index;
      QCRIL_LOG_INFO("conn_index recieved: %d", conn_index);

      conn_uri = ims_deflect_call_ptr->number;
      QCRIL_LOG_INFO("Deflect to conn_uri: %s", conn_uri);
    }
    else
    {
      QCRIL_LOG_ERROR("Request has no valid conn_index or conn_uri");
      break;
    }

    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_call_android_id(conn_index);
    if(NULL == call_info_entry)
    {
       QCRIL_LOG_ERROR("Failed to find call entry, aborting!");
       break;
    }

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_CALLBACK,
                                 QCRIL_EVT_NONE,
                                 NULL,
                                 &reqlist_entry );

    reqlist_entry.valid_sub_id = TRUE;
    reqlist_entry.sub_id = call_info_entry->qmi_call_id;
    if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry ) != E_SUCCESS )
    {
      QCRIL_LOG_INFO(".. failed to add requrd to req list");
      break;
    }

    manage_ip_calls_resp_msg_ptr = qcril_malloc(sizeof(*manage_ip_calls_resp_msg_ptr));
    if(NULL == manage_ip_calls_resp_msg_ptr)
    {
      QCRIL_LOG_ERROR("qcril_malloc failed");
      break;
    }

    memset( &manage_ip_calls_req_msg, 0, sizeof(manage_ip_calls_req_msg) );
    manage_ip_calls_req_msg.sups_type = VOIP_SUPS_TYPE_CALL_DEFLECTION_V02;
    manage_ip_calls_req_msg.call_id_valid = TRUE;
    manage_ip_calls_req_msg.call_id = call_info_entry->qmi_call_id;
    manage_ip_calls_req_msg.sip_uri_valid = TRUE;
    memcpy( manage_ip_calls_req_msg.sip_uri, conn_uri, strlen(conn_uri) );

    user_data = QCRIL_COMPOSE_USER_DATA( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry.req_id );
    client_error = qcril_qmi_client_send_msg_async ( QCRIL_QMI_CLIENT_VOICE,
                                                   QMI_VOICE_MANAGE_IP_CALLS_REQ_V02,
                                                   &manage_ip_calls_req_msg,
                                                   sizeof(manage_ip_calls_req_msg),
                                                   manage_ip_calls_resp_msg_ptr,
                                                   sizeof(*manage_ip_calls_resp_msg_ptr),
                                                   (void*)(uintptr_t)user_data );

    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(client_error, NULL);
  } while ( FALSE );

  if(ril_err != RIL_E_SUCCESS)
  {
    qcril_qmi_ims_socket_send(params_ptr->t, IMS__MSG_TYPE__RESPONSE, IMS__MSG_ID__REQUEST_DEFLECT_CALL, qcril_qmi_ims_map_ril_error_to_ims_error(ril_err), NULL, 0);
    if(manage_ip_calls_resp_msg_ptr)
    {
      qcril_free(manage_ip_calls_resp_msg_ptr);
    }
  }

  if(ims_deflect_call_ptr)
  {
    qcril_qmi_ims__deflect_call__free_unpacked(ims_deflect_call_ptr, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return;
}

//===========================================================================
// qcril_qmi_voice_set_audio_call_type
//===========================================================================
void qcril_qmi_voice_set_audio_call_type(const voice_call_info2_type_v02* iter_call_info, qcril_qmi_voice_voip_call_info_entry_type *call_info_entry)
{
   call_mode_enum_v02 call_mode;
   call_type_enum_v02 call_type;

   if (NULL == call_info_entry)
   {
      QCRIL_LOG_ERROR("call_info_entry is NULL");
   }
   else
   {
      if (NULL != iter_call_info)
      {
         call_type = iter_call_info->call_type;
         call_mode = iter_call_info->mode;
      }
      else
      {
         call_type = call_info_entry->voice_scv_info.call_type;
         call_mode = call_info_entry->voice_scv_info.mode;
      }

      switch(call_mode)
      {
        case CALL_MODE_NO_SRV_V02:
        case CALL_MODE_UNKNOWN_V02:
          switch(call_type)
          {
            case CALL_TYPE_VOICE_IP_V02:
            case CALL_TYPE_VT_V02:
            case CALL_TYPE_EMERGENCY_IP_V02:
              QCRIL_LOG_INFO("Set audio call_type as IMS");
              call_info_entry->audio_call_type = QMI_RIL_VOICE_IMS_AUDIO_CALL_TYPE_IMS;
              break;

            default:
              QCRIL_LOG_INFO("Set audio call_type as VOICE");
              call_info_entry->audio_call_type = QMI_RIL_VOICE_IMS_AUDIO_CALL_TYPE_VOICE;
              break;
          }
          break;

        case CALL_MODE_LTE_V02:
        case CALL_MODE_WLAN_V02:
          QCRIL_LOG_INFO("Set audio call_type as IMS");
          call_info_entry->audio_call_type = QMI_RIL_VOICE_IMS_AUDIO_CALL_TYPE_IMS;
          break;

        default:
          QCRIL_LOG_INFO("Set audio call_type as VOICE");
          call_info_entry->audio_call_type = QMI_RIL_VOICE_IMS_AUDIO_CALL_TYPE_VOICE;
          break;
      }
   }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_get_current_setup_calls

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_GET_CURRENT_SETUP_CALLS
*/
/*=========================================================================*/
void qcril_qmi_voice_get_current_setup_calls
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr
)
{
    qcril_qmi_voice_setup_call_info *payload_ptr = NULL;
    qcril_qmi_voice_voip_call_info_entry_type* call_info_entry = NULL;
    qcril_qmi_voice_current_calls_type *current_calls_type_ptr;
    qcril_request_resp_params_type resp;
    boolean is_call_present = FALSE;
    RIL_Errno result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    qcril_qmi_voice_voip_lock_overview();
    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state(CALL_STATE_SETUP_V02);
    do
    {
        if (NULL == call_info_entry)
        {
            result = RIL_E_SUCCESS;
            break;
        }
        is_call_present = TRUE;
        payload_ptr = (qcril_qmi_voice_setup_call_info *) qcril_malloc (sizeof (*payload_ptr));
        if (NULL == payload_ptr)
        {
            break;
        }
        memset (payload_ptr, 0, sizeof (*payload_ptr));
        current_calls_type_ptr = (qcril_qmi_voice_current_calls_type *) qcril_malloc (sizeof (*current_calls_type_ptr));
        if (NULL == current_calls_type_ptr)
        {
            break;
        }
        memset (current_calls_type_ptr, 0, sizeof (*current_calls_type_ptr));
        if (RIL_E_SUCCESS != qcril_qmi_voice_gather_current_call_information(
                                0, params_ptr, current_calls_type_ptr, call_info_entry))
        {
            break;
        }

        payload_ptr->index = current_calls_type_ptr->info[0].index;
        payload_ptr->toa = current_calls_type_ptr->info[0].toa;
        payload_ptr->als = current_calls_type_ptr->info[0].als;
        payload_ptr->isVoice = current_calls_type_ptr->info[0].isVoice;
        if (NULL != current_calls_type_ptr->info[0].number)
            strlcpy (payload_ptr->number, current_calls_type_ptr->info[0].number, QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN);
        payload_ptr->numberPresentation = current_calls_type_ptr->info[0].numberPresentation;
        if (NULL != current_calls_type_ptr->info[0].name)
            strlcpy (payload_ptr->name, current_calls_type_ptr->info[0].name, QCRIL_QMI_VOICE_DIAL_NUMBER_MAX_LEN);
        payload_ptr->namePresentation = current_calls_type_ptr->info[0].namePresentation;

        result = RIL_E_SUCCESS;
    } while (FALSE);

    qcril_qmi_voice_voip_unlock_overview();

    if (RIL_E_SUCCESS == result)
    {
        QCRIL_LOG_INFO ("Reply to OEM --> is_call_present %d", is_call_present);
        qcril_default_request_resp_params (QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp);
        if (is_call_present)
        {
            QCRIL_LOG_INFO ("Reply to OEM --> index %d, toa %d, als %d, isVoice %d",
                           payload_ptr->index,
                           payload_ptr->toa,
                           payload_ptr->als,
                           payload_ptr->isVoice);

            QCRIL_LOG_INFO ("...num %s, num presentation %d, name %s, name presentation %d",
                           payload_ptr->number,
                           payload_ptr->numberPresentation,
                           payload_ptr->name,
                           payload_ptr->namePresentation);
            resp.resp_pkt = (void *) payload_ptr;
            resp.resp_len = sizeof (*payload_ptr);
        }
    }
    else
    {
        qcril_default_request_resp_params (QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp);
    }
    qcril_send_request_response (&resp );
    if (current_calls_type_ptr)
        qcril_free (current_calls_type_ptr);
    if (payload_ptr)
        qcril_free (payload_ptr);
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_get_current_setup_calls

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_request_setup_answer

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_HOOK_REQUEST_SETUP_ANSWER
*/
/*=========================================================================*/
void qcril_qmi_voice_request_setup_answer
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr
)
{
    qcril_qmi_voice_voip_call_info_entry_type*                          call_info_entry = NULL;
    voice_setup_answer_req_msg_v02                                      setup_answer_req_msg;
    voice_setup_answer_resp_msg_v02*                                    setup_answer_resp_msg_ptr = NULL;
    qcril_qmi_voice_setup_answer_data_type*                             data = NULL;
    uint32_t                                                            user_data;
    qcril_reqlist_public_type                                           reqlist_entry;
    qmi_ril_voice_ims_command_exec_oversight_type*                      command_oversight = NULL;;
    qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type   oversight_cmd_params;
    RIL_Errno                                                           result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_voice_voip_lock_overview();
    call_info_entry = qcril_qmi_voice_voip_find_call_info_entry_by_qmi_call_state (CALL_STATE_SETUP_V02);
    do
    {
        if (RIL_E_SUCCESS != qcril_qmi_voice_perform_null_check_and_reqlist_entry
                             (params_ptr, &user_data, &reqlist_entry) ||
            !call_info_entry)
        {
            QCRIL_LOG_ERROR("Failure. call_info_entry %p", call_info_entry);
            break;
        }
        setup_answer_resp_msg_ptr = qcril_malloc (sizeof (*setup_answer_resp_msg_ptr));
        if (!setup_answer_resp_msg_ptr)
        {
            break;
        }

        command_oversight = qmi_ril_voice_ims_create_command_oversight (params_ptr->t, params_ptr->event_id, TRUE);
        if ( NULL == command_oversight )
        {
            break;
        }
        data = (qcril_qmi_voice_setup_answer_data_type *) params_ptr->data;
        qmi_ril_voice_ims_command_oversight_add_call_link (command_oversight,
                                                     QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_QMI_CALL_ID,
                                                     QCRIL_QMI_VOICE_VOIP_CALLINFO_ELA_NONE,
                                                     call_info_entry->qmi_call_id,
                                                     data->rejection ? CALL_STATE_END_V02 : CALL_STATE_INCOMING_V02);

        memset (&oversight_cmd_params, 0, sizeof (oversight_cmd_params));
        oversight_cmd_params.locator.command_oversight = command_oversight;

        qmi_ril_voice_ims_command_oversight_handle_event (QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_COMMENCE_AWAIT_RESP_IND,
                                                        QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                                        &oversight_cmd_params);

        memset (&setup_answer_req_msg, 0, sizeof (setup_answer_req_msg));
        setup_answer_req_msg.call_id = call_info_entry->qmi_call_id;
        setup_answer_req_msg.reject_setup_valid = TRUE;
        setup_answer_req_msg.reject_setup = data->rejection;
        if (E_SUCCESS != qcril_qmi_client_send_msg_async (
                                QCRIL_QMI_CLIENT_VOICE,
                                QMI_VOICE_SETUP_ANSWER_REQ_V02,
                                &setup_answer_req_msg,
                                sizeof (setup_answer_req_msg),
                                setup_answer_resp_msg_ptr,
                                sizeof (*setup_answer_resp_msg_ptr),
                                (void *)(intptr_t) user_data))
        {
            QCRIL_LOG_ERROR("failed to send setup answer request: qmi call id %d, rejection %d",
                       setup_answer_req_msg.call_id, setup_answer_req_msg.reject_setup);
            break;
        }
        QCRIL_LOG_INFO("successfully sent setup answer request: qmi call id %d, rejection %d",
                       setup_answer_req_msg.call_id, setup_answer_req_msg.reject_setup);
        result = RIL_E_SUCCESS;
    } while (FALSE);

    if (RIL_E_SUCCESS != result)
    {
        if (setup_answer_resp_msg_ptr)
            qcril_free (setup_answer_resp_msg_ptr);
        if (NULL != command_oversight)
            qmi_ril_voice_ims_destroy_command_oversight( command_oversight );
        qcril_send_empty_payload_request_response (QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, result);
    }

    qcril_qmi_voice_voip_unlock_overview();
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_voice_request_setup_answer

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_setup_answer_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handles response QMI_VOICE_SETUP_ANSWER_RESP_V02
*/
/*=========================================================================*/
void qcril_qmi_voice_setup_answer_resp_hdlr
(
   const qcril_request_params_type *const params_ptr
)
{
    voice_setup_answer_resp_msg_v02*                                    setup_answer_resp_ptr = NULL;
    qmi_ril_voice_ims_command_exec_oversight_handle_event_params_type   oversight_event_params;
    qmi_ril_voice_ims_command_exec_oversight_type*                      command_oversight = NULL;
    int                                                                 covered_by_oversight_handling = FALSE;
    RIL_Errno                                                           result = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();
    setup_answer_resp_ptr = (voice_setup_answer_resp_msg_v02 *) params_ptr->data;
    qcril_qmi_voice_voip_lock_overview();
    if (setup_answer_resp_ptr)
    {
        result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result (QMI_NO_ERR, &setup_answer_resp_ptr->resp);
        QCRIL_LOG_INFO("QMI result %d, QMI error %d, RIL result %d",
                       setup_answer_resp_ptr->resp.result, setup_answer_resp_ptr->resp.error, result);
        command_oversight = qmi_ril_voice_ims_find_command_oversight_by_token (params_ptr->t);
        if (NULL != command_oversight)
        {
            memset (&oversight_event_params, 0, sizeof (oversight_event_params));
            oversight_event_params.locator.command_oversight = command_oversight;
            covered_by_oversight_handling = qmi_ril_voice_ims_command_oversight_handle_event
                                            (
                                               (RIL_E_SUCCESS == result)?
                                               QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_SUCCESS :
                                               QMI_RIL_VOICE_IMS_EXEC_INTERMED_EVENT_RECEIVED_RESP_FAILURE,
                                               QMI_RIL_VOICE_IMS_EXEC_OVERSIGHT_LINKAGE_SPECIFIC_OVERSIGHT_OBJ,
                                               &oversight_event_params
                                            );
        }
    }
    qcril_qmi_voice_voip_unlock_overview();
    if (NULL == command_oversight || !covered_by_oversight_handling)
    {
        qcril_send_empty_payload_request_response (QCRIL_DEFAULT_INSTANCE_ID, params_ptr->t, params_ptr->event_id, result);
    }
    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION: qcril_qmi_voice_perform_null_check_and_reqlist_entry

===========================================================================*/
RIL_Errno qcril_qmi_voice_perform_null_check_and_reqlist_entry
(
   const qcril_request_params_type *const params_ptr,
   uint32_t *user_data_ptr,
   qcril_reqlist_public_type *reqlist_entry_ptr
)
{
    RIL_Errno result = RIL_E_GENERIC_FAILURE;
    do
    {
        if (NULL == params_ptr || NULL == user_data_ptr)
        {
            QCRIL_LOG_ERROR("failure: NULL pointer. params_ptr %p, user_data_ptr %p",
                            params_ptr, user_data_ptr);
            break;
        }
        if (NULL == params_ptr->data || 0 == params_ptr->datalen)
        {
            QCRIL_LOG_ERROR("failure: params_ptr->data %p, params_ptr->datalen %d",
                           params_ptr->data, (int) params_ptr->datalen);
            break;
        }
        qcril_reqlist_default_entry (params_ptr->t,
                                     params_ptr->event_id,
                                     QCRIL_DEFAULT_MODEM_ID,
                                     QCRIL_REQ_AWAITING_CALLBACK,
                                     QCRIL_EVT_NONE,
                                     NULL,
                                     reqlist_entry_ptr);
        if (E_SUCCESS != qcril_reqlist_new (QCRIL_DEFAULT_INSTANCE_ID, reqlist_entry_ptr))
        {
            QCRIL_LOG_ERROR("failure: could not add entry to reqlist");
            break;
        }
        *user_data_ptr = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, reqlist_entry_ptr->req_id);
        result = RIL_E_SUCCESS;
    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
    return result;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_request_get_colr

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_get_colr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type    reqlist_entry;
  uint32                       user_data;
  RIL_Errno                    res = RIL_E_GENERIC_FAILURE;
  IxErrnoType                  qmi_client_error;
  voice_get_colr_resp_msg_v02 *get_colr_resp_ptr = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    /* Add entry to ReqList */
    qcril_reqlist_default_entry(params_ptr->t,
            params_ptr->event_id,
            QCRIL_DEFAULT_MODEM_ID,
            QCRIL_REQ_AWAITING_CALLBACK,
            QCRIL_EVT_NONE,
            NULL,
            &reqlist_entry);

    if (qcril_reqlist_new(QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry) != E_SUCCESS)
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    get_colr_resp_ptr = qcril_malloc(sizeof(*get_colr_resp_ptr));
    if (get_colr_resp_ptr == NULL)
    {
      QCRIL_LOG_ERROR("qcril_malloc failed");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID,
            reqlist_entry.req_id);

    /* Send QMI VOICE DIAL CALL REQ */
    qmi_client_error = qcril_qmi_client_send_msg_async (QCRIL_QMI_CLIENT_VOICE,
                                         QMI_VOICE_GET_COLR_REQ_V02,
                                         NULL,
                                         0,
                                         get_colr_resp_ptr,
                                         sizeof(*get_colr_resp_ptr),
                                         (void*)(uintptr_t)user_data);
    QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );
    res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_client_error, NULL);
  } while(FALSE);

  if (res != RIL_E_SUCCESS)
  {
    qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
            params_ptr->t,
            params_ptr->event_id,
            res);
    qcril_free(get_colr_resp_ptr);
  }

  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_get_colr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_get_colr_resp_hdlr

===========================================================================*/
/*!
    @brief
    Handle QMI VOICE GET COLR RESP.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_get_colr_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
  voice_get_colr_resp_msg_v02 *get_colr_resp = NULL;
  RIL_Errno                    ril_result    = RIL_E_GENERIC_FAILURE;

  if( params_ptr->data != NULL )
  {
    get_colr_resp = (voice_get_colr_resp_msg_v02 *)params_ptr->data;

    ril_result = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                   QMI_NO_ERR,
                   &get_colr_resp->resp);

    if (ril_result == RIL_E_SUCCESS)
    {
      QCRIL_LOG_INFO("QCRIL QMI VOICE GET COLR RESP: SUCCESS");
      if (params_ptr->event_id == QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR)
      {
        Ims__Colr colr = IMS__COLR__INIT;

        QCRIL_LOG_INFO( "colr_pi_valid: %d, colr_pi: %d",
            get_colr_resp->colr_pi_valid, get_colr_resp->colr_pi);

        if (get_colr_resp->colr_pi_valid)
        {
          if (get_colr_resp->colr_pi == COLR_PRESENTATION_NOT_RESTRICTED_V02)
          {
            colr.has_presentation = TRUE;
            colr.presentation = IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_ALLOWED;
          }
          else if (get_colr_resp->colr_pi == COLR_PRESENTATION_RESTRICTED_V02)
          {
            colr.has_presentation = TRUE;
            colr.presentation = IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_RESTRICTED;
          }
          else
          {
            ril_result = RIL_E_GENERIC_FAILURE;
            QCRIL_LOG_ERROR("Unexpected value from modem\n");
          }

          if (ril_result == RIL_E_SUCCESS)
          {
            qcril_qmi_ims_socket_send(params_ptr->t,
                    IMS__MSG_TYPE__RESPONSE,
                    IMS__MSG_ID__REQUEST_GET_COLR,
                    IMS__ERROR__E_SUCCESS,
                    (void *)&colr,
                    sizeof(colr));
          }
        }
      }
    }
    else
    {
      QCRIL_LOG_INFO("QCRIL QMI VOICE GET COLR RESP: FAILURE");
    }
  }

  if (ril_result != RIL_E_SUCCESS)
  {
    qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
            params_ptr->t,
            params_ptr->event_id,
            ril_result);
  }
}/* qcril_qmi_voice_get_colr_resp_hdlr */

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_request_set_colr

===========================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_voice_request_set_colr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_reqlist_public_type  reqlist_entry;
  RIL_Errno                  res = RIL_E_GENERIC_FAILURE;
  IxErrnoType                qmi_client_error;
  uint32                     user_data;
  Ims__Colr                 *ims_colr_ptr = NULL;

  voice_set_sups_service_req_msg_v02   set_sups_req;
  voice_set_sups_service_resp_msg_v02 *set_sups_resp_ptr = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    if ( params_ptr->datalen == 0  || params_ptr->data == NULL )
    {
      QCRIL_LOG_INFO( "params_ptr->datalen == 0 or params_ptr is NULL" );
      break;
    }

    ims_colr_ptr = (Ims__Colr *) params_ptr->data;

    /* Add entry to ReqList */
    qcril_reqlist_default_entry(params_ptr->t,
            params_ptr->event_id,
            QCRIL_DEFAULT_MODEM_ID,
            QCRIL_REQ_AWAITING_CALLBACK,
            QCRIL_EVT_NONE,
            NULL,
            &reqlist_entry );
    if (qcril_reqlist_new(QCRIL_DEFAULT_INSTANCE_ID, &reqlist_entry) != E_SUCCESS)
    {
      /* Fail to add entry to ReqList */
      QCRIL_LOG_ERROR("Failed to Add into Req list");
      break;
    }

    set_sups_resp_ptr = qcril_malloc( sizeof(*set_sups_resp_ptr) );
    if( NULL == set_sups_resp_ptr )
    {
      QCRIL_LOG_ERROR("qcril_malloc failed");
      break;
    }

    memset(&set_sups_req, 0, sizeof(set_sups_req));

    QCRIL_LOG_INFO( "has_presentation: %d, presentation: %d",
          ims_colr_ptr->has_presentation,
          ims_colr_ptr->presentation);

    if (ims_colr_ptr->has_presentation)
    {
      set_sups_req.supplementary_service_info.reason = QMI_VOICE_REASON_COLR_V02;

      set_sups_req.colr_pi_valid = TRUE;
      if (ims_colr_ptr->presentation == IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_RESTRICTED)
      {
        set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_ACTIVATE_V02;
        set_sups_req.colr_pi = COLR_PRESENTATION_RESTRICTED_V02;
      }
      else if (ims_colr_ptr->presentation == IMS__IP_PRESENTATION__IP_PRESENTATION_NUM_ALLOWED)
      {
        set_sups_req.supplementary_service_info.voice_service = VOICE_SERVICE_DEACTIVATE_V02;
        set_sups_req.colr_pi = COLR_PRESENTATION_NOT_RESTRICTED_V02;
      }
      else
      {
        QCRIL_LOG_ERROR("requested colr is not a valid value");
        break;
      }
    }
    else
    {
      QCRIL_LOG_ERROR("request misses some necessary information");
      break;
    }

    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID,
            QCRIL_DEFAULT_MODEM_ID,
            reqlist_entry.req_id);

    /* Send QMI VOICE SET SUPS SERVICE REQ */
    qmi_client_error = qcril_qmi_client_send_msg_async (QCRIL_QMI_CLIENT_VOICE,
                                          QMI_VOICE_SET_SUPS_SERVICE_REQ_V02,
                                          &set_sups_req,
                                          sizeof(set_sups_req),
                                          set_sups_resp_ptr,
                                          sizeof(*set_sups_resp_ptr),
                                          (void*)(uintptr_t)user_data);
    QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );
    res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_client_error, NULL);
  } while(FALSE);

  if (res != RIL_E_SUCCESS)
  {
    qcril_send_empty_payload_request_response(QCRIL_DEFAULT_INSTANCE_ID,
            params_ptr->t,
            params_ptr->event_id,
            res);
    qcril_free(set_sups_resp_ptr);
  }

  if(ims_colr_ptr)
  {
    qcril_qmi_ims__colr__free_unpacked(ims_colr_ptr, NULL);
  }
  QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_request_set_colr */

void qcril_qmi_voice_voip_reset_answered_call_type
(
 qcril_qmi_voice_voip_call_info_entry_type *call_info,
 voice_modified_ind_msg_v02                *modify_ind_ptr
)
{
  if(call_info != NULL && call_info->answered_call_type_valid)
  {
    if (call_info->voice_scv_info.call_type == CALL_TYPE_VT_V02)
    {
      if ((modify_ind_ptr->audio_attrib_valid && 0 != modify_ind_ptr->audio_attrib) &&
          (modify_ind_ptr->video_attrib_valid && 0 != modify_ind_ptr->video_attrib))
      {
        call_info->answered_call_type_valid = FALSE;
      }
    }
    else if (call_info->voice_scv_info.call_type == CALL_TYPE_VOICE_IP_V02)
    {
      if (modify_ind_ptr->audio_attrib_valid && 0 != modify_ind_ptr->audio_attrib)
      {
        call_info->answered_call_type_valid = FALSE;
      }
    }
  }
}

//===========================================================================
// qcril_qmi_ims_socket_send_unsol_call_state_changed
//===========================================================================
void qcril_qmi_voice_send_ims_unsol_call_state_changed()
{
    qcril_request_params_type params = {{{0}}};
    params.event_id = QCRIL_EVT_IMS_SOCKET_SEND_UNSOL_CURRENT_CALLS;
    qcril_qmi_voice_send_current_ims_calls(&params, NULL);
} // qcril_qmi_ims_socket_send_unsol_call_state_changed

//===========================================================================
// qcril_qmi_voice_is_ims_send_calls
//===========================================================================
boolean qcril_qmi_voice_is_ims_send_calls(int event_id)
{
    return event_id == QCRIL_EVT_IMS_SOCKET_REQ_GET_CURRENT_CALLS ||
           event_id == QCRIL_EVT_IMS_SOCKET_SEND_UNSOL_CURRENT_CALLS;
} // qcril_qmi_voice_is_ims_send_calls

/*=========================================================================
  FUNCTION:  qcril_qmi_voice_send_ims_unsol_resp_handover

===========================================================================*/
/*!
    @brief
    Sends UNSOL_RESPONSE_HANDOVER on IMS socket.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_qmi_voice_send_ims_unsol_resp_handover(RIL_SrvccState ril_srvccstate)
{
    Ims__Handover ims_handover = IMS__HANDOVER__INIT;

    ims_handover.has_type = TRUE;
    switch(ril_srvccstate)
    {
    case HANDOVER_STARTED:
        ims_handover.type = IMS__HANDOVER__MSG__TYPE__START;
        break;
    case HANDOVER_COMPLETED:
        ims_handover.type = IMS__HANDOVER__MSG__TYPE__COMPLETE_SUCCESS;
        break;
    case HANDOVER_FAILED:
        ims_handover.type = IMS__HANDOVER__MSG__TYPE__COMPLETE_FAIL;
        break;
    case HANDOVER_CANCELED:
        ims_handover.type = IMS__HANDOVER__MSG__TYPE__CANCEL;
        break;
    default:
        ims_handover.has_type = FALSE;
        break;
    }

    QCRIL_LOG_INFO("Send UNSOL_RESPONSE_HANDOVER has_type = %d, type = %d",
            ims_handover.has_type, ims_handover.type);

    if (ims_handover.has_type)
    {
        qcril_qmi_ims_socket_send(
                0,
                IMS__MSG_TYPE__UNSOL_RESPONSE,
                IMS__MSG_ID__UNSOL_RESPONSE_HANDOVER,
                IMS__ERROR__E_SUCCESS,
                &ims_handover,
                sizeof(ims_handover));
    }
}
