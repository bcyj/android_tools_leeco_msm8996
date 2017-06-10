/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_call_obj.h"
#include "cri_voice_core.h"
#include "cri_voice_settings.h"
#include "cri_voice_utils.h"
//#include "qcril_cm_ss.h"
static void cri_voice_call_obj_set_ps_cs_call_bit
(
    cri_voice_call_obj_type *call_obj_ptr,
    const voice_call_info2_type_v02* qmi_voice_call_info_ptr
);

cri_voice_call_obj_type* cri_voice_call_obj_create_call_object(
    uint8_t qmi_call_id,
    uint8_t cri_call_id,
    cri_voice_call_obj_bit_field_type initial_bit
)
{
    cri_voice_call_obj_type* call_obj_ptr = NULL;
    call_obj_ptr = cri_voice_call_obj_create_empty_call_object();
    if (call_obj_ptr)
    {
        cri_voice_call_obj_set_qmi_call_id(call_obj_ptr, qmi_call_id);
        call_obj_ptr->cri_call_id = cri_call_id;
        cri_voice_call_obj_set_call_bit(call_obj_ptr, initial_bit);
    }
    return call_obj_ptr;
}

cri_voice_call_obj_type* cri_voice_call_obj_create_empty_call_object()
{
    cri_voice_call_obj_type* call_obj_ptr =
        (cri_voice_call_obj_type*)util_memory_alloc(sizeof(cri_voice_call_obj_type));
    if (call_obj_ptr)
    {
        call_obj_ptr->cri_call_id = CRI_VOICE_INVALID_CALL_ID;
        call_obj_ptr->qmi_call_id = CRI_VOICE_INVALID_CALL_ID;
    }
    else
    {
        QCRIL_LOG_ERROR("can't not alloc memory for call obj");
    }
    return call_obj_ptr;
}

void cri_voice_call_obj_destruct(cri_voice_call_obj_type** call_obj_dptr)
{
    if (call_obj_dptr)
    {
        cri_voice_call_obj_type *call_obj_ptr = *call_obj_dptr;
        cri_voice_call_obj_remove_child_relationship_with_others (call_obj_ptr);
        cri_voice_call_obj_remove_parent_relationship_with_others(call_obj_ptr);

        if (call_obj_ptr->hlos_user_data)
        {
            hlos_user_data_deleter_type hlos_user_data_deleter;
            if (call_obj_ptr->hlos_user_data_deleter_valid)
            {
                hlos_user_data_deleter = call_obj_ptr->hlos_user_data_deleter;
            }
            else
            {
                hlos_user_data_deleter = cri_voice_settings_get_default_hlos_user_data_deleter(cri_voice_core_get_settings());
            }

            if (hlos_user_data_deleter)
            {
                hlos_user_data_deleter((void**)&call_obj_ptr->hlos_user_data);
            }
        }
        util_list_cleanup(call_obj_ptr->cri_child_call_obj_list_ptr, NULL);
        util_list_cleanup(call_obj_ptr->cri_parent_call_obj_list_ptr, NULL);
        util_memory_free((void**)call_obj_dptr);
    }
}

void cri_voice_call_obj_add_child_relationship(
    cri_voice_call_obj_type * child_ptr,
    cri_voice_call_obj_type * parent_ptr
)
{
    if (!child_ptr || !parent_ptr)
    {
        QCRIL_LOG_DEBUG("child_ptr || parent_ptr is NULL");
        return;
    }

    if (!child_ptr->cri_parent_call_obj_list_ptr)
    {
        child_ptr->cri_parent_call_obj_list_ptr = util_list_create(
            NULL,
            NULL,
            NULL,
            UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP
        );
    }
    util_list_add(child_ptr->cri_parent_call_obj_list_ptr,
                  parent_ptr,
                  NULL,
                  NIL);

    if (!parent_ptr->cri_child_call_obj_list_ptr)
    {
        parent_ptr->cri_child_call_obj_list_ptr = util_list_create(
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP
                                              );
    }
    util_list_add(parent_ptr->cri_child_call_obj_list_ptr,
                               child_ptr,
                               NULL,
                               NIL);
}

void cri_voice_call_obj_add_parent_relationship(
    cri_voice_call_obj_type * parent_ptr,
    cri_voice_call_obj_type * child_ptr
)
{
    cri_voice_call_obj_add_child_relationship(child_ptr,
                                                       parent_ptr);
}

