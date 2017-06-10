/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE
#define CRI_VOICE

#include "voice_service_v02.h"
#include "cri_voice_qmi_client.h"
#include "utils_common.h"
//#include "qcril_qmi_voice.h"
#include "comdef.h"
cri_core_error_type cri_voice_init(hlos_ind_cb_type hlos_ind_cb_func_ptr);
cri_core_error_type cri_voice_deinit();

#define CRI_VOICE_INVALID_ENUM_VAL      (-1)

#define CRI_VOICE_LOWEST_CALL_ID        1
#define CRI_VOICE_HIGHEST_CALL_ID       0xFE
#define CRI_VOICE_INVALID_CALL_ID       0xFF

#define CRI_VOICE_SS_TA_UNKNOWN       129 /* 0x80|CM_TON_UNKNOWN      |CM_NPI_ISDN */
#define CRI_VOICE_SS_TA_INTERNATIONAL 145 /* 0x80|CM_TON_INTERNATIONAL|CM_NPI_ISDN */
#define CRI_VOICE_SS_TA_INTER_PREFIX  '+' /* ETSI international call dial prefix */

typedef struct
{
   size_t len;
   uint8_t *data;
} cri_voice_binary_data;

typedef enum {
    CRI_VOICE_IND_MIN                  = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_CALL_STATE_CHANGED_IND           = 0, // NULL payload
    CRI_VOICE_INFO_REC_IND                     = 1, // payload of voice_info_rec_ind_msg_v02
    CRI_VOICE_VOICE_CONFERENCE_INFO_IND        = 2, // payload of cri_voice_binary_data
    CRI_VOICE_VOICE_OTASP_STATUS_IND           = 3, // payload of voice_otasp_status_ind_msg_v02
    CRI_VOICE_VOICE_PRIVACY_IND                = 5, // payload of voice_privacy_ind_msg_v02
    CRI_VOICE_VOICE_EXT_BRST_INTL_IND          = 6, // payload of voice_ext_brst_intl_ind_msg_v02
    CRI_VOICE_VOICE_SUPS_NOTIFICATION_IND      = 7, // payload of voice_sups_notification_ind_msg_v02
    CRI_VOICE_IND_MAX
} cri_voice_ind_type;


typedef enum {
    CRI_VOICE_CALL_STATE_MIN      = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_CALL_STATE_ACTIVE   = 0,
    CRI_VOICE_CALL_STATE_HOLDING  = 1,
    CRI_VOICE_CALL_STATE_DIALING  = 2,
    CRI_VOICE_CALL_STATE_ALERTING = 3,
    CRI_VOICE_CALL_STATE_INCOMING = 4,
    CRI_VOICE_CALL_STATE_WAITING  = 5,
    CRI_VOICE_CALL_STATE_SETUP    = 6,
    CRI_VOICE_CALL_STATE_END      = 7,
    CRI_VOICE_CALL_STATE_MAX
} cri_voice_call_state_type;

typedef enum
{
    CRI_VOICE_CALL_TYPE_MIN = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_CALL_TYPE_VOICE,
    CRI_VOICE_CALL_TYPE_VT_TX,
    CRI_VOICE_CALL_TYPE_VT_RX,
    CRI_VOICE_CALL_TYPE_VT,
    CRI_VOICE_CALL_TYPE_VT_NODIR,
    CRI_VOICE_CALL_TYPE_CS_VS_TX,
    CRI_VOICE_CALL_TYPE_CS_VS_RX,
    CRI_VOICE_CALL_TYPE_PS_VS_TX,
    CRI_VOICE_CALL_TYPE_PS_VS_RX,
    CRI_VOICE_CALL_TYPE_MAX
} cri_voice_call_type_type;

typedef enum
{
    CRI_VOICE_CALL_DOMAIN_MIN = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_CALL_DOMAIN_CS,
    CRI_VOICE_CALL_DOMAIN_PS,
    CRI_VOICE_CALL_DOMAIN_AUTOMATIC,
    CRI_VOICE_CALL_DOMAIN_MAX
} cri_voice_call_domain_type;

