/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_dms_core.h"
#include "cri_dms_rules.h"
#include "cri_rule_handler.h"

extern int dms_client_id;

cri_core_error_type cri_dms_set_modem_request_handler( cri_core_context_type cri_core_context,
                                                    uint32_t opr_mode,
                                                    void *hlos_cb_data,
                                                    hlos_resp_cb_type hlos_resp_cb,
                                                    uint32_t *is_changed )
{
    qmi_error_type_v01 ret_val = QMI_ERR_NONE_V01;
    dms_set_operating_mode_req_msg_v01 operating_mode_req_msg;
    dms_set_operating_mode_resp_msg_v01 operating_mode_resp_msg;

    dms_get_operating_mode_resp_msg_v01 curr_opr_resp_msg;

    cri_dms_rules_generic_rule_data_type *cri_dms_rules_generic_rule_data;
    cri_rule_handler_user_rule_info_type  user_rule_info;

    UTIL_LOG_MSG("\n hlos_dms_set_modem_request_handler entry \n");

    memset(&operating_mode_req_msg, 0, sizeof(operating_mode_req_msg));
    memset(&operating_mode_resp_msg, 0, sizeof(operating_mode_resp_msg));
    memset(&curr_opr_resp_msg, 0, sizeof(curr_opr_resp_msg));
    cri_dms_rules_generic_rule_data = NULL;
    memset(&user_rule_info, 0, sizeof(user_rule_info));

    if ( QMI_ERR_NONE_V01 != cri_core_qmi_send_msg_sync( dms_client_id,
                                                        QMI_DMS_GET_OPERATING_MODE_REQ_V01,
                                                        NULL,
                                                        NIL,
                                                        &curr_opr_resp_msg,
                                                        sizeof(curr_opr_resp_msg),
                                                        CRI_CORE_MINIMAL_TIMEOUT )
       )
    {
        UTIL_LOG_MSG("DMS get modem status request failed!");
    }
    else
    {
        UTIL_LOG_MSG("DMS get modem status request error code: %d", curr_opr_resp_msg.resp.error);
    }

    if ( opr_mode != (uint32_t) curr_opr_resp_msg.operating_mode )
    {
        operating_mode_req_msg.operating_mode = opr_mode;

        cri_dms_rules_generic_rule_data = (cri_dms_rules_generic_rule_data_type*)
                                            util_memory_alloc(sizeof(*cri_dms_rules_generic_rule_data));

        if(cri_dms_rules_generic_rule_data)
        {
            cri_dms_rules_generic_rule_data->operating_mode = opr_mode;

            operating_mode_req_msg.operating_mode = opr_mode;

            user_rule_info.rule_data = (void *)cri_dms_rules_generic_rule_data;
            user_rule_info.rule_check_handler = cri_dms_rules_set_modem_rule_check_handler;
            user_rule_info.rule_data_free_handler = cri_dms_rules_generic_rule_data_free_handler;

            ret_val =  cri_core_qmi_send_msg_async(cri_core_context,
                                                    dms_client_id,
                                                    QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                                    &operating_mode_req_msg,
                                                    sizeof(operating_mode_req_msg),
                                                    sizeof(operating_mode_resp_msg),
                                                    hlos_cb_data,
                                                    hlos_resp_cb,
                                                    CRI_CORE_MAX_TIMEOUT,
                                                   &user_rule_info);
        }
        if(QMI_ERR_NONE_V01 != ret_val)
        {
            UTIL_LOG_MSG("DMS set modem status request failed!");
            *is_changed = FALSE;
        }
        else
        {
            UTIL_LOG_MSG("DMS set modem status request error code: %d", operating_mode_resp_msg.resp.error);
            *is_changed = TRUE;
        }

    }
    else
    {
        *is_changed = FALSE;
    }
    return CRI_ERR_NONE_V01;
}

cri_core_error_type cri_dms_get_modem_status_request_handler(dms_operating_mode_enum_v01 *info_ptr)
{
    dms_operating_mode_enum_v01 curr_opr_mode;
    dms_get_operating_mode_resp_msg_v01 operating_mode_resp_msg;

    if (!info_ptr)
    {
        return CRI_ERR_INTERNAL_V01;
    }

    memset(&operating_mode_resp_msg, 0, sizeof(operating_mode_resp_msg));

    if ( !cri_dms_utils_is_valid_operating_mode() )
    {
        if ( QMI_ERR_NONE_V01 != cri_core_qmi_send_msg_sync( dms_client_id,
                                                            QMI_DMS_GET_OPERATING_MODE_REQ_V01,
                                                            NULL,
                                                            NIL,
                                                            &operating_mode_resp_msg,
                                                            sizeof(operating_mode_resp_msg),
                                                            CRI_CORE_MINIMAL_TIMEOUT )
        )
        {
            UTIL_LOG_MSG("DMS get modem status request failed!");
        }
        else
        {
            UTIL_LOG_MSG("DMS get modem status request error code: %d", operating_mode_resp_msg.resp.error);
        }

        *info_ptr = operating_mode_resp_msg.operating_mode;
        cri_dms_utils_update_operating_mode(operating_mode_resp_msg.operating_mode);
    }
    else
    {
        cri_dms_utils_get_current_operating_mode(info_ptr);
    }
    return CRI_ERR_NONE_V01;
}


