/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_CALL_OBJ
#define CRI_VOICE_CALL_OBJ

#include "cri_voice.h"
#include "utils_common.h"
#include "voice_service_v02.h"

// creation
cri_voice_call_obj_type* cri_voice_call_obj_create_call_object(
    uint8_t qmi_call_id,
    uint8_t cri_call_id,
    cri_voice_call_obj_bit_field_type initial_bit
);
cri_voice_call_obj_type* cri_voice_call_obj_create_empty_call_object();

// destruction
void cri_voice_call_obj_destruct(cri_voice_call_obj_type** call_obj_dptr);

// relationship change
void cri_voice_call_obj_add_child_relationship(
    cri_voice_call_obj_type * child_ptr,
    cri_voice_call_obj_type * parent_ptr
);
void cri_voice_call_obj_add_parent_relationship(
    cri_voice_call_obj_type * parent_ptr,
    cri_voice_call_obj_type * child_ptr
);
void cri_voice_call_obj_remove_child_relationship_with_others(
    cri_voice_call_obj_type * call_obj_ptr
);
void cri_voice_call_obj_remove_parent_relationship_with_others(
    cri_voice_call_obj_type * call_obj_ptr
);

// call obj state checking
boolean cri_voice_call_obj_has_child(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_has_parent(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_cs(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_ps(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_3gpp(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_3gpp2(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_fg(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_bg(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_emergency_or_emergency_ip(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_ended(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_modem_call(
    const cri_voice_call_obj_type *call_obj_ptr
);
boolean cri_voice_call_obj_is_hlos_call(
    const cri_voice_call_obj_type *call_obj_ptr
);

// call elaboration
void cri_voice_call_obj_set_call_bit(
    cri_voice_call_obj_type* call_obj_ptr,
    cri_voice_call_obj_bit_field_type bit
);
void cri_voice_call_obj_unset_call_bit(
    cri_voice_call_obj_type* call_obj_ptr,
    cri_voice_call_obj_bit_field_type bit
);
boolean cri_voice_call_obj_is_call_bit_set(
    const cri_voice_call_obj_type * call_obj_ptr,
    cri_voice_call_obj_bit_field_type bit
);
boolean cri_voice_call_obj_is_call_bit_unset(
    const cri_voice_call_obj_type * call_obj_ptr,
    cri_voice_call_obj_bit_field_type bit
);

// logging
void cri_voice_call_obj_dump_call(const cri_voice_call_obj_type* call_obj_ptr);

// update
void cri_voice_call_obj_set_qmi_call_id(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t qmi_call_id
);
void cri_voice_call_obj_update_call_obj(
    cri_voice_call_obj_type* call_obj_ptr,
    const voice_call_info2_type_v02* call_info,
    uint8_t remote_party_number_valid,
    const voice_remote_party_number2_type_v02* remote_party_number,
    uint8_t remote_party_name_valid,
    const voice_remote_party_name2_type_v02* remote_party_name,
    uint8_t alerting_type_valid,
    const voice_alerting_type_type_v02* alerting_type,
    uint8_t srv_opt_valid,
    const voice_srv_opt_type_v02* srv_opt,
    uint8_t call_end_reason_valid,
    const voice_call_end_reason_type_v02* call_end_reason,
    uint8_t alpha_id_valid,
    const voice_alpha_ident_with_id_type_v02* alpha_id,
    uint8_t conn_party_num_valid,
    const voice_conn_num_with_id_type_v02* conn_party_num,
    uint8_t diagnostic_info_valid,
    const voice_diagnostic_info_with_id_type_v02* diagnostic_info,
    uint8_t called_party_num_valid,
    const voice_num_with_id_type_v02* called_party_num,
    uint8_t redirecting_party_num_valid,
    const voice_num_with_id_type_v02* redirecting_party_num,
    uint8_t cri_call_state_valid,
    const cri_voice_call_state_type cri_call_state,
    uint8_t audio_attrib_valid,
    const voice_call_attributes_type_v02 *audio_attrib,
    uint8_t video_attrib_valid,
    const voice_call_attributes_type_v02 *video_attrib,
    uint8_t is_srvcc_valid,
    const voice_is_srvcc_call_with_id_type_v02 *is_srvcc
);

void cri_voice_call_obj_update_call_info(
    cri_voice_call_obj_type* call_obj_ptr,
    const voice_call_info2_type_v02* call_info
);
void cri_voice_call_obj_update_remote_party_number(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t remote_party_number_valid,
    const voice_remote_party_number2_type_v02* remote_party_number
);
void cri_voice_call_obj_update_remote_party_name(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t remote_party_name_valid,
    const voice_remote_party_name2_type_v02* remote_party_name
);
void cri_voice_call_obj_update_alerting_type(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t alerting_type_valid,
    const voice_alerting_type_type_v02* alerting_type
);
void cri_voice_call_obj_update_srv_opt(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t srv_opt_valid,
    const voice_srv_opt_type_v02* srv_opt
);
void cri_voice_call_obj_update_call_end_reason(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t call_end_reason_valid,
    const voice_call_end_reason_type_v02* call_end_reason
);
void cri_voice_call_obj_update_alpha_id(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t alpha_id_valid,
    const voice_alpha_ident_with_id_type_v02* alpha_id
);
void cri_voice_call_obj_update_conn_party_num(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t conn_party_num_valid,
    const voice_conn_num_with_id_type_v02* conn_party_num
);
void cri_voice_call_obj_update_diagnostic_info(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t diagnostic_info_valid,
    const voice_diagnostic_info_with_id_type_v02* diagnostic_info
);
void cri_voice_call_obj_update_called_party_num(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t called_party_num_valid,
    const voice_num_with_id_type_v02* called_party_num
);
void cri_voice_call_obj_update_redirecting_party_num(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t redirecting_party_num_valid,
    const voice_num_with_id_type_v02* redirecting_party_num
);
void cri_voice_call_obj_update_cri_call_state(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t cri_call_state_valid,
    const cri_voice_call_state_type cri_call_state
);
void cri_voice_call_obj_update_audio_attrib(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t audio_attrib_valid,
    const voice_call_attributes_type_v02 *audio_attrib
);
void cri_voice_call_obj_update_video_attrib(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t video_attrib_valid,
    const voice_call_attributes_type_v02 *video_attrib
);
void cri_voice_call_obj_update_is_srvcc(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t is_srvcc_valid,
    const voice_is_srvcc_call_with_id_type_v02 *is_srvcc
);

cri_core_error_type cri_voice_call_obj_update_call_type_domain(
    cri_voice_call_obj_type *call_obj_ptr
);

void cri_voice_call_obj_update_remote_party_number_by_each_fields(
    cri_voice_call_obj_type* call_obj_ptr,
    boolean call_id_valid,
    uint8_t call_id,
    boolean number_pi_valid,
    pi_num_enum_v02 number_pi,
    boolean call_num_valid,
    const char* call_num,
    uint32_t call_num_len,
    voice_num_type_enum_v02 num_type
);
void cri_voice_call_obj_update_remote_party_name_by_each_fields(
    cri_voice_call_obj_type* call_obj_ptr,
    boolean name_pi_valid,
    pi_name_enum_v02 name_pi,
    boolean call_name_valid,
    const char* call_name,
    uint32_t call_name_len
);

void cri_voice_call_obj_set_hlos_data_and_deleter(
    cri_voice_call_obj_type* call_obj_ptr,
    void* user_data,
    hlos_user_data_deleter_type hlos_user_data_deleter
);

#endif
