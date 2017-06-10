/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_ind_hdlr.h"
#include "cri_voice_core.h"
#include "cri_rule_handler.h"
//#include "qcril_cm_ss.h"
static void cri_voice_ind_hdlr_update_remote_party_number_by_info_rec_ind(
    cri_voice_call_obj_type *call_obj_ptr,
    const voice_info_rec_ind_msg_v02* info_rec_ind_ptr
);
static void cri_voice_ind_hdlr_update_remote_party_name_by_info_rec_ind(
    cri_voice_call_obj_type *call_obj_ptr,
    const voice_info_rec_ind_msg_v02* info_rec_ind_ptr
);
static void cri_voice_ind_hdlr_update_1x_num_pending_as_needed(cri_voice_call_obj_type* call_obj_ptr);
static void cri_voice_ind_hdlr_remove_1x_num_pending_as_needed(cri_voice_call_obj_type* call_obj_ptr);
static void cri_voice_ind_hdlr_add_1x_num_pending_as_needed(cri_voice_call_obj_type* call_obj_ptr);

void cri_voice_ind_hdlr_all_call_status_ind(
    int qmi_service_client_id,
    voice_all_call_status_ind_msg_v02* call_status_ind_ptr
)
{
    unsigned int i,j;
    cri_voice_call_obj_type * call_obj_ptr = NULL;

    voice_call_info2_type_v02*                  iter_qmi_call_info;
    uint8_t                                     iter_call_id;
    uint8_t                                     remote_party_number_valid;
    voice_remote_party_number2_type_v02*        remote_party_number;
    uint8_t                                     remote_party_name_valid;
    voice_remote_party_name2_type_v02*          remote_party_name;
    uint8_t                                     alerting_type_valid;
    voice_alerting_type_type_v02*               alerting_type;
    uint8_t                                     srv_opt_valid;
    voice_srv_opt_type_v02*                     srv_opt;
    uint8_t                                     call_end_reason_valid;
    voice_call_end_reason_type_v02*             call_end_reason;
    uint8_t                                     alpha_id_valid;
    voice_alpha_ident_with_id_type_v02*         alpha_id;
    uint8_t                                     conn_party_num_valid;
    voice_conn_num_with_id_type_v02*            conn_party_num;
    uint8_t                                     diagnostic_info_valid;
    voice_diagnostic_info_with_id_type_v02*     diagnostic_info;
    uint8_t                                     called_party_num_valid;
    voice_num_with_id_type_v02*                 called_party_num;
    uint8_t                                     redirecting_party_num_valid;
    voice_num_with_id_type_v02*                 redirecting_party_num;
    uint8_t                                     cri_call_state_valid;
    cri_voice_call_state_type                   cri_call_state;
    uint8_t                                     audio_attrib_valid;
    voice_call_attributes_type_v02              *audio_attrib;
    uint8_t                                     video_attrib_valid;
    voice_call_attributes_type_v02              *video_attrib;
    uint8_t                                     is_srvcc_valid;
    voice_is_srvcc_call_with_id_type_v02        *is_srvcc;

    char                                        log_essence[ QCRIL_MAX_LOG_MSG_SIZE ];
    char                                        log_addon[ QCRIL_MAX_LOG_MSG_SIZE ];

    QCRIL_LOG_FUNC_ENTRY();
    snprintf( log_essence, QCRIL_MAX_LOG_MSG_SIZE, "RILVIMS: update" );

    cri_voice_cache_type *call_info = cri_voice_core_get_call_info();
    util_list_info_type *call_list = cri_voice_cache_get_call_list(call_info);

    if( NULL != call_status_ind_ptr && NULL != call_list )
    {
        snprintf( log_addon, QCRIL_MAX_LOG_MSG_SIZE, " %d calls", call_status_ind_ptr->call_info_len );
        strlcat( log_essence, log_addon, sizeof( log_essence ) );

        for ( i = 0; i < call_status_ind_ptr->call_info_len; i++ )
        {
            // call info
            iter_qmi_call_info = &call_status_ind_ptr->call_info[i];
            iter_call_id   = iter_qmi_call_info->call_id;

            call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(call_list, iter_call_id);

            if ( call_obj_ptr )
            {
                QCRIL_LOG_INFO(
                          "[c-c-id: %d, q-c-id %d, c-s %d -> %d]",
                          (int)call_obj_ptr->cri_call_id,
                          (int)iter_qmi_call_info->call_id,
                          (int)call_obj_ptr->qmi_voice_scv_info.call_state,
                          (int)iter_qmi_call_info->call_state
                         );
            }
            else
            {
                QCRIL_LOG_INFO(
                          "[uncached qmi call id %d, call state %d]",
                          (int)iter_qmi_call_info->call_id,
                          (int)iter_qmi_call_info->call_state );
            }

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

            // cri call state
            cri_call_state_valid = FALSE;
            cri_call_state = CRI_VOICE_INVALID_ENUM_VAL;

            QCRIL_LOG_INFO("call state %d, IsMT=%d", iter_qmi_call_info->call_state, iter_qmi_call_info->direction);
            switch ( iter_qmi_call_info->call_state )
            {
                case CALL_STATE_INCOMING_V02:
                    QCRIL_LOG_INFO("call state INCOMING for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_INCOMING;

                    if ( NULL == call_obj_ptr )
                    { // fresh
                        call_obj_ptr = cri_voice_call_list_add_new_call_object(call_list, iter_call_id, TRUE, CRI_VOICE_CALL_OBJ_BIT_FIELD_NONE);
                    }
                    break; // end of case CALL_STATE_INCOMING_V02

                case CALL_STATE_ALERTING_V02:
                    QCRIL_LOG_ESSENTIAL("call state ALERTING for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_ALERTING;
                    break; // end of case CALL_STATE_ALERTING_V02

                case CALL_STATE_CONVERSATION_V02:
                    QCRIL_LOG_ESSENTIAL("call state CONVERSATION for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_ACTIVE;

                    if ( ( NULL == call_obj_ptr ) && ( TRUE == is_srvcc_valid ) && ( NULL != is_srvcc ) && ( TRUE == is_srvcc->is_srvcc_call ) )
                    { // fresh creation of call_obj_ptr in case of SRVCC call
                        call_obj_ptr = cri_voice_call_list_add_new_call_object(call_list,iter_call_id, TRUE, CRI_VOICE_CALL_OBJ_BIT_FIELD_NONE);
                    }
                    break; // end of case CALL_STATE_CONVERSATION_V02

                case CALL_STATE_END_V02:
                    QCRIL_LOG_ESSENTIAL("call state END for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_END;
                    if ( call_obj_ptr )
                    {
                        QCRIL_LOG_INFO( "call mode %d, call type %d, call got connected %d",
                                        (int)iter_qmi_call_info->mode,
                                        (int)iter_qmi_call_info->call_type,
                                        (int) cri_voice_call_obj_is_call_bit_set(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_CALL_GOT_CONNECTED) );
                    }
                    //qcril_qmi_nas_voice_move_device_to_lpm_after_emer_call_conditionally();

                    break; // end of case CALL_STATE_END_V02

                case CALL_STATE_ORIGINATING_V02:
                    QCRIL_LOG_ESSENTIAL("call state ORIGINATING for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_DIALING;

                    if ( NULL == call_obj_ptr )
                    { // ghost call
                        // for voip conf, do not set hlos id
                        call_obj_ptr = cri_voice_call_list_add_new_call_object(call_list, iter_qmi_call_info->call_id, TRUE, CRI_VOICE_CALL_OBJ_BIT_FIELD_NONE);
                    }
                    break; // end of case CALL_STATE_ORIGINATING_V02

                case CALL_STATE_DISCONNECTING_V02:
                    QCRIL_LOG_ESSENTIAL("call state DISCONNECTING for conn id %d", iter_qmi_call_info->call_id);
                    break; // end of case CALL_STATE_DISCONNECTING_V02

                case CALL_STATE_WAITING_V02:
                    QCRIL_LOG_ESSENTIAL("call state WAITING for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_WAITING;

                    if ( NULL == call_obj_ptr )
                    { // fresh
                        call_obj_ptr = cri_voice_call_list_add_new_call_object(call_list, iter_qmi_call_info->call_id, TRUE, CRI_VOICE_CALL_OBJ_BIT_FIELD_NONE);
                    }
                    break; // end of case CALL_STATE_WAITING_V02

                case CALL_STATE_HOLD_V02:
                    QCRIL_LOG_ESSENTIAL("call state HOLD for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_HOLDING;
                    break; // end of case CALL_STATE_HOLD_V02

                case CALL_STATE_CC_IN_PROGRESS_V02:
                    QCRIL_LOG_ESSENTIAL("call state CC IN PROGRESS for conn id %d", iter_qmi_call_info->call_id);
                    break; // end of case CALL_STATE_CC_IN_PROGRESS_V02

                case CALL_STATE_SETUP_V02:
                    QCRIL_LOG_ESSENTIAL("call state SETUP for conn id %d", iter_qmi_call_info->call_id);
                    cri_call_state_valid = TRUE;
                    cri_call_state = CRI_VOICE_CALL_STATE_SETUP;
                    if ( !cri_voice_cache_get_all_call_supressed(call_info) )
                    {
                        if ( NULL == call_obj_ptr )
                        { // fresh
                            call_obj_ptr = cri_voice_call_list_add_new_call_object(call_list, iter_qmi_call_info->call_id, TRUE, CRI_VOICE_CALL_OBJ_BIT_FIELD_NONE );
                        }
                    }
                    break; // end of case CALL_STATE_SETUP_V02

                default:
                    QCRIL_LOG_ESSENTIAL("unexpected call state(%d)  for conn id %d", iter_qmi_call_info->call_state, iter_qmi_call_info->call_id);
                    call_obj_ptr = NULL;
                    break;
            }
            if ( call_obj_ptr )
            {
                cri_voice_call_obj_update_call_obj( call_obj_ptr, iter_qmi_call_info,
                                                     remote_party_number_valid, remote_party_number,
                                                     remote_party_name_valid, remote_party_name,
                                                     alerting_type_valid, alerting_type,
                                                     srv_opt_valid, srv_opt,
                                                     call_end_reason_valid, call_end_reason,
                                                     alpha_id_valid, alpha_id,
                                                     conn_party_num_valid, conn_party_num,
                                                     diagnostic_info_valid, diagnostic_info,
                                                     called_party_num_valid, called_party_num,
                                                     redirecting_party_num_valid, redirecting_party_num,
                                                     cri_call_state_valid, cri_call_state,
                                                     audio_attrib_valid, audio_attrib,
                                                     video_attrib_valid, video_attrib,
                                                     is_srvcc_valid, is_srvcc
                                                    );

                cri_voice_ind_hdlr_update_1x_num_pending_as_needed(call_obj_ptr);
            }
            cri_rule_handler_rule_check(NIL, QMI_ERR_NONE_V01, NULL);

            if ( NULL != call_obj_ptr && cri_voice_call_obj_is_ended(call_obj_ptr))
            { // qmi call id will no longer be valid for this call object
              call_obj_ptr->qmi_call_id = CRI_VOICE_INVALID_CALL_ID;
            }
        }

        QCRIL_LOG_INFO("qmi_service_client_id: %d", qmi_service_client_id );
        hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
        if(hlos_ind_cb_func_ptr)
        {
            QCRIL_LOG_INFO("called");
            hlos_ind_cb_func_ptr(CRI_VOICE_CALL_STATE_CHANGED_IND, NULL, NIL);
        }

        cri_voice_call_list_dump(call_list);
    }

    QCRIL_LOG_ESSENTIAL ( log_essence );

    QCRIL_LOG_FUNC_RETURN();
} // cri_voice_ind_hdlr_all_call_status_ind

void cri_voice_ind_hdlr_info_rec_ind(
    int qmi_service_client_id,
    const voice_info_rec_ind_msg_v02* info_rec_ind_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();

    if (info_rec_ind_ptr)
    {
        if (CALL_WAITING_NEW_CALL_V02 != info_rec_ind_ptr->call_waiting)
        {
            // get or create a call obj
            cri_voice_call_obj_type *call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(
                                                        cri_voice_core_get_call_list(),
                                                        info_rec_ind_ptr->call_id );

            if (call_obj_ptr && 254 != info_rec_ind_ptr->call_id)
            {
                // fresh, this should be an incoming call
                // and the info_rec_indcomes before all_call_status_ind
                // Create a call entry only if call_id is not 254,
                // which is used by modem to indicate a unknown call type.
                call_obj_ptr = cri_voice_call_list_add_new_call_object(
                                   cri_voice_core_get_call_list(),
                                   info_rec_ind_ptr->call_id,
                                   TRUE,
                                   NIL );
                if(call_obj_ptr)
                {
                    call_obj_ptr->cri_call_state = CRI_VOICE_CALL_STATE_INCOMING;
                    cri_voice_call_obj_set_call_bit(
                        call_obj_ptr,
                        CRI_VOICE_CALL_OBJ_BIT_FIELD_CRI_CALL_STATE_VALID
                    );
                }
            }

            if (call_obj_ptr)
            {
                cri_voice_ind_hdlr_update_remote_party_number_by_info_rec_ind(
                    call_obj_ptr,
                    info_rec_ind_ptr
                );
                cri_voice_ind_hdlr_remove_1x_num_pending_as_needed(call_obj_ptr);

                cri_voice_ind_hdlr_update_remote_party_name_by_info_rec_ind(
                    call_obj_ptr,
                    info_rec_ind_ptr
                );
            }

            cri_voice_ind_hdlr_remove_1x_num_pending_as_needed(call_obj_ptr);

            QCRIL_LOG_INFO("qmi_service_client_id: %d", qmi_service_client_id );
            hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
            if (hlos_ind_cb_func_ptr)
            {
                hlos_ind_cb_func_ptr(
                    CRI_VOICE_INFO_REC_IND,
                    (void*)info_rec_ind_ptr,
                                 sizeof(*info_rec_ind_ptr));
                hlos_ind_cb_func_ptr(CRI_VOICE_CALL_STATE_CHANGED_IND, NULL, NIL);
            }

        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

void cri_voice_ind_hdlr_conference_info_ind(
    int qmi_service_client_id,
    const voice_conference_info_ind_msg_v02* conf_info_ind_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();
    do
    {
        if (NULL == conf_info_ind_ptr)
        {
            QCRIL_LOG_ERROR("conf_info_ind_ptr is NULL");
            break;
        }

        QCRIL_LOG_INFO( "seq: %d, total_size_valid: %d, total_size: %d, conference_xml_len: %d",
                        conf_info_ind_ptr->sequence,
                        conf_info_ind_ptr->total_size_valid,
                        conf_info_ind_ptr->total_size,
                        conf_info_ind_ptr->conference_xml_len );

        cri_voice_cache_type *call_info_ptr = cri_voice_core_get_call_info();
        cri_voice_cache_conf_info_type *conf_info_ptr =
            cri_voice_cache_get_conf_info(call_info_ptr);

        if (NULL == conf_info_ptr)
        {
            QCRIL_LOG_ERROR("conf_info_ptr is NULL");
            break;
        }

        if (0 == conf_info_ind_ptr->sequence)
        {
            if (conf_info_ind_ptr->total_size_valid)
            {
                conf_info_ptr->total_size = conf_info_ind_ptr->total_size;
                conf_info_ptr->filled_size = 0;
                if (conf_info_ptr->buffer)
                {
                    QCRIL_LOG_DEBUG("qcril_qmi_voice_info.conf_xml.buffer is not freed unexpectedly");
                    util_memory_free((void**) &conf_info_ptr->buffer);
                }

                conf_info_ptr->buffer = util_memory_alloc(conf_info_ptr->total_size);
                if (NULL == conf_info_ptr->buffer)
                {
                    QCRIL_LOG_ERROR("malloc failed");
                    cri_voice_cache_reset_conf_info(call_info_ptr);
                    break;
                }
            }
            else
            {
                QCRIL_LOG_ERROR("no total size in the first sequence indication");
                break;
            }
        }

        if (conf_info_ptr->filled_size + conf_info_ind_ptr->conference_xml_len > conf_info_ptr->total_size)
        {
            QCRIL_LOG_ERROR("filled_size (%d) + new conference_xml_len (%d) > total_size (%d)",
                            conf_info_ptr->filled_size, conf_info_ind_ptr->conference_xml_len, conf_info_ptr->total_size);
            cri_voice_cache_reset_conf_info(call_info_ptr);
            break;
        }

        if (conf_info_ind_ptr->sequence != conf_info_ptr->last_sequence_number+1)
        {
            QCRIL_LOG_ERROR("sequence out of order! new msg seq: %d, last_seq: %d",
                            conf_info_ind_ptr->sequence, conf_info_ptr->last_sequence_number);
            cri_voice_cache_reset_conf_info(call_info_ptr);
            break;
        }

        if (NULL == conf_info_ptr->buffer)
        {
            QCRIL_LOG_ERROR("conf_info_ptr->buffer is NULL");
            break;
        }

        memcpy( &(conf_info_ptr->buffer[conf_info_ptr->filled_size]),
                conf_info_ind_ptr->conference_xml, conf_info_ind_ptr->conference_xml_len );
        conf_info_ptr->filled_size += conf_info_ind_ptr->conference_xml_len;
        conf_info_ptr->last_sequence_number = conf_info_ind_ptr->sequence;
#if 0
        if (conf_info_ptr->filled_size == conf_info_ptr->total_size)
        {
            Ims__ConfInfo conf_info = IMS__CONF_INFO__INIT;
            conf_info.has_conf_info_uri = TRUE;
            conf_info.conf_info_uri.len = conf_info_ptr->total_size;

            cri_voice_binary_data conf_binary_data;
            conf_binary_data.len = conf_info_ptr->total_size;
            conf_binary_data.data = conf_info_ptr->buffer;

            hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
            if (hlos_ind_cb_func_ptr)
            {
                hlos_ind_cb_func_ptr(CRI_VOICE_VOICE_CONFERENCE_INFO_IND,
                                     &conf_binary_data,
                                     sizeof(conf_binary_data));
            }

            cri_voice_cache_reset_conf_info(call_info_ptr);
        }
#endif
    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN();
}

void cri_voice_ind_hdlr_otasp_status_ind(
    int qmi_service_client_id,
    const voice_otasp_status_ind_msg_v02* otasp_status_ind_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        if(NULL == otasp_status_ind_ptr)
        {
            break;
        }

        hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
        if (NULL == hlos_ind_cb_func_ptr)
        {
            break;
        }

        QCRIL_LOG_DEBUG( "QCRIL_EVT_QMI_VOICE_OTASP_STATUS_IND status = %d for conn id %d",
                         otasp_status_ind_ptr->otasp_status_info.otasp_status,
                         otasp_status_ind_ptr->otasp_status_info.call_id );

        cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(
                                                    cri_voice_core_get_call_list(),
                                                    otasp_status_ind_ptr->otasp_status_info.call_id);

        if ( call_obj_ptr )
        {
            call_obj_ptr->qmi_otasp_status = otasp_status_ind_ptr->otasp_status_info.otasp_status;
            cri_voice_call_obj_set_call_bit(
                call_obj_ptr,
                CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_OTASP_STATUS_VALID );
            hlos_ind_cb_func_ptr(CRI_VOICE_CALL_STATE_CHANGED_IND, NULL, NIL);
        }

        hlos_ind_cb_func_ptr(
            CRI_VOICE_VOICE_OTASP_STATUS_IND,
            (void*)otasp_status_ind_ptr,
            sizeof(*otasp_status_ind_ptr) );
    } while (FALSE);

    QCRIL_LOG_FUNC_RETURN();
} /* qcril_qmi_voice_otasp_status_ind_hdlr */

void cri_voice_ind_hdlr_privacy_ind(
    int qmi_service_client_id,
    const voice_privacy_ind_msg_v02* privacy_ind_ptr
)
{
    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        if(NULL == privacy_ind_ptr)
        {
            break;
        }

        hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
        if (NULL == hlos_ind_cb_func_ptr)
        {
            break;
        }

        QCRIL_LOG_INFO("Privacy indication received with privacy %d for conn id %d",
                       privacy_ind_ptr->voice_privacy_info.voice_privacy,
                       privacy_ind_ptr->voice_privacy_info.call_id );

        cri_voice_call_obj_type* call_obj_ptr = cri_voice_call_list_find_by_qmi_call_id(
                                                    cri_voice_core_get_call_list(),
                                                    privacy_ind_ptr->voice_privacy_info.call_id);

        if ( call_obj_ptr )
        {
            call_obj_ptr->qmi_voice_privacy = privacy_ind_ptr->voice_privacy_info.voice_privacy;
            cri_voice_call_obj_set_call_bit(
                call_obj_ptr,
                CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_VOICE_PRIVACY_VALID );
            hlos_ind_cb_func_ptr(CRI_VOICE_CALL_STATE_CHANGED_IND, NULL, NIL);
        }
    } while (FALSE);
    QCRIL_LOG_FUNC_RETURN();
}

void cri_voice_ind_hdlr_ext_brst_intl_ind(
    int qmi_service_client_id,
    const voice_ext_brst_intl_ind_msg_v02* ext_brst_intl__ind_ptr
)
{
    hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);

    if (NULL != hlos_ind_cb_func_ptr)
    {
        hlos_ind_cb_func_ptr(
            CRI_VOICE_VOICE_EXT_BRST_INTL_IND,
            (void*) ext_brst_intl__ind_ptr,
            sizeof(*ext_brst_intl__ind_ptr) );
    }
}

void cri_voice_ind_hdlr_sups_notification_ind(
    int qmi_service_client_id,
    const voice_sups_notification_ind_msg_v02* sups_notification_ind_ptr
)
{
    hlos_ind_cb_type hlos_ind_cb_func_ptr = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);

    if (NULL != hlos_ind_cb_func_ptr)
    {
        hlos_ind_cb_func_ptr(
            CRI_VOICE_VOICE_SUPS_NOTIFICATION_IND,
            (void*) sups_notification_ind_ptr,
            sizeof(*sups_notification_ind_ptr) );
    }
}

void cri_voice_ind_hdlr_update_remote_party_number_by_info_rec_ind(
    cri_voice_call_obj_type *call_obj_ptr,
    const voice_info_rec_ind_msg_v02* info_rec_ind_ptr
)
{
    if (call_obj_ptr && info_rec_ind_ptr)
    {
        boolean call_id_valid = TRUE;
        uint8_t call_id = info_rec_ind_ptr->call_id;
        boolean number_pi_valid = FALSE;
        pi_num_enum_v02 number_pi;
        boolean call_num_valid = FALSE;
        const char* call_num;
        uint32_t call_num_len;
        voice_num_type_enum_v02 num_type;

        if (info_rec_ind_ptr->calling_party_info_valid)
        {
            number_pi_valid = TRUE;
            number_pi = info_rec_ind_ptr->calling_party_info.pi;

            call_num_valid = TRUE;
            call_num = info_rec_ind_ptr->calling_party_info.num;
            call_num_len = info_rec_ind_ptr->calling_party_info.num_len;
            num_type = info_rec_ind_ptr->calling_party_info.num_type;
        }
        else if (info_rec_ind_ptr->caller_id_info_valid)
        {
            number_pi_valid = TRUE;
            number_pi = info_rec_ind_ptr->caller_id_info.pi;

            call_num_valid = TRUE;
            call_num = info_rec_ind_ptr->caller_id_info.caller_id;
            call_num_len = info_rec_ind_ptr->caller_id_info.caller_id_len;
            num_type = QMI_VOICE_NUM_TYPE_UNKNOWN_V02;
        }

        if (info_rec_ind_ptr->clir_cause_valid)
        {
            if((!info_rec_ind_ptr->calling_party_info_valid) && (info_rec_ind_ptr->clir_cause == 0))
            {
                number_pi_valid = TRUE;
                number_pi = PRESENTATION_NUM_NUM_UNAVAILABLE_V02; // default value
            }
            else
            {
                switch((uint8)info_rec_ind_ptr->clir_cause)
                {
                    case QMI_VOICE_CLIR_CAUSE_NO_CAUSE_V02:
                        number_pi_valid = TRUE;
                        number_pi = PRESENTATION_NUM_ALLOWED_V02;
                        break;
                    case QMI_VOICE_CLIR_CAUSE_REJECTED_BY_USER_V02:
                        number_pi_valid = TRUE;
                        number_pi = PRESENTATION_NUM_RESTRICTED_V02;
                        break;
                    case QMI_VOICE_CLIR_CAUSE_COIN_LINE_V02:
                        number_pi_valid = TRUE;
                        number_pi = PRESENTATION_NUM_PAYPHONE_V02;
                        break;
                    default:
                        number_pi_valid = TRUE;
                        number_pi = PRESENTATION_NUM_NUM_UNAVAILABLE_V02;
                        break;
                }
            }
            QCRIL_LOG_INFO("Mapped Clir=%d, PI=%d",
                           info_rec_ind_ptr->clir_cause,
                           number_pi);
        }

        cri_voice_call_obj_update_remote_party_number_by_each_fields(
            call_obj_ptr,
            call_id_valid,
            call_id,
            number_pi_valid,
            number_pi,
            call_num_valid,
            call_num,
            call_num_len,
            num_type
        );
    }
}

void cri_voice_ind_hdlr_update_remote_party_name_by_info_rec_ind(
    cri_voice_call_obj_type *call_obj_ptr,
    const voice_info_rec_ind_msg_v02* info_rec_ind_ptr
)
{
    if (call_obj_ptr && info_rec_ind_ptr && info_rec_ind_ptr->caller_name_valid)
    {
        boolean name_pi_valid = TRUE;
        pi_name_enum_v02 name_pi = PRESENTATION_NAME_PRESENTATION_ALLOWED_V02;

        boolean name_valid = TRUE;
        char utf_name[ CRI_VOICE_INTERCODING_BUF_LEN ];
//        uint32_t name_len = qcril_cm_ss_ascii_to_utf8(
//                                (unsigned char*) info_rec_ind_ptr->caller_name,
//                                strlen(info_rec_ind_ptr->caller_name),
//                                utf_name,
//                                sizeof(utf_name)
//                            );

        uint32_t name_len = 20;
        cri_voice_call_obj_update_remote_party_name_by_each_fields(
            call_obj_ptr,
            name_pi_valid,
            name_pi,
            name_valid,
            utf_name,
            name_len
        );
    }
}

void cri_voice_ind_hdlr_update_1x_num_pending_as_needed(cri_voice_call_obj_type* call_obj_ptr)
{
    if (call_obj_ptr)
    {
        if (CALL_STATE_INCOMING_V02 == call_obj_ptr->qmi_voice_scv_info.call_state)
        {
            cri_voice_ind_hdlr_add_1x_num_pending_as_needed(call_obj_ptr);
        }
        else
        {
            cri_voice_ind_hdlr_remove_1x_num_pending_as_needed(call_obj_ptr);
        }
    }
}

void cri_voice_ind_hdlr_add_1x_num_pending_as_needed(cri_voice_call_obj_type* call_obj_ptr)
{
    if (call_obj_ptr)
    {
        if ( ( CALL_TYPE_VOICE_V02 == call_obj_ptr->qmi_voice_scv_info.call_type ||
               CALL_TYPE_VOICE_FORCED_V02 == call_obj_ptr->qmi_voice_scv_info.call_type
             ) &&
             CALL_MODE_CDMA_V02 == call_obj_ptr->qmi_voice_scv_info.mode &&
             cri_voice_call_obj_is_call_bit_unset(
                 call_obj_ptr,
                 CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NUMBER_VALID ) &&
             cri_voice_call_obj_is_call_bit_unset(
                 call_obj_ptr,
                 CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING ) &&
             CALL_STATE_INCOMING_V02 == call_obj_ptr->qmi_voice_scv_info.call_state
           )
        {
            const struct timeval num_1x_wait_timeout = { 1 , 0 }; // 1 second
            util_timer_add(
                (struct timeval*) &num_1x_wait_timeout,
                NULL,
                NULL,
                NIL );
            cri_voice_call_obj_set_call_bit(
                call_obj_ptr,
                CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING );
        }
    }
}

void cri_voice_ind_hdlr_remove_1x_num_pending_as_needed(cri_voice_call_obj_type* call_obj_ptr)
{
    if (call_obj_ptr)
    {
        if ( ( cri_voice_call_obj_is_call_bit_set(
                 call_obj_ptr,
                 CRI_VOICE_CALL_OBJ_BIT_FIELD_QMI_R_PARTY_NUMBER_VALID ) ||
               CALL_STATE_CONVERSATION_V02 == call_obj_ptr->qmi_voice_scv_info.call_state ||
               CALL_STATE_END_V02 == call_obj_ptr->qmi_voice_scv_info.call_state
             ) &&
             cri_voice_call_obj_is_call_bit_set(
                 call_obj_ptr,
                 CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING )
           )
        {
            util_timer_cancel(call_obj_ptr->cri_1x_num_pending_timer_id);
            cri_voice_call_obj_unset_call_bit(
                call_obj_ptr,
                CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING );
        }
    }
}

static void cri_voice_pending_1x_num_timeout(
    void *rule_timedout_cb_data,
    size_t rule_timedout_cb_data_len
)
{
    QCRIL_LOG_FUNC_ENTRY();

    cri_voice_call_obj_type *call_obj_ptr =
        cri_voice_call_list_find_by_call_bit(
            cri_voice_core_get_call_list(),
            CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING );

    if (call_obj_ptr)
    {
        cri_voice_call_obj_unset_call_bit(call_obj_ptr, CRI_VOICE_CALL_OBJ_BIT_FIELD_1X_REMOTE_NUM_PENDING);
        hlos_ind_cb_type hlos_ind_cb_func_ptr =
            cri_core_retrieve_hlos_ind_cb(
                cri_voice_core_get_qmi_client_info()->qmi_voice_client_id);
        if(hlos_ind_cb_func_ptr)
        {
            hlos_ind_cb_func_ptr(CRI_VOICE_CALL_STATE_CHANGED_IND, NULL, NIL);
        }

    }
    else
    {
        QCRIL_LOG_DEBUG("did not find the call obj with 1X_REMOTE_NUM_PENDING");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qmi_ril_voice_pending_1x_num_timeout