void cri_voice_call_obj_remove_parent_relationship_with_others(
    cri_voice_call_obj_type * call_obj_ptr
)
{
    if (!call_obj_ptr)
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
        return;
    }

    if (call_obj_ptr->cri_child_call_obj_list_ptr)
    {
        util_list_node_type *child_node_iter = call_obj_ptr->cri_child_call_obj_list_ptr->list_head;
        while (child_node_iter)
        {
            cri_voice_call_obj_type *child_call_obj_ptr =
                (cri_voice_call_obj_type *)child_node_iter->node_data.user_data;
            if (child_call_obj_ptr)
            {
                util_list_delete_data_from_list_by_user_data(
                    child_call_obj_ptr->cri_parent_call_obj_list_ptr,
                    call_obj_ptr,
                NULL);
            }
            child_node_iter = child_node_iter->next;
        }
        util_list_cleanup(call_obj_ptr->cri_child_call_obj_list_ptr, NULL);
    }
}

void cri_voice_call_obj_remove_child_relationship_with_others(
    cri_voice_call_obj_type * call_obj_ptr
)
{
    if (!call_obj_ptr)
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
        return;
    }

    if (call_obj_ptr->cri_parent_call_obj_list_ptr)
    {
        util_list_node_type *parent_node_iter = call_obj_ptr->cri_parent_call_obj_list_ptr->list_head;
        while (parent_node_iter)
        {
            cri_voice_call_obj_type *parent_call_obj_ptr =
                 (cri_voice_call_obj_type *)parent_node_iter->node_data.user_data;
            if (parent_call_obj_ptr)
            {
                util_list_delete_data_from_list_by_user_data(
                    parent_call_obj_ptr->cri_child_call_obj_list_ptr,
                    call_obj_ptr,
                    NULL
                );
                /*
                if (!cri_voice_call_obj_has_child(parent_call_obj_ptr)) // parent call should already gone
                {
                    util_list_delete_data_from_list_by_user_data(call_info.call_list_ptr, (void*) parent_call_obj_ptr, NULL);
                }
                */
            }
            parent_node_iter = parent_node_iter->next;
        }

        util_list_cleanup(call_obj_ptr->cri_child_call_obj_list_ptr,
                          NULL);
    }
}

boolean cri_voice_call_obj_has_child(const cri_voice_call_obj_type *call_obj_ptr)
{
    boolean ret = FALSE;
    if (call_obj_ptr)
    {
        if (call_obj_ptr->cri_child_call_obj_list_ptr &&
            call_obj_ptr->cri_child_call_obj_list_ptr->num_of_node)
        {
            ret = TRUE;
        }
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
    return ret;
}

boolean cri_voice_call_obj_has_parent(
    const cri_voice_call_obj_type *call_obj_ptr
)
{
    boolean ret = FALSE;
    if (call_obj_ptr)
    {
        if (call_obj_ptr->cri_child_call_obj_list_ptr &&
            call_obj_ptr->cri_child_call_obj_list_ptr->num_of_node)
        {
            ret = TRUE;
        }
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
    return ret;
}


boolean cri_voice_call_obj_is_cs(const cri_voice_call_obj_type *call_obj_ptr)
{
    return cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_CS_DOMAIN);
}

boolean cri_voice_call_obj_is_ps(const cri_voice_call_obj_type *call_obj_ptr)
{
    return cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_PS_DOMAIN);
}

boolean cri_voice_call_obj_is_3gpp(const cri_voice_call_obj_type *call_obj_ptr)
{
    return call_obj_ptr && (CALL_MODE_CDMA_V02 != call_obj_ptr->qmi_voice_scv_info.mode);
}

boolean cri_voice_call_obj_is_3gpp2(const cri_voice_call_obj_type *call_obj_ptr)
{
    return call_obj_ptr && (CALL_MODE_CDMA_V02 == call_obj_ptr->qmi_voice_scv_info.mode);
}

boolean cri_voice_call_obj_is_fg(const cri_voice_call_obj_type *call_obj_ptr)
{
    return call_obj_ptr && (CALL_STATE_CONVERSATION_V02 == call_obj_ptr->qmi_voice_scv_info.call_state);
}