cri_core_error_type cri_dms_get_modem_software_version_request_handler(char *version)
{
    cri_core_error_type ret_val;
    dms_get_sw_version_resp_msg_v01 resp_msg;

    memset(&resp_msg, 0, sizeof(resp_msg));

    ret_val = cri_core_qmi_send_msg_sync(dms_client_id,
                                        QMI_DMS_GET_SW_VERSION_REQ_V01,
                                        NULL,
                                        NIL,
                                        &resp_msg,
                                        sizeof(resp_msg),
                                        CRI_CORE_MINIMAL_TIMEOUT );
    if(CRI_ERR_NONE_V01 != ret_val)
    {
        UTIL_LOG_MSG("DMS get software version request failed!");
    }
    else
    {
        UTIL_LOG_MSG("DMS get software version request error code: %d", resp_msg.resp.error);
        strlcpy(version,resp_msg.sw_version,sizeof(version));
    }

    return ret_val;
}


cri_core_error_type cri_dms_reset_request_handler()
{
    cri_core_error_type ret_val;
    dms_reset_resp_msg_v01 resp_msg;

    memset(&resp_msg, 0, sizeof(resp_msg));

    ret_val = cri_core_qmi_send_msg_sync(dms_client_id,
                                        QMI_DMS_RESET_REQ_V01,
                                        NULL,
                                        NIL,
                                        &resp_msg,
                                        sizeof(resp_msg),
                                        CRI_CORE_MINIMAL_TIMEOUT );
    if(CRI_ERR_NONE_V01 != ret_val)
    {
        UTIL_LOG_MSG("DMS reset request failed!");
    }
    else
    {
        UTIL_LOG_MSG("DMS reset request error code: %d", resp_msg.resp.error);
    }

    return ret_val;
}


cri_core_error_type cri_dms_get_hw_version_request_handler(char *version)
{
    cri_core_error_type ret_val;
    dms_get_device_hardware_rev_resp_msg_v01 resp_msg;

    memset(&resp_msg, 0, sizeof(resp_msg));

    ret_val = cri_core_qmi_send_msg_sync(dms_client_id,
                                        QMI_DMS_GET_DEVICE_HARDWARE_REV_REQ_V01,
                                        NULL,
                                        NIL,
                                        &resp_msg,
                                        sizeof(resp_msg),
                                        CRI_CORE_MINIMAL_TIMEOUT );
    if(CRI_ERR_NONE_V01 != ret_val)
    {
        UTIL_LOG_MSG("DMS reset request failed!");
    }
    else
    {
        UTIL_LOG_MSG("DMS reset request error code: %d", resp_msg.resp.error);
        strlcpy(version,resp_msg.hardware_rev,sizeof(version));
    }
    return ret_val;
}



cri_core_error_type cri_dms_core_unsol_ind_handler(int qmi_service_client_id,
                                                    unsigned long message_id,
                                                    void *ind_data,
                                                    int ind_data_len)
{

    hlos_ind_cb_type hlos_ind_cb;
    dms_event_report_ind_msg_v01 *dms_ind_data;

    switch(message_id)
    {
        case QMI_DMS_EVENT_REPORT_IND_V01:
            dms_ind_data = (dms_event_report_ind_msg_v01*)ind_data;
            if ( dms_ind_data->operating_mode_valid == TRUE )
            {
                cri_dms_utils_update_operating_mode(dms_ind_data->operating_mode);
                cri_rule_handler_rule_check(NIL,
                                            QMI_ERR_NONE_V01,
                                            NULL);
            }
            hlos_ind_cb = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
            if(hlos_ind_cb)
            {
                (*hlos_ind_cb) (message_id, ind_data, ind_data_len);
            }
            else
            {
                UTIL_LOG_MSG("No registered HLOS ind handler for DMS");
            }
        break;

    }
    return CRI_ERR_NONE_V01;
}


void cri_dms_core_async_resp_handler(int qmi_service_client_id,
                                    unsigned long message_id,
                                    void *resp_data,
                                    int resp_data_len,
                                    cri_core_context_type cri_core_context)
{

    if(resp_data && resp_data_len)
    {
        switch(message_id)
        {
        case QMI_DMS_SET_OPERATING_MODE_RESP_V01:
            cri_dms_set_modem_resp_handler(qmi_service_client_id,
                                            resp_data,
                                            cri_core_context);
        break;

        default:
        //no action
        break;

        }
    }

}


void cri_dms_set_modem_resp_handler(int qmi_service_client_id,
                                    dms_set_operating_mode_resp_msg_v01 *set_opr_mode_resp_msg,
                                    cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;

    cri_core_error = QMI_ERR_NONE_V01;

    if(set_opr_mode_resp_msg)
    {
    cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR,
                                                &set_opr_mode_resp_msg->resp);
                                                cri_rule_handler_rule_check(cri_core_context,
                                                cri_core_error,
                                                NULL);
    }

}



#if 0

cri_core_error_type cri_dms_get_sw_version_request_handler(dms_get_sw_version_resp_msg_v01 *qmi_response)
{

}


cri_core_error_type cri_dms_get_hw_version_request_handler(dms_get_device_hardware_rev_resp_msg_v01 *qmi_response)
{
}


cri_core_error_type cri_dms_reset_request_handler(dms_reset_resp_msg_v01 *qmi_response)
{
}



cri_core_error_type cri_dms_core_unsol_ind_handler(int qmi_service_client_id,
                                                 unsigned long message_id,
                                                 void *ind_data,
                                                 int ind_data_len)
{

}

#endif