typedef enum
{
    CRI_VOICE_CLIR_MIN = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_CLIR_DEFAULT,
    CRI_VOICE_CLIR_INVOCATION,
    CRI_VOICE_CLIR_SUPPRESSION,
    CRI_VOICE_CLIR_MAX
} cri_voice_clir_type;

typedef enum
{
    CRI_VOICE_CALL_OBJ_BIT_FIELD_MIN                             = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_NONE                            = 0,
    // QMI information bits
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALL_ID_RECEIVED            = 0x01,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_SCV_INFO_VALID              = 0x02,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NUMBER_VALID        = 0x03,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NAME_VALID          = 0x04,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_ALERTING_TYPE_VALID         = 0x05,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_SRV_OPT_VALID               = 0x06,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALL_END_REASON_VALID       = 0x07,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_ALPHA_ID_VALID              = 0x08,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CONN_PARTY_NUM_VALID        = 0x09,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_DIAGNOSTIC_INFO_VALID       = 0x0A,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALLED_PARTY_NUM_VALID      = 0x0B,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_REDIRECTING_PARTY_NUM_VALID = 0x0C,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_OTASP_STATUS_VALID          = 0x0D,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_VOICE_PRIVACY_VALID         = 0x0E,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_UUS_VALID                   = 0x0F,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_AUDIO_ATTR_VALID            = 0x10,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_VIDEO_ATTR_VALID            = 0x11,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_SPEECH_CODEC_VALID          = 0x12,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_IS_SRVCC_VALID              = 0x13,

    // CRI derived information
    CRI_VOICE_CALL_OBJ_BIT_FIELD_CRI_CALL_STATE_VALID        = 0x20,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_CS_DOMAIN                   = 0x21,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_PS_DOMAIN                   = 0x22,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_AUTO_DOMAIN                 = 0x23,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_MO_CALL_BEING_SETUP         = 0x24,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_PENDING_INCOMING            = 0x25,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_CALL_GOT_CONNECTED          = 0x26,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING       = 0x27,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_CALL_END_BY_USER            = 0x28,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_REPORT_CACHED_RP_NUMBER     = 0x29,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_EMERGENCY_CALL              = 0x2A,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_MPTY_VOIP_CALL              = 0x2B,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_TO_BE_MPTY_VOIP_CALL        = 0x2C,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_MEMBER_OF_MPTY_VOIP_CALL    = 0x2D,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_TO_BE_MEMBER_OF_MPTY_VOIP_CALL = 0x2E,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_CALL_MODIFY_CONFIRM_PENDING = 0x2F,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_TO_BE_ADDED_TO_EXISTING_MPTY_VOIP_CALL = 0x30,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_CONF_PATICIAPNT_CALL_END_REPORTED = 0x31,
    CRI_VOICE_CALL_OBJ_BIT_FIELD_MAX
} cri_voice_call_obj_bit_field_type;

typedef void (*hlos_user_data_deleter_type) (void**);

#define CRI_VOICE_INTERCODING_BUF_LEN     1024
typedef struct {
  uint8_t call_id;
  pi_name_enum_v02 name_pi;
  uint32_t name_len;
  char name[ CRI_VOICE_INTERCODING_BUF_LEN ]; /* in utf8 format */
} cri_voice_remote_party_name_type;