boolean cri_voice_call_obj_is_bg(const cri_voice_call_obj_type *call_obj_ptr)
{
    return call_obj_ptr &&
           ( CALL_STATE_HOLD_V02 == call_obj_ptr->qmi_voice_scv_info.call_state ||
             CALL_STATE_WAITING_V02 == call_obj_ptr->qmi_voice_scv_info.call_state ||
             CALL_STATE_INCOMING_V02 == call_obj_ptr->qmi_voice_scv_info.call_state );
}

boolean cri_voice_call_obj_is_emergency_or_emergency_ip(const cri_voice_call_obj_type *call_obj_ptr)
{
    return call_obj_ptr &&
           ( CALL_TYPE_EMERGENCY_V02 == call_obj_ptr->qmi_voice_scv_info.call_type ||
             CALL_TYPE_EMERGENCY_IP_V02 == call_obj_ptr->qmi_voice_scv_info.call_type );
}

boolean cri_voice_call_obj_is_ended(const cri_voice_call_obj_type *call_obj_ptr)
{
    return call_obj_ptr && (CALL_STATE_END_V02 == call_obj_ptr->qmi_voice_scv_info.call_state);
}

boolean cri_voice_call_obj_is_modem_call(const cri_voice_call_obj_type *call_obj_ptr)
{
    boolean ret = FALSE;
    if (call_obj_ptr)
    {
        if ( /*( cri_voice_call_obj_is_call_bit_set(call_obj_ptr,
                                                  CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALL_ID_RECEIVED) )&&
             CRI_VOICE_INVALID_CALL_ID != call_i_obj_ptr->call_obj.qmi_call_id &&*/
             !cri_voice_call_obj_is_ended(call_obj_ptr)
           )
        {
            ret = TRUE;
        }
        QCRIL_LOG_INFO("call hlos id: %d, qmi id: %d, state: %d, ret: %d",
                       call_obj_ptr->cri_call_id,
                       call_obj_ptr->qmi_call_id,
                       call_obj_ptr->qmi_voice_scv_info.call_state,
                       ret);
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
    return ret;
}

boolean cri_voice_call_obj_is_hlos_call(const cri_voice_call_obj_type *call_obj_ptr)
{
    boolean ret = FALSE;
    if (call_obj_ptr)
    {
        if ( CRI_VOICE_INVALID_CALL_ID != call_obj_ptr->cri_call_id &&             // not a shadow call
             CALL_TYPE_OTAPA_V02 != call_obj_ptr->qmi_voice_scv_info.call_type && // notan OTAPA call
             cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_CRI_CALL_STATE_VALID) &&
             //(cri_voice_call_obj_is_emergency_or_emergency_ip(call_obj_ptr) || !qmi_ril_voice_is_calls_supressed_by_pil_vcl() &&
             cri_voice_call_obj_is_call_bit_unset(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING)
           )
        {
            ret = TRUE;
        }
        QCRIL_LOG_INFO("call cri id: %d, qmi id: %d, state: %d, ret: %d", call_obj_ptr->cri_call_id, call_obj_ptr->qmi_call_id,call_obj_ptr->qmi_voice_scv_info.call_state, ret);
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
    return ret;
}


cri_core_error_type cri_voice_call_obj_update_call_type_domain(
    cri_voice_call_obj_type *call_obj_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();
    cri_core_error_type err = QMI_ERR_NONE_V01;

    if (!call_obj_ptr)
    {
        err = QMI_ERR_INVALID_ARG_V01;
//        QCRIL_LOG_FUNC_RETURN_WITH_RET((int) err);
        return err;
    }

    call_type_enum_v02 qmi_call_type = call_obj_ptr->qmi_voice_scv_info.call_type;
    boolean video_attrib_valid = cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_VIDEO_ATTR_VALID);
    voice_call_attribute_type_mask_v02 video_attrib = call_obj_ptr->qmi_video_attrib;
    QCRIL_LOG_INFO( "QMI call_type: %d, video_attrib_valid: %d, video_attrib: %d",
                    (int)qmi_call_type, (int)video_attrib_valid, (int)video_attrib );

    if ( CALL_TYPE_VT_V02 == qmi_call_type )
    {
        if( FALSE == video_attrib_valid )
        {
            call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VT;
            call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
        }
        else
        {
            /* based on call attributes determine video call type */
            if( VOICE_CALL_ATTRIB_TX_V02 == video_attrib )
            {
                call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VT_TX;
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
            }
            else if( VOICE_CALL_ATTRIB_RX_V02 == video_attrib )
            {
                call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VT_RX;
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
            }
            else if( (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02) == video_attrib )
            {
                call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VT;
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
            }
            else if ( 0 == video_attrib )
            {
                call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VT_NODIR;
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
            }
            else
            {
                QCRIL_LOG_INFO("unexpected video attrib. Set call type/domain to VT/PS as default");
                call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VT;
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
            }
        }
    }
    else // not a VT call
    {
        call_obj_ptr->cri_call_type = CRI_VOICE_CALL_TYPE_VOICE;
        boolean call_domain_set = FALSE;

        call_domain_set = TRUE;
        if (cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_CS_DOMAIN))
        {
            call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_CS;
        }
        else if (cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_PS_DOMAIN))
        {
            call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
        }
        else if (cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_AUTO_DOMAIN))
        {
            call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_AUTOMATIC;
        }
        else
        {
            QCRIL_LOG_DEBUG("did not set call domain in elaboration.");
            call_domain_set = FALSE;
        }

        if (!call_domain_set)
        {
            switch( qmi_call_type )
            {
            case CALL_TYPE_EMERGENCY_IP_V02:
            case CALL_TYPE_VOICE_IP_V02:
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_PS;
                break;

            default:
                /* fall back to default voice case */
                call_obj_ptr->cri_call_domain = CRI_VOICE_CALL_DOMAIN_CS;
                break;
            }
        }
    } // end of "not a VT call"
    QCRIL_LOG_DEBUG( "cri call type = %d, cri call domain = %d", call_obj_ptr->cri_call_type, call_obj_ptr->cri_call_domain );
 //   QCRIL_LOG_FUNC_RETURN_WITH_RET((int) err);
    return err;
} // cri_voice_call_obj_update_call_type_domain

