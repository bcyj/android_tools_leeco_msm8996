/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_CALL_LIST
#define CRI_VOICE_CALL_LIST

#include "cri_voice_call_obj.h"
#include "utils_common.h"
#include "voice_service_v02.h"

util_list_info_type* cri_voice_call_list_create();

// create new call obj and add it to list
cri_voice_call_obj_type* cri_voice_call_list_add_new_call_object(
    util_list_info_type* call_list_ptr,
    uint8_t qmi_call_id,
    boolean need_cri_call_id,
    cri_voice_call_obj_bit_field_type bit
);
cri_voice_call_obj_type* cri_voice_call_list_add_new_empty_call_object(
    util_list_info_type* call_list_ptr
);

// delete a call object
cri_core_error_type cri_voice_call_list_delete_call_by_cri_call_id(util_list_info_type* call_list_ptr, uint8_t cri_call_id);
cri_core_error_type cri_voice_call_list_delete_by_qmi_id(util_list_info_type* call_list_ptr, uint8_t cri_call_id);
cri_core_error_type cri_voice_call_list_delete(util_list_info_type* call_list_ptr, cri_voice_call_obj_type* call_obj_ptr);

// get a call object
cri_voice_call_obj_type* cri_voice_call_list_find_by_qmi_call_id(const util_list_info_type* call_list_ptr, uint8_t qmi_call_id);
cri_voice_call_obj_type* cri_voice_call_list_find_by_cri_call_id(const util_list_info_type* call_list_ptr, uint8_t cri_call_id);
cri_voice_call_obj_type* cri_voice_call_list_find_by_qmi_call_state(const util_list_info_type* call_list_ptr, call_state_enum_v02 qmi_call_state);
cri_voice_call_obj_type* cri_voice_call_list_find_by_cri_call_state(const util_list_info_type* call_list_ptr, cri_voice_call_state_type cri_call_state);
cri_voice_call_obj_type* cri_voice_call_list_find_by_call_bit(const util_list_info_type* call_list_ptr, cri_voice_call_obj_bit_field_type elab);

// get call objects
typedef boolean (*cri_voice_call_obj_filter_type)(const cri_voice_call_obj_type*);
void cri_voice_call_list_get_filtered_call_objects(const util_list_info_type* call_list_ptr, cri_voice_call_obj_filter_type filter, uint32 *num_of_calls, cri_voice_call_obj_type*** call_obj_tptr);
void cri_voice_call_list_get_filtered_call_objects_with_filter_param(const util_list_info_type* call_list_ptr, boolean (*filter)(const cri_voice_call_obj_type*, const void* filter_param), const void* filter_param, uint32 *num_of_calls, cri_voice_call_obj_type*** call_obj_tptr);

// logging
void cri_voice_call_list_dump_by_cri_call_id(const util_list_info_type* call_list_ptr, uint8_t cri_call_id);
void cri_voice_call_list_dump(const util_list_info_type* call_list_ptr);

#endif