typedef struct
{
    // info from QMI Voice
    uint8_t                                 qmi_call_id;
    voice_call_info2_type_v02               qmi_voice_scv_info;
    voice_remote_party_number2_type_v02     qmi_remote_party_number;
    voice_alerting_type_type_v02            qmi_alerting_type;
    voice_srv_opt_type_v02                  qmi_srv_opt;
    voice_call_end_reason_type_v02          qmi_call_end_reason;
    voice_alpha_ident_with_id_type_v02      qmi_alpha_id;
    voice_conn_num_with_id_type_v02         qmi_conn_party_num;
    voice_diagnostic_info_with_id_type_v02  qmi_diagnostic_info;
    voice_num_with_id_type_v02              qmi_called_party_num;
    voice_num_with_id_type_v02              qmi_redirecting_party_num;
    otasp_status_enum_v02                   qmi_otasp_status;
    voice_privacy_enum_v02                  qmi_voice_privacy;
    voice_uus_type_v02                      qmi_uus;
    voice_call_attribute_type_mask_v02      qmi_audio_attrib;
    voice_call_attribute_type_mask_v02      qmi_video_attrib;
    voice_speech_codec_enum_v02             qmi_speech_codec;
    voice_is_srvcc_call_with_id_type_v02    qmi_is_srvcc;

    // info derived by CRF
    util_bit_field_type                     cri_bit_field;
    uint8_t                                 cri_call_id;
    cri_voice_call_state_type               cri_call_state;
    cri_voice_call_type_type                cri_call_type;
    cri_voice_call_domain_type              cri_call_domain;
    int                                     cri_1x_num_pending_timer_id;
    cri_voice_remote_party_name_type        cri_remote_party_name;
    util_list_info_type                     *cri_child_call_obj_list_ptr;
    util_list_info_type                     *cri_parent_call_obj_list_ptr;

    // info specific to HLOS and maintained by HLOS
    void                                    *hlos_user_data;
    boolean                                 hlos_user_data_deleter_valid;
    hlos_user_data_deleter_type             hlos_user_data_deleter;
} cri_voice_call_obj_type;

typedef struct
{
    uint8_t num_of_calls;
    cri_voice_call_obj_type **calls_dptr;
} cri_voice_call_list_type;

typedef struct
{
    const char * address;
    boolean is_conference_uri;
    cri_voice_clir_type clir;
    cri_voice_call_type_type call_type;
    cri_voice_call_domain_type call_domain;
    size_t n_extras;
    char **extras;
    voice_uus_type_v02 *uus_info;
} cri_voice_dial_request_type;

typedef struct
{
    int dial_call_id;
    cri_voice_call_list_type *call_list_ptr;
} cri_voice_call_dial_response_type;

typedef void (*cri_voice_request_dial_cb_type)
(
    cri_core_context_type cri_core_context,
    cri_core_error_type err_code,
    void *user_data,
    cri_voice_call_dial_response_type *resp_message_ptr
);
cri_core_error_type cri_voice_request_dial(cri_core_context_type cri_core_context, const cri_voice_dial_request_type *req_message_ptr, const void *user_data, cri_voice_request_dial_cb_type dial_cb);

// Hangup Api
typedef enum
{
    CRI_VOICE_HANGUP_TYPE_MIN = CRI_VOICE_INVALID_ENUM_VAL,
    CRI_VOICE_HANGUP_ALL_CALLS,
    CRI_VOICE_HANGUP_ALL_NONE_EMERGENCY_CALLS,
    CRI_VOICE_HANGUP_FOREGROUND_CALLS_RESUME_BACKGROUND_CALLS,
    CRI_VOICE_HANGUP_BACKGROUND_CALLS,
    CRI_VOICE_HANGUP_WITH_CALL_HLOS_ID,
    CRI_VOICE_HANGUP_WITH_CALL_URI,
    CRI_VOICE_HANGUP_WITH_CALL_HLOS_ID_AND_URI,
    CRI_VOICE_HANGUP_TYPE_MAX
} cri_voice_hangup_type_type;
typedef struct
{
    cri_voice_hangup_type_type hangup_type;
    boolean call_uri_valid;
    const char* call_uri;
    boolean call_hlos_id_valid;
    uint8   call_hlos_id;
    boolean hangup_cause_valid;
    int hangup_cause;
} cri_voice_call_hangup_request_type;
typedef void (*cri_voice_request_hangup_cb_type)
(
    cri_core_context_type cri_core_context,
    cri_core_error_type err_code,
    void *user_data
);
cri_core_error_type cri_voice_request_hangup(cri_core_context_type cri_core_context, const cri_voice_call_hangup_request_type *req_message_ptr, const void *user_data, cri_voice_request_hangup_cb_type hangup_cb);