void cri_voice_call_obj_set_call_bit(cri_voice_call_obj_type* call_obj_ptr,
                                      cri_voice_call_obj_bit_field_type bit)
{
    if (call_obj_ptr)
    {
        if (bit < CRI_VOICE_CALL_OBJ_BIT_FIELD_MAX && bit > CRI_VOICE_CALL_OBJ_BIT_FIELD_MIN)
        {
            // To be extend here if more than 64 enums
            util_bit_field_set_bits(&call_obj_ptr->cri_bit_field, (((uint64_t) 1) << bit));
        }
        else
        {
            QCRIL_LOG_DEBUG("bit is not in the valid range");
        }
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
}

void cri_voice_call_obj_unset_call_bit(cri_voice_call_obj_type* call_obj_ptr, cri_voice_call_obj_bit_field_type bit)
{
    if (call_obj_ptr)
    {
        if (bit < CRI_VOICE_CALL_OBJ_BIT_FIELD_MAX && bit > CRI_VOICE_CALL_OBJ_BIT_FIELD_MIN)
        {
            // To be extend here if more than 64 enums
            util_bit_field_remove_bits(&call_obj_ptr->cri_bit_field, (((uint64_t) 1) << bit));
        }
        else
        {
            QCRIL_LOG_DEBUG("bit is not in the valid range");
        }
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
}

boolean cri_voice_call_obj_is_call_bit_set(const cri_voice_call_obj_type * call_obj_ptr,
                                           cri_voice_call_obj_bit_field_type bit)
{
    boolean is_call_bit_set = FALSE;
    if (call_obj_ptr)
    {
        if (bit < CRI_VOICE_CALL_OBJ_BIT_FIELD_MAX && bit > CRI_VOICE_CALL_OBJ_BIT_FIELD_MIN)
        {
            is_call_bit_set = util_bit_field_is_bits_set( call_obj_ptr->cri_bit_field,
                                                           (((uint64_t) 1) << bit),
                                                           TRUE );
        }
        else
        {
            QCRIL_LOG_DEBUG("bit is not in the valid range");
        }
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
    return is_call_bit_set;
}

boolean cri_voice_call_obj_is_call_bit_unset(const cri_voice_call_obj_type * call_obj_ptr,
                                              cri_voice_call_obj_bit_field_type bit)
{
    return call_obj_ptr &&
           (!cri_voice_call_obj_is_call_bit_set(call_obj_ptr, bit));
}


void cri_voice_call_obj_dump_call(const cri_voice_call_obj_type* call_obj_ptr)
{
    if (call_obj_ptr)
    {
        QCRIL_LOG_ESSENTIAL("\tqmi call id: %d, call state: %d, call type: %d, call mode: %d",
                            (int)call_obj_ptr->qmi_call_id,
                            (int)call_obj_ptr->qmi_voice_scv_info.call_state,
                            (int)call_obj_ptr->qmi_voice_scv_info.call_type,
                            (int)call_obj_ptr->qmi_voice_scv_info.mode );
        QCRIL_LOG_ESSENTIAL( "cri_bit_field 0x%x, 0x%x",
                             (uint32) (call_obj_ptr->cri_bit_field >> 32),
                             (uint32)call_obj_ptr->cri_bit_field);
    }
    else
    {
        QCRIL_LOG_DEBUG("call_obj_ptr is NULL");
    }
}

void cri_voice_call_obj_set_qmi_call_id(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t qmi_call_id
)
{
    if (call_obj_ptr)
    {
        call_obj_ptr->qmi_call_id = qmi_call_id;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALL_ID_RECEIVED);

    }
}

void cri_voice_call_obj_update_call_obj(
    cri_voice_call_obj_type* call_obj_ptr,
    const voice_call_info2_type_v02* call_info,
    uint8_t remote_party_number_valid, const voice_remote_party_number2_type_v02* remote_party_number,
    uint8_t remote_party_name_valid, const voice_remote_party_name2_type_v02* remote_party_name,
    uint8_t alerting_type_valid, const voice_alerting_type_type_v02* alerting_type,
    uint8_t srv_opt_valid, const voice_srv_opt_type_v02* srv_opt,
    uint8_t call_end_reason_valid, const voice_call_end_reason_type_v02* call_end_reason,
    uint8_t alpha_id_valid, const voice_alpha_ident_with_id_type_v02* alpha_id,
    uint8_t conn_party_num_valid, const voice_conn_num_with_id_type_v02* conn_party_num,
    uint8_t diagnostic_info_valid, const voice_diagnostic_info_with_id_type_v02* diagnostic_info,
    uint8_t called_party_num_valid, const voice_num_with_id_type_v02* called_party_num,
    uint8_t redirecting_party_num_valid, const voice_num_with_id_type_v02* redirecting_party_num,
    uint8_t cri_call_state_valid, const cri_voice_call_state_type cri_call_state,
    uint8_t audio_attrib_valid, const voice_call_attributes_type_v02 *audio_attrib,
    uint8_t video_attrib_valid, const voice_call_attributes_type_v02 *video_attrib,
    uint8_t is_srvcc_valid, const voice_is_srvcc_call_with_id_type_v02 *is_srvcc
)
{
    QCRIL_LOG_INFO("call_obj_ptr: %p",
                   call_obj_ptr);

    if (call_obj_ptr)
    {
        QCRIL_LOG_INFO("call cri id: %d, call qmi id: %d",
                       (int)call_obj_ptr->cri_call_id,
                       (int)call_obj_ptr->qmi_call_id);

        cri_voice_call_obj_update_call_info(call_obj_ptr, call_info);
        cri_voice_call_obj_update_remote_party_number(call_obj_ptr,remote_party_number_valid,remote_party_number);
        cri_voice_call_obj_update_remote_party_name(call_obj_ptr,remote_party_name_valid,remote_party_name);
        cri_voice_call_obj_update_alerting_type(call_obj_ptr,alerting_type_valid,alerting_type);
        cri_voice_call_obj_update_srv_opt(call_obj_ptr,srv_opt_valid,srv_opt);
        cri_voice_call_obj_update_call_end_reason(call_obj_ptr,call_end_reason_valid,call_end_reason);
        cri_voice_call_obj_update_alpha_id(call_obj_ptr,alpha_id_valid,alpha_id);
        cri_voice_call_obj_update_conn_party_num(call_obj_ptr,conn_party_num_valid,conn_party_num);
        cri_voice_call_obj_update_diagnostic_info(call_obj_ptr,diagnostic_info_valid,diagnostic_info);
        cri_voice_call_obj_update_called_party_num(call_obj_ptr,called_party_num_valid,called_party_num);
        cri_voice_call_obj_update_redirecting_party_num(call_obj_ptr,redirecting_party_num_valid,redirecting_party_num);
        cri_voice_call_obj_update_cri_call_state(call_obj_ptr,cri_call_state_valid,cri_call_state);
        cri_voice_call_obj_update_audio_attrib(call_obj_ptr,audio_attrib_valid,audio_attrib);
        cri_voice_call_obj_update_video_attrib(call_obj_ptr,video_attrib_valid,video_attrib);
        cri_voice_call_obj_update_is_srvcc(call_obj_ptr,is_srvcc_valid,is_srvcc);
    }
    QCRIL_LOG_FUNC_RETURN();
}

void cri_voice_call_obj_update_call_info(
    cri_voice_call_obj_type* call_obj_ptr,
    const voice_call_info2_type_v02* call_info
)
{
    if (call_obj_ptr && call_info)
    {
        call_obj_ptr->qmi_voice_scv_info = *call_info;
        QCRIL_LOG_INFO("call state: %d, call type: %d, call mode: %d",
                       (int)call_obj_ptr->qmi_voice_scv_info.call_state,
                       (int)call_obj_ptr->qmi_voice_scv_info.call_type,
                       (int)call_obj_ptr->qmi_voice_scv_info.mode);
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_SCV_INFO_VALID);

        if (CALL_STATE_CONVERSATION_V02 == call_info->call_state)
        {
            cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                            CRI_VOICE_CALL_OBJ_BIT_FIELD_CALL_GOT_CONNECTED);
        }

        cri_voice_call_obj_set_ps_cs_call_bit(call_obj_ptr, call_info);

        cri_voice_call_obj_update_call_type_domain(call_obj_ptr);
    }
}
void cri_voice_call_obj_update_remote_party_number(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t remote_party_number_valid,
    const voice_remote_party_number2_type_v02* remote_party_number
)
{
    if (call_obj_ptr && remote_party_number_valid && NULL != remote_party_number)
    {
        call_obj_ptr->qmi_remote_party_number = *remote_party_number;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NUMBER_VALID);
    }
}

