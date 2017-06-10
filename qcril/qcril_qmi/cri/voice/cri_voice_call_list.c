/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_call_list.h"
#include "cri_voice_core.h"

static cri_core_error_type cri_voice_call_list_add(util_list_info_type* call_list_ptr, const cri_voice_call_obj_type* call_obj_ptr);
static cri_core_error_type cri_voice_call_list_allocate_cri_call_id(const util_list_info_type* call_list_ptr, uint8_t* new_cri_call_id_ptr);

static void cri_voice_call_list_delete_node(util_list_node_data_type *param);

static util_list_node_data_type* cri_voice_call_list_find_node(const util_list_info_type* call_list_ptr, int (*evaluator)(const util_list_node_data_type* node_data, void* data), const void* data);
static cri_voice_call_obj_type* cri_voice_call_list_find_call_obj(const util_list_info_type* call_list_ptr, int (*evaluator)(const util_list_node_data_type* node_data, void* data), const void* data);

int cri_voice_call_list_add_evaluator(util_list_node_data_type *to_be_added_data,
                                      util_list_node_data_type *to_be_evaluated_data)
{
    return FALSE;
}

util_list_info_type* cri_voice_call_list_create()
{
    return util_list_create( NULL, cri_voice_call_list_add_evaluator, cri_voice_call_list_delete_node, UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP );
}

cri_voice_call_obj_type* cri_voice_call_list_add_new_call_object(
    util_list_info_type* call_list_ptr,
    uint8_t qmi_call_id,
    boolean need_cri_call_id,
    cri_voice_call_obj_bit_field_type bit
)
{
    uint8 cri_call_id = CRI_VOICE_INVALID_CALL_ID;
    cri_voice_call_obj_type* call_obj_ptr = NULL;

    do
    {
        if (need_cri_call_id)
        {
            if (cri_voice_call_list_allocate_cri_call_id(call_list_ptr, &cri_call_id))
            {
                break;
            }
        }

        call_obj_ptr = cri_voice_call_obj_create_call_object(qmi_call_id, cri_call_id, bit);
        cri_voice_call_list_add(call_list_ptr, call_obj_ptr);
    }
    while (0);

    return call_obj_ptr;
}

cri_voice_call_obj_type* cri_voice_call_list_add_new_empty_call_object(
    util_list_info_type* call_list_ptr
)
{
    cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_obj_create_empty_call_object();
    cri_voice_call_list_add(call_list_ptr, call_obj_ptr);

    return call_obj_ptr;
}

cri_core_error_type cri_voice_call_list_add(util_list_info_type* call_list_ptr, const cri_voice_call_obj_type* call_obj_ptr)
{
    if (call_list_ptr && call_obj_ptr)
    {
        util_list_add(call_list_ptr, (void*)call_obj_ptr, NULL, 0);
        return 0;
    }
    else
    {
        return QMI_ERR_INTERNAL_V01;
    }
}

cri_core_error_type cri_voice_call_list_allocate_cri_call_id(const util_list_info_type* call_list_ptr, uint8_t* new_cri_call_id_ptr)
{
    cri_core_error_type err = QMI_ERR_INTERNAL_V01;
    uint8_t cri_call_id;
    if ( new_cri_call_id_ptr )
    {
        *new_cri_call_id_ptr = CRI_VOICE_INVALID_CALL_ID;
        for ( cri_call_id = CRI_VOICE_LOWEST_CALL_ID; cri_call_id <= CRI_VOICE_HIGHEST_CALL_ID; cri_call_id++ )
        {
            if ( NULL == cri_voice_call_list_find_by_cri_call_id(call_list_ptr, cri_call_id) )
            {
                QCRIL_LOG_ERROR("allocated cri call id: %d", cri_call_id);
                *new_cri_call_id_ptr = cri_call_id;
                err = 0;
                break;
            }
        }
    }
    else
    {
        QCRIL_LOG_ERROR("new_cri_call_id_ptr is NULL");
    }
    return err;
}