// Answer Api
typedef struct
{
    boolean clir_valid;
    cri_voice_clir_type clir;
    boolean call_type_valid;
    cri_voice_call_type_type call_type;
} cri_voice_answer_request_type;
typedef void (*cri_voice_request_answer_cb_type)
(
    cri_core_context_type cri_core_context,
    cri_core_error_type err_code,
    void *user_data
);
cri_core_error_type cri_voice_request_answer(cri_core_context_type cri_core_context,
                                             const cri_voice_answer_request_type *req_message_ptr,
                                             const void *user_data,
                                             cri_voice_request_answer_cb_type answer_cb);

// get current calls Api
cri_core_error_type cri_voice_request_get_current_all_calls(cri_voice_call_list_type** call_list_dptr);
cri_core_error_type cri_voice_request_get_current_cs_calls(cri_voice_call_list_type** call_list_dptr);
cri_core_error_type cri_voice_request_get_current_ps_calls(cri_voice_call_list_type** call_list_dptr);
cri_core_error_type cri_voice_request_get_current_csvt_calls(cri_voice_call_list_type** call_list_dptr);

typedef boolean (*cri_voice_is_specific_call) (const cri_voice_call_obj_type *call_obj_ptr);
typedef boolean (*cri_voice_is_specific_call_with_param) (const cri_voice_call_obj_type *call_obj_ptr, const void* param);
cri_core_error_type cri_voice_request_get_current_specific_calls(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call is_specific_call_checker);
cri_core_error_type cri_voice_request_get_current_specific_calls_with_param(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call_with_param is_specific_call_checker, const void* param);

// Switch calls Api
typedef struct
{
    boolean has_call_type;
    cri_voice_call_type_type call_type;
} cri_voice_switch_calls_request_type;
typedef void (*cri_voice_request_switch_calls_cb_type)
(
    cri_core_context_type cri_core_context,
    cri_core_error_type err_code,
    void *user_data
);
cri_core_error_type cri_voice_request_switch_calls(
    cri_core_context_type cri_core_context,
    const cri_voice_switch_calls_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_switch_calls_cb_type switch_cb
);

// Conference Api
typedef void (*cri_voice_request_conference_cb_type)
(
    cri_core_context_type cri_core_context,
    cri_core_error_type err_code,
    void *user_data
);
cri_core_error_type cri_voice_request_conference(
    cri_core_context_type cri_core_context,
    const void *user_data,
    cri_voice_request_conference_cb_type conf_cb
);

// Separate Connection Api
typedef struct
{
    int conn_id;
} cri_voice_separate_conn_request_type;
typedef void (*cri_voice_request_separate_conn_cb_type)
(
    cri_core_context_type cri_core_context,
    cri_core_error_type err_code,
    void *user_data
);
cri_core_error_type cri_voice_request_separate_conn(
    cri_core_context_type cri_core_context,
    const cri_voice_separate_conn_request_type *req_message_ptr,
    const void *user_data,
    cri_voice_request_separate_conn_cb_type separate_conn_cb
);

// call obj Api
cri_voice_call_obj_type* cri_voice_find_call_object_by_cri_call_state(cri_voice_call_state_type cri_call_state);
cri_voice_call_obj_type* cri_voice_find_call_object_by_call_bit(cri_voice_call_obj_bit_field_type bit_field);
cri_voice_call_obj_type* cri_voice_find_call_object_by_qmi_call_id(uint8_t qmi_call_id);

cri_core_error_type cri_voice_set_call_obj_user_data(uint8_t cri_call_id, void* user_data, hlos_user_data_deleter_type hlos_user_data_deleter);
cri_core_error_type cri_voice_set_default_call_obj_user_data_deleter(hlos_user_data_deleter_type hlos_user_data_deleter);

void cri_voice_delete_call_obj(cri_voice_call_obj_type *call_obj_ptr);
void cri_voice_free_call_list(cri_voice_call_list_type** call_list_dptr);

#endif