void cri_voice_call_obj_update_remote_party_name(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t remote_party_name_valid,
    const voice_remote_party_name2_type_v02* remote_party_name
)
{
// NO-OP
#if 0
    if (call_obj_ptr && remote_party_name_valid && NULL != remote_party_name)
    {
        call_obj_ptr->cri_remote_party_name.call_id = remote_party_name->call_id;
        call_obj_ptr->cri_remote_party_name.name_pi = remote_party_name->name_pi;

        *(call_obj_ptr->cri_remote_party_name.name) = 0;
        QCRIL_LOG_INFO("remote party name - coding scheme %d, len %d",
                       remote_party_name->coding_scheme,
                       remote_party_name->name_len);
        call_obj_ptr->cri_remote_party_name.name_len =
            qcril_cm_ss_convert_ussd_string_to_utf8(
                remote_party_name->coding_scheme,
                remote_party_name->name_len,
                (byte *)remote_party_name->name,
                call_obj_ptr->cri_remote_party_name.name);

        if ( !(*call_obj_ptr->cri_remote_party_name.name) ||
             ( call_obj_ptr->cri_remote_party_name.name_len >=
                   sizeof(call_obj_ptr->cri_remote_party_name.name) ) )
        {
            QCRIL_LOG_ERROR("Invalid conversion results, remote name");
        }
        else
        {
            cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                            CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NAME_VALID);
        }
    }