void cri_voice_call_list_delete_node(util_list_node_data_type *param)
{
    if (param)
    {
        cri_voice_call_obj_type *call_obj_ptr = (cri_voice_call_obj_type *) param->user_data;

        cri_voice_call_obj_destruct(&call_obj_ptr);
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
}

util_list_node_data_type* cri_voice_call_list_find_node(const util_list_info_type* call_list_ptr, int (*evaluator)(const util_list_node_data_type* node_data, void* data), const void* data)
{
    util_list_node_data_type* node_data_ptr = util_list_find_data_in_list_with_param(call_list_ptr, evaluator, data);
    return node_data_ptr;
}

cri_voice_call_obj_type* cri_voice_call_list_find_call_obj(const util_list_info_type* call_list_ptr, int (*evaluator)(const util_list_node_data_type* node_data, void* data), const void* data)
{
    cri_voice_call_obj_type* call_obj_ptr = NULL;
    util_list_node_data_type *node_data_ptr = cri_voice_call_list_find_node(call_list_ptr, evaluator, data);
    if (node_data_ptr)
    {
        call_obj_ptr = (cri_voice_call_obj_type*)(node_data_ptr->user_data);
    }
    return call_obj_ptr;
}

int cri_voice_compare_call_obj_cri_call_id(const util_list_node_data_type* node_data_ptr, void* cri_call_id_param)
{
    int ret = FALSE;
    uint8 cri_call_id = (uint8)((uint32)(uintptr_t)cri_call_id_param);
    if (node_data_ptr && node_data_ptr->user_data)
    {
        cri_voice_call_obj_type *call_obj_ptr = (cri_voice_call_obj_type *)node_data_ptr->user_data;
        ret = (call_obj_ptr->cri_call_id == cri_call_id) ? TRUE : FALSE;
    }
    return ret;
}

cri_voice_call_obj_type* cri_voice_call_list_find_by_cri_call_id(const util_list_info_type* call_list_ptr, uint8_t cri_call_id)
{
    return cri_voice_call_list_find_call_obj(call_list_ptr, cri_voice_compare_call_obj_cri_call_id, (void*)((uintptr_t)cri_call_id));
}

int cri_voice_compare_call_obj_qmi_voice_call_id(const util_list_node_data_type* node_data_ptr, void* cri_call_id_param)
{
    int ret = FALSE;
    uint8 qmi_call_id = (uint8)((uint32)(uintptr_t)cri_call_id_param);
    if (node_data_ptr && node_data_ptr->user_data)
    {
        cri_voice_call_obj_type *call_obj_ptr = (cri_voice_call_obj_type *)node_data_ptr->user_data;
        ret = (call_obj_ptr->qmi_call_id == qmi_call_id) ? TRUE : FALSE;
    }
    return ret;
}

cri_voice_call_obj_type* cri_voice_call_list_find_by_qmi_call_id(const util_list_info_type* call_list_ptr, uint8_t qmi_call_id)
{
    // TODO: HANDLE csvt
    return cri_voice_call_list_find_call_obj(call_list_ptr, cri_voice_compare_call_obj_qmi_voice_call_id, (void*)((uintptr_t)qmi_call_id));
}

int cri_voice_compare_call_obj_qmi_call_state(const util_list_node_data_type* node_data_ptr, void* param)
{
    int ret = FALSE;
    call_state_enum_v02 qmi_call_state = (call_state_enum_v02)param;
    if (node_data_ptr && node_data_ptr->user_data)
    {
        cri_voice_call_obj_type *call_obj_ptr = (cri_voice_call_obj_type *)node_data_ptr->user_data;
        ret = (qmi_call_state == call_obj_ptr->qmi_voice_scv_info.call_state) ? TRUE : FALSE;
    }
    return ret;
}

cri_voice_call_obj_type* cri_voice_call_list_find_by_qmi_call_state(const util_list_info_type* call_list_ptr, call_state_enum_v02 qmi_call_state)
{
    return cri_voice_call_list_find_call_obj(call_list_ptr, cri_voice_compare_call_obj_qmi_call_state, (void*)((uintptr_t)qmi_call_state));
}

int cri_voice_compare_call_obj_cri_call_state(const util_list_node_data_type* node_data_ptr, void* param)
{
    int ret = FALSE;
    cri_voice_call_state_type cri_call_state = (cri_voice_call_state_type)param;
    if (node_data_ptr && node_data_ptr->user_data)
    {
        cri_voice_call_obj_type *call_obj_ptr = (cri_voice_call_obj_type *)node_data_ptr->user_data;
        ret = (cri_call_state == call_obj_ptr->cri_call_state) ? TRUE : FALSE;
    }
    return ret;
}

cri_voice_call_obj_type* cri_voice_call_list_find_by_cri_call_state(const util_list_info_type* call_list_ptr, cri_voice_call_state_type cri_call_state)
{
    return cri_voice_call_list_find_call_obj(call_list_ptr, cri_voice_compare_call_obj_cri_call_state, (void*)((uintptr_t)cri_call_state));
}

void cri_voice_call_list_get_filtered_call_objects(const util_list_info_type* call_list_ptr, cri_voice_call_obj_filter_type filter, uint32 *num_of_calls, cri_voice_call_obj_type*** call_obj_tptr)
{
    *call_obj_tptr = NULL;
    *num_of_calls = 0;

    QCRIL_LOG_FUNC_ENTRY();
    util_list_node_type *iter = call_list_ptr->list_head;

    while (iter)
    {
        if (filter((const cri_voice_call_obj_type*) (iter->node_data.user_data)))
        {
            (*num_of_calls)++;
        }
        iter = iter->next;
    }

    QCRIL_LOG_INFO("num_of_calls: %d", *num_of_calls);

    if (*num_of_calls)
    {
        *call_obj_tptr = util_memory_alloc(sizeof(cri_voice_call_obj_type *) * (*num_of_calls));
        if (*call_obj_tptr)
        {
            uint32 idx = 0;
            iter = call_list_ptr->list_head;
            while (iter)
            {
                if (filter((const cri_voice_call_obj_type*) (iter->node_data.user_data)))
                {
                    (*call_obj_tptr)[idx] = (cri_voice_call_obj_type *)(iter->node_data.user_data);
                    idx++;
                }
                iter = iter->next;
            }
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

void cri_voice_call_list_get_filtered_call_objects_with_filter_param(const util_list_info_type* call_list_ptr, boolean (*filter)(const cri_voice_call_obj_type*, const void* filter_param), const void* filter_param, uint32 *num_of_calls, cri_voice_call_obj_type*** call_obj_tptr)
{
    *call_obj_tptr = NULL;
    *num_of_calls = 0;

    QCRIL_LOG_FUNC_ENTRY();
    util_list_node_type *iter = call_list_ptr->list_head;

    while (iter)
    {
        if (filter((const cri_voice_call_obj_type*) (iter->node_data.user_data), filter_param))
        {
            (*num_of_calls)++;
        }
        iter = iter->next;
    }

    QCRIL_LOG_INFO("num_of_calls: %d", *num_of_calls);

    if (*num_of_calls)
    {
        *call_obj_tptr = util_memory_alloc(sizeof(cri_voice_call_obj_type *) * (*num_of_calls));
        if (*call_obj_tptr)
        {
            uint32 idx = 0;
            iter = call_list_ptr->list_head;
            while (iter)
            {
                if (filter((const cri_voice_call_obj_type*) (iter->node_data.user_data), filter_param))
                {
                    (*call_obj_tptr)[idx] = (cri_voice_call_obj_type *)(iter->node_data.user_data);
                    idx++;
                }
                iter = iter->next;
            }
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

cri_voice_call_obj_type* cri_voice_call_list_find_by_call_bit(const util_list_info_type* call_list_ptr, cri_voice_call_obj_bit_field_type bit)
{
    cri_voice_call_obj_type* ret = NULL;

    uint32 num_of_calls;
    cri_voice_call_obj_type **call_obj_dptr;
    cri_voice_call_list_get_filtered_call_objects_with_filter_param(
        call_list_ptr,
        (boolean (*)(const cri_voice_call_obj_type*, const void* filter_param))cri_voice_call_obj_is_call_bit_set,
        (void*) bit,
        &num_of_calls,
        &call_obj_dptr
    );

    if (num_of_calls > 1)
    {
        QCRIL_LOG_DEBUG("more than one call found with bit: %d", bit);
    }

    if (num_of_calls >= 1 && call_obj_dptr)
    {
        ret = call_obj_dptr[0];
        util_memory_free((void**) &call_obj_dptr);
    }

    return ret;
}

void cri_voice_call_list_dump_by_cri_call_id(const util_list_info_type* call_list_ptr, uint8_t cri_call_id)
{
    cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_list_find_by_cri_call_id(call_list_ptr, cri_call_id);
    if (call_obj_ptr)
    {
        QCRIL_LOG_ESSENTIAL("dump call with cri_call_id: %d", cri_call_id);
        cri_voice_call_obj_dump_call(call_obj_ptr);
    }
    else
    {
        QCRIL_LOG_INFO("do not have call with cri_call_id: %d", cri_call_id);
    }
}

void cri_voice_call_list_dump(const util_list_info_type* call_list_ptr)
{
    util_list_node_type* iter = call_list_ptr->list_head;
    QCRIL_LOG_ESSENTIAL("dump all calls");
    while (iter)
    {
        cri_voice_call_obj_dump_call((cri_voice_call_obj_type* )iter->node_data.user_data);
        iter = iter->next;
    }
}

cri_core_error_type cri_voice_call_list_delete_call_by_cri_call_id(util_list_info_type* call_list_ptr, uint8_t cri_call_id)
{
    QCRIL_LOG_FUNC_ENTRY();
    util_list_node_data_type *node_ptr = cri_voice_call_list_find_node(call_list_ptr, cri_voice_compare_call_obj_cri_call_id, (void*)((uintptr_t)cri_call_id));
    QCRIL_LOG_INFO("cri_call_id: %d, node_ptr: %p", cri_call_id, node_ptr);
    util_list_delete(call_list_ptr, node_ptr, NULL);
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

cri_core_error_type cri_voice_call_list_delete(util_list_info_type* call_list_ptr, cri_voice_call_obj_type* call_obj_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();
    util_list_delete_data_from_list_by_user_data(call_list_ptr, call_obj_ptr, NULL);
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