#endif
}

void cri_voice_call_obj_update_alerting_type(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t alerting_type_valid,
    const voice_alerting_type_type_v02* alerting_type
)
{
    if (call_obj_ptr && alerting_type_valid && NULL != alerting_type)
    {
        call_obj_ptr->qmi_alerting_type = *alerting_type;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_ALERTING_TYPE_VALID);
    }
}

void cri_voice_call_obj_update_srv_opt(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t srv_opt_valid,
    const voice_srv_opt_type_v02* srv_opt
)
{
    if (call_obj_ptr && srv_opt_valid && NULL != srv_opt)
    {
        call_obj_ptr->qmi_srv_opt = *srv_opt;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_SRV_OPT_VALID);
    }
}

void cri_voice_call_obj_update_call_end_reason(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t call_end_reason_valid,
    const voice_call_end_reason_type_v02* call_end_reason
)
{
    if (call_obj_ptr && call_end_reason_valid && NULL != call_end_reason)
    {
        call_obj_ptr->qmi_call_end_reason = *call_end_reason;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALL_END_REASON_VALID);
    }
}

void cri_voice_call_obj_update_alpha_id(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t alpha_id_valid,
    const voice_alpha_ident_with_id_type_v02* alpha_id
)
{
    if (call_obj_ptr && alpha_id_valid && NULL != alpha_id)
    {
        call_obj_ptr->qmi_alpha_id = *alpha_id;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_ALPHA_ID_VALID);
    }
}

void cri_voice_call_obj_update_conn_party_num(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t conn_party_num_valid,
    const voice_conn_num_with_id_type_v02* conn_party_num
)
{
    if (call_obj_ptr && conn_party_num_valid && NULL != conn_party_num)
    {
        call_obj_ptr->qmi_conn_party_num = *conn_party_num;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CONN_PARTY_NUM_VALID);
    }
}

void cri_voice_call_obj_update_diagnostic_info(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t diagnostic_info_valid,
    const voice_diagnostic_info_with_id_type_v02* diagnostic_info
)
{
    if (call_obj_ptr && diagnostic_info_valid && NULL != diagnostic_info)
    {
        call_obj_ptr->qmi_diagnostic_info = *diagnostic_info;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_DIAGNOSTIC_INFO_VALID);
    }
}

void cri_voice_call_obj_update_called_party_num(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t called_party_num_valid,
    const voice_num_with_id_type_v02* called_party_num
)
{
    if (call_obj_ptr && called_party_num_valid && NULL != called_party_num)
    {
        call_obj_ptr->qmi_called_party_num = *called_party_num;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_CALLED_PARTY_NUM_VALID);
    }
}

void cri_voice_call_obj_update_redirecting_party_num(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t redirecting_party_num_valid,
    const voice_num_with_id_type_v02* redirecting_party_num
)
{
    if (call_obj_ptr && redirecting_party_num_valid && NULL != redirecting_party_num)
    {
        call_obj_ptr->qmi_redirecting_party_num = *redirecting_party_num;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_REDIRECTING_PARTY_NUM_VALID);
    }
}
void cri_voice_call_obj_update_cri_call_state(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t cri_call_state_valid,
    const cri_voice_call_state_type cri_call_state
)
{
    if (call_obj_ptr && cri_call_state_valid)
    {
        call_obj_ptr->cri_call_state = cri_call_state;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_CRI_CALL_STATE_VALID);
    }
}

void cri_voice_call_obj_update_audio_attrib(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t audio_attrib_valid,
    const voice_call_attributes_type_v02 *audio_attrib
)
{
    if(call_obj_ptr && audio_attrib_valid && NULL != audio_attrib)
    {
        call_obj_ptr->qmi_audio_attrib = audio_attrib->call_attributes;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_AUDIO_ATTR_VALID);
    }
}

void cri_voice_call_obj_update_video_attrib(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t video_attrib_valid,
    const voice_call_attributes_type_v02 *video_attrib
)
{
    if(call_obj_ptr && video_attrib_valid && NULL != video_attrib)
    {
        call_obj_ptr->qmi_video_attrib = video_attrib->call_attributes;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_VIDEO_ATTR_VALID);
        cri_voice_call_obj_update_call_type_domain(call_obj_ptr);
    }
}

void cri_voice_call_obj_update_is_srvcc(
    cri_voice_call_obj_type* call_obj_ptr,
    uint8_t is_srvcc_valid,
    const voice_is_srvcc_call_with_id_type_v02 *is_srvcc
)
{
    if (call_obj_ptr && is_srvcc_valid && NULL != is_srvcc)
    {
        call_obj_ptr->qmi_is_srvcc = *is_srvcc;
        cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                        CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_IS_SRVCC_VALID);
    }
}

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
)
{
    if (call_obj_ptr)
    {
        if (call_id_valid)
        {
            call_obj_ptr->qmi_remote_party_number.call_id = call_id;
        }

        if (number_pi_valid)
        {
            call_obj_ptr->qmi_remote_party_number.number_pi = number_pi;
        }

        if (call_num_valid)
        {
            cri_voice_utils_call_num_copy_with_toa_check(
                call_obj_ptr->qmi_remote_party_number.number,
                sizeof(call_obj_ptr->qmi_remote_party_number.number),
                call_num,
                call_num_len,
                num_type
            );
        }

        if (number_pi_valid || call_num_valid)
        {
            cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                            CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NUMBER_VALID);
        }
    }

}

void cri_voice_call_obj_update_remote_party_name_by_each_fields(
    cri_voice_call_obj_type* call_obj_ptr,
    boolean name_pi_valid,
    pi_name_enum_v02 name_pi,
    boolean call_name_valid,
    const char* call_name,
    uint32_t call_name_len
)
{
    if (call_obj_ptr)
    {
        if (name_pi_valid)
        {
            call_obj_ptr->cri_remote_party_name.name_pi = name_pi;
        }

        if (call_name_valid)
        {
            if (call_name_len > sizeof(call_obj_ptr->cri_remote_party_name.name) - 1)
            {
                call_name_len = sizeof(call_obj_ptr->cri_remote_party_name.name) - 1;
            }
            memcpy(call_obj_ptr->cri_remote_party_name.name, call_name, call_name_len);
            call_obj_ptr->cri_remote_party_name.name[call_name_len] = 0;
            call_obj_ptr->cri_remote_party_name.name_len = call_name_len;
        }


        if (name_pi_valid || call_name_valid)
        {
            cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                            CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NAME_VALID);
        }
    }
}

void cri_voice_call_obj_set_hlos_data_and_deleter(
    cri_voice_call_obj_type* call_obj_ptr,
    void* user_data,
    hlos_user_data_deleter_type hlos_user_data_deleter
)
{
    if (call_obj_ptr)
    {
        call_obj_ptr->hlos_user_data = user_data;
        call_obj_ptr->hlos_user_data_deleter_valid = TRUE;
        call_obj_ptr->hlos_user_data_deleter = hlos_user_data_deleter;
    }
}

void cri_voice_call_obj_set_ps_cs_call_bit(
    cri_voice_call_obj_type *call_obj_ptr,
    const voice_call_info2_type_v02* qmi_voice_call_info_ptr
)
{
    call_mode_enum_v02 call_mode;
    call_type_enum_v02 call_type;

    if (NULL == call_obj_ptr)
    {
        QCRIL_LOG_ERROR("call_info_entry is NULL");
    }
    else
    {
        if (NULL != qmi_voice_call_info_ptr)
        {
            call_type = qmi_voice_call_info_ptr->call_type;
            call_mode = qmi_voice_call_info_ptr->mode;
        }
        else
        {
            call_type = call_obj_ptr->qmi_voice_scv_info.call_type;
            call_mode = call_obj_ptr->qmi_voice_scv_info.mode;
        }

        if ( CALL_TYPE_VOICE_IP_V02 == call_type ||
             CALL_TYPE_VT_V02 == call_type ||
             CALL_TYPE_EMERGENCY_IP_V02 == call_type )
        {
            if ( (cri_voice_call_obj_is_call_bit_unset(call_obj_ptr,
                                                       CRI_VOICE_CALL_OBJ_BIT_FIELD_AUTO_DOMAIN)) ||
                 CALL_MODE_NO_SRV_V02 != call_mode )
            {
                // if the call type is a PS call, and we either get a determined call_mode,
                // or it is not a dialed call on auto domain, set it as a PS call
                QCRIL_LOG_INFO("set the call as a PS call");
                cri_voice_call_obj_unset_call_bit(call_obj_ptr,
                                                  CRI_VOICE_CALL_OBJ_BIT_FIELD_AUTO_DOMAIN);
                cri_voice_call_obj_unset_call_bit(call_obj_ptr,
                                                  CRI_VOICE_CALL_OBJ_BIT_FIELD_CS_DOMAIN);
                cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                                CRI_VOICE_CALL_OBJ_BIT_FIELD_PS_DOMAIN);
            }
        }
        else if ( CALL_MODE_NO_SRV_V02 != call_mode &&
                  CALL_MODE_UNKNOWN_V02 != call_mode &&
                  CALL_MODE_LTE_V02 != call_mode )
        {
            // if the call type is a CS call and the call_mode is determined(not no_srv),
            // set it as a CS call
            QCRIL_LOG_INFO("set the call as a CS call");
            cri_voice_call_obj_unset_call_bit(call_obj_ptr,
                                              CRI_VOICE_CALL_OBJ_BIT_FIELD_AUTO_DOMAIN);
            cri_voice_call_obj_unset_call_bit(call_obj_ptr,
                                              CRI_VOICE_CALL_OBJ_BIT_FIELD_PS_DOMAIN);
            cri_voice_call_obj_set_call_bit(call_obj_ptr,
                                            CRI_VOICE_CALL_OBJ_BIT_FIELD_CS_DOMAIN);
        }
    }
}


